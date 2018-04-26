#include <stdio.h>
#include <stdlib.h>

#include <che_enigma.h>
#include "linear.h"
#include "svdlib.h"

#define FILE_SVD_S  "svd-S"
#define FILE_SVD_Ut "svd-Ut"

int main(int argc, char* argv[])
{
   int slen;
   char* line = NULL;
   size_t len = 0;
   ssize_t read;
   int i, j;
   struct feature_node sparse_nodes[20480];
   //struct feature_node dense_nodes[20480];
   struct feature_node* dense_nodes;

   if (argc==1) 
   {
      printf("usage: %s train.in [svd-S] [svd-Ut]\n", argv[0]);
      printf("use to: translate liblinear train.in using SVD matricies S and Ut\n");
      return 0;
   }

   FILE* fin = fopen(argv[1],"r");
   double* sing = svdLoadDenseArray(argc<3?FILE_SVD_S:argv[2], &slen, 0);
   if (!sing) { printf("Error: svdLoadDenseArray\n"); }
   DMat mUt = svdLoadDenseMatrix(argc<4?FILE_SVD_Ut:argv[3], SVD_F_DT);
   if (!mUt) { printf("Error: svdLoadDenseMatrix\n"); }
   dense_nodes = malloc((slen+1)*sizeof(struct feature_node));

   while ((read = getline(&line, &len, fin)) != -1) {
      long class, fid, freq;

      sscanf(line, "+%ld", &class);
      j = 0;
      for (i=0; i<read; i++) 
      {
         if (line[i]==' ') 
         {
            sscanf(&line[i], "%ld:%ld", &fid, &freq);

            sparse_nodes[j].index = fid;
            sparse_nodes[j].value = freq;
            j++;

            while (line[i]==' ') { i++; }
         }
      }
      sparse_nodes[j].index = -1;
   
      FeaturesSvdTranslate(mUt, sing, sparse_nodes, dense_nodes);

      i = 0;
      printf("+%ld ", class);
      while (dense_nodes[i].index != -1) 
      {
         printf("%d:%f ", dense_nodes[i].index, dense_nodes[i].value);
         i++;
      }
      printf("\n");
   }
   
   if (line) { free(line); }
   free(sing);
   svdFreeDMat(mUt);
   free(dense_nodes);
   fclose(fin);

   return 0;
}

