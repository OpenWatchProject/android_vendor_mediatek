#include "StereoSizeProviderUT.h"

#define MY_LOGD(fmt, arg...)    printf("[D][%s]" fmt"\n", __func__, ##arg)
#define MY_LOGI(fmt, arg...)    printf("[I][%s]" fmt"\n", __func__, ##arg)
#define MY_LOGW(fmt, arg...)    printf("[W][%s] WRN(%5d):" fmt"\n", __func__, __LINE__, ##arg)
#define MY_LOGE(fmt, arg...)    printf("[E][%s] %s ERROR(%5d):" fmt"\n", __func__,__FILE__, __LINE__, ##arg)

class Capture_16_9_Rear_Rotate90_BM_VSDoF : public StereoSizeProviderUTBase
{
public:
    Capture_16_9_Rear_Rotate90_BM_VSDoF() : StereoSizeProviderUTBase() { init(); }

    virtual ~Capture_16_9_Rear_Rotate90_BM_VSDoF() {}

    virtual bool ignoreTest()
    {
        ENUM_ROTATION rotation = StereoSettingProvider::getModuleRotation();
        if(eRotate_0   == rotation ||
           eRotate_180 == rotation)
        {
            return true;
        }

        return false;
    }

    virtual ENUM_STEREO_SCENARIO        getScenario()       { return eSTEREO_SCENARIO_CAPTURE; }
    virtual ENUM_STEREO_RATIO           getImageRatio()     { return eRatio_16_9; }
    virtual ENUM_STEREO_SENSOR_PROFILE  getStereoProfile()  { return STEREO_SENSOR_PROFILE_REAR_FRONT; }
    virtual bool                        isDeNoise()         { return false; }
};

//=============================================================================
//  PASS 1
//=============================================================================
TEST_F(Capture_16_9_Rear_Rotate90_BM_VSDoF, TEST)
{
    if(ignoreTest()) {
        printf("Skip this test\n");
        return;
    }

    //=============================================================================
    //  Pass 1
    //=============================================================================
    //Capture scenario does not use RRZO of main sensor, so no need to test
    MYEXPECT_EQ( tgCropRect[1],     MRect(MPoint(0, 377),   MSize(4208, 2366)) );
    MYEXPECT_EQ( szRRZO[1],                                 MSize(2104, 1182)  );
    MYEXPECT_EQ( tgCropRect[2],     MRect(MPoint(0, 0),     MSize(4208, 3120)) );
    MYEXPECT_EQ( szMainIMGO,                                MSize(4208, 3120)  );
    MYEXPECT_EQ( StereoSizeProvider::getInstance()->getSBSImageSize(), MSize(2688, 1984));

    //=============================================================================
    //  Pass 2
    //=============================================================================
    //Pass2A-Crop
    printf("Crop %.2f, mode %d IS_FOV_CROP %d 1 %d\n", FOV_RATIO, StereoSettingProvider::isBMVSDoF(), IS_FOV_CROP,(StereoSettingProvider::isBMVSDoF() && FOV_RATIO < 1.0f && FOV_RATIO > 0.0f));
    if(IS_FOV_CROP) {
        //Use 63.3->57.3 crop setting, crop ratio = 0.886
        MYEXPECT_EQ(pass2SizeInfo[PASS2A_CROP].areaWDMA,    StereoArea(4208, 3120, 480, 1024, 240, 512));
    } else {
        MYEXPECT_EQ(pass2SizeInfo[PASS2A_CROP].areaWDMA,    StereoArea(4208, 3120, 0, 754, 0, 377));
    }

    //Pass2-A
    MYEXPECT_EQ(pass2SizeInfo[PASS2A].areaWDMA,             StereoArea(3072, 1728));
    MYEXPECT_EQ(pass2SizeInfo[PASS2A].areaWROT,             StereoArea(544, 960));
    MYEXPECT_EQ(pass2SizeInfo[PASS2A].areaFEO,              StereoArea(1600, 900));

    // PASS2A'
    MYEXPECT_EQ(pass2SizeInfo[PASS2A_P].areaWROT,           StereoArea(544, 960));
    MYEXPECT_EQ(pass2SizeInfo[PASS2A_P].areaFEO,            StereoArea(1600, 900));

    // PASS2A-2
    MYEXPECT_EQ(pass2SizeInfo[PASS2A_2].areaWDMA,           StereoArea(1344, 1984, 264, 64, 132, 32));
    MYEXPECT_EQ(pass2SizeInfo[PASS2A_2].areaIMG2O,          StereoArea(272, 480));
    MYEXPECT_EQ(pass2SizeInfo[PASS2A_2].areaFEO,            StereoArea(544, 960));

    // PASS2A'-2
    MYEXPECT_EQ(pass2SizeInfo[PASS2A_P_2].areaWDMA,         StereoArea(1344, 1984, 264, 64, 132, 32));
    MYEXPECT_EQ(pass2SizeInfo[PASS2A_P_2].areaIMG2O,        StereoArea(272, 480));
    MYEXPECT_EQ(pass2SizeInfo[PASS2A_P_2].areaFEO,          StereoArea(544, 960));

    // PASS2A-3
    MYEXPECT_EQ(pass2SizeInfo[PASS2A_3].areaIMG2O,          StereoArea(72, 128));
    MYEXPECT_EQ(pass2SizeInfo[PASS2A_3].areaFEO,            StereoArea(272, 480));

    // PASS2A'-3
    MYEXPECT_EQ(pass2SizeInfo[PASS2A_P_3].areaIMG2O,        StereoArea(72, 128));
    MYEXPECT_EQ(pass2SizeInfo[PASS2A_P_3].areaFEO,          StereoArea(272, 480));

    // PASS2B
    MYEXPECT_EQ(sizeProvider->getBufferSize(E_BOKEH_WROT, scenario), StereoArea(3072, 1728));
    MYEXPECT_EQ(sizeProvider->getBufferSize(E_BOKEH_WDMA, scenario), StereoArea(3072, 1728));
    MYEXPECT_EQ(sizeProvider->getBufferSize(E_BOKEH_3DNR, scenario), STEREO_AREA_ZERO);

    //=============================================================================
    //  Buffers
    //=============================================================================
    //N3D Output
    MYEXPECT_EQ(sizeProvider->getBufferSize(E_MV_Y, eSTEREO_SCENARIO_CAPTURE),      StereoArea(336, 248, 64, 8, 32, 4));
    MYEXPECT_EQ(sizeProvider->getBufferSize(E_MASK_M_Y, eSTEREO_SCENARIO_CAPTURE),  StereoArea(336, 248, 64, 8, 32, 4));
    MYEXPECT_EQ(sizeProvider->getBufferSize(E_SV_Y, eSTEREO_SCENARIO_CAPTURE),      StereoArea(336, 248, 64, 8, 32, 4));
    MYEXPECT_EQ(sizeProvider->getBufferSize(E_MASK_S_Y, eSTEREO_SCENARIO_CAPTURE),  StereoArea(336, 248, 64, 8, 32, 4));
    MYEXPECT_EQ(sizeProvider->getBufferSize(E_LDC, eSTEREO_SCENARIO_CAPTURE),       StereoArea(168, 248, 32, 8, 16, 4));

    //N3D before MDP for capture
    MYEXPECT_EQ(sizeProvider->getBufferSize(E_MV_Y_LARGE),      StereoArea(1344, 1984, 264, 64, 132, 32));
    MYEXPECT_EQ(sizeProvider->getBufferSize(E_MASK_M_Y_LARGE),  StereoArea(1344, 1984, 264, 64, 132, 32));
    MYEXPECT_EQ(sizeProvider->getBufferSize(E_SV_Y_LARGE),      StereoArea(1344, 1984, 264, 64, 132, 32));
    MYEXPECT_EQ(sizeProvider->getBufferSize(E_MASK_S_Y_LARGE),  StereoArea(1344, 1984, 264, 64, 132, 32));

    //DPE Output
    MYEXPECT_EQ(sizeProvider->getBufferSize(E_DMP_H, eSTEREO_SCENARIO_CAPTURE),     StereoArea(336, 248, 64, 8, 32, 4));
    MYEXPECT_EQ(sizeProvider->getBufferSize(E_CFM_H, eSTEREO_SCENARIO_CAPTURE),     StereoArea(336, 248, 64, 8, 32, 4));
    MYEXPECT_EQ(sizeProvider->getBufferSize(E_RESPO, eSTEREO_SCENARIO_CAPTURE),     StereoArea(336, 248, 64, 8, 32, 4));
}
