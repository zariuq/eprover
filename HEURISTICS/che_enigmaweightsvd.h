/*-----------------------------------------------------------------------

File  : che_enigmaweightsvd.h

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

#ifndef CHE_ENIGMAWEIGHTSVD

#define CHE_ENIGMAWEIGHTSVD

#include <ccl_relevance.h>
#include <che_refinedweight.h>
#include <che_enigma.h>
#include "linear.h"


/*---------------------------------------------------------------------*/
/*                    Data type declarations                           */
/*---------------------------------------------------------------------*/

typedef struct enigmaweightsvdparamcell
{
   OCB_p        ocb;
   ProofState_p proofstate;

   char* model_filename;
   char* features_filename;
   char* svd_Ut_filename;
   char* svd_S_filename;

   double len_mult;

   struct model* linear_model;
   Enigmap_p enigmap;
   
   struct feature_node* conj_features;
   int conj_features_count;
   struct feature_node* dense_features;

   DMat svd_Ut;
   double* svd_S;

   void   (*init_fun)(struct enigmaweightsvdparamcell*);
}EnigmaWeightSvdParamCell, *EnigmaWeightSvdParam_p;

/*---------------------------------------------------------------------*/
/*                Exported Functions and Variables                     */
/*---------------------------------------------------------------------*/

#define EnigmaWeightSvdParamCellAlloc() (EnigmaWeightSvdParamCell*) \
        SizeMalloc(sizeof(EnigmaWeightSvdParamCell))
#define EnigmaWeightSvdParamCellFree(junk) \
        SizeFree(junk, sizeof(EnigmaWeightSvdParamCell))

EnigmaWeightSvdParam_p EnigmaWeightSvdParamAlloc(void);
void              EnigmaWeightSvdParamFree(EnigmaWeightSvdParam_p junk);


WFCB_p EnigmaWeightSvdParse(
   Scanner_p in, 
   OCB_p ocb, 
   ProofState_p state);

WFCB_p EnigmaWeightSvdInit(
   ClausePrioFun prio_fun, 
   OCB_p ocb,
   ProofState_p proofstate,
   char* model_filename,
   char* features_filename,
   char* svd_Ut_filename,
   char* svd_S_filename,
   double len_mult);

double EnigmaWeightSvdCompute(void* data, Clause_p clause);

void EnigmaWeightSvdExit(void* data);

#endif

/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/

