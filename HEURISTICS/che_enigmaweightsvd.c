/*-----------------------------------------------------------------------

File  : che_enigmaweightsvd.c

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

#include "che_enigmaweightsvd.h"


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

static void extweight_init(EnigmaWeightSvdParam_p data)
{
   if (data->linear_model) 
   {
      return;
   }

   data->linear_model = load_model(data->model_filename);
   if (!data->linear_model) { Error("ENIGMA: Failed loading liblinear model: %s", FILE_ERROR, data->model_filename); }

   data->enigmap = EnigmapLoad(data->features_filename, data->ocb->sig);

   data->svd_Ut = svdLoadDenseMatrix(data->svd_Ut_filename, SVD_F_DT);
   if (!data->svd_Ut) { Error("ENIGMA: Failed loading SVD matrix Ut: %s", FILE_ERROR, data->svd_Ut_filename); }
   
   int len = 0;
   data->svd_S = svdLoadDenseArray(data->svd_S_filename, &len, 0);
   if (!data->svd_S) { Error("ENIGMA: Failed loading SVD matrix S: %s", FILE_ERROR, data->svd_S_filename); }

   data->dense_features = SizeMalloc((len+1)*sizeof(struct feature_node));

   Clause_p clause;
   Clause_p anchor;
   NumTree_p features = NULL;

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

   fprintf(GlobalOut, 
      "# ENIGMA: SVD Model '%s' loaded (%ld features, Ut %ldx%ld)\n", 
      data->model_filename, data->enigmap->feature_count, 
      data->svd_Ut->rows, data->svd_Ut->cols);
}

/*---------------------------------------------------------------------*/
/*                         Exported Functions                          */
/*---------------------------------------------------------------------*/

EnigmaWeightSvdParam_p EnigmaWeightSvdParamAlloc(void)
{
   EnigmaWeightSvdParam_p res = EnigmaWeightSvdParamCellAlloc();

   res->linear_model = NULL;
   res->enigmap = NULL;

   return res;
}

void EnigmaWeightSvdParamFree(EnigmaWeightSvdParam_p junk)
{
   free(junk->model_filename);
   free(junk->features_filename);
   junk->linear_model = NULL; // TODO: free model & enigmap
   // TODO: free SVD stuff
   free(junk->dense_features);

   EnigmaWeightSvdParamCellFree(junk);
}
 
WFCB_p EnigmaWeightSvdParse(
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
   
   DStr_p f_name = DStrAlloc();
   DStrAppendStr(f_name, d_enigma);
   DStrAppendStr(f_name, "/");
   DStrAppendStr(f_name, d_prefix);
   DStrAppendStr(f_name, "/");
   DStrAppendStr(f_name, "svd-Ut");
   char* svd_Ut_filename = SecureStrdup(DStrView(f_name));
   DStrFree(f_name);
   
   f_name = DStrAlloc();
   DStrAppendStr(f_name, d_enigma);
   DStrAppendStr(f_name, "/");
   DStrAppendStr(f_name, d_prefix);
   DStrAppendStr(f_name, "/");
   DStrAppendStr(f_name, "svd-S");
   char* svd_S_filename = SecureStrdup(DStrView(f_name));
   DStrFree(f_name);
  
   //fprintf(GlobalOut, "ENIGMA: MODEL: %s\n", model_filename);
   //fprintf(GlobalOut, "ENIGMA: FEATURES: %s\n", features_filename);

   free(d_prefix);

   return EnigmaWeightSvdInit(
      prio_fun, 
      ocb,
      state,
      model_filename,
      features_filename,
      svd_Ut_filename,
      svd_S_filename,
      len_mult);
}

WFCB_p EnigmaWeightSvdInit(
   ClausePrioFun prio_fun, 
   OCB_p ocb,
   ProofState_p proofstate,
   char* model_filename,
   char* features_filename,
   char* svd_Ut_filename,
   char* svd_S_filename,
   double len_mult)
{
   EnigmaWeightSvdParam_p data = EnigmaWeightSvdParamAlloc();

   data->init_fun   = extweight_init;
   data->ocb        = ocb;
   data->proofstate = proofstate;
   
   data->model_filename = model_filename;
   data->features_filename = features_filename;
   data->svd_Ut_filename = svd_Ut_filename;
   data->svd_S_filename = svd_S_filename;
   data->len_mult = len_mult;
   
   return WFCBAlloc(
      EnigmaWeightSvdCompute, 
      prio_fun,
      EnigmaWeightSvdExit, 
      data);
}

double EnigmaWeightSvdCompute(void* data, Clause_p clause)
{
   static struct feature_node nodes[2048]; // TODO: dynamic alloc
   //static struct feature_node dense_nodes[2048]; // TODO: dynamic alloc

   EnigmaWeightSvdParam_p local;
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
   
   FeaturesSvdTranslate(local->svd_Ut, local->svd_S, nodes, local->dense_features);

   res = predict(local->linear_model, local->dense_features);
   //fprintf(GlobalOut, "+%0.2f ", res);
   double clen = ClauseWeight(clause,1,1,1,1,1,false);
   res = (clen * local->len_mult) + res;

   if (OutputLevel>=1) {
      fprintf(GlobalOut, "=%.2f: ", res);
      ClausePrint(GlobalOut, clause, true);
      fprintf(GlobalOut, "\n");
   }

   return res;
}

void EnigmaWeightSvdExit(void* data)
{
   EnigmaWeightSvdParam_p junk = data;
   
   EnigmaWeightSvdParamFree(junk);
}

/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/

