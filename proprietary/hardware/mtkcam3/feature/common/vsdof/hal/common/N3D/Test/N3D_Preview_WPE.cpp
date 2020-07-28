#include "H3D_HAL_UT_WPE.h"

class N3D_Preview_WPE : public N3DHALUTWPEBase
{
public:
    N3D_Preview_WPE() { init(); }
    virtual ~N3D_Preview_WPE() {}
protected:
    virtual const char *getUTCaseName() { return "N3D_Preview_WPE"; }
};

TEST_F(N3D_Preview_WPE, TEST)
{
    _pN3DKernel->WarpMain1(_inputImageMain1.get(), _outputImageMain1.get(), _outputMaskMain1.get());
    // _pN3DKernel->WarpMain2(_previewParam, _afInfoMain, _afInfoAuxi, _previewOutput);
    // _pN3DKernel->runLearning(_hwfefmData);

    // _pN3DKernel->updateNVRAM(&_nvram);

    //Dump output
    char dumpPath[256];
    MSize dumpSize = _outputImageMain1.get()->getImgSize();
    sprintf(dumpPath, UT_CASE_OUT_FOLDER"/BID_N3D_OUT_MV_Y_reqID_0__%dx%d_8_s0.yv12", dumpSize.w, dumpSize.h);
    _outputImageMain1.get()->saveToFile(dumpPath);

    dumpSize = _outputMaskMain1.get()->getImgSize();
    sprintf(dumpPath, UT_CASE_OUT_FOLDER"/BID_N3D_OUT_MASK_M_reqID_0__%dx%d_8_s0.unknown", dumpSize.w, dumpSize.h);
    _outputMaskMain1.get()->saveToFile(dumpPath);

    // dumpSize = _previewOutput.warpMapMain2[0]->getImgSize();
    // sprintf(dumpPath, UT_CASE_OUT_FOLDER"/BID_N3D_OUT_WARP_MAP_S_X_%dx%d_8_s0.unknown", dumpSize.w, dumpSize.h);
    // _previewOutput.warpMapMain2[0]->saveToFile(dumpPath);
    // dumpSize = _previewOutput.warpMapMain2[1]->getImgSize();
    // sprintf(dumpPath, UT_CASE_OUT_FOLDER"/BID_N3D_OUT_WARP_MAP_S_Y_%dx%d_8_s0.unknown", dumpSize.w, dumpSize.h);
    // _previewOutput.warpMapMain2[1]->saveToFile(dumpPath);

    // dumpSize = _previewOutput.warpMapMain1[0]->getImgSize();
    // sprintf(dumpPath, UT_CASE_OUT_FOLDER"/BID_N3D_OUT_WARP_MAP_M_X_%dx%d.yuv", dumpSize.w, dumpSize.h);
    // _previewOutput.warpMapMain1[0]->saveToFile(dumpPath);
    // dumpSize = _previewOutput.warpMapMain1[1]->getImgSize();
    // sprintf(dumpPath, UT_CASE_OUT_FOLDER"/BID_N3D_OUT_WARP_MAP_M_Y_%dx%d.yuv", dumpSize.w, dumpSize.h);
    // _previewOutput.warpMapMain1[1]->saveToFile(dumpPath);

    // EXPECT_TRUE(!isResultEmpty());
    // EXPECT_TRUE(isBitTrue());
}
