/*-----------------------------------------------------------------------

  File  : ccl_watchlist.h

  Author: Stephan Schulz

  Contents

  Definitions dealing with collections of clauses

  Copyright 1998, 1999 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

  Created: Sat Jul  5 02:28:25 MET DST 1997

-----------------------------------------------------------------------*/

#ifndef CCL_WATCHLIST

#define CCL_WATCHLIST

#include <ccl_clausesets.h>
#include <ccl_global_indices.h>
#include <ccl_garbage_coll.h>



/*---------------------------------------------------------------------*/
/*                    Data type declarations                           */
/*---------------------------------------------------------------------*/

typedef struct proofcontrolcell *ProofControl_p;

typedef struct watchlistcontrolcell 
{
   ClauseSet_p     watchlist;
   GlobalIndices   wlindices;
   NumTree_p       watch_progress;
} WatchlistControlCell, *WatchlistControl_p;



/*---------------------------------------------------------------------*/
/*                Exported Functions and Variables                     */
/*---------------------------------------------------------------------*/

#define WatchlistControlCellAlloc() (WatchlistControlCell*) \
        SizeMalloc(sizeof(WatchlistControlCell))
#define WatchlistControlCellFree(junk) \
        SizeFree(junk, sizeof(WatchlistControlCell))

WatchlistControl_p WatchlistControlAlloc(void);
void WatchlistControlFree(WatchlistControl_p junk, GCAdmin_p gc, bool indfree);

void WatchlistInsertSet(WatchlistControl_p wlcontrol, ClauseSet_p tmpset);
void WatchlistGCRegister(GCAdmin_p gc, WatchlistControl_p wlcontrol);

ClauseSet_p WatchlistLoadFile(TB_p bank, char* watchlist_filename, 
   IOFormat parse_format);
void WatchlistLoadDir(WatchlistControl_p wlcontrol, TB_p bank, 
   char* watchlist_dir, IOFormat parse_format);
void WatchlistLoaded(WatchlistControl_p wlcontrol);
void WatchlistInit(WatchlistControl_p wlcontrol, OCB_p ocb);
void WatchlistReset(WatchlistControl_p wlcontrol);

void WatchlistInitFVI(WatchlistControl_p wlcontrol, FVCollect_p cspec, 
   PermVector_p perm);

void WatchlistClauseProcessed(WatchlistControl_p wlcontrol, Clause_p clause);

bool WatchlistEmpty(WatchlistControl_p wlcontrol);

void WatchlistArchive(WatchlistControl_p wlcontrol, ClauseSet_p archive);

void WatchlistUnfoldEqDef(WatchlistControl_p wlcontrol, ClausePos_p demod);

void WatchlistIndicesInit(WatchlistControl_p wlcontrol, Sig_p sig,
   char* rw_bw_index_type, char* pm_from_index_type, char* pm_into_index_type);

#endif

/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/
