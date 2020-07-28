#ifndef MTK_MTKCAM_FEATURE_TAG_TABLE_H
#define MTK_MTKCAM_FEATURE_TAG_TABLE_H

// This file is maintain by feature owner
#include <vendor/mediatek/hardware/camera/isphal/1.0/types.h>
#include <unordered_map>
#include <string>

using namespace vendor::mediatek::hardware::camera::isphal::V1_0;
/******************************************************************************
 *
 ******************************************************************************/
struct FeatureTagInfo
{
    std::string name;
    uint32_t datacount; // it is means data size of an item. e.g. uint8 = 1, uint32 = 4
    MetaDataType type; // data type for isp hidl, reference types.hal in isp hidl interface
};
static std::unordered_map<uint32_t, struct FeatureTagInfo> gFeatureMap =
{
    {MTK_CONTROL_CAPTURE_HINT_FOR_ISP_TUNING,
            {"com.mediatek.control.capture.hintForIspTuning", 4, MetaDataType::TYPE_INT32}},
    {MTK_CONTROL_CAPTURE_HINT_FOR_ISP_FRAME_TUNING_INDEX,
            {"com.mediatek.control.capture.hintForIspFrameTuningIndex", 8, MetaDataType::TYPE_INT64}}
};

#endif  // MTK_MTKCAM_FEATURE_TAG_TABLE_H