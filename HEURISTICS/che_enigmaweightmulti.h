/*-----------------------------------------------------------------------

File  : che_enigmaweightmulti.h

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

#ifndef CHE_ENIGMAWEIGHTMULTI

#define CHE_ENIGMAWEIGHTMULTI

#include <ccl_relevance.h>
#include <che_refinedweight.h>
#include <che_enigma.h>
#include "linear.h"


/*---------------------------------------------------------------------*/
/*                    Data type declarations                           */
/*---------------------------------------------------------------------*/

typedef struct enigmaweightmultiparamcell
{
   OCB_p        ocb;
   ProofState_p proofstate;

   char* models_dir;
   char* features_filename;

   double len_mult;
   
   PStack_p models;
   Enigmap_p enigmap;
   //struct model* linear_model;
   
   struct feature_node* conj_features;
   int conj_features_count;

   void   (*init_fun)(struct enigmaweightmultiparamcell*);
}EnigmaWeightMultiParamCell, *EnigmaWeightMultiParam_p;

/*---------------------------------------------------------------------*/
/*                Exported Functions and Variables                     */
/*---------------------------------------------------------------------*/

#define EnigmaWeightMultiParamCellAlloc() (EnigmaWeightMultiParamCell*) \
        SizeMalloc(sizeof(EnigmaWeightMultiParamCell))
#define EnigmaWeightMultiParamCellFree(junk) \
        SizeFree(junk, sizeof(EnigmaWeightMultiParamCell))

EnigmaWeightMultiParam_p EnigmaWeightMultiParamAlloc(void);
void              EnigmaWeightMultiParamFree(EnigmaWeightMultiParam_p junk);


WFCB_p EnigmaWeightMultiParse(
   Scanner_p in, 
   OCB_p ocb, 
   ProofState_p state);

WFCB_p EnigmaWeightMultiInit(
   ClausePrioFun prio_fun, 
   OCB_p ocb,
   ProofState_p proofstate,
   char* models_dir,
   char* features_filename,
   double len_mult);

double EnigmaWeightMultiCompute(void* data, Clause_p clause);

void EnigmaWeightMultiExit(void* data);

#endif

/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/

