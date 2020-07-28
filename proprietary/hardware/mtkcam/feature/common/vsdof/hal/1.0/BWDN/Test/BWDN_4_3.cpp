#include "BWDN_HAL_UT.h"

class BWDN_4_3 : public BWDNHALUTBase
{
public:
    BWDN_4_3() { init(); }

    virtual ~BWDN_4_3() {}
protected:
    virtual ENUM_STEREO_RATIO           getImageRatio()     { return eRatio_4_3; }
    virtual ENUM_STEREO_SENSOR_PROFILE  getStereoProfile()  { return STEREO_SENSOR_PROFILE_REAR_REAR; }
    virtual const char *                getUTCaseName()     { return "BWDN_4_3"; }
};

TEST_F(BWDN_4_3, TEST)
{
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
