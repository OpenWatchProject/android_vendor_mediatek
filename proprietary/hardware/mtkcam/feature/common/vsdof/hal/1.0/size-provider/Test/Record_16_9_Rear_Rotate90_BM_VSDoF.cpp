#include "StereoSizeProviderUT.h"

#define MY_LOGD(fmt, arg...)    printf("[D][%s]" fmt"\n", __func__, ##arg)
#define MY_LOGI(fmt, arg...)    printf("[I][%s]" fmt"\n", __func__, ##arg)
#define MY_LOGW(fmt, arg...)    printf("[W][%s] WRN(%5d):" fmt"\n", __func__, __LINE__, ##arg)
#define MY_LOGE(fmt, arg...)    printf("[E][%s] %s ERROR(%5d):" fmt"\n", __func__,__FILE__, __LINE__, ##arg)

class Record_16_9_Rear_Rotate90_BM_VSDoF : public StereoSizeProviderUTBase
{
public:
    Record_16_9_Rear_Rotate90_BM_VSDoF() : StereoSizeProviderUTBase() { init(); }

    virtual ~Record_16_9_Rear_Rotate90_BM_VSDoF() {}

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

    virtual ENUM_STEREO_SCENARIO        getScenario()       { return eSTEREO_SCENARIO_RECORD; }
    virtual ENUM_STEREO_RATIO           getImageRatio()     { return eRatio_16_9; }
    virtual ENUM_STEREO_SENSOR_PROFILE  getStereoProfile()  { return STEREO_SENSOR_PROFILE_REAR_REAR; }
    virtual bool                        isDeNoise()         { return false; }
};

//=============================================================================
//  PASS 1
//=============================================================================
TEST_F(Record_16_9_Rear_Rotate90_BM_VSDoF, TEST)
{
    if(ignoreTest()) {
        printf("Skip this test\n");
        return;
    }

    //=============================================================================
    //  Pass 1
    //=============================================================================
    if(IS_FOV_CROP) {
        //Use 63.3->57.3 crop setting, crop ratio = 0.886
        MYEXPECT_EQ( tgCropRect[0], MRect(MPoint(120, 257), MSize(1864, 1046)) );
        MYEXPECT_EQ( szRRZO[0],                             MSize(1864, 1046)  );
    } else {
        MYEXPECT_EQ( tgCropRect[0], MRect(MPoint(0, 189),   MSize(2104, 1182)) );
        MYEXPECT_EQ( szRRZO[0],                             MSize(2104, 1182)  );
    }
    MYEXPECT_EQ( tgCropRect[1],     MRect(MPoint(0, 189),   MSize(2104, 1182)) );
    MYEXPECT_EQ( szRRZO[1],                                 MSize(2104, 1182)  );
    MYEXPECT_EQ( activityArray[0],  MRect(MPoint(0, 377),   MSize(4208, 2366)) );
    MYEXPECT_EQ( activityArray[1],  MRect(MPoint(0, 377),   MSize(4208, 2366)) );

    //=============================================================================
    //  Pass 2
    //=============================================================================
    //Pass2A-Crop
    //Use 63.3->57.3 crop setting, crop ratio = 0.886
    MYEXPECT_EQ(pass2SizeInfo[PASS2A_CROP].areaWDMA,        StereoArea(1864, 1046));

    //Pass2A
    MYEXPECT_EQ(pass2SizeInfo[PASS2A].areaWROT,             StereoArea(544, 960));

    //Pass2A-B
    MYEXPECT_EQ(pass2SizeInfo[PASS2A].areaWDMA,             StereoArea(1920, 1088));
    MYEXPECT_EQ(pass2SizeInfo[PASS2A].areaWROT,             StereoArea(544, 960));

    // PASS2A'
    MYEXPECT_EQ(pass2SizeInfo[PASS2A_P].areaWROT,           StereoArea(544, 960));
    MYEXPECT_EQ(pass2SizeInfo[PASS2A_P].areaFEO,            StereoArea(1600, 900));

    // PASS2A-2
    MYEXPECT_EQ(pass2SizeInfo[PASS2A_2].areaWDMA,           StereoArea(336, 288));
    MYEXPECT_EQ(pass2SizeInfo[PASS2A_2].areaIMG2O,          StereoArea(272, 480));
    MYEXPECT_EQ(pass2SizeInfo[PASS2A_2].areaFEO,            StereoArea(544, 960));

    // PASS2A'-2
    MYEXPECT_EQ(pass2SizeInfo[PASS2A_P_2].areaWDMA,         StereoArea(504, 432));
    MYEXPECT_EQ(pass2SizeInfo[PASS2A_P_2].areaIMG2O,        StereoArea(272, 480));
    MYEXPECT_EQ(pass2SizeInfo[PASS2A_P_2].areaFEO,          StereoArea(544, 960));

    // PASS2A-3
    MYEXPECT_EQ(pass2SizeInfo[PASS2A_3].areaIMG2O,          StereoArea(72, 128));
    MYEXPECT_EQ(pass2SizeInfo[PASS2A_3].areaFEO,            StereoArea(272, 480));

    // PASS2A'-3
    MYEXPECT_EQ(pass2SizeInfo[PASS2A_P_3].areaIMG2O,        StereoArea(72, 128));
    MYEXPECT_EQ(pass2SizeInfo[PASS2A_P_3].areaFEO,          StereoArea(272, 480));

    // PASS2B
    MYEXPECT_EQ(sizeProvider->getBufferSize(E_BOKEH_WROT, scenario), StereoArea(1920, 1088));
    MYEXPECT_EQ(sizeProvider->getBufferSize(E_BOKEH_WDMA, scenario), StereoArea(1920, 1080));
    MYEXPECT_EQ(sizeProvider->getBufferSize(E_BOKEH_3DNR, scenario), StereoArea(1920, 1088));

    //=============================================================================
    //  Buffers
    //=============================================================================
    //N3D Output
    MYEXPECT_EQ(sizeProvider->getBufferSize(E_MV_Y),            StereoArea(336, 248, 64, 8, 32, 4));
    MYEXPECT_EQ(sizeProvider->getBufferSize(E_MASK_M_Y),        StereoArea(336, 248, 64, 8, 32, 4));
    MYEXPECT_EQ(sizeProvider->getBufferSize(E_SV_Y),            StereoArea(336, 248, 64, 8, 32, 4));
    MYEXPECT_EQ(sizeProvider->getBufferSize(E_MASK_S_Y),        StereoArea(336, 248, 64, 8, 32, 4));
    MYEXPECT_EQ(sizeProvider->getBufferSize(E_LDC),             StereoArea(168, 248, 32, 8, 16, 4));

    //DPE Output
    MYEXPECT_EQ(sizeProvider->getBufferSize(E_DMP_H),           StereoArea(336, 248, 64, 8, 32, 4));
    MYEXPECT_EQ(sizeProvider->getBufferSize(E_CFM_H),           StereoArea(336, 248, 64, 8, 32, 4));
    MYEXPECT_EQ(sizeProvider->getBufferSize(E_RESPO),           StereoArea(336, 248, 64, 8, 32, 4));

    //OCC Output
    MYEXPECT_EQ(sizeProvider->getBufferSize(E_MY_S),            StereoArea(136, 240));
    MYEXPECT_EQ(sizeProvider->getBufferSize(E_DMH),             StereoArea(136, 240));

    //WMF Output
    MYEXPECT_EQ(sizeProvider->getBufferSize(E_DMW),             StereoArea(136, 240));

    //GF Output
    MYEXPECT_EQ(sizeProvider->getBufferSize(E_DMG),             StereoArea(240, 136));
    MYEXPECT_EQ(sizeProvider->getBufferSize(E_DMBG),            StereoArea(240, 136));
    MYEXPECT_EQ(sizeProvider->getBufferSize(E_DEPTH_MAP),       StereoArea(480, 272));
}
