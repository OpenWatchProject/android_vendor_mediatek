#include "platform_common.h"
#undef DEBUG_LOG_TAG
#define DEBUG_LOG_TAG "PLTC"
#define ATRACE_TAG ATRACE_TAG_GRAPHICS

#include <utility>

#include <cutils/properties.h>

#include <DpAsyncBlitStream2.h>

#include "utils/tools.h"
#include "platform_common.h"
extern unsigned int mapDpOrientation(const uint32_t transform);

void PlatformCommon::initOverlay()
{
}

bool PlatformCommon::isUILayerValid(const sp<HWCLayer>& layer, int32_t* line)
{
    const PrivateHandle& priv_hnd = layer->getPrivateHandle();
    if (isCompressData(&priv_hnd) && !m_config.disp_support_decompress)
    {
        *line = __LINE__;
        return false;
    }

    if (layer->isNeedPQ())
    {
        HWC_LOGD("PQ id%" PRIu64 " version(%d) size(%d) reg_value(0x%x)",
            layer->getId(),priv_hnd.pq_info.version, priv_hnd.pq_info.value_size, priv_hnd.pq_info.reg_values[0]);
        *line = __LINE__;
        return false;
    }

    if (isGameHDR(&priv_hnd))
    {
        *line = __LINE__;
        return false;
    }

    if (m_config.is_disp_support_RGBA1010102)
    {
        if (priv_hnd.format != HAL_PIXEL_FORMAT_YUYV &&
            priv_hnd.format != HAL_PIXEL_FORMAT_YCbCr_422_I &&
            priv_hnd.format != HAL_PIXEL_FORMAT_IMG1_BGRX_8888 &&
            priv_hnd.format != HAL_PIXEL_FORMAT_RGBA_FP16 &&
            priv_hnd.format != HAL_PIXEL_FORMAT_RGBA_1010102 &&
            (priv_hnd.format < HAL_PIXEL_FORMAT_RGBA_8888 ||
             priv_hnd.format > HAL_PIXEL_FORMAT_BGRA_8888))
        {
            *line = __LINE__;
            return false;
        }
    }
    else if (priv_hnd.format != HAL_PIXEL_FORMAT_YUYV &&
        priv_hnd.format != HAL_PIXEL_FORMAT_YCbCr_422_I &&
        priv_hnd.format != HAL_PIXEL_FORMAT_IMG1_BGRX_8888 &&
        priv_hnd.format != HAL_PIXEL_FORMAT_RGBA_FP16 &&
        (priv_hnd.format < HAL_PIXEL_FORMAT_RGBA_8888 ||
         priv_hnd.format > HAL_PIXEL_FORMAT_BGRA_8888))
    {
        *line = __LINE__;
        return false;
    }

    // when Platform does not support 2-subsampled format with odd size of roi, we
    // should not use YUYV. If the roi size of YUYV buffer is odd, ovl shows abnormal
    // screen content. We want to avoid the layer was processed by UI or MM constantly.
    // Therefore we always use MM to process it.
    if (m_config.support_2subsample_with_odd_size_roi == false &&
        (priv_hnd.format == HAL_PIXEL_FORMAT_YUYV ||
         priv_hnd.format == HAL_PIXEL_FORMAT_YCbCr_422_I))
    {
        *line = __LINE__;
        return false;
    }

    switch (layer->getBlend())
    {
        case HWC2_BLEND_MODE_COVERAGE:
            // hw does not support HWC_BLENDING_COVERAGE
            *line = __LINE__;
            return false;

        case HWC2_BLEND_MODE_NONE:
            // opaqaue layer should ignore alpha channel
            if (priv_hnd.format == HAL_PIXEL_FORMAT_BGRA_8888)
            {
                *line = __LINE__;
                return false;
            }
    }

    if (priv_hnd.pq_enable)
    {
        // This layer must processed by MDP PQ
        return false;
    }

    if (!getHwDevice()->isConstantAlphaForRGBASupported())
    {
        // [NOTE]
        // 1. overlay engine does not support RGBX format
        //    the only exception is that the format is RGBX and the constant alpha is 0xFF
        //    in such a situation, the display driver would disable alpha blending automatically,
        //    treating this format as RGBA with ignoring the undefined alpha channel
        // 2. overlay does not support using constant alpah
        //    and premult blending at same time
        if ((layer->getPlaneAlpha() != 1.0f) &&
            (priv_hnd.format == HAL_PIXEL_FORMAT_RGBX_8888 ||
            priv_hnd.format == HAL_PIXEL_FORMAT_IMG1_BGRX_8888 ||
            layer->getBlend() == HWC2_BLEND_MODE_PREMULTIPLIED))
        {
            *line = __LINE__;
            return false;
        }
    }

    int w = getSrcWidth(layer);
    int h = getSrcHeight(layer);

    // ovl cannot accept <=0
    if (w <= 0 || h <= 0)
    {
        *line = __LINE__;
        return false;
    }

    // [NOTE]
    // Since OVL does not support float crop, adjust coordinate to interger
    // as what SurfaceFlinger did with hwc before version 1.2
    const int src_left = getSrcLeft(layer);
    const int src_top = getSrcTop(layer);

    // cannot handle source negative offset
    if (src_left < 0 || src_top < 0)
    {
        *line = __LINE__;
        return false;
    }

    // switch width and height for prexform with ROT_90
    if (0 != priv_hnd.prexform)
    {
        DbgLogger logger(DbgLogger::TYPE_DUMPSYS, 'I',
            "prexformUI:%d x:%d, prex:%d, f:%d/%d, s:%d/%d",
            m_config.prexformUI, layer->getTransform(), priv_hnd.prexform,
            WIDTH(layer->getDisplayFrame()), HEIGHT(layer->getDisplayFrame()), w, h);

        if (0 == m_config.prexformUI)
        {
            *line = __LINE__;
            return false;
        }

        if (0 != (priv_hnd.prexform & HAL_TRANSFORM_ROT_90))
            SWAP(w, h);
    }

    // cannot handle rotation
    if (layer->getTransform() != static_cast<int32_t>(priv_hnd.prexform))
    {
        *line = __LINE__;
        return false;
    }

    // for scaling case
    if (WIDTH(layer->getDisplayFrame()) != w || HEIGHT(layer->getDisplayFrame()) != h)
    {
        if (!getHwDevice()->isDispRpoSupported())
        {
            *line = __LINE__;
            return false;
        }
        else
        {
            const uint32_t src_crop_width = (layer->getXform() & HAL_TRANSFORM_ROT_90) ?
                HEIGHT(layer->getSourceCrop()):WIDTH(layer->getSourceCrop());
            if (src_crop_width > HWCMediator::getInstance().getOvlDevice(HWC_DISPLAY_PRIMARY)->getRszMaxWidthInput() &&
                (priv_hnd.format == HAL_PIXEL_FORMAT_RGBX_8888 ||
                 priv_hnd.format == HAL_PIXEL_FORMAT_BGRX_8888 ||
                 priv_hnd.format == HAL_PIXEL_FORMAT_RGB_888 ||
                 priv_hnd.format == HAL_PIXEL_FORMAT_RGB_565))
            {
                *line = __LINE__;
                return false;
            }
        }
    }
    *line = __LINE__;
    return true;
}

bool PlatformCommon::isMMLayerValid(const sp<HWCLayer>& layer, int32_t* line)
{
    if (layer->getBlend() == HWC2_BLEND_MODE_COVERAGE)
    {
        // only use MM layer without any blending consumption
        *line = __LINE__;
        return false;
    }

    if (!getHwDevice()->isConstantAlphaForRGBASupported())
    {
        // [NOTE]
        // 1. overlay engine does not support RGBX format
        //    the only exception is that the format is RGBX and the constant alpha is 0xFF
        //    in such a situation, the display driver would disable alpha blending automatically,
        //    treating this format as RGBA with ignoring the undefined alpha channel
        // 2. overlay does not support using constant alpah
        //    and premult blending at same time
        const PrivateHandle& priv_hnd = layer->getPrivateHandle();
        if ((layer->getPlaneAlpha() != 1.0f) &&
            (priv_hnd.format == HAL_PIXEL_FORMAT_RGBX_8888 ||
            priv_hnd.format == HAL_PIXEL_FORMAT_IMG1_BGRX_8888 ||
            layer->getBlend() == HWC2_BLEND_MODE_PREMULTIPLIED))
        {
            *line = __LINE__;
            return false;
        }
    }

    const int srcWidth = getSrcWidth(layer);
    const int srcHeight = getSrcHeight(layer);
    const int dstWidth = WIDTH(layer->getDisplayFrame());
    const int dstHeight = HEIGHT(layer->getDisplayFrame());
    if (srcWidth < 4 || srcHeight < 4 ||
        dstWidth < 4 || dstHeight < 4)
    {
        // Prevent bliter error.
        // RGB serise buffer bound with HW limitation, must large than 3x3
        // YUV serise buffer need to prevent width/height align to 0
        *line = __LINE__;
        return false;
    }

    const int srcLeft = getSrcLeft(layer);
    const int srcTop = getSrcTop(layer);
    if (srcLeft < 0 || srcTop < 0)
    {
        // cannot handle source negative offset
        *line = __LINE__;
        return false;
    }

    const PrivateHandle& priv_hnd = layer->getPrivateHandle();

    int curr_private_format = NOT_PRIVATE_FORMAT;
    if (HAL_PIXEL_FORMAT_YUV_PRIVATE == priv_hnd.format ||
        HAL_PIXEL_FORMAT_YCbCr_420_888 == priv_hnd.format ||
        HAL_PIXEL_FORMAT_YUV_PRIVATE_10BIT == priv_hnd.format)
    {
        curr_private_format = (priv_hnd.ext_info.status & GRALLOC_EXTRA_MASK_CM);
    }
    const uint32_t fmt = grallocColor2HalColor(priv_hnd.format, curr_private_format);
    if (DP_COLOR_UNKNOWN == mapDpFormat(fmt))
    {
        *line = __LINE__;
        return false;
    }

    if ((priv_hnd.format == HAL_PIXEL_FORMAT_RGBA_1010102 && !m_config.is_mdp_support_RGBA1010102) ||
        priv_hnd.format == HAL_PIXEL_FORMAT_RGBA_FP16)
    {
        *line = __LINE__;
        return false;
    }

    if (isCompressData(&priv_hnd) && !m_config.mdp_support_decompress)
    {
        *line = __LINE__;
        return false;
    }

    int32_t layer_caps = 0;
    if (priv_hnd.format == HAL_PIXEL_FORMAT_RGBA_8888 ||
        priv_hnd.format == HAL_PIXEL_FORMAT_BGRA_8888)
    {
        if (!m_config.enable_rgba_rotate)
        {
            // MDP cannot handle RGBA scale and rotate.
            *line = __LINE__;
            return false;
        }

        // MDP doesn't support RGBA scale, it must handle by DISP_RSZ or GLES
        if (layer->needScaling() &&
            !HWCMediator::getInstance().getOvlDevice(HWC_DISPLAY_PRIMARY)->isDispRpoSupported())
        {
            // Both of MDP and DISP cannot handle RGBA scaling
            *line = __LINE__;
            return false;
        }

        layer_caps |= layer->needRotate() ? HWC_MDP_ROT_LAYER : 0;
        if ((layer_caps & HWC_MDP_ROT_LAYER) &&
            (srcLeft != 0 || srcTop != 0 || srcWidth <= 8))
        {
            // MDP cannot handle RGBA rotate which buffer not align LR corner
            HWC_LOGD("RGBA rotate cannot handle by HWC such src(x,y,w,h)=(%d,%d,%d,%d) dst(w,h)=(%d,%d)",
                        srcLeft, srcTop, srcWidth, srcHeight, dstWidth, dstHeight);
            *line = __LINE__;
            return false;
        }
    }
    else if (priv_hnd.format == HAL_PIXEL_FORMAT_RGBX_8888 ||
             priv_hnd.format == HAL_PIXEL_FORMAT_BGRX_8888 ||
             priv_hnd.format == HAL_PIXEL_FORMAT_RGB_888 ||
             priv_hnd.format == HAL_PIXEL_FORMAT_RGB_565)
    {
        if (layer->needScaling())
        {
            if (!m_config.enable_rgbx_scaling &&
                !HWCMediator::getInstance().getOvlDevice(HWC_DISPLAY_PRIMARY)->isDispRpoSupported())
            {
                // Both of MDP and DISP cannot handle RGBX scaling
                *line = __LINE__;
                return false;
            }
            layer_caps |= m_config.enable_rgbx_scaling ? HWC_MDP_RSZ_LAYER : 0;
        }

        layer_caps |= layer->needRotate() ? HWC_MDP_ROT_LAYER : 0;
        if ((layer_caps & HWC_MDP_ROT_LAYER) &&
            (srcLeft != 0 || srcTop != 0 || srcWidth <= 8))
        {
            // MDP cannot handle RGBX rotate which buffer not align LR corner
            HWC_LOGD("RGBX rotate cannot handle by HWC such src(x,y,w,h)=(%d,%d,%d,%d) dst(w,h)=(%d,%d)",
                        srcLeft, srcTop, srcWidth, srcHeight, dstWidth, dstHeight);
            *line = __LINE__;
            return false;
        }
    }
    else
    {
        layer_caps |= layer->needRotate() ? HWC_MDP_ROT_LAYER : 0;
        const double& mdp_scale_percentage = m_config.mdp_scale_percentage;
        if (layer->needScaling() &&
            !(fabs(mdp_scale_percentage - 0.0f) < 0.05f))
        {
            layer_caps |= HWC_MDP_RSZ_LAYER;
        }
    }

    // if it is swdec with HDR content, uses default HDR flow in surfaceflinger
    const int&& type = (priv_hnd.ext_info.status & GRALLOC_EXTRA_MASK_TYPE);
    const uint32_t ds = (layer->getDataspace() & HAL_DATASPACE_STANDARD_MASK);
    const HwcHdrMetadata metadata = layer->getPerFrameMetadata();
    const bool is_hdr = (ds == HAL_DATASPACE_STANDARD_BT2020) ||
                        (metadata.type & SMPTE2086) == SMPTE2086 ||
                        (metadata.type & CTA861_3) == CTA861_3 ||
                        isGameHDR(&priv_hnd);

    if ((type != GRALLOC_EXTRA_BIT_TYPE_VIDEO) && is_hdr && !isGameHDR(&priv_hnd))
    {
        *line = __LINE__;
        return false;
    }

    if (is_hdr)
    {
        layer_caps |= HWC_MDP_HDR_LAYER;
    }

    // Because MDP can not process odd width and height, we will calculate the
    // crop area and roi later. Then we may adjust the size of source and
    // destination buffer. This behavior may cause that the scaling rate
    // increases, and therefore the scaling rate is over the limitation of MDP.
    sp<HWCDisplay> disp = layer->getDisplay().promote();
    bool is_p3 = isP3(layer->getDataspace());
    bool is_uipq = isUIPq(&(layer->getPrivateHandle()));
    int32_t dst_dataspace = getDstDataspace(layer->getDataspace());
    int assigned_output_format = -1;
    uint32_t pq_enhance = 0;
    DpPqParam dppq_param;
    const bool is_game = layer->isNeedPQ() || isGameHDR(&layer->getPrivateHandle());
    setPQEnhance(disp->getId(), layer->getPrivateHandle(), &pq_enhance, &assigned_output_format, is_game);
    setPQParam(&dppq_param, pq_enhance, is_uipq, layer->getPrivateHandle().ext_info.pool_id, is_p3,
            dst_dataspace, layer->getPrivateHandle().ext_info.timestamp,
            layer->getPrivateHandle().handle, layer->getPrivateHandle().pq_table_idx, is_game);

    DpRect src_roi;
    src_roi.x = (int)(layer->getSourceCrop().left);
    src_roi.y = (int)(layer->getSourceCrop().top);
    src_roi.w = WIDTH(layer->getSourceCrop());
    src_roi.h = HEIGHT(layer->getSourceCrop());

    const bool is_blit_valid =
        DpAsyncBlitStream2::queryHWSupport(
            srcWidth, srcHeight, dstWidth, dstHeight, mapDpOrientation(layer->getXform()),
            DP_COLOR_UNKNOWN, DP_COLOR_UNKNOWN, &dppq_param, &src_roi) &&
        DpAsyncBlitStream2::queryHWSupport(
            srcWidth - 1, srcHeight - 1, dstWidth + 2, dstHeight + 2, mapDpOrientation(layer->getXform()),
            DP_COLOR_UNKNOWN, DP_COLOR_UNKNOWN, &dppq_param, &src_roi);

    if (!is_blit_valid)
    {
        *line = __LINE__;
        return false;
    }

    const int secure = (priv_hnd.usage & (GRALLOC_USAGE_PROTECTED | GRALLOC_USAGE_SECURE));

    bool is_disp_valid = true;
    if (disp != nullptr)
    {
        is_disp_valid = (disp->getId() == HWC_DISPLAY_PRIMARY) || secure ||
                         (!(dstWidth & 0x01) && !(dstHeight & 0x01));
    }

    if (!is_disp_valid)
    {
        *line = __LINE__;
        return false;
    }

    layer->setLayerCaps(layer->getLayerCaps() | layer_caps);
    *line = __LINE__;
    return true;
}

size_t PlatformCommon::getLimitedVideoSize()
{
    // 4k resolution
    return 3840 * 2160;
}

size_t PlatformCommon::getLimitedExternalDisplaySize()
{
    // 2k resolution
    return 2048 * 1080;
}

bool PlatformCommon::isLowMemoryDevice()
{
    char value[PROPERTY_VALUE_MAX];
    int len = property_get("ro.config.low_ram", value, "0");
    if (len > 1 && (!strcmp(value, "yes") || !strcmp(value, "true"))) {
        return true;
    }
    return false;
}

PlatformCommon::PlatformConfig::PlatformConfig()
    : platform(PLATFORM_NOT_DEFINE)
    , compose_level(COMPOSE_DISABLE_ALL)
    , mirror_state(MIRROR_DISABLED)
    , mir_scale_ratio(0.0f)
    , format_mir_mhl(MIR_FORMAT_UNDEFINE)
#ifdef BYPASS_WLV1_CHECKING
    , bypass_wlv1_checking(true)
#else
    , bypass_wlv1_checking(false)
#endif
    , prexformUI(1)
    , rdma_roi_update(0)
    , force_full_invalidate(false)
    , use_async_bliter_ultra(false)
    , wait_fence_for_display(false)
    , enable_smart_layer(false)
    , enable_rgba_rotate(false)
    , enable_rgbx_scaling(true)
    , av_grouping(true)
    , dump_buf_type('A')
    , dump_buf(0)
    , dump_buf_cont_type('A')
    , dump_buf_cont(0)
    , dump_buf_log_enable(false)
    , fill_black_debug(false)
    , always_setup_priv_hnd(false)
    , uipq_debug(false)
#ifdef MTK_USER_BUILD
    , wdt_trace(false)
#else
    , wdt_trace(true)
#endif
    , only_wfd_by_hwc(false)
    , only_wfd_by_dispdev(false)
    , blitdev_for_virtual(false)
    , is_support_ext_path_for_virtual(true)
    , is_skip_validate(true)
    , support_color_transform(false)
    , mdp_scale_percentage(1.f)
    , extend_mdp_capacity(false)
    , rpo_ui_max_src_width(0)
    , disp_support_decompress(false)
    , mdp_support_decompress(false)
    , disp_wdma_fmt_for_vir_disp(HAL_PIXEL_FORMAT_RGB_888)
    , disable_color_transform_for_secondary_displays(true)
    , remove_invisible_layers(true)
    , latch_unsignaled_buffer(true)
    , use_dataspace_for_yuv(false)
    , fill_hwdec_hdr(false)
    , is_support_mdp_pmqos(false)
    , is_support_mdp_pmqos_debug(false)
    , buffer_slots(0)
    , is_disp_support_RGBA1010102(false)
    , force_pq_index(-1)
    , is_mdp_support_RGBA1010102(false)
    , support_2subsample_with_odd_size_roi(true)
{
    char value[PROPERTY_VALUE_MAX];
    property_get("vendor.debug.hwc.blitdev_for_virtual", value, "-1");
    const int32_t num_value = atoi(value);
    if (-1 != num_value)
    {
        if (num_value)
        {
            blitdev_for_virtual = true;
            is_support_ext_path_for_virtual = true;
        }
        else
        {
            blitdev_for_virtual = false;
        }
    }
}
