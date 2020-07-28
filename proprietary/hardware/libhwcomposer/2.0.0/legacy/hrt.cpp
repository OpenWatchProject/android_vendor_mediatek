#define DEBUG_LOG_TAG "HRT"

#include "hrt.h"

#include <vector>

#include "hwdev.h"
#include "overlay.h"
#include "sync.h"
#include "dispatcher.h"
#include "hwc2.h"
#include "dev_interface.h"

static int32_t mapLayeringCaps2HwcLayeringCaps(unsigned int caps)
{
    int32_t tmp = 0;

    if (caps & LAYERING_OVL_ONLY)
    {
        tmp |= HWC_LAYERING_OVL_ONLY;
    }

    if (caps & MDP_RSZ_LAYER)
    {
        tmp |= HWC_MDP_RSZ_LAYER;
    }

    if (caps & DISP_RSZ_LAYER)
    {
        tmp |= HWC_DISP_RSZ_LAYER;
    }

    if (caps & MDP_ROT_LAYER)
    {
        tmp |= HWC_MDP_ROT_LAYER;
    }

    if (caps & MDP_HDR_LAYER)
    {
        tmp |= HWC_MDP_HDR_LAYER;
    }

    if (caps & NO_FBDC)
    {
        tmp |= HWC_NO_FBDC;
    }

    return tmp;
}

static unsigned int mapHwcLayeringCaps2LayeringCaps(int32_t caps)
{
    unsigned int tmp = 0;

    if (caps & HWC_LAYERING_OVL_ONLY)
    {
        tmp |= LAYERING_OVL_ONLY;
    }

    if (caps & HWC_MDP_RSZ_LAYER)
    {
        tmp |= MDP_RSZ_LAYER;
    }

    if (caps & HWC_DISP_RSZ_LAYER)
    {
        tmp |= DISP_RSZ_LAYER;
    }

    if (caps & HWC_MDP_ROT_LAYER)
    {
        tmp |= MDP_ROT_LAYER;
    }

    if (caps & HWC_MDP_HDR_LAYER)
    {
        tmp |= MDP_HDR_LAYER;
    }

    if (caps & HWC_NO_FBDC)
    {
        tmp |= NO_FBDC;
    }

    return tmp;
}

bool Hrt::isEnabled() const
{
    return 0 != HWCMediator::getInstance().getOvlDevice(HWC_DISPLAY_PRIMARY)->isDisplayHrtSupport();
}

bool Hrt::isRPOEnabled() const
{
    return HWCMediator::getInstance().getOvlDevice(HWC_DISPLAY_PRIMARY)->isDispRpoSupported();
}

void Hrt::fillLayerConfigList(const std::vector<sp<HWCDisplay> >& displays)
{
    for (auto& display : displays)
    {
        if (!display->isConnected())
            continue;

        const int32_t disp_id = static_cast<int32_t>(display->getId());
        const std::vector<sp<HWCLayer> >& layers = display->getVisibleLayersSortedByZ();
        const int&& layers_num = layers.size();

        // reallocate layer_config_list if needed
        if (layers_num > m_layer_config_len[disp_id])
        {
            if (NULL != m_layer_config_list[disp_id])
                free(m_layer_config_list[disp_id]);

            m_layer_config_len[disp_id] = layers_num;
            m_layer_config_list[disp_id] = (layer_config*)calloc(m_layer_config_len[disp_id], sizeof(layer_config));
            if (NULL == m_layer_config_list[disp_id])
            {
                HWC_LOGE("(%d) Failed to malloc layer_config_list (len=%d)", disp_id, layers_num);
                m_layer_config_len[disp_id] = 0;
                return;
            }
        }

        // init and get PrivateHandle
        layer_config* layer_config = m_layer_config_list[disp_id];

        for (auto& layer : layers)
        {
            layer_config->ovl_id        = -1;
            layer_config->ext_sel_layer = -1;
            layer_config->src_fmt       =
                (layer->getHwlayerType() == HWC_LAYER_TYPE_DIM) ?
                    DISP_FORMAT_DIM : convertFormat4Hrt(layer->getPrivateHandle().format);
            if (layer->getOverrideMDPOutputFormat() != DISP_FORMAT_UNKNOWN)
            {
                layer_config->src_fmt = layer->getOverrideMDPOutputFormat();
            }
            layer_config->src_offset_y  = getSrcTop(layer);
            layer_config->src_offset_x  = getSrcLeft(layer);
            layer_config->dst_offset_y  = getDstTop(layer);
            layer_config->dst_offset_x  = getDstLeft(layer);
            layer_config->dst_width     = getDstWidth(layer);
            layer_config->dst_height    = getDstHeight(layer);
            layer_config->layer_caps    = mapHwcLayeringCaps2LayeringCaps(layer->getLayerCaps());
            layer_config->dataspace     = layer->getDataspace();
            layer_config->compress      = isCompressData(&(layer->getPrivateHandle()));

            const PrivateHandle& priv_hnd = layer->getPrivateHandle();
            switch(layer->getHwlayerType())
            {
                case HWC_LAYER_TYPE_DIM:
                    layer_config->src_width = getDstWidth(layer);
                    layer_config->src_height = getDstHeight(layer);
                    break;

                case HWC_LAYER_TYPE_MM:
                    layer_config->src_width = WIDTH(layer->getMdpDstRoi());
                    layer_config->src_height = HEIGHT(layer->getMdpDstRoi());
                    break;

                default:
                    if (layer->getHwlayerType() == HWC_LAYER_TYPE_UI &&
                        (priv_hnd.prexform & HAL_TRANSFORM_ROT_90))
                    {
                        layer_config->src_width  = getSrcHeight(layer);
                        layer_config->src_height = getSrcWidth(layer);
                    }
                    else
                    {
                        layer_config->src_width  = getSrcWidth(layer);
                        layer_config->src_height = getSrcHeight(layer);
                    }
                    break;
            }

            ++layer_config;
        }
    }
}

void Hrt::fillDispLayer(const std::vector<sp<HWCDisplay> >& displays)
{
    memset(&m_disp_layer, 0, sizeof(disp_layer_info));
    m_disp_layer.hrt_num = -1;
    for (auto& display : displays)
    {
        const int32_t disp_id = static_cast<int32_t>(display->getId());
        const size_t disp_input = (disp_id == HWC_DISPLAY_PRIMARY) ? 0 : 1;

        m_disp_layer.gles_head[disp_input] = -1;
        m_disp_layer.gles_tail[disp_input] = -1;
    }

    // prepare disp_layer_info for ioctl
    for (auto& display : displays)
    {
        // driver only supports two displays at the same time
        // disp_input 0: primary display; disp_input 1: secondry display(MHL or vds)
        // fill display info
        const uint64_t disp_id = display->getId();
        const size_t disp_input = (disp_id == HWC_DISPLAY_PRIMARY) ? 0 : 1;

        if (!display->isConnected() ||
            display->getMirrorSrc() != -1 ||
            HWCMediator::getInstance().getOvlDevice(display->getId())->getType() == OVL_DEVICE_TYPE_BLITDEV)
        {
            continue;
        }

        const int layers_num = display->getVisibleLayersSortedByZ().size();

        m_disp_layer.input_config[disp_input] = m_layer_config_list[disp_id];

        switch (disp_id) {
            case HWC_DISPLAY_PRIMARY:
                m_disp_layer.disp_mode[disp_input] =
                    mapHwcDispMode2Disp(HWCMediator::getInstance().getOvlDevice(HWC_DISPLAY_PRIMARY)->getOverlaySessionMode(disp_id));
                m_disp_layer.active_config_id[disp_input] = display->getActiveConfig();
                break;

            case HWC_DISPLAY_EXTERNAL:
                m_disp_layer.disp_mode[disp_input] = DISP_SESSION_DIRECT_LINK_MODE;
                break;

            case HWC_DISPLAY_VIRTUAL:
                m_disp_layer.disp_mode[disp_input] = DISP_SESSION_DECOUPLE_MODE;
                break;

            default:
                HWC_LOGE("%s: Unknown disp_id(" PRIu64 ")", __func__, disp_id);
        }

        m_disp_layer.layer_num[disp_input] =
            (m_layer_config_len[disp_id] < layers_num) ? m_layer_config_len[disp_id] : layers_num;

        display->getGlesRange(
            &m_disp_layer.gles_head[disp_input],
            &m_disp_layer.gles_tail[disp_input]);
        HWC_LOGV("%s disp:%" PRIu64 " m_disp_layer.gles_head[disp_input]:%d, m_disp_layer.gles_tail[disp_input]:%d",
            __func__, disp_id, m_disp_layer.gles_head[disp_input],m_disp_layer.gles_tail[disp_input] );
    }
}

void Hrt::fillLayerInfoOfDispatcherJob(const std::vector<sp<HWCDisplay> >& displays)
{
    // DbgLogger logger(DbgLogger::TYPE_HWC_LOG, 'D',"fillLayerInfoOfDispatcherJob()");
    for (auto& display : displays)
    {
        const int32_t disp_id = static_cast<int32_t>(display->getId());
        DispatcherJob* job = HWCDispatcher::getInstance().getExistJob(disp_id);

        if (!display->isConnected() || NULL == job || display->getMirrorSrc() != -1)
        {
            HWC_LOGV("fillLayerInfoOfDispatcherJob() job:%p display->getMirrorSrc():%d", job, display->getMirrorSrc());
            continue;
        }

        // only support two display at the same time
        // index 0: primary display; index 1: secondry display(MHL or vds)
        // fill display info
        const int disp_input = (disp_id == HWC_DISPLAY_PRIMARY) ? 0 : 1;

        if (m_disp_layer.input_config[disp_input] == NULL)
        {
            const std::vector<sp<HWCLayer> >& layers = display->getVisibleLayersSortedByZ();
            job->layer_info.max_overlap_layer_num = -1;
            job->layer_info.hrt_weight = 0;
            job->layer_info.hrt_idx    = 0;

            const int&& layers_num = layers.size();
            job->layer_info.gles_head = layers_num ? 0 : -1;
            job->layer_info.gles_tail = layers_num - 1;
            HWC_LOGV("fillLayerInfoOfDispatcherJob() disp:%d gles_head:%d gles_tail:%d with no hrt config",
                disp_id, job->layer_info.gles_head, job->layer_info.gles_tail);
            continue;
        }
        else
        {
            job->layer_info.max_overlap_layer_num = m_disp_layer.hrt_num;
            job->layer_info.hrt_weight = m_disp_layer.hrt_weight;
            job->layer_info.hrt_idx    = m_disp_layer.hrt_idx;
            job->layer_info.gles_head  = m_disp_layer.gles_head[disp_input];
            job->layer_info.gles_tail  = m_disp_layer.gles_tail[disp_input];
            HWC_LOGV("fillLayerInfoOfDispatcherJob() disp:%d gles_head:%d gles_tail:%d",
                disp_id, job->layer_info.gles_head, job->layer_info.gles_tail);
        }

        // fill layer info
        if (m_layer_config_len[disp_id] > m_hrt_config_len[disp_input])
        {
            int layers_num = m_layer_config_len[disp_id];
            if (NULL != m_hrt_config_list[disp_input])
                delete m_hrt_config_list[disp_input];

            m_hrt_config_len[disp_input] = layers_num;
            m_hrt_config_list[disp_input] = new HrtLayerConfig[layers_num];
            if (NULL == m_hrt_config_list[disp_input])
            {
                HWC_LOGE("(%d) Failed to malloc hrt_config_list (len=%d)", disp_id, layers_num);
                m_hrt_config_len[disp_input] = 0;
                return;
            }
        }
        for (int i = 0; i < m_layer_config_len[disp_id]; i++)
        {
            m_hrt_config_list[disp_input][i].ovl_id = m_disp_layer.input_config[disp_input][i].ovl_id;
            m_hrt_config_list[disp_input][i].ext_sel_layer = m_disp_layer.input_config[disp_input][i].ext_sel_layer;
        }
        job->layer_info.hrt_config_list = m_hrt_config_list[disp_input];

        for (size_t i = 0; i < display->getVisibleLayersSortedByZ().size(); ++i)
        {
            if (static_cast<int32_t>(i) >= m_disp_layer.layer_num[disp_input])
                break;

            auto& layer = display->getVisibleLayersSortedByZ()[i];
            layer->setLayerCaps(mapLayeringCaps2HwcLayeringCaps(m_disp_layer.input_config[disp_input][i].layer_caps));
        }
        // for (int32_t i = 0 ; i < m_disp_layer.layer_num[disp_input]; ++i)
        //    logger.printf("i:%d ovl_id:%d caps:%d, ", i, m_disp_layer.input_config[disp_input][i].ovl_id, m_disp_layer.input_config[disp_input][i].layer_caps);
    }
}

void Hrt::printQueryValidLayerResult()
{
    m_hrt_result.str("");
    m_hrt_result << "[HRT]";
    for (int32_t i = 0; i < 2; ++i)
    {
        m_hrt_result << " [(" << i << ") active_config_id:" << m_disp_layer.active_config_id[i] <<"]";
        for (int32_t j = 0; j < m_disp_layer.layer_num[i]; ++j)
        {
            const auto& cfg = m_disp_layer.input_config[i][j];
            m_hrt_result << " [(" << i << "," << j <<
                ") s_wh:" << cfg.src_width << "," << cfg.src_height <<
                " d_xywh:" << cfg.dst_offset_x << "," << cfg.dst_offset_y << "," << cfg.dst_width << ","<< cfg.dst_height <<
                " caps:" << cfg.layer_caps << "]";
        }
    }
    HWC_LOGD("%s", m_hrt_result.str().c_str());
}

bool Hrt::queryValidLayer()
{
    return HWCMediator::getInstance().getOvlDevice(HWC_DISPLAY_PRIMARY)->queryValidLayer(&m_disp_layer);
}
