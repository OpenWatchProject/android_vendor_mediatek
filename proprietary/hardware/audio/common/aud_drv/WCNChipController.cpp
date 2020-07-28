#include "WCNChipController.h"

#include <sys/ioctl.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <utils/threads.h>

#include <linux/fm.h>

#include <dlfcn.h>

#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "WCNChipController"

//#define MTK_BASIC_PACKAGE//Eenable For WCN test

/*
 * following declaration must sync with
 * vendor\mediatek\proprietary\hardware\connectivity\bluetooth\driver\pure\inc\bt_drv.h
 */

typedef enum {
    /*  [operation]
     *      Return audio configuration for BT SCO on current chipset
     *  [input param]
     *      A pointer to union type with content of BT_INFO.
     *      Typecasting conversion: (BT_INFO *) param.
     *  [return]
     *      0 - default, don't care.
     *  [callback]
     *      None.
     */
    BT_MTK_OP_AUDIO_GET_CONFIG,
    /* Audio config related information */
} bt_mtk_opcode_t;

typedef enum {
    PCM = 0,          // PCM 4 pins interface
    I2S,              // I2S interface
    MERGE_INTERFACE,  // PCM & I2S merge interface
    CVSD_REMOVAL      // SOC consys
} AUDIO_IF;

typedef enum {
    SYNC_8K = 0,
    SYNC_16K
} SYNC_CLK;        // DAIBT sample rate

typedef enum {
    SHORT_FRAME = 0,
    LONG_FRAME
} SYNC_FORMAT;     // DAIBT sync

typedef struct {
    AUDIO_IF           hw_if;
    SYNC_CLK           sample_rate;
    SYNC_FORMAT        sync_format;
    unsigned int       bit_len;  // bit-length of sync frame in long frame sync
} AUDIO_CONFIG;

/* Information carring for all OPs (In/Out) */
typedef union {
    AUDIO_CONFIG       audio_conf;
} BT_INFO;

namespace android {

/*==============================================================================
 *                     Property keys
 *============================================================================*/

/*==============================================================================
 *                     Const Value
 *============================================================================*/

static const uint32_t kMaxFMChipVolume = 15;

static const char     kFmAudPathName[4][16]       = {"FM_AUD_ANALOG", "FM_AUD_I2S", "FM_AUD_MRGIF", "FM_AUD_ERR"};
static const char     kFmI2sPadName[3][16]        = {"FM_I2S_PAD_CONN", "FM_I2S_PAD_IO", "FM_I2S_PAD_ERR"};
static const char     kFmI2sModeName[3][16]       = {"FM_I2S_MASTER", "FM_I2S_SLAVE", "FM_I2S_MODE_ERR"};
static const char     kFmI2sSampleRateName[4][16] = {"FM_I2S_32K", "FM_I2S_44K", "FM_I2S_48K", "FM_I2S_SR_ERR"};
static const uint32_t kFmI2sSampleRateMapNum[4]   = {32000, 44100, 48000, 44100}; // FM_I2S_SR_ERR => use default 44100Hz

/*==============================================================================
 *                     Enumerator
 *============================================================================*/

/*==============================================================================
 *                     Singleton Pattern
 *============================================================================*/

WCNChipController *WCNChipController::mWCNChipController = NULL;
AUDIO_CONFIG  mBTAudioInfo;


WCNChipController *WCNChipController::GetInstance() {
    static Mutex mGetInstanceLock;
    Mutex::Autolock _l(mGetInstanceLock);

    if (mWCNChipController == NULL) {
        mWCNChipController = new WCNChipController();
    }
    ASSERT(mWCNChipController != NULL);
    return mWCNChipController;
}

/*==============================================================================
 *                     Constructor / Destructor / Init / Deinit
 *============================================================================*/

WCNChipController::WCNChipController() {
    ALOGD("%s()", __FUNCTION__);

    mInitAudioFMInfoFlag = false;
    mInitAudioBTInfoFlag = false;
    mPlatformSupportFM = IsCustomizedPlatformSupportFM();

    // Default config error value
    mFmAudioInfo.aud_path = FM_AUD_ERR;
    mFmAudioInfo.i2s_info.status = FM_I2S_STATE_ERR;
    mFmAudioInfo.i2s_info.mode = FM_I2S_MODE_ERR;
    mFmAudioInfo.i2s_info.rate = FM_I2S_SR_ERR;
    mFmAudioInfo.i2s_pad = FM_I2S_PAD_ERR;

    mBTAudioInfo.hw_if = MERGE_INTERFACE;
    mBTAudioInfo.sample_rate = SYNC_8K;
    mBTAudioInfo.sync_format = SHORT_FRAME;
    mBTAudioInfo.bit_len = 0;
    //mBTAudioInfo.security_hi_lo = 0;

    mBTCurrentSamplingRateNumber = 8000;
}

WCNChipController::~WCNChipController() {
    ALOGD("%s()", __FUNCTION__);
}

/*==============================================================================
 *                     WCN FM Chip Control
 *============================================================================*/

bool WCNChipController::GetFmChipPowerInfo() {
    static const size_t BUF_LEN = 1;

    char rbuf[BUF_LEN] = {'\0'};
    char wbuf[BUF_LEN] = {'1'};
    const char *FM_POWER_STAUTS_PATH = "/proc/fm";

    ALOGD("+%s()", __FUNCTION__);
    if (!mPlatformSupportFM) {
        return false;
    }

    int fd = open(FM_POWER_STAUTS_PATH, O_RDONLY, 0);
    if (fd < 0) {
        ALOGE("-%s(), open(%s) fail!! fd = %d", __FUNCTION__, FM_POWER_STAUTS_PATH, fd);
        return false;
    }

    int ret = read(fd, rbuf, BUF_LEN);
    if (ret != BUF_LEN) {
        ALOGE("-%s(), read(%s) fail!! ret = %d", __FUNCTION__, FM_POWER_STAUTS_PATH, ret);
        close(fd);
        return false;
    }
    close(fd);

    const bool fm_power_on = (strncmp(wbuf, rbuf, BUF_LEN) == 0) ? true : false;

    ALOGD("-%s(), fm_power_on = %d", __FUNCTION__, fm_power_on);
    return fm_power_on;
}

status_t WCNChipController::SetFmChipVolume(const uint32_t fm_chip_volume) {
    ALOGD("+%s(), fm_chip_volume = %u", __FUNCTION__, fm_chip_volume);

    if (!mPlatformSupportFM) {
        return INVALID_OPERATION;
    }

    WARNING("No need to set FM Chip Volume in Audio Driver");

    ASSERT(0 <= fm_chip_volume && fm_chip_volume <= kMaxFMChipVolume);

    ASSERT(GetFmChipPowerInfo() == true);

    int fd_fm = open(FM_DEVICE_NAME, O_RDWR);
    ALOGD("%s(), open(%s), fd_fm = %d", __FUNCTION__, FM_DEVICE_NAME, fd_fm);

    if (fd_fm >= 0) {
        int ret = ::ioctl(fd_fm, FM_IOCTL_SETVOL, &fm_chip_volume);
        ALOGD("%s(), ioctl: FM_IOCTL_SETVOL, ret = %d", __FUNCTION__, ret);

        close(fd_fm);
    }

    ALOGD("-%s(), fm_chip_volume = %u", __FUNCTION__, fm_chip_volume);
    return NO_ERROR;
}


status_t WCNChipController::SetFmChipSampleRate(const uint32_t sample_rate) {
    ALOGD("+%s(), sample_rate = %u", __FUNCTION__, sample_rate);

    if (!mPlatformSupportFM) {
        return INVALID_OPERATION;
    }
    //ASSERT(GetFmChipPowerInfo() == true);

    if (mInitAudioFMInfoFlag == false) { InitAudioFMInfo(); }
    ASSERT(mFmAudioInfo.i2s_info.mode != FM_I2S_MODE_ERR);

    struct fm_i2s_setting fmSetting;
    fmSetting.onoff  = FM_I2S_ON;
    fmSetting.mode   = mFmAudioInfo.i2s_info.mode;
    if (sample_rate == 32000) {
        fmSetting.sample = FM_I2S_32K;
    } else if (sample_rate == 44100) {
        fmSetting.sample = FM_I2S_44K;
    } else if (sample_rate == 48000) {
        fmSetting.sample = FM_I2S_48K;
    } else {
        ALOGW("%s(), no such sample_rate = %u, return!!", __FUNCTION__, sample_rate);
        return BAD_VALUE;
    }


    int fd_fm = open(FM_DEVICE_NAME, O_RDWR);
    ALOGD("%s(), open(%s), fd_fm = %d", __FUNCTION__, FM_DEVICE_NAME, fd_fm);

    if (fd_fm >= 0) {
        int ret = ::ioctl(fd_fm, FM_IOCTL_I2S_SETTING, &fmSetting);
        ALOGD("%s(), ioctl: FM_IOCTL_I2S_SETTING, ret = %d", __FUNCTION__, ret);

        close(fd_fm);
    }

    ALOGD("-%s(), sample_rate = %u", __FUNCTION__, sample_rate);
    return NO_ERROR;
}


status_t WCNChipController::InitAudioFMInfo() {
    Mutex::Autolock _l(mLock);

    if (!mPlatformSupportFM) {
        return INVALID_OPERATION;
    }

    if (mInitAudioFMInfoFlag == true) {
        ALOGD("%s(), mInitAudioFMInfoFlag == true, return", __FUNCTION__);
        return NO_ERROR;
    }
    mInitAudioFMInfoFlag = true;

#if defined(MTK_FM_SUPPORT)
    // Get audio fm related info from fm driver
    int fd_fm = 0;
    const int kMaxTryCnt = 30; // max wait 3 sec
    for (int trycnt = 1; trycnt <= kMaxTryCnt; trycnt++) {
        ALOGD("%s(), +open(%s), fd_fm = %d", __FUNCTION__, FM_DEVICE_NAME, fd_fm);
        fd_fm = open(FM_DEVICE_NAME, O_RDWR);
        ALOGD("%s(), -open(%s), fd_fm = %d", __FUNCTION__, FM_DEVICE_NAME, fd_fm);
        if (fd_fm < 0) {
            ALOGE("%s(), open(%s) failed #%d times!! sleep 100 ms & try it again", __FUNCTION__, FM_DEVICE_NAME, trycnt);
            usleep(100 * 1000);
        } else {
            int ret = ::ioctl(fd_fm, FM_IOCTL_GET_AUDIO_INFO, &mFmAudioInfo);
            ALOGD("%s(), ioctl: FM_IOCTL_GET_AUDIO_INFO, ret = %d", __FUNCTION__, ret);

            // Not support analog line in, check here
            ASSERT(mFmAudioInfo.aud_path != FM_AUD_ANALOG);

            close(fd_fm);
            break;
        }
    }
#endif

    return NO_ERROR;
}

#if defined(__LP64__)
#define AUDIO_BT_LIB_VENDOR_PATH "/vendor/lib64/libbluetooth_mtk_pure.so"
#define AUDIO_BT_LIB_PATH "/system/lib64/libbluetooth_mtk_pure.so"
#else
#define AUDIO_BT_LIB_VENDOR_PATH "/vendor/lib/libbluetooth_mtk_pure.so"
#define AUDIO_BT_LIB_PATH "/system/lib/libbluetooth_mtk_pure.so"
#endif

status_t WCNChipController::InitAudioBTInfo() {
    Mutex::Autolock _l(mLock);

    if (mInitAudioBTInfoFlag == true) {
        ALOGD("%s(), mInitAudioBTInfoFlag == true, return", __FUNCTION__);
        return NO_ERROR;
    }
    mInitAudioBTInfoFlag = true;

    BT_INFO result;//BT_RESULT result;
    int ret;

    void *libHandle = NULL;
    int (*mtk_bt_op)(bt_mtk_opcode_t opcode, void *param);
    ALOGD("%s(), dlopen %s", __FUNCTION__, AUDIO_BT_LIB_VENDOR_PATH);
    // load bt lib
    libHandle = dlopen(AUDIO_BT_LIB_VENDOR_PATH, RTLD_NOW);
    if (!libHandle) {
        ALOGW("%s(), dlopen %s failed, dlerror = %s, try %s",
              __FUNCTION__, AUDIO_BT_LIB_VENDOR_PATH, dlerror(), AUDIO_BT_LIB_PATH);

        libHandle = dlopen(AUDIO_BT_LIB_PATH, RTLD_NOW);
        if (!libHandle) {
            ALOGW("%s(), dlopen %s failed, dlerror = %s",
                  __FUNCTION__, AUDIO_BT_LIB_PATH, dlerror());
        }
    }

    if (libHandle) {
        ALOGD("%s(), mtk_bt_op dlsym", __FUNCTION__);
        mtk_bt_op = (int (*)(bt_mtk_opcode_t, void *))dlsym(libHandle, "mtk_bt_op");
        if (!mtk_bt_op) {
            ALOGW("%s(), mtk_bt_op dlsym failed, dlerror = %s", __FUNCTION__, dlerror());
        }
    }

    if (libHandle && mtk_bt_op) {
        // get bt info
        for (int trycnt = 1; trycnt <= 10; trycnt++) {
            ALOGD("%s(), mtk_bt_op dlsym BT_MTK_OP_AUDIO_GET_CONFIG, trycnt=%d", __FUNCTION__, trycnt);
            ret = mtk_bt_op(BT_MTK_OP_AUDIO_GET_CONFIG, &result);
            //result->param.audio_config.hw_if
            ALOGD("%s(), query BT info status = %d", __FUNCTION__, ret);
            if (ret < 0) { //if (result.status == false)
                ALOGE("%s(), query BT info fail!! sleep 100 ms & try it again", __FUNCTION__);
                usleep(100 * 1000);
            } else {
                //mBTAudioInfo.hw_if = result.param.audio_conf.hw_if;
                //mBTAudioInfo.sample_rate = result.param.audio_conf.sample_rate;
                //mBTAudioInfo.sync_format = result.param.audio_conf.sync_format;
                //mBTAudioInfo.bit_len = result.param.audio_conf.bit_len;
                mBTAudioInfo.hw_if = result.audio_conf.hw_if;
                mBTAudioInfo.sample_rate = result.audio_conf.sample_rate;
                mBTAudioInfo.sync_format = result.audio_conf.sync_format;
                mBTAudioInfo.bit_len = result.audio_conf.bit_len;
                break;
            }
        }
    } else {
#ifdef SW_BTCVSD_ENABLE
        mBTAudioInfo.hw_if = CVSD_REMOVAL;
#else
        mBTAudioInfo.hw_if = MERGE_INTERFACE;
#endif
        mBTAudioInfo.sample_rate = SYNC_8K;
        mBTAudioInfo.sync_format = SHORT_FRAME;
        mBTAudioInfo.bit_len = 0;
    }

    // close lib
    if (libHandle) {
        if (dlclose(libHandle)) {
            ALOGE("%s(), dlclose failed, dlerror = %s", __FUNCTION__, dlerror());
        }
    }

    ALOGD("%s(), hw_if=%d, sample_rate=%d, sync_format=%d, bit_len=%d", __FUNCTION__,
          mBTAudioInfo.hw_if, mBTAudioInfo.sample_rate, mBTAudioInfo.sync_format, mBTAudioInfo.bit_len);

    return NO_ERROR;
}

bool WCNChipController::IsFMMergeInterfaceSupported() {
    if (!mPlatformSupportFM) {
        return false;
    }

    if (mInitAudioFMInfoFlag == false) { InitAudioFMInfo(); }

#if defined(MTK_FM_SUPPORT)
    ALOGV("%s(), mFmAudioInfo.aud_path = %s", __FUNCTION__, kFmAudPathName[mFmAudioInfo.aud_path]);
    ASSERT(mFmAudioInfo.aud_path != FM_AUD_ERR);
    return (mFmAudioInfo.aud_path == FM_AUD_MRGIF) ? true : false;
#else // no FM
    return false;
#endif
}

bool WCNChipController::IsBTMergeInterfaceSupported() {
    ALOGD("%s(), mPlatformSupportFM=%d, mInitAudioBTInfoFlag =%d",
          __FUNCTION__, mPlatformSupportFM, mInitAudioBTInfoFlag);

    if (!mPlatformSupportFM) {
        return false;
    }

    if (mInitAudioBTInfoFlag == false) { InitAudioBTInfo(); }

#if defined(MTK_BT_SUPPORT)
    ALOGD("%s(), BTChipHWInterface() = %d", __FUNCTION__, BTChipHWInterface());
    return (BTChipHWInterface() == MERGE_INTERFACE) ? true : false;
#else // no BT
    return false;
#endif
}


bool WCNChipController::IsFmChipPadSelConnSys() {
    if (!mPlatformSupportFM) {
        return false;
    }

#if defined(MTK_FM_SUPPORT)
    if (mInitAudioFMInfoFlag == false) { InitAudioFMInfo(); }

    ALOGD("%s(), mFmAudioInfo.i2s_pad = %s", __FUNCTION__, kFmI2sPadName[mFmAudioInfo.i2s_pad]);
    ASSERT(mFmAudioInfo.i2s_pad != FM_I2S_PAD_ERR);

    return (mFmAudioInfo.i2s_pad == FM_I2S_PAD_CONN) ? true : false;
#else
    return false; // default FM signal goes through IO_MUX
#endif
}

bool WCNChipController::IsFmChipUseSlaveMode() {
    if (!mPlatformSupportFM) {
        return false;
    }

#if defined(MTK_FM_SUPPORT)
    if (mInitAudioFMInfoFlag == false) { InitAudioFMInfo(); }

    ALOGD("%s(), mFmAudioInfo.i2s_info.mode = %s", __FUNCTION__, kFmI2sModeName[mFmAudioInfo.i2s_info.mode]);
    ASSERT(mFmAudioInfo.i2s_info.mode != FM_I2S_MODE_ERR);

    return (mFmAudioInfo.i2s_info.mode == FM_I2S_SLAVE) ? true : false;
#else
    return true; // default FM I2S slave mode, Audio I2S master mode
#endif
}

uint32_t WCNChipController::GetFmChipSamplingRate() {
    if (!mPlatformSupportFM) {
        return 44100;
    }

#if defined(MTK_FM_SUPPORT)
    if (mInitAudioFMInfoFlag == false) { InitAudioFMInfo(); }

    ALOGD("%s(), mFmAudioInfo.i2s_info.rate = %s, return %d", __FUNCTION__,
          kFmI2sSampleRateName[mFmAudioInfo.i2s_info.rate],
          kFmI2sSampleRateMapNum[mFmAudioInfo.i2s_info.rate]);
    ASSERT(mFmAudioInfo.i2s_info.rate != FM_I2S_SR_ERR);

    return kFmI2sSampleRateMapNum[mFmAudioInfo.i2s_info.rate];
#else
    return 44100; // default setting FM chip sampling rate 44100 Hz
#endif
}

uint32_t WCNChipController::BTChipHWInterface() {
    if (mInitAudioBTInfoFlag == false) { InitAudioBTInfo(); }

#if defined(MTK_BT_SUPPORT)
    return mBTAudioInfo.hw_if;
#else
#if defined(SW_BTCVSD_ENABLE)
    return CVSD_REMOVAL; // 0: PCM, 1: I2S, 2: MERGE_INTERFACE, 3: CVSD_REMOVAL
#else
    return MERGE_INTERFACE; // 0: PCM, 1: I2S, 2: MERGE_INTERFACE, 3: CVSD_REMOVAL
#endif

#endif
}

bool WCNChipController::BTUseCVSDRemoval() {
    if (mInitAudioBTInfoFlag == false) { InitAudioBTInfo(); }

    return (BTChipHWInterface() == CVSD_REMOVAL) ? true : false;
}

uint32_t WCNChipController::BTChipSamplingRate() {
#if defined(MTK_BT_SUPPORT)
    if (mInitAudioBTInfoFlag == false) { InitAudioBTInfo(); }
    return mBTAudioInfo.sample_rate; //0:SYNC_8K, 1: SYNC_16K
#else
    return SYNC_8K; // default setting is NB 8k
#endif
}

uint32_t WCNChipController::BTChipSamplingRateNumber() {
    return (BTChipSamplingRate() == SYNC_8K) ? 8000 : 16000;
}

uint32_t WCNChipController::BTChipSyncFormat() {
#if defined(MTK_BT_SUPPORT)
    if (mInitAudioBTInfoFlag == false) { InitAudioBTInfo(); }
    return mBTAudioInfo.sync_format; //0:SHORT_FRAME, 1: LONG_FRAME
#else
    return 0; // default setting is SHORT_FRAME
#endif
}

uint32_t WCNChipController::BTChipSyncLength() {
#if defined(MTK_BT_SUPPORT)
    if (mInitAudioBTInfoFlag == false) { InitAudioBTInfo(); }
    return mBTAudioInfo.bit_len;
#else
    return 0; // default setting is 0
#endif
}

uint32_t WCNChipController::BTChipSecurityHiLo() {
    return 0; // WCN does not provide this property
}


void WCNChipController::SetBTCurrentSamplingRateNumber(const uint32_t sample_rate) {
    ALOGD("%s(), mBTCurrentSamplingRateNumber: %d => %d", __FUNCTION__, mBTCurrentSamplingRateNumber, sample_rate);
    ASSERT(sample_rate == 8000 || sample_rate == 16000);
    mBTCurrentSamplingRateNumber = sample_rate;
}

uint32_t WCNChipController::GetBTCurrentSamplingRateNumber() {
    return mBTCurrentSamplingRateNumber;
}

bool WCNChipController::IsSupportFM() {
    // Don't Change this
    return mPlatformSupportFM;
}

bool WCNChipController::IsCustomizedPlatformSupportFM() {
    // Customer could modify the return value by other methods
    return true;
}

} // end of namespace android
