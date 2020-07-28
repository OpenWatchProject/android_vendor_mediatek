#include "DpEngineBase.h"
#include "DpEngineType.h"
#include "DpTileScaler.h"
#if CONFIG_FOR_VERIFY_FPGA
#include "ConfigInfo.h"
#include "TestControl.h"

DP_STATUS_ENUM DpEngineBase::configFrameISP(DpConfig &config)
{
    config.scenario = STREAM_ISP_IC;
    //config.ispMode  = 2;

    config.inWidth   = cfg.ISP_IN_WIDTH;
    config.inHeight  = cfg.ISP_IN_HEIGHT;
    config.outWidth  = cfg.ISP_IN_WIDTH;
    config.outHeight = cfg.ISP_IN_HEIGHT;
    config.inFormat  = eBAYER8; //Holmes is it needed?

    return DP_STATUS_RETURN_SUCCESS;
}

DP_STATUS_ENUM DpEngineBase::configFrameRotate(DpConfig &config)
{
    return DP_STATUS_RETURN_SUCCESS;
}

DP_STATUS_ENUM DpEngineBase::configFrameFlip(DpConfig &config)
{
    return DP_STATUS_RETURN_SUCCESS;
}

DP_STATUS_ENUM DpEngineBase::configFrameAlphaRot(DpConfig &config)
{
    const uint32_t id = m_identifier;
    int64_t feature;

    feature = queryFeature();

    if (feature & eRMEM) // RDMA
    {
        config.enAlphaRot = cfg.MDP_RDMA_alpha_rot[id];
    }
    else // WROT
    {
        config.enAlphaRot = cfg.MDP_WROT_alpha_rot[id];
    }
    return DP_STATUS_RETURN_SUCCESS;
}

/* ============= RDMA test register =============
    MDP_RDMA0_mf_src_w
    MDP_RDMA0_mf_src_h
    MDP_RDMA0_src_format
    MDP_RDMA0_swap
    MDP_RDMA0_block
    MDP_RDMA0_vdo_mode
    MDP_RDMA0_ring_buf_read
*/
DP_STATUS_ENUM DpEngineBase::configFrameRead(DpCommand &command, DpConfig &config)
{
    const uint32_t id = m_identifier;

    DpColorFormat colorFormat = mdp_test::getInstance()->mapUnifiedColor(cfg.MDP_RDMA_src_format[id],
                                                                         cfg.MDP_RDMA_swap[id],
                                                                         cfg.MDP_RDMA_block[id],
                                                                         cfg.MDP_RDMA_vdo_mode[id],
                                                                         cfg.MDP_RDMA_ufo_dec_en[id],
                                                                         cfg.MDP_RDMA_10bit_packed[id],
                                                                         cfg.MDP_RDMA_10bit_tile_mode[id]);

    uint32_t plane_num = DP_COLOR_GET_PLANE_COUNT(colorFormat);
    uint32_t width = cfg.MDP_RDMA_mf_src_w[id];
    uint32_t height = cfg.MDP_RDMA_mf_src_h[id];
    uint32_t pitch_y = DP_COLOR_GET_MIN_Y_PITCH(colorFormat, width);
    uint32_t pitch_uv = DP_COLOR_GET_MIN_UV_PITCH(colorFormat, width);
    uint32_t height_uv = height >> DP_COLOR_GET_V_SUBSAMPLE(colorFormat);
    uint32_t size[3];

    size[0] = DP_COLOR_GET_MIN_Y_SIZE(colorFormat, width, height);
    size[1] = (plane_num > 1) ? DP_COLOR_GET_MIN_UV_SIZE(colorFormat, width, height) : 0;
    size[2] = (plane_num > 2) ? DP_COLOR_GET_MIN_UV_SIZE(colorFormat, width, height) : 0;

    cfg.MDP_RDMA_mf_bkgd_wb[id] = pitch_y;
    cfg.MDP_RDMA_sf_bkgd_wb[id] = pitch_uv;

    if (cfg.MDP_RDMA_ufo_dec_en[id])
    {
        uint32_t u4PIC_SIZE_Y     = ((width * height + 511) >> 9) << 9;
        uint32_t u4PIC_SIZE_Y_BS;
        uint32_t u4PIC_SIZE_BS;
        if (0 != DP_COLOR_GET_10BIT_PACKED(colorFormat))
        {
            u4PIC_SIZE_Y_BS  = ((((u4PIC_SIZE_Y * 5 >> 2) + 4095) >> 12) << 12);
            u4PIC_SIZE_BS    = ((u4PIC_SIZE_Y_BS + (u4PIC_SIZE_Y * 5 >> 3) + 511) >> 9) << 9;
        }
        else
        {
            u4PIC_SIZE_Y_BS  = (((u4PIC_SIZE_Y + 4095) >> 12) << 12);
            u4PIC_SIZE_BS    = ((u4PIC_SIZE_Y_BS + (u4PIC_SIZE_Y >> 1) + 511) >> 9) << 9;
        }
        uint32_t u4UFO_LEN_SIZE_Y = ((((u4PIC_SIZE_Y + 255) >> 8) + 63 + (16*8)) >> 6) << 6;
        uint32_t u4UFO_LEN_SIZE_C = (((u4UFO_LEN_SIZE_Y >> 1) + 15 + (16*8)) >> 4) << 4;

        cfg.MDP_RDMA_src_base_1[id] = cfg.MDP_RDMA_src_base_0[id] + u4PIC_SIZE_Y_BS;
        cfg.MDP_RDMA_ufo_dec_length_base_y[id] = cfg.MDP_RDMA_src_base_0[id] + u4PIC_SIZE_BS;
        cfg.MDP_RDMA_ufo_dec_length_base_c[id] = cfg.MDP_RDMA_ufo_dec_length_base_y[id] + u4UFO_LEN_SIZE_Y;

        transferred_cfg.RDMA0_ufo_size[0] = u4PIC_SIZE_Y_BS;
        transferred_cfg.RDMA0_ufo_size[1] = u4PIC_SIZE_BS;
        transferred_cfg.RDMA0_ufo_size[2] = u4UFO_LEN_SIZE_Y;
        transferred_cfg.RDMA0_ufo_size[3] = u4UFO_LEN_SIZE_C;
    }
    else
    {
        cfg.MDP_RDMA_src_base_1[id] = cfg.MDP_RDMA_src_base_0[id] + size[0];
        cfg.MDP_RDMA_src_base_2[id] = cfg.MDP_RDMA_src_base_1[id] + size[1];
    }

    // Prepare transfer config
    if (cfg.MDP_RDMA_block[id])
    {
        pitch_y = (DP_COLOR_BITS_PER_PIXEL(colorFormat) * width) >> 8;
        pitch_uv = (DP_COLOR_BITS_PER_PIXEL(colorFormat) * width) >> 8;
    }
    if (0 == id)
    {
        transferred_cfg.RDMA0_pitch[0] = pitch_y;
        transferred_cfg.RDMA0_pitch[1] = (plane_num > 1) ? pitch_uv : 0;
        transferred_cfg.RDMA0_pitch[2] = (plane_num > 2) ? pitch_uv : 0;
        transferred_cfg.RDMA0_height[0] = height;
        transferred_cfg.RDMA0_height[1] = (plane_num > 1) ? height_uv : 0;
        transferred_cfg.RDMA0_height[2] = (plane_num > 2) ? height_uv : 0;
        transferred_cfg.RDMA0_plane = plane_num;
    }
    else
    {
        transferred_cfg.RDMA1_pitch[0] = pitch_y;
        transferred_cfg.RDMA1_pitch[1] = (plane_num > 1) ? pitch_uv : 0;
        transferred_cfg.RDMA1_pitch[2] = (plane_num > 2) ? pitch_uv : 0;
        transferred_cfg.RDMA1_height[0] = height;
        transferred_cfg.RDMA1_height[1] = (plane_num > 1) ? height_uv : 0;
        transferred_cfg.RDMA1_height[2] = (plane_num > 2) ? height_uv : 0;
        transferred_cfg.RDMA1_plane = plane_num;
    }

    // Prepare port config
    config.memAddr[0] = cfg.MDP_RDMA_src_base_0[id];
    config.memAddr[1] = cfg.MDP_RDMA_src_base_1[id];
    config.memAddr[2] = cfg.MDP_RDMA_src_base_2[id];
    config.memSize[0] = size[0];
    config.memSize[1] = size[1];
    config.memSize[2] = size[2];

    config.inFormat   = colorFormat;
    config.inWidth    = width;
    config.inHeight   = height;
    config.inYPitch   = cfg.MDP_RDMA_mf_bkgd_wb[id];
    config.inUVPitch  = cfg.MDP_RDMA_sf_bkgd_wb[id];

    config.inXOffset = 0;
    config.inYOffset = 0;
    config.inCropWidth = cfg.MDP_RDMA_mf_clip_w[id];
    config.inCropHeight = cfg.MDP_RDMA_mf_clip_h[id];

    if ((config.inCropWidth != config.inWidth) ||
        (config.inCropHeight != config.inHeight))
    {
        config.enRDMACrop = true;
    }

    config.outWidth   = cfg.MDP_RDMA_mf_clip_w[id];
    config.outHeight  = cfg.MDP_RDMA_mf_clip_h[id];
    config.enBottomField = cfg.MDP_RDMA_vdo_field[id] ? true : false;

    config.enUFODec = cfg.MDP_RDMA_ufo_dec_en[id] ? true : false;
    config.memUFOLenAddr[0] = cfg.MDP_RDMA_ufo_dec_length_base_y[id];
    config.memUFOLenAddr[1] = cfg.MDP_RDMA_ufo_dec_length_base_c[id];

    return DP_STATUS_RETURN_SUCCESS;
}

DP_STATUS_ENUM DpEngineBase::configFrameRingBuf(DpConfig &config)
{
    config.enRing = cfg.ring_buf_read ? true : false;
#ifdef RING_BUFFER
    if (cfg.ring_buf_read)
    {
        uint32_t ringBase[3];
        uint32_t ringSize[3];

        m_pRingBuffer->getHWBaseWithID(0,
                                       getEngineType(),
                                       ringBase,
                                       ringSize);

        // Set ring buffer source
        frameInfo.memAddr[0] = ringBase[0];
        frameInfo.memAddr[1] = ringBase[1];
        frameInfo.memAddr[2] = ringBase[2];

        frameInfo.memSize[0] = ringSize[0];
        frameInfo.memSize[1] = ringSize[1];
        frameInfo.memSize[2] = ringSize[2];

        transferred_cfg.RDMA0_ring_addr[0] = ringBase[0];
        transferred_cfg.RDMA0_ring_addr[1] = ringBase[1];
        transferred_cfg.RDMA0_ring_addr[2] = ringBase[2];

        transferred_cfg.RDMA0_ring_size[0] = ringSize[0];
        transferred_cfg.RDMA0_ring_size[1] = ringSize[1];
        transferred_cfg.RDMA0_ring_size[2] = ringSize[2];
    }
#endif // RING_BUFFER
    return DP_STATUS_RETURN_SUCCESS;
}

DP_STATUS_ENUM DpEngineBase::configFrameAal(DpConfig &config)
{
    const uint32_t id = m_identifier;

    return DP_STATUS_RETURN_SUCCESS;
}

DP_STATUS_ENUM DpEngineBase::configFrameCcorr(DpConfig &config)
{
    const uint32_t id = m_identifier;

    return DP_STATUS_RETURN_SUCCESS;
}



/* ============= PRZ test register =============
    MDP_PRZ0_input_img_w
    MDP_PRZ0_input_img_h
    MDP_PRZ0_output_img_w
    MDP_PRZ0_output_img_w
    MDP_PRZ0_luma_hor_int_ofst_ctl
    MDP_PRZ0_luma_hor_subpix_ofst_ctl
    MDP_PRZ0_luma_ver_int_ofst_ctl
    MDP_PRZ0_luma_ver_subpix_ofst_ctl
    MDP_PRZ0_crop_img_x
    MDP_PRZ0_crop_img_y
*/
DP_STATUS_ENUM DpEngineBase::configFrameScale(DpConfig &config)
{
    const uint32_t id = m_identifier;
#if 0
    if (0 == (getPrevEngine()->getFeature() & eISP)) // RDMA
    {
        const uint32_t prev_id = getPrevEngine()->m_identifier;

        if (cfg.MDP_RDMA_vdo_mode[prev_id] == 1)
        {
            if (cfg.MDP_PRZ_input_img_h[id] != cfg.MDP_RDMA_mf_clip_h[prev_id] >> 1)
            {
                cfg.MDP_PRZ_input_img_h[id] = cfg.MDP_RDMA_mf_clip_h[prev_id] >> 1;
            }

            if (cfg.MDP_PRZ_luma_ver_subpix_ofst_ctl[id])
            {
                cfg.MDP_PRZ_crop_img_y[id] = cfg.MDP_PRZ_input_img_h[id] - cfg.MDP_PRZ_luma_ver_int_ofst_ctl[id] - 1;
            }
            else
            {
                cfg.MDP_PRZ_crop_img_y[id] = cfg.MDP_PRZ_input_img_h[id] - cfg.MDP_PRZ_luma_ver_int_ofst_ctl[id];
            }
        }
    }
#endif
    //cfg.MDP_PRZ_bias_x[id] = cfg.MDP_PRZ_luma_hor_int_ofst_ctl[id];
    //cfg.MDP_PRZ_bias_y[id] = cfg.MDP_PRZ_luma_ver_int_ofst_ctl[id];
    //cfg.MDP_PRZ_offset_x[id] = cfg.MDP_PRZ_luma_hor_subpix_ofst_ctl[id];
    //cfg.MDP_PRZ_offset_y[id] = cfg.MDP_PRZ_luma_ver_subpix_ofst_ctl[id];

    // Prepare port config
    config.inWidth      = cfg.MDP_PRZ_input_img_w[id];
    config.inHeight     = cfg.MDP_PRZ_input_img_h[id];
    config.outWidth     = cfg.MDP_PRZ_output_img_w[id];
    config.outHeight    = cfg.MDP_PRZ_output_img_h[id];

    // Crop information
    config.inXOffset    = cfg.MDP_PRZ_luma_hor_int_ofst_ctl[id];
    config.inXSubpixel  = cfg.MDP_PRZ_luma_hor_subpix_ofst_ctl[id];
    config.inYOffset    = cfg.MDP_PRZ_luma_ver_int_ofst_ctl[id];
    config.inYSubpixel  = cfg.MDP_PRZ_luma_ver_subpix_ofst_ctl[id];
    config.inCropWidth  = cfg.MDP_PRZ_crop_img_x[id];
    config.inCropHeight = cfg.MDP_PRZ_crop_img_y[id];
    config.pEngine_cfg = &cfg.MDP_PRZ[id];

    return DP_STATUS_RETURN_SUCCESS;
}

DP_STATUS_ENUM DpEngineBase::configFrameSharpness(DpConfig &config)
{
    const uint32_t id = m_identifier;

    // Prepare port config
    config.inWidth   = cfg.MDP_TDSHP_in_hsize[id];
    config.inHeight  = cfg.MDP_TDSHP_in_vsize[id];
    config.outWidth  = cfg.MDP_TDSHP_out_hsize[id];
    config.outHeight = cfg.MDP_TDSHP_out_vsize[id];
    config.pEngine_cfg = &cfg.MDP_TDSHP[id];

    return DP_STATUS_RETURN_SUCCESS;
}

DP_STATUS_ENUM DpEngineBase::configFrameColor(DpConfig &config)
{
    const uint32_t id = m_identifier;

    // Prepare port config
    config.inWidth   = cfg.MDP_TDSHP_out_hsize[id];//cfg.MDP_COLOR_ip_width[id];
    config.inHeight  = cfg.MDP_TDSHP_out_vsize[id];//cfg.MDP_COLOR_ip_height[id];
    config.outWidth  = cfg.MDP_TDSHP_out_hsize[id];//cfg.MDP_COLOR_ip_width[id];
    config.outHeight = cfg.MDP_TDSHP_out_vsize[id];//cfg.MDP_COLOR_ip_height[id];
    config.pEngine_cfg = &cfg.MDP_COLOR[id];

    return DP_STATUS_RETURN_SUCCESS;
}

DP_STATUS_ENUM DpEngineBase::configFrameDither(DpConfig &config)
{
    return DP_STATUS_RETURN_SUCCESS;
}

DP_STATUS_ENUM DpEngineBase::configFrameMout(DpConfig &config)
{
    // TODO
    return DP_STATUS_RETURN_SUCCESS;
}

/* ============= WROT test register =============
    MDP_WROT0_tar_xsize
    MDP_WROT0_tar_ysize
    MDP_WROT0_uniform_format
    MDP_WROT0_uniform_swap
    MDP_WROT0_rot_en
    MDP_WROT0_rot_flip
*/
/* ============= WDMA test register =============
    MDP_WDMA_src_w
    MDP_WDMA_src_h
    MDP_WDMA_out_fmt
    MDP_WDMA_swap
*/
DP_STATUS_ENUM DpEngineBase::configFrameWrite(DpCommand &command, DpConfig &config)
{
    const uint32_t id = m_identifier;
    int64_t feature;

    feature = queryFeature();

    if (feature & eROT) // WROT
    {
        DpColorFormat colorFormat = mdp_test::getInstance()->mapUnifiedColor(cfg.MDP_WROT_uniform_format[id], cfg.MDP_WROT_uniform_swap[id]);

        uint32_t plane_num = DP_COLOR_GET_PLANE_COUNT(colorFormat);
        uint32_t width;
        uint32_t height;

        if ((0 == cfg.MDP_WROT_rot_en[id]) || (2 == cfg.MDP_WROT_rot_en[id]))
        {
            width  = cfg.MDP_WROT_tar_xsize[id];
            height = cfg.MDP_WROT_tar_ysize[id];
        }
        else
        {
            width  = cfg.MDP_WROT_tar_ysize[id];
            height = cfg.MDP_WROT_tar_xsize[id];
        }

        uint32_t pitch_y = DP_COLOR_GET_MIN_Y_PITCH(colorFormat, width);
        uint32_t pitch_uv = DP_COLOR_GET_MIN_UV_PITCH(colorFormat, width);
        uint32_t height_uv = height >> DP_COLOR_GET_V_SUBSAMPLE(colorFormat);
        uint32_t size[3];

        size[0] = DP_COLOR_GET_MIN_Y_SIZE(colorFormat, width, height);
        size[1] = (plane_num > 1) ? DP_COLOR_GET_MIN_UV_SIZE(colorFormat, width, height) : 0;
        size[2] = (plane_num > 2) ? DP_COLOR_GET_MIN_UV_SIZE(colorFormat, width, height) : 0;

        cfg.MDP_WROT_in_xsize[id] = cfg.MDP_WROT_tar_xsize[id];
        cfg.MDP_WROT_in_ysize[id] = cfg.MDP_WROT_tar_ysize[id];
        cfg.MDP_WROT_y_stride[id] = pitch_y;
        cfg.MDP_WROT_c_stride[id] = (plane_num > 1) ? pitch_uv : 0;
        cfg.MDP_WROT_v_stride[id] = (plane_num > 2) ? pitch_uv : 0;
        cfg.MDP_WROT_c_base_adr[id] = cfg.MDP_WROT_y_base_adr[id] + size[0];
        cfg.MDP_WROT_v_base_adr[id] = cfg.MDP_WROT_c_base_adr[id] + size[1];
        //cfg.MDP_WROT_rot_en[id] = cfg.MDP_WROT_rot_en[id] & 0x3;

        // Prepare transfer config
        if (0 == id)
        {
            transferred_cfg.WROT0_width[0] = width;
            transferred_cfg.WROT0_width[1] = (plane_num > 1) ? width : 0;
            transferred_cfg.WROT0_width[2] = (plane_num > 2) ? width : 0;
            transferred_cfg.WROT0_height[0] = height;
            transferred_cfg.WROT0_height[1] = (plane_num > 1) ? height_uv : 0;
            transferred_cfg.WROT0_height[2] = (plane_num > 2) ? height_uv : 0;
            transferred_cfg.WROT0_pitch[0] = pitch_y;
            transferred_cfg.WROT0_pitch[1] = (plane_num > 1) ? pitch_uv : 0;
            transferred_cfg.WROT0_pitch[2] = (plane_num > 2) ? pitch_uv : 0;
            transferred_cfg.WROT0_plane = plane_num;
#ifdef RING_BUFFER
            if (transferred_cfg.RDMA0_ringbuffermode)
                transferred_cfg.WROT0_invalid = 1;
#endif

        // Prepare port config
        config.memAddr[0] = cfg.MDP_WROT_y_base_adr[id];
        config.memAddr[1] = cfg.MDP_WROT_c_base_adr[id];
        config.memAddr[2] = cfg.MDP_WROT_v_base_adr[id];
        config.memSize[0] = size[0];
        config.memSize[1] = size[1];
        config.memSize[2] = size[2];
        config.inWidth    = cfg.MDP_WROT_tar_xsize[id];
        config.inHeight   = cfg.MDP_WROT_tar_ysize[id];
        config.outFormat  = colorFormat;
        config.outWidth   = cfg.MDP_WROT_tar_xsize[id];
        config.outHeight  = cfg.MDP_WROT_tar_ysize[id];
        config.outYPitch  = cfg.MDP_WROT_y_stride[id];
        config.outUVPitch = cfg.MDP_WROT_c_stride[id];
        //config.outXStart
        //config.outYStart
        config.enFlip     = cfg.MDP_WROT_rot_flip[id] ? true : false;
        config.rotation   = cfg.MDP_WROT_rot_en[id] * 90;
    }
    else // WDMA
    {
        DpColorFormat colorFormat = mdp_test::getInstance()->mapUnifiedColor(cfg.MDP_WDMA_out_fmt[id], cfg.MDP_WDMA_swap[id]);

        uint32_t plane_num = DP_COLOR_GET_PLANE_COUNT(colorFormat);
        uint32_t width = cfg.MDP_WDMA_src_w[id];
        uint32_t height = cfg.MDP_WDMA_src_h[id];
        uint32_t pitch_y = DP_COLOR_GET_MIN_Y_PITCH(colorFormat, width);
        uint32_t pitch_uv = DP_COLOR_GET_MIN_UV_PITCH(colorFormat, width);
        uint32_t height_uv = height >> DP_COLOR_GET_V_SUBSAMPLE(colorFormat);
        uint32_t size[3];

        size[0] = DP_COLOR_GET_MIN_Y_SIZE(colorFormat, width, height);
        size[1] = (plane_num > 1) ? DP_COLOR_GET_MIN_UV_SIZE(colorFormat, width, height) : 0;
        size[2] = (plane_num > 2) ? DP_COLOR_GET_MIN_UV_SIZE(colorFormat, width, height) : 0;

        cfg.MDP_WDMA_clip_w[id] = width;
        cfg.MDP_WDMA_clip_h[id] = height;
        cfg.MDP_WDMA_pitch[id] = pitch_y;
        cfg.MDP_WDMA_pitch_uv[id] = pitch_uv;
        cfg.MDP_WDMA_dst_u_addr[id] = cfg.MDP_WDMA_dst_addr[id] + size[0];
        cfg.MDP_WDMA_dst_v_addr[id] = cfg.MDP_WDMA_dst_u_addr[id] + size[1];

        // Prepare transfer config
        transferred_cfg.WDMA_width[0] = width;
        transferred_cfg.WDMA_width[1] = (plane_num > 1) ? width : 0;
        transferred_cfg.WDMA_width[2] = (plane_num > 2) ? width : 0;
        transferred_cfg.WDMA_height[0] = height;
        transferred_cfg.WDMA_height[1] = (plane_num > 1) ? height_uv : 0;
        transferred_cfg.WDMA_height[2] = (plane_num > 2) ? height_uv : 0;
        transferred_cfg.WDMA_pitch[0] = pitch_y;
        transferred_cfg.WDMA_pitch[1] = (plane_num > 1) ? pitch_uv : 0;
        transferred_cfg.WDMA_pitch[2] = (plane_num > 2) ? pitch_uv : 0;
        transferred_cfg.WDMA_plane = plane_num;
    #ifdef RING_BUFFER
        if (transferred_cfg.RDMA0_ringbuffermode)
            transferred_cfg.WDMA_invalid = 1;
    #endif

        // Prepare port config
        config.memAddr[0] = cfg.MDP_WDMA_dst_addr[id];
        config.memAddr[1] = cfg.MDP_WDMA_dst_u_addr[id];
        config.memAddr[2] = cfg.MDP_WDMA_dst_v_addr[id];
        config.memSize[0] = size[0];
        config.memSize[1] = size[1];
        config.memSize[2] = size[2];
        config.inWidth    = cfg.MDP_WDMA_src_w[id];
        config.inHeight   = cfg.MDP_WDMA_src_h[id];
        config.outFormat  = colorFormat;
        config.outWidth   = cfg.MDP_WDMA_clip_w[id];
        config.outHeight  = cfg.MDP_WDMA_clip_h[id];
        config.outYPitch  = cfg.MDP_WDMA_pitch[id];
        config.outUVPitch = cfg.MDP_WDMA_pitch_uv[id];
        //config.outXStart
        //config.outYStart
    }
    return DP_STATUS_RETURN_SUCCESS;
}

DP_STATUS_ENUM DpEngineBase::configFrameJPEG(DpConfig &config)
{
    return DP_STATUS_RETURN_SUCCESS;
}

DP_STATUS_ENUM DpEngineBase::configFrameVEnc(DpConfig &config)
{
    return DP_STATUS_RETURN_SUCCESS;
}

#endif // CONFIG_FOR_VERIFY_FPGA
