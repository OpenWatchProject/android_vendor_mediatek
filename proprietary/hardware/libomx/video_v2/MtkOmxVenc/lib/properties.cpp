
#include <properties.h>

namespace MtkVenc {

SELF_DEFINE_SYSTEM_PROPERTY(bool, EnableMoreLog, d, "0")
{
    char buildType[PROPERTY_VALUE_MAX];
    char enableMoreLog[PROPERTY_VALUE_MAX];
    property_get("ro.build.type", buildType, "eng");
    if (!strcmp(buildType, "user") || !strcmp(buildType, "userdebug")){
        property_get("vendor.mtk.omx.enable.venc.log", enableMoreLog, d);
        return atoi(enableMoreLog);
    }
    return 1;
}

DEFINE_SYSTEM_PROPERTY(int, MaxScaledWide, "mtk.omxvenc.input.scaling.max.wide", "1920");
DEFINE_SYSTEM_PROPERTY(int, MaxScaledNarrow, "mtk.omxvenc.input.scaling.max.narrow", "1088");
DEFINE_SYSTEM_PROPERTY(int, DumpCCNum, "mtk.omxvenc.ccdumpnum", "0");
DEFINE_SYSTEM_PROPERTY(int, WatchdogTimeout, "mtk.omxvenc.watchdog.timeout", "2000");
DEFINE_SYSTEM_PROPERTY(bool, EnableVencLog, "mtk.omx.enable.venc.log", "0");
DEFINE_SYSTEM_PROPERTY(bool, InputScalingMode, "mtk.omxvenc.input.scaling", "0");
DEFINE_SYSTEM_PROPERTY(bool, DrawStripe, "mtk.omxvenc.drawline", "0");
DEFINE_SYSTEM_PROPERTY(bool, DumpInputFrame, "mtk.omxvenc.dump", "0");

DEFINE_SYSTEM_PROPERTY(bool, DumpCts, "mtk.dumpCts", "0");
DEFINE_SYSTEM_PROPERTY(bool, RTDumpInputFrame, "mtk.omxvenc.rtdump", "0");
DEFINE_SYSTEM_PROPERTY(bool, DumpColorConvertFrame, "mtk.omxvenc.ccdump", "0");
DEFINE_SYSTEM_PROPERTY(bool, EnableDummy, "wfd.dummy.enable", "1");

DEFINE_SYSTEM_PROPERTY(bool, DumpDLBS, "mtk.omxvenc.dump.dlbs", "0");
DEFINE_SYSTEM_PROPERTY(bool, IsMtklog, "debug.MB.running", "0");
DEFINE_SYSTEM_PROPERTY(bool, IsViLTE, "mtk.omxvenc.vilte", "0");
DEFINE_SYSTEM_PROPERTY(bool, AVPFEnable, "mtk.omxvenc.avpf", "0");

DEFINE_SYSTEM_PROPERTY(bool, RecordBitstream, "mtk.omxvenc.wfd.record", "0");
DEFINE_SYSTEM_PROPERTY(bool, WFDLoopbackMode, "mtk.omxvenc.wfd.loopback", "0");
DEFINE_SYSTEM_PROPERTY(bool, DumpSecureInputFlag, "mtk.omxvenc.secrgb", "0");
DEFINE_SYSTEM_PROPERTY(bool, DumpSecureTmpInFlag, "mtk.omxvenc.secyuv", "0");
DEFINE_SYSTEM_PROPERTY(bool, DumpSecureOutputFlag, "mtk.omxvenc.secbs", "0");
DEFINE_SYSTEM_PROPERTY(bool, DumpSecureYv12Flag, "mtk.omxvenc.dumpyv12test", "0");

DEFINE_SYSTEM_PROPERTY(bool, DumpLog, "mtk.omx.venc.dl.dump.bs", "0");
DEFINE_SYSTEM_PROPERTY(int, BufferCountActual, "mtk.omx.venc.bufcount", "8");
DEFINE_SYSTEM_PROPERTY(int, BufferSize, "mtk.omxvenc.bufk", "0");

SELF_DEFINE_SYSTEM_PROPERTY(long, MaxDramSize, d, "0x40000000")
{
    char value[PROPERTY_VALUE_MAX];
    property_get("ro.mtk_config_max_dram_size", value, d);
    return strtol(value, NULL, 16);
}

DEFINE_SYSTEM_PROPERTY(bool, SvpSupport, "ro.mtk_sec_video_path_support", "0");
DEFINE_SYSTEM_PROPERTY(bool, TrustTonicTeeSupport, "ro.mtk_trustonic_tee_support", "0");
DEFINE_SYSTEM_PROPERTY(bool, InHouseReady, "ro.mtk_venc_inhouse_ready", "0");
DEFINE_SYSTEM_PROPERTY(bool, MicroTrustTeeSupport, "ro.mtk_microtrust_tee_support", "0");
DEFINE_SYSTEM_PROPERTY(bool, TeeGpSupport, "ro.mtk_tee_gp_support", "0");
DEFINE_SYSTEM_PROPERTY(int, MBAFF, "mtk.omxvenc.mbaff", "2");

DEFINE_SYSTEM_PROPERTY(int, BsSize, "mtk.omx.venc.bssize", "0");
DEFINE_SYSTEM_PROPERTY(bool, DumpBs, "mtk.omx.venc.dump.bs", "0");
DEFINE_SYSTEM_PROPERTY(bool, DumpVtBs, "mtk.omx.venc.vilte.dump.bs", "0");
DEFINE_SYSTEM_PROPERTY(bool, UnsupportPrepend, "mtk.omxvenc.unsupportprepend", "0");
DEFINE_SYSTEM_PROPERTY(bool, DisableANWInMetadata, "mtk.omxvdec.USANWInMetadata", "0");

DEFINE_SYSTEM_PROPERTY(bool, DynamicFps, "mtk.omx.venc.dynamicfps", "1");
}