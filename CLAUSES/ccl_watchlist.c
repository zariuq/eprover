/*-----------------------------------------------------------------------

  File  : ccl_watchlist.c

  Author: Stephan Schulz

  Contents

  Implementation of clause sets (doubly linked lists), with optional
  extras (in particular various indices)

  Copyright 1998, 1999 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

  Created: Sun May 10 03:03:20 MET DST 1998

  -----------------------------------------------------------------------*/

#include "ccl_watchlist.h"
//#include "cco_simplification.h"
//#include "cco_proofproc.h"
#include "ccl_unfold_defs.h"

#include <dirent.h>

/*---------------------------------------------------------------------*/
/*                        Global Variables                             */
/*---------------------------------------------------------------------*/


/*---------------------------------------------------------------------*/
/*                      Forward Declarations                           */
/*---------------------------------------------------------------------*/


/*---------------------------------------------------------------------*/
/*                         Internal Functions                          */
/*---------------------------------------------------------------------*/

int filename_compare(IntOrP* left, IntOrP* right)
{
   return strcmp(DStrView((DStr_p)(left->p_val)), DStrView((DStr_p)(right->p_val)));
}

int tops_compare(IntOrP* left, IntOrP* right)
{
   return CMP(left->i_val, right->i_val);
}

DStr_p clause_code(PStack_p codes, Sig_p sig)
{
   DStr_p code = DStrAlloc();
   char sgn;
   long top, i;
   char *sym;

   for (i=0; i<codes->current; i++)
   {
      top = PStackElementInt(codes, i);
      if (top > 0)
      {
         sgn = '+';
      }
      else
      {
         sgn = '-';
         top = -top;
      }
      sym = (top == SIG_TRUE_CODE) ? "=" : SigFindName(sig, top);
      DStrAppendChar(code, sgn);
      DStrAppendStr(code, sym);
      DStrAppendChar(code, '|');
   }
   DStrDeleteLastChar(code);

   return code;
}

void watchlists_insert_clause(WatchlistControl_p wlcontrol, Clause_p clause)
{
   PStack_p tops = WatchlistClauseTops(clause);
   DStr_p code = clause_code(tops, wlcontrol->sig);
   StrTree_p node;
   Watchlist_p wl;
   IntOrP val;
   long top, i;
   PStackPointer index;
   NumTree_p topnode;

   // get the watchlist for this clause code
   node = StrTreeFind(&(wlcontrol->codes), DStrView(code));
   if (node)
   {
      //fprintf(GlobalOut, "# Reusing watchlist index: %s\n", DStrView(code));
      wl = PStackElementP(wlcontrol->watchlists, node->val1.i_val);
      index = node->val1.i_val;
      DStrFree(code);
   }
   else
   {
      //fprintf(GlobalOut, "# Creating new watchlist index: %s\n", DStrView(code));

      wl = WatchlistAlloc();
      GlobalIndicesInit(&(wl->indices), wlcontrol->sig, wlcontrol->rw_bw_index_type,
         wlcontrol->pm_from_index_type, wlcontrol->pm_into_index_type);
      wl->set->fvindex = FVIAnchorAlloc(wlcontrol->cspec, wlcontrol->perm);
      //FIXME: do we need to copy perm vector?
      //wl->set->fvindex = FVIAnchorAlloc(wlcontrol->cspec, PermVectorCopy(wlcontrol->perm));
      wl->code = code;

      index = wlcontrol->watchlists->current;
      PStackPushP(wlcontrol->watchlists, wl);
      val.i_val = index;
      StrTreeStore(&(wlcontrol->codes), DStrView(code), val, (IntOrP)NULL);
   }

   // for each top-level symbol, append this watchlist to the set
   for (i=0; i<tops->current; i++)
   {
      top = PStackElementInt(tops, i);
      topnode = NumTreeFind(&(wlcontrol->tops), top);
      if (!topnode)
      {
         topnode = NumTreeCellAlloc();
         topnode->key = top;
         topnode->val1.p_val = NULL;
         NumTreeInsert(&(wlcontrol->tops), topnode);
      }

      val.i_val = 1; // used as intersection counter
      NumTreeStore((NumTree_p*)&(topnode->val1.p_val), index, val, (IntOrP)NULL);
   }

   // finally insert the clause
   ClauseSetIndexedInsertClause(wl->set, clause);
   GlobalIndicesInsertClause(&(wl->indices), clause);

   //DStrFree(code);
   PStackFree(tops);
}

/*---------------------------------------------------------------------*/
/*                         Exported Functions                          */
/*---------------------------------------------------------------------*/

WatchlistControl_p WatchlistControlAlloc(void)
{
   WatchlistControl_p res = WatchlistControlCellAlloc();

   res->watchlist0 = ClauseSetAlloc();
   res->watchlists = PStackAlloc();
   res->watch_progress = NULL;
   res->proof_len = NULL;
   res->members= 0L;
   res->proofs_count = 0L;
   res->codes = NULL;
   res->tops = NULL;
   res->sig = NULL;

   return res;
}

Watchlist_p WatchlistAlloc(void)
{
   Watchlist_p res = WatchlistCellAlloc();

   res->set = ClauseSetAlloc();
   GlobalIndicesNull(&(res->indices));

   return res;
}

void WatchlistFree(Watchlist_p junk)
{
   // TODO
}

void WatchlistControlFree(WatchlistControl_p junk, GCAdmin_p gc, bool indfree)
{
   if (junk->watchlist0)
   {
      if (gc)
      {
         GCDeregisterClauseSet(gc, junk->watchlist0);
      }
      ClauseSetFree(junk->watchlist0);
      junk->watchlist0 = NULL;
   }
   if (indfree)
   {
      //GlobalIndicesFreeIndices(&(junk->wlindices));
   }
   WatchlistControlCellFree(junk);
}

PStack_p WatchlistClauseTops(Clause_p clause)
{
   long top, sgn;
   PStack_p codes = PStackAlloc();

   for(Eqn_p lit = clause->literals; lit; lit = lit->next)
   {
      sgn = EqnIsPositive(lit) ? 1 : -1;
      top = (lit->rterm->f_code == SIG_TRUE_CODE) ? lit->lterm->f_code : SIG_TRUE_CODE;
      // use SIG_TRUE_CODE for equality
      PStackPushInt(codes, sgn*top);
   }
   PStackSort(codes, (ComparisonFunctionType)tops_compare);

   PStack_p out = PStackAlloc();
   PStack_p empty = PStackAlloc();
   PStackMerge(codes, empty, out, (ComparisonFunctionType)tops_compare); // remove dups
   PStackFree(codes);
   PStackFree(empty);

   return out;
}

void WatchlistInsertSet(WatchlistControl_p wlcontrol, ClauseSet_p from)
{
   ClauseSetInsertSet(wlcontrol->watchlist0, from);
}

void WatchlistGCRegister(GCAdmin_p gc, WatchlistControl_p wlcontrol)
{
   GCRegisterClauseSet(gc, wlcontrol->watchlist0);
}

ClauseSet_p WatchlistLoadFile(TB_p bank, char* watchlist_filename, IOFormat parse_format)
{
   Scanner_p in;
   ClauseSet_p out = ClauseSetAlloc();
   in = CreateScanner(StreamTypeFile, watchlist_filename, true, NULL);
   ScannerSetFormat(in, parse_format);
   ClauseSetParseList(in, out, bank);
   CheckInpTok(in, NoToken);
   DestroyScanner(in);

   return out;
}

void WatchlistLoadDir(WatchlistControl_p wlcontrol, TB_p bank,
                               char* watchlist_dir,
                               IOFormat parse_format)
{
   DIR *dp;
   struct dirent *ep;
   ClauseSet_p tmpset;
   DStr_p filename;
   Clause_p handle;
   IntOrP val1;
   long proof_no = 0;

   if (!watchlist_dir)
   {
      return;
   }

   dp = opendir(watchlist_dir);
   if (!dp)
   {
      Error("Can't access watchlist dir '%s'", OTHER_ERROR, watchlist_dir);
      return;
   }

   PStack_p filenames = PStackAlloc();
   while ((ep = readdir(dp)) != NULL)
   {
      if (ep->d_type == DT_DIR)
      {
         continue;
      }

      filename = DStrAlloc();
      DStrAppendStr(filename, watchlist_dir);
      DStrAppendChar(filename, '/');
      DStrAppendStr(filename, ep->d_name);

      PStackPushP(filenames, filename);
   }
   closedir(dp);
   PStackSort(filenames, (ComparisonFunctionType)filename_compare);

   for (proof_no=0; proof_no<filenames->current; proof_no++)
   {
      filename = filenames->stack[proof_no].p_val;

      tmpset = WatchlistLoadFile(bank, DStrView(filename), parse_format);

      // set origin proof number
      for(handle = tmpset->anchor->succ;
          handle != tmpset->anchor;
          handle = handle->succ)
      {
         handle->watch_proof = proof_no+1;
      }

      // initialize watchlist proof progress
      //val1.i_val = 0; // 0 matched so far ...
      //NumTreeStore(&wlcontrol->watch_progress, proof_no+1, val1, (IntOrP)NULL);
      val1.i_val = tmpset->members; // ... out of total
      NumTreeStore(&wlcontrol->proof_len, proof_no+1, val1, (IntOrP)NULL);

      if (OutputLevel >= 1)
      {
         fprintf(GlobalOut, "#   watchlist %4ld: %8ld clauses from '%s'\n",
            proof_no+1, tmpset->members, DStrView(filename));
      }

      //ClauseSetInsertSet(state->watchlist, tmpset);
      WatchlistInsertSet(wlcontrol, tmpset);
      ClauseSetFree(tmpset);
      DStrFree(filename);
      wlcontrol->proofs_count++;
   }

   PStackFree(filenames);
}

void WatchlistLoaded(WatchlistControl_p wlcontrol)
{
   ClauseSetSetTPTPType(wlcontrol->watchlist0, CPTypeWatchClause);
   ClauseSetSetProp(wlcontrol->watchlist0, CPWatchOnly);
   ClauseSetDefaultWeighClauses(wlcontrol->watchlist0);
   if(WLNormalizeSkolemSymbols)
   {
	   ClauseSetSortLiterals(wlcontrol->watchlist0, EqnSubsumeInverseCompareRefWL);
   }
   {
   	ClauseSetSortLiterals(wlcontrol->watchlist0, EqnSubsumeInverseCompareRef);
   }
   ClauseSetDocInital(GlobalOut, OutputLevel, wlcontrol->watchlist0);
}

void WatchlistUnfoldEqDef(WatchlistControl_p wlcontrol, ClausePos_p demod)
{
   ClauseSetUnfoldEqDef(wlcontrol->watchlist0, demod);
}

void WatchlistReset(WatchlistControl_p wlcontrol)
{  // never used???
   ClauseSetFreeClauses(wlcontrol->watchlist0);
}

void WatchlistArchive(WatchlistControl_p wlcontrol, ClauseSet_p archive)
{
   ClauseSetArchive(archive, wlcontrol->watchlist0);
}

// Helper function to print one line
void WatchlistPrintProgress(WatchlistControl_p wlcontrol, FILE* out, Clause_p clause)
{
   NumTree_p proof, len;
   PStack_p stack;

   stack = NumTreeTraverseInit(clause->watch_proof_state);
   while((proof = NumTreeTraverseNext(stack)))
   {
      len = NumTreeFind(&(wlcontrol->proof_len), proof->key);
      if (!len)
      {
         Error("Watchlist: Unknown proof length of proof #%ld.  Should not happen!",
            OTHER_ERROR, proof->key);
      }
      fprintf(out, "%ld:%0.3f(%ld/%ld),",
         proof->key, (double)proof->val1.i_val/len->val1.i_val,
         proof->val1.i_val, len->val1.i_val);
   }
   NumTreeTraverseExit(stack);
}



void WatchlistIndicesInit(WatchlistControl_p wlcontrol, Sig_p sig,
   char* rw_bw_index_type, char* pm_from_index_type, char* pm_into_index_type)
{
   wlcontrol->sig = sig;
   wlcontrol->rw_bw_index_type = rw_bw_index_type;
   wlcontrol->pm_from_index_type = pm_from_index_type;
   wlcontrol->pm_into_index_type = pm_into_index_type;
}

void WatchlistInit(WatchlistControl_p wlcontrol, OCB_p ocb)
{
   Clause_p handle;

   ClauseSetMarkMaximalTerms(ocb, wlcontrol->watchlist0);
   while(!ClauseSetEmpty(wlcontrol->watchlist0))
   {
      handle = ClauseSetExtractFirst(wlcontrol->watchlist0);
      watchlists_insert_clause(wlcontrol, handle);
   }

   wlcontrol->members = 0L;
   for (int i=0; i<wlcontrol->watchlists->current; i++)
   {
      Watchlist_p watchlist = PStackElementP(wlcontrol->watchlists,i);
      wlcontrol->members += watchlist->set->members;
      if (OutputLevel >= 1)
      {
         fprintf(GlobalOut, "# Watchlist index #%6d: members=%6ld, code='%s'\n",
            i, watchlist->set->members, watchlist->code ? DStrView(watchlist->code) : "!opt!");
      }
   }

   fprintf(GlobalOut, "# Total watchlist clauses: %ld\n", wlcontrol->members);
   fprintf(GlobalOut, "# Total watchlist indices: %ld\n", wlcontrol->watchlists->current);
   fprintf(GlobalOut, "# Watchlist proof vector length: %ld\n", wlcontrol->proofs_count);
}

void WatchlistInitFVI(WatchlistControl_p wlcontrol, FVCollect_p cspec,
   PermVector_p perm)
{
   //wlcontrol->watchlist0->fvindex = FVIAnchorAlloc(cspec, perm);

   wlcontrol->cspec = cspec;
   wlcontrol->perm = perm;

   //ClauseSetNewTerms(state->watchlist, state->terms);
}

void WatchlistClauseProcessed(WatchlistControl_p wlcontrol, Clause_p clause)
{
   if(ProofObjectRecordsGCSelection)
   {
      // Copy proof state at given clause selection into the clause.
      // Notably this is different from the proof-state immediately after clause selection.
      if (ProofObjectRecordsProofVector)
      {
         if (!clause->watch_proof_state)
         {  // keep previous copy, if any
            clause->watch_proof_state = NumTreeCopy(wlcontrol->watch_progress);
         }
      }
   }
}

bool WatchlistEmpty(WatchlistControl_p wlcontrol)
{
   //return (ClauseSetEmpty(wlcontrol->watchlist0));
   return (wlcontrol->members <= 0L);
}

/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/
