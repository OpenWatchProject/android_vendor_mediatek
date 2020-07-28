#include "android/memory/DpBlock2NV12Convert.h"
#include <ion/ion.h>
#include "libmtkconv.h"
#include "./android/logger/DpLogger_Android.h"
#include "linux/ion_drv.h"
#include <m4u_lib.h>
#include <sys/mman.h>
#include <linux/mman-proprietary.h>

#include <assert.h>
#include <string.h>
#include <cutils/properties.h>
#define HW_MT8167_E1 "0000"
#define HW_MT8167_E2 "0001"

#if 0
DpConvertBlk_NV12 * DpConvertBlk_NV12::m_pInstance = NULL;
DpMutex    DpConvertBlk_NV12::m_ConvertMutex;

static DpConvertBlk_NV12 * DpConvertBlk_NV12::getInstance()
{
    AutoMutex lock(m_ConvertMutex);
    if (NULL != m_pInstance)
        return m_pInstance;
    m_pInstance = new DpConvertBlk_NV12();
    atexit(destoryInstance);
    return m_pInstance;
}

static void DpConvertBlk_NV12::destoryInstance()
{
    AutoMutex lock(m_ConvertMutex);
    if (NULL != m_pInstance)
        delete m_pInstance;
}
#endif

DpConvertBlk_NV12::DpConvertBlk_NV12()
    : m_rdma_support_blk(false)
    , m_skip_b2r(0)
    , m_width(0)
    , m_height(0)
    , m_pSourceBuffer(NULL)
    , m_sourceBufferHandle(-1)
    , m_internalBufferFD(-1)
    , m_ionHandle(-1)
    , m_internalBufferHandle(0)
    , m_internalBufferVA(NULL)
    , m_isSecure(false)
    , m_internalBufferSecureHandle(-1)
{
    const char *files[] =
    {
        "/proc/chip/sw_ver",
    };
    int i = 0;
    char str[8] = {0};
    FILE* fp = fopen(files[i], "rb");
    int rd_size;

    assert(!fp);

    if (4 != (rd_size = fread(str, 1, 4, fp)))
    {
        DPLOGE("ERROR: read sw_ver error");
    }

    if (strcmp (str,HW_MT8167_E1) == 0)
    {
        m_rdma_support_blk = false;
    }
    else if (strcmp (str, HW_MT8167_E2) == 0)
    {
        m_rdma_support_blk = true;
    }
    else
    {
        DPLOGE("ERROR: sw_ver abnormal");
    }
    fclose(fp);
}

int DpConvertBlk_NV12::allocInternalBuffer(bool isSecure)
{
    if (false == isSecure)
    {
        // allocate ion buffer
        if (-1 != m_ionHandle)
            return 0;
        m_ionHandle = mt_ion_open("DpBlock2NV12Convert");
        int ion_prot_flags = ION_FLAG_CACHED | ION_FLAG_CACHED_NEEDS_SYNC;
        if (ion_alloc_mm(m_ionHandle, MAX_FHD_BLK_SIZE, 0x40, ion_prot_flags, &m_internalBufferHandle)) //For M4U cache alignment
        {
            DPLOGE("DpIonHandler: alloc memory buffer failed\n");
            return -1;
        }

        if (ion_share(m_ionHandle, m_internalBufferHandle, &m_internalBufferFD))
        {
            DPLOGE("DpIonHandler: map file descriptor failed\n");
            return -2;
        }
    }
    else
    {
        DPLOGE("allocInternalBuffer failed for not support nv12blk to nv12 svp");
        return -1;
    }
    return 0;
}

void *DpConvertBlk_NV12::mapInternalBufferVA(void)
{
    if (true == m_isSecure)
        return NULL;
    // normal buffer VA
    unsigned int flag = PROT_READ | PROT_WRITE;
    int ret = 0;
    if (NULL != m_internalBufferVA)
        return m_internalBufferVA;
    if (-1 == m_ionHandle)
    {
        DPLOGD("internal buffer is not allocated, allocate one\n");
        ret = allocInternalBuffer(false);
        if (0 != ret)
        {
            DPLOGE("allocate internal buffer failed[%d]\n", ret);
            return NULL;
        }
    }

    m_internalBufferVA = ion_mmap(m_ionHandle, 0, MAX_FHD_BLK_SIZE, flag, MAP_SHARED, m_internalBufferFD, 0);
    if (NULL == m_internalBufferVA)
    {
        DPLOGE("map internal buffer to VA fail\n");
    }
    return m_internalBufferVA;
}


int DpConvertBlk_NV12::convertBlk_NV12()
{
    if (false == m_isSecure)
    {
        unsigned char *pInternalBuffer_Y = (unsigned char *)mapInternalBufferVA();
        unsigned char *pInternalBuffer_C = pInternalBuffer_Y + (m_width * m_height);
        unsigned char *pBlockBuffer_Y = m_pSourceBuffer;
        unsigned char *pBlockBuffer_C = m_pSourceBuffer + (m_width * m_height);

        if (NULL == pInternalBuffer_Y)
        {
            DPLOGE("error: map internal buffer fail\n");
            return -1;
        }

        // convert from nv12blk to nv12
        do_b_to_nv12(pInternalBuffer_Y, pInternalBuffer_C,
                     pBlockBuffer_Y, pBlockBuffer_C,
                     m_width, m_height);
    }
    else
    {
        DPLOGE("convertBlk_NV12 failed for not support nv12blk to nv12 svp");
        return -1;
    }
    return flushInternalBuffer();
}

void DpConvertBlk_NV12::setSrcBufferInfo(void *pSourceYBuffer,
                                         size_t bufferWidth,
                                         size_t bufferHeight,
                                         bool secure)
{
    if (secure == true)
    {
        // secure buffer handle size is also 32bit in 64bit system
        m_sourceBufferHandle = (unsigned long)pSourceYBuffer;
    }
    else
    {
        m_pSourceBuffer = (unsigned char *)pSourceYBuffer;
    }
    m_width    = bufferWidth;
    m_height   = bufferHeight;
    m_isSecure = secure;
    return;
}


int DpConvertBlk_NV12::flushInternalBuffer()
{
    if (true == m_isSecure)
        return 0; // secure buffer is flush after convert, in trustzone

    // normal buffer flush
    int ret = 0;
    ret = ion_dma_map_area(m_ionHandle, m_internalBufferHandle, ION_DMA_TO_DEVICE);
    if (0 != ret)
    {
        DPLOGE("flush internal buffer failed\n");
        return -1;
    }
    return 0;
}

int DpConvertBlk_NV12::getInternalBufferFD()
{
    if (true == m_isSecure)
        return m_internalBufferSecureHandle;

    // normal buffer
    return m_internalBufferFD;
}

DpConvertBlk_NV12::~DpConvertBlk_NV12()
{
    if (m_internalBufferVA != NULL && false == m_isSecure)
    {
        ion_munmap(m_ionHandle, m_internalBufferVA, MAX_FHD_BLK_SIZE);    // free va
        ion_share_close(m_ionHandle, m_internalBufferFD);    // free ion fd
        ion_free(m_ionHandle, m_internalBufferHandle);    //free ion handle
        ion_close(m_ionHandle);    // free ion file fd
    }

    if (true == m_isSecure)
    {
        DPLOGE("destruct DpConvertBlk_NV12 failed for not support nv12blk to nv12 svp");
    }
}


