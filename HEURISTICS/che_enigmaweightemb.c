/*-----------------------------------------------------------------------

File  : che_enigmaweightemb.c

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

#include "che_enigmaweightemb.h"


/*---------------------------------------------------------------------*/
/*                        Global Variables                             */
/*---------------------------------------------------------------------*/

/*---------------------------------------------------------------------*/
/*                      Forward Declarations                           */
/*---------------------------------------------------------------------*/

/*---------------------------------------------------------------------*/
/*                         Internal Functions                          */
/*---------------------------------------------------------------------*/

/*
static unsigned conj_indices[2048]; // TODO: dynamic alloc
static float conj_data[2048]; // TODO: dynamic alloc
*/

char*  EmbFileName = NULL;

static void symbols_count_increase(FunCode f_code, NumTree_p* counts)
{
   NumTree_p node = NumTreeFind(counts, f_code);
   if (node)
   {
      node->val1.i_val++;
   }
   else
   {
      node = NumTreeCellAllocEmpty();
      node->key = f_code;
      node->val1.i_val = 1;
      NumTreeInsert(counts, node);
   }
}

static void symbols_count_term(Term_p term, NumTree_p* counts)
{
   if (TermIsVar(term))
   {
      return;
   }

   symbols_count_increase(term->f_code, counts);

   for (int i=0; i<term->arity; i++)
   {
      symbols_count_term(term->args[i], counts);
   }
}

static void symbols_count_clause(Clause_p clause, NumTree_p* counts)
{
   for (Eqn_p lit = clause->literals; lit; lit = lit->next)
   {
      symbols_count_term(lit->lterm, counts);
      symbols_count_term(lit->rterm, counts);
   }
}

static void emb_add(double* out, double* emb, int mult)
{
   int i;
   for (i=0; i<EMB_LEN; i++)
   {
      out[i] += (mult * emb[i]);
   }
}

static void emb_div(double* out, int fact)
{
   int i;
   
   if (fact == 0)
   {
      return;
   }

   for (i=0; i<EMB_LEN; i++)
   {
      out[i] /= fact;
   }
}

static void emb_null(double* vec)
{
   int i;
   for (i=0; i<EMB_LEN; i++) 
   {
      vec[i] = 0.0;
   }
}

static void emb_print_full(double* vec)
{
   fprintf(GlobalOut, "(");
   for (int i=0; i<EMB_LEN; i++)
   {
      fprintf(GlobalOut, "%f, ", vec[i]);
   }
   fprintf(GlobalOut, ")");
}

static void emb_print(double* vec)
{
   fprintf(GlobalOut, "(%f,%f,%f,...,%f,%f,%f)",
         vec[0], vec[1], vec[2], vec[EMB_LEN-3], vec[EMB_LEN-2], vec[EMB_LEN-1]);
}

static void emb_clause_add(double* vec, Clause_p clause, EnigmaWeightEmbParam_p data, int* len)
{
   NumTree_p counts = NULL;

   symbols_count_clause(clause, &counts);

   PStack_p stack = NumTreeTraverseInit(counts);
   NumTree_p cnode;
   while ((cnode = NumTreeTraverseNext(stack)))
   {
      NumTree_p enode = NumTreeFind(&data->embeds, cnode->key);
      if (!enode) 
      {
         continue;
      }

      double* emb = enode->val1.p_val;
      emb_add(vec, emb, cnode->val1.i_val);
      (*len) += cnode->val1.i_val;
   }
   NumTreeTraverseExit(stack);

   NumTreeFree(counts);
}

static void extweight_init(EnigmaWeightEmbParam_p data)
{
   static char str[256];
   double val;
   int ret;

   if (data->inited) 
   {
      return;
   }

   // read embeddings file
   if (!EmbFileName)
   {  
      Error("No embeddings file specified on the command line (--emb-file)!", USAGE_ERROR);
   }

   FILE* embs = fopen(EmbFileName, "r");
   if (!embs)
   {  
      Error("Can not open embeddings file '%s'!", FILE_ERROR, EmbFileName);
   }

   while (true)
   {
      ret = fscanf(embs, "%255[a-zA-Z0-9_=]", str);
      if (ret != 1)
      {
         Error("Wrong embeddings file format (last correct symbol: '%s')!", USAGE_ERROR, str);
      }
      //printf("%s (%d)\n", str, ret);

      double* vec = SizeMalloc(sizeof(double)*EMB_LEN); 
      for (int i=0; i<EMB_LEN; i++)
      {
         fscanf(embs, "%lf ", &val);
         vec[i] = val;
         //printf("%.3f ", val);
      }
      //printf("\n");

      FunCode f_code = SigFindFCode(data->ocb->sig, str);
      if (f_code == 0)
      {
         Warning("Unknown embedding symbol '%s'. Skipped.", str);
         SizeFree(vec, sizeof(double)*EMB_LEN);
      }
      else
      {
         NumTreeStore(&data->embeds, f_code, (IntOrP)(void*)vec, (IntOrP)NULL); 
      }

      if (feof(embs)) { break; }
   }
   fclose(embs);

   // debug print
   if (OutputLevel >= 1)
   {
      PStack_p stack = NumTreeTraverseInit(data->embeds);
      NumTree_p node;
      while ((node = NumTreeTraverseNext(stack)))
      {
         fprintf(GlobalOut, "# emb(%s) = ", SigFindName(data->ocb->sig, node->key));
         double* vec = node->val1.p_val;
         emb_print(vec);
         fprintf(GlobalOut, "\n");
      }
      NumTreeTraverseExit(stack);
   }

   // compute conjecture embedding
   int len = 0;
   Clause_p clause;
   Clause_p anchor;

   emb_null(data->conj_emb);
   anchor = data->proofstate->axioms->anchor;
   for (clause=anchor->succ; clause!=anchor; clause=clause->succ)
   {
      if (ClauseQueryTPTPType(clause)==CPTypeNegConjecture) 
      {
         emb_clause_add(data->conj_emb, clause, data, &len);
      }
   }
   emb_div(data->conj_emb, len);

   if (OutputLevel >= 1)
   {
      fprintf(GlobalOut, "# emb(GOAL) = "); 
      emb_print(data->conj_emb);
      fprintf(GlobalOut, "\n");
   }
   
   // load model
   XGBoosterCreate(NULL, 0, &data->xgboost_model);

   if (XGBoosterLoadModel(data->xgboost_model, data->model_filename) != 0)
   {
      Error("ENIGMA: Failed loading XGBoost model '%s':\n%s", FILE_ERROR,
         data->model_filename, XGBGetLastError());
   }
   //XGBoosterSetAttr(data->xgboost_model, "objective", "binary:logistic");

   data->inited = true;
 

   fprintf(GlobalOut, "# ENIGMA: XGBoost Embeddings model '%s' loaded.\n", 
      data->model_filename);
}

/*---------------------------------------------------------------------*/
/*                         Exported Functions                          */
/*---------------------------------------------------------------------*/

EnigmaWeightEmbParam_p EnigmaWeightEmbParamAlloc(void)
{
   EnigmaWeightEmbParam_p res = EnigmaWeightEmbParamCellAlloc();

   res->ocb = NULL;
   res->inited = false;

   return res;
}

void EnigmaWeightEmbParamFree(EnigmaWeightEmbParam_p junk)
{
   EnigmaWeightEmbParamCellFree(junk);
}
 
WFCB_p EnigmaWeightEmbParse(
   Scanner_p in,  
   OCB_p ocb, 
   ProofState_p state)
{   
   ClausePrioFun prio_fun;
   //double len_mult;

   AcceptInpTok(in, OpenBracket);
   prio_fun = ParsePrioFun(in);
   AcceptInpTok(in, Comma);
   char* d_prefix = ParseFilename(in);
   //AcceptInpTok(in, Comma);
   //len_mult = ParseFloat(in);
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
   DStrAppendStr(f_model, "model.emb");
   char* model_filename = SecureStrdup(DStrView(f_model));
   DStrFree(f_model);

   /*
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
   */

   return EnigmaWeightEmbInit(
      prio_fun, 
      ocb,
      state,
      model_filename);
}

WFCB_p EnigmaWeightEmbInit(
   ClausePrioFun prio_fun, 
   OCB_p ocb,
   ProofState_p proofstate,
   char* model_filename)
{
   EnigmaWeightEmbParam_p data = EnigmaWeightEmbParamAlloc();

   data->init_fun   = extweight_init;
   data->ocb        = ocb;
   data->proofstate = proofstate;
   data->inited     = false;
   data->embeds     = NULL;
   
   data->model_filename = model_filename;
   
   return WFCBAlloc(
      EnigmaWeightEmbCompute, 
      prio_fun,
      EnigmaWeightEmbExit, 
      data);
}

double EnigmaWeightEmbCompute(void* data, Clause_p clause)
{
   static unsigned xgb_indices[2048]; // TODO
   static float xgb_data[2048]; // TODO
   EnigmaWeightEmbParam_p local;
   local = data;
   local->init_fun(data);

   static double emb[EMB_LEN];
   long long start = GetUSecClock();

   int len = 0;
   emb_null(emb);
   emb_clause_add(emb, clause, local, &len);
   emb_div(emb, len);
   
   if (OutputLevel >= 1)
   {
      fprintf(GlobalOut, "# EVAL = "); 
      ClausePrint(GlobalOut, clause, true);
      fprintf(GlobalOut, "\n# emb(EVAL) = "); 
      emb_print(emb);
      fprintf(GlobalOut, "\n");
   }

   int i;
   for (i=0; i<EMB_LEN; i++)
   {
      xgb_indices[i] = i+1;
      xgb_data[i] = emb[i];
   }
   for (i=0; i<EMB_LEN; i++)
   {
      xgb_indices[EMB_LEN+i] = EMB_LEN+i+1;
      xgb_data[EMB_LEN+i] = local->conj_emb[i];
   }
   int total = 2*EMB_LEN;

   if (OutputLevel >= 2)
   {
      fprintf(GlobalOut, "# xgb vector: ");
      for (i=0; i<total; i++)
      {
         fprintf(GlobalOut, "%d:%f ", xgb_indices[i], xgb_data[i]);
      }
      fprintf(GlobalOut, "\n");
   }

   size_t xgb_nelem = total; 
   size_t xgb_num_col = 1 + total;
   size_t xgb_nindptr = 2;
   static bst_ulong xgb_indptr[2];
   xgb_indptr[0] = 0L;
   xgb_indptr[1] = xgb_nelem;
   DMatrixHandle xgb_matrix = NULL;
   if (XGDMatrixCreateFromCSREx(xgb_indptr, xgb_indices, xgb_data, 
          xgb_nindptr, xgb_nelem, xgb_num_col, &xgb_matrix) != 0)
   {
      Error("ENIGMA: Failed creating XGBoost prediction matrix:\n%s", 
         OTHER_ERROR, XGBGetLastError());
   }

   bst_ulong out_len = 0L;
   const float* pred;
   if (XGBoosterPredict(local->xgboost_model, xgb_matrix, 
          0, 0, &out_len, &pred) != 0)
   {
      Error("ENIGMA: Failed computing XGBoost prediction:\n%s", 
         OTHER_ERROR, XGBGetLastError());
   }
   
   //res = 1 + ((1.0 - pred[0]) * 10.0);
   double res;
   if (pred[0] <= 0.5) { res = 10.0; } else { res = 1.0; }

   XGDMatrixFree(xgb_matrix);

   if (OutputLevel>=1) {
      double clen = ClauseWeight(clause,1,1,1,1,1,false);
      fprintf(GlobalOut, "=%.2f (val=%.3f,t=%.3fms,clen=%.1f,vlen=%ld) : ", res, pred[0], (double)(GetUSecClock() - start)/ 1000.0, clen, xgb_nelem);
      ClausePrint(GlobalOut, clause, true);
      fprintf(GlobalOut, "\n");
   }

   return res;
}

void EnigmaWeightEmbExit(void* data)
{
   EnigmaWeightEmbParam_p junk = data;
   
   EnigmaWeightEmbParamFree(junk);
}

/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/

