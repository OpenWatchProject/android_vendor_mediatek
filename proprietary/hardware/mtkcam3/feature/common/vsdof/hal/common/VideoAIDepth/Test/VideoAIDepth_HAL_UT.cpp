#include "VideoAIDepth_HAL_UT.h"

class VideoAIDepthHALUT : public VideoAIDepthHALUTBase
{
public:
    VideoAIDepthHALUT() { init(); }
    virtual ~VideoAIDepthHALUT() {}
protected:
};

TEST_F(VideoAIDepthHALUT, TEST)
{
    if(!_isReadyToUT) {
        return;
    }

    _pAIDepthHAL->VideoAIDepthHALRun(_input, _output);

    char dumpPath[256];
    MSize dumpSize = _output.depthMap->getImgSize();
    sprintf(dumpPath, UT_CASE_OUT_FOLDER"/BID_AIDEPTH_OUT_DEPTHMAP_reqID_3000__%dx%d_8_s0.unknown", dumpSize.w, dumpSize.h);
    _output.depthMap->saveToFile(dumpPath);

    EXPECT_TRUE(!isResultEmpty());
    EXPECT_TRUE(isBitTrue());
}
