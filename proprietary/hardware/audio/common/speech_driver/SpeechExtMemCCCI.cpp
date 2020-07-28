#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "SpeechExtMemCCCI"
#include <SpeechExtMemCCCI.h>
#include <SpeechMessengerNormal.h>
#include <utils/Log.h>
#include <cutils/properties.h>

extern "C" {
#include <hardware/ccci_intf.h>
}

#include <AudioAssert.h>//Mutex/assert
#include <SpeechUtility.h>
#include <SpeechCCCIType.h>


namespace android {
/*
 * =============================================================================
 *                     typedef
 * =============================================================================
 */

#define SPEECH_CCCI_SHM_SIZE (52 * 1024)
#define SPEECH_CCCI_SHM_GUARD_REGION_SIZE (32)
#define SPEECH_CCCI_SHM_MD_DATA_SIZE ((32 * 1024) - 160) /* 160: SHM total 52K only... */
#define SPEECH_CCCI_SHM_SPEECH_PARAM_SIZE (12 * 1024)
#define SPEECH_CCCI_SHM_AP_DATA_SIZE (8  * 1024)

typedef struct region_info_t {
    uint32_t offset;        /* ex: 0x1000 */ /* count from the address of sph_shm_t */
    uint32_t size;          /* ex: 0x100 */
    uint32_t read_idx;      /* ex: 0x0 ~ 0xFF */
    uint32_t write_idx;     /* ex: 0x0 ~ 0xFF */
} region_info_t; /* 16 bytes */

typedef struct sph_shm_region_t {
    region_info_t sph_param_region;
    region_info_t ap_data_region;
    region_info_t md_data_region;
    region_info_t reserve_1;
    region_info_t reserve_2;
} sph_shm_region_t; /* 80 bytes */

typedef struct sph_shm_t {
    /* 32 bytes gurard region */
    uint8_t guard_region_pre[SPEECH_CCCI_SHM_GUARD_REGION_SIZE];

    /* 8 bytes init flag */
    uint32_t ap_flag; /* sph_shm_ap_flag_t: ap can r/w, md read only */
    uint32_t md_flag; /* sph_shm_md_flag_t: md can r/w, ap read only */

    /* 80 bytes SHM region base */
    sph_shm_region_t region; /* sph_shm_region_t */

    /* 4 bytes reserve */
    uint32_t reserve;

    /* 4 bytes strcut size check sum */
    uint32_t struct_checksum; /* assert(shm->struct_checksum == (&shm->struct_checksum  - shm)); */

    /* 12K bytes speech param */
    uint8_t sph_param[SPEECH_CCCI_SHM_SPEECH_PARAM_SIZE];

    /* 8K bytes AP data */
    uint8_t ap_data[SPEECH_CCCI_SHM_AP_DATA_SIZE];

    /* 32K bytes MD data */
    uint8_t md_data[SPEECH_CCCI_SHM_MD_DATA_SIZE];

    /* 32 bytes gurard region */
    uint8_t guard_region_post[SPEECH_CCCI_SHM_GUARD_REGION_SIZE];
} sph_shm_t;

typedef uint32_t sph_shm_ap_flag_t;
enum { /* sph_shm_ap_flag_t */
    /* AP init SHM ready or nor */
    SPH_SHM_AP_FLAG_READY               = (1 << 0),

    /* AP is writing SHM speech param */
    SPH_SHM_AP_FLAG_SPH_PARAM_WRITE     = (1 << 1),

    /* max 32 bit !! */
    SPH_SHM_AP_FLAG_MAX                 = (1 << 31)
};

typedef uint32_t sph_shm_md_flag_t;
enum { /* sph_shm_md_flag_t */
    /* modem is alive or not */
    SPH_SHM_MD_FLAG_ALIVE               = (1 << 0),

    /* MD is reading SHM speech param */
    SPH_SHM_MD_FLAG_SPH_PARAM_READ      = (1 << 1),

    /* max 32 bit !! */
    SPH_SHM_MD_FLAG_MAX                 = (1 << 31)
};

/*
 * =============================================================================
 *                     global value
 * =============================================================================
 */

static const char kPropertyKeyShareMemoryInit[PROPERTY_KEY_MAX] = "vendor.audiohal.speech.shm_init";


/*
 * =============================================================================
 *                     utility for modem
 * =============================================================================
 */
static CCCI_MD get_ccci_md_number(const modem_index_t mModemIndex) {
    CCCI_MD ccci_md = MD_SYS1;

    switch (mModemIndex) {
    case MODEM_1:
        ccci_md = MD_SYS1;
        break;
    case MODEM_2:
        ccci_md = MD_SYS2;
        break;
    default:
        ccci_md = MD_SYS1;
    }
    return ccci_md;
}

/*
 * =============================================================================
 *                     class implementation
 * =============================================================================
 */
SpeechExtMemCCCI::SpeechExtMemCCCI() {
    ALOGD("%s()", __FUNCTION__);
    mModemIndex = MODEM_1;
    mCcciShareMemoryHandler = -1;
    mShareMemoryBase = NULL;
    mShareMemoryLength = 0;
    mShareMemory = NULL;
}

SpeechExtMemCCCI::~SpeechExtMemCCCI() {
    ALOGD("%s()", __FUNCTION__);
}

int SpeechExtMemCCCI::openShareMemory(const modem_index_t modem_index) {
#ifndef MTK_CCCI_SHARE_BUFFER_SUPPORT
    return 0;
#endif
    mModemIndex = modem_index;
    struct timespec ts_start;
    struct timespec ts_stop;
    uint64_t time_diff_msg = 0;

    CCCI_MD ccci_md = get_ccci_md_number(mModemIndex);

    if (mCcciShareMemoryHandler >= 0 &&
        mShareMemoryBase != NULL &&
        mShareMemoryLength >= sizeof(sph_shm_t)) {
        ALOGD("%s(), ccci_md: %d, mCcciShareMemoryHandler: %d, mShareMemoryBase: %p, "
              "mShareMemoryLength: %u already open",
              __FUNCTION__, ccci_md, mCcciShareMemoryHandler, mShareMemoryBase, (uint32_t)mShareMemoryLength);
        return 0;
    }

    /* get time when get share momoey address start */
    audio_get_timespec_monotonic(&ts_start);
    // get share momoey address
    mCcciShareMemoryHandler = ccci_smem_get(ccci_md, USR_SMEM_RAW_AUDIO, &mShareMemoryBase, &mShareMemoryLength);
    /* get time when get share momoey address done */
    audio_get_timespec_monotonic(&ts_stop);
    time_diff_msg = get_time_diff_ms(&ts_start, &ts_stop);

    if (mCcciShareMemoryHandler < 0) {
        ALOGE("%s(), ccci_smem_get(%d) fail!! mCcciShareMemoryHandler: %d, errno: %d", __FUNCTION__,
              ccci_md, (int32_t)mCcciShareMemoryHandler, errno);
        return -ENODEV;
    }

    if (mShareMemoryBase == NULL || mShareMemoryLength == 0) {
        ALOGE("%s(), mShareMemoryBase(%p) == NULL || mShareMemoryLength(%u) == 0", __FUNCTION__,
              mShareMemoryBase, (uint32_t)mShareMemoryLength);
        closeShareMemory();
        return -EFAULT;
    }

    if (mShareMemoryLength < sizeof(sph_shm_t)) {
        ALOGE("%s(), mShareMemoryLength(%u) < sizeof(sph_shm_t): %u", __FUNCTION__,
              (uint32_t)mShareMemoryLength, (uint32_t)sizeof(sph_shm_t));
        closeShareMemory();
        return -ENOMEM;
    }
    ALOGD("%s(), ccci_md: %d, mCcciShareMemoryHandler: %d, mShareMemoryBase: %p, "
          "mShareMemoryLength: %u , get share momoey address time: %ju ms",
          __FUNCTION__, ccci_md, mCcciShareMemoryHandler, mShareMemoryBase,
          (uint32_t)mShareMemoryLength, time_diff_msg);

    return mCcciShareMemoryHandler;
}

int SpeechExtMemCCCI::closeShareMemory() {
#ifndef MTK_CCCI_SHARE_BUFFER_SUPPORT
    return 0;
#endif

    ALOGD("%s(), mCcciShareMemoryHandler: %d, mShareMemoryBase: %p, mShareMemoryLength: %u",
          __FUNCTION__, mCcciShareMemoryHandler, mShareMemoryBase, mShareMemoryLength);

    if (mCcciShareMemoryHandler >= 0) {
        ccci_smem_put(mCcciShareMemoryHandler, mShareMemoryBase, mShareMemoryLength);
        mCcciShareMemoryHandler = -1;
        mShareMemoryBase = NULL;
        mShareMemoryLength = 0;
        mShareMemory = NULL;
    }

    return 0;
}

int SpeechExtMemCCCI::formatShareMemory() {
#ifndef MTK_CCCI_SHARE_BUFFER_SUPPORT
    return 0;
#endif

    AL_AUTOLOCK(mShareMemoryLock);

    if (mShareMemoryBase == NULL || mShareMemoryLength < sizeof(sph_shm_t)) {
        ALOGE("%s(), mShareMemoryBase(%p) == NULL || mShareMemoryLength(%u) < sizeof(sph_shm_t): %u",
              __FUNCTION__, mShareMemoryBase, (uint32_t)mShareMemoryLength, (uint32_t)sizeof(sph_shm_t));
        WARNING("EFAULT");
        return -EFAULT;
    }

    mShareMemory = (sph_shm_t *)mShareMemoryBase;

    /* only format share memory once after boot */
    if (get_uint32_from_mixctrl(kPropertyKeyShareMemoryInit) != 0) {
        ALOGD("%s() only format share memory once after boot", __FUNCTION__);
        goto FORMAT_SHARE_MEMORY_DONE;
    }


    /* 32 bytes gurard region */
    sph_memset(mShareMemory->guard_region_pre, 0x0A, SPEECH_CCCI_SHM_GUARD_REGION_SIZE);

    /* ap_flag */
    mShareMemory->ap_flag = 0;

    /* md_flag */
    mShareMemory->md_flag = 0;

    /* sph_param region */
    mShareMemory->region.sph_param_region.offset = (uint8_t *)mShareMemory->sph_param - (uint8_t *)mShareMemory;
    mShareMemory->region.sph_param_region.size = SPEECH_CCCI_SHM_SPEECH_PARAM_SIZE;
    mShareMemory->region.sph_param_region.read_idx = 0;
    mShareMemory->region.sph_param_region.write_idx = 0;
    /* ap_data region */
    mShareMemory->region.ap_data_region.offset = (uint8_t *)mShareMemory->ap_data - (uint8_t *)mShareMemory;
    mShareMemory->region.ap_data_region.size = SPEECH_CCCI_SHM_AP_DATA_SIZE;
    mShareMemory->region.ap_data_region.read_idx = 0;
    mShareMemory->region.ap_data_region.write_idx = 0;

    /* md_data region */
    mShareMemory->region.md_data_region.offset = (uint8_t *)mShareMemory->md_data - (uint8_t *)mShareMemory;
    mShareMemory->region.md_data_region.size = SPEECH_CCCI_SHM_MD_DATA_SIZE;
    mShareMemory->region.md_data_region.read_idx = 0;
    mShareMemory->region.md_data_region.write_idx = 0;

    /* reserve_1 region */
    mShareMemory->region.reserve_1.offset = 0;
    mShareMemory->region.reserve_1.size = 0;
    mShareMemory->region.reserve_1.read_idx = 0;
    mShareMemory->region.reserve_1.write_idx = 0;

    /* reserve_2 region */
    mShareMemory->region.reserve_2.offset = 0;
    mShareMemory->region.reserve_2.size = 0;
    mShareMemory->region.reserve_2.read_idx = 0;
    mShareMemory->region.reserve_2.write_idx = 0;

    /* reserve */
    mShareMemory->reserve = 0;

    /* checksum */
    mShareMemory->struct_checksum = (uint8_t *)(&mShareMemory->struct_checksum) - (uint8_t *)mShareMemory;

    /* sph_param */
    sph_memset(mShareMemory->sph_param, 0, SPEECH_CCCI_SHM_SPEECH_PARAM_SIZE);

    /* ap_data */
    sph_memset(mShareMemory->ap_data, 0, SPEECH_CCCI_SHM_AP_DATA_SIZE);

    /* md_data */
    sph_memset(mShareMemory->md_data, 0, SPEECH_CCCI_SHM_MD_DATA_SIZE);

    /* 32 bytes gurard region */
    sph_memset(mShareMemory->guard_region_post, 0x0A, SPEECH_CCCI_SHM_GUARD_REGION_SIZE);


    /* share memory init ready */
    mShareMemory->ap_flag |= SPH_SHM_AP_FLAG_READY;

    /* init done flag */
    set_uint32_to_mixctrl(kPropertyKeyShareMemoryInit, 1);



FORMAT_SHARE_MEMORY_DONE:
    ALOGD("%s(), mShareMemory: %p, ap_flag: 0x%x, md_flag: 0x%x, struct_checksum: %u",
          __FUNCTION__,
          mShareMemory,
          mShareMemory->ap_flag,
          mShareMemory->md_flag,
          mShareMemory->struct_checksum);

    ALOGD("%s(), [sph_param] offset: %u, size: %u, [ap_data] offset: %u, size: %u, [md_data] offset: %u, size: %u",
          __FUNCTION__,
          mShareMemory->region.sph_param_region.offset,
          mShareMemory->region.sph_param_region.size,
          mShareMemory->region.ap_data_region.offset,
          mShareMemory->region.ap_data_region.size,
          mShareMemory->region.md_data_region.offset,
          mShareMemory->region.md_data_region.size);

    ALOGD("%s(), [sph_param] read_idx: %u, write_idx: %u, [ap_data] read_idx: %u, write_idx: %u, "
          "[md_data] read_idx: %u, write_idx: %u",
          __FUNCTION__,
          mShareMemory->region.sph_param_region.read_idx,
          mShareMemory->region.sph_param_region.write_idx,
          mShareMemory->region.ap_data_region.read_idx,
          mShareMemory->region.ap_data_region.write_idx,
          mShareMemory->region.md_data_region.read_idx,
          mShareMemory->region.md_data_region.write_idx);

    return 0;
}

int SpeechExtMemCCCI::resetShareMemoryIndex() {
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

    /* enable ap write flag */
    mShareMemory->ap_flag |= SPH_SHM_AP_FLAG_SPH_PARAM_WRITE;

    if (mShareMemory->md_flag & SPH_SHM_MD_FLAG_SPH_PARAM_READ) {
        ALOGE("%s(), modem still read!! md_flag: 0x%x", __FUNCTION__,
              mShareMemory->md_flag);
        WARNING("md_flag error!!");
        mShareMemory->ap_flag &= (~SPH_SHM_AP_FLAG_SPH_PARAM_WRITE);
        return -EBUSY;
    }


    /* sph_param */
    mShareMemory->region.sph_param_region.read_idx = 0;
    mShareMemory->region.sph_param_region.write_idx = 0;

    /* ap data */
    mShareMemory->region.ap_data_region.read_idx = 0;
    mShareMemory->region.ap_data_region.write_idx = 0;

    /* md data */
    mShareMemory->region.md_data_region.read_idx = 0;
    mShareMemory->region.md_data_region.write_idx = 0;

    /* disable ap write flag */
    mShareMemory->ap_flag &= (~SPH_SHM_AP_FLAG_SPH_PARAM_WRITE);

    return 0;
}

int SpeechExtMemCCCI::writeSphParamToShareMemory(const void *p_sph_param,
                                                 uint32_t sph_param_length,
                                                 uint32_t *p_write_idx) {
#ifndef MTK_CCCI_SHARE_BUFFER_SUPPORT
    return -ENODEV;
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

    if (!p_sph_param) {
        ALOGE("%s(), p_sph_param NULL!! return", __FUNCTION__);
        return -EFAULT;
    }

    if (!p_write_idx) {
        ALOGE("%s(), p_write_idx NULL!! return", __FUNCTION__);
        return -EFAULT;
    }

    mShareMemory->ap_flag |= SPH_SHM_AP_FLAG_SPH_PARAM_WRITE;

    if (mShareMemory->md_flag & SPH_SHM_MD_FLAG_SPH_PARAM_READ) {
        ALOGW("%s(), modem still read!! md_flag: 0x%x", __FUNCTION__,
              mShareMemory->md_flag);
        mShareMemory->ap_flag &= (~SPH_SHM_AP_FLAG_SPH_PARAM_WRITE);
        return -EBUSY;
    }

    region_info_t *p_region = &mShareMemory->region.sph_param_region;
    uint16_t free_space = (uint16_t)shm_region_free_space(p_region);

    if (sph_param_length > free_space) {
        ALOGW("%s(), sph_param_length %u > free_space %u!!", __FUNCTION__,
              sph_param_length, free_space);
        return -ENOMEM;
    }

    /* keep the data index before write */
    *p_write_idx = (uint16_t)p_region->write_idx;

    /* write sph param */
    shm_region_write_from_linear(p_region, p_sph_param, sph_param_length);

    mShareMemory->ap_flag &= (~SPH_SHM_AP_FLAG_SPH_PARAM_WRITE);
    return 0;
}


int SpeechExtMemCCCI::writeApDataToShareMemory(const void *p_data_buf,
                                               uint16_t data_type,
                                               uint16_t data_size,
                                               uint16_t *p_payload_length,
                                               uint32_t *p_write_idx) {
#ifndef MTK_CCCI_SHARE_BUFFER_SUPPORT
    return -ENODEV;
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

    if (!p_data_buf) {
        ALOGE("%s(), p_data_buf NULL!! return", __FUNCTION__);
        return -EFAULT;
    }
    if (!p_payload_length) {
        ALOGE("%s(), p_payload_length NULL!! return", __FUNCTION__);
        return -EFAULT;
    }
    if (!p_write_idx) {
        ALOGE("%s(), p_write_idx NULL!! return", __FUNCTION__);
        return -EFAULT;
    }

    region_info_t *p_region = &mShareMemory->region.ap_data_region;

    uint16_t payload_length = CCCI_MAX_AP_PAYLOAD_HEADER_SIZE + data_size;
    uint16_t free_space = (uint16_t)shm_region_free_space(p_region);

    if (payload_length > free_space) {
        ALOGW("%s(), payload_length %u > free_space %u!!", __FUNCTION__,
              payload_length, free_space);
        *p_payload_length = 0;
        return -ENOMEM;
    }

    /* keep the data index before write */
    *p_write_idx = p_region->write_idx;

    /* write header */
    uint16_t header[3] = {0};
    header[0] = CCCI_AP_PAYLOAD_SYNC;
    header[1] = data_type;
    header[2] = data_size;
    shm_region_write_from_linear(p_region, header, CCCI_MAX_AP_PAYLOAD_HEADER_SIZE);

    /* write data */
    shm_region_write_from_linear(p_region, p_data_buf, data_size);

    *p_payload_length = payload_length;
    return 0;
}


int SpeechExtMemCCCI::readMdDataFromShareMemory(void *p_data_buf,
                                                uint16_t *p_data_type,
                                                uint16_t *p_data_size,
                                                uint16_t payload_length,
                                                uint32_t read_idx) {
#ifndef MTK_CCCI_SHARE_BUFFER_SUPPORT
    return -ENODEV;
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

    if (!p_data_buf) {
        ALOGE("%s(), p_data_buf NULL!! return", __FUNCTION__);
        return -EFAULT;
    }
    if (!p_data_type) {
        ALOGE("%s(), p_data_type NULL!! return", __FUNCTION__);
        return -EFAULT;
    }
    if (!p_data_size) {
        ALOGE("%s(), p_data_size NULL!! return", __FUNCTION__);
        return -EFAULT;
    }

    region_info_t *p_region = &mShareMemory->region.md_data_region;

    uint16_t data_size = payload_length - CCCI_MAX_MD_PAYLOAD_HEADER_SIZE;
    uint32_t available_count = shm_region_data_count(p_region);

    if (data_size > *p_data_size) {
        ALOGW("%s(), data_size %u > p_data_buf size %u!!", __FUNCTION__,
              data_size, *p_data_size);
        *p_data_size = 0;
        WARNING("-ENOMEM");
        return -ENOMEM;
    }

    if (payload_length > available_count) {
        ALOGW("%s(), payload_length %u > available_count %u!!", __FUNCTION__,
              payload_length, available_count);
        *p_data_size = 0;
        return -ENOMEM;
    }

    /* check read index */
    if (read_idx != p_region->read_idx) {
        ALOGW("%s(), read_idx 0x%x != p_region->read_idx 0x%x!!", __FUNCTION__,
              read_idx, p_region->read_idx);
        WARNING("bad read_idx!!");
        ALOGE("%s(), [sph_param] read_idx: %u, write_idx: %u, [ap_data] read_idx: %u, write_idx: %u, "
              "[md_data] read_idx: %u, write_idx: %u",
              __FUNCTION__,
              mShareMemory->region.sph_param_region.read_idx,
              mShareMemory->region.sph_param_region.write_idx,
              mShareMemory->region.ap_data_region.read_idx,
              mShareMemory->region.ap_data_region.write_idx,
              mShareMemory->region.md_data_region.read_idx,
              mShareMemory->region.md_data_region.write_idx);

        p_region->read_idx = read_idx;
    }

    /* read header */
    uint16_t header[5] = {0};
    shm_region_read_to_linear(header, p_region, CCCI_MAX_MD_PAYLOAD_HEADER_SIZE);

    if (header[0] != CCCI_MD_PAYLOAD_SYNC ||
        header[2] != data_size ||
        header[3] != header[4]) {
        ALOGE("%s(), sync: 0x%x, type: %d, size: 0x%x, idx: %d, total_idx: %d",
              __FUNCTION__, header[0], header[1], header[2], header[3], header[4]);
        WARNING("md data header error");
        *p_data_size = 0;
        return -EINVAL;
    }

    *p_data_type = header[1];


    /* read data */
    shm_region_read_to_linear(p_data_buf, p_region, data_size);

    *p_data_size = data_size;
    return 0;

}

uint32_t SpeechExtMemCCCI::shm_region_data_count(region_info_t *p_region) {
    if (!p_region) {
        ALOGE("%s(), p_region NULL!! return 0", __FUNCTION__);
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


uint32_t SpeechExtMemCCCI::shm_region_free_space(region_info_t *p_region) {
    if (!p_region) {
        ALOGE("%s(), p_region NULL!! return 0", __FUNCTION__);
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


void SpeechExtMemCCCI::shm_region_write_from_linear(region_info_t *p_region,
                                                    const void *linear_buf,
                                                    uint32_t count) {
    if (!p_region) {
        ALOGE("%s(), p_region NULL!! return", __FUNCTION__);
        return;
    }
    if (!linear_buf) {
        ALOGE("%s(), linear_buf NULL!! return", __FUNCTION__);
        return;
    }
    if (!mShareMemory) {
        ALOGE("%s(), mShareMemory NULL!! return", __FUNCTION__);
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

    SPH_LOG_V("%s(-), offset: 0x%x, size: 0x%x, read_idx : 0x%x, write_idx: 0x%x, count: 0x%x", __FUNCTION__,
              p_region->offset, p_region->size, p_region->read_idx, p_region->write_idx, count);
}




void SpeechExtMemCCCI::shm_region_read_to_linear(void *linear_buf,
                                                 region_info_t *p_region,
                                                 uint32_t count) {
    if (!p_region) {
        ALOGE("%s(), p_region NULL!! return", __FUNCTION__);
        return;
    }
    if (!linear_buf) {
        ALOGE("%s(), linear_buf NULL!! return", __FUNCTION__);
        return;
    }
    if (!mShareMemory) {
        ALOGE("%s(), mShareMemory NULL!! return", __FUNCTION__);
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

    uint32_t available_count = shm_region_data_count(p_region);
    uint8_t *p_buf = ((uint8_t *)mShareMemory) + p_region->offset;

    ASSERT(count <= available_count);

    if (p_region->read_idx <= p_region->write_idx) {
        sph_memcpy(linear_buf, p_buf + p_region->read_idx, count);
        p_region->read_idx += count;
    } else {
        uint32_t r2e = p_region->size - p_region->read_idx;
        if (r2e >= count) {
            sph_memcpy(linear_buf, p_buf + p_region->read_idx, count);
            p_region->read_idx += count;
            if (p_region->read_idx == p_region->size) {
                p_region->read_idx = 0;
            }
        } else {
            sph_memcpy(linear_buf, p_buf + p_region->read_idx, r2e);
            sph_memcpy((uint8_t *)linear_buf + r2e, p_buf, count - r2e);
            p_region->read_idx = count - r2e;
        }
    }

    SPH_LOG_V("%s(-), offset: 0x%x, size: 0x%x, read_idx : 0x%x, write_idx: 0x%x, count: 0x%x", __FUNCTION__,
              p_region->offset, p_region->size, p_region->read_idx, p_region->write_idx, count);
}

bool SpeechExtMemCCCI::checkModemAlive() {
    if (!mShareMemory) {
        ALOGW("%s(), mShareMemory NULL!! return false", __FUNCTION__);
        return false;
    }

    return ((mShareMemory->md_flag & SPH_SHM_MD_FLAG_ALIVE) > 0);
}

} // end of namespace android
