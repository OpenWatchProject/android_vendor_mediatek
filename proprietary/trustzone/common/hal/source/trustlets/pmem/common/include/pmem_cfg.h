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

#ifndef PMEM_CFG_H
#define PMEM_CFG_H

/* clang-format off */
enum PMEM_CONFIG_FLAGS {
    PMEM_CONFIG_MOCK_NONE = 1 << 0,
    PMEM_CONFIG_MOCK_GZ = 1 << 1,
    PMEM_CONFIG_TEE_MAP_NONE_SECURE = 1 << 2,
};
/* clang-format on */

#define IS_GZ_MOCKED(flags) (flags & PMEM_CONFIG_MOCK_GZ)
#define IS_NON_SECURE_MAP(flags) (flags & PMEM_CONFIG_TEE_MAP_NONE_SECURE)

#endif /* end of PMEM_CFG_H */
