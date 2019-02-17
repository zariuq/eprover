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

static long watchlist_check(
   WatchlistControl_p wlcontrol, 
   long index, 
   Clause_p clause, 
   ClauseSet_p archive)
{
   Watchlist_p watchlist = PDArrayElementP(wlcontrol->watchlists, index);
   if (!watchlist)
   {
      Warning("Empty watchlist cluster #%ld.\n", index);
      return 0;
   }
   FVPackedClause_p pclause = FVIndexPackClause(clause, watchlist->set->fvindex);
   long removed = RemoveSubsumed(&(watchlist->indices), pclause, watchlist->set, 
      archive, &(wlcontrol->watch_progress), wlcontrol->sig);
   FVUnpackClause(pclause);

   //if (removed) {
   fprintf(GlobalOut, "# checking cluster #%ld: removed=%ld\n", index, removed);
   //}
   return removed;
}

static PStack_p guess_clause_clusters(WatchlistControl_p wlcontrol, Clause_p clause)
{
   PStack_p clusters = PStackAlloc();
   static unsigned xgb_indices[2048]; // TODO: dynamic alloc
   static float xgb_data[2048]; // TODO: dynamic alloc
   int len = 0;
   NumTree_p cell; 
   long long start = GetUSecClock();
   
   NumTree_p features = FeaturesClauseCollect(clause, wlcontrol->enigmap, &len);
   if (len >= 2048) { Error("EnigmaWatch: Too many clause features!", OTHER_ERROR); }
  
   int i = 0;
   PStack_p stack = NumTreeTraverseInit(features);
   while((cell = NumTreeTraverseNext(stack)))
   {
      xgb_indices[i] = cell->key;
      xgb_data[i] = (float)cell->val1.i_val;
      printf("%d:%.0f ", xgb_indices[i], xgb_data[i]);
      i++;
   }
   printf("\n");
   NumTreeTraverseExit(stack);
   NumTreeFree(features);
   
   start = GetUSecClock();
   size_t xgb_nelem = i;
   size_t xgb_num_col = 1 + wlcontrol->enigmap->feature_count;
   size_t xgb_nindptr = 2;
   static bst_ulong xgb_indptr[2];
   xgb_indptr[0] = 0L;
   xgb_indptr[1] = xgb_nelem;
   DMatrixHandle xgb_matrix = NULL;
   if (XGDMatrixCreateFromCSREx(xgb_indptr, xgb_indices, xgb_data, 
          xgb_nindptr, xgb_nelem, xgb_num_col, &xgb_matrix) != 0)
   {
      Error("EnigmaWatch: Failed creating XGBoost prediction matrix:\n%s", 
         OTHER_ERROR, XGBGetLastError());
   }
   //printf("[duration] xgb matrix: %f.2 ms\n", (double)(clock() - start)/ (CLOCKS_PER_SEC / 1000));

   bst_ulong out_len = 0L;
   const float* pred;
   if (XGBoosterPredict(wlcontrol->xgboost_model, xgb_matrix, 
          0, 0, &out_len, &pred) != 0)
   {
      Error("EnigmaWatch: Failed computing XGBoost prediction:\n%s", 
         OTHER_ERROR, XGBGetLastError());
   }
   printf("prediction: len=%ld non-zeros: ", out_len);
   for (int ii=0; ii<out_len; ii++) 
   { 
      if (pred[ii] > 0.1) 
      { 
         printf("%d:%f ", ii, pred[ii]);
         PStackPushInt(clusters, ii);
      } 
   }
   printf("\n");
   
   //res = 1 + ((1.0 - pred[0]) * 10.0);
   //if (pred[0] <= 0.5) { res = 10.0; } else { res = 1.0; }
      
   ClausePrint(GlobalOut, clause, true);
   fprintf(GlobalOut, "\n");

   XGDMatrixFree(xgb_matrix);
   
   double clen = ClauseWeight(clause,1,1,1,1,1,false);
   printf("[duration] xgb predict: %.3f ms   (clen=%.1f, vlen=%ld)\n", (double)(GetUSecClock() - start)/ 1000.0, clen, xgb_nelem);



   //PStackPushInt(clusters, 1);
   //PStackPushInt(clusters, 10);
   //PStackPushInt(clusters, 33);

   return clusters;
}

static long watchlists_check(WatchlistControl_p wlcontrol, Clause_p clause, ClauseSet_p archive)
{
   long removed = 0;
   PStack_p clusters;

   //fprintf(GlobalOut, "# watchlist check for: ");
   //ClausePrint(GlobalOut, clause, true);
   //fprintf(GlobalOut, "\n");
   
   clusters = guess_clause_clusters(wlcontrol, clause);
   for (int i=0; i<clusters->current; i++)
   {
      long idx = PStackElementInt(clusters, i);
      removed += watchlist_check(wlcontrol, idx, clause, archive);
   }
   PStackFree(clusters);
   
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
   for (long i=0; i<wlcontrol->watchlists->size; i++)
   {
      Watchlist_p watchlist = PDArrayElementP(wlcontrol->watchlists, i);
      if (watchlist)
      {
         WatchlistRemoveRewritables(watchlist, rws, control->ocb, archive, clause);
         WatchlistInsertRewritten(watchlist, rws, control, terms, demodulators);
      }
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

   if (!wlcontrol->ready)
   {
      XGBoosterCreate(NULL, 0, &wlcontrol->xgboost_model);

      if (XGBoosterLoadModel(wlcontrol->xgboost_model, wlcontrol->xgboost_filename) != 0)
      {
         Error("EnigmaWatch: Failed loading XGBoost model '%s':\n%s", FILE_ERROR,
            wlcontrol->xgboost_filename, XGBGetLastError());
      }
      wlcontrol->enigmap = EnigmapLoad(wlcontrol->enigmap_filename, wlcontrol->sig);
      wlcontrol->ready = true;
   }

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

