#include "StereoSizeProviderUT.h"

#define MY_LOGD(fmt, arg...)    printf("[D][%s]" fmt"\n", __func__, ##arg)
#define MY_LOGI(fmt, arg...)    printf("[I][%s]" fmt"\n", __func__, ##arg)
#define MY_LOGW(fmt, arg...)    printf("[W][%s] WRN(%5d):" fmt"\n", __func__, __LINE__, ##arg)
#define MY_LOGE(fmt, arg...)    printf("[E][%s] %s ERROR(%5d):" fmt"\n", __func__,__FILE__, __LINE__, ##arg)

class Record_4_3_Rear_Rotate0_BB : public StereoSizeProviderUTBase
{
public:
    Record_4_3_Rear_Rotate0_BB() : StereoSizeProviderUTBase() { init(); }

    virtual ~Record_4_3_Rear_Rotate0_BB() {}

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

    virtual ENUM_STEREO_SCENARIO        getScenario()       { return eSTEREO_SCENARIO_RECORD; }
    virtual ENUM_STEREO_RATIO           getImageRatio()     { return eRatio_4_3; }
    virtual ENUM_STEREO_SENSOR_PROFILE  getStereoProfile()  { return STEREO_SENSOR_PROFILE_REAR_REAR; }
    virtual bool                        isDeNoise()         { return false; }
};

//=============================================================================
//  PASS 1
//=============================================================================
TEST_F(Record_4_3_Rear_Rotate0_BB, TEST)
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
    MYEXPECT_EQ( tgCropRect[1],     MRect(MPoint(0, 0),     MSize(1280, 960))  );
    MYEXPECT_EQ( szRRZO[1],                                 MSize(1280, 960)   );
    MYEXPECT_EQ( activityArray[0],  MRect(MPoint(0, 0),     MSize(4208, 3120)) );
    MYEXPECT_EQ( activityArray[1],  MRect(MPoint(0, 0),     MSize(2560, 1920)) );

    //=============================================================================
    //  Pass 2
    //=============================================================================
    //Pass2-A
    MYEXPECT_EQ(pass2SizeInfo[PASS2A].areaWDMA,             StereoArea(1440, 1088));
    MYEXPECT_EQ(pass2SizeInfo[PASS2A].areaWROT,             StereoArea(832, 624));
    MYEXPECT_EQ(pass2SizeInfo[PASS2A].areaFEO,              StereoArea(1386, 1038));
    MYEXPECT_EQ(pass2SizeInfo[PASS2A].areaIMG2O,            StereoArea(640, 480));

    // PASS2A'
    MYEXPECT_EQ(pass2SizeInfo[PASS2A_P].areaWROT,           StereoArea(1008, 752));  //868x640 * 1.2, 16-align
    MYEXPECT_EQ(pass2SizeInfo[PASS2A_P].areaFEO,            StereoArea(1386, 1038));

    // PASS2A-2
    MYEXPECT_EQ(pass2SizeInfo[PASS2A_2].areaWDMA,           StereoArea(256, 186));
    MYEXPECT_EQ(pass2SizeInfo[PASS2A_2].areaIMG2O,          StereoArea(416, 312));
    MYEXPECT_EQ(pass2SizeInfo[PASS2A_2].areaFEO,            StereoArea(832, 624));

    // PASS2A'-2
    MYEXPECT_EQ(pass2SizeInfo[PASS2A_P_2].areaWDMA,         StereoArea(384, 280));   //256x186 * 1.5
    MYEXPECT_EQ(pass2SizeInfo[PASS2A_P_2].areaIMG2O,        StereoArea(504, 376));   //424x312 * 1.2, 8-align
    MYEXPECT_EQ(pass2SizeInfo[PASS2A_P_2].areaFEO,          StereoArea(MSize(1008, 752), MSize(176, 128), MPoint(88, 64)));

    // PASS2A-3
    MYEXPECT_EQ(pass2SizeInfo[PASS2A_3].areaIMG2O,          StereoArea(110, 82));
    MYEXPECT_EQ(pass2SizeInfo[PASS2A_3].areaFEO,            StereoArea(416, 312));

    // PASS2A'-3
    MYEXPECT_EQ(pass2SizeInfo[PASS2A_P_3].areaIMG2O,        StereoArea(110, 82));
    MYEXPECT_EQ(pass2SizeInfo[PASS2A_P_3].areaFEO,          StereoArea(MSize(504, 376), MSize(88, 64), MPoint(44, 32)));

    // PASS2B
    MYEXPECT_EQ(sizeProvider->getBufferSize(E_BOKEH_WROT, scenario), StereoArea(1440, 1088));
    MYEXPECT_EQ(sizeProvider->getBufferSize(E_BOKEH_WDMA, scenario), StereoArea(1440, 1080));
    MYEXPECT_EQ(sizeProvider->getBufferSize(E_BOKEH_3DNR, scenario), StereoArea(1440, 1088));

    //=============================================================================
    //  Buffers
    //=============================================================================
    //N3D Output
    MYEXPECT_EQ(sizeProvider->getBufferSize(E_MV_Y),            StereoArea(232, 162, 24, 6, 12, 3));
    MYEXPECT_EQ(sizeProvider->getBufferSize(E_MASK_M_Y),        StereoArea(232, 162, 24, 6, 12, 3));
    MYEXPECT_EQ(sizeProvider->getBufferSize(E_SV_Y),            StereoArea(232, 162, 24, 6, 12, 3));
    MYEXPECT_EQ(sizeProvider->getBufferSize(E_MASK_S_Y),        StereoArea(232, 162, 24, 6, 12, 3));
    MYEXPECT_EQ(sizeProvider->getBufferSize(E_LDC),             StereoArea(232, 162, 24, 6, 12, 3));

    //DPE Output
    MYEXPECT_EQ(sizeProvider->getBufferSize(E_DMP_H),           StereoArea(232, 162, 24, 6, 12, 3));
    MYEXPECT_EQ(sizeProvider->getBufferSize(E_CFM_H),           StereoArea(232, 162, 24, 6, 12, 3));
    MYEXPECT_EQ(sizeProvider->getBufferSize(E_RESPO),           StereoArea(232, 162, 24, 6, 12, 3));

    //OCC Output
    MYEXPECT_EQ(sizeProvider->getBufferSize(E_MY_S),            StereoArea(208, 156));
    MYEXPECT_EQ(sizeProvider->getBufferSize(E_DMH),             StereoArea(208, 156));

    //WMF Output
    MYEXPECT_EQ(sizeProvider->getBufferSize(E_DMW),             StereoArea(208, 156));

    //GF Output
    MYEXPECT_EQ(sizeProvider->getBufferSize(E_DMG),             StereoArea(208, 156));
    MYEXPECT_EQ(sizeProvider->getBufferSize(E_DMBG),            StereoArea(208, 156));
    MYEXPECT_EQ(sizeProvider->getBufferSize(E_DEPTH_MAP),       StereoArea(416, 312));
}
