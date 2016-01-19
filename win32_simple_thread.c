#include "win32_simple_thread_internal.h"

DWORD WINAPI simple_thread_worker(void *data)
{
	int rc = SIMPLE_THREAD_OK;
	SimpleThreadTaskWorker *simple_thread_worker = (SimpleThreadTaskWorker*)data;
	if(!simple_thread_worker) {
		rc = -1;
	} else {
		rc = (*simple_thread_worker->worker)(simple_thread_worker->data);
		free(simple_thread_worker);
	}
	return rc;
}

/* TODO:
		Stack size considerations
		Handle inheritance
		Security Attributes
		Detached
		Joinable
		Suspendable
		SECURITY_ATTRIBUTES attr;
		attr.nLength = sizeof(SECURITY_ATTRIBUTES);
		attr.bInheritHandle = FALSE;
*/
int SimpleThread_createThread(SimpleThreadHandle **_handle, SimpleThreadTask task, void *data)
{
	int rc = 0;
	unsigned long id = 0;
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
	
	/* 5th argument of  0x0000 0004 starts thread in suspended state */
	handle->handle = CreateThread(NULL, 0 , simple_thread_worker, worker, 0, &id);
	*_handle = handle;
	return rc;
}

int SimpleThread_closeHandle(SimpleThreadHandle *handle)
{
	if(!handle)
	{
		return SIMPLE_THREAD_INVAL;
	}
	CloseHandle(handle->handle);
	free(handle);
	return 0;
}
int SimpleThread_joinThread(SimpleThreadHandle *handle)
{
	return WaitForSingleObject(handle->handle,INFINITE);
}

/* condition variables */
int ThreadConditionVar_init(ThreadConditionVar **_condvar)
{
	int rc = SIMPLE_THREAD_OK;
	ThreadConditionVar *condvar = NULL;
	condvar = calloc(1,sizeof(ThreadConditionVar));
	if(!condvar)
	{
		return SIMPLE_THREAD_NOMEM;
	}
	InitializeConditionVariable(&condvar->condition);
	*_condvar = condvar;
	return rc;
}

int ThreadConditionVar_cleanup(ThreadConditionVar *var)
{
	/* NOTE:
		Windows provides no delete function for CONDITION
	*/
	free(var); var = NULL;
	return SIMPLE_THREAD_OK;
}

int ThreadConditionVar_wait_lock(ThreadConditionVar *var, ThreadLock *lock)
{
	if(!var||!lock)
	{
		return SIMPLE_THREAD_INVAL;
	}
	int rc = NIME_ERROR;
	/*
		SleepConditionVariableCS returns the following:
		Non-zero for succeeding
		Zero for not succeeding
		To get extended error information call GetLastError
	*/
	rc = SleepConditionVariableCS(&var->condition, &lock->lock, INFINITE);
	/* SleepConditionVariableCS returns non-zero if it succeeds */
	if( rc > 0 )
	{
		/* success !*/
		rc = SIMPLE_THREAD_OK;
	}
	return rc;
}

int ThreadConditionVar_wait_lock_ms(ThreadConditionVar *var, ThreadLock *lock, unsigned long ms_wait)
{
	if(!var||!lock||ms_wait<0)
	{
		return SIMPLE_THREAD_INVAL;
	}
	int rc = NIME_ERROR;
	/*
		SleepConditionVariableCS returns the following:
		Non-zero for succeeding
		Zero for not succeeding
		To get extended error information call GetLastError
	*/
	rc = SleepConditionVariableCS(&var->condition, &lock->lock, ms_wait);
	/* SleepConditionVariableCS returns non-zero if it succeeds */
	if( rc > 0 )
	{
		/* success !*/
		rc = SIMPLE_THREAD_OK;
	} else {
		rc = GetLastError();
		if(rc==ERROR_TIMEOUT)
		{
			rc = THREAD_CONDITION_TIMEOUT;
		}
	}
	return rc;
}

int ThreadConditionVar_signal(ThreadConditionVar *condvar)
{
	if(!condvar)
	{
		return SIMPLE_THREAD_INVAL;
	}
	WakeConditionVariable(&var->condition);
	return 0;
}

int ThreadConditionVar_signal_all(ThreadConditionVar *condvar)
{
	if(!condvar)
	{
		return SIMPLE_THREAD_INVAL;
	}
	WakeAllConditionVariable(&var->condition);
	return 0;
}

/* locks */
int ThreadLock_init(ThreadLock **_lock)
{
	int rc = SIMPLE_THREAD_OK;
	ThreadLock *lock = NULL;
	lock = calloc(1,sizeof(ThreadLock));
	if(!lock)
	{
		return SIMPLE_THREAD_NOMEM;
	}
	InitializeCriticalSection(&lock->lock);
	*_lock = lock;
	return rc;
}
int ThreadLock_cleanup(ThreadLock *lock)
{
	int rc = SIMPLE_THREAD_OK;
	if(!lock)
	{
		return SIMPLE_THREAD_INVAL;
	}
	DeleteCriticalSection(&lock->lock);
	free(lock);
	return rc;
}
int ThreadLock_lock(ThreadLock *lock)
{
	if(!lock)
	{
		return SIMPLE_THREAD_INVAL;
	}
	EnterCriticalSection(&lock->lock);
	return 0;
}
int ThreadLock_trylock(ThreadLock *lock)
{
	if(!lock)
	{
		return SIMPLE_THREAD_INVAL;
	}
	/* 
		return value of TryEnterCriticalSection is 0 if the lock can't be acquired -
		for consistency sake it uses the inverse result
	*/
	return !TryEnterCriticalSection(&lock->lock);
}
int ThreadLock_unlock(ThreadLock *lock)
{
	if(!lock)
	{
		return SIMPLE_THREAD_INVAL;
	}
	LeaveCriticalSection(&lock->lock);
	return 0;
}