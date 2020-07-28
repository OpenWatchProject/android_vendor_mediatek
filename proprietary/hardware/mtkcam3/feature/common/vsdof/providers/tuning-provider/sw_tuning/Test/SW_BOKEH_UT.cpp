#include "StereoTuningProviderUT.h"

#define MY_LOGD(fmt, arg...)    printf("[D][%s]" fmt"\n", __func__, ##arg)
#define MY_LOGI(fmt, arg...)    printf("[I][%s]" fmt"\n", __func__, ##arg)
#define MY_LOGW(fmt, arg...)    printf("[W][%s] WRN(%5d):" fmt"\n", __func__, __LINE__, ##arg)
#define MY_LOGE(fmt, arg...)    printf("[E][%s] %s ERROR(%5d):" fmt"\n", __func__,__FILE__, __LINE__, ##arg)

class SW_BOKEH_UT : public StereoTuningProviderUTBase
{
public:
    SW_BOKEH_UT() : StereoTuningProviderUTBase() { init(); }

    virtual ~SW_BOKEH_UT() {}

    virtual ENUM_STEREO_RATIO           getImageRatio()         { return eRatio_4_3; }
    virtual MUINT32                     getLogicalDeviceID()    { return 3; }
    virtual MINT32                      getStereoFeatureMode()  { return (E_STEREO_FEATURE_VSDOF); }
};

TEST_F(SW_BOKEH_UT, TEST)
{
    std::vector<std::pair<std::string, int>> tuningParams;
    std::vector<int> clearTable;
    StereoTuningProvider::getSWBokehTuningInfo(tuningParams, clearTable);

    MY_LOGD("======== SW Bokeh Parameters ========");
    std::ostringstream oss;
    for(auto i : clearTable) {
        oss << i << " ";
    }
    MY_LOGD("Clear Table(%zu): %s", clearTable.size(), oss.str().c_str());

    for(auto &param : tuningParams) {
        MY_LOGD("\"%s\": %d", param.first.c_str(), param.second);
    }
    MY_LOGD("=====================================");
}