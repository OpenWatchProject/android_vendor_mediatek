#include "StereoTuningProviderUT.h"

#define MY_LOGD(fmt, arg...)    printf("[D][%s]" fmt"\n", __func__, ##arg)
#define MY_LOGI(fmt, arg...)    printf("[I][%s]" fmt"\n", __func__, ##arg)
#define MY_LOGW(fmt, arg...)    printf("[W][%s] WRN(%5d):" fmt"\n", __func__, __LINE__, ##arg)
#define MY_LOGE(fmt, arg...)    printf("[E][%s] %s ERROR(%5d):" fmt"\n", __func__,__FILE__, __LINE__, ##arg)

class SW_OCC_UT : public StereoTuningProviderUTBase
{
public:
    SW_OCC_UT() : StereoTuningProviderUTBase() { init(); }

    virtual ~SW_OCC_UT() {}

    virtual ENUM_STEREO_RATIO           getImageRatio()         { return eRatio_4_3; }
    virtual MUINT32                     getLogicalDeviceID()    { return 3; }
    virtual MINT32                      getStereoFeatureMode()  { return (E_STEREO_FEATURE_VSDOF|E_STEREO_FEATURE_MTK_DEPTHMAP); }
};

TEST_F(SW_OCC_UT, TEST)
{
    MUINT32 coreNumber = 0;
    std::vector<std::pair<std::string, int>> tuningParams;
    ENUM_STEREO_SCENARIO scenarios[] = {eSTEREO_SCENARIO_PREVIEW, eSTEREO_SCENARIO_RECORD, eSTEREO_SCENARIO_CAPTURE};

    MY_LOGD("======== SW OCC Parameters ========");
    for(auto scenario : scenarios) {
        if(StereoTuningProvider::getOCCTuningInfo(coreNumber, tuningParams, scenario)) {
            MY_LOGD("Scenario: %s", SCENARIO_NAMES[scenario]);
            MY_LOGD("CoreNumber: %d", coreNumber);

            for(auto &param : tuningParams) {
                MY_LOGD("\"%s\": %d", param.first.c_str(), param.second);
            }
        } else {
            MY_LOGD("Cannot get tuning result for scenario %s", SCENARIO_NAMES[scenario]);
        }

        MY_LOGD("===================================");
    }
}