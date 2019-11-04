/*-----------------------------------------------------------------------

File  : cto_iden.h

Author: Zar Goertzel

Contents

  Definitions for implementing an identity ordering

-----------------------------------------------------------------------*/

#ifndef CTO_IDEN

#define CTO_IDEN

#include <cto_ocb.h>

/*---------------------------------------------------------------------*/
/*                    Data type declarations                           */
/*---------------------------------------------------------------------*/


/*---------------------------------------------------------------------*/
/*                Exported Functions and Variables                     */
/*---------------------------------------------------------------------*/

bool          IDENGreater(OCB_p ocb, Term_p s, Term_p t, DerefType
          deref_s, DerefType deref_t);

CompareResult IDENCompare(OCB_p ocb, Term_p t1, Term_p t2,
          DerefType deref_t1, DerefType deref_t2);

#endif

/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/
