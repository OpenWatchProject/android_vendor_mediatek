#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "SpeechExtMemUSIP"
#include <SpeechExtMemUSIP.h>
#include <SpeechMessengerNormal.h>
#include <utils/Log.h>
#include <cutils/properties.h>

extern "C" {
#include <hardware/ccci_intf.h>
}

#include <sys/mman.h>
#include <AudioAssert.h>//Mutex/assert
#include <SpeechUtility.h>
#include <SpeechCCCIType.h>
#include <AudioUtility.h>


namespace android {

/*
 * =============================================================================
 *                     typedef
 * =============================================================================
 */
//#define ENABLE_SPH_USIP_DUMP
#define SPH_DUMP_STR_SIZE (1024)
#define USIP_EMP_IOC_MAGIC 'D'
#define GET_USIP_EMI_SIZE _IOWR(USIP_EMP_IOC_MAGIC, 0xF0, unsigned long long)

#define SPEECH_USIP_SHM_SIZE (192* 1024)
#define SPEECH_USIP_SHM_CACHE_RESERVE_SIZE (3072)
#define SPEECH_USIP_SHM_GUARD_REGION_SIZE (32)
#define SPEECH_USIP_SHM_SPEECH_PARAM_SIZE (180192)//184k-32
#define SPEECH_USIP_SHM_MD_PRIVATE_SIZE (13272)//16k-3k-40

typedef struct region_info_t {
    uint32_t offset;        /* ex: 0x1000 */ /* count from the address of sph_usip_shm_t */
    uint32_t size;          /* ex: 0x100 */
    uint32_t read_idx;      /* ex: 0x0 ~ 0xFF */
    uint32_t write_idx;     /* ex: 0x0 ~ 0xFF */
} region_info_t; /* 16 bytes */

typedef struct sph_usip_shm_t {

    /* 3072 bytes reserve */
    uint8_t cache_reserved[SPEECH_USIP_SHM_CACHE_RESERVE_SIZE];

    /*32 bytes gurard region */
    uint8_t guard_region_pre[SPEECH_USIP_SHM_GUARD_REGION_SIZE];

    /* 8 bytes memory block offset */
    uint32_t md_private_range_offset; /* offset of md_private_range*/
    uint32_t sph_param_offset; /* offset of sph_param*/

    /* 13272 bytes reserve */
    uint8_t md_private_range[SPEECH_USIP_SHM_MD_PRIVATE_SIZE];

    /* 184K-32 bytes speech param */
    uint8_t sph_param[SPEECH_USIP_SHM_SPEECH_PARAM_SIZE];

    /*32 bytes gurard region */
    uint8_t guard_region_post[SPEECH_USIP_SHM_GUARD_REGION_SIZE];
} sph_usip_shm_t;

/*
 * =============================================================================
 *                     global value
 * =============================================================================
 */

static const char kPropertyKeyShareMemoryInit[PROPERTY_KEY_MAX] = "vendor.audiohal.speech.shm_usip";
static char const *const kUsipDeviceName = "/dev/usip";
static const char kPropertyKeySphParamWriteIdx[PROPERTY_KEY_MAX] = "vendor.audiohal.speech.shm_widx";

/*
 * =============================================================================
 *                     class implementation
 * =============================================================================
 */
SpeechExtMemUSIP::SpeechExtMemUSIP() {
    ALOGD("%s()", __FUNCTION__);
    mModemIndex = MODEM_1;
    mCcciShareMemoryHandler = -1;
    mShareMemoryBase = NULL;
    mShareMemoryLength = 0;
    mShareMemory = NULL;
    mSpeechParamRegion = NULL;
    AUDIO_ALLOC_STRUCT(region_info_t, mSpeechParamRegion);
}

SpeechExtMemUSIP::~SpeechExtMemUSIP() {
    ALOGD("%s()", __FUNCTION__);
    AUDIO_FREE_POINTER(mSpeechParamRegion);
}

int SpeechExtMemUSIP::openShareMemory(const modem_index_t modem_index) {
#ifndef MTK_CCCI_SHARE_BUFFER_SUPPORT
    return 0;
#endif
    mModemIndex = modem_index;

    if (mCcciShareMemoryHandler >= 0 &&
        mShareMemoryBase != NULL &&
        mShareMemoryLength >= sizeof(sph_usip_shm_t)) {
        ALOGD("%s(), modem_index: %d, mCcciShareMemoryHandler: %d, mShareMemoryBase: %p, "
              "mShareMemoryLength: %u already open",
              __FUNCTION__, modem_index, mCcciShareMemoryHandler, mShareMemoryBase, (uint32_t)mShareMemoryLength);
        return 0;
    }

    // get share momoey address
    int mCcciShareMemoryHandler = open(kUsipDeviceName, O_RDWR);
    unsigned long long byteMemory = 0;

    if (mCcciShareMemoryHandler < 0) {
        ALOGE("%s(), open(%s) fail, mCcciShareMemoryHandler = %d, errno: %d",
              __FUNCTION__, kUsipDeviceName, mCcciShareMemoryHandler, errno);
        return -ENODEV;
    }

    ::ioctl(mCcciShareMemoryHandler, GET_USIP_EMI_SIZE, &byteMemory);
    ALOGD("%s(), mCcciShareMemoryHandler = %d, GET_USIP_EMI_SIZE byteMemory = %llu",  __FUNCTION__, mCcciShareMemoryHandler, byteMemory);
    if (byteMemory == 0) {
        ALOGE("%s(),  byteMemory(%u) == 0", __FUNCTION__,
              (uint32_t)byteMemory);
        closeShareMemory();
        return -EFAULT;
    }

    mShareMemoryLength = (unsigned int)byteMemory;
    mShareMemoryBase = (unsigned char *)mmap(NULL, byteMemory,
                                             PROT_READ | PROT_WRITE, MAP_FILE | MAP_SHARED, mCcciShareMemoryHandler, 0);

    if (mShareMemoryBase == NULL || mShareMemoryBase == MAP_FAILED) {
        ALOGE("%s(), failed to mmap buffer %d bytes, errno %d\n",
              __FUNCTION__, 0x1000, errno);
        closeShareMemory();
        return -EFAULT;
    }

    ALOGD("%s(), mShareMemoryBase = %p, mShareMemoryLength = %d, ShareMemoryBase[0~4] = 0x%x,0x%x,0x%x,0x%x,0x%x",
          __FUNCTION__, mShareMemoryBase, mShareMemoryLength,
          mShareMemoryBase[0], mShareMemoryBase[1], mShareMemoryBase[2], mShareMemoryBase[3], mShareMemoryBase[4]);

    if (mShareMemoryLength < sizeof(sph_usip_shm_t)) {
        ALOGE("%s(), mShareMemoryLength(%u) < sizeof(sph_usip_shm_t): %u", __FUNCTION__,
              (uint32_t)mShareMemoryLength, (uint32_t)sizeof(sph_usip_shm_t));
        closeShareMemory();
        return -ENOMEM;
    }

    int retval = 0;
    if (get_uint32_from_mixctrl(kPropertyKeyShareMemoryInit) != 0) {
        mShareMemory = (sph_usip_shm_t *)mShareMemoryBase;

    } else {
        retval = formatShareMemory();
    }

    return retval;
}


int SpeechExtMemUSIP::closeShareMemory() {
#ifndef MTK_CCCI_SHARE_BUFFER_SUPPORT
    return 0;
#endif

    ALOGD("%s(), mCcciShareMemoryHandler: %d, mShareMemoryBase: %p, mShareMemoryLength: %u",
          __FUNCTION__, mCcciShareMemoryHandler, mShareMemoryBase, mShareMemoryLength);

    if (mCcciShareMemoryHandler >= 0) {

        ::close(mCcciShareMemoryHandler);
        mCcciShareMemoryHandler = -1;
        mShareMemoryBase = NULL;
        mShareMemoryLength = 0;
        mShareMemory = NULL;
    }

    return 0;
}

int SpeechExtMemUSIP::formatShareMemory() {
#ifndef MTK_CCCI_SHARE_BUFFER_SUPPORT
    return 0;
#endif

    AL_AUTOLOCK(mShareMemoryLock);

    if (mShareMemoryBase == NULL || mShareMemoryLength < sizeof(sph_usip_shm_t)) {
        ALOGE("%s(), mShareMemoryBase(%p) == NULL || mShareMemoryLength(%u) < sizeof(sph_usip_shm_t): %u",
              __FUNCTION__, mShareMemoryBase, (uint32_t)mShareMemoryLength, (uint32_t)sizeof(sph_usip_shm_t));
        return -EFAULT;
    }

    mShareMemory = (sph_usip_shm_t *)mShareMemoryBase;
    /* sph_param region */
    mSpeechParamRegion->offset = (uint8_t *)mShareMemory->sph_param - (uint8_t *)mShareMemory;
    mSpeechParamRegion->size = SPEECH_USIP_SHM_SPEECH_PARAM_SIZE;
    mSpeechParamRegion->read_idx = 0;

    /* only format share memory once after boot */
    if (get_uint32_from_mixctrl(kPropertyKeyShareMemoryInit) != 0) {
        ALOGD("%s() only format share memory once after boot", __FUNCTION__);
        mSpeechParamRegion->write_idx = get_uint32_from_mixctrl(kPropertyKeySphParamWriteIdx);
        goto FORMAT_SHARE_MEMORY_DONE;
    }
    /*32 bytes gurard region */
    sph_memset(mShareMemory->guard_region_pre, 0x0A, SPEECH_USIP_SHM_GUARD_REGION_SIZE);

    /* md_private_range_offset */
    mShareMemory->md_private_range_offset = (uint8_t *)mShareMemory->md_private_range - (uint8_t *)mShareMemory;

    /* sph_param_offset */
    mShareMemory->sph_param_offset = (uint8_t *)mShareMemory->sph_param - (uint8_t *)mShareMemory;

    /*32 bytes gurard region */
    sph_memset(mShareMemory->guard_region_post, 0x0A, SPEECH_USIP_SHM_GUARD_REGION_SIZE);

    mSpeechParamRegion->write_idx = 0;
    // save write_idx in kernel to avoid medieserver die
    set_uint32_to_mixctrl(kPropertyKeySphParamWriteIdx, 0);

    /* init done flag */
    set_uint32_to_mixctrl(kPropertyKeyShareMemoryInit, 1);


FORMAT_SHARE_MEMORY_DONE:
    ALOGD("%s(), [md_private_range] offset: %u, [sph_param] offset: %u",
          __FUNCTION__,
          mShareMemory->md_private_range_offset,
          mShareMemory->sph_param_offset);

    return 0;
}

int SpeechExtMemUSIP::resetShareMemoryIndex() {
#ifndef MTK_CCCI_SHARE_BUFFER_SUPPORT
    return 0;
#endif

    if (!mShareMemory) {
        ALOGE("%s(), mShareMemory NULL!! formatShareMemory", __FUNCTION__);
        int retval = formatShareMemory();
        if (!mShareMemory || retval != 0) {
            ALOGE("%s(), formatShareMemory Fail! mShareMemory NULL!! return", __FUNCTION__);
            return -EFAULT;
        }
    }

    AL_AUTOLOCK(mShareMemoryLock);

    /* sph_param */
    mSpeechParamRegion->read_idx = 0;
    mSpeechParamRegion->write_idx = 0;
    // save write_idx in kernel to avoid medieserver die
    set_uint32_to_mixctrl(kPropertyKeySphParamWriteIdx, 0);

    return 0;
}

int SpeechExtMemUSIP::writeSphParamToShareMemory(const void *p_sph_param,
                                                 uint32_t sph_param_length,
                                                 uint32_t *p_write_idx) {
#ifndef MTK_CCCI_SHARE_BUFFER_SUPPORT
    return -ENODEV;
#endif

    int retval = 0;
    ALOGV("%s()", __FUNCTION__);

    if (!mShareMemory) {
        ALOGE("%s(), mShareMemory NULL!! formatShareMemory", __FUNCTION__);
        int retval = formatShareMemory();
        if (!mShareMemory || retval != 0) {
            ALOGE("%s(), formatShareMemory Fail! mShareMemory NULL!! return", __FUNCTION__);
            return -EFAULT;
        }
    }

    AL_AUTOLOCK(mShareMemoryLock);

    if (!p_sph_param || !p_write_idx) {
        ALOGW("%s(), NULL!! return", __FUNCTION__);
        return -EFAULT;
    }
    if (sph_param_length == 0) {
        ALOGW("%s(), sph_param_length=%d return", __FUNCTION__, sph_param_length);
        return 0;
    }

    region_info_t *p_region = mSpeechParamRegion;

    updateWriteIndex(p_region, sph_param_length);

    /* keep the data index before write */
    *p_write_idx = p_region->write_idx;

#if defined(ENABLE_SPH_USIP_DUMP)
    uint32_t dumpWriteIdx =  p_region->write_idx / 2; //word
    int idxDump = 10, numDump = 40, skipOffset = 3072 / 2;
    char sphDumpStr[SPH_DUMP_STR_SIZE] = {0};
    snprintf(sphDumpStr, SPH_DUMP_STR_SIZE, "dump param[%p]= ", mSpeechParamRegion + p_region->write_idx);
#endif

    /* write sph param */
    shm_region_write_from_linear(p_region, p_sph_param, sph_param_length);

#if defined(ENABLE_SPH_USIP_DUMP)
    for (int idxDump = 0; idxDump < numDump; idxDump++) {
        char sphDumpTemp[100];
        snprintf(sphDumpTemp, 100, "[%d]=0x%x,", idxDump,
                 *((uint16_t *)mSpeechParamRegion + dumpWriteIdx + skipOffset + idxDump));
        audio_strncat(sphDumpStr, sphDumpTemp, SPH_DUMP_STR_SIZE);
    }
    if (idxDump != 0) {
        ALOGD("%s(), %s", __FUNCTION__, sphDumpStr);
    }
#endif

    return 0;
}


uint32_t SpeechExtMemUSIP::shm_region_data_count(region_info_t *p_region) {
    if (!p_region) {
        return 0;
    }

    if (p_region->read_idx >= p_region->size) {
        ALOGE("%s(), offset: 0x%x, size: 0x%x, read_idx : 0x%x, write_idx: 0x%x", __FUNCTION__,
              p_region->offset, p_region->size, p_region->read_idx, p_region->write_idx);
        WARNING("read idx error");
        p_region->read_idx %= p_region->size;
    } else if (p_region->write_idx >= p_region->size) {
        ALOGE("%s(), offset: 0x%x, size: 0x%x, read_idx : 0x%x, write_idx: 0x%x", __FUNCTION__,
              p_region->offset, p_region->size, p_region->read_idx, p_region->write_idx);
        WARNING("write idx error");
        p_region->write_idx %= p_region->size;
    }

    uint32_t count = 0;
    if (p_region->write_idx >= p_region->read_idx) {
        count = p_region->write_idx - p_region->read_idx;
    } else {
        count = p_region->size - (p_region->read_idx - p_region->write_idx);
    }

    return count;
}


uint32_t SpeechExtMemUSIP::shm_region_free_space(region_info_t *p_region) {
    if (!p_region) {
        return 0;
    }

    uint32_t count = p_region->size - shm_region_data_count(p_region);

    if (count >= MAX_SIZE_OF_ONE_FRAME) {
        count -= MAX_SIZE_OF_ONE_FRAME;
    } else {
        count = 0;
    }

    return count;
}

int SpeechExtMemUSIP::updateWriteIndex(region_info_t *p_region,
                                       uint32_t count) {
    if (!p_region) {
        return 0;
    }

    uint32_t leftBytes = p_region->size - p_region->write_idx;

    if (count > leftBytes) {
        p_region->write_idx = 0;
        // save write_idx in kernel to avoid medieserver die
        set_uint32_to_mixctrl(kPropertyKeySphParamWriteIdx, p_region->write_idx);
    }
    ALOGD("%s(), offset: 0x%x, size: 0x%x, read_idx : 0x%x, write_idx: 0x%x, count: 0x%x, leftBytes: 0x%x",
          __FUNCTION__, p_region->offset, p_region->size, p_region->read_idx, p_region->write_idx, count, leftBytes);
    return 0;
}

void SpeechExtMemUSIP::shm_region_write_from_linear(region_info_t *p_region,
                                                    const void *linear_buf,
                                                    uint32_t count) {
    if (!p_region || !linear_buf || !mShareMemory) {
        return;
    }

    if (p_region->read_idx >= p_region->size) {
        ALOGE("%s(), offset: 0x%x, size: 0x%x, read_idx : 0x%x, write_idx: 0x%x, count: 0x%x", __FUNCTION__,
              p_region->offset, p_region->size, p_region->read_idx, p_region->write_idx, count);
        WARNING("read idx error");
        p_region->read_idx %= p_region->size;
    } else if (p_region->write_idx >= p_region->size) {
        ALOGE("%s(), offset: 0x%x, size: 0x%x, read_idx : 0x%x, write_idx: 0x%x, count: 0x%x", __FUNCTION__,
              p_region->offset, p_region->size, p_region->read_idx, p_region->write_idx, count);
        WARNING("write idx error");
        p_region->write_idx %= p_region->size;
    }

    SPH_LOG_V("%s(+), offset: 0x%x, size: 0x%x, read_idx : 0x%x, write_idx: 0x%x, count: 0x%x", __FUNCTION__,
              p_region->offset, p_region->size, p_region->read_idx, p_region->write_idx, count);

    uint32_t free_space = shm_region_free_space(p_region);
    uint8_t *p_buf = ((uint8_t *)mShareMemory) + p_region->offset;

    ASSERT(free_space >= count);

    if (p_region->read_idx <= p_region->write_idx) {
        uint32_t w2e = p_region->size - p_region->write_idx;
        if (count <= w2e) {
            sph_memcpy(p_buf + p_region->write_idx, linear_buf, count);
            p_region->write_idx += count;
            if (p_region->write_idx == p_region->size) {
                p_region->write_idx = 0;
            }
        } else {
            sph_memcpy(p_buf + p_region->write_idx, linear_buf, w2e);
            sph_memcpy(p_buf, (uint8_t *)linear_buf + w2e, count - w2e);
            p_region->write_idx = count - w2e;
        }
    } else {
        sph_memcpy(p_buf + p_region->write_idx, linear_buf, count);
        p_region->write_idx += count;
    }
    // save write_idx in kernel to avoid medieserver die
    set_uint32_to_mixctrl(kPropertyKeySphParamWriteIdx, p_region->write_idx);

    SPH_LOG_V("%s(-), offset: 0x%x, size: 0x%x, read_idx : 0x%x, write_idx: 0x%x, count: 0x%x", __FUNCTION__,
              p_region->offset, p_region->size, p_region->read_idx, p_region->write_idx, count);
}


} // end of namespace android
