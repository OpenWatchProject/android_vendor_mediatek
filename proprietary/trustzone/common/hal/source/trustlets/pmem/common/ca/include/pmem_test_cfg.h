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

#ifndef PMEM_TEST_PARAM_H
#define PMEM_TEST_PARAM_H

#include <list>

#define PMEM_MIN_CHUNK_SIZE (SIZE_2M)
#define PMEM_MAX_CHUNK_SIZE (SIZE_16M)

struct AllocParameters {
    int size;
    int alignment;
    int flags;
};

// clang-format off
static std::list<AllocParameters> alloc_test_params = {
    {SIZE_512B, 0,          0},
    {SIZE_1K,   0,          0},
    {SIZE_2K,   0,          0},
    {SIZE_4K,   0,          0},
    {SIZE_8K,   0,          0},
    {SIZE_16K,  0,          0},
    {SIZE_32K,  0,          0},
    {SIZE_64K,  0,          0},
    {SIZE_128K, 0,          0},
    {SIZE_256K, 0,          0},
    {SIZE_512K, 0,          0},
    {SIZE_1M,   0,          0},
    {SIZE_2M,   0,          0},
    {SIZE_4M,   0,          0},
    {SIZE_8M,   0,          0},
    {SIZE_16M,  0,          0},
    {SIZE_32M,  0,          0},
    {SIZE_64M,  0,          0}
};

static std::list<AllocParameters> alloc_zero_test_params = {
    {SIZE_512B, 0,          ION_FLAG_MM_HEAP_INIT_ZERO},
    {SIZE_1K,   0,          ION_FLAG_MM_HEAP_INIT_ZERO},
    {SIZE_2K,   0,          ION_FLAG_MM_HEAP_INIT_ZERO},
    {SIZE_4K,   0,          ION_FLAG_MM_HEAP_INIT_ZERO},
    {SIZE_8K,   0,          ION_FLAG_MM_HEAP_INIT_ZERO},
    {SIZE_16K,  0,          ION_FLAG_MM_HEAP_INIT_ZERO},
    {SIZE_32K,  0,          ION_FLAG_MM_HEAP_INIT_ZERO},
    {SIZE_64K,  0,          ION_FLAG_MM_HEAP_INIT_ZERO},
    {SIZE_128K, 0,          ION_FLAG_MM_HEAP_INIT_ZERO},
    {SIZE_256K, 0,          ION_FLAG_MM_HEAP_INIT_ZERO},
    {SIZE_512K, 0,          ION_FLAG_MM_HEAP_INIT_ZERO},
    {SIZE_1M,   0,          ION_FLAG_MM_HEAP_INIT_ZERO},
    {SIZE_2M,   0,          ION_FLAG_MM_HEAP_INIT_ZERO},
    {SIZE_4M,   0,          ION_FLAG_MM_HEAP_INIT_ZERO},
    {SIZE_8M,   0,          ION_FLAG_MM_HEAP_INIT_ZERO},
    {SIZE_16M,  0,          ION_FLAG_MM_HEAP_INIT_ZERO},
    {SIZE_32M,  0,          ION_FLAG_MM_HEAP_INIT_ZERO},
    {SIZE_64M,  0,          ION_FLAG_MM_HEAP_INIT_ZERO}
};

static std::list<AllocParameters> alloc_aligned_test_params = {
    {SIZE_512B, SIZE_512B,  0},
    {SIZE_1K,   SIZE_1K,    0},
    {SIZE_2K,   SIZE_2K,    0},
    {SIZE_4K,   SIZE_4K,    0},
    {SIZE_8K,   SIZE_8K,    0},
    {SIZE_16K,  SIZE_16K,   0},
    {SIZE_32K,  SIZE_32K,   0},
    {SIZE_64K,  SIZE_64K,   0},
    {SIZE_128K, SIZE_128K,  0},
    {SIZE_256K, SIZE_256K,  0},
    {SIZE_512K, SIZE_512K,  0},
    {SIZE_1M,   SIZE_1M,    0},
    {SIZE_2M,   SIZE_2M,    0},
    {SIZE_4M,   SIZE_4M,    0},
    {SIZE_8M,   SIZE_8M,    0},
    {SIZE_16M,  SIZE_16M,   0},
    {SIZE_32M,  SIZE_32M,   0},
    {SIZE_64M,  SIZE_64M,   0}
};

static std::list<AllocParameters> alloc_zero_aligned_test_params = {
    {SIZE_512B, SIZE_512B,  ION_FLAG_MM_HEAP_INIT_ZERO},
    {SIZE_1K,   SIZE_1K,    ION_FLAG_MM_HEAP_INIT_ZERO},
    {SIZE_2K,   SIZE_2K,    ION_FLAG_MM_HEAP_INIT_ZERO},
    {SIZE_4K,   SIZE_4K,    ION_FLAG_MM_HEAP_INIT_ZERO},
    {SIZE_8K,   SIZE_8K,    ION_FLAG_MM_HEAP_INIT_ZERO},
    {SIZE_16K,  SIZE_16K,   ION_FLAG_MM_HEAP_INIT_ZERO},
    {SIZE_32K,  SIZE_32K,   ION_FLAG_MM_HEAP_INIT_ZERO},
    {SIZE_64K,  SIZE_64K,   ION_FLAG_MM_HEAP_INIT_ZERO},
    {SIZE_128K, SIZE_128K,  ION_FLAG_MM_HEAP_INIT_ZERO},
    {SIZE_256K, SIZE_256K,  ION_FLAG_MM_HEAP_INIT_ZERO},
    {SIZE_512K, SIZE_512K,  ION_FLAG_MM_HEAP_INIT_ZERO},
    {SIZE_1M,   SIZE_1M,    ION_FLAG_MM_HEAP_INIT_ZERO},
    {SIZE_2M,   SIZE_2M,    ION_FLAG_MM_HEAP_INIT_ZERO},
    {SIZE_4M,   SIZE_4M,    ION_FLAG_MM_HEAP_INIT_ZERO},
    {SIZE_8M,   SIZE_8M,    ION_FLAG_MM_HEAP_INIT_ZERO},
    {SIZE_16M,  SIZE_16M,   ION_FLAG_MM_HEAP_INIT_ZERO},
    {SIZE_32M,  SIZE_32M,   ION_FLAG_MM_HEAP_INIT_ZERO},
    {SIZE_64M,  SIZE_64M,   ION_FLAG_MM_HEAP_INIT_ZERO}
};
// clang-format on

#endif /* end of PMEM_TEST_PARAM_H */
