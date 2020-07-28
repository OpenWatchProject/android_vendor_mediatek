#include "H3D_HAL_UT.h"

class N3D_Preview : public N3DHALUTBase
{
public:
    N3D_Preview() { init(); }
    virtual ~N3D_Preview() {}
protected:
    virtual const char *getUTCaseName() { return "N3D_Preview"; }
};

TEST_F(N3D_Preview, TEST)
{
    _previewParam.rectifyImgMain2  = _initParam.inputImageBuffers[_ringIdx];
    _previewOutput.rectifyImgMain2 = _initParam.outputImageBuffers[_ringIdx];
    _previewOutput.maskMain2       = _initParam.outputMaskBuffers[_ringIdx];
    _pN3DKernel->WarpMain2(_previewParam, _afInfoMain, _afInfoAuxi, _previewOutput);

    //Main1 must warp after main2
    _pN3DKernel->WarpMain1(_inputImageMain1.get(), _outputImageMain1.get(), _outputMaskMain1.get());

    _pN3DKernel->updateNVRAM(&_nvram);

    //Dump output
    char dumpPath[256];
    MSize dumpSize = _outputImageMain1.get()->getImgSize();
    sprintf(dumpPath, UT_CASE_OUT_FOLDER"/BID_N3D_OUT_MV_Y_%dx%d.yuv", dumpSize.w, dumpSize.h);
    _outputImageMain1.get()->saveToFile(dumpPath);

    dumpSize = _outputMaskMain1.get()->getImgSize();
    sprintf(dumpPath, UT_CASE_OUT_FOLDER"/BID_N3D_OUT_MASK_M_%dx%d.yuv", dumpSize.w, dumpSize.h);
    _outputMaskMain1.get()->saveToFile(dumpPath);

    dumpSize = _previewOutput.rectifyImgMain2->getImgSize();
    sprintf(dumpPath, UT_CASE_OUT_FOLDER"/BID_N3D_OUT_SV_Y_%dx%d.yuv", dumpSize.w, dumpSize.h);
    _previewOutput.rectifyImgMain2->saveToFile(dumpPath);

    dumpSize = _previewOutput.maskMain2->getImgSize();
    sprintf(dumpPath, UT_CASE_OUT_FOLDER"/BID_N3D_OUT_MASK_S_%dx%d.yuv", dumpSize.w, dumpSize.h);
    _previewOutput.maskMain2->saveToFile(dumpPath);

    // EXPECT_TRUE(!isResultEmpty());
    // EXPECT_TRUE(isBitTrue());
}
