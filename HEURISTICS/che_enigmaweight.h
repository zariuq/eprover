/*-----------------------------------------------------------------------

File  : che_enigmaweight.h

Author: could be anyone

Contents
 
  Auto generated. Your comment goes here ;-).

  Copyright 2016 by the author.
  This code is released under the GNU General Public Licence.
  See the file COPYING in the main directory for details.
  Run "eprover -h" for contact information.

Changes

<1> Tue Mar  8 22:40:31 CET 2016
    New

-----------------------------------------------------------------------*/

#ifndef CHE_ENIGMAWEIGHT

#define CHE_ENIGMAWEIGHT

#include <ccl_relevance.h>
#include <che_refinedweight.h>
#include <che_enigma.h>
#include "linear.h"


/*---------------------------------------------------------------------*/
/*                    Data type declarations                           */
/*---------------------------------------------------------------------*/

typedef struct enigmaweightparamcell
{
   OCB_p        ocb;
   ProofState_p proofstate;

   char* model_filename;
   char* features_filename;

   double len_mult;

   struct model* linear_model;
   Enigmap_p enigmap;
   
   struct feature_node* conj_features;
   int conj_features_count;

   void   (*init_fun)(struct enigmaweightparamcell*);
}EnigmaWeightParamCell, *EnigmaWeightParam_p;

/*---------------------------------------------------------------------*/
/*                Exported Functions and Variables                     */
/*---------------------------------------------------------------------*/

#define EnigmaWeightParamCellAlloc() (EnigmaWeightParamCell*) \
        SizeMalloc(sizeof(EnigmaWeightParamCell))
#define EnigmaWeightParamCellFree(junk) \
        SizeFree(junk, sizeof(EnigmaWeightParamCell))

EnigmaWeightParam_p EnigmaWeightParamAlloc(void);
void              EnigmaWeightParamFree(EnigmaWeightParam_p junk);


WFCB_p EnigmaWeightParse(
   Scanner_p in, 
   OCB_p ocb, 
   ProofState_p state);

WFCB_p EnigmaWeightInit(
   ClausePrioFun prio_fun, 
   OCB_p ocb,
   ProofState_p proofstate,
   char* model_filename,
   char* features_filename,
   double len_mult);

double EnigmaWeightCompute(void* data, Clause_p clause);

void EnigmaWeightExit(void* data);

#endif

/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/

