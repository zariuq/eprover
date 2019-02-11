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

/*---------------------------------------------------------------------*/
/*                         Exported Functions                          */
/*---------------------------------------------------------------------*/

WatchlistControl_p WatchlistControlAlloc(void)
{
   WatchlistControl_p res = WatchlistControlCellAlloc();

   res->watchlist = ClauseSetAlloc();
   res->watch_progress = NULL;
   GlobalIndicesNull(&(res->wlindices));

   return res;
}

void WatchlistControlFree(WatchlistControl_p junk, GCAdmin_p gc, bool indfree)
{
   if (junk->watchlist)
   {
      if (gc) 
      {
         GCDeregisterClauseSet(gc, junk->watchlist);
      }
      ClauseSetFree(junk->watchlist);
      junk->watchlist = NULL;
   }
   if (indfree)
   {
      GlobalIndicesFreeIndices(&(junk->wlindices));
   }
   WatchlistControlCellFree(junk);
}

void WatchlistInsertSet(WatchlistControl_p wlcontrol, ClauseSet_p tmpset)
{
   ClauseSetInsertSet(wlcontrol->watchlist, tmpset);
}

void WatchlistGCRegister(GCAdmin_p gc, WatchlistControl_p wlcontrol)
{
   GCRegisterClauseSet(gc, wlcontrol->watchlist);
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
   IntOrP val1, val2;
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
      val1.i_val = 0; // 0 matched so far ...
      val2.i_val = tmpset->members; // ... out of "val2" total
      NumTreeStore(&wlcontrol->watch_progress, proof_no+1, val1, val2);
      
      if (OutputLevel >= 1)
      {
         fprintf(GlobalOut, "#   watchlist %4ld: %8ld clauses from '%s'\n", 
            proof_no+1, tmpset->members, DStrView(filename));
      }

      //ClauseSetInsertSet(state->watchlist, tmpset);
      WatchlistInsertSet(wlcontrol, tmpset);
      ClauseSetFree(tmpset);
      DStrFree(filename);
   }

   PStackFree(filenames);
}


void WatchlistLoaded(WatchlistControl_p wlcontrol)
{
   ClauseSetSetTPTPType(wlcontrol->watchlist, CPTypeWatchClause);
   ClauseSetSetProp(wlcontrol->watchlist, CPWatchOnly);
   ClauseSetDefaultWeighClauses(wlcontrol->watchlist);
   if(WLNormalizeSkolemSymbols)
   {
	   ClauseSetSortLiterals(wlcontrol->watchlist, EqnSubsumeInverseCompareRefWL);
   }
   {
   	ClauseSetSortLiterals(wlcontrol->watchlist, EqnSubsumeInverseCompareRef);
   }
   ClauseSetDocInital(GlobalOut, OutputLevel, wlcontrol->watchlist);
}

void WatchlistInit(WatchlistControl_p wlcontrol, OCB_p ocb)
{
   ClauseSet_p tmpset;
   Clause_p handle;

   tmpset = ClauseSetAlloc();

   ClauseSetMarkMaximalTerms(ocb, wlcontrol->watchlist);
   while(!ClauseSetEmpty(wlcontrol->watchlist))
   {
      handle = ClauseSetExtractFirst(wlcontrol->watchlist);
      ClauseSetInsert(tmpset, handle);
   }
   ClauseSetIndexedInsertClauseSet(wlcontrol->watchlist, tmpset);
   ClauseSetFree(tmpset);
   GlobalIndicesInsertClauseSet(&(wlcontrol->wlindices),wlcontrol->watchlist);

   if (OutputLevel >= 1)
   {
      fprintf(GlobalOut, "# Total watchlist clauses: %ld\n", 
         wlcontrol->watchlist->members);
   }
   // ClauseSetPrint(stdout, wlcontrol->watchlist, true);
}

void WatchlistReset(WatchlistControl_p wlcontrol)
{
   ClauseSetFreeClauses(wlcontrol->watchlist);
   GlobalIndicesReset(&(wlcontrol->wlindices));
}

void WatchlistInitFVI(WatchlistControl_p wlcontrol, FVCollect_p cspec, 
   PermVector_p perm)
{
   wlcontrol->watchlist->fvindex = FVIAnchorAlloc(cspec, perm);
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
   return (ClauseSetEmpty(wlcontrol->watchlist));
}

void WatchlistArchive(WatchlistControl_p wlcontrol, ClauseSet_p archive)
{
   ClauseSetArchive(archive, wlcontrol->watchlist);
}

void WatchlistUnfoldEqDef(WatchlistControl_p wlcontrol, ClausePos_p demod)
{
   ClauseSetUnfoldEqDef(wlcontrol->watchlist, demod);
}

void WatchlistIndicesInit(WatchlistControl_p wlcontrol, Sig_p sig,
   char* rw_bw_index_type, char* pm_from_index_type, char* pm_into_index_type)
{
   GlobalIndicesInit(&(wlcontrol->wlindices), sig, rw_bw_index_type, pm_from_index_type, pm_into_index_type);
}



/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/
