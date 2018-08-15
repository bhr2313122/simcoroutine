#include "coroutine.h"

static coroutineEnv_t *g_thread_envs[204800] = {0};

static coroutine_t *co_new(coroutineEnv_t *env, const int ssize, co_func_t func, void *arg)
{
   coroutine_t *co = (coroutine_t*)malloc(sizeof(coroutine_t));
   memset(co, 0, (long)sizeof(coroutine_t));

   co->env = env;
   co->func = func;
   co->arg = arg;
   co->cStart = 0;
   co->cEnd = 0;
   co->cIsMain = 0;

   if(ssize <= 0)
   {
      co->stackSize = 128 * 1024;
   }
   else if(ssize > 1024 * 1024 * 8)
   {
      co->stackSize = 1024 * 1024 * 8;
   }

   if(ssize & 0xFFF)
   {
      co->stackSize &= ~0xFFF;
      co->stackSize += 0x1000;
   }
   co->stack = (char*)malloc(co->stackSize);
   memset(co->stack, 0, co->stackSize);
   getcontext(&co->ctx);
   co->ctx.uc_stack.ss_sp = co->stack;
   co->ctx.uc_stack.ss_size = co->stackSize;

   return co;
}
static coroutineEnv_t* get_current_thread_env()
{
   return g_thread_envs[getpid()];
}

static void init_current_thread_env()
{
   pid_t pid = getpid();
   g_thread_envs[pid] = (coroutineEnv_t*)calloc(1,sizeof(coroutineEnv_t));
   coroutineEnv_t *env = g_thread_envs[pid];
   env->iCallStackSize = 0;
   coroutine_t *self = co_new(get_current_thread_env(), 0, NULL, NULL);
   self->cIsMain = 1;
   env->pCallStack[env->iCallStackSize++] = self;
}

static void cofun(void *arg)
{
   coroutine_t *co = (coroutine_t*)arg;
   if(co->func)
   {
      co->func(co->arg);
   }
   co->cEnd = 1;
   co_yield_env(co->env);
}

int co_create(coroutine_t **ppco, int ssize, co_func_t func, void *arg)
{
   if(!get_current_thread_env())
   {
      init_current_thread_env();
   }
   coroutine_t *co = co_new(get_current_thread_env(), ssize, func, arg);
   *ppco = co;
   return 0;
}

void co_resume(coroutine_t *co)
{
   coroutineEnv_t *env = co->env;
   coroutine_t *curr = env->pCallStack[env->iCallStackSize - 1];
   if(!co->cStart)
   {
      makecontext(&co->ctx, (void (*)(void))cofun, 1, co);
      co->cStart = 1;
   }
   env->pCallStack[env->iCallStackSize++] = co;
   swapcontext(&curr->ctx, &co->ctx);
}

void co_yield_env(coroutineEnv_t *env)
{
   coroutine_t *last = env->pCallStack[env->iCallStackSize - 2];
   coroutine_t *curr = env->pCallStack[env->iCallStackSize - 1];

   env->iCallStackSize--;
   swapcontext(&curr->ctx, &last->ctx);
}

void co_yield()
{
   co_yield_env(get_current_thread_env());
}

void co_free(coroutine_t *co)
{
   free(co->stack);
   free(co);
}

static coroutine_t* GetCurrCo(coroutineEnv_t* env)
{
   return env->pCallStack[ env->iCallStackSize - 1 ];
}

static coroutine_t* GetCurrThreadCo()
{
   coroutineEnv_t *env = get_current_thread_env();
   if( !env ) return 0;
   return GetCurrCo(env);
}

coroutine_t* co_self()
{
   return GetCurrThreadCo();   
}

int co_cond_free(co_cond_t *cond)
{
   free(cond);
   return 0;
}

int co_cond_wait(co_cond_t* cond)
{
   coroutine_t *co = GetCurrThreadCo(); 
   cond->routines.push_back(co);
   co_yield();
   return 0;
}

int co_cond_signal(co_cond_t* cond)
{
   coroutine_t *co = cond->routines.front();
   cond->routines.pop_front();
   co_resume(co);
   return 0;
}

int co_cond_broadcast(co_cond_t* cond)
{
   while(1)
   {
      coroutine_t *co = cond->routines.front();
      if(co == NULL)
         return 0;
      cond->routines.pop_front();
      co_resume(co);
   }
   return 0;
}
