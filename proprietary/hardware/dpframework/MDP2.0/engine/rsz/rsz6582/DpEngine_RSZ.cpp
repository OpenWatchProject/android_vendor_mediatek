#include "DpTileEngine.h"
#include "DpEngineType.h"
#include "DpTileScaler.h"
#include "mdp_reg_rsz.h"
#include "tile_mdp_reg.h"

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

    int64_t onQueryFeature()
    {
        return eSCL;
    }

    uint32_t rsz_4tapFIR_tbl_select(uint32_t stepSize);
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

EngineReg SCL0Reg(SCL0Factory);
EngineReg SCL1Reg(SCL1Factory);

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


uint32_t DpEngine_SCL::rsz_4tapFIR_tbl_select(uint32_t stepSize)
{
    uint32_t table_idx;

    if (stepSize <= 32768)
    {
        table_idx = 20;
    }
    else if (stepSize < 36408)
    {
        table_idx = 21;
    }
    else if (stepSize < 40960)
    {
        table_idx = 22;
    }
    else if (stepSize < 46811)
    {
        table_idx = 23;
    }
    else if (stepSize < 54613)
    {
        table_idx = 24;
    }
    else					   
    {
        table_idx = 25;
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

    if ((m_cropWidth     == m_inFrameWidth) &&
        (m_inFrameWidth  == m_outFrameWidth) &&
        (m_cropHeight    == m_inFrameHeight) &&
        (m_inFrameHeight == m_outFrameHeight))
    {

        m_bypassEngine = true;

        MM_REG_WRITE(command, PRZ_ENABLE, 0x00000000, 0x00000001);
        
        return DP_STATUS_RETURN_SUCCESS;
    }
    else
    {
        m_bypassEngine = false;
    }

    controlVal = 0;

    // Vertical first
    controlVal |= (m_outFrameWidth > m_cropWidth)? 0x00000010: 0x00000000;

    if (m_outFrameWidth == m_cropWidth)
    {
        m_horAlgorithm  = DP_TILE_SCALER_ALG0_4_TAPS;
        m_horDirScale   = true;
        controlVal     |= (0 << 5);

        // Setup coeffstep and precision
        m_precisionX = (1 << 15);
        m_coeffStepX = m_precisionX;

        horTable        = rsz_4tapFIR_tbl_select(m_coeffStepX);
        controlVal     |= (horTable << 16); 
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

        if((m_cropWidth - 1) < 2 * (m_outFrameWidth - 1))
        {
            m_horAlgorithm  = DP_TILE_SCALER_ALG0_4_TAPS;
            m_horDirScale   = true;
            controlVal     |= (0 << 5);

            // Setup coeffstep and precision
            m_precisionX = (1 << 15);
            m_coeffStepX = (int32_t)((float)(cropWidth - 1) * m_precisionX / (outWidth - 1) + 0.5);

            horTable        = rsz_4tapFIR_tbl_select(m_coeffStepX);
            controlVal     |= (horTable << 16);
        }
        else if ((m_cropWidth - 1) < 64 * (m_outFrameWidth - 1))
        {
            m_horAlgorithm  = DP_TILE_SCALER_ALGO_CUB_ACC;
            m_horDirScale   = true;
            controlVal     |= (2 << 5);

            // Setup coeffstep and precision
            m_precisionX = (1 << 20);
            m_coeffStepX = (int32_t)((float)(outWidth - 1) * m_precisionX / (cropWidth - 1) + 1);

            horTable        = 15; // table can be selected between 0~19 by request
            controlVal     |= (horTable << 16);
        }
        else
        {
            m_horAlgorithm  = DP_TILE_SCALER_ALG0_SRC_ACC;
            m_horDirScale   = true;
            controlVal     |= (1 << 5);

            // Setup coeffstep and precision
            m_precisionX = (1 << 20);
            m_coeffStepX = (int32_t)((float)(outWidth - 1) * m_precisionX / (cropWidth - 1) + 1);

            horTable        = 0;
            controlVal     |= (horTable << 16);
        }
    }

    MM_REG_WRITE(command, PRZ_HORIZONTAL_COEFF_STEP, m_coeffStepX, 0x007FFFFF);

    if (m_outFrameHeight == m_cropHeight)
    {
        m_verAlgorithm  = DP_TILE_SCALER_ALG0_4_TAPS;
        m_verDirScale   = true;
        controlVal     |= (0 << 7);

        // Setup coeffstep and precision
        m_precisionY = (1 << 15);
        m_coeffStepY = m_precisionY;

        verTable        = rsz_4tapFIR_tbl_select(m_coeffStepY);
        controlVal     |= (verTable << 21); 
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

        if((m_cropHeight - 1) < 2 * (m_outFrameHeight - 1))
        {
            m_verAlgorithm  = DP_TILE_SCALER_ALG0_4_TAPS;
            m_verDirScale   = true;
            controlVal     |= (0 << 7);

            // Setup coeffstep and precision
            m_precisionY = (1 << 15);
            m_coeffStepY = (int32_t)((float)(cropHeight - 1) * m_precisionY / (outHeight - 1) + 0.5);

            verTable        = rsz_4tapFIR_tbl_select(m_coeffStepY);
            controlVal     |= (verTable << 21);      
        }
        else if ((m_cropHeight - 1) < 64 * (m_outFrameHeight - 1))
        {
            m_verAlgorithm  = DP_TILE_SCALER_ALGO_CUB_ACC;
            m_verDirScale   = true;
            controlVal     |= (2 << 7);
        
            // Setup coeffstep and precision
            m_precisionY = (1 << 20);
            m_coeffStepY = (int32_t)((float)(outHeight - 1) * m_precisionY / (cropHeight - 1) + 1);

            verTable        = 15; // table can be selected between 0~19 by request
            controlVal     |= (verTable << 21); 
        }
        else
        {
            m_verAlgorithm  = DP_TILE_SCALER_ALG0_SRC_ACC;
            m_verDirScale   = true;
            controlVal     |= (1 << 7);
    
            // Setup coeffstep and precision
            m_precisionY = (1 << 20);
            m_coeffStepY = (int32_t)((float)(outHeight - 1) * m_precisionY / (cropHeight - 1) + 1);

            verTable        = 0;
            controlVal     |= (verTable << 21); 
        }
    }

    MM_REG_WRITE(command, PRZ_VERTICAL_COEFF_STEP, m_coeffStepY, 0x007FFFFF);

    if (DP_COLOR_GET_H_SUBSAMPLE(config.inFormat))
    {
        // C42 conversion: drop if source is YUV422 or YUV420
        m_use121filter = false;
    }

    controlVal |= (true == m_horDirScale)?  0x00000001: 0x00000000;
    controlVal |= (true == m_verDirScale)?  0x00000002: 0x00000000;
    controlVal |= (true == m_use121filter)? 0x04000000: 0x00000000;

    MM_REG_WRITE(command, PRZ_CONTROL, controlVal, 0x07FF01F3);
 
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
