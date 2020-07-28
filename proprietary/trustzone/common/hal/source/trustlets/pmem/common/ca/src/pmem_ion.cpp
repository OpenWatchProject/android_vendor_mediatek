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
#include <unistd.h>

#include <ion.h>
#include <ion/ion.h>
#include <linux/ion.h>
#include <linux/ion_drv.h>
#ifdef ION_NUM_HEAP_IDS
#undef ION_NUM_HEAP_IDS
#endif
#include <linux/mtk_ion.h>

#include "pmem_common.h"
#include "pmem_test_cfg.h"

#define ION_MIN_CHUNK_SIZE (SIZE_4K)
#define PMEM_ION_MASK ION_HEAP_MULTIMEDIA_TYPE_PROT_MASK

#define SVP_ION_MASK ION_HEAP_MULTIMEDIA_SEC_MASK
#define SVP_MIN_CHUNK_SIZE SIZE_64K

#define FR_ION_MASK ION_HEAP_MULTIMEDIA_TYPE_2D_FR_MASK
#define FR_MIN_CHUNK_SIZE SIZE_64K
#define FR_MAX_SIZE SIZE_16M

class PmemIonSingleTest : public ::testing::Test
{
  protected:
    void SetUp()
    {
    }
    void TearDown()
    {
    }
};

class PmemIonBaseTest : public ::testing::Test
{
  public:
    static void SetUpTestCase()
    {
    }
    static void TearDownTestCase()
    {
    }
};

class PmemUnOrderedTest : public PmemIonBaseTest,
                          public ::testing::WithParamInterface<AllocParameters>
{
};

class PmemAllocTest : public PmemIonBaseTest,
                      public ::testing::WithParamInterface<AllocParameters>
{
};

class PmemSaturationTest : public PmemIonBaseTest,
                           public ::testing::WithParamInterface<AllocParameters>
{
};

class PmemBoundaryTest : public PmemIonBaseTest,
                         public ::testing::WithParamInterface<AllocParameters>
{
};

class PmemAlignmentTest : public PmemIonBaseTest,
                          public ::testing::WithParamInterface<AllocParameters>
{
};

class FrSaturationTest : public PmemIonBaseTest,
                         public ::testing::WithParamInterface<AllocParameters>
{
};

/****************************************************************************
 ** ION device open/close test
 ****************************************************************************/
TEST_F(PmemIonSingleTest, ion_device_open_Success)
{
    int ion_fd;

    ASSERT_TRUE((ion_fd = ion_open()) >= 0);
    ASSERT_EQ(0, ion_close(ion_fd));
};

/****************************************************************************
 ** ION device multiple region online test
 ****************************************************************************/
TEST_F(PmemIonSingleTest, ion_multiple_region_online_Success)
{
    int ion_fd;
    ion_user_handle_t pmem_handle;
    ion_user_handle_t fr_handle;
    ion_user_handle_t svp_handle;

    ASSERT_TRUE((ion_fd = ion_open()) >= 0);

    /* Allocate FR region first, then SVP region is not allowed */
    ASSERT_EQ(0, ion_alloc(ion_fd, ION_MIN_CHUNK_SIZE, 0, PMEM_ION_MASK, 0,
                           &pmem_handle));
    ASSERT_NE(0, pmem_handle);
#if defined(CAM_2D_FR_REGION_ENABLED) & defined(SVP_SECMEM_REGION_ENABLED)
    ASSERT_EQ(
        0, ion_alloc(ion_fd, FR_MIN_CHUNK_SIZE, 0, FR_ION_MASK, 0, &fr_handle));
    ASSERT_NE(0, fr_handle);
    ASSERT_NE(0, ion_alloc(ion_fd, SVP_MIN_CHUNK_SIZE, 0, SVP_ION_MASK, 0,
                           &svp_handle));
    ASSERT_EQ(0, ion_free(ion_fd, fr_handle));
    ASSERT_NE(0, ion_alloc(ion_fd, SVP_MIN_CHUNK_SIZE, 0, SVP_ION_MASK, 0,
                           &svp_handle));
#elif defined(CAM_2D_FR_REGION_ENABLED)
    ASSERT_EQ(
        0, ion_alloc(ion_fd, FR_MIN_CHUNK_SIZE, 0, FR_ION_MASK, 0, &fr_handle));
    ASSERT_NE(0, fr_handle);
    ASSERT_EQ(0, ion_free(ion_fd, fr_handle));
    ASSERT_NE(0, ion_alloc(ion_fd, SVP_MIN_CHUNK_SIZE, 0, SVP_ION_MASK, 0,
                           &svp_handle));
#elif defined(SVP_SECMEM_REGION_ENABLED)
    ASSERT_EQ(0, ion_alloc(ion_fd, SVP_MIN_CHUNK_SIZE, 0, SVP_ION_MASK, 0,
                           &svp_handle));
    ASSERT_NE(0, svp_handle);
    ASSERT_EQ(0, ion_free(ion_fd, svp_handle));
    ASSERT_NE(
        0, ion_alloc(ion_fd, FR_MIN_CHUNK_SIZE, 0, FR_ION_MASK, 0, &fr_handle));
#endif
    ASSERT_EQ(0, ion_free(ion_fd, pmem_handle));

    /* Trusted mem region off delay is set to 1s */
    sleep(2);

    /* Allocate SVP region first, then FR region is not allowed */
    ASSERT_EQ(0, ion_alloc(ion_fd, ION_MIN_CHUNK_SIZE, 0, PMEM_ION_MASK, 0,
                           &pmem_handle));
    ASSERT_NE(0, pmem_handle);
#if defined(CAM_2D_FR_REGION_ENABLED) & defined(SVP_SECMEM_REGION_ENABLED)
    ASSERT_EQ(0, ion_alloc(ion_fd, SVP_MIN_CHUNK_SIZE, 0, SVP_ION_MASK, 0,
                           &svp_handle));
    ASSERT_NE(0, svp_handle);
    ASSERT_NE(
        0, ion_alloc(ion_fd, FR_MIN_CHUNK_SIZE, 0, FR_ION_MASK, 0, &fr_handle));
    ASSERT_EQ(0, ion_free(ion_fd, svp_handle));
    ASSERT_NE(
        0, ion_alloc(ion_fd, FR_MIN_CHUNK_SIZE, 0, FR_ION_MASK, 0, &fr_handle));
#endif
    ASSERT_EQ(0, ion_free(ion_fd, pmem_handle));

    ASSERT_EQ(0, ion_close(ion_fd));
};

/****************************************************************************
 ** ION allocate/free test (small size - ION will align 4KB page)
 ****************************************************************************/
TEST_F(PmemIonSingleTest, ion_alloc_unsupported_size_Success)
{
    int ion_fd;
    ion_user_handle_t handle;

    ASSERT_TRUE((ion_fd = ion_open()) >= 0);

    /* small size with zero alignment */
    ASSERT_EQ(0, ion_alloc(ion_fd, SIZE_512B, 0, PMEM_ION_MASK, 0, &handle));
    ASSERT_NE(0, handle);
    ASSERT_EQ(0, ion_free(ion_fd, handle));
    ASSERT_EQ(0, ion_alloc(ion_fd, SIZE_1K, 0, PMEM_ION_MASK, 0, &handle));
    ASSERT_NE(0, handle);
    ASSERT_EQ(0, ion_free(ion_fd, handle));
    ASSERT_EQ(0, ion_alloc(ion_fd, SIZE_2K, 0, PMEM_ION_MASK, 0, &handle));
    ASSERT_NE(0, handle);
    ASSERT_EQ(0, ion_free(ion_fd, handle));

    /* small size with wrong alignment, will be auto adjusted */
    ASSERT_EQ(0,
              ion_alloc(ion_fd, SIZE_1K, SIZE_512B, PMEM_ION_MASK, 0, &handle));
    ASSERT_NE(0, handle);
    ASSERT_EQ(0, ion_free(ion_fd, handle));
    ASSERT_EQ(0, ion_alloc(ion_fd, SIZE_2K, SIZE_1K, PMEM_ION_MASK, 0, &handle));
    ASSERT_NE(0, handle);
    ASSERT_EQ(0, ion_free(ion_fd, handle));
    ASSERT_EQ(0, ion_alloc(ion_fd, SIZE_4K, SIZE_2K, PMEM_ION_MASK, 0, &handle));
    ASSERT_NE(0, handle);
    ASSERT_EQ(0, ion_free(ion_fd, handle));

    /* small size with correct alignment */
    ASSERT_EQ(0,
              ion_alloc(ion_fd, SIZE_512B, SIZE_4K, PMEM_ION_MASK, 0, &handle));
    ASSERT_NE(0, handle);
    ASSERT_EQ(0, ion_free(ion_fd, handle));
    ASSERT_EQ(0, ion_alloc(ion_fd, SIZE_1K, SIZE_4K, PMEM_ION_MASK, 0, &handle));
    ASSERT_NE(0, handle);
    ASSERT_EQ(0, ion_free(ion_fd, handle));
    ASSERT_EQ(0, ion_alloc(ion_fd, SIZE_2K, SIZE_4K, PMEM_ION_MASK, 0, &handle));
    ASSERT_NE(0, handle);
    ASSERT_EQ(0, ion_free(ion_fd, handle));

    ASSERT_EQ(0, ion_close(ion_fd));
}

/****************************************************************************
 ** ION allocate/free unordered size test
 ****************************************************************************/
TEST_P(PmemUnOrderedTest, ion_alloc_Success)
{
    int ion_fd;
    ion_user_handle_t handle;
    size_t alignment = GetParam().alignment;
    int try_size = GetParam().size + SIZE_4K;

    /* the limitation is set only when using pmem mock interface */
    if (try_size > PMEM_MAX_CHUNK_SIZE) {
        return;
    }

    /* ion alignment is aligned to ion page size */
    if ((alignment != 0) && (try_size < ION_MIN_CHUNK_SIZE)) {
        alignment = ION_MIN_CHUNK_SIZE;
    }

    ASSERT_TRUE((ion_fd = ion_open()) >= 0);
    if (alignment == 0) {
        /* alignment equals 0, then alignment will be adjusted automatically */
        ASSERT_EQ(0, ion_alloc(ion_fd, try_size, alignment, PMEM_ION_MASK,
                               GetParam().flags, &handle));
        ASSERT_NE(0, handle);
        ASSERT_EQ(0, ion_free(ion_fd, handle));
    } else {
        /* alignment < size, then alignment will be adjusted automatically also
         */
        ASSERT_EQ(0, ion_alloc(ion_fd, try_size, alignment, PMEM_ION_MASK,
                               GetParam().flags, &handle));
        ASSERT_NE(0, handle);
        ASSERT_EQ(0, ion_free(ion_fd, handle));
    }
    ASSERT_EQ(0, ion_close(ion_fd));
}

INSTANTIATE_TEST_CASE_P(ION_ALLOC, PmemUnOrderedTest,
                        ::testing::ValuesIn(alloc_test_params));

INSTANTIATE_TEST_CASE_P(ION_ALLOC_ZERO, PmemUnOrderedTest,
                        ::testing::ValuesIn(alloc_zero_test_params));

INSTANTIATE_TEST_CASE_P(ION_ALLOC_ALIGNED, PmemUnOrderedTest,
                        ::testing::ValuesIn(alloc_aligned_test_params));

INSTANTIATE_TEST_CASE_P(ION_ALLOC_ZERO_ALIGNED, PmemUnOrderedTest,
                        ::testing::ValuesIn(alloc_zero_aligned_test_params));

/****************************************************************************
 ** ION allocate/free test
 ****************************************************************************/
TEST_P(PmemAllocTest, ion_alloc_Success)
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

INSTANTIATE_TEST_CASE_P(ION_ALLOC, PmemAllocTest,
                        ::testing::ValuesIn(alloc_test_params));

INSTANTIATE_TEST_CASE_P(ION_ALLOC_ZERO, PmemAllocTest,
                        ::testing::ValuesIn(alloc_zero_test_params));

INSTANTIATE_TEST_CASE_P(ION_ALLOC_ALIGNED, PmemAllocTest,
                        ::testing::ValuesIn(alloc_aligned_test_params));

INSTANTIATE_TEST_CASE_P(ION_ALLOC_ZERO_ALIGNED, PmemAllocTest,
                        ::testing::ValuesIn(alloc_zero_aligned_test_params));

/****************************************************************************
 ** ION maximum pool size auto detection
 ****************************************************************************/
#ifdef TEST_REGION_SIZE_AUTO_DETECTION
#include "pmem_dbg.h"
static int max_pmem_pool_size = SIZE_1M;
TEST_F(PmemIonSingleTest, ion_maximum_size_auto_detection_Success)
{
    int ion_fd;
    ion_user_handle_t handle;
    int ret;
    int try_size = SIZE_8M; /* Starting from 8MB */

    ASSERT_TRUE((ion_fd = ion_open()) >= 0);

    while (try_size <= SIZE_1G) {
        ret = ion_alloc(ion_fd, try_size, 0, PMEM_ION_MASK, 0, &handle);
        if (ret == 0 && handle != 0) {
            max_pmem_pool_size = try_size;
            try_size += SIZE_8M;
            ASSERT_EQ(0, ion_free(ion_fd, handle));
        } else {
            CONSOLE_FORCE("pmem max pool size is=", max_pmem_pool_size);
            break;
        }
    }

    ASSERT_EQ(0, ion_close(ion_fd));
};
#else
static int max_pmem_pool_size = PMEM_MAX_SIZE;
#endif

/****************************************************************************
 ** ION allocate/free saturation test
 ****************************************************************************/
TEST_P(PmemSaturationTest, ion_alloc_saturation_Success)
{
    int ion_fd, ret;
    int max_items = max_pmem_pool_size / GetParam().size;
    ion_user_handle_t* p_handle_list = NULL;

    /* we only test for size larger than ION page size */
    if (GetParam().size < PMEM_MIN_CHUNK_SIZE) {
        return;
    }

#ifdef TEST_LARGE_CHUNK_SIZE_ONLY
    if (GetParam().size < SIZE_1M) {
        return;
    }
#endif

    /* the limitation is set only when using pmem mock interface */
    if (GetParam().size > PMEM_MAX_CHUNK_SIZE) {
        return;
    }

    p_handle_list =
        (ion_user_handle_t*)malloc(sizeof(ion_user_handle_t) * max_items);
    ASSERT_TRUE(NULL != p_handle_list);

    ASSERT_TRUE((ion_fd = ion_open()) >= 0);

    for (int i = 0; i < max_items; i++) {
        ret = ion_alloc(ion_fd, GetParam().size, GetParam().alignment,
                        PMEM_ION_MASK, GetParam().flags, &p_handle_list[i]);
        ASSERT_EQ(0, ret);
        ASSERT_NE(0, p_handle_list[i]);
    }

    for (int i = (max_items - 1); i >= 0; i--) {
        ret = ion_free(ion_fd, p_handle_list[i]);
        ASSERT_EQ(0, ret);
    }

    ASSERT_EQ(0, ion_close(ion_fd));

    free(p_handle_list);
}

INSTANTIATE_TEST_CASE_P(ION_ALLOC, PmemSaturationTest,
                        ::testing::ValuesIn(alloc_test_params));

INSTANTIATE_TEST_CASE_P(ION_ALLOC_ZERO, PmemSaturationTest,
                        ::testing::ValuesIn(alloc_zero_test_params));

INSTANTIATE_TEST_CASE_P(ION_ALLOC_ALIGNED, PmemSaturationTest,
                        ::testing::ValuesIn(alloc_aligned_test_params));

INSTANTIATE_TEST_CASE_P(ION_ALLOC_ZERO_ALIGNED, PmemSaturationTest,
                        ::testing::ValuesIn(alloc_zero_aligned_test_params));

/****************************************************************************
 ** ION allocate/free boundary test
 ****************************************************************************/
TEST_P(PmemBoundaryTest, ion_alloc_boundary_Success)
{
    int ion_fd, ret;
    int max_items = max_pmem_pool_size / GetParam().size;
    ion_user_handle_t* p_handle_list = NULL;
    ion_user_handle_t extra_handle = 0;

    /* we only test for size larger than ION page size */
    if (GetParam().size < PMEM_MIN_CHUNK_SIZE) {
        return;
    }

#ifdef TEST_LARGE_CHUNK_SIZE_ONLY
    if (GetParam().size < SIZE_1M) {
        return;
    }
#endif

    /* the limitation is set only when using pmem mock interface */
    if (GetParam().size > PMEM_MAX_CHUNK_SIZE) {
        return;
    }

    p_handle_list =
        (ion_user_handle_t*)malloc(sizeof(ion_user_handle_t) * max_items);
    ASSERT_TRUE(NULL != p_handle_list);

    ASSERT_TRUE((ion_fd = ion_open()) >= 0);

    for (int i = 0; i < max_items; i++) {
        ret = ion_alloc(ion_fd, GetParam().size, GetParam().alignment,
                        PMEM_ION_MASK, GetParam().flags, &p_handle_list[i]);
        ASSERT_EQ(0, ret);
        ASSERT_NE(0, p_handle_list[i]);
    }

    /* one more allocation, and expect the allocation is failed */
    ret = ion_alloc(ion_fd, SIZE_1K, SIZE_1K, PMEM_ION_MASK, 0, &extra_handle);
    ASSERT_NE(0, ret);

    for (int i = (max_items - 1); i >= 0; i--) {
        ret = ion_free(ion_fd, p_handle_list[i]);
        ASSERT_EQ(0, ret);
    }

    ASSERT_EQ(0, ion_close(ion_fd));

    free(p_handle_list);
}

INSTANTIATE_TEST_CASE_P(ION_ALLOC, PmemBoundaryTest,
                        ::testing::ValuesIn(alloc_test_params));

INSTANTIATE_TEST_CASE_P(ION_ALLOC_ZERO, PmemBoundaryTest,
                        ::testing::ValuesIn(alloc_zero_test_params));

INSTANTIATE_TEST_CASE_P(ION_ALLOC_ALIGNED, PmemBoundaryTest,
                        ::testing::ValuesIn(alloc_aligned_test_params));

INSTANTIATE_TEST_CASE_P(ION_ALLOC_ZERO_ALIGNED, PmemBoundaryTest,
                        ::testing::ValuesIn(alloc_zero_aligned_test_params));

/****************************************************************************
 ** ION allocate/free alignment test (size != alignment)
 ****************************************************************************/
TEST_P(PmemAlignmentTest, ion_alloc_smaller_align_size_Success)
{
    int ion_fd;
    ion_user_handle_t handle;

    /* we only test for size larger than ION page size */
    if (GetParam().size < ION_MIN_CHUNK_SIZE) {
        return;
    }

    /* the limitation is set only when using pmem mock interface */
    if (GetParam().size > PMEM_MAX_CHUNK_SIZE) {
        return;
    }

    ASSERT_TRUE((ion_fd = ion_open()) >= 0);

    /* alignment is less than size, we expect the allocation is success also */
    ASSERT_EQ(0, ion_alloc(ion_fd, GetParam().size, GetParam().size / 2,
                           PMEM_ION_MASK, GetParam().flags, &handle));
    ASSERT_NE(0, handle);
    ASSERT_EQ(0, ion_free(ion_fd, handle));

    /* alignment is larger than size, we expect the allocation is success */
    ASSERT_EQ(0, ion_alloc(ion_fd, GetParam().size, GetParam().size * 2,
                           PMEM_ION_MASK, GetParam().flags, &handle));
    ASSERT_NE(0, handle);
    ASSERT_EQ(0, ion_free(ion_fd, handle));

    /* alignment is zero, we expect the allocation is success */
    ASSERT_EQ(0, ion_alloc(ion_fd, GetParam().size, 0, PMEM_ION_MASK,
                           GetParam().flags, &handle));
    ASSERT_NE(0, handle);
    ASSERT_EQ(0, ion_free(ion_fd, handle));

    ASSERT_EQ(0, ion_close(ion_fd));
}

INSTANTIATE_TEST_CASE_P(ION_ALLOC_UNALIGNED, PmemAlignmentTest,
                        ::testing::ValuesIn(alloc_test_params));

#if defined(CAM_2D_FR_REGION_ENABLED)
/****************************************************************************
 ** FR ION allocate/free saturation test
 ****************************************************************************/
TEST_P(FrSaturationTest, ion_alloc_saturation_Success)
{
    int ion_fd, ret;
    int max_items = FR_MAX_SIZE / GetParam().size;
    ion_user_handle_t* p_handle_list = NULL;

    /* we only test for size larger than ION page size */
    if (GetParam().size < FR_MIN_CHUNK_SIZE) {
        return;
    }

    /* the limitation is set only when using pmem mock interface */
    if (GetParam().size > FR_MAX_SIZE) {
        return;
    }

    p_handle_list =
        (ion_user_handle_t*)malloc(sizeof(ion_user_handle_t) * max_items);
    ASSERT_TRUE(NULL != p_handle_list);

    ASSERT_TRUE((ion_fd = ion_open()) >= 0);

    for (int i = 0; i < max_items; i++) {
        ret = ion_alloc(ion_fd, GetParam().size, GetParam().alignment,
                        FR_ION_MASK, GetParam().flags, &p_handle_list[i]);
        ASSERT_EQ(0, ret);
        ASSERT_NE(0, p_handle_list[i]);
    }

    for (int i = (max_items - 1); i >= 0; i--) {
        ret = ion_free(ion_fd, p_handle_list[i]);
        ASSERT_EQ(0, ret);
    }

    ASSERT_EQ(0, ion_close(ion_fd));

    free(p_handle_list);
}

INSTANTIATE_TEST_CASE_P(ION_ALLOC, FrSaturationTest,
                        ::testing::ValuesIn(alloc_test_params));

INSTANTIATE_TEST_CASE_P(ION_ALLOC_ZERO, FrSaturationTest,
                        ::testing::ValuesIn(alloc_zero_test_params));

INSTANTIATE_TEST_CASE_P(ION_ALLOC_ALIGNED, FrSaturationTest,
                        ::testing::ValuesIn(alloc_aligned_test_params));

INSTANTIATE_TEST_CASE_P(ION_ALLOC_ZERO_ALIGNED, FrSaturationTest,
                        ::testing::ValuesIn(alloc_zero_aligned_test_params));
#endif
