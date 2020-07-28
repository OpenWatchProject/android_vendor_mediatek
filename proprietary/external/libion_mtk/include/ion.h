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

#ifndef __MTK_ION_H
#define __MTK_ION_H

#include <linux/ion.h>

__BEGIN_DECLS

// Allocate buffer from multimedia heap.
int ion_alloc_mm(int fd, size_t len, size_t align,
                 unsigned int flags, ion_user_handle_t *handle);
// Allocate buffer from camera heap.
int ion_alloc_camera(int fd, size_t len, size_t align,
                     unsigned int flags, ion_user_handle_t *handle);
// Allocate buffer from system contiguous heap.
int ion_alloc_syscontig(int fd, size_t len, size_t align,
                        unsigned int flags, ion_user_handle_t *handle);
int ion_alloc_camera_pool(int fd, size_t len, size_t align, unsigned int flags,
              unsigned int *ret, int cache_pool_cmd);
// Map virtual address of ION buffer
void* ion_mmap(int fd, void *addr, size_t length, int prot, int flags, int share_fd, off_t offset);
// UnMap virtual address of ION buffer
int ion_munmap(int fd, void *addr, size_t length);
// Close share fd of ION buffer.
int ion_share_close(int fd, int share_fd);
// ION custom IOCTL
int ion_custom_ioctl(int fd, unsigned int cmd, void *arg);
int mt_ion_open(const char *name);
int mt_ion_close(int fd);

int ion_cache_sync_flush_all(int fd);
int ion_cache_sync_flush_range(int fd);
int ion_cache_sync_flush_range_va(int fd, void *addr, size_t length);
int ion_dma_map_area(int fd, ion_user_handle_t handle, int dir);
int ion_dma_unmap_area(int fd, ion_user_handle_t handle, int dir);
int ion_dma_map_area_va(int fd, void *addr, size_t length, int dir);
int ion_dma_unmap_area_va(int fd, void *addr, size_t length, int dir);

__END_DECLS

#endif /* __MTK_ION_H */
