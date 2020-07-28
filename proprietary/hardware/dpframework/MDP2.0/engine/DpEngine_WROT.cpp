#include "DpTileEngine.h"
#include "DpEngineType.h"
#include "DpColorConv.h"
#include "DpPlatform.h"
#include "mdp_reg_wrot.h"
#include "tile_mdp_reg.h"
#include "DpDataType.h"
#if ESL_SETTING
#include "DpESLControl.h"
#endif
#include "DpEngineSetting_WROT.h"

//--------------------------------------------------------
// WROT driver engine
//--------------------------------------------------------
class DpEngine_WROT: public DpTileEngine
{
public:
    DpEngine_WROT(uint32_t identifier)
        : DpTileEngine(identifier),
          m_targetFormat(m_data.m_targetFormat),
          m_targetPitchY(0),
          m_targetPitchUV(0),
          m_rotationAngle(m_data.m_rotationAngle),
          m_flipHorizontal(m_data.m_flipHorizontal),
          m_enWriteCrop(false),
          m_outXOffset(0),
          m_outCropWidth(0),
          m_bitsPerPixelY(0),
          m_bitsPerPixelUV(0),
          m_horRightShiftUV(0),
          m_verRightShiftUV(0),
          m_FIFOMaxSize(m_data.m_FIFOMaxSize),
          m_maxLineCount(m_data.m_maxLineCount),
          m_enAlphaRot(m_data.m_enAlphaRot)
    {
        m_data.m_identifier = m_identifier;
        m_pData = &m_data;
    }

    ~DpEngine_WROT()
    {
    }

private:
    static const uint32_t s_uvTable[2][4][2][4];
    static const DpColorMatrix RGB_TO_BT601;
    static const DpColorMatrix RGB_TO_JPEG;

    MDP_WROT_DATA   m_data;

    DpColorFormat   &m_targetFormat;
    uint32_t        m_targetPitchY;
    uint32_t        m_targetPitchUV;
    uint32_t        &m_rotationAngle;
    bool            &m_flipHorizontal;
    bool            m_enWriteCrop;
    int32_t         m_outXOffset;
    int32_t         m_outCropWidth;
    uint32_t        m_bitsPerPixelY;
    uint32_t        m_bitsPerPixelUV;
    uint32_t        m_horRightShiftUV;
    uint32_t        m_verRightShiftUV;
    uint32_t        &m_FIFOMaxSize;
    uint32_t        &m_maxLineCount;
    bool            &m_enAlphaRot;

    DP_STATUS_ENUM mapColorFormat(DP_PROFILE_ENUM inProfile,
                                  DP_PROFILE_ENUM outProfile,
                                  DpColorFormat   colorFormat,
                                  uint32_t        &ROTFormat,
                                  uint32_t        &ROTSwap,
                                  uint32_t        &ROTMatEn,
                                  uint32_t        &matrixSel)
    {
        // Default values
        ROTMatEn  = 0;
        matrixSel = 0;

        ROTFormat        = DP_COLOR_GET_HW_FORMAT(colorFormat);
        ROTSwap          = DP_COLOR_GET_SWAP_ENABLE(colorFormat);
        m_bitsPerPixelY = DP_COLOR_BITS_PER_PIXEL(colorFormat);

        switch (colorFormat)
        {
            case DP_COLOR_GREY:
                // Y only
                m_bitsPerPixelUV = 0;
                m_horRightShiftUV = 0;
                m_verRightShiftUV = 0;
                break;
            case DP_COLOR_RGB565:
            case DP_COLOR_BGR565:
            case DP_COLOR_RGB888:
            case DP_COLOR_BGR888:
            case DP_COLOR_RGBA8888:
            case DP_COLOR_BGRA8888:
            case DP_COLOR_ARGB8888:
            case DP_COLOR_ABGR8888:
#ifdef HW_SUPPORT_10BIT_PATH
            case DP_COLOR_RGBA1010102:
            case DP_COLOR_BGRA1010102:
            case DP_COLOR_ARGB1010102:
            case DP_COLOR_ABGR1010102:
#endif // HW_SUPPORT_10BIT_PATH
                m_bitsPerPixelUV = 0;
                m_horRightShiftUV = 0;
                m_verRightShiftUV = 0;
                ROTMatEn          = 1;
                break;
            case DP_COLOR_UYVY:
            case DP_COLOR_VYUY:
            case DP_COLOR_YUYV:
            case DP_COLOR_YVYU:
            case DP_COLOR_RGB565_RAW:
                // YUV422, 1 plane
                m_bitsPerPixelUV = 0;
                m_horRightShiftUV = 0;
                m_verRightShiftUV = 0;
                break;
            case DP_COLOR_I420:
            case DP_COLOR_YV12:
                // YUV420, 3 plane
                m_bitsPerPixelUV = 8;
                m_horRightShiftUV = 1;
                m_verRightShiftUV = 1;
                break;
            case DP_COLOR_I422:
            case DP_COLOR_YV16:
                // YUV422, 3 plane
                m_bitsPerPixelUV = 8;
                m_horRightShiftUV = 1;
                m_verRightShiftUV = 0;
                break;
            case DP_COLOR_I444:
            case DP_COLOR_YV24:
                // YUV444, 3 plane
                m_bitsPerPixelUV = 8;
                m_horRightShiftUV = 0;
                m_verRightShiftUV = 0;
                break;
            case DP_COLOR_NV12:
            case DP_COLOR_NV21:
                // YUV420, 2 plane
                m_bitsPerPixelUV = 16;
                m_horRightShiftUV = 1;
                m_verRightShiftUV = 1;
                break;
            case DP_COLOR_NV16:
            case DP_COLOR_NV61:
                // YUV422, 2 plane
                m_bitsPerPixelUV = 16;
                m_horRightShiftUV = 1;
                m_verRightShiftUV = 0;
                break;
#ifdef HW_SUPPORT_10BIT_PATH
            case DP_COLOR_NV12_10L:
            case DP_COLOR_NV21_10L:
                // P010 YUV420, 2 plane 10bit
                m_bitsPerPixelUV = 32;
                m_horRightShiftUV = 1;
                m_verRightShiftUV = 1;
                break;
            case DP_COLOR_NV12_10P:
            case DP_COLOR_NV21_10P:
                // MTK packet YUV420, 2 plane 10bit
                m_bitsPerPixelUV = 20;
                m_horRightShiftUV = 1;
                m_verRightShiftUV = 1;
                break;
#endif // HW_SUPPORT_10BIT_PATH
            default:
                DPLOGE("DpEngine_WROT: unsupported color format\n");
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

        if (1 == ROTMatEn)
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
                DPLOGE("DpEngineWROT: unknown color conversion\n");
                assert(0);
                return DP_STATUS_INVALID_PARAX;
            }
        }
        else
        {
            if ((DP_PROFILE_JPEG  == inProfile) &&
                (DP_PROFILE_BT601 == outProfile))
            {
                ROTMatEn  = 1;
                matrixSel = 8;
            }
            else if ((DP_PROFILE_JPEG  == inProfile) &&
                     (DP_PROFILE_BT709 == outProfile))
            {
                ROTMatEn  = 1;
                matrixSel = 9;
            }
            else if ((DP_PROFILE_BT601 == inProfile) &&
                     (DP_PROFILE_JPEG  == outProfile))
            {
                ROTMatEn  = 1;
                matrixSel = 10;
            }
            else if ((DP_PROFILE_BT709 == inProfile) &&
                     (DP_PROFILE_JPEG  == outProfile))
            {
                ROTMatEn  = 1;
                matrixSel = 11;
            }
            else if ((DP_PROFILE_BT709 == inProfile) &&
                     (DP_PROFILE_BT601 == outProfile))
            {
                ROTMatEn  = 1;
                matrixSel = 12;
            }
            else if ((DP_PROFILE_BT601 == inProfile) &&
                     (DP_PROFILE_BT709 == outProfile))
            {
                ROTMatEn  = 1;
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
#if DMA_SUPPORT_ALPHA_ROT
        return eROT | eFLIP | eWMEM | eALPHA_ROT;
#else
        return eROT | eFLIP | eWMEM;
#endif
    }
};


// register factory function
static DpEngineBase* WROT0Factory(DpEngineType type)
{
    if (tWROT0 == type)
    {
        return new DpEngine_WROT(0);
    }
    return NULL;
};

// register factory function
static DpEngineBase* WROT1Factory(DpEngineType type)
{
    if (tWROT1 == type)
    {
        return new DpEngine_WROT(1);
    }
    return NULL;
};

#ifdef tWROT2
// register factory function
static DpEngineBase* WROT2Factory(DpEngineType type)
{
    if (tWROT2 == type)
    {
        return new DpEngine_WROT(2);
    }
    return NULL;
};
#endif

#ifdef tWROT3
// register factory function
static DpEngineBase* WROT3Factory(DpEngineType type)
{
    if (tWROT3 == type)
    {
        return new DpEngine_WROT(3);
    }
    return NULL;
};
#endif

// register factory function
EngineReg WROT0Reg(WROT0Factory);
EngineReg WROT1Reg(WROT1Factory);
#ifdef tWROT2
EngineReg WROT2Reg(WROT2Factory);
#endif
#ifdef tWROT3
EngineReg WROT3Reg(WROT3Factory);
#endif

// filt_h, filt_v, uv_xsel, uv_ysel
const uint32_t DpEngine_WROT::s_uvTable[2][4][2][4] = {
    {    // YUV422
        {    // 0
            { 1 /* [1 2 1] */, 0 /* drop  */, 0, 2 },
            { 2 /* [1 2 1] */, 0 /* drop  */, 1, 2 }, // flip
        }, { // 90
            { 0 /* drop    */, 4 /* [1 1] */, 2, 1 },
            { 0 /* drop    */, 3 /* [1 1] */, 2, 0 }, // flip
        }, { // 180
            { 2 /* [1 2 1] */, 0 /* drop  */, 1, 2 },
            { 1 /* [1 2 1] */, 0 /* drop  */, 0, 2 }, // flip
        }, { // 270
            { 0 /* drop    */, 3 /* [1 1] */, 2, 0 },
            { 0 /* drop    */, 4 /* [1 1] */, 2, 1 }, // flip
        },
    }, { // YUV420
        {    // 0
#if defined(tWDMA) && tWDMA != tNone
             // Follow WDMA behavior
            { 1 /* [1 2 1] */, 3 /* [1 1] */, 0, 1 },
            { 2 /* [1 2 1] */, 3 /* [1 1] */, 1, 1 }, // flip
#else
            { 1 /* [1 2 1] */, 3 /* [1 1] */, 0, 0 },
            { 2 /* [1 2 1] */, 3 /* [1 1] */, 1, 0 }, // flip
#endif
        }, { // 90
            { 1 /* [1 2 1] */, 4 /* [1 1] */, 0, 1 },
            { 1 /* [1 2 1] */, 3 /* [1 1] */, 0, 0 }, // flip
        }, { // 180
            { 2 /* [1 2 1] */, 4 /* [1 1] */, 1, 1 },
            { 1 /* [1 2 1] */, 4 /* [1 1] */, 0, 1 }, // flip
        }, { // 270
            { 2 /* [1 2 1] */, 3 /* [1 1] */, 1, 0 },
            { 2 /* [1 2 1] */, 4 /* [1 1] */, 1, 1 }, // flip
        },
    }
};

const DpColorMatrix DpEngine_WROT::RGB_TO_BT601 = DpColorMatrix(
     0.256788,  0.504129,  0.097906,
    -0.148223, -0.290993,  0.439216,
     0.439216, -0.367788, -0.071427);

const DpColorMatrix DpEngine_WROT::RGB_TO_JPEG = DpColorMatrix(
     0.299000,  0.587000,  0.114000,
    -0.168736, -0.331264,  0.500000,
     0.500000, -0.418688, -0.081312);


DP_STATUS_ENUM DpEngine_WROT::onInitEngine(DpCommand &command)
{
#ifdef WROT_FILTER_CONSTRAINT
    // Wait WROT SRAM shared to DISP RDMA
    if (0 == m_identifier)
    {
        MM_REG_WAIT_NO_CLEAR(command, DpCommand::SYNC_WROT0_SRAM_READY);
    }
    else
    {
        MM_REG_WAIT_NO_CLEAR(command, DpCommand::SYNC_WROT1_SRAM_READY);
    }
#endif

    // Reset engine
    MM_REG_WRITE(command, VIDO_SOFT_RST, 0x01, 0x00000001);
#ifndef MDP_VERSION_8173
    MM_REG_POLL(command, VIDO_SOFT_RST_STAT, 0x01, 0x00000001);
#else
    MM_REG_POLL(command, VIDO_SOFT_RST_STAT, 0x00, 0x00000001);
#endif
    MM_REG_WRITE(command, VIDO_SOFT_RST, 0x00, 0x00000001);
    MM_REG_POLL(command, VIDO_SOFT_RST_STAT, 0x00, 0x00000001);

    return DP_STATUS_RETURN_SUCCESS;
}


DP_STATUS_ENUM DpEngine_WROT::onDeInitEngine(DpCommand &)
{
    return DP_STATUS_RETURN_SUCCESS;
}


DP_STATUS_ENUM DpEngine_WROT::onConfigFrame(DpCommand &command, DpConfig &config)
{
    DP_STATUS_ENUM status;
    uint32_t       WROT_rotation;
    uint32_t       WROT_flipStatus;
    uint32_t       WROT_crop_en;
    uint32_t       WROT_uv_xsel = 0;
    uint32_t       WROT_uv_ysel = 0;
    uint32_t       WROT_format;
    uint32_t       WROT_swap;
    uint32_t       WROT_mat_en;
    uint32_t       WROT_ext_mat;
    uint32_t       WROT_mat_sel;
    uint32_t       WROT_alpha_rotation;
    uint32_t       WROT_fifo_test = 0;
#ifdef MDP_VERSION_8173
    uint32_t       WROT_max_burst_len;
#endif
    uint32_t       WROT_filt_h = 0;
    uint32_t       WROT_filt_v = 0;
    int32_t        targetXStart;
    int32_t        targetYStart;
    int32_t        outRoiWidth;
    int32_t        outRoiHeight;
    uint32_t       hSubsample;
    uint32_t       vSubsample;
    uint32_t       handle;
    uint32_t       secureRegAddr[3] = {(uint32_t)VIDO_BASE_ADDR, (uint32_t)VIDO_BASE_ADDR_C, (uint32_t)VIDO_BASE_ADDR_V};
    uint32_t       offset[3] = {0};
    uint32_t       planeOffset[3] = {0};
    int32_t        index = 0;
    uint32_t       WROT_preultra = 0;
#ifdef HW_SUPPORT_10BIT_PATH
    uint32_t       WROT_scan_10bit = 0;
    uint32_t       WROT_pending_zero = 0;
#endif // HW_SUPPORT_10BIT_PATH

    // Save frame related information
    m_targetFormat   = config.outFormat;
    m_targetPitchY   = config.outYPitch;
    m_targetPitchUV  = config.outUVPitch;
    m_rotationAngle  = config.rotation;
    m_flipHorizontal = config.enFlip;
    targetXStart     = config.outXStart;
    targetYStart     = config.outYStart;

    if (m_rotationAngle == 90 || m_rotationAngle ==270)
    {
        outRoiHeight    = config.outRoiWidth;
        outRoiWidth     = config.outRoiHeight;
    }
    else
    {
        outRoiWidth      = config.outRoiWidth;
        outRoiHeight     = config.outRoiHeight;
    }

    m_enWriteCrop    = config.enWROTCrop;
    m_outXOffset     = config.outXOffset;
    m_outCropWidth   = config.outCropWidth;
    m_enAlphaRot     = config.enAlphaRot;

    status = mapColorFormat(config.inProfile,
                            config.outProfile,
                            config.outFormat,
                            WROT_format,
                            WROT_swap,
                            WROT_mat_en,
                            WROT_mat_sel);
    if (DP_STATUS_RETURN_SUCCESS != status)
    {
        DPLOGE("DpEngine_WROT: map color format failed\n");
        return status;
    }

    command.addMetLog((m_identifier?"MDP_WROT1__VIDO_TAR_SIZE":"MDP_WROT0__VIDO_TAR_SIZE"),
            (config.outWidth << 0) + (config.outHeight <<  16));

    command.addMMpathLog((m_identifier?"out_2_width":"out_1_width"), config.outWidth, DP_MMPATH_OTHER);
    command.addMMpathLog((m_identifier?"out_2_height":"out_1_height"), config.outHeight, DP_MMPATH_OTHER);
    command.addMMpathLog((m_identifier?"hw=MDP,out_2":"hw=MDP,out_1"), config.outFormat, DP_MMPATH_FORMAT);
    command.addMMpathLog((m_identifier?"MDP_WROT1":"MDP_WROT0"), 1, DP_MMPATH_OTHER);
    command.addMMpathLog((m_identifier?"out_2":"out_1"), config.memAddr[0], DP_MMPATH_ADDR);

    if (m_enAlphaRot)
    {
        WROT_alpha_rotation = 1;
        WROT_mat_en = 0;
        WROT_swap = (WROT_swap == 1 ? 0 : 1);//sw workaround, wait DE feedback
    }
    else
    {
        WROT_alpha_rotation = 0;
    }

    if (config.pColorMatrix && config.pColorMatrix->enable)
    {
        WROT_mat_en = 1;
        WROT_ext_mat = 1;
    }
    else
    {
        WROT_ext_mat = 0;
    }

    switch (m_rotationAngle)
    {
        case 0:
            WROT_rotation = 0;
            break;
        case 90:
            WROT_rotation = 1;
            break;
        case 180:
            WROT_rotation = 2;
            break;
        case 270:
            WROT_rotation = 3;
            break;
        default:
            assert(0);
            return DP_STATUS_INVALID_ANGLE;
    }

    if (m_flipHorizontal)
    {
        WROT_flipStatus = 1;
    }
    else
    {
        WROT_flipStatus = 0;
    }

    // Force crop to be 1
    WROT_crop_en = 1;

    hSubsample = DP_COLOR_GET_H_SUBSAMPLE(m_targetFormat);
    vSubsample = DP_COLOR_GET_V_SUBSAMPLE(m_targetFormat);

    if (hSubsample == 1)    // YUV422/420 out
    {
        //WROT_filt_h  = s_uvTable[vSubsample][WROT_rotation][WROT_flipStatus][0];
        WROT_filt_v  = (DP_COLOR_GET_V_SUBSAMPLE(config.inFormat) ||
                        (DP_COLOR_GET_COLOR_GROUP(config.inFormat) == 2)
#if defined(tWDMA) && tWDMA != tNone
                        || (config.rootAndLeaf > 2)
#endif
                       ) ? 0    // YUV420 or direct link in
                     : s_uvTable[vSubsample][WROT_rotation][WROT_flipStatus][1];
        WROT_uv_xsel = s_uvTable[vSubsample][WROT_rotation][WROT_flipStatus][2];
        WROT_uv_ysel = s_uvTable[vSubsample][WROT_rotation][WROT_flipStatus][3];
    }
    else if (DP_COLOR_GREY == m_targetFormat)
    {
        WROT_uv_xsel = 0;
        WROT_uv_ysel = 0;
    }
    else
    {
        WROT_uv_xsel = 2;
        WROT_uv_ysel = 2;
    }

    // Note: check odd size roi_w & roi_h for uv_xsel/uv_ysel
    if ((outRoiWidth & 0x1) && (WROT_uv_xsel == 1))
    {
        WROT_uv_xsel = 0;
    }
    if ((outRoiHeight & 0x1) && (WROT_uv_ysel == 1))
    {
        WROT_uv_ysel = 0;
    }

    // Note: WROT not support UV swap
    if ((1 == DP_COLOR_GET_SWAP_ENABLE(config.outFormat)) && (3 == DP_COLOR_GET_PLANE_COUNT(config.outFormat)))
    {
        uint32_t memAddrTmp = config.memAddr[1];
        config.memAddr[1] = config.memAddr[2];
        config.memAddr[2] = memAddrTmp;
    }

    handle = config.memAddr[0];
    planeOffset[0] = 0;
    planeOffset[1] = config.memAddr[1];
    planeOffset[2] = config.memAddr[2];

    if ((targetXStart != 0) ||
        (targetYStart != 0))
    {
        offset[0] = (targetXStart * m_bitsPerPixelY >> 3) + (m_targetPitchY * targetYStart);

        offset[1] = ((targetXStart >> m_horRightShiftUV) * m_bitsPerPixelUV >> 3) +
                     (targetYStart >> m_verRightShiftUV) * m_targetPitchUV;

        offset[2] = ((targetXStart >> m_horRightShiftUV) * m_bitsPerPixelUV >> 3) +
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

    uint32_t frameMemAddrInfo[3];
    frameMemAddrInfo[0] = config.memAddr[0];
    frameMemAddrInfo[1] = config.memAddr[1];
    frameMemAddrInfo[2] = config.memAddr[2];

    if ((1 == DP_COLOR_GET_SWAP_ENABLE(config.outFormat)) && (3 == DP_COLOR_GET_PLANE_COUNT(config.outFormat)))
    {
        frameMemAddrInfo[1] = config.memAddr[2];
        frameMemAddrInfo[2] = config.memAddr[1];
    }

    if(0 == m_rotationAngle || 180 == m_rotationAngle)
    {
        command.frameDstInfo(config.outFormat,
                             config.outWidth,
                             config.outHeight,
                             config.outYPitch,
                             config.outUVPitch,
                             config.outXStart,
                             config.outYStart,
                             frameMemAddrInfo,
                             config.memSize,
                             m_pOutDataPort[0]->getSecureStatus());
    }
    else
    {
        command.frameDstInfo(config.outFormat,
                             config.outHeight,
                             config.outWidth,
                             config.outYPitch,
                             config.outUVPitch,
                             config.outXStart,
                             config.outYStart,
                             frameMemAddrInfo,
                             config.memSize,
                             m_pOutDataPort[0]->getSecureStatus());
    }

    // Write frame base address
    MM_REG_WRITE(command, VIDO_BASE_ADDR,   config.memAddr[0], 0xFFFFFFFF, &m_frameConfigLabel[index++]);
    MM_REG_WRITE(command, VIDO_BASE_ADDR_C, config.memAddr[1], 0xFFFFFFFF, &m_frameConfigLabel[index++]);
    MM_REG_WRITE(command, VIDO_BASE_ADDR_V, config.memAddr[2], 0xFFFFFFFF, &m_frameConfigLabel[index++]);

    DPLOGI("DpEngine_WROT: output0 MVA 0x%08x\n", config.memAddr[0]);
    DPLOGI("DpEngine_WROT: output1 MVA 0x%08x\n", config.memAddr[1]);
    DPLOGI("DpEngine_WROT: output2 MVA 0x%08x\n", config.memAddr[2]);

#ifdef WROT_DISABLE_PREULTRA
    WROT_preultra = 0;
#else
    WROT_preultra = 1;
#endif // WROT_DISABLE_PREULTRA

    // Write frame related registers
    MM_REG_WRITE(command, VIDO_CTRL, (WROT_uv_ysel    << 30) +
                                     (WROT_uv_xsel    << 28) +
                                     (WROT_flipStatus << 24) +
                                     (WROT_rotation   << 20) +
                                     (WROT_alpha_rotation << 16) + //enable alpha
                                     (WROT_preultra   << 14) + //enable pre-ultra
                                     (WROT_crop_en    << 12) +
                                     (WROT_swap       <<  8) +
                                     (WROT_format     <<  0), 0xF131510F);
    command.addMetLog((m_identifier?"MDP_WROT1__VIDO_CTRL":"MDP_WROT0__VIDO_CTRL"),
                                     (WROT_uv_ysel    << 30) +
                                     (WROT_uv_xsel    << 28) +
                                     (WROT_flipStatus << 24) +
                                     (WROT_rotation   << 20) +
                                     (WROT_alpha_rotation << 16) + //enable alpha
                                     (WROT_preultra   << 14) + //enable pre-ultra
                                     (WROT_crop_en    << 12) +
                                     (WROT_swap       <<  8) +
                                     (WROT_format     <<  0));

#ifdef HW_SUPPORT_10BIT_PATH
    if (DP_COLOR_GET_10BIT_LOOSE(m_targetFormat))
    {
        WROT_scan_10bit = 1;
    }
    if (DP_COLOR_GET_10BIT_PACKED(m_targetFormat))
    {
        WROT_scan_10bit = 3;
        WROT_pending_zero = (1 << 26);
    }

    MM_REG_WRITE(command, VIDO_SCAN_10BIT, WROT_scan_10bit, 0x0000000F);
    MM_REG_WRITE(command, VIDO_PENDING_ZERO, WROT_pending_zero, 0x04000000);
#endif // HW_SUPPORT_10BIT_PATH

    // Write pre-ultra threshold
#if ESL_SETTING
    setESLWROT(command, m_identifier, config.outFormat);
#else
    MM_REG_WRITE(command, VIDO_DMA_PREULTRA, 0x0006005C, 0x00FFFFFF);
#endif

    // Write frame Y pitch
    MM_REG_WRITE(command, VIDO_STRIDE, m_targetPitchY, 0x0000FFFF);

    // Write frame UV pitch
    MM_REG_WRITE(command, VIDO_STRIDE_C, m_targetPitchUV, 0x0000FFFF);
    MM_REG_WRITE(command, VIDO_STRIDE_V, m_targetPitchUV, 0x0000FFFF);

#ifdef MDP_VERSION_8173
    // Write bust length
    WROT_max_burst_len = 7;
    MM_REG_WRITE(command, VIDO_DMA_PERF, WROT_max_burst_len << 24, 0x1F000000);
#endif

    // Write matrix control
    MM_REG_WRITE(command, VIDO_MAT_CTRL, (WROT_mat_sel << 4) +
                                         (WROT_ext_mat << 1) +
                                         (WROT_mat_en  << 0), 0x000000F3);

    // Set external matrix
    if (WROT_ext_mat)
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
        MM_REG_WRITE(command, VIDO_MAT_RMY, (TO_MAT_REG(mat.c00) <<  0) +
                                            (TO_MAT_REG(mat.c01) << 16), 0x1FFF1FFF);
        MM_REG_WRITE(command, VIDO_MAT_RMV, (TO_MAT_REG(mat.c02) <<  0) +
                                            (TO_MAT_REG(mat.c10) << 16), 0x1FFF1FFF);
        MM_REG_WRITE(command, VIDO_MAT_GMY, (TO_MAT_REG(mat.c11) <<  0) +
                                            (TO_MAT_REG(mat.c12) << 16), 0x1FFF1FFF);
        MM_REG_WRITE(command, VIDO_MAT_BMY, (TO_MAT_REG(mat.c20) <<  0) +
                                            (TO_MAT_REG(mat.c21) << 16), 0x1FFF1FFF);
        MM_REG_WRITE(command, VIDO_MAT_BMV, (TO_MAT_REG(mat.c22) <<  0), 0x00001FFF);

        // Set pre-add
        MM_REG_WRITE(command, VIDO_MAT_PREADD, (  0x0 <<  0) +             //    0
                                               (0x180 << 10) +             // -128
                                               (0x180 << 20), 0x1FF7FDFF); // -128

        // Set post-add
        MM_REG_WRITE(command, VIDO_MAT_POSTADD, (postAdd0 <<  0) +
                                                (128      << 10) +
                                                (128      << 20), 0x1FF7FDFF);
    }

    // Set the fixed ALPHA as 0xFF
    MM_REG_WRITE(command, VIDO_DITHER, 0xFF000000, 0xFF000000);

    // Set VIDO_EOL_SEL
    MM_REG_WRITE(command, VIDO_RSV_1, 0x80000000, 0x80000000);


    // Set VIDO_FIFO_TEST
    WROT_fifo_test = getWROTFIFOSetting(m_identifier);

#ifdef WROT_FIFO_CONSTRAINT
    if ((DP_COLOR_NV16 == m_targetFormat) ||
        (DP_COLOR_NV61 == m_targetFormat))
    {
        WROT_fifo_test = WROT_fifo_test >> 1;
    }
#endif //WROT_FIFO_CONSTRAINT

    if (WROT_fifo_test != 0)
    {
        MM_REG_WRITE(command, VIDO_FIFO_TEST, WROT_fifo_test, 0x00000FFF);
    }

#ifdef WROT_FILTER_CONSTRAINT
    if (0 == m_rotationAngle)
#endif
    {
        // Filter Enable
        MM_REG_WRITE(command, VIDO_MAIN_BUF_SIZE, (WROT_filt_v << 4) +
                                                  (WROT_filt_h << 0), 0x00000077);
    }
#if 0 //Dithering Enable
    MM_REG_WRITE(command, VIDO_DITHER_06, (0x1 << 19) + // new_bit_mode
        (0x0 << 12) + // rshift_b
        (0x0 << 8) + // rshift_g
        (0x0 << 4) + // rshift_r
        (0x1 << 0), 0x00087773); // table_en

    MM_REG_WRITE(command, VIDO_DITHER_07, (0x5 << 12) + // lsb_err_shift_r
        (0x0 << 8) + // ovflw_bit_r
        (0x5 << 4) + // add_lshift_r
        (0x0 << 0), 0x00007777); // input_rshift_r

    MM_REG_WRITE(command, VIDO_DITHER_08, (0x4 << 12) + // lsb_err_shift_g
        (0x0 << 8) + // ovflw_bit_g
        (0x4 << 4) + // add_lshift_g
        (0x0 << 0), 0x00007777); // input_rshift_g

    MM_REG_WRITE(command, VIDO_DITHER_09, (0x5 << 12) + // lsb_err_shift_b
        (0x0 << 8) + // ovflw_bit_b
        (0x5 << 4) + // add_lshift_b
        (0x0 << 0), 0x00007777); // input_rshift_b

    MM_REG_WRITE(command, VIDO_DITHER_02, (0x0 << 13) + // round_en
        (0x1 << 12) + // rdither_en
        (0x0 << 11) + // lfsr_en
        (0x0 << 10) + // edither_en
        (0x1 << 7) + // fphase_r
        (0x1 << 6), 0x00003CC0); // fphase_en

    MM_REG_WRITE(command, VIDO_DITHER_05, (0x1 << 14) + // lsb_off
        (0x0 << 0), 0x00004003); // fphase_ctrl

    MM_REG_WRITE(command, VIDO_DITHER_00, (0x0 << 1) + // out_sel
        (0x1 << 0), 0x00000003); // start

    MM_REG_WRITE(command, VIDO_DITHER, 0x1, 0x00000001);
#endif

    // turn off WROT dma dcm
    MM_REG_WRITE(command, VIDO_ROT_EN, (0x1 << 23) +
                                       (0x1 << 20), 0x00900000);

    command.addMetLog((m_identifier?"MDP_WROT1__VIDO_ROT_EN":"MDP_WROT0__VIDO_ROT_EN"), 0x01);
    return DP_STATUS_RETURN_SUCCESS;
}


DP_STATUS_ENUM DpEngine_WROT::onReconfigFrame(DpCommand &command, DpConfig &config)
{
    uint32_t       handle;
    uint32_t       secureRegAddr[3] = {(uint32_t)VIDO_BASE_ADDR, (uint32_t)VIDO_BASE_ADDR_C, (uint32_t)VIDO_BASE_ADDR_V};
    uint32_t       offset[3] = {0};
    uint32_t       planeOffset[3] = {0};
    int32_t        targetXStart;
    int32_t        targetYStart;
    int32_t        index = 0;

    targetXStart     = config.outXStart;
    targetYStart     = config.outYStart;

    DPLOGI("DpEngine_WROT: onReconfigFrame");
    // Note: WROT not support UV swap
    if ((1 == DP_COLOR_GET_SWAP_ENABLE(config.outFormat)) && (3 == DP_COLOR_GET_PLANE_COUNT(config.outFormat)))
    {
        uint32_t memAddrTmp = config.memAddr[1];
        config.memAddr[1] = config.memAddr[2];
        config.memAddr[2] = memAddrTmp;
    }

    handle = config.memAddr[0];
    planeOffset[0] = 0;
    planeOffset[1] = config.memAddr[1];
    planeOffset[2] = config.memAddr[2];

    if ((targetXStart != 0) ||
        (targetYStart != 0))
    {
        offset[0] = (targetXStart * m_bitsPerPixelY >> 3) + (m_targetPitchY * targetYStart);

        offset[1] = ((targetXStart >> m_horRightShiftUV) * m_bitsPerPixelUV >> 3) +
                     (targetYStart >> m_verRightShiftUV) * m_targetPitchUV;

        offset[2] = ((targetXStart >> m_horRightShiftUV) * m_bitsPerPixelUV >> 3) +
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

    // Setup source buffer end
    MM_REG_WRITE(command, VIDO_BASE_ADDR,   config.memAddr[0], 0xFFFFFFFF, NULL, m_frameConfigLabel[index++]);
    MM_REG_WRITE(command, VIDO_BASE_ADDR_C, config.memAddr[1], 0xFFFFFFFF, NULL, m_frameConfigLabel[index++]);
    MM_REG_WRITE(command, VIDO_BASE_ADDR_V, config.memAddr[2], 0xFFFFFFFF, NULL, m_frameConfigLabel[index++]);

    DPLOGI("DpEngine_WROT: output0 MVA 0x%08x\n", config.memAddr[0]);
    DPLOGI("DpEngine_WROT: output1 MVA 0x%08x\n", config.memAddr[1]);
    DPLOGI("DpEngine_WROT: output2 MVA 0x%08x\n", config.memAddr[2]);

    command.addMMpathLog((m_identifier?"out_2_width":"out_1_width"), config.outWidth, DP_MMPATH_OTHER);
    command.addMMpathLog((m_identifier?"out_2_height":"out_1_height"), config.outHeight, DP_MMPATH_OTHER);
    command.addMMpathLog((m_identifier?"hw=MDP,out_2":"hw=MDP,out_1"), config.outFormat, DP_MMPATH_FORMAT);
    command.addMMpathLog((m_identifier?"MDP_WROT1":"MDP_WROT0"), 1, DP_MMPATH_OTHER);
    command.addMMpathLog((m_identifier?"out_2":"out_1"), config.memAddr[0], DP_MMPATH_ADDR);

    return DP_STATUS_RETURN_SUCCESS;
}


DP_STATUS_ENUM DpEngine_WROT::onConfigTile(DpCommand &command)
{
    uint32_t WROT_y_ofst_adr;
    uint32_t WROT_c_ofst_adr;
    uint32_t WROT_v_ofst_adr;
    uint32_t WROT_in_xsize;
    uint32_t WROT_in_ysize;
    uint32_t WROT_tar_xsize;
    uint32_t WROT_tar_ysize;
    uint32_t WROT_crop_ofst_x;
    uint32_t WROT_crop_ofst_y;

    // Default values
    WROT_y_ofst_adr = 0;
    WROT_c_ofst_adr = 0;
    WROT_v_ofst_adr = 0;

    /*
     * Fill the the tile settings into the global command queue
     */
    if ((0 == m_rotationAngle) && (false == m_flipHorizontal))
    {
        // Target Y offset
        WROT_y_ofst_adr = m_outTileYTop * m_targetPitchY + (m_outTileXLeft * m_bitsPerPixelY >> 3);

        // Target U offset
        WROT_c_ofst_adr = (m_outTileYTop  >> m_verRightShiftUV) * m_targetPitchUV +
                          ((m_outTileXLeft >> m_horRightShiftUV) * m_bitsPerPixelUV >> 3);

        // Target V offset
        WROT_v_ofst_adr = (m_outTileYTop  >> m_verRightShiftUV) * m_targetPitchUV +
                          ((m_outTileXLeft >> m_horRightShiftUV) * m_bitsPerPixelUV >> 3);

        DPLOGI("DpEngine_WROT: No flip and no rotation: offset Y: 0x%08x, U: 0x%08x, V: 0x%08x\n", WROT_y_ofst_adr, WROT_c_ofst_adr, WROT_v_ofst_adr);
    }
    else if ((0 == m_rotationAngle) && (true == m_flipHorizontal))
    {
        // Target Y offset
        WROT_y_ofst_adr = m_outTileYTop * m_targetPitchY + ((m_outFrameWidth - m_outTileXLeft) * m_bitsPerPixelY >> 3) - 1;

        // Target U offset
        WROT_c_ofst_adr = (m_outTileYTop >> m_verRightShiftUV) * m_targetPitchUV +
                          (((m_outFrameWidth - m_outTileXLeft) >> m_horRightShiftUV) * m_bitsPerPixelUV >> 3) - 1;

        // Target V offset
        WROT_v_ofst_adr = (m_outTileYTop >> m_verRightShiftUV) * m_targetPitchUV +
                          (((m_outFrameWidth - m_outTileXLeft) >> m_horRightShiftUV) * m_bitsPerPixelUV >> 3) - 1;

        DPLOGI("DpEngine_WROT: Flip without rotation: offset Y: 0x%08x, U: 0x%08x, V: 0x%08x\n", WROT_y_ofst_adr, WROT_c_ofst_adr, WROT_v_ofst_adr);
    }
    else if ((90 == m_rotationAngle) && (false == m_flipHorizontal))
    {
        // Target Y offset
        WROT_y_ofst_adr = m_outTileXLeft * m_targetPitchY + ((m_outFrameHeight - m_outTileYTop) * m_bitsPerPixelY >> 3) - 1;

        // Target U offset
        WROT_c_ofst_adr = (m_outTileXLeft >> m_verRightShiftUV) * m_targetPitchUV +
                          (((m_outFrameHeight - m_outTileYTop) >> m_horRightShiftUV) * m_bitsPerPixelUV >> 3) - 1;

        // Target V offset
        WROT_v_ofst_adr = (m_outTileXLeft >> m_verRightShiftUV) * m_targetPitchUV +
                          (((m_outFrameHeight - m_outTileYTop) >> m_horRightShiftUV) * m_bitsPerPixelUV >> 3) - 1;

        DPLOGI("DpEngine_WROT: Rotate 90 degree only: offset Y: 0x%08x, U: 0x%08x, V: 0x%08x\n", WROT_y_ofst_adr, WROT_c_ofst_adr, WROT_v_ofst_adr);
    }
    else if ((90 == m_rotationAngle) && (true == m_flipHorizontal))
    {
        // Target Y offset
        WROT_y_ofst_adr = m_outTileXLeft * m_targetPitchY + (m_outTileYTop * m_bitsPerPixelY >> 3);

        // Target U offset
        WROT_c_ofst_adr = (m_outTileXLeft >> m_verRightShiftUV) * m_targetPitchUV +
                          ((m_outTileYTop >> m_horRightShiftUV) * m_bitsPerPixelUV >> 3);

        // Target V offset
        WROT_v_ofst_adr = (m_outTileXLeft >> m_verRightShiftUV) * m_targetPitchUV +
                          ((m_outTileYTop >> m_horRightShiftUV) * m_bitsPerPixelUV >> 3);

        DPLOGI("DpEngine_WROT: Flip and Rotate 90 degree: offset Y: 0x%08x, U: 0x%08x, V: 0x%08x\n", WROT_y_ofst_adr, WROT_c_ofst_adr, WROT_v_ofst_adr);
    }
    else if ((180 == m_rotationAngle) && (false == m_flipHorizontal))
    {
        // Target Y offset
        WROT_y_ofst_adr = (m_outFrameHeight - m_outTileYTop - 1) * m_targetPitchY +
                          ((m_outFrameWidth - m_outTileXLeft) * m_bitsPerPixelY >> 3) - 1;

        // Target U offset
        WROT_c_ofst_adr = ((m_outFrameHeight - m_outTileYTop - 1) >> m_verRightShiftUV) * m_targetPitchUV +
                          (((m_outFrameWidth - m_outTileXLeft) >> m_horRightShiftUV) * m_bitsPerPixelUV >> 3) - 1;

        // Target V offset
        WROT_v_ofst_adr = ((m_outFrameHeight - m_outTileYTop - 1) >> m_verRightShiftUV) * m_targetPitchUV +
                          (((m_outFrameWidth - m_outTileXLeft) >> m_horRightShiftUV) * m_bitsPerPixelUV >> 3) - 1;

        DPLOGI("DpEngine_WROT: Rotate 180 degree only: offset Y: 0x%08x, U: 0x%08x, V: 0x%08x\n", WROT_y_ofst_adr, WROT_c_ofst_adr, WROT_v_ofst_adr);
    }
    else if ((180 == m_rotationAngle) && (true == m_flipHorizontal))
    {
        // Target Y offset
        WROT_y_ofst_adr = (m_outFrameHeight - m_outTileYTop - 1) * m_targetPitchY + (m_outTileXLeft * m_bitsPerPixelY >> 3);

        // Target U offset
        WROT_c_ofst_adr = ((m_outFrameHeight - m_outTileYTop - 1) >> m_verRightShiftUV) * m_targetPitchUV +
                          ((m_outTileXLeft >> m_horRightShiftUV) * m_bitsPerPixelUV >> 3);

        // Target V offset
        WROT_v_ofst_adr = ((m_outFrameHeight - m_outTileYTop - 1) >> m_verRightShiftUV) * m_targetPitchUV +
                          ((m_outTileXLeft >> m_horRightShiftUV) * m_bitsPerPixelUV >> 3);

        DPLOGI("DpEngine_WROT: Flip and Rotate 180 degree: offset Y: 0x%08x, U: 0x%08x, V: 0x%08x\n", WROT_y_ofst_adr, WROT_c_ofst_adr, WROT_v_ofst_adr);
    }
    else if ((270 == m_rotationAngle) && (false == m_flipHorizontal))
    {
        // Target Y offset
        WROT_y_ofst_adr = (m_outFrameWidth - m_outTileXLeft - 1) * m_targetPitchY + (m_outTileYTop * m_bitsPerPixelY >> 3);

        // Target U offset
        WROT_c_ofst_adr = ((m_outFrameWidth - m_outTileXLeft - 1) >> m_verRightShiftUV) * m_targetPitchUV +
                          ((m_outTileYTop >> m_horRightShiftUV) * m_bitsPerPixelUV >> 3);

        // Target V offset
        WROT_v_ofst_adr = ((m_outFrameWidth - m_outTileXLeft - 1) >> m_verRightShiftUV) * m_targetPitchUV +
                          ((m_outTileYTop >> m_horRightShiftUV) * m_bitsPerPixelUV >> 3);

        DPLOGI("DpEngine_WROT: Rotate 270 degree only: offset Y: 0x%08x, U: 0x%08x, V: 0x%08x\n", WROT_y_ofst_adr, WROT_c_ofst_adr, WROT_v_ofst_adr);
    }
    else if ((270 == m_rotationAngle) && (true == m_flipHorizontal))
    {
        // Target Y offset
        WROT_y_ofst_adr = (m_outFrameWidth - m_outTileXLeft - 1) * m_targetPitchY +
                          ((m_outFrameHeight - m_outTileYTop) * m_bitsPerPixelY >> 3) - 1;

        // Target U offset
        WROT_c_ofst_adr = ((m_outFrameWidth - m_outTileXLeft - 1) >> m_verRightShiftUV) * m_targetPitchUV +
                          (((m_outFrameHeight - m_outTileYTop) >> m_horRightShiftUV) * m_bitsPerPixelUV >> 3) - 1;

        // Target V offset
        WROT_v_ofst_adr = ((m_outFrameWidth - m_outTileXLeft - 1) >> m_verRightShiftUV) * m_targetPitchUV +
                          (((m_outFrameHeight - m_outTileYTop) >> m_horRightShiftUV) * m_bitsPerPixelUV >> 3) - 1;

        DPLOGI("DpEngine_WROT: Flip and Rotate 270 degree: offset Y: 0x%08x, U: 0x%08x, V: 0x%08x\n", WROT_y_ofst_adr, WROT_c_ofst_adr, WROT_v_ofst_adr);
    }

    // Write Y pixel offset
    MM_REG_WRITE(command, VIDO_OFST_ADDR, WROT_y_ofst_adr, 0x0FFFFFFF);

    // Write U pixel offset
    MM_REG_WRITE(command, VIDO_OFST_ADDR_C, WROT_c_ofst_adr, 0x0FFFFFFF);

    // Write V pixel offset
    MM_REG_WRITE(command, VIDO_OFST_ADDR_V, WROT_v_ofst_adr, 0x0FFFFFFF);

    // Write source size
    WROT_in_xsize    = m_inTileXRight   - m_inTileXLeft + 1;
    WROT_in_ysize    = m_inTileYBottom  - m_inTileYTop  + 1;
    MM_REG_WRITE(command, VIDO_IN_SIZE, (WROT_in_ysize << 16) +
                                        (WROT_in_xsize <<  0), 0x1FFF1FFF);

    // Write target size
    WROT_tar_xsize   = m_outTileXRight  - m_outTileXLeft + 1;
    WROT_tar_ysize   = m_outTileYBottom - m_outTileYTop + 1;
    MM_REG_WRITE(command, VIDO_TAR_SIZE, (WROT_tar_ysize << 16) +
                                         (WROT_tar_xsize <<  0), 0x1FFF1FFF);

    WROT_crop_ofst_x = m_lumaXOffset;
    WROT_crop_ofst_y = m_lumaYOffset;
    MM_REG_WRITE(command, VIDO_CROP_OFST, (WROT_crop_ofst_y << 16) +
                                          (WROT_crop_ofst_x <<  0), 0x1FFF1FFF);

    // set max internal buffer for tile usage, and check for internal buffer size
    WrotInput inParam;
    WrotOutput outParam;
    memset(&inParam, 0, sizeof(WrotInput));
    memset(&outParam, 0, sizeof(WrotOutput));

    inParam.identifier = m_identifier;
    inParam.colorFormat = m_targetFormat;
    inParam.rotationAngle = m_rotationAngle;
    inParam.WROT_tar_xsize = WROT_tar_xsize;
    inParam.FIFOMaxSize = m_FIFOMaxSize;
    inParam.maxLineCount = m_maxLineCount;

    calWROTSetting(&inParam, &outParam);

    MM_REG_WRITE(command, VIDO_MAIN_BUF_SIZE, (outParam.WROT_main_blk_width    << 16) +
                                              (outParam.WROT_main_buf_line_num <<  8), 0x1FFF7F00);

    // Enable engine
    MM_REG_WRITE(command, VIDO_ROT_EN, 0x01, 0x00000001);

    DPLOGI("DpEngine_WROT: min block width: %d\n", outParam.WROT_main_blk_width);
    DPLOGI("DpEngine_WROT: min buf line num: %d\n", outParam.WROT_main_buf_line_num);

    return DP_STATUS_RETURN_SUCCESS;
}


DP_STATUS_ENUM DpEngine_WROT::onWaitEvent(DpCommand &command)
{
    if (0 == m_identifier)
    {
        MM_REG_WAIT(command, DpCommand::WROT0_FRAME_DONE);
    }
    else
    {
        MM_REG_WAIT(command, DpCommand::WROT1_FRAME_DONE);
    }
#ifdef WROT_FILTER_CONSTRAINT
    // Filter Disable
    MM_REG_WRITE(command, VIDO_MAIN_BUF_SIZE, (0 << 4) +
                                              (0 << 0), 0x00000077);
#endif
    // Disable engine
    MM_REG_WRITE(command, VIDO_ROT_EN, 0x00, 0x00000001);

    return DP_STATUS_RETURN_SUCCESS;
}

DP_STATUS_ENUM DpEngine_WROT::onUpdatePMQOS(DpCommand &command, uint32_t& totalPixel, uint32_t& totalDataSzie, int32_t& engineType, struct timeval& endtime)
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

    DPLOGI("DpEngine_WROT::updatePMQOS totalDataSzie = %d totalPixel = %d\n", totalDataSzie, totalPixel);

    return DP_STATUS_RETURN_SUCCESS;
}
