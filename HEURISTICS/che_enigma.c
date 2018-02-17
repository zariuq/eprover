/*-----------------------------------------------------------------------

File  : che_feature.c

Author: Jan Jakubuv

Contents
 
  Common functions for ENIGMA.

  Copyright 1998, 1999 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

Changes

<1> Sat Feb  4 13:33:11 CET 2017
    New


-----------------------------------------------------------------------*/

#include "che_enigma.h"



/*---------------------------------------------------------------------*/
/*                        Global Variables                             */
/*---------------------------------------------------------------------*/

/*---------------------------------------------------------------------*/
/*                      Forward Declarations                           */
/*---------------------------------------------------------------------*/


/*---------------------------------------------------------------------*/
/*                         Internal Functions                          */
/*---------------------------------------------------------------------*/

static char* top_symbol_string(Term_p term, Sig_p sig)
{
   if (TermIsVar(term)) 
   {
      return ENIGMA_VAR;
   }
   else if ((strncmp(SigFindName(sig, term->f_code), "esk", 3) == 0)) 
   {
      return ENIGMA_SKO;
   }
   else 
   {
      return SigFindName(sig, term->f_code);
   }
}

static void feature_increase(
   char* str,
   NumTree_p* counts, 
   Enigmap_p enigmap, 
   int* len)
{
   StrTree_p snode = StrTreeFind(&enigmap->feature_map, str);
   if (snode) 
   {
      long fid = snode->val1.i_val;
      NumTree_p cnode = NumTreeFind(counts, fid);
      if (cnode) 
      {
         cnode->val1.i_val++;
      }
      else 
      {
         cnode = NumTreeCellAllocEmpty();
         cnode->key = fid;
         cnode->val1.i_val = 1;
         NumTreeInsert(counts, cnode);
         (*len)++;
      }
   }
   else
   {
      //Warning("ENIGMA: Unknown feature \"%s\" skipped.");
   }
}

static int features_term_collect(
   NumTree_p* counts, 
   Term_p term, 
   Enigmap_p enigmap, 
   char* sym1, 
   char* sym2)
{
   char* sym3 = top_symbol_string(term, enigmap->sig);
   static char str[128];
   int len = 0;

   // verticals
   if (snprintf(str, 128, "%s:%s:%s", sym1, sym2, sym3) >= 128) 
   {
      Error("ENIGMA: Your symbol names are too long (%s:%s:%s)!", OTHER_ERROR, sym1, sym2, sym3);
   }
   feature_increase(str, counts, enigmap, &len);
   
   if (TermIsVar(term)||(TermIsConst(term))) { return len; }
   for (int i=0; i<term->arity; i++)
   {
      len += features_term_collect(counts, term->args[i], enigmap, sym2, sym3);
   }

   // horizontals
   DStr_p hstr = DStrAlloc();
   DStrAppendStr(hstr, sym3);
   for (int i=0; i<term->arity; i++)
   {
      DStrAppendChar(hstr, '~');
      DStrAppendStr(hstr, top_symbol_string(term->args[i], enigmap->sig));
   }
   feature_increase(DStrView(hstr), counts, enigmap, &len);
   DStrFree(hstr);

   return len;
}

/*---------------------------------------------------------------------*/
/*                         Exported Functions                          */
/*---------------------------------------------------------------------*/

Enigmap_p EnigmapAlloc(void)
{
   Enigmap_p res = EnigmapCellAlloc();

   res->sig = NULL;
   res->feature_map = NULL;

   return res;
}

void EnigmapFree(Enigmap_p junk)
{
   StrTreeFree(junk->feature_map);
   
   EnigmapCellFree(junk);
}

Enigmap_p EnigmapLoad(char* features_filename, Sig_p sig)
{
   Enigmap_p enigmap = EnigmapAlloc();
   long count = 0;

   enigmap->sig = sig;
   enigmap->feature_map = NULL;
   
   Scanner_p in = CreateScanner(StreamTypeFile, features_filename, true, NULL);
   ScannerSetFormat(in, TSTPFormat);                                             
   while (TestInpId(in, "feature"))
   {
      AcceptInpId(in, "feature");
      AcceptInpTok(in, OpenBracket);
      ParseNumString(in);
      long fid = atoi(in->accu->string);
      AcceptInpTok(in, Comma);

      CheckInpTok(in, String);
      StrTree_p cell = StrTreeCellAllocEmpty();
      cell->key = DStrCopyCore(AktToken(in)->literal);
      cell->val1.i_val = fid;
      StrTreeInsert(&enigmap->feature_map, cell);
      count++;
      //printf("%ld ... %s\n", fid, DStrCopyCore(AktToken(in)->literal));
      
      NextToken(in);
      AcceptInpTok(in, CloseBracket);
      AcceptInpTok(in, Fullstop);
   }
   CheckInpTok(in, NoToken);
   DestroyScanner(in);

   enigmap->feature_count = count;
   
   return enigmap;
}

int FeaturesClauseExtend(NumTree_p* counts, Clause_p clause, Enigmap_p enigmap)
{
   int len = 0;
   DStr_p hstr = DStrAlloc();

   for(Eqn_p lit = clause->literals; lit; lit = lit->next)
   {
      char* sym1 = EqnIsPositive(lit)?ENIGMA_POS:ENIGMA_NEG;
      if (lit->rterm->f_code == SIG_TRUE_CODE)
      {
         // verticals
         char* sym2 = SigFindName(enigmap->sig, lit->lterm->f_code);
         for (int i=0; i<lit->lterm->arity; i++) // here we ignore prop. constants
         {
            len += features_term_collect(counts, lit->lterm->args[i], enigmap, sym1, sym2);
         }

         // horizontals
         if (lit->lterm->arity > 0) 
         {
            DStrAppendStr(hstr, sym2);
            for (int i=0; i<lit->lterm->arity; i++)
            {
               DStrAppendChar(hstr, '~');
               DStrAppendStr(hstr, top_symbol_string(lit->lterm->args[i], enigmap->sig));
            }
            feature_increase(DStrView(hstr), counts, enigmap, &len);
            DStrReset(hstr);
         }
      }
      else
      {
         // verticals
         char* sym2 = ENIGMA_EQ;
         len += features_term_collect(counts, lit->lterm, enigmap, sym1, sym2);
         len += features_term_collect(counts, lit->rterm, enigmap, sym1, sym2);

         // horizontals
         DStrAppendStr(hstr, sym2);
         DStrAppendChar(hstr, '~');
         DStrAppendStr(hstr, top_symbol_string(lit->lterm, enigmap->sig));
         DStrAppendChar(hstr, '~');
         DStrAppendStr(hstr, top_symbol_string(lit->rterm, enigmap->sig));
         feature_increase(DStrView(hstr), counts, enigmap, &len);
         DStrReset(hstr);
      }
   }

   DStrFree(hstr);
   return len;
}

NumTree_p FeaturesClauseCollect(Clause_p clause, Enigmap_p enigmap, int* len)
{
   NumTree_p counts = NULL;
   *len = FeaturesClauseExtend(&counts, clause, enigmap);
   return counts;
}

void FeaturesSvdTranslate(DMat matUt, double* sing, 
   struct feature_node* in, struct feature_node* out)
{
   int i,j;
   double dot;

   for (i=0; i<matUt->rows; i++)
   {
      dot = 0.0;
      j = 0;
      while (in[j].index != -1) 
      {
         dot += matUt->value[i][in[j].index-1] * in[j].value;
         j++;
      }

      out[i].index = i+1;
      out[i].value = dot / sing[i];
   }
   out[i].index = -1;
}

/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/


