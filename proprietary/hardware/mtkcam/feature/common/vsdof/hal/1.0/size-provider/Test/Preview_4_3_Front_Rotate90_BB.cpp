#include "StereoSizeProviderUT.h"

#define MY_LOGD(fmt, arg...)    printf("[D][%s]" fmt"\n", __func__, ##arg)
#define MY_LOGI(fmt, arg...)    printf("[I][%s]" fmt"\n", __func__, ##arg)
#define MY_LOGW(fmt, arg...)    printf("[W][%s] WRN(%5d):" fmt"\n", __func__, __LINE__, ##arg)
#define MY_LOGE(fmt, arg...)    printf("[E][%s] %s ERROR(%5d):" fmt"\n", __func__,__FILE__, __LINE__, ##arg)

class Preview_4_3_Front_Rotate90_BB : public StereoSizeProviderUTBase
{
public:
    Preview_4_3_Front_Rotate90_BB() : StereoSizeProviderUTBase() { init(); }

    virtual ~Preview_4_3_Front_Rotate90_BB() {}

    virtual bool ignoreTest()
    {
        ENUM_ROTATION rotation = StereoSettingProvider::getModuleRotation();
        if(eRotate_0   == rotation ||
           eRotate_180 == rotation ||
           SENSOR_COUNT < FRONT_STEREO_SENSOR_COUNT)
        {
            return true;
        }

        return false;
    }

    virtual ENUM_STEREO_SCENARIO        getScenario()       { return eSTEREO_SCENARIO_PREVIEW; }
    virtual ENUM_STEREO_RATIO           getImageRatio()     { return eRatio_4_3; }
    virtual ENUM_STEREO_SENSOR_PROFILE  getStereoProfile()  { return STEREO_SENSOR_PROFILE_FRONT_FRONT; }
    virtual bool                        isDeNoise()         { return false; }
};

//=============================================================================
//  PASS 1
//=============================================================================
TEST_F(Preview_4_3_Front_Rotate90_BB, TEST)
{
    if(ignoreTest()) {
        printf("Skip this test\n");
        return;
    }

    //=============================================================================
    //  Pass 1
    //=============================================================================
    MYEXPECT_EQ( tgCropRect[0],     MRect(MPoint(0, 0),     MSize(2100, 1560)) );
    MYEXPECT_EQ( szRRZO[0],                                 MSize(2100, 1560)  );
    MYEXPECT_EQ( tgCropRect[1],     MRect(MPoint(0, 0),     MSize(1296, 972))  );
    MYEXPECT_EQ( szRRZO[1],                                 MSize(1296, 972)   );
    MYEXPECT_EQ( activityArray[0],  MRect(MPoint(0, 0),     MSize(4208, 3120)) );
    MYEXPECT_EQ( activityArray[1],  MRect(MPoint(0, 0),     MSize(2592, 1944)) );

    //=============================================================================
    //  Pass 2
    //=============================================================================
    //Pass2-A
    MYEXPECT_EQ(pass2SizeInfo[PASS2A].areaWDMA,             StereoArea(1440, 1080));
    MYEXPECT_EQ(pass2SizeInfo[PASS2A].areaWROT,             StereoArea(624, 832));
    MYEXPECT_EQ(pass2SizeInfo[PASS2A].areaFEO,              StereoArea(1386, 1038));
    MYEXPECT_EQ(pass2SizeInfo[PASS2A].areaIMG2O,            StereoArea(640, 480));

    // PASS2A'
    MYEXPECT_EQ(pass2SizeInfo[PASS2A_P].areaWROT,           StereoArea(624, 832));
    MYEXPECT_EQ(pass2SizeInfo[PASS2A_P].areaFEO,            StereoArea(1386, 1038));

    // PASS2A-2
    MYEXPECT_EQ(pass2SizeInfo[PASS2A_2].areaWDMA,           StereoArea(160, 208));
    MYEXPECT_EQ(pass2SizeInfo[PASS2A_2].areaIMG2O,          StereoArea(312, 416));
    MYEXPECT_EQ(pass2SizeInfo[PASS2A_2].areaFEO,            StereoArea(624, 832));

    // PASS2A'-2
    MYEXPECT_EQ(pass2SizeInfo[PASS2A_P_2].areaWDMA,         StereoArea(240, 312));   //(160x208) * 1.5, 2 align
    MYEXPECT_EQ(pass2SizeInfo[PASS2A_P_2].areaIMG2O,        StereoArea(312, 416));
    MYEXPECT_EQ(pass2SizeInfo[PASS2A_P_2].areaFEO,          StereoArea(624, 832));

    // PASS2A-3
    MYEXPECT_EQ(pass2SizeInfo[PASS2A_3].areaIMG2O,          StereoArea(82, 110));
    MYEXPECT_EQ(pass2SizeInfo[PASS2A_3].areaFEO,            StereoArea(312, 416));

    // PASS2A'-3
    MYEXPECT_EQ(pass2SizeInfo[PASS2A_P_3].areaIMG2O,        StereoArea(82, 110));
    MYEXPECT_EQ(pass2SizeInfo[PASS2A_P_3].areaFEO,          StereoArea(312, 416));

    // PASS2B
    MYEXPECT_EQ(sizeProvider->getBufferSize(E_BOKEH_WROT, scenario), STEREO_AREA_ZERO);
    MYEXPECT_EQ(sizeProvider->getBufferSize(E_BOKEH_WDMA, scenario), StereoArea(1440, 1080));
    MYEXPECT_EQ(sizeProvider->getBufferSize(E_BOKEH_3DNR, scenario), StereoArea(1440, 1080));

    //=============================================================================
    //  Buffers
    //=============================================================================
    //N3D Output
    MYEXPECT_EQ(sizeProvider->getBufferSize(E_MV_Y),            StereoArea(184, 214, 24, 6, 12, 3));
    MYEXPECT_EQ(sizeProvider->getBufferSize(E_MASK_M_Y),        StereoArea(184, 214, 24, 6, 12, 3));
    MYEXPECT_EQ(sizeProvider->getBufferSize(E_SV_Y),            StereoArea(184, 214, 24, 6, 12, 3));
    MYEXPECT_EQ(sizeProvider->getBufferSize(E_MASK_S_Y),        StereoArea(184, 214, 24, 6, 12, 3));
    MYEXPECT_EQ(sizeProvider->getBufferSize(E_LDC),             StereoArea(184, 214, 24, 6, 12, 3));

    //DPE Output
    MYEXPECT_EQ(sizeProvider->getBufferSize(E_DMP_H),           StereoArea(184, 214, 24, 6, 12, 3));
    MYEXPECT_EQ(sizeProvider->getBufferSize(E_CFM_H),           StereoArea(184, 214, 24, 6, 12, 3));
    MYEXPECT_EQ(sizeProvider->getBufferSize(E_RESPO),           StereoArea(184, 214, 24, 6, 12, 3));

    //OCC Output
    MYEXPECT_EQ(sizeProvider->getBufferSize(E_MY_S),            StereoArea(160, 208));
    MYEXPECT_EQ(sizeProvider->getBufferSize(E_DMH),             StereoArea(160, 208));

    //WMF Output
    MYEXPECT_EQ(sizeProvider->getBufferSize(E_DMW),             StereoArea(160, 208));

    //GF Output
    MYEXPECT_EQ(sizeProvider->getBufferSize(E_DMG),             StereoArea(208, 156));
    MYEXPECT_EQ(sizeProvider->getBufferSize(E_DMBG),            StereoArea(208, 156));
    MYEXPECT_EQ(sizeProvider->getBufferSize(E_DEPTH_MAP),       StereoArea(416, 312));
}
