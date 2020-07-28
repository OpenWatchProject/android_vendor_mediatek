#include "StereoSizeProviderUT.h"

#define MY_LOGD(fmt, arg...)    printf("[D][%s]" fmt"\n", __func__, ##arg)
#define MY_LOGI(fmt, arg...)    printf("[I][%s]" fmt"\n", __func__, ##arg)
#define MY_LOGW(fmt, arg...)    printf("[W][%s] WRN(%5d):" fmt"\n", __func__, __LINE__, ##arg)
#define MY_LOGE(fmt, arg...)    printf("[E][%s] %s ERROR(%5d):" fmt"\n", __func__,__FILE__, __LINE__, ##arg)

class StereoSizeProviderUT : public StereoSizeProviderUTBase
{
public:
    StereoSizeProviderUT() : StereoSizeProviderUTBase() { init(); }

    virtual ~StereoSizeProviderUT() {}

    virtual bool ignoreTest()
    {
        return false;
    }
};

//=============================================================================
//  PASS 1
//=============================================================================
TEST_F(StereoSizeProviderUT, TEST)
{
    if(ignoreTest()) {
        printf("Skip this test\n");
        return;
    }

    //=============================================================================
    //  Pass 1
    //=============================================================================
    print("Main1 TG Crop", tgCropRect[0]);
    print("Main1 IMGO   ", szIMGO[0]);
    print("Main1 RRZO   ", szRRZO[0]);
    print("Main2 TG Crop", tgCropRect[1]);
    print("Main2 IMGO   ", szIMGO[1]);
    print("Main2 RRZO   ", szRRZO[1]);

    //=============================================================================
    //  Pass 2
    //=============================================================================
    //Pass2-A
    print("PASS2A.WDMA", pass2SizeInfo[PASS2A].areaWDMA);
    print("PASS2A.WROT", pass2SizeInfo[PASS2A].areaWROT);

    // PASS2A'
    print("PASS2A_P.WROT", pass2SizeInfo[PASS2A_P].areaWROT);
    print("PASS2A_P.FEO ", pass2SizeInfo[PASS2A_P].areaFEO);

    // PASS2A-2
    print("PASS2A_2.WDMA ", pass2SizeInfo[PASS2A_2].areaWDMA);
    print("PASS2A_2.IMG2O", pass2SizeInfo[PASS2A_2].areaIMG2O);
    print("PASS2A_2.FEO  ", pass2SizeInfo[PASS2A_2].areaFEO);

    // PASS2A'-2
    print("PASS2A_P_2.WDMA ", pass2SizeInfo[PASS2A_P_2].areaWDMA);
    print("PASS2A_P_2.IMG2O", pass2SizeInfo[PASS2A_P_2].areaIMG2O);
    print("PASS2A_P_2.FEO  ", pass2SizeInfo[PASS2A_P_2].areaFEO);

    // PASS2A-3
    print("PASS2A_3.IMG2O", pass2SizeInfo[PASS2A_3].areaIMG2O);
    print("PASS2A_2.FEO  ", pass2SizeInfo[PASS2A_3].areaFEO);

    // PASS2A'-3
    print("PASS2A_P_3.IMG2O", pass2SizeInfo[PASS2A_P_3].areaIMG2O);
    print("PASS2A_P_3.FEO  ", pass2SizeInfo[PASS2A_P_3].areaFEO);

    // PASS2A Crop
    print("PASS2A_CROP.WDMA", pass2SizeInfo[PASS2A_CROP].areaWDMA);

    // PASS2A_B Crop
    print("PASS2A_B_CROP.WDMA", pass2SizeInfo[PASS2A_B_CROP].areaWDMA);

    // PASS2A_P Crop
    print("PASS2A_P_CROP.WDMA", pass2SizeInfo[PASS2A_P_CROP].areaWDMA);

    // PASS2B
    print("PASS2B.WROT", sizeProvider->getBufferSize(E_BOKEH_WROT, scenario));
    print("PASS2B.WDMA", sizeProvider->getBufferSize(E_BOKEH_WDMA, scenario));
    print("PASS2B.3DNR", sizeProvider->getBufferSize(E_BOKEH_3DNR, scenario));

    //=============================================================================
    //  Buffers
    //=============================================================================
    //N3D Output
    //N3D before MDP for capture
    if(eSTEREO_SCENARIO_CAPTURE == scenario) {
        print("MV_Y_LARGE    ", sizeProvider->getBufferSize(E_MV_Y_LARGE));
        print("MASK_M_Y_LARGE", sizeProvider->getBufferSize(E_MASK_M_Y_LARGE));
        print("SV_Y_LARGE    ", sizeProvider->getBufferSize(E_SV_Y_LARGE));
        print("MASK_S_Y_LARGE", sizeProvider->getBufferSize(E_MASK_S_Y_LARGE));
    } else {
        print("MV_Y    ", sizeProvider->getBufferSize(E_MV_Y));
        print("MASK_M_Y", sizeProvider->getBufferSize(E_MASK_M_Y));
        print("SV_Y    ", sizeProvider->getBufferSize(E_SV_Y));
        print("MASK_S_Y", sizeProvider->getBufferSize(E_MASK_S_Y));
    }

    //DPE Output
    print("DMP_H", sizeProvider->getBufferSize(E_DMP_H));
    print("CFM_H", sizeProvider->getBufferSize(E_CFM_H));
    print("RESPO", sizeProvider->getBufferSize(E_RESPO));

    //OCC Output
    print("MY_S", sizeProvider->getBufferSize(E_MY_S));
    print("DMH ", sizeProvider->getBufferSize(E_DMH));

    //WMF Output
    print("DMW", sizeProvider->getBufferSize(E_DMW));

    //GF Output
    print("DMG      ", sizeProvider->getBufferSize(E_DMG));
    print("DMBG     ", sizeProvider->getBufferSize(E_DMBG));
    print("DEPTH_MAP", sizeProvider->getBufferSize(E_DEPTH_MAP));
}
