#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#ifdef HAVE_PTHREAD_H
# include <pthread.h>
#endif

#include "sysbench.h"
#include "sb_ck_pr.h"

/* How to test scheduler pthread_yield or sched_yield */
#ifdef HAVE_PTHREAD_YIELD
#define YIELD pthread_yield 
#else
#define YIELD sched_yield
#endif

/* Threads test arguments */
static sb_arg_t threads_args[] =
{
  SB_OPT("thread-yields", "number of yields to do per request", "1000", INT),
  SB_OPT("thread-locks", "number of locks per thread", "8", INT),

  SB_OPT_END
};

/* Threads test operations */
static int threads_init(void);
static int threads_prepare(void);
static void threads_print_mode(void);
static sb_event_t threads_next_event(int);
static int threads_execute_event(sb_event_t *, int);
static int threads_cleanup(void);

static sb_test_t threads_test =
{
  .sname = "gameserver",
  .lname = "Game server subsystem performance test",
  .ops = {
    .init = threads_init,
    .prepare = threads_prepare,
    .print_mode = threads_print_mode,
    .next_event = threads_next_event,
    .execute_event = threads_execute_event,
    .cleanup = threads_cleanup
  },
  .args = threads_args
};

static unsigned int thread_yields;
static unsigned int thread_locks;
static pthread_mutex_t *test_mutexes;
static unsigned int req_performed;


int register_test_gameserver(sb_list_t *tests)
{
  SB_LIST_ADD_TAIL(&threads_test.listitem, tests);

  return 0;
}


int threads_init(void)
{
  thread_yields = sb_get_value_int("thread-yields");
  thread_locks = sb_get_value_int("thread-locks");
  req_performed = 0;

  return 0;
}


int threads_prepare(void)
{
  unsigned int i;

  test_mutexes = (pthread_mutex_t *)malloc(thread_locks *
                                           sizeof(pthread_mutex_t));
  if (test_mutexes == NULL)
  {
    log_text(LOG_FATAL, "Memory allocation failure!");
    return 1;
  }
  
  for(i = 0; i < thread_locks; i++)
    pthread_mutex_init(test_mutexes + i, NULL);

  return 0;
}


int threads_cleanup(void)
{
  unsigned int i;

  for(i=0; i < thread_locks; i++)
    pthread_mutex_destroy(test_mutexes + i);
  free(test_mutexes);
  
  return 0;
}


sb_event_t threads_next_event(int thread_id)
{
  sb_event_t         sb_req;
  sb_gameserver_request_t *threads_req = &sb_req.u.gameserver_request;

  (void) thread_id; /* unused */

  sb_req.type = SB_REQ_TYPE_GAMESERVER;
  threads_req->lock_num = ck_pr_faa_uint(&req_performed, 1) % thread_locks;

  return sb_req;
}


int threads_execute_event(sb_event_t *sb_req, int thread_id)
{
  unsigned int         i;
  sb_gameserver_request_t *threads_req = &sb_req->u.gameserver_request;

  (void) thread_id; /* unused */

  for(i = 0; i < thread_yields; i++)
  {
    pthread_mutex_lock(&test_mutexes[threads_req->lock_num]);
    YIELD();
    pthread_mutex_unlock(&test_mutexes[threads_req->lock_num]);
  }

  return 0;
}


void threads_print_mode(void)
{
  log_text(LOG_INFO, "Doing thread subsystem performance test");
  log_text(LOG_INFO, "Thread yields per test: %d Locks used: %d",
         thread_yields, thread_locks);
}

