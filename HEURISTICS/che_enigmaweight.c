/*-----------------------------------------------------------------------

File  : che_enigmaweight.c

Author: could be anyone

Contents
 
  Auto generated. Your comment goes here ;-).

  Copyright 2016 by the author.
  This code is released under the GNU General Public Licence.
  See the file COPYING in the main directory for details.
  Run "eprover -h" for contact information.

Changes

<1> Tue Mar  8 22:40:31 CET 2016
    New

-----------------------------------------------------------------------*/

#include "che_enigmaweight.h"


/*---------------------------------------------------------------------*/
/*                        Global Variables                             */
/*---------------------------------------------------------------------*/

/*---------------------------------------------------------------------*/
/*                      Forward Declarations                           */
/*---------------------------------------------------------------------*/

/*---------------------------------------------------------------------*/
/*                         Internal Functions                          */
/*---------------------------------------------------------------------*/

static struct feature_node conj_nodes[2048]; // TODO: dynamic alloc

static void extweight_init(EnigmaWeightParam_p data)
{
   if (data->linear_model) 
   {
      return;
   }

   data->linear_model = load_model(data->model_filename);
   if (!data->linear_model) 
   {
      Error("ENIGMA: Failed loading liblinear model: %s", FILE_ERROR,
         data->model_filename);
   }
   data->enigmap = EnigmapLoad(data->features_filename, data->ocb->sig);

   int len = 0;
   if (data->enigmap->version & EFConjecture)
   {
      Clause_p clause;
      Clause_p anchor;
      NumTree_p features = NULL;

      anchor = data->proofstate->axioms->anchor;
      for (clause=anchor->succ; clause!=anchor; clause=clause->succ)
      {
         if(ClauseQueryTPTPType(clause)==CPTypeNegConjecture) 
         {
            len += FeaturesClauseExtend(&features, clause, data->enigmap);
            FeaturesAddClauseStatic(&features, clause, data->enigmap, &len);
         }
      }

      if (len >= 2048) { Error("ENIGMA: Too many conjecture features!", OTHER_ERROR); } 
     
      //printf("CONJ FEATURES: ");
      int i = 0;
      while (features) 
      {
         NumTree_p cell = NumTreeExtractEntry(&features,NumTreeMinNode(features)->key);
         conj_nodes[i].index = cell->key + data->enigmap->feature_count;
         conj_nodes[i].value = (double)cell->val1.i_val;
         //printf("%d:%ld ", conj_nodes[i].index, cell->val1.i_val);
         i++;
         NumTreeCellFree(cell);
      }
      //printf("\n");
      conj_nodes[i].index = -1;
      assert(i==len);
   }

   data->conj_features = conj_nodes;
   data->conj_features_count = len;

   fprintf(GlobalOut, 
      "# ENIGMA: Model '%s' loaded (features: %ld; vector len: %d; version: %ld)\n", 
      data->model_filename, data->enigmap->feature_count, data->linear_model->nr_feature,
      data->enigmap->version);
}

/*---------------------------------------------------------------------*/
/*                         Exported Functions                          */
/*---------------------------------------------------------------------*/

EnigmaWeightParam_p EnigmaWeightParamAlloc(void)
{
   EnigmaWeightParam_p res = EnigmaWeightParamCellAlloc();

   res->linear_model = NULL;
   res->enigmap = NULL;

   return res;
}

void EnigmaWeightParamFree(EnigmaWeightParam_p junk)
{
   free(junk->model_filename);
   free(junk->features_filename);
   junk->linear_model = NULL; // TODO: free model & enigmap

   EnigmaWeightParamCellFree(junk);
}
 
WFCB_p EnigmaWeightParse(
   Scanner_p in,  
   OCB_p ocb, 
   ProofState_p state)
{   
   ClausePrioFun prio_fun;
   double len_mult;

   AcceptInpTok(in, OpenBracket);
   prio_fun = ParsePrioFun(in);
   AcceptInpTok(in, Comma);
   char* d_prefix = ParseFilename(in);
   AcceptInpTok(in, Comma);
   len_mult = ParseFloat(in);
   AcceptInpTok(in, CloseBracket);

   char* d_enigma = getenv("ENIGMA_ROOT");
   if (!d_enigma) {
      d_enigma = "Enigma";
   }

   DStr_p f_model = DStrAlloc();
   DStrAppendStr(f_model, d_enigma);
   DStrAppendStr(f_model, "/");
   DStrAppendStr(f_model, d_prefix);
   DStrAppendStr(f_model, "/");
   DStrAppendStr(f_model, "model.lin");
   char* model_filename = SecureStrdup(DStrView(f_model));
   DStrFree(f_model);

   DStr_p f_featmap = DStrAlloc();
   DStrAppendStr(f_featmap, d_enigma);
   DStrAppendStr(f_featmap, "/");
   DStrAppendStr(f_featmap, d_prefix);
   DStrAppendStr(f_featmap, "/");
   DStrAppendStr(f_featmap, "enigma.map");
   char* features_filename = SecureStrdup(DStrView(f_featmap));
   DStrFree(f_featmap);
  
   //fprintf(GlobalOut, "ENIGMA: MODEL: %s\n", model_filename);
   //fprintf(GlobalOut, "ENIGMA: FEATURES: %s\n", features_filename);

   free(d_prefix);

   return EnigmaWeightInit(
      prio_fun, 
      ocb,
      state,
      model_filename,
      features_filename,
      len_mult);
}

WFCB_p EnigmaWeightInit(
   ClausePrioFun prio_fun, 
   OCB_p ocb,
   ProofState_p proofstate,
   char* model_filename,
   char* features_filename,
   double len_mult)
{
   EnigmaWeightParam_p data = EnigmaWeightParamAlloc();

   data->init_fun   = extweight_init;
   data->ocb        = ocb;
   data->proofstate = proofstate;
   
   data->model_filename = model_filename;
   data->features_filename = features_filename;
   data->len_mult = len_mult;
   
   return WFCBAlloc(
      EnigmaWeightCompute, 
      prio_fun,
      EnigmaWeightExit, 
      data);
}

double EnigmaWeightCompute(void* data, Clause_p clause)
{
   static struct feature_node nodes[2048]; // TODO: dynamic alloc
   EnigmaWeightParam_p local;
   double res = 1.0;
   
   local = data;
   local->init_fun(data);

   long long start = GetUSecClock();
   int len = 0;
   NumTree_p features = FeaturesClauseCollect(clause, local->enigmap, &len);
   //printf("features count: %d\n", len);
      
   if (len+local->conj_features_count >= 2048) { Error("ENIGMA: Too many clause features!", OTHER_ERROR); }

   int i = 0;
   while (features) 
   {
      NumTree_p cell = NumTreeExtractEntry(&features,NumTreeMinNode(features)->key);
      //printf("%ld:%ld ", cell->key, cell->val1.i_val);
      nodes[i].index = cell->key;
      nodes[i].value = (double)cell->val1.i_val;
      i++;
      NumTreeCellFree(cell);
   }
   //printf("|");

   for (int j=0; j<local->conj_features_count; j++) 
   {
      nodes[i+j].index = local->conj_features[j].index;
      nodes[i+j].value = local->conj_features[j].value;
      //printf("%d:%.0f ", nodes[i+j].index, nodes[i+j].value);
   }
   nodes[i+local->conj_features_count].index = -1;
   int total = i+local->conj_features_count;

   // detect proofwatch version
   if (local->enigmap->version & EFProofWatch)
   {
      //printf("|");
      NumTree_p proof;
      PStack_p stack;
      int k = i + local->conj_features_count;
      int offset = local->enigmap->feature_count;
      if (local->enigmap->version & EFConjecture) 
      {
         offset += local->enigmap->feature_count;
      }

      stack = NumTreeTraverseInit(local->proofstate->wlcontrol->watch_progress);
      while((proof = NumTreeTraverseNext(stack)))
      {
         if (proof->val1.i_val == 0) 
         { 
            continue; 
         }
         NumTree_p len = NumTreeFind(&(local->proofstate->wlcontrol->proof_len), proof->key);
         if (!len)
         {
            Error("Watchlist: Unknown proof length of proof #%ld.  Should not happen!", 
               OTHER_ERROR, proof->key);
         }
         nodes[k].index = proof->key + offset;
         nodes[k].value = (double)proof->val1.i_val/len->val1.i_val;
         //printf("%d:%.3f ", nodes[k].index, nodes[k].value);
         k++;
         if (k >= 2048) { Error("ENIGMA: Too many proof watch features!", OTHER_ERROR); }
      }
      NumTreeTraverseExit(stack);
      nodes[k].index = -1;
      total = k;
   }
   //printf("\n");
   //printf("[duration] feature extract: %f.2 ms\n", (double)(GetUSecClock() - start)/ 1000.0);
   
   res = predict(local->linear_model, nodes);
   res = 1 + ((1.0 - res) * 10); // 0 maps to 11, 1 maps to 1
   //fprintf(GlobalOut, "+%0.2f ", res);
   double clen = ClauseWeight(clause,1,1,1,1,1,false);
   res = (clen * local->len_mult) + res;

   if (OutputLevel == 1) 
   {
      fprintf(GlobalOut, "=%.2f (lin,t=%.3fms,clen=%.1f,vlen=%d): ", res, (double)(GetUSecClock() - start)/ 1000.0, clen, total);
      ClausePrint(GlobalOut, clause, true);
      fprintf(GlobalOut, "\n");
   }
   
   return res;
}

void EnigmaWeightExit(void* data)
{
   EnigmaWeightParam_p junk = data;
   
   EnigmaWeightParamFree(junk);
}

/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/

