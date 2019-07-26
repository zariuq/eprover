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
#include "clb_regmem.h"



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
static int string_compare(const void* p1, const void* p2)
{
   const char* s1 = ((const IntOrP*)p1)->p_val;
   const char* s2 = ((const IntOrP*)p2)->p_val;
   return -strcmp(s1, s2);
}
*/

static char* top_symbol_string(Term_p term, Sig_p sig)
{
   if (TermIsVar(term)) 
   {
      return ENIGMA_VAR;
   }
   else 
   {
      char* name = SigFindName(sig, term->f_code);
      if (name[0] == 'e') // be fast
      {
         if ((strncmp(name, "esk", 3) == 0) || (strncmp(name, "epred", 5) == 0))
         {
            return ENIGMA_SKO;
         }
      }
      return name;
   }
}

static unsigned long feature_sdbm(char* str)
{
   unsigned long hash = 0;
   int c;

   while ((c = *str++))
   {
      hash = c + (hash << 6) + (hash << 16) - hash;
   }

   return hash;
}

static long feature_hash(char* str, long base)
{
   return 1 + (feature_sdbm(str) % base);
}

static void feature_increase(
   char* str,
   int inc,
   NumTree_p* counts, 
   Enigmap_p enigmap, 
   int* len)
{
   if (inc == 0)
   {
      return;
   }
   long fid = 0;

   if (enigmap->version & EFHashing)
   {
      fid = feature_hash(str, enigmap->feature_count);
   }
   else
   {
      StrTree_p snode = StrTreeFind(&enigmap->feature_map, str);
      if (snode) 
      {
         fid = snode->val1.i_val;
      }
      else
      {
         //Warning("ENIGMA: Unknown feature \"%s\" skipped.", str);
         return;
      }
   }

   NumTree_p cnode = NumTreeFind(counts, fid);
   if (cnode) 
   {
      cnode->val1.i_val += inc;
   }
   else 
   {
      cnode = NumTreeCellAllocEmpty();
      cnode->key = fid;
      cnode->val1.i_val = inc;
      NumTreeInsert(counts, cnode);
      (*len)++;
   }

   if (enigmap->collect_stats)
   {  
      StrTree_p snode = StrTreeFind(&enigmap->stats, str);
      if (snode)
      {
         snode->val2.i_val += 1;
      }
      else
      {  
         StrTreeStore(&enigmap->stats, str, (IntOrP)fid, (IntOrP)1L);
      }
      //StrTreeUpdate(&enigmap->stats, str, cnode->val1, cnode->val2);
   }
}

static void feature_symbol_increase(
   char* prefix,
   char* fname,
   int inc,
   NumTree_p* counts, 
   Enigmap_p enigmap, 
   int* len)
{
   static char str[128]; // TODO: make dynamic DStr_p

   if (inc == 0)
   {
      return;
   }
   snprintf(str, 128, "%s%s", prefix, fname);
   feature_increase(str, inc, counts, enigmap, len);
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
   if (enigmap->version & EFVertical)
   {
      if (snprintf(str, 128, "%s:%s:%s", sym1, sym2, sym3) >= 128) 
      {
         Error("ENIGMA: Your symbol names are too long (%s:%s:%s)!", OTHER_ERROR, 
            sym1, sym2, sym3);
      }
      feature_increase(str, 1, counts, enigmap, &len);
   }
   
   if (TermIsVar(term)||(TermIsConst(term))) { return len; }
   for (int i=0; i<term->arity; i++)
   {
      len += features_term_collect(counts, term->args[i], enigmap, sym2, sym3);
   }

   // horizontals
   if (enigmap->version & EFHorizontal)
   {
      DStr_p hstr = FeaturesGetTermHorizontal(sym3, term, enigmap->sig);
      feature_increase(DStrView(hstr), 1, counts, enigmap, &len);
      DStrFree(hstr);
   }

   return len;
}

static void features_term_variables(
   NumTree_p* stat,
   Term_p term,
   int* distinct,
   int offset)
{
   if (TermIsVar(term))
   {
      NumTree_p vnode = NumTreeFind(stat, term->f_code - offset);
      if (vnode)
      {
         vnode->val1.i_val += 1;
      }
      else
      {
         vnode = NumTreeCellAllocEmpty();
         vnode->key = term->f_code - offset;
         vnode->val1.i_val = 1;
         NumTreeInsert(stat, vnode);
         if (distinct)
         {
            (*distinct)++;
         }
      }
   }
   else
   {
      for (int i=0; i<term->arity; i++)
      {
         features_term_variables(stat, term->args[i], distinct, offset);
      }
   }
}

/*---------------------------------------------------------------------*/
/*                         Exported Functions                          */
/*---------------------------------------------------------------------*/

DStr_p FeaturesGetTermHorizontal(char* top, Term_p term, Sig_p sig)
{
   DStr_p str = DStrAlloc();
   PStack_p args = PStackAlloc();
   for (int i=0; i<term->arity; i++)
   {
      PStackPushP(args, top_symbol_string(term->args[i], sig));
   }
   //PStackSort(args, string_compare);
   DStrAppendStr(str, top);
   while (!PStackEmpty(args))
   {  
      DStrAppendChar(str, '.');
      DStrAppendStr(str, PStackPopP(args));
   }
   PStackFree(args);
   return str;
}

DStr_p FeaturesGetEqHorizontal(Term_p lterm, Term_p rterm, Sig_p sig)
{
   DStr_p str = DStrAlloc();
   char* lstr = top_symbol_string(lterm, sig);
   char* rstr = top_symbol_string(rterm, sig);
   bool cond = (strcmp(lstr, rstr) < 0);
   DStrAppendStr(str, ENIGMA_EQ);
   DStrAppendChar(str, '.');
   DStrAppendStr(str, cond ? lstr : rstr);
   DStrAppendChar(str, '.');
   DStrAppendStr(str, cond ? rstr : lstr);
   return str;
}

Enigmap_p EnigmapAlloc(void)
{
   Enigmap_p res = EnigmapCellAlloc();

   res->sig = NULL;
   res->feature_map = NULL;
   res->version = EFNone;
   res->feature_count = 0L;
   res->collect_stats = false;
   res->stats = NULL;

   return res;
}

void EnigmapFree(Enigmap_p junk)
{
   StrTreeFree(junk->feature_map);
   if (junk->stats)
   {
      StrTreeFree(junk->stats);
   }
   
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
   enigmap->version = EFAll;
   if (TestInpId(in, "version"))
   {
      AcceptInpId(in, "version");
      AcceptInpTok(in, OpenBracket);
      CheckInpTok(in, String);
      enigmap->version = ParseEnigmaFeaturesSpec(AktToken(in)->literal->string);
      NextToken(in);
      AcceptInpTok(in, CloseBracket);
      AcceptInpTok(in, Fullstop);
   }

   if (enigmap->version & EFHashing)
   {
      AcceptInpId(in, "hash_base");
      AcceptInpTok(in, OpenBracket);
      ParseNumString(in);
      enigmap->feature_count = atoi(in->accu->string);
      AcceptInpTok(in, CloseBracket);
      AcceptInpTok(in, Fullstop);
      // skip the rest of the file (buckets collisions info)
      DestroyScanner(in);
   }
   else
   {
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
   }
   
   return enigmap;
}

EnigmaFeatures ParseEnigmaFeaturesSpec(char *spec)
{
   EnigmaFeatures enigma_features = EFNone;
   while (*spec) 
   {
      switch (*spec) 
      {
         case 'V': enigma_features |= EFVertical; break;
         case 'H': enigma_features |= EFHorizontal; break;
         case 'S': enigma_features |= EFSymbols; break;
         case 'L': enigma_features |= EFLengths; break;
         case 'C': enigma_features |= EFConjecture; break;
         case 'W': enigma_features |= EFProofWatch; break;
         case 'X': enigma_features |= EFVariables; break;
         case 'h': enigma_features |= EFHashing; break;
         case '"': break;
         default:
                   Error("Invalid Enigma features specifier '%c'. Valid characters are 'VHSLCWXh'.",
                         USAGE_ERROR, *spec);
                   break;
      }
      spec++;
   }
   return enigma_features;
}

int FeaturesClauseExtend(NumTree_p* counts, Clause_p clause, Enigmap_p enigmap)
{
   int len = 0;
   DStr_p hstr;

   for(Eqn_p lit = clause->literals; lit; lit = lit->next)
   {
      char* sym1 = EqnIsPositive(lit)?ENIGMA_POS:ENIGMA_NEG;
      if (lit->rterm->f_code == SIG_TRUE_CODE)
      {
         // verticals & horizontals
         char* sym2 = SigFindName(enigmap->sig, lit->lterm->f_code);
         for (int i=0; i<lit->lterm->arity; i++) // here we ignore prop. constants
         {
            len += features_term_collect(counts, lit->lterm->args[i], enigmap, sym1, sym2);
         }

         // top-level horizontal
         if ((enigmap->version & EFHorizontal) && (lit->lterm->arity > 0))
         {
            hstr = FeaturesGetTermHorizontal(sym2, lit->lterm, enigmap->sig);
            feature_increase(DStrView(hstr), 1, counts, enigmap, &len);
            DStrFree(hstr);
         }
      }
      else
      {
         // verticals & horizontals
         char* sym2 = ENIGMA_EQ;
         len += features_term_collect(counts, lit->lterm, enigmap, sym1, sym2);
         len += features_term_collect(counts, lit->rterm, enigmap, sym1, sym2);

         // top-level horizontal
         if (enigmap->version & EFHorizontal)
         {
            hstr = FeaturesGetEqHorizontal(lit->lterm, lit->rterm, enigmap->sig);
            feature_increase(DStrView(hstr), 1, counts, enigmap, &len);
            DStrFree(hstr);
         }
      }
   }

   return len;
}

void FeaturesClauseVariablesExtend(
   NumTree_p* stat,
   Clause_p clause,
   int* distinct,
   int offset)
{
   for(Eqn_p lit = clause->literals; lit; lit = lit->next)
   {
      features_term_variables(stat, lit->lterm, distinct, offset);
      features_term_variables(stat, lit->rterm, distinct, offset);
   }
}

/*
   out[0]: distinct variable count
   out[1]: variable occurances count
   out[2]: unique (non-shared) variables count
   out[3]: shared variables count
   out[4]: max1 (the occurence count of the most occuring variable)
   out[5]: max2 ( --- second most --- )
   'm a strange loopout[6]: max3 ( --- third most --- )
*/
void FeaturesClauseVariablesStat(
   NumTree_p* stat,
   long* out)
{
   PStack_p stack;
   NumTree_p vnode;

   stack = NumTreeTraverseInit(*stat);
   while ((vnode = NumTreeTraverseNext(stack)))
   {
      out[0]++;
      out[1] += vnode->val1.i_val;
      if (vnode->val1.i_val == 1)
      {
         out[2]++;
      }
      else
      {
         out[3]++;
      }
      // Handle max assuming most variables will be less frequent than the current 3rd
      if (out[0] > 3)
      {
         if (vnode->val1.i_val >= out[6])
         {
            if (vnode->val1.i_val >= out[5])
            {
                if (vnode->val1.i_val >= out[4])
                {
                   out[6] = out[5];
                   out[5] = out[4];
                   out[4] = vnode->val1.i_val;
                }
                else 
                {
                   out[6] = out[5];
                   out[5] = vnode->val1.i_val;
                }
            }
            else
            {
                out[6] = vnode->val1.i_val;
            }
         }
      }
      else
      {
         if (vnode->val1.i_val >= out[4])
         {
            out[6] = out[5];
            out[5] = out[4];
            out[4] = vnode->val1.i_val;
         }
         else if (vnode->val1.i_val >= out[5])
         {
            out[6] = out[5];
            out[5] = vnode->val1.i_val;
         }
         else if (vnode->val1.i_val >= out[6])
         {
            out[6] = vnode->val1.i_val;
         }
      }
   }
   NumTreeTraverseExit(stack);
}

void FeaturesAddVariables(NumTree_p* counts, NumTree_p* varstat, Enigmap_p enigmap, int *len)
{
   if (!(enigmap->version & EFVariables))
   {
      return;
   }

   long vars[10] = { 0L };
   
   FeaturesClauseVariablesStat(varstat, vars);

   feature_increase("!X!COUNT", vars[0], counts, enigmap, len);
   feature_increase("!X!OCCUR", vars[1], counts, enigmap, len);
   feature_increase("!X!UNIQ", vars[2], counts, enigmap, len);
   feature_increase("!X!SHARED", vars[3], counts, enigmap, len);
   feature_increase("!X!MAX1", vars[4], counts, enigmap, len);
   feature_increase("!X!MAX2", vars[5], counts, enigmap, len);
   feature_increase("!X!MAX3", vars[6], counts, enigmap, len);

   NumTreeFree(*varstat);
   *varstat = NULL;
}

      
void FeaturesAddClauseStatic(NumTree_p* counts, Clause_p clause, Enigmap_p enigmap, int *len, 
         NumTree_p* varstat, int* varoffset)
{
   static long* vec = NULL;
   static size_t size = 0;
   long vars[10];

   if (!vec)
   {
      size = (4*(enigmap->sig->f_count+1))*sizeof(long);
      vec = RegMemAlloc(size);
   }
   vec = RegMemProvide(vec, &size, (4*(enigmap->sig->f_count+1))*sizeof(long)); // when sig changes
   for (int i=0; i<4*(enigmap->sig->f_count+1); i++) { vec[i] = 0L; }

   if (enigmap->version & EFLengths)
   {
      feature_increase("!LEN", (long)ClauseWeight(clause,1,1,1,1,1,false), counts, enigmap, len);
      feature_increase("!POS", clause->pos_lit_no, counts, enigmap, len);
      feature_increase("!NEG", clause->neg_lit_no, counts, enigmap, len);
   }

   if (enigmap->version & EFSymbols)
   {
      PStack_p mod_stack = PStackAlloc();
      ClauseAddSymbolFeatures(clause, mod_stack, vec);
      PStackFree(mod_stack);
     
      for (long f=enigmap->sig->internal_symbols+1; f<=enigmap->sig->f_count; f++)
      {
         char* fname = SigFindName(enigmap->sig, f);
         if ((strlen(fname)>3) && ((strncmp(fname, "esk", 3) == 0) || (strncmp(fname, "epred", 5) == 0)))
         {
            continue;
         }

         if (vec[4*f+0] > 0)
         {
            feature_symbol_increase("#+", fname, vec[4*f+0], counts, enigmap, len);
            feature_symbol_increase("%+", fname, vec[4*f+1]+1, counts, enigmap, len);
         }
         if (vec[4*f+2] > 0)
         {
            feature_symbol_increase("#-", fname, vec[4*f+2], counts, enigmap, len);
            feature_symbol_increase("%-", fname, vec[4*f+3]+1, counts, enigmap, len);
         }
      }
   }

   if (varoffset && (enigmap->version & EFVariables))
   { 
      int distinct = 0;
      FeaturesClauseVariablesExtend(varstat, clause, &distinct, *varoffset);
      (*varoffset) += (2 * distinct);
   }
}

NumTree_p FeaturesClauseCollect(Clause_p clause, Enigmap_p enigmap, int* len)
{
   NumTree_p counts = NULL;
   NumTree_p varstat = NULL;
   int varoffset = 0;

   *len = FeaturesClauseExtend(&counts, clause, enigmap);
   FeaturesAddClauseStatic(&counts, clause, enigmap, len, &varstat, &varoffset);
   FeaturesAddVariables(&counts, &varstat, enigmap, len);
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

