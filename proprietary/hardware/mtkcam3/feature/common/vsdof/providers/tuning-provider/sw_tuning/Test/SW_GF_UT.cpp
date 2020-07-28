#include "StereoTuningProviderUT.h"

#define MY_LOGD(fmt, arg...)    printf("[D][%s]" fmt"\n", __func__, ##arg)
#define MY_LOGI(fmt, arg...)    printf("[I][%s]" fmt"\n", __func__, ##arg)
#define MY_LOGW(fmt, arg...)    printf("[W][%s] WRN(%5d):" fmt"\n", __func__, __LINE__, ##arg)
#define MY_LOGE(fmt, arg...)    printf("[E][%s] %s ERROR(%5d):" fmt"\n", __func__,__FILE__, __LINE__, ##arg)

class SW_GF_UT : public StereoTuningProviderUTBase
{
public:
    SW_GF_UT() : StereoTuningProviderUTBase() { init(); }

    virtual ~SW_GF_UT() {}

    virtual ENUM_STEREO_RATIO           getImageRatio()         { return eRatio_4_3; }
    virtual MUINT32                     getLogicalDeviceID()    { return 3; }
    virtual MINT32                      getStereoFeatureMode()  { return (E_STEREO_FEATURE_VSDOF|E_STEREO_FEATURE_MTK_DEPTHMAP); }
};

TEST_F(SW_GF_UT, TEST)
{
    MUINT32 coreNumber = 0;
    GF_TUNING_T clearTable;
    GF_TUNING_T dispCtrlPoints;
    GF_TUNING_T blurGainTable;
    std::vector<std::pair<std::string, int>> tuningParamsList;

    ENUM_STEREO_SCENARIO scenarios[] = {eSTEREO_SCENARIO_PREVIEW, eSTEREO_SCENARIO_RECORD, eSTEREO_SCENARIO_CAPTURE};

    MY_LOGD("======== SW GF Parameters ========");
    for(auto scenario : scenarios) {
        if(StereoTuningProvider::getGFTuningInfo(coreNumber, clearTable, dispCtrlPoints, blurGainTable, tuningParamsList, scenario)) {
            MY_LOGD("Scenario: %s", SCENARIO_NAMES[scenario]);
            MY_LOGD("CoreNumber: %d", coreNumber);

            std::ostringstream ossClearTbl;
            for(auto v : clearTable) {
                ossClearTbl << v << " ";
            }
            MY_LOGD("Clear Table: %s", ossClearTbl.str().c_str());

            std::ostringstream ossDispCtrlPts;
            for(auto v : dispCtrlPoints) {
                ossDispCtrlPts << v << " ";
            }
            MY_LOGD("dispCtrlPoints: %s", ossDispCtrlPts.str().c_str());

            std::ostringstream ossBlurGainTble;
            for(auto v : blurGainTable) {
                ossBlurGainTble << v << " ";
            }
            MY_LOGD("blurGainTable: %s", ossBlurGainTble.str().c_str());

            for(auto &param : tuningParamsList) {
                MY_LOGD("\"%s\": %d", param.first.c_str(), param.second);
            }
        } else {
            MY_LOGD("Cannot get tuning result for scenario %s", SCENARIO_NAMES[scenario]);
        }

        MY_LOGD("===================================");
    }
}