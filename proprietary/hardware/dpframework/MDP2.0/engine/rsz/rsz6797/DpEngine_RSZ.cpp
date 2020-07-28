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
#include "PQRszImpl.h"
#if CONFIG_FOR_OS_ANDROID
#include "cust_tdshp.h"

#include "PQSessionManager.h"
#include "PQAlgorithmFactory.h"
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
          m_verticalFirst(m_data.m_verticalFirst)
    {
        m_pData = &m_data;
    }

    ~DpEngine_SCL()
    {
    }

private:
    MDP_PRZ_DATA                m_data;

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
    uint32_t                    &m_verticalFirst;

    DP_STATUS_ENUM onInitEngine(DpCommand&);

    DP_STATUS_ENUM onDeInitEngine(DpCommand&);

    DP_STATUS_ENUM onConfigFrame(DpCommand&,
                                 DpConfig&);

    DP_STATUS_ENUM onConfigTile(DpCommand&);

#ifdef MDP_RSZ_DISABLE_DCM_SMALL_TILE
    DP_STATUS_ENUM onAdvanceTile(DpCommand&);
#endif

    int32_t getRsz6TapTableIndex(int32_t stepSize);

    int64_t onQueryFeature()
    {
        return eSCL;
    }

#if ENABLE_PQ_RSZ
    void onCalcRsz(DpCommand &command, PQSession *pPQSession);
#endif // ENABLE_PQ_RSZ
};


// Register factory function
static DpEngineBase* SCL0Factory(DpEngineType type)
{
    if (tSCL0 == type)
    {
        return new DpEngine_SCL(0);
    }
    return NULL;
};

// Register factory function
static DpEngineBase* SCL1Factory(DpEngineType type)
{
    if (tSCL1 == type)
    {
        return new DpEngine_SCL(1);
    }
    return NULL;
};

// Register factory function
static DpEngineBase* SCL2Factory(DpEngineType type)
{
    if (tSCL2 == type)
    {
        return new DpEngine_SCL(2);
    }
    return NULL;
};

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


int32_t DpEngine_SCL::getRsz6TapTableIndex(int32_t stepSize)
{
    int32_t table_idx = 0;

    if (stepSize <= 32768) // resize ratio >=1
    {
        table_idx = 27;
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
    if (pPQSession != NULL)
    {
        onCalcRsz(command, pPQSession);
        return DP_STATUS_RETURN_SUCCESS;
    }
#endif // ENABLE_PQ_RSZ

    controlVal = 0;

    if (m_outFrameWidth == m_cropWidth)
    {
        m_horAlgorithm  = DP_TILE_SCALER_ALG0_6_TAPS;
        m_horDirScale   = true;

        // Setup coeffstep and precision
        m_precisionX = (1 << 15);
        m_coeffStepX = m_precisionX;
        horTable        = 27;
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

        if ((m_cropWidth - 1) < (m_outFrameWidth - 1))      // ratio > (1)
        {
            m_horAlgorithm  = DP_TILE_SCALER_ALG0_6_TAPS;
            m_horDirScale   = true;

            // Setup coeffstep and precision
            m_precisionX = (1 << 15);
            m_coeffStepX = (int32_t)((float)(cropWidth - 1) * m_precisionX / (outWidth - 1) + 0.5);
            horTable        = getRsz6TapTableIndex(m_coeffStepX);
        }
        else if ((m_cropWidth - 1) < 24 * (m_outFrameWidth - 1))    // (1/24) < ratio < (1)
        {
            m_horAlgorithm  = DP_TILE_SCALER_ALGO_6N_CUB_ACC;
            m_horDirScale   = true;

            // Setup coeffstep and precision
            m_precisionX = (1 << 20);
            m_coeffStepX = (int32_t)((float)(outWidth - 1) * m_precisionX / (cropWidth - 1) + 1);
            horTable        = 15; // table can be selected between 0~19 by request
        }
        else    // (1/24) > ratio
        {
            m_horAlgorithm  = DP_TILE_SCALER_ALG0_SRC_ACC;
            m_horDirScale   = true;

            // Setup coeffstep and precision
            m_precisionX = (1 << 20);
            m_coeffStepX = (int32_t)((float)(outWidth - 1) * m_precisionX / (cropWidth - 1) + 1);
            horTable        = 0;
        }
    }

    MM_REG_WRITE(command, PRZ_HORIZONTAL_COEFF_STEP, m_coeffStepX, 0x007FFFFF);

    if (m_outFrameHeight == m_cropHeight)
    {
        m_verAlgorithm  = DP_TILE_SCALER_ALG0_6_TAPS;
        m_verDirScale   = true;

        // Setup coeffstep and precision
        m_precisionY = (1 << 15);
        m_coeffStepY = m_precisionY;
        verTable        = 27;
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

        if ((m_cropHeight - 1) < (m_outFrameHeight - 1))    // ratio > (1)
        {
            m_verAlgorithm  = DP_TILE_SCALER_ALG0_6_TAPS;
            m_verDirScale   = true;

            // Setup coeffstep and precision
            m_precisionY = (1 << 15);
            m_coeffStepY = (int32_t)((float)(cropHeight - 1) * m_precisionY / (outHeight - 1) + 0.5);
            verTable        = getRsz6TapTableIndex(m_coeffStepY);
        }
        else if ((m_cropHeight - 1) < 24 * (m_outFrameHeight - 1))  // (1/24) < ratio < (1)
        {
            m_verAlgorithm  = DP_TILE_SCALER_ALGO_6N_CUB_ACC;
            m_verDirScale   = true;

            // Setup coeffstep and precision
            m_precisionY = (1 << 20);
            m_coeffStepY = (int32_t)((float)(outHeight - 1) * m_precisionY / (cropHeight - 1) + 1);
            verTable        = 15; // table can be selected between 0~19 by request
        }
        else    // (1/24) > ratio
        {
            m_verAlgorithm  = DP_TILE_SCALER_ALG0_SRC_ACC;
            m_verDirScale   = true;

            // Setup coeffstep and precision
            m_precisionY = (1 << 20);
            m_coeffStepY = (int32_t)((float)(outHeight - 1) * m_precisionY / (cropHeight - 1) + 1);
            verTable        = 0;
        }
    }

    MM_REG_WRITE(command, PRZ_VERTICAL_COEFF_STEP, m_coeffStepY, 0x007FFFFF);

    // Vertical first
    controlVal |= (m_outFrameWidth > m_cropWidth)? (1 << 4): 0;

    controlVal |= (m_horAlgorithm << 5);
    controlVal |= (horTable << 16);

    controlVal |= (m_verAlgorithm << 7);
    controlVal |= (verTable << 21);

    controlVal |= (true == m_horDirScale)? (1 << 0): 0;
    controlVal |= (true == m_verDirScale)? (1 << 1): 0;

    MM_REG_WRITE(command, PRZ_CONTROL_1, controlVal, 0x03FF03F3);

    DPLOGI("DpEngine_RSZ: H CoeffStep %d Table %d\n", m_coeffStepX, horTable);
    DPLOGI("DpEngine_RSZ: V CoeffStep %d Table %d\n", m_coeffStepY, verTable);

    return DP_STATUS_RETURN_SUCCESS;
}


DP_STATUS_ENUM DpEngine_SCL::onConfigTile(DpCommand &command)
{
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

    MM_REG_WRITE(command, PRZ_INPUT_IMAGE, (PRZ_input_image_h   << 16) +
                                           (PRZ_input_image_w   <<  0), 0xFFFFFFFF);

#ifdef MDP_RSZ_DISABLE_DCM_SMALL_TILE
    if (PRZ_input_image_w <= 16)
    {
        MM_REG_WRITE(command, PRZ_CONTROL_1, 1 << 27, 1 << 27); //rsz_dcm_dis
    }
#endif

    MM_REG_WRITE(command, PRZ_LUMA_HORIZONTAL_INTEGER_OFFSET, PRZ_luma_hor_int_ofst, 0x0000FFFF);
    MM_REG_WRITE(command, PRZ_LUMA_HORIZONTAL_SUBPIXEL_OFFSET, PRZ_luma_hor_subpix_ofst, 0x001FFFFF);
    MM_REG_WRITE(command, PRZ_LUMA_VERTICAL_INTEGER_OFFSET, PRZ_luma_ver_int_ofst, 0x0000FFFF);
    MM_REG_WRITE(command, PRZ_LUMA_VERTICAL_SUBPIXEL_OFFSET, PRZ_luma_ver_subpix_ofst, 0x001FFFFF);
    MM_REG_WRITE(command, PRZ_CHROMA_HORIZONTAL_INTEGER_OFFSET, PRZ_chroma_hor_int_ofst, 0x0000FFFF);
    MM_REG_WRITE(command, PRZ_CHROMA_HORIZONTAL_SUBPIXEL_OFFSET, PRZ_chroma_hor_subpix_ofst, 0x001FFFFF);
    MM_REG_WRITE(command, PRZ_CHROMA_VERTICAL_INTEGER_OFFSET, PRZ_chroma_ver_int_ofst, 0x0000FFFF);
    MM_REG_WRITE(command, PRZ_CHROMA_VERTICAL_SUBPIXEL_OFFSET, PRZ_chroma_ver_subpix_ofst, 0x001FFFFF);

    MM_REG_WRITE(command, PRZ_OUTPUT_IMAGE, (PRZ_output_image_h << 16) +
                                            (PRZ_output_image_w << 0), 0xFFFFFFFF);

    return DP_STATUS_RETURN_SUCCESS;
}

#ifdef MDP_RSZ_DISABLE_DCM_SMALL_TILE
DP_STATUS_ENUM DpEngine_SCL::onAdvanceTile(DpCommand &command)
{
    uint32_t PRZ_input_image_w;

    PRZ_input_image_w = m_inTileXRight - m_inTileXLeft + 1;

    if (PRZ_input_image_w <= 16)
    {
        MM_REG_WRITE(command, PRZ_CONTROL_1, 0 << 27, 1 << 27); //rsz_dcm_dis
    }

    return DP_STATUS_RETURN_SUCCESS;
}
#endif

#if ENABLE_PQ_RSZ
void DpEngine_SCL::onCalcRsz(DpCommand &command, PQSession *pPQSession)
{
    RszInput inParam;
    RszOutput outParam;

    memset(&inParam, 0, sizeof(RszInput));
    memset(&outParam, 0, sizeof(RszOutput));

    // Prepare RszInput parameter
    inParam.srcWidth = m_inFrameWidth;
    inParam.srcHeight = m_inFrameHeight;
    inParam.dstWidth = m_outFrameWidth;
    inParam.dstHeight = m_outFrameHeight;
    inParam.cropOffsetX = m_cropOffsetX;
    inParam.cropSubpixX = m_cropSubpixX;
    inParam.cropOffsetY = m_cropOffsetY;
    inParam.cropSubpixY = m_cropSubpixY;
    inParam.cropWidth = m_cropWidth;
    inParam.cropHeight = m_cropHeight;

    inParam.enable = 1; //from comment
    inParam.yuv422Tyuv444 = 0; //from comment
    inParam.demoEnable = 0; //from comment

    /*
    // Demo Window
    inParam.demoOutHStart = 0; //from CODA default
    inParam.demoOutHEnd = 0; //from CODA default
    inParam.demoOutVStart = 0; //from CODA default
    inParam.demoOutVEnd = 0; //from CODA default
    */

    DPLOGI("DpEngine_RSZ: inParam.srcWidth[%d], inParam.srcHeight[%d]\n", inParam.srcWidth, inParam.srcHeight);
    DPLOGI("DpEngine_RSZ: inParam.dstWidth[%d], inParam.dstHeight[%d]\n", inParam.dstWidth, inParam.dstHeight);

    DPLOGI("DpEngine_RSZ: m_identifier[%d]\n", m_identifier);

    PQAlgorithmFactory* pPQAlgorithmFactory = PQAlgorithmFactory::getInstance();
    PQRSZAdaptor* pPQRSZAdaptor = PQAlgorithmFactory::getInstance()->getRSZ(m_identifier);

    pPQRSZAdaptor->calRegs(pPQSession, command, &inParam, &outParam);

    // Need for tile calculation
    m_horAlgorithm  = (DP_TILE_SCALER_ALGO_ENUM)outParam.horAlgo;
    m_horDirScale   = outParam.horEnable;
    m_precisionX = outParam.precX;
    m_coeffStepX = outParam.coeffStepX;

    m_verAlgorithm  = (DP_TILE_SCALER_ALGO_ENUM)outParam.verAlgo;
    m_verDirScale   = outParam.verEnable;
    m_precisionY = outParam.precY;
    m_coeffStepY = outParam.coeffStepY;

    m_verticalFirst = outParam.verticalFirst;
}
#endif // ENABLE_PQ_RSZ
