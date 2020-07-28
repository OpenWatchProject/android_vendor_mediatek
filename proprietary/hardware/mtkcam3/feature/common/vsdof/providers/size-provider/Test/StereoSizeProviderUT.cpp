#include "StereoSizeProviderUT.h"

#define MY_LOGD(fmt, arg...)    printf("[D][%s]" fmt"\n ", __func__, ##arg)
#define MY_LOGI(fmt, arg...)    printf("[I][%s]" fmt"\n ", __func__, ##arg)
#define MY_LOGW(fmt, arg...)    printf("[W][%s] WRN(%5d):" fmt"\n   ", __func__, __LINE__, ##arg)
#define MY_LOGE(fmt, arg...)    printf("[E][%s] %s ERROR(%5d):" fmt"\n  ", __func__,__FILE__, __LINE__, ##arg)

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
    print("ActiveArray Main1     ", activityArray[0]);
    print("ActiveArray Main2     ", activityArray[1]);
    printf("----------------------\n");
    print("Main1 TG Crop         ", tgCropRect[0]);
    print("Main1 IMGO            ", szMainIMGO);
    print("Main1 RRZO            ", szRRZO[0]);
    print("Main1 P1 YUV TG Crop  ", tgCropRectYUV);
    print("Main1 P1 YUV Size     ", szP1YUV);
    print("Main1 RSSO_R2 TG Crop ", tgCropRectRSSOR2[0]);
    print("Main1 RSSO_R2         ", szRSSO_R2[0]);
    print("Main2 TG Crop         ", tgCropRect[1]);
    print("Main2 RRZO            ", szRRZO[1]);
    print("Main2 RSSO_R2 TG Crop ", tgCropRectRSSOR2[1]);
    print("Main2 RSSO_R2         ", szRSSO_R2[1]);
    printf("----------------------\n");
    //=============================================================================
    //  Pass 2
    //=============================================================================
    //Pass2-A
    print("PASS2A.WDMA          ", pass2SizeInfo[PASS2A].areaWDMA);
    print("PASS2A.WROT          ", pass2SizeInfo[PASS2A].areaWROT);

    // PASS2A'
    print("PASS2A_P.WROT        ", pass2SizeInfo[PASS2A_P].areaWROT);
    print("PASS2A_P.FEO         ", pass2SizeInfo[PASS2A_P].areaFEO);

    // PASS2A-2
    print("PASS2A_2.WDMA        ", pass2SizeInfo[PASS2A_2].areaWDMA);
    print("PASS2A_2.IMG2O       ", pass2SizeInfo[PASS2A_2].areaIMG2O);
    print("PASS2A_2.FEO         ", pass2SizeInfo[PASS2A_2].areaFEO);

    // PASS2A'-2
    print("PASS2A_P_2.WDMA      ", pass2SizeInfo[PASS2A_P_2].areaWDMA);
    print("PASS2A_P_2.IMG2O     ", pass2SizeInfo[PASS2A_P_2].areaIMG2O);
    print("PASS2A_P_2.FEO       ", pass2SizeInfo[PASS2A_P_2].areaFEO);

    // PASS2A-3
    print("PASS2A_3.IMG2O       ", pass2SizeInfo[PASS2A_3].areaIMG2O);
    print("PASS2A_2.FEO         ", pass2SizeInfo[PASS2A_3].areaFEO);

    // PASS2A'-3
    print("PASS2A_P_3.IMG2O     ", pass2SizeInfo[PASS2A_P_3].areaIMG2O);
    print("PASS2A_P_3.FEO       ", pass2SizeInfo[PASS2A_P_3].areaFEO);

    // PASS2B
    print("PASS2B.WROT          ", sizeProvider->getBufferSize(E_BOKEH_WROT, scenario));
    print("PASS2B.WDMA          ", sizeProvider->getBufferSize(E_BOKEH_WDMA, scenario));
    print("PASS2B.3DNR          ", sizeProvider->getBufferSize(E_BOKEH_3DNR, scenario));

    // PASS2A'-3
    if(eSTEREO_SCENARIO_CAPTURE == scenario) {
        print("PASS2A_CROP.WDMA     ", pass2SizeInfo[PASS2A_CROP].areaWDMA);
        print("PASS2A_B_CROP.WDMA   ", pass2SizeInfo[PASS2A_B_CROP].areaWDMA);
        print("PASS2A_P_CROP.WDMA   ", pass2SizeInfo[PASS2A_P_CROP].areaWDMA);
    }
    printf("----------------------\n");

    //=============================================================================
    //  Buffers
    //=============================================================================
    //N3D Input
    print("RECT_IN_M            ", sizeProvider->getBufferSize(E_RECT_IN_M,    scenario));
    print("MASK_IN_M            ", sizeProvider->getBufferSize(E_MASK_IN_M,    scenario));
    print("RECT_IN_S            ", sizeProvider->getBufferSize(E_RECT_IN_S,    scenario));
    print("MASK_IN_S            ", sizeProvider->getBufferSize(E_MASK_IN_S,    scenario));
    printf("----------------------\n");
    //N3D Output
    print("MV_Y                 ", sizeProvider->getBufferSize(E_MV_Y,         scenario));
    print("MASK_M_Y             ", sizeProvider->getBufferSize(E_MASK_M_Y,     scenario));
    print("SV_Y                 ", sizeProvider->getBufferSize(E_SV_Y,         scenario));
    print("MASK_S_Y             ", sizeProvider->getBufferSize(E_MASK_S_Y,     scenario));

    print("E_WARP_MAP_M         ", sizeProvider->getBufferSize(E_WARP_MAP_M,   scenario));
    print("E_WARP_MAP_S         ", sizeProvider->getBufferSize(E_WARP_MAP_S,   scenario));
    printf("----------------------\n");

    //DL Depth
    //P2A Output
    if(eSTEREO_SCENARIO_CAPTURE == scenario) {
        print("DLD_P2A_M            ", sizeProvider->getBufferSize(E_DLD_P2A_M));
        print("DLD_P2A_S            ", sizeProvider->getBufferSize(E_DLD_P2A_S));
        //Final output
        print("DL_DEPTHMAP          ", sizeProvider->getBufferSize(E_DL_DEPTHMAP));
        printf("----------------------\n");
    } else {
        //DPE Output
        print("MY_S                 ", sizeProvider->getBufferSize(E_MY_S,         scenario));

        print("[DPE2]CFM_M          ", sizeProvider->getBufferSize(E_CFM_M,        scenario));
        print("[DPE2]DV_LR          ", sizeProvider->getBufferSize(E_DV_LR,        scenario));
        print("[DPE2]ASF_HF         ", sizeProvider->getBufferSize(E_ASF_HF,       scenario));
        print("[DPE2]ASF_CRM        ", sizeProvider->getBufferSize(E_ASF_CRM,      scenario));
        print("[DPE2]ASF_RD         ", sizeProvider->getBufferSize(E_ASF_RD,       scenario));

        print("[DPE]DMP_H           ", sizeProvider->getBufferSize(E_DMP_H,        scenario));
        print("[DPE]RESPO           ", sizeProvider->getBufferSize(E_RESPO,        scenario));
        print("[DPE]DMH             ", sizeProvider->getBufferSize(E_DMH,          scenario));

        print("NOC                  ", sizeProvider->getBufferSize(E_NOC,          scenario));
        print("CFM_H                ", sizeProvider->getBufferSize(E_CFM_H,        scenario));
        print("DMW                  ", sizeProvider->getBufferSize(E_DMW,          scenario));

        //GF Output
        print("DMG                  ", sizeProvider->getBufferSize(E_DMG,          scenario));
        print("DMBG                 ", sizeProvider->getBufferSize(E_DMBG,         scenario));
        print("DEPTH_MAP            ", sizeProvider->getBufferSize(E_DEPTH_MAP,    scenario));
    }

    if(eSTEREO_SCENARIO_RECORD == scenario)
    {
        print("VAIDEPTH_DEPTHMAP    ", sizeProvider->getBufferSize(E_VAIDEPTH_DEPTHMAP,    scenario));
    }
}
