#include "DpTileEngine.h"
#include "DpEngineType.h"
#include "DpColorConv.h"
#include "mdp_reg_wdma.h"
#include "tile_mdp_reg.h"
#if ESL_SETTING
#include "DpESLControl.h"
#endif
//--------------------------------------------------------
// WDMA driver engine
//--------------------------------------------------------
class DpEngine_WDMA: public DpTileEngine
{
public:
    DpEngine_WDMA()
        : DpTileEngine(0),
          m_targetFormat(m_data.m_targetFormat),
          m_targetPitchY(0),
          m_targetPitchUV(0),
          m_bytesPerPixelY(0),
          m_bytesPerPixelUV(0),
          m_horRightShiftUV(0),
          m_verRightShiftUV(0)
    {
        m_pData = &m_data;
    }

    ~DpEngine_WDMA()
    {
    }

private:
    static const DpColorMatrix RGB_TO_BT601;
    static const DpColorMatrix RGB_TO_JPEG;

    MDP_WDMA_DATA   m_data;

    DpColorFormat   &m_targetFormat;
    uint32_t        m_targetPitchY;
    uint32_t        m_targetPitchUV;
    uint32_t        m_bytesPerPixelY;
    uint32_t        m_bytesPerPixelUV;
    uint32_t        m_horRightShiftUV;
    uint32_t        m_verRightShiftUV;

    DP_STATUS_ENUM mapColorFormat(DP_PROFILE_ENUM inProfile,
                                  DP_PROFILE_ENUM outProfile,
                                  DpColorFormat   colorFormat,
                                  uint32_t        &targetFormat,
                                  uint32_t        &enSwap,
                                  uint32_t        &colorTran,
                                  uint32_t        &matrixSel)
    {
        // Default values
        colorTran = 0;
        matrixSel = 0;

        targetFormat     = DP_COLOR_GET_HW_FORMAT(colorFormat);
        enSwap           = DP_COLOR_GET_SWAP_ENABLE(colorFormat);
        m_bytesPerPixelY = DP_COLOR_BITS_PER_PIXEL(colorFormat) >> 3;

        switch (colorFormat)
        {
            case DP_COLOR_GREY:
                // Y only
                m_bytesPerPixelUV = 0;
                m_horRightShiftUV = 0;
                m_verRightShiftUV = 0;
                break;
            case DP_COLOR_RGB565:
            case DP_COLOR_RGB888:
            case DP_COLOR_BGR888:
            case DP_COLOR_RGBA8888:
            case DP_COLOR_BGRA8888:
            case DP_COLOR_ARGB8888:
            case DP_COLOR_ABGR8888:
                m_bytesPerPixelUV = 0;
                m_horRightShiftUV = 0;
                m_verRightShiftUV = 0;
                colorTran         = 1;
                break;
            case DP_COLOR_UYVY:
            case DP_COLOR_VYUY:
            case DP_COLOR_YUYV:
            case DP_COLOR_YVYU:
                // YUV422, 1 plane
                m_bytesPerPixelUV = 0;
                m_horRightShiftUV = 0;
                m_verRightShiftUV = 0;
                break;
            case DP_COLOR_I420:
            case DP_COLOR_YV12:
                // YUV420, 3 plane
                m_bytesPerPixelUV = 1;
                m_horRightShiftUV = 1;
                m_verRightShiftUV = 1;
                break;
            case DP_COLOR_NV12:
            case DP_COLOR_NV21:
                // YUV420, 2 plane
                m_bytesPerPixelUV = 2;
                m_horRightShiftUV = 1;
                m_verRightShiftUV = 1;
                break;
            default:
                DPLOGE("DpEngine_WDMA: unsupported color format\n");
                assert(0);
                return DP_STATUS_INVALID_FORMAT;
        }

        /*
         * 4'b0000: RGB to JPEG
         * 4'b0010: RGB to BT601
         * 4'b0011: RGB to BT709
         * 4'b0100: JPEG to RGB
         * 4'b0110: BT601 to RGB
         * 4'b0111: BT709 to RGB
         * 4'b1000: JPEG to BT601
         * 4'b1001: JPEG to BT709
         * 4'b1010: BT601 to JPEG
         * 4'b1011: BT709 to JPEG
         * 4'b1100: BT709 to BT601
         * 4'b1101: BT601 to BT709
         */
        if (DP_PROFILE_BT2020      == inProfile ||
            DP_PROFILE_FULL_BT709  == inProfile ||
            DP_PROFILE_FULL_BT2020 == inProfile)
        {
            inProfile = DP_PROFILE_BT709;
        }

        if (1 == colorTran)
        {
            if (DP_PROFILE_BT601 == inProfile)
            {
                matrixSel = 6;
            }
            else if (DP_PROFILE_BT709 == inProfile)
            {
                matrixSel = 7;
            }
            else if (DP_PROFILE_JPEG == inProfile)
            {
                matrixSel = 4;
            }
            else
            {
                DPLOGE("DpEngineWDMA: unknown color conversion\n");
                assert(0);
                return DP_STATUS_INVALID_PARAX;
            }
        }
        else
        {
            if ((DP_PROFILE_JPEG  == inProfile) &&
                (DP_PROFILE_BT601 == outProfile))
            {
                colorTran = 1;
                matrixSel = 8;
            }
            else if ((DP_PROFILE_JPEG  == inProfile) &&
                     (DP_PROFILE_BT709 == outProfile))
            {
                colorTran = 1;
                matrixSel = 9;
            }
            else if ((DP_PROFILE_BT601 == inProfile) &&
                     (DP_PROFILE_JPEG == outProfile))
            {
                colorTran = 1;
                matrixSel = 10;
            }
            else if ((DP_PROFILE_BT709 == inProfile) &&
                     (DP_PROFILE_JPEG == outProfile))
            {
                colorTran = 1;
                matrixSel = 11;
            }
            else if ((DP_PROFILE_BT709 == inProfile) &&
                     (DP_PROFILE_BT601 == outProfile))
            {
                colorTran = 1;
                matrixSel = 12;
            }
            else if ((DP_PROFILE_BT601 == inProfile) &&
                     (DP_PROFILE_BT709 == outProfile))
            {
                colorTran = 1;
                matrixSel = 13;
            }
        }

        return DP_STATUS_RETURN_SUCCESS;
    }

    DP_STATUS_ENUM onInitEngine(DpCommand&);

    DP_STATUS_ENUM onDeInitEngine(DpCommand&);

    DP_STATUS_ENUM onConfigFrame(DpCommand&, DpConfig&);

    DP_STATUS_ENUM onReconfigFrame(DpCommand&, DpConfig&);

    DP_STATUS_ENUM onConfigTile(DpCommand&);

    DP_STATUS_ENUM onWaitEvent(DpCommand&);

    DP_STATUS_ENUM onUpdatePMQOS(DpCommand&, uint32_t&, uint32_t&, int32_t&, struct timeval&);

    int64_t onQueryFeature()
    {
        return eWMEM;
    }
};


// register factory function
static DpEngineBase* WDMAFactory(DpEngineType type)
{
    if (tWDMA == type)
    {
        return new DpEngine_WDMA();
    }
    return NULL;
};

// register factory function
EngineReg WDMAReg(WDMAFactory);


const DpColorMatrix DpEngine_WDMA::RGB_TO_BT601 = DpColorMatrix(
     0.256788,  0.504129,  0.097906,
    -0.148223, -0.290993,  0.439216,
     0.439216, -0.367788, -0.071427);

const DpColorMatrix DpEngine_WDMA::RGB_TO_JPEG = DpColorMatrix(
     0.299000,  0.587000,  0.114000,
    -0.168736, -0.331264,  0.500000,
     0.500000, -0.418688, -0.081312);


DP_STATUS_ENUM DpEngine_WDMA::onInitEngine(DpCommand &command)
{
    // Reset engine
    MM_REG_WRITE(command, WDMA_RST, 0x1, 0x00000001);
    MM_REG_POLL(command,  WDMA_FLOW_CTRL_DBG, 0x01, 0x00000001);
    MM_REG_WRITE(command, WDMA_RST, 0x0, 0x00000001);

    return DP_STATUS_RETURN_SUCCESS;
}


DP_STATUS_ENUM DpEngine_WDMA::onDeInitEngine(DpCommand &)
{
    return DP_STATUS_RETURN_SUCCESS;
}


DP_STATUS_ENUM DpEngine_WDMA::onConfigFrame(DpCommand &command, DpConfig &config)
{
    DP_STATUS_ENUM status;
    uint32_t       targetFormat;
    uint32_t       enSwap;
    uint32_t       colorTran;
    uint32_t       extMatrix;
    uint32_t       matrixSel;

    int32_t        targetXStart;
    int32_t        targetYStart;
    uint32_t       handle;
    uint32_t       secureRegAddr[3] = {WDMA_DST_ADDR, WDMA_DST_U_ADDR, WDMA_DST_V_ADDR};
    uint32_t       offset[3] = {0};
    uint32_t       planeOffset[3] = {0};
    int32_t        index = 0;

    m_targetFormat  = config.outFormat;
    m_targetPitchY  = config.outYPitch;
    m_targetPitchUV = config.outUVPitch;
    targetXStart    = config.outXStart;
    targetYStart    = config.outYStart;

    status = mapColorFormat(config.inProfile,
                            config.outProfile,
                            config.outFormat,
                            targetFormat,
                            enSwap,
                            colorTran,
                            matrixSel);
    if (DP_STATUS_RETURN_SUCCESS != status)
    {
        DPLOGE("DpEngine_WDMA: map color format failed\n");
        return status;
    }

    if (config.pColorMatrix && config.pColorMatrix->enable)
    {
        colorTran = 1;
        extMatrix = 1;
    }
    else
    {
        extMatrix = 0;
    }

    DPLOGI("DpEngine_WDMA: output Y pitch %d, output UV pitch %d\n", config.outYPitch, config.outUVPitch);

#if ESL_SETTING
    setESLWDMA(command, config.outFormat);
#else
    // Setup buffer control
    MM_REG_WRITE(command, WDMA_BUF_CON1, 0x40000000, 0x40000000);
#endif
    MM_REG_WRITE(command, WDMA_BUF_CON2, 0x10101050, 0xFFFFFFFF);

    // Setup frame information
    MM_REG_WRITE(command, WDMA_CFG, (targetFormat <<  4) +
                                    (colorTran    << 11) +
                                    //(1            << 12) + //Enable Vertical filter
                                    (extMatrix    << 13) +
                                    //(1            << 15) + //Enable Horizontal filter
                                    (enSwap       << 16) +
                                    (matrixSel    << 24), 0x0F01B8F0);

    command.addMetLog("DISP_WDMA0__WDMA_CFG", (targetFormat <<  4) +
                                  (colorTran    << 11) +
                                  //(1            << 12) + //Enable Vertical filter
                                  (extMatrix    << 13) +
                                  //(1            << 15) + //Enable Horizontal filter
                                  (enSwap       << 16) +
                                  (matrixSel    << 24));

    command.addMetLog("DISP_WDMA0__WDMA_CLIP_SIZE", (config.outHeight << 16) +
                                        (config.outWidth <<  0));

    command.addMMpathLog("out_0_width", config.outWidth, DP_MMPATH_OTHER);
    command.addMMpathLog("out_0_height", config.outHeight, DP_MMPATH_OTHER);
    command.addMMpathLog((m_identifier?"hw=MDP,out_2":"hw=MDP,out_1"), config.outFormat, DP_MMPATH_FORMAT);
    command.addMMpathLog((m_identifier?"MDP_WDMA1":"MDP_WDMA0"), 1, DP_MMPATH_OTHER);
    command.addMMpathLog("out_0", config.memAddr[0], DP_MMPATH_ADDR);

    handle = config.memAddr[0];
    planeOffset[0] = 0;
    planeOffset[1] = config.memAddr[1];
    planeOffset[2] = config.memAddr[2];

    if ((targetXStart != 0) ||
        (targetYStart != 0))
    {
        offset[0] = (m_bytesPerPixelY * targetXStart) + (m_targetPitchY * targetYStart);

        offset[1] = (targetXStart >> m_horRightShiftUV) * m_bytesPerPixelUV +
                     (targetYStart >> m_verRightShiftUV) * m_targetPitchUV;

        offset[2] = (targetXStart >> m_horRightShiftUV) * m_bytesPerPixelUV +
                     (targetYStart >> m_verRightShiftUV) * m_targetPitchUV;

#ifdef MDP_VERSION_TABLET
        if (!m_pOutDataPort[0]->getSecureStatus())
#endif
        {
            config.memAddr[0] += offset[0];
            config.memAddr[1] += offset[1];
            config.memAddr[2] += offset[2];
        }
    }

    command.secure(m_engineType,
                   getEngineFlag(),
                   secureRegAddr,
                   m_pOutDataPort[0]->getSecureStatus(),
                   handle,
                   offset,
                   config.memSize,
                   planeOffset);

    command.frameDstInfo(config.outFormat,
                         config.outWidth,
                         config.outHeight,
                         config.outYPitch,
                         config.outUVPitch,
                         config.outXStart,
                         config.outYStart,
                         config.memAddr,
                         config.memSize,
                         m_pOutDataPort[0]->getSecureStatus());

    // Setup frame base address
    MM_REG_WRITE(command, WDMA_DST_ADDR,   config.memAddr[0], 0xFFFFFFFF, &m_frameConfigLabel[index++]);
    MM_REG_WRITE(command, WDMA_DST_U_ADDR, config.memAddr[1], 0xFFFFFFFF, &m_frameConfigLabel[index++]);
    MM_REG_WRITE(command, WDMA_DST_V_ADDR, config.memAddr[2], 0xFFFFFFFF, &m_frameConfigLabel[index++]);

    DPLOGI("DpEngine_WDMA: output0 MVA 0x%08x\n", config.memAddr[0]);
    DPLOGI("DpEngine_WDMA: output1 MVA 0x%08x\n", config.memAddr[1]);
    DPLOGI("DpEngine_WDMA: output2 MVA 0x%08x\n", config.memAddr[2]);

    // Setup Y pitch
    MM_REG_WRITE(command, WDMA_DST_W_IN_BYTE, m_targetPitchY, 0x0000FFFF);

    // Setup UV pitch
    MM_REG_WRITE(command, WDMA_DST_UV_PITCH, m_targetPitchUV, 0x0000FFFF);

    // Set the fixed ALPHA as 0xFF
    MM_REG_WRITE(command, WDMA_ALPHA, 0x800000FF, 0x800000FF);

#if 0
    {
        MM_REG_WRITE(command, WDMA_DITHER_13, (0x0 << 8) + // rshift_b
            (0x0 << 4) + // rshift_g
            (0x0 << 0), 0x00000777); // rshift_r

        MM_REG_WRITE(command, WDMA_DITHER_15, (0x5 << 28) + // lsb_err_shift_r
            (0x0 << 24) + // ovflw_bit_r
            (0x5 << 20) + // add_lshift_r
            (0x0 << 16) + // input_rshift_r
            (0x1 << 0), 0x77770001); // new_bit_mode

        MM_REG_WRITE(command, WDMA_DITHER_16, (0x5 << 28) + // lsb_err_shift_b
            (0x0 << 24) + // ovflw_bit_b
            (0x5 << 20) + // add_lshift_b
            (0x0 << 16) + // input_rshift_b
            (0x4 << 12) + // lsb_err_shift_g
            (0x0 << 8)  + // ovflw_bit_g
            (0x4 << 4)  + // add_lshift_g
            (0x0 << 0), 0x77777777); // input_rshift_g

        MM_REG_WRITE(command, WDMA_DITHER_6,  (0x1 << 13) + // fphase_r
            (0x1 << 12) + // fphase_en
            (0x0 << 3) + // round_en
            (0x1 << 2) + // rdither_en
            (0x0 << 1) + // lfsr_en
            (0x0 << 0), 0x0000300F); // edither_en

        MM_REG_WRITE(command, WDMA_DITHER_10, (0x0 << 0), 0x00000003); // fphase_ctrl

        MM_REG_WRITE(command, WDMA_DITHER_12, (0x1 << 4) + // table_en
            (0x1 << 0), 0x00000031); // lsb_off

        MM_REG_WRITE(command, WDMA_DITHER_0,  (0x0 << 4) + // out_sel
            (0x1 << 0), 0x00000011); // start
    }
#endif

    // Set external matrix
    if (extMatrix)
    {
        DpColorMatrix mat;
        int32_t postAdd0;

        if (DP_PROFILE_BT601 == config.outProfile)
        {
            mat = RGB_TO_BT601 * (*config.pColorMatrix); // config must be YUV2RGB matrix
            postAdd0 = 16;
        }
        else if (DP_PROFILE_JPEG == config.outProfile)
        {
            mat = RGB_TO_JPEG * (*config.pColorMatrix); // config must be YUV2RGB matrix
            postAdd0 = 0;
        }
        else
        {
            return DP_STATUS_INVALID_FORMAT;
        }

        #define TO_MAT_REG(c)   FLOAT_TO_SIGNED_FIXED(c, 2, 10)

        // Set matrix coefficient
        MM_REG_WRITE(command, WDMA_C00, (TO_MAT_REG(mat.c00) <<  0) +
                                        (TO_MAT_REG(mat.c01) << 16), 0x1FFF1FFF);
        MM_REG_WRITE(command, WDMA_C02, (TO_MAT_REG(mat.c02) <<  0), 0x00001FFF);
        MM_REG_WRITE(command, WDMA_C10, (TO_MAT_REG(mat.c10) <<  0) +
                                        (TO_MAT_REG(mat.c11) << 16), 0x1FFF1FFF);
        MM_REG_WRITE(command, WDMA_C12, (TO_MAT_REG(mat.c12) <<  0), 0x00001FFF);
        MM_REG_WRITE(command, WDMA_C20, (TO_MAT_REG(mat.c20) <<  0) +
                                        (TO_MAT_REG(mat.c21) << 16), 0x1FFF1FFF);
        MM_REG_WRITE(command, WDMA_C22, (TO_MAT_REG(mat.c22) <<  0), 0x00001FFF);

        // Set pre-add
        MM_REG_WRITE(command, WDMA_PRE_ADD0, (  0x0 <<  0) +             //    0
                                             (0x180 << 16), 0x01FF01FF); // -128
        MM_REG_WRITE(command, WDMA_PRE_ADD2, (0x180 <<  0), 0x000001FF); // -128

        // Set post-add
        MM_REG_WRITE(command, WDMA_POST_ADD0, (postAdd0 <<  0) +
                                              (128      << 16), 0x01FF01FF);
        MM_REG_WRITE(command, WDMA_POST_ADD2, (128      <<  0), 0x000001FF);
    }

    command.addMetLog("DISP_WDMA0__WDMA_EN", 1);
    return DP_STATUS_RETURN_SUCCESS;
}


DP_STATUS_ENUM DpEngine_WDMA::onReconfigFrame(DpCommand &command, DpConfig &config)
{
    int32_t        targetXStart;
    int32_t        targetYStart;
    uint32_t       handle;
    uint32_t       secureRegAddr[3] = {WDMA_DST_ADDR, WDMA_DST_U_ADDR, WDMA_DST_V_ADDR};
    uint32_t       offset[3] = {0};
    uint32_t       planeOffset[3] = {0};
    int32_t        index = 0;

    DPLOGI("DpEngine_WDMA: onReconfigFrame");

    m_targetPitchY  = config.outYPitch;
    m_targetPitchUV = config.outUVPitch;
    targetXStart    = config.outXStart;
    targetYStart    = config.outYStart;

    handle = config.memAddr[0];
    planeOffset[0] = 0;
    planeOffset[1] = config.memAddr[1];
    planeOffset[2] = config.memAddr[2];

    if ((targetXStart != 0) ||
        (targetYStart != 0))
    {
        offset[0] = (m_bytesPerPixelY * targetXStart) + (m_targetPitchY * targetYStart);

        offset[1] = (targetXStart >> m_horRightShiftUV) * m_bytesPerPixelUV +
                     (targetYStart >> m_verRightShiftUV) * m_targetPitchUV;

        offset[2] = (targetXStart >> m_horRightShiftUV) * m_bytesPerPixelUV +
                     (targetYStart >> m_verRightShiftUV) * m_targetPitchUV;

        config.memAddr[0] += offset[0];
        config.memAddr[1] += offset[1];
        config.memAddr[2] += offset[2];
    }

    command.secure(m_engineType,
                   getEngineFlag(),
                   secureRegAddr,
                   m_pOutDataPort[0]->getSecureStatus(),
                   handle,
                   offset,
                   config.memSize,
                   planeOffset);

    // Setup frame base address
    MM_REG_WRITE(command, WDMA_DST_ADDR,   config.memAddr[0], 0xFFFFFFFF, NULL, m_frameConfigLabel[index++]);
    MM_REG_WRITE(command, WDMA_DST_U_ADDR, config.memAddr[1], 0xFFFFFFFF, NULL, m_frameConfigLabel[index++]);
    MM_REG_WRITE(command, WDMA_DST_V_ADDR, config.memAddr[2], 0xFFFFFFFF, NULL, m_frameConfigLabel[index++]);

    command.addMMpathLog("out_0_width", config.outWidth, DP_MMPATH_OTHER);
    command.addMMpathLog("out_0_height", config.outHeight, DP_MMPATH_OTHER);
    command.addMMpathLog((m_identifier?"hw=MDP,out_2":"hw=MDP,out_1"), config.outFormat, DP_MMPATH_FORMAT);
    command.addMMpathLog((m_identifier?"MDP_WDMA1":"MDP_WDMA0"), 1, DP_MMPATH_OTHER);
    command.addMMpathLog("out_0", config.memAddr[0], DP_MMPATH_ADDR);

    return DP_STATUS_RETURN_SUCCESS;
}


DP_STATUS_ENUM DpEngine_WDMA::onConfigTile(DpCommand &command)
{
    uint32_t WDMA1_addr_offset_y;
    uint32_t WDMA1_addr_offset_u;
    uint32_t WDMA1_addr_offset_v;
    uint32_t WDMA1_src_w;
    uint32_t WDMA1_src_h;
    uint32_t WDMA1_clip_w;
    uint32_t WDMA1_clip_h;
    uint32_t WDMA1_offset_x;
    uint32_t WDMA1_offset_y;

    // Write Y pixel offset
    WDMA1_addr_offset_y = m_outTileXLeft * m_bytesPerPixelY +
                          m_outTileYTop  * m_targetPitchY;
    MM_REG_WRITE(command, WDMA_DST_ADDR_OFFSET, WDMA1_addr_offset_y, 0x0FFFFFFF);

    // Write U pixel offset
    WDMA1_addr_offset_u = (m_outTileXLeft >> m_horRightShiftUV) * m_bytesPerPixelUV +
                          (m_outTileYTop  >> m_verRightShiftUV) * m_targetPitchUV;
    MM_REG_WRITE(command, WDMA_DST_U_ADDR_OFFSET, WDMA1_addr_offset_u, 0x0FFFFFFF);

    // Write V pixel offset
    WDMA1_addr_offset_v = (m_outTileXLeft >> m_horRightShiftUV) * m_bytesPerPixelUV +
                          (m_outTileYTop  >> m_verRightShiftUV) * m_targetPitchUV;
    MM_REG_WRITE(command, WDMA_DST_V_ADDR_OFFSET, WDMA1_addr_offset_v, 0x0FFFFFFF);

    // Write source size
    WDMA1_src_w = m_inTileXRight - m_inTileXLeft + 1;
    WDMA1_src_h = m_inTileYBottom - m_inTileYTop + 1;
    MM_REG_WRITE(command, WDMA_SRC_SIZE, (WDMA1_src_h << 16) +
                                         (WDMA1_src_w <<  0), 0x3FFF3FFF);

    // Write target size
    WDMA1_clip_w = m_outTileXRight - m_outTileXLeft + 1;
    WDMA1_clip_h = m_outTileYBottom - m_outTileYTop + 1;
    MM_REG_WRITE(command, WDMA_CLIP_SIZE, (WDMA1_clip_h << 16) +
                                          (WDMA1_clip_w <<  0), 0x3FFF3FFF);

    // Write clip offset
    WDMA1_offset_x = m_lumaXOffset;
    WDMA1_offset_y = m_lumaYOffset;
    MM_REG_WRITE(command, WDMA_CLIP_COORD, (WDMA1_offset_y << 16) +
                                           (WDMA1_offset_x <<  0), 0x3FFF3FFF);

    // Enable engine
    MM_REG_WRITE(command, WDMA_EN, 0x01, 0x00000001);
    return DP_STATUS_RETURN_SUCCESS;
}


DP_STATUS_ENUM DpEngine_WDMA::onWaitEvent(DpCommand &command)
{
    MM_REG_WAIT(command, DpCommand::WDMA_FRAME_DONE);

    // Disable engine
    MM_REG_WRITE(command, WDMA_EN, 0x00, 0x00000001);

    return DP_STATUS_RETURN_SUCCESS;
}

DP_STATUS_ENUM DpEngine_WDMA::onUpdatePMQOS(DpCommand &command, uint32_t& totalPixel, uint32_t& totalDataSzie, int32_t& engineType, struct timeval& endtime)
{
    uint32_t        data = 0;
    uint32_t        plane = 0;
    uint32_t        pixelCount = 0;

    DP_UNUSED(endtime);

    plane = DP_COLOR_GET_PLANE_COUNT(m_targetFormat);

    data = DP_COLOR_GET_MIN_Y_SIZE(m_targetFormat, m_outTileAccumulation, m_outFrameHeight);
    data += (plane > 1) ? DP_COLOR_GET_MIN_UV_SIZE(m_targetFormat, m_outTileAccumulation, m_outFrameHeight) : 0;
    data += (plane > 2) ? DP_COLOR_GET_MIN_UV_SIZE(m_targetFormat, m_outTileAccumulation, m_outFrameHeight) : 0;

    totalDataSzie += data;

    pixelCount = m_outTileAccumulation * m_outFrameHeight;

    if (pixelCount > totalPixel)
    {
        totalPixel = pixelCount;
    }

    engineType = m_engineType;

    DPLOGI("DpEngine_WDMA::updatePMQOS totalDataSzie = %d totalPixel = %d\n", totalDataSzie, totalPixel);

    return DP_STATUS_RETURN_SUCCESS;
}
