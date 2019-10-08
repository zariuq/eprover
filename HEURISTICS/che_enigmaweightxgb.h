/*-----------------------------------------------------------------------

File  : che_enigmaweightxgb.h

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

#ifndef CHE_ENIGMAWEIGHTXGB

#define CHE_ENIGMAWEIGHTXGB

#include <ccl_relevance.h>
#include <che_refinedweight.h>
#include <ccl_processed_state.h>
#include <che_enigma.h>
#include "xgboost.h"


/*---------------------------------------------------------------------*/
/*                    Data type declarations                           */
/*---------------------------------------------------------------------*/

typedef struct enigmaweightxgbparamcell
{
   OCB_p        ocb;
   ProofState_p proofstate;

   char* model_filename;
   char* features_filename;

   double len_mult;

   BoosterHandle xgboost_model;
   Enigmap_p enigmap;

   unsigned* conj_features_indices;
   float* conj_features_data;
   int conj_features_count;

   void   (*init_fun)(struct enigmaweightxgbparamcell*);
}EnigmaWeightXgbParamCell, *EnigmaWeightXgbParam_p;

/*---------------------------------------------------------------------*/
/*                Exported Functions and Variables                     */
/*---------------------------------------------------------------------*/

#define EnigmaWeightXgbParamCellAlloc() (EnigmaWeightXgbParamCell*) \
        SizeMalloc(sizeof(EnigmaWeightXgbParamCell))
#define EnigmaWeightXgbParamCellFree(junk) \
        SizeFree(junk, sizeof(EnigmaWeightXgbParamCell))

EnigmaWeightXgbParam_p EnigmaWeightXgbParamAlloc(void);
void              EnigmaWeightXgbParamFree(EnigmaWeightXgbParam_p junk);


WFCB_p EnigmaWeightXgbParse(
   Scanner_p in,
   OCB_p ocb,
   ProofState_p state);

WFCB_p EnigmaWeightXgbInit(
   ClausePrioFun prio_fun,
   OCB_p ocb,
   ProofState_p proofstate,
   char* model_filename,
   char* features_filename,
   double len_mult);

void ProcessedClauseVectorAddClause(ProcessedState_p processed_state, Clause_p clause, unsigned long processed_count);
double EnigmaWeightXgbCompute(void* data, Clause_p clause);

void EnigmaWeightXgbExit(void* data);

#endif

/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/
