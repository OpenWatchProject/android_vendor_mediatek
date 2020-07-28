#define LOG_TAG "ifunc_buf"

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
//#include "dual_isp_config.h"
#include "Cam_Notify_datatype.h"
#include "tuning_drv.h"


#include "imageio_log.h"                    // Note: DBG_LOG_TAG/LEVEL will be used in header file, so header must be included after definition.
#undef   DBG_LOG_TAG                        // Decide a Log TAG for current file.
#ifndef USING_MTK_LDVT   // Not using LDVT.
#define  DBG_LOG_TAG        ""
#else
#define  DBG_LOG_TAG        LOG_TAG
#endif


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

#define CAM_FUNC_WRN(fmt, arg...)        do { if (func_cam_DbgLogEnable_WARN   ) { BASE_LOG_WRN(fmt, ##arg); } } while(0)

#define CAM_FUNC_ERR(fmt, arg...)        do {\
    if (func_cam_DbgLogEnable_ERROR  ) { \
        BASE_LOG_ERR("[0x%x]:" fmt,this->m_hwModule, ##arg); \
    }\
} while(0)

#define CAM_FUNC_AST(cond, fmt, arg...)  do { if (func_cam_DbgLogEnable_ASSERT ) { BASE_LOG_AST(cond, fmt, ##arg); } } while(0)

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
    else\
        _port = _dma_max_wr_;\
    _port;\
})
/*/////////////////////////////////////////////////////////////////////////////
    CAM_BUF_CTRL
  /////////////////////////////////////////////////////////////////////////////*/
template<typename typ>
void CAM_BUF_CTRL::QueueMgr<typ>::init(void)
{
    this->m_v_pQueue.clear();
}

template<typename typ>
void CAM_BUF_CTRL::QueueMgr<typ>::uninit(void)
{
    this->m_v_pQueue.clear();
}

template<typename typ>
MBOOL CAM_BUF_CTRL::QueueMgr<typ>::sendCmd(QueueMgr_cmd cmd,MUINTPTR arg1,MUINTPTR arg2)
{
    MBOOL ret = MTRUE;
    Mutex::Autolock lock(this->mQueueLock);
    switch(cmd){
        case eCmd_getsize:
            *(MUINTPTR*)arg1 = this->m_v_pQueue.size();
            break;
        case eCmd_push:
            this->m_v_pQueue.push_back(*(typ*)(arg1));
            break;
        case eCmd_pop:
            this->m_v_pQueue.pop_front();
            break;
        case eCmd_front:
            *(typ*)arg1 = this->m_v_pQueue.front();
            break;
        case eCmd_at:
            if(this->m_v_pQueue.size()>arg1){
                *(typ*)arg2 = this->m_v_pQueue.at(arg1);
            }
            else{
                BASE_LOG_INF("size have been reduce to 0x%x(0x%x) by enque\n",this->m_v_pQueue.size(),arg1);
                ret = MFALSE;
            }
            break;
        case eCmd_pop_back:
            this->m_v_pQueue.pop_back();
            break;
        case eCmd_end:
        default:
            BASE_LOG_ERR("error: unsopported cmd:0x%x\n",cmd);
            ret = MFALSE;
            break;
    }
    return ret;
}

MUINT32 CAM_BUF_CTRL::m_fps[CAM_MAX] = {30, 30};
MUINT32 CAM_BUF_CTRL::m_recentFrmTimeMs[CAM_MAX][MAX_RECENT_GRPFRM_TIME] = {
    {MIN_GRPFRM_TIME_MS, MIN_GRPFRM_TIME_MS},
    {MIN_GRPFRM_TIME_MS, MIN_GRPFRM_TIME_MS}
};
CAM_BUF_CTRL::CAM_BUF_CTRL(void)
{
    //m_fps = 30;
    m_tail_idx = -1;
    m_buf_cnt = 0;
    m_pUniDrv = NULL;
    m_pDrv = NULL;
    m_prvClk = 0;
    m_nOverFlow = 0;
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
        ptr = (ISP_DRV_CAM*)ISP_DRV_CAM::createInstance(CAM_B,ISP_DRV_CQ_THRE0,0,"PIPE_CHK");
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

    if(bTwin){
        ppc.m_pDrv = ptr;
        if( ppc.AF_CHECK() == MTRUE){
            CAM_FUNC_INF("af/afo at CAM_B check pass!\n");
        }
        else
            rst += 1;

#if PDO_SW_WORK_AROUND
        if( ppc.PDO_CHECK() == MTRUE){
            CAM_FUNC_INF("pd/pdo at CAM_B check pass!\n");
        }
        else
            rst += 1;
#endif
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
            sprintf(ptr,"UNI DMA ERR");
            while (*ptr++ != '\0'){}
            ptr--;
            rst += 1;
        }
        if(uni_int & FLKO_A_ERR_INT_EN_){
            sprintf(ptr,"FLKO ERR");
            while (*ptr++ != '\0'){}
            ptr--;
            rst += 1;
        }
        if(uni_int & EISO_A_ERR_INT_EN_){
            sprintf(ptr,"EISO ERR");
            while (*ptr++ != '\0'){}
            ptr--;
            rst += 1;
        }
        if(uni_int & RSSO_A_ERR_INT_EN_){
            sprintf(ptr,"RSSO ERR");
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

EXIT:
    if(bTwin == 1){
        ptr->destroyInstance();
    }

    if(rst)
        return MFALSE;
    else
        return MTRUE;
}

MBOOL CAM_BUF_CTRL::waitBufReady(CAM_STATE_NOTIFY *pNotify)
{
#define _LOOP_  (5)
    MBOOL ret = MTRUE;
    E_ISP_CAM_CQ cq;
    MUINT32 page;

    Header_IMGO fh_imgo;
    Header_RRZO fh_rrzo;
    Header_UFEO fh_ufeo;
    Header_LCSO fh_lcso;
    Header_EISO fh_eiso;
    //Header_RSSO fh_rsso;

    ISP_WAIT_IRQ_ST irq;
    MUINT32 loopCnt = _LOOP_;
    MUINT32 subSample;
    char str[128] = {'\0'};

    MUINT32 FbcCnt=0;
    //MUINT32 WCnt=0,newWCnt=0; //not used in olympus
    MUINT32 sw_enque_cnt = 0;
    ST_BUF_INFO _buf;
    MUINT32 _header_data[E_HEADER_MAX];
    ISP_DRV_CAM* pDrv = NULL;
    MUINT32 _size = 0;
    MBOOL bWaitPass = MTRUE;
    MUINT32 cam_dmao, cur_id;

    if ( 0xffffffff == this->id() ) {
        CAM_FUNC_ERR("dma channel error ");
        return MFALSE;
    }

    this->m_Queue_deque_ptr.sendCmd(CAM_BUF_CTRL::QueueMgr<MUINTPTR>::eCmd_getsize,(MUINTPTR)&_size,0);
    if(_size <1){
        CAM_FUNC_ERR("queue depth is empty for deque\n");
        return MFALSE;
    }

    //
    this->m_Queue_deque_ptr.sendCmd(CAM_BUF_CTRL::QueueMgr<MUINTPTR>::eCmd_front  ,(MUINTPTR)&pDrv,0);
    //can't pop here, pop at deque
    //this->m_Queue_deque_ptr.sendCmd(CAM_BUF_CTRL::QueueMgr<MUINTPTR>::eCmd_pop  ,0,0);
    //dbg log
    pDrv->getCurObjInfo(&this->m_hwModule,&cq,&page);


    //fbc reg dbg log
    this->FBC_STATUS(pDrv->getPhyObj());

    subSample = CAM_READ_BITS(pDrv->getPhyObj(),CAM_CTL_SW_PASS1_DONE,DOWN_SAMPLE_PERIOD) + 1;

    irq.Clear = ISP_IRQ_CLEAR_WAIT;
    irq.UserKey = 0;

    if (1) { //OLY_E1
        switch(this->id()){
            case _imgo_:
                irq.St_type = DMA_INT;
                irq.Status = (MUINT32)IMGO_DONE_ST;
                break;
            case _rrzo_:
                irq.St_type = DMA_INT;
                irq.Status = (MUINT32)RRZO_DONE_ST;
                break;
            case _ufeo_:
                irq.St_type = DMA_INT;
                irq.Status = (MUINT32)UFEO_DONE_ST;
                break;
            case _lcso_:
                irq.St_type = DMA_INT;
                irq.Status = (MUINT32)LCSO_DONE_ST;
                break;
            case _eiso_:
                irq.St_type = DMA_INT;
                irq.Status = (MUINT32)EISO_DONE_ST;
                break;
            case _aao_:
            case _afo_:
            case _pdo_:
            case _flko_:
            case _rsso_:
                //irq.St_type = DMA_INT;
                //irq.Status = (MUINT32)RSSO_DONE_ST;
            default:
                CAM_FUNC_ERR("un-supported DMAO:0x%x\n",this->id());
                return MFALSE;
            break;
        }
    }
    else {
        switch(this->id()){
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
            case _aao_:
            case _afo_:
            case _pdo_:
            case _flko_:
            case _rsso_:
                //irq.St_type = DMA_INT;
                //irq.Status = (MUINT32)RSSO_DONE_ST;
            default:
                CAM_FUNC_ERR("un-supported DMAO:0x%x\n",this->id());
                return MFALSE;
            break;
        }
    }

    do{
        //irq.Timeout = ((1000 + (this->m_fps - 1)) / this->m_fps) * subSample;
        //irq.Timeout *= 5; //amplify 3 times is for the case of enque latch timing
        irq.Timeout = this->estimateTimeout(subSample);

        if( (this->m_buf_cnt != 0) && (this->m_buf_cnt >= subSample) ){
            //
            this->m_Queue.sendCmd(CAM_BUF_CTRL::QueueMgr<ST_BUF_INFO>::eCmd_at,(subSample-1),(MUINTPTR)&_buf);
            //
            /*
             * MUST hold fbcPhyLock_XXX when read FBC CTL registers, due to isp reset flow period will
             * clear FBC register value before. Hold lock will wait until reset flow recovers FBC register values
             */
            switch(this->id()){
                case _imgo_:
                    if(loopCnt == _LOOP_){
                        fh_imgo.Header_Deque(_buf.header.va_addr,(void*)_header_data);
                        sw_enque_cnt = _header_data[Header_IMGO::E_ENQUE_CNT];
                    }
                    break;
                case _rrzo_:
                    if(loopCnt == _LOOP_){
                        fh_rrzo.Header_Deque(_buf.header.va_addr,(void*)_header_data);
                        sw_enque_cnt = _header_data[Header_RRZO::E_ENQUE_CNT];
                    }
                    break;
                case _ufeo_:
                    if(loopCnt == _LOOP_){
                        fh_ufeo.Header_Deque(_buf.header.va_addr,(void*)_header_data);
                        sw_enque_cnt = _header_data[Header_UFEO::E_ENQUE_CNT];
                    }
                    break;
                case _lcso_:
                    if(loopCnt == _LOOP_){
                        fh_lcso.Header_Deque(_buf.header.va_addr,(void*)_header_data);
                        sw_enque_cnt = _header_data[Header_LCSO::E_ENQUE_CNT];
                    }
                    break;
                case _eiso_:
                    if(loopCnt == _LOOP_){
                        fh_eiso.Header_Deque(_buf.header.va_addr,(void*)_header_data);
                        sw_enque_cnt = _header_data[Header_EISO::E_ENQUE_CNT];
                    }
                    break;
                case _aao_:
                case _afo_:
                case _pdo_:
                case _flko_:
                case _rsso_:
                    //irq.St_type = DMA_INT;
                    //irq.Status = (MUINT32)RSSO_DONE_ST;
                    //FbcCnt = UNI_READ_BITS(this->m_pUniDrv,CAM_UNI_FBC_RSSO_A_CTL2,FBC_CNT);
                    //break;
                default:
                    CAM_FUNC_ERR("un-supported DMAO:0x%x\n",this->id());
                    return MFALSE;
                break;
            }

            //only using sw_enque_cnt at 1st time.
            //because dmao may still output even if no sw p1 done . e.g. : some module setting err.
            //if no sw p1 done, hw fbc cnt won't be update.
            if(loopCnt == _LOOP_){
                //because rcnt_in is locate on CQ, and m_buf_cnt is updated directly when enque.
                //so it will have a time difference between FBC hw cnt latch and m_buf_cnt update.
                //so can't be just using FbcCnt != m_buf_cnt
                if(sw_enque_cnt>0){
                    MUINT32 waitOps[3];

                    waitOps[0] = _cam_max_;
                    waitOps[1] = this->id();

                    if (this->m_pDrv->getDeviceInfo(_GET_START_TIME, (MUINT8*)waitOps) == MFALSE) {
                        CAM_FUNC_ERR("Wait timestamp ready fail\n");
                    }

                    CAM_FUNC_DBG("dma:%s already have avail_buf on dram, bWaitBufReady  = 0\n",this->name_Str());
                    return MTRUE;
                }
                else{
                    sprintf(str,"dma:%s start wait:[enque record:0x%x_0x%x_0x%x],",this->name_Str(),\
                        this->m_buf_cnt,sw_enque_cnt,FbcCnt);
                }
            }
            else{
                char _tmp[16];

                if (sw_enque_cnt > 0) {
                    ret = MTRUE;
                    //add for timing error when deque twice between dmao_done to p1_done.
                    //currently , only imgo/rrzo using p1 done, so only these 2 dmao need to check.
                    if(bWaitPass == MTRUE)
                        goto EXIT;
                }

                sprintf(_tmp,"[%d]time,",((_LOOP_-loopCnt)+1));
                strcat(str,_tmp);
            }

        }
        else{
            if(loopCnt == _LOOP_){
                sprintf(str,"dma:%s bWaitBufReady = 1,wait for sw enque[0x%x_0x%x],timeout:%d x %d ms,[1] ",\
                    this->name_Str(),this->m_buf_cnt,subSample,irq.Timeout,loopCnt);
            }
            else{//save how many times of wait_irq before sw enque
                char _tmp[8];
                sprintf(_tmp,"[%d],",((_LOOP_-loopCnt)+1) + 1);
                strcat(str,_tmp);
            }
        }

        //
        if (pNotify) {
            pNotify->fpNotifyState(CAM_ENTER_WAIT, pNotify->Obj);
        }

        ret = pDrv->waitIrq(&irq);

        if (pNotify) {
            pNotify->fpNotifyState(CAM_EXIT_WAIT, pNotify->Obj);
        }

        /* no matter waitIrq is timeout or not, VF_EN=0 is no need to go further...*/
        if(CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_TG_VF_CON,VFDATA_EN) == 0){
            CAM_FUNC_WRN("%s:VF_EN=0, %s", this->name_Str(),str);
            bWaitPass = MFALSE;//leave while loop
            return MFALSE;
        }

        if((ret == MTRUE) && (this->m_buf_cnt != 0) && (this->m_buf_cnt >= subSample) ){
            if (subSample > 1) {
                /*
                 * Subsample case, dmao done signal not subsampled, need wait sw_p1_done
                 * to guarantee all subsampled frames are outputed
                 */
                ISP_WAIT_IRQ_ST irq_SwP1Done;

                irq_SwP1Done.Clear = ISP_IRQ_CLEAR_WAIT;
                irq_SwP1Done.UserKey = 0;
                irq_SwP1Done.Timeout = this->estimateTimeout(subSample);
                irq_SwP1Done.St_type = SIGNAL_INT;
                irq_SwP1Done.Status = (MUINT32)SW_PASS1_DON_ST;

                if (pNotify) {
                    pNotify->fpNotifyState(CAM_ENTER_WAIT, pNotify->Obj);
                }

                ret = pDrv->waitIrq(&irq_SwP1Done);

                if (pNotify) {
                    pNotify->fpNotifyState(CAM_EXIT_WAIT, pNotify->Obj);
                }

                /* no matter waitIrq is timeout or not, VF_EN=0 is no need to go further...*/
                if (CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_TG_VF_CON,VFDATA_EN) == 0) {
                    CAM_FUNC_WRN("%s:VF_EN=0, %s after wait sw_p1done", this->name_Str(),str);
                    return MFALSE;
                }
            }

            //add for timing error when deque twice between dmao_done to p1_done.
            //currently , only imgo/rrzo using p1 done, so only these 2 dmao need to check.
            //olympus need not this check, even in subsample flow
            switch(this->id()){
                case _imgo_:
                    {
                        Header_IMGO fh_imgo;
                        ST_BUF_INFO _buf;

                        MUINT32 _header_data[E_HEADER_MAX];
                        //this->m_Queue.sendCmd(CAM_BUF_CTRL::QueueMgr<ST_BUF_INFO>::eCmd_front  ,(MUINTPTR)&_buf,0);
                        this->m_Queue.sendCmd(CAM_BUF_CTRL::QueueMgr<ST_BUF_INFO>::eCmd_at,(subSample-1),(MUINTPTR)&_buf);

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
                        //this->m_Queue.sendCmd(CAM_BUF_CTRL::QueueMgr<ST_BUF_INFO>::eCmd_front  ,(MUINTPTR)&_buf,0);
                        this->m_Queue.sendCmd(CAM_BUF_CTRL::QueueMgr<ST_BUF_INFO>::eCmd_at,(subSample-1),(MUINTPTR)&_buf);

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
                        //this->m_Queue.sendCmd(CAM_BUF_CTRL::QueueMgr<ST_BUF_INFO>::eCmd_front  ,(MUINTPTR)&_buf,0);
						this->m_Queue.sendCmd(CAM_BUF_CTRL::QueueMgr<ST_BUF_INFO>::eCmd_at,(subSample-1),(MUINTPTR)&_buf);
                        fh_ufeo.Header_Deque(_buf.header.va_addr,(void*)_header_data);

                        if( _header_data[Header_UFEO::E_IMG_PA] == 0){
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
    if(ret == MFALSE){
        if(this->m_buf_cnt != 0){
            CAM_FUNC_ERR("%s, waitbufready fail. start fail check\n",str);
            this->PipeCheck();
        }
        else{
            CAM_FUNC_ERR("%s, waitbufready fail is caused by no enque\n",str);
        }
    }
    else{
        /* for log reduction, only print rrzo or imgo logs */
        cam_dmao = CAM_READ_REG(this->m_pDrv->getPhyObj(),CAM_CTL_DMA_EN);
        if (this->id() == _rrzo_)
            cur_id = _rrzo_;
        else if ((this->id() == _imgo_) && !(cam_dmao&RRZO_EN_))
            cur_id = _imgo_;
        else
            cur_id = _cam_max_;

        switch(cur_id){
            case _imgo_:
            case _rrzo_:
                CAM_FUNC_INF("%s, waitbufready pass\n",str);
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
    this->m_prvClk = 0;
    this->m_nOverFlow = 0;
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

    if(CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_TG_VF_CON,VFDATA_EN) == 1){
        CAM_FUNC_ERR("can't enable FBC at streaming\n");
        return 1;
    }

    switch(this->id()){
        case _imgo_:
            CAM_WRITE_BITS(this->m_pDrv,CAM_FBC_IMGO_CTL1,SUB_RATIO,0);
            CAM_WRITE_BITS(this->m_pDrv,CAM_FBC_IMGO_CTL1,FBC_EN,1);
            break;
        case _rrzo_:
            CAM_WRITE_BITS(this->m_pDrv,CAM_FBC_RRZO_CTL1,SUB_RATIO,0);
            CAM_WRITE_BITS(this->m_pDrv,CAM_FBC_RRZO_CTL1,FBC_EN,1);
            break;
        case _ufeo_:
            CAM_WRITE_BITS(this->m_pDrv,CAM_FBC_UFEO_CTL1,SUB_RATIO,0);
            CAM_WRITE_BITS(this->m_pDrv,CAM_FBC_UFEO_CTL1,FBC_EN,1);
            break;
        case _aao_:
            if(pParam == NULL) {
                CAM_FUNC_ERR("%s: Null pParam\n",this->name_Str());
                break;
            }
            CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),CAM_FBC_AAO_CTL1,SUB_RATIO,*(MUINT32*)pParam);
            CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),CAM_FBC_AAO_CTL1,FBC_EN,1);
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
            break;
        case _eiso_:
            UNI_WRITE_BITS(this->m_pDrv,CAM_UNI_FBC_EISO_A_CTL1,SUB_RATIO,0);
            UNI_WRITE_BITS(this->m_pDrv,CAM_UNI_FBC_EISO_A_CTL1,FBC_EN,1);
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
        CAM_FUNC_INF("warning_%s:disable FBC at streaming\n",this->name_Str());
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
        default:
            CAM_FUNC_ERR("un-supported DMAO:0x%x\n",this->id());
            return 1;
            break;
    }

    return 0;
}

MBOOL CAM_BUF_CTRL::enqueueHwBuf( stISP_BUF_INFO& buf_info,MBOOL bImdMode)
{
    ST_BUF_INFO* ptr = NULL;
    E_ISP_CAM_CQ cq;
    MUINT32 page;
    MUINT32 _sof_idx;
    MUINT32 _size;
    ST_BUF_INFO _buf;
    MUINT32 cam_dmao, cur_id;

    this->m_pDrv = (ISP_DRV_CAM*)this->m_pIspDrv;
    (void)bImdMode;

    Mutex::Autolock lock(this->m_bufctrl_lock);

    //dbg log
    this->m_pDrv->getCurObjInfo(&this->m_hwModule,&cq,&page);



    if ( 0xffffffff == this->id() ) {
        CAM_FUNC_ERR("dma channel error\n");
        return MFALSE;
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
        ptr = &buf_info.u_op.enque;
    }

    //check over enque or not
    this->m_Queue.sendCmd(CAM_BUF_CTRL::QueueMgr<ST_BUF_INFO>::eCmd_getsize,(MUINTPTR)&_size,0);
    for(MUINT32 i=0;i<_size;i++){
        if(MFALSE == this->m_Queue.sendCmd(CAM_BUF_CTRL::QueueMgr<ST_BUF_INFO>::eCmd_at,i,(MUINTPTR)&_buf))
            CAM_FUNC_ERR("%s: get no element at %d\n",this->name_Str(),i);
        else{
            if(_buf.image.mem_info.pa_addr == ptr->image.mem_info.pa_addr){
                CAM_FUNC_ERR("%s: over enque at %d.[addr:0x%x]\n",this->name_Str(),i,_buf.image.mem_info.pa_addr);
                return MFALSE;
            }
        }
    }

    //
    this->m_Queue.sendCmd(CAM_BUF_CTRL::QueueMgr<ST_BUF_INFO>::eCmd_push,(MUINTPTR)ptr,0);

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
    //
    this->m_buf_cnt++;
    this->m_tail_idx = (this->m_buf_cnt + 1) % MAX_DEPTH;

    //
    switch(this->id()){
        case _imgo_:
            {
                Header_IMGO fh_imgo;
                CAM_WRITE_BITS(this->m_pDrv,CAM_CTL_FBC_RCNT_INC,IMGO_RCNT_INC,1);
                //
                fh_imgo.Header_Enque(Header_IMGO::E_ENQUE_SOF,this->m_pDrv,_sof_idx);
                fh_imgo.Header_Enque(Header_IMGO::E_ENQUE_CNT,this->m_pDrv,this->m_buf_cnt);
            }
            break;
        case _rrzo_:
            {
                Header_RRZO fh_rrzo;
                CAM_WRITE_BITS(this->m_pDrv,CAM_CTL_FBC_RCNT_INC,RRZO_RCNT_INC,1);
                //
                fh_rrzo.Header_Enque(Header_RRZO::E_ENQUE_SOF,this->m_pDrv,_sof_idx);
                fh_rrzo.Header_Enque(Header_RRZO::E_ENQUE_CNT,this->m_pDrv,this->m_buf_cnt);
            }
            break;
        case _ufeo_:
            {
                Header_UFEO fh_ufeo;
                CAM_WRITE_BITS(this->m_pDrv,CAM_CTL_FBC_RCNT_INC,UFEO_RCNT_INC,1);
                //
                fh_ufeo.Header_Enque(Header_UFEO::E_ENQUE_SOF,this->m_pDrv,_sof_idx);
                fh_ufeo.Header_Enque(Header_UFEO::E_ENQUE_CNT,this->m_pDrv,this->m_buf_cnt);
            }
            break;
        case _aao_:
            {
                Header_AAO  fh_aao;
                CQ_RingBuf_ST Buf_ctrl;

                Buf_ctrl.ctrl = GET_RING_DEPTH;
                if(this->m_pDrv->cqRingBuf(&Buf_ctrl) == 0){
                    CAM_FUNC_ERR("%s, can't enque before PA push into CQ\n",this->name_Str());
                    return MFALSE;
                }

                //statistic pipe enque to hw directly
                if(CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_FBC_AAO_CTL1,FBC_EN) != 1){
                    CAM_FUNC_ERR("%s: can't enque before enabe fbc\n",this->name_Str());
                    return MFALSE;
                }
                CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),CAM_CTL_FBC_RCNT_INC,AAO_RCNT_INC,1);
                //
                Buf_ctrl.ctrl = SET_FH_SPARE;
                Buf_ctrl.dma_PA = ptr->image.mem_info.pa_addr;
                this->m_pDrv->cqRingBuf(&Buf_ctrl);
                fh_aao.Header_Enque(Header_AAO::E_ENQUE_SOF,(ISP_DRV_CAM*)Buf_ctrl.pDma_fh_spare,_sof_idx);
            }
            break;
        case _afo_:
            {
                Header_AFO  fh_afo;
                CQ_RingBuf_ST Buf_ctrl;

                Buf_ctrl.ctrl = GET_RING_DEPTH;
                if(this->m_pDrv->cqRingBuf(&Buf_ctrl) == 0){
                    CAM_FUNC_ERR("%s, can't enque before PA push into CQ\n",this->name_Str());
                    return MFALSE;
                }
                //statistic pipe enque to hw directly
                if(CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_FBC_AFO_CTL1,FBC_EN) != 1){
                    CAM_FUNC_ERR("%s: can't enque before enabe fbc\n",this->name_Str());
                    return MFALSE;
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
                this->m_pDrv->cqRingBuf(&Buf_ctrl);
                fh_afo.Header_Enque(Header_AFO::E_ENQUE_SOF,(ISP_DRV_CAM*)Buf_ctrl.pDma_fh_spare,_sof_idx);

            }
            break;
        case _pdo_:
            {
                Header_PDO  fh_pdo;
                CQ_RingBuf_ST Buf_ctrl;

                Buf_ctrl.ctrl = GET_RING_DEPTH;
                if(this->m_pDrv->cqRingBuf(&Buf_ctrl) == 0){
                    CAM_FUNC_ERR("%s, can't enque before PA push into CQ\n",this->name_Str());
                    return MFALSE;
                }
                //statistic pipe enque to hw directly
                if(CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_FBC_PDO_CTL1,FBC_EN) != 1){
                    CAM_FUNC_ERR("%s: can't enque before enabe fbc\n",this->name_Str());
                    return MFALSE;
                }
                CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),CAM_CTL_FBC_RCNT_INC,PDO_A_RCNT_INC,1);
                //
                Buf_ctrl.ctrl = SET_FH_SPARE;
                Buf_ctrl.dma_PA = ptr->image.mem_info.pa_addr;
                this->m_pDrv->cqRingBuf(&Buf_ctrl);
                fh_pdo.Header_Enque(Header_PDO::E_ENQUE_SOF,(ISP_DRV_CAM*)Buf_ctrl.pDma_fh_spare,_sof_idx);

            }
            break;
        case _lcso_:
            {
                Header_LCSO fh_lcso;
                if(CAM_READ_BITS(this->m_pDrv,CAM_FBC_LCSO_CTL1,FBC_EN) != 1){
                    CAM_FUNC_ERR("%s: can't enque before enabe fbc\n",this->name_Str());
                    return MFALSE;
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
                if(UNI_READ_BITS(this->m_pDrv,CAM_UNI_FBC_EISO_A_CTL1,FBC_EN) != 1){
                    CAM_FUNC_ERR("%s: can't enque before enabe fbc\n",this->name_Str());
                    return MFALSE;
                }
                CAM_WRITE_BITS(this->m_pDrv,CAM_CTL_FBC_RCNT_INC,EISO_RCNT_INC,1);
                //
                fh_eiso.Header_Enque(Header_EISO::E_ENQUE_SOF,this->m_pDrv,_sof_idx);
                fh_eiso.Header_Enque(Header_EISO::E_ENQUE_CNT,this->m_pDrv,this->m_buf_cnt);
            }
            break;
        case _flko_:
            {
                Header_FLKO fh_flko;
                CQ_RingBuf_ST Buf_ctrl;

                Buf_ctrl.ctrl = GET_RING_DEPTH;
                if(this->m_pDrv->cqRingBuf(&Buf_ctrl) == 0){
                    CAM_FUNC_ERR("%s, can't enque before PA push into CQ\n",this->name_Str());
                    return MFALSE;
                }

                //statistic pipe enque to hw directly
                if(UNI_READ_BITS(this->m_pUniDrv,CAM_UNI_FBC_FLKO_A_CTL1,FBC_EN) != 1){
                    CAM_FUNC_ERR("%s: can't enque before enabe fbc\n",this->name_Str());
                    return MFALSE;
                }
                CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),CAM_CTL_FBC_RCNT_INC,FLKO_RCNT_INC,1);
                //
                Buf_ctrl.ctrl = SET_FH_SPARE;
                Buf_ctrl.dma_PA = ptr->image.mem_info.pa_addr;
                this->m_pDrv->cqRingBuf(&Buf_ctrl);
                fh_flko.Header_Enque(Header_FLKO::E_ENQUE_SOF,(ISP_DRV_CAM*)Buf_ctrl.pDma_fh_spare,_sof_idx);

            }
            break;
        case _rsso_:
            {
                Header_RSSO fh_rsso;
                if(UNI_READ_BITS(this->m_pUniDrv,CAM_UNI_FBC_RSSO_A_CTL1,FBC_EN) != 1){
                    CAM_FUNC_ERR("%s: can't enque before enabe fbc\n",this->name_Str());
                    return MFALSE;
                }
                CAM_WRITE_BITS(this->m_pDrv,CAM_CTL_FBC_RCNT_INC,RSSO_RCNT_INC,1);
                //
                fh_rsso.Header_Enque(Header_RSSO::E_ENQUE_SOF,this->m_pDrv,_sof_idx);
            }
            break;
        default:
            CAM_FUNC_ERR("un-supported DMAO:0x%x\n",this->id());
            this->m_Queue.sendCmd(CAM_BUF_CTRL::QueueMgr<ST_BUF_INFO>::eCmd_pop  ,0,0);
            return MFALSE;
            break;
    }

    /* for log reduction, only print rrzo or imgo logs */
    cam_dmao = CAM_READ_REG(this->m_pDrv->getPhyObj(),CAM_CTL_DMA_EN);
    if (this->id() == _rrzo_)
        cur_id = _rrzo_;
    else if ((this->id() == _imgo_)&& !(cam_dmao&RRZO_EN_))
        cur_id = _imgo_;
    else
        cur_id = _cam_max_;
    //
    if(buf_info.bReplace == MFALSE){
        switch(cur_id){ /*this->id()*/
            case _imgo_:
            case _rrzo_:
                CAM_FUNC_INF("%s:(0x%x_0x%x),PA(0x%x_0x%x),FH_VA(0x%x),size(0x%x),enque_sof(%d)",\
                    this->name_Str(),\
                    cq,page,\
                    ptr->image.mem_info.pa_addr ,\
                    ptr->header.pa_addr ,\
                    ptr->header.va_addr ,\
                    ptr->image.mem_info.size ,\
                    _sof_idx);
                break;
            default:
                CAM_FUNC_DBG("%s:(0x%x_0x%x),PA(0x%x_0x%x),FH_VA(0x%x),size(0x%x),enque_sof(%d)",\
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
        switch(cur_id){
            case _imgo_:
            case _rrzo_:
                CAM_FUNC_INF("%s:(0x%x_0x%x),PA(0x%x_0x%x,new:0x%x_0x%x),FH_VA(0x%x,new:0x%x),size(0x%x),enque_sof(%d)",\
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
                CAM_FUNC_DBG("%s:(0x%x_0x%x),PA(0x%x_0x%x,new:0x%x_0x%x),FH_VA(0x%x,new:0x%x),size(0x%x),enque_sof(%d)",\
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

    this->m_Queue_deque_ptr.sendCmd(CAM_BUF_CTRL::QueueMgr<MUINTPTR>::eCmd_push,(MUINTPTR)&this->m_pDrv,0);
    //
    return MTRUE;

}



/* return value:
 1: isp is already stopped
 0: sucessed
-1: fail */
MINT32 CAM_BUF_CTRL::dequeueHwBuf(NSImageio::NSIspio::BufInfo& buf_info )
{
    MINT32 ret = 0;
    E_ISP_CAM_CQ cq;
    MUINT32 page;
    MUINT32 _size;
    MUINT32 _sof_idx;
    ST_BUF_INFO _buf;
    MUINT32 _header_data[E_HEADER_MAX];
    MUINT32 cam_dmao, cur_id;
    char str[128] = {'\0'};
    ISP_DRV_CAM* pDrv = NULL;
    Mutex::Autolock lock(this->m_bufctrl_lock);


    if ( 0xffffffff == this->id() ) {
        CAM_FUNC_ERR("dma channel error ");
        return -1;
    }

    this->m_Queue_deque_ptr.sendCmd(CAM_BUF_CTRL::QueueMgr<MUINTPTR>::eCmd_front  ,(MUINTPTR)&pDrv,0);


    //dbg log
    pDrv->getCurObjInfo(&this->m_hwModule,&cq,&page);

    //fbc reg dbg log
    this->FBC_STATUS(pDrv->getPhyObj());
    //
    this->m_Queue.sendCmd(CAM_BUF_CTRL::QueueMgr<ST_BUF_INFO>::eCmd_getsize,(MUINTPTR)&_size,0);
    if(_size <1){
        CAM_FUNC_ERR("queue depth is empty for deque\n");
        return -1;
    }
    //
    this->m_Queue.sendCmd(CAM_BUF_CTRL::QueueMgr<ST_BUF_INFO>::eCmd_front  ,(MUINTPTR)&_buf,0);

    //
    pDrv->getDeviceInfo(_GET_SOF_CNT,(MUINT8 *) &_sof_idx);

    //
    this->Header_Parser(_buf.header.va_addr, buf_info,_header_data,str);
    if( _buf.image.mem_info.pa_addr != buf_info.u4BufPA[0]){
        if(CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_TG_VF_CON,VFDATA_EN) == 0){
            CAM_FUNC_INF("%s:VF_EN=0, header mismatch can be ignored.",this->name_Str());
            ret = 1;
            goto EXIT;
        }
        CAM_FUNC_ERR("%s:PA in header is mismatch with PA in list [0x%x_0x%x]\n",this->name_Str(),\
            buf_info.u4BufPA[0],_buf.image.mem_info.pa_addr);
        ret = -1;
        this->PipeCheck();
        goto EXIT;
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
    this->m_Queue.sendCmd(CAM_BUF_CTRL::QueueMgr<ST_BUF_INFO>::eCmd_pop  ,0,0);
    this->m_Queue_deque_ptr.sendCmd(CAM_BUF_CTRL::QueueMgr<MUINTPTR>::eCmd_pop  ,0,0);

    //ion handle: buffer
    ISP_DEV_ION_NODE_STRUCT IonNode;
    IonNode.devNode = this->m_hwModule;
    IonNode.memID =  _buf.image.mem_info.memID;
    IonNode.dmaPort = _DMA_Mapping(this->id());
    this->m_pDrv->setDeviceInfo(_SET_ION_FREE,(MUINT8*)&IonNode);
    //ion handle: header
    IonNode.memID =  _buf.header.memID;
    IonNode.dmaPort = _DMA_Header_Mapping(this->id());
    this->m_pDrv->setDeviceInfo(_SET_ION_FREE,(MUINT8*)&IonNode);

EXIT:
    /* for log reduction, only print rrzo or imgo logs */
    cam_dmao = CAM_READ_REG(this->m_pDrv->getPhyObj(),CAM_CTL_DMA_EN);
    if (this->id() == _rrzo_)
        cur_id = _rrzo_;
    else if ((this->id() == _imgo_)&& !(cam_dmao&RRZO_EN_))
        cur_id = _imgo_;
    else
        cur_id = _cam_max_;

    switch(cur_id){
        case _imgo_:
        case _rrzo_:
            CAM_FUNC_INF("%s:(0x%x_0x%x),PA(0x%x_0x%x),FH_VA(0x%x),size(0x%x),cur sof(%d),%s\n",\
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
            CAM_FUNC_DBG("%s:(0x%x_0x%x),PA(0x%x_0x%x),FH_VA(0x%x),size(0x%x),cur sof(%d),%s\n",\
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
    this->m_buf_cnt--;

    return ret;
}

MBOOL CAM_BUF_CTRL::TimeStamp_mapping(MUINT32* pSec, MUINT32* pUsec)
{
    MBOOL ret = MTRUE;
    MUINT32 start_time[3];
    MUINT64 time;

    //overflow case
    if(*pSec < this->m_prvClk){
        this->m_nOverFlow++;
    }

    this->m_prvClk = *pSec;

    start_time[0] = this->id();//pass dma_id to kernel

    if( (ret = this->m_pDrv->getDeviceInfo(_GET_START_TIME,(MUINT8*) start_time)) == MFALSE){
        CAM_FUNC_ERR("timestamp calibration fail\n");
    }
    time = (MUINT64)start_time[0] * 1000000 + start_time[1];

    //current time-axis
    time += ( ((MUINT64)(*pSec) + (MUINT64)this->m_nOverFlow * 0xFFFFFFFF) * 10 / this->m_TimeClk ); //us
    //CAM_FUNC_ERR("time stamp test:%u_%u_%lld\n",*pSec,this->m_nOverFlow,time);

    *pUsec = time % 1000000;
    if((time/1000000) > 0xffffffff)
        CAM_FUNC_INF("overflow in sec\n");

    *pSec = time/1000000;


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

void BUF_CTRL_IMGO::FBC_STATUS(IspDrv* ptr)
{
    CAM_FUNC_DBG("BUF_CTRL_IMGO::FBC_STATUS: 0x%x_0x%x",CAM_READ_REG(ptr,CAM_FBC_IMGO_CTL1),CAM_READ_REG(ptr,CAM_FBC_IMGO_CTL2));
}


void BUF_CTRL_IMGO::Header_Parser(MUINTPTR fh_va,NSImageio::NSIspio::BufInfo& buf_info,MUINT32* header_data ,char* pStr)
{
    Header_IMGO fh_imgo;

    CAM_REG_TG_SEN_GRAB_PXL TG_W;
    CAM_REG_TG_SEN_GRAB_LIN TG_H;
    //
    fh_imgo.Header_Deque(fh_va,(void*)header_data);
    fh_imgo.Header_Flash(fh_va);
    //
    buf_info.u4BufPA[0] = header_data[Header_IMGO::E_IMG_PA];
    buf_info.m_num = header_data[Header_IMGO::E_Magic];
    buf_info.img_fmt = header_data[Header_IMGO::E_FMT];        //hw define, need to transfer to user-reconized enum in camiopipe
    buf_info.img_pxl_id = header_data[Header_IMGO::E_PIX_ID];  //hw define, need to transfer to user-reconized enum in camiopipe
    buf_info.frm_cnt = 0;

    TG_W.Raw = CAM_READ_REG(this->m_pDrv->getPhyObj(),CAM_TG_SEN_GRAB_PXL);
    TG_H.Raw = CAM_READ_REG(this->m_pDrv->getPhyObj(),CAM_TG_SEN_GRAB_LIN);
    buf_info.img_w = TG_W.Bits.PXL_E - TG_W.Bits.PXL_S;
    buf_info.img_w >>= (CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_CTL_EN,DBN_EN));
    buf_info.img_h = TG_H.Bits.LIN_E - TG_H.Bits.LIN_S;

    buf_info.crop_win.p.x = (header_data[Header_IMGO::E_CROP_START]& 0xffff);
    buf_info.crop_win.p.y = (header_data[Header_IMGO::E_CROP_START] >> 16);
    buf_info.crop_win.s.w = (header_data[Header_IMGO::E_CROP_SIZE]& 0xffff);
    buf_info.crop_win.s.h = (header_data[Header_IMGO::E_CROP_SIZE] >> 16);

    buf_info.DstSize.w = buf_info.crop_win.s.w;
    buf_info.DstSize.h = buf_info.crop_win.s.h;

    buf_info.i4TimeStamp_sec = header_data[Header_IMGO::E_TimeStamp];
    this->TimeStamp_mapping(&buf_info.i4TimeStamp_sec,&buf_info.i4TimeStamp_us);

    buf_info.raw_type = header_data[Header_IMGO::E_RAW_TYPE];  //hw define, need to transfer to user-reconized enum in camiopipe
    buf_info.xoffset = 0;
    buf_info.yoffset = 0;
    //
    if(pStr){
        sprintf(pStr,"enq_sof:0x%x,mag:0x%x,timestamp:%u.%u,RawType:0x%x",\
            header_data[Header_IMGO::E_ENQUE_SOF],\
            header_data[Header_IMGO::E_Magic], \
            buf_info.i4TimeStamp_sec, \
            buf_info.i4TimeStamp_us,\
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
    this->TimeStamp_mapping(&buf_info.i4TimeStamp_sec,&buf_info.i4TimeStamp_us);

    buf_info.raw_type = 0;
    buf_info.xoffset = 0;
    buf_info.yoffset = 0;

    //
    if(pStr){
        sprintf(pStr,"enq_sof:0x%x,mag:0x%x,timestamp:%u.%u",\
            header_data[Header_RRZO::E_ENQUE_SOF],\
            header_data[Header_RRZO::E_Magic], \
            buf_info.i4TimeStamp_sec, \
            buf_info.i4TimeStamp_us);
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
    this->TimeStamp_mapping(&buf_info.i4TimeStamp_sec,&buf_info.i4TimeStamp_us);

    buf_info.raw_type = 0;
    buf_info.xoffset = 0;
    buf_info.yoffset = 0;

    //
    if(pStr){
        sprintf(pStr,"enq_sof:0x%x,mag:0x%x,timestamp:%u.%u",\
            header_data[Header_UFEO::E_ENQUE_SOF],\
            header_data[Header_UFEO::E_Magic], \
            buf_info.i4TimeStamp_sec, \
            buf_info.i4TimeStamp_us);
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
    this->TimeStamp_mapping(&buf_info.i4TimeStamp_sec,&buf_info.i4TimeStamp_us);

    //
    if(pStr){
        sprintf(pStr,"enq_sof:0x%x,mag:0x%x,timestamp:%u.%u",\
            header_data[Header_LCSO::E_ENQUE_SOF],\
            header_data[Header_LCSO::E_Magic], \
            buf_info.i4TimeStamp_sec, \
            buf_info.i4TimeStamp_us);
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
    this->TimeStamp_mapping(&buf_info.i4TimeStamp_sec,&buf_info.i4TimeStamp_us);

    //
    if(pStr){
        sprintf(pStr,"enq_sof:0x%x,mag:0x%x,timestamp:%u.%u",\
            header_data[Header_EISO::E_ENQUE_SOF],\
            header_data[Header_EISO::E_Magic], \
            buf_info.i4TimeStamp_sec, \
            buf_info.i4TimeStamp_us);
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
    this->TimeStamp_mapping(&buf_info.i4TimeStamp_sec,&buf_info.i4TimeStamp_us);

    //
    if(pStr){
        sprintf(pStr,"enq_sof:0x%x,mag:0x%x,timestamp:%u.%u",\
            header_data[Header_RSSO::E_ENQUE_SOF],\
            header_data[Header_RSSO::E_Magic], \
            buf_info.i4TimeStamp_sec, \
            buf_info.i4TimeStamp_us);
    }
}


MBOOL BUF_CTRL_AAO::waitBufReady(CAM_STATE_NOTIFY *pNotify)
{
#define _LOOP_  (5)
    MBOOL ret = MTRUE;
    E_ISP_CAM_CQ cq;
    MUINT32 page;

    ISP_WAIT_IRQ_ST irq;
    MUINT32 loopCnt = _LOOP_;
    char str[128] = {'\0'};

    MUINT32 FbcCnt;
    MUINT32 subSample;


    this->m_pDrv = (ISP_DRV_CAM*)this->m_pIspDrv;

    //dbg log
    this->m_pDrv->getCurObjInfo(&this->m_hwModule,&cq,&page);
    if ( 0xffffffff == this->id() ) {
        CAM_FUNC_ERR("dma channel error \n");
        return MFALSE;
    }

    //fbc reg dbg log
    this->FBC_STATUS(this->m_pDrv->getPhyObj());

    subSample = CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_CTL_SW_PASS1_DONE,DOWN_SAMPLE_PERIOD) + 1;

    irq.Clear = ISP_IRQ_CLEAR_WAIT;
    irq.UserKey = 0;
    irq.St_type = DMA_INT;
    irq.Status = (MUINT32)AAO_DONE_ST;

    do{
        //irq.Timeout = ((1000 + (this->m_fps - 1)) / this->m_fps) * subSample;
        //irq.Timeout *= 4; //amplify 3 times is for the case of enque latch timing
        irq.Timeout = this->estimateTimeout(subSample);

        if(this->m_buf_cnt != 0){

            //
            IspFunction_B::fbcPhyLock_AAO[this->m_hwModule].lock();
            FbcCnt = CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_FBC_AAO_CTL2,FBC_CNT);
            IspFunction_B::fbcPhyLock_AAO[this->m_hwModule].unlock();

            if(FbcCnt< this->m_buf_cnt){
                if(loopCnt == _LOOP_){
                    // for SW timestamp queue ready
                    MUINT32 waitOps[3];

                    waitOps[0] = _cam_max_;
                    waitOps[1] = this->id();

                    if (this->m_pDrv->getDeviceInfo(_GET_START_TIME, (MUINT8*)waitOps) == MFALSE) {
                        CAM_FUNC_ERR("Wait timestamp ready fail\n");
                    }

                    CAM_FUNC_DBG("dma:%s already have avail_buf on dram, bWaitBufReady  = 0\n",this->name_Str());
                    return MTRUE;
                }
                else{
                    ret = MTRUE;
                    goto EXIT;
                }
            }
            else{
                if(loopCnt == _LOOP_)
                    sprintf(str,"dma:%s start wait:[enque record:0x%x_0x%x],",this->name_Str(),this->m_buf_cnt,FbcCnt);
                else{
                    char _tmp[16];
                    sprintf(_tmp,"[%d]time,",((_LOOP_-loopCnt)+1));
                    strcat(str,_tmp);
                }
            }
        }
        else{
            if(loopCnt == _LOOP_){
                sprintf(str,"dma:%s bWaitBufReady = 1,wait for sw enque[0x%x],timeout:%d x %d ms,[1] ",this->name_Str(),this->m_buf_cnt,irq.Timeout,loopCnt);
            }
            else{//save how many times of wait_irq before sw enque
                char _tmp[16];
                sprintf(_tmp,"[%d],",((_LOOP_-loopCnt)+1)+1);
                strcat(str,_tmp);
            }
        }

        //
        if (pNotify) {
            pNotify->fpNotifyState(CAM_ENTER_WAIT, pNotify->Obj);
        }

        ret = this->m_pDrv->waitIrq(&irq);

        if (pNotify) {
            pNotify->fpNotifyState(CAM_EXIT_WAIT, pNotify->Obj);
        }
        /* no matter waitIrq is timeout or not, VF_EN=0 is no need to go further...*/
        if(CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_TG_VF_CON,VFDATA_EN) == 0){
            CAM_FUNC_INF("%s:VF_EN=0, %s", this->name_Str(),str);
            return MFALSE;
        }

        if(ret == MTRUE)
            break;
        //
        irq.Clear = ISP_IRQ_CLEAR_NONE;

    }while(--loopCnt > 0);
EXIT:

    if(ret == MFALSE){
        if(this->m_buf_cnt != 0){
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
    this->TimeStamp_mapping(&buf_info.i4TimeStamp_sec,&buf_info.i4TimeStamp_us);

    //
    if(pStr){
        sprintf(pStr,"enq_sof:0x%x,mag:0x%x,timestamp:%u.%u",\
            header_data[Header_AAO::E_ENQUE_SOF],\
            header_data[Header_AAO::E_Magic], \
            buf_info.i4TimeStamp_sec, \
            buf_info.i4TimeStamp_us);
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
    this->TimeStamp_mapping(&buf_info.i4TimeStamp_sec,&buf_info.i4TimeStamp_us);

    buf_info.img_stride = header_data[Header_AFO::E_STRIDE];
    //
    if(pStr){
        sprintf(pStr,"enq_sof:0x%x,mag:0x%x,timestamp:%u.%u,stride:0x%x",\
            header_data[Header_AFO::E_ENQUE_SOF],\
            header_data[Header_AFO::E_Magic], \
            buf_info.i4TimeStamp_sec, \
            buf_info.i4TimeStamp_us,
            buf_info.img_stride);
    }
}

MBOOL BUF_CTRL_AFO::waitBufReady( MBOOL bClear, CAM_STATE_NOTIFY *pNotify)
{
    #define _LOOP_  (5)
    MBOOL ret = MTRUE;
    E_ISP_CAM_CQ cq;
    MUINT32 page;

    ISP_WAIT_IRQ_ST irq;
    MUINT32 loopCnt = _LOOP_;
    char str[128] = {'\0'};

    MUINT32 FbcCnt;
    MUINT32 _header_data[E_HEADER_MAX];
    MUINT32 _pa;
    Header_AFO fh_afo;
    ST_BUF_INFO _buf;
    MUINT32 _size;
    MUINT32 subSample;
    this->m_Queue_deque_ptr.sendCmd(CAM_BUF_CTRL::QueueMgr<MUINTPTR>::eCmd_getsize,(MUINTPTR)&_size,0);
    if(_size <1){
        CAM_FUNC_ERR("queue depth is empty for deque\n");
        return MFALSE;
    }

    this->m_pDrv = (ISP_DRV_CAM*)this->m_pIspDrv;

    //dbg log
    this->m_pDrv->getCurObjInfo(&this->m_hwModule,&cq,&page);
    ;
    if ( 0xffffffff == this->id() ) {
        CAM_FUNC_ERR("dma channel error \n");
        return MFALSE;
    }

    //fbc reg dbg log
    this->FBC_STATUS(this->m_pDrv->getPhyObj());

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

        if(this->m_buf_cnt != 0){

            //
            this->m_Queue.sendCmd(CAM_BUF_CTRL::QueueMgr<ST_BUF_INFO>::eCmd_at,0,(MUINTPTR)&_buf);
            IspFunction_B::fbcPhyLock_AFO[this->m_hwModule].lock();
            FbcCnt = CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_FBC_AFO_CTL2,FBC_CNT);
            IspFunction_B::fbcPhyLock_AFO[this->m_hwModule].unlock();
            //af need to check header is because of fbccnt is updated at p1_don, not af don
            fh_afo.Header_Deque(_buf.header.va_addr,(void*)_header_data);
            _pa = _header_data[Header_AFO::E_IMG_PA];

            if( (FbcCnt < this->m_buf_cnt)||(_pa != 0)){
                if(loopCnt == _LOOP_){
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

                    CAM_FUNC_DBG("dma:%s already have avail_buf on dram, bWaitBufReady  = 0\n",this->name_Str());
                    return MTRUE;
                }
                else{
                    ret = MTRUE;
                    goto EXIT;
                }
            }
            else{
                if(loopCnt == _LOOP_)
                    sprintf(str,"dma:%s _pa:0x%x start wait:[enque record:0x%x_0x%x],",this->name_Str(), _pa, this->m_buf_cnt,FbcCnt);
                else{
                    char _tmp[16];
                    sprintf(_tmp,"[%d]time,",((_LOOP_-loopCnt)+1));
                    strcat(str,_tmp);
                }
            }
        }
        else{
            if(loopCnt == _LOOP_){
                sprintf(str,"dma:%s bWaitBufReady = 1,wait for sw enque[0x%x],timeout:%d x %d ms,[1] ",\
                    this->name_Str(),this->m_buf_cnt,irq.Timeout,loopCnt);
            }
            else{//save how many times of wait_irq before sw enque
                char _tmp[16];
                sprintf(_tmp,"[%d],",((_LOOP_-loopCnt)+1)+1);
                strcat(str,_tmp);
            }
        }

        //
        if (pNotify) {
            pNotify->fpNotifyState(CAM_ENTER_WAIT, pNotify->Obj);
        }

        ret = this->m_pDrv->waitIrq(&irq);

        if (pNotify) {
            pNotify->fpNotifyState(CAM_EXIT_WAIT, pNotify->Obj);
        }

        if (!(CAM_READ_BITS(this->m_pDrv->getPhyObj(), CAM_CTL_TWIN_STATUS,TWIN_EN) &&
            (this->m_hwModule == CAM_READ_BITS(this->m_pDrv->getPhyObj(), CAM_CTL_TWIN_STATUS, TWIN_MODULE)))) {
            /* no matter waitIrq is timeout or not, VF_EN=0 is no need to go further...*/
            if(CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_TG_VF_CON,VFDATA_EN) == 0){
                CAM_FUNC_INF("%s:VF_EN=0, %s", this->name_Str(),str);
                return MFALSE;
            }
        }

        if(ret == MTRUE)
            break;
        //
        irq.Clear = ISP_IRQ_CLEAR_NONE;

    }while(--loopCnt > 0);
EXIT:

    if(ret == MFALSE){
        if(this->m_buf_cnt != 0){
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
#if PDO_SW_WORK_AROUND
    if(CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_CTL_TWIN_STATUS,TWIN_EN)){
        if(this->m_bClear == MTRUE){
            fh_pdo.Header_Flash(fh_va);
        }
    }
    else
        fh_pdo.Header_Flash(fh_va);
#else
    fh_pdo.Header_Flash(fh_va);
#endif
    //
    buf_info.u4BufPA[0] = header_data[Header_PDO::E_IMG_PA];
    buf_info.m_num = header_data[Header_PDO::E_Magic];
    buf_info.img_stride = header_data[Header_PDO::E_STRIDE];
    buf_info.u4BufOffset[0] = header_data[Header_PDO::E_IMG_PA_OFSET];

    //
    if(buf_info.m_pPrivate != NULL){
#if PDO_SW_WORK_AROUND
        ((MRect*)buf_info.m_pPrivate)[0].p.x = header_data[Header_PDO::E_PMX_A_CROP]&0xFFFF;
        ((MRect*)buf_info.m_pPrivate)[0].p.y = (header_data[Header_PDO::E_PMX_A_CROP]>>16);

        ((MRect*)buf_info.m_pPrivate)[1].p.x = header_data[Header_PDO::E_PMX_B_CROP]&0xFFFF;
        ((MRect*)buf_info.m_pPrivate)[1].p.y = (header_data[Header_PDO::E_PMX_B_CROP]>>16);

        //
        ((MRect*)buf_info.m_pPrivate)[2].p.x = header_data[Header_PDO::E_BMX_A_CROP]&0xFFFF;
        ((MRect*)buf_info.m_pPrivate)[2].p.y = (header_data[Header_PDO::E_BMX_A_CROP]>>16);

        ((MRect*)buf_info.m_pPrivate)[3].p.x = header_data[Header_PDO::E_BMX_B_CROP]&0xFFFF;
        ((MRect*)buf_info.m_pPrivate)[3].p.y = (header_data[Header_PDO::E_BMX_B_CROP]>>16);
#else
        ((MRect*)buf_info.m_pPrivate)[0].p.x = header_data[Header_PDO::E_PMX_A_CROP]&0xFFFF;
        ((MRect*)buf_info.m_pPrivate)[0].p.y = (header_data[Header_PDO::E_PMX_A_CROP]>>16);
        //
        ((MRect*)buf_info.m_pPrivate)[1].p.x = header_data[Header_PDO::E_BMX_A_CROP]&0xFFFF;
        ((MRect*)buf_info.m_pPrivate)[1].p.y = (header_data[Header_PDO::E_BMX_A_CROP]>>16);
#endif


    }
    buf_info.i4TimeStamp_sec = header_data[Header_PDO::E_TimeStamp];
    this->TimeStamp_mapping(&buf_info.i4TimeStamp_sec,&buf_info.i4TimeStamp_us);

    //
    if(pStr){
        sprintf(pStr,"enq_sof:0x%x,mag:0x%x,timestamp:%u.%u",\
            header_data[Header_PDO::E_ENQUE_SOF],\
            header_data[Header_PDO::E_Magic], \
            buf_info.i4TimeStamp_sec, \
            buf_info.i4TimeStamp_us);
    }
}


MBOOL BUF_CTRL_PDO::waitBufReady( MBOOL bClear, CAM_STATE_NOTIFY *pNotify)
{
    #define _LOOP_  (5)
    MBOOL ret = MTRUE;
    E_ISP_CAM_CQ cq;
    MUINT32 page;

    Header_PDO  fh_pdo;

    ISP_WAIT_IRQ_ST irq;
    MUINT32 loopCnt = _LOOP_;
    char str[128] = {'\0'};

    MUINT32 FbcCnt;
    MUINT32 _header_data[E_HEADER_MAX];
    MUINT32 _pa;
    ST_BUF_INFO _buf;
    MUINT32 _size;
    MUINT32 subSample;
    this->m_Queue_deque_ptr.sendCmd(CAM_BUF_CTRL::QueueMgr<MUINTPTR>::eCmd_getsize,(MUINTPTR)&_size,0);
    if(_size <1){
        CAM_FUNC_ERR("queue depth is empty for deque\n");
        return MFALSE;
    }

    this->m_pDrv = (ISP_DRV_CAM*)this->m_pIspDrv;

    //dbg log
    this->m_pDrv->getCurObjInfo(&this->m_hwModule,&cq,&page);
    if ( 0xffffffff == this->id() ) {
        CAM_FUNC_ERR("dma channel error \n");
        return MFALSE;
    }

    //fbc reg dbg log
    this->FBC_STATUS(this->m_pDrv->getPhyObj());

    subSample = CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_CTL_SW_PASS1_DONE,DOWN_SAMPLE_PERIOD) + 1;

    this->m_bClear = (bClear == MTRUE)?(MFALSE):(MTRUE);
#if PDO_SW_WORK_AROUND
    if(bClear == MTRUE)
        irq.Clear = ISP_IRQ_CLEAR_WAIT;
    else
        irq.Clear = ISP_IRQ_CLEAR_NONE;
#else
    irq.Clear = ISP_IRQ_CLEAR_WAIT;
#endif

    irq.UserKey = 0;
    irq.St_type = DMA_INT;
    irq.Status = (MUINT32)PDO_DONE_ST;

    do{
        //irq.Timeout = ((1000 + (this->m_fps - 1)) / this->m_fps) * subSample;
        //irq.Timeout *= 4; //amplify 3 times is for the case of enque latch timing
        irq.Timeout = this->estimateTimeout(subSample);

        if(this->m_buf_cnt != 0){

            //
            this->m_Queue.sendCmd(CAM_BUF_CTRL::QueueMgr<ST_BUF_INFO>::eCmd_at,0,(MUINTPTR)&_buf);
            IspFunction_B::fbcPhyLock_PDO[this->m_hwModule].lock();
            FbcCnt = CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_FBC_PDO_CTL2,FBC_CNT);
            IspFunction_B::fbcPhyLock_PDO[this->m_hwModule].unlock();
            //pd need to check header is because of fbccnt is updated at p1_don, not pd don
            fh_pdo.Header_Deque(_buf.header.va_addr,(void*)_header_data);
            _pa = _header_data[Header_PDO::E_IMG_PA];

            if( (FbcCnt < this->m_buf_cnt)||(_pa != 0)){
                if(loopCnt == _LOOP_){
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


                    CAM_FUNC_DBG("dma:%s already have avail_buf on dram, bWaitBufReady  = 0\n",this->name_Str());
                    return MTRUE;
                }
                else{
                    ret = MTRUE;
                    goto EXIT;
                }
#if 0
                else{//sw timing just right on the boundary of p1 done signal
                    char _str[32] = "critical timing,";
                    strcpy(str,_str);
                    ret = MTRUE;
                    break;
                }
#endif
            }
            else{
                if(loopCnt == _LOOP_)
                    sprintf(str,"dma:%s start wait:[enque record:0x%x_0x%x],",this->name_Str(),this->m_buf_cnt,FbcCnt);
                else{
                    char _tmp[16];
                    sprintf(_tmp,"[%d]time,",((_LOOP_-loopCnt)+1));
                    strcat(str,_tmp);
                }
            }
        }
        else{
            if(loopCnt == _LOOP_){
                sprintf(str,"dma:%s bWaitBufReady = 1,wait for sw enque[0x%x],timeout:%d x %d ms,[1] ",this->name_Str(),this->m_buf_cnt,irq.Timeout,loopCnt);
            }
            else{//save how many times of wait_irq before sw enque
                char _tmp[16];
                sprintf(_tmp,"[%d],",((_LOOP_-loopCnt)+1)+1);
                strcat(str,_tmp);
            }
        }

        //
        if (pNotify) {
            pNotify->fpNotifyState(CAM_ENTER_WAIT, pNotify->Obj);
        }

        ret = this->m_pDrv->waitIrq(&irq);

        if (pNotify) {
            pNotify->fpNotifyState(CAM_EXIT_WAIT, pNotify->Obj);
        }

        if (!(CAM_READ_BITS(this->m_pDrv->getPhyObj(), CAM_CTL_TWIN_STATUS,TWIN_EN) &&
            (this->m_hwModule == CAM_READ_BITS(this->m_pDrv->getPhyObj(), CAM_CTL_TWIN_STATUS, TWIN_MODULE)))) {
            /* no matter waitIrq is timeout or not, VF_EN=0 is no need to go further...*/
            if(CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_TG_VF_CON,VFDATA_EN) == 0){
                CAM_FUNC_INF("%s:VF_EN=0, %s", this->name_Str(),str);
                return MFALSE;
            }
        }

        if(ret == MTRUE)
            break;
        //
        irq.Clear = ISP_IRQ_CLEAR_NONE;

    }while(--loopCnt > 0);
EXIT:

    if(ret == MFALSE){
        if(this->m_buf_cnt != 0){
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
    this->TimeStamp_mapping(&buf_info.i4TimeStamp_sec,&buf_info.i4TimeStamp_us);

    //
    if(pStr){
        sprintf(pStr,"enq_sof:0x%x,mag:0x%x,timestamp:%u.%u",\
            header_data[Header_FLKO::E_ENQUE_SOF],\
            header_data[Header_FLKO::E_Magic], \
            buf_info.i4TimeStamp_sec, \
            buf_info.i4TimeStamp_us);
    }
}

MBOOL BUF_CTRL_FLKO::waitBufReady(CAM_STATE_NOTIFY *pNotify)
{
    #define _LOOP_  (5)
    MBOOL ret = MTRUE;
    E_ISP_CAM_CQ cq;
    MUINT32 page;

    ISP_WAIT_IRQ_ST irq;
    MUINT32 loopCnt = _LOOP_;
    char str[128] = {'\0'};

    MUINT32 FbcCnt;
    MUINT32 subSample;


    this->m_pDrv = (ISP_DRV_CAM*)this->m_pIspDrv;

    //dbg log
    this->m_pDrv->getCurObjInfo(&this->m_hwModule,&cq,&page);
    if ( 0xffffffff == this->id() ) {
        CAM_FUNC_ERR("dma channel error \n");
        return MFALSE;
    }

    //fbc reg dbg log
    this->FBC_STATUS(this->m_pDrv->getPhyObj());

    subSample = CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_CTL_SW_PASS1_DONE,DOWN_SAMPLE_PERIOD) + 1;

    irq.Clear = ISP_IRQ_CLEAR_WAIT;
    irq.UserKey = 0;
    irq.St_type = DMA_INT;
    irq.Status = (MUINT32)FLKO_DONE_ST;

    do{
        //irq.Timeout = ((1000 + (this->m_fps - 1)) / this->m_fps) * subSample;
        //irq.Timeout *= 4; //amplify 3 times is for the case of enque latch timing
        irq.Timeout = this->estimateTimeout(subSample);

        if(this->m_buf_cnt != 0){

            //
            IspFunction_B::fbcPhyLock_FLKO[this->m_hwModule].lock();
            FbcCnt = UNI_READ_BITS(this->m_pUniDrv,CAM_UNI_FBC_FLKO_A_CTL2,FBC_CNT);
            IspFunction_B::fbcPhyLock_FLKO[this->m_hwModule].unlock();

            if(FbcCnt < this->m_buf_cnt){
                if(loopCnt == _LOOP_){
                    // for SW timestamp queue ready
                    MUINT32 waitOps[3];

                    waitOps[0] = _cam_max_;
                    waitOps[1] = this->id();

                    if (this->m_pDrv->getDeviceInfo(_GET_START_TIME, (MUINT8*)waitOps) == MFALSE) {
                        CAM_FUNC_ERR("Wait timestamp ready fail\n");
                    }

                    CAM_FUNC_DBG("dma:%s already have avail_buf on dram, bWaitBufReady  = 0\n",this->name_Str());
                    return MTRUE;
                }
                else{
                    ret = MTRUE;
                    goto EXIT;
                }
            }
            else{
                if(loopCnt == _LOOP_)
                    sprintf(str,"dma:%s start wait:[enque record:0x%x_0x%x],",this->name_Str(),this->m_buf_cnt,FbcCnt);
                else{
                    char _tmp[16];
                    sprintf(_tmp,"[%d]time,",((_LOOP_-loopCnt)+1));
                    strcat(str,_tmp);
                }
            }
        }
        else{
            if(loopCnt == _LOOP_){
                sprintf(str,"dma:%s bWaitBufReady = 1,wait for sw enque[0x%x],timeout:%d x %d ms,[1] ",this->name_Str(),this->m_buf_cnt,irq.Timeout,loopCnt);
            }
            else{//save how many times of wait_irq before sw enque
                char _tmp[16];
                sprintf(_tmp,"[%d],",((_LOOP_-loopCnt)+1)+1);
                strcat(str,_tmp);
            }
        }

        //
        if (pNotify) {
            pNotify->fpNotifyState(CAM_ENTER_WAIT, pNotify->Obj);
        }

        ret = this->m_pDrv->waitIrq(&irq);

        if (pNotify) {
            pNotify->fpNotifyState(CAM_EXIT_WAIT, pNotify->Obj);
        }
        /* no matter waitIrq is timeout or not, VF_EN=0 is no need to go further...*/
        if(CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_TG_VF_CON,VFDATA_EN) == 0){
            CAM_FUNC_INF("%s:VF_EN=0, %s", this->name_Str(),str);
            return MFALSE;
        }

        if(ret == MTRUE)
            break;
        //
        irq.Clear = ISP_IRQ_CLEAR_NONE;

    }while(--loopCnt > 0);
EXIT:
    if(ret == MFALSE){
        if(this->m_buf_cnt != 0){
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

MUINT32 Header_RRZO::GetRegInfo(MUINT32 Tag,IspDrv* DrvPtr){
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


