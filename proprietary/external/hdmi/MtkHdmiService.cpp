#include "MtkHdmiService.h"

#define LOG_TAG "MtkHdmiService"
#define LOG_CTL "debug.MtkHdmiService.enablelog"

#include <utils/String16.h>
#include <stdint.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <cutils/properties.h>
#include <dlfcn.h>
#include <math.h>
#include "event/hdmi_event.h"

#include "linux/hdmitx.h"

#if defined (MTK_IN_HOUSE_TEE_SUPPORT)
#if defined (MTK_DRM_KEY_MNG_SUPPORT)

#include "keyblock.h"
#include "Keymanage.h"
#include <vendor/mediatek/hardware/keymanage/1.0/IKeymanage.h>
using namespace vendor::mediatek::hardware::keymanage::V1_0;
#endif
#endif

using vendor::mediatek::hardware::hdmi::V1_0::EDID_t;

#define HDMI_ENABLE "persist.vendor.sys.hdmi_hidl.enable"
//#define HDCP_ENABLE "persist.vendor.sys.hdcp.enable"
#define HDMI_VIDEO_AUTO "persist.vendor.sys.hdmi_hidl.auto"
#define HDMI_VIDEO_RESOLUTION "persist.vendor.sys.hdmi_hidl.resolution"
#define HDMI_COLOR_SPACE "persist.vendor.sys.hdmi_hidl.color_space"
#define HDMI_DEEP_COLOR "persist.vendor.sys.hdmi_hidl.deep_color"

#define EDIDNUM 4
#define AUTO_RESOLUTION 100
/**
 * HDMI resolution definition
 */
#define HDMI_VIDEO_720x480i_60Hz 0
#define HDMI_VIDEO_720x576i_50Hz 1
#define RESOLUTION_720X480P_60HZ 2
#define RESOLUTION_720X576P_50HZ 3
#define RESOLUTION_1280X720P_60HZ 4
#define RESOLUTION_1280X720P_50HZ 5
#define RESOLUTION_1920X1080I_60HZ 6
#define RESOLUTION_1920X1080I_50HZ 7
#define RESOLUTION_1920X1080P_30HZ 8
#define RESOLUTION_1920X1080P_25HZ 9
#define RESOLUTION_1920X1080P_24HZ 10
#define RESOLUTION_1920X1080P_23HZ 11
#define RESOLUTION_1920X1080P_29HZ 12
#define RESOLUTION_1920X1080P_60HZ 13
#define RESOLUTION_1920X1080P_50HZ 14
#define RESOLUTION_1280X720P3D_60HZ 15
#define RESOLUTION_1280X720P3D_50HZ 16
#define RESOLUTION_1920X1080I3D_60HZ 17
#define RESOLUTION_1920X1080I3D_50HZ 18
#define RESOLUTION_1920X1080P3D_24HZ 19
#define RESOLUTION_1920X1080P3D_23HZ 20

#define RESOLUTION_3840X2160P23_976HZ 21
#define RESOLUTION_3840X2160P_24HZ 22
#define RESOLUTION_3840X2160P_25HZ 23
#define RESOLUTION_3840X2160P19_97HZ 24
#define RESOLUTION_3840X2160P_30HZ 25
#define RESOLUTION_4096X2161P_24HZ 26

#define RESOLUTION_3840X2160P_60HZ 27
#define RESOLUTION_3840X2160P_50HZ 28
#define RESOLUTION_4096X2161P_60HZ 29
#define RESOLUTION_4096X2161P_50HZ 30


namespace vendor {
namespace mediatek {
namespace hardware {
namespace hdmi {
namespace V1_0 {
namespace implementation {

sp<HdmiUEventThread> event_thread;

static int hdmi_ioctl(int code, unsigned long value);
int enableHDMI(int enable);
int enableHDMIInit(int enable);
int enableHDCP(int value);
int setHdcpKey(char* key);
bool setDrmKey();
int getCapabilities();
int getDisplayType();
EDID_t getResolutionMask();
int setVideoResolution(int res);


static int mEdid[4];
static int DEFAULT_RESOLUTIONS[4] = {RESOLUTION_1920X1080P_30HZ,RESOLUTION_1920X1080P_60HZ,RESOLUTION_1280X720P_60HZ,RESOLUTION_720X480P_60HZ};
static int DEFAULT_ALL_RESOLUTIONS[24] = { RESOLUTION_4096X2161P_24HZ,
                RESOLUTION_3840X2160P_30HZ,RESOLUTION_3840X2160P19_97HZ,
                RESOLUTION_3840X2160P_25HZ,RESOLUTION_3840X2160P_24HZ,
                RESOLUTION_3840X2160P23_976HZ,RESOLUTION_1920X1080P_60HZ,
                RESOLUTION_1920X1080P_50HZ, RESOLUTION_1920X1080P_30HZ,
                RESOLUTION_1920X1080P_25HZ, RESOLUTION_1920X1080P_24HZ,
                RESOLUTION_1920X1080P_23HZ, RESOLUTION_1920X1080I_60HZ,
                RESOLUTION_1920X1080I_50HZ, RESOLUTION_1280X720P_60HZ,
                RESOLUTION_1280X720P_50HZ, RESOLUTION_720X480P_60HZ,
                RESOLUTION_720X576P_50HZ };

static int PREFERED_RESOLUTIONS[22]{
                    RESOLUTION_4096X2161P_50HZ,
                    RESOLUTION_4096X2161P_60HZ,
                    RESOLUTION_3840X2160P_50HZ,
                    RESOLUTION_3840X2160P_60HZ,
                    RESOLUTION_4096X2161P_24HZ,
                    RESOLUTION_3840X2160P_30HZ,
                    RESOLUTION_3840X2160P19_97HZ,
                    RESOLUTION_3840X2160P_25HZ,
                    RESOLUTION_3840X2160P_24HZ,
                    RESOLUTION_3840X2160P23_976HZ,
                    RESOLUTION_1920X1080P_60HZ,
                    RESOLUTION_1920X1080P_50HZ,
                    RESOLUTION_1920X1080P_30HZ,
                    RESOLUTION_1920X1080P_25HZ,
                    RESOLUTION_1920X1080P_24HZ,
                    RESOLUTION_1920X1080P_23HZ,
                    RESOLUTION_1920X1080I_60HZ,
                    RESOLUTION_1920X1080I_50HZ,
                    RESOLUTION_1280X720P_60HZ,
                    RESOLUTION_1280X720P_50HZ,
                    RESOLUTION_720X480P_60HZ,
                    RESOLUTION_720X576P_50HZ };

static int sResolutionMask[15] = {0,0,SINK_480P, SINK_576P,
        SINK_720P60, SINK_720P50, SINK_1080I60, SINK_1080I50, SINK_1080P30,
        SINK_1080P25, SINK_1080P24, SINK_1080P23976, SINK_1080P2997,
        SINK_1080P60, SINK_1080P50 };

#if defined (MTK_INTERNAL_HDMI_SUPPORT)
static int sResolutionMask_4k2k[10] = { SINK_2160P_23_976HZ, SINK_2160P_24HZ,
    SINK_2160P_25HZ, SINK_2160P_29_97HZ, SINK_2160P_30HZ, SINK_2161P_24HZ,
    SINK_2160P_60HZ, SINK_2160P_50HZ, SINK_2161P_60HZ, SINK_2161P_50HZ};
#endif

bool mHdmiStateChanged= false;
int mHdmiEnable = 0;
int mHdmiResolution = 0;
int mHdmiAutoMode = 0;

int getValue(char* key, char* defValue) {
    char buf[PROPERTY_VALUE_MAX];
    property_get(key,buf,defValue);
    ALOGD("getValue: %s, %s" , key, buf);
    return (atoi(buf));
}
int setValue(char* key, int value) {
    char buf[PROPERTY_VALUE_MAX];
    sprintf(buf,"%d",value);
    int ret = property_set(key,buf);
    ALOGD("setValue: %s, %s" , key, buf);
    return ret;
}


void MtkHdmiService::refreshEdid() {
    EDID_t s_edid = getResolutionMask();
    //int preEdid = getValue(HDMI_edid,"0")
    ALOGI("refreshEdid s_edid.edid[0] %d" , s_edid.edid[0]);
    ALOGI("refreshEdid s_edid.edid[1] %d" , s_edid.edid[1]);

    //ALOGI("refresh preEdid %d" , preEdid);
    mEdid[0] = s_edid.edid[0];
    mEdid[1] = s_edid.edid[1];
    mEdid[2] = s_edid.edid[2];
    mEdid[3] = s_edid.edid[3];
    mHdmiStateChanged = true;
    setVideoResolution(getValue(HDMI_VIDEO_RESOLUTION,"100"));
}

int* getSupportedResolutions() {
    if (0 == mEdid[0] && 0 == mEdid[1] && 0 == mEdid[2]) {
        return DEFAULT_RESOLUTIONS;
    }
    return DEFAULT_ALL_RESOLUTIONS;
}

bool isResolutionSupport(int resolution) {
    ALOGI("isResolutionSupport: %d ", resolution);
    if (resolution >= AUTO_RESOLUTION) {
        return false;
    }
    int* supportedResolutions = getSupportedResolutions();
    for (int i =0 ;i < sizeof(supportedResolutions)/sizeof(int); i++) {
        if (*(supportedResolutions +i) == resolution) {
            return true;
        }
    }
    return false;
}

int getSuitableResolution(int resolution) {
    ALOGI("getSuitableResolution: %d ",resolution);
    int SuitableResolution = resolution;
    if (mEdid[0]!= 0 || mEdid[1]!= 0 || mEdid[2]!= 0 ) {
        int edidTemp = mEdid[0] | mEdid[1];
        int edidTemp_4k2k = mEdid[2];
        ALOGI("getSuitableResolution edidTemp: %d ",edidTemp);
        ALOGI("getSuitableResolution edidTemp_4k2k: %d ",edidTemp_4k2k);
        int* prefered = PREFERED_RESOLUTIONS;
        for (int i = 0; i < 22; i++) {
            int act = *(prefered + i);
            ALOGI("getSuitableResolution act: %d ",act);
            if(act < RESOLUTION_3840X2160P23_976HZ){
               if (0 != (edidTemp & sResolutionMask[act])) {
                    SuitableResolution = act;
                    ALOGI("getSuitableResolution resolution: %d ",SuitableResolution);
                    break;
              }
            }
        #if defined (MTK_INTERNAL_HDMI_SUPPORT)
            else{
                if (0 != (edidTemp_4k2k & sResolutionMask_4k2k[act - RESOLUTION_3840X2160P23_976HZ])) {
                    ALOGI("getSuitableResolution resolution 4k: %d ",SuitableResolution);
                    SuitableResolution = act;
                    break;
                }
            }
        #endif
        }
    } else {
        SuitableResolution = 2;
        ALOGI("getSuitableResolution edid==null,set solution to 480P60");
    }
    ALOGI("getSuitableResolution resolution final: %d ",SuitableResolution);
    return SuitableResolution;
}

////////////////////////////////////////////

bool setDrmKey() {
    bool ret = false;
#if defined (MTK_HDMI_SUPPORT)
#if defined (MTK_IN_HOUSE_TEE_SUPPORT)
#if defined (MTK_DRM_KEY_MNG_SUPPORT)

    ALOGI("setDrmKey\n");
    hdmi_hdcp_drmkey hKey;
    int i;
    int ret_temp = 0;
    unsigned char* enckbdrm = NULL;
    unsigned int inlength = 0;

    android::sp<IKeymanage> hdmi_hdcp_client = IKeymanage::getService();

    auto hdmi_hdcp_callback = [&] (const android::hardware::hidl_vec<uint8_t>& oneDrmkeyBlock,
        uint32_t blockLeng, int32_t ret_val)
    {
        ret_temp = ret_val;

        ALOGI("[KM_HIDL] blockLeng = %u\n", blockLeng);
        enckbdrm = (unsigned char *)malloc(blockLeng);
        if (enckbdrm == NULL)
        {
            ALOGI("[KM_HIDL] malloc failed----\n");
        }

        convertVector2Array_U(oneDrmkeyBlock, enckbdrm);
        inlength = blockLeng;
    };

    hdmi_hdcp_client->get_encrypt_drmkey_hidl(HDCP_1X_TX_ID,  hdmi_hdcp_callback);

    if(ret_temp != 0 )
    {
        ALOGI("[KM_HIDL] setHDMIDRMKey get_encrypt_drmkey failed %d", ret_temp);
        if (enckbdrm != NULL)
        {
            free(enckbdrm);
            enckbdrm = NULL;
        }
        return ret;
    }

    memcpy(hKey.u1Hdcpkey, (unsigned char*)enckbdrm, sizeof(hKey.u1Hdcpkey));
    ret = hdmi_ioctl(MTK_HDMI_HDCP_KEY, (long)&hKey);
    ALOGI("setHDMIDRMKey ret = %d\n",ret);

    if (enckbdrm != NULL)
    {
        free(enckbdrm);
        enckbdrm = NULL;
    }

#endif
#endif
#endif

    return ret;
}

void startObserving(){
    event_thread = new HdmiUEventThread();
    if (event_thread == NULL)
    {
        ALOGE("Failed to initialize UEvent thread!!");
        abort();
    }
    int ret = event_thread->run("HdmiUEventThread");
    ALOGI("HdmiUEventThread run: %d", ret);
}

void initialize(){
    ALOGI("hdmi initialize()");
#if defined (MTK_MT8193_HDCP_SUPPORT)||defined (MTK_HDMI_HDCP_SUPPORT)
    #if defined (MTK_IN_HOUSE_TEE_SUPPORT)
        #if defined (MTK_DRM_KEY_MNG_SUPPORT)
            setDrmKey();
        #endif
    #endif
#endif
    startObserving();

    enableHDMIInit(getValue(HDMI_ENABLE,"1"));
}

int getCapabilities() {
    int result = 0;
#if defined (MTK_HDMI_SUPPORT)
    //if (hdmi_ioctl(MTK_HDMI_GET_CAPABILITY, (long)&result) == false) {
    //    result = 0;
    //}
#endif
    ALOGI("getCapabilities(%d)\n", result);
    return result;
}

int getDisplayType() {
    int result = 0;
#if defined (MTK_HDMI_SUPPORT)
   /* bool ret = false;
    mtk_dispif_info_t hdmi_info;
    memset((void *)&hdmi_info,0,sizeof(mtk_dispif_info_t));
    ret = hdmi_ioctl(MTK_HDMI_GET_DEV_INFO, (long)&hdmi_info);
    if (ret) {
        if (hdmi_info.displayType == HDMI_SMARTBOOK) {
            result = 1;
        } else if (hdmi_info.displayType == MHL) {
            result = 2;
        } else if (hdmi_info.displayType == SLIMPORT) {
            result = 3;
        }
    }*/
#endif
    ALOGI("getDisplayType(%d)\n", result);
    return result;
}


static int setDeepColor(int color, int deep) {
    int ret = -1;
#if defined (MTK_MT8193_HDMI_SUPPORT)||defined (MTK_INTERNAL_HDMI_SUPPORT)
    hdmi_para_setting h;
      h.u4Data1 = color;
      h.u4Data2 = deep;
      ret = hdmi_ioctl(MTK_HDMI_COLOR_DEEP, (long)&h);
      if (ret >= 0) {
          ALOGI("setDeepColor(%d,%d)\n", color, deep);
      }
#endif
    return ret;
}

//---------------------------hdmi implementation start-----------------------

static int hdmi_ioctl(int code, unsigned long value){
    int fd = open("/dev/hdmitx", O_RDONLY, 0);//O_RDONLY;O_RDWR
    int ret = -1;
    if (fd >= 0) {
        ret = ioctl(fd, code, value);
        if (ret < 0) {
            ALOGE("[%s] failed. ioctlCode: %d, errno: %d",
                 __func__, code, errno);
        }
        close(fd);
    } else {
        ALOGE("[%s] open hdmitx failed. errno: %d", __func__, errno);
    }
    ALOGI("[%s] lv ret: %d", __func__,ret);
    return ret;
}

int enableHDMIInit(int value){
    ALOGI("enableHDMIInit = %d", value);
    int ret = -1;
#if defined (MTK_HDMI_SUPPORT)
    ret = hdmi_ioctl(MTK_HDMI_AUDIO_VIDEO_ENABLE, value);
#endif
    if(ret >= 0 && value == 1)
    {
        setValue(HDMI_ENABLE, value);
    }

    return ret;
}

int enableHDMI(int value) {
    ALOGI("enableHDMI = %d", value);
    //bool enable = (value) > 0 ? true : false;
    int ret = -1;
#if defined (MTK_HDMI_SUPPORT)
    ret = hdmi_ioctl(MTK_HDMI_AUDIO_VIDEO_ENABLE, value);
#endif
    if(ret >= 0)
    {
        setValue(HDMI_ENABLE, value);
    }

    return ret;
}

int enableHDCP(int value) {
    ALOGI("enableHDCP = %d", value);
    bool enable = (value) > 0 ? true : false;
    int ret = -1;
#if defined (MTK_MT8193_HDCP_SUPPORT)||defined (MTK_HDMI_HDCP_SUPPORT)
    ret = hdmi_ioctl(MTK_HDMI_ENABLE_HDCP, (unsigned long)&enable);
#endif
    if(ret >= 0)
    {
        //setValue(HDCP_ENABLE, value);
    }

    return ret;
}

#define HDMI_RES_MASK 0x80
#define LOGO_DEV_NAME "/dev/block/mmcblk0p9"

int setVideoConfig(int vformat) {
    ALOGI("setVideoConfig = %d", vformat);
    int ret = -1;

#if defined (MTK_HDMI_SUPPORT)
    ret = hdmi_ioctl(MTK_HDMI_VIDEO_CONFIG, vformat);
#endif

#if defined (MTK_ALPS_BOX_SUPPORT)
    int fd = open(LOGO_DEV_NAME, O_RDWR);
    if (fd >= 0) {
        int hdmi_res = vformat|HDMI_RES_MASK;

        lseek(fd, -512 ,SEEK_END);

        ret = write(fd, (void*)&hdmi_res, sizeof(hdmi_res));
        ALOGI("setVideoConfig hdmi_res 0x%x",hdmi_res);
        if (ret < 0) {
            ALOGE("[%s] failed. ioctlCode: %d, errno: %d",
                __func__, vformat, errno);
        }
        close(fd);
    } else {
        ALOGE("[%s] open %s failed. errno:%d %s", __func__, LOGO_DEV_NAME, errno, strerror(errno));
    }
#endif
    return ret;
}

//-------------------------hdmi implementation end---------------------------------


MtkHdmiService::MtkHdmiService(){
    ALOGD("HIDL MtkHdmiService()");
    initialize();
}

MtkHdmiService::~MtkHdmiService(){
    ALOGD("~HIDL MtkHdmiService()");
    if (event_thread != NULL) {
        ALOGE("~MtkHdmiService requestExit");
        event_thread->requestExit();
        ALOGE("~MtkHdmiService requestExitAndWait");
        event_thread->requestExitAndWait();
        ALOGE("MtkHdmiService clear");
        event_thread = NULL;
        ALOGE("~uevent_thread done");
    }
}

int setVideoResolution(int resolution) {
    if(getValue(HDMI_VIDEO_AUTO,"1") == 1)
        setValue(HDMI_VIDEO_AUTO, 1);

    ALOGI("setVideoResolution = %d" , resolution);
    int suitableResolution = resolution;
    if (resolution >= AUTO_RESOLUTION || getValue(HDMI_VIDEO_AUTO,"1") == 1) {
        suitableResolution = getSuitableResolution(resolution);
    }
    ALOGI("suitableResolution = %d" , suitableResolution);
    if (suitableResolution == getValue(HDMI_VIDEO_RESOLUTION,"100")) {
        ALOGI("setVideoResolution is the same");
        if (!mHdmiStateChanged) {
            ALOGI("setVideoResolution is the same return");
            return true;
        }
    }
    int finalResolution = suitableResolution >= AUTO_RESOLUTION ? (suitableResolution - AUTO_RESOLUTION)
                : suitableResolution;
    ALOGI("final video resolution = %d ", finalResolution);

    /*if (finalResolution == 27) {
        int* edid_temp = getResolutionMask();
        if (edid_temp[3] & SINK_YCBCR_420) {
            setDeepColor(4, 1);
        } else if (edid_temp[3] & SINK_YCBCR_420_CAPABILITY) {
            setDeepColor(3, 1);
        }
    } else {
        setDeepColor(3, 1);
    }*/
    setValue(HDMI_VIDEO_RESOLUTION, finalResolution);
    mHdmiStateChanged = false;
    int param = (finalResolution & 0xff);
    return setVideoConfig(param);
}

EDID_t getResolutionMask(){
    ALOGI("getResolutionMask in");
    int ret = -1;
    EDID_t cResult;
#if defined (MTK_HDMI_SUPPORT)
    HDMI_EDID_T edid;
    ret = hdmi_ioctl(MTK_HDMI_GET_EDID, (long)&edid);
    ALOGI("hdmi_ioctl,ret = %d",ret);
    if (ret >= 0) {
        ALOGI("edid.ui4_ntsc_resolution %4X\n", edid.ui4_ntsc_resolution);
        ALOGI("edid.ui4_pal_resolution %4X\n", edid.ui4_pal_resolution);
        #if defined (MTK_INTERNAL_HDMI_SUPPORT)
        ALOGI("edid.ui4_sink_hdmi_4k2kvic %4X\n", edid.ui4_sink_hdmi_4k2kvic);
        #endif
        ALOGI("edid.ui2_sink_colorimetry %4X\n", edid.ui2_sink_colorimetry);

        cResult.edid[0] = edid.ui4_ntsc_resolution;
        cResult.edid[1] = edid.ui4_pal_resolution;
        #if defined (MTK_INTERNAL_HDMI_SUPPORT)
        cResult.edid[2] = edid.ui4_sink_hdmi_4k2kvic;
        cResult.edid[3] = edid.ui2_sink_colorimetry;
        #else
        cResult.edid[2] = 0;
        cResult.edid[3] = edid.ui2_sink_colorimetry;
        #endif
    }
#endif
    ALOGI("getEDID\n");
    return cResult;
}


// Methods from ::vendor::mediatek::hardware::hdmi::V1_0::IMtkHdmiService follow.
Return<void> MtkHdmiService::get_resolution_mask(get_resolution_mask_cb _hidl_cb) {
    Mutex::Autolock _l(mLock);
    ALOGD("get_resolution_mask...");
    EDID_t edid = getResolutionMask();
    _hidl_cb(Result::SUCCEED, edid);
    return Void();
}

Return<::vendor::mediatek::hardware::hdmi::V1_0::Result> MtkHdmiService::enable_hdcp(bool enable) {
    // TODO implement
    ALOGD("enable_hdcp...");
    return Result::SUCCEED;
}

Return<::vendor::mediatek::hardware::hdmi::V1_0::Result> MtkHdmiService::enable_hdmi(bool enable) {
    Mutex::Autolock _l(mLock);
    ALOGD("enable_hdmi,enable = %d" , enable);
    int ret = enableHDMI(enable);
    if (ret < 0){
        return Result::FAILED;
    }
    return Result::SUCCEED;
}

Return<::vendor::mediatek::hardware::hdmi::V1_0::Result> MtkHdmiService::set_video_resolution(int32_t resolution) {
    Mutex::Autolock _l(mLock);
    ALOGD("set_video_resolution,resolution = %d" , resolution);
    int ret = setVideoResolution(resolution);
    if (ret < 0){
        return Result::FAILED;
    }
    return Result::SUCCEED;
}

Return<::vendor::mediatek::hardware::hdmi::V1_0::Result> MtkHdmiService::enable_hdmi_hdr(bool enable) {
    // TODO implement
    ALOGD("enable_hdmi_hdr,enable = %d" , enable);
    return Result::SUCCEED;
}

Return<::vendor::mediatek::hardware::hdmi::V1_0::Result> MtkHdmiService::set_auto_mode(bool enable) {
    Mutex::Autolock _l(mLock);
    ALOGD("set_auto_mode,enable = %d" , enable);
    if (enable){
        setValue(HDMI_VIDEO_AUTO, 1);
    } else {
        setValue(HDMI_VIDEO_AUTO, 0);
    }
    return Result::SUCCEED;
}

Return<::vendor::mediatek::hardware::hdmi::V1_0::Result> MtkHdmiService::set_color_format(int32_t color_format) {
    // TODO implement
    //set color format property here
    ALOGD("set_color_format,color_format = %d" , color_format);
    return Result::SUCCEED;
}

Return<::vendor::mediatek::hardware::hdmi::V1_0::Result> MtkHdmiService::set_color_depth(int32_t color_depth) {
    // TODO implement
    //set color depth property here
    ALOGD("set_color_depth,color_depth = %d" , color_depth);
    return Result::SUCCEED;
}


// Methods from ::android::hidl::base::V1_0::IBase follow.

IMtkHdmiService* HIDL_FETCH_IMtkHdmiService(const char* /* name */) {
    return new MtkHdmiService();
}

//
}  // namespace implementation
}  // namespace V1_0
}  // namespace hdmi
}  // namespace hardware
}  // namespace mediatek
}  // namespace vendor
