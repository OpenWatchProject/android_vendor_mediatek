
#ifndef __MTKOMXVENC_PROPERTIES_H__
#define __MTKOMXVENC_PROPERTIES_H__

#include <stdlib.h>
#include <string.h>
#include <cutils/properties.h>

#define SELF_DEFINE_SYSTEM_PROPERTY(TYPE, FUNC, VAR, DEFAULT) \
    inline TYPE FUNC(const char* VAR);\
    inline TYPE FUNC(){return FUNC(DEFAULT);} \
    inline TYPE FUNC(const char* VAR)

#define DEFINE_SYSTEM_PROPERTY(TYPE, FUNC, KEY, DEFAULT) \
    inline TYPE FUNC(const char* d) { \
        char value[PROPERTY_VALUE_MAX]; \
        property_get(KEY, value, d); \
        return (TYPE) atoi(value); \
    }\
    inline TYPE FUNC(){return FUNC(DEFAULT);}

#define GET_SYSTEM_PROP(TYPE, VAR, KEY, DEFAULT) \
    {\
        char value[PROPERTY_VALUE_MAX]; \
        property_get(KEY, value, DEFAULT); \
        VAR = (TYPE) atoi(value); \
    }

namespace MtkVenc {

SELF_DEFINE_SYSTEM_PROPERTY(int, EnableMoreLog, d, "0")
{
    char buildType[PROPERTY_VALUE_MAX];
    char enableMoreLog[PROPERTY_VALUE_MAX];
    property_get("ro.build.type", buildType, "eng");
    if (!strcmp(buildType, "user") || !strcmp(buildType, "userdebug")) {
        property_get("vendor.mtk.omx.enable.venc.log", enableMoreLog, d);
        return atoi(enableMoreLog);
    }
    else {
        property_get("vendor.mtk.omx.enable.venc.log", enableMoreLog, "1");
        return atoi(enableMoreLog);
    }
}

DEFINE_SYSTEM_PROPERTY(int, MaxScaledWide, "vendor.mtk.omxvenc.input.scaling.max.wide", "1920");
DEFINE_SYSTEM_PROPERTY(int, MaxScaledNarrow, "vendor.mtk.omxvenc.input.scaling.max.narrow", "1088");
DEFINE_SYSTEM_PROPERTY(int, DumpCCNum, "vendor.mtk.omxvenc.ccdumpnum", "0");
DEFINE_SYSTEM_PROPERTY(int, WatchdogTimeout, "vendor.mtk.omxvenc.watchdog.timeout", "2000");
DEFINE_SYSTEM_PROPERTY(bool, EnableVencLog, "vendor.mtk.omx.enable.venc.log", "0");
DEFINE_SYSTEM_PROPERTY(bool, InputScalingMode, "vendor.mtk.omxvenc.input.scaling", "0");
DEFINE_SYSTEM_PROPERTY(bool, DrawStripe, "vendor.mtk.omxvenc.drawline", "0");
DEFINE_SYSTEM_PROPERTY(bool, DumpInputFrame, "vendor.mtk.omxvenc.dump", "0");

DEFINE_SYSTEM_PROPERTY(bool, DumpCts, "vendor.mtk.dumpCts", "0");
DEFINE_SYSTEM_PROPERTY(bool, RTDumpInputFrame, "vendor.mtk.omxvenc.rtdump", "0");
DEFINE_SYSTEM_PROPERTY(bool, DumpColorConvertFrame, "vendor.mtk.omxvenc.ccdump", "0");
DEFINE_SYSTEM_PROPERTY(bool, EnableDummy, "ro.vendor.wfd.dummy.enable", "1");

DEFINE_SYSTEM_PROPERTY(bool, DumpDLBS, "vendor.mtk.omxvenc.dump.dlbs", "0");
DEFINE_SYSTEM_PROPERTY(bool, IsMtklog, "vendor.debug.MB.running", "0");
DEFINE_SYSTEM_PROPERTY(bool, IsViLTE, "vendor.mtk.omxvenc.vilte", "0");
DEFINE_SYSTEM_PROPERTY(bool, AVPFEnable, "vendor.mtk.omxvenc.avpf", "0");

DEFINE_SYSTEM_PROPERTY(bool, RecordBitstream, "vendor.mtk.omxvenc.wfd.record", "0");
DEFINE_SYSTEM_PROPERTY(bool, WFDLoopbackMode, "vendor.mtk.omxvenc.wfd.loopback", "0");
DEFINE_SYSTEM_PROPERTY(bool, DumpSecureInputFlag, "vendor.mtk.omxvenc.secrgb", "0");
DEFINE_SYSTEM_PROPERTY(bool, DumpSecureTmpInFlag, "vendor.mtk.omxvenc.secyuv", "0");
DEFINE_SYSTEM_PROPERTY(bool, DumpSecureOutputFlag, "vendor.mtk.omxvenc.secbs", "0");
DEFINE_SYSTEM_PROPERTY(bool, DumpSecureYv12Flag, "vendor.mtk.omxvenc.dumpyv12test", "0");

DEFINE_SYSTEM_PROPERTY(bool, DumpLog, "vendor.mtk.omx.venc.dl.dump.bs", "0");
DEFINE_SYSTEM_PROPERTY(int, BufferCountActual, "vendor.mtk.omx.venc.bufcount", "8");
DEFINE_SYSTEM_PROPERTY(int, BufferSize, "vendor.mtk.omxvenc.bufk", "0");

DEFINE_SYSTEM_PROPERTY(bool, IsWeChatRecording, "vendor.mtk.omxvenc.wechat.recording", "0");

SELF_DEFINE_SYSTEM_PROPERTY(long, MaxDramSize, d, "0x40000000")
{
    char value[PROPERTY_VALUE_MAX];
    property_get("ro.vendor.mtk_config_max_dram_size", value, d);
    return strtol(value, NULL, 16);
}

DEFINE_SYSTEM_PROPERTY(bool, SvpSupport, "ro.vendor.mtk_sec_video_path_support", "0");
DEFINE_SYSTEM_PROPERTY(bool, TrustTonicTeeSupport, "ro.vendor.mtk_trustonic_tee_support", "0");
DEFINE_SYSTEM_PROPERTY(bool, InHouseReady, "ro.vendor.mtk_venc_inhouse_ready", "0");
DEFINE_SYSTEM_PROPERTY(bool, MicroTrustTeeSupport, "ro.vendor.mtk_microtrust_tee_support", "0");
DEFINE_SYSTEM_PROPERTY(bool, TeeGpSupport, "ro.vendor.mtk_tee_gp_support", "0");
DEFINE_SYSTEM_PROPERTY(int, MBAFF, "vendor.mtk.omxvenc.mbaff", "2");

DEFINE_SYSTEM_PROPERTY(int, BsSize, "vendor.mtk.omx.venc.bssize", "0");
DEFINE_SYSTEM_PROPERTY(bool, DumpBs, "vendor.mtk.omx.venc.dump.bs", "0");
DEFINE_SYSTEM_PROPERTY(bool, DumpVtBs, "vendor.mtk.omx.venc.vilte.dump.bs", "0");
DEFINE_SYSTEM_PROPERTY(bool, UnsupportPrepend, "vendor.mtk.omxvenc.unsupportprepend", "0");
DEFINE_SYSTEM_PROPERTY(bool, DisableANWInMetadata, "vendor.mtk.omxvdec.USANWInMetadata", "0");

DEFINE_SYSTEM_PROPERTY(bool, DynamicFps, "vendor.mtk.omx.venc.dynamicfps", "1");
DEFINE_SYSTEM_PROPERTY(bool, StrideAlign, "vendor.mtk.omx.venc.align", "1");

DEFINE_SYSTEM_PROPERTY(unsigned long, CondTimeoutMs, "vendor.mtk.omx.venc.condtimeout", "500");
} //NS MtkVenc

#endif //__MTKOMXVENC_PROPERTIES_H__