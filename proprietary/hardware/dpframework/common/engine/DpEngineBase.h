/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2012. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
 * AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek Software")
 * have been modified by MediaTek Inc. All revisions are subject to any receiver's
 * applicable license agreements with MediaTek Inc.
 */

#ifndef __DP_ENGINE_BASE_H__
#define __DP_ENGINE_BASE_H__

#include "DpDataType.h"
#include "DpEngineType.h"
#include "DpFRegistry.h"
#include "DpPortAdapt.h"
#include "DpCommand.h"
#include "DpLogger.h"
#include "DpProfiler.h"
#include "DpProperty.h"
#include "DpDriver.h"

#define DP_ENGINE_MAX_BRANCH_COUNT      5

#define DP_MAX_PABUFFER_COUNT           MAX_NUM_READBACK_REGS
#define DP_MAX_SINGLE_PABUFFER_COUNT    4
#if defined(SUPPORT_DRE) || defined(SUPPORT_HDR)
#define DP_MAX_FRAME_CONFIG_LABEL_COUNT 2048
#else
#define DP_MAX_FRAME_CONFIG_LABEL_COUNT 64
#endif

enum STREAM_TYPE_ENUM
{
    STREAM_UNKNOWN,

    STREAM_BITBLT,
    STREAM_GPU_BITBLT,
    STREAM_DUAL_BITBLT,
    STREAM_2ND_BITBLT,
    STREAM_COLOR_BITBLT,

    STREAM_FRAG,
    STREAM_FRAG_JPEGDEC,

    STREAM_ISP_IC,
    STREAM_ISP_VR,
    STREAM_ISP_ZSD,
    STREAM_ISP_IP,
    STREAM_ISP_VSS,
    STREAM_ISP_ZSD_SLOW,
    //STREAM_ISP_ZSD_ONE

    STREAM_WPE,
    STREAM_WPE2
};

#define TILE_ORDER_Y_FIRST          (0x1)
#define TILE_ORDER_RIGHT_TO_LEFT    (0x2)
#define TILE_ORDER_BOTTOM_TO_TOP    (0x4)

struct DpConfig
{
    STREAM_TYPE_ENUM    scenario;

    uint32_t            memAddr[3];   // for memory port
    uint32_t            memSize[3];   // for memory port

    // Input frame info
    DpColorFormat       inFormat;
    int32_t             inWidth;
    int32_t             inHeight;
    int32_t             inYPitch;
    int32_t             inUVPitch;
    DP_PROFILE_ENUM     inProfile;

    int32_t             rszHorTable;
    int32_t             rszVerTable;

    // Output frame info
    DpColorFormat       outFormat;
    int32_t             outWidth;
    int32_t             outHeight;
    int32_t             outYPitch;
    int32_t             outUVPitch;
    int32_t             outXStart;
    int32_t             outYStart;
    int32_t             outRoiWidth;
    int32_t             outRoiHeight;
    DP_PROFILE_ENUM     outProfile;

    // Source crop
    int32_t             inXOffset;
    int32_t             inXSubpixel;
    int32_t             inYOffset;
    int32_t             inYSubpixel;
    int32_t             inCropWidth;
    int32_t             inCropWidthSubpixel;
    int32_t             inCropHeight;
    int32_t             inCropHeightSubpixel;

    // Target crop
    int32_t             pipeID;
    int32_t             outXOffset;
    int32_t             outCropWidth;
    bool                enWROTCrop;

    // MDP features
    bool                enISP;
    bool                enRDMACrop;
    bool                enRing;
    bool                enUFODec;
    bool                enBottomField;
    bool                enAlphaRot;
    uint32_t            memUFOLenAddr[2];   // for memory port

    DpJPEGEnc_Config    jpegEnc_cfg;
    DpVEnc_Config*      pVEnc_cfg;
#if CONFIG_FOR_VERIFY_FPGA
    void*               pEngine_cfg;
#endif

    // Rotation angle
    int32_t             rotation;

    // Flip status
    bool                enFlip;

    // Color transform
    DpColorMatrix       *pColorMatrix;

    // PQ id
    uint64_t            pqSessionId;

    // PQ parameters
    uint32_t            parTDSHP;

    // Dithering
    bool                enDither;

    // Tile processing
    uint32_t            inCalOrder;
    uint32_t            outCalOrder;

    uint32_t            inStreamOrder;
    uint32_t            outDumpOrder;

    uint32_t            rootAndLeaf;
};


class DpEngineBase
{
public:
    enum DpEngineFeature
    {
        // ISP features
        eISP            = 0x01000000,

        // MDP features
        eRMEM           = 0x00000001,
        eRING           = 0x00000002,
        eSCL            = 0x00000004,
        eTDSHP          = 0x00000008,
        eROT            = 0x00000010,
        eFLIP           = 0x00000020,
        eWMEM           = 0x00000040,
        eDITHER         = 0x00000080,
        eJPEG           = 0x00000100,
        eMDPSOUT        = 0x00000200,
        eVENC           = 0x00000400,
        eCOLOR          = 0x00000800,
        eCOLOR_EX       = 0x00001000,
        eALPHA_ROT      = 0x00002000,
        eAAL            = 0x00004000,
        eCCORR          = 0x00008000,
        eHDR            = 0x00010000,
        eTCC            = 0x00020000,
        eFG             = 0x00040000,
    };

    enum FLUSH_TYPE_ENUM
    {
        FLUSH_BEFORE_HW_READ,
        FLUSH_AFTER_HW_WRITE
    };

    static DpEngineBase* Factory(DpEngineType type);

    virtual ~DpEngineBase()
    {
    }

    DpEngineType getEngineType()
    {
        return m_engineType;
    }

    virtual int64_t getEngineFlag()
    {
        return 1LL << getEngineType();
    }

    DpEngineType getParentType()
    {
        return onGetParentType();
    }

    const char* getEngineName()
    {
        return onGetEngineName();
    }

    DP_STATUS_ENUM bindInPort(DpPortAdapt *port)
    {
        m_pInDataPort = port;

        return onBindInPort(port);
    }

    DP_STATUS_ENUM bindOutPort(DpPortAdapt *port);

    DP_STATUS_ENUM pollPort(bool waitPort)
    {
        return onPollPort(waitPort);
    }

    DP_STATUS_ENUM abortPollPort()
    {
        return onAbortPollPort();
    }

    DP_STATUS_ENUM flushBuffer(FLUSH_TYPE_ENUM type)
    {
        return onFlushBuffer(type);
    }

    DP_STATUS_ENUM flipBuffer()
    {
        return onFlipBuffer();
    }

    DP_STATUS_ENUM initEngine(DpCommand &command)
    {
        DP_TRACE_CALL();
        DP_STATUS_ENUM status;

        status = onInitEngine(command);
        //m_engineState = eInit;

        return status;
    }

    DP_STATUS_ENUM configFrame(DpCommand &command,
                               DpConfig  &config);

    DP_STATUS_ENUM reconfigFrame(DpCommand &command,
                                 DpConfig  &config);

    DP_STATUS_ENUM waitEngine(DpCommand &command)
    {
        return onWaitEngine(command);
    }

    DP_STATUS_ENUM waitEvent(DpCommand &command)
    {
        return onWaitEvent(command);
    }

    DP_STATUS_ENUM deInitEngine(DpCommand &command)
    {
        DP_STATUS_ENUM status;

        status = onDeInitEngine(command);
        //m_engineState = eUnknown;

        return status;
    }

    DP_STATUS_ENUM dumpBuffer()
    {
        DPLOGI("DpEngineBase: vendor.dp.dumpbuffer.enable %d\n", DpDriver::getInstance()->getEnableDumpBuffer());

        if (0 != DpDriver::getInstance()->getEnableDumpBuffer())
        {
            return onDumpBuffer();
        }

        return DP_STATUS_RETURN_SUCCESS;
    }

    DP_STATUS_ENUM dumpDebugInfo()
    {
        return onDumpDebugInfo();
    }

    int64_t queryFeature()
    {
        if (0 == m_featureSet)
        {
            m_featureSet = onQueryFeature();
        }
        return m_featureSet;
    }

    virtual bool isOutputDisable()
    {
        return m_outputDisable;
    }

    DP_STATUS_ENUM waitFence()
    {
        return onWaitFence();
    }

protected:
    const uint32_t  m_identifier;

    bool         m_outputDisable;
    DpEngineType m_engineType;
    DpPortAdapt  *m_pInDataPort;
    DpPortAdapt  *m_pOutDataPort[DP_ENGINE_MAX_BRANCH_COUNT];
    int32_t      m_outPortCount;
    int32_t      m_inFrameWidth;
    int32_t      m_inFrameHeight;
    int32_t      m_outFrameWidth;
    int32_t      m_outFrameHeight;
    int64_t      m_featureSet;
    int32_t      m_frameConfigLabel[DP_MAX_FRAME_CONFIG_LABEL_COUNT];

    DpEngineBase(uint32_t identifier)
        : m_identifier(identifier),
          m_outputDisable(false),
          m_engineType(tNone),
          m_pInDataPort(0),
          m_outPortCount(0),
          m_inFrameWidth(0),
          m_inFrameHeight(0),
          m_outFrameWidth(0),
          m_outFrameHeight(0),
          m_featureSet(0)
    {
        memset(m_pOutDataPort, 0x0, sizeof(m_pOutDataPort));
        memset(m_frameConfigLabel, -1, sizeof(m_frameConfigLabel));
    }

    void setEngineType(DpEngineType type)
    {
        m_engineType = type;
    }

    virtual DpEngineType onGetParentType()
    {
        return tNone;
    }

    virtual const char* onGetEngineName();

    virtual DP_STATUS_ENUM onBindInPort(DpPortAdapt*)
    {
        return DP_STATUS_RETURN_SUCCESS;
    }

    virtual DP_STATUS_ENUM onBindOutPort(DpPortAdapt*)
    {
        return DP_STATUS_RETURN_SUCCESS;
    }

    virtual DP_STATUS_ENUM onPollPort(bool waitPort);

    virtual DP_STATUS_ENUM onAbortPollPort();

    virtual DP_STATUS_ENUM onFlushBuffer(FLUSH_TYPE_ENUM type);

    virtual DP_STATUS_ENUM onFlipBuffer();

    virtual DP_STATUS_ENUM onInitEngine(DpCommand&)
    {
        return DP_STATUS_RETURN_SUCCESS;
    }

#if CONFIG_FOR_VERIFY_FPGA
    #define inline // into void
#endif

    inline DP_STATUS_ENUM configFrameISP(DpConfig &config);

    inline DP_STATUS_ENUM configFrameRotate(DpConfig &config);

    inline DP_STATUS_ENUM configFrameFlip(DpConfig &config);

    inline DP_STATUS_ENUM configFrameAlphaRot(DpConfig &config);

    inline DP_STATUS_ENUM configFrameRead(DpCommand &command, DpConfig &config);

    inline DP_STATUS_ENUM configFrameRingBuf(DpConfig &config);

    inline DP_STATUS_ENUM configFrameAal(DpConfig &config);

    inline DP_STATUS_ENUM configFrameCcorr(DpConfig &config);

    inline DP_STATUS_ENUM configFrameHdr(DpConfig &config);

    inline DP_STATUS_ENUM configFrameScale(DpConfig &config);

    inline DP_STATUS_ENUM configFrameSharpness(DpConfig &config);

    inline DP_STATUS_ENUM configFrameColor(DpConfig &config);

    inline DP_STATUS_ENUM configFrameDither(DpConfig &config);

    inline DP_STATUS_ENUM configFrameMout(DpConfig &config);

    inline DP_STATUS_ENUM configFrameWrite(DpCommand &command, DpConfig &config);

    inline DP_STATUS_ENUM configFrameJPEG(DpConfig &config);

    inline DP_STATUS_ENUM configFrameVEnc(DpConfig &config);

#if CONFIG_FOR_VERIFY_FPGA
    #undef inline
#endif

    virtual DP_STATUS_ENUM onConfigFrame(DpCommand&,
                                         DpConfig&)
    {
        return DP_STATUS_RETURN_SUCCESS;
    }

    virtual DP_STATUS_ENUM onReconfigFrame(DpCommand&,
                                           DpConfig&)
    {
        return DP_STATUS_RETURN_SUCCESS;
    }

    virtual DP_STATUS_ENUM onWaitEngine(DpCommand&)
    {
        return DP_STATUS_RETURN_SUCCESS;
    }

    virtual DP_STATUS_ENUM onWaitEvent(DpCommand&)
    {
        return DP_STATUS_RETURN_SUCCESS;
    }

    virtual DP_STATUS_ENUM onDeInitEngine(DpCommand&)
    {
        return DP_STATUS_RETURN_SUCCESS;
    }

    virtual DP_STATUS_ENUM onDumpBuffer();

    virtual DP_STATUS_ENUM onDumpDebugInfo();

    virtual DP_STATUS_ENUM onSetTileOrder(uint32_t inCalOrder    __unused,
                                          uint32_t outCalOrder   __unused,
                                          uint32_t inStreamOrder __unused,
                                          uint32_t outDumpOrder  __unused)
    {
        return DP_STATUS_RETURN_SUCCESS;
    }

    virtual int64_t onQueryFeature()
    {
        return 0;
    }

    virtual DP_STATUS_ENUM onWaitFence();
};


template <typename T>
class DpEngineNode
{
public:
    DpEngineNode()
        : m_pPrevEngine(NULL),
          m_branchCount(0)
    {
        memset(m_pNextEngine, 0x0, sizeof(m_pNextEngine));
    }

    virtual ~DpEngineNode()
    {
    }

    DP_STATUS_ENUM setPrevEngine(T *pPrev)
    {
        if (NULL != m_pPrevEngine)
        {
            assert(0);
            return DP_STATUS_INVALID_STATE;
        }

        m_pPrevEngine = pPrev;

        return DP_STATUS_RETURN_SUCCESS;
    }

    T* getPrevEngine()
    {
        return m_pPrevEngine;
    }

    DpEngineType getPrevType()
    {
        if (NULL == m_pPrevEngine)
        {
            return tNone;
        }
        else
        {
            return m_pPrevEngine->getEngineType();
        }
    }

    DP_STATUS_ENUM addNextEngine(T *pNext)
    {
        if (m_branchCount >= DP_ENGINE_MAX_BRANCH_COUNT)
        {
            assert(0);
            return DP_STATUS_OVER_MAX_BRANCH;
        }

        m_pNextEngine[m_branchCount] = pNext;
        m_branchCount++;

        return DP_STATUS_RETURN_SUCCESS;
    }

    int32_t getNextCount()
    {
        return m_branchCount;
    }

    T* getNextEngine(int32_t index)
    {
        if (index >= m_branchCount)
        {
            return NULL;
        }
        else
        {
            return m_pNextEngine[index];
        }
    }

    bool isLeafEngine()
    {
        return (0 == m_branchCount)? true: false;
    }

protected:
    T       *m_pPrevEngine;
    int32_t m_branchCount;
    T       *m_pNextEngine[DP_ENGINE_MAX_BRANCH_COUNT];
};

typedef DpFRegistry<DpEngineBase*, DpEngineType> EngineReg;

#endif  // __DP_ENGINE_BASE_H__
