#include "../src/coroutine.h"
#include <stdio.h>
#include <queue>
using namespace std;

struct stTask_t
{
   int id;
};

struct stEnv_t
{
   co_cond_t cond;
   queue<stTask_t*> task_queue;
};

static void Producer(void *ud)
{
   stEnv_t* env = (stEnv_t*)ud;
   int id=0;
   while(id<10)
   {
      stTask_t* task = (stTask_t*)calloc(1, sizeof(stTask_t));
      task->id = id++;
      env->task_queue.push(task);
      printf("%s:%d produce task %d\n", __func__, __LINE__, task->id);
      co_cond_signal(&env->cond);
   }
}

static void Consumer(void* ud)
{
   stEnv_t* env = (stEnv_t*)ud;
   while(1)
   {
      if (env->task_queue.empty())
      {
         co_cond_wait(&env->cond);
         continue;
      }
      stTask_t* task = env->task_queue.front();
      env->task_queue.pop();
      printf("%s:%d consume task %d\n", __func__, __LINE__, task->id);
      free(task);
   }
}

int main()
{
   stEnv_t* env = new stEnv_t;
   coroutine_t* consumer_routine; 
   co_create(&consumer_routine,0,Consumer,env);
   co_resume(consumer_routine);
   
   coroutine_t* producer_routine;
   co_create(&producer_routine,0,Producer,env);
   co_resume(producer_routine);

   co_free(consumer_routine);
   co_free(producer_routine);
   return 0;
}
