/*
 *  ion.h
 *
 * Memory Allocator functions for ion
 *
 *   MediaTek Inc. (C) 2011. All rights reserved.
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */

#ifndef __ION_ULIT_H
#define __ION_ULIT_H

#ifdef __cplusplus
extern "C"  {
#endif

#define _MTK_ION_DUMP_CS
void dumping_callstack(const char *func_name, int ion_fd, int share_fd);

#ifdef __cplusplus
}
#endif


#endif /* __MTK_ION_H */
