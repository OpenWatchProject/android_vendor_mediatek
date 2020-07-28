#ifndef ANDROID_WCN_CHIP_CONTROLLER_H
#define ANDROID_WCN_CHIP_CONTROLLER_H

#include "AudioType.h"
#include <linux/fm.h>

#include <utils/threads.h>

namespace android {

class WCNChipController {
public:
    virtual ~WCNChipController();

    static WCNChipController *GetInstance();

    virtual bool     GetFmChipPowerInfo();

    virtual status_t SetFmChipVolume(const uint32_t fm_chip_volume);
    virtual status_t SetFmChipSampleRate(const uint32_t sample_rate);


    virtual status_t InitAudioFMInfo();
    virtual status_t InitAudioBTInfo();

    virtual bool     IsFMMergeInterfaceSupported();
    virtual bool     IsBTMergeInterfaceSupported();

    virtual bool     IsFmChipPadSelConnSys();
    virtual bool     IsFmChipUseSlaveMode();
    virtual uint32_t GetFmChipSamplingRate();


    virtual uint32_t BTChipHWInterface();
    virtual bool     BTUseCVSDRemoval();

    virtual uint32_t BTChipSamplingRate();
    virtual uint32_t BTChipSamplingRateNumber();

    virtual uint32_t BTChipSyncFormat();
    virtual uint32_t BTChipSyncLength();
    virtual uint32_t BTChipSecurityHiLo();


    // might be configured to 8k/16k according to bt device.
    virtual void     SetBTCurrentSamplingRateNumber(const uint32_t sample_rate);
    virtual uint32_t GetBTCurrentSamplingRateNumber();
    virtual bool IsSupportFM();

protected:
    WCNChipController();

    Mutex            mLock;

    bool             mInitAudioFMInfoFlag;
    bool             mInitAudioBTInfoFlag;
    bool             mPlatformSupportFM;
    fm_audio_info_t  mFmAudioInfo;
    //AUDIO_CONFIG     mBTAudioInfo;

    uint32_t         mBTCurrentSamplingRateNumber;



private:
    static WCNChipController *mWCNChipController; // singleton
    bool IsCustomizedPlatformSupportFM();
};

} // end namespace android

#endif // end of ANDROID_WCN_CHIP_CONTROLLER_H
