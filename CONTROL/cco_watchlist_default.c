/*-----------------------------------------------------------------------

  File  : cco_watchlist.c

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

#include "cco_watchlist.h"
#include "cco_simplification.h"
#include "cco_proofproc.h"

/*---------------------------------------------------------------------*/
/*                        Global Variables                             */
/*---------------------------------------------------------------------*/


/*---------------------------------------------------------------------*/
/*                      Forward Declarations                           */
/*---------------------------------------------------------------------*/


/*---------------------------------------------------------------------*/
/*                         Internal Functions                          */
/*---------------------------------------------------------------------*/

static void watch_progress_print(NumTree_p watch_progress)
{
   NumTree_p proof;
   PStack_p stack;

   fprintf(GlobalOut, "# Watchlist proofs progress:\n");
   stack = NumTreeTraverseInit(watch_progress);
   while((proof = NumTreeTraverseNext(stack)))
   {
      fprintf(GlobalOut, "#   watchlist %4ld: %0.3f (%8ld / %8ld)\n",
         proof->key, (double)proof->val1.i_val/proof->val2.i_val,
         proof->val1.i_val, proof->val2.i_val);
   }
   NumTreeTraverseExit(stack);
}

/*---------------------------------------------------------------------*/
/*                         Exported Functions                          */
/*---------------------------------------------------------------------*/

void WatchlistRemoveRewritables(WatchlistControl_p wlcontrol, ClauseSet_p rws,
   OCB_p ocb, ClauseSet_p archive, Clause_p clause)
{
   if(wlcontrol->wlindices.bw_rw_index)
   {
      // printf("# Simpclause: "); ClausePrint(stdout, clause, true); printf("\n");
      RemoveRewritableClausesIndexed(ocb, rws, archive, clause, clause->date,
         &(wlcontrol->wlindices));
      // printf("# Simpclause done\n");
   }
   else
   {
      RemoveRewritableClauses(ocb, wlcontrol->watchlist, rws, archive, 
         clause, clause->date, &(wlcontrol->wlindices));
   }
}

void WatchlistInsertRewritten(WatchlistControl_p wlcontrol, ClauseSet_p rws, 
   ProofControl_p control, TB_p terms, ClauseSet_p *demodulators)
{
   long     removed_lits;
   Clause_p handle;

   while((handle = ClauseSetExtractFirst(rws)))
   {
      // printf("# WL simplify: "); ClausePrint(stdout, handle, true);
      // printf("\n");
      ClauseComputeLINormalform(control->ocb,
                                terms,
                                handle,
                                demodulators,
                                control->heuristic_parms.forward_demod,
                                control->heuristic_parms.prefer_general);
      removed_lits = ClauseRemoveSuperfluousLiterals(handle);
      if(removed_lits)
      {
         DocClauseModificationDefault(handle, inf_minimize, NULL);
      }
      if(control->ac_handling_active)
      {
         ClauseRemoveACResolved(handle);
      }
      handle->weight = ClauseStandardWeight(handle);
      ClauseMarkMaximalTerms(control->ocb, handle);
      ClauseSetIndexedInsertClause(wlcontrol->watchlist, handle);
      // printf("# WL Inserting: "); ClausePrint(stdout, handle, true); printf("\n");
      GlobalIndicesInsertClause(&(wlcontrol->wlindices), handle);
   }
}

void WatchlistSimplify(WatchlistControl_p wlcontrol, Clause_p clause, ProofControl_p control, 
   TB_p terms, ClauseSet_p archive, ClauseSet_p* demodulators)
{
   ClauseSet_p rws;
   rws = ClauseSetAlloc();
   WatchlistRemoveRewritables(wlcontrol, rws, control->ocb, archive, clause);
   WatchlistInsertRewritten(wlcontrol, rws, control, terms, demodulators);
   ClauseSetFree(rws);
}

/*-----------------------------------------------------------------------
//
// Function: check_watchlist()
//
//   Check if a clause subsumes one or more watchlist clauses, if yes,
//   set appropriate property in clause and remove subsumed clauses.
//
// Global Variables: -
//
// Side Effects    : As decribed.
//
/----------------------------------------------------------------------*/

void WatchlistCheck(WatchlistControl_p wlcontrol, Clause_p clause, ClauseSet_p archive, 
   bool static_watchlist, Sig_p sig)
{
   FVPackedClause_p pclause = FVIndexPackClause(clause, wlcontrol->watchlist->fvindex);
   long removed;

   // printf("# check_watchlist(%p)...\n", indices);
   if (WLNormalizeSkolemSymbols)
   { // Don't know how to pass Sig to the qsort
	   ClauseSubsumeOrderSortLitsWL(clause);
   }
   else 
   {
	   ClauseSubsumeOrderSortLits(clause);
   }
   // assert(ClauseIsSubsumeOrdered(clause));

   clause->weight = ClauseStandardWeight(clause);

   if (static_watchlist)
   {
      Clause_p subsumed;

      subsumed = ClauseSetFindFirstSubsumedClause(wlcontrol->watchlist, clause, sig);
      if(subsumed)
      {
         ClauseSetProp(clause, CPSubsumesWatch);
      }
   }
   else
   {
      if((removed = RemoveSubsumed(&(wlcontrol->wlindices), pclause, wlcontrol->watchlist, 
         archive, &(wlcontrol->watch_progress), sig)))
      {
         ClauseSetProp(clause, CPSubsumesWatch);
         if(OutputLevel >= 1)
         {
            fprintf(GlobalOut,"# Watchlist reduced by %ld clause%s\n",
                    removed,removed==1?"":"s");
            if (wlcontrol->watch_progress)
            {
               watch_progress_print(wlcontrol->watch_progress);
            }
         }
         //printf("# XCL "); ClausePrint(GlobalOut, clause, true); printf("\n");
         DocClauseQuote(GlobalOut, OutputLevel, 6, clause,
                        "extract_subsumed_watched", NULL);   }
   }
   FVUnpackClause(pclause);
   // printf("# ...check_watchlist()\n");
}

/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/

