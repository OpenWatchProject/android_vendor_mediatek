/*
 * Copyright (C) 2011-2014 MediaTek Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef __GE_H__
#define __GE_H__

#include <ui/gralloc_extra.h>
#include <ged/ged_ge.h>

__BEGIN_DECLS

/* Alloc ge_fd, imply ge_retain() */
GEFD ge_alloc(buffer_handle_t hnd);

/* buffer_handle_t is copied, init GE backend */
int ge_retain(buffer_handle_t hnd);

/* buffer_handle_t is going to be closed, deinit GE backend */
int ge_release(buffer_handle_t hnd);

/* A helper function to close ge_fd.
 * If you are buffer owner (allocater) must close ge FD by himself
 * if the implementation does not call native_handle_close().
 */
int ge_free(buffer_handle_t hnd);

__END_DECLS

#endif
