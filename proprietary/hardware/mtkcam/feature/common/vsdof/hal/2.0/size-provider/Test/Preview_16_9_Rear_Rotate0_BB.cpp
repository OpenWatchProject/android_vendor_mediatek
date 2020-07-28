#include "StereoSizeProviderUT.h"

#define MY_LOGD(fmt, arg...)    printf("[D][%s]" fmt"\n", __func__, ##arg)
#define MY_LOGI(fmt, arg...)    printf("[I][%s]" fmt"\n", __func__, ##arg)
#define MY_LOGW(fmt, arg...)    printf("[W][%s] WRN(%5d):" fmt"\n", __func__, __LINE__, ##arg)
#define MY_LOGE(fmt, arg...)    printf("[E][%s] %s ERROR(%5d):" fmt"\n", __func__,__FILE__, __LINE__, ##arg)

class Preview_16_9_Rear_Rotate0_BB : public StereoSizeProviderUTBase
{
public:
    Preview_16_9_Rear_Rotate0_BB() : StereoSizeProviderUTBase() { init(); }

    virtual ~Preview_16_9_Rear_Rotate0_BB() {}

    virtual bool ignoreTest()
    {
        ENUM_ROTATION rotation = StereoSettingProvider::getModuleRotation();
        if(eRotate_90  == rotation ||
           eRotate_270 == rotation)
        {
            return true;
        }

        return false;
    }

    virtual ENUM_STEREO_SCENARIO        getScenario()       { return eSTEREO_SCENARIO_PREVIEW; }
    virtual ENUM_STEREO_RATIO           getImageRatio()     { return eRatio_16_9; }
    virtual ENUM_STEREO_SENSOR_PROFILE  getStereoProfile()  { return STEREO_SENSOR_PROFILE_REAR_REAR; }
    virtual bool                        isDeNoise()         { return false; }
};

//=============================================================================
//  PASS 1
//=============================================================================
TEST_F(Preview_16_9_Rear_Rotate0_BB, TEST)
{
    if(ignoreTest()) {
        printf("Skip this test\n");
        return;
    }

    //=============================================================================
    //  Pass 1
    //=============================================================================
    MYEXPECT_EQ( tgCropRect[0],     MRect(MPoint(0, 190),   MSize(2100, 1180)) );
    MYEXPECT_EQ( szRRZO[0],                                 MSize(2100, 1180)  );
    MYEXPECT_EQ( tgCropRect[1],     MRect(MPoint(0, 120),   MSize(1280, 720))  );
    MYEXPECT_EQ( szRRZO[1],                                 MSize(1280, 720)   );
    MYEXPECT_EQ( activityArray[0],  MRect(MPoint(0, 377),   MSize(4208, 2366)) );
    MYEXPECT_EQ( activityArray[1],  MRect(MPoint(0, 240),   MSize(2560, 1440)) );

    //=============================================================================
    //  Pass 2
    //=============================================================================
    //Pass2-A
    MYEXPECT_EQ(pass2SizeInfo[PASS2A].areaWDMA,             StereoArea(1920, 1080));
    MYEXPECT_EQ(pass2SizeInfo[PASS2A].areaWROT,             StereoArea(960, 544));
    MYEXPECT_EQ(pass2SizeInfo[PASS2A].areaFEO,              StereoArea(1600, 900));
    MYEXPECT_EQ(pass2SizeInfo[PASS2A].areaIMG2O,            StereoArea(640, 480));

    // PASS2A'
    MYEXPECT_EQ(pass2SizeInfo[PASS2A_P].areaWROT,           StereoArea(1152, 656));  //(960x544) * 1.2, 16-align
    MYEXPECT_EQ(pass2SizeInfo[PASS2A_P].areaFEO,            StereoArea(1600, 900));

    // PASS2A-2
    MYEXPECT_EQ(pass2SizeInfo[PASS2A_2].areaWDMA,           StereoArea(240, 136));
    MYEXPECT_EQ(pass2SizeInfo[PASS2A_2].areaIMG2O,          StereoArea(480, 272));
    MYEXPECT_EQ(pass2SizeInfo[PASS2A_2].areaFEO,            StereoArea(960, 544));

    // PASS2A'-2
    MYEXPECT_EQ(pass2SizeInfo[PASS2A_P_2].areaWDMA,         StereoArea(360, 204));   //(240x136) * 1.5
    MYEXPECT_EQ(pass2SizeInfo[PASS2A_P_2].areaIMG2O,        StereoArea(576, 328));   //(480x272 * 1.2), 8-align
    MYEXPECT_EQ(pass2SizeInfo[PASS2A_P_2].areaFEO,          StereoArea(MSize(1152, 656), MSize(192, 112), MPoint(96, 56)));

    // PASS2A-3
    MYEXPECT_EQ(pass2SizeInfo[PASS2A_3].areaIMG2O,          StereoArea(128, 72));
    MYEXPECT_EQ(pass2SizeInfo[PASS2A_3].areaFEO,            StereoArea(480, 272));

    // PASS2A'-3
    MYEXPECT_EQ(pass2SizeInfo[PASS2A_P_3].areaIMG2O,        StereoArea(128, 72));
    MYEXPECT_EQ(pass2SizeInfo[PASS2A_P_3].areaFEO,          StereoArea(MSize(576, 328), MSize(96, 56), MPoint(48, 28)));

    // PASS2B
    MYEXPECT_EQ(sizeProvider->getBufferSize(E_BOKEH_WROT, scenario), STEREO_AREA_ZERO);
    MYEXPECT_EQ(sizeProvider->getBufferSize(E_BOKEH_WDMA, scenario), StereoArea(1920, 1080));
    MYEXPECT_EQ(sizeProvider->getBufferSize(E_BOKEH_3DNR, scenario), StereoArea(1920, 1080));

    //=============================================================================
    //  Buffers
    //=============================================================================
    //N3D Output
    MYEXPECT_EQ(sizeProvider->getBufferSize(E_MV_Y),            StereoArea(272, 144, 32, 8, 16, 4));
    MYEXPECT_EQ(sizeProvider->getBufferSize(E_MASK_M_Y),        StereoArea(272, 144, 32, 8, 16, 4));
    MYEXPECT_EQ(sizeProvider->getBufferSize(E_SV_Y),            StereoArea(272, 144, 32, 8, 16, 4));
    MYEXPECT_EQ(sizeProvider->getBufferSize(E_MASK_S_Y),        StereoArea(272, 144, 32, 8, 16, 4));
    MYEXPECT_EQ(sizeProvider->getBufferSize(E_LDC),             StereoArea(272, 144, 32, 8, 16, 4));

    //DPE Output
    MYEXPECT_EQ(sizeProvider->getBufferSize(E_DMP_H),           StereoArea(272, 144, 32, 8, 16, 4));
    MYEXPECT_EQ(sizeProvider->getBufferSize(E_CFM_H),           StereoArea(272, 144, 32, 8, 16, 4));
    MYEXPECT_EQ(sizeProvider->getBufferSize(E_RESPO),           StereoArea(272, 144, 32, 8, 16, 4));

    //OCC Output
    MYEXPECT_EQ(sizeProvider->getBufferSize(E_MY_S),            StereoArea(240, 136));
    MYEXPECT_EQ(sizeProvider->getBufferSize(E_DMH),             StereoArea(240, 136));

    //WMF Output
    MYEXPECT_EQ(sizeProvider->getBufferSize(E_DMW),             StereoArea(240, 136));

    //GF Output
    MYEXPECT_EQ(sizeProvider->getBufferSize(E_DMG),             StereoArea(240, 136));
    MYEXPECT_EQ(sizeProvider->getBufferSize(E_DMBG),            StereoArea(240, 136));
    MYEXPECT_EQ(sizeProvider->getBufferSize(E_DEPTH_MAP),       StereoArea(480, 272));
}
