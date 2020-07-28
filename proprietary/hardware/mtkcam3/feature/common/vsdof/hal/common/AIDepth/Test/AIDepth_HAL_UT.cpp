#include "AIDepth_HAL_UT.h"

class AIDepthHALUT : public AIDepthHALUTBase
{
public:
    AIDepthHALUT() { init(); }
    virtual ~AIDepthHALUT() {}
protected:
};

TEST_F(AIDepthHALUT, TEST)
{
    if(!_isReadyToUT) {
        return;
    }

    _pAIDepthHAL->AIDepthHALRun(_input, _output);

    char dumpPath[256];
    MSize dumpSize = _output.depthMap->getImgSize();
    sprintf(dumpPath, UT_CASE_OUT_FOLDER"/BID_AIDEPTH_OUT_DEPTHMAP_reqID_3000__%dx%d_8_s0.unknown", dumpSize.w, dumpSize.h);
    _output.depthMap->saveToFile(dumpPath);

    EXPECT_TRUE(!isResultEmpty());
    EXPECT_TRUE(isBitTrue());
}
