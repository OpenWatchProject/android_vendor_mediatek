#include "DpTileEngine.h"
#include "DpEngineType.h"
#include "DpThread.h"
#include "tile_driver.h"
#include "tile_param.h"
#include "mdp_reg_wdma.h"

/* Venc Modify + */
#if defined(MTK_SLOW_MOTION_HEVC_SUPPORT)
#include "val_types_public.h"
#include "venc_drv_if_public.h"
#include <cutils/properties.h>
#include "venc_drv_hevc_if.h"
#elif defined(MTK_SLOW_MOTION_H264_SUPPORT)
#include "val_types_public.h"
#include "venc_drv_if_public.h"
#include <cutils/properties.h>
#include "venc_drv_h264_if.h"
#endif // MTK_SLOW_MOTION_HEVC_SUPPORT
/* Venc Modify - */

//--------------------------------------------------------
// VENC driver engine
//--------------------------------------------------------
class DpEngine_VENC: public DpTileEngine, private DpMultiThread
{
public:
    DpEngine_VENC()
        : DpTileEngine(0),
          m_pVEncConfig(NULL),
          m_pVEncCommander(NULL),
          m_configFrameCount(0),
          m_tileXEndFlag(false),
          m_tileYEndFlag(false)
    {
        memset(&m_config, 0x0, sizeof(m_config));
    }

    ~DpEngine_VENC()
    {
    }

private:
    DP_STATUS_ENUM onConfigFrame(DpCommand&, DpConfig&);
    DP_STATUS_ENUM onReconfigTiles(DpCommand &command);

    virtual DP_STATUS_ENUM threadLoop();

    /* Venc Modify + */
#if defined(MTK_SLOW_MOTION_HEVC_SUPPORT)
    DP_STATUS_ENUM onConfigFrame_HEVC(DpCommand&, DpConfig&);
#elif defined(MTK_SLOW_MOTION_H264_SUPPORT)
    DP_STATUS_ENUM onConfigFrame_H264(DpCommand&, DpConfig&);
#endif // MTK_SLOW_MOTION_HEVC_SUPPORT
    /* Venc Modify - */

    DP_STATUS_ENUM onInitTileCalc(struct TILE_PARAM_STRUCT*);

    DP_STATUS_ENUM onRetrieveTileParam(struct TILE_PARAM_STRUCT*);

    DP_STATUS_ENUM onWaitEvent(DpCommand&);

    int64_t onQueryFeature()
    {
        return (eVENC|eWMEM);
    }

    DpConfig      m_config;
    DpVEnc_Config *m_pVEncConfig;
    DpCommand     *m_pVEncCommander;
    uint64_t      m_configFrameCount;

    bool          m_tileXEndFlag;
    bool          m_tileYEndFlag;
};


// register factory function
static DpEngineBase* VENCFactory(DpEngineType type)
{
    if (tVENC == type)
    {
        return new DpEngine_VENC();
    }
    return NULL;
};

// register factory function
EngineReg VENCReg(VENCFactory);

void VENCPostProc(DpVEnc_Config* pVEnc, uint32_t numReadBack, uint32_t* pReadBack)
{
#if defined(MTK_SLOW_MOTION_HEVC_SUPPORT)
    DP_UNUSED(numReadBack);
    uint32_t dequeueFrameCount;
    VENC_DRV_HEVC_POST_PROC_INFO_T rPostProcInfo;
    memset(&rPostProcInfo,0,sizeof(rPostProcInfo));

    dequeueFrameCount = *(pVEnc->pDequeueFrameCount);
    DPLOGI("DequeueFramecount: %d\n",dequeueFrameCount);

    if(dequeueFrameCount < 1) //First frame only 3 header
    {
        // for header used
        rPostProcInfo.u4SPSSize = pReadBack[0];
        rPostProcInfo.u4VPSSize = pReadBack[1];
        rPostProcInfo.u4PPSSize = pReadBack[2];
    }else
    {
        rPostProcInfo.u4BSSize = pReadBack[0];
    }

    // for encoded frame used
    rPostProcInfo.fgBSStatus = VAL_TRUE;

    /* [ToDo] Need to get from dpframework - */

    /* [ToDo] Need to move to dpframework api + */

    // for encoded bs with post processing
    if (VENC_DRV_MRESULT_FAIL == eVEncDrvSetParam(pVEnc->rVencDrvHandle, VENC_DRV_SET_TYPE_SLOW_MOTION_POST_PROC, &rPostProcInfo, VAL_NULL))
    {
        DPLOGE("[onConfigFrame_HEVC][ERROR] cannot set param VENC_DRV_SET_TYPE_SLOW_MOTION_POST_PROC\n");
    }
#elif defined(MTK_SLOW_MOTION_H264_SUPPORT)
    uint32_t dequeueFrameCount;
    VENC_DRV_H264_POST_PROC_INFO_T rPostProcInfo;
    memset(&rPostProcInfo,0,sizeof(rPostProcInfo));

    dequeueFrameCount = *(pVEnc->pDequeueFrameCount);
    DPLOGI("DequeueFramecount: %d\n",dequeueFrameCount);

    if(dequeueFrameCount < 1) //First frame only 3 header
    {
        // for header used
        DPLOGD("rPostProcInfo: numReadBack: %d, %d, %d, %d, %d\n", numReadBack, pReadBack[0], pReadBack[1] , pReadBack[2], pReadBack[3]);
        rPostProcInfo.u4SPSSize = pReadBack[0];
        rPostProcInfo.u4PPSSize = pReadBack[1];
    }else
    {
        rPostProcInfo.u4BSSize = pReadBack[0];
    }

    // for encoded frame used
    rPostProcInfo.fgBSStatus = VAL_TRUE;

    /* [ToDo] Need to get from dpframework - */

    /* [ToDo] Need to move to dpframework api + */

    // for encoded bs with post processing
    if (VENC_DRV_MRESULT_FAIL == eVEncDrvSetParam(pVEnc->rVencDrvHandle, VENC_DRV_SET_TYPE_SLOW_MOTION_POST_PROC, &rPostProcInfo, VAL_NULL))
    {
        DPLOGE("[onConfigFrame_H264][ERROR] cannot set param VENC_DRV_SET_TYPE_SLOW_MOTION_POST_PROC\n");
    }
#else
    DP_UNUSED(pVEnc);
    DP_UNUSED(numReadBack);
    DP_UNUSED(pReadBack);
#endif // MTK_SLOW_MOTION_HEVC_SUPPORT
    /* [ToDo] Need to move to dpframework api - */
}

/* Venc Modify + */
#if defined(MTK_SLOW_MOTION_HEVC_SUPPORT)
DP_STATUS_ENUM DpEngine_VENC::onConfigFrame_HEVC(DpCommand &command, DpConfig &config)
{
    uint32_t offset = 0;
    uint32_t diff = 0;

    VAL_CHAR_T  icvalue[PROPERTY_VALUE_MAX];
    VAL_BOOL_T  fgForceIFrame;
    VAL_UINT32_T _current_tid = gettid();
    VENC_DRV_HEVC_BUF_T rBuf;
    VENC_DRV_HEVC_CMDQ_T rCmdQ;
    VAL_UINT32_T u4I = 0;

    /* [ToDo] Need to get from dpframework + */
    rBuf.rFrmBuf.rFrmBufAddr.u4VA = (unsigned long)m_pVEncConfig->memYUVSWAddr[0];
    rBuf.rFrmBuf.rFrmBufAddr.u4PA = m_pVEncConfig->memYUVMVAAddr[0];
    rBuf.rFrmBuf.rFrmBufAddr.u4Size = m_pVEncConfig->memYUVMVASize[0] +
                                      m_pVEncConfig->memYUVMVASize[1] +
                                      m_pVEncConfig->memYUVMVASize[2];

    rBuf.rBSBuf.rBSAddr.u4VA = (unsigned long)m_pVEncConfig->memOutputSWAddr[0];
    rBuf.rBSBuf.rBSAddr.u4PA = config.memAddr[0];
    rBuf.rBSBuf.rBSAddr.u4Size = config.memSize[0] +
                                 config.memSize[1] +
                                 config.memSize[2] ;

    rBuf.rBSBuf.u4BSStartVA = rBuf.rBSBuf.rBSAddr.u4VA;
    rBuf.rBSBuf.u4BSSize = 0;

    DPLOGI("rBuf.rFrmBuf.rFrmBufAddr.u4VA: %x\n",rBuf.rFrmBuf.rFrmBufAddr.u4VA);
    DPLOGI("rBuf.rFrmBuf.rFrmBufAddr.u4PA: %x\n",rBuf.rFrmBuf.rFrmBufAddr.u4PA);
    DPLOGI("rBuf.rFrmBuf.rFrmBufAddr.u4Size: %x\n",rBuf.rFrmBuf.rFrmBufAddr.u4Size);

    DPLOGI("rBuf.rBSBuf.rBSAddr.u4VA: %x\n",rBuf.rBSBuf.rBSAddr.u4VA);
    DPLOGI("rBuf.rBSBuf.rBSAddr.u4PA: %x\n",rBuf.rBSBuf.rBSAddr.u4PA);
    DPLOGI("rBuf.rBSBuf.rBSAddr.u4Size: %x\n",rBuf.rBSBuf.rBSAddr.u4Size);

    rCmdQ.u4CmdCnt = 0;

    // check force I frame
    property_get("vendor.dl.vr.force.iframe", icvalue, "0");
    fgForceIFrame = (VAL_BOOL_T) atoi(icvalue);
    DPLOGD("[tid: %d][onConfigFrame_HEVC] ForceIFrame = %d\n", _current_tid, fgForceIFrame);

    if (fgForceIFrame == VAL_TRUE)
    {
        if (VENC_DRV_MRESULT_FAIL == eVEncDrvSetParam(m_pVEncConfig->rVencDrvHandle, VENC_DRV_SET_TYPE_FORCE_INTRA_ON, &rBuf, &rCmdQ))
        {
            DPLOGE("[tid: %d][onConfigFrame_HEVC][ERROR] cannot set param VENC_DRV_SET_TYPE_FORCE_INTRA_ON\n", _current_tid);
            goto onConfigFrame_HEVC_Deinit_Release_Driver_ERROR;
        }

        property_set("vendor.dl.vr.force.iframe", "0");

        if (VENC_DRV_MRESULT_FAIL == eVEncDrvSetParam(m_pVEncConfig->rVencDrvHandle, VENC_DRV_SET_TYPE_FORCE_INTRA_OFF, &rBuf, &rCmdQ))
        {
            DPLOGE("[tid: %d][onConfigFrame_HEVC][ERROR] cannot set param VENC_DRV_SET_TYPE_FORCE_INTRA_OFF\n", _current_tid);
            goto onConfigFrame_HEVC_Deinit_Release_Driver_ERROR;
        }
    }

    // encode sequence header or frame
    if (VENC_DRV_MRESULT_FAIL == eVEncDrvSetParam(m_pVEncConfig->rVencDrvHandle, VENC_DRV_SET_TYPE_SLOW_MOTION_ENCODE, &rBuf, &rCmdQ))
    {
        DPLOGE("[tid: %d][onConfigFrame_HEVC][ERROR] cannot set param VENC_DRV_SET_TYPE_SLOW_MOTION_ENCODE\n", _current_tid);
        goto onConfigFrame_HEVC_Deinit_Release_Driver_ERROR;
    }

    *(m_pVEncConfig->pNumPABuffer) = 0;
    offset = (m_configFrameCount & 0xF) << 2;

    (m_pVEncConfig->pFrameMutex)->lock();
    diff = *(m_pVEncConfig->pConfigFrameCount) - *(m_pVEncConfig->pDequeueFrameCount);
    (m_pVEncConfig->pFrameMutex)->unlock();

    if (diff > 0xF)
    {
        DPLOGW("DpEngine_VENC: PA buffer overrun, m_configFrameCount - m_dequeueFrameCount: %d\n",diff);
    }

    for (u4I = 0; u4I < rCmdQ.u4CmdCnt; u4I++)
    {
        switch (rCmdQ.eCmd[u4I])
        {
            case HEVC_CMDQ_CMD_READ:
            {
                /* [ToDo] Need to move to dpframework api */
                /* add more addr condition check for different platform*/
                MM_REG_READ_BEGIN(command);
                if((0x17002098 == rCmdQ.u4Addr[u4I]) || (0x17020098 == rCmdQ.u4Addr[u4I]))
                {
                    DPLOGI("PABuffer[%d] : 0x%x\n", (*(m_pVEncConfig->pNumPABuffer))+offset, m_pVEncConfig->pPABuffer[(*m_pVEncConfig->pNumPABuffer)+offset]);
                    MM_REG_READ(command, rCmdQ.u4Addr[u4I], (uint32_t)m_pVEncConfig->pPABuffer[(*m_pVEncConfig->pNumPABuffer)+offset]);
                    (*m_pVEncConfig->pNumPABuffer)++;
                }
                else
                {
                    MM_REG_READ(command, rCmdQ.u4Addr[u4I],(uint32_t)m_pVEncConfig->pPABuffer[DP_MAX_PABUFFER_COUNT - 1]);
                }
                MM_REG_READ_END(command);
                DPLOGI("[tid: %d][onConfigFrame_HEVC] Read (0x%x)\n", _current_tid, (uint32_t)rCmdQ.u4Addr[u4I]);
                break;
            }

            case HEVC_CMDQ_CMD_WRITE:
            {
                MM_REG_WRITE_MASK(command, rCmdQ.u4Addr[u4I], rCmdQ.u4Data[u4I], rCmdQ.u4Mask[u4I]);
                DPLOGI("[tid: %d][onConfigFrame_HEVC] Write (0x%x, 0x%x, 0x%x)\n",
                    _current_tid, rCmdQ.u4Addr[u4I], rCmdQ.u4Data[u4I], rCmdQ.u4Mask[u4I]);
                break;
            }

            case HEVC_CMDQ_CMD_POLL:
            {
                MM_REG_POLL_MASK(command, rCmdQ.u4Addr[u4I], rCmdQ.u4Data[u4I], rCmdQ.u4Mask[u4I]);
                DPLOGI("[tid: %d][onConfigFrame_HEVC] Poll (0x%x, 0x%x, 0x%x)\n",
                    _current_tid, rCmdQ.u4Addr[u4I], rCmdQ.u4Data[u4I], rCmdQ.u4Mask[u4I]);
                break;
            }

            default:
            {
                DPLOGE("[tid: %d][onConfigFrame_HEVC][ERROR] rCmdQ.eCmd[%d] = %d\n", _current_tid, u4I, rCmdQ.eCmd[u4I]);
                goto onConfigFrame_HEVC_Deinit_Release_Driver_ERROR;
                break;
            }
        }
    }

    DPLOGI("DpEngine_VENC: total read cmd: %d\n",(*m_pVEncConfig->pNumPABuffer));

    return DP_STATUS_RETURN_SUCCESS;

onConfigFrame_HEVC_Deinit_Release_Driver_ERROR:
    eVEncDrvDeInit(m_pVEncConfig->rVencDrvHandle);
    eVEncDrvRelease(m_pVEncConfig->rVencDrvHandle, VENC_DRV_VIDEO_FORMAT_HEVC);

    return DP_STATUS_UNKNOWN_ERROR;
}
#elif defined(MTK_SLOW_MOTION_H264_SUPPORT)
DP_STATUS_ENUM DpEngine_VENC::onConfigFrame_H264(DpCommand &command, DpConfig &config)
{
    uint32_t offset = 0;
    VAL_CHAR_T  icvalue[PROPERTY_VALUE_MAX];
    VAL_BOOL_T  fgForceIFrame;
    VAL_UINT32_T _current_tid = gettid();
    VENC_DRV_H264_BUF_T rBuf;
    VENC_DRV_H264_CMDQ_T rCmdQ;
    VAL_UINT32_T u4I = 0;

    /* [ToDo] Need to get from dpframework + */
    rBuf.rFrmBuf.rFrmBufAddr.u4VA = m_pVEncConfig->memYUVSWAddr[0];
    rBuf.rFrmBuf.rFrmBufAddr.u4PA = m_pVEncConfig->memYUVMVAAddr[0];
    rBuf.rFrmBuf.rFrmBufAddr.u4Size = m_pVEncConfig->memYUVMVASize[0] +
                                      m_pVEncConfig->memYUVMVASize[1] +
                                      m_pVEncConfig->memYUVMVASize[2];

    rBuf.rBSBuf.rBSAddr.u4VA = m_pVEncConfig->memOutputSWAddr[0];
    rBuf.rBSBuf.rBSAddr.u4PA = config.memAddr[0];
    rBuf.rBSBuf.rBSAddr.u4Size = config.memSize[0] +
                                 config.memSize[1] +
                                 config.memSize[2] ;

    rBuf.rBSBuf.u4BSStartVA = rBuf.rBSBuf.rBSAddr.u4VA;
    rBuf.rBSBuf.u4BSSize = 0;

    DPLOGI("rBuf.rFrmBuf.rFrmBufAddr.u4VA: %x\n",rBuf.rFrmBuf.rFrmBufAddr.u4VA);
    DPLOGI("rBuf.rFrmBuf.rFrmBufAddr.u4PA: %x\n",rBuf.rFrmBuf.rFrmBufAddr.u4PA);
    DPLOGI("rBuf.rFrmBuf.rFrmBufAddr.u4Size: %x\n",rBuf.rFrmBuf.rFrmBufAddr.u4Size);

    DPLOGI("rBuf.rBSBuf.rBSAddr.u4VA: %x\n",rBuf.rBSBuf.rBSAddr.u4VA);
    DPLOGI("rBuf.rBSBuf.rBSAddr.u4PA: %x\n",rBuf.rBSBuf.rBSAddr.u4PA);
    DPLOGI("rBuf.rBSBuf.rBSAddr.u4Size: %x\n",rBuf.rBSBuf.rBSAddr.u4Size);

    rCmdQ.u4CmdCnt = 0;

    // check force I frame
    property_get("vendor.dl.vr.force.iframe", icvalue, "0");
    fgForceIFrame = (VAL_BOOL_T) atoi(icvalue);
    DPLOGD("[tid: %d][onConfigFrame_H264] ForceIFrame = %d\n", _current_tid, fgForceIFrame);
    property_set("vendor.dl.vr.force.iframe", "0");

    if (fgForceIFrame == VAL_TRUE)
    {
        if (VENC_DRV_MRESULT_FAIL == eVEncDrvSetParam(m_pVEncConfig->rVencDrvHandle, VENC_DRV_SET_TYPE_FORCE_INTRA_ON, &rBuf, &rCmdQ))
        {
            DPLOGE("[tid: %d][onConfigFrame_H264][ERROR] cannot set param VENC_DRV_SET_TYPE_FORCE_INTRA_ON\n", _current_tid);
            goto onConfigFrame_H264_Deinit_Release_Driver_ERROR;
        }

        property_set("vendor.dl.vr.force.iframe", "0");

        if (VENC_DRV_MRESULT_FAIL == eVEncDrvSetParam(m_pVEncConfig->rVencDrvHandle, VENC_DRV_SET_TYPE_FORCE_INTRA_OFF, &rBuf, &rCmdQ))
        {
            DPLOGE("[tid: %d][onConfigFrame_H264][ERROR] cannot set param VENC_DRV_SET_TYPE_FORCE_INTRA_OFF\n", _current_tid);
            goto onConfigFrame_H264_Deinit_Release_Driver_ERROR;
        }
    }

    // encode sequence header or frame
    if (VENC_DRV_MRESULT_FAIL == eVEncDrvSetParam(m_pVEncConfig->rVencDrvHandle, VENC_DRV_SET_TYPE_SLOW_MOTION_ENCODE, &rBuf, &rCmdQ))
    {
        DPLOGE("[tid: %d][onConfigFrame_H264][ERROR] cannot set param VENC_DRV_SET_TYPE_SLOW_MOTION_ENCODE\n", _current_tid);
        goto onConfigFrame_H264_Deinit_Release_Driver_ERROR;
    }

    *(m_pVEncConfig->pNumPABuffer) = 0;
    offset = (m_configFrameCount & 0x3) << 2;

    for (u4I = 0; u4I < rCmdQ.u4CmdCnt; u4I++)
    {
        switch (rCmdQ.eCmd[u4I])
        {
            case H264_CMDQ_CMD_READ:
            {
                /* [ToDo] Need to move to dpframework api */
                /* add more addr condition check for different platform*/
                MM_REG_READ_BEGIN(command);
                if((0x17002098 == rCmdQ.u4Addr[u4I]) || (0x17020098 == rCmdQ.u4Addr[u4I])) // VENC_PIC_BITSTREAM_BYTE_CNT
                {
                    DPLOGI("PABuffer[%d] : 0x%x\n", (*(m_pVEncConfig->pNumPABuffer))+offset, m_pVEncConfig->pPABuffer[(*m_pVEncConfig->pNumPABuffer)+offset]);
                    MM_REG_READ(command, rCmdQ.u4Addr[u4I], m_pVEncConfig->pPABuffer[(*m_pVEncConfig->pNumPABuffer)+offset]);
                    (*m_pVEncConfig->pNumPABuffer)++;
                }
                else
                {
                    MM_REG_READ(command, rCmdQ.u4Addr[u4I],(uint32_t*) m_pVEncConfig->pPABuffer[DP_MAX_PABUFFER_COUNT - 1]);
                }
                MM_REG_READ_END(command);
                DPLOGI("[tid: %d][onConfigFrame_H264] Read (0x%x)\n", _current_tid, (uint32_t)rCmdQ.u4Addr[u4I]);
                break;
            }

            case H264_CMDQ_CMD_WRITE:
            {
                MM_REG_WRITE_MASK(command, rCmdQ.u4Addr[u4I], rCmdQ.u4Data[u4I], rCmdQ.u4Mask[u4I]);
                DPLOGI("[tid: %d][onConfigFrame_H264] Write (0x%x, 0x%x, 0x%x)\n",
                    _current_tid, rCmdQ.u4Addr[u4I], rCmdQ.u4Data[u4I], rCmdQ.u4Mask[u4I]);
                break;
            }

            case H264_CMDQ_CMD_POLL:
            {
                MM_REG_POLL_MASK(command, rCmdQ.u4Addr[u4I], rCmdQ.u4Data[u4I], rCmdQ.u4Mask[u4I]);
                DPLOGI("[tid: %d][onConfigFrame_H264] Poll (0x%x, 0x%x, 0x%x)\n",
                    _current_tid, rCmdQ.u4Addr[u4I], rCmdQ.u4Data[u4I], rCmdQ.u4Mask[u4I]);
                break;
            }

            default:
            {
                DPLOGE("[tid: %d][onConfigFrame_H264][ERROR] rCmdQ.eCmd[%d] = %d\n", _current_tid, u4I, rCmdQ.eCmd[u4I]);
                goto onConfigFrame_H264_Deinit_Release_Driver_ERROR;
                break;
            }
        }
    }

    DPLOGI("DpEngine_VENC: total read cmd: %d\n",(*m_pVEncConfig->pNumPABuffer));

    return DP_STATUS_RETURN_SUCCESS;

onConfigFrame_H264_Deinit_Release_Driver_ERROR:
    eVEncDrvDeInit(m_pVEncConfig->rVencDrvHandle);
    eVEncDrvRelease(m_pVEncConfig->rVencDrvHandle, VENC_DRV_VIDEO_FORMAT_H264);

    return DP_STATUS_UNKNOWN_ERROR;
}
#endif // MTK_SLOW_MOTION_HEVC_SUPPORT
/* Venc Modify - */

DP_STATUS_ENUM DpEngine_VENC::onConfigFrame(DpCommand&, DpConfig &config)
{
    m_config = config;

    return run(PRIORITY_HIGH);
}


DP_STATUS_ENUM DpEngine_VENC::threadLoop()
{
    DP_TRACE_CALL();
    DP_STATUS_ENUM status = DP_STATUS_RETURN_SUCCESS;

    m_pVEncConfig = m_config.pVEnc_cfg;
    m_pVEncCommander = m_pVEncConfig->pVEncCommander;
    m_pVEncCommander->mark(DpCommand::FRAME_COMMAND);
    MM_REG_WAIT(*m_pVEncCommander, DpCommand::SYNC_TOKEN_VENC_INPUT_READY);
    m_configFrameCount = *(m_pVEncConfig->pConfigFrameCount);

    /* Venc Modify + */
#if defined(MTK_SLOW_MOTION_HEVC_SUPPORT)
    VAL_UINT32_T _current_tid = gettid();

    if (DP_STATUS_RETURN_SUCCESS != onConfigFrame_HEVC(*m_pVEncCommander, m_config))
    {
        DPLOGE("[tid: %d][onConfigFrame][ERROR] onConfigFrame_HEVC\n", _current_tid);
        status = DP_STATUS_UNKNOWN_ERROR;
    }
#elif defined(MTK_SLOW_MOTION_H264_SUPPORT)
    VAL_UINT32_T _current_tid = gettid();

    if (DP_STATUS_RETURN_SUCCESS != onConfigFrame_H264(*m_pVEncCommander, m_config))
    {
        DPLOGE("[tid: %d][onConfigFrame][ERROR] onConfigFrame_H264\n", _current_tid);
        status = DP_STATUS_UNKNOWN_ERROR;
    }
#endif // MTK_SLOW_MOTION_HEVC_SUPPORT
    /* Venc Modify - */

    return status;
}


DP_STATUS_ENUM DpEngine_VENC::onInitTileCalc(struct TILE_PARAM_STRUCT*)
{
    return DP_STATUS_RETURN_SUCCESS;
}


DP_STATUS_ENUM DpEngine_VENC::onRetrieveTileParam(struct TILE_PARAM_STRUCT* p_tile_param)
{
    /* tile core property */
    FUNC_DESCRIPTION_STRUCT *ptr_tile_func_param = p_tile_param->ptr_tile_func_param;
    TILE_REG_MAP_STRUCT *ptr_tile_reg_map = p_tile_param->ptr_tile_reg_map;

    m_tileXEndFlag = ptr_tile_func_param->func_list[ptr_tile_reg_map->first_func_en_no].h_end_flag;
    m_tileYEndFlag = ptr_tile_func_param->func_list[ptr_tile_reg_map->first_func_en_no].v_end_flag;

    return DP_STATUS_RETURN_SUCCESS;
}


// VENC MUST be in the last engine in waiting frame done queue.
DP_STATUS_ENUM DpEngine_VENC::onWaitEvent(DpCommand &command)
{
    MM_REG_WAIT(command, DpCommand::WDMA_FRAME_DONE);

    // Disable engine
    MM_REG_WRITE(command, WDMA_EN, 0x00, 0x00000001);

    if (m_tileXEndFlag && m_tileYEndFlag)
    {
        // Wait VENC config thread
        DP_STATUS_ENUM status = join();
        if (DP_STATUS_RETURN_SUCCESS != status)
        {
            return status;
        }

        // Wait frame done
        // POLLING VENC register for wait VENC done
        MM_REG_SET_EVENT(*m_pVEncCommander, DpCommand::SYNC_TOKEN_VENC_EOF);

        // For VENC sync
        DPLOGI("DpEngine_VENC: m_configFrameCount:%d\n", m_configFrameCount);
        if (m_configFrameCount > 0) // Means 1st frame no need to wait VENC done
        {
            MM_REG_WAIT(command, DpCommand::SYNC_TOKEN_VENC_EOF);
        }

        MM_REG_SET_EVENT(command, DpCommand::SYNC_TOKEN_VENC_INPUT_READY);
    }

    return DP_STATUS_RETURN_SUCCESS;
}

DP_STATUS_ENUM DpEngine_VENC::onReconfigTiles(DpCommand&)
{
    DP_STATUS_ENUM status = join();
    if (DP_STATUS_RETURN_SUCCESS != status)
    {
        return status;
    }

    MM_REG_SET_EVENT(*m_pVEncCommander, DpCommand::SYNC_TOKEN_VENC_EOF);

    return DP_STATUS_RETURN_SUCCESS;
}

