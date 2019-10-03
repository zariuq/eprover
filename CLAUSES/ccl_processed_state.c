/*-----------------------------------------------------------------------

  File  : ccl_processed_state.c

  Author: Zar Goertzel

  Contents

  Functions and data structures for the Processed Clause Proof State Vector

  Copyright 1998, 1999 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

  Created: Sun May 10 03:03:20 MET DST 1998

  -----------------------------------------------------------------------*/

#include "ccl_processed_state.h"
//#include "ccl_watchlist.h"
//#include "cco_simplification.h"
//#include "cco_proofproc.h"
//#include "ccl_unfold_defs.h"

//#include <dirent.h>

/*---------------------------------------------------------------------*/
/*                        Global Variables                             */
/*---------------------------------------------------------------------*/


/*---------------------------------------------------------------------*/
/*                      Forward Declarations                           */
/*---------------------------------------------------------------------*/


/*---------------------------------------------------------------------*/
/*                         Internal Functions                          */
/*---------------------------------------------------------------------*/


/*---------------------------------------------------------------------*/
/*                         Exported Functions                          */
/*---------------------------------------------------------------------*/

ProcessedState_p ProcessedStateAlloc(void)
{
  ProcessedState_p res = ProcessedStateCellAlloc();

  res->enigmap = NULL;
  res->features = NULL;
  res->features_count = 0;

  return res;
}

void ProcessedStateFree(ProcessedState_p junk)
{
  // TODO: free enigmap

  ProcessedStateCellFree(junk);
}

// Record the processed clause proof state at given clause selection (before it's added ot the state)
void ProcessedClauseStateRecord(ProcessedState_p processed_state, Clause_p clause)
{
  if(ProofObjectRecordsGCSelection)
  {
     if (ProofObjectRecordsProcessedState)
     {
        if (!clause->processed_proof_state)
        {  // keep previous copy, if any
           clause->processed_proof_state = NumTreeCopy(processed_state->features);
        }
     }
  }
}

// Helper function to print one line
// Destroys the data while
void ProcessedClauseStatePrintProgress(ProcessedState_p processed_state, FILE* out, Clause_p clause)
{
  NumTree_p processed_proof_state = NumTreeCopy(clause->processed_proof_state);
  while (processed_proof_state)
  {
    NumTree_p cell = NumTreeExtractEntry(&processed_proof_state, NumTreeMinNode(processed_proof_state)->key);
    fprintf(out, "%ld:%0.1f,", cell->key, (float)cell->val1.i_val);
    NumTreeCellFree(cell);
  }
}

/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/
