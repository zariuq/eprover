/*-----------------------------------------------------------------------

File:    cto_iden.c
Author:  Zar Goertzel
Contents:

  Definitions for implementing an identity ordering

Copyright 1998, 1999,2004 by the author.
This code is released under the GNU General Public Licence and
the GNU Lesser General Public License.
See the file COPYING in the main E directory for details..
Run "eprover -h" for contact information.
Changes
-----------------------------------------------------------------------*/

#include "cto_iden.h"

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
/*                      Exported Functions                             */
/*---------------------------------------------------------------------*/

/*-----------------------------------------------------------------------
//
// Function: WPOCompare(ocb, s, t)
//
// Global Variables: -
//
// Side Effects    : -
//
-----------------------------------------------------------------------*/

CompareResult IDENCompare(OCB_p ocb, Term_p s, Term_p t,
      DerefType deref_s, DerefType deref_t)
{
   if (TermStructEqualDeref(s, t, deref_s, deref_t))
   {
      return to_equal;
   }
   return to_uncomparable;
}

/*-----------------------------------------------------------------------
//
// Function: WPOGreater(ocb, s, t)
//
// Global Variables: -
//
// Side Effects    : -
//
-----------------------------------------------------------------------*/

bool IDENGreater(OCB_p ocb, Term_p s, Term_p t,
      DerefType deref_s, DerefType deref_t)
{
   return to_uncomparable;
}

/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/
