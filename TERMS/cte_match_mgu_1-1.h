/*-----------------------------------------------------------------------

File  : cte_match_mgu_1-1.h

Author: Stephan Schulz

Contents

  Interface to simple, non-indexed 1-1 match and unification
  routines on shared terms (and unshared terms with shared
  variables).

  Copyright 1998, 1999 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

Changes

<1> Wed Mar 11 16:17:33 MET 1998
    New

-----------------------------------------------------------------------*/

#ifndef CTE_MATCH_MGU_1_1

#define CTE_MATCH_MGU_1_1

#include <clb_os_wrapper.h>
#include <cte_subst.h>

/*---------------------------------------------------------------------*/
/*                    Data type declarations                           */
/*---------------------------------------------------------------------*/
typedef enum which_term {
   NoTerm = 0,
   LeftTerm = 1,
   RightTerm = 2
} WhichTerm;

typedef struct unif_res{
   WhichTerm term_side;
   int       term_remaining;
} UnificationResult;

extern const UnificationResult UNIF_FAILED;
extern const UnificationResult UNIF_INIT;

#define UnifFailed(u_res) ((u_res).term_side == NoTerm)
#define UnifIsInit(u_res) ((u_res).term_side == NoTerm && (u_res).term_remaining == -2)


#define GetSideStr(ur) ((ur).term_side == NoTerm ? "Failed" : \
                          (ur).term_side == LeftTerm ? "Left" : "Right")


/*---------------------------------------------------------------------*/
/*                Exported Functions and Variables                     */
/*---------------------------------------------------------------------*/

#ifdef MEASURE_UNIFICATION
extern long UnifAttempts;
extern long UnifSuccesses;
#endif

PERF_CTR_DECL(MguTimer);

#define NOT_MATCHED -1

// FO matching and unification
bool SubstComputeMatch(Term_p matcher, Term_p to_match, Subst_p subst);
bool SubstComputeMgu(Term_p t1, Term_p t2, Subst_p subst);

// HO matching and unification
int  PartiallyMatchVar(Term_p var_matcher, Term_p to_match, Sig_p sig, bool perform_occur_check);
int SubstComputeMatchHO(Term_p matcher, Term_p to_match, Subst_p subst, TB_p bank);
UnificationResult SubstComputeMguHO(Term_p t1, Term_p t2, Subst_p subst, TB_p bank);


#ifdef ENABLE_LFHO

// If we're working in HOL mode, we choose run FO/HO unification/matching
// based ont the problem type.
bool SubstMatchComplete(Term_p t, Term_p s, Subst_p subst, TB_p bank);
bool SubstMguComplete(Term_p t, Term_p s, Subst_p subst, TB_p bank);
int SubstMatchPossiblyPartial(Term_p t, Term_p s, Subst_p subst, TB_p bank);

#else

// If we are working in FOL mode, we revert to normal E behavior.
#define SubstMatchComplete(t, s, subst, b) (SubstComputeMatch(t, s, subst))
#define SubstMguComplete(t, s, subst, b)   (SubstComputeMgu(t, s, subst))
#define SubstMatchPossiblyPartial(t, s, subst, b)  (SubstComputeMatch(t, s, subst) ? 0 : NOT_MATCHED)

#endif

// the return result is considerably more complex, so we have to run wrapper
UnificationResult SubstMguPossiblyPartial(Term_p t, Term_p s, Subst_p subst, TB_p sig);



#define VerifyMatch(matcher, to_match) \
        TermStructEqualDeref((matcher), (to_match), \
              DEREF_ONCE, DEREF_NEVER)

#endif

/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/





