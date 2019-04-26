#include "c_api.h"
#include "stdio.h"

int main(void)
{
   BoosterHandle handle;
   int iters, ret;

   ret = LGBM_BoosterCreateFromModelfile("model.lgb", &iters, &handle);

   printf("ret = %d\n", ret);
   printf("%s\n", LGBM_GetLastError());

}


