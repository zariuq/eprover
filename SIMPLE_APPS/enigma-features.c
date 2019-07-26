/*-----------------------------------------------------------------------

File  : fofshared.c

Author: Josef Urban

Contents
 
  Read an initial set of fof terms and print the (shared) codes of all subterms
  present in them. If file names given (or file and stdin), read both
  in, but only print the codes for the second one (this is intended to
  allow consistent codes over several runs).
 

  Copyright 1998, 1999 by the author.
  This code is released under the GNU General Public Licence.
  See the file COPYING in the main CLIB directory for details.
  Run "eprover -h" for contact information.

Changes

<1> Fri Nov 28 00:27:40 MET 1997

-----------------------------------------------------------------------*/

#include <stdio.h>
#include <string.h>
#include <cio_commandline.h>
#include <cio_output.h>
#include <cte_termbanks.h>
#include <ccl_formulafunc.h>
#include <ccl_proofstate.h>
#include <che_enigma.h>

/*---------------------------------------------------------------------*/
/*                  Data types                                         */
/*---------------------------------------------------------------------*/

typedef enum
{
   OPT_NOOPT=0,
   OPT_HELP,
   OPT_VERBOSE,
   OPT_FREE_NUMBERS,
   OPT_ENIGMA_FEATURES,
   OPT_FEATURE_HASHING,
   OPT_OUTPUT,
   OPT_ENIGMAP_OUTPUT,
}OptionCodes;

/*---------------------------------------------------------------------*/
/*                        Global Variables                             */
/*---------------------------------------------------------------------*/

OptCell opts[] =
{
    {OPT_HELP, 
        'h', "help", 
        NoArg, NULL,
        "Print a short description of program usage and options."},
    {OPT_VERBOSE, 
        'v', "verbose", 
        OptArg, "1",
        "Verbose comments on the progress of the program."},
    {OPT_OUTPUT,
        'o', "output-file",
        ReqArg, NULL,
        "Redirect output into the named file."},
    {OPT_ENIGMAP_OUTPUT,
        '\0', "enigmap-file",
        ReqArg, NULL,
        "Save Enigma feature map to the named file."},
   {OPT_ENIGMA_FEATURES,
      '\0', "enigma-features",
      ReqArg, NULL,
      "Enigma features to be generate. The value is a string of characters "
      "determining the features to be used. Valid characters are 'V' "
      "(vertical features), 'H' (horizontal), 'S' (symbols), 'L' "
      "(length stats), 'C' (conjecture)."},
   {OPT_FEATURE_HASHING,
      '\0', "feature-hashing",
      ReqArg, NULL,
      "Enigma features hashing is always on.  The argument specifies the numeric "
      "hash base.  The default is 32768, that is, 2^15."},
   {OPT_FREE_NUMBERS,
    '\0', "free-numbers",
     NoArg, NULL,
     "Treat numbers (strings of decimal digits) as normal free function "
    "symbols in the input. By default, number now are supposed to denote"
    " domain constants and to be implicitly different from each other."},
    {OPT_NOOPT,
        '\0', NULL,
        NoArg, NULL,
        NULL}
};

char *outname = NULL;
char *enigmapname= NULL;
FunctionProperties free_symb_prop = FPIgnoreProps;
EnigmaFeatures Enigma = EFAll;
unsigned long FeatureHashing = 32768L;

/*---------------------------------------------------------------------*/
/*                      Forward Declarations                           */
/*---------------------------------------------------------------------*/

CLState_p process_options(int argc, char* argv[]);
void print_help(FILE* out);

/*---------------------------------------------------------------------*/
/*                         Internal Functions                          */
/*---------------------------------------------------------------------*/


static NumTree_p get_conjecture_features(char* filename, TB_p bank, Enigmap_p enigmap)
{
   int len = 0;
   NumTree_p features = NULL;
   Scanner_p in = CreateScanner(StreamTypeFile, filename, true, NULL);
   ScannerSetFormat(in, TSTPFormat);
   while (TestInpId(in, "cnf"))
   {
      Clause_p clause = ClauseParse(in, bank);
      if (ClauseQueryTPTPType(clause) == CPTypeNegConjecture) 
      {
         len += FeaturesClauseExtend(&features, clause, enigmap);
         FeaturesAddClauseStatic(&features, clause, enigmap, &len);
      }
      //if (len >= 2048) { Error("ENIGMA: Too many conjecture features!", OTHER_ERROR); } 
      ClauseFree(clause);
   }
   CheckInpTok(in, NoToken);
   DestroyScanner(in);

   return features;
}

static void dump_features_hashes(FILE* out, char* filename, TB_p bank, char* prefix, NumTree_p conj_features, Enigmap_p enigmap)
{
   PStack_p stack;
   Scanner_p in = CreateScanner(StreamTypeFile, filename, true, NULL);
   ScannerSetFormat(in, TSTPFormat);
   NumTree_p node;

   while (TestInpId(in, "cnf"))
   {
      Clause_p clause = ClauseParse(in, bank);
      fprintf(out, prefix);
   
      int len = 0;
      NumTree_p features = FeaturesClauseCollect(clause, enigmap, &len);
      stack = NumTreeTraverseInit(features);
      while ((node = NumTreeTraverseNext(stack)))
      {
         fprintf(out, " %ld:%ld", node->key, node->val1.i_val);
      }
      NumTreeTraverseExit(stack);
      NumTreeFree(features);

      stack = NumTreeTraverseInit(conj_features);
      while ((node = NumTreeTraverseNext(stack)))
      {
         fprintf(out, " %ld:%ld", node->key+enigmap->feature_count, node->val1.i_val);
      }
      NumTreeTraverseExit(stack);

      ClauseFree(clause);
      fprintf(out, "\n");
   }
   CheckInpTok(in, NoToken);
   DestroyScanner(in);
}
      
static void dump_enigmap_stats(char* fname, Enigmap_p enigmap)
{
   PStack_p stack;
   StrTree_p node;

   FILE* out = fopen(fname, "w");
   
   stack = StrTreeTraverseInit(enigmap->stats);
   while((node = StrTreeTraverseNext(stack)))
   {
      fprintf(out, "usage(\"%s\",%ld,%ld).\n", node->key, node->val1.i_val, node->val2.i_val);
   }
   StrTreeTraverseExit(stack);

   fclose(out);
}

int main(int argc, char* argv[])
{
   InitIO(argv[0]);
   CLState_p args = process_options(argc, argv);
   //SetMemoryLimit(2L*1024*MEGA);
   OutputFormat = TSTPFormat;
   if (outname) { OpenGlobalOut(outname); }
   ProofState_p state = ProofStateAlloc(free_symb_prop);
   TB_p bank = state->terms;
  
   DStr_p dstr = NULL;
   NumTree_p conj_features = NULL;

   Enigmap_p enigmap = NULL;
   enigmap = EnigmapAlloc();
   enigmap->feature_count = FeatureHashing;
   enigmap->version = Enigma;
   enigmap->sig = bank->sig;
   enigmap->collect_stats = (enigmapname) ? true : false;
   enigmap->stats = NULL;

   if ((Enigma & EFConjecture) && (args->argc == 3))
   {
      conj_features = get_conjecture_features(args->argv[2], bank, enigmap);
   }
   if (args->argc == 1)
   {
      dump_features_hashes(GlobalOut, args->argv[0], bank, "+?", conj_features, enigmap);
   }
   else
   {
      dump_features_hashes(GlobalOut, args->argv[0], bank, "+1", conj_features, enigmap);
      dump_features_hashes(GlobalOut, args->argv[1], bank, "+0", conj_features, enigmap);
   }

   if (enigmapname) 
   {
      dump_enigmap_stats(enigmapname, enigmap);
   }

   if (dstr) { DStrFree(dstr); }
   if (enigmap) { EnigmapFree(enigmap); }
   if (conj_features) { NumTreeFree(conj_features); }
   //TBFree(bank);
   //SigFree(sig);
   ProofStateFree(state);
   CLStateFree(args);
   ExitIO();

   return 0;
}


/*-----------------------------------------------------------------------
//
// Function: process_options()
//
//   Read and process the command line option, return (the pointer to)
//   a CLState object containing the remaining arguments.
//
// Global Variables: opts, Verbose, TermPrologArgs,
//                   TBPrintInternalInfo 
//
// Side Effects    : Sets variables, may terminate with program
//                   description if option -h or --help was present
//
/----------------------------------------------------------------------*/

CLState_p process_options(int argc, char* argv[])
{
   Opt_p handle;
   CLState_p state;
   char*  arg;
   
   state = CLStateAlloc(argc,argv);
   
   while((handle = CLStateGetOpt(state, &arg, opts)))
   {
      switch(handle->option_code)
      {
      case OPT_VERBOSE:
             Verbose = CLStateGetIntArg(handle, arg);
             break;
      case OPT_HELP: 
             print_help(stdout);
             exit(NO_ERROR);
      case OPT_OUTPUT:
             outname = arg;
             break;
      case OPT_ENIGMAP_OUTPUT:
             enigmapname = arg;
             break;
      case OPT_ENIGMA_FEATURES:
             Enigma = ParseEnigmaFeaturesSpec(arg);
             break;
      case OPT_FEATURE_HASHING:
             FeatureHashing = CLStateGetIntArg(handle, arg);
      case OPT_FREE_NUMBERS:
            free_symb_prop = free_symb_prop|FPIsInteger|FPIsRational|FPIsFloat;
            break;
      default:
          assert(false);
          break;
      }
   }
   
   if (state->argc < 1 || state->argc > 3)
   {
      print_help(stdout);
      exit(NO_ERROR);
   }
   
   if (FeatureHashing && !(Enigma & EFHashing))
   {
      Error("ENIGMA: You specified a hash base but forgot 'h' in features string (--enigma-features).", USAGE_ERROR); 
   }
   //if (!FeatureHashing && (Enigma & EFHashing))
   //{
   //   Error("ENIGMA: You turned on hashing but haven't specified a hash base (--feature-hashing).", USAGE_ERROR); 
   //}

   return state;
}
 
void print_help(FILE* out)
{
   fprintf(out, "\n\
\n\
Usage: enigma-features [options] cnfs.tptp\n\
   or: enigma-features [options] train.pos train.neg [train.cnf]\n\
\n\
Make enigma features from TPTP cnf clauses.  Optionally prefixing with\n\
sign (+ or -), and postfixing with conjecture features from train.cnf.\n\
Output line format is 'sign|clause|conjecture'.\n\
\n");
   PrintOptions(stdout, opts, NULL);
}


/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/


