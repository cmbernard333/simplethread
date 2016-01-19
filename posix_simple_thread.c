#include "posix_simple_thread_internal.h"

void *simple_thread_worker(void *data)
{
	int rc = SIMPLE_THREAD_OK;
	SimpleThreadTaskWorker *simple_thread_worker = (SimpleThreadTaskWorker*)data;
	if(!simple_thread_worker) {
		rc = -1;
	} else {
		rc = (*simple_thread_worker->worker)(simple_thread_worker->data);
		free(simple_thread_worker);
	}
	pthread_exit(rc);
}

/* TODO:
		Stack size considerations
		Handle inheritance
		Security Attributes
		Detached
		Joinable
		Suspenable
		pthread_attr_t attr
*/
int SimpleThread_createThread(SimpleThreadHandle **_handle,SimpleThreadTask task, void *data)
{
	int rc = 0;
	pthread_attr_t attr;
	SimpleThreadHandle *handle = NULL;
	SimpleThreadTaskWorker *worker = NULL;

	if(!task)
	{
		return SIMPLE_THREAD_INVAL;
	}

	handle = calloc(1,sizeof(SimpleThreadHandle));
	if(!handle)
	{
		return SIMPLE_THREAD_NOMEM;	
	}

	worker = calloc(1,sizeof(SimpleThreadTaskWorker));
	if(worker)
	{
		worker->worker = task;
		worker->data = data;
	} else {
		return SIMPLE_THREAD_NOMEM;
	}

	rc = pthread_attr_init(&attr);
	if(rc)
	{
		return rc;
	}

	rc = pthread_create(&handle->handle, &attr, simple_thread_worker, worker);
	if(rc)
	{
		return rc;
	}
	*_handle = handle;
	return SIMPLE_THREAD_OK;
}

int SimpleThread_closeHandle(SimpleThreadHandle *handle)
{
	if(!handle)
	{
		return SIMPLE_THREAD_INVAL;
	}
	free(handle); handle = NULL;
	return 0;
}

int SimpleThread_joinThread(SimpleThreadHandle *handle)
{
	int rc = 0;
	int join = 0;
	if(!handle)
	{
		return SIMPLE_THREAD_INVAL;
	}
	join = pthread_join(handle->handle,&rc);
	if(join) {
		/* pthread_join dead lock detected */
		return join;
	} else {
		/* return the return code from the thread */
		return rc;
	}
}

/* condition variables */
int ThreadConditionVar_init(ThreadConditionVar **_condvar)
{
	int rc = SIMPLE_THREAD_INVAL;
	ThreadConditionVar *condvar = NULL;
	pthread_condattr_t attr;
	condvar = calloc(1,sizeof(ThreadConditionVar));
	if(!condvar)
	{
		return SIMPLE_THREAD_NOMEM;
	}
	pthread_condattr_init(&attr);
	/* restrict to this process */
	pthread_condattr_setpshared(&attr,PTHREAD_PROCESS_PRIVATE);
	rc = pthread_cond_init(&condvar->condition, &attr);
	if(rc==0)
	{
		*_condvar = condvar;
	}
	return rc;
}

int ThreadConditionVar_cleanup(ThreadConditionVar *condvar)
{
	int rc = SIMPLE_THREAD_OK;
	if(!condvar)
	{
		return SIMPLE_THREAD_INVAL;
	}

	if(&condvar->condition)
	{
		rc = pthread_cond_destroy(&condvar->condition);
	}

	free(condvar); condvar = NULL;
	return rc;
}

int ThreadConditionVar_wait_lock(ThreadConditionVar *condvar, ThreadLock *lock)
{
	if(!condvar||!lock)
	{
		return SIMPLE_THREAD_INVAL;
	}
	return pthread_cond_wait(&condvar->condition, &lock->lock);
}

int ThreadConditionVar_wait_lock_ms(ThreadConditionVar *condvar, ThreadLock *lock, unsigned long ms_wait)
{
	if(!condvar||!lock||ms_wait<0)
	{
		return SIMPLE_THREAD_INVAL;
	}
	struct timeval now;
	struct timespec ts;
	int rc = 0;
	
	if(!condvar||!lock)
	{
		return SIMPLE_THREAD_INVAL;
	}

	gettimeofday(&now,NULL);

	/* 
		pthread_cond_timewait uses absolute time.
		You need to calculate the current time and then calculate the time based on 
		the wait in milliseconds.
	*/
	ts.tv_sec = time(NULL) + ms_wait / 1000;
    ts.tv_nsec = now.tv_usec * 1000 + 1000 * 1000 * (ms_wait % 1000);
    ts.tv_sec += ts.tv_nsec / (1000 * 1000 * 1000);
    ts.tv_nsec %= (1000 * 1000 * 1000);

	return pthread_cond_timedwait(&condvar->condition, &lock->lock,&ts);
}

int ThreadConditionVar_signal(ThreadConditionVar *condvar)
{
	if(!condvar)
	{
		return SIMPLE_THREAD_INVAL;
	}
	return pthread_cond_signal(&condvar->condition);
}

int ThreadConditionVar_signal_all(ThreadConditionVar *condvar)
{
	if(!condvar)
	{
		return SIMPLE_THREAD_INVAL;
	}
	return pthread_cond_broadcast(&condvar->condition);
}

/* locks */
int ThreadLock_init(ThreadLock **_lock)
{
	int rc = SIMPLE_THREAD_INVAL;
	ThreadLock *lock = NULL;
	lock = calloc(1,sizeof(ThreadLock));
	if(!lock)
	{
		return SIMPLE_THREAD_NOMEM;
	}
	/* TODO: pthread_mutexattr_init - need attributes support */
	rc = pthread_mutex_init(&lock->lock,NULL);
	if(rc==0)
	{
		/* set the pointer */
		*_lock = lock;
	}
	return rc;
}
int ThreadLock_cleanup(ThreadLock *lock)
{
	int rc = SIMPLE_THREAD_OK;
	if(!lock)
	{
		return SIMPLE_THREAD_INVAL;
	}
	rc = pthread_mutex_destroy(&lock->lock);
	free(lock);
	return rc;
}
int ThreadLock_lock(ThreadLock *lock)
{
	if(!lock)
	{
		return SIMPLE_THREAD_INVAL;
	}
	return pthread_mutex_lock(&lock->lock);
}
int ThreadLock_trylock(ThreadLock *lock)
{
	if(!lock)
	{
		return SIMPLE_THREAD_INVAL;
	}
	return pthread_mutex_trylock(&lock->lock);
}
int ThreadLock_unlock(ThreadLock *lock)
{
	if(!lock)
	{
		return SIMPLE_THREAD_INVAL;
	}
	return pthread_mutex_unlock(&lock->lock);
}
