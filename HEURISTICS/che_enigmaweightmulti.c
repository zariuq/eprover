/*-----------------------------------------------------------------------

File  : che_enigmaweightmulti.c

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

#include <stdio.h>
#include <sys/types.h>
#include <dirent.h>

#include "che_enigmaweightmulti.h"

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

static void extweight_init_models(EnigmaWeightMultiParam_p data)
{
   DIR *dp;
   struct dirent *ep;
   struct model* model;
   DStr_p filename;
   
   data->models = PStackAlloc();
   
   dp = opendir(data->models_dir);
   if (!dp)
   {
      Error("EnigmaMulti: Can not open models directory: %s", FILE_ERROR, data->models_dir);
   }
   while ((ep = readdir(dp)) != NULL)
   {
      if (ep->d_type != DT_DIR || ep->d_name[0]=='.')
      {
         continue;
      }

      filename = DStrAlloc();
      DStrAppendStr(filename, data->models_dir);
      DStrAppendChar(filename, '/');
      DStrAppendStr(filename, ep->d_name);
      DStrAppendChar(filename, '/');
      DStrAppendStr(filename, "model.lin");
   
      model = load_model(DStrView(filename));
      if (!model) 
      { 
         Error("ENIGMA: Failed loading liblinear model: %s", FILE_ERROR, DStrView(filename)); 
      }

      DStrFree(filename);
      PStackPushP(data->models, model);
   }
}

static void extweight_init_conjecture(EnigmaWeightMultiParam_p data)
{
   Clause_p clause;
   Clause_p anchor;
   NumTree_p features = NULL;
   long len = 0;

   anchor = data->proofstate->axioms->anchor;
   for (clause=anchor->succ; clause!=anchor; clause=clause->succ)
   {
      if(ClauseQueryTPTPType(clause)==CPTypeNegConjecture) {
         len += FeaturesClauseExtend(&features, clause, data->enigmap);
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

   data->conj_features = conj_nodes;
   data->conj_features_count = len;
}

static void extweight_init(EnigmaWeightMultiParam_p data)
{
   if (data->models) 
   {
      return;
   }

   double start = GetTotalCPUTime();
   data->enigmap = EnigmapLoad(data->features_filename, data->ocb->sig);
   extweight_init_models(data);
   extweight_init_conjecture(data);

   fprintf(GlobalOut, 
      "# ENIGMA: Multi model '%s' loaded (%ld models, %ld features) in %.3f seconds\n", 
      data->models_dir, 
      data->models->current, 
      data->enigmap->feature_count,
      GetTotalCPUTime()-start);
}

/*---------------------------------------------------------------------*/
/*                         Exported Functions                          */
/*---------------------------------------------------------------------*/

EnigmaWeightMultiParam_p EnigmaWeightMultiParamAlloc(void)
{
   EnigmaWeightMultiParam_p res = EnigmaWeightMultiParamCellAlloc();

   res->models = NULL;
   res->enigmap = NULL;

   return res;
}

void EnigmaWeightMultiParamFree(EnigmaWeightMultiParam_p junk)
{
   free(junk->models_dir);
   free(junk->features_filename);
   junk->models = NULL; // TODO: free model & enigmap

   EnigmaWeightMultiParamCellFree(junk);
}
 
WFCB_p EnigmaWeightMultiParse(
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

   DStr_p str = DStrAlloc();
   DStrAppendStr(str, d_enigma);
   DStrAppendStr(str, "/");
   DStrAppendStr(str, d_prefix);
   char* models_dir = SecureStrdup(DStrView(str));
   DStrFree(str);
   
   str = DStrAlloc();
   DStrAppendStr(str, models_dir);
   DStrAppendStr(str, "/");
   DStrAppendStr(str, "enigma.map");
   char* features_filename = SecureStrdup(DStrView(str));
   DStrFree(str);

   //fprintf(GlobalOut, "ENIGMA: MODEL: %s\n", model_filename);
   //fprintf(GlobalOut, "ENIGMA: FEATURES: %s\n", features_filename);

   free(d_prefix);

   return EnigmaWeightMultiInit(
      prio_fun, 
      ocb,
      state,
      models_dir,
      features_filename,
      len_mult);
}

WFCB_p EnigmaWeightMultiInit(
   ClausePrioFun prio_fun, 
   OCB_p ocb,
   ProofState_p proofstate,
   char* models_dir,
   char* features_filename,
   double len_mult)
{
   EnigmaWeightMultiParam_p data = EnigmaWeightMultiParamAlloc();

   data->init_fun   = extweight_init;
   data->ocb        = ocb;
   data->proofstate = proofstate;
   
   data->models_dir = models_dir;
   data->features_filename = features_filename;
   data->len_mult = len_mult;
   
   return WFCBAlloc(
      EnigmaWeightMultiCompute, 
      prio_fun,
      EnigmaWeightMultiExit, 
      data);
}

double EnigmaWeightMultiCompute(void* data, Clause_p clause)
{
   static struct feature_node nodes[2048]; // TODO: dynamic alloc
   //static struct feature_node dense_nodes[2048]; // TODO: dynamic alloc

   EnigmaWeightMultiParam_p local;
   double res = 1.0;
   
   local = data;
   local->init_fun(data);

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
   //printf("\n");
   
   double avg = 0.0;
   for (i=0; i<local->models->current; i++)
   {  
      avg += predict(PStackElementP(local->models,i), nodes);
   }
   avg = avg / local->models->current;
   double clen = ClauseWeight(clause,1,1,1,1,1,false);
   res = (clen * local->len_mult) + avg;

   if (OutputLevel>=1) 
   {
      fprintf(GlobalOut, "# EnigmaMulti eval: %.2f (avg=%.2f): ", res, avg);
      ClausePrint(GlobalOut, clause, true);
      fprintf(GlobalOut, "\n");
   }

   return res;
}

void EnigmaWeightMultiExit(void* data)
{
   EnigmaWeightMultiParam_p junk = data;
   
   EnigmaWeightMultiParamFree(junk);
}

/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/

