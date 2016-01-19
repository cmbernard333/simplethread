#ifndef _simple_thread_internal_h_
#define _simple_thread_internal_h_

#include "simple_thread.h"

/* 
	internal api for thread functions.
	since threading cross-platform is platform specific 
	the API provided here should NOT be used external to this library.
*/

typedef struct simple_thread_task_worker_t
{
	SimpleThreadTask worker; /* generic function that returns an int */
	void *data;
}SimpleThreadTaskWorker;


#endif /* _simple_thread_internal_h_ */