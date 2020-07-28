/*	$OpenBSD: atexit.c,v 1.14 2007/09/05 20:47:47 chl Exp $ */
/*
 * Copyright (c) 2002 Daniel Hartmeier
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 *    - Redistributions of source code must retain the above copyright
 *      notice, this list of conditions and the following disclaimer.
 *    - Redistributions in binary form must reproduce the above
 *      copyright notice, this list of conditions and the following
 *      disclaimer in the documentation and/or other materials provided
 *      with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT HOLDERS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 */

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <trusty_std.h>

#include "atexit.h"

int __atexit_invalid = 1;
struct atexit *__atexit;

/*
 * Function pointers are stored in a linked list of pages. The list
 * is initially empty, and pages are allocated on demand.
 */

/*
 * Register a function to be performed at exit.
 * For more info on this API, see:
 *
 *	http://www.codesourcery.com/cxx-abi/abi.html#dso-dtor
 */
int
__cxa_atexit(void (*func)(void *), void *arg)
{
	struct atexit *p = __atexit;
	struct atexit_fn *fnp;
	int size = 64;
	int ret = -1;

	if (p != NULL) {
		if (p->ind + 1 >= p->max)
			p = NULL;
	}
	if (p == NULL) {
		p = malloc(size);
		if (!p)
			goto done;
		if (__atexit == NULL) {
			memset(&p->fns[0], 0, sizeof(p->fns[0]));
			p->ind = 1;
		} else
			p->ind = 0;
		p->max = (size - ((char *)&p->fns[0] - (char *)p)) /
		    sizeof(p->fns[0]);
		p->next = __atexit;
		__atexit = p;
		if (__atexit_invalid)
			__atexit_invalid = 0;
	}
	fnp = &p->fns[p->ind++];
	fnp->fn_ptr.cxa_func = func;
	fnp->fn_arg = arg;
	ret = 0;
done:
	return (ret);
}

/*
 * Call all handlers registered with __cxa_atexit().
 */
void
__cxa_finalize()
{
	struct atexit *p, *q;
	struct atexit_fn fn;
	int n;
	static int call_depth;

	if (__atexit_invalid)
		return;

	call_depth++;

	for (p = __atexit; p != NULL; p = p->next) {
		for (n = p->ind; --n >= 0;) {
			if (p->fns[n].fn_ptr.cxa_func == NULL)
				continue;	/* already called */

			/*
			 * Mark handler as having been already called to avoid
			 * dupes and loops, then call the appropriate function.
			 */
			fn = p->fns[n];
			p->fns[n].fn_ptr.cxa_func = NULL;
                        (*fn.fn_ptr.cxa_func)(fn.fn_arg);
		}
	}

	/*
	 * If called via exit(), unmap the pages since we have now run
	 * all the handlers.  We defer this until calldepth == 0 so that
	 * we don't unmap things prematurely if called recursively.
	 */
	if (--call_depth == 0) {
		for (p = __atexit; p != NULL; ) {
			q = p;
			p = p->next;
			free(q);
		}
		__atexit = NULL;
	}
}

void __cxa_pure_virtual(void)
{
	fprintf(stderr, "Pure virtual function called.\n");
	exit(1);
}

int __attribute__((weak))
__aeabi_atexit(void *object, void (*destructor) (void *)) {
	return __cxa_atexit(destructor, object);
}
