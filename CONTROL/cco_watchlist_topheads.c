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

static double watch_progress_get(NumTree_p* watch_progress, long proof_no)
{
   NumTree_p proof;

   // find the proof progress statistics ...
   proof = NumTreeFind(watch_progress, proof_no);
   if (!proof) 
   {
      Error("Unknown proof number (%ld) of a watchlist clause! Should not happen!", 
         OTHER_ERROR, proof_no);
   }
   
   return (double)proof->val1.i_val/proof->val2.i_val;
}

static double watch_parents_relevance(Clause_p clause)
{
   PStackPointer i, sp;
   DerivationCodes op;
   Clause_p parent;
   double relevance = 0.0;
   int parents = 0;
  
   if (OutputLevel >= 2)
   {
      fprintf(GlobalOut, "# PARENTS OF ");
      ClausePrint(GlobalOut, clause, true);
      fprintf(GlobalOut, "\n");
   }

   if (!clause->derivation)
   {
      if (ClauseQueryProp(clause, CPInitial))
      {
         return 0.0;
      }
      Error("Clause has no derivation.  Are you running with -p option?", USAGE_ERROR);
   }
   
   sp = PStackGetSP(clause->derivation);
   i = 0;
   while (i<sp)
   {
      op = PStackElementInt(clause->derivation, i);
      i++;
      
      if(DCOpHasCnfArg1(op))
      {
         parent = PStackElementP(clause->derivation, i);
         relevance += parent->watch_relevance;
         parents++;
    
         if (OutputLevel >= 2)
         {
            fprintf(GlobalOut, "# -> ");
            ClausePrint(GlobalOut, parent, true);
            fprintf(GlobalOut, "\n");
         }
      }
      if(DCOpHasArg1(op))
      {
         i++;
      }

      if(DCOpHasCnfArg2(op))
      {
         parent = PStackElementP(clause->derivation, i);
         relevance += parent->watch_relevance;
         parents++;

         if (OutputLevel >= 2)
         {
            fprintf(GlobalOut, "# -> ");
            ClausePrint(GlobalOut, parent, true);
            fprintf(GlobalOut, "\n");
         }
      }
      if(DCOpHasArg2(op))
      {
         i++;
      }
   }

   return (parents>0) ? (relevance/parents) : 0.0;
}

static void watchlist_set_relevance(Clause_p clause, NumTree_p* watch_progress)
{
   if (watch_progress && *watch_progress) 
   {
      double proof_progress = 0.0;
      
      if (clause->watch_proof > 0)
      {
         proof_progress = watch_progress_get(watch_progress, clause->watch_proof);
      }
     
	  if (WLInheritRelevance)
	  { 
		  double parents_relevance = watch_parents_relevance(clause);
		  //double decay_factor = 0.1; // transformed into an option
		  double combined_relevance = proof_progress + (decay_factor*parents_relevance);

		  if (OutputLevel >= 2 || (OutputLevel == 1 && clause->watch_proof > 0))
		  {
			 fprintf(GlobalOut, "# WATCHLIST RELEVANCE: relevance=%1.3f(=%1.3f+%1.3f*%1.3f); proof=%ld; clause=", 
				combined_relevance,
				proof_progress,
				decay_factor,
				parents_relevance,
				clause->watch_proof);
			 ClausePrint(GlobalOut, clause, true);
			 fprintf(GlobalOut, "\n");
		  }
		  clause->watch_relevance = combined_relevance;
	  }
	  else
	  {
		  if (OutputLevel >= 2 || (OutputLevel == 1 && clause->watch_proof > 0))
		  {
			 fprintf(GlobalOut, "# WATCHLIST RELEVANCE: relevance=%1.3f; proof=%ld; clause=", 
				proof_progress,
				clause->watch_proof);
			 ClausePrint(GlobalOut, clause, true);
			 fprintf(GlobalOut, "\n");
		  }
		  clause->watch_relevance = proof_progress;
	  }
   }
}

static long watchlist_check(WatchlistControl_p wlcontrol, long index, Clause_p clause, ClauseSet_p archive)
{
   Watchlist_p watchlist = PStackElementP(wlcontrol->watchlists, index);
   FVPackedClause_p pclause = FVIndexPackClause(clause, watchlist->set->fvindex);
   long removed = RemoveSubsumed(&(watchlist->indices), pclause, watchlist->set, archive, &(wlcontrol->watch_progress), wlcontrol->sig);
   FVUnpackClause(pclause);
   //if (removed)
   //{
   //   fprintf(GlobalOut, "# checking index '%s': removed=%ld\n", DStrView(watchlist->code), removed);
   //}
   return removed;
}

static long watchlists_check(WatchlistControl_p wlcontrol, Clause_p clause, ClauseSet_p archive)
{
   long removed = 0;
   PStack_p stack;

   //fprintf(GlobalOut, "# watchlist check for: ");
   //ClausePrint(GlobalOut, clause, true);
   //fprintf(GlobalOut, "\n");

   PStack_p tops = WatchlistClauseTops(clause);

   NumTree_p top0 = NULL;
   NumTree_p counts = NULL;
   NumTree_p topwl, countwl;
   for (long i=0; i<tops->current; i++)
   {
      long top = PStackElementInt(tops, i);
      if (!top0)
      {
         top0 = NumTreeFind(&(wlcontrol->tops), top);
         continue;
      }
      if (!counts)
      {
         counts = NumTreeCopy(top0->val1.p_val);
      }
      NumTree_p topi = NumTreeFind(&(wlcontrol->tops), top);
      if (!topi) { continue; }

      stack = NumTreeTraverseInit((NumTree_p)(topi->val1.p_val));
      while ((topwl = NumTreeTraverseNext(stack)))
      {
         countwl = NumTreeFind(&counts, topwl->key);
         if (countwl)
         {
            countwl->val1.i_val++; // increase intersection count
         }
      }
      NumTreeTraverseExit(stack);
   }
      
   stack = NumTreeTraverseInit(counts);
   while ((countwl = NumTreeTraverseNext(stack)))
   {
      if (countwl->val1.i_val == tops->current) // check only watchlists in the full intersection
      {
         removed += watchlist_check(wlcontrol, countwl->key, clause, archive);
      }
   }
   NumTreeTraverseExit(stack);

   NumTreeFree(counts);
   PStackFree(tops);

   watchlist_set_relevance(clause, &(wlcontrol->watch_progress));
  
   return removed;
}

/*---------------------------------------------------------------------*/
/*                         Exported Functions                          */
/*---------------------------------------------------------------------*/

void WatchlistRemoveRewritables(Watchlist_p watchlist, ClauseSet_p rws,
   OCB_p ocb, ClauseSet_p archive, Clause_p clause)
{
   if (watchlist->indices.bw_rw_index)
   {
      // printf("# Simpclause: "); ClausePrint(stdout, clause, true); printf("\n");
      RemoveRewritableClausesIndexed(ocb, rws, archive, clause, clause->date,
         &(watchlist->indices));
      // printf("# Simpclause done\n");
   }
   else
   {
      RemoveRewritableClauses(ocb, watchlist->set, rws, archive, 
         clause, clause->date, &(watchlist->indices));
   }
}

void WatchlistInsertRewritten(Watchlist_p watchlist, ClauseSet_p rws, 
   ProofControl_p control, TB_p terms, ClauseSet_p *demodulators)
{
   long     removed_lits;
   Clause_p handle;

   while ((handle = ClauseSetExtractFirst(rws)))
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
      ClauseSetIndexedInsertClause(watchlist->set, handle);
      // printf("# WL Inserting: "); ClausePrint(stdout, handle, true); printf("\n");
      GlobalIndicesInsertClause(&(watchlist->indices), handle);
   }
}

void WatchlistSimplify(WatchlistControl_p wlcontrol, Clause_p clause, ProofControl_p control, 
   TB_p terms, ClauseSet_p archive, ClauseSet_p* demodulators)
{
   ClauseSet_p rws;

   rws = ClauseSetAlloc();
   for (long i=0; i<wlcontrol->watchlists->current; i++)
   {
      Watchlist_p watchlist = PStackElementP(wlcontrol->watchlists, i);
      WatchlistRemoveRewritables(watchlist, rws, control->ocb, archive, clause);
      WatchlistInsertRewritten(watchlist, rws, control, terms, demodulators);
   }
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

      subsumed = ClauseSetFindFirstSubsumedClause(wlcontrol->watchlist0, clause, sig);
      if(subsumed)
      {
         ClauseSetProp(clause, CPSubsumesWatch);
      }
   }
   else
   {
      if ((removed = watchlists_check(wlcontrol,clause,archive)))
      {
         ClauseSetProp(clause, CPSubsumesWatch);
         wlcontrol->members -= removed;
         DocClauseQuote(GlobalOut, OutputLevel, 6, clause,
                        "extract_subsumed_watched", NULL);   
         if(OutputLevel >= 1)
         {
            fprintf(GlobalOut,"# Watchlist reduced by %ld clause%s\n",
                    removed,removed==1?"":"s");
            if (wlcontrol->watch_progress)
            {
               watch_progress_print(wlcontrol->watch_progress);
            }
         }

      }
   }
   // printf("# ...check_watchlist()\n");
}

/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/

