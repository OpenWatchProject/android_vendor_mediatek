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

#include <gtest/gtest.h>

#include <ion.h>
#include <ion/ion.h>
#include <linux/ion.h>
#include <linux/ion_drv.h>
#ifdef ION_NUM_HEAP_IDS
#undef ION_NUM_HEAP_IDS
#endif
#include <linux/mtk_ion.h>

#include "pmem_common.h"
#include "pmem_dbg.h"
#include "pmem_share.h"
#include "pmem_test_cfg.h"

#define ION_MIN_CHUNK_SIZE (SIZE_4K)
#define PMEM_ION_MASK ION_HEAP_MULTIMEDIA_TYPE_PROT_MASK

static int get_protected_handle(int ion_fd, ion_user_handle_t ion_handle,
                                int* mem_handle)
{
    struct ion_sys_data sys_data;
    sys_data.sys_cmd = ION_SYS_GET_PHYS;
    sys_data.get_phys_param.handle = ion_handle;
    if (ion_custom_ioctl(ion_fd, ION_CMD_SYSTEM, &sys_data)) {
        return PMEM_FAIL;
    }

    *mem_handle = sys_data.get_phys_param.phy_addr;
    return PMEM_SUCCESS;
}

class PmemShareSingleTest : public ::testing::Test
{
  protected:
    void SetUp()
    {
    }
    void TearDown()
    {
    }
};

class PmemShareBaseTest : public ::testing::Test
{
  public:
    static void SetUpTestCase()
    {
    }
    static void TearDownTestCase()
    {
    }
};

class PmemShareAllocTest : public PmemShareBaseTest,
                           public ::testing::WithParamInterface<AllocParameters>
{
};

class PmemDevOpenTest : public ::testing::Test
{
  public:
    static void SetUpTestCase()
    {
        ASSERT_EQ(0, tee_open());
        ASSERT_EQ(0, gz_open());
    }
    static void TearDownTestCase()
    {
        ASSERT_EQ(0, tee_close());
        ASSERT_EQ(0, gz_close());
    }
};

class PmemShareTeeWriteTest
    : public PmemDevOpenTest,
      public ::testing::WithParamInterface<AllocParameters>
{
};

class PmemShareGzWriteTest
    : public PmemDevOpenTest,
      public ::testing::WithParamInterface<AllocParameters>
{
};

class PmemShareReadWriteTest
    : public PmemDevOpenTest,
      public ::testing::WithParamInterface<AllocParameters>
{
};

/****************************************************************************
 ** ION device open/close test
 ****************************************************************************/
TEST_F(PmemShareSingleTest, ion_device_open_Success)
{
    int ion_fd;

    ASSERT_TRUE((ion_fd = ion_open()) >= 0);
    ASSERT_EQ(0, ion_close(ion_fd));
};

/****************************************************************************
 ** TEE device open/close test
 ****************************************************************************/
TEST_F(PmemShareSingleTest, tee_device_open_Success)
{
    ASSERT_EQ(0, tee_open());
    ASSERT_EQ(0, tee_close());
};

/****************************************************************************
 ** GZ device open/close test
 ****************************************************************************/
TEST_F(PmemShareSingleTest, gz_device_open_Success)
{
    ASSERT_EQ(0, gz_open());
    ASSERT_EQ(0, gz_close());
};

#ifdef TEE_MEMORY_MAP_TEST_ENABLE
/****************************************************************************
 ** TEE device max map size test
 ****************************************************************************/
#define PMEM_64BIT_PHYS_SHIFT (10)
#define HANDLE_TO_PA(handle)                       \
    (((uint64_t)handle << PMEM_64BIT_PHYS_SHIFT) & \
     ~((1 << PMEM_64BIT_PHYS_SHIFT) - 1))
#define CHUNK_TOTOL_SIZE (SIZE_256M)
#define CHUNK_SIZE (SIZE_16M)
#define CHUNK_COUNT (CHUNK_TOTOL_SIZE / CHUNK_SIZE)
TEST_F(PmemShareSingleTest, tee_device_map_Success)
{
    int max_size = CHUNK_TOTOL_SIZE;
    int map_size;
    int ion_fd;
    ion_user_handle_t handle[CHUNK_COUNT];
    int mem_handle[CHUNK_COUNT];
    int idx;
    uint64_t found_min_pa_addr = 0x10000000000;
    uint64_t pa_addr;
    int found_min_pa_handle;
    int ret;
    int try_map_total_sz;

    ASSERT_EQ(0, tee_open());
    ASSERT_TRUE((ion_fd = ion_open()) >= 0);

    for (idx = 0; idx < CHUNK_COUNT; idx++) {
        ASSERT_EQ(0, ion_alloc(ion_fd, CHUNK_SIZE, 0, PMEM_ION_MASK, 0,
                               &handle[idx]));
        ASSERT_NE(0, handle[idx]);
        ASSERT_EQ(0,
                  get_protected_handle(ion_fd, handle[idx], &mem_handle[idx]));
        pa_addr = HANDLE_TO_PA(mem_handle[idx]);
        if (found_min_pa_addr > pa_addr) {
            found_min_pa_addr = pa_addr;
            found_min_pa_handle = mem_handle[idx];
        }
        ASSERT_NE(0, mem_handle[idx]);
    }

    for (map_size = SIZE_2M; map_size <= max_size; map_size *= 2) {
        for (try_map_total_sz = map_size; try_map_total_sz <= max_size;
             try_map_total_sz *= 2) {
            ret = tee_mem_map(found_min_pa_handle, max_size, map_size,
                              try_map_total_sz);
            if (try_map_total_sz >= SIZE_128M) {
                ASSERT_NE(0, ret);
            } else {
                ASSERT_EQ(0, ret);
            }
        }
    }

    for (idx = (CHUNK_COUNT - 1); idx >= 0; idx--) {
        ASSERT_EQ(0, ion_free(ion_fd, handle[idx]));
    }

    ASSERT_EQ(0, ion_close(ion_fd));
    ASSERT_EQ(0, tee_close());
};
#endif

/****************************************************************************
 ** Protect memory allocate/free basic test
 ****************************************************************************/
TEST_P(PmemShareAllocTest, ion_alloc_Success)
{
    int ion_fd;
    ion_user_handle_t handle;
    size_t alignment = GetParam().alignment;

    /* the limitation is set only when using pmem mock interface */
    if (GetParam().size > PMEM_MAX_CHUNK_SIZE) {
        return;
    }

    /* ion alignment is aligned to ion page size */
    if ((alignment != 0) && (GetParam().size < ION_MIN_CHUNK_SIZE)) {
        alignment = ION_MIN_CHUNK_SIZE;
    }

    ASSERT_TRUE((ion_fd = ion_open()) >= 0);
    ASSERT_EQ(0, ion_alloc(ion_fd, GetParam().size, alignment, PMEM_ION_MASK,
                           GetParam().flags, &handle));
    ASSERT_NE(0, handle);
    ASSERT_EQ(0, ion_free(ion_fd, handle));
    ASSERT_EQ(0, ion_close(ion_fd));
}

INSTANTIATE_TEST_CASE_P(ION_ALLOC, PmemShareAllocTest,
                        ::testing::ValuesIn(alloc_test_params));

INSTANTIATE_TEST_CASE_P(ION_ALLOC_ZERO, PmemShareAllocTest,
                        ::testing::ValuesIn(alloc_zero_test_params));

/****************************************************************************
 ** Protect memory allocate/free TEE-write, GZ-read basic test
 ** - write by TEE
 ** - read and check by GZ
 ****************************************************************************/
TEST_P(PmemShareTeeWriteTest, tee_w_gz_r_Success)
{
    int ion_fd;
    ion_user_handle_t handle;
    size_t alignment = GetParam().alignment;
    int ret;
    bool check_zero = GetParam().flags & ION_FLAG_MM_HEAP_INIT_ZERO;
    int mem_handle;

    /* the limitation is set only when using pmem mock interface */
    if (GetParam().size > PMEM_MAX_CHUNK_SIZE) {
        return;
    }

    /* ion alignment is aligned to ion page size */
    if ((alignment != 0) && (GetParam().size < ION_MIN_CHUNK_SIZE)) {
        alignment = ION_MIN_CHUNK_SIZE;
    }

    ASSERT_TRUE((ion_fd = ion_open()) >= 0);
    ASSERT_EQ(0, ion_alloc(ion_fd, GetParam().size, alignment, PMEM_ION_MASK,
                           GetParam().flags, &handle));
    ASSERT_NE(0, handle);
    ASSERT_EQ(0, get_protected_handle(ion_fd, handle, &mem_handle));
    ASSERT_NE(0, mem_handle);

    /* pass handle to TEE for write */
    ret = tee_write(mem_handle, GetParam().size, TEE_MEM_WRITE_PATTERN,
                    check_zero);
    ASSERT_EQ(0, ret);

    /* pass handle to GZ for read */
    ret = gz_read(mem_handle, GetParam().size, TEE_MEM_WRITE_PATTERN);
    ASSERT_EQ(0, ret);
    ret = gz_read(mem_handle, GetParam().size, INVALID_MEM_WRITE_PATTERN);
    ASSERT_NE(0, ret);

    ASSERT_EQ(0, ion_free(ion_fd, handle));
    ASSERT_EQ(0, ion_close(ion_fd));
}

INSTANTIATE_TEST_CASE_P(ION_ALLOC, PmemShareTeeWriteTest,
                        ::testing::ValuesIn(alloc_test_params));

INSTANTIATE_TEST_CASE_P(ION_ALLOC_ZERO, PmemShareTeeWriteTest,
                        ::testing::ValuesIn(alloc_zero_test_params));

/****************************************************************************
 ** Protect memory allocate/free GZ-write, TEE-read basic test
 ** - write by GZ
 ** - read and check by TEE
 ****************************************************************************/
TEST_P(PmemShareGzWriteTest, gz_w_tee_r_Success)
{
    int ion_fd;
    ion_user_handle_t handle;
    size_t alignment = GetParam().alignment;
    int ret;
    bool check_zero = GetParam().flags & ION_FLAG_MM_HEAP_INIT_ZERO;
    int mem_handle;

    /* the limitation is set only when using pmem mock interface */
    if (GetParam().size > PMEM_MAX_CHUNK_SIZE) {
        return;
    }

    /* ion alignment is aligned to ion page size */
    if ((alignment != 0) && (GetParam().size < ION_MIN_CHUNK_SIZE)) {
        alignment = ION_MIN_CHUNK_SIZE;
    }

    ASSERT_TRUE((ion_fd = ion_open()) >= 0);
    ASSERT_EQ(0, ion_alloc(ion_fd, GetParam().size, alignment, PMEM_ION_MASK,
                           GetParam().flags, &handle));
    ASSERT_NE(0, handle);
    ASSERT_EQ(0, get_protected_handle(ion_fd, handle, &mem_handle));
    ASSERT_NE(0, mem_handle);

    /* pass handle to GZ for write */
    ret =
        gz_write(mem_handle, GetParam().size, GZ_MEM_WRITE_PATTERN, check_zero);
    ASSERT_EQ(0, ret);

    /* pass handle to TEE for read */
    ret = tee_read(mem_handle, GetParam().size, GZ_MEM_WRITE_PATTERN);
    ASSERT_EQ(0, ret);
    ret = tee_read(mem_handle, GetParam().size, INVALID_MEM_WRITE_PATTERN);
    ASSERT_NE(0, ret);

    ASSERT_EQ(0, ion_free(ion_fd, handle));
    ASSERT_EQ(0, ion_close(ion_fd));
}

INSTANTIATE_TEST_CASE_P(ION_ALLOC, PmemShareGzWriteTest,
                        ::testing::ValuesIn(alloc_test_params));

INSTANTIATE_TEST_CASE_P(ION_ALLOC_ZERO, PmemShareGzWriteTest,
                        ::testing::ValuesIn(alloc_zero_test_params));

/****************************************************************************
 ** Protect memory allocate/free GZ-rw, TEE-rw basic test
 ** - read/write by GZ
 ** - read/write by TEE
 ** - clean by GZ
 ** - zero check by TEE
 ****************************************************************************/
TEST_P(PmemShareReadWriteTest, gz_rw_tee_rw_Success)
{
    int ion_fd;
    ion_user_handle_t handle;
    size_t alignment = GetParam().alignment;
    int ret;
    bool check_zero = GetParam().flags & ION_FLAG_MM_HEAP_INIT_ZERO;
    int mem_handle;

    /* the limitation is set only when using pmem mock interface */
    if (GetParam().size > PMEM_MAX_CHUNK_SIZE) {
        return;
    }

    /* ion alignment is aligned to ion page size */
    if ((alignment != 0) && (GetParam().size < ION_MIN_CHUNK_SIZE)) {
        alignment = ION_MIN_CHUNK_SIZE;
    }

    ASSERT_TRUE((ion_fd = ion_open()) >= 0);
    ASSERT_EQ(0, ion_alloc(ion_fd, GetParam().size, alignment, PMEM_ION_MASK,
                           GetParam().flags, &handle));
    ASSERT_NE(0, handle);
    ASSERT_EQ(0, get_protected_handle(ion_fd, handle, &mem_handle));
    ASSERT_NE(0, mem_handle);

    /* pass handle to GZ for write */
    ret =
        gz_write(mem_handle, GetParam().size, GZ_MEM_WRITE_PATTERN, check_zero);
    ASSERT_EQ(0, ret);

    /* pass handle to TEE for read */
    ret = tee_read(mem_handle, GetParam().size, GZ_MEM_WRITE_PATTERN);
    ASSERT_EQ(0, ret);
    ret = tee_read(mem_handle, GetParam().size, INVALID_MEM_WRITE_PATTERN);
    ASSERT_NE(0, ret);

    /* pass handle to TEE for write */
    ret = tee_write(mem_handle, GetParam().size, TEE_MEM_WRITE_PATTERN, false);
    ASSERT_EQ(0, ret);

    /* pass handle to GZ for read */
    ret = gz_read(mem_handle, GetParam().size, TEE_MEM_WRITE_PATTERN);
    ASSERT_EQ(0, ret);
    ret = gz_read(mem_handle, GetParam().size, INVALID_MEM_WRITE_PATTERN);
    ASSERT_NE(0, ret);

    /* pass handle to GZ for clean content */
    ret = gz_write(mem_handle, GetParam().size, 0x00000000, false);
    ASSERT_EQ(0, ret);

    /* pass handle to TEE for read */
    ret = tee_read(mem_handle, GetParam().size, 0x00000000);
    ASSERT_EQ(0, ret);
    ret = tee_read(mem_handle, GetParam().size, INVALID_MEM_WRITE_PATTERN);
    ASSERT_NE(0, ret);

    ASSERT_EQ(0, ion_free(ion_fd, handle));
    ASSERT_EQ(0, ion_close(ion_fd));
}

INSTANTIATE_TEST_CASE_P(ION_ALLOC, PmemShareReadWriteTest,
                        ::testing::ValuesIn(alloc_test_params));

INSTANTIATE_TEST_CASE_P(ION_ALLOC_ZERO, PmemShareReadWriteTest,
                        ::testing::ValuesIn(alloc_zero_test_params));
