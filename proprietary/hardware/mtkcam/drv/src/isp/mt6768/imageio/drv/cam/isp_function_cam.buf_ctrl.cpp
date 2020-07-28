/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein is
 * confidential and proprietary to MediaTek Inc. and/or its licensors. Without
 * the prior written permission of MediaTek inc. and/or its licensors, any
 * reproduction, modification, use or disclosure of MediaTek Software, and
 * information contained herein, in whole or in part, shall be strictly
 * prohibited.
 *
 * MediaTek Inc. (C) 2010. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
 * ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 * NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH
 * RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 * INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES
 * TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
 * RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
 * OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN MEDIATEK
 * SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE
 * RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S
 * ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE
 * RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE
 * MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE
 * CHARGE PAID BY RECEIVER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek
 * Software") have been modified by MediaTek Inc. All revisions are subject to
 * any receiver's applicable license agreements with MediaTek Inc.
 */

#define LOG_TAG "ifunc_cam_buf"

//open syscall
#include <mtkcam/def/PriorityDefs.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
//close syscall
#include <unistd.h>
//mmap syscall
#include <sys/mman.h>
//mutex
#include <pthread.h>
#include <utils/StrongPointer.h>


#include <cutils/properties.h>  // For property_get().

#include <ispio_pipe_ports.h>
#include "isp_function_cam.h"
#include "Cam_Notify_datatype.h"
#include "tuning_drv.h"
#include "sec_mgr.h"

#include "imageio_log.h"                    // Note: DBG_LOG_TAG/LEVEL will be used in header file, so header must be included after definition.
#undef   DBG_LOG_TAG                        // Decide a Log TAG for current file.
#ifndef USING_MTK_LDVT   // Not using LDVT.
#define  DBG_LOG_TAG        ""
#else
#define  DBG_LOG_TAG        LOG_TAG
#endif

#define  ION_BUFFER_KCTRL   1
#define  TS_BUF_SIZE 128

EXTERN_DBG_LOG_VARIABLE(func_cam);

// Clear previous define, use our own define.
#undef CAM_FUNC_VRB
#undef CAM_FUNC_DBG
#undef CAM_FUNC_INF
#undef CAM_FUNC_WRN
#undef CAM_FUNC_ERR
#undef CAM_FUNC_AST
#define CAM_FUNC_VRB(fmt, arg...)        do { if (func_cam_DbgLogEnable_VERBOSE) { BASE_LOG_VRB(fmt, ##arg); } } while(0)
#define CAM_FUNC_DBG(fmt, arg...)        do {\
    if (func_cam_DbgLogEnable_DEBUG  ) { \
        BASE_LOG_DBG("[0x%x]:" fmt,this->m_hwModule, ##arg); \
    }\
} while(0)

#define CAM_FUNC_INF(fmt, arg...)        do {\
    if (func_cam_DbgLogEnable_INFO  ) { \
        BASE_LOG_INF("[0x%x]:" fmt,this->m_hwModule, ##arg); \
    }\
} while(0)

#define CAM_FUNC_WRN(fmt, arg...)        do {\
    if (func_cam_DbgLogEnable_WARN   ) { \
        BASE_LOG_WRN("[0x%x]:" fmt,this->m_hwModule, ##arg); \
    }\
} while(0)

#define CAM_FUNC_ERR(fmt, arg...)        do {\
    if (func_cam_DbgLogEnable_ERROR  ) { \
        BASE_LOG_ERR("[0x%x]:" fmt,this->m_hwModule, ##arg); \
    }\
} while(0)

#define CAM_FUNC_AST(cond, fmt, arg...)  do { if (func_cam_DbgLogEnable_ASSERT ) { BASE_LOG_AST(cond, fmt, ##arg); } } while(0)


#define CAM_SFUNC_DBG(fmt, arg...)        do {\
    if (func_cam_DbgLogEnable_DEBUG  ) { \
        BASE_LOG_DBG(fmt, ##arg); \
    }\
} while(0)
#define CAM_SFUNC_INF(fmt, arg...)        do {\
    if (func_cam_DbgLogEnable_INFO  ) { \
        BASE_LOG_INF(fmt, ##arg); \
    }\
} while(0)
#define CAM_SFUNC_ERR(fmt, arg...)        do {\
    if (func_cam_DbgLogEnable_ERROR  ) { \
        BASE_LOG_ERR(fmt, ##arg); \
    }\
} while(0)


#define _DMA_Mapping(x)({\
    ISP_WRDMA_ENUM _port;\
    if(x ==_imgo_)      _port = _dma_imgo_;\
    else if(x ==_rrzo_) _port = _dma_rrzo_;\
    else if(x == _aao_) _port = _dma_aao_;\
    else if(x == _afo_) _port = _dma_afo_;\
    else if(x == _lcso_)_port = _dma_lcso_;\
    else if(x == _ufeo_)_port = _dma_ufeo_;\
    else if(x == _pdo_) _port = _dma_pdo_;\
    else if(x == _eiso_)_port = _dma_eiso_;\
    else if(x == _flko_)_port = _dma_flko_;\
    else if(x == _rsso_)_port = _dma_rsso_;\
    else if(x == _pso_) _port = _dma_pso_;\
    else\
        _port = _dma_max_wr_;\
    _port;\
})
#define _DMA_Header_Mapping(x)({\
    ISP_WRDMA_ENUM _port;\
    if(x ==_imgo_)      _port = _dma_imgo_fh_;\
    else if(x ==_rrzo_) _port = _dma_rrzo_fh_;\
    else if(x == _aao_) _port = _dma_aao_fh_;\
    else if(x == _afo_) _port = _dma_afo_fh_;\
    else if(x == _lcso_)_port = _dma_lcso_fh_;\
    else if(x == _ufeo_)_port = _dma_ufeo_fh_;\
    else if(x == _pdo_) _port = _dma_pdo_fh_;\
    else if(x == _eiso_)_port = _dma_eiso_fh_;\
    else if(x == _flko_)_port = _dma_flko_fh_;\
    else if(x == _rsso_)_port = _dma_rsso_fh_;\
    else if(x == _pso_) _port = _dma_pso_fh_;\
    else\
        _port = _dma_max_wr_;\
    _port;\
})

/*/////////////////////////////////////////////////////////////////////////////
    CAM_BUF_CTRL
  /////////////////////////////////////////////////////////////////////////////*/


MUINT32 CAM_BUF_CTRL::m_fps[CAM_MAX] = {30, 30};
MUINT32 CAM_BUF_CTRL::m_recentFrmTimeMs[CAM_MAX][MAX_RECENT_GRPFRM_TIME] = {
    {MIN_GRPFRM_TIME_MS, MIN_GRPFRM_TIME_MS},
    {MIN_GRPFRM_TIME_MS, MIN_GRPFRM_TIME_MS}
};
CAM_BUF_CTRL::E_CAM_STATE CAM_BUF_CTRL::m_CamState[CAM_MAX] = {CAM_BUF_CTRL::eCamState_runnable, CAM_BUF_CTRL::eCamState_runnable};
CAM_BUF_CTRL::E_CAM_STATE CAM_BUF_CTRL::m_SttState[CAM_MAX] = {CAM_BUF_CTRL::eCamState_runnable, CAM_BUF_CTRL::eCamState_runnable};

CAM_BUF_CTRL::CAM_BUF_CTRL(void)
{
    //m_fps = 30;
    m_tail_idx = -1;
    m_buf_cnt = 0;
    m_pUniDrv = NULL;
    m_pTimeStamp = 0;
    extArg = 0;
    m_pDrv = NULL;
}

MBOOL CAM_BUF_CTRL::PipeCheck(void)
{
    MUINT32 err_status;
    PIPE_CHECK ppc;
    MUINT32 rst = 0;
    MUINT32 bTwin;
    ISP_DRV_CAM* ptr = NULL;

    bTwin = CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_CTL_TWIN_STATUS,TWIN_EN);
    if(bTwin == 1){
        capibility CamInfo;
        if(CamInfo.m_DTwin.GetDTwin() == MFALSE)
            ptr = (ISP_DRV_CAM*)ISP_DRV_CAM::createInstance(CAM_B,ISP_DRV_CQ_THRE0,0,"PIPE_CHK");
        else
            ptr = (ISP_DRV_CAM*)ISP_DRV_CAM::createInstance(CAM_A_TWIN,ISP_DRV_CQ_THRE0,0,"PIPE_CHK");
    }

    CAM_FUNC_ERR("start PipeCheck when deque fail at wait signal, current twin is %d\n",bTwin);

    ppc.m_pDrv = this->m_pDrv;
    if(this->m_pUniDrv)
        ppc.m_pUniDrv = this->m_pUniDrv;
    else
        ppc.m_pUniDrv = NULL;
    //err status:
    this->m_pDrv->getDeviceInfo(_GET_INT_ERR,(MUINT8 *)&err_status);
    if(err_status == 0){
    }
    else{
        CAM_FUNC_ERR("accumulated err status:0x%x\n",err_status);
        rst += 1;
    }
    //

    if(err_status & DMA_ERR_EN_){
        CAM_FUNC_ERR("all dmao err status:\n");
        ppc.DMAO_STATUS();
        rst += 1;
    }

    if( ppc.FIFO_FULL_STATUS() == MTRUE){
        CAM_FUNC_INF("FIFO check done!\n");
    }
    else
        rst += 1;

    if( ppc.RRZ_CHECK() == MTRUE){
        CAM_FUNC_INF("RRZ check pass!\n");
    }
    else
        rst += 1;

    if( ppc.AA_CHECK() == MTRUE){
        CAM_FUNC_INF("AA/AAO check pass!\n");
    }
    else
        rst += 1;

    if( ppc.LCS_CHECK() == MTRUE){
        CAM_FUNC_INF("LCS/LCSO check pass!\n");
    }
    else
        rst += 1;

    if(bTwin == MFALSE){
        if( ppc.LSC_CHECK() == MTRUE){
            CAM_FUNC_INF("LSC/LSCI check pass!\n");
        }
        else
            rst += 1;
    }

    if( ppc.AF_CHECK() == MTRUE){
        CAM_FUNC_INF("af/afo check pass!\n");
    }
    else
        rst += 1;

    if(ppc.PDO_CHECK() == MTRUE){
        CAM_FUNC_INF("pd/pdo check pass!\n");
    }
    else
        rst += 1;

    if( ppc.PS_CHECK() == MTRUE){
        CAM_FUNC_INF("PS/PSO check pass!\n");
    }
    else
        rst += 1;

    if(ppc.XMX_CHECK() == MTRUE){
        CAM_FUNC_INF("XMX check pass!\n");
    }
    else
        rst += 1;


    if(bTwin){
        ppc.m_pDrv = ptr;
        if( ppc.AF_CHECK() == MTRUE){
            CAM_FUNC_INF("af/afo at CAM_B check pass!\n");
        }
        else
            rst += 1;

        if(ppc.XMX_CHECK() == MTRUE){
            CAM_FUNC_INF("XMX at CAM_B check pass!\n");
        }
        else
            rst += 1;

        ppc.m_pDrv = this->m_pDrv;
        if(ppc.TWIN_CHECK(ptr) == MFALSE)
            rst += 1;
    }


    if(ppc.m_pUniDrv){
        if( ppc.FLK_CHECK() == MTRUE){
            CAM_FUNC_INF("FLK/FLKO check pass!\n");
        }
        else
            rst += 1;

        if( ppc.EIS_CHECK() == MTRUE){
            CAM_FUNC_INF("EIS/EISO check pass!\n");
        }
        else
            rst += 1;

        if( ppc.RSS_CHECK() == MTRUE){
            CAM_FUNC_INF("RSS/RSSO check pass!\n");
        }
        else
            rst += 1;
    }

    if( ppc.TG_CHECK(err_status) == MTRUE){
        CAM_FUNC_INF("TG check pass!\n");
    }
    else
        rst += 1;

    //uni int check
    if(this->m_pUniDrv){
        MUINT32 uni_int;
        char add[256] = {"\0"};
        char* ptr = add;

        uni_int = UNI_READ_REG(this->m_pUniDrv,CAM_UNI_TOP_DMA_INT_STATUS);

        if(uni_int & DMA_ERR_INT_EN_){
            snprintf(ptr, sizeof(add)-1, "UNI DMA ERR");
            while (*ptr++ != '\0'){}
            ptr--;
            rst += 1;
        }
        if(uni_int & FLKO_A_ERR_INT_EN_){
            snprintf(ptr, sizeof(add)-1, "FLKO ERR");
            while (*ptr++ != '\0'){}
            ptr--;
            rst += 1;
        }
        if(uni_int & EISO_A_ERR_INT_EN_){
            snprintf(ptr, sizeof(add)-1, "EISO ERR");
            while (*ptr++ != '\0'){}
            ptr--;
            rst += 1;
        }
        if(uni_int & RSSO_A_ERR_INT_EN_){
            snprintf(ptr, sizeof(add)-1, "RSSO ERR");
            while (*ptr++ != '\0'){}
            ptr--;
            rst += 1;
        }
        if(uni_int)
            CAM_FUNC_ERR("%s\n",add);
    }

    if( rst == 0){
        if(ppc.ENQUE_CHECK() == MFALSE){
            //goto EXIT;
        }
        else{
            CAM_FUNC_ERR("find no err, plz look for previous err ,like enque flow err\n");
        }
    }




    //dump phy reg
    if(CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_TG_VF_CON,VFDATA_EN) == 1){
        CAM_FUNC_ERR("start dump CAM register\n");
        this->m_pDrv->DumpReg(MTRUE);
        if(bTwin == 1){
            CAM_FUNC_ERR("twin is enabled, start dump CAM_B register\n");
            ptr->DumpReg(MTRUE);
        }
        if(this->m_pUniDrv){
            CAM_FUNC_ERR("start dump UNI register\n");
            this->m_pUniDrv->DumpReg();
        }
    }
    else{
        CAM_FUNC_ERR("viewfinder off,no reg dump\n");
    }

EXIT:
    if(bTwin == 1){
        ptr->destroyInstance();
    }

    if(rst)
        return MFALSE;
    else
        return MTRUE;
}

E_BC_STATUS CAM_BUF_CTRL::waitBufReady(CAM_STATE_NOTIFY *pNotify)
{
#define _LOOP_  (5)
    E_BC_STATUS ret = eCmd_Pass;
    E_ISP_CAM_CQ cq;
    MUINT32 page;

    ISP_WAIT_IRQ_ST irq;
    MUINT32 loopCnt = _LOOP_;
    MUINT32 subSample;
    char str[128] = {'\0'};

    MUINT32 FbcCnt=0;
    //MUINT32 WCnt=0,newWCnt=0; //not used in mt6757
    MUINT32 sw_enque_cnt = 0, _pa = 0;
    ST_BUF_INFO _buf;
    ISP_DRV_CAM* pDrv = NULL;
    MUINT32 _size = 0;
    MBOOL bWaitPass = MTRUE;
    MBOOL bDbgLog = (func_cam_DbgLogEnable_DEBUG) ? MTRUE : MFALSE;

    if (0xffffffff == this->id()) {
        CAM_FUNC_ERR("dma channel error ");
        return eCmd_Fail;
    }

    this->m_Queue_deque_ptr.sendCmd(QueueMgr<MUINTPTR>::eCmd_getsize,(MUINTPTR)&_size,0);
    if (_size < 1) {
        CAM_FUNC_ERR("queue depth is empty for deque\n");
        return eCmd_Fail;
    }

    //
    this->m_Queue_deque_ptr.sendCmd(QueueMgr<MUINTPTR>::eCmd_front, (MUINTPTR)&pDrv, 0);
    //can't pop here, pop at deque
    //this->m_Queue_deque_ptr.sendCmd(CAM_BUF_CTRL::QueueMgr<MUINTPTR>::eCmd_pop  ,0,0);

    //dbg log
    if (bDbgLog) {
        pDrv->getCurObjInfo(&this->m_hwModule, &cq, &page);
    }

#if 0
    //fbc reg dbg log
    this->FBC_STATUS(pDrv->getPhyObj());
#endif

    switch (CAM_BUF_CTRL::m_CamState[this->m_hwModule]) {
        // theoratically, it no need to reference normalpipe's status under sttpipe.
        // but due to suspend/resume is only implemented in noramlpipe, so sttpipe need to use suspend/resume status.
        case CAM_BUF_CTRL::eCamState_suspending:
            return eCmd_Suspending_Pass;
            break;
        default:
            break;
    }

    subSample = CAM_READ_BITS(pDrv->getPhyObj(), CAM_CTL_SW_PASS1_DONE,DOWN_SAMPLE_PERIOD) + 1;

    irq.Clear = ISP_IRQ_CLEAR_WAIT;
    irq.UserKey = 0;

    switch (this->id()) {
        case _imgo_:
            irq.St_type = SIGNAL_INT;
            irq.Status = (MUINT32)SW_PASS1_DON_ST;
            break;
        case _rrzo_:
            irq.St_type = SIGNAL_INT;
            irq.Status = (MUINT32)SW_PASS1_DON_ST;
            break;
        case _ufeo_:
            irq.St_type = SIGNAL_INT;
            irq.Status = (MUINT32)SW_PASS1_DON_ST;
            break;
        case _lcso_:
            irq.St_type = SIGNAL_INT;
            irq.Status = (MUINT32)SW_PASS1_DON_ST;
            break;
        case _eiso_:
            irq.St_type = SIGNAL_INT;
            irq.Status = (MUINT32)SW_PASS1_DON_ST;
            break;
        case _rsso_:
            irq.St_type = SIGNAL_INT;
            irq.Status = (MUINT32)SW_PASS1_DON_ST;
            break;
        case _aao_:
        case _afo_:
        case _pdo_:
        case _flko_:
        case _pso_:
            //irq.St_type = DMA_INT;
            //irq.Status = (MUINT32)RSSO_DONE_ST;
        default:
            CAM_FUNC_ERR("un-supported DMAO:0x%x\n", this->id());
            return eCmd_Fail;
    }

    do {
        //irq.Timeout = ((1000 + (this->m_fps - 1)) / this->m_fps) * subSample;
        //irq.Timeout *= 5; //amplify 3 times is for the case of enque latch timing
        irq.Timeout = this->estimateTimeout(subSample);

        if ((this->m_buf_cnt != 0) && (this->m_buf_cnt >= subSample)) {
            // Burst mode
            this->m_Queue.sendCmd(QueueMgr<ST_BUF_INFO>::eCmd_at,(subSample-1),(MUINTPTR)&_buf);

            //only using sw_enque_cnt at 1st time.
            //because dmao may still output even if no sw p1 done . e.g. : some module setting err.
            //if no sw p1 done, hw fbc cnt won't be update.
            if (loopCnt == _LOOP_) {
                /*
                 * MUST hold fbcPhyLock_XXX when read FBC CTL registers, due to isp reset flow period will
                 * clear FBC register value before. Hold lock will wait until reset flow recovers FBC register values
                 */

                // Read back from security world for frame header content
                if (SecMgr::SecMgr_GetSecurePortStatus(this->m_hwModule, this->id())) {
                    SecMgr *mpSecMgr = SecMgr::SecMgr_GetMgrObj();
                    SecMgr_SecInfo secinfo;

                    secinfo.type = SECMEM_FRAME_HEADER;
                    secinfo.module = this->m_hwModule;
                    secinfo.buff_va = _buf.header.va_addr;
                    secinfo.port = this->id();

                    CAM_FUNC_INF("1-Migrate FH: port:0x%x buff_va:0x%lx", secinfo.port, secinfo.buff_va);
                    if (mpSecMgr->SecMgr_P1MigrateTable(&secinfo) == MFALSE) {
                        CAM_FUNC_ERR("Migrate sec FH failed:(0x%x)\n", this->id());
                        return eCmd_Fail;
                    }
                }

                switch (this->id()) {
                    case _imgo_:
                        sw_enque_cnt = *((MUINT32*)_buf.header.va_addr + Header_IMGO::E_ENQUE_CNT);
                        _pa = *((MUINT32*)_buf.header.va_addr + Header_IMGO::E_IMG_PA);
                        break;
                    case _rrzo_:
                        sw_enque_cnt = *((MUINT32*)_buf.header.va_addr + Header_RRZO::E_ENQUE_CNT);
                        _pa = *((MUINT32*)_buf.header.va_addr + Header_RRZO::E_IMG_PA);
                        break;
                    case _ufeo_:
                        sw_enque_cnt = *((MUINT32*)_buf.header.va_addr + Header_UFEO::E_ENQUE_CNT);
                        _pa = *((MUINT32*)_buf.header.va_addr + Header_UFEO::E_IMG_PA);
                        break;
                    case _lcso_:
                        sw_enque_cnt = *((MUINT32*)_buf.header.va_addr + Header_LCSO::E_ENQUE_CNT);
                        _pa = *((MUINT32*)_buf.header.va_addr + Header_LCSO::E_IMG_PA);
                        break;
                    case _eiso_:
                        sw_enque_cnt = *((MUINT32*)_buf.header.va_addr + Header_EISO::E_ENQUE_CNT);
                        _pa = *((MUINT32*)_buf.header.va_addr + Header_EISO::E_IMG_PA);
                        break;
                    case _rsso_:
                        sw_enque_cnt = *((MUINT32*)_buf.header.va_addr + Header_RSSO::E_ENQUE_CNT);
                        _pa = *((MUINT32*)_buf.header.va_addr + Header_RSSO::E_IMG_PA);
                        break;
                    case _aao_:
                        case _afo_:
                    case _pdo_:
                    case _flko_:
                    case _pso_:
                        //irq.St_type = DMA_INT;
                        //irq.Status = (MUINT32)RSSO_DONE_ST;
                        //FbcCnt = UNI_READ_BITS(this->m_pUniDrv,CAM_UNI_FBC_RSSO_A_CTL2,FBC_CNT);
                        //break;
                    default:
                        CAM_FUNC_ERR("un-supported DMAO:0x%x\n",this->id());
                        return eCmd_Fail;
                    break;
                }

                //because rcnt_in is locate on CQ, and m_buf_cnt is updated directly when enque.
                //so it will have a time difference between FBC hw cnt latch and m_buf_cnt update.
                //so can't be just using FbcCnt != m_buf_cnt
                if (sw_enque_cnt > 0) {
#if TEMP_SW_TIMESTAMP   //under twin mode, only main-path have to wait SW timestamp
                    MUINT32 waitOps[3];
                    waitOps[0] = _cam_max_;
                    waitOps[1] = this->id();

                    switch (this->id()) {
                        case _imgo_:
                        case _rrzo_:
                            //only cam_a need to get sw timestamp, cam_b can't get timettamp in order to avoid sw hang!!
                            //and this temp flow can't support DMAO output with CAM_B's only!!!!!!!
                            if(CAM_READ_BITS(pDrv,CAM_CTL_TWIN_STATUS,TWIN_EN)){
                                CAM_FUNC_WRN("need to change SW timestamp to HW timestamp after SB\n");

                                if(CAM_READ_BITS(pDrv,CAM_CTL_TWIN_STATUS,TWIN_MODULE) != this->m_hwModule){
                                    if (pDrv->getDeviceInfo(_GET_START_TIME, (MUINT8*)waitOps) == MFALSE) {
                                        CAM_FUNC_ERR("Wait timestamp ready fail\n");
                                    }
                                }
                            }
                            break;
                         default:
                            if (pDrv->getDeviceInfo(_GET_START_TIME, (MUINT8*)waitOps) == MFALSE) {
                                CAM_FUNC_ERR("Wait timestamp ready fail\n");
                            }
                            break;
                    }
#endif
                    CAM_FUNC_DBG("dma:%s already have avail_buf on dram, bWaitBufReady  = 0,ba:0x%x\n",this->name_Str(),_pa);
                    return eCmd_Pass;
                } else {
                    if (bDbgLog) {
                        snprintf(str, sizeof(str), "dma:%s start wait:[enque record:0x%x_0x%x_0x%x],",this->name_Str(),\
                            this->m_buf_cnt,sw_enque_cnt,FbcCnt);
                    }
                }
            } else {
                // Enter second time of lop
                char _tmp[16] = {0,};

                snprintf(_tmp, sizeof(_tmp), "[%d]time,",((_LOOP_-loopCnt)+1));
                strncat(str,_tmp, (sizeof(str) - strlen(str)) - 1);
            }
        } else {
            // Single mode
            if (loopCnt == _LOOP_) {
                snprintf(str, sizeof(str), "dma:%s bWaitBufReady = 1,wait for sw enque[0x%x_0x%x],timeout:%d x %d ms,[1] ",\
                    this->name_Str(),this->m_buf_cnt,subSample,irq.Timeout,loopCnt);
            } else {
                // save how many times of wait_irq before sw enque
                char _tmp[8];
                snprintf(_tmp, sizeof(_tmp), "[%d],",((_LOOP_-loopCnt)+1) + 1);
                strncat(str,_tmp, (sizeof(str) - strlen(str)) - 1);
            }
        }

        //
        // Check if status == suspending and return ture , in order to make sure all request in driver is poped when suspend.
        CAM_FUNC_DBG("camstate:%d\n", CAM_BUF_CTRL::m_CamState[this->m_hwModule]);
        switch (CAM_BUF_CTRL::m_CamState[this->m_hwModule]) {
            case CAM_BUF_CTRL::eCamState_suspending:
                ret = eCmd_Suspending_Pass;
                goto EXIT;
                break;
            default:
                break;
        }
        //
        if (pNotify) {
            pNotify->fpNotifyState(CAM_ENTER_WAIT, pNotify->Obj);
        }
        //

        if (pDrv->waitIrq(&irq) == MFALSE) {
            ret = eCmd_Fail;
        } else {
            ret = eCmd_Pass;
        }

        if (pNotify) {
            pNotify->fpNotifyState(CAM_EXIT_WAIT, pNotify->Obj);
        }

        CAM_FUNC_DBG("camstate:%d\n", CAM_BUF_CTRL::m_CamState[this->m_hwModule]);
        switch (CAM_BUF_CTRL::m_CamState[this->m_hwModule]) {
            case CAM_BUF_CTRL::eCamState_runnable:
            case CAM_BUF_CTRL::eCamState_suspend:
                break;
            case CAM_BUF_CTRL::eCamState_stop:
                CAM_FUNC_INF("%s fast stop %s\n", this->name_Str(),str);
                return eCmd_Stop_Pass;
                break;
            case CAM_BUF_CTRL::eCamState_suspending:
                // return ture , in order to make sure all request in driver is poped when suspend.
                ret = eCmd_Suspending_Pass;
                snprintf(str, sizeof(str), "suspending pass, ");
                goto EXIT;
                break;
            default:
                CAM_FUNC_ERR("Unknown state: %d\n", CAM_BUF_CTRL::m_CamState[this->m_hwModule]);
                break;
        }

        if ((ret == eCmd_Pass) && (this->m_buf_cnt != 0) && (this->m_buf_cnt >= subSample)) {

            // add for timing error when deque twice between dmao_done to p1_done.
            // currently , only imgo/rrzo using p1 done, so only these 2 dmao need to check.
            // mt6757 need not this check, even in subsample flow

            if (SecMgr::SecMgr_GetSecurePortStatus(this->m_hwModule,this->id())) {
                SecMgr *mpSecMgr = SecMgr::SecMgr_GetMgrObj();
                SecMgr_SecInfo secinfo;

                secinfo.type = SECMEM_FRAME_HEADER;
                secinfo.module = this->m_hwModule;
                secinfo.buff_va = _buf.header.va_addr;
                secinfo.port = this->id();

                CAM_FUNC_INF("2-Migrate FH: port:0x%x buff_va:0x%lx", secinfo.port, secinfo.buff_va);
                if (mpSecMgr->SecMgr_P1MigrateTable(&secinfo) == MFALSE) {
                    CAM_FUNC_ERR("Migrate sec FH failed:(0x%x)\n", this->id());
                    return eCmd_Fail;
                }
            }

            switch (this->id()) {
                case _imgo_:
                {
                    ST_BUF_INFO _buf;

                    //this->m_Queue.sendCmd(CAM_BUF_CTRL::QueueMgr<ST_BUF_INFO>::eCmd_front  ,(MUINTPTR)&_buf,0);
                    this->m_Queue.sendCmd(QueueMgr<ST_BUF_INFO>::eCmd_at,(subSample-1), (MUINTPTR)&_buf);

                    _pa = *((MUINT32*)_buf.header.va_addr + Header_IMGO::E_IMG_PA);

                    if (_pa == 0) {
                        CAM_FUNC_INF("%s:wait 1 more signal for header output\n", this->name_Str());
                        bWaitPass = MFALSE;
                    } else
                        bWaitPass = MTRUE;
                }
                    break;
                case _rrzo_:
                {
                    ST_BUF_INFO _buf;

                    //this->m_Queue.sendCmd(CAM_BUF_CTRL::QueueMgr<ST_BUF_INFO>::eCmd_front  ,(MUINTPTR)&_buf,0);
                    this->m_Queue.sendCmd(QueueMgr<ST_BUF_INFO>::eCmd_at, (subSample-1), (MUINTPTR)&_buf);

                    _pa = *((MUINT32*)_buf.header.va_addr + Header_RRZO::E_IMG_PA);

                    if (_pa == 0) {
                        CAM_FUNC_INF("%s:wait 1 more signal for header output\n", this->name_Str());
                        bWaitPass = MFALSE;
                    } else {
                        bWaitPass = MTRUE;
                    }
                }
                    break;
                case _ufeo_:
                {
                    ST_BUF_INFO _buf;

                    //this->m_Queue.sendCmd(CAM_BUF_CTRL::QueueMgr<ST_BUF_INFO>::eCmd_front  ,(MUINTPTR)&_buf,0);
                    this->m_Queue.sendCmd(QueueMgr<ST_BUF_INFO>::eCmd_at, (subSample-1), (MUINTPTR)&_buf);

                    _pa = *((MUINT32*)_buf.header.va_addr + Header_UFEO::E_IMG_PA);

                    if (_pa == 0) {
                        CAM_FUNC_INF("%s:wait 1 more signal for header output\n", this->name_Str());
                        bWaitPass = MFALSE;
                    } else {
                        bWaitPass = MTRUE;
                    }
                }
                    break;
                case _lcso_:
                {
                    ST_BUF_INFO _buf;

                    this->m_Queue.sendCmd(QueueMgr<ST_BUF_INFO>::eCmd_at, (subSample-1), (MUINTPTR)&_buf);

                    _pa = *((MUINT32*)_buf.header.va_addr + Header_LCSO::E_IMG_PA);

                    if (_pa == 0) {
                        CAM_FUNC_INF("%s:wait 1 more signal for header output\n", this->name_Str());
                        bWaitPass = MFALSE;
                    } else {
                        bWaitPass = MTRUE;
                    }
                }
                    break;
                case _eiso_:
                {
                    ST_BUF_INFO _buf;

                    this->m_Queue.sendCmd(QueueMgr<ST_BUF_INFO>::eCmd_at, (subSample-1), (MUINTPTR)&_buf);

                    _pa = *((MUINT32*)_buf.header.va_addr + Header_EISO::E_IMG_PA);

                    if (_pa == 0) {
                        CAM_FUNC_INF("%s:wait 1 more signal for header output\n", this->name_Str());
                        bWaitPass = MFALSE;
                    } else {
                        bWaitPass = MTRUE;
                    }
                }
                    break;
                default:
                    break;
            }

            if (bWaitPass == MTRUE) {
                // leave while loop
                break;
            }
        }
        //
        irq.Clear = ISP_IRQ_CLEAR_NONE;
    }while (--loopCnt > 0);
EXIT:
    if (ret == eCmd_Fail) {
        if (this->m_buf_cnt != 0) {
            CAM_FUNC_ERR("%s, waitbufready fail. start fail check\n", str);
            this->PipeCheck();
        } else {
            CAM_FUNC_ERR("%s, waitbufready fail is caused by no enque\n", str);
        }
    } else {
        switch (this->id()) {
            case _rrzo_:
                CAM_FUNC_INF("%s, [0x%x] waitbufready pass\n", str, _pa);
                break;
            default:
                CAM_FUNC_DBG("%s, waitbufready pass\n", str);
                break;
        }
    }

    return ret;
}

MINT32 CAM_BUF_CTRL::_config(void)
{
    E_ISP_CAM_CQ cq;
    MUINT32 page;
    this->m_pDrv = (ISP_DRV_CAM*)this->m_pIspDrv;
    //dbg log
    this->m_pDrv->getCurObjInfo(&this->m_hwModule,&cq,&page);

    CAM_FUNC_DBG("%s::_config+ cq:0x%x,page:0x%x\n",this->name_Str(),cq,page);


    this->m_Queue.init();
    this->m_Queue_deque_ptr.init();

#if (TEMP_SW_TIMESTAMP == 1)
    this->m_TimeStamp.TimeStamp_Init((const ISP_DRV_CAM*)this->m_pDrv);
#endif
    switch(this->id()){
        case _imgo_:

            CAM_WRITE_REG(this->m_pDrv,CAM_FBC_IMGO_CTL1,0);
            //que mode
            CAM_WRITE_BITS(this->m_pDrv,CAM_FBC_IMGO_CTL1,FBC_MODE,1);
            break;
        case _rrzo_:

            CAM_WRITE_REG(this->m_pDrv,CAM_FBC_RRZO_CTL1,0);
            CAM_WRITE_BITS(this->m_pDrv,CAM_FBC_RRZO_CTL1,FBC_MODE,1);
            break;
        case _ufeo_:
            CAM_WRITE_REG(this->m_pDrv,CAM_FBC_UFEO_CTL1,0);
            CAM_WRITE_BITS(this->m_pDrv,CAM_FBC_UFEO_CTL1,FBC_MODE,1);
            break;
        case _aao_:

            CAM_WRITE_REG(this->m_pDrv->getPhyObj(),CAM_FBC_AAO_CTL1,0);
            CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),CAM_FBC_AAO_CTL1,FBC_MODE,1);
            break;
        case _pso_:

            CAM_WRITE_REG(this->m_pDrv->getPhyObj(),CAM_FBC_PSO_CTL1,0);
            CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),CAM_FBC_PSO_CTL1,FBC_MODE,1);
            break;
        case _afo_:

            CAM_WRITE_REG(this->m_pDrv->getPhyObj(),CAM_FBC_AFO_CTL1,0);
            CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),CAM_FBC_AFO_CTL1,FBC_MODE,1);
            break;
        case _pdo_:

            CAM_WRITE_REG(this->m_pDrv->getPhyObj(),CAM_FBC_PDO_CTL1,0);
            CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),CAM_FBC_PDO_CTL1,FBC_MODE,1);
            break;
        case _lcso_:

            CAM_WRITE_REG(this->m_pDrv,CAM_FBC_LCSO_CTL1,0);
            CAM_WRITE_BITS(this->m_pDrv,CAM_FBC_LCSO_CTL1,FBC_MODE,1);
            break;
        case _eiso_:

            UNI_WRITE_REG(this->m_pDrv,CAM_UNI_FBC_EISO_A_CTL1,0);
            UNI_WRITE_BITS(this->m_pDrv,CAM_UNI_FBC_EISO_A_CTL1,FBC_MODE,1);
            break;
        case _flko_:

            UNI_WRITE_REG(this->m_pUniDrv,CAM_UNI_FBC_FLKO_A_CTL1,0);
            UNI_WRITE_BITS(this->m_pUniDrv,CAM_UNI_FBC_FLKO_A_CTL1,FBC_MODE,1);
            break;
        case _rsso_:

            UNI_WRITE_REG(this->m_pDrv,CAM_UNI_FBC_RSSO_A_CTL1,0);
            UNI_WRITE_BITS(this->m_pDrv,CAM_UNI_FBC_RSSO_A_CTL1,FBC_MODE,1);
            break;
        default:
            CAM_FUNC_ERR("un-supported DMAO:0x%x\n",this->id());
            return 1;
            break;
    }
    //reset buf cnt
    this->m_buf_cnt = 0;
    return 0;
}

MINT32 CAM_BUF_CTRL::_enable( void* pParam  )
{
    E_ISP_CAM_CQ cq;
    MUINT32 page;
    this->m_pDrv = (ISP_DRV_CAM*)this->m_pIspDrv;
    //dbg log
    this->m_pDrv->getCurObjInfo(&this->m_hwModule,&cq,&page);
    if(pParam != NULL)
        CAM_FUNC_DBG("%s::_enable: subsample:0x%x\n",this->name_Str(),*(MUINT32*)pParam);
    else
        CAM_FUNC_DBG("%s::_enable: subsample:0x%x\n",this->name_Str(),0);
#if 0 //commend out for Dynamic Twin test
    if(CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_TG_VF_CON,VFDATA_EN) == 1){
        CAM_FUNC_ERR("can't enable FBC at streaming\n");
        return 1;
    }
#endif

    switch(this->id()){
        case _imgo_:
            CAM_WRITE_BITS(this->m_pDrv,CAM_FBC_IMGO_CTL1,SUB_RATIO,0);
            CAM_WRITE_BITS(this->m_pDrv,CAM_FBC_IMGO_CTL1,FBC_EN,1);
            #if (FBC_DRPGRP_SW_WORK_AROUND == 1)
            CAM_WRITE_BITS(this->m_pDrv,CAM_CTL_SW_PASS1_DONE,IMGO_FIFO_FULL_DROP,0);
            #else
            CAM_WRITE_BITS(this->m_pDrv,CAM_CTL_SW_PASS1_DONE,IMGO_FIFO_FULL_DROP,1);
            #endif
            break;
        case _rrzo_:
            CAM_WRITE_BITS(this->m_pDrv,CAM_FBC_RRZO_CTL1,SUB_RATIO,0);
            CAM_WRITE_BITS(this->m_pDrv,CAM_FBC_RRZO_CTL1,FBC_EN,1);
            #if (FBC_DRPGRP_SW_WORK_AROUND == 1)
            CAM_WRITE_BITS(this->m_pDrv,CAM_CTL_SW_PASS1_DONE,RRZO_FIFO_FULL_DROP,0);
            #else
            CAM_WRITE_BITS(this->m_pDrv,CAM_CTL_SW_PASS1_DONE,RRZO_FIFO_FULL_DROP,1);
            #endif
            break;
        case _ufeo_:
            CAM_WRITE_BITS(this->m_pDrv,CAM_FBC_UFEO_CTL1,SUB_RATIO,0);
            CAM_WRITE_BITS(this->m_pDrv,CAM_FBC_UFEO_CTL1,FBC_EN,1);
            #if (FBC_DRPGRP_SW_WORK_AROUND == 1)
            CAM_WRITE_BITS(this->m_pDrv,CAM_CTL_SW_PASS1_DONE,UFEO_FIFO_FULL_DROP,0);
            #else
            CAM_WRITE_BITS(this->m_pDrv,CAM_CTL_SW_PASS1_DONE,UFEO_FIFO_FULL_DROP,1);
            #endif
            break;
        case _aao_:
            if(pParam == NULL) {
                CAM_FUNC_ERR("%s: Null pParam\n",this->name_Str());
                break;
            }
            CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),CAM_FBC_AAO_CTL1,SUB_RATIO,*(MUINT32*)pParam);
            CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),CAM_FBC_AAO_CTL1,FBC_EN,1);
            break;
        case _pso_:
            if(pParam == NULL) {
                CAM_FUNC_ERR("%s: Null pParam\n",this->name_Str());
                break;
            }
            CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),CAM_FBC_PSO_CTL1,SUB_RATIO,*(MUINT32*)pParam);
            CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),CAM_FBC_PSO_CTL1,FBC_EN,1);
            break;
        case _afo_:
            if(pParam == NULL) {
                CAM_FUNC_ERR("%s: Null pParam\n",this->name_Str());
                break;
            }
            CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),CAM_FBC_AFO_CTL1,SUB_RATIO,*(MUINT32*)pParam);
            CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),CAM_FBC_AFO_CTL1,FBC_EN,1);
            break;
        case _pdo_:
            CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),CAM_FBC_PDO_CTL1,SUB_RATIO,0);
            CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),CAM_FBC_PDO_CTL1,FBC_EN,1);
            break;
        case _lcso_:
            CAM_WRITE_BITS(this->m_pDrv,CAM_FBC_LCSO_CTL1,SUB_RATIO,0);
            CAM_WRITE_BITS(this->m_pDrv,CAM_FBC_LCSO_CTL1,FBC_EN,1);
            #if (FBC_DRPGRP_SW_WORK_AROUND == 1)
            CAM_WRITE_BITS(this->m_pDrv,CAM_CTL_SW_PASS1_DONE,LCSO_FIFO_FULL_DROP,0);
            #else
            CAM_WRITE_BITS(this->m_pDrv,CAM_CTL_SW_PASS1_DONE,LCSO_FIFO_FULL_DROP,1);
            #endif
            break;
        case _eiso_:
            UNI_WRITE_BITS(this->m_pDrv,CAM_UNI_FBC_EISO_A_CTL1,SUB_RATIO,0);
            UNI_WRITE_BITS(this->m_pDrv,CAM_UNI_FBC_EISO_A_CTL1,FBC_EN,1);
            #if (FBC_DRPGRP_SW_WORK_AROUND == 1)
            CAM_WRITE_BITS(this->m_pDrv,CAM_CTL_SW_PASS1_DONE,EISO_FIFO_FULL_DROP,0);
            #else
            if ((E_EXT_ARG)this->extArg == eExtArg_PhyUni) {
                CAM_WRITE_BITS(this->m_pDrv,CAM_CTL_SW_PASS1_DONE,EISO_FIFO_FULL_DROP,1);
            }
            else {
                CAM_WRITE_BITS(this->m_pDrv,CAM_CTL_SW_PASS1_DONE,EISO_FIFO_FULL_DROP,0);
            }
            #endif
            break;
        case _flko_:
            if(pParam == NULL) {
                CAM_FUNC_ERR("%s: Null pParam\n",this->name_Str());
                break;
            }
            UNI_WRITE_BITS(this->m_pUniDrv,CAM_UNI_FBC_FLKO_A_CTL1,SUB_RATIO,*(MUINT32*)pParam);
            UNI_WRITE_BITS(this->m_pUniDrv,CAM_UNI_FBC_FLKO_A_CTL1,FBC_EN,1);
            break;
        case _rsso_:
            UNI_WRITE_BITS(this->m_pDrv,CAM_UNI_FBC_RSSO_A_CTL1,SUB_RATIO,0);
            UNI_WRITE_BITS(this->m_pDrv,CAM_UNI_FBC_RSSO_A_CTL1,FBC_EN,1);
            #if (FBC_DRPGRP_SW_WORK_AROUND == 1)
            CAM_WRITE_BITS(this->m_pDrv,CAM_CTL_SW_PASS1_DONE,RSSO_FIFO_FULL_DROP,0);
            #else
            if ((E_EXT_ARG)this->extArg == eExtArg_PhyUni) {
                CAM_WRITE_BITS(this->m_pDrv,CAM_CTL_SW_PASS1_DONE,RSSO_FIFO_FULL_DROP,1);
            }
            else {
                CAM_WRITE_BITS(this->m_pDrv,CAM_CTL_SW_PASS1_DONE,RSSO_FIFO_FULL_DROP,0);
            }
            #endif
            break;
        default:
            CAM_FUNC_ERR("un-supported DMAO:0x%x\n",this->id());
            return 1;
            break;
    }


    return 0;
}

MINT32 CAM_BUF_CTRL::_disable( void* pParam )
{
    E_ISP_CAM_CQ cq;
    MUINT32 page;
    (void)pParam;

    this->m_pDrv = (ISP_DRV_CAM*)this->m_pIspDrv;
    //dbg log
    this->m_pDrv->getCurObjInfo(&this->m_hwModule,&cq,&page);

    CAM_FUNC_DBG("%s::_disable: cq:0x%x,page:0x%x\n",this->name_Str(),cq,page);


    if(CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_TG_VF_CON,VFDATA_EN) == 1){
        CAM_FUNC_WRN("%s:disable FBC at streaming\n",this->name_Str());
    }

    switch(this->id()){
        case _imgo_:
            CAM_WRITE_REG(this->m_pDrv,CAM_FBC_IMGO_CTL1,0);
            break;
        case _rrzo_:
            CAM_WRITE_REG(this->m_pDrv,CAM_FBC_RRZO_CTL1,0);
            break;
        case _ufeo_:
            CAM_WRITE_REG(this->m_pDrv,CAM_FBC_UFEO_CTL1,0);
            break;
        case _aao_:
            CAM_WRITE_REG(this->m_pDrv->getPhyObj(),CAM_FBC_AAO_CTL1,0);
            break;
        case _pso_:
            CAM_WRITE_REG(this->m_pDrv->getPhyObj(),CAM_FBC_PSO_CTL1,0);
            break;
        case _afo_:
            CAM_WRITE_REG(this->m_pDrv->getPhyObj(),CAM_FBC_AFO_CTL1,0);
            break;
        case _pdo_:
            CAM_WRITE_REG(this->m_pDrv->getPhyObj(),CAM_FBC_PDO_CTL1,0);
            break;
        case _lcso_:
            CAM_WRITE_REG(this->m_pDrv,CAM_FBC_LCSO_CTL1,0);
            break;
        case _eiso_:
            UNI_WRITE_REG(this->m_pDrv,CAM_UNI_FBC_EISO_A_CTL1,0);
            break;
        case _flko_:
            UNI_WRITE_REG(this->m_pUniDrv,CAM_UNI_FBC_FLKO_A_CTL1,0);
            break;
        case _rsso_:
            UNI_WRITE_REG(this->m_pDrv,CAM_UNI_FBC_RSSO_A_CTL1,0);
            break;
        default:
            CAM_FUNC_ERR("un-supported DMAO:0x%x\n",this->id());
            return 1;
            break;
    }

    return 0;
}

MINT32 CAM_BUF_CTRL::_write2CQ( void )
{
    E_ISP_CAM_CQ cq;
    MUINT32 page;
    this->m_pDrv = (ISP_DRV_CAM*)this->m_pIspDrv;
    //dbg log
    this->m_pDrv->getCurObjInfo(&this->m_hwModule,&cq,&page);
    CAM_FUNC_DBG("%s::_write2CQ: cq:0x%x,page:0x%x,subsample:0x%x\n",this->name_Str(),cq,page,0);

    switch(this->id()){
        case _imgo_:
            this->m_pDrv->cqAddModule(CAM_FBC_IMGO_);
            break;
        case _rrzo_:
            this->m_pDrv->cqAddModule(CAM_FBC_RRZO_);
            break;
        case _ufeo_:
            this->m_pDrv->cqAddModule(CAM_FBC_UFEO_);
            break;
        case _eiso_:
            this->m_pDrv->cqAddModule(UNI_FBC_EISO_);
            break;
        case _rsso_:
            this->m_pDrv->cqAddModule(UNI_FBC_RSSO_);
            break;
        case _lcso_:
            this->m_pDrv->cqAddModule(CAM_FBC_LCSO_);
            break;
        case _flko_:
        case _aao_:
        case _afo_:
        case _pdo_:
        case _pso_:
        default:
            CAM_FUNC_ERR("un-supported DMAO:0x%x\n",this->id());
            return 1;
            break;
    }

    return 0;
}

E_BC_STATUS CAM_BUF_CTRL::enqueueHwBuf( stISP_BUF_INFO& buf_info,MBOOL bImdMode)
{
    ST_BUF_INFO* ptr = NULL;
    E_ISP_CAM_CQ cq;
    MUINT32 page;
    MUINT32 _sof_idx;
    MUINT32 _size;
    ST_BUF_INFO _buf;

    this->m_pDrv = (ISP_DRV_CAM*)this->m_pIspDrv;
    (void)bImdMode;

    Mutex::Autolock lock(this->m_bufctrl_lock);

    //dbg log
    this->m_pDrv->getCurObjInfo(&this->m_hwModule,&cq,&page);

    if ( 0xffffffff == this->id() ) {
        CAM_FUNC_ERR("dma channel error\n");
        return eCmd_Fail;
    }

    //
    this->m_pDrv->getDeviceInfo(_GET_SOF_CNT,(MUINT8 *) &_sof_idx);

#if 0
    //fbc reg dbg log
    this->FBC_STATUS(this->m_pDrv->getPhyObj());
#endif

    //
    if(buf_info.bReplace == MTRUE){
        ptr = &buf_info.Replace;
    }
    else{
        ptr = &buf_info.u_op.enque;
    }

    //check over enque or not
    this->m_Queue.sendCmd(QueueMgr<ST_BUF_INFO>::eCmd_getsize,(MUINTPTR)&_size,0);
    for(MUINT32 i=0;i<_size;i++){
        if(MFALSE == this->m_Queue.sendCmd(QueueMgr<ST_BUF_INFO>::eCmd_at,i,(MUINTPTR)&_buf))
            CAM_FUNC_ERR("%s: get no element at %d\n",this->name_Str(),i);
        else{
            if(_buf.image.mem_info.pa_addr == ptr->image.mem_info.pa_addr){
                CAM_FUNC_ERR("%s: over enque at %d.[addr:0x%" PRIXPTR "]\n",this->name_Str(),i,_buf.image.mem_info.pa_addr);
                return eCmd_Fail;
            }
        }
    }

    //
    this->m_Queue.sendCmd(QueueMgr<ST_BUF_INFO>::eCmd_push,(MUINTPTR)ptr,0);

#if ION_BUFFER_KCTRL
    //ion handle: buffer
    ISP_DEV_ION_NODE_STRUCT IonNode;
    IonNode.devNode = this->m_hwModule;
    IonNode.memID = ptr->image.mem_info.memID;
    IonNode.dmaPort = _DMA_Mapping(this->id());
    this->m_pDrv->setDeviceInfo(_SET_ION_HANDLE,(MUINT8*)&IonNode);
    //ion handle: header
    IonNode.memID = ptr->header.memID;
    IonNode.dmaPort = _DMA_Header_Mapping(this->id());
    this->m_pDrv->setDeviceInfo(_SET_ION_HANDLE,(MUINT8*)&IonNode);
#endif

    //
    this->m_buf_cnt++;
    this->m_tail_idx = (this->m_buf_cnt + 1) % MAX_DEPTH;

    //
    switch(this->id()){
        case _imgo_:
            {
                Header_IMGO fh_imgo;
                fh_imgo.m_hwModule = this->m_hwModule;
                CAM_WRITE_BITS(this->m_pDrv,CAM_CTL_FBC_RCNT_INC,IMGO_RCNT_INC,1);
                //
                fh_imgo.Header_Enque(Header_IMGO::E_ENQUE_SOF,this->m_pDrv,_sof_idx);
                fh_imgo.Header_Enque(Header_IMGO::E_ENQUE_CNT,this->m_pDrv,this->m_buf_cnt);
            }
            break;
        case _rrzo_:
            {
                Header_RRZO fh_rrzo;
                fh_rrzo.m_hwModule = this->m_hwModule;
                CAM_WRITE_BITS(this->m_pDrv,CAM_CTL_FBC_RCNT_INC,RRZO_RCNT_INC,1);
                //
                fh_rrzo.Header_Enque(Header_RRZO::E_ENQUE_SOF,this->m_pDrv,_sof_idx);
                fh_rrzo.Header_Enque(Header_RRZO::E_ENQUE_CNT,this->m_pDrv,this->m_buf_cnt);
            }
            break;
        case _ufeo_:
            {
                Header_UFEO fh_ufeo;
                fh_ufeo.m_hwModule = this->m_hwModule;
                CAM_WRITE_BITS(this->m_pDrv,CAM_CTL_FBC_RCNT_INC,UFEO_RCNT_INC,1);
                //
                fh_ufeo.Header_Enque(Header_UFEO::E_ENQUE_SOF,this->m_pDrv,_sof_idx);
                fh_ufeo.Header_Enque(Header_UFEO::E_ENQUE_CNT,this->m_pDrv,this->m_buf_cnt);
            }
            break;
        case _aao_:
            {
                CQ_RingBuf_ST Buf_ctrl;
                Header_AAO  fh_aao;
                fh_aao.m_hwModule = this->m_hwModule;

                Buf_ctrl.ctrl = GET_RING_DEPTH;
                if(this->m_pDrv->cqRingBuf(&Buf_ctrl) == 0){
                    CAM_FUNC_ERR("%s, can't enque before PA push into CQ\n",this->name_Str());
                    return eCmd_Fail;
                }

                //statistic pipe enque to hw directly
                if(CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_FBC_AAO_CTL1,FBC_EN) != 1){
                    CAM_FUNC_ERR("%s: can't enque before enabe fbc\n",this->name_Str());
                    return eCmd_Fail;
                }
                CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),CAM_CTL_FBC_RCNT_INC,AAO_RCNT_INC,1);
                //
                Buf_ctrl.pDma_fh_spare = NULL;
                Buf_ctrl.ctrl = SET_FH_SPARE;
                Buf_ctrl.dma_PA = ptr->image.mem_info.pa_addr;
                if(this->m_pDrv->cqRingBuf(&Buf_ctrl)) {
                    fh_aao.Header_Enque(Header_AAO::E_ENQUE_SOF,(ISP_DRV_CAM*)Buf_ctrl.pDma_fh_spare,_sof_idx);
                }
            }
            break;
        case _pso_:
            {
                Header_PSO  fh_pso;
                CQ_RingBuf_ST Buf_ctrl;

                Buf_ctrl.ctrl = GET_RING_DEPTH;
                if(this->m_pDrv->cqRingBuf(&Buf_ctrl) == 0){
                    CAM_FUNC_ERR("%s, can't enque before PA push into CQ\n",this->name_Str());
                    return eCmd_Fail;
                }

                //statistic pipe enque to hw directly
                if(CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_FBC_PSO_CTL1,FBC_EN) != 1){
                    CAM_FUNC_ERR("%s: can't enque before enabe fbc\n",this->name_Str());
                    return eCmd_Fail;
                }
                CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),CAM_CTL_FBC_RCNT_INC,PSO_RCNT_INC,1);
                //
                Buf_ctrl.ctrl = SET_FH_SPARE;
                Buf_ctrl.dma_PA = ptr->image.mem_info.pa_addr;
                if (this->m_pDrv->cqRingBuf(&Buf_ctrl)) {
                    fh_pso.Header_Enque(Header_PSO::E_ENQUE_SOF,(ISP_DRV_CAM*)Buf_ctrl.pDma_fh_spare,_sof_idx);
                }
            }
            break;
        case _afo_:
            {
                CQ_RingBuf_ST Buf_ctrl;
                Header_AFO  fh_afo;
                fh_afo.m_hwModule = this->m_hwModule;

                Buf_ctrl.ctrl = GET_RING_DEPTH;
                if(this->m_pDrv->cqRingBuf(&Buf_ctrl) == 0){
                    CAM_FUNC_ERR("%s, can't enque before PA push into CQ\n",this->name_Str());
                    return eCmd_Fail;
                }
                //statistic pipe enque to hw directly
                if(CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_FBC_AFO_CTL1,FBC_EN) != 1){
                    CAM_FUNC_ERR("%s: can't enque before enabe fbc\n",this->name_Str());
                    return eCmd_Fail;
                }

                switch(this->m_hwModule){
                    case CAM_A:
                        CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),CAM_CTL_FBC_RCNT_INC,AFO_A_RCNT_INC,1);
                        break;
                    case CAM_B:
                        CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),CAM_CTL_FBC_RCNT_INC,AFO_B_RCNT_INC,1);
                        break;
                    default:
                        CAM_FUNC_ERR("unsupported module:0x%x\n",this->m_hwModule);
                        break;
                }


                //
                Buf_ctrl.ctrl = SET_FH_SPARE;
                Buf_ctrl.dma_PA = ptr->image.mem_info.pa_addr;
                if(this->m_pDrv->cqRingBuf(&Buf_ctrl)) {
                    fh_afo.Header_Enque(Header_AFO::E_ENQUE_SOF,(ISP_DRV_CAM*)Buf_ctrl.pDma_fh_spare,_sof_idx);
                }
            }
            break;
        case _pdo_:
            {
                CQ_RingBuf_ST Buf_ctrl;
                Header_PDO  fh_pdo;
                fh_pdo.m_hwModule = this->m_hwModule;

                Buf_ctrl.ctrl = GET_RING_DEPTH;
                if(this->m_pDrv->cqRingBuf(&Buf_ctrl) == 0){
                    CAM_FUNC_ERR("%s, can't enque before PA push into CQ\n",this->name_Str());
                    return eCmd_Fail;
                }
                //statistic pipe enque to hw directly
                if(CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_FBC_PDO_CTL1,FBC_EN) != 1){
                    CAM_FUNC_ERR("%s: can't enque before enabe fbc\n",this->name_Str());
                    return eCmd_Fail;
                }
                CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),CAM_CTL_FBC_RCNT_INC,PDO_A_RCNT_INC,1);
                //
                Buf_ctrl.ctrl = SET_FH_SPARE;
                Buf_ctrl.dma_PA = ptr->image.mem_info.pa_addr;
                if(this->m_pDrv->cqRingBuf(&Buf_ctrl)) {
                    fh_pdo.Header_Enque(Header_PDO::E_ENQUE_SOF,(ISP_DRV_CAM*)Buf_ctrl.pDma_fh_spare,_sof_idx);
                }
            }
            break;
        case _lcso_:
            {
                Header_LCSO fh_lcso;
                fh_lcso.m_hwModule = this->m_hwModule;

                if(CAM_READ_BITS(this->m_pDrv,CAM_FBC_LCSO_CTL1,FBC_EN) != 1){
                    CAM_FUNC_ERR("%s: can't enque before enabe fbc\n",this->name_Str());
                    return eCmd_Fail;
                }
                CAM_WRITE_BITS(this->m_pDrv,CAM_CTL_FBC_RCNT_INC,LCSO_RCNT_INC,1);
                //
                fh_lcso.Header_Enque(Header_LCSO::E_ENQUE_SOF,this->m_pDrv,_sof_idx);
                fh_lcso.Header_Enque(Header_LCSO::E_ENQUE_CNT,this->m_pDrv,this->m_buf_cnt);
            }
            break;
        case _eiso_:
            {
                Header_EISO fh_eiso;
                fh_eiso.m_hwModule = this->m_hwModule;

                if(UNI_READ_BITS(this->m_pDrv,CAM_UNI_FBC_EISO_A_CTL1,FBC_EN) != 1){
                    CAM_FUNC_ERR("%s: can't enque before enable fbc\n",this->name_Str());
                    return eCmd_Fail;
                }

                CAM_WRITE_BITS(this->m_pDrv,CAM_CTL_FBC_RCNT_INC,EISO_RCNT_INC,1);
                    //
                fh_eiso.Header_Enque(Header_EISO::E_ENQUE_SOF,this->m_pDrv,_sof_idx);
                fh_eiso.Header_Enque(Header_EISO::E_ENQUE_CNT,this->m_pDrv,this->m_buf_cnt);
            }
            break;
        case _flko_:
            {
                CQ_RingBuf_ST Buf_ctrl;
                Header_FLKO fh_flko;
                fh_flko.m_hwModule = this->m_hwModule;

                Buf_ctrl.ctrl = GET_RING_DEPTH;
                if(this->m_pDrv->cqRingBuf(&Buf_ctrl) == 0){
                    CAM_FUNC_ERR("%s, can't enque before PA push into CQ\n",this->name_Str());
                    return eCmd_Fail;
                }

                //statistic pipe enque to hw directly
                if(UNI_READ_BITS(this->m_pUniDrv,CAM_UNI_FBC_FLKO_A_CTL1,FBC_EN) != 1){
                    CAM_FUNC_ERR("%s: can't enque before enable fbc\n",this->name_Str());
                    return eCmd_Fail;
                }
                CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),CAM_CTL_FBC_RCNT_INC,FLKO_RCNT_INC,1);
                //
                Buf_ctrl.ctrl = SET_FH_SPARE;
                Buf_ctrl.dma_PA = ptr->image.mem_info.pa_addr;
                if(this->m_pDrv->cqRingBuf(&Buf_ctrl)) {
                    fh_flko.Header_Enque(Header_FLKO::E_ENQUE_SOF,(ISP_DRV_CAM*)Buf_ctrl.pDma_fh_spare,_sof_idx);
                }
            }
            break;
        case _rsso_:
            {
                Header_RSSO fh_rsso;
                fh_rsso.m_hwModule = this->m_hwModule;

                if(UNI_READ_BITS(this->m_pDrv,CAM_UNI_FBC_RSSO_A_CTL1,FBC_EN) != 1){
                    CAM_FUNC_ERR("%s: can't enque before enable fbc\n",this->name_Str());
                    return eCmd_Fail;
                }

                CAM_WRITE_BITS(this->m_pDrv,CAM_CTL_FBC_RCNT_INC,RSSO_RCNT_INC,1);
                //
                fh_rsso.Header_Enque(Header_RSSO::E_ENQUE_SOF,this->m_pDrv,_sof_idx);
                fh_rsso.Header_Enque(Header_RSSO::E_ENQUE_CNT,this->m_pDrv,this->m_buf_cnt);
            }
            break;
        default:
            CAM_FUNC_ERR("un-supported DMAO:0x%x\n",this->id());
            this->m_Queue.sendCmd(QueueMgr<ST_BUF_INFO>::eCmd_pop  ,0,0);
            return eCmd_Fail;
            break;
    }

    //
    if(buf_info.bReplace == MFALSE){
        switch(this->id()){ /*this->id()*/
            case _rrzo_:
                CAM_FUNC_DBG("%s:(0x%x_0x%x),PA(0x%" PRIXPTR "_0x%" PRIXPTR "),FH_VA(0x%" PRIXPTR "),size(0x%x),enque_sof(%d)",\
                    this->name_Str(),\
                    cq,page,\
                    ptr->image.mem_info.pa_addr ,\
                    ptr->header.pa_addr ,\
                    ptr->header.va_addr ,\
                    ptr->image.mem_info.size ,\
                    _sof_idx);
                break;
            default:
                CAM_FUNC_DBG("%s:(0x%x_0x%x),PA(0x%" PRIXPTR "_0x%" PRIXPTR "),FH_VA(0x%" PRIXPTR "),size(0x%x),enque_sof(%d)",\
                    this->name_Str(),\
                    cq,page,\
                    ptr->image.mem_info.pa_addr ,\
                    ptr->header.pa_addr ,\
                    ptr->header.va_addr ,\
                    ptr->image.mem_info.size ,\
                    _sof_idx);
                break;
        }
    }
    else{//replace
        switch(this->id()){
            case _imgo_:
            case _rrzo_:
                CAM_FUNC_DBG("%s:(0x%x_0x%x),PA(0x%" PRIXPTR "_0x%" PRIXPTR ",new:0x%" PRIXPTR "_0x%" PRIXPTR "),FH_VA(0x%" PRIXPTR ",new:0x%" PRIXPTR "),size(0x%x),enque_sof(%d)",\
                    this->name_Str(),\
                    cq,page,\
                    ptr->image.mem_info.pa_addr ,\
                    ptr->header.pa_addr ,\
                    buf_info.u_op.enque.image.mem_info.pa_addr ,\
                    buf_info.u_op.enque.header.pa_addr ,\
                    ptr->header.va_addr ,\
                    buf_info.u_op.enque.header.va_addr ,\
                    ptr->image.mem_info.size ,\
                    _sof_idx);
                break;
            default:
                CAM_FUNC_DBG("%s:(0x%x_0x%x),PA(0x%" PRIXPTR "_0x%" PRIXPTR ",new:0x%" PRIXPTR "_0x%" PRIXPTR "),FH_VA(0x%" PRIXPTR ",new:0x%" PRIXPTR "),size(0x%x),enque_sof(%d)",\
                    this->name_Str(),\
                    cq,page,\
                    ptr->image.mem_info.pa_addr ,\
                    ptr->header.pa_addr ,\
                    buf_info.u_op.enque.image.mem_info.pa_addr ,\
                    buf_info.u_op.enque.header.pa_addr ,\
                    ptr->header.va_addr ,\
                    buf_info.u_op.enque.header.va_addr ,\
                    ptr->image.mem_info.size ,\
                    _sof_idx);
                break;
        }
    }


    this->m_Queue_deque_ptr.sendCmd(QueueMgr<MUINTPTR>::eCmd_push,(MUINTPTR)&this->m_pDrv,0);
    //
    return eCmd_Pass;

}


E_BC_STATUS CAM_BUF_CTRL::dequeueHwBuf(NSImageio::NSIspio::BufInfo& buf_info )
{
    E_BC_STATUS ret = eCmd_Pass;
    E_ISP_CAM_CQ cq = ISP_DRV_CQ_NONE;
    MUINT32 page = 0;
    MUINT32 _size;
    MUINT32 _sof_idx = 0;
    ST_BUF_INFO _buf;
    MUINT32 _header_data[E_HEADER_MAX];
    char str[TS_BUF_SIZE] = {'\0'};
    ISP_DRV_CAM* pDrv = NULL;
    MBOOL bheaderchk = MTRUE;   //this is for dummy buf on virtual UNI
    MBOOL bDbgLog = (func_cam_DbgLogEnable_DEBUG)?MTRUE:MFALSE;
    Mutex::Autolock lock(this->m_bufctrl_lock);

    if ( 0xffffffff == this->id() ) {
        CAM_FUNC_ERR("dma channel error ");
        ret = eCmd_Fail;
        goto EXIT;
    }

    this->m_Queue_deque_ptr.sendCmd(QueueMgr<MUINTPTR>::eCmd_getsize  ,(MUINTPTR)&_size,0);
    if (_size == 0) {
        CAM_FUNC_ERR("no enque record, plz enque first");
        return eCmd_Fail;
    }

    this->m_Queue_deque_ptr.sendCmd(QueueMgr<MUINTPTR>::eCmd_front  ,(MUINTPTR)&pDrv,0);

    //dbg log
    if(bDbgLog){
        pDrv->getCurObjInfo(&this->m_hwModule,&cq,&page);
        pDrv->getDeviceInfo(_GET_SOF_CNT,(MUINT8 *) &_sof_idx);
    }

#if 0
    //fbc reg dbg log
    this->FBC_STATUS(pDrv->getPhyObj());
#endif

    //
    this->m_Queue.sendCmd(QueueMgr<ST_BUF_INFO>::eCmd_getsize,(MUINTPTR)&_size,0);
    if(_size <1){
        CAM_FUNC_ERR("queue depth is empty for deque\n");
        ret = eCmd_Fail;
        goto EXIT;
    }

    //
    this->m_Queue.sendCmd(QueueMgr<ST_BUF_INFO>::eCmd_front  ,(MUINTPTR)&_buf,0);

    switch(CAM_BUF_CTRL::m_CamState[this->m_hwModule]){
        case CAM_BUF_CTRL::eCamState_runnable:
        case CAM_BUF_CTRL::eCamState_suspend:   //this timing is for twin deque is happaned after dynamic twin suspend ready
            if(bheaderchk){
                //
                this->Header_Parser(_buf.header.va_addr, buf_info,_header_data,(bDbgLog)?str:NULL);
                //
                if( _buf.image.mem_info.pa_addr != buf_info.u4BufPA[0]){
                    CAM_FUNC_ERR("%s:PA in header is mismatch with PA in list [0x%" PRIXPTR "_0x%" PRIXPTR "]\n",this->name_Str(),\
                        buf_info.u4BufPA[0],_buf.image.mem_info.pa_addr);
                    ret = eCmd_Fail;
                    this->PipeCheck();
                    goto EXIT;
                }
            }
            break;
        case CAM_BUF_CTRL::eCamState_suspending:
            ret = eCmd_Suspending_Pass;

            switch(this->id()){
            case _aao_:
            case _afo_:
            case _pdo_:
            case _flko_:
            case _pso_:
                //stt will return fail during suspending
                return ret;
            default:
                break;
            }

            //do not check header,dram on header's address is garbage!
            //need to pop all requests in driver when suspending for the drop method in iopipe.

            //Flush FH
            this->Header_Parser(_buf.header.va_addr, buf_info,_header_data,(bDbgLog)?str:NULL);

            //asign this maigc number is for drop request back to MW
            buf_info.m_num = _buf.image.mem_info.magicIdx;

            CAM_FUNC_INF("pop request(%d) in driver under status:%d\n",buf_info.m_num,CAM_BUF_CTRL::m_CamState[this->m_hwModule]);

            break;
        case CAM_BUF_CTRL::eCamState_stop:
            //do not check header,dram on header's address is garbage.
            //need to pop all requests in driver when suspending for the drop method in iopipe.
            CAM_FUNC_WRN("pop request in driver under status:%d\n",CAM_BUF_CTRL::m_CamState[this->m_hwModule]);
            ret = eCmd_Stop_Pass;
            break;
        default:
            CAM_FUNC_ERR("Unknown state: %d\n", CAM_BUF_CTRL::m_CamState[this->m_hwModule]);
            ret = eCmd_Fail;
            goto EXIT;
            break;
    }
    //
    buf_info.memID[0]          = _buf.image.mem_info.memID;
    buf_info.u4BufSize[0]      = _buf.image.mem_info.size;
    buf_info.u4BufVA[0]        = _buf.image.mem_info.va_addr;
    buf_info.u4BufPA[0]        = _buf.image.mem_info.pa_addr;
    buf_info.bufCohe[0]        = _buf.image.mem_info.bufCohe;
    buf_info.bufSecu[0]        = _buf.image.mem_info.bufSecu;

    buf_info.Frame_Header.u4BufPA   = _buf.header.pa_addr;
    buf_info.Frame_Header.u4BufVA   = _buf.header.va_addr;
    buf_info.Frame_Header.u4BufSize = _buf.header.size;
    buf_info.Frame_Header.memID     = _buf.header.memID;
    buf_info.Frame_Header.bufCohe   = _buf.header.bufCohe;
    buf_info.Frame_Header.bufSecu   = _buf.header.bufSecu;
    //
    this->m_Queue.sendCmd(QueueMgr<ST_BUF_INFO>::eCmd_pop  ,0,0);
    this->m_Queue_deque_ptr.sendCmd(QueueMgr<MUINTPTR>::eCmd_pop  ,0,0);

    this->m_buf_cnt--;

#if ION_BUFFER_KCTRL
    //ion handle: buffer
    ISP_DEV_ION_NODE_STRUCT IonNode;
    IonNode.devNode = this->m_hwModule;
    IonNode.memID =  _buf.image.mem_info.memID;
    IonNode.dmaPort = _DMA_Mapping(this->id());
    pDrv->setDeviceInfo(_SET_ION_FREE,(MUINT8*)&IonNode);
    //ion handle: header
    IonNode.memID =  _buf.header.memID;
    IonNode.dmaPort = _DMA_Header_Mapping(this->id());
    pDrv->setDeviceInfo(_SET_ION_FREE,(MUINT8*)&IonNode);
#endif

EXIT:
    switch(this->id()){
        case _rrzo_:
            CAM_FUNC_INF("%s:(0x%x_0x%x),PA(0x%" PRIXPTR "_0x%" PRIXPTR "),FH_VA(0x%" PRIXPTR "),size(0x%x),cur sof(%d),%s\n",\
                this->name_Str(),\
                cq,page,\
                _buf.image.mem_info.pa_addr ,\
                _buf.header.pa_addr ,\
                _buf.header.va_addr ,\
                _buf.image.mem_info.size ,\
                _sof_idx,\
                str);
            break;
        default:
            CAM_FUNC_DBG("%s:(0x%x_0x%x),PA(0x%" PRIXPTR "_0x%" PRIXPTR "),FH_VA(0x%" PRIXPTR "),size(0x%x),cur sof(%d),%s\n",\
                this->name_Str(),\
                cq,page,\
                _buf.image.mem_info.pa_addr ,\
                _buf.header.pa_addr ,\
                _buf.header.va_addr ,\
                _buf.image.mem_info.size ,\
                _sof_idx,\
                str);
            break;
    }
    return ret;
}

E_BC_STATUS CAM_BUF_CTRL::dropEnqueueHwBuf(void)
{
    E_BC_STATUS ret = eCmd_Pass;
    MUINT32 _size;

    switch(this->id()){
        case _imgo_:
        case _rrzo_:
        case _ufeo_:
        case _lcso_:
        case _eiso_:
        case _rsso_:
            this->m_Queue_deque_ptr.sendCmd(QueueMgr<MUINTPTR>::eCmd_getsize, (MUINTPTR)&_size, 0);
            CAM_SFUNC_INF("dropEnqueueHwBuf port:%d, m_Queue_deque_ptr size:%d\n", this->id(), _size);
            if (_size > 0) {
                this->m_Queue_deque_ptr.sendCmd(QueueMgr<MUINTPTR>::eCmd_pop_back, 0, 0);
            }
            this->m_Queue.sendCmd(QueueMgr<ST_BUF_INFO>::eCmd_getsize, (MUINTPTR)&_size, 0);
            CAM_SFUNC_INF("dropEnqueueHwBuf port:%d, m_Queue size:%d\n", this->id(), _size);
            if (_size > 0) {
                this->m_Queue.sendCmd(QueueMgr<ST_BUF_INFO>::eCmd_pop_back,0,0);
                this->m_buf_cnt--;
            }
        default:
            break;
    }
    return ret;
}

MUINT32 CAM_BUF_CTRL::estimateTimeout(MUINT32 subSample)
{
    MUINT32 _t_ms = (m_recentFrmTimeMs[this->m_hwModule][0] >= m_recentFrmTimeMs[this->m_hwModule][1])?
        m_recentFrmTimeMs[this->m_hwModule][0]: m_recentFrmTimeMs[this->m_hwModule][1];

    _t_ms *= subSample;

    _t_ms = (MIN_GRPFRM_TIME_MS > _t_ms)? MIN_GRPFRM_TIME_MS: _t_ms;

    _t_ms *= 2; /* x2 for enque post deque sem timing */

    return _t_ms;
}


MBOOL CAM_BUF_CTRL::updateFrameTime(MUINT32 timeInMs, MUINT32 reqOffset, ISP_HW_MODULE camModule)
{
    if (reqOffset >= MAX_RECENT_GRPFRM_TIME) {
        return MFALSE;
    }

    m_recentFrmTimeMs[camModule][reqOffset] = timeInMs;

    return MTRUE;
}

MBOOL CAM_BUF_CTRL::suspend(ISP_HW_MODULE camModule, ISP_DRV_CAM *pDrvCam, UniDrvImp* pUniDrv, E_SUSPEND_OP op)
{
    MBOOL ret;
    ISP_WAIT_IRQ_ST irq;
    MUINT32 _t_ms, subSample = 1;
    //CAM_REG_CTL_TWIN_STATUS twin;

    //twin.Raw = CAM_READ_REG(pDrvCam,CAM_CTL_TWIN_STATUS);

    if(op & eSus_HWOFF){
        subSample = CAM_READ_BITS(pDrvCam->getPhyObj(),CAM_CTL_SW_PASS1_DONE,DOWN_SAMPLE_PERIOD) + 1;

        /* estimateTimeout */
        _t_ms = (m_recentFrmTimeMs[camModule][0] >= m_recentFrmTimeMs[camModule][1])?
            m_recentFrmTimeMs[camModule][0]: m_recentFrmTimeMs[camModule][1];
        _t_ms *= subSample;
        _t_ms = (MIN_GRPFRM_TIME_MS > _t_ms)? MIN_GRPFRM_TIME_MS: _t_ms;
        _t_ms *= 2; /* x2 for enque post deque sem timing */

        irq.Clear   = ISP_IRQ_CLEAR_WAIT;
        irq.UserKey = 0;
        irq.St_type = SIGNAL_INT;
        irq.Status  = VS_INT_ST;
        irq.Timeout = _t_ms;

        CAM_SFUNC_INF("[%d] enter suspending, wait one vsync timeout:%d subSample:%d\n", camModule, irq.Timeout, subSample);

        pDrvCam->setDeviceInfo(_SET_VF_OFF,(MUINT8*)&camModule);

        ret = pDrvCam->waitIrq(&irq);
        if (ret == MFALSE) {
            CAM_SFUNC_ERR("[%d] wait vsync when enter suspend fail\n", camModule);
        }
    }

    if(op & eSus_HW_SW_STATE){
        CAM_BUF_CTRL::updateState(CAM_BUF_CTRL::eCamState_suspending, camModule, CAM_BUF_CTRL::eCamDmaType_main);

        //reset FBC to avoid the latest cq_done setting rcnt_inc.
        //resume's enque will be affected by above behavior
        if(CAM_READ_BITS(pDrvCam->getPhyObj(),CAM_FBC_IMGO_CTL1,FBC_EN)) {
            CAM_WRITE_BITS(pDrvCam->getPhyObj(),CAM_FBC_IMGO_CTL1,FBC_RESET,1);
        }
        if(CAM_READ_BITS(pDrvCam->getPhyObj(),CAM_FBC_RRZO_CTL1,FBC_EN)) {
            CAM_WRITE_BITS(pDrvCam->getPhyObj(),CAM_FBC_RRZO_CTL1,FBC_RESET,1);
        }
        if(CAM_READ_BITS(pDrvCam->getPhyObj(),CAM_FBC_UFEO_CTL1,FBC_EN)){
            CAM_WRITE_BITS(pDrvCam->getPhyObj(),CAM_FBC_UFEO_CTL1,FBC_RESET,1);
        }
        if(CAM_READ_BITS(pDrvCam->getPhyObj(),CAM_FBC_LCSO_CTL1,FBC_EN)){
            CAM_WRITE_BITS(pDrvCam->getPhyObj(),CAM_FBC_LCSO_CTL1,FBC_RESET,1);
        }
        if (pUniDrv) {
            if(UNI_READ_BITS(pUniDrv,CAM_UNI_FBC_EISO_A_CTL1,FBC_EN)){
                UNI_WRITE_BITS(pUniDrv,CAM_UNI_FBC_EISO_A_CTL1,FBC_RESET,1);
            }
            if(UNI_READ_BITS(pUniDrv,CAM_UNI_FBC_RSSO_A_CTL1,FBC_EN)){
                UNI_WRITE_BITS(pUniDrv,CAM_UNI_FBC_RSSO_A_CTL1,FBC_RESET,1);
            }
        }

        if(CAM_READ_BITS(pDrvCam->getPhyObj(),CAM_FBC_IMGO_CTL1,FBC_EN)) {
            CAM_WRITE_BITS(pDrvCam->getPhyObj(),CAM_FBC_IMGO_CTL1,FBC_RESET,0);
        }
        if(CAM_READ_BITS(pDrvCam->getPhyObj(),CAM_FBC_RRZO_CTL1,FBC_EN)) {
            CAM_WRITE_BITS(pDrvCam->getPhyObj(),CAM_FBC_RRZO_CTL1,FBC_RESET,0);
        }
        if(CAM_READ_BITS(pDrvCam->getPhyObj(),CAM_FBC_UFEO_CTL1,FBC_EN)){
            CAM_WRITE_BITS(pDrvCam->getPhyObj(),CAM_FBC_UFEO_CTL1,FBC_RESET,0);
        }
        if(CAM_READ_BITS(pDrvCam->getPhyObj(),CAM_FBC_LCSO_CTL1,FBC_EN)){
            CAM_WRITE_BITS(pDrvCam->getPhyObj(),CAM_FBC_LCSO_CTL1,FBC_RESET,0);
        }
        if (pUniDrv) {
            if(UNI_READ_BITS(pUniDrv,CAM_UNI_FBC_EISO_A_CTL1,FBC_EN)){
                UNI_WRITE_BITS(pUniDrv,CAM_UNI_FBC_EISO_A_CTL1,FBC_RESET,0);
            }
            if(UNI_READ_BITS(pUniDrv,CAM_UNI_FBC_RSSO_A_CTL1,FBC_EN)){
                UNI_WRITE_BITS(pUniDrv,CAM_UNI_FBC_RSSO_A_CTL1,FBC_RESET,0);
            }
        }
    }

    if(op & eSus_SIGNAL){
        /* flush irq for dma ports */
        CAM_SFUNC_INF("[%d] flushing signals\n", camModule);

        irq.Clear = ISP_IRQ_CLEAR_WAIT;
        irq.UserKey = 0;
        irq.Timeout = 1000;
        irq.St_type = SIGNAL_INT;
        irq.Status = SW_PASS1_DON_ST;
        pDrvCam->signalIrq(&irq);

        irq.St_type = DMA_INT;

        irq.Status = (AAO_DONE_ST|AFO_DONE_ST|PDO_DONE_ST|FLKO_DONE_ST|PSO_DONE_ST);
        pDrvCam->signalIrq(&irq);

        //no need to signal twin's signal,because state will be switch to eCamState_suspending when slave cam's fbc access!
    }
    return MTRUE;
}

MBOOL CAM_BUF_CTRL::resume(ISP_HW_MODULE camModule, ISP_DRV_CAM *pDrvCam)
{
    if (pDrvCam == NULL) {
        //slave cam is only need to update status.
        CAM_SFUNC_INF("bypass slave cam\n");
        goto EXIT;
    }
#if (TEMP_SW_TIMESTAMP == 0)
    {
        CAM_TIMESTAMP* pTime = CAM_TIMESTAMP::getInstance(camModule,(const ISP_DRV_CAM*)pDrvCam);
        pTime->TimeStamp_reset();
    }
#endif
    pDrvCam->setDeviceInfo(_SET_VF_ON,(MUINT8*)&camModule);

EXIT:
    CAM_BUF_CTRL::updateState(CAM_BUF_CTRL::eCamState_runnable, camModule, CAM_BUF_CTRL::eCamDmaType_main);

    return MTRUE;
}

MBOOL CAM_BUF_CTRL::updateState(CAM_BUF_CTRL::E_CAM_STATE camState, ISP_HW_MODULE camModule, E_CAM_DMA_TYPE dmaType)
{
    E_CAM_STATE *pStates = NULL;

    if (camState >= CAM_BUF_CTRL::eCamState_max) {
        CAM_SFUNC_ERR("Wrong camera state: %d\n", camState);
        return MFALSE;
    }

    //virtual cam is not supported due to waitbufready()!
    //only phy cam is suppoeted at waitbufready()
    switch (camModule) {
    case CAM_A:
    case CAM_B:
        break;
    default:
        CAM_SFUNC_ERR("Unkown moduel id: %d\n", camModule);
        return MFALSE;
    }

    switch (dmaType) {
    case eCamDmaType_stt:
        pStates = &(CAM_BUF_CTRL::m_SttState[0]);
        break;
    case eCamDmaType_main:
    default:
        pStates = &(CAM_BUF_CTRL::m_CamState[0]);
        break;
    }
    pStates[camModule] = camState;


    return MTRUE;
}

void BUF_CTRL_IMGO::FBC_STATUS(IspDrv* ptr)
{
    CAM_FUNC_DBG("BUF_CTRL_IMGO::FBC_STATUS: 0x%x_0x%x",CAM_READ_REG(ptr,CAM_FBC_IMGO_CTL1),CAM_READ_REG(ptr,CAM_FBC_IMGO_CTL2));
}


void BUF_CTRL_IMGO::Header_Parser(MUINTPTR fh_va,NSImageio::NSIspio::BufInfo& buf_info,MUINT32* header_data ,char* pStr)
{
    Header_IMGO fh_imgo;
    ISP_DRV_CAM* pDrv = NULL;

    this->m_Queue_deque_ptr.sendCmd(QueueMgr<MUINTPTR>::eCmd_front  ,(MUINTPTR)&pDrv,0);
    //
    fh_imgo.Header_Deque(fh_va,(void*)header_data);
    fh_imgo.Header_Flash(fh_va);
    //
    buf_info.u4BufPA[0] = header_data[Header_IMGO::E_IMG_PA];
    buf_info.m_num = header_data[Header_IMGO::E_Magic];
    buf_info.img_fmt = header_data[Header_IMGO::E_FMT];        //hw define, need to transfer to user-reconized enum in camiopipe
    buf_info.img_pxl_id = header_data[Header_IMGO::E_PIX_ID];  //hw define, need to transfer to user-reconized enum in camiopipe
    buf_info.frm_cnt = 0;

    buf_info.img_w = ((header_data[Header_IMGO::E_SRC_SIZE] & 0xffff) >> (header_data[Header_IMGO::E_SRC_SIZE]>>31));
    buf_info.img_h = (header_data[Header_IMGO::E_SRC_SIZE]>>16)&0x7ffff;

    buf_info.crop_win.p.x = (header_data[Header_IMGO::E_CROP_START]& 0xffff);
    buf_info.crop_win.p.y = (header_data[Header_IMGO::E_CROP_START] >> 16);
    buf_info.crop_win.s.w = (header_data[Header_IMGO::E_CROP_SIZE]& 0xffff);
    buf_info.crop_win.s.h = (header_data[Header_IMGO::E_CROP_SIZE] >> 16);

    buf_info.DstSize.w = buf_info.crop_win.s.w;
    buf_info.DstSize.h = buf_info.crop_win.s.h;

    buf_info.i4TimeStamp_sec = header_data[Header_IMGO::E_TimeStamp]; //per-frame clock count

    //
#if TEMP_SW_TIMESTAMP   //under twin mode, only main-path have to wait SW timestamp
    //only cam_a need to get sw timestamp, cam_b can't get timettamp in order to avoid sw hang!!
    //and this temp flow can't support DMAO output with CAM_B's only!!!!!!!
    if(CAM_READ_BITS(pDrv,CAM_CTL_TWIN_STATUS,TWIN_EN)){
        CAM_FUNC_WRN("need to change SW timestamp to HW timestamp after SB\n");

        if(CAM_READ_BITS(pDrv,CAM_CTL_TWIN_STATUS,TWIN_MODULE) != this->m_hwModule){
            this->m_TimeStamp.TimeStamp_mapping(&buf_info.i4TimeStamp_sec,&buf_info.i4TimeStamp_us,&buf_info.i4TimeStamp_sec_B,&buf_info.i4TimeStamp_us_B,
                (_isp_dma_enum_)this->id(),this->m_hwModule);
        }
    }
    else{
        this->m_TimeStamp.TimeStamp_mapping(&buf_info.i4TimeStamp_sec,&buf_info.i4TimeStamp_us,&buf_info.i4TimeStamp_sec_B,&buf_info.i4TimeStamp_us_B,
            (_isp_dma_enum_)this->id(),this->m_hwModule);
    }
#else
    this->m_pTimeStamp->TimeStamp_mapping(&buf_info.i4TimeStamp_sec,&buf_info.i4TimeStamp_us,&buf_info.i4TimeStamp_sec_B,&buf_info.i4TimeStamp_us_B,
        (_isp_dma_enum_)this->id(),this->m_hwModule);
#endif

    buf_info.raw_type = header_data[Header_IMGO::E_RAW_TYPE];  //hw define, need to transfer to user-reconized enum in camiopipe
    buf_info.xoffset = 0;
    buf_info.yoffset = 0;
    //
    if(pStr){
        snprintf(pStr, TS_BUF_SIZE, "enq_sof:0x%x,mag:0x%x,timestamp:%u.%u.%u.%u,RawType:0x%x",\
            header_data[Header_IMGO::E_ENQUE_SOF],\
            header_data[Header_IMGO::E_Magic], \
            buf_info.i4TimeStamp_sec, \
            buf_info.i4TimeStamp_us,\
            buf_info.i4TimeStamp_sec_B,\
            buf_info.i4TimeStamp_us_B,\
            header_data[Header_IMGO::E_RAW_TYPE]);
    }
}



void BUF_CTRL_RRZO::FBC_STATUS(IspDrv* ptr)
{
    CAM_FUNC_DBG("BUF_CTRL_RRZO::FBC_STATUS: 0x%x_0x%x",CAM_READ_REG(ptr,CAM_FBC_RRZO_CTL1),CAM_READ_REG(ptr,CAM_FBC_RRZO_CTL2));
}

void BUF_CTRL_RRZO::Header_Parser(MUINTPTR fh_va,NSImageio::NSIspio::BufInfo& buf_info,MUINT32* header_data ,char* pStr)
{
    Header_RRZO fh_rrzo;
    ISP_DRV_CAM* pDrv = NULL;

    this->m_Queue_deque_ptr.sendCmd(QueueMgr<MUINTPTR>::eCmd_front  ,(MUINTPTR)&pDrv,0);
    //
    fh_rrzo.Header_Deque(fh_va,(void*)header_data);
    fh_rrzo.Header_Flash(fh_va);

    //
    buf_info.u4BufPA[0] = header_data[Header_RRZO::E_IMG_PA];
    buf_info.m_num = header_data[Header_RRZO::E_Magic];
    buf_info.img_fmt = header_data[Header_RRZO::E_FMT];        //hw define, need to transfer to user-reconized enum in camiopipe
    buf_info.img_pxl_id = header_data[Header_RRZO::E_PIX_ID];  //hw define, need to transfer to user-reconized enum in camiopipe
    buf_info.frm_cnt = 0;


    buf_info.crop_win.p.x = (header_data[Header_RRZO::E_RRZ_CRP_START]& 0xffff);
    buf_info.crop_win.p.y = (header_data[Header_RRZO::E_RRZ_CRP_START] >> 16);
    buf_info.crop_win.s.w = (header_data[Header_RRZO::E_RRZ_CRP_SIZE]& 0xffff);
    buf_info.crop_win.s.h = (header_data[Header_RRZO::E_RRZ_CRP_SIZE] >> 16);

    buf_info.DstSize.w = (header_data[Header_RRZO::E_RRZ_DST_SIZE]& 0xffff);
    buf_info.DstSize.h = (header_data[Header_RRZO::E_RRZ_DST_SIZE] >> 16);

    buf_info.i4TimeStamp_sec = header_data[Header_RRZO::E_TimeStamp];

    //
#if TEMP_SW_TIMESTAMP   //under twin mode, only main-path have to wait SW timestamp
    //only cam_a need to get sw timestamp, cam_b can't get timettamp in order to avoid sw hang!!
    //and this temp flow can't support DMAO output with CAM_B's only!!!!!!!
    if(CAM_READ_BITS(pDrv,CAM_CTL_TWIN_STATUS,TWIN_EN)){
        CAM_FUNC_WRN("need to change SW timestamp to HW timestamp after SB\n");

        if(CAM_READ_BITS(pDrv,CAM_CTL_TWIN_STATUS,TWIN_MODULE) != this->m_hwModule){
            this->m_TimeStamp.TimeStamp_mapping(&buf_info.i4TimeStamp_sec,&buf_info.i4TimeStamp_us,&buf_info.i4TimeStamp_sec_B,&buf_info.i4TimeStamp_us_B,
                (_isp_dma_enum_)this->id(),this->m_hwModule);
        }
    }
    else{
        this->m_TimeStamp.TimeStamp_mapping(&buf_info.i4TimeStamp_sec,&buf_info.i4TimeStamp_us,&buf_info.i4TimeStamp_sec_B,&buf_info.i4TimeStamp_us_B,
            (_isp_dma_enum_)this->id(),this->m_hwModule);
    }
#else
    this->m_pTimeStamp->TimeStamp_mapping(&buf_info.i4TimeStamp_sec,&buf_info.i4TimeStamp_us,&buf_info.i4TimeStamp_sec_B,&buf_info.i4TimeStamp_us_B,
        (_isp_dma_enum_)this->id(),this->m_hwModule);
#endif

    buf_info.raw_type = 0;
    buf_info.xoffset = 0;
    buf_info.yoffset = 0;

    //
    if(pStr){
        snprintf(pStr, TS_BUF_SIZE, "enq_sof:0x%x,mag:0x%x,timestamp:%u.%u.%u.%u",\
            header_data[Header_RRZO::E_ENQUE_SOF],\
            header_data[Header_RRZO::E_Magic], \
            buf_info.i4TimeStamp_sec, \
            buf_info.i4TimeStamp_us, \
            buf_info.i4TimeStamp_sec_B,\
            buf_info.i4TimeStamp_us_B);
    }
}

void BUF_CTRL_UFEO::FBC_STATUS(IspDrv* ptr)
{
    CAM_FUNC_DBG("BUF_CTRL_UFEO::FBC_STATUS: 0x%x_0x%x",CAM_READ_REG(ptr,CAM_FBC_UFEO_CTL1),CAM_READ_REG(ptr,CAM_FBC_UFEO_CTL2));
}

void BUF_CTRL_UFEO::Header_Parser(MUINTPTR fh_va,NSImageio::NSIspio::BufInfo& buf_info,MUINT32* header_data ,char* pStr)
{
    Header_UFEO fh_ufeo;

    CAM_REG_TG_SEN_GRAB_PXL TG_W;
    CAM_REG_TG_SEN_GRAB_LIN TG_H;
    //
    fh_ufeo.Header_Deque(fh_va,(void*)header_data);
    fh_ufeo.Header_Flash(fh_va);
    //
    buf_info.u4BufPA[0] = header_data[Header_UFEO::E_IMG_PA];
    buf_info.m_num = header_data[Header_UFEO::E_Magic];
    buf_info.img_fmt = header_data[Header_UFEO::E_FMT];        //hw define, need to transfer to user-reconized enum in camiopipe
    //buf_info.img_pxl_id = header_data[Header_UFEO::E_PIX_ID];  //hw define, need to transfer to user-reconized enum in camiopipe
    buf_info.frm_cnt = 0;

    TG_W.Raw = CAM_READ_REG(this->m_pDrv->getPhyObj(),CAM_TG_SEN_GRAB_PXL);
    TG_H.Raw = CAM_READ_REG(this->m_pDrv->getPhyObj(),CAM_TG_SEN_GRAB_LIN);
    buf_info.img_w = TG_W.Bits.PXL_E - TG_W.Bits.PXL_S;
    buf_info.img_h = TG_H.Bits.LIN_E - TG_H.Bits.LIN_S;

    buf_info.i4TimeStamp_sec = header_data[Header_UFEO::E_TimeStamp];

#if TEMP_SW_TIMESTAMP
    this->m_TimeStamp.TimeStamp_mapping(&buf_info.i4TimeStamp_sec,&buf_info.i4TimeStamp_us,&buf_info.i4TimeStamp_sec_B,&buf_info.i4TimeStamp_us_B,
        (_isp_dma_enum_)this->id(),this->m_hwModule);
#else
    this->m_pTimeStamp->TimeStamp_mapping(&buf_info.i4TimeStamp_sec,&buf_info.i4TimeStamp_us,&buf_info.i4TimeStamp_sec_B,&buf_info.i4TimeStamp_us_B,
        (_isp_dma_enum_)this->id(),this->m_hwModule);
#endif

    buf_info.raw_type = 0;
    buf_info.xoffset = 0;
    buf_info.yoffset = 0;

    //
    if(pStr){
        snprintf(pStr, TS_BUF_SIZE, "enq_sof:0x%x,mag:0x%x,timestamp:%u.%u.%u.%u",\
            header_data[Header_UFEO::E_ENQUE_SOF],\
            header_data[Header_UFEO::E_Magic], \
            buf_info.i4TimeStamp_sec, \
            buf_info.i4TimeStamp_us, \
            buf_info.i4TimeStamp_sec_B,\
            buf_info.i4TimeStamp_us_B);
    }
}


void BUF_CTRL_LCSO::FBC_STATUS(IspDrv* ptr)
{
    CAM_FUNC_DBG("BUF_CTRL_LCSO::FBC_STATUS: 0x%x_0x%x",CAM_READ_REG(ptr,CAM_FBC_LCSO_CTL1),CAM_READ_REG(ptr,CAM_FBC_LCSO_CTL2));
}

void BUF_CTRL_LCSO::Header_Parser(MUINTPTR fh_va,NSImageio::NSIspio::BufInfo& buf_info,MUINT32* header_data ,char* pStr)
{
    Header_LCSO fh_lcso;
    //
    fh_lcso.Header_Deque(fh_va,(void*)header_data);
    fh_lcso.Header_Flash(fh_va);
    //
    buf_info.u4BufPA[0] = header_data[Header_LCSO::E_IMG_PA];
    buf_info.m_num = header_data[Header_LCSO::E_Magic];

    buf_info.i4TimeStamp_sec = header_data[Header_LCSO::E_TimeStamp];

#if TEMP_SW_TIMESTAMP
    this->m_TimeStamp.TimeStamp_mapping(&buf_info.i4TimeStamp_sec,&buf_info.i4TimeStamp_us,&buf_info.i4TimeStamp_sec_B,&buf_info.i4TimeStamp_us_B,
        (_isp_dma_enum_)this->id(),this->m_hwModule);
#else
    this->m_pTimeStamp->TimeStamp_mapping(&buf_info.i4TimeStamp_sec,&buf_info.i4TimeStamp_us,&buf_info.i4TimeStamp_sec_B,&buf_info.i4TimeStamp_us_B,
        (_isp_dma_enum_)this->id(),this->m_hwModule);
#endif

    //
    if(pStr){
        snprintf(pStr, TS_BUF_SIZE, "enq_sof:0x%x,mag:0x%x,timestamp:%u.%u.%u.%u",\
            header_data[Header_LCSO::E_ENQUE_SOF],\
            header_data[Header_LCSO::E_Magic], \
            buf_info.i4TimeStamp_sec, \
            buf_info.i4TimeStamp_us, \
            buf_info.i4TimeStamp_sec_B,\
            buf_info.i4TimeStamp_us_B);
    }
}

void BUF_CTRL_EISO::FBC_STATUS(IspDrv* ptr)
{
    (void)ptr;
    CAM_FUNC_DBG("BUF_CTRL_EISO::FBC_STATUS: 0x%x_0x%x",UNI_READ_REG(this->m_pUniDrv,CAM_UNI_FBC_EISO_A_CTL1),UNI_READ_REG(this->m_pUniDrv,CAM_UNI_FBC_EISO_A_CTL2));
}

void BUF_CTRL_EISO::Header_Parser(MUINTPTR fh_va,NSImageio::NSIspio::BufInfo& buf_info,MUINT32* header_data ,char* pStr)
{
    Header_EISO fh_eiso;
    //
    fh_eiso.Header_Deque(fh_va,(void*)header_data);
    fh_eiso.Header_Flash(fh_va);
    //
    buf_info.u4BufPA[0] = header_data[Header_EISO::E_IMG_PA];
    buf_info.m_num = header_data[Header_EISO::E_Magic];

    buf_info.i4TimeStamp_sec = header_data[Header_EISO::E_TimeStamp];

#if TEMP_SW_TIMESTAMP
    this->m_TimeStamp.TimeStamp_mapping(&buf_info.i4TimeStamp_sec,&buf_info.i4TimeStamp_us,&buf_info.i4TimeStamp_sec_B,&buf_info.i4TimeStamp_us_B,
        (_isp_dma_enum_)this->id(),this->m_hwModule);
#else
    this->m_pTimeStamp->TimeStamp_mapping(&buf_info.i4TimeStamp_sec,&buf_info.i4TimeStamp_us,&buf_info.i4TimeStamp_sec_B,&buf_info.i4TimeStamp_us_B,
        (_isp_dma_enum_)this->id(),this->m_hwModule);
#endif
    //
    if(pStr){
        snprintf(pStr, TS_BUF_SIZE, "enq_sof:0x%x,mag:0x%x,timestamp:%u.%u.%u.%u",\
            header_data[Header_EISO::E_ENQUE_SOF],\
            header_data[Header_EISO::E_Magic], \
            buf_info.i4TimeStamp_sec, \
            buf_info.i4TimeStamp_us, \
            buf_info.i4TimeStamp_sec_B,\
            buf_info.i4TimeStamp_us_B);
    }
}

void BUF_CTRL_RSSO::FBC_STATUS(IspDrv* ptr)
{
    (void)ptr;
    CAM_FUNC_DBG("BUF_CTRL_RSSO::FBC_STATUS: 0x%x_0x%x",UNI_READ_REG(this->m_pUniDrv,CAM_UNI_FBC_RSSO_A_CTL1),UNI_READ_REG(this->m_pUniDrv,CAM_UNI_FBC_RSSO_A_CTL2));
}

void BUF_CTRL_RSSO::Header_Parser(MUINTPTR fh_va,NSImageio::NSIspio::BufInfo& buf_info,MUINT32* header_data ,char* pStr)
{
    Header_RSSO fh_rsso;
    //
    fh_rsso.Header_Deque(fh_va,(void*)header_data);
    fh_rsso.Header_Flash(fh_va);
    //
    buf_info.u4BufPA[0] = header_data[Header_RSSO::E_IMG_PA];
    buf_info.m_num = header_data[Header_RSSO::E_Magic];

    buf_info.i4TimeStamp_sec = header_data[Header_RSSO::E_TimeStamp];

#if TEMP_SW_TIMESTAMP
    this->m_TimeStamp.TimeStamp_mapping(&buf_info.i4TimeStamp_sec,&buf_info.i4TimeStamp_us,&buf_info.i4TimeStamp_sec_B,&buf_info.i4TimeStamp_us_B,
        (_isp_dma_enum_)this->id(),this->m_hwModule);
#else
    this->m_pTimeStamp->TimeStamp_mapping(&buf_info.i4TimeStamp_sec,&buf_info.i4TimeStamp_us,&buf_info.i4TimeStamp_sec_B,&buf_info.i4TimeStamp_us_B,
        (_isp_dma_enum_)this->id(),this->m_hwModule);
#endif
    buf_info.DstSize.w = (header_data[Header_RSSO::E_DST_SIZE] & 0xFFFF);
    buf_info.DstSize.h = (header_data[Header_RSSO::E_DST_SIZE] >> 16);
    //
    if(pStr){
        snprintf(pStr, TS_BUF_SIZE, "enq_sof:0x%x,mag:0x%x,timestamp:%u.%u.%u.%u",\
            header_data[Header_RSSO::E_ENQUE_SOF],\
            header_data[Header_RSSO::E_Magic], \
            buf_info.i4TimeStamp_sec, \
            buf_info.i4TimeStamp_us, \
            buf_info.i4TimeStamp_sec_B,\
            buf_info.i4TimeStamp_us_B);
    }
}

#define CHECK_STT_STOP_ST(_dmaName, _str)       do { \
        if (CAM_BUF_CTRL::m_SttState[this->m_hwModule] == CAM_BUF_CTRL::eCamState_stop) { \
            CAM_FUNC_INF("%s fast stt stop %s\n",(_dmaName),(_str)); /* stt stop before main*/ \
            return eCmd_Stop_Pass; \
        } \
    } while (0)

E_BC_STATUS BUF_CTRL_AAO::waitBufReady(CAM_STATE_NOTIFY *pNotify)
{
#define _LOOP_  (5)
    E_BC_STATUS ret = eCmd_Pass;
    E_ISP_CAM_CQ cq;
    MUINT32 page;

    ISP_WAIT_IRQ_ST irq;
    MUINT32 loopCnt = _LOOP_;
    char str[128] = {'\0'};

    MUINT32 FbcCnt, _buf_cnt;
    MUINT32 subSample;


    this->m_pDrv = (ISP_DRV_CAM*)this->m_pIspDrv;

    //dbg log
    this->m_pDrv->getCurObjInfo(&this->m_hwModule,&cq,&page);
    if ( 0xffffffff == this->id() ) {
        CAM_FUNC_ERR("dma channel error \n");
        return eCmd_Fail;
    }

#if 0
    //fbc reg dbg log
    this->FBC_STATUS(this->m_pDrv->getPhyObj());
#endif

    switch (CAM_BUF_CTRL::m_CamState[this->m_hwModule]) {
        //theoratically, it no need to reference normalpipe's status under sttpipe.
        //but due to suspend/resume is only implemented in noramlpipe, so sttpipe need to use suspend/resume status.
        case CAM_BUF_CTRL::eCamState_suspending:
            return eCmd_Suspending_Pass;
            break;
        default:
            break;
    }

    subSample = CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_CTL_SW_PASS1_DONE,DOWN_SAMPLE_PERIOD) + 1;

    irq.Clear = ISP_IRQ_CLEAR_WAIT;
    irq.UserKey = 0;
    irq.St_type = DMA_INT;
    irq.Status = (MUINT32)AAO_DONE_ST;

    do{
        //irq.Timeout = ((1000 + (this->m_fps - 1)) / this->m_fps) * subSample;
        //irq.Timeout *= 4; //amplify 3 times is for the case of enque latch timing
        irq.Timeout = this->estimateTimeout(subSample);

        this->m_bufctrl_lock.lock();
        _buf_cnt = this->m_buf_cnt;
        this->m_bufctrl_lock.unlock();

        if(_buf_cnt != 0){

            //
            IspFunction_B::fbcPhyLock_AAO[this->m_hwModule].lock();
            FbcCnt = CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_FBC_AAO_CTL2,FBC_CNT);
            IspFunction_B::fbcPhyLock_AAO[this->m_hwModule].unlock();

            if(FbcCnt< _buf_cnt){
#if TEMP_SW_TIMESTAMP
                if(loopCnt == _LOOP_){
                    // for SW timestamp queue ready
                    MUINT32 waitOps[3];

                    waitOps[0] = _cam_max_;
                    waitOps[1] = this->id();

                    if (this->m_pDrv->getDeviceInfo(_GET_START_TIME, (MUINT8*)waitOps) == MFALSE) {
                        CAM_FUNC_ERR("Wait timestamp ready fail\n");
                    }

                    CAM_FUNC_DBG("dma:%s already have avail_buf on dram, bWaitBufReady  = 0\n",this->name_Str());
                    return eCmd_Pass;
                }
                else{
                    ret = eCmd_Pass;
                    goto EXIT;
                }
#else
                ret = eCmd_Pass;
                goto EXIT;
#endif
            }
            else{
                if(loopCnt == _LOOP_)
                    snprintf(str, sizeof(str), "dma:%s start wait:[enque record:0x%x_0x%x],",this->name_Str(),this->m_buf_cnt,FbcCnt);
                else{
                    char _tmp[16];
                    snprintf(_tmp, sizeof(_tmp), "[%d]time,",((_LOOP_-loopCnt)+1));
                    strncat(str,_tmp, (sizeof(str) - strlen(str)) - 1);
                }
            }
        }
        else{
            if(loopCnt == _LOOP_){
                snprintf(str, sizeof(str), "dma:%s bWaitBufReady = 1,wait for sw enque[0x%x],timeout:%d x %d ms,[1] ",this->name_Str(),this->m_buf_cnt,irq.Timeout,loopCnt);
            }
            else{//save how many times of wait_irq before sw enque
                char _tmp[16];
                snprintf(_tmp, sizeof(_tmp), "[%d],",((_LOOP_-loopCnt)+1)+1);
                strncat(str,_tmp, (sizeof(str) - strlen(str)) - 1);
            }
        }

        //
        if (pNotify) {
            pNotify->fpNotifyState(CAM_ENTER_WAIT, pNotify->Obj);
        }

        if(this->m_pDrv->waitIrq(&irq) == MFALSE) {
            ret = eCmd_Fail;
        } else {
            ret = eCmd_Pass;
        }

        if (pNotify) {
            pNotify->fpNotifyState(CAM_EXIT_WAIT, pNotify->Obj);
        }

        switch (CAM_BUF_CTRL::m_CamState[this->m_hwModule]) {
            //theoratically, it no need to reference normalpipe's status under sttpipe.
            //but due to suspend/resume is only implemented in noramlpipe, so sttpipe need to use suspend/resume status.
            case eCamState_suspending:
                return eCmd_Suspending_Pass;
                break;
            default:
                break;
        }
        CHECK_STT_STOP_ST(this->name_Str(),str);

        //
        irq.Clear = ISP_IRQ_CLEAR_NONE;
        //
        if(ret == eCmd_Pass)
            break;

    }while(--loopCnt > 0);
EXIT:

    if(ret == eCmd_Fail){
        if(_buf_cnt != 0){
            CAM_FUNC_ERR("%s, waitbufready fail. start fail check\n",str);
            this->PipeCheck();
        }
        else{
            CAM_FUNC_ERR("%s, waitbufready fail is caused by no enque\n",str);
        }
    }
    else{
        CAM_FUNC_DBG("%s, waitbufready pass\n",str);
    }
    return ret;
}


void BUF_CTRL_AAO::FBC_STATUS(IspDrv* ptr)
{
    CAM_FUNC_DBG("BUF_CTRL_AAO::FBC_STATUS: 0x%x_0x%x",CAM_READ_REG(ptr,CAM_FBC_AAO_CTL1),CAM_READ_REG(ptr,CAM_FBC_AAO_CTL2));
}

void BUF_CTRL_AAO::Header_Parser(MUINTPTR fh_va,NSImageio::NSIspio::BufInfo& buf_info,MUINT32* header_data ,char* pStr)
{
    Header_AAO  fh_aao;
    //
    fh_aao.Header_Deque(fh_va,(void*)header_data);
    fh_aao.Header_Flash(fh_va);
    //
    buf_info.u4BufPA[0] = header_data[Header_AAO::E_IMG_PA];
    buf_info.m_num = header_data[Header_AAO::E_Magic];

    buf_info.i4TimeStamp_sec = header_data[Header_AAO::E_TimeStamp];

#if TEMP_SW_TIMESTAMP
    this->m_TimeStamp.TimeStamp_mapping(&buf_info.i4TimeStamp_sec,&buf_info.i4TimeStamp_us,&buf_info.i4TimeStamp_sec_B,&buf_info.i4TimeStamp_us_B,
        (_isp_dma_enum_)this->id(),this->m_hwModule);
#else
    this->m_pTimeStamp->TimeStamp_mapping(&buf_info.i4TimeStamp_sec,&buf_info.i4TimeStamp_us,&buf_info.i4TimeStamp_sec_B,&buf_info.i4TimeStamp_us_B,
        (_isp_dma_enum_)this->id(),this->m_hwModule);
#endif
    //
    if(pStr){
        snprintf(pStr, TS_BUF_SIZE, "enq_sof:0x%x,mag:0x%x,timestamp:%u.%u.%u.%u",\
            header_data[Header_AAO::E_ENQUE_SOF],\
            header_data[Header_AAO::E_Magic], \
            buf_info.i4TimeStamp_sec, \
            buf_info.i4TimeStamp_us, \
            buf_info.i4TimeStamp_sec_B,\
            buf_info.i4TimeStamp_us_B);
    }
}

E_BC_STATUS BUF_CTRL_PSO::waitBufReady(CAM_STATE_NOTIFY *pNotify)
{
#define _LOOP_  (5)
    E_BC_STATUS ret = eCmd_Pass;
    E_ISP_CAM_CQ cq;
    MUINT32 page;

    ISP_WAIT_IRQ_ST irq;
    MUINT32 loopCnt = _LOOP_;
    char str[128] = {'\0'};

    MUINT32 FbcCnt, _buf_cnt;
    MUINT32 subSample;


    this->m_pDrv = (ISP_DRV_CAM*)this->m_pIspDrv;

    //dbg log
    this->m_pDrv->getCurObjInfo(&this->m_hwModule,&cq,&page);
    if ( 0xffffffff == this->id() ) {
        CAM_FUNC_ERR("dma channel error \n");
        return eCmd_Fail;
    }

#if 0
    //fbc reg dbg log
    this->FBC_STATUS(this->m_pDrv->getPhyObj());
#endif

    switch (CAM_BUF_CTRL::m_CamState[this->m_hwModule]) {
        //theoratically, it no need to reference normalpipe's status under sttpipe.
        //but due to suspend/resume is only implemented in noramlpipe, so sttpipe need to use suspend/resume status.
        case CAM_BUF_CTRL::eCamState_suspending:
            return eCmd_Suspending_Pass;
            break;
        default:
            break;
    }

    subSample = CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_CTL_SW_PASS1_DONE,DOWN_SAMPLE_PERIOD) + 1;

    irq.Clear = ISP_IRQ_CLEAR_WAIT;
    irq.UserKey = 0;
    irq.St_type = DMA_INT;
    irq.Status = (MUINT32)PSO_DONE_ST;

    do{
        //irq.Timeout = ((1000 + (this->m_fps - 1)) / this->m_fps) * subSample;
        //irq.Timeout *= 4; //amplify 3 times is for the case of enque latch timing
        irq.Timeout = this->estimateTimeout(subSample);

        this->m_bufctrl_lock.lock();
        _buf_cnt = this->m_buf_cnt;
        this->m_bufctrl_lock.unlock();

        if(_buf_cnt != 0){

            //
            IspFunction_B::fbcPhyLock_PSO[this->m_hwModule].lock();
            FbcCnt = CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_FBC_PSO_CTL2,FBC_CNT);
            IspFunction_B::fbcPhyLock_PSO[this->m_hwModule].unlock();

            if(FbcCnt< _buf_cnt){
#if TEMP_SW_TIMESTAMP
                if(loopCnt == _LOOP_){
                    // for SW timestamp queue ready
                    MUINT32 waitOps[3];

                    waitOps[0] = _cam_max_;
                    waitOps[1] = this->id();

                    if (this->m_pDrv->getDeviceInfo(_GET_START_TIME, (MUINT8*)waitOps) == MFALSE) {
                        CAM_FUNC_ERR("Wait timestamp ready fail\n");
                    }

                    CAM_FUNC_DBG("dma:%s already have avail_buf on dram, bWaitBufReady  = 0\n",this->name_Str());
                    return eCmd_Pass;
                }
                else{
                    ret = eCmd_Pass;
                    goto EXIT;
                }
#else
                ret = eCmd_Pass;
                goto EXIT;
#endif
            }
            else{
                if(loopCnt == _LOOP_)
                    snprintf(str, sizeof(str), "dma:%s start wait:[enque record:0x%x_0x%x],",this->name_Str(),this->m_buf_cnt,FbcCnt);
                else{
                    char _tmp[16];
                    snprintf(_tmp, sizeof(_tmp), "[%d]time,",((_LOOP_-loopCnt)+1));
                    strncat(str,_tmp, (sizeof(str) - strlen(str)) - 1);
                }
            }
        }
        else{
            if(loopCnt == _LOOP_){
                snprintf(str, sizeof(str), "dma:%s bWaitBufReady = 1,wait for sw enque[0x%x],timeout:%d x %d ms,[1] ",this->name_Str(),this->m_buf_cnt,irq.Timeout,loopCnt);
            }
            else{//save how many times of wait_irq before sw enque
                char _tmp[16];
                snprintf(_tmp, sizeof(_tmp), "[%d],",((_LOOP_-loopCnt)+1)+1);
                strncat(str,_tmp, (sizeof(str) - strlen(str)) - 1);
            }
        }

        //
        if (pNotify) {
            pNotify->fpNotifyState(CAM_ENTER_WAIT, pNotify->Obj);
        }

        if(this->m_pDrv->waitIrq(&irq) == MFALSE) {
            ret = eCmd_Fail;
        } else {
            ret = eCmd_Pass;
        }

        if (pNotify) {
            pNotify->fpNotifyState(CAM_EXIT_WAIT, pNotify->Obj);
        }

        switch (CAM_BUF_CTRL::m_CamState[this->m_hwModule]) {
            //theoratically, it no need to reference normalpipe's status under sttpipe.
            //but due to suspend/resume is only implemented in noramlpipe, so sttpipe need to use suspend/resume status.
            case CAM_BUF_CTRL::eCamState_suspending:
                return eCmd_Suspending_Pass;
                break;
            default:
                break;
        }
        CHECK_STT_STOP_ST(this->name_Str(),str);
        //
        irq.Clear = ISP_IRQ_CLEAR_NONE;
        //
        if(ret == eCmd_Pass)
            break;
    }while(--loopCnt > 0);
EXIT:

    if(ret == eCmd_Fail){
        if(_buf_cnt != 0){
            CAM_FUNC_ERR("%s, waitbufready fail. start fail check\n",str);
            this->PipeCheck();
        }
        else{
            CAM_FUNC_ERR("%s, waitbufready fail is caused by no enque\n",str);
        }
    }
    else{
        CAM_FUNC_DBG("%s, waitbufready pass\n",str);
    }
    return ret;
}


void BUF_CTRL_PSO::FBC_STATUS(IspDrv* ptr)
{
    CAM_FUNC_DBG("BUF_CTRL_PSO::FBC_STATUS: 0x%x_0x%x",CAM_READ_REG(ptr,CAM_FBC_PSO_CTL1),CAM_READ_REG(ptr,CAM_FBC_PSO_CTL2));
}

void BUF_CTRL_PSO::Header_Parser(MUINTPTR fh_va,NSImageio::NSIspio::BufInfo& buf_info,MUINT32* header_data ,char* pStr)
{
    Header_PSO  fh_pso;
    //
    fh_pso.Header_Deque(fh_va,(void*)header_data);
    fh_pso.Header_Flash(fh_va);

    //
    buf_info.u4BufPA[0] = header_data[Header_PSO::E_IMG_PA];
    buf_info.m_num = header_data[Header_PSO::E_Magic];
    buf_info.raw_type = CAM_READ_BITS(this->m_pDrv->getPhyObj(), CAM_CTL_SEL, QBIN4_SEL);

    buf_info.i4TimeStamp_sec = header_data[Header_PSO::E_TimeStamp];

#if TEMP_SW_TIMESTAMP
    this->m_TimeStamp.TimeStamp_mapping(&buf_info.i4TimeStamp_sec,&buf_info.i4TimeStamp_us,&buf_info.i4TimeStamp_sec_B,&buf_info.i4TimeStamp_us_B,
        (_isp_dma_enum_)this->id(),this->m_hwModule);
#else
    this->m_pTimeStamp->TimeStamp_mapping(&buf_info.i4TimeStamp_sec,&buf_info.i4TimeStamp_us,&buf_info.i4TimeStamp_sec_B,&buf_info.i4TimeStamp_us_B,
        (_isp_dma_enum_)this->id(),this->m_hwModule);
#endif
    //
    if(pStr){
        snprintf(pStr, TS_BUF_SIZE, "enq_sof:0x%x,mag:0x%x,timestamp:%u.%u.%u.%u,bmx_sel:0x%x",\
            header_data[Header_PSO::E_ENQUE_SOF],\
            header_data[Header_PSO::E_Magic], \
            buf_info.i4TimeStamp_sec, \
            buf_info.i4TimeStamp_us, \
            buf_info.i4TimeStamp_sec_B, \
            buf_info.i4TimeStamp_us_B, \
            buf_info.raw_type);
    }
}

void BUF_CTRL_AFO::FBC_STATUS(IspDrv* ptr)
{
    CAM_FUNC_DBG("BUF_CTRL_AFO::FBC_STATUS: 0x%x_0x%x",CAM_READ_REG(ptr,CAM_FBC_AFO_CTL1),CAM_READ_REG(ptr,CAM_FBC_AFO_CTL2));
}

void BUF_CTRL_AFO::Header_Parser(MUINTPTR fh_va,NSImageio::NSIspio::BufInfo& buf_info,MUINT32* header_data ,char* pStr)
{
    Header_AFO  fh_afo;
    //
    fh_afo.Header_Deque(fh_va,(void*)header_data);
    if(CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_CTL_TWIN_STATUS,TWIN_EN)){
        if(this->m_bClear == MTRUE){
            fh_afo.Header_Flash(fh_va);
        }
    }
    else
        fh_afo.Header_Flash(fh_va);
    //
    buf_info.u4BufPA[0] = header_data[Header_AFO::E_IMG_PA];
    buf_info.m_num = header_data[Header_AFO::E_Magic];

    buf_info.i4TimeStamp_sec = header_data[Header_AFO::E_TimeStamp];

#if TEMP_SW_TIMESTAMP
    this->m_TimeStamp.TimeStamp_mapping(&buf_info.i4TimeStamp_sec,&buf_info.i4TimeStamp_us,&buf_info.i4TimeStamp_sec_B,&buf_info.i4TimeStamp_us_B,
        (_isp_dma_enum_)this->id(),this->m_hwModule);
#else
    this->m_pTimeStamp->TimeStamp_mapping(&buf_info.i4TimeStamp_sec,&buf_info.i4TimeStamp_us,&buf_info.i4TimeStamp_sec_B,&buf_info.i4TimeStamp_us_B,
        (_isp_dma_enum_)this->id(),this->m_hwModule);
#endif
    buf_info.img_stride = header_data[Header_AFO::E_STRIDE];
    //
    if(pStr){
        snprintf(pStr, TS_BUF_SIZE, "enq_sof:0x%x,mag:0x%x,timestamp:%u.%u.%u.%u,stride:0x%x",\
            header_data[Header_AFO::E_ENQUE_SOF],\
            header_data[Header_AFO::E_Magic], \
            buf_info.i4TimeStamp_sec, \
            buf_info.i4TimeStamp_us, \
            buf_info.i4TimeStamp_sec_B,\
            buf_info.i4TimeStamp_us_B, \
            buf_info.img_stride);
    }
}

E_BC_STATUS BUF_CTRL_AFO::waitBufReady( MBOOL bClear, CAM_STATE_NOTIFY *pNotify)
{
    #define _LOOP_  (5)
    E_BC_STATUS ret = eCmd_Pass;
    E_ISP_CAM_CQ cq;
    MUINT32 page;

    ISP_WAIT_IRQ_ST irq;
    MUINT32 loopCnt = _LOOP_;
    char str[128] = {'\0'};

    MUINT32 FbcCnt, _buf_cnt;
    MUINT32 _pa;
    ST_BUF_INFO _buf;
    MUINT32 _size;
    MUINT32 subSample;
    this->m_Queue_deque_ptr.sendCmd(QueueMgr<MUINTPTR>::eCmd_getsize,(MUINTPTR)&_size,0);
    if(_size <1){
        CAM_FUNC_ERR("queue depth is empty for deque\n");
        return eCmd_Fail;
    }

    this->m_pDrv = (ISP_DRV_CAM*)this->m_pIspDrv;

    //dbg log
    this->m_pDrv->getCurObjInfo(&this->m_hwModule,&cq,&page);
    if ( 0xffffffff == this->id() ) {
        CAM_FUNC_ERR("dma channel error \n");
        return eCmd_Fail;
    }

#if 0
    //fbc reg dbg log
    this->FBC_STATUS(this->m_pDrv->getPhyObj());
#endif

    switch (CAM_BUF_CTRL::m_CamState[this->m_hwModule]) {
        //theoratically, it no need to reference normalpipe's status under sttpipe.
        //but due to suspend/resume is only implemented in noramlpipe, so sttpipe need to use suspend/resume status.
        case CAM_BUF_CTRL::eCamState_suspending:
            return eCmd_Suspending_Pass;
            break;
        default:
            break;
    }

    subSample = CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_CTL_SW_PASS1_DONE,DOWN_SAMPLE_PERIOD) + 1;

    this->m_bClear = (bClear == MTRUE)?(MFALSE):(MTRUE);

    if(bClear == MTRUE)
        irq.Clear = ISP_IRQ_CLEAR_WAIT;
    else
        irq.Clear = ISP_IRQ_CLEAR_NONE;

    irq.UserKey = 0;
    irq.St_type = DMA_INT;
    irq.Status = (MUINT32)AFO_DONE_ST;

    do{
        //irq.Timeout = ((1000 + (this->m_fps - 1)) / this->m_fps) * subSample;
        //irq.Timeout *= 4; //amplify 3 times is for the case of enque latch timing
        irq.Timeout = this->estimateTimeout(subSample);

        this->m_bufctrl_lock.lock();
        _buf_cnt = this->m_buf_cnt;
        this->m_bufctrl_lock.unlock();

        if(_buf_cnt != 0){

            //
            this->m_Queue.sendCmd(QueueMgr<ST_BUF_INFO>::eCmd_at,0,(MUINTPTR)&_buf);
            IspFunction_B::fbcPhyLock_AFO[this->m_hwModule].lock();
            FbcCnt = CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_FBC_AFO_CTL2,FBC_CNT);
            IspFunction_B::fbcPhyLock_AFO[this->m_hwModule].unlock();
            //af need to check header is because of fbccnt is updated at p1_don, not af don
            _pa = *((MUINT32*)_buf.header.va_addr + Header_AFO::E_IMG_PA);

            if( (FbcCnt < _buf_cnt)||(_pa != 0)){
                if(loopCnt == _LOOP_){
#if TEMP_SW_TIMESTAMP
                    if (!(CAM_READ_BITS(this->m_pDrv->getPhyObj(), CAM_CTL_TWIN_STATUS,TWIN_EN) &&
                        (this->m_hwModule == CAM_READ_BITS(this->m_pDrv->getPhyObj(), CAM_CTL_TWIN_STATUS, TWIN_MODULE)))) {
                        // for SW timestamp queue ready
                        MUINT32 waitOps[3];


                        waitOps[0] = _cam_max_;
                        waitOps[1] = this->id();

                        if (this->m_pDrv->getDeviceInfo(_GET_START_TIME, (MUINT8*)waitOps) == MFALSE) {
                            CAM_FUNC_ERR("Wait timestamp ready fail\n");
                        }
                    }
#endif
                    CAM_FUNC_DBG("dma:%s already have avail_buf on dram, bWaitBufReady  = 0\n",this->name_Str());
                    return eCmd_Pass;
                }
                else{
                    ret = eCmd_Pass;
                    goto EXIT;
                }
            }
            else{
                if(loopCnt == _LOOP_)
                    snprintf(str, sizeof(str), "dma:%s start wait:[enque record:0x%x_0x%x],",this->name_Str(),this->m_buf_cnt,FbcCnt);
                else{
                    char _tmp[16];
                    snprintf(_tmp, sizeof(_tmp), "[%d]time,",((_LOOP_-loopCnt)+1));
                    strncat(str,_tmp, (sizeof(str) - strlen(str)) - 1);
                }
            }
        }
        else{
            if(loopCnt == _LOOP_){
                snprintf(str, sizeof(str), "dma:%s bWaitBufReady = 1,wait for sw enque[0x%x],timeout:%d x %d ms,[1] ",this->name_Str(),this->m_buf_cnt,irq.Timeout,loopCnt);
            }
            else{//save how many times of wait_irq before sw enque
                char _tmp[16];
                snprintf(_tmp, sizeof(_tmp), "[%d],",((_LOOP_-loopCnt)+1)+1);
                strncat(str,_tmp, (sizeof(str) - strlen(str)) - 1);
            }
        }

        //
        if (pNotify) {
            pNotify->fpNotifyState(CAM_ENTER_WAIT, pNotify->Obj);
        }

        if(this->m_pDrv->waitIrq(&irq) == MFALSE) {
            ret = eCmd_Fail;
        } else {
            ret = eCmd_Pass;
        }

        if (pNotify) {
            pNotify->fpNotifyState(CAM_EXIT_WAIT, pNotify->Obj);
        }

        switch (CAM_BUF_CTRL::m_CamState[this->m_hwModule]) {
            //theoratically, it no need to reference normalpipe's status under sttpipe.
            //but due to suspend/resume is only implemented in noramlpipe, so sttpipe need to use suspend/resume status.
            case CAM_BUF_CTRL::eCamState_suspending:
                return eCmd_Suspending_Pass;
                break;
            default:
                break;
        }
        CHECK_STT_STOP_ST(this->name_Str(),str);
        //
        irq.Clear = ISP_IRQ_CLEAR_NONE;
        //
        if(ret == eCmd_Pass)
            break;
    }while(--loopCnt > 0);
EXIT:

    if(ret == eCmd_Fail){
        if(_buf_cnt != 0){
            CAM_FUNC_ERR("%s, waitbufready fail. start fail check\n",str);
            this->PipeCheck();
        }
        else{
            CAM_FUNC_ERR("%s, waitbufready fail is caused by no enque\n",str);
        }
    }
    else{
        CAM_FUNC_DBG("%s, waitbufready pass\n",str);
    }
    return ret;
}

void BUF_CTRL_PDO::FBC_STATUS(IspDrv* ptr)
{
    CAM_FUNC_DBG("BUF_CTRL_PDO::FBC_STATUS: 0x%x_0x%x",CAM_READ_REG(ptr,CAM_FBC_PDO_CTL1),CAM_READ_REG(ptr,CAM_FBC_PDO_CTL2));
}

void BUF_CTRL_PDO::Header_Parser(MUINTPTR fh_va,NSImageio::NSIspio::BufInfo& buf_info,MUINT32* header_data ,char* pStr)
{
    Header_PDO  fh_pdo;
    MUINT32 bin, dbin, psel;
    //
    fh_pdo.Header_Deque(fh_va,(void*)header_data);
    fh_pdo.Header_Flash(fh_va);
    //
    buf_info.u4BufPA[0] = header_data[Header_PDO::E_IMG_PA];
    buf_info.m_num = header_data[Header_PDO::E_Magic];
    buf_info.img_stride = header_data[Header_PDO::E_STRIDE];
    buf_info.u4BufOffset[0] = header_data[Header_PDO::E_IMG_PA_OFSET];
    //
    if(buf_info.m_pPrivate != NULL){
        ((MRect*)buf_info.m_pPrivate)[0].p.x = header_data[Header_PDO::E_PMX_A_CROP]&0xFFFF;
        ((MRect*)buf_info.m_pPrivate)[0].p.y = (header_data[Header_PDO::E_PMX_A_CROP]>>16);
        //
        ((MRect*)buf_info.m_pPrivate)[1].p.x = header_data[Header_PDO::E_BMX_A_CROP]&0xFFFF;
        ((MRect*)buf_info.m_pPrivate)[1].p.y = (header_data[Header_PDO::E_BMX_A_CROP]>>16);
    }

    buf_info.i4TimeStamp_sec = header_data[Header_PDO::E_TimeStamp];

#if TEMP_SW_TIMESTAMP
    this->m_TimeStamp.TimeStamp_mapping(&buf_info.i4TimeStamp_sec,&buf_info.i4TimeStamp_us,&buf_info.i4TimeStamp_sec_B,&buf_info.i4TimeStamp_us_B,
        (_isp_dma_enum_)this->id(),this->m_hwModule);
#else
    this->m_pTimeStamp->TimeStamp_mapping(&buf_info.i4TimeStamp_sec,&buf_info.i4TimeStamp_us,&buf_info.i4TimeStamp_sec_B,&buf_info.i4TimeStamp_us_B,
        (_isp_dma_enum_)this->id(),this->m_hwModule);
#endif
    //
    if(pStr){
        snprintf(pStr, TS_BUF_SIZE, "enq_sof:0x%x,mag:0x%x,timestamp:%u.%u.%u.%u",\
            header_data[Header_PDO::E_ENQUE_SOF],\
            header_data[Header_PDO::E_Magic], \
            buf_info.i4TimeStamp_sec, \
            buf_info.i4TimeStamp_us, \
            buf_info.i4TimeStamp_sec_B,\
            buf_info.i4TimeStamp_us_B);
    }
}


E_BC_STATUS BUF_CTRL_PDO::waitBufReady( CAM_STATE_NOTIFY *pNotify)
{
    #define _LOOP_  (5)
    E_BC_STATUS ret = eCmd_Pass;
    E_ISP_CAM_CQ cq;
    MUINT32 page;

    ISP_WAIT_IRQ_ST irq;
    MUINT32 loopCnt = _LOOP_;
    char str[128] = {'\0'};

    MUINT32 FbcCnt, _buf_cnt;
    MUINT32 _pa;
    ST_BUF_INFO _buf;
    MUINT32 _size;
    MUINT32 subSample;
    this->m_Queue_deque_ptr.sendCmd(QueueMgr<MUINTPTR>::eCmd_getsize,(MUINTPTR)&_size,0);
    if(_size <1){
        CAM_FUNC_ERR("queue depth is empty for deque\n");
        return eCmd_Fail;
    }

    this->m_pDrv = (ISP_DRV_CAM*)this->m_pIspDrv;

    //dbg log
    this->m_pDrv->getCurObjInfo(&this->m_hwModule,&cq,&page);
    if ( 0xffffffff == this->id() ) {
        CAM_FUNC_ERR("dma channel error \n");
        return eCmd_Fail;
    }

#if 0
    //fbc reg dbg log
    this->FBC_STATUS(this->m_pDrv->getPhyObj());
#endif

    switch (CAM_BUF_CTRL::m_CamState[this->m_hwModule]) {
        //theoratically, it no need to reference normalpipe's status under sttpipe.
        //but due to suspend/resume is only implemented in noramlpipe, so sttpipe need to use suspend/resume status.
        case CAM_BUF_CTRL::eCamState_suspending:
            return eCmd_Suspending_Pass;
            break;
        default:
            break;
    }

    subSample = CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_CTL_SW_PASS1_DONE,DOWN_SAMPLE_PERIOD) + 1;
    irq.Clear = ISP_IRQ_CLEAR_WAIT;


    irq.UserKey = 0;
    irq.St_type = DMA_INT;
    irq.Status = (MUINT32)PDO_DONE_ST;

    do{
        //irq.Timeout = ((1000 + (this->m_fps - 1)) / this->m_fps) * subSample;
        //irq.Timeout *= 4; //amplify 3 times is for the case of enque latch timing
        irq.Timeout = this->estimateTimeout(subSample);

        this->m_bufctrl_lock.lock();
        _buf_cnt = this->m_buf_cnt;
        this->m_bufctrl_lock.unlock();

        if(_buf_cnt != 0){

            //
            this->m_Queue.sendCmd(QueueMgr<ST_BUF_INFO>::eCmd_at,0,(MUINTPTR)&_buf);
            IspFunction_B::fbcPhyLock_PDO[this->m_hwModule].lock();
            FbcCnt = CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_FBC_PDO_CTL2,FBC_CNT);
            IspFunction_B::fbcPhyLock_PDO[this->m_hwModule].unlock();
            //pd need to check header is because of fbccnt is updated at p1_don, not pd don
            _pa = *((MUINT32*)_buf.header.va_addr + Header_PDO::E_IMG_PA);

            if( (FbcCnt < _buf_cnt)||(_pa != 0)){
#if TEMP_SW_TIMESTAMP
                if(loopCnt == _LOOP_){
                    // for SW timestamp queue ready
                    MUINT32 waitOps[3];


                    waitOps[0] = _cam_max_;
                    waitOps[1] = this->id();

                    if (this->m_pDrv->getDeviceInfo(_GET_START_TIME, (MUINT8*)waitOps) == MFALSE) {
                        CAM_FUNC_ERR("Wait timestamp ready fail\n");
                    }

                    CAM_FUNC_DBG("dma:%s already have avail_buf on dram, bWaitBufReady  = 0\n",this->name_Str());
                    return eCmd_Pass;
                }
                else{
                    ret = eCmd_Pass;
                    goto EXIT;
                }
#else
                ret = eCmd_Pass;
                goto EXIT;
#endif
            }
            else{
                if(loopCnt == _LOOP_)
                    snprintf(str, sizeof(str), "dma:%s start wait:[enque record:0x%x_0x%x],",this->name_Str(),this->m_buf_cnt,FbcCnt);
                else{
                    char _tmp[16];
                    snprintf(_tmp, sizeof(_tmp), "[%d]time,",((_LOOP_-loopCnt)+1));
                    strncat(str,_tmp, (sizeof(str) - strlen(str)) - 1);
                }
            }
        }
        else{
            if(loopCnt == _LOOP_){
                snprintf(str, sizeof(str), "dma:%s bWaitBufReady = 1,wait for sw enque[0x%x],timeout:%d x %d ms,[1] ",this->name_Str(),this->m_buf_cnt,irq.Timeout,loopCnt);
            }
            else{//save how many times of wait_irq before sw enque
                char _tmp[16];
                snprintf(_tmp, sizeof(_tmp), "[%d],",((_LOOP_-loopCnt)+1)+1);
                strncat(str,_tmp, (sizeof(str) - strlen(str)) - 1);
            }
        }

        //
        if (pNotify) {
            pNotify->fpNotifyState(CAM_ENTER_WAIT, pNotify->Obj);
        }

        if(this->m_pDrv->waitIrq(&irq) == MFALSE) {
            ret = eCmd_Fail;
        } else {
            ret = eCmd_Pass;
        }

        if (pNotify) {
            pNotify->fpNotifyState(CAM_EXIT_WAIT, pNotify->Obj);
        }

        switch (CAM_BUF_CTRL::m_CamState[this->m_hwModule]) {
            //theoratically, it no need to reference normalpipe's status under sttpipe.
            //but due to suspend/resume is only implemented in noramlpipe, so sttpipe need to use suspend/resume status.
            case CAM_BUF_CTRL::eCamState_suspending:
                return eCmd_Suspending_Pass;
                break;
            default:
                break;
        }
        CHECK_STT_STOP_ST(this->name_Str(),str);
        //
        irq.Clear = ISP_IRQ_CLEAR_NONE;
        //
        if(ret == eCmd_Pass)
            break;
    }while(--loopCnt > 0);
EXIT:

    if(ret == eCmd_Fail){
        if(_buf_cnt != 0){
            CAM_FUNC_ERR("%s, waitbufready fail. start fail check\n",str);
            this->PipeCheck();
        }
        else{
            CAM_FUNC_ERR("%s, waitbufready fail is caused by no enque\n",str);
        }
    }
    else{
        CAM_FUNC_DBG("%s, waitbufready pass\n",str);
    }
    return ret;
}


void BUF_CTRL_FLKO::FBC_STATUS(IspDrv* ptr)
{
    (void)ptr;
    CAM_FUNC_DBG("BUF_CTRL_FLKO::FBC_STATUS: 0x%x_0x%x",UNI_READ_REG(this->m_pUniDrv,CAM_UNI_FBC_FLKO_A_CTL1),UNI_READ_REG(this->m_pUniDrv,CAM_UNI_FBC_FLKO_A_CTL2));
}

void BUF_CTRL_FLKO::Header_Parser(MUINTPTR fh_va,NSImageio::NSIspio::BufInfo& buf_info,MUINT32* header_data ,char* pStr)
{
    Header_FLKO  fh_flko;
    //
    fh_flko.Header_Deque(fh_va,(void*)header_data);
    fh_flko.Header_Flash(fh_va);
    //
    buf_info.u4BufPA[0] = header_data[Header_FLKO::E_IMG_PA];
    buf_info.m_num = header_data[Header_FLKO::E_Magic];

    buf_info.i4TimeStamp_sec = header_data[Header_FLKO::E_TimeStamp];

#if TEMP_SW_TIMESTAMP
    this->m_TimeStamp.TimeStamp_mapping(&buf_info.i4TimeStamp_sec,&buf_info.i4TimeStamp_us,&buf_info.i4TimeStamp_sec_B,&buf_info.i4TimeStamp_us_B,
        (_isp_dma_enum_)this->id(),this->m_hwModule);
#else
    this->m_pTimeStamp->TimeStamp_mapping(&buf_info.i4TimeStamp_sec,&buf_info.i4TimeStamp_us,&buf_info.i4TimeStamp_sec_B,&buf_info.i4TimeStamp_us_B,
        (_isp_dma_enum_)this->id(),this->m_hwModule);
#endif
    //
    if(pStr){
        snprintf(pStr, TS_BUF_SIZE, "enq_sof:0x%x,mag:0x%x,timestamp:%u.%u.%u.%u",\
            header_data[Header_FLKO::E_ENQUE_SOF],\
            header_data[Header_FLKO::E_Magic], \
            buf_info.i4TimeStamp_sec, \
            buf_info.i4TimeStamp_us, \
            buf_info.i4TimeStamp_sec_B,\
            buf_info.i4TimeStamp_us_B);
    }
}

E_BC_STATUS BUF_CTRL_FLKO::waitBufReady(CAM_STATE_NOTIFY *pNotify)
{
    #define _LOOP_  (5)
    E_BC_STATUS ret = eCmd_Pass;
    E_ISP_CAM_CQ cq;
    MUINT32 page;

    ISP_WAIT_IRQ_ST irq;
    MUINT32 loopCnt = _LOOP_;
    char str[128] = {'\0'};

    MUINT32 FbcCnt, _buf_cnt;
    MUINT32 subSample;

    this->m_pDrv = (ISP_DRV_CAM*)this->m_pIspDrv;

    //dbg log
    this->m_pDrv->getCurObjInfo(&this->m_hwModule,&cq,&page);
    if ( 0xffffffff == this->id() ) {
        CAM_FUNC_ERR("dma channel error \n");
        return eCmd_Fail;
    }

#if 0
    //fbc reg dbg log
    this->FBC_STATUS(this->m_pDrv->getPhyObj());
#endif

    switch (CAM_BUF_CTRL::m_CamState[this->m_hwModule]) {
        //theoratically, it no need to reference normalpipe's status under sttpipe.
        //but due to suspend/resume is only implemented in noramlpipe, so sttpipe need to use suspend/resume status.
        case CAM_BUF_CTRL::eCamState_suspending:
            return eCmd_Suspending_Pass;
            break;
        default:
            break;
    }

    subSample = CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_CTL_SW_PASS1_DONE,DOWN_SAMPLE_PERIOD) + 1;

    irq.Clear = ISP_IRQ_CLEAR_WAIT;
    irq.UserKey = 0;
    irq.St_type = DMA_INT;
    irq.Status = (MUINT32)FLKO_DONE_ST;

    do{
        //irq.Timeout = ((1000 + (this->m_fps - 1)) / this->m_fps) * subSample;
        //irq.Timeout *= 4; //amplify 3 times is for the case of enque latch timing
        irq.Timeout = this->estimateTimeout(subSample);

        this->m_bufctrl_lock.lock();
        _buf_cnt = this->m_buf_cnt;
        this->m_bufctrl_lock.unlock();

        if(_buf_cnt != 0){

            //
            IspFunction_B::fbcPhyLock_FLKO[this->m_hwModule].lock();
            FbcCnt = UNI_READ_BITS(this->m_pUniDrv,CAM_UNI_FBC_FLKO_A_CTL2,FBC_CNT);
            IspFunction_B::fbcPhyLock_FLKO[this->m_hwModule].unlock();

            if(FbcCnt < _buf_cnt){
#if TEMP_SW_TIMESTAMP
                if(loopCnt == _LOOP_){
                    // for SW timestamp queue ready
                    MUINT32 waitOps[3];

                    waitOps[0] = _cam_max_;
                    waitOps[1] = this->id();

                    if (this->m_pDrv->getDeviceInfo(_GET_START_TIME, (MUINT8*)waitOps) == MFALSE) {
                        CAM_FUNC_ERR("Wait timestamp ready fail\n");
                    }

                    CAM_FUNC_DBG("dma:%s already have avail_buf on dram, bWaitBufReady  = 0\n",this->name_Str());
                    return eCmd_Pass;
                }
                else{
                    ret = eCmd_Pass;
                    goto EXIT;
                }
#else
                ret = eCmd_Pass;
                goto EXIT;
#endif
            }
            else{
                if(loopCnt == _LOOP_)
                    snprintf(str, sizeof(str), "dma:%s start wait:[enque record:0x%x_0x%x],",this->name_Str(),this->m_buf_cnt,FbcCnt);
                else{
                    char _tmp[16];
                    snprintf(_tmp, sizeof(_tmp), "[%d]time,",((_LOOP_-loopCnt)+1));
                    strncat(str,_tmp, (sizeof(str) - strlen(str)) - 1);
                }
            }
        }
        else{
            if(loopCnt == _LOOP_){
                snprintf(str, sizeof(str), "dma:%s bWaitBufReady = 1,wait for sw enque[0x%x],timeout:%d x %d ms,[1] ",this->name_Str(),this->m_buf_cnt,irq.Timeout,loopCnt);
            }
            else{//save how many times of wait_irq before sw enque
                char _tmp[16];
                snprintf(_tmp, sizeof(_tmp), "[%d],",((_LOOP_-loopCnt)+1)+1);
                strncat(str,_tmp, (sizeof(str) - strlen(str)) - 1);
            }
        }

        //
        if (pNotify) {
            pNotify->fpNotifyState(CAM_ENTER_WAIT, pNotify->Obj);
        }

        if(this->m_pDrv->waitIrq(&irq) == MFALSE) {
            ret = eCmd_Fail;
        } else {
            ret = eCmd_Pass;
        }

        if (pNotify) {
            pNotify->fpNotifyState(CAM_EXIT_WAIT, pNotify->Obj);
        }

        switch (CAM_BUF_CTRL::m_CamState[this->m_hwModule]) {
            //theoratically, it no need to reference normalpipe's status under sttpipe.
            //but due to suspend/resume is only implemented in noramlpipe, so sttpipe need to use suspend/resume status.
            case CAM_BUF_CTRL::eCamState_suspending:
                return eCmd_Suspending_Pass;
                break;
            default:
                break;
        }
        CHECK_STT_STOP_ST(this->name_Str(),str);
        //
        irq.Clear = ISP_IRQ_CLEAR_NONE;
        //
        if(ret == eCmd_Pass)
            break;
    }while(--loopCnt > 0);
EXIT:
    if(ret == eCmd_Fail){
        if(_buf_cnt != 0){
            CAM_FUNC_ERR("%s, waitbufready fail. start fail check\n",str);
            this->PipeCheck();
        }
        else{
            CAM_FUNC_ERR("%s, waitbufready fail is caused by no enque\n",str);
        }
    }
    else{
        CAM_FUNC_DBG("%s, waitbufready pass\n",str);
    }
    return ret;
}


void Header_IMGO::Header_Enque(MUINT32 Tag,ISP_DRV_CAM* DrvPtr,MUINT32 value)
{
    switch(Tag){
        case Header_IMGO::E_ENQUE_SOF:
            CAM_WRITE_REG(DrvPtr,CAM_IMGO_FH_SPARE_3,value);
            break;
        case Header_IMGO::E_Magic:
            CAM_WRITE_REG(DrvPtr,CAM_IMGO_FH_SPARE_2,value);
            break;
        case Header_IMGO::E_RAW_TYPE:
            CAM_WRITE_REG(DrvPtr,CAM_IMGO_FH_SPARE_4,value);
            break;
        case Header_IMGO::E_PIX_ID:
            CAM_WRITE_REG(DrvPtr,CAM_IMGO_FH_SPARE_5,value);
            break;
        case Header_IMGO::E_FMT:
            CAM_WRITE_REG(DrvPtr,CAM_IMGO_FH_SPARE_6,value);
            break;
        case Header_IMGO::E_CROP_START:
            CAM_WRITE_REG(DrvPtr,CAM_IMGO_FH_SPARE_7,value);
            break;
        case Header_IMGO::E_CROP_SIZE:
            CAM_WRITE_REG(DrvPtr,CAM_IMGO_FH_SPARE_8,value);
            break;
        case Header_IMGO::E_ENQUE_CNT:
            CAM_WRITE_REG(DrvPtr,CAM_IMGO_FH_SPARE_9,value);
            break;
        case Header_IMGO::E_IMG_PA:
            CAM_WRITE_REG(DrvPtr,CAM_IMGO_FH_SPARE_10,value);
            break;
        case Header_IMGO::E_SRC_SIZE:
            CAM_WRITE_REG(DrvPtr,CAM_IMGO_FH_SPARE_11,value);
            break;
        default:
        case Header_IMGO::E_TimeStamp:
            CAM_FUNC_ERR("unsupported tag:0x%x\n",Tag);
            break;
    }
}

void Header_IMGO::Header_Deque(MUINTPTR FH_VA,void* pOut)
{
    void* ptr = (void*)FH_VA;
    memcpy(pOut,ptr,sizeof(MUINT32)*E_MAX);
}

void Header_IMGO::Header_Flash(MUINTPTR FH_VA)
{
    void* ptr = (void*)FH_VA;
    memset(ptr,0,sizeof(MUINT32)*E_MAX);

}


void Header_RRZO::Header_Enque(MUINT32 Tag,ISP_DRV_CAM* DrvPtr,MUINT32 value)
{
    switch(Tag){
        case Header_RRZO::E_ENQUE_SOF:
            CAM_WRITE_REG(DrvPtr,CAM_RRZO_FH_SPARE_3,value);
            break;
        case Header_RRZO::E_Magic:
            CAM_WRITE_REG(DrvPtr,CAM_RRZO_FH_SPARE_2,value);
            break;
        case Header_RRZO::E_PIX_ID:
            CAM_WRITE_REG(DrvPtr,CAM_RRZO_FH_SPARE_4,value);
            break;
        case Header_RRZO::E_FMT:
            CAM_WRITE_REG(DrvPtr,CAM_RRZO_FH_SPARE_5,value);
            break;
        case Header_RRZO::E_RRZ_CRP_START:
            CAM_WRITE_REG(DrvPtr,CAM_RRZO_FH_SPARE_6,value);
            break;
        case Header_RRZO::E_RRZ_CRP_SIZE:
            CAM_WRITE_REG(DrvPtr,CAM_RRZO_FH_SPARE_7,value);
            break;
        case Header_RRZO::E_RRZ_DST_SIZE:
            CAM_WRITE_REG(DrvPtr,CAM_RRZO_FH_SPARE_8,value);
            break;
        case Header_RRZO::E_ENQUE_CNT:
            CAM_WRITE_REG(DrvPtr,CAM_RRZO_FH_SPARE_9,value);
            break;
        case Header_RRZO::E_IMG_PA:
            CAM_WRITE_REG(DrvPtr,CAM_RRZO_FH_SPARE_10,value);
            break;
        default:
        case Header_RRZO::E_TimeStamp:
            CAM_FUNC_ERR("unsupported tag:0x%x\n",Tag);
            break;
    }
}


void Header_RRZO::Header_Deque(MUINTPTR FH_VA,void* pOut)
{
    void* ptr = (void*)FH_VA;
    memcpy(pOut,ptr,sizeof(MUINT32)*E_MAX);
}

void Header_RRZO::Header_Flash(MUINTPTR FH_VA)
{
    void* ptr = (void*)FH_VA;
    memset(ptr,0,sizeof(MUINT32)*E_MAX);

}

MUINT32 Header_RRZO::GetRegInfo(MUINT32 Tag,IspDrv* DrvPtr)
{
    switch(Tag){
        case Header_RRZO::E_ENQUE_SOF:
            return CAM_READ_REG(DrvPtr,CAM_RRZO_FH_SPARE_3,value);
            break;
        case Header_RRZO::E_Magic:
            return CAM_READ_REG(DrvPtr,CAM_RRZO_FH_SPARE_2,value);
            break;
        case Header_RRZO::E_PIX_ID:
            return CAM_READ_REG(DrvPtr,CAM_RRZO_FH_SPARE_4,value);
            break;
        case Header_RRZO::E_FMT:
            return CAM_READ_REG(DrvPtr,CAM_RRZO_FH_SPARE_5,value);
            break;
        case Header_RRZO::E_RRZ_CRP_START:
            return CAM_READ_REG(DrvPtr,CAM_RRZO_FH_SPARE_6,value);
            break;
        case Header_RRZO::E_RRZ_CRP_SIZE:
            return CAM_READ_REG(DrvPtr,CAM_RRZO_FH_SPARE_7,value);
            break;
        case Header_RRZO::E_RRZ_DST_SIZE:
            return CAM_READ_REG(DrvPtr,CAM_RRZO_FH_SPARE_8,value);
            break;
        case Header_RRZO::E_ENQUE_CNT:
            return CAM_READ_REG(DrvPtr,CAM_RRZO_FH_SPARE_9,value);
            break;
        default:
        case Header_RRZO::E_TimeStamp:
            CAM_FUNC_ERR("unsupported tag:0x%x\n",Tag);
            break;
    }
    return 0;
}

MUINT32 Header_RRZO::GetTagAddr(MUINT32 Tag)
{
    ISP_DRV_CAM* ptr = (ISP_DRV_CAM*)ISP_DRV_CAM::createInstance(CAM_A,ISP_DRV_CQ_THRE0,0,"TagAddr");
    MUINT32 address = 0;

    switch(Tag){
        case Header_RRZO::E_ENQUE_SOF:
            address = CAM_REG_ADDR(ptr,CAM_RRZO_FH_SPARE_3);
            break;
        case Header_RRZO::E_Magic:
            address = CAM_REG_ADDR(ptr,CAM_RRZO_FH_SPARE_2);
            break;
        case Header_RRZO::E_PIX_ID:
            address = CAM_REG_ADDR(ptr,CAM_RRZO_FH_SPARE_4);
            break;
        case Header_RRZO::E_FMT:
            address = CAM_REG_ADDR(ptr,CAM_RRZO_FH_SPARE_5);
            break;
        case Header_RRZO::E_RRZ_CRP_START:
            address = CAM_REG_ADDR(ptr,CAM_RRZO_FH_SPARE_6);
            break;
        case Header_RRZO::E_RRZ_CRP_SIZE:
            address = CAM_REG_ADDR(ptr,CAM_RRZO_FH_SPARE_7);
            break;
        case Header_RRZO::E_RRZ_DST_SIZE:
            address = CAM_REG_ADDR(ptr,CAM_RRZO_FH_SPARE_8);
            break;
        case Header_RRZO::E_ENQUE_CNT:
            address = CAM_REG_ADDR(ptr,CAM_RRZO_FH_SPARE_9);
            break;
        default:
        case Header_RRZO::E_TimeStamp:
            break;
    }

    ptr->destroyInstance();
    return address;

}

void Header_UFEO::Header_Enque(MUINT32 Tag,ISP_DRV_CAM* DrvPtr,MUINT32 value)
{
    switch(Tag){
        case Header_UFEO::E_Magic:
            CAM_WRITE_REG(DrvPtr,CAM_UFEO_FH_SPARE_2,value);
            break;
        case Header_UFEO::E_ENQUE_SOF:
            CAM_WRITE_REG(DrvPtr,CAM_UFEO_FH_SPARE_3,value);
            break;
        case Header_UFEO::E_IMG_PA:
            CAM_WRITE_REG(DrvPtr,CAM_UFEO_FH_SPARE_4,value);
            break;
        case Header_UFEO::E_ENQUE_CNT:
            CAM_WRITE_REG(DrvPtr,CAM_UFEO_FH_SPARE_5,value);
            break;
        case Header_UFEO::E_FMT:
            CAM_WRITE_REG(DrvPtr,CAM_UFEO_FH_SPARE_6,value);
            break;
        default:
        case Header_UFEO::E_TimeStamp:
            CAM_FUNC_ERR("unsupported tag:0x%x\n",Tag);
            break;
    }
}


void Header_UFEO::Header_Deque(MUINTPTR FH_VA,void* pOut)
{
    void* ptr = (void*)FH_VA;
    memcpy(pOut,ptr,sizeof(MUINT32)*E_MAX);
}

void Header_UFEO::Header_Flash(MUINTPTR FH_VA)
{
    void* ptr = (void*)FH_VA;
    memset(ptr,0,sizeof(MUINT32)*E_MAX);

}


void Header_AAO::Header_Enque(MUINT32 Tag,ISP_DRV_CAM* DrvPtr,MUINT32 value)
{
    switch(Tag){
        case Header_AAO::E_ENQUE_SOF:
            ((MUINT32*)DrvPtr)[Tag-1 - 1] = value;
            break;
        case Header_AAO::E_Magic:
            CAM_WRITE_REG(DrvPtr,CAM_AAO_FH_SPARE_2,value);
            break;
        case Header_AAO::E_IMG_PA:
           ((MUINT32*)DrvPtr)[Tag-1 - 1] = value;
            break;
        default:
        case Header_AAO::E_TimeStamp:
            CAM_FUNC_ERR("unsupported tag:0x%x\n",Tag);
            break;
    }
}


void Header_AAO::Header_Deque(MUINTPTR FH_VA,void* pOut)
{
    void* ptr = (void*)FH_VA;
    memcpy(pOut,ptr,sizeof(MUINT32)*E_MAX);
}

void Header_AAO::Header_Flash(MUINTPTR FH_VA)
{
    void* ptr = (void*)FH_VA;
    memset(ptr,0,sizeof(MUINT32)*E_MAX);

}

void Header_PSO::Header_Enque(MUINT32 Tag,ISP_DRV_CAM* DrvPtr,MUINT32 value)
{
    switch(Tag){
        case Header_PSO::E_ENQUE_SOF:
            ((MUINT32*)DrvPtr)[Tag-1 - 1] = value;
            break;
        case Header_PSO::E_Magic:
            CAM_WRITE_REG(DrvPtr,CAM_PSO_FH_SPARE_2,value);
            break;
        case Header_PSO::E_IMG_PA:
           ((MUINT32*)DrvPtr)[Tag-1 - 1] = value;
            break;
        default:
        case Header_PSO::E_TimeStamp:
            CAM_FUNC_ERR("unsupported tag:0x%x\n",Tag);
            break;
    }
}


void Header_PSO::Header_Deque(MUINTPTR FH_VA,void* pOut)
{
    void* ptr = (void*)FH_VA;
    memcpy(pOut,ptr,sizeof(MUINT32)*E_MAX);
}

void Header_PSO::Header_Flash(MUINTPTR FH_VA)
{
    void* ptr = (void*)FH_VA;
    memset(ptr,0,sizeof(MUINT32)*E_MAX);

}

void Header_AFO::Header_Enque(MUINT32 Tag,ISP_DRV_CAM* DrvPtr,MUINT32 value)
{
    switch(Tag){
        case Header_AFO::E_ENQUE_SOF:
            ((MUINT32*)DrvPtr)[Tag- 1 - 2] = value; //1st -1 is timestamp, 2nd -2 is for stride
            break;
        case Header_AFO::E_Magic:
            CAM_WRITE_REG(DrvPtr,CAM_AFO_FH_SPARE_2,value);
            break;
        case Header_AFO::E_STRIDE:
            CAM_WRITE_REG(DrvPtr,CAM_AFO_FH_SPARE_3,value);
            break;
        case Header_AFO::E_IMG_PA:
           ((MUINT32*)DrvPtr)[Tag-1 - 2] = value;//1st -1 is timestamp, 2nd -2 is for magic stride
            break;
        default:
        case Header_AFO::E_TimeStamp:
            CAM_FUNC_ERR("unsupported tag:0x%x\n",Tag);
            break;
    }
}


void Header_AFO::Header_Deque(MUINTPTR FH_VA,void* pOut)
{
    void* ptr = (void*)FH_VA;
    memcpy(pOut,ptr,sizeof(MUINT32)*E_MAX);
}

void Header_AFO::Header_Flash(MUINTPTR FH_VA)
{
    void* ptr = (void*)FH_VA;
    memset(ptr,0,sizeof(MUINT32)*E_MAX);

}


void Header_LCSO::Header_Enque(MUINT32 Tag,ISP_DRV_CAM* DrvPtr,MUINT32 value)
{
    switch(Tag){
        case Header_LCSO::E_ENQUE_SOF:
            CAM_WRITE_REG(DrvPtr,CAM_LCSO_FH_SPARE_3,value);
            break;
        case Header_LCSO::E_Magic:
            CAM_WRITE_REG(DrvPtr,CAM_LCSO_FH_SPARE_2,value);
            break;
        case Header_LCSO::E_IMG_PA:
            CAM_WRITE_REG(DrvPtr,CAM_LCSO_FH_SPARE_4,value);
            break;
        case Header_LCSO::E_ENQUE_CNT:
            CAM_WRITE_REG(DrvPtr,CAM_LCSO_FH_SPARE_5,value);
            break;
        default:
        case Header_LCSO::E_TimeStamp:
            CAM_FUNC_ERR("unsupported tag:0x%x\n",Tag);
            break;
    }
}


void Header_LCSO::Header_Deque(MUINTPTR FH_VA,void* pOut)
{
    void* ptr = (void*)FH_VA;
    memcpy(pOut,ptr,sizeof(MUINT32)*E_MAX);
}

void Header_LCSO::Header_Flash(MUINTPTR FH_VA)
{
    void* ptr = (void*)FH_VA;
    memset(ptr,0,sizeof(MUINT32)*E_MAX);

}



void Header_PDO::Header_Enque(MUINT32 Tag,ISP_DRV_CAM* DrvPtr,MUINT32 value)
{
    switch(Tag){
        case Header_PDO::E_ENQUE_SOF:
           ((MUINT32*)DrvPtr)[Tag-1 - 7] = value;//1st -1 is timestamp, 2nd -7 is for pa offset
            break;
        case Header_PDO::E_Magic:
            CAM_WRITE_REG(DrvPtr,CAM_PDO_FH_SPARE_2,value);
            break;
        case Header_PDO::E_STRIDE:
            CAM_WRITE_REG(DrvPtr,CAM_PDO_FH_SPARE_3,value);
            break;
        case Header_PDO::E_PMX_A_CROP:
            CAM_WRITE_REG(DrvPtr,CAM_PDO_FH_SPARE_4,value);
            break;
        case Header_PDO::E_PMX_B_CROP:
            CAM_WRITE_REG(DrvPtr,CAM_PDO_FH_SPARE_5,value);
            break;
        case Header_PDO::E_BMX_A_CROP:
            CAM_WRITE_REG(DrvPtr,CAM_PDO_FH_SPARE_6,value);
            break;
        case Header_PDO::E_BMX_B_CROP:
            CAM_WRITE_REG(DrvPtr,CAM_PDO_FH_SPARE_7,value);
            break;
        case Header_PDO::E_IMG_PA_OFSET:
            CAM_WRITE_REG(DrvPtr,CAM_PDO_FH_SPARE_8,value);
            break;
        case Header_PDO::E_IMG_PA:
           ((MUINT32*)DrvPtr)[Tag-1 - 7] = value;//1st -1 is timestamp, 2nd -7 is for pa offset
            break;
        default:
        case Header_PDO::E_TimeStamp:
            CAM_FUNC_ERR("unsupported tag:0x%x\n",Tag);
            break;
    }
}


void Header_PDO::Header_Deque(MUINTPTR FH_VA,void* pOut)
{
    void* ptr = (void*)FH_VA;
    memcpy(pOut,ptr,sizeof(MUINT32)*E_MAX);
}

void Header_PDO::Header_Flash(MUINTPTR FH_VA)
{
    void* ptr = (void*)FH_VA;
    memset(ptr,0,sizeof(MUINT32)*E_MAX);

}


void Header_EISO::Header_Enque(MUINT32 Tag,ISP_DRV_CAM* DrvPtr,MUINT32 value)
{
    switch(Tag){
        case Header_EISO::E_ENQUE_SOF:
            UNI_WRITE_REG(DrvPtr,CAM_UNI_EISO_FH_SPARE_3,value);
            break;
        case Header_EISO::E_Magic:
            UNI_WRITE_REG(DrvPtr,CAM_UNI_EISO_FH_SPARE_2,value);
            break;
        case Header_EISO::E_IMG_PA:
            UNI_WRITE_REG(DrvPtr,CAM_UNI_EISO_FH_SPARE_4,value);
            break;
        case Header_EISO::E_ENQUE_CNT:
            UNI_WRITE_REG(DrvPtr,CAM_UNI_EISO_FH_SPARE_5,value);
            break;
        default:
        case Header_EISO::E_TimeStamp:
            CAM_FUNC_ERR("unsupported tag:0x%x\n",Tag);
            break;
    }
}


void Header_EISO::Header_Deque(MUINTPTR FH_VA,void* pOut)
{
    void* ptr = (void*)FH_VA;
    memcpy(pOut,ptr,sizeof(MUINT32)*E_MAX);
}

void Header_EISO::Header_Flash(MUINTPTR FH_VA)
{
    void* ptr = (void*)FH_VA;
    memset(ptr,0,sizeof(MUINT32)*E_MAX);

}


void Header_FLKO::Header_Enque(MUINT32 Tag,ISP_DRV_CAM* DrvPtr,MUINT32 value)
{
    switch(Tag){
        case Header_FLKO::E_ENQUE_SOF:
           ((MUINT32*)DrvPtr)[Tag-1 - 1] = value;//1st -1 is timestamp, 2nd -1 is for magic number
            break;
        case Header_FLKO::E_Magic:  //magic number using CQ0
            UNI_WRITE_REG(DrvPtr,CAM_UNI_FLKO_FH_SPARE_2,value);
            break;
        case Header_FLKO::E_IMG_PA:
           ((MUINT32*)DrvPtr)[Tag-1 - 1] = value;//1st -1 is timestamp, 2nd -1 is for magic number
            break;
        default:
        case Header_FLKO::E_TimeStamp:
            CAM_FUNC_ERR("unsupported tag:0x%x\n",Tag);
            break;
    }
}


void Header_FLKO::Header_Deque(MUINTPTR FH_VA,void* pOut)
{
    void* ptr = (void*)FH_VA;
    memcpy(pOut,ptr,sizeof(MUINT32)*E_MAX);
}


void Header_FLKO::Header_Flash(MUINTPTR FH_VA)
{
    void* ptr = (void*)FH_VA;
    memset(ptr,0,sizeof(MUINT32)*E_MAX);

}


void Header_RSSO::Header_Enque(MUINT32 Tag,ISP_DRV_CAM* DrvPtr,MUINT32 value)
{
    switch(Tag){
        case Header_RSSO::E_ENQUE_SOF:
            UNI_WRITE_REG(DrvPtr,CAM_UNI_RSSO_A_FH_SPARE_3,value);
            break;
        case Header_RSSO::E_Magic:
            UNI_WRITE_REG(DrvPtr,CAM_UNI_RSSO_A_FH_SPARE_2,value);
            break;
        case Header_RSSO::E_IMG_PA:
            UNI_WRITE_REG(DrvPtr,CAM_UNI_RSSO_A_FH_SPARE_4,value);
            break;
        case Header_RSSO::E_ENQUE_CNT:
            UNI_WRITE_REG(DrvPtr,CAM_UNI_RSSO_A_FH_SPARE_5,value);
            break;
        case Header_RSSO::E_DST_SIZE:
            UNI_WRITE_REG(DrvPtr,CAM_UNI_RSSO_A_FH_SPARE_6,value);
            break;
        default:
        case Header_RSSO::E_TimeStamp:
            CAM_FUNC_ERR("unsupported tag:0x%x\n",Tag);
            break;
    }
}


void Header_RSSO::Header_Deque(MUINTPTR FH_VA,void* pOut)
{
    void* ptr = (void*)FH_VA;
    memcpy(pOut,ptr,sizeof(MUINT32)*E_MAX);
}

void Header_RSSO::Header_Flash(MUINTPTR FH_VA)
{
    void* ptr = (void*)FH_VA;
    memset(ptr,0,sizeof(MUINT32)*E_MAX);

}


