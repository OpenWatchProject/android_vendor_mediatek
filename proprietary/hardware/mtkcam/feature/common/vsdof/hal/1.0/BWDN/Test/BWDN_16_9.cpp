#include "BWDN_HAL_UT.h"

class BWDN_16_9 : public BWDNHALUTBase
{
public:
    BWDN_16_9() { init(); }

    virtual ~BWDN_16_9() {}
protected:
    virtual ENUM_STEREO_RATIO           getImageRatio()     { return eRatio_16_9; }
    virtual ENUM_STEREO_SENSOR_PROFILE  getStereoProfile()  { return STEREO_SENSOR_PROFILE_REAR_REAR; }
    virtual const char *                getUTCaseName()     { return "BWDN_16_9"; }
};

TEST_F(BWDN_16_9, TEST)
{
    if(!_isReadyToUT) {
        return;
    }

    if(!_isReadyToUT) {
        return;
    }

    _pBWDNHAL->BWDNHALRun(_input, _output);

    void* prVa;
    lockGraphicBuffer(_output.outBuffer, prVa);

    EXPECT_TRUE(!isResultEmpty(prVa));
    EXPECT_TRUE(isBitTrue(prVa));

    unlockGraphicBuffer(_output.outBuffer);
}
