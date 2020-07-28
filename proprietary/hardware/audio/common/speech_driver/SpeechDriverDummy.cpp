#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "SpeechDriverDummy"
#include <SpeechDriverDummy.h>
#include <utils/threads.h>

namespace android {

/*==============================================================================
 *                     Constructor / Destructor / Init / Deinit
 *============================================================================*/

SpeechDriverDummy::SpeechDriverDummy(modem_index_t modem_index) {
    ALOGW("%s(), modem_index = %d", __FUNCTION__, modem_index);
}

SpeechDriverDummy::~SpeechDriverDummy() {
    ALOGW("%s()", __FUNCTION__);
}

/*==============================================================================
 *                     Speech Control
 *============================================================================*/

status_t SpeechDriverDummy::SetSpeechMode(const audio_devices_t input_device, const audio_devices_t output_device) {
    ALOGW("%s(), input_device = 0x%x, output_device = 0x%x", __FUNCTION__, input_device, output_device);
    return INVALID_OPERATION;
}

status_t SpeechDriverDummy::setMDVolumeIndex(int stream, int device, int index) {
    ALOGV("+%s() stream=%d, device=%d, index=%d", __FUNCTION__, stream, device, index);
    return INVALID_OPERATION;
}

status_t SpeechDriverDummy::SpeechOn() {
    ALOGW("%s()", __FUNCTION__);
    CheckApSideModemStatusAllOffOrDie();
    SetApSideModemStatus(SPEECH_STATUS_MASK);
    return INVALID_OPERATION;
}

status_t SpeechDriverDummy::SpeechOff() {
    ALOGW("%s()", __FUNCTION__);
    ResetApSideModemStatus(SPEECH_STATUS_MASK);
    CheckApSideModemStatusAllOffOrDie();
    return INVALID_OPERATION;
}

status_t SpeechDriverDummy::VideoTelephonyOn() {
    ALOGW("%s()", __FUNCTION__);
    CheckApSideModemStatusAllOffOrDie();
    SetApSideModemStatus(VT_STATUS_MASK);
    return INVALID_OPERATION;
}

status_t SpeechDriverDummy::VideoTelephonyOff() {
    ALOGW("%s()", __FUNCTION__);
    ResetApSideModemStatus(VT_STATUS_MASK);
    CheckApSideModemStatusAllOffOrDie();
    return INVALID_OPERATION;
}

status_t SpeechDriverDummy::SpeechRouterOn() {
    ALOGW("%s()", __FUNCTION__);
    CheckApSideModemStatusAllOffOrDie();
    SetApSideModemStatus(SPEECH_ROUTER_STATUS_MASK);
    return INVALID_OPERATION;
}

status_t SpeechDriverDummy::SpeechRouterOff() {
    ALOGW("%s()", __FUNCTION__);
    ResetApSideModemStatus(SPEECH_ROUTER_STATUS_MASK);
    CheckApSideModemStatusAllOffOrDie();
    return INVALID_OPERATION;
}

/*==============================================================================
 *                     Recording Control
 *============================================================================*/

status_t SpeechDriverDummy::recordOn() {
    ALOGW("%s()", __FUNCTION__);
    SetApSideModemStatus(RECORD_STATUS_MASK);
    return INVALID_OPERATION;
}

status_t SpeechDriverDummy::recordOn(SpcRecordTypeStruct typeRecord) {
    ALOGW("%s() typeRecord=%d", __FUNCTION__, typeRecord.direction);
    SetApSideModemStatus(RAW_RECORD_STATUS_MASK);
    return INVALID_OPERATION;
}

status_t SpeechDriverDummy::recordOff() {
    ALOGW("%s()", __FUNCTION__);
    ResetApSideModemStatus(RECORD_STATUS_MASK);
    return INVALID_OPERATION;
}

status_t SpeechDriverDummy::recordOff(SpcRecordTypeStruct typeRecord) {
    ALOGW("%s() typeRecord=%d", __FUNCTION__, typeRecord.direction);
    ResetApSideModemStatus(RAW_RECORD_STATUS_MASK);
    return INVALID_OPERATION;
}

status_t SpeechDriverDummy::setPcmRecordType(SpcRecordTypeStruct typeRecord) {
    ALOGW("%s() typeRecord=%d", __FUNCTION__, typeRecord.direction);
    return INVALID_OPERATION;
}

status_t SpeechDriverDummy::VoiceMemoRecordOn() {
    ALOGW("%s()", __FUNCTION__);
    SetApSideModemStatus(VM_RECORD_STATUS_MASK);
    return INVALID_OPERATION;
}

status_t SpeechDriverDummy::VoiceMemoRecordOff() {
    ALOGW("%s()", __FUNCTION__);
    ResetApSideModemStatus(VM_RECORD_STATUS_MASK);
    return INVALID_OPERATION;
}

uint16_t SpeechDriverDummy::GetRecordSampleRate() const {
    ALOGW("%s(), num_sample_rate = 8000", __FUNCTION__);
    return 8000;
}

uint16_t SpeechDriverDummy::GetRecordChannelNumber() const {
    ALOGW("%s(), num_channel = 1", __FUNCTION__);
    return 1;
}

/*==============================================================================
 *                     Background Sound
 *============================================================================*/

status_t SpeechDriverDummy::BGSoundOn() {
    ALOGW("%s()", __FUNCTION__);
    SetApSideModemStatus(BGS_STATUS_MASK);
    return INVALID_OPERATION;
}

status_t SpeechDriverDummy::BGSoundConfig(uint8_t ul_gain, uint8_t dl_gain) {
    ALOGW("%s(), ul_gain = 0x%x, dl_gain = 0x%x", __FUNCTION__, ul_gain, dl_gain);
    return INVALID_OPERATION;
}

status_t SpeechDriverDummy::BGSoundOff() {
    ALOGW("%s()", __FUNCTION__);
    ResetApSideModemStatus(BGS_STATUS_MASK);
    return INVALID_OPERATION;
}

/*==============================================================================
 *                     PCM 2 Way
 *============================================================================*/

status_t SpeechDriverDummy::PCM2WayOn(const bool wideband_on) {
    ALOGW("%s() wideband_on=%d", __FUNCTION__, wideband_on);
    SetApSideModemStatus(P2W_STATUS_MASK);
    return INVALID_OPERATION;
}

status_t SpeechDriverDummy::PCM2WayOff() {
    ALOGW("%s()", __FUNCTION__);
    ResetApSideModemStatus(P2W_STATUS_MASK);
    return INVALID_OPERATION;
}


/*==============================================================================
 *                     TTY-CTM Control
 *============================================================================*/
status_t SpeechDriverDummy::TtyCtmOn() {
    ALOGW("%s()", __FUNCTION__);
    SetApSideModemStatus(TTY_STATUS_MASK);
    return INVALID_OPERATION;
}

status_t SpeechDriverDummy::TtyCtmOff() {
    ALOGW("%s()", __FUNCTION__);
    ResetApSideModemStatus(TTY_STATUS_MASK);
    return INVALID_OPERATION;
}

/*==============================================================================
 *                     Modem Audio DVT and Debug
 *============================================================================*/

status_t SpeechDriverDummy::SetModemLoopbackPoint(uint16_t loopback_point) {
    ALOGW("%s(), loopback_point = %d", __FUNCTION__, loopback_point);
    return INVALID_OPERATION;
}


/*==============================================================================
 *                     Acoustic Loopback
 *============================================================================*/

status_t SpeechDriverDummy::SetAcousticLoopback(bool loopback_on) {
    ALOGW("%s(), loopback_on = %d", __FUNCTION__, loopback_on);

    if (loopback_on == true) {
        CheckApSideModemStatusAllOffOrDie();
        SetApSideModemStatus(LOOPBACK_STATUS_MASK);
    } else {
        ResetApSideModemStatus(LOOPBACK_STATUS_MASK);
        CheckApSideModemStatusAllOffOrDie();
    }

    return INVALID_OPERATION;
}

status_t SpeechDriverDummy::SetAcousticLoopbackBtCodec(bool enable_codec) {
    ALOGW("%s(), enable_codec = %d", __FUNCTION__, enable_codec);
    return INVALID_OPERATION;
}

status_t SpeechDriverDummy::SetAcousticLoopbackDelayFrames(int32_t delay_frames) {
    ALOGW("%s(), delay_frames = %d", __FUNCTION__, delay_frames);
    return INVALID_OPERATION;
}

/*==============================================================================
 *                     Volume Control
 *============================================================================*/

status_t SpeechDriverDummy::SetDownlinkGain(int16_t gain) {
    ALOGW("%s(), gain = 0x%x, old mDownlinkGain = 0x%x", __FUNCTION__, gain, mDownlinkGain);
    return INVALID_OPERATION;
}

status_t SpeechDriverDummy::SetEnh1DownlinkGain(int16_t gain) {
    ALOGW("%s(), gain = 0x%x, old SetEnh1DownlinkGain = 0x%x", __FUNCTION__, gain, mDownlinkenh1Gain);
    return INVALID_OPERATION;
}

status_t SpeechDriverDummy::SetUplinkGain(int16_t gain) {
    ALOGW("%s(), gain = 0x%x, old mUplinkGain = 0x%x", __FUNCTION__, gain, mUplinkGain);
    return INVALID_OPERATION;
}

status_t SpeechDriverDummy::SetDownlinkMute(bool mute_on) {
    ALOGW("%s(), mute_on = %d, old mDownlinkMuteOn = %d", __FUNCTION__, mute_on, mDownlinkMuteOn);
    return INVALID_OPERATION;
}

status_t SpeechDriverDummy::SetDownlinkMuteCodec(bool mute_on) {
    ALOGW("%s(), mute_on = %d, old mDownlinkMuteOn = %d", __FUNCTION__, mute_on, mDownlinkMuteOn);
    return INVALID_OPERATION;
}

status_t SpeechDriverDummy::SetUplinkMute(bool mute_on) {
    ALOGW("%s(), mute_on = %d, old mUplinkMuteOn = %d", __FUNCTION__, mute_on, mUplinkMuteOn);
    return INVALID_OPERATION;
}

status_t SpeechDriverDummy::SetUplinkSourceMute(bool mute_on) {
    ALOGW("%s(), mute_on = %d, old mUplinkSourceMuteOn = %d", __FUNCTION__, mute_on, mUplinkSourceMuteOn);
    return INVALID_OPERATION;
}

status_t SpeechDriverDummy::SetSidetoneGain(int16_t gain) {
    ALOGW("%s(), gain = 0x%x, old mSideToneGain = 0x%x", __FUNCTION__, gain, mSideToneGain);
    return INVALID_OPERATION;
}

/*==============================================================================
 *                     Device related Config
 *============================================================================*/

status_t SpeechDriverDummy::SetModemSideSamplingRate(uint16_t sample_rate) {
    ALOGW("%s(), sample_rate = %d", __FUNCTION__, sample_rate);
    return INVALID_OPERATION;
}

/*==============================================================================
 *                     Speech Enhancement Control
 *============================================================================*/
status_t SpeechDriverDummy::SetSpeechEnhancement(bool enhance_on) {
    ALOGW("%s(), enhance_on = %d", __FUNCTION__, enhance_on);
    return INVALID_OPERATION;
}

status_t SpeechDriverDummy::SetSpeechEnhancementMask(const sph_enh_mask_struct_t &mask) {
    ALOGW("%s(), main_func = 0x%x, dynamic_func = 0x%x", __FUNCTION__, mask.main_func, mask.dynamic_func);
    return INVALID_OPERATION;
}

status_t SpeechDriverDummy::SetBtHeadsetNrecOn(const bool bt_headset_nrec_on) {
    ALOGW("%s(), bt_headset_nrec_on = %d", __FUNCTION__, bt_headset_nrec_on);
    return INVALID_OPERATION;
}

/*==============================================================================
 *                     Recover State
 *============================================================================*/

void SpeechDriverDummy::RecoverModemSideStatusToInitState() {
    ALOGW("%s()", __FUNCTION__);
}

/*==============================================================================
 *                     Check Modem Status
 *============================================================================*/
bool SpeechDriverDummy::CheckModemIsReady() {
    ALOGW("%s()", __FUNCTION__);
    return false;
};


} // end of namespace android

