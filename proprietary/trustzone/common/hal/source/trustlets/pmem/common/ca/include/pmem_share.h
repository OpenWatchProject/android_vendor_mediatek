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

#ifndef PMEM_SHARED_H
#define PMEM_SHARED_H

#define TEE_MEM_WRITE_PATTERN (0xA2458082)
#define GZ_MEM_WRITE_PATTERN (0x14BA32A4)
#define INVALID_MEM_WRITE_PATTERN (0xABCDEFAA)

/* TEE related APIs */
int tee_open();
int tee_close();
int tee_write(int mem_handle, int mem_size, unsigned int pattern,
              bool check_zero);
int tee_read(int mem_handle, int mem_size, unsigned int pattern);
#ifdef TEE_MEMORY_MAP_TEST_ENABLE
int tee_mem_map(int mem_handle, int total_size, int map_chunk_size,
                int try_map_total);
#endif

/* GZ related APIs */
int gz_open();
int gz_close();
int gz_write(int mem_handle, int mem_size, unsigned int pattern,
             bool check_zero);
int gz_read(int mem_handle, int mem_size, unsigned int pattern);

#endif /* end of PMEM_SHARED_H */
