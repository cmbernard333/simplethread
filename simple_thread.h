#ifndef _simple_thread_h_
#define _simple_thread_h_

/* forward declared structures */ 
typedef struct simple_thread_handle_t SimpleThreadHandle; 
typedef struct thread_condition_var_t ThreadConditionVar;
typedef struct thread_lock_t ThreadLock;

/* generic thread task that returns an integer */
typedef int (*SimpleThreadTask)(void*);

/* TODO: anything currently returning int should return one of these enums */
typedef enum {
	SIMPLE_THREAD_OK=0, /* success! */
	SIMPLE_THREAD_TIMEOUT, /* condition timeout lasped */
	SIMPLE_THREAD_INVAL, /* invalid lock or condition variable */
	SIMPLE_THREAD_DEADLOCK, /* lock has already been locked by current thread */
	SIMPLE_THREAD_NOMEM, /* low on memory usage */
	SIMPLE_THREAD_BUSY, /* lock is busy */
	SIMPLE_THREAD_ERROR, /* general error */
	SIMPLE_THREAD_PERMISSION_DENIED, /* lock owned by another thread */
}SimpleThreadResult;

/* creating threads */
extern int SimpleThread_createThread(SimpleThreadHandle **_handle, SimpleThreadTask task, void *data);
extern int SimpleThread_closeHandle(SimpleThreadHandle *handle);
extern int SimpleThread_joinThread(SimpleThreadHandle *handle);

/* condition variables */
extern int ThreadConditionVar_init(ThreadConditionVar **_var);
extern int ThreadConditionVar_cleanup(ThreadConditionVar *var);
extern int ThreadConditionVar_wait_lock(ThreadConditionVar *var, ThreadLock *lock);
extern int ThreadConditionVar_wait_lock_ms(ThreadConditionVar *var, ThreadLock *lock, unsigned long ms_wait);
extern int ThreadConditionVar_signal(ThreadConditionVar *var);
extern int ThreadConditionVar_signal_all(ThreadConditionVar *var);

/* locks */
extern int ThreadLock_init(ThreadLock **_lock);
extern int ThreadLock_cleanup(ThreadLock *lock);
/*
	lock function will block until lock is acquireable
*/
extern int ThreadLock_lock(ThreadLock *lock);
/* 
	try lock function will return immediately if the lock is currently acquired by another thread
*/
extern int ThreadLock_trylock(ThreadLock *lock);
extern int ThreadLock_unlock(ThreadLock *lock);

#endif /* _simple_thread_h_ */
