/*
 * Copyright (c) 2018 MediaTek Inc.
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

#ifndef DRFOD_API_H
#define DRFOD_API_H

int fod_tdrv_setup_shm_pa(uint64_t shm_pa, uint32_t shm_size);
int fod_tdrv_mmap_shm(uintptr_t tapp_mem_va, uint32_t tapp_mem_size, uint32_t shm_offset, int* drv_ret);
int fod_tdrv_unmap_shm(uintptr_t tapp_mem_va, uint32_t tapp_mem_size, uint32_t shm_offset, int* drv_ret);
int fod_tdrv_copy_2_shm(uintptr_t tapp_mem_va, uint32_t tapp_mem_size, uint32_t shm_offset, int* drv_ret);
int fod_tdrv_copy_from_shm(uintptr_t tapp_mem_va, uint32_t tapp_mem_size, uint32_t shm_offset, int* drv_ret);
int fod_tdrv_setup_shm_cmd(uint32_t *buf_offset, uint32_t *buf_num, uint32_t *prop_offset, uint32_t *prop_size, uint32_t core, uint32_t cmd);
int fod_tdrv_mmap1_StaticGzMteeShm(uint64_t pa, uint32_t sz);
int fod_tdrv_mmap2_StaticGzMteeShm(uint64_t pa, uint32_t sz);
int fod_tdrv_test_StaticGzMteeShm(uint64_t pa, uint32_t sz);
int fod_tdrv_umap_StaticGzMteeShm(uint64_t pa, uint32_t sz);

#endif /* end of DRFOD_API_H */
