#include "Bokeh_HAL_UT.h"

class Bokeh_16_9_rotate_0 : public BokehHALUTBase
{
public:
    Bokeh_16_9_rotate_0() { init(); }
    virtual ~Bokeh_16_9_rotate_0() {}
protected:
    virtual ENUM_STEREO_RATIO           getImageRatio()         { return eRatio_16_9; }
    virtual ENUM_STEREO_SENSOR_PROFILE  getStereoProfile()      { return STEREO_SENSOR_PROFILE_REAR_REAR; }
    virtual int                         getCaptureOrientation() { return 0; }
    //(1658, 506) (2040,888), center: (1849, 697)
    virtual AF_WIN_COORDINATE_STRUCT    getFocusROI()           { return AF_WIN_COORDINATE_STRUCT(79, -414, 328, 27); }
    virtual int                         getDOFLevel()           { return 25; }
    virtual const char *                getUTCaseName()         { return "Bokeh_16_9_rotate_0"; }
};

TEST_F(Bokeh_16_9_rotate_0, TEST)
{
    if(!_isReadyToUT) {
        return;
    }

    _pBokehHAL->Run(_input, _output);

    EXPECT_TRUE(!isResultEmpty());
    EXPECT_TRUE(isBitTrue());
}
