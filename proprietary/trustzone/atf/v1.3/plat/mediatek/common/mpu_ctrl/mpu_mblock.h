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

#ifndef __MPU_MBLOCK_H__
#define __MPU_MBLOCK_H__

/*
 * mblock header referencing PL version,
 * make sure the structure definition is
 * consistent with the PL one
 */

#define MPU_MBLOCK_RESERVED_NAME_SIZE  128
#define MPU_MBLOCK_RESERVED_NUM_MAX    128
#define MPU_MBLOCK_NUM_MAX             128
#define MPU_MBLOCK_MAGIC               0x99999999
#define MPU_MBLOCK_VERSION             0x2

struct mem_desc {
	uint64_t start;
	uint64_t size;
};

struct mblock {
	uint64_t start;
	uint64_t size;
	uint32_t rank;
};

struct reserved {
	uint64_t start;
	uint64_t size;
	uint32_t mapping;
	char name[MPU_MBLOCK_RESERVED_NAME_SIZE];
};

struct mblock_info {
	uint32_t mblock_num;
	struct mblock mblock[MPU_MBLOCK_NUM_MAX];
	uint32_t mblock_magic;
	uint32_t mblock_version;
	uint32_t reserved_num;
	struct reserved reserved_mem[MPU_MBLOCK_RESERVED_NUM_MAX];
};

struct dram_info {
	uint32_t rank_num;
	struct mem_desc rank_info[4];
};

#endif
