#include <hidl/LegacySupport.h>
#include "stdlib.h"
#include "Em_hidl_service.h"
#include <cutils/properties.h>
#include "ModuleMcfGetFileList.h"
#include "BluetoothTest.h"
#ifdef LOG_TAG
#undef LOG_TAG
#define LOG_TAG "em_hidl"
#endif

namespace vendor {
namespace mediatek {
namespace hardware {
namespace engineermode {
namespace V1_2 {
namespace implementation {


using ::android::status_t;



//EM tlephony property
#define PROPERTY_RADIO_CAPABILITY_SWITCH_ENABLE  "persist.vendor.radio.simswitch.emmode"
#define PROPERTY_RADIO_VOLTE_MAL_PCTID  "vendor.ril.volte.mal.pctid"
#define PROPERTY_RADIO_DSBP_SUPPORT  "persist.vendor.radio.mtk_dsbp_support"
#define PROPERTY_RADIO_IMS_TEST_MODE  "persist.vendor.radio.imstestmode"
#define PROPERTY_RADIO_SMS_FORMAT  "persist.vendor.radio.smsformat"
#define PROPERTY_RADIO_GPRS_PREFER  "persist.vendor.radio.gprs.prefer"
#define PROPERTY_RADIO_TESTSIM_CARDTYPE  "persist.vendor.radio.testsim.cardtype"
#define PROPERTY_RADIO_CT_IR_ENGMODE  "persist.vendor.radio.ct.ir.engmode"
#define PROPERTY_RADIO_DISABLE_C2K_CAP  "persist.vendor.radio.disable_c2k_cap"

//EM log and debug property
#define PROPERTY_DEBUG_MD_RESET_WAIT "vendor.mediatek.debug.md.reset.wait"
#define PROPERTY_CONNSYS_COREDUMP_MODE "persist.vendor.connsys.coredump.mode"
#define PROPERTY_OMX_ENABLE_VENC_LOG "vendor.mtk.omx.enable.venc.log"
#define PROPERTY_OMX_ENABLE_MVAMGR_LOG "vendor.mtk.omx.enable.mvamgr.log"
#define PROPERTY_OMX_VDEC_LOG "vendor.mtk.omx.vdec.log"
#define PROPERTY_OMX_VDEC_PERF_LOG "vendor.mtk.omx.vdec.perf.log"
#define PROPERTY_VDEC_LOG "vendor.mtk.vdec.log"
#define PROPERTY_VDECTLC_LOG "vendor.mtk.vdectlc.log"
#define PROPERTY_OMX_CORE_LOG "vendor.mtk.omx.core.log"
#define PROPERTY_VENC_H264_SHOWLOG "vendor.mtk.venc.h264.showlog"
#define PROPERTY_MODEM_WARNING "persist.vendor.radio.modem.warning"

//Hardware-testing UartUsb switch
#define PROPERTY_USB_PORT "vendor.usb.port.mode"
//Hardware-testing usb
#define PROPERTY_USB_OTG_SWITCH "persist.vendor.usb.otg.switch"
#define PROPERTY_EM_USB_VAL "vendor.em.usb.value"
#define PROPERTY_EM_USB_TYPE "vendor.em.usb.set"
#define PROPERTY_BYPASS_EN "vendor.em.usb.bypass.enable"
#define PROPERTY_BYPASS_DIS "vendor.em.usb.bypass.disable"
#define PROPERTY_BYPASS_SERVICE "vendor.em.usb.bypass.service"

//EM others property
#define PROPERTY_MOMS "persist.vendor.sys.disable.moms"

char *safe_strncpy(char *dest, const char *src, size_t n) {
    if (dest && n > 0) {
        dest[0] = '\0';
        strncat(dest, src, --n);
    }
    return dest;
}

int cpp_main() {
    ALOGI("log_hidl_service_main runs start");

    ::android::hardware::configureRpcThreadpool(20, true);
    sp<IEmd> mEmHidlService = new EmHidlService("EmHidlServer");
    ::android::hardware::joinRpcThreadpool();

    ALOGI("log_hidl_service_main runs end");
    return 0;
}

//EmHidlService
EmHidlService::EmHidlService(const char* name) {
    ALOGI("EmHidlService name = [%s]", name);
    status_t status;
    status = this->registerAsService(name);
    if(status != 0) {
        ALOGI("EmHidlService() registerAsService() for name=[%s] failed status=[%d]",
                name, status);
    } else {
        ALOGI("EmHidlService() registerAsService() for name=[%s] successful status=[%d]",
                    name, status);
    }
    mLogCallback = nullptr;
    safe_strncpy(this->mName, name, sizeof(mName));
    ALOGI("EmHidlService Done!");
}

EmHidlService::~EmHidlService() {
    ALOGI("[%s] ~EmHidlService()", mName);
}

Return<void> EmHidlService::setCallback(const sp<IEmCallback>& callback) {
    ALOGI("[%s] setCallback()", mName);
    mLogCallback = callback;
    return Void();
}

Return<bool> EmHidlService::sendToServer(const hidl_string& data) {
    bool ret = true;
    ALOGI("[%s] sendToServer() data = [%s]!!!!", mName, data.c_str());
    if (mLogCallback == nullptr) {
        ALOGI("[%s] mLogCallback is null", mName);
        return false;
    }
    ret = mLogCallback->callbackToClient("I am HIDL service from EMd!");
    return ret;
}

Return<bool> EmHidlService::sendToServerWithCallBack(const hidl_string& data,
           const sp<IEmCallback>& callback) {
    bool ret = true;
    ALOGI("[%s] sendToServer() data = [%s]!!!!", mName, data.c_str());
    if (callback == nullptr) {
        ALOGI("[%s] callback is null", mName);
        return false;
    }
    ret = callback->callbackToClient("I am HIDL service from EMd!");
    return ret;
}

Return<bool> EmHidlService::setSmsFormat(const hidl_string& data) {
    bool ret = true;
    ALOGI("[%s] setSmsFormat() data = [%s]!!!!", mName, data.c_str());
    property_set(PROPERTY_RADIO_SMS_FORMAT, data.c_str());
    return ret;
}

Return<bool> EmHidlService::setCtIREngMode(const hidl_string& data) {
    bool ret = true;
    ALOGI("[%s] setCtIREngMode() data = [%s]!!!!", mName, data.c_str());
    property_set(PROPERTY_RADIO_CT_IR_ENGMODE, data.c_str());
    return ret;
}

Return<bool> EmHidlService::setTestSimCardType(const hidl_string& data) {
    bool ret = true;
    ALOGI("[%s] setTestSimCardType() data = [%s]!!!!", mName, data.c_str());
    property_set(PROPERTY_RADIO_TESTSIM_CARDTYPE, data.c_str());
    return ret;
}

Return<bool> EmHidlService::setPreferGprsMode(const hidl_string& data) {
    bool ret = true;
    ALOGI("[%s] setPreferGprsMode() data = [%s]!!!!", mName, data.c_str());
    property_set(PROPERTY_RADIO_GPRS_PREFER, data.c_str());
    return ret;
}

Return<bool> EmHidlService::setRadioCapabilitySwitchEnable(const hidl_string& data) {
    bool ret = true;
    ALOGI("[%s] setRadioCapabilitySwitchEnable() data = [%s]!!!!", mName, data.c_str());
    property_set(PROPERTY_RADIO_CAPABILITY_SWITCH_ENABLE, data.c_str());
    return ret;
}

Return<bool> EmHidlService::setDisableC2kCap(const hidl_string& data) {
    bool ret = true;
    ALOGI("[%s] setDisableC2kCap() data = [%s]!!!!", mName, data.c_str());
    property_set(PROPERTY_RADIO_DISABLE_C2K_CAP, data.c_str());
    return ret;
}

Return<bool> EmHidlService::setImsTestMode(const hidl_string& data) {
    bool ret = true;
    ALOGI("[%s] setImsTestMode() data = [%s]!!!!", mName, data.c_str());
    property_set(PROPERTY_RADIO_IMS_TEST_MODE, data.c_str());
    return ret;
}

Return<bool> EmHidlService::setDsbpSupport(const hidl_string& data) {
    bool ret = true;
    ALOGI("[%s] setDsbpSupport() data = [%s]!!!!", mName, data.c_str());
    property_set(PROPERTY_RADIO_DSBP_SUPPORT, data.c_str());
    return ret;
}

Return<bool> EmHidlService::setVolteMalPctid(const hidl_string& data) {
    bool ret = true;
    ALOGI("[%s] setVolteMalPctid() data = [%s]!!!!", mName, data.c_str());
    property_set(PROPERTY_RADIO_VOLTE_MAL_PCTID, data.c_str());
    return ret;
}

Return<bool> EmHidlService::setMdResetDelay(const hidl_string& data) {
    bool ret = true;
    ALOGI("[%s] setMdResetDelay() data = [%s]!!!!", mName, data.c_str());
    property_set(PROPERTY_DEBUG_MD_RESET_WAIT, data.c_str());
    return ret;
}

Return<bool> EmHidlService::setWcnCoreDump(const hidl_string& data) {
    bool ret = true;
    ALOGI("[%s] setWcnCoreDump() data = [%s]!!!!", mName, data.c_str());
    property_set(PROPERTY_CONNSYS_COREDUMP_MODE, data.c_str());
    return ret;
}

Return<bool> EmHidlService::setOmxVencLogEnable(const hidl_string& data) {
    bool ret = true;
    ALOGI("[%s] setOmxVencLogEnable() data = [%s]!!!!", mName, data.c_str());
    property_set(PROPERTY_OMX_ENABLE_VENC_LOG, data.c_str());
    return ret;
}

Return<bool> EmHidlService::setOmxVdecLogEnable(const hidl_string& data) {
    bool ret = true;
    ALOGI("[%s] setOmxVdecLogEnable() data = [%s]!!!!", mName, data.c_str());
    property_set(PROPERTY_OMX_ENABLE_MVAMGR_LOG, data.c_str());
    property_set(PROPERTY_OMX_VDEC_LOG, data.c_str());
    property_set(PROPERTY_OMX_VDEC_PERF_LOG, data.c_str());
    return ret;
}

Return<bool> EmHidlService::setVdecDriverLogEnable(const hidl_string& data) {
    bool ret = true;
    ALOGI("[%s] setVdecDriverLogEnable() data = [%s]!!!!", mName, data.c_str());
    property_set(PROPERTY_VDEC_LOG, data.c_str());
    return ret;
}

Return<bool> EmHidlService::setSvpLogEnable(const hidl_string& data) {
    bool ret = true;
    ALOGI("[%s] setSvpLogEnable() data = [%s]!!!!", mName, data.c_str());
    property_set(PROPERTY_VDECTLC_LOG, data.c_str());
    return ret;
}

Return<bool> EmHidlService::setOmxCoreLogEnable(const hidl_string& data) {
    bool ret = true;
    ALOGI("[%s] setOmxCoreLogEnable() data = [%s]!!!!", mName, data.c_str());
    property_set(PROPERTY_OMX_CORE_LOG, data.c_str());
    return ret;
}

Return<bool> EmHidlService::setVencDriverLogEnable(const hidl_string& data) {
    bool ret = true;
    ALOGI("[%s] setVencDriverLogEnable() data = [%s]!!!!", mName, data.c_str());
    property_set(PROPERTY_VENC_H264_SHOWLOG, data.c_str());
    return ret;
}

Return<bool> EmHidlService::setModemWarningEnable(const hidl_string& data) {
    bool ret = true;
    ALOGI("[%s] setModemWarningEnable() data = [%s]!!!!", mName, data.c_str());
    property_set(PROPERTY_MODEM_WARNING, data.c_str());
    return ret;
}

//Hardware-testing UartUsb switch
Return<bool> EmHidlService::setUsbPort(const hidl_string& data) {
    bool ret = true;
    ALOGI("[%s] setUsbPort() data = [%s]!!!!", mName, data.c_str());
    property_set(PROPERTY_USB_PORT, data.c_str());
    return ret;
}

Return<bool> EmHidlService::setUsbOtgSwitch(const hidl_string& data) {
    bool ret = true;
    ALOGI("[%s] setUsbOtgSwitch() data = [%s]!!!!", mName, data.c_str());
    property_set(PROPERTY_USB_OTG_SWITCH, data.c_str());
    return ret;
}

Return<bool> EmHidlService::setEmUsbValue(const hidl_string& data) {
    bool ret = true;
    ALOGI("[%s] setEmUsbValue() data = [%s]!!!!", mName, data.c_str());
    property_set(PROPERTY_EM_USB_VAL, data.c_str());
    return ret;
}

Return<bool> EmHidlService::getFilePathListWithCallBack(const hidl_string& data,
            const sp<IEmCallback>& callback) {
    bool ret = true;
    char *file_buff = (char *)malloc(SEND_BUFFER_SIZE);
    if (file_buff == NULL)
        return false;
    memset(file_buff, 0, SEND_BUFFER_SIZE);
    ALOGI("[%s] sendToServer() data = [%s]", mName, data.c_str());
    if (callback == NULL) {
        ALOGI("[%s] callback is null", mName);
        return false;
    }
    ALOGD("[McfConfig] Receive : %s", data.c_str());
    ModuleMcfGetFileList::searchdir(data.c_str(), file_buff);
    ret = callback->callbackToClient(file_buff);
    free(file_buff);
    return ret;
}

Return<bool> EmHidlService::setEmUsbType(const hidl_string& data) {
    bool ret = true;
    ALOGI("[%s] setEmUsbSet() data = [%s]!!!!", mName, data.c_str());
    property_set(PROPERTY_EM_USB_TYPE, data.c_str());
    return ret;
}

Return<bool> EmHidlService::setBypassEn(const hidl_string& data) {
    bool ret = true;
    ALOGI("[%s] setEmBypassEn() data = [%s]!!!!", mName, data.c_str());
    property_set(PROPERTY_BYPASS_EN, data.c_str());
    return ret;
}

Return<bool> EmHidlService::setBypassDis(const hidl_string& data) {
    bool ret = true;
    ALOGI("[%s] setEmBypassDis() data = [%s]!!!!", mName, data.c_str());
    property_set(PROPERTY_BYPASS_DIS, data.c_str());
    return ret;
}

Return<bool> EmHidlService::setBypassService(const hidl_string& data) {
    bool ret = true;
    ALOGI("[%s] setEmBypassService() data = [%s]!!!!", mName, data.c_str());
    property_set(PROPERTY_BYPASS_SERVICE, data.c_str());
    return ret;
}

Return<bool> EmHidlService::setMoms(const hidl_string& data) {
    bool ret = true;
    ALOGI("[%s] setMoms() data = [%s]!!!!", mName, data.c_str());
    property_set(PROPERTY_MOMS, data.c_str());
    return ret;
}

//EM BT
Return<int32_t> EmHidlService::btStartRelayer(int32_t port, int32_t speed) {
#ifdef MTK_BT_EM_HIDL_SUPPORT
    return BluetoothTest::startBtRelayer(port, speed);
#else
    return 0;
#endif
}

Return<int32_t> EmHidlService::btStopRelayer() {
#ifdef MTK_BT_EM_HIDL_SUPPORT
    return BluetoothTest::stopBtRelayer();
#else
    return 0;
#endif
}

Return<bool> EmHidlService::btIsBLEEnhancedSupport() {
#ifdef MTK_BT_EM_HIDL_SUPPORT
    return BluetoothTest::isBLEEnhancedSupport();
#else
    return false;
#endif
}

Return<int32_t> EmHidlService::btIsBLESupport() {
#ifdef MTK_BT_EM_HIDL_SUPPORT
    return BluetoothTest::isBLESupport();
#else
    return 0;
#endif
}

Return<int32_t> EmHidlService::btInit() {
    int32_t result = 0;
#ifdef MTK_BT_EM_HIDL_SUPPORT
    result =  BluetoothTest::init();
#endif
    return result;
}
Return<int32_t> EmHidlService::btUninit() {
#ifdef MTK_BT_EM_HIDL_SUPPORT
    return BluetoothTest::uninit();
#else
    return 0;
#endif
}

Return<int32_t> EmHidlService::btDoTest(int32_t kind, int32_t pattern, int32_t channel, int32_t pocketType,
        int32_t pocketTypeLen, int32_t freq, int32_t power) {
#ifdef MTK_BT_EM_HIDL_SUPPORT
    return BluetoothTest::doBtTest(kind, pattern, channel, pocketType, pocketTypeLen, freq, power);
#else
    return 0;
#endif
}

Return<void> EmHidlService::btHciCommandRun(const hidl_vec<uint8_t>& input, btHciCommandRun_cb _hidl_cb) {
#ifdef MTK_BT_EM_HIDL_SUPPORT
    _hidl_cb(BluetoothTest::hciCommandRun(input));
#endif
    return Void();
}

Return<bool> EmHidlService::btStartNoSigRxTest(int32_t pattern, int32_t pockettype,
    int32_t freq, int32_t address) {
#ifdef MTK_BT_EM_HIDL_SUPPORT
    return BluetoothTest::startNoSigRxTest(pattern, pockettype, freq, address);
#else
    return false;
#endif
}

Return<void> EmHidlService::btEndNoSigRxTest(btEndNoSigRxTest_cb _hidl_cb) {
#ifdef MTK_BT_EM_HIDL_SUPPORT
    _hidl_cb(BluetoothTest::endNoSigRxTest());
#endif
    return Void();
}

Return<int32_t> EmHidlService::btIsComboSupport() {
#ifdef MTK_BT_EM_HIDL_SUPPORT
    return BluetoothTest::isComboSupport();
#else
    return 0;
#endif
}

Return<int32_t> EmHidlService::btPollingStart() {
#ifdef MTK_BT_EM_HIDL_SUPPORT
    return BluetoothTest::pollingStart();
#else
    return 0;
#endif
}

Return<int32_t> EmHidlService::btPollingStop() {
#ifdef MTK_BT_EM_HIDL_SUPPORT
    return BluetoothTest::pollingStop();
#else
    return 0;
#endif
}

Return<int32_t> EmHidlService::btIsEmSupport() {
#ifdef MTK_BT_EM_HIDL_SUPPORT
    ALOGI("btIsEmSupport true");
    return 1;
#else
    ALOGI("btIsEmSupport false");
    return 0;
#endif
}

//EM MD log filter
Return<bool> EmHidlService::genMdLogFilter(const hidl_string&,
    const hidl_string&) {
    return true;
}

//EM RSC
Return<bool> EmHidlService::clearItemsforRsc() {
    return true;
}

Return<bool> EmHidlService::setEmConfigure(const hidl_string& name,const hidl_string& value) {
    bool ret = true;
    ALOGI("setEmConfigure() name = [%s],value = [%s]!!!!", name.c_str(), value.c_str());
    property_set(name.c_str(), value.c_str());
    return ret;
}

//EM NFC support check
Return<int32_t> EmHidlService::isNfcSupport() {
#ifdef ST_NFC_SUPPORT
    ALOGI("isNfcSupport true");
    return 1;
#else
    ALOGI("isNfcSupport false");
    return 0;
#endif
}

//EM Power GAUGE 30 support check
Return<int32_t> EmHidlService::isGauge30Support() {
#ifdef MTK_GAUGE_VERSION
    return 1;
#else
    return 0;
#endif
}
}  // implementation
}  // namespace V1_0
}  // namespace log
}  // namespace hardware
}  // namespace mediatek
}  // namespace vendor

