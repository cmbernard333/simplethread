#ifndef _win32_simple_thread_internal_h_
#define _win32_simple_thread_internal_h_

#include <windows.h>
#include <stdlib.h>
#include <stdio.h>
#include <ws2_32.h>
#define WINDOWS_LEAN_AND_MEAN

#include "simple_thread_internal.h"

struct simple_thread_handle_t
{
	HANDLE handle;
	SimpleThreadTask *task;
	void *data;
};

struct thread_condition_var_t
{
	CONDITION_VARIABLE condition;
};

struct thread_lock_t
{
	CRITICAL_SECTION lock;
};

/* executes generic tasks */
DWORD WINAPI bus_worker_thread(void*);

#endif /* _win32_simple_thread_internal_h_ */