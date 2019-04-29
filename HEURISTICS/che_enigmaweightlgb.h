/*-----------------------------------------------------------------------

File  : che_enigmaweightlgb.h

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

#ifndef CHE_ENIGMAWEIGHTLGB

#define CHE_ENIGMAWEIGHTLGB

#include <ccl_relevance.h>
#include <che_refinedweight.h>
#include <che_enigma.h>
#include "lightgbm.h"


/*---------------------------------------------------------------------*/
/*                    Data type declarations                           */
/*---------------------------------------------------------------------*/

typedef struct enigmaweightlgbparamcell
{
   OCB_p        ocb;
   ProofState_p proofstate;

   char* model_filename;
   char* features_filename;

   double len_mult;

   BoosterHandle lgb_model;
   Enigmap_p enigmap;
   
   int32_t* conj_features_indices;
   float* conj_features_data;
   int conj_features_count;

   void   (*init_fun)(struct enigmaweightlgbparamcell*);
}EnigmaWeightLgbParamCell, *EnigmaWeightLgbParam_p;

/*---------------------------------------------------------------------*/
/*                Exported Functions and Variables                     */
/*---------------------------------------------------------------------*/

#define EnigmaWeightLgbParamCellAlloc() (EnigmaWeightLgbParamCell*) \
        SizeMalloc(sizeof(EnigmaWeightLgbParamCell))
#define EnigmaWeightLgbParamCellFree(junk) \
        SizeFree(junk, sizeof(EnigmaWeightLgbParamCell))

EnigmaWeightLgbParam_p EnigmaWeightLgbParamAlloc(void);
void              EnigmaWeightLgbParamFree(EnigmaWeightLgbParam_p junk);


WFCB_p EnigmaWeightLgbParse(
   Scanner_p in, 
   OCB_p ocb, 
   ProofState_p state);

WFCB_p EnigmaWeightLgbInit(
   ClausePrioFun prio_fun, 
   OCB_p ocb,
   ProofState_p proofstate,
   char* model_filename,
   char* features_filename,
   double len_mult);

double EnigmaWeightLgbCompute(void* data, Clause_p clause);

void EnigmaWeightLgbExit(void* data);

#endif

/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/

