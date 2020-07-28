#include "LomoEffect.h"

#ifdef LOG_TAG
#undef LOG_TAG
#endif

#define LOG_TAG "LomoEffect"

#include <cutils/log.h>

#include <android/hidl/memory/1.0/IMemory.h>
//#include <android/hidl/allocator/1.0/IAllocator.h>
//#include <libhidl/base/include/hidl/HidlSupport.h>
#include <libhidl/libhidlmemory/include/hidlmemory/mapping.h>
#include <cutils/properties.h>

#include <dlfcn.h> // for dlopen

using ::android::hidl::memory::V1_0::IMemory;
using ::android::hardware::Status;

using namespace android;
using namespace android::hardware;

// === mkdbg: start ===
#define DEBUG_HIDL_LOMO_ENABLED "debug.hidl.lomo.enabled"

#ifdef FUNC_START
#undef FUNC_START
#endif
#ifdef FUNC_START
#undef FUNC_END
#endif

#if 1
#define FUNC_START if (m_logLevel) { ALOGD("%s +", __FUNCTION__); }
#define FUNC_END if (m_logLevel) { ALOGD("%s -", __FUNCTION__); }
#else
#define FUNC_START
#define FUNC_END
#endif

#define MK_ION_MMAP_SHARE_PER_FRAME 0 // 1: OK, 0:  looks OK when NO allocLomoDst flow, but NG with it
#define MK_UT_HIDLLOMO_DUMP_SRC 0
#define MK_UT_DST_BUF_WO_P2_ENQUE 0
#define DEBUG_LOMO_LOG_LEVEL "vendor.debug.lomo.loglevel"
#define DEBUG_LOMO_DUMPSRC_ENABLED "vendor.debug.lomo.dumpsrc.enabled"
#define DEBUG_LOMO_DUMPSRC_COUNT "vendor.debug.lomo.dumpsrc.yv12.count"

#define LOMO_EFFECT_DST_BUFF_NUM 48 // this should align lomo_hal_jni.cpp
// === mkdbg: end ===

namespace vendor {
namespace mediatek {
namespace hardware {
namespace camera {
namespace lomoeffect {
namespace V1_0 {
namespace implementation {

LomoIonInfo::LomoIonInfo()
{
    reset();
}


LomoIonInfo::~LomoIonInfo()
{
    reset();
}

void LomoIonInfo::reset()
{
    m_pBuffer = NULL;
    m_bufLen = 0;
    m_ionShareFd = -1;
    m_ionImportHandle = -1;
}

void LomoIonInfo::cleanup(const int ion_dev_fd)
{
    if (ion_dev_fd > 0)
    {
        if (m_pBuffer != NULL)
        {
            ion_munmap(ion_dev_fd, m_pBuffer, m_bufLen);
        }
        if (m_ionShareFd != -1)
        {
            ion_share_close(ion_dev_fd, m_ionShareFd);
        }

        if (m_ionImportHandle != -1)
        {
            ion_free(ion_dev_fd, m_ionImportHandle);
        }
    }
    else
    {
        ALOGW("mkdbg: cleanup: ion_dev_fd: %d <= 0", ion_dev_fd);
    }

    reset();
}

void LomoIonInfo::print(const int idx, const int logLevel)
{
    if (logLevel)
    {
        ALOGD("%s: mkdbg: "
            "Number=%.3d, m_ionShareFd=%d, m_pBuffer=%p, m_bufLen=%d, m_ionImportHandle=%d",
            __FUNCTION__,
            idx, m_ionShareFd, m_pBuffer, m_bufLen, m_ionImportHandle);
    }
}

int LomoEffect::dlopenLomoHal(void)
{
    FUNC_START;
    char const szLomoModulePath[] = "libcam.jni.lomohaljni.so";
    char const szLomoEntrySymbol[] = "getLomoHalJniInstance";

    m_lomoLib = ::dlopen(szLomoModulePath, RTLD_NOW);
    if ( !m_lomoLib )
    {
        char const *err_str = ::dlerror();
        ALOGE("dlopen: %s error=%s", szLomoModulePath, ((err_str != NULL) ? err_str : "unknown") );
        goto err_dlopenLomoHal;
    }
    else
    {
        ALOGD("dlopen(%s) ok: %p", szLomoModulePath, m_lomoLib);
        m_pLomofnEntry = ::dlsym(m_lomoLib, szLomoEntrySymbol);
        if ( !m_pLomofnEntry )
        {
            char const *err_str = ::dlerror();
            ALOGE("dlsym: %s error=%s", szLomoEntrySymbol, ((err_str != NULL)  ? err_str : "unknown") );
            goto err_dlopenLomoHal;
        }
        else
        {
            ALOGD("dlsym ok: %p", m_pLomofnEntry);
            mpLomoHalJni = reinterpret_cast<LomoHalJni*(*)()>(m_pLomofnEntry)();
            if ( !mpLomoHalJni)
            {
                ALOGE(" reinterpret_cast<LomoHalJni*(*)()) fail");
                goto err_dlopenLomoHal;

            }
            else
            {
                ALOGD("reinterpret_cast<LomoHalJni*(*)()> ok: %p", mpLomoHalJni);
            }
        }
    }
    FUNC_END;
    return 0;

err_dlopenLomoHal:

    if (m_lomoLib)
    {
        ::dlclose(m_lomoLib);
        m_lomoLib = NULL;
    }
    FUNC_END;
    return -1;
}

Return<Status> LomoEffect::init() {

    m_logLevel = ::property_get_int32(DEBUG_LOMO_LOG_LEVEL, 0);

    FUNC_START;

    m_isInited = 0;

    m_srcImgWidth = 0;
    m_srcImgHeight = 0;
    // init member variables
    for (int i = 0; i < LOMO_EFFECT_DST_BUFF_NUM; ++i)
    {
        m_dstIonInfo[i].reset();
    }
    m_ion_dev_fd = mt_ion_open("hidl_lomoeffect");
    if(m_ion_dev_fd < 0)
    {
        ALOGE("mkdbg: ion_open fail");
        goto err_Init;
    }
    else
    {
        ALOGD("mkdbg: ion_open OK: %d", m_ion_dev_fd);
    }


#if 1 // dlopen method
    if (dlopenLomoHal() != 0)
    {
        ALOGE("mkdbg: mpLomoHalJni::dlopen FAIL");
        goto err_Init;
    }
#else
    mpLomoHalJni = LomoHalJni::createInstance();
    if (mpLomoHalJni == NULL)
    {
        ALOGE("mkdbg: mpLomoHalJni::createInstance FAIL");
        goto err_Init;
    }
#endif

    if (mpLomoHalJni->init() != 0)
    {
        ALOGE("mkdbg: mpLomoHalJni::createInstance FAIL");
        goto err_Init;
    }

    m_isInited = 1;

    FUNC_END;
    return ::android::hardware::camera::common::V1_0::Status {};

err_Init:
    if (m_ion_dev_fd > 0)
    {
        close(m_ion_dev_fd);
        m_ion_dev_fd = -1;
    }

    if (mpLomoHalJni)
    {
        mpLomoHalJni = NULL;
    }
    m_isInited = 0;

    FUNC_END;
    return Status(android::hardware::Status::EX_ILLEGAL_ARGUMENT); // error
}

Return<Status> LomoEffect::uninit() {
    FUNC_START;

    m_isInited = 0;

    if (mpLomoHalJni)
    {
        if (mpLomoHalJni->uninit() != 0)
        {
            ALOGE("mkdbg: mpLomoHalJni->uninit() FAIL");
        }
        if (mpLomoHalJni)
        {
            mpLomoHalJni = NULL;
        }
    }
    else
    {
        ALOGE("mkdbg: mpLomoHalJni == NULL");
    }

    // uninit member variables here.
    for (int i = 0; i < LOMO_EFFECT_DST_BUFF_NUM; ++i)
    {
        m_dstIonInfo[i].print(i, m_logLevel);
        m_dstIonInfo[i].cleanup(m_ion_dev_fd);
    }

    m_srcIonInfo.print(0, m_logLevel);
    m_srcIonInfo.cleanup(m_ion_dev_fd);

    if (m_ion_dev_fd > 0)
    {
        ion_close(m_ion_dev_fd);
        m_ion_dev_fd = -1;
    }

    m_srcImgWidth = 0;
    m_srcImgHeight = 0;

    if (m_lomoLib)
    {
        ::dlclose(m_lomoLib);
        m_lomoLib = NULL;
    }
    FUNC_END;
    return ::android::hardware::camera::common::V1_0::Status {};
}

Return<Status> LomoEffect::allocLomoSrcImage(uint32_t pvCBWidth, uint32_t pvCBHeight, uint32_t pvCBformat, uint32_t ppSrcImgWidth, uint32_t ppSrcImgHeight) {
    FUNC_START;

    if (m_isInited == 0 || mpLomoHalJni == NULL)
    {
        ALOGE("mkdbg: init error or mpLomoHalJni == NULL");
        FUNC_END;
        return Status(android::hardware::Status::EX_ILLEGAL_ARGUMENT);
    }

    #if 0
        ALOGD("mkdbg: AllocLomoSrcImage: funcParam: pvCBWidth=%u, pvCBHeight=%u, pvCBformat=%u, ppSrcImgWidth=%u, ppSrcImgHeight=%u",
            pvCBWidth, pvCBHeight, pvCBformat, ppSrcImgWidth, ppSrcImgHeight);
    #endif

    m_srcImgWidth = pvCBWidth;
    m_srcImgHeight = pvCBHeight;
    if (mpLomoHalJni)
    {
        if (mpLomoHalJni->AllocLomoSrcImage(pvCBWidth, pvCBHeight, pvCBformat, ppSrcImgWidth, ppSrcImgHeight)  != 0)
        {
            ALOGE("mkdbg: mpLomoHalJni->AllocLomoSrcImage() FAIL");
        }
    }
    else
    {
        ALOGE("mkdbg: mpLomoHalJni == NULL");
    }

    FUNC_END;
    return ::android::hardware::camera::common::V1_0::Status {};
}

Return<Status> LomoEffect::allocLomoDstImage(uint32_t Number, int32_t size, const hidl_array<uint32_t, 3>& arrayPlaneOffset, const hidl_handle& dstHidlShareFd) {

    FUNC_START;
    if (m_isInited == 0 || mpLomoHalJni == NULL)
    {
        ALOGE("%s: m_isInited=%d, mpLomoHalJni=%p", __FUNCTION__, m_isInited, mpLomoHalJni);
        FUNC_END;
        return Status(android::hardware::Status::EX_ILLEGAL_ARGUMENT);
    }

    int ion_share_fd = -1;
    int old_ion_share_fd = -1;
    if (dstHidlShareFd != nullptr)
    {
        // !!NOTES: IMPORTANT: need to duplicate, otherwise, HwBinder will free this ion_share_fd
        old_ion_share_fd = dstHidlShareFd->data[0];
        ion_share_fd = dup(old_ion_share_fd);
    }
    else
    {
        ion_share_fd = m_dstIonInfo[Number].m_ionShareFd;
    }
    void *dstBuffer= NULL;
    unsigned char *pDstBuffer = NULL;
#if MK_ION_MMAP_SHARE_PER_FRAME
        // do nothing
#else
//    if (m_dstIonInfo[Number].m_ionImportHandle == -1)
//     if (m_dstIonInfo[Number].m_ionShareFd != ion_share_fd)
    if (dstHidlShareFd.getNativeHandle() != nullptr) // it means that we get this before
#endif
    {
        if (ion_import(m_ion_dev_fd, ion_share_fd, &m_dstIonInfo[Number].m_ionImportHandle) != 0)
        {
            ALOGE("mkdbg: AllocLomoDstImage: ion_import failed: Number: %d", Number);
            goto err_AllocLomoDstImage;
        }

        m_dstIonInfo[Number].m_ionShareFd = ion_share_fd;
        pDstBuffer= (unsigned char*) ion_mmap(m_ion_dev_fd, 0, size, PROT_READ|PROT_WRITE, MAP_SHARED, ion_share_fd, 0);
        if (pDstBuffer == NULL || pDstBuffer == (void*) (-1))
        {
            ALOGE("mkdbg: AllocLomoDstImage: ion_mmap failed: Number: %d", Number);
            goto err_AllocLomoDstImage;
        }

        m_dstIonInfo[Number].m_pBuffer = (unsigned char*) pDstBuffer;
        m_dstIonInfo[Number].m_bufLen = size;

        ALOGD("%s: mkdbg: 1st dst alloc: "
            "old_ion_share_fd=%d, Number=%.3d, m_ionShareFd=%d, m_pBuffer=%p, m_bufLen=%d, m_ionImportHandle=%d",
            __FUNCTION__,
            old_ion_share_fd, Number, m_dstIonInfo[Number].m_ionShareFd, m_dstIonInfo[Number].m_pBuffer, m_dstIonInfo[Number].m_bufLen, m_dstIonInfo[Number].m_ionImportHandle);
    }

#if 0
    ALOGD("mkdbg: %s: funcParam: Number=%u, size=%u, arrayPlaneOffset[3]={%u,%u,%u}, dst_ion_share_id=%d, mappedBuffer=%p",
            __FUNCTION__,
            Number, size,
            arrayPlaneOffset[0], arrayPlaneOffset[1], arrayPlaneOffset[2],
            m_dstIonInfo[Number].m_ionShareFd, m_dstIonInfo[Number].m_pBuffer);
#endif

    // --> to call lomo_hal_jni
    uint8_t *u8lEffectBuff[3];
    u8lEffectBuff[0]=(m_dstIonInfo[Number].m_pBuffer+arrayPlaneOffset[0]);
    u8lEffectBuff[1]=(m_dstIonInfo[Number].m_pBuffer+arrayPlaneOffset[1]);
    u8lEffectBuff[2]=(m_dstIonInfo[Number].m_pBuffer+arrayPlaneOffset[2]);

    if (mpLomoHalJni)
    {
        if (mpLomoHalJni->AllocLomoDstImage(Number, u8lEffectBuff, ion_share_fd) != 0)
        {
            ALOGE("mkdbg: AllocLomoDstImage: mpLomoHalJni->AllocLomoDstImage() FAIL");
        }
    }
    else
    {
        ALOGE("mkdbg: AllocLomoDstImage: mpLomoHalJni == NULL");
    }

#if MK_ION_MMAP_SHARE_PER_FRAME
    m_srcIonInfo.cleanup(ion_dev_fd);
    m_dstIonInfo[Number].cleanup(ion_dev_fd);
#endif

    FUNC_END;
    return ::android::hardware::camera::common::V1_0::Status {};

err_AllocLomoDstImage:

    m_dstIonInfo[Number].cleanup(m_ion_dev_fd);

    FUNC_END;
    return Status(android::hardware::Status::EX_ILLEGAL_ARGUMENT); // error
}


// hidl_memory changed to hidl_handle
Return<Status> LomoEffect::uploadLomoSrcImage(const hidl_handle& srcHidlHandleShareFD, const hidl_array<uint32_t, 3>& arrayPlaneOffset) {
    FUNC_START;

    if (m_isInited == 0 || mpLomoHalJni == NULL)
    {
        ALOGE("mkdbg: m_isInited=%d, mpLomoHalJni=%p", m_isInited, mpLomoHalJni);
        FUNC_END;
        return Status(android::hardware::Status::EX_ILLEGAL_ARGUMENT);
    }

    int ion_share_fd= -1;
    int old_ion_share_fd= -1;
    if (srcHidlHandleShareFD != nullptr)
    {
        // !!NOTES: IMPORTANT: need to duplicate, otherwise, HwBinder will free this ion_share_fd
        old_ion_share_fd= srcHidlHandleShareFD->data[0];
        ion_share_fd= dup(old_ion_share_fd);
    }
    else
    {
        ion_share_fd = m_srcIonInfo.m_ionShareFd;
    }
    void *pSrcBuffer= NULL;
    int srcBufSize = m_srcImgWidth*m_srcImgHeight*3/2;

#if MK_ION_MMAP_SHARE_PER_FRAME
    // do nothing
#else
//    if (m_srcIonInfo.m_ionImportHandle == -1)
    if (srcHidlHandleShareFD != nullptr)
#endif
    {
        if (ion_import(m_ion_dev_fd, ion_share_fd, &m_srcIonInfo.m_ionImportHandle) != 0)
        {
            ALOGE("mkdbg: UploadLomoSrcImage: ion_import failed");
            m_srcIonInfo.m_ionImportHandle   = -1;
            goto err_UploadLomoSrcImage;
        }
        pSrcBuffer = (unsigned char*)ion_mmap(m_ion_dev_fd, 0, srcBufSize,
           PROT_READ|PROT_WRITE, MAP_SHARED, ion_share_fd, 0);
        if (pSrcBuffer == NULL || pSrcBuffer == (void*) (-1) )
        {
            ALOGE("mkdbg: UploadLomoSrcImage: ion_mmap failed");
            goto err_UploadLomoSrcImage;
        }

        m_srcIonInfo.m_ionShareFd = ion_share_fd;
        m_srcIonInfo.m_pBuffer = (unsigned char *) pSrcBuffer ;
        m_srcIonInfo.m_bufLen = srcBufSize ;

        ALOGD("%s: mkdbg: 1st src alloc: "
            "old_ion_share_fd=%d, m_ionShareFd=%d, m_pBuffer=%p, m_bufLen=%d, m_ionImportHandle=%d",
            __FUNCTION__,
            old_ion_share_fd, m_srcIonInfo.m_ionShareFd, m_srcIonInfo.m_pBuffer, m_srcIonInfo.m_bufLen, m_srcIonInfo.m_ionImportHandle);
    }

#if 0
    ALOGD("mkdbg: UploadLomoSrcImage::arrayPlaneOffset[3]={%u, %u, %u}, src_ion_shard_fd=%d, mappedBuffer=%p",
        arrayPlaneOffset[0], arrayPlaneOffset[1], arrayPlaneOffset[2],
        m_srcIonInfo.m_ionShareFd, m_srcIonInfo.m_pBuffer);
#endif


#if MK_UT_HIDLLOMO_DUMP_SRC// mkdbg: debug
    static int bDebugDumpSrcEnabled = 0;
    bDebugDumpSrcEnabled = ::property_get_int32(DEBUG_LOMO_DUMPSRC_ENABLED, 0);
    if (bDebugDumpSrcEnabled)
    {

        int fileSize = srcBufSize;
        int prop_dump_count = ::property_get_int32(DEBUG_LOMO_DUMPSRC_COUNT, 0);
        unsigned char* pvCBVA = (unsigned char *)m_srcIonInfo.m_pBuffer ;

        ALOGD("mkdbg: UploadLomoSrcImage: [%d]: dumpSrcEnabled=%d, dumpSrcImg_count=%d, prop_dump_count=%d, pvCBVA=%p, fileSize=%d",
            hidlLomo_dumpSrcImg_count, bDebugDumpSrcEnabled,
            hidlLomo_dumpSrcImg_count, prop_dump_count, pvCBVA, fileSize);
        if (hidlLomo_dumpSrcImg_count < prop_dump_count)
        {
            char fileName[128];
            sprintf(fileName, "/sdcard/hidllomo_srcImg_%dx%d_%.4d", 960, 540, ++hidlLomo_dumpSrcImg_count);
            FILE *fp = fopen(fileName, "wb");
            if (NULL == fp)
            {
                ALOGE("fail to open src img: %s", fileName);
            }
            else
            {
                int total_write = 0;
                while(total_write < fileSize)
                    {
                        int write_size = fwrite(pvCBVA+total_write, 1, fileSize-total_write, fp);
                        if (write_size <= 0)
                        {
                            ALOGD("mkdbg: UploadLomoSrcImage: write_size=%d", write_size);
                        }
                        total_write += write_size;
                    }
                ALOGD("mkdbg: UploadLomoSrcImage: total_write: %d", total_write);
                fclose(fp);
            }
        }
        else
        {
            hidlLomo_dumpSrcImg_count = 0;

            char value[4];
            value[0] = '0';
            value[0] = '\0';
            ::property_set(DEBUG_LOMO_DUMPSRC_COUNT, value);
        }
    }
#endif

    if (mpLomoHalJni)
    {
        mpLomoHalJni->UploadLomoSrcImage((unsigned char*)m_srcIonInfo.m_pBuffer);
    }
    else
    {
        ALOGE("mkdbg: mpLomoHalJni == NULL");
    }

// clean ion data
#if MK_ION_MMAP_SHARE_PER_FRAME
    m_srcIonInfo.cleanup(m_ion_dev_fd);
#endif

//    ALOGD("mkdbg: UploadLomoSrcImage: -");

    return ::android::hardware::camera::common::V1_0::Status {};

err_UploadLomoSrcImage:

    m_srcIonInfo.cleanup(m_ion_dev_fd);

//    ALOGD("mkdbg: UploadLomoSrcImage: -");
    return Status(android::hardware::Status::EX_ILLEGAL_ARGUMENT); // error

}

Return<Status> LomoEffect::freeLomoSrcImage() {
    FUNC_START;

    if (m_isInited == 0 || mpLomoHalJni == NULL)
    {
        ALOGE("mkdbg: m_isInited=%d, mpLomoHalJni=%p", m_isInited, mpLomoHalJni);
        return Status(android::hardware::Status::EX_ILLEGAL_ARGUMENT);
    }

    if (mpLomoHalJni)
    {
        if (mpLomoHalJni->FreeLomoSrcImage() != 0)
        {
            ALOGE("mkdbg: mpLomoHalJni->FreeLomoSrcImage() FAIL");
        }
    }
    else
    {
        ALOGE("mkdbg: mpLomoHalJni == NULL");
    }

    FUNC_END;
    return ::android::hardware::camera::common::V1_0::Status {};
}

Return<Status> LomoEffect::lomoImageEnque(uint32_t Number, const hidl_array<uint32_t, 3>& arrayPlaneOffset, int32_t ppEffectIdx) {
    FUNC_START;

#if 0
    ALOGD("%s: mkdbg: 111: Number=%d, arrayPlaneOffset={%d,%d,%d}, ppEffectIdx=%d",
        __FUNCTION__,
        Number, arrayPlaneOffset[0], arrayPlaneOffset[1], arrayPlaneOffset[2], ppEffectIdx);
#endif

    if (m_isInited == 0 || mpLomoHalJni == NULL)
    {
        ALOGE("mkdbg: m_isInited=%d, mpLomoHalJni=%p", m_isInited, mpLomoHalJni);
        FUNC_END;
        return Status(android::hardware::Status::EX_ILLEGAL_ARGUMENT);
    }

#if MK_UT_DST_BUF_WO_P2_ENQUE
    for (int i = 0; i < 36; ++i)
    {
        if(m_dstIonInfo[i].m_pBuffer != NULL)
        {
            ALOGD("%s, found: idx=%d", __FUNCTION__, i);

            uint8_t *pBuffer[3];
            pBuffer[0]=(unsigned char*) (m_dstIonInfo[i].m_pBuffer +arrayPlaneOffset[0]);
            pBuffer[1]=(unsigned char*) (m_dstIonInfo[i].m_pBuffer +arrayPlaneOffset[1]);
            pBuffer[2]=(unsigned char*) (m_dstIonInfo[i].m_pBuffer +arrayPlaneOffset[2]);

           srand(time(NULL));
           int random_num = (rand()%100)+1;

                if (random_num % 3 == 0)
                {
                    memset(pBuffer[0], 128, arrayPlaneOffset[2]);
//                    memset(pBuffer[0], 128, 50);
//                    memset(pBuffer[1], 192, 50);
//                    memset(pBuffer[2], 255, 50);
                }
                else if (random_num % 3 == 1)
                {
                    memset(pBuffer[0], 192, arrayPlaneOffset[2]);
//                    memset(pBuffer[0], 255, 50);
//                    memset(pBuffer[1], 128, 50);
//                    memset(pBuffer[2], 192, 50);
                }
                else
                {
                    memset(pBuffer[0], 255, arrayPlaneOffset[2]);
//                    memset(pBuffer[0], 192, 50);
//                    memset(pBuffer[1], 255, 50);
//                    memset(pBuffer[2], 128, 50);
                }
          }
    }
#endif

    uint8_t *u8lEffectBuff[3];

    u8lEffectBuff[0]=(unsigned char*) (m_dstIonInfo[Number].m_pBuffer +arrayPlaneOffset[0]);
    u8lEffectBuff[1]=(unsigned char*) (m_dstIonInfo[Number].m_pBuffer +arrayPlaneOffset[1]);
    u8lEffectBuff[2]=(unsigned char*) (m_dstIonInfo[Number].m_pBuffer +arrayPlaneOffset[2]);

#if 0
    ALOGD("%s: mkdbg: 222: m_dstIonInfo[%d].m_pBuffer=%p", __FUNCTION__, Number, m_dstIonInfo[Number].m_pBuffer);
#endif

    if (mpLomoHalJni)
    {
        if (mpLomoHalJni->LomoImageEnque(u8lEffectBuff, ppEffectIdx) != 0)
        {
            ALOGE("mkdbg: mpLomoHalJni->LomoImageEnque() FAIL");
        }
    }
    else
    {
        ALOGE("mkdbg: mpLomoHalJni == NULL");
    }

    FUNC_END;

    return ::android::hardware::camera::common::V1_0::Status {};
}

Return<Status> LomoEffect::lomoImageDeque(uint32_t Number, const hidl_array<uint32_t, 3>& arrayPlaneOffset, int32_t ppEffectIdx) {
    FUNC_START;

#if 0
    ALOGD("%s: mkdbg: Number=%d, arrayPlaneOffset={%p, %p, %p}, ppEffectIdx=%d",
        __FUNCTION__,
        Number, arrayPlaneOffset[0], arrayPlaneOffset[1], arrayPlaneOffset[2], ppEffectIdx);
#endif

    if (m_isInited == 0 || mpLomoHalJni == NULL)
    {
        ALOGE("mkdbg: m_isInited=%d, mpLomoHalJni=%p", m_isInited, mpLomoHalJni);
        ALOGD("mkdbg: LomoImageDeque: -");
        return Status(android::hardware::Status::EX_ILLEGAL_ARGUMENT);
    }

    if (ppEffectIdx < 0)
    {
        ALOGD("mkdbg: LomoImageDeque: -");
        return Status(android::hardware::Status::EX_ILLEGAL_ARGUMENT);
    }


    uint8_t *u8lEffectBuff[3];
    u8lEffectBuff[0]=(unsigned char*) (m_dstIonInfo[Number].m_pBuffer+arrayPlaneOffset[0]);
    u8lEffectBuff[1]=(unsigned char*) (m_dstIonInfo[Number].m_pBuffer +arrayPlaneOffset[1]);
    u8lEffectBuff[2]=(unsigned char*) (m_dstIonInfo[Number].m_pBuffer +arrayPlaneOffset[2]);

    if (mpLomoHalJni)
    {
        if (mpLomoHalJni->LomoImageDeque(u8lEffectBuff, ppEffectIdx) != 0)
        {
            ALOGE("mkdbg: mpLomoHalJni->LomoImageEnque() FAIL");
        }
    }
    else
    {
        ALOGE("mkdbg: mpLomoHalJni == NULL");
    }

    FUNC_END;

    return ::android::hardware::camera::common::V1_0::Status {};
}

Return<Status> LomoEffect::freeLomoDstImage() {
    FUNC_START;

    if (m_isInited == 0 || mpLomoHalJni == NULL)
    {
        ALOGE("mkdbg: m_isInited=%d, mpLomoHalJni=%p", m_isInited, mpLomoHalJni);
        return Status(android::hardware::Status::EX_ILLEGAL_ARGUMENT);
    }

    if (mpLomoHalJni)
    {
        if (mpLomoHalJni->FreeLomoDstImage() != 0)
        {
            ALOGE("mkdbg: mpLomoHalJni->FreeLomoDstImage() FAIL");
        }
    }
    else
    {
        ALOGE("mkdbg: mpLomoHalJni == NULL");
    }

    FUNC_END;
    return ::android::hardware::camera::common::V1_0::Status {};
}

// Methods from ::android::hidl::base::V1_0::IBase follow.

ILomoEffect* HIDL_FETCH_ILomoEffect(const char* /* name */) {
    return new LomoEffect();
}

}  // namespace implementation
}  // namespace V1_0
}  // namespace hidl_lomo_hal
}  // namespace camera
}  // namespace hardware
}  // namespace mediatek
}  // namespace vendor
