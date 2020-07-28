/*
 * FeatureSettingPolicy.h
 *
 *  Created on: Jul 13, 2019
 *      Author: mtk54498
 */

#ifndef MAIN_CORE_INCLUDE_PROCESSOR_FEATURESETTINGPOLICY_H_
#define MAIN_CORE_INCLUDE_PROCESSOR_FEATURESETTINGPOLICY_H_

#include <header_base.h>
#include "PA_Request.h"
#include <campostalgo/pipe/PipeInfo.h>
#include <plugin/PipelinePlugin.h>
#include <plugin/PipelinePluginType.h>
#include <IFeaturePipe.h>
#include <mtk/mtk_feature_type.h>

#include <memory>

using ::com::mediatek::campostalgo::AlgoType;
using ::com::mediatek::campostalgo::StreamType;
using ::NSCam::NSPipelinePlugin::eFeatureIndexMtk;
using ::NSCam::NSPipelinePlugin::Policy;

namespace NSPA {

class FeatureSettingPolicy: public virtual android::RefBase {
public:
    struct CreationParams {
        StreamType type;
        int32_t mIntfId;
        std::vector<AlgoType> algos;
    };

    /*
     * capture multi frame mode
     * Request Count
     * frame index
     */
    typedef struct {
        MBOOL doProcess = MFALSE;
        uint32_t request_count = 1;
        uint32_t frame_index = 0;
        Policy::State state ;
    } FS_CaptureMultiFrameContext_t;

    typedef NSCam::NSPipelinePlugin::MultiFramePlugin::Selection MFP_Selection;

    class MFPPluginWrapper;
    using MFPPluginWrapperPtr = std::shared_ptr<MFPPluginWrapper>;

    class YuvPluginWrapper;
    using YuvPluginWrapperPtr = std::shared_ptr<YuvPluginWrapper>;
    bool updatePluginSelection(const uint32_t requestNo,bool isFeatureTrigger,uint8_t frameCount);

    FeatureSettingPolicy(const CreationParams& cp);
    status_t evaluateRequest(const sp<PA_Request>& request);
    status_t evaluateStreamSetting(const sp<PA_Request>& request);
    status_t evaluateCaptureSetting(const sp<PA_Request>& request);
    eFeatureIndexMtk convertAlgoType2FeatureType(AlgoType type);
    virtual ~FeatureSettingPolicy();
private:
    StreamType mStreamType;
    int32_t mIntfId = -1;
    int32_t mUniqueKey = -1;
    FS_CaptureMultiFrameContext_t fsMultiFrameCntx;
    std::vector<AlgoType> mStreamAlgos;

    // MultiFrame:
    MFPPluginWrapperPtr mMFPPluginWrapperPtr;
    // Yuv plugin: (for 4Cell key feature)
    YuvPluginWrapperPtr mYuvPluginWrapperPtr;
};

} /* namespace NSPA */

#endif /* MAIN_CORE_INCLUDE_PROCESSOR_FEATURESETTINGPOLICY_H_ */
