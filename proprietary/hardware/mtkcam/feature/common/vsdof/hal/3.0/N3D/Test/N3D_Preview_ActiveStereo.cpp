#include "H3D_HAL_UT.h"

class N3D_Preview_ActiveStereo : public N3DHALUTBase
{
public:
    N3D_Preview_ActiveStereo() { init(); }
    virtual ~N3D_Preview_ActiveStereo() {}
protected:
    virtual const char *getUTCaseName() { return "N3D_Preview_ActiveStereo"; }
    virtual int getStereoFeatureMode() { return E_STEREO_FEATURE_ACTIVE_STEREO; }
};

TEST_F(N3D_Preview_ActiveStereo, TEST)
{
    //For active stereo, WarpMain2 will complete main1 and main2 warping at once
    _pN3DKernel->WarpMain2(_previewParam, _afInfoMain, _afInfoAuxi, _previewOutput);

    _pN3DKernel->updateNVRAM(&_nvram);

    //Dump output
    char dumpPath[256];
    MSize dumpSize = _warpMapMain1.get()->getImgSize();
    sprintf(dumpPath, UT_CASE_OUT_FOLDER"/BID_N3D_OUT_WARP_M_%dx%d.yuv", dumpSize.w, dumpSize.h);
    _warpMapMain1.get()->saveToFile(dumpPath);

    dumpSize = _warpMapMain2.get()->getImgSize();
    sprintf(dumpPath, UT_CASE_OUT_FOLDER"/BID_N3D_OUT_WARP_S_%dx%d.yuv", dumpSize.w, dumpSize.h);
    _warpMapMain2.get()->saveToFile(dumpPath);

    // EXPECT_TRUE(!isResultEmpty());
    // EXPECT_TRUE(isBitTrue());
}
