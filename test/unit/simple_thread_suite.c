#include "cutest.h"
#include "../../simple_thread_internal.h"

#ifdef NT
#define sleep(x) Sleep(x*1000)
#endif

unsigned int count = 0;

/* fake atomic integer structure */
typedef struct atomic_integer_t
{
  int x;
  ThreadLock *lock;
}AtomicInteger;

typedef struct monitor_t
{
  ThreadConditionVar *condition;
  ThreadLock *lock;
}ThreadMonitor;


int test_thread(void *data)
{
	fprintf(stdout,"Test thread: Hello, World!\n");
	return 0;
}

int worker_trylock_one(void *data)
{
  ThreadLock *lock = (ThreadLock*)data;
  ThreadLock_lock(lock);
  sleep(10);
  ThreadLock_unlock(lock);
  return 0;
}

int worker_trylock_two(void *data)
{
  int rc = 0;
  ThreadLock *lock = (ThreadLock*)data;
  rc = ThreadLock_trylock(lock);
  CU_ASSERT_NOT_EQUAL(rc,0);
  return 0;
}

int worker_one(void *data)
{
  int rc = 0;
  AtomicInteger *integer = (AtomicInteger*)data;
  fprintf(stdout,"worker_one: running!\n");
  ThreadLock_lock(integer->lock);
  fprintf(stdout,"worker_one: lock!\n");
  integer->x = 10;
  CU_ASSERT_EQUAL(integer->x, 10);

  sleep(5);
  CU_ASSERT_EQUAL(integer->x, 10);
  ThreadLock_unlock(integer->lock);
  fprintf(stdout,"worker_one: unlock!\n");
  return 0;
}

int worker_two(void *data)
{
  int rc = 0;
  AtomicInteger *integer = (AtomicInteger*)data;
  fprintf(stdout,"worker_two: running!\n");
  ThreadLock_lock(integer->lock);
  fprintf(stdout,"worker_two: lock!\n");
  integer->x = 11;
  CU_ASSERT_EQUAL(integer->x, 11);

  sleep(5);
  CU_ASSERT_EQUAL(integer->x, 11);
  ThreadLock_unlock(integer->lock);
  fprintf(stdout,"worker_two: unlock!\n");
  return 0;
}

int condition_worker_one(void *data)
{
  int rc = 0;
  ThreadMonitor *monitor = (ThreadMonitor*)data;
  fprintf(stdout,"condition_worker_one: running!\n");
  fprintf(stdout,"condition_worker_one: waiting on condition!\n");
  ThreadLock_lock(monitor->lock);
  while( count == 0 ) {
	rc = ThreadConditionVar_wait_lock_ms(monitor->condition,monitor->lock,10*1000);
  }
  ThreadLock_unlock(monitor->lock);
  CU_ASSERT_EQUAL(rc, 0);
  fprintf(stdout,"condition_worker_one: woke up!");
  return rc;
}

int condition_worker_two(void *data)
{
  int rc = 0;
  ThreadMonitor *monitor = (ThreadMonitor*)data;
  fprintf(stdout,"condition_worker_two: running!\n");
  fprintf(stdout,"condition_worker_two: signaling condition!\n");
  sleep(5);
  
  ThreadLock_lock(monitor->lock);
  count = count + 1;
  ThreadLock_unlock(monitor->lock);
  
  rc = ThreadConditionVar_signal(monitor->condition);
  CU_ASSERT_EQUAL(rc, 0);
  return rc;
}


static int init_simple_thread_suite(void)
{
    return 0;
}

static int clean_simple_thread_suite(void)
{
    return 0;
}

static void test_thread_condition_init_and_cleanup(void)
{
	int rc = 0;
	ThreadConditionVar *condvar = NULL;

	rc = ThreadConditionVar_init(&condvar);

	CU_ASSERT_EQUAL(rc, 0);
	CU_ASSERT_PTR_NOT_NULL(condvar);

	rc = ThreadConditionVar_cleanup(condvar);
	
	CU_ASSERT_EQUAL(rc, 0);
}

static void test_thread_start_join(void)
{
	int rc = 0;
	SimpleThreadHandle *handle;
	rc = SimpleThread_createThread(&handle, test_thread,NULL);
  CU_ASSERT_EQUAL(rc, 0);
	CU_ASSERT_PTR_NOT_NULL(handle);

	rc = SimpleThread_joinThread(handle);
	CU_ASSERT_EQUAL(rc, 0);
	SimpleThread_closeHandle(handle);
}

static void test_lock_init_cleanup(void)
{
  int rc = 0;
  ThreadLock *lock;
  rc = ThreadLock_init(&lock);

  CU_ASSERT_EQUAL(rc, 0);
  CU_ASSERT_PTR_NOT_NULL(lock);

  rc = ThreadLock_cleanup(lock);
  CU_ASSERT_EQUAL(rc, 0);
}

static void test_thread_lock_on_integer(void)
{
  int rc = 0;
  AtomicInteger atomic_integer;
  SimpleThreadHandle *handle_one;
  SimpleThreadHandle *handle_two;

  /* lock */
  rc = ThreadLock_init(&atomic_integer.lock);
  CU_ASSERT_EQUAL(rc,0);

  /* threads */
  rc = SimpleThread_createThread(&handle_one, worker_one, &atomic_integer);
  CU_ASSERT_EQUAL(rc, 0);
  CU_ASSERT_PTR_NOT_NULL(handle_one);
  rc = SimpleThread_createThread(&handle_two, worker_two, &atomic_integer);
  CU_ASSERT_EQUAL(rc, 0);
  CU_ASSERT_PTR_NOT_NULL(handle_two);

  /* wait for the threads */
  rc = SimpleThread_joinThread(handle_one);
  CU_ASSERT_EQUAL(rc,0);
  rc = SimpleThread_joinThread(handle_two);
  CU_ASSERT_EQUAL(rc,0);

  /* cleanup */
  rc = ThreadLock_cleanup(atomic_integer.lock);
  CU_ASSERT_EQUAL(rc,0);
  SimpleThread_closeHandle(handle_one);
  SimpleThread_closeHandle(handle_two);
}

static void test_thread_condition_var_wait_and_signal(void)
{
  int rc = 0;
  SimpleThreadHandle *handle_one;
  SimpleThreadHandle *handle_two;
  ThreadConditionVar *signal_wake;
  ThreadLock *signal_lock;
  ThreadMonitor monitor;

  /* setup the signal */
  rc = ThreadConditionVar_init(&signal_wake);
  CU_ASSERT_EQUAL(rc, 0);

  /* setup the lock */
  rc = ThreadLock_init(&signal_lock);
  CU_ASSERT_EQUAL(rc, 0);
  
  monitor.condition = signal_wake;
  monitor.lock = signal_lock;

  /* threads */
  rc = SimpleThread_createThread(&handle_one, condition_worker_one, &monitor);
  CU_ASSERT_EQUAL(rc, 0);
  CU_ASSERT_PTR_NOT_NULL(handle_one);
  rc = SimpleThread_createThread(&handle_two, condition_worker_two, &monitor);
  CU_ASSERT_EQUAL(rc, 0);
  CU_ASSERT_PTR_NOT_NULL(handle_two);

  /* join with thread 1 */
  rc = SimpleThread_joinThread(handle_one);
  CU_ASSERT_EQUAL(rc, 0);

  /* join with thread 2 */
  rc = SimpleThread_joinThread(handle_two);
  CU_ASSERT_EQUAL(rc, 0);

  /* clean up */
  rc = ThreadConditionVar_cleanup(monitor.condition);
  CU_ASSERT_EQUAL(rc, 0);
  rc = ThreadLock_cleanup(monitor.lock);
  CU_ASSERT_EQUAL(rc, 0);
  SimpleThread_closeHandle(handle_one);
  SimpleThread_closeHandle(handle_two);
}

static void test_thread_try_lock(void)
{
  int rc = 0;
  SimpleThreadHandle *handle_one;
  SimpleThreadHandle *handle_two;
  ThreadLock *lock;
  /* setup lock */
  rc = ThreadLock_init(&lock);
  CU_ASSERT_EQUAL(rc,0);

  /* threads */
  rc = SimpleThread_createThread(&handle_one,worker_trylock_one,lock);
  CU_ASSERT_EQUAL(rc, 0);
  CU_ASSERT_PTR_NOT_NULL(handle_one);
  rc = SimpleThread_createThread(&handle_two,worker_trylock_two,lock);
  CU_ASSERT_EQUAL(rc,0);
  CU_ASSERT_PTR_NOT_NULL(handle_two);

  /* join with thread 1 */
  rc = SimpleThread_joinThread(handle_one);
  CU_ASSERT_EQUAL(rc, 0);

  /* join with thread 2 */
  rc = SimpleThread_joinThread(handle_two);
  CU_ASSERT_EQUAL(rc, 0);

  rc = ThreadLock_cleanup(lock);
  CU_ASSERT_EQUAL(rc, 0);
  SimpleThread_closeHandle(handle_one);
  SimpleThread_closeHandle(handle_two);
}

int setup_simple_thread_suite(void)
{
   CU_pSuite pSuite = NULL;

   pSuite = CU_add_suite("bus thread unit tests", init_simple_thread_suite, clean_simple_thread_suite);
   if (NULL == pSuite) {
      CU_cleanup_registry();
      return -1;
   }

   /* add the tests to the suite */
   if ((NULL == CU_add_test(pSuite, "test thread condition init and cleanup",
      test_thread_condition_init_and_cleanup)
    || NULL == CU_add_test(pSuite, "test thread start and join",
      test_thread_start_join)
    || NULL == CU_add_test(pSuite, "test thread lock init and cleanup",
      test_lock_init_cleanup)
    || NULL == CU_add_test(pSuite, "test thread locking with two workers",
      test_thread_lock_on_integer)
    || NULL == CU_add_test(pSuite, "test thread wait and signal condition",
      test_thread_condition_var_wait_and_signal)
    || NULL == CU_add_test(pSuite, "test thread try lock function",
      test_thread_try_lock))
   ) {
       CU_cleanup_registry();
       return -1;
   }

   return 0;
}

