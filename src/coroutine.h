#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ucontext.h>
#include <unistd.h>
#include <deque>

typedef void (*co_func_t)(void *);
typedef struct coroutine_t
{
   struct coroutineEnv_t *env;
   co_func_t func;
   void *arg;
   ucontext_t ctx;
   char cStart;
   char cEnd;
   char cIsMain;
   char *stack;
   int stackSize;
}coroutine_t;

typedef struct coroutineEnv_t
{
   struct coroutine_t *pCallStack[128];
   int iCallStackSize;
}coroutineEnv_t;

typedef struct
{
   std::deque<coroutine_t* > routines;
}co_cond_t;

int co_create(coroutine_t **ppco, int stackSize, co_func_t func, void *arg);
void co_resume(coroutine_t *co);
void co_yield_env(coroutineEnv_t *env);
void co_yield();
void co_free(coroutine_t *co);
coroutine_t* co_self();

int co_cond_signal(co_cond_t* cond);
int co_cond_broadcast(co_cond_t* cond);
int co_cond_wait(co_cond_t* cond);

