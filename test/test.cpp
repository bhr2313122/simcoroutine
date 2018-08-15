/**
  * Copyright(C), 2017, Nsfocus
  * Name:
  * Author: Wilson Lan
  * Description:
  */
#include "../src/coroutine.h"
#include <stdio.h>

struct args {
   int n;
};


static void foo1(void* ud)
{
   struct args *arg = (struct args*)ud;
   int start = arg->n;
   int i;
   for(i=0;i<5;++i)
   {
      printf("%d\n",start+i);
      co_yield();
   }
}
int main()
{
  //co_create(coroutine_t *co, int ssize, co_func_t func, void *arg)
   struct args arg1 = { 0 };
   struct args arg2 = { 100 };
   coroutine_t* co1;
   co_create(&co1, 0, foo1, &arg1);
   coroutine_t* co2;
   co_create(&co2, 0, foo1, &arg2);
   printf("main start");
   while(1){
      co_resume(co1);
      co_resume(co2);
   }

  co_free(co1);
  co_free(co2);
}
