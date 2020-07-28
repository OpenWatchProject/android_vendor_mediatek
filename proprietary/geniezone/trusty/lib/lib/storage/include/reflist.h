/*
 * Copyright (c) 2013, Google, Inc. All rights reserved
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files
 * (the "Software"), to deal in the Software without restriction,
 * including without limitation the rights to use, copy, modify, merge,
 * publish, distribute, sublicense, and/or sell copies of the Software,
 * and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#ifndef __REFLIST_H
#define __REFLIST_H

#include <assert.h>
#include <compiler.h>
#include <list.h>

typedef struct obj_ref {
	struct list_node ref_node;
} obj_ref_t;

typedef struct obj {
	struct list_node ref_list;
} obj_t;

typedef void (*obj_destroy_func)(obj_t *obj);

#define OBJ_REF_INITIAL_VALUE(r) \
{ \
	.ref_node = LIST_INITIAL_CLEARED_VALUE, \
}

static inline __ALWAYS_INLINE
void obj_ref_init(obj_ref_t *ref)
{
	*ref = (obj_ref_t)OBJ_REF_INITIAL_VALUE(*ref);
}

static inline __ALWAYS_INLINE
void obj_init(obj_t *obj, obj_ref_t *ref)
{
	list_initialize(&obj->ref_list);
	list_add_tail(&obj->ref_list, &ref->ref_node);
}

static inline __ALWAYS_INLINE
void obj_add_ref(obj_t *obj, obj_ref_t *ref)
{
	assert(!list_in_list(&ref->ref_node));
	list_add_tail(&obj->ref_list, &ref->ref_node);
}

static inline __ALWAYS_INLINE
bool obj_del_ref(obj_t *obj, obj_ref_t *ref, obj_destroy_func destroy)
{
	bool dead;

	assert(list_in_list(&ref->ref_node));
	assert(destroy);

	list_delete(&ref->ref_node);
	dead = list_is_empty(&obj->ref_list);
	if (dead)
		destroy(obj);
	return dead;
}

#endif
