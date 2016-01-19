#ifndef _posix_simple_thread_internal_h_
#define _posix_simple_thread_internal_h_

#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include "simple_thread_internal.h"

struct simple_thread_handle_t
{
	pthread_t handle;
	SimpleThreadTask task;
	void *data;
};

struct thread_condition_var_t
{
	pthread_cond_t condition;
};

struct thread_lock_t
{
	pthread_mutex_t lock;
};

/* executes generic tasks */
void *bus_worker_thread(void*);

#endif /* _posix_simple_thread_internal_h_ */