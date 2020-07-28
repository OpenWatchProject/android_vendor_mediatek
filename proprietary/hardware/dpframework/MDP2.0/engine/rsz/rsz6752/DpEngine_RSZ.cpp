#include "DpTileEngine.h"
#include "DpEngineType.h"
#include "DpTileScaler.h"
#include "mdp_reg_rsz.h"
#include "tile_mdp_reg.h"

#ifndef BASIC_PACKAGE
#define ENABLE_PQ_RSZ (1)
#else
#define ENABLE_PQ_RSZ (0)
#endif // BASIC_PACKAGE

#if ENABLE_PQ_RSZ
#if CONFIG_FOR_OS_ANDROID
#include "PQSessionManager.h"
#endif // CONFIG_FOR_OS_ANDROID
#endif // ENABLE_PQ_RSZ

//--------------------------------------------------------
// Scaler driver engine
//--------------------------------------------------------
class DpEngine_SCL: public DpTileEngine
{
public:
    DpEngine_SCL(uint32_t identifier)
        : DpTileEngine(identifier),
          m_use121filter(m_data.m_use121filter),
          m_coeffStepX(m_data.m_coeffStepX),
          m_coeffStepY(m_data.m_coeffStepY),
          m_precisionX(m_data.m_precisionX),
          m_precisionY(m_data.m_precisionY),
          m_cropOffsetX(m_data.m_cropOffsetX),
          m_cropSubpixX(m_data.m_cropSubpixX),
          m_cropWidth(m_data.m_cropWidth),
          m_cropOffsetY(m_data.m_cropOffsetY),
          m_cropSubpixY(m_data.m_cropSubpixY),
          m_cropHeight(m_data.m_cropHeight),
          m_horDirScale(m_data.m_horDirScale),
          m_horAlgorithm(m_data.m_horAlgorithm),
          m_verDirScale(m_data.m_verDirScale),
          m_verAlgorithm(m_data.m_verAlgorithm),
          m_C42OutFrameW(m_data.m_C42OutFrameW),
          m_C24InFrameW(m_data.m_C24InFrameW),
          m_rszBackXLeft(m_data.m_rszBackXLeft),
          m_rszBackXRight(m_data.m_rszBackXRight)
    {
        m_pData = &m_data;
    }

    ~DpEngine_SCL()
    {
    }

private:
    MDP_PRZ_DATA                m_data;

    bool                        &m_use121filter;
    uint32_t                    &m_coeffStepX;
    uint32_t                    &m_coeffStepY;
    uint32_t                    &m_precisionX;
    uint32_t                    &m_precisionY;

    int32_t                     &m_cropOffsetX;
    int32_t                     &m_cropSubpixX;
    int32_t                     &m_cropWidth;
    int32_t                     &m_cropOffsetY;
    int32_t                     &m_cropSubpixY;
    int32_t                     &m_cropHeight;

    bool                        &m_horDirScale;
    DP_TILE_SCALER_ALGO_ENUM    &m_horAlgorithm;
    bool                        &m_verDirScale;
    DP_TILE_SCALER_ALGO_ENUM    &m_verAlgorithm;
    int32_t                     &m_C42OutFrameW;
    int32_t                     &m_C24InFrameW;
    int32_t                     &m_rszBackXLeft;
    int32_t                     &m_rszBackXRight;

    DP_STATUS_ENUM onInitEngine(DpCommand&);

    DP_STATUS_ENUM onDeInitEngine(DpCommand&);

    DP_STATUS_ENUM onConfigFrame(DpCommand&,
                                 DpConfig&);

    DP_STATUS_ENUM onConfigTile(DpCommand&);

#ifdef MDP_RSZ_DISABLE_DCM_SMALL_TILE
    DP_STATUS_ENUM onAdvanceTile(DpCommand&);
#endif

    int32_t getRsz6TapTableIndex(int32_t stepSize, bool clearZoom);

    int64_t onQueryFeature()
    {
        return eSCL;
    }
};


// Register factory function
#if CONFIG_FOR_OS_ANDROID
static DpEngineBase* SCL0Factory(DpEngineType type)
{
    if(tSCL0 == type)
    {
        return new DpEngine_SCL(0);
    }

    return 0;
};
#else
DpEngineBase* SCL0Factory(DpEngineType type)
{
    if(tSCL0 == type)
    {
        return new DpEngine_SCL(0);
    }

    return 0;
};
#endif  // CONFIG_FOR_OS_ANDROID


// Register factory function
#if CONFIG_FOR_OS_ANDROID
static DpEngineBase* SCL1Factory(DpEngineType type)
{
    if(tSCL1 == type)
    {
        return new DpEngine_SCL(1);
    }

    return 0;
};
#else
DpEngineBase* SCL1Factory(DpEngineType type)
{
    if(tSCL1 == type)
    {
        return new DpEngine_SCL(1);
    }

    return 0;
};
#endif  // CONFIG_FOR_OS_ANDROID

// Register factory function
#if CONFIG_FOR_OS_ANDROID
static DpEngineBase* SCL2Factory(DpEngineType type)
{
    if(tSCL2 == type)
    {
        return new DpEngine_SCL(2);
    }

    return 0;
};
#else
DpEngineBase* SCL2Factory(DpEngineType type)
{
    if(tSCL2 == type)
    {
        return new DpEngine_SCL(2);
    }

    return 0;
};
#endif  // CONFIG_FOR_OS_ANDROID


// Register factory function

EngineReg SCL0Reg(SCL0Factory);
EngineReg SCL1Reg(SCL1Factory);
EngineReg SCL2Reg(SCL2Factory);


DP_STATUS_ENUM DpEngine_SCL::onInitEngine(DpCommand &command)
{
    // Reset engine
    MM_REG_WRITE(command, PRZ_ENABLE, 0x00010000, 0x00010000);
    MM_REG_WRITE(command, PRZ_ENABLE, 0x00000000, 0x00010000);

    // Enable engine
    MM_REG_WRITE(command, PRZ_ENABLE, 0x00000001, 0x00000001);

    return DP_STATUS_RETURN_SUCCESS;
}


DP_STATUS_ENUM DpEngine_SCL::onDeInitEngine(DpCommand &command)
{
    // Disable engine
    MM_REG_WRITE(command, PRZ_ENABLE, 0x00000000, 0x00000001);

    return DP_STATUS_RETURN_SUCCESS;
}


int32_t DpEngine_SCL::getRsz6TapTableIndex(int32_t stepSize, bool clearZoom)
{
    int32_t table_idx = 0;

    if (stepSize <= 32768) // resize ratio >=1
    {
        if (clearZoom == true)
        {
            table_idx = 23;
        }
        else
        {
            table_idx = 27;
        }
    }
    else if (stepSize < 36409)  // resize ratio >= 0.9
    {
        table_idx = 20;
    }
    else if (stepSize < 40961) // resize ratio >= 0.8
    {
        table_idx = 21;
    }
    else if (stepSize < 46812) // resize ratio >= 0.7
    {
        table_idx = 22;
    }
    else if (stepSize < 54614) // resize ratio >= 0.6
    {
        table_idx = 23;
    }
    else if (stepSize < 59579) // resize ratio >= 0.55
    {
        table_idx = 24;
    }
    else if (stepSize < 65537) // resize ratio >= 0.5
    {
        table_idx = 25;
    }
    else                       // resize ratio < 0.5
    {
        table_idx = 26;
    }

    return table_idx;
}

DP_STATUS_ENUM DpEngine_SCL::onConfigFrame(DpCommand &command,
                                           DpConfig  &config)
{
    //DP_STATUS_ENUM status;
    uint32_t controlVal;
    int32_t  horTable;
    int32_t  verTable;

    int32_t  cropWidth;
    int32_t  outWidth;
    int32_t  cropHeight;
    int32_t  outHeight;

    bool isRefocus = false;
    bool isClearZoom = false;
    uint8_t isp_RFUpTable;
    uint8_t isp_RFDownTable;

    m_cropOffsetX = config.inXOffset;
    m_cropSubpixX = config.inXSubpixel;
    m_cropOffsetY = config.inYOffset;
    m_cropSubpixY = config.inYSubpixel;
    m_cropWidth   = config.inCropWidth;
    m_cropHeight  = config.inCropHeight;

    horTable = config.rszHorTable;
    verTable = config.rszVerTable;

    if (0 == m_cropWidth)
    {
        m_cropWidth = m_inFrameWidth;
    }

    if (0 == m_cropHeight)
    {
        m_cropHeight = m_inFrameHeight;
    }

    if ((config.rootAndLeaf == 2) && // 1-in + 1-out = 2
        (m_cropWidth     == m_inFrameWidth) &&
        (m_inFrameWidth  == m_outFrameWidth) &&
        (m_cropHeight    == m_inFrameHeight) &&
        (m_inFrameHeight == m_outFrameHeight) &&
        (m_cropSubpixX   == 0) &&
        (m_cropSubpixY   == 0))
    {
        m_bypassEngine = true;

        MM_REG_WRITE(command, PRZ_ENABLE, 0x00000000, 0x00000001);

        return DP_STATUS_RETURN_SUCCESS;
    }
    else
    {
        m_bypassEngine = false;
    }

#if ENABLE_PQ_RSZ
    PQSession* pPQSession = PQSessionManager::getInstance()->getPQSession(config.pqSessionId);
    DpPqParam pqParam;

    do {
        if (pPQSession == NULL)
            break;

        pPQSession->getPQParam(&pqParam);

        if (pqParam.scenario != MEDIA_ISP_PREVIEW && pqParam.scenario != MEDIA_ISP_CAPTURE)
        {
            break;
        }

        if (pqParam.u.isp.isIspScenario != 1)
        {
            break;
        }

        DpPqConfig* DpPqConfig;
        pPQSession->getDpPqConfig(&DpPqConfig);

        if (DpPqConfig->enUR == 1)
        {
            /* check enable flag from setPQparameter */
            if (DpPqConfig->enUR == 0 || DpPqConfig->enUR == 1)
                isClearZoom = DpPqConfig->enUR;
            else
                isClearZoom = true;
        }
        else if (DpPqConfig->enReFocus == 1)
        {
            isRefocus = pqParam.u.isp.vsdofParam.isRefocus; /* will modify to DpPqConfig.enRefocus */
            if (isRefocus == true)
            {
                isp_RFUpTable = pqParam.u.isp.vsdofParam.defaultDownTable;
                isp_RFDownTable = pqParam.u.isp.vsdofParam.defaultUpTable;
            }

            DPLOGI("DpEngine_RSZ: isRefocus %d\n", isRefocus);
            DPLOGI("DpEngine_RSZ: isp_RFUpTable %d\n", isp_RFUpTable);
            DPLOGI("DpEngine_RSZ: isp_RFDownTable %d\n", isp_RFDownTable);
        }
    } while (0);
#endif // ENABLE_PQ_RSZ

    controlVal = 0;

    // Vertical first
    controlVal |= (m_outFrameWidth > m_cropWidth)? 0x00000010: 0x00000000;

    if (m_outFrameWidth == m_cropWidth)
    {
        m_horAlgorithm  = DP_TILE_SCALER_ALG0_6_TAPS;
        m_horDirScale   = true;
        controlVal     |= (DP_TILE_SCALER_ALG0_6_TAPS << 5);
        horTable        = 27;
        controlVal     |= (horTable << 16);

        // Setup coeffstep and precision
        m_precisionX = (1 << 15);
        m_coeffStepX = m_precisionX;
    }
    else
    {
        if (m_cropWidth <= 1)
        {
            cropWidth = 2;
        }
        else
        {
            cropWidth = m_cropWidth;
        }

        if (m_outFrameWidth <= 1)
        {
            outWidth = 2;
        }
        else
        {
            outWidth = m_outFrameWidth;
        }

        if ((m_cropWidth - 1) < 2 * (m_outFrameWidth - 1))      // ratio > (1/2)
        {
            m_horAlgorithm  = DP_TILE_SCALER_ALG0_6_TAPS;
            m_horDirScale   = true;
            controlVal     |= (DP_TILE_SCALER_ALG0_6_TAPS << 5);

            // Setup coeffstep and precision
            m_precisionX = (1 << 15);
            m_coeffStepX = (int32_t)((float)(cropWidth - 1) * m_precisionX / (outWidth - 1) + 0.5);

            horTable        = getRsz6TapTableIndex(m_coeffStepX, isClearZoom);
            controlVal     |= (horTable << 16);
        }
        else if ((m_cropWidth - 1) < 24 * (m_outFrameWidth - 1))  // (1/24) < ratio < (1/2)
        {
            m_horAlgorithm  = DP_TILE_SCALER_ALGO_CUB_ACC;
            m_horDirScale   = true;
            controlVal     |= (DP_TILE_SCALER_ALGO_CUB_ACC << 5);
            horTable        = 15; // table can be selected between 0~19 by request
            controlVal     |= (horTable << 16);

            // Setup coeffstep and precision
            m_precisionX = (1 << 20);
            m_coeffStepX = (int32_t)((float)(outWidth - 1) * m_precisionX / (cropWidth - 1) + 1);
        }
        else    // (1/24) > ratio
        {
            m_horAlgorithm  = DP_TILE_SCALER_ALG0_SRC_ACC;
            m_horDirScale   = true;
            controlVal     |= (DP_TILE_SCALER_ALG0_SRC_ACC << 5);

            // Setup coeffstep and precision
            m_precisionX = (1 << 20);
            m_coeffStepX = (int32_t)((float)(outWidth - 1) * m_precisionX / (cropWidth - 1) + 1);
        }
    }

    MM_REG_WRITE(command, PRZ_HORIZONTAL_COEFF_STEP, m_coeffStepX, 0x007FFFFF);

    if (m_outFrameHeight == m_cropHeight)
    {
        m_verAlgorithm  = DP_TILE_SCALER_ALG0_6_TAPS;
        m_verDirScale   = true;
        controlVal     |= (DP_TILE_SCALER_ALG0_6_TAPS << 7);
        verTable        = 27;
        controlVal     |= (verTable << 21);

        // Setup coeffstep and precision
        m_precisionY = (1 << 15);
        m_coeffStepY = m_precisionY;
    }
    else
    {
        if (m_cropHeight <= 1)
        {
            cropHeight = 2;
        }
        else
        {
            cropHeight = m_cropHeight;
        }

        if (m_outFrameHeight <= 1)
        {
            outHeight = 2;
        }
        else
        {
            outHeight = m_outFrameHeight;
        }

        if ((m_cropHeight - 1) < 2 * (m_outFrameHeight - 1))    // ratio > (1/2)
        {
            m_verAlgorithm  = DP_TILE_SCALER_ALG0_6_TAPS;
            m_verDirScale   = true;
            controlVal     |= (DP_TILE_SCALER_ALG0_6_TAPS << 7);

            // Setup coeffstep and precision
            m_precisionY = (1 << 15);
            m_coeffStepY = (int32_t)((float)(cropHeight - 1) * m_precisionY / (outHeight - 1) + 0.5);

            verTable        = getRsz6TapTableIndex(m_coeffStepY, isClearZoom);
            controlVal     |= (verTable << 21);
        }
        else if ((m_cropHeight - 1) < 24 * (m_outFrameHeight - 1))  // (1/24) < ratio < (1/2)
        {
            m_verAlgorithm  = DP_TILE_SCALER_ALGO_CUB_ACC;
            m_verDirScale   = true;
            controlVal     |= (DP_TILE_SCALER_ALGO_CUB_ACC << 7);
            verTable        = 15; // table can be selected between 0~19 by request
            controlVal     |= (verTable << 21);

            // Setup coeffstep and precision
            m_precisionY = (1 << 20);
            m_coeffStepY = (int32_t)((float)(outHeight - 1) * m_precisionY / (cropHeight - 1) + 1);
        }
        else    // ratio < (1/24)
        {
            m_verAlgorithm  = DP_TILE_SCALER_ALG0_SRC_ACC;
            m_verDirScale   = true;
            controlVal     |= (DP_TILE_SCALER_ALG0_SRC_ACC << 7);

            // Setup coeffstep and precision
            m_precisionY = (1 << 20);
            m_coeffStepY = (int32_t)((float)(outHeight - 1) * m_precisionY / (cropHeight - 1) + 1);
        }
    }

    MM_REG_WRITE(command, PRZ_VERTICAL_COEFF_STEP, m_coeffStepY, 0x007FFFFF);

    if (isRefocus == true)
    {
        controlVal &= 0x04001DF3; //reset verTable and horTable
        controlVal |= (isp_RFDownTable << 16);
        controlVal |= (isp_RFDownTable << 21);
        DPLOGI("DpEngine_RSZ: m_verAlgorithm %d\n", m_verAlgorithm);
    }

    if (DP_COLOR_GET_H_SUBSAMPLE(config.inFormat))
    {
        // C42 conversion: drop if source is YUV422 or YUV420
        m_use121filter = false;
    }

    controlVal |= (true == m_horDirScale)?  0x00000001: 0x00000000;
    controlVal |= (true == m_verDirScale)?  0x00000002: 0x00000000;
    controlVal |= (true == m_use121filter)? 0x04000000: 0x00000000;

    MM_REG_WRITE(command, PRZ_CONTROL, controlVal, 0x07FF1DF3);

    DPLOGI("DpEngine_RSZ: H CoeffStep %d   Table  %d \n", m_coeffStepX, horTable);
    DPLOGI("DpEngine_RSZ: V CoeffStep %d   Table  %d \n", m_coeffStepY, verTable);

    return DP_STATUS_RETURN_SUCCESS;
}


DP_STATUS_ENUM DpEngine_SCL::onConfigTile(DpCommand &command)
{
    uint32_t PRZ_drs_padding_dis;
    uint32_t PRZ_drs_lclip_en;
    uint32_t PRZ_input_image_w;
    uint32_t PRZ_input_image_h;
    uint32_t PRZ_luma_hor_int_ofst;
    uint32_t PRZ_luma_hor_subpix_ofst;
    uint32_t PRZ_luma_ver_int_ofst;
    uint32_t PRZ_luma_ver_subpix_ofst;
    uint32_t PRZ_chroma_hor_int_ofst;
    uint32_t PRZ_chroma_hor_subpix_ofst;
    uint32_t PRZ_chroma_ver_int_ofst;
    uint32_t PRZ_chroma_ver_subpix_ofst;
    uint32_t PRZ_output_image_w;
    uint32_t PRZ_output_image_h;
    uint32_t PRZ_urs_clip_en;

    // YUV444->YUV422 downsampler
    if (0 == (m_inTileXRight & 0x1))
    {
        // Odd coordinate, should pad 1 column
        PRZ_drs_padding_dis = 0;
    }
    else
    {
        // Even coordinate, no padding required
        PRZ_drs_padding_dis = 1;
    }

    if ((true == m_use121filter) && (m_inTileXLeft > 0))
    {
        PRZ_drs_lclip_en = 1;
    }
    else
    {
        PRZ_drs_lclip_en = 0;
    }

    PRZ_input_image_w = m_inTileXRight - m_inTileXLeft + 1;
    PRZ_input_image_h = m_inTileYBottom - m_inTileYTop + 1;

    PRZ_luma_hor_int_ofst      = m_lumaXOffset;
    PRZ_luma_hor_subpix_ofst   = m_lumaXSubpixel;
    PRZ_luma_ver_int_ofst      = m_lumaYOffset;
    PRZ_luma_ver_subpix_ofst   = m_lumaYSubpixel;
    PRZ_chroma_hor_int_ofst    = m_chromaXOffset;
    PRZ_chroma_hor_subpix_ofst = m_chromaXSubpixel;
    PRZ_chroma_ver_int_ofst    = m_chromaYOffset;
    PRZ_chroma_ver_subpix_ofst = m_chromaYSubpixel;

    PRZ_output_image_w = m_outTileXRight - m_outTileXLeft + 1;
    PRZ_output_image_h = m_outTileYBottom - m_outTileYTop + 1;

    // YUV422->YUV444 upsampler
    if (m_outTileXRight >= (m_outFrameWidth - 1))
    {
        PRZ_urs_clip_en = 0;
    }
    else
    {
        PRZ_urs_clip_en = 1;
    }

    MM_REG_WRITE(command, PRZ_INPUT_IMAGE, (PRZ_drs_padding_dis << 31) +
                                           (PRZ_drs_lclip_en    << 30) +
                                           (PRZ_input_image_h   << 16) +
                                           (PRZ_input_image_w   <<  0), 0xDFFF1FFF);

#ifdef MDP_RSZ_DISABLE_DCM_SMALL_TILE
    if (PRZ_input_image_w <= 16)
    {
        MM_REG_WRITE(command, PRZ_CONTROL, 1 << 27, 1 << 27); //rsz_dcm_dis
    }
#endif

    MM_REG_WRITE(command, PRZ_LUMA_HORIZONTAL_INTEGER_OFFSET, PRZ_luma_hor_int_ofst, 0x00001FFF);
    MM_REG_WRITE(command, PRZ_LUMA_HORIZONTAL_SUBPIXEL_OFFSET, PRZ_luma_hor_subpix_ofst, 0x001FFFFF);
    MM_REG_WRITE(command, PRZ_LUMA_VERTICAL_INTEGER_OFFSET, PRZ_luma_ver_int_ofst, 0x00001FFF);
    MM_REG_WRITE(command, PRZ_LUMA_VERTICAL_SUBPIXEL_OFFSET, PRZ_luma_ver_subpix_ofst, 0x001FFFFF);
    MM_REG_WRITE(command, PRZ_CHROMA_HORIZONTAL_INTEGER_OFFSET, PRZ_chroma_hor_int_ofst, 0x00001FFF);
    MM_REG_WRITE(command, PRZ_CHROMA_HORIZONTAL_SUBPIXEL_OFFSET, PRZ_chroma_hor_subpix_ofst, 0x001FFFFF);
    MM_REG_WRITE(command, PRZ_CHROMA_VERTICAL_INTEGER_OFFSET, PRZ_chroma_ver_int_ofst, 0x00001FFF);
    MM_REG_WRITE(command, PRZ_CHROMA_VERTICAL_SUBPIXEL_OFFSET, PRZ_chroma_ver_subpix_ofst, 0x001FFFFF);


    MM_REG_WRITE(command, PRZ_OUTPUT_IMAGE, (PRZ_urs_clip_en    << 31) +
                                            (PRZ_output_image_h << 16) +
                                            (PRZ_output_image_w << 0), 0x9FFF1FFF);

    return DP_STATUS_RETURN_SUCCESS;
}

#ifdef MDP_RSZ_DISABLE_DCM_SMALL_TILE
DP_STATUS_ENUM DpEngine_SCL::onAdvanceTile(DpCommand &command)
{
    uint32_t PRZ_input_image_w;

    PRZ_input_image_w = m_inTileXRight - m_inTileXLeft + 1;

    if (PRZ_input_image_w <= 16)
    {
        MM_REG_WRITE(command, PRZ_CONTROL, 0 << 27, 1 << 27); //rsz_dcm_dis
    }

    return DP_STATUS_RETURN_SUCCESS;
}
#endif
