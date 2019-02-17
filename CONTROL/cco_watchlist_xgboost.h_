/*-----------------------------------------------------------------------

  File  : cco_watchlist.h

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

#ifndef CCO_WATCHLIST

#define CCO_WATCHLIST

#include <ccl_watchlist.h>



/*---------------------------------------------------------------------*/
/*                    Data type declarations                           */
/*---------------------------------------------------------------------*/

/*---------------------------------------------------------------------*/
/*                Exported Functions and Variables                     */
/*---------------------------------------------------------------------*/

void WatchlistRemoveRewritables(Watchlist_p watchlist, ClauseSet_p rws,
   OCB_p ocb, ClauseSet_p archive, Clause_p clause);

void WatchlistInsertRewritten(Watchlist_p watchlist, ClauseSet_p rws, 
   ProofControl_p control, TB_p terms, ClauseSet_p *demodulators);

void WatchlistCheck(WatchlistControl_p wlcontrol, Clause_p clause, ClauseSet_p archive, 
   bool static_watchlist, Sig_p sig);

void WatchlistSimplify(WatchlistControl_p wlcontrol, Clause_p clause, ProofControl_p control, 
   TB_p terms, ClauseSet_p archive, ClauseSet_p* demodulators);

#endif

/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/

