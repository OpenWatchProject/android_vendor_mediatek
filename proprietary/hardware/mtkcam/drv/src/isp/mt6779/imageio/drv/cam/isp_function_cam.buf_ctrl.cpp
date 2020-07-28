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

using namespace NSImageio;
using namespace NSIspio;

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

#define CAM_READ_REG_INR(IspDrvPtr,RegName,...)({\
    MUINT32 __offset;\
    __offset = (MUINT8*)(&( ((cam_reg_t*)(IspDrvPtr->m_pIspRegMap))->RegName)) - \
                (MUINT8*)(IspDrvPtr->m_pIspRegMap);\
    IspDrvPtr->readInnerReg(__offset,CAM_A);\
})


#define _DMA_Mapping(x)({\
    ISP_WRDMA_ENUM _port;\
    if(x == _imgo_)          _port = _dma_imgo_;\
    else if(x == _rrzo_)     _port = _dma_rrzo_;\
    else if(x == _aao_)      _port = _dma_aao_;\
    else if(x == _afo_)      _port = _dma_afo_;\
    else if(x == _lcso_)     _port = _dma_lcso_;\
    else if(x == _ufeo_)     _port = _dma_ufeo_;\
    else if(x == _pdo_)      _port = _dma_pdo_;\
    else if(x == _lmvo_)     _port = _dma_eiso_;\
    else if(x == _flko_)     _port = _dma_flko_;\
    else if(x == _rsso_)     _port = _dma_rsso_;\
    else if(x == _tsfso_)    _port = _dma_tsfso_;\
    else if(x == _ufgo_)     _port = _dma_ufgo_;\
    else if(x == _yuvo_)     _port = _dma_yuvo_;\
    else if(x == _yuvbo_)    _port = _dma_yuvbo_;\
    else if(x == _yuvco_)    _port = _dma_yuvco_;\
    else if(x == _crzo_)     _port = _dma_crzo_;\
    else if(x == _crzbo_)    _port = _dma_crzbo_;\
    else if(x == _crzo_r2_)  _port = _dma_crzo_r2_;\
    else if(x == _crzbo_r2_) _port = _dma_crzbo_r2_;\
    else if(x == _rawi_)     _port = _dma_rawi_r2_;\
    else if(x == _ufdi_r2)     _port = _dma_ufdi_r2_;\
    else\
        _port = _dma_max_wr_;\
    _port;\
})
#define _DMA_Header_Mapping(x)({\
    ISP_WRDMA_ENUM _port;\
    if(x ==_imgo_)           _port = _dma_imgo_fh_;\
    else if(x ==_rrzo_)      _port = _dma_rrzo_fh_;\
    else if(x == _aao_)      _port = _dma_aao_fh_;\
    else if(x == _afo_)      _port = _dma_afo_fh_;\
    else if(x == _lcso_)     _port = _dma_lcso_fh_;\
    else if(x == _ufeo_)     _port = _dma_ufeo_fh_;\
    else if(x == _pdo_)      _port = _dma_pdo_fh_;\
    else if(x == _lmvo_)     _port = _dma_eiso_fh_;\
    else if(x == _flko_)     _port = _dma_flko_fh_;\
    else if(x == _rsso_)     _port = _dma_rsso_fh_;\
    else if(x == _tsfso_)    _port = _dma_tsfso_fh_;\
    else if(x == _ufgo_)     _port = _dma_ufgo_fh_;\
    else if(x == _yuvo_)     _port = _dma_yuvo_fh_;\
    else if(x == _yuvbo_)    _port = _dma_yuvbo_fh_;\
    else if(x == _yuvco_)    _port = _dma_yuvco_fh_;\
    else if(x == _crzo_)     _port = _dma_crzo_fh_;\
    else if(x == _crzbo_)    _port = _dma_crzbo_fh_;\
    else if(x == _crzo_r2_)  _port = _dma_crzo_r2_fh_;\
    else if(x == _crzbo_r2_) _port = _dma_crzbo_r2_fh_;\
    else\
        _port = _dma_max_wr_;\
    _port;\
})



/*/////////////////////////////////////////////////////////////////////////////
    CAM_BUF_CTRL
  /////////////////////////////////////////////////////////////////////////////*/

MUINT32 CAM_BUF_CTRL::m_fps[CAM_MAX] = {30, 30, 30};
MUINT32 CAM_BUF_CTRL::m_recentFrmTimeMs[CAM_MAX][MAX_RECENT_GRPFRM_TIME] = {
    {MIN_GRPFRM_TIME_MS, MIN_GRPFRM_TIME_MS},
    {MIN_GRPFRM_TIME_MS, MIN_GRPFRM_TIME_MS},
    {MIN_GRPFRM_TIME_MS, MIN_GRPFRM_TIME_MS}
};
CAM_BUF_CTRL::E_CAM_STATE CAM_BUF_CTRL::m_CamState[CAM_MAX] = {CAM_BUF_CTRL::eCamState_runnable, CAM_BUF_CTRL::eCamState_runnable, CAM_BUF_CTRL::eCamState_runnable};
CAM_BUF_CTRL::E_CAM_STATE CAM_BUF_CTRL::m_SttState[CAM_MAX] = {CAM_BUF_CTRL::eCamState_runnable, CAM_BUF_CTRL::eCamState_runnable, CAM_BUF_CTRL::eCamState_runnable};

CAM_BUF_CTRL::CAM_BUF_CTRL(void)
{
    //m_fps = 30;
    m_buf_cnt = 0;
    m_pTimeStamp = 0;
    m_pDrv = NULL;
}

MBOOL CAM_BUF_CTRL::PipeCheck(void)
{
#if 1
    PIPE_CHECK ppc;
    E_ISP_CAM_CQ cq;
    MUINT32 page;
    REG_CAMCTL_R1_CAMCTL_TWIN_STATUS twin_status;
    ISP_HW_MODULE CAM;


    CAM_FUNC_ERR("start PipeCheck when deque fail at wait signal\n");
    this->m_pDrv->getCurObjInfo(&this->m_hwModule,&cq,&page);
    twin_status.Raw = CAM_READ_REG_INR(this->m_pDrv->getPhyObj(),CAMCTL_R1_CAMCTL_TWIN_STATUS);

    if (twin_status.Bits.MASTER_MODULE != this->m_hwModule) {
        CAM = (ISP_HW_MODULE)twin_status.Bits.MASTER_MODULE;
        CAM_FUNC_DBG("Create master cam to do PPC!!!\n");
        ppc.m_pDrv = (ISP_DRV_CAM*)ISP_DRV_CAM::createInstance(CAM, cq, page, "PPC");
    }
    else
        ppc.m_pDrv = this->m_pDrv;
    return ppc.Check_Start();
#else
    return MTRUE;
#endif
}

E_BC_STATUS CAM_BUF_CTRL::waitBufReady(CAM_STATE_NOTIFY *pNotify)
{
#define _LOOP_  (5)
    E_BC_STATUS ret = eCmd_Pass;
    E_ISP_CAM_CQ cq;
    MUINT32 page;

    Header_IMGO     fh_imgo;
    Header_RRZO     fh_rrzo;
    Header_UFEO     fh_ufeo;
    Header_LCSO     fh_lcso;
    Header_LMVO     fh_lmvo;
    Header_RSSO     fh_rsso;
    Header_UFGO     fh_ufgo;
    Header_YUVO     fh_yuvo;
    Header_YUVBO    fh_yuvbo;
    Header_YUVCO    fh_yuvco;
    Header_CRZO_R1  fh_crzo_r1;
    Header_CRZBO_R1 fh_crzbo_r1;
    Header_CRZO_R2  fh_crzo_r2;
    Header_CRZBO_R2 fh_crzbo_r2;

    ISP_WAIT_IRQ_ST irq;
    MUINT32 loopCnt = _LOOP_;
    MUINT32 subSample;
    char str[128] = {'\0'};

    MUINT32 FbcCnt=0;
    MUINT32 sw_enque_cnt = 0;
    ST_BUF_INFO _buf;
    MUINT32 _header_data[E_HEADER_MAX];
    ISP_DRV_CAM* pDrv = NULL;
    IspDrv* pWaitDrv = NULL;
    MUINT32 _size = 0;
    MBOOL bWaitPass = MTRUE;
    REG_CAMCTL_R1_CAMCTL_TWIN_STATUS twin_status;
    ISP_HW_MODULE secModule = PHY_CAM;

    if ( 0xffffffff == this->id() ) {
        CAM_FUNC_ERR("dma channel error ");
        return eCmd_Fail;
    }

    this->m_Queue_deque_ptr.sendCmd(QueueMgr<MUINTPTR>::eCmd_getsize,(MUINTPTR)&_size,0);
    if(_size <1){
        CAM_FUNC_ERR("queue depth is empty for deque\n");
        return eCmd_Fail;
    }

    this->m_Queue.sendCmd(QueueMgr<ST_BUF_INFO>::eCmd_front,(MUINTPTR)&_buf,0);

    //
    this->m_Queue_deque_ptr.sendCmd(QueueMgr<MUINTPTR>::eCmd_front  ,(MUINTPTR)&pDrv,0);
    //can't pop here, pop at deque
    //this->m_Queue_deque_ptr.sendCmd(CAM_BUF_CTRL::QueueMgr<MUINTPTR>::eCmd_pop  ,0,0);
    //dbg log
    pDrv->getCurObjInfo(&this->m_hwModule,&cq,&page);

    //secure camera
    secModule = this->m_hwModule;

    twin_status.Raw = CAM_READ_REG(pDrv, CAMCTL_R1_CAMCTL_TWIN_STATUS);
    //fbc reg dbg log
    this->FBC_STATUS(pDrv->getPhyObj());

    switch (CAM_BUF_CTRL::m_CamState[this->m_hwModule]) {
        //theoratically, it no need to reference normalpipe's status under sttpipe.
        //but due to suspend/resume is only implemented in noramlpipe, so sttpipe need to use suspend/resume status.
        case CAM_BUF_CTRL::eCamState_suspending:
            return eCmd_Suspending_Pass;
            break;
        default:
            break;
    }

    subSample = CAM_READ_BITS(pDrv->getPhyObj(),CAMCTL_R1_CAMCTL_SW_PASS1_DONE,CAMCTL_DOWN_SAMPLE_PERIOD) + 1;

    irq.Clear = ISP_IRQ_CLEAR_WAIT;
    irq.UserKey = 0;

    switch(this->id()){
        case _imgo_:
        case _rrzo_:
        case _ufeo_:
        case _ufgo_:
        case _lcso_:
        case _lmvo_:
        case _rsso_:
        case _yuvo_:
        case _yuvbo_:
        case _yuvco_:
        case _crzo_:
        case _crzbo_:
        case _crzo_r2_:
        case _crzbo_r2_:
            irq.St_type = SIGNAL_INT;
            irq.Status = (MUINT32)SW_PASS1_DON_ST;
            break;
        case _aao_:
        case _afo_:
        case _pdo_:
        case _flko_:
        case _tsfso_:
        default:
            CAM_FUNC_ERR("un-supported DMAO:0x%x\n",this->id());
            return eCmd_Fail;
    }

    do{
        //irq.Timeout = ((1000 + (this->m_fps - 1)) / this->m_fps) * subSample;
        //irq.Timeout *= 5; //amplify 3 times is for the case of enque latch timing
        irq.Timeout = this->estimateTimeout(subSample);

        if( (this->m_buf_cnt != 0) && (this->m_buf_cnt >= subSample) ){
            //
            this->m_Queue.sendCmd(QueueMgr<ST_BUF_INFO>::eCmd_at,(subSample-1),(MUINTPTR)&_buf);

            /*
                 * MUST hold fbcPhyLock_XXX when read FBC CTL registers, due to isp reset flow period will
                 * clear FBC register value before. Hold lock will wait until reset flow recovers FBC register values
                 */
             //For secure cam
             if(SecMgr::SecMgr_GetSecurePortStatus(this->m_hwModule,this->id())){
                SecMgr *mpSecMgr = SecMgr::SecMgr_GetMgrObj();
                SecMgr_SecInfo secinfo;

                secinfo.type = SECMEM_FRAME_HEADER;
                secinfo.module = secModule;
                secinfo.buff_va = _buf.header.va_addr;
                secinfo.port = this->id();

                CAM_FUNC_INF("1-Migrate FH: port:0x%x fh_va:0x%lx buff:0x%x",secinfo.port,secinfo.buff_va,_buf.image.mem_info.pa_addr);
                if(mpSecMgr->SecMgr_P1MigrateTable(&secinfo) == MFALSE){
                    CAM_FUNC_ERR("Migrate sec FH failed:(0x%x)\n",this->id());
                    return eCmd_Fail;
                }
            }
            switch(this->id()){
                case _imgo_:
                    fh_imgo.Header_Deque(_buf.header.va_addr,(void*)_header_data);
                    sw_enque_cnt = _header_data[Header_IMGO::E_ENQUE_CNT];
                    if(twin_status.Bits.TWIN_EN){
                        if(this->m_hwModule != (ISP_HW_MODULE)twin_status.Bits.MASTER_MODULE){
                            pWaitDrv = IspDrvImp::createInstance((ISP_HW_MODULE)twin_status.Bits.MASTER_MODULE);
                        }
                        else
                            pWaitDrv = (IspDrv*)pDrv;
                    }
                    break;
                case _rrzo_:
                    fh_rrzo.Header_Deque(_buf.header.va_addr,(void*)_header_data);
                    sw_enque_cnt = _header_data[Header_RRZO::E_ENQUE_CNT];
                    if(twin_status.Bits.TWIN_EN){
                        if(this->m_hwModule != (ISP_HW_MODULE)twin_status.Bits.MASTER_MODULE){
                            pWaitDrv = IspDrvImp::createInstance((ISP_HW_MODULE)twin_status.Bits.MASTER_MODULE);
                        }
                        else
                            pWaitDrv = (IspDrv*)pDrv;
                    }
                    break;
                case _ufeo_:
                    fh_ufeo.Header_Deque(_buf.header.va_addr,(void*)_header_data);
                    sw_enque_cnt = _header_data[Header_UFEO::E_ENQUE_CNT];
                    if(twin_status.Bits.TWIN_EN){
                        if(this->m_hwModule != (ISP_HW_MODULE)twin_status.Bits.MASTER_MODULE){
                            pWaitDrv = IspDrvImp::createInstance((ISP_HW_MODULE)twin_status.Bits.MASTER_MODULE);
                        }
                        else
                            pWaitDrv = (IspDrv*)pDrv;
                    }
                    break;
                case _lcso_:
                    fh_lcso.Header_Deque(_buf.header.va_addr,(void*)_header_data);
                    sw_enque_cnt = _header_data[Header_LCSO::E_ENQUE_CNT];
                    break;
                case _lmvo_:
                    fh_lmvo.Header_Deque(_buf.header.va_addr,(void*)_header_data);
                    sw_enque_cnt = _header_data[Header_LMVO::E_ENQUE_CNT];
                    break;
                case _rsso_:
                    fh_rsso.Header_Deque(_buf.header.va_addr,(void*)_header_data);
                    sw_enque_cnt = _header_data[Header_RSSO::E_ENQUE_CNT];
                    break;
                case _ufgo_:
                    fh_ufgo.Header_Deque(_buf.header.va_addr,(void*)_header_data);
                    sw_enque_cnt = _header_data[Header_UFGO::E_ENQUE_CNT];
                    if(twin_status.Bits.TWIN_EN){
                        if(this->m_hwModule != (ISP_HW_MODULE)twin_status.Bits.MASTER_MODULE){
                            pWaitDrv = IspDrvImp::createInstance((ISP_HW_MODULE)twin_status.Bits.MASTER_MODULE);
                        }
                        else
                            pWaitDrv = (IspDrv*)pDrv;
                    }
                    break;
                case _yuvo_:
                    fh_yuvo.Header_Deque(_buf.header.va_addr,(void*)_header_data);
                    sw_enque_cnt = _header_data[Header_YUVO::E_ENQUE_CNT];
                    if(twin_status.Bits.TWIN_EN){
                        if(this->m_hwModule != (ISP_HW_MODULE)twin_status.Bits.MASTER_MODULE){
                            pWaitDrv = IspDrvImp::createInstance((ISP_HW_MODULE)twin_status.Bits.MASTER_MODULE);
                        }
                        else
                            pWaitDrv = (IspDrv*)pDrv;
                    }
                    break;
                case _yuvbo_:
                    fh_yuvbo.Header_Deque(_buf.header.va_addr,(void*)_header_data);
                    sw_enque_cnt = _header_data[Header_YUVBO::E_ENQUE_CNT];
                    if(twin_status.Bits.TWIN_EN){
                        if(this->m_hwModule != (ISP_HW_MODULE)twin_status.Bits.MASTER_MODULE){
                            pWaitDrv = IspDrvImp::createInstance((ISP_HW_MODULE)twin_status.Bits.MASTER_MODULE);
                        }
                        else
                            pWaitDrv = (IspDrv*)pDrv;
                    }
                    break;
                case _yuvco_:
                    fh_yuvco.Header_Deque(_buf.header.va_addr,(void*)_header_data);
                    sw_enque_cnt = _header_data[Header_YUVCO::E_ENQUE_CNT];
                    if(twin_status.Bits.TWIN_EN){
                        if(this->m_hwModule != (ISP_HW_MODULE)twin_status.Bits.MASTER_MODULE){
                            pWaitDrv = IspDrvImp::createInstance((ISP_HW_MODULE)twin_status.Bits.MASTER_MODULE);
                        }
                        else
                            pWaitDrv = (IspDrv*)pDrv;
                    }
                    break;
                case _crzo_:
                    fh_crzo_r1.Header_Deque(_buf.header.va_addr,(void*)_header_data);
                    sw_enque_cnt = _header_data[Header_CRZO_R1::E_ENQUE_CNT];
                    if(twin_status.Bits.TWIN_EN){
                        if(this->m_hwModule != (ISP_HW_MODULE)twin_status.Bits.MASTER_MODULE){
                            pWaitDrv = IspDrvImp::createInstance((ISP_HW_MODULE)twin_status.Bits.MASTER_MODULE);
                        }
                        else
                            pWaitDrv = (IspDrv*)pDrv;
                    }
                    break;
                case _crzbo_:
                    fh_crzbo_r1.Header_Deque(_buf.header.va_addr,(void*)_header_data);
                    sw_enque_cnt = _header_data[Header_CRZBO_R1::E_ENQUE_CNT];
                    if(twin_status.Bits.TWIN_EN){
                        if(this->m_hwModule != (ISP_HW_MODULE)twin_status.Bits.MASTER_MODULE){
                            pWaitDrv = IspDrvImp::createInstance((ISP_HW_MODULE)twin_status.Bits.MASTER_MODULE);
                        }
                        else
                            pWaitDrv = (IspDrv*)pDrv;
                    }
                    break;
                case _crzo_r2_:
                    fh_crzo_r2.Header_Deque(_buf.header.va_addr,(void*)_header_data);
                    sw_enque_cnt = _header_data[Header_CRZO_R2::E_ENQUE_CNT];
                    if(twin_status.Bits.TWIN_EN){
                        if(this->m_hwModule != (ISP_HW_MODULE)twin_status.Bits.MASTER_MODULE){
                            pWaitDrv = IspDrvImp::createInstance((ISP_HW_MODULE)twin_status.Bits.MASTER_MODULE);
                        }
                        else
                            pWaitDrv = (IspDrv*)pDrv;
                    }
                    break;
                case _crzbo_r2_:
                    fh_crzbo_r2.Header_Deque(_buf.header.va_addr,(void*)_header_data);
                    sw_enque_cnt = _header_data[Header_CRZBO_R2::E_ENQUE_CNT];
                    if(twin_status.Bits.TWIN_EN){
                        if(this->m_hwModule != (ISP_HW_MODULE)twin_status.Bits.MASTER_MODULE){
                            pWaitDrv = IspDrvImp::createInstance((ISP_HW_MODULE)twin_status.Bits.MASTER_MODULE);
                        }
                        else
                            pWaitDrv = (IspDrv*)pDrv;
                    }
                    break;
                case _aao_:
                case _afo_:
                case _pdo_:
                case _flko_:
                case _tsfso_:
                default:
                    CAM_FUNC_ERR("un-supported DMAO:0x%x\n",this->id());
                    return eCmd_Fail;
                break;
            }

            if(loopCnt == _LOOP_){

                //because rcnt_in is locate on CQ, and m_buf_cnt is updated directly when enque.
                //so it will have a time difference between FBC hw cnt latch and m_buf_cnt update.
                //so can't be just using FbcCnt != m_buf_cnt
                if(sw_enque_cnt>0){
                    snprintf(str, sizeof(str), "dma:%s already have data on dram, [PA:0x%x]\n",\
                                                           this->name_Str(), _header_data[Header_IMGO::E_IMG_PA]);
                    ret = eCmd_Pass;
                    goto EXIT;
                }
                else{
                    snprintf(str, sizeof(str), "dma:%s start wait:[enque record:SW_buf_cnt:0x%x, SW_enq_cnt:0x%x, FbcCnt:0x%x],",\
                                                           this->name_Str(), this->m_buf_cnt, sw_enque_cnt, FbcCnt);
                }
            }
            else{ //enter second time of loop
                char _tmp[16] = {0,};

                snprintf(_tmp, sizeof(_tmp), "[%d]time,",((_LOOP_-loopCnt)+1));
                strncat(str,_tmp, strlen(_tmp));
            }
        }
        else{
            if(loopCnt == _LOOP_){
                snprintf(str, sizeof(str), "dma:%s bWaitBufReady = 1,wait for sw enque[0x%x_0x%x],timeout:%d x %d ms,[1] ",\
                    this->name_Str(),this->m_buf_cnt,subSample,irq.Timeout,loopCnt);
            }
            else{//save how many times of wait_irq before sw enque
                char _tmp[8];
                snprintf(_tmp, sizeof(_tmp), "[%d],",((_LOOP_-loopCnt)+1) + 1);
                strncat(str,_tmp, strlen(_tmp));
            }
        }

        //
        //check if status == suspending and return ture , in order to make sure all request in driver is poped when suspend.
        CAM_FUNC_DBG("camstate:%d\n",CAM_BUF_CTRL::m_CamState[this->m_hwModule]);
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
            if(pNotify->fpNotifyState(CAM_ENTER_WAIT, pNotify->Obj) == NOTIFY_FAIL){
                CAM_FUNC_ERR("dma:%s NotifyCB fail",this->name_Str());
                ret = eCmd_Fail;
            }
        }

        if(pWaitDrv == NULL)
            pWaitDrv = (IspDrv*)pDrv;
        if (pWaitDrv->waitIrq(&irq) == MFALSE) {
            ret = eCmd_Fail;
        } else {
            ret = eCmd_Pass;
        }

        if (pNotify) {
            if(pNotify->fpNotifyState(CAM_EXIT_WAIT, pNotify->Obj) == NOTIFY_FAIL){
                CAM_FUNC_ERR("dma:%s NotifyCB fail",this->name_Str());
                ret = eCmd_Fail;
            }
        }

        CAM_FUNC_DBG("camstate:%d\n",CAM_BUF_CTRL::m_CamState[this->m_hwModule]);
        switch (CAM_BUF_CTRL::m_CamState[this->m_hwModule]) {
            case CAM_BUF_CTRL::eCamState_runnable:
            case CAM_BUF_CTRL::eCamState_suspend:
                break;
            case CAM_BUF_CTRL::eCamState_stop:
                CAM_FUNC_INF("%s fast stop %s\n",this->name_Str(),str);
                ret = eCmd_Stop_Pass;
                goto EXIT;
                break;
            case CAM_BUF_CTRL::eCamState_suspending:
                //return ture , in order to make sure all request in driver is poped when suspend.
                ret = eCmd_Suspending_Pass;
                snprintf(str, sizeof(str), "suspending pass, ");
                goto EXIT;
                break;
            default:
                CAM_FUNC_ERR("Unknown state: %d\n", CAM_BUF_CTRL::m_CamState[this->m_hwModule]);
                break;
        }

        if((ret == eCmd_Pass) && (this->m_buf_cnt != 0) && (this->m_buf_cnt >= subSample) ){

            //add for timing error when deque twice between dmao_done to p1_done.
            //currently , only imgo/rrzo using p1 done, so only these 2 dmao need to check.
            //mt6757 need not this check, even in subsample flow

            //For secure camera
            if(SecMgr::SecMgr_GetSecurePortStatus(this->m_hwModule,this->id())){
                SecMgr *mpSecMgr = SecMgr::SecMgr_GetMgrObj();
                SecMgr_SecInfo secinfo;

                secinfo.type = SECMEM_FRAME_HEADER;
                secinfo.module = secModule;
                secinfo.buff_va = _buf.header.va_addr;
                secinfo.port = this->id();

                CAM_FUNC_INF("2-Migrate FH: port:0x%x fh_va:0x%lx buff:0x%x",secinfo.port,secinfo.buff_va,_buf.image.mem_info.pa_addr);
                if(mpSecMgr->SecMgr_P1MigrateTable(&secinfo) == MFALSE){
                    CAM_FUNC_ERR("Migrate sec FH failed:(0x%x)\n",this->id());
                    return eCmd_Fail;
                }
            }

            switch(this->id()){
                case _imgo_:
                    {
                        Header_IMGO fh_imgo;
                        ST_BUF_INFO _buf;

                        MUINT32 _header_data[E_HEADER_MAX];
                        this->m_Queue.sendCmd(QueueMgr<ST_BUF_INFO>::eCmd_at,(subSample-1),(MUINTPTR)&_buf);

                        fh_imgo.Header_Deque(_buf.header.va_addr,(void*)_header_data);

                        if( _header_data[Header_IMGO::E_IMG_PA] == 0){
                            CAM_FUNC_INF("%s:wait 1 more signal for header output\n",this->name_Str());
                            bWaitPass = MFALSE;
                        }
                        else
                            bWaitPass = MTRUE;
                    }
                    break;
                case _rrzo_:
                    {
                        Header_RRZO fh_rrzo;
                        ST_BUF_INFO _buf;

                        MUINT32 _header_data[E_HEADER_MAX];
                        this->m_Queue.sendCmd(QueueMgr<ST_BUF_INFO>::eCmd_at,(subSample-1),(MUINTPTR)&_buf);

                        fh_rrzo.Header_Deque(_buf.header.va_addr,(void*)_header_data);

                        if( _header_data[Header_RRZO::E_IMG_PA] == 0){
                            CAM_FUNC_INF("%s:wait 1 more signal for header output\n",this->name_Str());
                            bWaitPass = MFALSE;
                        }
                        else
                            bWaitPass = MTRUE;

                    }
                    break;
                case _ufeo_:
                    {
                        Header_UFEO fh_ufeo;
                        ST_BUF_INFO _buf;

                        MUINT32 _header_data[E_HEADER_MAX];
                        this->m_Queue.sendCmd(QueueMgr<ST_BUF_INFO>::eCmd_at,(subSample-1),(MUINTPTR)&_buf);
                        fh_ufeo.Header_Deque(_buf.header.va_addr,(void*)_header_data);

                        if( _header_data[Header_UFEO::E_IMG_PA] == 0){
                            CAM_FUNC_INF("%s:wait 1 more signal for header output\n",this->name_Str());
                            bWaitPass = MFALSE;
                        }
                        else
                            bWaitPass = MTRUE;

                    }
                    break;
                case _ufgo_:
                    {
                        Header_UFGO fh_ufgo;
                        ST_BUF_INFO _buf;

                        MUINT32 _header_data[E_HEADER_MAX];
                        this->m_Queue.sendCmd(QueueMgr<ST_BUF_INFO>::eCmd_at,(subSample-1),(MUINTPTR)&_buf);
                        fh_ufgo.Header_Deque(_buf.header.va_addr,(void*)_header_data);

                        if( _header_data[Header_UFGO::E_IMG_PA] == 0){
                            CAM_FUNC_INF("%s:wait 1 more signal for header output\n",this->name_Str());
                            bWaitPass = MFALSE;
                        }
                        else
                            bWaitPass = MTRUE;

                    }
                    break;
                case _lcso_:
                    {
                        Header_LCSO fh_dmao;
                        ST_BUF_INFO _buf;

                        MUINT32 _header_data[E_HEADER_MAX];
                        this->m_Queue.sendCmd(QueueMgr<ST_BUF_INFO>::eCmd_at,(subSample-1),(MUINTPTR)&_buf);

                        fh_dmao.Header_Deque(_buf.header.va_addr,(void*)_header_data);

                        if( _header_data[Header_LCSO::E_IMG_PA] == 0){
                            CAM_FUNC_INF("%s:wait 1 more signal for header output\n",this->name_Str());
                            bWaitPass = MFALSE;
                        }
                        else
                            bWaitPass = MTRUE;

                    }
                    break;
                case _lmvo_:
                    {
                        Header_LMVO fh_dmao;
                        ST_BUF_INFO _buf;

                        MUINT32 _header_data[E_HEADER_MAX];
                        this->m_Queue.sendCmd(QueueMgr<ST_BUF_INFO>::eCmd_at,(subSample-1),(MUINTPTR)&_buf);

                        fh_dmao.Header_Deque(_buf.header.va_addr,(void*)_header_data);

                        if( _header_data[Header_LMVO::E_IMG_PA] == 0){
                            CAM_FUNC_INF("%s:wait 1 more signal for header output\n",this->name_Str());
                            bWaitPass = MFALSE;
                        }
                        else
                            bWaitPass = MTRUE;

                    }
                    break;
                case _rsso_:
                    {
                        Header_RSSO fh_dmao;
                        ST_BUF_INFO _buf;

                        MUINT32 _header_data[E_HEADER_MAX];
                        this->m_Queue.sendCmd(QueueMgr<ST_BUF_INFO>::eCmd_at,(subSample-1),(MUINTPTR)&_buf);

                        fh_rsso.Header_Deque(_buf.header.va_addr,(void*)_header_data);

                        if( _header_data[Header_RSSO::E_IMG_PA] == 0){
                            CAM_FUNC_INF("%s:wait 1 more signal for header output\n",this->name_Str());
                            bWaitPass = MFALSE;
                        }
                        else
                            bWaitPass = MTRUE;

                    }
                    break;
                case _yuvo_:
                    {
                        Header_YUVO fh_yuvo;
                        ST_BUF_INFO _buf;

                        MUINT32 _header_data[E_HEADER_MAX];
                        this->m_Queue.sendCmd(QueueMgr<ST_BUF_INFO>::eCmd_at,(subSample-1),(MUINTPTR)&_buf);

                        fh_yuvo.Header_Deque(_buf.header.va_addr,(void*)_header_data);

                        if( _header_data[Header_YUVO::E_IMG_PA] == 0){
                            CAM_FUNC_INF("%s:wait 1 more signal for header output\n",this->name_Str());
                            bWaitPass = MFALSE;
                        }
                        else
                            bWaitPass = MTRUE;
                    }
                    break;
                case _yuvbo_:
                    {
                        Header_YUVBO fh_yuvbo;
                        ST_BUF_INFO _buf;

                        MUINT32 _header_data[E_HEADER_MAX];
                        this->m_Queue.sendCmd(QueueMgr<ST_BUF_INFO>::eCmd_at,(subSample-1),(MUINTPTR)&_buf);

                        fh_yuvbo.Header_Deque(_buf.header.va_addr,(void*)_header_data);

                        if( _header_data[Header_YUVBO::E_IMG_PA] == 0){
                            CAM_FUNC_INF("%s:wait 1 more signal for header output\n",this->name_Str());
                            bWaitPass = MFALSE;
                        }
                        else
                            bWaitPass = MTRUE;
                    }
                    break;
                case _yuvco_:
                    {
                        Header_YUVCO fh_yuvco;
                        ST_BUF_INFO _buf;

                        MUINT32 _header_data[E_HEADER_MAX];
                        this->m_Queue.sendCmd(QueueMgr<ST_BUF_INFO>::eCmd_at,(subSample-1),(MUINTPTR)&_buf);

                        fh_yuvco.Header_Deque(_buf.header.va_addr,(void*)_header_data);

                        if( _header_data[Header_YUVCO::E_IMG_PA] == 0){
                            CAM_FUNC_INF("%s:wait 1 more signal for header output\n",this->name_Str());
                            bWaitPass = MFALSE;
                        }
                        else
                            bWaitPass = MTRUE;
                    }
                    break;
                case _crzo_:
                    {
                        Header_CRZO_R1 fh_crzo_r1;
                        ST_BUF_INFO _buf;

                        MUINT32 _header_data[E_HEADER_MAX];
                        this->m_Queue.sendCmd(QueueMgr<ST_BUF_INFO>::eCmd_at,(subSample-1),(MUINTPTR)&_buf);

                        fh_crzo_r1.Header_Deque(_buf.header.va_addr,(void*)_header_data);

                        if( _header_data[Header_CRZO_R1::E_IMG_PA] == 0){
                            CAM_FUNC_INF("%s:wait 1 more signal for header output\n",this->name_Str());
                            bWaitPass = MFALSE;
                        }
                        else
                            bWaitPass = MTRUE;
                    }
                    break;
                case _crzbo_:
                    {
                        Header_CRZBO_R1 fh_crzbo_r1;
                        ST_BUF_INFO _buf;

                        MUINT32 _header_data[E_HEADER_MAX];
                        this->m_Queue.sendCmd(QueueMgr<ST_BUF_INFO>::eCmd_at,(subSample-1),(MUINTPTR)&_buf);

                        fh_crzbo_r1.Header_Deque(_buf.header.va_addr,(void*)_header_data);

                        if( _header_data[Header_CRZBO_R1::E_IMG_PA] == 0){
                            CAM_FUNC_INF("%s:wait 1 more signal for header output\n",this->name_Str());
                            bWaitPass = MFALSE;
                        }
                        else
                            bWaitPass = MTRUE;
                    }
                    break;
                case _crzo_r2_:
                    {
                        Header_CRZO_R2 fh_crzo_r2;
                        ST_BUF_INFO _buf;

                        MUINT32 _header_data[E_HEADER_MAX];
                        this->m_Queue.sendCmd(QueueMgr<ST_BUF_INFO>::eCmd_at,(subSample-1),(MUINTPTR)&_buf);

                        fh_crzo_r2.Header_Deque(_buf.header.va_addr,(void*)_header_data);

                        if( _header_data[Header_CRZO_R2::E_IMG_PA] == 0){
                            CAM_FUNC_INF("%s:wait 1 more signal for header output\n",this->name_Str());
                            bWaitPass = MFALSE;
                        }
                        else
                            bWaitPass = MTRUE;
                    }
                    break;
                case _crzbo_r2_:
                    {
                        Header_CRZBO_R2 fh_crzbo_r2;
                        ST_BUF_INFO _buf;

                        MUINT32 _header_data[E_HEADER_MAX];
                        this->m_Queue.sendCmd(QueueMgr<ST_BUF_INFO>::eCmd_at,(subSample-1),(MUINTPTR)&_buf);

                        fh_crzbo_r2.Header_Deque(_buf.header.va_addr,(void*)_header_data);

                        if( _header_data[Header_CRZBO_R2::E_IMG_PA] == 0){
                            CAM_FUNC_INF("%s:wait 1 more signal for header output\n",this->name_Str());
                            bWaitPass = MFALSE;
                        }
                        else
                            bWaitPass = MTRUE;
                    }
                    break;
                default:
                    break;
            }
            if(bWaitPass == MTRUE)
                break;//leave while loop
        }
        //
        irq.Clear = ISP_IRQ_CLEAR_NONE;
    }while(--loopCnt > 0);
EXIT:
    if(ret == eCmd_Fail){
        if(this->m_buf_cnt != 0){
            CAM_FUNC_ERR("%s, waitbufready fail. start fail check\n",str);
            this->PipeCheck();
        }
        else{
            CAM_FUNC_ERR("%s, waitbufready fail is caused by no enque\n",str);
        }
    }
    else{
        switch(this->id()){
            case _imgo_:
            case _rrzo_:
            case _yuvo_:
            case _crzo_:
            case _crzo_r2_:
                CAM_FUNC_DBG("%s, waitbufready pass\n",str);
                break;
            default:
                CAM_FUNC_DBG("%s, waitbufready pass\n",str);
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

    switch(this->id()){
        case _imgo_:
            CAM_WRITE_REG(this->m_pDrv,FBC_R1_FBC_IMGO_R1_CTL1,0);
            break;
        case _rrzo_:
            CAM_WRITE_REG(this->m_pDrv,FBC_R1_FBC_RRZO_R1_CTL1,0);
            break;
        case _ufeo_:
            CAM_WRITE_REG(this->m_pDrv,FBC_R1_FBC_UFEO_R1_CTL1,0);
            break;
        case _ufgo_:
            CAM_WRITE_REG(this->m_pDrv,FBC_R1_FBC_UFGO_R1_CTL1,0);
            break;
        case _aao_:
            if (cq == ISP_DRV_CQ_THRE4) {
                CAM_WRITE_REG(this->m_pDrv->getPhyObj(),FBC_R1_FBC_AAO_R1_CTL1,0);
            } else {
                CAM_WRITE_REG(this->m_pDrv,FBC_R1_FBC_AAO_R1_CTL1,0);
            }
            break;
        case _afo_:
            if(cq == ISP_DRV_CQ_THRE5){
                CAM_WRITE_REG(this->m_pDrv->getPhyObj(),FBC_R1_FBC_AFO_R1_CTL1,0);
            }else{
                CAM_WRITE_REG(this->m_pDrv,FBC_R1_FBC_AFO_R1_CTL1,0);
            }
            break;
        case _pdo_:
            if (cq == ISP_DRV_CQ_THRE7) {
                CAM_WRITE_REG(this->m_pDrv->getPhyObj(),FBC_R1_FBC_PDO_R1_CTL1,0);
            } else {
                CAM_WRITE_REG(this->m_pDrv,FBC_R1_FBC_PDO_R1_CTL1,0);
            }
            break;
        case _lcso_:
            CAM_WRITE_REG(this->m_pDrv,FBC_R1_FBC_LCESO_R1_CTL1,0);
            break;
        case _lmvo_:
            CAM_WRITE_REG(this->m_pDrv,FBC_R1_FBC_LMVO_R1_CTL1,0);
            break;
        case _flko_:
            if (cq == ISP_DRV_CQ_THRE8) {
                CAM_WRITE_REG(this->m_pDrv->getPhyObj(),FBC_R1_FBC_FLKO_R1_CTL1,0);
            } else {
                CAM_WRITE_REG(this->m_pDrv,FBC_R1_FBC_FLKO_R1_CTL1,0);
            }
            break;
        case _rsso_:
            CAM_WRITE_REG(this->m_pDrv,FBC_R1_FBC_RSSO_R1_CTL1,0);
            break;
        case _tsfso_:
            if (cq == ISP_DRV_CQ_THRE12) {
                CAM_WRITE_REG(this->m_pDrv->getPhyObj(),FBC_R1_FBC_TSFSO_R1_CTL1,0);
            } else {
                CAM_WRITE_REG(this->m_pDrv,FBC_R1_FBC_TSFSO_R1_CTL1,0);
            }
            break;
        case _yuvo_:
            CAM_WRITE_REG(this->m_pDrv,FBC_R1_FBC_YUVO_R1_CTL1,0);
            break;
        case _yuvbo_:
            CAM_WRITE_REG(this->m_pDrv,FBC_R1_FBC_YUVBO_R1_CTL1,0);
            break;
        case _yuvco_:
            CAM_WRITE_REG(this->m_pDrv,FBC_R1_FBC_YUVCO_R1_CTL1,0);
            break;
        case _crzo_:
            CAM_WRITE_REG(this->m_pDrv,FBC_R1_FBC_CRZO_R1_CTL1,0);
            break;
        case _crzbo_:
            CAM_WRITE_REG(this->m_pDrv,FBC_R1_FBC_CRZBO_R1_CTL1,0);
            break;
        case _crzo_r2_:
            CAM_WRITE_REG(this->m_pDrv,FBC_R1_FBC_CRZO_R2_CTL1,0);
            break;
        case _crzbo_r2_:
            CAM_WRITE_REG(this->m_pDrv,FBC_R1_FBC_CRZBO_R2_CTL1,0);
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

    switch(this->id()){
        case _imgo_:
            CAM_WRITE_BITS(this->m_pDrv,FBC_R1_FBC_IMGO_R1_CTL1,FBC_IMGO_R1_SUB_RATIO,0);
            CAM_WRITE_BITS(this->m_pDrv,FBC_R1_FBC_IMGO_R1_CTL1,FBC_IMGO_R1_FBC_EN,1);
            CAM_WRITE_BITS(this->m_pDrv,FBC_R1_FBC_IMGO_R1_CTL1,FBC_IMGO_R1_FBC_MODE,1);
            CAM_WRITE_BITS(this->m_pDrv,CAMCTL_R1_CAMCTL_FBC_GROUP,CAMCTL_IMGO_R1_FIFO_FULL_DROP,1);
            break;
        case _rrzo_:
            CAM_WRITE_BITS(this->m_pDrv,FBC_R1_FBC_RRZO_R1_CTL1,FBC_RRZO_R1_SUB_RATIO,0);
            CAM_WRITE_BITS(this->m_pDrv,FBC_R1_FBC_RRZO_R1_CTL1,FBC_RRZO_R1_FBC_EN,1);
            CAM_WRITE_BITS(this->m_pDrv,FBC_R1_FBC_RRZO_R1_CTL1,FBC_RRZO_R1_FBC_MODE,1);
            CAM_WRITE_BITS(this->m_pDrv,CAMCTL_R1_CAMCTL_FBC_GROUP,CAMCTL_RRZO_R1_FIFO_FULL_DROP,1);
            break;
        case _ufeo_:
            CAM_WRITE_BITS(this->m_pDrv,FBC_R1_FBC_UFEO_R1_CTL1,FBC_UFEO_R1_SUB_RATIO,0);
            CAM_WRITE_BITS(this->m_pDrv,FBC_R1_FBC_UFEO_R1_CTL1,FBC_UFEO_R1_FBC_EN,1);
            CAM_WRITE_BITS(this->m_pDrv,FBC_R1_FBC_UFEO_R1_CTL1,FBC_UFEO_R1_FBC_MODE,1);
            CAM_WRITE_BITS(this->m_pDrv,CAMCTL_R1_CAMCTL_FBC_GROUP,CAMCTL_UFEO_R1_FIFO_FULL_DROP,1);
            break;
        case _ufgo_:
            CAM_WRITE_BITS(this->m_pDrv,FBC_R1_FBC_UFGO_R1_CTL1,FBC_UFGO_R1_SUB_RATIO,0);
            CAM_WRITE_BITS(this->m_pDrv,FBC_R1_FBC_UFGO_R1_CTL1,FBC_UFGO_R1_FBC_EN,1);
            CAM_WRITE_BITS(this->m_pDrv,FBC_R1_FBC_UFGO_R1_CTL1,FBC_UFGO_R1_FBC_MODE,1);
            CAM_WRITE_BITS(this->m_pDrv,CAMCTL_R1_CAMCTL_FBC_GROUP,CAMCTL_UFGO_R1_FIFO_FULL_DROP,1);
            break;
        case _aao_:
            if(pParam == NULL) {
                CAM_FUNC_ERR("%s: Null pParam\n",this->name_Str());
                break;
            }
            if (cq == ISP_DRV_CQ_THRE4) {
                CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),FBC_R1_FBC_AAO_R1_CTL1,FBC_AAO_R1_SUB_RATIO,*(MUINT32*)pParam);
                CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),FBC_R1_FBC_AAO_R1_CTL1,FBC_AAO_R1_FBC_EN,1);
                CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),FBC_R1_FBC_AAO_R1_CTL1,FBC_AAO_R1_FBC_MODE,1);
#if AAO_FBC_LOCK_EN
                CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),FBC_R1_FBC_AAO_R1_CTL1,FBC_AAO_R1_LOCK_EN,0);
#endif
                CAM_FUNC_INF("dma:%d scenario-level func.\n",this->id());

            } else {
                CAM_WRITE_BITS(this->m_pDrv,FBC_R1_FBC_AAO_R1_CTL1,FBC_AAO_R1_SUB_RATIO,*(MUINT32*)pParam);
                CAM_WRITE_BITS(this->m_pDrv,FBC_R1_FBC_AAO_R1_CTL1,FBC_AAO_R1_FBC_EN,1);
                CAM_WRITE_BITS(this->m_pDrv,FBC_R1_FBC_AAO_R1_CTL1,FBC_AAO_R1_FBC_MODE,1);

            }
            break;
        case _afo_:
            if(pParam == NULL) {
                CAM_FUNC_ERR("%s: Null pParam\n",this->name_Str());
                break;
            }
            if(cq == ISP_DRV_CQ_THRE5){
                CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),FBC_R1_FBC_AFO_R1_CTL1,FBC_AFO_R1_SUB_RATIO,*(MUINT32*)pParam);
                CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),FBC_R1_FBC_AFO_R1_CTL1,FBC_AFO_R1_FBC_EN,1);
                CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),FBC_R1_FBC_AFO_R1_CTL1,FBC_AFO_R1_FBC_MODE,1);
                CAM_FUNC_INF("dma:%d scenario-level func.\n",this->id());
            } else {
                CAM_WRITE_BITS(this->m_pDrv,FBC_R1_FBC_AFO_R1_CTL1,FBC_AFO_R1_SUB_RATIO,*(MUINT32*)pParam);
                CAM_WRITE_BITS(this->m_pDrv,FBC_R1_FBC_AFO_R1_CTL1,FBC_AFO_R1_FBC_EN,1);
                CAM_WRITE_BITS(this->m_pDrv,FBC_R1_FBC_AFO_R1_CTL1,FBC_AFO_R1_FBC_MODE,1);
                CAM_FUNC_DBG("dma:%d frame-level func.\n",this->id());
            }
            break;
        case _pdo_:
            if(pParam == NULL) {
                CAM_FUNC_ERR("%s: Null pParam\n",this->name_Str());
                break;
            }
            if (cq == ISP_DRV_CQ_THRE7) {
                CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),FBC_R1_FBC_PDO_R1_CTL1,FBC_PDO_R1_SUB_RATIO,*(MUINT32*)pParam);
                CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),FBC_R1_FBC_PDO_R1_CTL1,FBC_PDO_R1_FBC_EN,1);
                CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),FBC_R1_FBC_PDO_R1_CTL1,FBC_PDO_R1_FBC_MODE,1);
                CAM_FUNC_INF("dma:%d scenario-level func.\n",this->id());
            } else {
                CAM_WRITE_BITS(this->m_pDrv,FBC_R1_FBC_PDO_R1_CTL1,FBC_PDO_R1_SUB_RATIO,*(MUINT32*)pParam);
                CAM_WRITE_BITS(this->m_pDrv,FBC_R1_FBC_PDO_R1_CTL1,FBC_PDO_R1_FBC_EN,1);
                CAM_WRITE_BITS(this->m_pDrv,FBC_R1_FBC_PDO_R1_CTL1,FBC_PDO_R1_FBC_MODE,1);
                CAM_FUNC_DBG("dma:%d frame-level func.\n",this->id());
            }
            break;
        case _lcso_:
            CAM_WRITE_BITS(this->m_pDrv,FBC_R1_FBC_LCESO_R1_CTL1,FBC_LCESO_R1_SUB_RATIO,0);
            CAM_WRITE_BITS(this->m_pDrv,FBC_R1_FBC_LCESO_R1_CTL1,FBC_LCESO_R1_FBC_EN,1);
            CAM_WRITE_BITS(this->m_pDrv,FBC_R1_FBC_LCESO_R1_CTL1,FBC_LCESO_R1_FBC_MODE,1);
            CAM_WRITE_BITS(this->m_pDrv,CAMCTL_R1_CAMCTL_FBC_GROUP,CAMCTL_LCESO_R1_FIFO_FULL_DROP,1);
            break;
        case _lmvo_:
            CAM_WRITE_BITS(this->m_pDrv,FBC_R1_FBC_LMVO_R1_CTL1,FBC_LMVO_R1_SUB_RATIO,0);
            CAM_WRITE_BITS(this->m_pDrv,FBC_R1_FBC_LMVO_R1_CTL1,FBC_LMVO_R1_FBC_EN,1);
            CAM_WRITE_BITS(this->m_pDrv,FBC_R1_FBC_LMVO_R1_CTL1,FBC_LMVO_R1_FBC_MODE,1);
            CAM_WRITE_BITS(this->m_pDrv,CAMCTL_R1_CAMCTL_FBC_GROUP,CAMCTL_LMVO_R1_FIFO_FULL_DROP,1);
            break;
        case _flko_:
            if(pParam == NULL) {
                CAM_FUNC_ERR("%s: Null pParam\n",this->name_Str());
                break;
            }
            if (cq == ISP_DRV_CQ_THRE8) {
                CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),FBC_R1_FBC_FLKO_R1_CTL1,FBC_FLKO_R1_SUB_RATIO,*(MUINT32*)pParam);
                CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),FBC_R1_FBC_FLKO_R1_CTL1,FBC_FLKO_R1_FBC_EN,1);
                CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),FBC_R1_FBC_FLKO_R1_CTL1,FBC_FLKO_R1_FBC_MODE,1);
                CAM_FUNC_INF("dma:%d scenario-level func.\n",this->id());
            } else {
                CAM_WRITE_BITS(this->m_pDrv,FBC_R1_FBC_FLKO_R1_CTL1,FBC_FLKO_R1_SUB_RATIO,*(MUINT32*)pParam);
                CAM_WRITE_BITS(this->m_pDrv,FBC_R1_FBC_FLKO_R1_CTL1,FBC_FLKO_R1_FBC_EN,1);
                CAM_WRITE_BITS(this->m_pDrv,FBC_R1_FBC_FLKO_R1_CTL1,FBC_FLKO_R1_FBC_MODE,1);
                CAM_FUNC_DBG("dma:%d frame-level func.\n",this->id());
            }
            break;
        case _rsso_:
            CAM_WRITE_BITS(this->m_pDrv,FBC_R1_FBC_RSSO_R1_CTL1,FBC_RSSO_R1_SUB_RATIO,0);
            CAM_WRITE_BITS(this->m_pDrv,FBC_R1_FBC_RSSO_R1_CTL1,FBC_RSSO_R1_FBC_EN,1);
            CAM_WRITE_BITS(this->m_pDrv,FBC_R1_FBC_RSSO_R1_CTL1,FBC_RSSO_R1_FBC_MODE,1);
            CAM_WRITE_BITS(this->m_pDrv,CAMCTL_R1_CAMCTL_FBC_GROUP,CAMCTL_RSSO_R1_FIFO_FULL_DROP,1);
            break;
        case _tsfso_:
            if(pParam == NULL) {
                CAM_FUNC_ERR("%s: Null pParam\n",this->name_Str());
                break;
            }
            if (cq == ISP_DRV_CQ_THRE12) {
                CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),FBC_R1_FBC_TSFSO_R1_CTL1,FBC_TSFSO_R1_SUB_RATIO,*(MUINT32*)pParam);
                CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),FBC_R1_FBC_TSFSO_R1_CTL1,FBC_TSFSO_R1_FBC_EN,1);
                CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),FBC_R1_FBC_TSFSO_R1_CTL1,FBC_TSFSO_R1_FBC_MODE,1);
                CAM_FUNC_INF("dma:%d scenario-level func.\n",this->id());
            } else {
                CAM_WRITE_BITS(this->m_pDrv,FBC_R1_FBC_TSFSO_R1_CTL1,FBC_TSFSO_R1_SUB_RATIO,*(MUINT32*)pParam);
                CAM_WRITE_BITS(this->m_pDrv,FBC_R1_FBC_TSFSO_R1_CTL1,FBC_TSFSO_R1_FBC_EN,1);
                CAM_WRITE_BITS(this->m_pDrv,FBC_R1_FBC_TSFSO_R1_CTL1,FBC_TSFSO_R1_FBC_MODE,1);
                CAM_FUNC_DBG("dma:%d frame-level func.\n",this->id());
            }
            break;
        case _yuvo_:
            CAM_WRITE_BITS(this->m_pDrv,FBC_R1_FBC_YUVO_R1_CTL1,FBC_YUVO_R1_SUB_RATIO,0);
            CAM_WRITE_BITS(this->m_pDrv,FBC_R1_FBC_YUVO_R1_CTL1,FBC_YUVO_R1_FBC_EN,1);
            CAM_WRITE_BITS(this->m_pDrv,FBC_R1_FBC_YUVO_R1_CTL1,FBC_YUVO_R1_FBC_MODE,1);
            CAM_WRITE_BITS(this->m_pDrv,CAMCTL_R1_CAMCTL_FBC_GROUP,CAMCTL_YUVO_R1_FIFO_FULL_DROP,1);
            break;
        case _yuvbo_:
            CAM_WRITE_BITS(this->m_pDrv,FBC_R1_FBC_YUVBO_R1_CTL1,FBC_YUVBO_R1_SUB_RATIO,0);
            CAM_WRITE_BITS(this->m_pDrv,FBC_R1_FBC_YUVBO_R1_CTL1,FBC_YUVBO_R1_FBC_EN,1);
            CAM_WRITE_BITS(this->m_pDrv,FBC_R1_FBC_YUVBO_R1_CTL1,FBC_YUVBO_R1_FBC_MODE,1);
            CAM_WRITE_BITS(this->m_pDrv,CAMCTL_R1_CAMCTL_FBC_GROUP,CAMCTL_YUVBO_R1_FIFO_FULL_DROP,1);
            break;
        case _yuvco_:
            CAM_WRITE_BITS(this->m_pDrv,FBC_R1_FBC_YUVCO_R1_CTL1,FBC_YUVCO_R1_SUB_RATIO,0);
            CAM_WRITE_BITS(this->m_pDrv,FBC_R1_FBC_YUVCO_R1_CTL1,FBC_YUVCO_R1_FBC_EN,1);
            CAM_WRITE_BITS(this->m_pDrv,FBC_R1_FBC_YUVCO_R1_CTL1,FBC_YUVCO_R1_FBC_MODE,1);
            CAM_WRITE_BITS(this->m_pDrv,CAMCTL_R1_CAMCTL_FBC_GROUP,CAMCTL_YUVCO_R1_FIFO_FULL_DROP,1);
            break;
        case _crzo_:
            CAM_WRITE_BITS(this->m_pDrv,FBC_R1_FBC_CRZO_R1_CTL1,FBC_CRZO_R1_SUB_RATIO,0);
            CAM_WRITE_BITS(this->m_pDrv,FBC_R1_FBC_CRZO_R1_CTL1,FBC_CRZO_R1_FBC_EN,1);
            CAM_WRITE_BITS(this->m_pDrv,FBC_R1_FBC_CRZO_R1_CTL1,FBC_CRZO_R1_FBC_MODE,1);
            CAM_WRITE_BITS(this->m_pDrv,CAMCTL_R1_CAMCTL_FBC_GROUP,CAMCTL_CRZO_R1_FIFO_FULL_DROP,1);
            break;
        case _crzbo_:
            CAM_WRITE_BITS(this->m_pDrv,FBC_R1_FBC_CRZBO_R1_CTL1,FBC_CRZBO_R1_SUB_RATIO,0);
            CAM_WRITE_BITS(this->m_pDrv,FBC_R1_FBC_CRZBO_R1_CTL1,FBC_CRZBO_R1_FBC_EN,1);
            CAM_WRITE_BITS(this->m_pDrv,FBC_R1_FBC_CRZBO_R1_CTL1,FBC_CRZBO_R1_FBC_MODE,1);
            CAM_WRITE_BITS(this->m_pDrv,CAMCTL_R1_CAMCTL_FBC_GROUP,CAMCTL_CRZBO_R1_FIFO_FULL_DROP,1);
            break;
        case _crzo_r2_:
            CAM_WRITE_BITS(this->m_pDrv,FBC_R1_FBC_CRZO_R2_CTL1,FBC_CRZO_R2_SUB_RATIO,0);
            CAM_WRITE_BITS(this->m_pDrv,FBC_R1_FBC_CRZO_R2_CTL1,FBC_CRZO_R2_FBC_EN,1);
            CAM_WRITE_BITS(this->m_pDrv,FBC_R1_FBC_CRZO_R2_CTL1,FBC_CRZO_R2_FBC_MODE,1);
            CAM_WRITE_BITS(this->m_pDrv,CAMCTL_R1_CAMCTL_FBC_GROUP,CAMCTL_CRZO_R2_FIFO_FULL_DROP,1);
            break;
        case _crzbo_r2_:
            CAM_WRITE_BITS(this->m_pDrv,FBC_R1_FBC_CRZBO_R2_CTL1,FBC_CRZBO_R2_SUB_RATIO,0);
            CAM_WRITE_BITS(this->m_pDrv,FBC_R1_FBC_CRZBO_R2_CTL1,FBC_CRZBO_R2_FBC_EN,1);
            CAM_WRITE_BITS(this->m_pDrv,FBC_R1_FBC_CRZBO_R2_CTL1,FBC_CRZBO_R2_FBC_MODE,1);
            CAM_WRITE_BITS(this->m_pDrv,CAMCTL_R1_CAMCTL_FBC_GROUP,CAMCTL_CRZBO_R2_FIFO_FULL_DROP,1);
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
    MUINT32 bPhy = MFALSE;
    (void)pParam;

    this->m_pDrv = (ISP_DRV_CAM*)this->m_pIspDrv;
        //dbg log
    this->m_pDrv->getCurObjInfo(&this->m_hwModule,&cq,&page);

    CAM_FUNC_DBG("%s::_disable: cq:0x%x,page:0x%x\n",this->name_Str(),cq,page);

    switch(this->id()){
        case _imgo_:
            CAM_WRITE_REG(this->m_pDrv,FBC_R1_FBC_IMGO_R1_CTL1,0);
            break;
        case _rrzo_:
            CAM_WRITE_REG(this->m_pDrv,FBC_R1_FBC_RRZO_R1_CTL1,0);
            break;
        case _ufeo_:
            CAM_WRITE_REG(this->m_pDrv,FBC_R1_FBC_UFEO_R1_CTL1,0);
            CAM_WRITE_BITS(this->m_pDrv,CAMCTL_R1_CAMCTL_FBC_RCNT_INC,CAMCTL_UFEO_R1_RCNT_INC,0);
            break;
        case _ufgo_:
            CAM_WRITE_REG(this->m_pDrv,FBC_R1_FBC_UFGO_R1_CTL1,0);
            CAM_WRITE_BITS(this->m_pDrv,CAMCTL_R1_CAMCTL_FBC_RCNT_INC,CAMCTL_UFGO_R1_RCNT_INC,0);
            break;
        case _aao_:
#if AAO_FBC_LOCK_EN
            CAM_WRITE_REG(this->m_pDrv->getPhyObj(),FBC_R1_FBC_AAO_R1_CTL1,0x20000);
#else
            CAM_WRITE_REG(this->m_pDrv->getPhyObj(),FBC_R1_FBC_AAO_R1_CTL1,0);
#endif
            CAM_FUNC_INF("dma:%d scenario-level func.\n",this->id());
            bPhy = MTRUE;
            break;
        case _afo_:
            CAM_WRITE_REG(this->m_pDrv->getPhyObj(),FBC_R1_FBC_AFO_R1_CTL1,0);
            CAM_FUNC_INF("dma:%d scenario-level func.\n",this->id());
            bPhy = MTRUE;
            break;
        case _pdo_:
            if (cq == ISP_DRV_CQ_THRE7) {
                CAM_WRITE_REG(this->m_pDrv->getPhyObj(),FBC_R1_FBC_PDO_R1_CTL1,0);
                CAM_FUNC_INF("dma:%d scenario-level func.\n",this->id());
            }
            else{
                CAM_WRITE_REG(this->m_pDrv,FBC_R1_FBC_PDO_R1_CTL1,0);
                CAM_FUNC_INF("dma:%d frame-level func.\n",this->id());
            }
            bPhy = MTRUE;
            break;
        case _flko_:
            CAM_WRITE_REG(this->m_pDrv->getPhyObj(),FBC_R1_FBC_FLKO_R1_CTL1,0);
            CAM_FUNC_INF("dma:%d scenario-level func.\n",this->id());
            bPhy = MTRUE;
            break;
        case _lcso_:
            CAM_WRITE_REG(this->m_pDrv,FBC_R1_FBC_LCESO_R1_CTL1,0);
            break;
        case _lmvo_:
            CAM_WRITE_REG(this->m_pDrv,FBC_R1_FBC_LMVO_R1_CTL1,0);
            break;
        case _rsso_:
            CAM_WRITE_REG(this->m_pDrv,FBC_R1_FBC_RSSO_R1_CTL1,0);
            break;
        case _tsfso_:
            CAM_WRITE_REG(this->m_pDrv->getPhyObj(),FBC_R1_FBC_TSFSO_R1_CTL1,0);
            CAM_FUNC_INF("dma:%d scenario-level func.\n",this->id());
            bPhy = MTRUE;
            break;
        case _yuvo_:
            CAM_WRITE_REG(this->m_pDrv,FBC_R1_FBC_YUVO_R1_CTL1,0);
            break;
        case _yuvbo_:
            CAM_WRITE_REG(this->m_pDrv,FBC_R1_FBC_YUVBO_R1_CTL1,0);
            break;
        case _yuvco_:
            CAM_WRITE_REG(this->m_pDrv,FBC_R1_FBC_YUVCO_R1_CTL1,0);
            break;
        case _crzo_:
            CAM_WRITE_REG(this->m_pDrv,FBC_R1_FBC_CRZO_R1_CTL1,0);
            break;
        case _crzbo_:
            CAM_WRITE_REG(this->m_pDrv,FBC_R1_FBC_CRZBO_R1_CTL1,0);
            break;
        case _crzo_r2_:
            CAM_WRITE_REG(this->m_pDrv,FBC_R1_FBC_CRZO_R2_CTL1,0);
            break;
        case _crzbo_r2_:
            CAM_WRITE_REG(this->m_pDrv,FBC_R1_FBC_CRZBO_R2_CTL1,0);
            break;
        default:
            CAM_FUNC_ERR("un-supported DMAO:0x%x\n",this->id());
            return 1;
            break;
    }

    if(bPhy){
        //STT::stop is before ISP stop, this log will print at STT:stop
        if(CAM_READ_BITS(this->m_pDrv->getPhyObj(),TG_R1_TG_VF_CON,TG_VFDATA_EN) == 1){
            CAM_FUNC_WRN("%s: dma:%d: disable FBC at streaming\n",this->name_Str(),this->id());
        }
    }

    return 0;
}

MINT32 CAM_BUF_CTRL::_write2CQ( MUINT32 burstIndex )
{
    E_ISP_CAM_CQ cq;
    MUINT32 page;
    this->m_pDrv = (ISP_DRV_CAM*)this->m_pIspDrv;
    //dbg log
    this->m_pDrv->getCurObjInfo(&this->m_hwModule,&cq,&page);
    CAM_FUNC_DBG("%s::_write2CQ: cq:0x%x,page:0x%x,subsample:0x%x\n",this->name_Str(),cq,page,burstIndex);

    if(burstIndex > 0){
        return 0;
    }

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
        case _ufgo_:
            this->m_pDrv->cqAddModule(CAM_FBC_UFGO_);
            break;
        case _lmvo_:
            this->m_pDrv->cqAddModule(CAM_FBC_LMVO_);
            break;
        case _rsso_:
            this->m_pDrv->cqAddModule(CAM_FBC_RSSO_);
            break;
        case _lcso_:
            this->m_pDrv->cqAddModule(CAM_FBC_LCSO_);
            break;
        case _aao_:
            this->m_pDrv->cqAddModule(CAM_FBC_AAO_);
            break;
        case _pdo_:
            this->m_pDrv->cqAddModule(CAM_FBC_PDO_);
            break;
        case _flko_:
            this->m_pDrv->cqAddModule(CAM_FBC_FLKO_);
            break;
        case _afo_:
            this->m_pDrv->cqAddModule(CAM_FBC_AFO_);
            break;
        case _tsfso_:
            this->m_pDrv->cqAddModule(CAM_FBC_TSFSO_);
            break;
        case _yuvo_:
            this->m_pDrv->cqAddModule(CAM_FBC_YUVO_);
            break;
        case _yuvbo_:
            this->m_pDrv->cqAddModule(CAM_FBC_YUVBO_);
            break;
        case _yuvco_:
            this->m_pDrv->cqAddModule(CAM_FBC_YUVCO_);
            break;
        case _crzo_:
            this->m_pDrv->cqAddModule(CAM_FBC_CRZO_);
            break;
        case _crzbo_:
            this->m_pDrv->cqAddModule(CAM_FBC_CRZBO_);
            break;
        case _crzo_r2_:
            this->m_pDrv->cqAddModule(CAM_FBC_CRZO_R2_);
            break;
        case _crzbo_r2_:
            this->m_pDrv->cqAddModule(CAM_FBC_CRZBO_R2_);
            break;
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
    MUINT32 page = 0;
    MUINT32 _sof_idx = this->m_hwModule; // assign hwModule as input argument
    MUINT32 _size = 0, subSample = 0;
    ST_BUF_INFO _buf;

    this->m_pDrv = (ISP_DRV_CAM*)this->m_pIspDrv;
    (void)bImdMode;

    Mutex::Autolock lock(this->m_bufctrl_lock);

    //dbg log
    this->m_pDrv->getCurObjInfo(&this->m_hwModule,&cq,&page);

    subSample = CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAMCTL_R1_CAMCTL_SW_PASS1_DONE,CAMCTL_DOWN_SAMPLE_PERIOD) + 1;

    if ( 0xffffffff == this->id() ) {
        CAM_FUNC_ERR("dma channel error\n");
        return eCmd_Fail;
    }

    //
    this->m_pDrv->getDeviceInfo(_GET_SOF_CNT,(MUINT8 *) &_sof_idx);
    //fbc reg dbg log
    this->FBC_STATUS(this->m_pDrv->getPhyObj());

    //
    if(buf_info.bReplace == MTRUE){
        ptr = &buf_info.Replace;
    }
    else{
        ptr = &buf_info.u_op.enque.at(ePlane_1st);
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

    //ion handle: buffer
    this->ion_handle_lock(this->m_pDrv,MTRUE,ptr->image.mem_info.memID,ptr->header.memID);

    //
    this->m_Queue.sendCmd(QueueMgr<ST_BUF_INFO>::eCmd_push,(MUINTPTR)ptr,0);
    this->m_buf_cnt++;

    //
    switch(this->id()){
        case _imgo_:
            {
                Header_IMGO fh_imgo;

                fh_imgo.m_hwModule = this->m_hwModule;
                CAM_WRITE_BITS(this->m_pDrv,CAMCTL_R1_CAMCTL_FBC_RCNT_INC,CAMCTL_IMGO_R1_RCNT_INC,1);
                //
                fh_imgo.Header_Enque(Header_IMGO::E_ENQUE_SOF,this->m_pDrv,_sof_idx);
                fh_imgo.Header_Enque(Header_IMGO::E_ENQUE_CNT,this->m_pDrv,this->m_buf_cnt);

                this->m_pDrv->m_HWmasterModule = (CAM_READ_BITS(this->m_pDrv, CAMCTL_R1_CAMCTL_TWIN_STATUS, TWIN_EN) == MTRUE) ?
                                                 ((ISP_HW_MODULE)CAM_READ_BITS(this->m_pDrv, CAMCTL_R1_CAMCTL_TWIN_STATUS, MASTER_MODULE)) : (this->m_hwModule);
            }
            break;
        case _rrzo_:
            {
                Header_RRZO fh_rrzo;

                fh_rrzo.m_hwModule = this->m_hwModule;
                CAM_WRITE_BITS(this->m_pDrv,CAMCTL_R1_CAMCTL_FBC_RCNT_INC,CAMCTL_RRZO_R1_RCNT_INC,1);
                //
                fh_rrzo.Header_Enque(Header_RRZO::E_ENQUE_SOF,this->m_pDrv,_sof_idx);
                fh_rrzo.Header_Enque(Header_RRZO::E_ENQUE_CNT,this->m_pDrv,this->m_buf_cnt);

                this->m_pDrv->m_HWmasterModule = (CAM_READ_BITS(this->m_pDrv, CAMCTL_R1_CAMCTL_TWIN_STATUS, TWIN_EN) == MTRUE) ?
                                                 ((ISP_HW_MODULE)CAM_READ_BITS(this->m_pDrv, CAMCTL_R1_CAMCTL_TWIN_STATUS, MASTER_MODULE)) : (this->m_hwModule);
            }
            break;
        case _ufeo_:
            {
                Header_UFEO fh_ufeo;

                fh_ufeo.m_hwModule = this->m_hwModule;
                CAM_WRITE_BITS(this->m_pDrv,CAMCTL_R1_CAMCTL_FBC_RCNT_INC,CAMCTL_UFEO_R1_RCNT_INC,1);
                //
                fh_ufeo.Header_Enque(Header_UFEO::E_ENQUE_SOF,this->m_pDrv,_sof_idx);
                fh_ufeo.Header_Enque(Header_UFEO::E_ENQUE_CNT,this->m_pDrv,this->m_buf_cnt);

                this->m_pDrv->m_HWmasterModule = (CAM_READ_BITS(this->m_pDrv, CAMCTL_R1_CAMCTL_TWIN_STATUS, TWIN_EN) == MTRUE) ?
                                                 ((ISP_HW_MODULE)CAM_READ_BITS(this->m_pDrv, CAMCTL_R1_CAMCTL_TWIN_STATUS, MASTER_MODULE)) : (this->m_hwModule);
            }
            break;
        case _ufgo_:
            {
                Header_UFGO fh_ufgo;

                fh_ufgo.m_hwModule = this->m_hwModule;
                CAM_WRITE_BITS(this->m_pDrv,CAMCTL_R1_CAMCTL_FBC_RCNT_INC,CAMCTL_UFGO_R1_RCNT_INC,1);
                //
                fh_ufgo.Header_Enque(Header_UFGO::E_ENQUE_SOF,this->m_pDrv,_sof_idx);
                fh_ufgo.Header_Enque(Header_UFGO::E_ENQUE_CNT,this->m_pDrv,this->m_buf_cnt);

                this->m_pDrv->m_HWmasterModule = (CAM_READ_BITS(this->m_pDrv, CAMCTL_R1_CAMCTL_TWIN_STATUS, TWIN_EN) == MTRUE) ?
                                                 ((ISP_HW_MODULE)CAM_READ_BITS(this->m_pDrv, CAMCTL_R1_CAMCTL_TWIN_STATUS, MASTER_MODULE)) : (this->m_hwModule);
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
                if(CAM_READ_BITS(this->m_pDrv->getPhyObj(),FBC_R1_FBC_AAO_R1_CTL1,FBC_AAO_R1_FBC_EN) != 1){
                    CAM_FUNC_ERR("%s: can't enque before enabe fbc\n",this->name_Str());
                    return eCmd_Fail;
                }
                CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),CAMCTL_R1_CAMCTL_FBC_RCNT_INC,CAMCTL_AAO_R1_RCNT_INC,1);

                //
                Buf_ctrl.ctrl = SET_FH_SPARE;
                Buf_ctrl.dma_PA = ptr->image.mem_info.pa_addr;
                this->m_pDrv->cqRingBuf(&Buf_ctrl);
                fh_aao.Header_Enque(Header_AAO::E_ENQUE_SOF,(ISP_DRV_CAM*)Buf_ctrl.pDma_fh_spare,_sof_idx);

                this->m_pDrv->m_HWmasterModule = (CAM_READ_BITS(this->m_pDrv->getPhyObj(), CAMCTL_R1_CAMCTL_TWIN_STATUS, TWIN_EN) == MTRUE) ?
                                                 ((ISP_HW_MODULE)CAM_READ_BITS(this->m_pDrv->getPhyObj(), CAMCTL_R1_CAMCTL_TWIN_STATUS, MASTER_MODULE)) : (this->m_hwModule);
            }
            break;
        case _afo_:
            {
                CQ_RingBuf_ST Buf_ctrl;
                Header_AFO  fh_afo;
                capibility CamInfo;

                fh_afo.m_hwModule = this->m_hwModule;
                Buf_ctrl.ctrl = GET_RING_DEPTH;

                if((this->id() == _afo_) && (CamInfo.m_DTwin.GetDTwin() == MTRUE)) {
                    CAM_FUNC_ERR("%s, in D-Twin case, afo should use by ccu.",this->name_Str());
                    return eCmd_Fail;
                }
                if(this->m_pDrv->cqRingBuf(&Buf_ctrl) == 0){
                    CAM_FUNC_ERR("%s, can't enque before PA push into CQ\n",this->name_Str());
                    return eCmd_Fail;
                }
                //statistic pipe enque to hw directly

                if(CAM_READ_BITS(this->m_pDrv->getPhyObj(),FBC_R1_FBC_AFO_R1_CTL1,FBC_AFO_R1_FBC_EN) != 1){
                    CAM_FUNC_ERR("%s: can't enque before enabe fbc\n",this->name_Str());
                    return eCmd_Fail;
                }

                CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),CAMCTL_R1_CAMCTL_FBC_RCNT_INC,CAMCTL_AFO_R1_RCNT_INC,1);

                //
                Buf_ctrl.ctrl = SET_FH_SPARE;
                Buf_ctrl.dma_PA = ptr->image.mem_info.pa_addr;
                this->m_pDrv->cqRingBuf(&Buf_ctrl);
                fh_afo.Header_Enque(Header_AFO::E_ENQUE_SOF,(ISP_DRV_CAM*)Buf_ctrl.pDma_fh_spare,_sof_idx);

                this->m_pDrv->m_HWmasterModule = (CAM_READ_BITS(this->m_pDrv->getPhyObj(), CAMCTL_R1_CAMCTL_TWIN_STATUS, TWIN_EN) == MTRUE) ?
                                                 ((ISP_HW_MODULE)CAM_READ_BITS(this->m_pDrv->getPhyObj(), CAMCTL_R1_CAMCTL_TWIN_STATUS, MASTER_MODULE)) : (this->m_hwModule);
            }
            break;
        case _lcso_:
            {
                Header_LCSO fh_lcso;

                fh_lcso.m_hwModule = this->m_hwModule;
                #if 0 //burst page not add fbc module
                if(CAM_READ_BITS(this->m_pDrv,FBC_R1_FBC_LCESO_R1_CTL1,FBC_LCESO_R1_FBC_EN) != 1){
                    CAM_FUNC_ERR("%s: can't enque before enabe fbc\n",this->name_Str());
                    return eCmd_Fail;
                }
                #endif
                CAM_WRITE_BITS(this->m_pDrv,CAMCTL_R1_CAMCTL_FBC_RCNT_INC,CAMCTL_LCESO_R1_RCNT_INC,1);
                //
                fh_lcso.Header_Enque(Header_LCSO::E_ENQUE_SOF,this->m_pDrv,_sof_idx);
                fh_lcso.Header_Enque(Header_LCSO::E_ENQUE_CNT,this->m_pDrv,this->m_buf_cnt);

                this->m_pDrv->m_HWmasterModule = (CAM_READ_BITS(this->m_pDrv, CAMCTL_R1_CAMCTL_TWIN_STATUS, TWIN_EN) == MTRUE) ?
                                                 ((ISP_HW_MODULE)CAM_READ_BITS(this->m_pDrv, CAMCTL_R1_CAMCTL_TWIN_STATUS, MASTER_MODULE)) : (this->m_hwModule);
            }
            break;
        case _lmvo_:
            {
                Header_LMVO fh_lmvo;

                fh_lmvo.m_hwModule = this->m_hwModule;
                #if 0 //burst page not add fbc module
                if(CAM_READ_BITS(this->m_pDrv,FBC_R1_FBC_LMVO_R1_CTL1,FBC_LMVO_R1_FBC_EN) != 1){
                    CAM_FUNC_ERR("%s: can't enque before enabe fbc\n",this->name_Str());
                    return eCmd_Fail;
                }
                #endif
                CAM_WRITE_BITS(this->m_pDrv,CAMCTL_R1_CAMCTL_FBC_RCNT_INC,CAMCTL_LMVO_R1_RCNT_INC,1);
                //
                fh_lmvo.Header_Enque(Header_LMVO::E_ENQUE_SOF,this->m_pDrv,_sof_idx);
                fh_lmvo.Header_Enque(Header_LMVO::E_ENQUE_CNT,this->m_pDrv,this->m_buf_cnt);

                this->m_pDrv->m_HWmasterModule = (CAM_READ_BITS(this->m_pDrv, CAMCTL_R1_CAMCTL_TWIN_STATUS, TWIN_EN) == MTRUE) ?
                                                 ((ISP_HW_MODULE)CAM_READ_BITS(this->m_pDrv, CAMCTL_R1_CAMCTL_TWIN_STATUS, MASTER_MODULE)) : (this->m_hwModule);
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
                if(CAM_READ_BITS(this->m_pDrv->getPhyObj(),FBC_R1_FBC_FLKO_R1_CTL1,FBC_FLKO_R1_FBC_EN) != 1){
                    CAM_FUNC_ERR("%s: can't enque before enabe fbc\n",this->name_Str());
                    return eCmd_Fail;
                }
                CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),CAMCTL_R1_CAMCTL_FBC_RCNT_INC,CAMCTL_FLKO_R1_RCNT_INC,1);
                //
                Buf_ctrl.ctrl = SET_FH_SPARE;
                Buf_ctrl.dma_PA = ptr->image.mem_info.pa_addr;
                this->m_pDrv->cqRingBuf(&Buf_ctrl);
                fh_flko.Header_Enque(Header_FLKO::E_ENQUE_SOF,(ISP_DRV_CAM*)Buf_ctrl.pDma_fh_spare,_sof_idx);

                this->m_pDrv->m_HWmasterModule = (CAM_READ_BITS(this->m_pDrv->getPhyObj(), CAMCTL_R1_CAMCTL_TWIN_STATUS, TWIN_EN) == MTRUE) ?
                                                 ((ISP_HW_MODULE)CAM_READ_BITS(this->m_pDrv->getPhyObj(), CAMCTL_R1_CAMCTL_TWIN_STATUS, MASTER_MODULE)) : (this->m_hwModule);
            }
            break;
        case _rsso_:
            {
                Header_RSSO fh_rsso;

                fh_rsso.m_hwModule = this->m_hwModule;
                #if 0 //burst page not add fbc module
                if(CAM_READ_BITS(this->m_pDrv,FBC_R1_FBC_RSSO_R1_CTL1,FBC_RSSO_R1_FBC_EN) != 1){
                    CAM_FUNC_ERR("%s: can't enque before enabe fbc\n",this->name_Str());
                    return eCmd_Fail;
                }
                #endif
                CAM_WRITE_BITS(this->m_pDrv,CAMCTL_R1_CAMCTL_FBC_RCNT_INC,CAMCTL_RSSO_R1_RCNT_INC,1);
                //
                fh_rsso.Header_Enque(Header_RSSO::E_ENQUE_SOF,this->m_pDrv,_sof_idx);
                fh_rsso.Header_Enque(Header_RSSO::E_ENQUE_CNT,this->m_pDrv,this->m_buf_cnt);

                this->m_pDrv->m_HWmasterModule = (CAM_READ_BITS(this->m_pDrv, CAMCTL_R1_CAMCTL_TWIN_STATUS, TWIN_EN) == MTRUE) ?
                                                 ((ISP_HW_MODULE)CAM_READ_BITS(this->m_pDrv, CAMCTL_R1_CAMCTL_TWIN_STATUS, MASTER_MODULE)) : (this->m_hwModule);
            }
            break;
        case _tsfso_:
            {
                CQ_RingBuf_ST Buf_ctrl;
                Header_TSFSO fh_tsfso;

                fh_tsfso.m_hwModule = this->m_hwModule;

                Buf_ctrl.ctrl = GET_RING_DEPTH;
                if(this->m_pDrv->cqRingBuf(&Buf_ctrl) == 0){
                    CAM_FUNC_ERR("%s, can't enque before PA push into CQ\n",this->name_Str());
                    return eCmd_Fail;
                }
                //statistic pipe enque to hw directly
                if(CAM_READ_BITS(this->m_pDrv->getPhyObj(),FBC_R1_FBC_TSFSO_R1_CTL1,FBC_TSFSO_R1_FBC_EN) != 1){
                    CAM_FUNC_ERR("%s: can't enque before enabe fbc\n",this->name_Str());
                    return eCmd_Fail;
                }
                CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),CAMCTL_R1_CAMCTL_FBC_RCNT_INC,CAMCTL_TSFSO_R1_RCNT_INC,1);
                //
                Buf_ctrl.ctrl = SET_FH_SPARE;
                Buf_ctrl.dma_PA = ptr->image.mem_info.pa_addr;
                this->m_pDrv->cqRingBuf(&Buf_ctrl);
                fh_tsfso.Header_Enque(Header_TSFSO::E_ENQUE_SOF,(ISP_DRV_CAM*)Buf_ctrl.pDma_fh_spare,_sof_idx);

                this->m_pDrv->m_HWmasterModule = (CAM_READ_BITS(this->m_pDrv->getPhyObj(), CAMCTL_R1_CAMCTL_TWIN_STATUS, TWIN_EN) == MTRUE) ?
                                                 ((ISP_HW_MODULE)CAM_READ_BITS(this->m_pDrv->getPhyObj(), CAMCTL_R1_CAMCTL_TWIN_STATUS, MASTER_MODULE)) : (this->m_hwModule);
            }
            break;
        case _yuvo_:
            {
                Header_YUVO fh_yuvo;

                fh_yuvo.m_hwModule = this->m_hwModule;
                CAM_WRITE_BITS(this->m_pDrv,CAMCTL_R1_CAMCTL_FBC_RCNT_INC,CAMCTL_YUVO_R1_RCNT_INC,1);
                //
                fh_yuvo.Header_Enque(Header_YUVO::E_ENQUE_SOF,this->m_pDrv,_sof_idx);
                fh_yuvo.Header_Enque(Header_YUVO::E_ENQUE_CNT,this->m_pDrv,this->m_buf_cnt);

                this->m_pDrv->m_HWmasterModule = (CAM_READ_BITS(this->m_pDrv, CAMCTL_R1_CAMCTL_TWIN_STATUS, TWIN_EN) == MTRUE) ?
                                                 ((ISP_HW_MODULE)CAM_READ_BITS(this->m_pDrv, CAMCTL_R1_CAMCTL_TWIN_STATUS, MASTER_MODULE)) : (this->m_hwModule);
            }
            break;
        case _yuvbo_:
            {
                Header_YUVBO fh_yuvbo;

                fh_yuvbo.m_hwModule = this->m_hwModule;
                CAM_WRITE_BITS(this->m_pDrv,CAMCTL_R1_CAMCTL_FBC_RCNT_INC,CAMCTL_YUVBO_R1_RCNT_INC,1);
                //
                fh_yuvbo.Header_Enque(Header_YUVBO::E_ENQUE_SOF,this->m_pDrv,_sof_idx);
                fh_yuvbo.Header_Enque(Header_YUVBO::E_ENQUE_CNT,this->m_pDrv,this->m_buf_cnt);

                this->m_pDrv->m_HWmasterModule = (CAM_READ_BITS(this->m_pDrv, CAMCTL_R1_CAMCTL_TWIN_STATUS, TWIN_EN) == MTRUE) ?
                                                 ((ISP_HW_MODULE)CAM_READ_BITS(this->m_pDrv, CAMCTL_R1_CAMCTL_TWIN_STATUS, MASTER_MODULE)) : (this->m_hwModule);
            }
            break;
        case _yuvco_:
            {
                Header_YUVCO fh_yuvco;

                fh_yuvco.m_hwModule = this->m_hwModule;
                CAM_WRITE_BITS(this->m_pDrv,CAMCTL_R1_CAMCTL_FBC_RCNT_INC,CAMCTL_YUVCO_R1_RCNT_INC,1);
                //
                fh_yuvco.Header_Enque(Header_YUVCO::E_ENQUE_SOF,this->m_pDrv,_sof_idx);
                fh_yuvco.Header_Enque(Header_YUVCO::E_ENQUE_CNT,this->m_pDrv,this->m_buf_cnt);

                this->m_pDrv->m_HWmasterModule = (CAM_READ_BITS(this->m_pDrv, CAMCTL_R1_CAMCTL_TWIN_STATUS, TWIN_EN) == MTRUE) ?
                                                 ((ISP_HW_MODULE)CAM_READ_BITS(this->m_pDrv, CAMCTL_R1_CAMCTL_TWIN_STATUS, MASTER_MODULE)) : (this->m_hwModule);
            }
            break;
        case _crzo_:
            {
                Header_CRZO_R1 fh_crzo_r1;

                fh_crzo_r1.m_hwModule = this->m_hwModule;
                CAM_WRITE_BITS(this->m_pDrv,CAMCTL_R1_CAMCTL_FBC_RCNT_INC,CAMCTL_CRZO_R1_RCNT_INC,1);
                //
                fh_crzo_r1.Header_Enque(Header_CRZO_R1::E_ENQUE_SOF,this->m_pDrv,_sof_idx);
                fh_crzo_r1.Header_Enque(Header_CRZO_R1::E_ENQUE_CNT,this->m_pDrv,this->m_buf_cnt);

                this->m_pDrv->m_HWmasterModule = (CAM_READ_BITS(this->m_pDrv, CAMCTL_R1_CAMCTL_TWIN_STATUS, TWIN_EN) == MTRUE) ?
                                                 ((ISP_HW_MODULE)CAM_READ_BITS(this->m_pDrv, CAMCTL_R1_CAMCTL_TWIN_STATUS, MASTER_MODULE)) : (this->m_hwModule);
            }
            break;
        case _crzbo_:
            {
                Header_CRZBO_R1 fh_crzbo_r1;

                fh_crzbo_r1.m_hwModule = this->m_hwModule;
                CAM_WRITE_BITS(this->m_pDrv,CAMCTL_R1_CAMCTL_FBC_RCNT_INC,CAMCTL_CRZBO_R1_RCNT_INC,1);
                //
                fh_crzbo_r1.Header_Enque(Header_CRZBO_R1::E_ENQUE_SOF,this->m_pDrv,_sof_idx);
                fh_crzbo_r1.Header_Enque(Header_CRZBO_R1::E_ENQUE_CNT,this->m_pDrv,this->m_buf_cnt);

                this->m_pDrv->m_HWmasterModule = (CAM_READ_BITS(this->m_pDrv, CAMCTL_R1_CAMCTL_TWIN_STATUS, TWIN_EN) == MTRUE) ?
                                                 ((ISP_HW_MODULE)CAM_READ_BITS(this->m_pDrv, CAMCTL_R1_CAMCTL_TWIN_STATUS, MASTER_MODULE)) : (this->m_hwModule);
            }
            break;
        case _crzo_r2_:
            {
                Header_CRZO_R2 fh_crzo_r2;

                fh_crzo_r2.m_hwModule = this->m_hwModule;
                CAM_WRITE_BITS(this->m_pDrv,CAMCTL_R1_CAMCTL_FBC_RCNT_INC,CAMCTL_CRZO_R2_RCNT_INC,1);
                //
                fh_crzo_r2.Header_Enque(Header_CRZO_R2::E_ENQUE_SOF,this->m_pDrv,_sof_idx);
                fh_crzo_r2.Header_Enque(Header_CRZO_R2::E_ENQUE_CNT,this->m_pDrv,this->m_buf_cnt);

                this->m_pDrv->m_HWmasterModule = (CAM_READ_BITS(this->m_pDrv, CAMCTL_R1_CAMCTL_TWIN_STATUS, TWIN_EN) == MTRUE) ?
                                                 ((ISP_HW_MODULE)CAM_READ_BITS(this->m_pDrv, CAMCTL_R1_CAMCTL_TWIN_STATUS, MASTER_MODULE)) : (this->m_hwModule);
            }
            break;
        case _crzbo_r2_:
            {
                Header_CRZBO_R2 fh_crzbo_r2;

                fh_crzbo_r2.m_hwModule = this->m_hwModule;
                CAM_WRITE_BITS(this->m_pDrv,CAMCTL_R1_CAMCTL_FBC_RCNT_INC,CAMCTL_CRZBO_R2_RCNT_INC,1);
                //
                fh_crzbo_r2.Header_Enque(Header_CRZBO_R2::E_ENQUE_SOF,this->m_pDrv,_sof_idx);
                fh_crzbo_r2.Header_Enque(Header_CRZBO_R2::E_ENQUE_CNT,this->m_pDrv,this->m_buf_cnt);

                this->m_pDrv->m_HWmasterModule = (CAM_READ_BITS(this->m_pDrv, CAMCTL_R1_CAMCTL_TWIN_STATUS, TWIN_EN) == MTRUE) ?
                                                 ((ISP_HW_MODULE)CAM_READ_BITS(this->m_pDrv, CAMCTL_R1_CAMCTL_TWIN_STATUS, MASTER_MODULE)) : (this->m_hwModule);
            }
            break;
        default:
            CAM_FUNC_ERR("un-supported DMAO:0x%x\n",this->id());
            this->m_Queue.sendCmd(QueueMgr<ST_BUF_INFO>::eCmd_pop  ,0,0);
            return eCmd_Fail;
            break;
    }

    //
    {
        Header_RRZO fh_rrzo;
        Header_IMGO fh_imgo;
        MBOOL isUF = MFALSE;

        switch(this->id()){ /*this->id()*/
            case _imgo_:
                isUF = fh_imgo.GetRegInfo(Header_IMGO::E_IS_UFE_FMT,this->m_pDrv);
                break;
            case _rrzo_:
                isUF = fh_rrzo.GetRegInfo(Header_RRZO::E_IS_UFG_FMT,this->m_pDrv);
                break;
            default:
                break;
        }
        if(buf_info.bReplace == MFALSE){
            switch(this->id()){ /*this->id()*/
                case _imgo_:
                case _rrzo_:
                case _yuvo_:
                case _crzo_:
                case _crzbo_:
                case _crzo_r2_:
                case _crzbo_r2_:
                    if(this->id() == _rrzo_){
                        static MUINT32 RegDumpLoop = 0;
                        MUINT32 _mod = CAM_READ_BITS(this->m_pDrv, CAMCTL_R1_CAMCTL_TWIN_STATUS, SLAVE_CAM_NUM) + 1;
                        MUINT32 tmp = 0;
                        char value[PROPERTY_VALUE_MAX] = {'\0'};
                        property_get("vendor.camsys.dumpreg", value, "0");
                        tmp = atoi(value);
                        if(tmp){
                            if((RegDumpLoop++ % _mod) == 0)
                                this->reg_dump("vendor.camsys.dumpreg");
                        }
                        else{
                            RegDumpLoop = 0;
                        }
                    }
                    if (page % subSample == 0) {
                        CAM_FUNC_INF("%s:(0x%x_0x%x),PA(0x%" PRIXPTR "_0x%" PRIXPTR "),FH_VA(0x%" PRIXPTR "),size(0x%x),enque_sof(%d),isUF(%d)",\
                            this->name_Str(),\
                            cq,page,\
                            ptr->image.mem_info.pa_addr ,\
                            ptr->header.pa_addr ,\
                            ptr->header.va_addr ,\
                            ptr->image.mem_info.size ,\
                            _sof_idx,\
                            isUF);
                    }
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
                case _ufeo_:
                case _ufgo_:
                    CAM_FUNC_INF("%s:(0x%x_0x%x),PA(0x%" PRIXPTR "_0x%" PRIXPTR ",new:0x%" PRIXPTR "_0x%" PRIXPTR "),FH_VA(0x%" PRIXPTR ",new:0x%" PRIXPTR "),size(0x%x),enque_sof(%d),isUF(%d)",\
                        this->name_Str(),\
                        cq,page,\
                        ptr->image.mem_info.pa_addr ,\
                        ptr->header.pa_addr ,\
                        buf_info.u_op.enque.at(ePlane_1st).image.mem_info.pa_addr ,\
                        buf_info.u_op.enque.at(ePlane_1st).header.pa_addr ,\
                        ptr->header.va_addr ,\
                        buf_info.u_op.enque.at(ePlane_1st).header.va_addr ,\
                        ptr->image.mem_info.size ,\
                        _sof_idx,\
                        isUF);
                    break;
                default:
                    CAM_FUNC_DBG("%s:(0x%x_0x%x),PA(0x%" PRIXPTR "_0x%" PRIXPTR ",new:0x%" PRIXPTR "_0x%" PRIXPTR "),FH_VA(0x%" PRIXPTR ",new:0x%" PRIXPTR "),size(0x%x),enque_sof(%d)",\
                        this->name_Str(),\
                        cq,page,\
                        ptr->image.mem_info.pa_addr ,\
                        ptr->header.pa_addr ,\
                        buf_info.u_op.enque.at(ePlane_1st).image.mem_info.pa_addr ,\
                        buf_info.u_op.enque.at(ePlane_1st).header.pa_addr ,\
                        ptr->header.va_addr ,\
                        buf_info.u_op.enque.at(ePlane_1st).header.va_addr ,\
                        ptr->image.mem_info.size ,\
                        _sof_idx);
                    break;
            }
        }
    }
    this->m_Queue_deque_ptr.sendCmd(QueueMgr<MUINTPTR>::eCmd_push,(MUINTPTR)&this->m_pDrv,0);
    //
    return eCmd_Pass;

}

MBOOL CAM_BUF_CTRL::reg_dump(const char* pStr)
{
    REG_CAMCTL_R1_CAMCTL_TWIN_STATUS twin_status;
    twin_status.Raw = CAM_READ_REG(this->m_pDrv->getPhyObj(),CAMCTL_R1_CAMCTL_TWIN_STATUS);

    if( twin_status.Bits.MASTER_MODULE != this->m_hwModule){
        MUINT32 tmp = 0;
        char value[PROPERTY_VALUE_MAX] = {'\0'};
        property_get(pStr, value, "0");
        tmp = atoi(value);
        if(tmp){
            MUINT32 slave;
            MUINT32 TwinNum;
            ISP_DRV_CAM* ptr[PHY_CAM-1];
            memset((void*)ptr, 0, sizeof(ptr));
            TwinNum = twin_status.Bits.SLAVE_CAM_NUM;
            ptr[0] = (ISP_DRV_CAM*)ISP_DRV_CAM::createInstance( (ISP_HW_MODULE)twin_status.Bits.MASTER_MODULE,ISP_DRV_CQ_THRE0,0,"TWIN_DUMP");
            if(ptr[0]){
                ptr[0]->DumpReg(MTRUE);
                ptr[0]->destroyInstance();
            }
            for(MUINT32 i=0;i<TwinNum;i++){
                if(i==0){
                    slave = twin_status.Bits.TWIN_MODULE;
                    CAM_FUNC_ERR("1st slave cam:%d\n",slave);
                }
                else if(i==1){
                    CAM_FUNC_ERR("no 2nd slave cam\n");
                }

                CAM_FUNC_ERR("twin is enabled, start dump cam_%d register\n",slave);
                ptr[i] = (ISP_DRV_CAM*)ISP_DRV_CAM::createInstance( (ISP_HW_MODULE)slave,ISP_DRV_CQ_THRE0,0,"TWIN_DUMP");
                if(ptr[i]){
                    ptr[i]->DumpReg(MTRUE);
                    ptr[i]->destroyInstance();
                }
            }
        }
    }

    return MTRUE;
}

E_BC_STATUS CAM_BUF_CTRL::dequeueHwBuf(NSImageio::NSIspio::BufInfo& buf_info )
{
    E_BC_STATUS ret = eCmd_Pass;
    E_ISP_CAM_CQ cq = ISP_DRV_CQ_NONE;
    MUINT32 page = 0;
    MUINT32 _size = 0;
    MUINT32 _sof_idx = this->m_hwModule; // assign hwModule as input argument
    MUINT32 subSample = 0;
    ST_BUF_INFO _buf;
    MUINT32 _header_data[E_HEADER_MAX];
    char str[128] = {'\0'};
    ISP_DRV_CAM* pDrv = NULL;
    Mutex::Autolock lock(this->m_bufctrl_lock);

    if ( 0xffffffff == this->id() ) {
        CAM_FUNC_ERR("dma channel error ");
        ret = eCmd_Fail;
        goto EXIT;
    }

    subSample = CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAMCTL_R1_CAMCTL_SW_PASS1_DONE,CAMCTL_DOWN_SAMPLE_PERIOD) + 1;

    this->m_Queue_deque_ptr.sendCmd(QueueMgr<MUINTPTR>::eCmd_getsize  ,(MUINTPTR)&_size,0);
    if (_size == 0) {
        CAM_FUNC_ERR("no enque record, plz enque first");
        return eCmd_Fail;
    }

    this->m_Queue.sendCmd(QueueMgr<ST_BUF_INFO>::eCmd_getsize,(MUINTPTR)&_size,0);
    if(_size <1){
        CAM_FUNC_ERR("queue depth is empty for deque\n");
        ret = eCmd_Fail;
        goto EXIT;
    }

    this->m_Queue_deque_ptr.sendCmd(QueueMgr<MUINTPTR>::eCmd_front  ,(MUINTPTR)&pDrv,0);

    //dbg log
    pDrv->getCurObjInfo(&this->m_hwModule,&cq,&page);

    pDrv->getDeviceInfo(_GET_SOF_CNT,(MUINT8 *) &_sof_idx);

    //fbc reg dbg log
    this->FBC_STATUS(pDrv->getPhyObj());

    this->m_Queue.sendCmd(QueueMgr<ST_BUF_INFO>::eCmd_front  ,(MUINTPTR)&_buf,0);

    switch(CAM_BUF_CTRL::m_CamState[this->m_hwModule]){
        case CAM_BUF_CTRL::eCamState_runnable:
        case CAM_BUF_CTRL::eCamState_suspend:   //this timing is for twin deque is happaned after dynamic twin suspend ready
            //
            this->Header_Parser(_buf.header.va_addr, buf_info,_header_data,str, pDrv->m_HWmasterModule);
                        //
            if( _buf.image.mem_info.pa_addr != buf_info.u4BufPA[ePlane_1st]){
                CAM_FUNC_ERR("%s:PA in header is mismatch with PA in list [0x%" PRIXPTR "_0x%" PRIXPTR "]\n",this->name_Str(),\
                    buf_info.u4BufPA[ePlane_1st],_buf.image.mem_info.pa_addr);

                ret = eCmd_Fail;
                this->PipeCheck();
                goto EXIT;
            }
            break;
        case CAM_BUF_CTRL::eCamState_suspending:
            ret = eCmd_Suspending_Pass;
            //do not check header,dram on header's address is garbage!
            //need to pop all requests in driver when suspending for the drop method in iopipe.

            //Flush FH
            this->Header_Parser(_buf.header.va_addr, buf_info,_header_data,str, pDrv->m_HWmasterModule);

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
    buf_data_parsing(buf_info,ePlane_1st,_buf);

    //
    this->m_Queue.sendCmd(QueueMgr<ST_BUF_INFO>::eCmd_pop  ,0,0);
    this->m_Queue_deque_ptr.sendCmd(QueueMgr<MUINTPTR>::eCmd_pop  ,0,0);
    this->m_buf_cnt--;

    //ion handle: buffer
    this->ion_handle_lock(pDrv,MFALSE,_buf.image.mem_info.memID,_buf.header.memID);

EXIT:

    switch(this->id()){
        case _imgo_:
        case _rrzo_:
            if (page % subSample == 0) {
                CAM_FUNC_INF("%s:(0x%x_0x%x),PA(0x%" PRIXPTR "_0x%" PRIXPTR "),FH_VA(0x%" PRIXPTR "),size(0x%x),cur sof(%d),%s\n",\
                    this->name_Str(),\
                    cq,page,\
                    _buf.image.mem_info.pa_addr ,\
                    _buf.header.pa_addr ,\
                    _buf.header.va_addr ,\
                    _buf.image.mem_info.size ,\
                    _sof_idx,\
                    str);
            }
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

MBOOL CAM_BUF_CTRL::suspend(ISP_HW_MODULE camModule, ISP_DRV_CAM *pDrvCam, ISP_HW_MODULE srcTgModule, ISP_DRV_CAM *pTgDrvCam, E_SUSPEND_OP op)
{
    MBOOL ret;
    MUINT32 _t_ms, subSample = 1;
    if(op & eSus_HWOFF){
        ISP_WAIT_IRQ_STRUCT irq;

        subSample = CAM_READ_BITS(pDrvCam->getPhyObj(),CAMCTL_R1_CAMCTL_SW_PASS1_DONE,CAMCTL_DOWN_SAMPLE_PERIOD) + 1;

        switch(srcTgModule){
            case CAM_A:
                irq.Type = ISP_IRQ_TYPE_INT_CAM_A_ST;
                break;
            case CAM_B:
                irq.Type = ISP_IRQ_TYPE_INT_CAM_B_ST;
                break;
            case CAM_C:
                irq.Type = ISP_IRQ_TYPE_INT_CAM_C_ST;
                break;
            default:
                CAM_SFUNC_ERR("unsupported hwModule:0x%x\n", srcTgModule);
                return MFALSE;
                break;
        }

        /* estimateTimeout */
        _t_ms = (m_recentFrmTimeMs[camModule][0] >= m_recentFrmTimeMs[camModule][1])?
            m_recentFrmTimeMs[camModule][0]: m_recentFrmTimeMs[camModule][1];
        _t_ms *= subSample;
        _t_ms = (MIN_GRPFRM_TIME_MS > _t_ms)? MIN_GRPFRM_TIME_MS: _t_ms;
        _t_ms *= 2; /* x2 for enque post deque sem timing */

        irq.EventInfo.Clear   = ISP_IRQ_CLEAR_WAIT;
        irq.EventInfo.UserKey = 0;
        irq.EventInfo.St_type = SIGNAL_INT;
        irq.EventInfo.Status  = VS_INT_ST;
        irq.EventInfo.Timeout = _t_ms;

        CAM_SFUNC_INF("[%d] enter suspending, wait one vsync timeout:%d subSample:%d\n", srcTgModule, irq.EventInfo.Timeout, subSample);

        pTgDrvCam->setDeviceInfo(_SET_VF_OFF,(MUINT8*)&srcTgModule);

#if (BMX_AMX_DTWIN_WROK_AROUND == 1)
        //resume will write by CQ, no need to keep CAM_CTL_EN/CAM_CTL_EN2
        CAM_WRITE_REG(pDrvCam->getPhyObj(), CAMCTL_R1_CAMCTL_EN, 0);
        CAM_WRITE_REG(pDrvCam->getPhyObj(), CAMCTL_R1_CAMCTL_EN2, 0);
        CAM_WRITE_REG(pDrvCam->getPhyObj(), CAMCTL_R1_CAMCTL_EN3, 0);
        CAM_WRITE_REG(pDrvCam->getPhyObj(), CAMCTL_R1_CAMCTL_EN4, 0);
#endif

        ret = pTgDrvCam->waitIrq(&irq);
        if (ret == MFALSE) {
            CAM_SFUNC_ERR("[%d] wait vsync when enter suspend fail\n", srcTgModule);
        }
    }

    if(op & eSus_HW_SW_STATE){
        CAM_BUF_CTRL::updateState(CAM_BUF_CTRL::eCamState_suspending, camModule, CAM_BUF_CTRL::eCamDmaType_main);

        //reset FBC to avoid the latest cq_done setting rcnt_inc.
        //resume's enque will be affected by above behavior
        if(CAM_READ_BITS(pDrvCam->getPhyObj(),FBC_R1_FBC_IMGO_R1_CTL1,FBC_IMGO_R1_FBC_EN)) {
            CAM_WRITE_BITS(pDrvCam->getPhyObj(),FBC_R1_FBC_IMGO_R1_CTL1,FBC_IMGO_R1_FBC_RESET,1);
        }
        if(CAM_READ_BITS(pDrvCam->getPhyObj(),FBC_R1_FBC_RRZO_R1_CTL1,FBC_RRZO_R1_FBC_EN)) {
            CAM_WRITE_BITS(pDrvCam->getPhyObj(),FBC_R1_FBC_RRZO_R1_CTL1,FBC_RRZO_R1_FBC_RESET,1);
        }
        if(CAM_READ_BITS(pDrvCam->getPhyObj(),FBC_R1_FBC_UFEO_R1_CTL1,FBC_UFEO_R1_FBC_EN)){
            CAM_WRITE_BITS(pDrvCam->getPhyObj(),FBC_R1_FBC_UFEO_R1_CTL1,FBC_UFEO_R1_FBC_RESET,1);
        }
        if(CAM_READ_BITS(pDrvCam->getPhyObj(),FBC_R1_FBC_UFGO_R1_CTL1,FBC_UFGO_R1_FBC_EN)){
            CAM_WRITE_BITS(pDrvCam->getPhyObj(),FBC_R1_FBC_UFGO_R1_CTL1,FBC_UFGO_R1_FBC_RESET,1);
        }
        if(CAM_READ_BITS(pDrvCam->getPhyObj(),FBC_R1_FBC_LCESO_R1_CTL1,FBC_LCESO_R1_FBC_EN)){
            CAM_WRITE_BITS(pDrvCam->getPhyObj(),FBC_R1_FBC_LCESO_R1_CTL1,FBC_LCESO_R1_FBC_RESET,1);
        }
        if(CAM_READ_BITS(pDrvCam->getPhyObj(),FBC_R1_FBC_LMVO_R1_CTL1,FBC_LMVO_R1_FBC_EN)){
            CAM_WRITE_BITS(pDrvCam->getPhyObj(),FBC_R1_FBC_LMVO_R1_CTL1,FBC_LMVO_R1_FBC_RESET,1);
        }
        if(CAM_READ_BITS(pDrvCam->getPhyObj(),FBC_R1_FBC_RSSO_R1_CTL1,FBC_RSSO_R1_FBC_EN)){
            CAM_WRITE_BITS(pDrvCam->getPhyObj(),FBC_R1_FBC_RSSO_R1_CTL1,FBC_RSSO_R1_FBC_RESET,1);
        }
        if(CAM_READ_BITS(pDrvCam->getPhyObj(),FBC_R1_FBC_YUVO_R1_CTL1,FBC_YUVO_R1_FBC_EN)) {
            CAM_WRITE_BITS(pDrvCam->getPhyObj(),FBC_R1_FBC_YUVO_R1_CTL1,FBC_YUVO_R1_FBC_RESET,1);
        }
        if(CAM_READ_BITS(pDrvCam->getPhyObj(),FBC_R1_FBC_YUVBO_R1_CTL1,FBC_YUVBO_R1_FBC_EN)) {
            CAM_WRITE_BITS(pDrvCam->getPhyObj(),FBC_R1_FBC_YUVBO_R1_CTL1,FBC_YUVBO_R1_FBC_RESET,1);
        }
        if(CAM_READ_BITS(pDrvCam->getPhyObj(),FBC_R1_FBC_YUVCO_R1_CTL1,FBC_YUVCO_R1_FBC_EN)) {
            CAM_WRITE_BITS(pDrvCam->getPhyObj(),FBC_R1_FBC_YUVCO_R1_CTL1,FBC_YUVCO_R1_FBC_RESET,1);
        }
        if(CAM_READ_BITS(pDrvCam->getPhyObj(),FBC_R1_FBC_CRZO_R1_CTL1,FBC_CRZO_R1_FBC_EN)) {
            CAM_WRITE_BITS(pDrvCam->getPhyObj(),FBC_R1_FBC_CRZO_R1_CTL1,FBC_CRZO_R1_FBC_RESET,1);
        }
        if(CAM_READ_BITS(pDrvCam->getPhyObj(),FBC_R1_FBC_CRZBO_R1_CTL1,FBC_CRZBO_R1_FBC_EN)) {
            CAM_WRITE_BITS(pDrvCam->getPhyObj(),FBC_R1_FBC_CRZBO_R1_CTL1,FBC_CRZBO_R1_FBC_RESET,1);
        }
        if(CAM_READ_BITS(pDrvCam->getPhyObj(),FBC_R1_FBC_CRZO_R2_CTL1,FBC_CRZO_R2_FBC_EN)) {
            CAM_WRITE_BITS(pDrvCam->getPhyObj(),FBC_R1_FBC_CRZO_R2_CTL1,FBC_CRZO_R2_FBC_RESET,1);
        }
        if(CAM_READ_BITS(pDrvCam->getPhyObj(),FBC_R1_FBC_CRZBO_R2_CTL1,FBC_CRZBO_R2_FBC_EN)) {
            CAM_WRITE_BITS(pDrvCam->getPhyObj(),FBC_R1_FBC_CRZBO_R2_CTL1,FBC_CRZBO_R2_FBC_RESET,1);
        }

        if(CAM_READ_BITS(pDrvCam->getPhyObj(),FBC_R1_FBC_IMGO_R1_CTL1,FBC_IMGO_R1_FBC_EN)) {
            CAM_WRITE_BITS(pDrvCam->getPhyObj(),FBC_R1_FBC_IMGO_R1_CTL1,FBC_IMGO_R1_FBC_RESET,0);
        }
        if(CAM_READ_BITS(pDrvCam->getPhyObj(),FBC_R1_FBC_RRZO_R1_CTL1,FBC_RRZO_R1_FBC_EN)) {
            CAM_WRITE_BITS(pDrvCam->getPhyObj(),FBC_R1_FBC_RRZO_R1_CTL1,FBC_RRZO_R1_FBC_RESET,0);
        }
        if(CAM_READ_BITS(pDrvCam->getPhyObj(),FBC_R1_FBC_UFEO_R1_CTL1,FBC_UFEO_R1_FBC_EN)){
            CAM_WRITE_BITS(pDrvCam->getPhyObj(),FBC_R1_FBC_UFEO_R1_CTL1,FBC_UFEO_R1_FBC_RESET,0);
        }
        if(CAM_READ_BITS(pDrvCam->getPhyObj(),FBC_R1_FBC_UFGO_R1_CTL1,FBC_UFGO_R1_FBC_EN)){
            CAM_WRITE_BITS(pDrvCam->getPhyObj(),FBC_R1_FBC_UFGO_R1_CTL1,FBC_UFGO_R1_FBC_RESET,0);
        }
        if(CAM_READ_BITS(pDrvCam->getPhyObj(),FBC_R1_FBC_LCESO_R1_CTL1,FBC_LCESO_R1_FBC_EN)){
            CAM_WRITE_BITS(pDrvCam->getPhyObj(),FBC_R1_FBC_LCESO_R1_CTL1,FBC_LCESO_R1_FBC_RESET,0);
        }
        if(CAM_READ_BITS(pDrvCam->getPhyObj(),FBC_R1_FBC_LMVO_R1_CTL1,FBC_LMVO_R1_FBC_EN)){
            CAM_WRITE_BITS(pDrvCam->getPhyObj(),FBC_R1_FBC_LMVO_R1_CTL1,FBC_LMVO_R1_FBC_RESET,0);
        }
        if(CAM_READ_BITS(pDrvCam->getPhyObj(),FBC_R1_FBC_RSSO_R1_CTL1,FBC_RSSO_R1_FBC_EN)){
            CAM_WRITE_BITS(pDrvCam->getPhyObj(),FBC_R1_FBC_RSSO_R1_CTL1,FBC_RSSO_R1_FBC_RESET,0);
        }
        if(CAM_READ_BITS(pDrvCam->getPhyObj(),FBC_R1_FBC_YUVO_R1_CTL1,FBC_YUVO_R1_FBC_EN)) {
            CAM_WRITE_BITS(pDrvCam->getPhyObj(),FBC_R1_FBC_YUVO_R1_CTL1,FBC_YUVO_R1_FBC_RESET,0);
        }
        if(CAM_READ_BITS(pDrvCam->getPhyObj(),FBC_R1_FBC_YUVBO_R1_CTL1,FBC_YUVBO_R1_FBC_EN)) {
            CAM_WRITE_BITS(pDrvCam->getPhyObj(),FBC_R1_FBC_YUVBO_R1_CTL1,FBC_YUVBO_R1_FBC_RESET,0);
        }
        if(CAM_READ_BITS(pDrvCam->getPhyObj(),FBC_R1_FBC_YUVCO_R1_CTL1,FBC_YUVCO_R1_FBC_EN)) {
            CAM_WRITE_BITS(pDrvCam->getPhyObj(),FBC_R1_FBC_YUVCO_R1_CTL1,FBC_YUVCO_R1_FBC_RESET,0);
        }
        if(CAM_READ_BITS(pDrvCam->getPhyObj(),FBC_R1_FBC_CRZO_R1_CTL1,FBC_CRZO_R1_FBC_EN)) {
            CAM_WRITE_BITS(pDrvCam->getPhyObj(),FBC_R1_FBC_CRZO_R1_CTL1,FBC_CRZO_R1_FBC_RESET,0);
        }
        if(CAM_READ_BITS(pDrvCam->getPhyObj(),FBC_R1_FBC_CRZBO_R1_CTL1,FBC_CRZBO_R1_FBC_EN)) {
            CAM_WRITE_BITS(pDrvCam->getPhyObj(),FBC_R1_FBC_CRZBO_R1_CTL1,FBC_CRZBO_R1_FBC_RESET,0);
        }
        if(CAM_READ_BITS(pDrvCam->getPhyObj(),FBC_R1_FBC_CRZO_R2_CTL1,FBC_CRZO_R2_FBC_EN)) {
            CAM_WRITE_BITS(pDrvCam->getPhyObj(),FBC_R1_FBC_CRZO_R2_CTL1,FBC_CRZO_R2_FBC_RESET,0);
        }
        if(CAM_READ_BITS(pDrvCam->getPhyObj(),FBC_R1_FBC_CRZBO_R2_CTL1,FBC_CRZBO_R2_FBC_EN)) {
            CAM_WRITE_BITS(pDrvCam->getPhyObj(),FBC_R1_FBC_CRZBO_R2_CTL1,FBC_CRZBO_R2_FBC_RESET,0);
        }
    }
    if(op & eSus_SIGNAL){
        ISP_WAIT_IRQ_ST irq;

        /* flush irq for dma ports */
        CAM_SFUNC_INF("[%d] flushing signals\n", camModule);

        irq.Clear = ISP_IRQ_CLEAR_WAIT;
        irq.UserKey = 0;
        irq.Timeout = 1000;
        irq.St_type = SIGNAL_INT;
        irq.Status = SW_PASS1_DON_ST;
        pDrvCam->signalIrq(&irq);

        irq.St_type = DMA_INT;

        irq.Status = (AAO_DONE_ST|AFO_DONE_ST|PDO_DONE_ST|FLKO_DONE_ST|TSFSO_DONE_ST);
        pDrvCam->signalIrq(&irq);

        //no need to signal twin's signal,because state will be switch to eCamState_suspending when slave cam's fbc access!
    }
    return MTRUE;
}

MBOOL CAM_BUF_CTRL::resume(ISP_HW_MODULE camModule, ISP_DRV_CAM *pDrvCam, ISP_DRV_CAM *pTgDrvCam, E_RESUME_OP op)
{
    if (pDrvCam == NULL) {
        //slave cam is only need to update status.
        CAM_SFUNC_INF("bypass slave cam\n");
        goto BYPASS_HW_OP;
    }

    if (op & eRes_HW_STATE) {
        CAM_TIMESTAMP* pTime = CAM_TIMESTAMP::getInstance(camModule,(const ISP_DRV_CAM*)pDrvCam);
        pTime->TimeStamp_reset();

        //enable db due to dynamic twin(cam will have time-sharing,slave cam will disable db_en)
        CAM_WRITE_BITS(pDrvCam->getPhyObj(),CAMCTL_R1_CAMCTL_MISC,CAMCTL_DB_EN,1);
    }

    if (op & eRes_HWON) {
        pTgDrvCam->setDeviceInfo(_SET_VF_ON,(MUINT8*)&camModule);
    }

BYPASS_HW_OP:
    if (op & eRes_SW_STATE) {
        CAM_BUF_CTRL::updateState(CAM_BUF_CTRL::eCamState_runnable, camModule, CAM_BUF_CTRL::eCamDmaType_main);
    }

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
    case CAM_C:
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

MBOOL CAM_BUF_CTRL::ion_handle_lock(ISP_DRV_CAM* obj,MBOOL block,MUINT32 img_id,MUINT32 hd_id)
{
    MBOOL rst = MTRUE;
    ISP_DEV_ION_NODE_STRUCT IonNode;

    if(block){
        //ion handle: buffer
        IonNode.devNode = this->m_hwModule;
        IonNode.memID = img_id;
        IonNode.dmaPort = _DMA_Mapping(this->id());
        if(IonNode.dmaPort != _dma_max_wr_)
            obj->setDeviceInfo(_SET_ION_HANDLE,(MUINT8*)&IonNode);
        else
            CAM_FUNC_DBG("img no this port:%d\n",this->id());
        //ion handle: header
        IonNode.memID = hd_id;
        IonNode.dmaPort = _DMA_Header_Mapping(this->id());
        if(IonNode.dmaPort != _dma_max_wr_)
            obj->setDeviceInfo(_SET_ION_HANDLE,(MUINT8*)&IonNode);
        else
            CAM_FUNC_DBG("hd no this port:%d\n",this->id());
    }
    else{
        //ion handle: buffer
        IonNode.devNode = this->m_hwModule;
        IonNode.memID =  img_id;
        IonNode.dmaPort = _DMA_Mapping(this->id());
        if(IonNode.dmaPort != _dma_max_wr_)
            obj->setDeviceInfo(_SET_ION_FREE,(MUINT8*)&IonNode);
        //ion handle: header
        IonNode.memID =  hd_id;
        IonNode.dmaPort = _DMA_Header_Mapping(this->id());
        if(IonNode.dmaPort != _dma_max_wr_)
            obj->setDeviceInfo(_SET_ION_FREE,(MUINT8*)&IonNode);
    }

    return rst;
}

void BUF_CTRL_IMGO::FBC_STATUS(IspDrv* ptr)
{
    CAM_FUNC_DBG("BUF_CTRL_IMGO::FBC_STATUS: 0x%x_0x%x\n",CAM_READ_REG(ptr,FBC_R1_FBC_IMGO_R1_CTL1),CAM_READ_REG(ptr,FBC_R1_FBC_IMGO_R1_CTL2));
}


void BUF_CTRL_IMGO::Header_Parser(MUINTPTR fh_va,NSImageio::NSIspio::BufInfo& buf_info,MUINT32* header_data ,char* pStr, ISP_HW_MODULE masterModule)
{
    Header_IMGO fh_imgo;
    ISP_DRV_CAM* pDrv = NULL;
    MUINT64 tmp;

    this->m_Queue_deque_ptr.sendCmd(QueueMgr<MUINTPTR>::eCmd_front  ,(MUINTPTR)&pDrv,0);

    //
    fh_imgo.Header_Deque(fh_va,(void*)header_data);
    fh_imgo.Header_Flash(fh_va);
    //
    buf_info.u4BufPA[ePlane_1st] = header_data[Header_IMGO::E_IMG_PA];
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

    buf_info.i4TimeStamp_us = header_data[Header_IMGO::E_TimeStamp_LSB];
    tmp = header_data[Header_IMGO::E_TimeStamp_MSB];
    buf_info.i4TimeStamp_us += (tmp << 32);

    this->m_pTimeStamp->TimeStamp_mapping(&buf_info.i4TimeStamp_us,&buf_info.i4TimeStamp_us_B,(_isp_dma_enum_)this->id(), masterModule);

    buf_info.raw_type = header_data[Header_IMGO::E_RAW_TYPE];  //hw define, need to transfer to user-reconized enum in camiopipe
    buf_info.xoffset = 0;
    buf_info.yoffset = 0;

    buf_info.eIQlv = (E_CamIQLevel)(header_data[Header_IMGO::E_IQ_LEVEL] & 0x3); //spare12 is shared with UF_DataFmt and eIQlv.

    buf_info.bUF_DataFmt = (header_data[Header_IMGO::E_IQ_LEVEL] & 0x4) ? MTRUE : MFALSE; //spare12 is shared with UF_DataFmt and eIQlv.
    //
    if(pStr){
        sprintf(pStr, "enq_sof:0x%x,mag:0x%x,timestamp:%" PRIu64 ",RawType:0x%x,eIQlv:0x%x,isUF:0x%x",\
            header_data[Header_IMGO::E_ENQUE_SOF],\
            header_data[Header_IMGO::E_Magic], \
            buf_info.i4TimeStamp_us,\
            header_data[Header_IMGO::E_RAW_TYPE], \
            buf_info.eIQlv,\
            buf_info.bUF_DataFmt);
    }
}



void BUF_CTRL_RRZO::FBC_STATUS(IspDrv* ptr)
{
    CAM_FUNC_DBG("BUF_CTRL_RRZO::FBC_STATUS: 0x%x_0x%x\n",CAM_READ_REG(ptr,FBC_R1_FBC_RRZO_R1_CTL1),CAM_READ_REG(ptr,FBC_R1_FBC_RRZO_R1_CTL2));
}

void BUF_CTRL_RRZO::Header_Parser(MUINTPTR fh_va,NSImageio::NSIspio::BufInfo& buf_info,MUINT32* header_data ,char* pStr, ISP_HW_MODULE masterModule)
{
    Header_RRZO fh_rrzo;
    ISP_DRV_CAM* pDrv = NULL;
    MUINT64 tmp;

    this->m_Queue_deque_ptr.sendCmd(QueueMgr<MUINTPTR>::eCmd_front  ,(MUINTPTR)&pDrv,0);

    //
    fh_rrzo.Header_Deque(fh_va,(void*)header_data);
    fh_rrzo.Header_Flash(fh_va);

    //
    buf_info.u4BufPA[ePlane_1st] = header_data[Header_RRZO::E_IMG_PA];
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

    buf_info.i4TimeStamp_us = header_data[Header_RRZO::E_TimeStamp_LSB];
    tmp = header_data[Header_RRZO::E_TimeStamp_MSB];
    buf_info.i4TimeStamp_us += (tmp << 32);

    this->m_pTimeStamp->TimeStamp_mapping(&buf_info.i4TimeStamp_us,&buf_info.i4TimeStamp_us_B,(_isp_dma_enum_)this->id(), masterModule);

    buf_info.raw_type = 0;
    buf_info.xoffset = 0;
    buf_info.yoffset = 0;

    buf_info.eIQlv = (E_CamIQLevel)header_data[Header_RRZO::E_IQ_LEVEL];

    buf_info.bUF_DataFmt = header_data[Header_RRZO::E_IS_UFG_FMT];

    //
    if(pStr){
        sprintf(pStr, "enq_sof:0x%x,mag:0x%x,timestamp:%" PRIu64 ",eIQlv:0x%x,isUF:0x%x",\
            header_data[Header_RRZO::E_ENQUE_SOF],\
            header_data[Header_RRZO::E_Magic], \
            buf_info.i4TimeStamp_us, \
            buf_info.eIQlv,\
            buf_info.bUF_DataFmt);
    }
}


E_BC_STATUS BUF_CTRL_UFEO::dequeueHwBuf(NSImageio::NSIspio::BufInfo& buf_info )
{
    E_BC_STATUS ret = eCmd_Pass;
    E_ISP_CAM_CQ cq = ISP_DRV_CQ_NONE;
    MUINT32 page = 0;
    MUINT32 _size;
    MUINT32 _sof_idx = this->m_hwModule; // assign hwModule as input argument
    ST_BUF_INFO _buf;
    MUINT32 _header_data[E_HEADER_MAX];
    char str[128] = {'\0'};
    ISP_DRV_CAM* pDrv = NULL;
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

    this->m_Queue.sendCmd(QueueMgr<ST_BUF_INFO>::eCmd_getsize,(MUINTPTR)&_size,0);
    if(_size <1){
        CAM_FUNC_ERR("queue depth is empty for deque\n");
        ret = eCmd_Fail;
        goto EXIT;
    }

    this->m_Queue_deque_ptr.sendCmd(QueueMgr<MUINTPTR>::eCmd_front  ,(MUINTPTR)&pDrv,0);

    //dbg log
    pDrv->getCurObjInfo(&this->m_hwModule,&cq,&page);

    //fbc reg dbg log
    this->FBC_STATUS(pDrv->getPhyObj());

    //
    this->m_Queue.sendCmd(QueueMgr<ST_BUF_INFO>::eCmd_front  ,(MUINTPTR)&_buf,0);

    //
    pDrv->getDeviceInfo(_GET_SOF_CNT,(MUINT8 *) &_sof_idx);

    //
    switch(CAM_BUF_CTRL::m_CamState[this->m_hwModule]){
        case CAM_BUF_CTRL::eCamState_runnable:
        case CAM_BUF_CTRL::eCamState_suspend:   //this timing is for twin deque is happaned after dynamic twin suspend ready
            //
            this->Header_Parser(_buf.header.va_addr, buf_info,_header_data,str, pDrv->m_HWmasterModule);
                        //
            if( _buf.image.mem_info.pa_addr != buf_info.u4BufPA[ePlane_2nd]){
                CAM_FUNC_ERR("%s:PA in header is mismatch with PA in list [0x%" PRIXPTR "_0x%" PRIXPTR "]\n",this->name_Str(),\
                    buf_info.u4BufPA[ePlane_2nd],_buf.image.mem_info.pa_addr);

                ret = eCmd_Fail;
                this->PipeCheck();
                goto EXIT;
            }
            break;
        case CAM_BUF_CTRL::eCamState_suspending:
            ret = eCmd_Suspending_Pass;

            //do not check header,dram on header's address is garbage!
            //need to pop all requests in driver when suspending for the drop method in iopipe.

            //Flush FH
            this->Header_Parser(_buf.header.va_addr, buf_info,_header_data,str, pDrv->m_HWmasterModule);

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
    buf_data_parsing(buf_info,ePlane_2nd,_buf);
    //
    this->m_Queue.sendCmd(QueueMgr<ST_BUF_INFO>::eCmd_pop  ,0,0);
    this->m_Queue_deque_ptr.sendCmd(QueueMgr<MUINTPTR>::eCmd_pop  ,0,0);
    this->m_buf_cnt--;

    //ion handle: buffer
    this->ion_handle_lock(pDrv,MFALSE,_buf.image.mem_info.memID,_buf.header.memID);

EXIT:

    CAM_FUNC_DBG("%s:(0x%x_0x%x),PA(0x%" PRIXPTR "_0x%" PRIXPTR "),FH_VA(0x%" PRIXPTR "),size(0x%x),cur sof(%d),%s\n",\
        this->name_Str(),\
        cq,page,\
        _buf.image.mem_info.pa_addr ,\
        _buf.header.pa_addr ,\
        _buf.header.va_addr ,\
        _buf.image.mem_info.size ,\
        _sof_idx,\
        str);

    return ret;
}


void BUF_CTRL_UFEO::FBC_STATUS(IspDrv* ptr)
{
    CAM_FUNC_DBG("BUF_CTRL_UFEO::FBC_STATUS: 0x%x_0x%x\n",CAM_READ_REG(ptr,FBC_R1_FBC_UFEO_R1_CTL1),CAM_READ_REG(ptr,FBC_R1_FBC_UFEO_R1_CTL2));
}

void BUF_CTRL_UFEO::Header_Parser(MUINTPTR fh_va,NSImageio::NSIspio::BufInfo& buf_info,MUINT32* header_data ,char* pStr, ISP_HW_MODULE masterModule)
{
    Header_UFEO fh_ufeo;
    MUINT64 tmp;

    REG_TG_R1_TG_SEN_GRAB_PXL TG_W;
    REG_TG_R1_TG_SEN_GRAB_LIN TG_H;
    //
    fh_ufeo.Header_Deque(fh_va,(void*)header_data);
    fh_ufeo.Header_Flash(fh_va);
    //
    buf_info.u4BufPA[ePlane_2nd] = header_data[Header_UFEO::E_IMG_PA];
    buf_info.m_num = header_data[Header_UFEO::E_Magic];
    buf_info.img_fmt = header_data[Header_UFEO::E_FMT];        //hw define, need to transfer to user-reconized enum in camiopipe
    //buf_info.img_pxl_id = header_data[Header_UFEO::E_PIX_ID];  //hw define, need to transfer to user-reconized enum in camiopipe
    buf_info.frm_cnt = 0;

    TG_W.Raw = CAM_READ_REG(this->m_pDrv->getPhyObj(),TG_R1_TG_SEN_GRAB_PXL);
    TG_H.Raw = CAM_READ_REG(this->m_pDrv->getPhyObj(),TG_R1_TG_SEN_GRAB_LIN);
    buf_info.img_w = TG_W.Bits.TG_PXL_E - TG_W.Bits.TG_PXL_S;
    buf_info.img_h = TG_H.Bits.TG_LIN_E - TG_H.Bits.TG_LIN_S;

    buf_info.i4TimeStamp_us = header_data[Header_UFEO::E_TimeStamp_LSB];
    tmp = header_data[Header_UFEO::E_TimeStamp_MSB];
    buf_info.i4TimeStamp_us += (tmp << 32);

    this->m_pTimeStamp->TimeStamp_mapping(&buf_info.i4TimeStamp_us,&buf_info.i4TimeStamp_us_B,(_isp_dma_enum_)this->id(), masterModule);

    buf_info.raw_type = 0;
    buf_info.xoffset = 0;
    buf_info.yoffset = 0;

    buf_info.eIQlv = (E_CamIQLevel)header_data[Header_UFEO::E_IQ_LEVEL];

    if(buf_info.m_pPrivate != NULL){
        ((UFDG_META_INFO*)buf_info.m_pPrivate)->bUF = MTRUE;
        ((UFDG_META_INFO*)buf_info.m_pPrivate)->UFDG_BITSTREAM_OFST_ADDR = header_data[Header_UFEO::E_IMGO_OFFST_ADDR];
        ((UFDG_META_INFO*)buf_info.m_pPrivate)->UFDG_BS_AU_START = header_data[Header_UFEO::E_UFEO_OFFST_ADDR];
        ((UFDG_META_INFO*)buf_info.m_pPrivate)->UFDG_AU2_SIZE = header_data[Header_UFEO::E_UFEO_XSIZE];
    } else {
        CAM_FUNC_WRN("Deque UFEG info fail due to m_pPrivate is NULL");
    }

    //
    if(pStr){
        sprintf(pStr, "enq_sof:0x%x,mag:0x%x,timestamp:%" PRIu64 ",eIQlv:0x%x",\
            header_data[Header_UFEO::E_ENQUE_SOF],\
            header_data[Header_UFEO::E_Magic], \
            buf_info.i4TimeStamp_us, \
            buf_info.eIQlv);
    }
}


E_BC_STATUS BUF_CTRL_UFGO::dequeueHwBuf(NSImageio::NSIspio::BufInfo& buf_info )
{
    E_BC_STATUS ret = eCmd_Pass;
    E_ISP_CAM_CQ cq = ISP_DRV_CQ_NONE;
    MUINT32 page = 0;
    MUINT32 _size;
    MUINT32 _sof_idx = this->m_hwModule; // assign hwModule as input argument
    ST_BUF_INFO _buf;
    MUINT32 _header_data[E_HEADER_MAX];
    char str[128] = {'\0'};
    ISP_DRV_CAM* pDrv = NULL;
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

    this->m_Queue.sendCmd(QueueMgr<ST_BUF_INFO>::eCmd_getsize,(MUINTPTR)&_size,0);
    if(_size <1){
        CAM_FUNC_ERR("queue depth is empty for deque\n");
        ret = eCmd_Fail;
        goto EXIT;
    }

    this->m_Queue_deque_ptr.sendCmd(QueueMgr<MUINTPTR>::eCmd_front  ,(MUINTPTR)&pDrv,0);

    //dbg log
    pDrv->getCurObjInfo(&this->m_hwModule,&cq,&page);

    //fbc reg dbg log
    this->FBC_STATUS(pDrv->getPhyObj());

    //
    this->m_Queue.sendCmd(QueueMgr<ST_BUF_INFO>::eCmd_front  ,(MUINTPTR)&_buf,0);

    //
    pDrv->getDeviceInfo(_GET_SOF_CNT,(MUINT8 *) &_sof_idx);

    //
    switch(CAM_BUF_CTRL::m_CamState[this->m_hwModule]){
        case CAM_BUF_CTRL::eCamState_runnable:
        case CAM_BUF_CTRL::eCamState_suspend:   //this timing is for twin deque is happaned after dynamic twin suspend ready
            //
            this->Header_Parser(_buf.header.va_addr, buf_info,_header_data,str, pDrv->m_HWmasterModule);
                        //
            if( _buf.image.mem_info.pa_addr != buf_info.u4BufPA[ePlane_2nd]){
                CAM_FUNC_ERR("%s:PA in header is mismatch with PA in list [0x%" PRIXPTR "_0x%" PRIXPTR "]\n",this->name_Str(),\
                    buf_info.u4BufPA[ePlane_2nd],_buf.image.mem_info.pa_addr);

                ret = eCmd_Fail;
                this->PipeCheck();
                goto EXIT;
            }
            break;
        case CAM_BUF_CTRL::eCamState_suspending:
            ret = eCmd_Suspending_Pass;

            //do not check header,dram on header's address is garbage!
            //need to pop all requests in driver when suspending for the drop method in iopipe.

            //Flush FH
            this->Header_Parser(_buf.header.va_addr, buf_info,_header_data,str, pDrv->m_HWmasterModule);

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
    buf_data_parsing(buf_info,ePlane_2nd,_buf);

    //
    this->m_Queue.sendCmd(QueueMgr<ST_BUF_INFO>::eCmd_pop  ,0,0);
    this->m_Queue_deque_ptr.sendCmd(QueueMgr<MUINTPTR>::eCmd_pop  ,0,0);
    this->m_buf_cnt--;

    //ion handle: buffer
    this->ion_handle_lock(pDrv,MFALSE,_buf.image.mem_info.memID,_buf.header.memID);

EXIT:

    CAM_FUNC_DBG("%s:(0x%x_0x%x),PA(0x%" PRIXPTR "_0x%" PRIXPTR "),FH_VA(0x%" PRIXPTR "),size(0x%x),cur sof(%d),%s\n",\
        this->name_Str(),\
        cq,page,\
        _buf.image.mem_info.pa_addr ,\
        _buf.header.pa_addr ,\
        _buf.header.va_addr ,\
        _buf.image.mem_info.size ,\
        _sof_idx,\
        str);

    return ret;
}


void BUF_CTRL_UFGO::FBC_STATUS(IspDrv* ptr)
{
    CAM_FUNC_DBG("BUF_CTRL_UFGO::FBC_STATUS: 0x%x_0x%x\n",CAM_READ_REG(ptr,FBC_R1_FBC_UFGO_R1_CTL1),CAM_READ_REG(ptr,FBC_R1_FBC_UFGO_R1_CTL2));
}

void BUF_CTRL_UFGO::Header_Parser(MUINTPTR fh_va,NSImageio::NSIspio::BufInfo& buf_info,MUINT32* header_data ,char* pStr, ISP_HW_MODULE masterModule)
{
    Header_UFGO fh_ufgo;
    MUINT64 tmp;
    REG_TG_R1_TG_SEN_GRAB_PXL TG_W;
    REG_TG_R1_TG_SEN_GRAB_LIN TG_H;
    //
    fh_ufgo.Header_Deque(fh_va,(void*)header_data);
    fh_ufgo.Header_Flash(fh_va);
    //
    buf_info.u4BufPA[ePlane_2nd] = header_data[Header_UFGO::E_IMG_PA];
    buf_info.m_num = header_data[Header_UFGO::E_Magic];
    buf_info.img_fmt = header_data[Header_UFGO::E_FMT];        //hw define, need to transfer to user-reconized enum in camiopipe
    //buf_info.img_pxl_id = header_data[Header_UFGO::E_PIX_ID];  //hw define, need to transfer to user-reconized enum in camiopipe
    buf_info.frm_cnt = 0;

    TG_W.Raw = CAM_READ_REG(this->m_pDrv->getPhyObj(),TG_R1_TG_SEN_GRAB_PXL);
    TG_H.Raw = CAM_READ_REG(this->m_pDrv->getPhyObj(),TG_R1_TG_SEN_GRAB_LIN);
    buf_info.img_w = TG_W.Bits.TG_PXL_E - TG_W.Bits.TG_PXL_S;
    buf_info.img_h = TG_H.Bits.TG_LIN_E - TG_H.Bits.TG_LIN_S;

    buf_info.i4TimeStamp_us = header_data[Header_UFGO::E_TimeStamp_LSB];
    tmp = header_data[Header_UFGO::E_TimeStamp_MSB];
    buf_info.i4TimeStamp_us += (tmp << 32);

    this->m_pTimeStamp->TimeStamp_mapping(&buf_info.i4TimeStamp_us,&buf_info.i4TimeStamp_us_B,(_isp_dma_enum_)this->id(), masterModule);

    buf_info.raw_type = 0;
    buf_info.xoffset = 0;
    buf_info.yoffset = 0;

    buf_info.eIQlv = (E_CamIQLevel)header_data[Header_UFGO::E_IQ_LEVEL];

    if(buf_info.m_pPrivate != NULL){
        ((UFDG_META_INFO*)buf_info.m_pPrivate)->bUF = MTRUE;
        ((UFDG_META_INFO*)buf_info.m_pPrivate)->UFDG_BITSTREAM_OFST_ADDR = header_data[Header_UFGO::E_RRZO_OFFST_ADDR];
        ((UFDG_META_INFO*)buf_info.m_pPrivate)->UFDG_BS_AU_START = header_data[Header_UFGO::E_UFGO_OFFST_ADDR];
        ((UFDG_META_INFO*)buf_info.m_pPrivate)->UFDG_AU2_SIZE = header_data[Header_UFGO::E_UFGO_XSIZE];
    } else {
        CAM_FUNC_WRN("Deque UFEG info fail due to m_pPrivate is NULL");
    }

    //
    if(pStr){
        sprintf(pStr, "enq_sof:0x%x,mag:0x%x,timestamp:%" PRIu64 ",eIQlv:0x%x",\
            header_data[Header_UFGO::E_ENQUE_SOF],\
            header_data[Header_UFGO::E_Magic], \
            buf_info.i4TimeStamp_us, \
            buf_info.eIQlv);
    }
}

void BUF_CTRL_YUVO::FBC_STATUS(IspDrv* ptr)
{
    CAM_FUNC_DBG("BUF_CTRL_YUVO::FBC_STATUS: 0x%x_0x%x\n",CAM_READ_REG(ptr,FBC_R1_FBC_YUVO_R1_CTL1),CAM_READ_REG(ptr,FBC_R1_FBC_YUVO_R1_CTL2));
}

void BUF_CTRL_YUVO::Header_Parser(MUINTPTR fh_va,NSImageio::NSIspio::BufInfo& buf_info,MUINT32* header_data ,char* pStr, ISP_HW_MODULE masterModule)
{
    Header_YUVO fh_yuvo;
    ISP_DRV_CAM* pDrv = NULL;
    MUINT64 tmp;

    this->m_Queue_deque_ptr.sendCmd(QueueMgr<MUINTPTR>::eCmd_front,(MUINTPTR)&pDrv,0);

    //
    fh_yuvo.Header_Deque(fh_va,(void*)header_data);
    fh_yuvo.Header_Flash(fh_va);
    //
    buf_info.u4BufPA[ePlane_1st] = header_data[Header_YUVO::E_IMG_PA];
    buf_info.m_num = header_data[Header_YUVO::E_Magic];
    buf_info.img_fmt = header_data[Header_YUVO::E_FMT];         //hw define, need to transfer to user-reconized enum in camiopipe
    buf_info.sw_fmt = header_data[Header_YUVO::E_SW_FMT];
    buf_info.frm_cnt = 0;

    buf_info.img_w = (header_data[Header_YUVO::E_SRC_SIZE] & 0xffff);
    buf_info.img_h = (header_data[Header_YUVO::E_SRC_SIZE] >> 16);

    buf_info.crop_win.p.x = (header_data[Header_YUVO::E_CROP_START] & 0xffff);
    buf_info.crop_win.p.y = (header_data[Header_YUVO::E_CROP_START] >> 16);
    buf_info.crop_win.s.w = (header_data[Header_YUVO::E_CROP_SIZE] & 0xffff);
    buf_info.crop_win.s.h = (header_data[Header_YUVO::E_CROP_SIZE] >> 16);

    buf_info.DstSize.w = buf_info.crop_win.s.w;
    buf_info.DstSize.h = buf_info.crop_win.s.h;

    buf_info.i4TimeStamp_us = header_data[Header_YUVO::E_TimeStamp_LSB];
    tmp = header_data[Header_YUVO::E_TimeStamp_MSB];
    buf_info.i4TimeStamp_us += (tmp << 32);

    this->m_pTimeStamp->TimeStamp_mapping(&buf_info.i4TimeStamp_us,&buf_info.i4TimeStamp_us_B,(_isp_dma_enum_)this->id(),masterModule);

    buf_info.raw_type = 0;
    buf_info.xoffset = 0;
    buf_info.yoffset = 0;

    buf_info.eIQlv = (E_CamIQLevel)(header_data[Header_YUVO::E_IQ_LEVEL]);
    //
    if(pStr){
        sprintf(pStr, "enq_sof:0x%x,mag:0x%x,timestamp:%" PRIu64 ",eIQlv:0x%x,enYUVPlane:0x%x",\
            header_data[Header_YUVO::E_ENQUE_SOF],\
            header_data[Header_YUVO::E_Magic], \
            buf_info.i4TimeStamp_us,\
            buf_info.eIQlv,\
            header_data[Header_YUVO::E_PLANE_EN]);
    }
}

E_BC_STATUS BUF_CTRL_YUVBO::dequeueHwBuf(NSImageio::NSIspio::BufInfo& buf_info)
{
    E_BC_STATUS ret = eCmd_Pass;
    E_ISP_CAM_CQ cq = ISP_DRV_CQ_NONE;
    MUINT32 page = 0;
    MUINT32 _size;
    MUINT32 _sof_idx = this->m_hwModule; // assign hwModule as input argument
    ST_BUF_INFO _buf;
    MUINT32 _header_data[E_HEADER_MAX];
    char str[128] = {'\0'};
    ISP_DRV_CAM* pDrv = NULL;
    Mutex::Autolock lock(this->m_bufctrl_lock);

    if(0xffffffff == this->id()){
        CAM_FUNC_ERR("dma channel error ");
        ret = eCmd_Fail;
        goto EXIT;
    }

    this->m_Queue_deque_ptr.sendCmd(QueueMgr<MUINTPTR>::eCmd_getsize,(MUINTPTR)&_size,0);
    if(_size == 0){
        CAM_FUNC_ERR("no enque record, plz enque first");
        return eCmd_Fail;
    }

    this->m_Queue.sendCmd(QueueMgr<ST_BUF_INFO>::eCmd_getsize,(MUINTPTR)&_size,0);
    if(_size < 1){
        CAM_FUNC_ERR("queue depth is empty for deque\n");
        ret = eCmd_Fail;
        goto EXIT;
    }

    this->m_Queue_deque_ptr.sendCmd(QueueMgr<MUINTPTR>::eCmd_front,(MUINTPTR)&pDrv,0);

    //dbg log
    pDrv->getCurObjInfo(&this->m_hwModule,&cq,&page);

    //fbc reg dbg log
    this->FBC_STATUS(pDrv->getPhyObj());

    //
    this->m_Queue.sendCmd(QueueMgr<ST_BUF_INFO>::eCmd_front,(MUINTPTR)&_buf,0);

    //
    pDrv->getDeviceInfo(_GET_SOF_CNT,(MUINT8 *)&_sof_idx);

    //
    switch(CAM_BUF_CTRL::m_CamState[this->m_hwModule]){
        case CAM_BUF_CTRL::eCamState_runnable:
        case CAM_BUF_CTRL::eCamState_suspend:   //this timing is for twin deque is happaned after dynamic twin suspend ready
            //
            this->Header_Parser(_buf.header.va_addr,buf_info,_header_data,str,pDrv->m_HWmasterModule);
            //
            if(_buf.image.mem_info.pa_addr != buf_info.u4BufPA[ePlane_2nd]){
                CAM_FUNC_ERR("%s:PA in header is mismatch with PA in list [0x%" PRIXPTR "_0x%" PRIXPTR "]\n",this->name_Str(),\
                    buf_info.u4BufPA[ePlane_2nd],_buf.image.mem_info.pa_addr);

                ret = eCmd_Fail;
                this->PipeCheck();
                goto EXIT;
            }
            break;
        case CAM_BUF_CTRL::eCamState_suspending:
            ret = eCmd_Suspending_Pass;

            //do not check header,dram on header's address is garbage!
            //need to pop all requests in driver when suspending for the drop method in iopipe.

            //Flush FH
            this->Header_Parser(_buf.header.va_addr,buf_info,_header_data,str,pDrv->m_HWmasterModule);

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
    buf_data_parsing(buf_info,ePlane_2nd,_buf);

    //
    this->m_Queue.sendCmd(QueueMgr<ST_BUF_INFO>::eCmd_pop,0,0);
    this->m_Queue_deque_ptr.sendCmd(QueueMgr<MUINTPTR>::eCmd_pop,0,0);
    this->m_buf_cnt--;

    //ion handle: buffer
    this->ion_handle_lock(pDrv,MFALSE,_buf.image.mem_info.memID,_buf.header.memID);


EXIT:
    CAM_FUNC_INF("%s:(0x%x_0x%x),PA(0x%" PRIXPTR "_0x%" PRIXPTR "),FH_VA(0x%" PRIXPTR "),size(0x%x),cur sof(%d),%s\n",\
        this->name_Str(),\
        cq,page,\
        _buf.image.mem_info.pa_addr ,\
        _buf.header.pa_addr ,\
        _buf.header.va_addr ,\
        _buf.image.mem_info.size ,\
        _sof_idx,\
        str);

    return ret;
}

void BUF_CTRL_YUVBO::FBC_STATUS(IspDrv* ptr)
{
    CAM_FUNC_DBG("BUF_CTRL_YUVBO::FBC_STATUS: 0x%x_0x%x\n",CAM_READ_REG(ptr,FBC_R1_FBC_YUVBO_R1_CTL1),CAM_READ_REG(ptr,FBC_R1_FBC_YUVBO_R1_CTL2));
}

void BUF_CTRL_YUVBO::Header_Parser(MUINTPTR fh_va,NSImageio::NSIspio::BufInfo& buf_info,MUINT32* header_data ,char* pStr, ISP_HW_MODULE masterModule)
{
    Header_YUVBO fh_yuvbo;
    ISP_DRV_CAM* pDrv = NULL;
    MUINT64 tmp;

    this->m_Queue_deque_ptr.sendCmd(QueueMgr<MUINTPTR>::eCmd_front,(MUINTPTR)&pDrv,0);

    //
    fh_yuvbo.Header_Deque(fh_va,(void*)header_data);
    fh_yuvbo.Header_Flash(fh_va);
    //
    buf_info.u4BufPA[ePlane_2nd] = header_data[Header_YUVBO::E_IMG_PA];
    buf_info.m_num = header_data[Header_YUVBO::E_Magic];
    buf_info.img_fmt = header_data[Header_YUVBO::E_FMT];        //hw define, need to transfer to user-reconized enum in camiopipe
    buf_info.img_pxl_id = header_data[Header_YUVBO::E_PIX_ID];  //hw define, need to transfer to user-reconized enum in camiopipe
    buf_info.frm_cnt = 0;

    buf_info.img_w = (header_data[Header_YUVBO::E_SRC_SIZE] & 0xffff);
    buf_info.img_h = (header_data[Header_YUVBO::E_SRC_SIZE] >> 16);

    buf_info.crop_win.p.x = (header_data[Header_YUVBO::E_CROP_START] & 0xffff);
    buf_info.crop_win.p.y = (header_data[Header_YUVBO::E_CROP_START] >> 16);
    buf_info.crop_win.s.w = (header_data[Header_YUVBO::E_CROP_SIZE] & 0xffff);
    buf_info.crop_win.s.h = (header_data[Header_YUVBO::E_CROP_SIZE] >> 16);

    buf_info.DstSize.w = buf_info.crop_win.s.w;
    buf_info.DstSize.h = buf_info.crop_win.s.h;

    buf_info.i4TimeStamp_us = header_data[Header_YUVBO::E_TimeStamp_LSB];
    tmp = header_data[Header_YUVBO::E_TimeStamp_MSB];
    buf_info.i4TimeStamp_us += (tmp << 32);

    this->m_pTimeStamp->TimeStamp_mapping(&buf_info.i4TimeStamp_us,&buf_info.i4TimeStamp_us_B,(_isp_dma_enum_)this->id(),masterModule);

    buf_info.raw_type = 0;
    buf_info.xoffset = 0;
    buf_info.yoffset = 0;

    buf_info.eIQlv = (E_CamIQLevel)(header_data[Header_YUVBO::E_IQ_LEVEL]);
    //
    if(pStr){
        sprintf(pStr, "enq_sof:0x%x,mag:0x%x,timestamp:%" PRIu64 ",eIQlv:0x%x",\
            header_data[Header_YUVBO::E_ENQUE_SOF],\
            header_data[Header_YUVBO::E_Magic], \
            buf_info.i4TimeStamp_us,\
            buf_info.eIQlv);
    }
}

E_BC_STATUS BUF_CTRL_YUVCO::dequeueHwBuf(NSImageio::NSIspio::BufInfo& buf_info)
{
    E_BC_STATUS ret = eCmd_Pass;
    E_ISP_CAM_CQ cq = ISP_DRV_CQ_NONE;
    MUINT32 page = 0;
    MUINT32 _size;
    MUINT32 _sof_idx = this->m_hwModule; // assign hwModule as input argument
    ST_BUF_INFO _buf;
    MUINT32 _header_data[E_HEADER_MAX];
    char str[128] = {'\0'};
    ISP_DRV_CAM* pDrv = NULL;
    Mutex::Autolock lock(this->m_bufctrl_lock);

    if(0xffffffff == this->id()){
        CAM_FUNC_ERR("dma channel error ");
        ret = eCmd_Fail;
        goto EXIT;
    }

    this->m_Queue_deque_ptr.sendCmd(QueueMgr<MUINTPTR>::eCmd_getsize,(MUINTPTR)&_size,0);
    if(_size == 0){
        CAM_FUNC_ERR("no enque record, plz enque first");
        return eCmd_Fail;
    }

    this->m_Queue.sendCmd(QueueMgr<ST_BUF_INFO>::eCmd_getsize,(MUINTPTR)&_size,0);
    if(_size < 1){
        CAM_FUNC_ERR("queue depth is empty for deque\n");
        ret = eCmd_Fail;
        goto EXIT;
    }

    this->m_Queue_deque_ptr.sendCmd(QueueMgr<MUINTPTR>::eCmd_front,(MUINTPTR)&pDrv,0);

    //dbg log
    pDrv->getCurObjInfo(&this->m_hwModule,&cq,&page);

    //fbc reg dbg log
    this->FBC_STATUS(pDrv->getPhyObj());

    //
    this->m_Queue.sendCmd(QueueMgr<ST_BUF_INFO>::eCmd_front,(MUINTPTR)&_buf,0);

    //
    pDrv->getDeviceInfo(_GET_SOF_CNT,(MUINT8 *)&_sof_idx);

    //
    switch(CAM_BUF_CTRL::m_CamState[this->m_hwModule]){
        case CAM_BUF_CTRL::eCamState_runnable:
        case CAM_BUF_CTRL::eCamState_suspend:   //this timing is for twin deque is happaned after dynamic twin suspend ready
            //
            this->Header_Parser(_buf.header.va_addr,buf_info,_header_data,str,pDrv->m_HWmasterModule);
            //
            if(_buf.image.mem_info.pa_addr != buf_info.u4BufPA[ePlane_3rd]){
                CAM_FUNC_ERR("%s:PA in header is mismatch with PA in list [0x%" PRIXPTR "_0x%" PRIXPTR "]\n",this->name_Str(),\
                    buf_info.u4BufPA[ePlane_3rd],_buf.image.mem_info.pa_addr);

                ret = eCmd_Fail;
                this->PipeCheck();
                goto EXIT;
            }
            break;
        case CAM_BUF_CTRL::eCamState_suspending:
            ret = eCmd_Suspending_Pass;

            //do not check header,dram on header's address is garbage!
            //need to pop all requests in driver when suspending for the drop method in iopipe.

            //Flush FH
            this->Header_Parser(_buf.header.va_addr,buf_info,_header_data,str,pDrv->m_HWmasterModule);

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
    buf_data_parsing(buf_info,ePlane_3rd,_buf);

    //
    this->m_Queue.sendCmd(QueueMgr<ST_BUF_INFO>::eCmd_pop,0,0);
    this->m_Queue_deque_ptr.sendCmd(QueueMgr<MUINTPTR>::eCmd_pop,0,0);
    this->m_buf_cnt--;

    //ion handle: buffer
    this->ion_handle_lock(pDrv,MFALSE,_buf.image.mem_info.memID,_buf.header.memID);


EXIT:
    CAM_FUNC_INF("%s:(0x%x_0x%x),PA(0x%" PRIXPTR "_0x%" PRIXPTR "),FH_VA(0x%" PRIXPTR "),size(0x%x),cur sof(%d),%s\n",\
        this->name_Str(),\
        cq,page,\
        _buf.image.mem_info.pa_addr ,\
        _buf.header.pa_addr ,\
        _buf.header.va_addr ,\
        _buf.image.mem_info.size ,\
        _sof_idx,\
        str);

    return ret;
}

void BUF_CTRL_YUVCO::FBC_STATUS(IspDrv* ptr)
{
    CAM_FUNC_DBG("BUF_CTRL_YUVCO::FBC_STATUS: 0x%x_0x%x\n",CAM_READ_REG(ptr,FBC_R1_FBC_YUVCO_R1_CTL1),CAM_READ_REG(ptr,FBC_R1_FBC_YUVCO_R1_CTL2));
}

void BUF_CTRL_YUVCO::Header_Parser(MUINTPTR fh_va,NSImageio::NSIspio::BufInfo& buf_info,MUINT32* header_data ,char* pStr, ISP_HW_MODULE masterModule)
{
    Header_YUVCO fh_yuvco;
    ISP_DRV_CAM* pDrv = NULL;
    MUINT64 tmp;

    this->m_Queue_deque_ptr.sendCmd(QueueMgr<MUINTPTR>::eCmd_front,(MUINTPTR)&pDrv,0);

    //
    fh_yuvco.Header_Deque(fh_va,(void*)header_data);
    fh_yuvco.Header_Flash(fh_va);
    //
    buf_info.u4BufPA[ePlane_3rd] = header_data[Header_YUVCO::E_IMG_PA];
    buf_info.m_num = header_data[Header_YUVCO::E_Magic];
    buf_info.img_fmt = header_data[Header_YUVCO::E_FMT];        //hw define, need to transfer to user-reconized enum in camiopipe
    buf_info.img_pxl_id = header_data[Header_YUVCO::E_PIX_ID];  //hw define, need to transfer to user-reconized enum in camiopipe
    buf_info.frm_cnt = 0;

    buf_info.img_w = (header_data[Header_YUVCO::E_SRC_SIZE] & 0xffff);
    buf_info.img_h = (header_data[Header_YUVCO::E_SRC_SIZE] >> 16);

    buf_info.crop_win.p.x = (header_data[Header_YUVCO::E_CROP_START] & 0xffff);
    buf_info.crop_win.p.y = (header_data[Header_YUVCO::E_CROP_START] >> 16);
    buf_info.crop_win.s.w = (header_data[Header_YUVCO::E_CROP_SIZE] & 0xffff);
    buf_info.crop_win.s.h = (header_data[Header_YUVCO::E_CROP_SIZE] >> 16);

    buf_info.DstSize.w = buf_info.crop_win.s.w;
    buf_info.DstSize.h = buf_info.crop_win.s.h;

    buf_info.i4TimeStamp_us = header_data[Header_YUVCO::E_TimeStamp_LSB];
    tmp = header_data[Header_YUVCO::E_TimeStamp_MSB];
    buf_info.i4TimeStamp_us += (tmp << 32);

    this->m_pTimeStamp->TimeStamp_mapping(&buf_info.i4TimeStamp_us,&buf_info.i4TimeStamp_us_B,(_isp_dma_enum_)this->id(),masterModule);

    buf_info.raw_type = 0;
    buf_info.xoffset = 0;
    buf_info.yoffset = 0;

    buf_info.eIQlv = (E_CamIQLevel)(header_data[Header_YUVCO::E_IQ_LEVEL]);
    //
    if(pStr){
        sprintf(pStr, "enq_sof:0x%x,mag:0x%x,timestamp:%" PRIu64 ",eIQlv:0x%x",\
            header_data[Header_YUVCO::E_ENQUE_SOF],\
            header_data[Header_YUVCO::E_Magic], \
            buf_info.i4TimeStamp_us,\
            buf_info.eIQlv);
    }
}

void BUF_CTRL_CRZO_R1::FBC_STATUS(IspDrv* ptr)
{
    CAM_FUNC_DBG("BUF_CTRL_CRZO_R1::FBC_STATUS: 0x%x_0x%x\n",CAM_READ_REG(ptr,FBC_R1_FBC_CRZO_R1_CTL1),CAM_READ_REG(ptr,FBC_R1_FBC_CRZO_R1_CTL2));
}

void BUF_CTRL_CRZO_R1::Header_Parser(MUINTPTR fh_va,NSImageio::NSIspio::BufInfo& buf_info,MUINT32* header_data ,char* pStr, ISP_HW_MODULE masterModule)
{
    Header_CRZO_R1 fh_crzo_r1;
    ISP_DRV_CAM* pDrv = NULL;
    MUINT64 tmp;

    this->m_Queue_deque_ptr.sendCmd(QueueMgr<MUINTPTR>::eCmd_front,(MUINTPTR)&pDrv,0);

    //
    fh_crzo_r1.Header_Deque(fh_va,(void*)header_data);
    fh_crzo_r1.Header_Flash(fh_va);

    //
    buf_info.u4BufPA[ePlane_1st] = header_data[Header_CRZO_R1::E_IMG_PA];
    buf_info.m_num = header_data[Header_CRZO_R1::E_Magic];
    buf_info.img_fmt = header_data[Header_CRZO_R1::E_FMT];          //hw define, need to transfer to user-reconized enum in camiopipe
    buf_info.sw_fmt = header_data[Header_CRZO_R1::E_SW_FMT];
    buf_info.frm_cnt = 0;


    buf_info.crop_win.p.x = (header_data[Header_CRZO_R1::E_CRZ_CROP_START] & 0xffff);
    buf_info.crop_win.p.y = (header_data[Header_CRZO_R1::E_CRZ_CROP_START] >> 16);
    buf_info.crop_win.s.w = (header_data[Header_CRZO_R1::E_CRZ_CROP_SIZE] & 0xffff);
    buf_info.crop_win.s.h = (header_data[Header_CRZO_R1::E_CRZ_CROP_SIZE] >> 16);

    buf_info.DstSize.w = (header_data[Header_CRZO_R1::E_CRZ_DST_SIZE] & 0xffff);
    buf_info.DstSize.h = (header_data[Header_CRZO_R1::E_CRZ_DST_SIZE] >> 16);

    buf_info.i4TimeStamp_us = header_data[Header_CRZO_R1::E_TimeStamp_LSB];
    tmp = header_data[Header_CRZO_R1::E_TimeStamp_MSB];
    buf_info.i4TimeStamp_us += (tmp << 32);

    this->m_pTimeStamp->TimeStamp_mapping(&buf_info.i4TimeStamp_us,&buf_info.i4TimeStamp_us_B,(_isp_dma_enum_)this->id(), masterModule);

    buf_info.raw_type = 0;
    buf_info.xoffset = 0;
    buf_info.yoffset = 0;

    buf_info.eIQlv = (E_CamIQLevel)header_data[Header_CRZO_R1::E_IQ_LEVEL];
    //
    if(pStr){
        sprintf(pStr, "enq_sof:0x%x,mag:0x%x,timestamp:%" PRIu64 ",eIQlv:0x%x,enYUVPlane:0x%x",\
            header_data[Header_CRZO_R1::E_ENQUE_SOF],\
            header_data[Header_CRZO_R1::E_Magic], \
            buf_info.i4TimeStamp_us, \
            buf_info.eIQlv,\
            header_data[Header_CRZO_R1::E_PLANE_EN]);
    }
}

E_BC_STATUS BUF_CTRL_CRZBO_R1::dequeueHwBuf(NSImageio::NSIspio::BufInfo& buf_info)
{
    E_BC_STATUS ret = eCmd_Pass;
    E_ISP_CAM_CQ cq = ISP_DRV_CQ_NONE;
    MUINT32 page = 0;
    MUINT32 _size;
    MUINT32 _sof_idx = this->m_hwModule; // assign hwModule as input argument
    ST_BUF_INFO _buf;
    MUINT32 _header_data[E_HEADER_MAX];
    char str[128] = {'\0'};
    ISP_DRV_CAM* pDrv = NULL;
    Mutex::Autolock lock(this->m_bufctrl_lock);

    if(0xffffffff == this->id()){
        CAM_FUNC_ERR("dma channel error ");
        ret = eCmd_Fail;
        goto EXIT;
    }

    this->m_Queue_deque_ptr.sendCmd(QueueMgr<MUINTPTR>::eCmd_getsize,(MUINTPTR)&_size,0);
    if(_size == 0){
        CAM_FUNC_ERR("no enque record, plz enque first");
        return eCmd_Fail;
    }

    this->m_Queue.sendCmd(QueueMgr<ST_BUF_INFO>::eCmd_getsize,(MUINTPTR)&_size,0);
    if(_size < 1){
        CAM_FUNC_ERR("queue depth is empty for deque\n");
        ret = eCmd_Fail;
        goto EXIT;
    }

    this->m_Queue_deque_ptr.sendCmd(QueueMgr<MUINTPTR>::eCmd_front,(MUINTPTR)&pDrv,0);

    //dbg log
    pDrv->getCurObjInfo(&this->m_hwModule,&cq,&page);

    //fbc reg dbg log
    this->FBC_STATUS(pDrv->getPhyObj());

    //
    this->m_Queue.sendCmd(QueueMgr<ST_BUF_INFO>::eCmd_front,(MUINTPTR)&_buf,0);

    //
    pDrv->getDeviceInfo(_GET_SOF_CNT,(MUINT8 *)&_sof_idx);

    //
    switch(CAM_BUF_CTRL::m_CamState[this->m_hwModule]){
        case CAM_BUF_CTRL::eCamState_runnable:
        case CAM_BUF_CTRL::eCamState_suspend:   //this timing is for twin deque is happaned after dynamic twin suspend ready
            //
            this->Header_Parser(_buf.header.va_addr,buf_info,_header_data,str,pDrv->m_HWmasterModule);
            //
            if(_buf.image.mem_info.pa_addr != buf_info.u4BufPA[ePlane_2nd]){
                CAM_FUNC_ERR("%s:PA in header is mismatch with PA in list [0x%" PRIXPTR "_0x%" PRIXPTR "]\n",this->name_Str(),\
                    buf_info.u4BufPA[ePlane_2nd],_buf.image.mem_info.pa_addr);

                ret = eCmd_Fail;
                this->PipeCheck();
                goto EXIT;
            }
            break;
        case CAM_BUF_CTRL::eCamState_suspending:
            ret = eCmd_Suspending_Pass;

            //do not check header,dram on header's address is garbage!
            //need to pop all requests in driver when suspending for the drop method in iopipe.

            //Flush FH
            this->Header_Parser(_buf.header.va_addr,buf_info,_header_data,str,pDrv->m_HWmasterModule);

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
    buf_data_parsing(buf_info,ePlane_2nd,_buf);

    //
    this->m_Queue.sendCmd(QueueMgr<ST_BUF_INFO>::eCmd_pop,0,0);
    this->m_Queue_deque_ptr.sendCmd(QueueMgr<MUINTPTR>::eCmd_pop,0,0);
    this->m_buf_cnt--;

    //ion handle: buffer
    this->ion_handle_lock(pDrv,MFALSE,_buf.image.mem_info.memID,_buf.header.memID);

EXIT:
    CAM_FUNC_INF("%s:(0x%x_0x%x),PA(0x%" PRIXPTR "_0x%" PRIXPTR "),FH_VA(0x%" PRIXPTR "),size(0x%x),cur sof(%d),%s\n",\
        this->name_Str(),\
        cq,page,\
        _buf.image.mem_info.pa_addr ,\
        _buf.header.pa_addr ,\
        _buf.header.va_addr ,\
        _buf.image.mem_info.size ,\
        _sof_idx,\
        str);

    return ret;
}

void BUF_CTRL_CRZBO_R1::FBC_STATUS(IspDrv* ptr)
{
    CAM_FUNC_DBG("BUF_CTRL_CRZBO_R1::FBC_STATUS: 0x%x_0x%x\n",CAM_READ_REG(ptr,FBC_R1_FBC_CRZBO_R1_CTL1),CAM_READ_REG(ptr,FBC_R1_FBC_CRZBO_R1_CTL2));
}

void BUF_CTRL_CRZBO_R1::Header_Parser(MUINTPTR fh_va,NSImageio::NSIspio::BufInfo& buf_info,MUINT32* header_data ,char* pStr, ISP_HW_MODULE masterModule)
{
    Header_CRZBO_R1 fh_crzbo_r1;
    ISP_DRV_CAM* pDrv = NULL;
    MUINT64 tmp;

    this->m_Queue_deque_ptr.sendCmd(QueueMgr<MUINTPTR>::eCmd_front,(MUINTPTR)&pDrv,0);

    //
    fh_crzbo_r1.Header_Deque(fh_va,(void*)header_data);
    fh_crzbo_r1.Header_Flash(fh_va);

    //
    buf_info.u4BufPA[ePlane_2nd] = header_data[Header_CRZBO_R1::E_IMG_PA];
    buf_info.m_num = header_data[Header_CRZBO_R1::E_Magic];
    buf_info.img_fmt = header_data[Header_CRZBO_R1::E_FMT];        //hw define, need to transfer to user-reconized enum in camiopipe
    buf_info.img_pxl_id = header_data[Header_CRZBO_R1::E_PIX_ID];  //hw define, need to transfer to user-reconized enum in camiopipe
    buf_info.frm_cnt = 0;


    buf_info.crop_win.p.x = (header_data[Header_CRZBO_R1::E_CRZ_CROP_START] & 0xffff);
    buf_info.crop_win.p.y = (header_data[Header_CRZBO_R1::E_CRZ_CROP_START] >> 16);
    buf_info.crop_win.s.w = (header_data[Header_CRZBO_R1::E_CRZ_CROP_SIZE] & 0xffff);
    buf_info.crop_win.s.h = (header_data[Header_CRZBO_R1::E_CRZ_CROP_SIZE] >> 16);

    buf_info.DstSize.w = (header_data[Header_CRZBO_R1::E_CRZ_DST_SIZE] & 0xffff);
    buf_info.DstSize.h = (header_data[Header_CRZBO_R1::E_CRZ_DST_SIZE] >> 16);

    buf_info.i4TimeStamp_us = header_data[Header_CRZBO_R1::E_TimeStamp_LSB];
    tmp = header_data[Header_CRZBO_R1::E_TimeStamp_MSB];
    buf_info.i4TimeStamp_us += (tmp << 32);

    this->m_pTimeStamp->TimeStamp_mapping(&buf_info.i4TimeStamp_us,&buf_info.i4TimeStamp_us_B,(_isp_dma_enum_)this->id(), masterModule);

    buf_info.raw_type = 0;
    buf_info.xoffset = 0;
    buf_info.yoffset = 0;

    buf_info.eIQlv = (E_CamIQLevel)header_data[Header_CRZBO_R1::E_IQ_LEVEL];
    //
    if(pStr){
        sprintf(pStr, "enq_sof:0x%x,mag:0x%x,timestamp:%" PRIu64 ",eIQlv:0x%x",\
            header_data[Header_CRZBO_R1::E_ENQUE_SOF],\
            header_data[Header_CRZBO_R1::E_Magic], \
            buf_info.i4TimeStamp_us, \
            buf_info.eIQlv);
    }
}

void BUF_CTRL_CRZO_R2::FBC_STATUS(IspDrv* ptr)
{
    CAM_FUNC_DBG("BUF_CTRL_CRZO_R2::FBC_STATUS: 0x%x_0x%x\n",CAM_READ_REG(ptr,FBC_R1_FBC_CRZO_R2_CTL1),CAM_READ_REG(ptr,FBC_R1_FBC_CRZO_R2_CTL2));
}

void BUF_CTRL_CRZO_R2::Header_Parser(MUINTPTR fh_va,NSImageio::NSIspio::BufInfo& buf_info,MUINT32* header_data ,char* pStr, ISP_HW_MODULE masterModule)
{
    Header_CRZO_R2 fh_crzo_r2;
    ISP_DRV_CAM* pDrv = NULL;
    MUINT64 tmp;

    this->m_Queue_deque_ptr.sendCmd(QueueMgr<MUINTPTR>::eCmd_front,(MUINTPTR)&pDrv,0);

    //
    fh_crzo_r2.Header_Deque(fh_va,(void*)header_data);
    fh_crzo_r2.Header_Flash(fh_va);

    //
    buf_info.u4BufPA[ePlane_1st] = header_data[Header_CRZO_R2::E_IMG_PA];
    buf_info.m_num = header_data[Header_CRZO_R2::E_Magic];
    buf_info.img_fmt = header_data[Header_CRZO_R2::E_FMT];          //hw define, need to transfer to user-reconized enum in camiopipe
    buf_info.sw_fmt = header_data[Header_CRZO_R2::E_SW_FMT];
    buf_info.frm_cnt = 0;


    buf_info.crop_win.p.x = (header_data[Header_CRZO_R2::E_CRZ_CROP_START] & 0xffff);
    buf_info.crop_win.p.y = (header_data[Header_CRZO_R2::E_CRZ_CROP_START] >> 16);
    buf_info.crop_win.s.w = (header_data[Header_CRZO_R2::E_CRZ_CROP_SIZE] & 0xffff);
    buf_info.crop_win.s.h = (header_data[Header_CRZO_R2::E_CRZ_CROP_SIZE] >> 16);

    buf_info.DstSize.w = (header_data[Header_CRZO_R2::E_CRZ_DST_SIZE] & 0xffff);
    buf_info.DstSize.h = (header_data[Header_CRZO_R2::E_CRZ_DST_SIZE] >> 16);

    buf_info.i4TimeStamp_us = header_data[Header_CRZO_R2::E_TimeStamp_LSB];
    tmp = header_data[Header_CRZO_R2::E_TimeStamp_MSB];
    buf_info.i4TimeStamp_us += (tmp << 32);

    this->m_pTimeStamp->TimeStamp_mapping(&buf_info.i4TimeStamp_us,&buf_info.i4TimeStamp_us_B,(_isp_dma_enum_)this->id(), masterModule);

    buf_info.raw_type = 0;
    buf_info.xoffset = 0;
    buf_info.yoffset = 0;

    buf_info.eIQlv = (E_CamIQLevel)header_data[Header_CRZO_R2::E_IQ_LEVEL];
    //
    if(pStr){
        sprintf(pStr, "enq_sof:0x%x,mag:0x%x,timestamp:%" PRIu64 ",eIQlv:0x%x,enYUVPlane:0x%x",\
            header_data[Header_CRZO_R2::E_ENQUE_SOF],\
            header_data[Header_CRZO_R2::E_Magic], \
            buf_info.i4TimeStamp_us, \
            buf_info.eIQlv,\
            header_data[Header_CRZO_R2::E_PLANE_EN]);
    }
}

E_BC_STATUS BUF_CTRL_CRZBO_R2::dequeueHwBuf(NSImageio::NSIspio::BufInfo& buf_info)
{
    E_BC_STATUS ret = eCmd_Pass;
    E_ISP_CAM_CQ cq = ISP_DRV_CQ_NONE;
    MUINT32 page = 0;
    MUINT32 _size;
    MUINT32 _sof_idx = this->m_hwModule; // assign hwModule as input argument
    ST_BUF_INFO _buf;
    MUINT32 _header_data[E_HEADER_MAX];
    char str[128] = {'\0'};
    ISP_DRV_CAM* pDrv = NULL;
    Mutex::Autolock lock(this->m_bufctrl_lock);

    if(0xffffffff == this->id()){
        CAM_FUNC_ERR("dma channel error ");
        ret = eCmd_Fail;
        goto EXIT;
    }

    this->m_Queue_deque_ptr.sendCmd(QueueMgr<MUINTPTR>::eCmd_getsize,(MUINTPTR)&_size,0);
    if(_size == 0){
        CAM_FUNC_ERR("no enque record, plz enque first");
        return eCmd_Fail;
    }

    this->m_Queue.sendCmd(QueueMgr<ST_BUF_INFO>::eCmd_getsize,(MUINTPTR)&_size,0);
    if(_size < 1){
        CAM_FUNC_ERR("queue depth is empty for deque\n");
        ret = eCmd_Fail;
        goto EXIT;
    }

    this->m_Queue_deque_ptr.sendCmd(QueueMgr<MUINTPTR>::eCmd_front,(MUINTPTR)&pDrv,0);

    //dbg log
    pDrv->getCurObjInfo(&this->m_hwModule,&cq,&page);

    //fbc reg dbg log
    this->FBC_STATUS(pDrv->getPhyObj());

    //
    this->m_Queue.sendCmd(QueueMgr<ST_BUF_INFO>::eCmd_front,(MUINTPTR)&_buf,0);

    //
    pDrv->getDeviceInfo(_GET_SOF_CNT,(MUINT8 *)&_sof_idx);

    //
    switch(CAM_BUF_CTRL::m_CamState[this->m_hwModule]){
        case CAM_BUF_CTRL::eCamState_runnable:
        case CAM_BUF_CTRL::eCamState_suspend:   //this timing is for twin deque is happaned after dynamic twin suspend ready
            //
            this->Header_Parser(_buf.header.va_addr,buf_info,_header_data,str,pDrv->m_HWmasterModule);
            //
            if(_buf.image.mem_info.pa_addr != buf_info.u4BufPA[ePlane_2nd]){
                CAM_FUNC_ERR("%s:PA in header is mismatch with PA in list [0x%" PRIXPTR "_0x%" PRIXPTR "]\n",this->name_Str(),\
                    buf_info.u4BufPA[ePlane_2nd],_buf.image.mem_info.pa_addr);

                ret = eCmd_Fail;
                this->PipeCheck();
                goto EXIT;
            }
            break;
        case CAM_BUF_CTRL::eCamState_suspending:
            ret = eCmd_Suspending_Pass;

            //do not check header,dram on header's address is garbage!
            //need to pop all requests in driver when suspending for the drop method in iopipe.

            //Flush FH
            this->Header_Parser(_buf.header.va_addr,buf_info,_header_data,str,pDrv->m_HWmasterModule);

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
    buf_data_parsing(buf_info,ePlane_2nd,_buf);

    //
    this->m_Queue.sendCmd(QueueMgr<ST_BUF_INFO>::eCmd_pop,0,0);
    this->m_Queue_deque_ptr.sendCmd(QueueMgr<MUINTPTR>::eCmd_pop,0,0);
    this->m_buf_cnt--;

    //ion handle: buffer
    this->ion_handle_lock(pDrv,MFALSE,_buf.image.mem_info.memID,_buf.header.memID);

EXIT:
    CAM_FUNC_INF("%s:(0x%x_0x%x),PA(0x%" PRIXPTR "_0x%" PRIXPTR "),FH_VA(0x%" PRIXPTR "),size(0x%x),cur sof(%d),%s\n",\
        this->name_Str(),\
        cq,page,\
        _buf.image.mem_info.pa_addr ,\
        _buf.header.pa_addr ,\
        _buf.header.va_addr ,\
        _buf.image.mem_info.size ,\
        _sof_idx,\
        str);

    return ret;
}

void BUF_CTRL_CRZBO_R2::FBC_STATUS(IspDrv* ptr)
{
    CAM_FUNC_DBG("BUF_CTRL_CRZBO_R2::FBC_STATUS: 0x%x_0x%x\n",CAM_READ_REG(ptr,FBC_R1_FBC_CRZBO_R2_CTL1),CAM_READ_REG(ptr,FBC_R1_FBC_CRZBO_R2_CTL2));
}

void BUF_CTRL_CRZBO_R2::Header_Parser(MUINTPTR fh_va,NSImageio::NSIspio::BufInfo& buf_info,MUINT32* header_data ,char* pStr, ISP_HW_MODULE masterModule)
{
    Header_CRZBO_R2 fh_crzbo_r2;
    ISP_DRV_CAM* pDrv = NULL;
    MUINT64 tmp;

    this->m_Queue_deque_ptr.sendCmd(QueueMgr<MUINTPTR>::eCmd_front,(MUINTPTR)&pDrv,0);

    //
    fh_crzbo_r2.Header_Deque(fh_va,(void*)header_data);
    fh_crzbo_r2.Header_Flash(fh_va);

    //
    buf_info.u4BufPA[ePlane_2nd] = header_data[Header_CRZBO_R2::E_IMG_PA];
    buf_info.m_num = header_data[Header_CRZBO_R2::E_Magic];
    buf_info.img_fmt = header_data[Header_CRZBO_R2::E_FMT];        //hw define, need to transfer to user-reconized enum in camiopipe
    buf_info.img_pxl_id = header_data[Header_CRZBO_R2::E_PIX_ID];  //hw define, need to transfer to user-reconized enum in camiopipe
    buf_info.frm_cnt = 0;


    buf_info.crop_win.p.x = (header_data[Header_CRZBO_R2::E_CRZ_CROP_START] & 0xffff);
    buf_info.crop_win.p.y = (header_data[Header_CRZBO_R2::E_CRZ_CROP_START] >> 16);
    buf_info.crop_win.s.w = (header_data[Header_CRZBO_R2::E_CRZ_CROP_SIZE] & 0xffff);
    buf_info.crop_win.s.h = (header_data[Header_CRZBO_R2::E_CRZ_CROP_SIZE] >> 16);

    buf_info.DstSize.w = (header_data[Header_CRZBO_R2::E_CRZ_DST_SIZE] & 0xffff);
    buf_info.DstSize.h = (header_data[Header_CRZBO_R2::E_CRZ_DST_SIZE] >> 16);

    buf_info.i4TimeStamp_us = header_data[Header_CRZBO_R2::E_TimeStamp_LSB];
    tmp = header_data[Header_CRZBO_R2::E_TimeStamp_MSB];
    buf_info.i4TimeStamp_us += (tmp << 32);

    this->m_pTimeStamp->TimeStamp_mapping(&buf_info.i4TimeStamp_us,&buf_info.i4TimeStamp_us_B,(_isp_dma_enum_)this->id(), masterModule);

    buf_info.raw_type = 0;
    buf_info.xoffset = 0;
    buf_info.yoffset = 0;

    buf_info.eIQlv = (E_CamIQLevel)header_data[Header_CRZBO_R2::E_IQ_LEVEL];
    //
    if(pStr){
        sprintf(pStr, "enq_sof:0x%x,mag:0x%x,timestamp:%" PRIu64 ",eIQlv:0x%x",\
            header_data[Header_CRZBO_R2::E_ENQUE_SOF],\
            header_data[Header_CRZBO_R2::E_Magic], \
            buf_info.i4TimeStamp_us, \
            buf_info.eIQlv);
    }
}

void BUF_CTRL_LCSO::FBC_STATUS(IspDrv* ptr)
{
    CAM_FUNC_DBG("BUF_CTRL_LCSO::FBC_STATUS: 0x%x_0x%x\n",CAM_READ_REG(ptr,FBC_R1_FBC_LCESO_R1_CTL1),CAM_READ_REG(ptr,FBC_R1_FBC_LCESO_R1_CTL2));
}

void BUF_CTRL_LCSO::Header_Parser(MUINTPTR fh_va,NSImageio::NSIspio::BufInfo& buf_info,MUINT32* header_data ,char* pStr, ISP_HW_MODULE masterModule)
{
    Header_LCSO fh_lcso;
    MUINT64 tmp;

    //
    fh_lcso.Header_Deque(fh_va,(void*)header_data);
    fh_lcso.Header_Flash(fh_va);
    //
    buf_info.u4BufPA[ePlane_1st] = header_data[Header_LCSO::E_IMG_PA];
    buf_info.m_num = header_data[Header_LCSO::E_Magic];

    buf_info.i4TimeStamp_us = header_data[Header_LCSO::E_TimeStamp_LSB];
    tmp = header_data[Header_LCSO::E_TimeStamp_MSB];
    buf_info.i4TimeStamp_us += (tmp << 32);

    this->m_pTimeStamp->TimeStamp_mapping(&buf_info.i4TimeStamp_us,&buf_info.i4TimeStamp_us_B,(_isp_dma_enum_)this->id(), masterModule);

    buf_info.eIQlv = (E_CamIQLevel)header_data[Header_LCSO::E_IQ_LEVEL];

    //
    if(pStr){
        sprintf(pStr, "enq_sof:0x%x,mag:0x%x,timestamp:%" PRIu64 ",eIQlv:0x%x",\
            header_data[Header_LCSO::E_ENQUE_SOF],\
            header_data[Header_LCSO::E_Magic], \
            buf_info.i4TimeStamp_us, \
            buf_info.eIQlv);
    }
}

void BUF_CTRL_LMVO::FBC_STATUS(IspDrv* ptr)
{
    (void)ptr;
    CAM_FUNC_DBG("BUF_CTRL_LMVO::FBC_STATUS: 0x%x_0x%x\n",CAM_READ_REG(this->m_pDrv,FBC_R1_FBC_LMVO_R1_CTL1),CAM_READ_REG(this->m_pDrv,FBC_R1_FBC_LMVO_R1_CTL2));
}

void BUF_CTRL_LMVO::Header_Parser(MUINTPTR fh_va,NSImageio::NSIspio::BufInfo& buf_info,MUINT32* header_data ,char* pStr, ISP_HW_MODULE masterModule)
{
    Header_LMVO fh_lmvo;
    MUINT64 tmp;

    //
    fh_lmvo.Header_Deque(fh_va,(void*)header_data);
    fh_lmvo.Header_Flash(fh_va);
    //
    buf_info.u4BufPA[ePlane_1st] = header_data[Header_LMVO::E_IMG_PA];
    buf_info.m_num = header_data[Header_LMVO::E_Magic];

    buf_info.i4TimeStamp_us = header_data[Header_LMVO::E_TimeStamp_LSB];
    tmp = header_data[Header_LMVO::E_TimeStamp_MSB];
    buf_info.i4TimeStamp_us += (tmp << 32);

    this->m_pTimeStamp->TimeStamp_mapping(&buf_info.i4TimeStamp_us,&buf_info.i4TimeStamp_us_B,(_isp_dma_enum_)this->id(), masterModule);

    buf_info.eIQlv = (E_CamIQLevel)header_data[Header_LMVO::E_IQ_LEVEL];

    //
    if(pStr){
        sprintf(pStr, "enq_sof:0x%x,mag:0x%x,timestamp:%" PRIu64 ",eIQlv:0x%x",\
            header_data[Header_LMVO::E_ENQUE_SOF],\
            header_data[Header_LMVO::E_Magic], \
            buf_info.i4TimeStamp_us, \
            buf_info.eIQlv);
    }
}

void BUF_CTRL_RSSO::FBC_STATUS(IspDrv* ptr)
{
    (void)ptr;
     CAM_FUNC_DBG("BUF_CTRL_RSSO::FBC_STATUS: 0x%x_0x%x\n",CAM_READ_REG(this->m_pDrv,FBC_R1_FBC_RSSO_R1_CTL1),CAM_READ_REG(this->m_pDrv,FBC_R1_FBC_RSSO_R1_CTL2));
}

void BUF_CTRL_RSSO::Header_Parser(MUINTPTR fh_va,NSImageio::NSIspio::BufInfo& buf_info,MUINT32* header_data ,char* pStr, ISP_HW_MODULE masterModule)
{
    Header_RSSO fh_rsso;
    MUINT64 tmp;

    //
    fh_rsso.Header_Deque(fh_va,(void*)header_data);
    fh_rsso.Header_Flash(fh_va);
    //
    buf_info.u4BufPA[ePlane_1st] = header_data[Header_RSSO::E_IMG_PA];
    buf_info.m_num = header_data[Header_RSSO::E_Magic];

    buf_info.i4TimeStamp_us = header_data[Header_RSSO::E_TimeStamp_LSB];
    tmp = header_data[Header_RSSO::E_TimeStamp_MSB];
    buf_info.i4TimeStamp_us += (tmp << 32);

    this->m_pTimeStamp->TimeStamp_mapping(&buf_info.i4TimeStamp_us,&buf_info.i4TimeStamp_us_B,(_isp_dma_enum_)this->id(), masterModule);

    buf_info.eIQlv = (E_CamIQLevel)header_data[Header_RSSO::E_IQ_LEVEL];
    buf_info.DstSize.w = (header_data[Header_RSSO::E_DST_SIZE] & 0xFFFF);
    buf_info.DstSize.h = (header_data[Header_RSSO::E_DST_SIZE] >> 16);

    //
    if(pStr){
        sprintf(pStr, "enq_sof:0x%x,mag:0x%x,timestamp:%" PRIu64 ",eIQlv:0x%x",\
            header_data[Header_RSSO::E_ENQUE_SOF],\
            header_data[Header_RSSO::E_Magic], \
            buf_info.i4TimeStamp_us, \
            buf_info.eIQlv);
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

    Header_AAO fh_aao;
    MUINT32 _pa;
    ST_BUF_INFO _buf;
    MUINT32 _header_data[E_HEADER_MAX];

    ISP_WAIT_IRQ_ST irq;
    MUINT32 loopCnt = _LOOP_;
    char str[128] = {'\0'};

    MUINT32 FbcCnt, sw_buf_cnt;
    MUINT32 subSample;
    MUINT32 reWaitCnt = 1;

    this->m_pDrv = (ISP_DRV_CAM*)this->m_pIspDrv;

    //dbg log
    this->m_pDrv->getCurObjInfo(&this->m_hwModule,&cq,&page);
    if ( 0xffffffff == this->id() ) {
        CAM_FUNC_ERR("dma channel error \n");
        return eCmd_Fail;
    }

    //fbc reg dbg log
    this->FBC_STATUS(this->m_pDrv->getPhyObj());

    switch (CAM_BUF_CTRL::m_CamState[this->m_hwModule]) {
        //theoratically, it no need to reference normalpipe's status under sttpipe.
        //but due to suspend/resume is only implemented in noramlpipe, so sttpipe need to use suspend/resume status.
        case CAM_BUF_CTRL::eCamState_suspending:
            return eCmd_Suspending_Pass;
            break;
        default:
            break;
    }

    subSample = CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAMCTL_R1_CAMCTL_SW_PASS1_DONE,CAMCTL_DOWN_SAMPLE_PERIOD) + 1;

    irq.Clear = ISP_IRQ_CLEAR_WAIT;
    irq.UserKey = 0;
    irq.St_type = DMA_INT;
    irq.Status = (MUINT32)AAO_DONE_ST;

    do{
        //irq.Timeout = ((1000 + (this->m_fps - 1)) / this->m_fps) * subSample;
        //irq.Timeout *= 4; //amplify 3 times is for the case of enque latch timing
        irq.Timeout = this->estimateTimeout(subSample);

        this->m_bufctrl_lock.lock();
        sw_buf_cnt = this->m_buf_cnt;
        this->m_bufctrl_lock.unlock();

        if(sw_buf_cnt != 0){

            //
            this->m_Queue.sendCmd(QueueMgr<ST_BUF_INFO>::eCmd_at,0,(MUINTPTR)&_buf);
            IspFunction_B::fbcPhyLock[this->m_hwModule][this->id()].lock();
            FbcCnt = CAM_READ_BITS(this->m_pDrv->getPhyObj(),FBC_R1_FBC_AAO_R1_CTL2,FBC_AAO_R1_FBC_CNT);
            IspFunction_B::fbcPhyLock[this->m_hwModule][this->id()].unlock();
            fh_aao.Header_Deque(_buf.header.va_addr,(void*)_header_data);
            _pa = _header_data[Header_AAO::E_IMG_PA];
            if((FbcCnt< sw_buf_cnt) && (_pa != 0)){
                ret = eCmd_Pass;
                goto EXIT;
            }
            else{
                if(loopCnt == _LOOP_)
                    snprintf(str, sizeof(str), "dma:%s start wait:[enque record:0x%x_0x%x],",this->name_Str(),sw_buf_cnt,FbcCnt);
                else{
                    char _tmp[16];
                    snprintf(_tmp, sizeof(_tmp), "[%d]time,",((_LOOP_-loopCnt)+1));
                    strncat(str,_tmp, strlen(_tmp));
                }
            }
        }
        else{
            if(loopCnt == _LOOP_){
                snprintf(str, sizeof(str), "dma:%s bWaitBufReady = 1,wait for sw enque[0x%x],timeout:%d x %d ms,[1] ",this->name_Str(),sw_buf_cnt,irq.Timeout,loopCnt);
            }
            else{//save how many times of wait_irq before sw enque
                char _tmp[16];
                snprintf(_tmp, sizeof(_tmp), "[%d],",((_LOOP_-loopCnt)+1)+1);
                strncat(str,_tmp, strlen(_tmp));
            }
        }

        //
        if (pNotify) {
            if(pNotify->fpNotifyState(CAM_ENTER_WAIT, pNotify->Obj) == NOTIFY_FAIL){
                CAM_FUNC_ERR("dma:%s NotifyCB fail",this->name_Str());
                ret = eCmd_Fail;
            }
        }

        if(this->m_pDrv->waitIrq(&irq) == MFALSE) {
            ret = eCmd_Fail;
        } else {
            ret = eCmd_Pass;
        }

        // if user doesn't deque for a while, in next deque, waitBufReady will be passed directly by checking frame header.
        // under this condtion, the done status in kernel is not cleared, and this may cause pa mismatch when dequeHwBuf.
        // therefore, check fh after waitIrq pass. if pa != 0, wait again.
        if(ret == eCmd_Pass){
            if(reWaitCnt > 0){
                this->m_Queue.sendCmd(QueueMgr<ST_BUF_INFO>::eCmd_at, 0, (MUINTPTR)&_buf);
                fh_aao.Header_Deque(_buf.header.va_addr, (void*)_header_data);
                _pa = _header_data[Header_AAO::E_IMG_PA];
                if(_pa == 0){
                    reWaitCnt = 0;
                    continue;
                }
                else{
                    reWaitCnt = 0;
                }
            }
        }
        else{
            reWaitCnt = 0;
        }

        if (pNotify) {
            if(pNotify->fpNotifyState(CAM_EXIT_WAIT, pNotify->Obj) == NOTIFY_FAIL){
                CAM_FUNC_ERR("dma:%s NotifyCB fail",this->name_Str());
                ret = eCmd_Fail;
            }
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
        if(sw_buf_cnt != 0){
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
    CAM_FUNC_DBG("BUF_CTRL_AAO::FBC_STATUS: 0x%x_0x%x\n",CAM_READ_REG(ptr,FBC_R1_FBC_AAO_R1_CTL1),CAM_READ_REG(ptr,FBC_R1_FBC_AAO_R1_CTL2));
}

void BUF_CTRL_AAO::Header_Parser(MUINTPTR fh_va,NSImageio::NSIspio::BufInfo& buf_info,MUINT32* header_data ,char* pStr, ISP_HW_MODULE masterModule)
{
    Header_AAO  fh_aao;
    MUINT64 tmp;

    //
    fh_aao.Header_Deque(fh_va,(void*)header_data);
    fh_aao.Header_Flash(fh_va);

    //
    buf_info.u4BufPA[ePlane_1st] = header_data[Header_AAO::E_IMG_PA];
    buf_info.m_num = header_data[Header_AAO::E_Magic];

    buf_info.i4TimeStamp_us = header_data[Header_AAO::E_TimeStamp_LSB];
    tmp = header_data[Header_AAO::E_TimeStamp_MSB];
    buf_info.i4TimeStamp_us += (tmp << 32);

    this->m_pTimeStamp->TimeStamp_mapping(&buf_info.i4TimeStamp_us,&buf_info.i4TimeStamp_us_B,(_isp_dma_enum_)this->id(), masterModule);

    buf_info.eIQlv = (E_CamIQLevel)header_data[Header_AAO::E_IQ_LEVEL];
    buf_info.sensorId = header_data[Header_AAO::E_SENSOR_ID];

    //
    if(pStr){
        sprintf(pStr, "enq_sof:0x%x,mag:0x%x,timestamp:%" PRIu64 ",eIQlv:0x%x",\
            header_data[Header_AAO::E_ENQUE_SOF],\
            header_data[Header_AAO::E_Magic], \
            buf_info.i4TimeStamp_us, \
            buf_info.eIQlv);
    }
}
void BUF_CTRL_AFO::FBC_STATUS(IspDrv* ptr)
{
    CAM_FUNC_DBG("BUF_CTRL_AFO::FBC_STATUS: 0x%x_0x%x\n",CAM_READ_REG(ptr,FBC_R1_FBC_AFO_R1_CTL1),CAM_READ_REG(ptr,FBC_R1_FBC_AFO_R1_CTL2));
}

void BUF_CTRL_AFO::Header_Parser(MUINTPTR fh_va,NSImageio::NSIspio::BufInfo& buf_info,MUINT32* header_data ,char* pStr, ISP_HW_MODULE masterModule)
{
    Header_AFO  fh_afo;
    MUINT64 tmp;

    //
    fh_afo.Header_Deque(fh_va,(void*)header_data);
    fh_afo.Header_Flash(fh_va);

    //
    buf_info.u4BufPA[ePlane_1st] = header_data[Header_AFO::E_IMG_PA];
    buf_info.m_num = header_data[Header_AFO::E_Magic];

    buf_info.i4TimeStamp_us = header_data[Header_AFO::E_TimeStamp_LSB];
    tmp = header_data[Header_AFO::E_TimeStamp_MSB];
    buf_info.i4TimeStamp_us += (tmp << 32);

    this->m_pTimeStamp->TimeStamp_mapping(&buf_info.i4TimeStamp_us,&buf_info.i4TimeStamp_us_B,(_isp_dma_enum_)this->id(), masterModule);

    buf_info.img_stride = header_data[Header_AFO::E_STRIDE];

    buf_info.eIQlv = (E_CamIQLevel)header_data[Header_AFO::E_IQ_LEVEL];
    buf_info.sensorId = header_data[Header_AFO::E_SENSOR_ID];

    //
    if(pStr){
        sprintf(pStr, "enq_sof:0x%x,mag:0x%x,timestamp:%" PRIu64 ",stride:0x%x,eIQlv:0x%x",\
            header_data[Header_AFO::E_ENQUE_SOF],\
            header_data[Header_AFO::E_Magic], \
            buf_info.i4TimeStamp_us,
            buf_info.img_stride, \
            buf_info.eIQlv);
    }
}

E_BC_STATUS BUF_CTRL_AFO::waitBufReady(CAM_STATE_NOTIFY *pNotify)
{
    #define _LOOP_  (5)
    E_BC_STATUS ret = eCmd_Pass;
    E_ISP_CAM_CQ cq;
    MUINT32 page;

    ISP_WAIT_IRQ_ST irq;
    MUINT32 loopCnt = _LOOP_;
    char str[128] = {'\0'};

    MUINT32 FbcCnt, sw_buf_cnt;
    MUINT32 _header_data[E_HEADER_MAX];
    MUINT32 _pa;
    Header_AFO fh_afo;
    ST_BUF_INFO _buf;
    MUINT32 _size;
    MUINT32 subSample;
    MUINT32 reWaitCnt = 1;

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

    //fbc reg dbg log
    this->FBC_STATUS(this->m_pDrv->getPhyObj());

    switch (CAM_BUF_CTRL::m_CamState[this->m_hwModule]) {
        //theoratically, it no need to reference normalpipe's status under sttpipe.
        //but due to suspend/resume is only implemented in noramlpipe, so sttpipe need to use suspend/resume status.
        case CAM_BUF_CTRL::eCamState_suspending:
            return eCmd_Suspending_Pass;
            break;
        default:
            break;
    }

    subSample = CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAMCTL_R1_CAMCTL_SW_PASS1_DONE,CAMCTL_DOWN_SAMPLE_PERIOD) + 1;

    //Need to check HW AFO_B must have done under twin mode.
    irq.Clear = ISP_IRQ_CLEAR_WAIT;
    irq.UserKey = 0;
    irq.St_type = DMA_INT;
    irq.Status = (MUINT32)AFO_DONE_ST;

    do{
        //irq.Timeout = ((1000 + (this->m_fps - 1)) / this->m_fps) * subSample;
        //irq.Timeout *= 4; //amplify 3 times is for the case of enque latch timing
        irq.Timeout = this->estimateTimeout(subSample);

        this->m_bufctrl_lock.lock();
        sw_buf_cnt = this->m_buf_cnt;
        this->m_bufctrl_lock.unlock();

        if(sw_buf_cnt != 0){

            //
            this->m_Queue.sendCmd(QueueMgr<ST_BUF_INFO>::eCmd_at,0,(MUINTPTR)&_buf);
            IspFunction_B::fbcPhyLock[this->m_hwModule][this->id()].lock();
            FbcCnt = CAM_READ_BITS(this->m_pDrv->getPhyObj(),FBC_R1_FBC_AFO_R1_CTL2,FBC_AFO_R1_FBC_CNT);
            IspFunction_B::fbcPhyLock[this->m_hwModule][this->id()].unlock();
            //af need to check header is because of fbccnt is updated at p1_don, not af don
            fh_afo.Header_Deque(_buf.header.va_addr,(void*)_header_data);
            _pa = _header_data[Header_AFO::E_IMG_PA];
            if((FbcCnt < sw_buf_cnt) && (_pa != 0)){
                if(loopCnt == _LOOP_){
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
                    snprintf(str, sizeof(str), "dma:%s _pa:0x%x start wait:[enque record:0x%x_0x%x],",this->name_Str(), _pa, sw_buf_cnt,FbcCnt);
                else{
                    char _tmp[16];
                    snprintf(_tmp, sizeof(_tmp), "[%d]time,",((_LOOP_-loopCnt)+1));
                    strncat(str,_tmp, strlen(_tmp));
                }
            }
        }
        else{
            if(loopCnt == _LOOP_){
                snprintf(str, sizeof(str), "dma:%s bWaitBufReady = 1,wait for sw enque[0x%x],timeout:%d x %d ms,[1] ",\
                    this->name_Str(),sw_buf_cnt,irq.Timeout,loopCnt);
            }
            else{//save how many times of wait_irq before sw enque
                char _tmp[16];
                snprintf(_tmp, sizeof(_tmp), "[%d],",((_LOOP_-loopCnt)+1)+1);
                strncat(str,_tmp, strlen(_tmp));
            }
        }

        //
        if (pNotify) {
            if(pNotify->fpNotifyState(CAM_ENTER_WAIT, pNotify->Obj) == NOTIFY_FAIL){
                CAM_FUNC_ERR("dma:%s NotifyCB fail",this->name_Str());
                ret = eCmd_Fail;
            }
        }

        if(this->m_pDrv->waitIrq(&irq) == MFALSE) {
            ret = eCmd_Fail;
        } else {
            ret = eCmd_Pass;
        }

        // if user doesn't deque for a while, in next deque, waitBufReady will be passed directly by checking frame header.
        // under this condtion, the done status in kernel is not cleared, and this may cause pa mismatch when dequeHwBuf.
        // therefore, check fh after waitIrq pass. if pa != 0, wait again.
        if(ret == eCmd_Pass){
            if(reWaitCnt > 0){
                this->m_Queue.sendCmd(QueueMgr<ST_BUF_INFO>::eCmd_at, 0, (MUINTPTR)&_buf);
                fh_afo.Header_Deque(_buf.header.va_addr, (void*)_header_data);
                _pa = _header_data[Header_AFO::E_IMG_PA];
                if(_pa == 0){
                    reWaitCnt = 0;
                    continue;
                }
                else{
                    reWaitCnt = 0;
                }
            }
        }
        else{
            reWaitCnt = 0;
        }

        if (pNotify) {
            if(pNotify->fpNotifyState(CAM_EXIT_WAIT, pNotify->Obj) == NOTIFY_FAIL){
                CAM_FUNC_ERR("dma:%s NotifyCB fail",this->name_Str());
                ret = eCmd_Fail;
            }
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
        if(sw_buf_cnt != 0){
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
    CAM_FUNC_DBG("BUF_CTRL_PDO::FBC_STATUS: 0x%x_0x%x\n",CAM_READ_REG(ptr,FBC_R1_FBC_PDO_R1_CTL1),CAM_READ_REG(ptr,FBC_R1_FBC_PDO_R1_CTL2));
}

MINT32 BUF_CTRL_PDO::_config(void)
{
    E_ISP_CAM_CQ cq;
    MUINT32 page;
    this->m_pDrv = (ISP_DRV_CAM*)this->m_pIspDrv;
    //dbg log
    this->m_pDrv->getCurObjInfo(&this->m_hwModule,&cq,&page);

    CAM_FUNC_DBG("%s::_config+ cq:0x%x,page:0x%x\n",this->name_Str(),cq,page);

    this->m_Queue.init();
    this->m_Queue_deque_ptr.init();

    if (cq == ISP_DRV_CQ_THRE7) {
        CAM_WRITE_REG(this->m_pDrv->getPhyObj(),FBC_R1_FBC_PDO_R1_CTL1,0);
    } else {
        CAM_WRITE_REG(this->m_pDrv,FBC_R1_FBC_PDO_R1_CTL1,0);
    }

    //reset buf cnt
    this->m_buf_cnt = 0;
    this->m_enqCnt_pdo_suspending = 0;
    return 0;
}


void BUF_CTRL_PDO::Header_Parser(MUINTPTR fh_va,NSImageio::NSIspio::BufInfo& buf_info,MUINT32* header_data ,char* pStr, ISP_HW_MODULE masterModule)
{
    Header_PDO  fh_pdo;

    MUINT32 bin, dbin, psel;
    MUINT64 tmp;

    //
    fh_pdo.Header_Deque(fh_va,(void*)header_data);
    fh_pdo.Header_Flash(fh_va);

    //
    buf_info.u4BufPA[ePlane_1st] = header_data[Header_PDO::E_IMG_PA];
    buf_info.m_num = header_data[Header_PDO::E_Magic];
    buf_info.img_stride = header_data[Header_PDO::E_STRIDE];
    buf_info.u4BufOffset[ePlane_1st] = header_data[Header_PDO::E_IMG_PA_OFSET];

    //
    buf_info.i4TimeStamp_us = header_data[Header_PDO::E_TimeStamp_LSB];
    tmp = header_data[Header_PDO::E_TimeStamp_MSB];
    buf_info.i4TimeStamp_us += (tmp << 32);

    this->m_pTimeStamp->TimeStamp_mapping(&buf_info.i4TimeStamp_us,&buf_info.i4TimeStamp_us_B,(_isp_dma_enum_)this->id(), masterModule);

    buf_info.eIQlv = (E_CamIQLevel)header_data[Header_PDO::E_IQ_LEVEL];
    buf_info.sensorId = header_data[Header_PDO::E_SENSOR_ID];

    //
    if(pStr){
        sprintf(pStr, "enq_sof:0x%x,mag:0x%x,timestamp:%" PRIu64 ",eIQlv:0x%x",\
            header_data[Header_PDO::E_ENQUE_SOF],\
            header_data[Header_PDO::E_Magic], \
            buf_info.i4TimeStamp_us, \
            buf_info.eIQlv);
    }
}


E_BC_STATUS BUF_CTRL_PDO::waitBufReady( CAM_STATE_NOTIFY *pNotify)
{
    #define _LOOP_  (5)
    E_BC_STATUS ret = eCmd_Pass;
    E_ISP_CAM_CQ cq;
    MUINT32 page;

    Header_PDO  fh_pdo;

    ISP_WAIT_IRQ_ST irq;
    MUINT32 loopCnt = _LOOP_;
    char str[128] = {'\0'};

    MUINT32 FbcCnt, sw_buf_cnt;
    MUINT32 _header_data[E_HEADER_MAX];
    MUINT32 _pa;
    ST_BUF_INFO _buf;
    MUINT32 _size;
    MUINT32 subSample;
    MUINT32 reWaitCnt = 1;

    this->m_Queue_deque_ptr.sendCmd(QueueMgr<MUINTPTR>::eCmd_getsize,(MUINTPTR)&_size,0);
    if(_size <1){
        CAM_FUNC_ERR("queue depth is empty for deque\n");
        return eCmd_Fail;
    }

    this->m_pDrv = (ISP_DRV_CAM*)this->m_pIspDrv;

    //dbg log
    this->m_pDrv->getCurObjInfo(&this->m_hwModule,&cq,&page);
    if ( _pdo_ != this->id() ) {
        CAM_FUNC_ERR("dma channel error \n");
        return eCmd_Fail;
    }

    //fbc reg dbg log
    this->FBC_STATUS(this->m_pDrv->getPhyObj());

    switch (CAM_BUF_CTRL::m_CamState[this->m_hwModule]) {
        //theoratically, it no need to reference normalpipe's status under sttpipe.
        //but due to suspend/resume is only implemented in noramlpipe, so sttpipe need to use suspend/resume status.
        case CAM_BUF_CTRL::eCamState_suspending:
            return eCmd_Suspending_Pass;
            break;
        default:
            break;
    }
    //for raw_sel switched with different sensor pattern
    switch(CAM_BUF_CTRL::m_SttState[this->m_hwModule]){
        case CAM_BUF_CTRL::eState_forPDOnly:
            return eCmd_Dummy;
            break;
        default:
            break;
    }


    subSample = CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAMCTL_R1_CAMCTL_SW_PASS1_DONE,CAMCTL_DOWN_SAMPLE_PERIOD) + 1;
    irq.Clear = ISP_IRQ_CLEAR_WAIT;


    irq.UserKey = 0;
    irq.St_type = DMA_INT;
    irq.Status = (MUINT32)PDO_DONE_ST;

    do{
        //irq.Timeout = ((1000 + (this->m_fps - 1)) / this->m_fps) * subSample;
        //irq.Timeout *= 4; //amplify 3 times is for the case of enque latch timing
        irq.Timeout = this->estimateTimeout(subSample);

        this->m_bufctrl_lock.lock();
        sw_buf_cnt = this->m_buf_cnt;
        this->m_bufctrl_lock.unlock();

        if(sw_buf_cnt != 0){

            //
            this->m_Queue.sendCmd(QueueMgr<ST_BUF_INFO>::eCmd_at,0,(MUINTPTR)&_buf);
            IspFunction_B::fbcPhyLock[this->m_hwModule][this->id()].lock();
            FbcCnt = CAM_READ_BITS(this->m_pDrv->getPhyObj(),FBC_R1_FBC_PDO_R1_CTL2,FBC_PDO_R1_FBC_CNT);
            IspFunction_B::fbcPhyLock[this->m_hwModule][this->id()].unlock();
            //pd need to check header is because of fbccnt is updated at p1_don, not pd don
            fh_pdo.Header_Deque(_buf.header.va_addr,(void*)_header_data);
            _pa = _header_data[Header_PDO::E_IMG_PA];
            if((FbcCnt < sw_buf_cnt) && (_pa != 0)){
                ret = eCmd_Pass;
                goto EXIT;
            }
            else{
                if(loopCnt == _LOOP_)
                    snprintf(str, sizeof(str), "dma:%s start wait:[enque record:0x%x_0x%x],",this->name_Str(),sw_buf_cnt,FbcCnt);
                else{
                    char _tmp[16];
                    snprintf(_tmp, sizeof(_tmp), "[%d]time,",((_LOOP_-loopCnt)+1));
                    strncat(str,_tmp, strlen(_tmp));
                }
            }
        }
        else{
            if(loopCnt == _LOOP_){
                snprintf(str, sizeof(str), "dma:%s bWaitBufReady = 1,wait for sw enque[0x%x],timeout:%d x %d ms,[1] ",this->name_Str(),sw_buf_cnt,irq.Timeout,loopCnt);
            }
            else{//save how many times of wait_irq before sw enque
                char _tmp[16];
                snprintf(_tmp, sizeof(_tmp), "[%d],",((_LOOP_-loopCnt)+1)+1);
                strncat(str,_tmp, strlen(_tmp));
            }
        }

        //
        if (pNotify) {
            if(pNotify->fpNotifyState(CAM_ENTER_WAIT, pNotify->Obj) == NOTIFY_FAIL){
                CAM_FUNC_ERR("dma:%s NotifyCB fail",this->name_Str());
                ret = eCmd_Fail;
            }
        }

        if(this->m_pDrv->waitIrq(&irq) == MFALSE) {
            ret = eCmd_Fail;
        } else {
            ret = eCmd_Pass;
        }

        // if user doesn't deque for a while, in next deque, waitBufReady will be passed directly by checking frame header.
        // under this condtion, the done status in kernel is not cleared, and this may cause pa mismatch when dequeHwBuf.
        // therefore, check fh after waitIrq pass. if pa != 0, wait again.
        if(ret == eCmd_Pass){
            if(reWaitCnt > 0){
                this->m_Queue.sendCmd(QueueMgr<ST_BUF_INFO>::eCmd_at, 0, (MUINTPTR)&_buf);
                fh_pdo.Header_Deque(_buf.header.va_addr, (void*)_header_data);
                _pa = _header_data[Header_PDO::E_IMG_PA];
                if(_pa == 0){
                    reWaitCnt = 0;
                    continue;
                }
                else{
                    reWaitCnt = 0;
                }
            }
        }
        else{
            reWaitCnt = 0;
        }

        if (pNotify) {
            if(pNotify->fpNotifyState(CAM_EXIT_WAIT, pNotify->Obj) == NOTIFY_FAIL){
                CAM_FUNC_ERR("dma:%s NotifyCB fail",this->name_Str());
                ret = eCmd_Fail;
            }
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
        switch(CAM_BUF_CTRL::m_SttState[this->m_hwModule]){
            case CAM_BUF_CTRL::eCamState_stop:
                CAM_FUNC_INF("%s fast stt stop %s\n",this->name_Str(),str); /* stt stop before main*/
                return eCmd_Stop_Pass;
                break;
            case CAM_BUF_CTRL::eState_forPDOnly:
                return eCmd_Dummy;
                break;
            default:
                break;
        }
        //
        irq.Clear = ISP_IRQ_CLEAR_NONE;
        //
        if(ret == eCmd_Pass)
            break;
    }while(--loopCnt > 0);
EXIT:

    if(ret == eCmd_Fail){
        if(sw_buf_cnt != 0){
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

E_BC_STATUS BUF_CTRL_PDO::enqueueHwBuf( stISP_BUF_INFO& buf_info,MBOOL bImdMode)
{
    ST_BUF_INFO* ptr = NULL;
    E_ISP_CAM_CQ cq;
    MUINT32 page = 0;
    MUINT32 _sof_idx = this->m_hwModule; // assign hwModule as input argument
    MUINT32 _size = 0, subSample = 0;
    ST_BUF_INFO _buf;

    this->m_pDrv = (ISP_DRV_CAM*)this->m_pIspDrv;
    (void)bImdMode;

    Mutex::Autolock lock(this->m_bufctrl_lock);

    //dbg log
    this->m_pDrv->getCurObjInfo(&this->m_hwModule,&cq,&page);

    subSample = CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAMCTL_R1_CAMCTL_SW_PASS1_DONE,CAMCTL_DOWN_SAMPLE_PERIOD) + 1;

    if ( _pdo_ != this->id() ) {
        CAM_FUNC_ERR("dma channel error\n");
        return eCmd_Fail;
    }

    //
    this->m_pDrv->getDeviceInfo(_GET_SOF_CNT,(MUINT8 *) &_sof_idx);
    //fbc reg dbg log
    this->FBC_STATUS(this->m_pDrv->getPhyObj());

    //
    if(buf_info.bReplace == MTRUE){
        ptr = &buf_info.Replace;
    }
    else{
        ptr = &buf_info.u_op.enque.at(ePlane_1st);
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

    //ion handle: buffer
    this->ion_handle_lock(this->m_pDrv,MTRUE,ptr->image.mem_info.memID,ptr->header.memID);
    //
    this->m_Queue.sendCmd(QueueMgr<ST_BUF_INFO>::eCmd_push,(MUINTPTR)ptr,0);
    this->m_buf_cnt++;

    //
    {
        CQ_RingBuf_ST Buf_ctrl = {};
        Header_PDO  fh_pdo;

        fh_pdo.m_hwModule = this->m_hwModule;

        Buf_ctrl.ctrl = GET_RING_DEPTH;
        if(this->m_pDrv->cqRingBuf(&Buf_ctrl) == 0){
            CAM_FUNC_ERR("%s, can't enque before PA push into CQ\n",this->name_Str());
            return eCmd_Fail;
        }
        //statistic pipe enque to hw directly
        if(CAM_READ_BITS(this->m_pDrv->getPhyObj(),FBC_R1_FBC_PDO_R1_CTL1,FBC_PDO_R1_FBC_EN) != 1){
            switch(CAM_BUF_CTRL::m_SttState[this->m_hwModule]){
                case CAM_BUF_CTRL::eState_forPDOnly:
                    this->m_enqCnt_pdo_suspending++;
                    break;
                default:
                    CAM_FUNC_WRN("%s: can't enque before enabe fbc(not a problem for w+t switch)\n",this->name_Str());
                    break;
            }
        }
        while(this->m_enqCnt_pdo_suspending){
            CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),CAMCTL_R1_CAMCTL_FBC_RCNT_INC,CAMCTL_PDO_R1_RCNT_INC,1);
            this->m_enqCnt_pdo_suspending--;
            CAM_FUNC_INF("burst enque_%d\n",this->m_enqCnt_pdo_suspending);
        }
        CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),CAMCTL_R1_CAMCTL_FBC_RCNT_INC,CAMCTL_PDO_R1_RCNT_INC,1);

        //
        Buf_ctrl.ctrl = SET_FH_SPARE;
        Buf_ctrl.dma_PA = ptr->image.mem_info.pa_addr;
        if (this->m_pDrv->cqRingBuf(&Buf_ctrl)) {
            fh_pdo.Header_Enque(Header_PDO::E_ENQUE_SOF,(ISP_DRV_CAM*)Buf_ctrl.pDma_fh_spare,_sof_idx);
        }

        this->m_pDrv->m_HWmasterModule = (CAM_READ_BITS(this->m_pDrv->getPhyObj(), CAMCTL_R1_CAMCTL_TWIN_STATUS, TWIN_EN) == MTRUE) ?
                                         ((ISP_HW_MODULE)CAM_READ_BITS(this->m_pDrv->getPhyObj(), CAMCTL_R1_CAMCTL_TWIN_STATUS, MASTER_MODULE)) : (this->m_hwModule);
    }

    //
    this->m_Queue_deque_ptr.sendCmd(QueueMgr<MUINTPTR>::eCmd_push,(MUINTPTR)&this->m_pDrv,0);
    //
    return eCmd_Pass;

}

E_BC_STATUS BUF_CTRL_PDO::dequeueHwBuf(NSImageio::NSIspio::BufInfo& buf_info )
{
    E_BC_STATUS ret = eCmd_Pass;
    E_ISP_CAM_CQ cq = ISP_DRV_CQ_NONE;
    MUINT32 page = 0;
    MUINT32 _size = 0;
    MUINT32 _sof_idx = this->m_hwModule; // assign hwModule as input argument
    MUINT32 subSample = 0;
    ST_BUF_INFO _buf;
    MUINT32 _header_data[E_HEADER_MAX];
    char str[128] = {'\0'};
    ISP_DRV_CAM* pDrv = NULL;
    Mutex::Autolock lock(this->m_bufctrl_lock);

    if ( _pdo_ != this->id() ) {
        CAM_FUNC_ERR("dma channel error ");
        ret = eCmd_Fail;
        goto EXIT;
    }

    switch(CAM_BUF_CTRL::m_SttState[this->m_hwModule]){
        case CAM_BUF_CTRL::eState_forPDOnly:
            return eCmd_Dummy;
            break;
        default:
            break;
    }

    subSample = CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAMCTL_R1_CAMCTL_SW_PASS1_DONE,CAMCTL_DOWN_SAMPLE_PERIOD) + 1;

    this->m_Queue_deque_ptr.sendCmd(QueueMgr<MUINTPTR>::eCmd_getsize  ,(MUINTPTR)&_size,0);
    if (_size == 0) {
        CAM_FUNC_ERR("no enque record, plz enque first");
        return eCmd_Fail;
    }

    this->m_Queue.sendCmd(QueueMgr<ST_BUF_INFO>::eCmd_getsize,(MUINTPTR)&_size,0);
    if(_size <1){
        CAM_FUNC_ERR("queue depth is empty for deque\n");
        ret = eCmd_Fail;
        goto EXIT;
    }

    this->m_Queue_deque_ptr.sendCmd(QueueMgr<MUINTPTR>::eCmd_front  ,(MUINTPTR)&pDrv,0);

    //dbg log
    pDrv->getCurObjInfo(&this->m_hwModule,&cq,&page);

    pDrv->getDeviceInfo(_GET_SOF_CNT,(MUINT8 *) &_sof_idx);

    //fbc reg dbg log
    this->FBC_STATUS(pDrv->getPhyObj());

    this->m_Queue.sendCmd(QueueMgr<ST_BUF_INFO>::eCmd_front  ,(MUINTPTR)&_buf,0);

    switch(CAM_BUF_CTRL::m_CamState[this->m_hwModule]){
        case CAM_BUF_CTRL::eCamState_runnable:
        case CAM_BUF_CTRL::eCamState_suspend:   //this timing is for twin deque is happaned after dynamic twin suspend ready
            //
            this->Header_Parser(_buf.header.va_addr, buf_info,_header_data,str, pDrv->m_HWmasterModule);
                        //
            if( _buf.image.mem_info.pa_addr != buf_info.u4BufPA[ePlane_1st]){
                CAM_FUNC_ERR("%s:PA in header is mismatch with PA in list [0x%" PRIXPTR "_0x%" PRIXPTR "]\n",this->name_Str(),\
                    buf_info.u4BufPA[ePlane_1st],_buf.image.mem_info.pa_addr);

                ret = eCmd_Fail;
                this->PipeCheck();
                goto EXIT;
            }
            break;
        case CAM_BUF_CTRL::eCamState_suspending:
            ret = eCmd_Suspending_Pass;
            //do not check header,dram on header's address is garbage!
            //need to pop all requests in driver when suspending for the drop method in iopipe.

            //Flush FH
            this->Header_Parser(_buf.header.va_addr, buf_info,_header_data,str, pDrv->m_HWmasterModule);

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
    buf_data_parsing(buf_info,ePlane_1st,_buf);

    //
    this->m_Queue.sendCmd(QueueMgr<ST_BUF_INFO>::eCmd_pop  ,0,0);
    this->m_Queue_deque_ptr.sendCmd(QueueMgr<MUINTPTR>::eCmd_pop  ,0,0);
    this->m_buf_cnt--;

    //ion handle: buffer
    this->ion_handle_lock(pDrv,MFALSE,_buf.image.mem_info.memID,_buf.header.memID);


EXIT:

    CAM_FUNC_DBG("%s:(0x%x_0x%x),PA(0x%" PRIXPTR "_0x%" PRIXPTR "),FH_VA(0x%" PRIXPTR "),size(0x%x),cur sof(%d),%s\n",\
                this->name_Str(),\
                cq,page,\
                _buf.image.mem_info.pa_addr ,\
                _buf.header.pa_addr ,\
                _buf.header.va_addr ,\
                _buf.image.mem_info.size ,\
                _sof_idx,\
                str);
    return ret;
}


void BUF_CTRL_FLKO::FBC_STATUS(IspDrv* ptr)
{
    CAM_FUNC_DBG("BUF_CTRL_FLKO::FBC_STATUS: 0x%x_0x%x\n",CAM_READ_REG(ptr,FBC_R1_FBC_FLKO_R1_CTL1),CAM_READ_REG(ptr,FBC_R1_FBC_FLKO_R1_CTL2));
}

void BUF_CTRL_FLKO::Header_Parser(MUINTPTR fh_va,NSImageio::NSIspio::BufInfo& buf_info,MUINT32* header_data ,char* pStr, ISP_HW_MODULE masterModule)
{
    Header_FLKO  fh_flko;
    MUINT64 tmp;

    //
    fh_flko.Header_Deque(fh_va,(void*)header_data);
    fh_flko.Header_Flash(fh_va);
    //
    buf_info.u4BufPA[ePlane_1st] = header_data[Header_FLKO::E_IMG_PA];
    buf_info.m_num = header_data[Header_FLKO::E_Magic];

    buf_info.i4TimeStamp_us = header_data[Header_FLKO::E_TimeStamp_LSB];
    tmp = header_data[Header_FLKO::E_TimeStamp_MSB];
    buf_info.i4TimeStamp_us += (tmp << 32);

    this->m_pTimeStamp->TimeStamp_mapping(&buf_info.i4TimeStamp_us,&buf_info.i4TimeStamp_us_B,(_isp_dma_enum_)this->id(), masterModule);

    buf_info.eIQlv = (E_CamIQLevel)header_data[Header_FLKO::E_IQ_LEVEL];
    buf_info.sensorId = header_data[Header_FLKO::E_SENSOR_ID];

    //
    if(pStr){
        snprintf(pStr, sizeof(*pStr), "enq_sof:0x%x,mag:0x%x,timestamp:%" PRIu64 ",eIQlv:0x%x",\
            header_data[Header_FLKO::E_ENQUE_SOF],\
            header_data[Header_FLKO::E_Magic], \
            buf_info.i4TimeStamp_us, \
            buf_info.eIQlv);
    }
}

E_BC_STATUS BUF_CTRL_FLKO::waitBufReady(CAM_STATE_NOTIFY *pNotify)
{
    #define _LOOP_  (5)
    E_BC_STATUS ret = eCmd_Pass;
    E_ISP_CAM_CQ cq;
    MUINT32 page;

    Header_FLKO fh_flko;
    MUINT32 _pa;
    ST_BUF_INFO _buf;
    MUINT32 _header_data[E_HEADER_MAX];

    ISP_WAIT_IRQ_ST irq;
    MUINT32 loopCnt = _LOOP_;
    char str[128] = {'\0'};

    MUINT32 FbcCnt, sw_buf_cnt;
    MUINT32 subSample;
    MUINT32 reWaitCnt = 1;

    this->m_pDrv = (ISP_DRV_CAM*)this->m_pIspDrv;

    //dbg log
    this->m_pDrv->getCurObjInfo(&this->m_hwModule,&cq,&page);
    if ( 0xffffffff == this->id() ) {
        CAM_FUNC_ERR("dma channel error \n");
        return eCmd_Fail;
    }

    //fbc reg dbg log
    this->FBC_STATUS(this->m_pDrv->getPhyObj());

    switch (CAM_BUF_CTRL::m_CamState[this->m_hwModule]) {
        //theoratically, it no need to reference normalpipe's status under sttpipe.
        //but due to suspend/resume is only implemented in noramlpipe, so sttpipe need to use suspend/resume status.
        case CAM_BUF_CTRL::eCamState_suspending:
            return eCmd_Suspending_Pass;
            break;
        default:
            break;
    }

    subSample = CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAMCTL_R1_CAMCTL_SW_PASS1_DONE,CAMCTL_DOWN_SAMPLE_PERIOD) + 1;

    irq.Clear = ISP_IRQ_CLEAR_WAIT;
    irq.UserKey = 0;
    irq.St_type = DMA_INT;
    irq.Status = (MUINT32)FLKO_DONE_ST;

    do{
        //irq.Timeout = ((1000 + (this->m_fps - 1)) / this->m_fps) * subSample;
        //irq.Timeout *= 4; //amplify 3 times is for the case of enque latch timing
        irq.Timeout = this->estimateTimeout(subSample);

        this->m_bufctrl_lock.lock();
        sw_buf_cnt = this->m_buf_cnt;
        this->m_bufctrl_lock.unlock();

        if(sw_buf_cnt != 0){

            //
            this->m_Queue.sendCmd(QueueMgr<ST_BUF_INFO>::eCmd_at,0,(MUINTPTR)&_buf);
            IspFunction_B::fbcPhyLock[this->m_hwModule][this->id()].lock();
            FbcCnt = CAM_READ_BITS(this->m_pDrv->getPhyObj(),FBC_R1_FBC_FLKO_R1_CTL2,FBC_FLKO_R1_FBC_CNT);
            IspFunction_B::fbcPhyLock[this->m_hwModule][this->id()].unlock();
            fh_flko.Header_Deque(_buf.header.va_addr,(void*)_header_data);
            _pa = _header_data[Header_FLKO::E_IMG_PA];
            if((FbcCnt < sw_buf_cnt) && (_pa != 0)){
                ret = eCmd_Pass;
                goto EXIT;
            }
            else{
                if(loopCnt == _LOOP_)
                    snprintf(str, sizeof(str), "dma:%s start wait:[enque record:0x%x_0x%x],",this->name_Str(),sw_buf_cnt,FbcCnt);
                else{
                    char _tmp[16];
                    snprintf(_tmp, sizeof(_tmp), "[%d]time,",((_LOOP_-loopCnt)+1));
                    strncat(str,_tmp, strlen(_tmp));
                }
            }
        }
        else{
            if(loopCnt == _LOOP_){
                snprintf(str, sizeof(str), "dma:%s bWaitBufReady = 1,wait for sw enque[0x%x],timeout:%d x %d ms,[1] ",this->name_Str(),sw_buf_cnt,irq.Timeout,loopCnt);
            }
            else{//save how many times of wait_irq before sw enque
                char _tmp[16];
                snprintf(_tmp, sizeof(_tmp), "[%d],",((_LOOP_-loopCnt)+1)+1);
                strncat(str,_tmp, strlen(_tmp));
            }
        }

        //
        if (pNotify) {
            if(pNotify->fpNotifyState(CAM_ENTER_WAIT, pNotify->Obj) == NOTIFY_FAIL){
                CAM_FUNC_ERR("dma:%s NotifyCB fail",this->name_Str());
                ret = eCmd_Fail;
            }
        }

        if(this->m_pDrv->waitIrq(&irq) == MFALSE) {
            ret = eCmd_Fail;
        } else {
            ret = eCmd_Pass;
        }

        // if user doesn't deque for a while, in next deque, waitBufReady will be passed directly by checking frame header.
        // under this condtion, the done status in kernel is not cleared, and this may cause pa mismatch when dequeHwBuf.
        // therefore, check fh after waitIrq pass. if pa != 0, wait again.
        if(ret == eCmd_Pass){
            if(reWaitCnt > 0){
                this->m_Queue.sendCmd(QueueMgr<ST_BUF_INFO>::eCmd_at, 0, (MUINTPTR)&_buf);
                fh_flko.Header_Deque(_buf.header.va_addr, (void*)_header_data);
                _pa = _header_data[Header_FLKO::E_IMG_PA];
                if(_pa == 0){
                    reWaitCnt = 0;
                    continue;
                }
                else{
                    reWaitCnt = 0;
                }
            }
        }
        else{
            reWaitCnt = 0;
        }

        if (pNotify) {
            if(pNotify->fpNotifyState(CAM_EXIT_WAIT, pNotify->Obj) == NOTIFY_FAIL){
                CAM_FUNC_ERR("dma:%s NotifyCB fail",this->name_Str());
                ret = eCmd_Fail;
            }
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
        if(sw_buf_cnt != 0){
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


E_BC_STATUS BUF_CTRL_TSFSO::waitBufReady(CAM_STATE_NOTIFY *pNotify)
{
#define _LOOP_  (5)
    E_BC_STATUS ret = eCmd_Pass;
    E_ISP_CAM_CQ cq;
    MUINT32 page;

    Header_TSFSO fh_tsfso;
    MUINT32 _pa;
    ST_BUF_INFO _buf;
    MUINT32 _header_data[E_HEADER_MAX];

    ISP_WAIT_IRQ_ST irq;
    MUINT32 loopCnt = _LOOP_;
    char str[128] = {'\0'};

    MUINT32 FbcCnt, sw_buf_cnt;
    MUINT32 subSample;
    MUINT32 reWaitCnt = 1;


    this->m_pDrv = (ISP_DRV_CAM*)this->m_pIspDrv;

    //dbg log
    this->m_pDrv->getCurObjInfo(&this->m_hwModule,&cq,&page);
    if ( 0xffffffff == this->id() ) {
        CAM_FUNC_ERR("dma channel error \n");
        return eCmd_Fail;
    }

    //fbc reg dbg log
    this->FBC_STATUS(this->m_pDrv->getPhyObj());

    switch (CAM_BUF_CTRL::m_CamState[this->m_hwModule]) {
        //theoratically, it no need to reference normalpipe's status under sttpipe.
        //but due to suspend/resume is only implemented in noramlpipe, so sttpipe need to use suspend/resume status.
        case CAM_BUF_CTRL::eCamState_suspending:
            return eCmd_Suspending_Pass;
            break;
        default:
            break;
    }

    subSample = CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAMCTL_R1_CAMCTL_SW_PASS1_DONE,CAMCTL_DOWN_SAMPLE_PERIOD) + 1;

    irq.Clear = ISP_IRQ_CLEAR_WAIT;
    irq.UserKey = 0;
    irq.St_type = DMA_INT;
    irq.Status = (MUINT32)TSFSO_DONE_ST;

    do{
        //irq.Timeout = ((1000 + (this->m_fps - 1)) / this->m_fps) * subSample;
        //irq.Timeout *= 4; //amplify 3 times is for the case of enque latch timing
        irq.Timeout = this->estimateTimeout(subSample);

        this->m_bufctrl_lock.lock();
        sw_buf_cnt = this->m_buf_cnt;
        this->m_bufctrl_lock.unlock();

        if(sw_buf_cnt != 0){

            //
            this->m_Queue.sendCmd(QueueMgr<ST_BUF_INFO>::eCmd_at,0,(MUINTPTR)&_buf);
            IspFunction_B::fbcPhyLock[this->m_hwModule][this->id()].lock();
            FbcCnt = CAM_READ_BITS(this->m_pDrv->getPhyObj(),FBC_R1_FBC_TSFSO_R1_CTL2,FBC_TSFSO_R1_FBC_CNT);
            IspFunction_B::fbcPhyLock[this->m_hwModule][this->id()].unlock();
            fh_tsfso.Header_Deque(_buf.header.va_addr,(void*)_header_data);
            _pa = _header_data[Header_TSFSO::E_IMG_PA];
            if((FbcCnt< sw_buf_cnt) && (_pa != 0)){
                ret = eCmd_Pass;
                goto EXIT;
            }
            else{
                if(loopCnt == _LOOP_)
                    snprintf(str, sizeof(str), "dma:%s start wait:[enque record:0x%x_0x%x],",this->name_Str(),sw_buf_cnt,FbcCnt);
                else{
                    char _tmp[16];
                    snprintf(_tmp, sizeof(_tmp), "[%d]time,",((_LOOP_-loopCnt)+1));
                    strncat(str,_tmp, strlen(_tmp));
                }
            }
        }
        else{
            if(loopCnt == _LOOP_){
                snprintf(str, sizeof(str), "dma:%s bWaitBufReady = 1,wait for sw enque[0x%x],timeout:%d x %d ms,[1] ",this->name_Str(),sw_buf_cnt,irq.Timeout,loopCnt);
            }
            else{//save how many times of wait_irq before sw enque
                char _tmp[16];
                snprintf(_tmp, sizeof(_tmp), "[%d],",((_LOOP_-loopCnt)+1)+1);
                strncat(str,_tmp, strlen(_tmp));
            }
        }

        //
        if (pNotify) {
            if(pNotify->fpNotifyState(CAM_ENTER_WAIT, pNotify->Obj) == NOTIFY_FAIL){
                CAM_FUNC_ERR("dma:%s NotifyCB fail",this->name_Str());
                ret = eCmd_Fail;
            }
        }

        if(this->m_pDrv->waitIrq(&irq) == MFALSE) {
            ret = eCmd_Fail;
        } else {
            ret = eCmd_Pass;
        }

        // if user doesn't deque for a while, in next deque, waitBufReady will be passed directly by checking frame header.
        // under this condtion, the done status in kernel is not cleared, and this may cause pa mismatch when dequeHwBuf.
        // therefore, check fh after waitIrq pass. if pa != 0, wait again.
        if(ret == eCmd_Pass){
            if(reWaitCnt > 0){
                this->m_Queue.sendCmd(QueueMgr<ST_BUF_INFO>::eCmd_at, 0, (MUINTPTR)&_buf);
                fh_tsfso.Header_Deque(_buf.header.va_addr, (void*)_header_data);
                _pa = _header_data[Header_TSFSO::E_IMG_PA];
                if(_pa == 0){
                    reWaitCnt = 0;
                    continue;
                }
                else{
                    reWaitCnt = 0;
                }
            }
        }
        else{
            reWaitCnt = 0;
        }

        if (pNotify) {
            if(pNotify->fpNotifyState(CAM_EXIT_WAIT, pNotify->Obj) == NOTIFY_FAIL){
                CAM_FUNC_ERR("dma:%s NotifyCB fail",this->name_Str());
                ret = eCmd_Fail;
            }
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
        if(sw_buf_cnt != 0){
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

void BUF_CTRL_TSFSO::FBC_STATUS(IspDrv* ptr)
{
    CAM_FUNC_DBG("BUF_CTRL_TSFSO::FBC_STATUS: 0x%x_0x%x\n",CAM_READ_REG(ptr,FBC_R1_FBC_TSFSO_R1_CTL1),CAM_READ_REG(ptr,FBC_R1_FBC_TSFSO_R1_CTL2));
}

void BUF_CTRL_TSFSO::Header_Parser(MUINTPTR fh_va,NSImageio::NSIspio::BufInfo& buf_info,MUINT32* header_data ,char* pStr, ISP_HW_MODULE masterModule)
{
    Header_TSFSO  fh_tsfso;
    MUINT64 tmp;

    //
    fh_tsfso.Header_Deque(fh_va,(void*)header_data);
    fh_tsfso.Header_Flash(fh_va);

    //
    buf_info.u4BufPA[ePlane_1st] = header_data[Header_TSFSO::E_IMG_PA];
    buf_info.m_num = header_data[Header_TSFSO::E_Magic];

    buf_info.i4TimeStamp_us = header_data[Header_TSFSO::E_TimeStamp_LSB];
    tmp = header_data[Header_TSFSO::E_TimeStamp_MSB];
    buf_info.i4TimeStamp_us += (tmp << 32);

    this->m_pTimeStamp->TimeStamp_mapping(&buf_info.i4TimeStamp_us,&buf_info.i4TimeStamp_us_B,(_isp_dma_enum_)this->id(), masterModule);

    buf_info.eIQlv = (E_CamIQLevel)header_data[Header_TSFSO::E_IQ_LEVEL];
    buf_info.sensorId = header_data[Header_TSFSO::E_SENSOR_ID];

    //
    if(pStr){
        sprintf(pStr, "enq_sof:0x%x,mag:0x%x,timestamp:%" PRIu64 ",eIQlv:0x%x",\
            header_data[Header_TSFSO::E_ENQUE_SOF],\
            header_data[Header_TSFSO::E_Magic], \
            buf_info.i4TimeStamp_us, \
            buf_info.eIQlv);
    }
}

MINT32 BUF_CTRL_RAWI::_config(void)
{
    E_ISP_CAM_CQ cq;
    MUINT32 page;
    this->m_pDrv = (ISP_DRV_CAM*)this->m_pIspDrv;
    //dbg log
    this->m_pDrv->getCurObjInfo(&this->m_hwModule,&cq,&page);

    CAM_FUNC_DBG("%s::_config+ cq:0x%x,page:0x%x\n",this->name_Str(),cq,page);

    this->m_Queue.init();

    //reset buf cnt
    this->m_buf_cnt = 0;
    return 0;
}

E_BC_STATUS BUF_CTRL_RAWI::enqueueHwBuf(stISP_BUF_INFO& buf_info, MBOOL bImdMode)
{
    ST_BUF_INFO* ptr = NULL;
    E_ISP_CAM_CQ cq;
    MUINT32 page;
    MUINT32 _sof_idx = this->m_hwModule; // assign hwModule as input argument
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

    //
    if(buf_info.bReplace == MTRUE){
        ptr = &buf_info.Replace;
    }
    else{
        ptr = &buf_info.u_op.enque.at(ePlane_1st);
    }

#if 0 //RAWI can enque the same BA
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
#endif
    //ion handle: buffer
    this->ion_handle_lock(this->m_pDrv,MTRUE,ptr->image.mem_info.memID,ptr->header.memID);

    //
    this->m_Queue.sendCmd(QueueMgr<ST_BUF_INFO>::eCmd_push,(MUINTPTR)ptr,0);
    this->m_buf_cnt++;

    //
    this->m_Queue_deque_ptr.sendCmd(QueueMgr<MUINTPTR>::eCmd_push,(MUINTPTR)&this->m_pDrv,0);

    //
    CAM_FUNC_INF("%s:(0x%x_0x%x),PA(0x%" PRIXPTR "_0x%" PRIXPTR "),FH_VA(0x%" PRIXPTR "),size(0x%x/0x%x),enque_sof(%d)\n",
        this->name_Str(),
        cq, page,
        ptr->image.mem_info.pa_addr,
        ptr->header.pa_addr,
        ptr->header.va_addr,
        ptr->image.mem_info.size,
        ptr->header.size,
       _sof_idx);

    //
    return eCmd_Pass;
}


E_BC_STATUS BUF_CTRL_RAWI::dequeueHwBuf(NSImageio::NSIspio::BufInfo& buf_info )
{
    E_BC_STATUS ret = eCmd_Pass;
    E_ISP_CAM_CQ cq = ISP_DRV_CQ_NONE;
    MUINT32 page = 0;
    MUINT32 _size;
    MUINT32 _sof_idx = this->m_hwModule; // assign hwModule as input argument
    ST_BUF_INFO _buf;
    ISP_DRV_CAM* pDrv = NULL;

    Mutex::Autolock lock(this->m_bufctrl_lock);

    if ( _rawi_ != this->id() ) {
        CAM_FUNC_ERR("dma channel error ");
        return eCmd_Fail;
    }

    this->m_Queue_deque_ptr.sendCmd(QueueMgr<MUINTPTR>::eCmd_getsize  ,(MUINTPTR)&_size,0);
    if (_size == 0) {
        CAM_FUNC_ERR("no enque record, plz enque first");
        return eCmd_Fail;
    }

    this->m_Queue.sendCmd(QueueMgr<ST_BUF_INFO>::eCmd_getsize,(MUINTPTR)&_size,0);
    if(_size <1){
        CAM_FUNC_ERR("queue depth is empty for deque\n");
        return eCmd_Fail;
    }

    this->m_Queue_deque_ptr.sendCmd(QueueMgr<MUINTPTR>::eCmd_front  ,(MUINTPTR)&pDrv,0);

    //dbg log
    pDrv->getCurObjInfo(&this->m_hwModule,&cq,&page);

    pDrv->getDeviceInfo(_GET_SOF_CNT,(MUINT8 *) &_sof_idx);

    this->m_Queue.sendCmd(QueueMgr<ST_BUF_INFO>::eCmd_front  ,(MUINTPTR)&_buf,0);

    switch(CAM_BUF_CTRL::m_CamState[this->m_hwModule]){
        case CAM_BUF_CTRL::eCamState_runnable:
        case CAM_BUF_CTRL::eCamState_suspend:
            break;
        case CAM_BUF_CTRL::eCamState_suspending:
            ret = eCmd_Suspending_Pass;

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
    buf_data_parsing(buf_info,ePlane_1st,_buf);

    //
    this->m_Queue.sendCmd(QueueMgr<ST_BUF_INFO>::eCmd_pop  ,0,0);
    this->m_Queue_deque_ptr.sendCmd(QueueMgr<MUINTPTR>::eCmd_pop  ,0,0);
    this->m_buf_cnt--;

    //ion handle: buffer
    this->ion_handle_lock(pDrv,MFALSE,_buf.image.mem_info.memID,_buf.header.memID);

EXIT:
    CAM_FUNC_INF("%s:(0x%x_0x%x),PA(0x%" PRIXPTR "_0x%" PRIXPTR "),FH_VA(0x%" PRIXPTR "),size(0x%x/0x%x),cur sof(%d)\n",
        this->name_Str(),
        cq, page,
        _buf.image.mem_info.pa_addr,
        _buf.header.pa_addr,
        _buf.header.va_addr,
        _buf.image.mem_info.size,
        _buf.header.size,
        _sof_idx);

    return ret;
}

MINT32 BUF_CTRL_UFDI::_config(void)
{
    E_ISP_CAM_CQ cq;
    MUINT32 page;
    this->m_pDrv = (ISP_DRV_CAM*)this->m_pIspDrv;
    //dbg log
    this->m_pDrv->getCurObjInfo(&this->m_hwModule,&cq,&page);

    CAM_FUNC_DBG("%s::_config+ cq:0x%x,page:0x%x\n",this->name_Str(),cq,page);

    this->m_Queue.init();

    //reset buf cnt
    this->m_buf_cnt = 0;
    return 0;
}


E_BC_STATUS BUF_CTRL_UFDI::enqueueHwBuf(stISP_BUF_INFO& buf_info, MBOOL bImdMode)
{
    ST_BUF_INFO* ptr = NULL;
    E_ISP_CAM_CQ cq;
    MUINT32 page;
    MUINT32 _sof_idx = this->m_hwModule; // assign hwModule as input argument
    MUINT32 _size;
    ST_BUF_INFO _buf;

    this->m_pDrv = (ISP_DRV_CAM*)this->m_pIspDrv;
    (void)bImdMode;

    Mutex::Autolock lock(this->m_bufctrl_lock);

    //dbg log
    this->m_pDrv->getCurObjInfo(&this->m_hwModule,&cq,&page);

    if ( _ufdi_r2 != this->id() ) {
        CAM_FUNC_ERR("dma channel error\n");
        return eCmd_Fail;
    }

    //
    this->m_pDrv->getDeviceInfo(_GET_SOF_CNT,(MUINT8 *) &_sof_idx);

    //
    if(buf_info.bReplace == MTRUE){
        ptr = &buf_info.Replace;
    }
    else{
        ptr = &buf_info.u_op.enque.at(ePlane_1st);
    }

#if 0 //RAWI can enque the same BA
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
#endif
    //ion handle: buffer
    this->ion_handle_lock(this->m_pDrv,MTRUE,ptr->image.mem_info.memID,ptr->header.memID);

    //
    this->m_Queue.sendCmd(QueueMgr<ST_BUF_INFO>::eCmd_push,(MUINTPTR)ptr,0);
    this->m_buf_cnt++;

    //
    this->m_Queue_deque_ptr.sendCmd(QueueMgr<MUINTPTR>::eCmd_push,(MUINTPTR)&this->m_pDrv,0);

    ///
    CAM_FUNC_INF("%s:(0x%x_0x%x),PA(0x%" PRIXPTR "_0x%" PRIXPTR "),FH_VA(0x%" PRIXPTR "),size(0x%x/0x%x),enque_sof(%d)\n",
        this->name_Str(),
        cq, page,
        ptr->image.mem_info.pa_addr,
        ptr->header.pa_addr,
        ptr->header.va_addr,
        ptr->image.mem_info.size,
        ptr->header.size,
        _sof_idx);

    //
    return eCmd_Pass;
}

E_BC_STATUS BUF_CTRL_UFDI::dequeueHwBuf(NSImageio::NSIspio::BufInfo& buf_info )
{
    E_BC_STATUS ret = eCmd_Pass;
    E_ISP_CAM_CQ cq = ISP_DRV_CQ_NONE;
    MUINT32 page = 0;
    MUINT32 _size;
    MUINT32 _sof_idx = this->m_hwModule; // assign hwModule as input argument
    ST_BUF_INFO _buf;
    ISP_DRV_CAM* pDrv = NULL;

    Mutex::Autolock lock(this->m_bufctrl_lock);

    if ( 0xffffffff == this->id() ) {
        CAM_FUNC_ERR("dma channel error ");
        return eCmd_Fail;
    }

    this->m_Queue_deque_ptr.sendCmd(QueueMgr<MUINTPTR>::eCmd_getsize  ,(MUINTPTR)&_size,0);
    if (_size == 0) {
        CAM_FUNC_ERR("no enque record, plz enque first");
        return eCmd_Fail;
    }

    this->m_Queue.sendCmd(QueueMgr<ST_BUF_INFO>::eCmd_getsize,(MUINTPTR)&_size,0);
    if(_size <1){
        CAM_FUNC_ERR("queue depth is empty for deque\n");
        return eCmd_Fail;
    }

    this->m_Queue_deque_ptr.sendCmd(QueueMgr<MUINTPTR>::eCmd_front  ,(MUINTPTR)&pDrv,0);

    //dbg log
    pDrv->getCurObjInfo(&this->m_hwModule,&cq,&page);

    pDrv->getDeviceInfo(_GET_SOF_CNT,(MUINT8 *) &_sof_idx);

    this->m_Queue.sendCmd(QueueMgr<ST_BUF_INFO>::eCmd_front  ,(MUINTPTR)&_buf,0);

    switch(CAM_BUF_CTRL::m_CamState[this->m_hwModule]){
        case CAM_BUF_CTRL::eCamState_runnable:
        case CAM_BUF_CTRL::eCamState_suspend:
            break;
        case CAM_BUF_CTRL::eCamState_suspending:
            ret = eCmd_Suspending_Pass;

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
    buf_data_parsing(buf_info,ePlane_2nd,_buf);

    //
    this->m_Queue.sendCmd(QueueMgr<ST_BUF_INFO>::eCmd_pop  ,0,0);
    this->m_Queue_deque_ptr.sendCmd(QueueMgr<MUINTPTR>::eCmd_pop  ,0,0);
    this->m_buf_cnt--;

    //ion handle: buffer
    this->ion_handle_lock(pDrv,MFALSE,_buf.image.mem_info.memID,_buf.header.memID);


EXIT:
    CAM_FUNC_INF("%s:(0x%x_0x%x),PA(0x%" PRIXPTR "_0x%" PRIXPTR "),FH_VA(0x%" PRIXPTR "),size(0x%x/0x%x),cur sof(%d)\n",
        this->name_Str(),
        cq, page,
        _buf.image.mem_info.pa_addr,
        _buf.header.pa_addr,
        _buf.header.va_addr,
        _buf.image.mem_info.size,
        _buf.header.size,
        _sof_idx);

    return ret;
}

void Header_IMGO::Header_Enque(MUINT32 Tag,ISP_DRV_CAM* DrvPtr,MUINT32 value)
{
    switch(Tag){
        case Header_IMGO::E_Magic:
            CAM_WRITE_REG(DrvPtr,IMGO_R1_IMGO_FH_SPARE_2,value);
            break;
        case Header_IMGO::E_ENQUE_SOF:
            CAM_WRITE_REG(DrvPtr,IMGO_R1_IMGO_FH_SPARE_3,value);
            break;
        case Header_IMGO::E_RAW_TYPE:
            CAM_WRITE_REG(DrvPtr,IMGO_R1_IMGO_FH_SPARE_4,value);
            break;
        case Header_IMGO::E_PIX_ID:
            CAM_WRITE_REG(DrvPtr,IMGO_R1_IMGO_FH_SPARE_5,value);
            break;
        case Header_IMGO::E_FMT:
            CAM_WRITE_REG(DrvPtr,IMGO_R1_IMGO_FH_SPARE_6,value);
            break;
        case Header_IMGO::E_ENQUE_CNT:
            CAM_WRITE_REG(DrvPtr,IMGO_R1_IMGO_FH_SPARE_7,value);
            break;
        case Header_IMGO::E_SRC_SIZE:
            CAM_WRITE_REG(DrvPtr,IMGO_R1_IMGO_FH_SPARE_8,value);
            break;
        case Header_IMGO::E_IQ_LEVEL:
            CAM_WRITE_BITS(DrvPtr,IMGO_R1_IMGO_FH_SPARE_9,E_IQ_LEVEL,value);
            break;
        case Header_IMGO::E_CROP_START:
            CAM_WRITE_REG(DrvPtr,IMGO_R1_IMGO_FH_SPARE_10,value);
            break;
        case Header_IMGO::E_CROP_SIZE:
            CAM_WRITE_REG(DrvPtr,IMGO_R1_IMGO_FH_SPARE_11,value);
            break;
        case Header_IMGO::E_IMG_PA:
            CAM_WRITE_REG(DrvPtr,IMGO_R1_IMGO_FH_SPARE_12,value);
            break;
        case Header_IMGO::E_IS_UFE_FMT:
            CAM_WRITE_BITS(DrvPtr,IMGO_R1_IMGO_FH_SPARE_9,E_IS_UFE_FMT,value);
            break;
        default:
        case Header_IMGO::E_TimeStamp_LSB:
        case Header_IMGO::E_TimeStamp_MSB:
            CAM_FUNC_ERR("unsupported tag:0x%x\n",Tag);
            break;
    }
}

void Header_IMGO::Header_Deque(MUINTPTR FH_VA,void* pOut)
{
    void* ptr = (void*)FH_VA;
    memcpy(pOut,ptr,sizeof(MUINT32)*Header_IMGO::E_MAX);
}

void Header_IMGO::Header_Flash(MUINTPTR FH_VA)
{
    void* ptr = (void*)FH_VA;
    memset(ptr,0,sizeof(MUINT32)*Header_IMGO::E_MAX);

}


MUINT32 Header_IMGO::GetRegInfo(MUINT32 Tag,IspDrv* DrvPtr)
{
    switch(Tag){
        case Header_IMGO::E_Magic:
            return CAM_READ_REG(DrvPtr,IMGO_R1_IMGO_FH_SPARE_2);
            break;
        case Header_IMGO::E_ENQUE_SOF:
            return CAM_READ_REG(DrvPtr,IMGO_R1_IMGO_FH_SPARE_3);
            break;
        case Header_IMGO::E_RAW_TYPE:
            return CAM_READ_REG(DrvPtr,IMGO_R1_IMGO_FH_SPARE_4);
            break;
        case Header_IMGO::E_PIX_ID:
            return CAM_READ_REG(DrvPtr,IMGO_R1_IMGO_FH_SPARE_5);
            break;
        case Header_IMGO::E_FMT:
            return CAM_READ_REG(DrvPtr,IMGO_R1_IMGO_FH_SPARE_6);
            break;
        case Header_IMGO::E_ENQUE_CNT:
            return CAM_READ_REG(DrvPtr,IMGO_R1_IMGO_FH_SPARE_7);
            break;
        case Header_IMGO::E_SRC_SIZE:
            return CAM_READ_REG(DrvPtr,IMGO_R1_IMGO_FH_SPARE_8);
            break;
        case Header_IMGO::E_IQ_LEVEL:
            return CAM_READ_BITS(DrvPtr,IMGO_R1_IMGO_FH_SPARE_9,E_IQ_LEVEL);
            break;
        case Header_IMGO::E_CROP_START:
            return CAM_READ_REG(DrvPtr,IMGO_R1_IMGO_FH_SPARE_10);
            break;
        case Header_IMGO::E_CROP_SIZE:
            return CAM_READ_REG(DrvPtr,IMGO_R1_IMGO_FH_SPARE_11);
            break;
        case Header_IMGO::E_IMG_PA:
            return CAM_READ_REG(DrvPtr,IMGO_R1_IMGO_FH_SPARE_12);
            break;
        case Header_IMGO::E_IS_UFE_FMT:
            return CAM_READ_BITS(DrvPtr,IMGO_R1_IMGO_FH_SPARE_9,E_IS_UFE_FMT);
            break;
        default:
        case Header_IMGO::E_TimeStamp_LSB:
        case Header_IMGO::E_TimeStamp_MSB:
            CAM_FUNC_ERR("unsupported tag:0x%x\n",Tag);
            break;
    }
    return 0;
}

Mutex  Header_RRZO::m_tagAddrLock;

void Header_RRZO::Header_Enque(MUINT32 Tag,ISP_DRV_CAM* DrvPtr,MUINT32 value)
{
    switch(Tag){
        case Header_RRZO::E_Magic:
            CAM_WRITE_REG(DrvPtr,RRZO_R1_RRZO_FH_SPARE_2,value);
            break;
        case Header_RRZO::E_ENQUE_SOF:
            CAM_WRITE_REG(DrvPtr,RRZO_R1_RRZO_FH_SPARE_3,value);
            break;
        case Header_RRZO::E_PIX_ID:
            CAM_WRITE_REG(DrvPtr,RRZO_R1_RRZO_FH_SPARE_4,value);
            break;
        case Header_RRZO::E_FMT:
            CAM_WRITE_REG(DrvPtr,RRZO_R1_RRZO_FH_SPARE_5,value);
            break;
        case Header_RRZO::E_ENQUE_CNT:
            CAM_WRITE_REG(DrvPtr,RRZO_R1_RRZO_FH_SPARE_6,value);
            break;
        case Header_RRZO::E_IS_UFG_FMT:
            CAM_WRITE_REG(DrvPtr,RRZO_R1_RRZO_FH_SPARE_7,value);
            break;
        case Header_RRZO::E_IQ_LEVEL:
            CAM_WRITE_REG(DrvPtr,RRZO_R1_RRZO_FH_SPARE_8,value);
            break;
        case Header_RRZO::E_RRZ_CRP_START:
            CAM_WRITE_REG(DrvPtr,RRZO_R1_RRZO_FH_SPARE_9,value);
            break;
        case Header_RRZO::E_RRZ_CRP_SIZE:
            CAM_WRITE_REG(DrvPtr,RRZO_R1_RRZO_FH_SPARE_10,value);
            break;
        case Header_RRZO::E_RRZ_DST_SIZE:
            CAM_WRITE_REG(DrvPtr,RRZO_R1_RRZO_FH_SPARE_11,value);
            break;
        case Header_RRZO::E_IMG_PA:
            CAM_WRITE_REG(DrvPtr,RRZO_R1_RRZO_FH_SPARE_12,value);
            break;
        default:
        case Header_RRZO::E_TimeStamp_LSB:
        case Header_RRZO::E_TimeStamp_MSB:
            CAM_FUNC_ERR("unsupported tag:0x%x\n",Tag);
            break;
    }
}


void Header_RRZO::Header_Deque(MUINTPTR FH_VA,void* pOut)
{
    void* ptr = (void*)FH_VA;
    memcpy(pOut,ptr,sizeof(MUINT32)*Header_RRZO::E_MAX);
}

void Header_RRZO::Header_Flash(MUINTPTR FH_VA)
{
    void* ptr = (void*)FH_VA;
    memset(ptr,0,sizeof(MUINT32)*Header_RRZO::E_MAX);
}

MUINT32 Header_RRZO::GetRegInfo(MUINT32 Tag,IspDrv* DrvPtr)
{
    switch(Tag){
        case Header_RRZO::E_Magic:
            return CAM_READ_REG(DrvPtr,RRZO_R1_RRZO_FH_SPARE_2);
            break;
        case Header_RRZO::E_ENQUE_SOF:
            return CAM_READ_REG(DrvPtr,RRZO_R1_RRZO_FH_SPARE_3);
            break;
        case Header_RRZO::E_PIX_ID:
            return CAM_READ_REG(DrvPtr,RRZO_R1_RRZO_FH_SPARE_4);
            break;
        case Header_RRZO::E_FMT:
            return CAM_READ_REG(DrvPtr,RRZO_R1_RRZO_FH_SPARE_5);
            break;
        case Header_RRZO::E_ENQUE_CNT:
            return CAM_READ_REG(DrvPtr,RRZO_R1_RRZO_FH_SPARE_6);
            break;
        case Header_RRZO::E_IS_UFG_FMT:
            return CAM_READ_REG(DrvPtr,RRZO_R1_RRZO_FH_SPARE_7);
            break;
        case Header_RRZO::E_IQ_LEVEL:
            return CAM_READ_REG(DrvPtr,RRZO_R1_RRZO_FH_SPARE_8);
            break;
        case Header_RRZO::E_RRZ_CRP_START:
            return CAM_READ_REG(DrvPtr,RRZO_R1_RRZO_FH_SPARE_9);
            break;
        case Header_RRZO::E_RRZ_CRP_SIZE:
            return CAM_READ_REG(DrvPtr,RRZO_R1_RRZO_FH_SPARE_10);
            break;
        case Header_RRZO::E_RRZ_DST_SIZE:
            return CAM_READ_REG(DrvPtr,RRZO_R1_RRZO_FH_SPARE_11);
            break;
        case Header_RRZO::E_IMG_PA:
            return CAM_READ_REG(DrvPtr,RRZO_R1_RRZO_FH_SPARE_12);
            break;
        default:
        case Header_RRZO::E_TimeStamp_LSB:
        case Header_RRZO::E_TimeStamp_MSB:
            CAM_FUNC_ERR("unsupported tag:0x%x\n",Tag);
            break;
    }
    return 0;
}

MUINT32 Header_RRZO::GetTagAddr(MUINT32 Tag)
{
    Mutex::Autolock lock(m_tagAddrLock);

    ISP_DRV_CAM* ptr = (ISP_DRV_CAM*)ISP_DRV_CAM::createInstance(CAM_A,ISP_DRV_CQ_THRE3,0,"TagAddr");
    MUINT32 address = 0;
    switch(Tag){
        case Header_RRZO::E_Magic:
            address = CAM_REG_ADDR(ptr,RRZO_R1_RRZO_FH_SPARE_2);
            break;
        case Header_RRZO::E_ENQUE_SOF:
            address = CAM_REG_ADDR(ptr,RRZO_R1_RRZO_FH_SPARE_3);
            break;
        case Header_RRZO::E_PIX_ID:
            address = CAM_REG_ADDR(ptr,RRZO_R1_RRZO_FH_SPARE_4);
            break;
        case Header_RRZO::E_FMT:
            address = CAM_REG_ADDR(ptr,RRZO_R1_RRZO_FH_SPARE_5);
            break;
        case Header_RRZO::E_ENQUE_CNT:
            address = CAM_REG_ADDR(ptr,RRZO_R1_RRZO_FH_SPARE_6);
            break;
        case Header_RRZO::E_IS_UFG_FMT:
            address = CAM_REG_ADDR(ptr,RRZO_R1_RRZO_FH_SPARE_7);
            break;
        case Header_RRZO::E_IQ_LEVEL:
            address = CAM_REG_ADDR(ptr,RRZO_R1_RRZO_FH_SPARE_8);
            break;
        case Header_RRZO::E_RRZ_CRP_START:
            address = CAM_REG_ADDR(ptr,RRZO_R1_RRZO_FH_SPARE_9);
            break;
        case Header_RRZO::E_RRZ_CRP_SIZE:
            address = CAM_REG_ADDR(ptr,RRZO_R1_RRZO_FH_SPARE_10);
            break;
        case Header_RRZO::E_RRZ_DST_SIZE:
            address = CAM_REG_ADDR(ptr,RRZO_R1_RRZO_FH_SPARE_11);
            break;
        case Header_RRZO::E_IMG_PA:
            address = CAM_REG_ADDR(ptr,RRZO_R1_RRZO_FH_SPARE_12);
            break;
        default:
        case Header_RRZO::E_TimeStamp_LSB:
        case Header_RRZO::E_TimeStamp_MSB:
            break;
    }
    ptr->destroyInstance();
    return address;

}

void Header_UFEO::Header_Enque(MUINT32 Tag,ISP_DRV_CAM* DrvPtr,MUINT32 value)
{
    switch(Tag){
        case Header_UFEO::E_Magic:
            CAM_WRITE_REG(DrvPtr,UFEO_R1_UFEO_FH_SPARE_2,value);
            break;
        case Header_UFEO::E_ENQUE_SOF:
            CAM_WRITE_REG(DrvPtr,UFEO_R1_UFEO_FH_SPARE_3,value);
            break;
        case Header_UFEO::E_ENQUE_CNT:
            CAM_WRITE_REG(DrvPtr,UFEO_R1_UFEO_FH_SPARE_4,value);
            break;
        case Header_UFEO::E_FMT:
            CAM_WRITE_REG(DrvPtr,UFEO_R1_UFEO_FH_SPARE_5,value);
            break;
        case Header_UFEO::E_UFEO_OFFST_ADDR:
            CAM_WRITE_REG(DrvPtr,UFEO_R1_UFEO_FH_SPARE_6,value);
            break;
        case Header_UFEO::E_UFEO_XSIZE:
            CAM_WRITE_REG(DrvPtr,UFEO_R1_UFEO_FH_SPARE_7,value);
            break;
        case Header_UFEO::E_IMGO_OFFST_ADDR:
            CAM_WRITE_REG(DrvPtr,UFEO_R1_UFEO_FH_SPARE_8,value);
            break;
        case Header_UFEO::E_IQ_LEVEL:
            CAM_WRITE_REG(DrvPtr,UFEO_R1_UFEO_FH_SPARE_9,value);
            break;
        case Header_UFEO::E_IMG_PA:
            CAM_WRITE_REG(DrvPtr,UFEO_R1_UFEO_FH_SPARE_12,value);
            break;
        default:
        case Header_UFEO::E_TimeStamp_LSB:
        case Header_UFEO::E_TimeStamp_MSB:
            CAM_FUNC_ERR("unsupported tag:0x%x\n",Tag);
            break;
    }
}


void Header_UFEO::Header_Deque(MUINTPTR FH_VA,void* pOut)
{
    void* ptr = (void*)FH_VA;
    memcpy(pOut,ptr,sizeof(MUINT32)*Header_UFEO::E_MAX);
}

void Header_UFEO::Header_Flash(MUINTPTR FH_VA)
{
    void* ptr = (void*)FH_VA;
    memset(ptr,0,sizeof(MUINT32)*Header_UFEO::E_MAX);

}

MUINT32 Header_UFEO::GetRegInfo(MUINT32 Tag,IspDrv* DrvPtr)
{
    switch(Tag){
        case Header_UFEO::E_Magic:
            return CAM_READ_REG(DrvPtr,UFEO_R1_UFEO_FH_SPARE_2);
            break;
        case Header_UFEO::E_ENQUE_SOF:
            return CAM_READ_REG(DrvPtr,UFEO_R1_UFEO_FH_SPARE_3);
            break;
        case Header_UFEO::E_ENQUE_CNT:
            return CAM_READ_REG(DrvPtr,UFEO_R1_UFEO_FH_SPARE_4);
            break;
        case Header_UFEO::E_FMT:
            return CAM_READ_REG(DrvPtr,UFEO_R1_UFEO_FH_SPARE_5);
            break;
        case Header_UFEO::E_UFEO_OFFST_ADDR:
            return CAM_READ_REG(DrvPtr,UFEO_R1_UFEO_FH_SPARE_6);
            break;
        case Header_UFEO::E_UFEO_XSIZE:
            return CAM_READ_REG(DrvPtr,UFEO_R1_UFEO_FH_SPARE_7);
            break;
        case Header_UFEO::E_IMGO_OFFST_ADDR:
            return CAM_READ_REG(DrvPtr,UFEO_R1_UFEO_FH_SPARE_8);
            break;
        case Header_UFEO::E_IQ_LEVEL:
            return CAM_READ_REG(DrvPtr,UFEO_R1_UFEO_FH_SPARE_9);
            break;
        case Header_UFEO::E_IMG_PA:
            return CAM_READ_REG(DrvPtr,UFEO_R1_UFEO_FH_SPARE_12);
            break;
        default:
        case Header_UFEO::E_TimeStamp_LSB:
        case Header_UFEO::E_TimeStamp_MSB:
            CAM_FUNC_ERR("unsupported tag:0x%x\n",Tag);
            break;
    }
    return 0;
}



void Header_UFGO::Header_Enque(MUINT32 Tag,ISP_DRV_CAM* DrvPtr,MUINT32 value)
{
    switch(Tag){
        case Header_UFGO::E_Magic:
            CAM_WRITE_REG(DrvPtr,UFGO_R1_UFGO_FH_SPARE_2,value);
            break;
        case Header_UFGO::E_ENQUE_SOF:
            CAM_WRITE_REG(DrvPtr,UFGO_R1_UFGO_FH_SPARE_3,value);
            break;
        case Header_UFGO::E_ENQUE_CNT:
            CAM_WRITE_REG(DrvPtr,UFGO_R1_UFGO_FH_SPARE_4,value);
            break;
        case Header_UFGO::E_FMT:
            CAM_WRITE_REG(DrvPtr,UFGO_R1_UFGO_FH_SPARE_5,value);
            break;
        case Header_UFGO::E_UFGO_OFFST_ADDR:
            CAM_WRITE_REG(DrvPtr,UFGO_R1_UFGO_FH_SPARE_6,value);
            break;
        case Header_UFGO::E_UFGO_XSIZE:
            CAM_WRITE_REG(DrvPtr,UFGO_R1_UFGO_FH_SPARE_7,value);
            break;
        case Header_UFGO::E_RRZO_OFFST_ADDR:
            CAM_WRITE_REG(DrvPtr,UFGO_R1_UFGO_FH_SPARE_8,value);
            break;
        case Header_UFGO::E_IQ_LEVEL:
            CAM_WRITE_REG(DrvPtr,UFGO_R1_UFGO_FH_SPARE_9,value);
            break;
        case Header_UFGO::E_IMG_PA:
            CAM_WRITE_REG(DrvPtr,UFGO_R1_UFGO_FH_SPARE_12,value);
            break;
        default:
        case Header_UFGO::E_TimeStamp_LSB:
        case Header_UFGO::E_TimeStamp_MSB:
            CAM_FUNC_ERR("unsupported tag:0x%x\n",Tag);
            break;
    }
}


void Header_UFGO::Header_Deque(MUINTPTR FH_VA,void* pOut)
{
    void* ptr = (void*)FH_VA;
    memcpy(pOut,ptr,sizeof(MUINT32)*Header_UFGO::E_MAX);
}

void Header_UFGO::Header_Flash(MUINTPTR FH_VA)
{
    void* ptr = (void*)FH_VA;
    memset(ptr,0,sizeof(MUINT32)*Header_UFGO::E_MAX);

}

MUINT32 Header_UFGO::GetRegInfo(MUINT32 Tag,IspDrv* DrvPtr)
{
    switch(Tag){
        case Header_UFGO::E_Magic:
            return CAM_READ_REG(DrvPtr,UFGO_R1_UFGO_FH_SPARE_2);
            break;
        case Header_UFGO::E_ENQUE_SOF:
            return CAM_READ_REG(DrvPtr,UFGO_R1_UFGO_FH_SPARE_3);
            break;
        case Header_UFGO::E_ENQUE_CNT:
            return CAM_READ_REG(DrvPtr,UFGO_R1_UFGO_FH_SPARE_4);
            break;
        case Header_UFGO::E_FMT:
            return CAM_READ_REG(DrvPtr,UFGO_R1_UFGO_FH_SPARE_5);
            break;
        case Header_UFGO::E_UFGO_OFFST_ADDR:
            return CAM_READ_REG(DrvPtr,UFGO_R1_UFGO_FH_SPARE_6);
            break;
        case Header_UFGO::E_UFGO_XSIZE:
            return CAM_READ_REG(DrvPtr,UFGO_R1_UFGO_FH_SPARE_7);
            break;
        case Header_UFGO::E_RRZO_OFFST_ADDR:
            return CAM_READ_REG(DrvPtr,UFGO_R1_UFGO_FH_SPARE_8);
            break;
        case Header_UFGO::E_IQ_LEVEL:
            return CAM_READ_REG(DrvPtr,UFGO_R1_UFGO_FH_SPARE_9);
            break;
        case Header_UFGO::E_IMG_PA:
            return CAM_READ_REG(DrvPtr,UFGO_R1_UFGO_FH_SPARE_12);
            break;
        default:
        case Header_UFGO::E_TimeStamp_LSB:
        case Header_UFGO::E_TimeStamp_MSB:
            CAM_FUNC_ERR("unsupported tag:0x%x\n",Tag);
            break;
    }
    return 0;
}

void Header_AAO::Header_Enque(MUINT32 Tag,ISP_DRV_CAM* DrvPtr,MUINT32 value)
{
    switch(Tag){
        case Header_AAO::E_Magic:
            CAM_WRITE_REG(DrvPtr,AAO_R1_AAO_FH_SPARE_2,value);
            break;
        case Header_AAO::E_IQ_LEVEL:
            CAM_WRITE_REG(DrvPtr,AAO_R1_AAO_FH_SPARE_3,value);
            break;
        case Header_AAO::E_SENSOR_ID:
            CAM_WRITE_REG(DrvPtr,AAO_R1_AAO_FH_SPARE_4,value);
            break;
        case Header_AAO::E_ENQUE_SOF:
            ((MUINT32*)DrvPtr)[Tag - Header_AAO::E_ENQUE_SOF] = value;
            break;
        case Header_AAO::E_IMG_PA:
           ((MUINT32*)DrvPtr)[Tag - Header_AAO::E_ENQUE_SOF] = value;
            break;
        default:
        case Header_AAO::E_TimeStamp_LSB:
        case Header_AAO::E_TimeStamp_MSB:
            CAM_FUNC_ERR("unsupported tag:0x%x\n",Tag);
            break;
    }
}


void Header_AAO::Header_Deque(MUINTPTR FH_VA,void* pOut)
{
    void* ptr = (void*)FH_VA;
    memcpy(pOut,ptr,sizeof(MUINT32)*Header_AAO::E_MAX);
}

void Header_AAO::Header_Flash(MUINTPTR FH_VA)
{
    void* ptr = (void*)FH_VA;
    memset(ptr,0,sizeof(MUINT32)*Header_AAO::E_MAX);

}
void Header_AFO::Header_Enque(MUINT32 Tag,ISP_DRV_CAM* DrvPtr,MUINT32 value)
{
    switch(Tag){
        case Header_AFO::E_Magic:
            CAM_WRITE_REG(DrvPtr,AFO_R1_AFO_FH_SPARE_2,value);
            break;
        case Header_AFO::E_STRIDE:
            CAM_WRITE_REG(DrvPtr,AFO_R1_AFO_FH_SPARE_3,value);
            break;
        case Header_AFO::E_IQ_LEVEL:
            CAM_WRITE_REG(DrvPtr,AFO_R1_AFO_FH_SPARE_4,value);
            break;
        case Header_AFO::E_SENSOR_ID:
            CAM_WRITE_REG(DrvPtr,AFO_R1_AFO_FH_SPARE_5,value);
            break;
        case Header_AFO::E_ENQUE_SOF:
            ((MUINT32*)DrvPtr)[Tag- Header_AFO::E_ENQUE_SOF] = value;
            break;
        case Header_AFO::E_IMG_PA:
            ((MUINT32*)DrvPtr)[Tag-Header_AFO::E_ENQUE_SOF] = value;
            break;
        default:
        case Header_AFO::E_TimeStamp_LSB:
        case Header_AFO::E_TimeStamp_MSB:
            CAM_FUNC_ERR("unsupported tag:0x%x\n",Tag);
            break;
    }
}


void Header_AFO::Header_Deque(MUINTPTR FH_VA,void* pOut)
{
    void* ptr = (void*)FH_VA;
    memcpy(pOut,ptr,sizeof(MUINT32)*Header_AFO::E_MAX);
}

void Header_AFO::Header_Flash(MUINTPTR FH_VA)
{
    void* ptr = (void*)FH_VA;
    memset(ptr,0,sizeof(MUINT32)*Header_AFO::E_MAX);

}


void Header_LCSO::Header_Enque(MUINT32 Tag,ISP_DRV_CAM* DrvPtr,MUINT32 value)
{
    switch(Tag){
        case Header_LCSO::E_Magic:
            CAM_WRITE_REG(DrvPtr,LCESO_R1_LCESO_FH_SPARE_2,value);
            break;
        case Header_LCSO::E_ENQUE_SOF:
            CAM_WRITE_REG(DrvPtr,LCESO_R1_LCESO_FH_SPARE_3,value);
            break;
        case Header_LCSO::E_ENQUE_CNT:
            CAM_WRITE_REG(DrvPtr,LCESO_R1_LCESO_FH_SPARE_4,value);
            break;
        case Header_LCSO::E_IQ_LEVEL:
            CAM_WRITE_REG(DrvPtr,LCESO_R1_LCESO_FH_SPARE_5,value);
            break;
        case Header_LCSO::E_IMG_PA:
            CAM_WRITE_REG(DrvPtr,LCESO_R1_LCESO_FH_SPARE_12,value);
            break;
        default:
        case Header_LCSO::E_TimeStamp_LSB:
        case Header_LCSO::E_TimeStamp_MSB:
            CAM_FUNC_ERR("unsupported tag:0x%x\n",Tag);
            break;
    }
}


void Header_LCSO::Header_Deque(MUINTPTR FH_VA,void* pOut)
{
    void* ptr = (void*)FH_VA;
    memcpy(pOut,ptr,sizeof(MUINT32)*Header_LCSO::E_MAX);
}

void Header_LCSO::Header_Flash(MUINTPTR FH_VA)
{
    void* ptr = (void*)FH_VA;
    memset(ptr,0,sizeof(MUINT32)*Header_LCSO::E_MAX);

}

void Header_PDO::Header_Enque(MUINT32 Tag,ISP_DRV_CAM* DrvPtr,MUINT32 value)
{
    switch(Tag){
        case Header_PDO::E_Magic:
            CAM_WRITE_REG(DrvPtr,PDO_R1_PDO_FH_SPARE_2,value);
            break;
        case Header_PDO::E_STRIDE:
            CAM_WRITE_REG(DrvPtr,PDO_R1_PDO_FH_SPARE_3,value);
            break;
        case Header_PDO::E_SENSOR_ID:
            CAM_WRITE_REG(DrvPtr,PDO_R1_PDO_FH_SPARE_4,value);
            break;
        case Header_PDO::E_IMG_PA_OFSET:
            CAM_WRITE_REG(DrvPtr,PDO_R1_PDO_FH_SPARE_8,value);
            break;
        case Header_PDO::E_IQ_LEVEL:
            CAM_WRITE_REG(DrvPtr,PDO_R1_PDO_FH_SPARE_9,value);
            break;
        case Header_PDO::E_ENQUE_SOF:
           ((MUINT32*)DrvPtr)[Tag- Header_PDO::E_ENQUE_SOF] = value;
            break;
        case Header_PDO::E_IMG_PA:
           ((MUINT32*)DrvPtr)[Tag- Header_PDO::E_ENQUE_SOF] = value;
            break;
        default:
        case Header_PDO::E_TimeStamp_LSB:
        case Header_PDO::E_TimeStamp_MSB:
            CAM_FUNC_ERR("unsupported tag:0x%x\n",Tag);
            break;
    }
}


void Header_PDO::Header_Deque(MUINTPTR FH_VA,void* pOut)
{
    void* ptr = (void*)FH_VA;
    memcpy(pOut,ptr,sizeof(MUINT32)*Header_PDO::E_MAX);
}

void Header_PDO::Header_Flash(MUINTPTR FH_VA)
{
    void* ptr = (void*)FH_VA;
    memset(ptr,0,sizeof(MUINT32)*Header_PDO::E_MAX);

}

void Header_TSFSO::Header_Enque(MUINT32 Tag,ISP_DRV_CAM* DrvPtr,MUINT32 value)
{
    switch(Tag){
        case Header_TSFSO::E_Magic:
            CAM_WRITE_REG(DrvPtr,TSFSO_R1_TSFSO_FH_SPARE_2,value);
            break;
        case Header_TSFSO::E_IQ_LEVEL:
            CAM_WRITE_REG(DrvPtr,TSFSO_R1_TSFSO_FH_SPARE_3,value);
            break;
        case Header_TSFSO::E_SENSOR_ID:
            CAM_WRITE_REG(DrvPtr,TSFSO_R1_TSFSO_FH_SPARE_4,value);
            break;
        case Header_TSFSO::E_ENQUE_SOF:
            ((MUINT32*)DrvPtr)[Tag- Header_TSFSO::E_ENQUE_SOF] = value;
            break;
        case Header_TSFSO::E_IMG_PA:
           ((MUINT32*)DrvPtr)[Tag- Header_TSFSO::E_ENQUE_SOF] = value;
            break;
        default:
        case Header_TSFSO::E_TimeStamp_LSB:
        case Header_TSFSO::E_TimeStamp_MSB:
            CAM_FUNC_ERR("unsupported tag:0x%x\n",Tag);
            break;
    }
}


void Header_TSFSO::Header_Deque(MUINTPTR FH_VA,void* pOut)
{
    void* ptr = (void*)FH_VA;
    memcpy(pOut,ptr,sizeof(MUINT32)*Header_TSFSO::E_MAX);
}

void Header_TSFSO::Header_Flash(MUINTPTR FH_VA)
{
    void* ptr = (void*)FH_VA;
    memset(ptr,0,sizeof(MUINT32)*Header_TSFSO::E_MAX);

}

void Header_LMVO::Header_Enque(MUINT32 Tag,ISP_DRV_CAM* DrvPtr,MUINT32 value)
{
    switch(Tag){
        case Header_LMVO::E_Magic:
            CAM_WRITE_REG(DrvPtr,LMVO_R1_LMVO_FH_SPARE_2,value);
            break;
        case Header_LMVO::E_ENQUE_SOF:
            CAM_WRITE_REG(DrvPtr,LMVO_R1_LMVO_FH_SPARE_3,value);
            break;
        case Header_LMVO::E_ENQUE_CNT:
            CAM_WRITE_REG(DrvPtr,LMVO_R1_LMVO_FH_SPARE_4,value);
            break;
        case Header_LMVO::E_IQ_LEVEL:
            CAM_WRITE_REG(DrvPtr,LMVO_R1_LMVO_FH_SPARE_5,value);
            break;
        case Header_LMVO::E_IMG_PA:
            CAM_WRITE_REG(DrvPtr,LMVO_R1_LMVO_FH_SPARE_12,value);
            break;
        default:
        case Header_LMVO::E_TimeStamp_LSB:
        case Header_LMVO::E_TimeStamp_MSB:
            CAM_FUNC_ERR("unsupported tag:0x%x\n",Tag);
            break;
    }
}

void Header_LMVO::Header_Deque(MUINTPTR FH_VA,void* pOut)
{
    void* ptr = (void*)FH_VA;
    memcpy(pOut,ptr,sizeof(MUINT32)*Header_LMVO::E_MAX);
}

void Header_LMVO::Header_Flash(MUINTPTR FH_VA)
{
    void* ptr = (void*)FH_VA;
    memset(ptr,0,sizeof(MUINT32)*Header_LMVO::E_MAX);

}

void Header_FLKO::Header_Enque(MUINT32 Tag,ISP_DRV_CAM* DrvPtr,MUINT32 value)
{
    switch(Tag){
        case Header_FLKO::E_Magic:  //magic number using CQ0
            CAM_WRITE_REG(DrvPtr,FLKO_R1_FLKO_FH_SPARE_2,value);
            break;
        case Header_FLKO::E_IQ_LEVEL:
            CAM_WRITE_REG(DrvPtr,FLKO_R1_FLKO_FH_SPARE_3,value);
            break;
        case Header_FLKO::E_SENSOR_ID:
            CAM_WRITE_REG(DrvPtr,FLKO_R1_FLKO_FH_SPARE_4,value);
            break;
        case Header_FLKO::E_ENQUE_SOF:
           ((MUINT32*)DrvPtr)[Tag- Header_FLKO::E_ENQUE_SOF] = value;
            break;
        case Header_FLKO::E_IMG_PA:
           ((MUINT32*)DrvPtr)[Tag- Header_FLKO::E_ENQUE_SOF] = value;
            break;
        default:
        case Header_FLKO::E_TimeStamp_LSB:
        case Header_FLKO::E_TimeStamp_MSB:
            CAM_FUNC_ERR("unsupported tag:0x%x\n",Tag);
            break;
    }
}


void Header_FLKO::Header_Deque(MUINTPTR FH_VA,void* pOut)
{
    void* ptr = (void*)FH_VA;
    memcpy(pOut,ptr,sizeof(MUINT32)*Header_FLKO::E_MAX);
}


void Header_FLKO::Header_Flash(MUINTPTR FH_VA)
{
    void* ptr = (void*)FH_VA;
    memset(ptr,0,sizeof(MUINT32)*Header_FLKO::E_MAX);

}


void Header_RSSO::Header_Enque(MUINT32 Tag,ISP_DRV_CAM* DrvPtr,MUINT32 value)
{
    switch(Tag){
        case Header_RSSO::E_Magic:
            CAM_WRITE_REG(DrvPtr,RSSO_R1_RSSO_FH_SPARE_2,value);
            break;
        case Header_RSSO::E_ENQUE_SOF:
            CAM_WRITE_REG(DrvPtr,RSSO_R1_RSSO_FH_SPARE_3,value);
            break;
        case Header_RSSO::E_ENQUE_CNT:
            CAM_WRITE_REG(DrvPtr,RSSO_R1_RSSO_FH_SPARE_4,value);
            break;
        case Header_RSSO::E_DST_SIZE:
            CAM_WRITE_REG(DrvPtr,RSSO_R1_RSSO_FH_SPARE_5,value);
            break;
        case Header_RSSO::E_IQ_LEVEL:
            CAM_WRITE_REG(DrvPtr,RSSO_R1_RSSO_FH_SPARE_6,value);
            break;
        case Header_RSSO::E_IMG_PA:
            CAM_WRITE_REG(DrvPtr,RSSO_R1_RSSO_FH_SPARE_12,value);
            break;
        default:
        case Header_RSSO::E_TimeStamp_LSB:
        case Header_RSSO::E_TimeStamp_MSB:
            CAM_FUNC_ERR("unsupported tag:0x%x\n",Tag);
            break;
    }
}


void Header_RSSO::Header_Deque(MUINTPTR FH_VA,void* pOut)
{
    void* ptr = (void*)FH_VA;
    memcpy(pOut,ptr,sizeof(MUINT32)*Header_RSSO::E_MAX);
}

void Header_RSSO::Header_Flash(MUINTPTR FH_VA)
{
    void* ptr = (void*)FH_VA;
    memset(ptr,0,sizeof(MUINT32)*Header_RSSO::E_MAX);

}

void Header_YUVO::Header_Enque(MUINT32 Tag,ISP_DRV_CAM* DrvPtr,MUINT32 value)
{
    switch(Tag){
        case Header_YUVO::E_Magic:
            CAM_WRITE_REG(DrvPtr,YUVO_R1_YUVO_FH_SPARE_2,value);
            break;
        case Header_YUVO::E_ENQUE_SOF:
            CAM_WRITE_REG(DrvPtr,YUVO_R1_YUVO_FH_SPARE_3,value);
            break;
        case Header_YUVO::E_PLANE_EN:
            CAM_WRITE_REG(DrvPtr,YUVO_R1_YUVO_FH_SPARE_4,value);
            break;
        case Header_YUVO::E_SW_FMT:
            CAM_WRITE_REG(DrvPtr,YUVO_R1_YUVO_FH_SPARE_5,value);
            break;
        case Header_YUVO::E_FMT:
            CAM_WRITE_REG(DrvPtr,YUVO_R1_YUVO_FH_SPARE_6,value);
            break;
        case Header_YUVO::E_CROP_START:
            CAM_WRITE_REG(DrvPtr,YUVO_R1_YUVO_FH_SPARE_7,value);
            break;
        case Header_YUVO::E_CROP_SIZE:
            CAM_WRITE_REG(DrvPtr,YUVO_R1_YUVO_FH_SPARE_8,value);
            break;
        case Header_YUVO::E_ENQUE_CNT:
            CAM_WRITE_REG(DrvPtr,YUVO_R1_YUVO_FH_SPARE_9,value);
            break;
        case Header_YUVO::E_SRC_SIZE:
            CAM_WRITE_REG(DrvPtr,YUVO_R1_YUVO_FH_SPARE_10,value);
            break;
        case Header_YUVO::E_IQ_LEVEL:
            CAM_WRITE_REG(DrvPtr,YUVO_R1_YUVO_FH_SPARE_11,value);
            break;
        case Header_YUVO::E_IMG_PA:
            CAM_WRITE_REG(DrvPtr,YUVO_R1_YUVO_FH_SPARE_12,value);
            break;
        default:
        case Header_YUVO::E_TimeStamp_LSB:
        case Header_YUVO::E_TimeStamp_MSB:
            CAM_FUNC_ERR("unsupported tag:0x%x\n",Tag);
            break;
    }
}

void Header_YUVO::Header_Deque(MUINTPTR FH_VA,void* pOut)
{
    void* ptr = (void*)FH_VA;
    memcpy(pOut,ptr,sizeof(MUINT32)*Header_YUVO::E_MAX);
}

void Header_YUVO::Header_Flash(MUINTPTR FH_VA)
{
    void* ptr = (void*)FH_VA;
    memset(ptr,0,sizeof(MUINT32)*Header_YUVO::E_MAX);

}

MUINT32 Header_YUVO::GetRegInfo(MUINT32 Tag,IspDrv* DrvPtr)
{
    switch(Tag){
        case Header_YUVO::E_Magic:
            return CAM_READ_REG(DrvPtr,YUVO_R1_YUVO_FH_SPARE_2);
            break;
        case Header_YUVO::E_ENQUE_SOF:
            return CAM_READ_REG(DrvPtr,YUVO_R1_YUVO_FH_SPARE_3);
            break;
        case Header_YUVO::E_PLANE_EN:
            return CAM_READ_REG(DrvPtr,YUVO_R1_YUVO_FH_SPARE_4);
            break;
        case Header_YUVO::E_SW_FMT:
            return CAM_READ_REG(DrvPtr,YUVO_R1_YUVO_FH_SPARE_5);
            break;
        case Header_YUVO::E_FMT:
            return CAM_READ_REG(DrvPtr,YUVO_R1_YUVO_FH_SPARE_6);
            break;
        case Header_YUVO::E_CROP_START:
            return CAM_READ_REG(DrvPtr,YUVO_R1_YUVO_FH_SPARE_7);
            break;
        case Header_YUVO::E_CROP_SIZE:
            return CAM_READ_REG(DrvPtr,YUVO_R1_YUVO_FH_SPARE_8);
            break;
        case Header_YUVO::E_ENQUE_CNT:
            return CAM_READ_REG(DrvPtr,YUVO_R1_YUVO_FH_SPARE_9);
            break;
        case Header_YUVO::E_SRC_SIZE:
            return CAM_READ_REG(DrvPtr,YUVO_R1_YUVO_FH_SPARE_10);
            break;
        case Header_YUVO::E_IQ_LEVEL:
            return CAM_READ_REG(DrvPtr,YUVO_R1_YUVO_FH_SPARE_11);
            break;
        case Header_YUVO::E_IMG_PA:
            return CAM_READ_REG(DrvPtr,YUVO_R1_YUVO_FH_SPARE_12);
            break;
        default:
        case Header_YUVO::E_TimeStamp_LSB:
        case Header_YUVO::E_TimeStamp_MSB:
            CAM_FUNC_ERR("unsupported tag:0x%x\n",Tag);
            break;
    }
    return 0;
}

void Header_YUVBO::Header_Enque(MUINT32 Tag,ISP_DRV_CAM* DrvPtr,MUINT32 value)
{
    switch(Tag){
        case Header_YUVBO::E_Magic:
            CAM_WRITE_REG(DrvPtr,YUVBO_R1_YUVBO_FH_SPARE_2,value);
            break;
        case Header_YUVBO::E_ENQUE_SOF:
            CAM_WRITE_REG(DrvPtr,YUVBO_R1_YUVBO_FH_SPARE_3,value);
            break;
        case Header_YUVBO::E_PIX_ID:
            CAM_WRITE_REG(DrvPtr,YUVBO_R1_YUVBO_FH_SPARE_5,value);
            break;
        case Header_YUVBO::E_FMT:
            CAM_WRITE_REG(DrvPtr,YUVBO_R1_YUVBO_FH_SPARE_6,value);
            break;
        case Header_YUVBO::E_CROP_START:
            CAM_WRITE_REG(DrvPtr,YUVBO_R1_YUVBO_FH_SPARE_7,value);
            break;
        case Header_YUVBO::E_CROP_SIZE:
            CAM_WRITE_REG(DrvPtr,YUVBO_R1_YUVBO_FH_SPARE_8,value);
            break;
        case Header_YUVBO::E_ENQUE_CNT:
            CAM_WRITE_REG(DrvPtr,YUVBO_R1_YUVBO_FH_SPARE_9,value);
            break;
        case Header_YUVBO::E_SRC_SIZE:
            CAM_WRITE_REG(DrvPtr,YUVBO_R1_YUVBO_FH_SPARE_10,value);
            break;
        case Header_YUVBO::E_IQ_LEVEL:
            CAM_WRITE_REG(DrvPtr,YUVBO_R1_YUVBO_FH_SPARE_11,value);
            break;
        case Header_YUVBO::E_IMG_PA:
            CAM_WRITE_REG(DrvPtr,YUVBO_R1_YUVBO_FH_SPARE_12,value);
            break;
        default:
        case Header_YUVBO::E_TimeStamp_LSB:
        case Header_YUVBO::E_TimeStamp_MSB:
            CAM_FUNC_ERR("unsupported tag:0x%x\n",Tag);
            break;
    }
}

void Header_YUVBO::Header_Deque(MUINTPTR FH_VA,void* pOut)
{
    void* ptr = (void*)FH_VA;
    memcpy(pOut,ptr,sizeof(MUINT32)*Header_YUVBO::E_MAX);
}

void Header_YUVBO::Header_Flash(MUINTPTR FH_VA)
{
    void* ptr = (void*)FH_VA;
    memset(ptr,0,sizeof(MUINT32)*Header_YUVBO::E_MAX);

}

MUINT32 Header_YUVBO::GetRegInfo(MUINT32 Tag,IspDrv* DrvPtr)
{
    switch(Tag){
        case Header_YUVBO::E_Magic:
            return CAM_READ_REG(DrvPtr,YUVBO_R1_YUVBO_FH_SPARE_2);
            break;
        case Header_YUVBO::E_ENQUE_SOF:
            return CAM_READ_REG(DrvPtr,YUVBO_R1_YUVBO_FH_SPARE_3);
            break;
        case Header_YUVBO::E_PIX_ID:
            return CAM_READ_REG(DrvPtr,YUVBO_R1_YUVBO_FH_SPARE_5);
            break;
        case Header_YUVBO::E_FMT:
            return CAM_READ_REG(DrvPtr,YUVBO_R1_YUVBO_FH_SPARE_6);
            break;
        case Header_YUVBO::E_CROP_START:
            return CAM_READ_REG(DrvPtr,YUVBO_R1_YUVBO_FH_SPARE_7);
            break;
        case Header_YUVBO::E_CROP_SIZE:
            return CAM_READ_REG(DrvPtr,YUVBO_R1_YUVBO_FH_SPARE_8);
            break;
        case Header_YUVBO::E_ENQUE_CNT:
            return CAM_READ_REG(DrvPtr,YUVBO_R1_YUVBO_FH_SPARE_9);
            break;
        case Header_YUVBO::E_SRC_SIZE:
            return CAM_READ_REG(DrvPtr,YUVBO_R1_YUVBO_FH_SPARE_10);
            break;
        case Header_YUVBO::E_IQ_LEVEL:
            return CAM_READ_REG(DrvPtr,YUVBO_R1_YUVBO_FH_SPARE_11);
            break;
        case Header_YUVBO::E_IMG_PA:
            return CAM_READ_REG(DrvPtr,YUVBO_R1_YUVBO_FH_SPARE_12);
            break;
        default:
        case Header_YUVBO::E_TimeStamp_LSB:
        case Header_YUVBO::E_TimeStamp_MSB:
            CAM_FUNC_ERR("unsupported tag:0x%x\n",Tag);
            break;
    }
    return 0;
}

void Header_YUVCO::Header_Enque(MUINT32 Tag,ISP_DRV_CAM* DrvPtr,MUINT32 value)
{
    switch(Tag){
        case Header_YUVCO::E_Magic:
            CAM_WRITE_REG(DrvPtr,YUVCO_R1_YUVCO_FH_SPARE_2,value);
            break;
        case Header_YUVCO::E_ENQUE_SOF:
            CAM_WRITE_REG(DrvPtr,YUVCO_R1_YUVCO_FH_SPARE_3,value);
            break;
        case Header_YUVCO::E_PIX_ID:
            CAM_WRITE_REG(DrvPtr,YUVCO_R1_YUVCO_FH_SPARE_5,value);
            break;
        case Header_YUVCO::E_FMT:
            CAM_WRITE_REG(DrvPtr,YUVCO_R1_YUVCO_FH_SPARE_6,value);
            break;
        case Header_YUVCO::E_CROP_START:
            CAM_WRITE_REG(DrvPtr,YUVCO_R1_YUVCO_FH_SPARE_7,value);
            break;
        case Header_YUVCO::E_CROP_SIZE:
            CAM_WRITE_REG(DrvPtr,YUVCO_R1_YUVCO_FH_SPARE_8,value);
            break;
        case Header_YUVCO::E_ENQUE_CNT:
            CAM_WRITE_REG(DrvPtr,YUVCO_R1_YUVCO_FH_SPARE_9,value);
            break;
        case Header_YUVCO::E_SRC_SIZE:
            CAM_WRITE_REG(DrvPtr,YUVCO_R1_YUVCO_FH_SPARE_10,value);
            break;
        case Header_YUVCO::E_IQ_LEVEL:
            CAM_WRITE_REG(DrvPtr,YUVCO_R1_YUVCO_FH_SPARE_11,value);
            break;
        case Header_YUVCO::E_IMG_PA:
            CAM_WRITE_REG(DrvPtr,YUVCO_R1_YUVCO_FH_SPARE_12,value);
            break;
        default:
        case Header_YUVCO::E_TimeStamp_LSB:
        case Header_YUVCO::E_TimeStamp_MSB:
            CAM_FUNC_ERR("unsupported tag:0x%x\n",Tag);
            break;
    }
}

void Header_YUVCO::Header_Deque(MUINTPTR FH_VA,void* pOut)
{
    void* ptr = (void*)FH_VA;
    memcpy(pOut,ptr,sizeof(MUINT32)*Header_YUVCO::E_MAX);
}

void Header_YUVCO::Header_Flash(MUINTPTR FH_VA)
{
    void* ptr = (void*)FH_VA;
    memset(ptr,0,sizeof(MUINT32)*Header_YUVCO::E_MAX);

}

MUINT32 Header_YUVCO::GetRegInfo(MUINT32 Tag,IspDrv* DrvPtr)
{
    switch(Tag){
        case Header_YUVCO::E_Magic:
            return CAM_READ_REG(DrvPtr,YUVCO_R1_YUVCO_FH_SPARE_2);
            break;
        case Header_YUVCO::E_ENQUE_SOF:
            return CAM_READ_REG(DrvPtr,YUVCO_R1_YUVCO_FH_SPARE_3);
            break;
        case Header_YUVCO::E_PIX_ID:
            return CAM_READ_REG(DrvPtr,YUVCO_R1_YUVCO_FH_SPARE_5);
            break;
        case Header_YUVCO::E_FMT:
            return CAM_READ_REG(DrvPtr,YUVCO_R1_YUVCO_FH_SPARE_6);
            break;
        case Header_YUVCO::E_CROP_START:
            return CAM_READ_REG(DrvPtr,YUVCO_R1_YUVCO_FH_SPARE_7);
            break;
        case Header_YUVCO::E_CROP_SIZE:
            return CAM_READ_REG(DrvPtr,YUVCO_R1_YUVCO_FH_SPARE_8);
            break;
        case Header_YUVCO::E_ENQUE_CNT:
            return CAM_READ_REG(DrvPtr,YUVCO_R1_YUVCO_FH_SPARE_9);
            break;
        case Header_YUVCO::E_SRC_SIZE:
            return CAM_READ_REG(DrvPtr,YUVCO_R1_YUVCO_FH_SPARE_10);
            break;
        case Header_YUVCO::E_IQ_LEVEL:
            return CAM_READ_REG(DrvPtr,YUVCO_R1_YUVCO_FH_SPARE_11);
            break;
        case Header_YUVCO::E_IMG_PA:
            return CAM_READ_REG(DrvPtr,YUVCO_R1_YUVCO_FH_SPARE_12);
            break;
        default:
        case Header_YUVCO::E_TimeStamp_LSB:
        case Header_YUVCO::E_TimeStamp_MSB:
            CAM_FUNC_ERR("unsupported tag:0x%x\n",Tag);
            break;
    }
    return 0;
}

void Header_CRZO_R1::Header_Enque(MUINT32 Tag,ISP_DRV_CAM* DrvPtr,MUINT32 value)
{
    switch(Tag){
        case Header_CRZO_R1::E_Magic:
            CAM_WRITE_REG(DrvPtr,CRZO_R1_CRZO_FH_SPARE_2,value);
            break;
        case Header_CRZO_R1::E_ENQUE_SOF:
            CAM_WRITE_REG(DrvPtr,CRZO_R1_CRZO_FH_SPARE_3,value);
            break;
        case Header_CRZO_R1::E_SW_FMT:
            CAM_WRITE_REG(DrvPtr,CRZO_R1_CRZO_FH_SPARE_4,value);
            break;
        case Header_CRZO_R1::E_FMT:
            CAM_WRITE_REG(DrvPtr,CRZO_R1_CRZO_FH_SPARE_5,value);
            break;
        case Header_CRZO_R1::E_CRZ_CROP_START:
            CAM_WRITE_REG(DrvPtr,CRZO_R1_CRZO_FH_SPARE_6,value);
            break;
        case Header_CRZO_R1::E_CRZ_CROP_SIZE:
            CAM_WRITE_REG(DrvPtr,CRZO_R1_CRZO_FH_SPARE_7,value);
            break;
        case Header_CRZO_R1::E_CRZ_DST_SIZE:
            CAM_WRITE_REG(DrvPtr,CRZO_R1_CRZO_FH_SPARE_8,value);
            break;
        case Header_CRZO_R1::E_ENQUE_CNT:
            CAM_WRITE_REG(DrvPtr,CRZO_R1_CRZO_FH_SPARE_9,value);
            break;
        case Header_CRZO_R1::E_PLANE_EN:
            CAM_WRITE_REG(DrvPtr,CRZO_R1_CRZO_FH_SPARE_10,value);
            break;
        case Header_CRZO_R1::E_IQ_LEVEL:
            CAM_WRITE_REG(DrvPtr,CRZO_R1_CRZO_FH_SPARE_11,value);
            break;
        case Header_CRZO_R1::E_IMG_PA:
            CAM_WRITE_REG(DrvPtr,CRZO_R1_CRZO_FH_SPARE_12,value);
            break;
        default:
        case Header_CRZO_R1::E_TimeStamp_LSB:
        case Header_CRZO_R1::E_TimeStamp_MSB:
            CAM_FUNC_ERR("unsupported tag:0x%x\n",Tag);
            break;
    }
}

void Header_CRZO_R1::Header_Deque(MUINTPTR FH_VA,void* pOut)
{
    void* ptr = (void*)FH_VA;
    memcpy(pOut,ptr,sizeof(MUINT32)*Header_CRZO_R1::E_MAX);
}

void Header_CRZO_R1::Header_Flash(MUINTPTR FH_VA)
{
    void* ptr = (void*)FH_VA;
    memset(ptr,0,sizeof(MUINT32)*Header_CRZO_R1::E_MAX);

}

MUINT32 Header_CRZO_R1::GetRegInfo(MUINT32 Tag,IspDrv* DrvPtr)
{
    switch(Tag){
        case Header_CRZO_R1::E_Magic:
            return CAM_READ_REG(DrvPtr,CRZO_R1_CRZO_FH_SPARE_2);
            break;
        case Header_CRZO_R1::E_ENQUE_SOF:
            return CAM_READ_REG(DrvPtr,CRZO_R1_CRZO_FH_SPARE_3);
            break;
        case Header_CRZO_R1::E_SW_FMT:
            return CAM_READ_REG(DrvPtr,CRZO_R1_CRZO_FH_SPARE_4);
            break;
        case Header_CRZO_R1::E_FMT:
            return CAM_READ_REG(DrvPtr,CRZO_R1_CRZO_FH_SPARE_5);
            break;
        case Header_CRZO_R1::E_CRZ_CROP_START:
            return CAM_READ_REG(DrvPtr,CRZO_R1_CRZO_FH_SPARE_6);
            break;
        case Header_CRZO_R1::E_CRZ_CROP_SIZE:
            return CAM_READ_REG(DrvPtr,CRZO_R1_CRZO_FH_SPARE_7);
            break;
        case Header_CRZO_R1::E_CRZ_DST_SIZE:
            return CAM_READ_REG(DrvPtr,CRZO_R1_CRZO_FH_SPARE_8);
            break;
        case Header_CRZO_R1::E_ENQUE_CNT:
            return CAM_READ_REG(DrvPtr,CRZO_R1_CRZO_FH_SPARE_9);
            break;
        case Header_CRZO_R1::E_PLANE_EN:
            return CAM_READ_REG(DrvPtr,CRZO_R1_CRZO_FH_SPARE_10);
            break;
        case Header_CRZO_R1::E_IQ_LEVEL:
            return CAM_READ_REG(DrvPtr,CRZO_R1_CRZO_FH_SPARE_11);
            break;
        case Header_CRZO_R1::E_IMG_PA:
            return CAM_READ_REG(DrvPtr,CRZO_R1_CRZO_FH_SPARE_12);
            break;
        default:
        case Header_CRZO_R1::E_TimeStamp_LSB:
        case Header_CRZO_R1::E_TimeStamp_MSB:
            CAM_FUNC_ERR("unsupported tag:0x%x\n",Tag);
            break;
    }
    return 0;
}

void Header_CRZBO_R1::Header_Enque(MUINT32 Tag,ISP_DRV_CAM* DrvPtr,MUINT32 value)
{
    switch(Tag){
        case Header_CRZBO_R1::E_Magic:
            CAM_WRITE_REG(DrvPtr,CRZBO_R1_CRZBO_FH_SPARE_2,value);
            break;
        case Header_CRZBO_R1::E_ENQUE_SOF:
            CAM_WRITE_REG(DrvPtr,CRZBO_R1_CRZBO_FH_SPARE_3,value);
            break;
        case Header_CRZBO_R1::E_PIX_ID:
            CAM_WRITE_REG(DrvPtr,CRZBO_R1_CRZBO_FH_SPARE_4,value);
            break;
        case Header_CRZBO_R1::E_FMT:
            CAM_WRITE_REG(DrvPtr,CRZBO_R1_CRZBO_FH_SPARE_5,value);
            break;
        case Header_CRZBO_R1::E_CRZ_CROP_START:
            CAM_WRITE_REG(DrvPtr,CRZBO_R1_CRZBO_FH_SPARE_6,value);
            break;
        case Header_CRZBO_R1::E_CRZ_CROP_SIZE:
            CAM_WRITE_REG(DrvPtr,CRZBO_R1_CRZBO_FH_SPARE_7,value);
            break;
        case Header_CRZBO_R1::E_CRZ_DST_SIZE:
            CAM_WRITE_REG(DrvPtr,CRZBO_R1_CRZBO_FH_SPARE_8,value);
            break;
        case Header_CRZBO_R1::E_ENQUE_CNT:
            CAM_WRITE_REG(DrvPtr,CRZBO_R1_CRZBO_FH_SPARE_9,value);
            break;
        case Header_CRZBO_R1::E_IQ_LEVEL:
            CAM_WRITE_REG(DrvPtr,CRZBO_R1_CRZBO_FH_SPARE_10,value);
            break;
        case Header_CRZBO_R1::E_IMG_PA:
            CAM_WRITE_REG(DrvPtr,CRZBO_R1_CRZBO_FH_SPARE_12,value);
            break;
        default:
        case Header_CRZBO_R1::E_TimeStamp_LSB:
        case Header_CRZBO_R1::E_TimeStamp_MSB:
            CAM_FUNC_ERR("unsupported tag:0x%x\n",Tag);
            break;
    }
}

void Header_CRZBO_R1::Header_Deque(MUINTPTR FH_VA,void* pOut)
{
    void* ptr = (void*)FH_VA;
    memcpy(pOut,ptr,sizeof(MUINT32)*Header_CRZBO_R1::E_MAX);
}

void Header_CRZBO_R1::Header_Flash(MUINTPTR FH_VA)
{
    void* ptr = (void*)FH_VA;
    memset(ptr,0,sizeof(MUINT32)*Header_CRZBO_R1::E_MAX);

}

MUINT32 Header_CRZBO_R1::GetRegInfo(MUINT32 Tag,IspDrv* DrvPtr)
{
    switch(Tag){
        case Header_CRZBO_R1::E_Magic:
            return CAM_READ_REG(DrvPtr,CRZBO_R1_CRZBO_FH_SPARE_2);
            break;
        case Header_CRZBO_R1::E_ENQUE_SOF:
            return CAM_READ_REG(DrvPtr,CRZBO_R1_CRZBO_FH_SPARE_3);
            break;
        case Header_CRZBO_R1::E_PIX_ID:
            return CAM_READ_REG(DrvPtr,CRZBO_R1_CRZBO_FH_SPARE_4);
            break;
        case Header_CRZBO_R1::E_FMT:
            return CAM_READ_REG(DrvPtr,CRZBO_R1_CRZBO_FH_SPARE_5);
            break;
        case Header_CRZBO_R1::E_CRZ_CROP_START:
            return CAM_READ_REG(DrvPtr,CRZBO_R1_CRZBO_FH_SPARE_6);
            break;
        case Header_CRZBO_R1::E_CRZ_CROP_SIZE:
            return CAM_READ_REG(DrvPtr,CRZBO_R1_CRZBO_FH_SPARE_7);
            break;
        case Header_CRZBO_R1::E_CRZ_DST_SIZE:
            return CAM_READ_REG(DrvPtr,CRZBO_R1_CRZBO_FH_SPARE_8);
            break;
        case Header_CRZBO_R1::E_ENQUE_CNT:
            return CAM_READ_REG(DrvPtr,CRZBO_R1_CRZBO_FH_SPARE_9);
            break;
        case Header_CRZBO_R1::E_IQ_LEVEL:
            return CAM_READ_REG(DrvPtr,CRZBO_R1_CRZBO_FH_SPARE_10);
            break;
        case Header_CRZBO_R1::E_IMG_PA:
            return CAM_READ_REG(DrvPtr,CRZBO_R1_CRZBO_FH_SPARE_12);
            break;
        default:
        case Header_CRZBO_R1::E_TimeStamp_LSB:
        case Header_CRZBO_R1::E_TimeStamp_MSB:
            CAM_FUNC_ERR("unsupported tag:0x%x\n",Tag);
            break;
    }
    return 0;
}

void Header_CRZO_R2::Header_Enque(MUINT32 Tag,ISP_DRV_CAM* DrvPtr,MUINT32 value)
{
    switch(Tag){
        case Header_CRZO_R2::E_Magic:
            CAM_WRITE_REG(DrvPtr,CRZO_R2_CRZO_FH_SPARE_2,value);
            break;
        case Header_CRZO_R2::E_ENQUE_SOF:
            CAM_WRITE_REG(DrvPtr,CRZO_R2_CRZO_FH_SPARE_3,value);
            break;
        case Header_CRZO_R2::E_SW_FMT:
            CAM_WRITE_REG(DrvPtr,CRZO_R2_CRZO_FH_SPARE_4,value);
            break;
        case Header_CRZO_R2::E_FMT:
            CAM_WRITE_REG(DrvPtr,CRZO_R2_CRZO_FH_SPARE_5,value);
            break;
        case Header_CRZO_R2::E_CRZ_CROP_START:
            CAM_WRITE_REG(DrvPtr,CRZO_R2_CRZO_FH_SPARE_6,value);
            break;
        case Header_CRZO_R2::E_CRZ_CROP_SIZE:
            CAM_WRITE_REG(DrvPtr,CRZO_R2_CRZO_FH_SPARE_7,value);
            break;
        case Header_CRZO_R2::E_CRZ_DST_SIZE:
            CAM_WRITE_REG(DrvPtr,CRZO_R2_CRZO_FH_SPARE_8,value);
            break;
        case Header_CRZO_R2::E_ENQUE_CNT:
            CAM_WRITE_REG(DrvPtr,CRZO_R2_CRZO_FH_SPARE_9,value);
            break;
        case Header_CRZO_R2::E_PLANE_EN:
            CAM_WRITE_REG(DrvPtr,CRZO_R2_CRZO_FH_SPARE_10,value);
            break;
        case Header_CRZO_R2::E_IQ_LEVEL:
            CAM_WRITE_REG(DrvPtr,CRZO_R2_CRZO_FH_SPARE_11,value);
            break;
        case Header_CRZO_R2::E_IMG_PA:
            CAM_WRITE_REG(DrvPtr,CRZO_R2_CRZO_FH_SPARE_12,value);
            break;
        default:
        case Header_CRZO_R2::E_TimeStamp_LSB:
        case Header_CRZO_R2::E_TimeStamp_MSB:
            CAM_FUNC_ERR("unsupported tag:0x%x\n",Tag);
            break;
    }
}

void Header_CRZO_R2::Header_Deque(MUINTPTR FH_VA,void* pOut)
{
    void* ptr = (void*)FH_VA;
    memcpy(pOut,ptr,sizeof(MUINT32)*Header_CRZO_R2::E_MAX);
}

void Header_CRZO_R2::Header_Flash(MUINTPTR FH_VA)
{
    void* ptr = (void*)FH_VA;
    memset(ptr,0,sizeof(MUINT32)*Header_CRZO_R2::E_MAX);

}

MUINT32 Header_CRZO_R2::GetRegInfo(MUINT32 Tag,IspDrv* DrvPtr)
{
    switch(Tag){
        case Header_CRZO_R2::E_Magic:
            return CAM_READ_REG(DrvPtr,CRZO_R2_CRZO_FH_SPARE_2);
            break;
        case Header_CRZO_R2::E_ENQUE_SOF:
            return CAM_READ_REG(DrvPtr,CRZO_R2_CRZO_FH_SPARE_3);
            break;
        case Header_CRZO_R2::E_SW_FMT:
            return CAM_READ_REG(DrvPtr,CRZO_R2_CRZO_FH_SPARE_4);
            break;
        case Header_CRZO_R2::E_FMT:
            return CAM_READ_REG(DrvPtr,CRZO_R2_CRZO_FH_SPARE_5);
            break;
        case Header_CRZO_R2::E_CRZ_CROP_START:
            return CAM_READ_REG(DrvPtr,CRZO_R2_CRZO_FH_SPARE_6);
            break;
        case Header_CRZO_R2::E_CRZ_CROP_SIZE:
            return CAM_READ_REG(DrvPtr,CRZO_R2_CRZO_FH_SPARE_7);
            break;
        case Header_CRZO_R2::E_CRZ_DST_SIZE:
            return CAM_READ_REG(DrvPtr,CRZO_R2_CRZO_FH_SPARE_8);
            break;
        case Header_CRZO_R2::E_ENQUE_CNT:
            return CAM_READ_REG(DrvPtr,CRZO_R2_CRZO_FH_SPARE_9);
            break;
        case Header_CRZO_R2::E_PLANE_EN:
            return CAM_READ_REG(DrvPtr,CRZO_R2_CRZO_FH_SPARE_10);
            break;
        case Header_CRZO_R2::E_IQ_LEVEL:
            return CAM_READ_REG(DrvPtr,CRZO_R2_CRZO_FH_SPARE_11);
            break;
        case Header_CRZO_R2::E_IMG_PA:
            return CAM_READ_REG(DrvPtr,CRZO_R2_CRZO_FH_SPARE_12);
            break;
        default:
        case Header_CRZO_R2::E_TimeStamp_LSB:
        case Header_CRZO_R2::E_TimeStamp_MSB:
            CAM_FUNC_ERR("unsupported tag:0x%x\n",Tag);
            break;
    }
    return 0;
}

void Header_CRZBO_R2::Header_Enque(MUINT32 Tag,ISP_DRV_CAM* DrvPtr,MUINT32 value)
{
    switch(Tag){
        case Header_CRZBO_R2::E_Magic:
            CAM_WRITE_REG(DrvPtr,CRZBO_R2_CRZBO_FH_SPARE_2,value);
            break;
        case Header_CRZBO_R2::E_ENQUE_SOF:
            CAM_WRITE_REG(DrvPtr,CRZBO_R2_CRZBO_FH_SPARE_3,value);
            break;
        case Header_CRZBO_R2::E_PIX_ID:
            CAM_WRITE_REG(DrvPtr,CRZBO_R2_CRZBO_FH_SPARE_4,value);
            break;
        case Header_CRZBO_R2::E_FMT:
            CAM_WRITE_REG(DrvPtr,CRZBO_R2_CRZBO_FH_SPARE_5,value);
            break;
        case Header_CRZBO_R2::E_CRZ_CROP_START:
            CAM_WRITE_REG(DrvPtr,CRZBO_R2_CRZBO_FH_SPARE_6,value);
            break;
        case Header_CRZBO_R2::E_CRZ_CROP_SIZE:
            CAM_WRITE_REG(DrvPtr,CRZBO_R2_CRZBO_FH_SPARE_7,value);
            break;
        case Header_CRZBO_R2::E_CRZ_DST_SIZE:
            CAM_WRITE_REG(DrvPtr,CRZBO_R2_CRZBO_FH_SPARE_8,value);
            break;
        case Header_CRZBO_R2::E_ENQUE_CNT:
            CAM_WRITE_REG(DrvPtr,CRZBO_R2_CRZBO_FH_SPARE_9,value);
            break;
        case Header_CRZBO_R2::E_IQ_LEVEL:
            CAM_WRITE_REG(DrvPtr,CRZBO_R2_CRZBO_FH_SPARE_10,value);
            break;
        case Header_CRZBO_R2::E_IMG_PA:
            CAM_WRITE_REG(DrvPtr,CRZBO_R2_CRZBO_FH_SPARE_12,value);
            break;
        default:
        case Header_CRZBO_R2::E_TimeStamp_LSB:
        case Header_CRZBO_R2::E_TimeStamp_MSB:
            CAM_FUNC_ERR("unsupported tag:0x%x\n",Tag);
            break;
    }
}

void Header_CRZBO_R2::Header_Deque(MUINTPTR FH_VA,void* pOut)
{
    void* ptr = (void*)FH_VA;
    memcpy(pOut,ptr,sizeof(MUINT32)*Header_CRZBO_R2::E_MAX);
}

void Header_CRZBO_R2::Header_Flash(MUINTPTR FH_VA)
{
    void* ptr = (void*)FH_VA;
    memset(ptr,0,sizeof(MUINT32)*Header_CRZBO_R2::E_MAX);

}

MUINT32 Header_CRZBO_R2::GetRegInfo(MUINT32 Tag,IspDrv* DrvPtr)
{
    switch(Tag){
        case Header_CRZBO_R2::E_Magic:
            return CAM_READ_REG(DrvPtr,CRZBO_R2_CRZBO_FH_SPARE_2);
            break;
        case Header_CRZBO_R2::E_ENQUE_SOF:
            return CAM_READ_REG(DrvPtr,CRZBO_R2_CRZBO_FH_SPARE_3);
            break;
        case Header_CRZBO_R2::E_PIX_ID:
            return CAM_READ_REG(DrvPtr,CRZBO_R2_CRZBO_FH_SPARE_4);
            break;
        case Header_CRZBO_R2::E_FMT:
            return CAM_READ_REG(DrvPtr,CRZBO_R2_CRZBO_FH_SPARE_5);
            break;
        case Header_CRZBO_R2::E_CRZ_CROP_START:
            return CAM_READ_REG(DrvPtr,CRZBO_R2_CRZBO_FH_SPARE_6);
            break;
        case Header_CRZBO_R2::E_CRZ_CROP_SIZE:
            return CAM_READ_REG(DrvPtr,CRZBO_R2_CRZBO_FH_SPARE_7);
            break;
        case Header_CRZBO_R2::E_CRZ_DST_SIZE:
            return CAM_READ_REG(DrvPtr,CRZBO_R2_CRZBO_FH_SPARE_8);
            break;
        case Header_CRZBO_R2::E_ENQUE_CNT:
            return CAM_READ_REG(DrvPtr,CRZBO_R2_CRZBO_FH_SPARE_9);
            break;
        case Header_CRZBO_R2::E_IQ_LEVEL:
            return CAM_READ_REG(DrvPtr,CRZBO_R2_CRZBO_FH_SPARE_10);
            break;
        case Header_CRZBO_R2::E_IMG_PA:
            return CAM_READ_REG(DrvPtr,CRZBO_R2_CRZBO_FH_SPARE_12);
            break;
        default:
        case Header_CRZBO_R2::E_TimeStamp_LSB:
        case Header_CRZBO_R2::E_TimeStamp_MSB:
            CAM_FUNC_ERR("unsupported tag:0x%x\n",Tag);
            break;
    }
    return 0;
}

