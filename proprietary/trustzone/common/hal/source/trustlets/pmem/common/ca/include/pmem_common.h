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

#ifndef PMEM_COMMON_H
#define PMEM_COMMON_H

// clang-format off
#define SIZE_512B     0x00000200    /* 512B  */
#define SIZE_1K       0x00000400    /* 1K    */
#define SIZE_2K       0x00000800    /* 2K    */
#define SIZE_4K       0x00001000    /* 4K    */
#define SIZE_8K       0x00002000    /* 8K    */
#define SIZE_16K      0x00004000    /* 16K   */
#define SIZE_32K      0x00008000    /* 32K   */
#define SIZE_64K      0x00010000    /* 64K   */
#define SIZE_128K     0x00020000    /* 128K  */
#define SIZE_256K     0x00040000    /* 256K  */
#define SIZE_512K     0x00080000    /* 512K  */
#define SIZE_1M       0x00100000    /* 1M    */
#define SIZE_2M       0x00200000    /* 2M    */
#define SIZE_4M       0x00400000    /* 4M    */
#define SIZE_8M       0x00800000    /* 8M    */
#define SIZE_16M      0x01000000    /* 16M   */
#define SIZE_32M      0x02000000    /* 32M   */
#define SIZE_64M      0x04000000    /* 64M   */
#define SIZE_128M     0x08000000    /* 128M  */
#define SIZE_256M     0x10000000    /* 256M  */
#define SIZE_320M     0x14000000    /* 320M  */
#define SIZE_1G       0x40000000    /* 1G    */
#define SIZE_2G       0x80000000    /* 2G    */
#define SIZE_U32_MAX  0xFFFFFFFF    /* 4G-1  */

#define PMEM_MAX_SIZE SIZE_128M     /* 128M  */
// clang-format on

#endif /* end of PMEM_COMMON_H */
