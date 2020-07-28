#define DEBUG_LOG_TAG "HRT"

#include "hrt_common.h"

#include <vector>

#include "overlay.h"
#include "dispatcher.h"
#include "hwc2.h"

#ifndef MTK_USE_DRM_DEVICE
#include "legacy/hrt.h"
#endif

bool HrtCommon::isEnabled() const
{
    return false;
}

bool HrtCommon::isRPOEnabled() const
{
    return false;
}

void HrtCommon::setCompType(const std::vector<sp<HWCDisplay> >& displays)
{
    for (auto& display : displays)
    {
        const int32_t disp_id = static_cast<int32_t>(display->getId());

        DispatcherJob* job = HWCDispatcher::getInstance().getExistJob(disp_id);

        if (!display->isConnected() || NULL == job || display->getMirrorSrc() != -1)
            continue;

        // only support two display at the same time
        // index 0: primary display; index 1: secondry display(MHL or vds)
        // fill display info
        int32_t gles_head = -1, gles_tail = -1;
        display->getGlesRange(&gles_head, &gles_tail);
        if (gles_head != job->layer_info.gles_head || gles_tail != job->layer_info.gles_tail)
        {
            gles_head = job->layer_info.gles_head;
            gles_tail = job->layer_info.gles_tail;
            display->setGlesRange(gles_head, gles_tail);
        }
    }
}

void HrtCommon::dump(String8* str)
{
    str->appendFormat("%s\n", m_hrt_result.str().c_str());
}

void HrtCommon::printQueryValidLayerResult()
{
    m_hrt_result.str("");
    m_hrt_result << "[HRT Interface]";
    HWC_LOGD("%s", m_hrt_result.str().c_str());
}

void HrtCommon::modifyMdpDstRoiIfRejectedByRpo(const std::vector<sp<HWCDisplay> >& displays)
{
    for (auto& disp : displays)
    {
        if (!disp->isValid())
            continue;

        for (auto& layer : disp->getVisibleLayersSortedByZ())
        {
            if (layer->getHwlayerType() == HWC_LAYER_TYPE_MM &&
                (WIDTH(layer->getMdpDstRoi()) != WIDTH(layer->getDisplayFrame()) ||
                HEIGHT(layer->getMdpDstRoi()) != HEIGHT(layer->getDisplayFrame())) &&
                (layer->getLayerCaps() & HWC_DISP_RSZ_LAYER) == 0)
            {
                layer->editMdpDstRoi() = layer->getDisplayFrame();
            }
        }
    }
}

void HrtCommon::run(std::vector<sp<HWCDisplay> >& displays, const bool& is_skip_validate)
{
    if (0 == isEnabled())
    {
        for (auto& hwc_display : displays)
        {
            if (!hwc_display->isValid())
                continue;

            if (hwc_display->getMirrorSrc() != -1)
                continue;

            int32_t gles_head = -1, gles_tail = -1;
            hwc_display->getGlesRange(&gles_head, &gles_tail);

            const int32_t disp_id = static_cast<int32_t>(hwc_display->getId());
            DispatcherJob* job = HWCDispatcher::getInstance().getExistJob(disp_id);

            if (NULL == job)
                continue;

            const int32_t max_layer = job->num_layers;
            const bool only_hwc_comp = (gles_tail == -1);
            const int32_t gles_count = only_hwc_comp ? 0 : gles_tail - gles_head + 1;
            const int32_t hwc_count = hwc_display->getVisibleLayersSortedByZ().size() - gles_count;
            const int32_t committed_count = only_hwc_comp ? hwc_count : hwc_count + 1;
            const int32_t over_layer_count = (committed_count > max_layer) ?
                committed_count - max_layer + 1: 0;
            if (over_layer_count > 0)
            {
                int32_t new_gles_tail = -1;
                int32_t new_gles_head = -1;
                if (gles_tail == -1 && gles_head == -1)
                {
                    new_gles_tail = hwc_display->getVisibleLayersSortedByZ().size() - 1;
                    new_gles_head = hwc_display->getVisibleLayersSortedByZ().size() - 1 - over_layer_count + 1;
                    hwc_display->setGlesRange(new_gles_head, new_gles_tail);
                }
                else if (gles_tail > -1 && gles_head > -1 && gles_tail >= gles_head)
                {
                    const int32_t hwc_num_after_gles_tail = hwc_display->getVisibleLayersSortedByZ().size() - 1 - gles_tail;
                    const int32_t hwc_num_before_gles_head = gles_head;
                    const int32_t excess_layer = over_layer_count > hwc_num_after_gles_tail ? over_layer_count - hwc_num_after_gles_tail - 1: 0;
                    if (excess_layer > hwc_num_before_gles_head)
                    {
                        HWC_LOGE("wrong GLES head range (%d,%d) (%d,%d)", gles_head, gles_tail, hwc_count, max_layer);
                        abort();
                    }
                    new_gles_tail = excess_layer == 0 ? gles_tail + over_layer_count - 1 : hwc_display->getVisibleLayersSortedByZ().size() - 1;
                    new_gles_head = excess_layer == 0 ? gles_head : gles_head - excess_layer;
                    hwc_display->setGlesRange(new_gles_head, new_gles_tail);
                }
                else
                {
                    HWC_LOGE("wrong GLES range (%d,%d) (%d,%d)", gles_head, gles_tail, hwc_count, max_layer);
                    abort();
                }
                hwc_display->setGlesRange(new_gles_head, new_gles_tail);

            }
        }
        return;
    }

    if (is_skip_validate)
    {
        fillLayerInfoOfDispatcherJob(displays);
        return;
    }

    fillLayerConfigList(displays);

    fillDispLayer(displays);

    if (queryValidLayer())
    {
        fillLayerInfoOfDispatcherJob(displays);
        setCompType(displays);
        if (isRPOEnabled())
        {
            modifyMdpDstRoiIfRejectedByRpo(displays);
            printQueryValidLayerResult();
        }
    }
    else
    {
        HWC_LOGE("%s: an error when hrt calculating!", __func__);

        for (auto& display : displays)
        {
            if (!display->isConnected())
                continue;

            const int32_t disp_id = static_cast<int32_t>(display->getId());
            DispatcherJob* job = HWCDispatcher::getInstance().getExistJob(disp_id);

            if (NULL == job || display->getMirrorSrc() != -1)
                continue;

            job->layer_info.max_overlap_layer_num = -1;
        }
    }
}

HrtCommon* createHrt()
{
#ifndef MTK_USE_DRM_DEVICE
    return new Hrt();
#endif
}
