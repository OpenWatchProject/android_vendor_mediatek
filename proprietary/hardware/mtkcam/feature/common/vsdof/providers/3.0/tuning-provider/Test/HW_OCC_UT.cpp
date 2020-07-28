#include "StereoTuningProviderUT.h"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wreorder"
#include <mtkcam/drv/def/owecommon.h>
#pragma GCC diagnostic pop

#define MY_LOGD(fmt, arg...)    printf("[D][%s]" fmt"\n", __func__, ##arg)
#define MY_LOGI(fmt, arg...)    printf("[I][%s]" fmt"\n", __func__, ##arg)
#define MY_LOGW(fmt, arg...)    printf("[W][%s] WRN(%5d):" fmt"\n", __func__, __LINE__, ##arg)
#define MY_LOGE(fmt, arg...)    printf("[E][%s] %s ERROR(%5d):" fmt"\n", __func__,__FILE__, __LINE__, ##arg)

class HW_OCC_UT : public StereoTuningProviderUTBase
{
public:
    HW_OCC_UT() : StereoTuningProviderUTBase() { init(); }

    virtual ~HW_OCC_UT() {}

    virtual ENUM_STEREO_RATIO           getImageRatio()     { return eRatio_4_3; }
    virtual ENUM_STEREO_SENSOR_PROFILE  getStereoProfile()  { return STEREO_SENSOR_PROFILE_REAR_REAR; }
    virtual MINT32                      getStereoFeatureMode() { return (E_STEREO_FEATURE_VSDOF|E_STEREO_FEATURE_MTK_DEPTHMAP); }
};

//=============================================================================
//  PASS 1
//=============================================================================
TEST_F(HW_OCC_UT, TEST)
{
    NSCam::NSIoPipe::OCCConfig config;
    ENUM_STEREO_SCENARIO eScenario = eSTEREO_SCENARIO_PREVIEW;
    StereoTuningProvider::getHWOCCTuningInfo(config, eScenario);

    MY_LOGD("Scenario:             %s", SCENARIO_NAMES[eScenario]);
    MY_LOGD("occ_scan_r2l          %d", config.occ_scan_r2l);
    MY_LOGD("occ_horz_ds4          %d", config.occ_horz_ds4);
    MY_LOGD("occ_vert_ds4          %d", config.occ_vert_ds4);
    MY_LOGD("occ_h_skip_mode       %d", config.occ_h_skip_mode);

    MY_LOGD("occ_hsize             %d", config.occ_hsize);
    MY_LOGD("occ_vsize             %d", config.occ_vsize);
    MY_LOGD("occ_v_crop_s          %d", config.occ_v_crop_s);
    MY_LOGD("occ_v_crop_e          %d", config.occ_v_crop_e);
    MY_LOGD("occ_h_crop_s          %d", config.occ_h_crop_s);
    MY_LOGD("occ_h_crop_e          %d", config.occ_h_crop_e);

    MY_LOGD("occ_th_luma           %d", config.occ_th_luma);
    MY_LOGD("occ_th_h              %d", config.occ_th_h);
    MY_LOGD("occ_th_v              %d", config.occ_th_v);
    MY_LOGD("occ_vec_shift         %d", config.occ_vec_shift);
    MY_LOGD("occ_vec_offset        %d", config.occ_vec_offset);
    MY_LOGD("occ_invalid_value     %d", config.occ_invalid_value);
    MY_LOGD("occ_owc_th            %d", config.occ_owc_th);
    MY_LOGD("occ_owc_en            %d", config.occ_owc_en);
    MY_LOGD("occ_depth_clip_en     %d", config.occ_depth_clip_en);
    MY_LOGD("occ_spare             %d", config.occ_spare);
    MY_LOGD("===========================");
}