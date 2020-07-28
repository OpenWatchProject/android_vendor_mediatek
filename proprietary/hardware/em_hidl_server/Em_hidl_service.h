
#include <string.h>
#include <vendor/mediatek/hardware/engineermode/1.2/IEmd.h>
namespace vendor {
namespace mediatek {
namespace hardware {
namespace engineermode {
namespace V1_2 {
namespace implementation {

using ::android::sp;
using ::android::wp;

using ::android::hardware::Return;
using ::android::hardware::Void;
using ::android::hardware::hidl_string;
using ::android::hardware::hidl_vec;
using ::vendor::mediatek::hardware::engineermode::V1_0::IEmCallback;

class EmHidlService: public IEmd {
public:
    EmHidlService(const char* name);
    ~EmHidlService();
    //EM hidl V1.0
    virtual Return<void> setCallback(const sp<IEmCallback>& callback) override;
    virtual Return<bool> sendToServer(const hidl_string& data) override;
    virtual Return<bool> sendToServerWithCallBack(const hidl_string& data, const sp<IEmCallback>& callback) override;

    //EM telephony property set
    virtual Return<bool> setSmsFormat(const hidl_string& data) override;
    virtual Return<bool> setCtIREngMode(const hidl_string& data) override;
    virtual Return<bool> setTestSimCardType(const hidl_string& data) override;
    virtual Return<bool> setPreferGprsMode(const hidl_string& data) override;

    virtual Return<bool> setRadioCapabilitySwitchEnable(const hidl_string& data) override;
    virtual Return<bool> setDisableC2kCap(const hidl_string& data) override;
    virtual Return<bool> setImsTestMode(const hidl_string& data) override;
    virtual Return<bool> setDsbpSupport(const hidl_string& data) override;
    virtual Return<bool> setVolteMalPctid(const hidl_string& data) override;

    //EM BT
    virtual Return<int32_t> btStartRelayer(int32_t port, int32_t speed) override;
    virtual Return<int32_t> btStopRelayer() override;
    virtual Return<int32_t> btIsBLESupport() override;
    virtual Return<bool> btIsBLEEnhancedSupport() override;
    virtual Return<int32_t> btInit() override;
    virtual Return<int32_t> btUninit() override;
    virtual Return<int32_t> btDoTest(int32_t kind, int32_t pattern, int32_t channel,
            int32_t pocketType, int32_t pocketTypeLen, int32_t freq, int32_t power) override;
    virtual Return<void> btHciCommandRun(const hidl_vec<uint8_t>& input,
            btHciCommandRun_cb _hidl_cb) override;
    virtual Return<bool> btStartNoSigRxTest(int32_t pattern, int32_t pockettype,
        int32_t freq, int32_t address) override;
    virtual Return<void> btEndNoSigRxTest(btEndNoSigRxTest_cb _hidl_cb) override;
    virtual Return<int32_t> btIsComboSupport() override;
    virtual Return<int32_t> btPollingStart() override;
    virtual Return<int32_t> btPollingStop() override;

    //EM log and debug property set
    virtual Return<bool> setMdResetDelay(const hidl_string& data) override;
    virtual Return<bool> setWcnCoreDump(const hidl_string& data) override;
    virtual Return<bool> setOmxVencLogEnable(const hidl_string& data) override;
    virtual Return<bool> setOmxVdecLogEnable(const hidl_string& data) override;
    virtual Return<bool> setVdecDriverLogEnable(const hidl_string& data) override;
    virtual Return<bool> setSvpLogEnable(const hidl_string& data) override;
    virtual Return<bool> setOmxCoreLogEnable(const hidl_string& data) override;
    virtual Return<bool> setVencDriverLogEnable(const hidl_string& data) override;
    virtual Return<bool> setModemWarningEnable(const hidl_string& data) override;


    //EM MD log filter
    virtual Return<bool> genMdLogFilter(const hidl_string&, const hidl_string&);

    //Hardware-testing UartUsb switch
    virtual Return<bool> setUsbPort(const hidl_string& data) override;

    //Hardware-testing usb
    virtual Return<bool> setUsbOtgSwitch(const hidl_string& data) override;
    virtual Return<bool> setEmUsbValue(const hidl_string& data) override;
    virtual Return<bool> setEmUsbType(const hidl_string& data) override;

    //USB bypass mode
    virtual Return<bool> setBypassEn(const hidl_string& data) override;
    virtual Return<bool> setBypassDis(const hidl_string& data) override;
    virtual Return<bool> setBypassService(const hidl_string& data) override;

    //EM others property set
    virtual Return<bool> setMoms(const hidl_string& data) override;

    //EM RSC
    virtual Return<bool> clearItemsforRsc() override;

    //EM MCF read nvdata directory
    virtual Return<bool> getFilePathListWithCallBack(const hidl_string& data, const sp<IEmCallback>& callback) override;

    //EM hidl V1.1
    virtual Return<bool> setEmConfigure(const hidl_string& name,const hidl_string& value) override;

    //EM hidl V1.2
    virtual Return<int32_t> btIsEmSupport() override;
    virtual Return<int32_t> isNfcSupport() override;
    virtual Return<int32_t> isGauge30Support() override;
private:
    char mName[64];
    sp<IEmCallback> mLogCallback;
};
extern int cpp_main();
}  // implementation
}  // namespace V1_0
}  // namespace log
}  // namespace hardware
}  // namespace mediatek
}  // namespace vendor
