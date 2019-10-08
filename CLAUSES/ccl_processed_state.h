/*-----------------------------------------------------------------------

  File  : ccl_processed_state.h

  Author: Zar Goertzel

  Contents

  Functions and data structures for the Processed Clause Proof State Vector

  Copyright 1998, 1999 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

  Created: Sat Jul  5 02:28:25 MET DST 1997

-----------------------------------------------------------------------*/

#ifndef CCL_PROCESSEDSTATE

#define CCL_PROCESSEDSTATE

#include <clb_numtrees.h>
//#include <ccl_clausesets.h>
//#include <ccl_global_indices.h>
#include <ccl_garbage_coll.h>



/*---------------------------------------------------------------------*/
/*                    Data type declarations                           */
/*---------------------------------------------------------------------*/

typedef struct enigmapcell *Enigmap_p;

typedef struct processedstatecell
{
  NumTree_p features;
  NumTree_p varstat;
  int varoffset;
  unsigned indices[2048]; // TODO: dynamic alloc
  float data[2048]; // TODO: dynamic alloc
  int features_count;
  Enigmap_p enigmap;
} ProcessedStateCell, *ProcessedState_p;

/*---------------------------------------------------------------------*/
/*                Exported Functions and Variables                     */
/*---------------------------------------------------------------------*/

#define ProcessedStateCellAlloc() (ProcessedStateCell*) \
        SizeMalloc(sizeof(ProcessedStateCell))
#define ProcessedStateCellFree(junk) \
        SizeFree(junk, sizeof(ProcessedStateCell))

ProcessedState_p ProcessedStateAlloc(void);
void             ProcessedStateFree(ProcessedState_p junk);

void ProcessedClauseStateRecord(ProcessedState_p processed_state, Clause_p clause, unsigned long processed_count);
void ProcessedClauseStatePrintProgress(ProcessedState_p processed_state, FILE* out, Clause_p clause);

#endif

/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/
