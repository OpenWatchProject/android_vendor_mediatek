#define LOG_TAG "ifunc_cam"

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

DECLARE_DBG_LOG_VARIABLE(func_cam);
//EXTERN_DBG_LOG_VARIABLE(func_cam);

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

/*//////////////////////////////////////////////////////////////////////////////
  CAM_TG_CTRL
///////////////////////////////////////////////////////////////////////////////*/
CAM_TG_CTRL::CAM_TG_CTRL()
{
    m_PixMode = _1_pix_;
    m_Datapat = _tg_normal_;
    m_continuous = MFALSE;
    m_SubSample  = 0;
    m_pDrv = NULL;
}

MINT32 CAM_TG_CTRL::_config( void )
{
    E_ISP_CAM_CQ cq;
    MUINT32 page;
    this->m_pDrv = (ISP_DRV_CAM*)this->m_pIspDrv;

    //dbg log
    this->m_pDrv->getCurObjInfo(&this->m_hwModule,&cq,&page);
    if (page == 0) { /*reduce log, only show page 0*/
        CAM_FUNC_INF("CAM_TG_CTRL::_config +:xcropping:(0x%x_0x%x_0x%x_0x%x),is_continuous(0x%x),SubSample(0x%x),pixmode(0x%x),data-pattern(0x%x)\n", \
            this->m_Crop.x, \
            this->m_Crop.y, \
            this->m_Crop.w, \
            this->m_Crop.h,  \
            this->m_continuous,  \
            this->m_SubSample,\
            this->m_PixMode,\
            this->m_Datapat);
    }

    //subsample
    //for vsync subsample function, need to make sure cmos_en is off.
    CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),CAM_TG_SEN_MODE,CMOS_EN,0);
    if(this->m_SubSample){
        //sof
        CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),CAM_TG_SEN_MODE,SOF_SUB_EN,1);
        //vsync
        CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),CAM_TG_SEN_MODE,VS_SUB_EN,1);
        CAM_WRITE_REG(this->m_pDrv->getPhyObj(),CAM_TG_SUB_PERIOD,((this->m_SubSample<<8)|this->m_SubSample));
    }
    else{
        //sof
        CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),CAM_TG_SEN_MODE,SOF_SUB_EN,0);
        //vsync
        CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),CAM_TG_SEN_MODE,VS_SUB_EN,0);
        CAM_WRITE_REG(this->m_pDrv->getPhyObj(),CAM_TG_SUB_PERIOD,((this->m_SubSample<<8)|this->m_SubSample));
    }

    //timestamp
    //!! disable hw timestamp, since only TGx all ref to seninf1, which casuse sub cam fail !!
    CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),CAM_TG_SEN_MODE,TIME_STP_EN,0);



    //trig mode
    if(this->m_continuous){
        CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),CAM_TG_VF_CON,SINGLE_MODE,0);
    }
    else{
        CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),CAM_TG_VF_CON,SINGLE_MODE,1);
    }

    //pix mode
    switch(this->m_PixMode){
        case _1_pix_:
            CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),CAM_TG_SEN_MODE,DBL_DATA_BUS,0);
            CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),CAM_TG_SEN_MODE,DBL_DATA_BUS1,0);
            break;
        case _2_pix_:
            CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),CAM_TG_SEN_MODE,DBL_DATA_BUS,1);
            CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),CAM_TG_SEN_MODE,DBL_DATA_BUS1,0);
            break;
        case _4_pix_:
            CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),CAM_TG_SEN_MODE,DBL_DATA_BUS,1);
            CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),CAM_TG_SEN_MODE,DBL_DATA_BUS1,1);
            break;
        default:
            CAM_FUNC_ERR("- unsupported pix mode:0x%x\n",this->m_PixMode);
            break;
    }
    //data format
    switch(this->m_Datapat){
        case _tg_dual_pix_:
            this->m_Crop.w *= 2;
            if((this->m_Crop.w %16) != 0){
                CAM_FUNC_ERR("TG cropping size need 16-alignment at dual_pix pattern\n");
                return 1;
            }
            break;
        case _tg_normal_:
        default:
            break;
    }
    //cropping window
    if((this->m_Crop.w % 4) != 0){
        CAM_FUNC_ERR("TG cropping size need 4-alignment\n");
        return 1;
    }
    CAM_WRITE_REG(this->m_pDrv->getPhyObj(),CAM_TG_SEN_GRAB_PXL,(((this->m_Crop.w+this->m_Crop.x)<<16)|this->m_Crop.x));
    CAM_WRITE_REG(this->m_pDrv->getPhyObj(),CAM_TG_SEN_GRAB_LIN,(((this->m_Crop.h+this->m_Crop.y)<<16)|this->m_Crop.y));

    CAM_WRITE_REG(this->m_pDrv,CAM_TG_SEN_GRAB_PXL,(((this->m_Crop.w+this->m_Crop.x)<<16)|this->m_Crop.x));
    CAM_WRITE_REG(this->m_pDrv,CAM_TG_SEN_GRAB_LIN,(((this->m_Crop.h+this->m_Crop.y)<<16)|this->m_Crop.y));

    //
    CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),CAM_TG_ERR_CTL,GRAB_ERR_EN,1);
#if 0
    //
    CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),CAM_TG_SEN_MODE,FIFO_FULL_CTL_EN,1);
    CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),CAM_TG_PATH_CFG,TG_FULL_SEL2,1);
#endif
    CAM_FUNC_DBG("-");
    return 0;
}

MINT32 CAM_TG_CTRL::_enable( void* pParam  )
{
    E_ISP_CAM_CQ cq;
    MUINT32 page;
    (void)pParam;
    this->m_pDrv = (ISP_DRV_CAM*)this->m_pIspDrv;

    //dbg log
    this->m_pDrv->getCurObjInfo(&this->m_hwModule,&cq,&page);
    CAM_FUNC_DBG("CAM_TG_CTRL::_enable +:\n");

    CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),CAM_TG_SEN_MODE,CMOS_EN,1);
    return 0;
}

MINT32 CAM_TG_CTRL::_disable( void* pParam )
{
    E_ISP_CAM_CQ cq;
    MUINT32 page;
    (void)pParam;

    this->m_pDrv = (ISP_DRV_CAM*)this->m_pIspDrv;

    //dbg log
    this->m_pDrv->getCurObjInfo(&this->m_hwModule,&cq,&page);
    CAM_FUNC_DBG("CAM_TG_CTRL::_disable +:\n");

    CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),CAM_TG_SEN_MODE,CMOS_EN,0);

    //clear TG interrupt setting
    CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),CAM_TG_INT1,TG_INT1_PXLNO,0);
    CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),CAM_TG_INT1,TG_INT1_LINENO,0);
    return 0;
}

/*/////////////////////////////////////////////////////////////////////////////
  CAM_TOP_CTRL
/////////////////////////////////////////////////////////////////////////////*/

CAM_TOP_CTRL::CAM_TOP_CTRL()
{
    m_bBusy = MFALSE;
    CAM_Path = IF_CAM_MAX;
    SubSample = 0;
    m_pDrv = NULL;
    DBG_LOG_CONFIG(imageio, func_cam);
}

MINT32 CAM_TOP_CTRL::_config( void )
{
    char value[PROPERTY_VALUE_MAX] = {'\0'};
    MUINT32 _loglevel = 0;
    E_ISP_CAM_CQ cq;
    MUINT32 page;
    this->m_pDrv = (ISP_DRV_CAM*)this->m_pIspDrv;

    //dbg log
    this->m_pDrv->getCurObjInfo(&this->m_hwModule,&cq,&page);
    if (page == 0) { /*reduce log, only show page 0*/
        CAM_FUNC_INF("CAM_TOP_CTRL::_config +,path(0x%x),en(0x%08x),fmtSel(0x%08x),ctrlsel(0x%08x),intEn(0x%08x),dmaintEn(0x%08x),SubSample(0x%x),cq(0x%x),page_num(0x%x)\n", \
            this->CAM_Path, \
            this->cam_top_ctl.FUNC_EN.Raw, \
            this->cam_top_ctl.FMT_SEL.Raw, \
            this->cam_top_ctl.CTRL_SEL.Raw, \
            this->cam_top_ctl.INTE_EN.Raw,  \
            this->cam_top_ctl.DMA_INTE_EN.Raw,  \
            this->SubSample,\
            cq,page);
    }

    //kernel log level:
    property_get("debug.isp", value, "0");
    _loglevel = atoi(value);
    if(_loglevel != 0){
        this->m_pDrv->setDeviceInfo(_SET_DBG_INT,(MUINT8*)&_loglevel);
    }

    //avoid previous scenario exit abnormally. e.g.: exit without stop();
    ISP_BUFFER_CTRL_STRUCT buf_ctrl;
    MUINT32 _clrPort = 2;
    MUINT32 _dma[2] = {_imgo_,_rrzo_};

    while(_clrPort--){
        buf_ctrl.ctrl = ISP_RT_BUF_CTRL_CLEAR;
        buf_ctrl.buf_id = (_isp_dma_enum_)_dma[_clrPort];
        buf_ctrl.data_ptr = NULL;
        buf_ctrl.pExtend = 0;
        if ( MTRUE != this->m_pDrv->setDeviceInfo(_SET_BUF_CTRL,(MUINT8 *) &buf_ctrl)) {
            CAM_FUNC_ERR("ERROR:clear buf ctrl fail\n");
        }
    }


    //reset
    CAM_FUNC_DBG("TOP reset\n");
    CAM_WRITE_REG(this->m_pDrv->getPhyObj(),CAM_CTL_SW_CTL,0x1);
    while(CAM_READ_REG(this->m_pDrv->getPhyObj(),CAM_CTL_SW_CTL) != 0x2){
        CAM_FUNC_DBG("TOP reseting...\n");
    }
    CAM_WRITE_REG(this->m_pDrv->getPhyObj(),CAM_CTL_SW_CTL,0x4);
    CAM_WRITE_REG(this->m_pDrv->getPhyObj(),CAM_CTL_SW_CTL,0x0);


    /********************************************************************************************************************/
    /********************************************************************************************************************/
    /********************************************************************************************************************/

    //cq db setting
    //CAM_WRITE_REG(this->m_pDrv->getPhyObj(),CAM_CQ_THR0_CTL,this->cam_top_ctl.CQ0_CTRL.Raw);
    CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),CAM_CQ_EN,CQ_DB_EN,1); //enable double buffer

    //func en
    CAM_WRITE_REG(this->m_pDrv->getPhyObj(),CAM_CTL_EN,this->cam_top_ctl.FUNC_EN.Raw);
    CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),CAM_CTL_EN,TG_EN,1);
    CAM_WRITE_REG(this->m_pDrv,CAM_CTL_EN,this->cam_top_ctl.FUNC_EN.Raw);
    CAM_WRITE_BITS(this->m_pDrv,CAM_CTL_EN,TG_EN,1);

    //fmt sel
    CAM_WRITE_REG(this->m_pDrv->getPhyObj(),CAM_CTL_FMT_SEL,this->cam_top_ctl.FMT_SEL.Raw);
    CAM_WRITE_REG(this->m_pDrv,CAM_CTL_FMT_SEL,this->cam_top_ctl.FMT_SEL.Raw);

    switch(this->m_hwModule){
        case CAM_A:
            //dmx id
            CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),CAM_CTL_FMT_SEL,DMX_ID,0x0);
            CAM_WRITE_BITS(this->m_pDrv,CAM_CTL_FMT_SEL,DMX_ID,0x0);
#if DMX_ID_SW_WORK_AROUND
            //conflict with 8-lanes sensor.
            //currently, 8lanes sensor is not supported
            if(this->m_bCfg ==MFALSE)
            {
                IspDrv* ptr = IspDrvImp::createInstance(CAM_B);
                MUINT32 tmp;
                ptr->init("DMX_ID");
                tmp = ptr->readReg(0xc);
                ptr->writeReg(0xc,tmp|(1<<20));
                ptr->uninit("DMX_ID");
                ptr->destroyInstance();
                this->m_bCfg = MTRUE;
            }
#endif
            break;
        case CAM_B:
            //dmx id
            CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),CAM_CTL_FMT_SEL,DMX_ID,0x1);
            CAM_WRITE_BITS(this->m_pDrv,CAM_CTL_FMT_SEL,DMX_ID,0x1);
            break;
        default:
            CAM_FUNC_ERR("unsupported \n");
            return -1;
            break;
    }


    //ctl sel
    CAM_WRITE_REG(this->m_pDrv->getPhyObj(),CAM_CTL_SEL,this->cam_top_ctl.CTRL_SEL.Raw);
    CAM_WRITE_REG(this->m_pDrv,CAM_CTL_SEL,this->cam_top_ctl.CTRL_SEL.Raw);

    //inte en
    CAM_WRITE_REG(this->m_pDrv->getPhyObj(),CAM_CTL_RAW_INT_EN,this->cam_top_ctl.INTE_EN.Raw);

    //dma inte en
    CAM_WRITE_REG(this->m_pDrv->getPhyObj(),CAM_CTL_RAW_INT2_EN,this->cam_top_ctl.DMA_INTE_EN.Raw);

    //db_en
    CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),CAM_CTL_MISC,DB_EN,1);

    //reset FH
    CAM_WRITE_REG(this->m_pDrv->getPhyObj(),CAM_DMA_FRAME_HEADER_EN,0);
    CAM_WRITE_REG(this->m_pDrv,CAM_DMA_FRAME_HEADER_EN,0);

    //reset dma en
    CAM_WRITE_REG(this->m_pDrv->getPhyObj(),CAM_CTL_DMA_EN,0);
    CAM_WRITE_REG(this->m_pDrv,CAM_CTL_DMA_EN,0);

    //subsample p1 done
    if(this->SubSample){
        CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),CAM_CTL_SW_PASS1_DONE,DOWN_SAMPLE_EN,1);
    }
    else{
        CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),CAM_CTL_SW_PASS1_DONE,DOWN_SAMPLE_EN,0);
    }

    CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),CAM_CTL_SW_PASS1_DONE,DOWN_SAMPLE_PERIOD,this->SubSample);


    //special
    CAM_WRITE_REG(this->m_pDrv->getPhyObj(),CAM_DMA_RSV1,(CAM_READ_REG(this->m_pDrv->getPhyObj(),CAM_DMA_RSV1)&0x7fffffff));//bit31 for smi mask, blocking imgo output
    CAM_WRITE_REG(this->m_pDrv->getPhyObj(),CAM_DMA_RSV6,0xffffffff);


    //dma performance, this register in CAM_B is useless, CAM_B's performance is controlled by CAM_A's special_fun_en
    //and this register can't be modified when CAM_B is running. so always program CAM_A's special_func_en even if
    //current module is CAM_B
    switch(this->m_hwModule){
        case CAM_B:
            {
                IspDrv* ptr = IspDrvImp::createInstance(CAM_A);
                CAM_REG_SPECIAL_FUN_EN tmp;
                ptr->init("SPEC_FUNC");
                tmp.Raw = ptr->readReg(0x218);
                tmp.Bits.CQ_ULTRA_BPCI_EN = 0x1;
                tmp.Bits.CQ_ULTRA_LSCI_EN = 0x1;
                tmp.Bits.MULTI_PLANE_ID_EN = 0x1;
                ptr->writeReg(0x218,tmp.Raw);
                ptr->uninit("SPEC_FUNC");
                //CAM_FUNC_INF("CAMB write CAMA: sepcial fun:0x%x\n", tmp.Raw);
                ptr->destroyInstance();
            }
            break;
        case CAM_A:
            {
                CAM_REG_SPECIAL_FUN_EN spe_fun;
                spe_fun.Raw = CAM_READ_REG(this->m_pDrv->getPhyObj(),CAM_SPECIAL_FUN_EN);
                spe_fun.Bits.CQ_ULTRA_BPCI_EN = 0x1;
                spe_fun.Bits.CQ_ULTRA_LSCI_EN = 0x1;
                spe_fun.Bits.MULTI_PLANE_ID_EN = 0x1;
                //CAM_FUNC_INF("CAMA: sepcial fun:0x%x\n", spe_fun.Raw);
                CAM_WRITE_REG(this->m_pDrv->getPhyObj(),CAM_SPECIAL_FUN_EN,spe_fun.Raw);
            }
            break;
        default:
            CAM_FUNC_ERR("unsupported \n");
            return -1;
            break;
    }


    //
    CAM_FUNC_DBG("-");
    return 0;
}


MINT32 CAM_TOP_CTRL::_enable( void* pParam  )
{
    E_ISP_CAM_CQ cq;
    MUINT32 page;
    ISP_CLEAR_IRQ_ST clr_irq;
    clr_irq.Status = SW_PASS1_DON_ST;
    clr_irq.UserKey = 0x0;
    clr_irq.St_type = SIGNAL_INT;
    MBOOL wakelock = MTRUE;
    UniDrvImp* pUni = (UniDrvImp*)pParam;
    MUINT32 dma_en,uni_dma_en = 0;
    MUINT32 done_sel = 0;
    MUINT32 uni_sel = 0;
    #if (FBC_DRPGRP_SW_WORK_AROUND == 0)
    MUINT32 _tmp;
    #endif
    this->m_pDrv = (ISP_DRV_CAM*)this->m_pIspDrv;

    //dbg log
    this->m_pDrv->getCurObjInfo(&this->m_hwModule,&cq,&page);
    CAM_FUNC_DBG("CAM_TOP_CTRL::_enable: + cq:0x%x,page:0x%x\n",cq,page);

    //lock pmic for seninf, need to review this mechanism.[power issue]
    if ( MFALSE == this->m_pDrv->setDeviceInfo(_SET_WAKE_LOCK,(MUINT8*)& wakelock)) {
        CAM_FUNC_ERR("ISPWakeLockCtrl enable fail!!");
    }



    //register dump before start()
    if ( MTRUE == func_cam_DbgLogEnable_VERBOSE) {
        CAM_FUNC_DBG("DUMP CAM REGISTER BEFORE START\n");
        //dump phy
        this->m_pDrv->DumpReg(MTRUE);
    }
    //dump CQ
    if ( MTRUE == func_cam_DbgLogEnable_DEBUG) {
        this->m_pDrv->dumpCQTable();
    }


    #if (FBC_DRPGRP_SW_WORK_AROUND == 1)
    CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),CAM_CTL_SW_PASS1_DONE,IMGO_FIFO_FULL_DROP,0);
    CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),CAM_CTL_SW_PASS1_DONE,RRZO_FIFO_FULL_DROP,0);
    CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),CAM_CTL_SW_PASS1_DONE,UFEO_FIFO_FULL_DROP,0);
    CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),CAM_CTL_SW_PASS1_DONE,LCSO_FIFO_FULL_DROP,0);

    #else
    //signal control, can't run-time change
    //imgo
    _tmp = CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_FBC_IMGO_CTL1,FBC_EN);
    CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),CAM_CTL_SW_PASS1_DONE,IMGO_FIFO_FULL_DROP,_tmp);

    //rrzo
    _tmp = CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_FBC_RRZO_CTL1,FBC_EN);
    CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),CAM_CTL_SW_PASS1_DONE,RRZO_FIFO_FULL_DROP,_tmp);
    //ufeo
    _tmp = CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_FBC_UFEO_CTL1,FBC_EN);
    CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),CAM_CTL_SW_PASS1_DONE,UFEO_FIFO_FULL_DROP,_tmp);
    //lcso
    _tmp = CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_FBC_LCSO_CTL1,FBC_EN);
    CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),CAM_CTL_SW_PASS1_DONE,LCSO_FIFO_FULL_DROP,_tmp);
    #endif

    //uni domain
    if(pUni){
        #if (FBC_DRPGRP_SW_WORK_AROUND == 1)
        CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),CAM_CTL_SW_PASS1_DONE,EISO_FIFO_FULL_DROP,0);
        CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),CAM_CTL_SW_PASS1_DONE,RSSO_FIFO_FULL_DROP,0);
        #else
        //eiso
        _tmp = UNI_READ_BITS(pUni,CAM_UNI_FBC_EISO_A_CTL1,FBC_EN);
        CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),CAM_CTL_SW_PASS1_DONE,EISO_FIFO_FULL_DROP,_tmp);

        //rsso
        _tmp = UNI_READ_BITS(pUni,CAM_UNI_FBC_RSSO_A_CTL1,FBC_EN);
        CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),CAM_CTL_SW_PASS1_DONE,RSSO_FIFO_FULL_DROP,_tmp);
        #endif

        CAM_FUNC_INF("unit is linked with cur module\n");
    }
    CAM_FUNC_INF("CAM_TOP_CTRL::sw pass1 done ctrl:0x%x\n",CAM_READ_REG(this->m_pDrv->getPhyObj(),CAM_CTL_SW_PASS1_DONE));
#if 0   //using sw signal sel if 1, otherwise , using hw auto sel
    //signal sel
    dma_en = CAM_READ_REG(this->m_pDrv->getPhyObj(),CAM_CTL_DMA_EN);
    if(pUni)
        uni_dma_en = UNI_READ_REG(pUni,CAM_UNI_TOP_DMA_EN);
    switch(module){
        case CAM_A:
            done_sel |= ((dma_en & IMGO_EN_)?(IMGO_A_):(0));
            done_sel |= ((dma_en & RRZO_EN_)?(RRZO_A_):(0));
            done_sel |= ((dma_en & AFO_EN_)?(AFO_A_):(0));
            done_sel |= ((dma_en & LCSO_EN_)?(LCSO_A_):(0));
            done_sel |= ((dma_en & AAO_EN_)?(AAO_A_):(0));
            done_sel |= ((dma_en & BPCI_EN_)?(BPCI_A_):(0));
            done_sel |= ((dma_en & LSCI_EN_)?(LSCI_A_):(0));
            done_sel |= ((dma_en & PDO_EN_)?(PDO_A_):(0));
            done_sel |= TG_A_;
            CAM_WRITE_REG(this->m_pDrv->getPhyObj(),CAM_CTL_AB_DONE_SEL,done_sel);

            CAM_FUNC_ERR("########################\n");
            CAM_FUNC_ERR("########################\n");
            CAM_FUNC_ERR("twin mode is not ready yet\n");
            CAM_FUNC_ERR("########################\n");
            CAM_FUNC_ERR("########################\n");
            break;
        case CAM_B:
            done_sel |= ((dma_en & IMGO_EN_)?(IMGO_B_):(0));
            done_sel |= ((dma_en & RRZO_EN_)?(RRZO_B_):(0));
            done_sel |= ((dma_en & AFO_EN_)?(AFO_B_):(0));
            done_sel |= ((dma_en & LCSO_EN_)?(LCSO_B_):(0));
            done_sel |= ((dma_en & AAO_EN_)?(AAO_B_):(0));
            done_sel |= ((dma_en & BPCI_EN_)?(BPCI_B_):(0));
            done_sel |= ((dma_en & LSCI_EN_)?(LSCI_B_):(0));
            done_sel |= ((dma_en & PDO_EN_)?(PDO_B_):(0));
            done_sel |= TG_B_;
            CAM_WRITE_REG(this->m_pDrv->getPhyObj(),CAM_CTL_AB_DONE_SEL,done_sel);

            CAM_FUNC_ERR("########################\n");
            CAM_FUNC_ERR("########################\n");
            CAM_FUNC_ERR("twin mode is not ready yet\n");
            CAM_FUNC_ERR("########################\n");
            CAM_FUNC_ERR("########################\n");
            break;
        default:
            CAM_FUNC_ERR("unsupported module:0x%x\n",module);
            return -1;
            break;
    }
    if(pUni){
        uni_sel |= ((uni_dma_en & FLKO_A_EN_) ? (FLKO_A_):(0));
        uni_sel |= ((uni_dma_en & EISO_A_EN_) ? (EISO_A_):(0));
        uni_sel |= ((uni_dma_en & RSSO_A_EN_) ? (RSSO_A_):(0));
    }
    uni_sel |= DONE_SEL_EN_;
    CAM_WRITE_REG(this->m_pDrv->getPhyObj(),CAM_CTL_UNI_DONE_SEL,uni_sel);
#else
    CAM_WRITE_REG(this->m_pDrv->getPhyObj(),CAM_CTL_UNI_DONE_SEL,0);
#endif

    //
    //change CQ load mode before streaming start.
    //because this bit is shared to all cq, so put here to make sure all cq timing
    if(CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_CTL_SW_PASS1_DONE,DOWN_SAMPLE_EN) == 1){
        //load mode = 0 if the latch-timing of the cq-baseaddress-reg is by sw p1_done
        CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),CAM_CQ_EN,CQ_DB_LOAD_MODE,0);
    }
    else{
        //load mode = 1 if the latch-timing of the cq-baseaddress-reg is by hw p1_done
        //when mode = 0 , inner reg is useless
        CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),CAM_CQ_EN,CQ_DB_LOAD_MODE,1);
    }



    //
    switch(this->CAM_Path){
        case IF_CAM_A:
        case IF_CAM_B:
            if(CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_TG_SEN_MODE,CMOS_EN)){
                //
                this->m_pDrv->clearIrq(&clr_irq);
                //
                this->m_pDrv->setDeviceInfo(_SET_VF_ON,(MUINT8*)&this->m_hwModule);
                //
                CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),CAM_TG_VF_CON,VFDATA_EN,1);

                CAM_FUNC_INF("LOCK WAKE LOCK\n");
            }
            else{
                CAM_FUNC_INF("cmos_en is still off,start fail\n");
                return -1;
            }
            break;
        case IF_RAWI_CAM_A:
        case IF_RAWI_CAM_B:
            CAM_FUNC_INF("useing RAWI path\n");
            break;
        default:
            CAM_FUNC_ERR("unsupported path:0x%x\n",this->CAM_Path);
            return -1;
            break;

    }

    this->m_bBusy = MTRUE;
    //
    CAM_FUNC_DBG("-\n");
    return 0;
}

#define IS_TG_IDLE(_TGStat)      (\
        (_TGStat.Bits.TG_CAM_CS == 1) && (_TGStat.Bits.SYN_VF_DATA_EN == 0) && (_TGStat.Bits.OUT_REQ == 0))

MINT32 CAM_TOP_CTRL::_disable( void* pParam )
{
    E_ISP_CAM_CQ cq;
    MUINT32 page;
    ISP_WAIT_IRQ_ST irq;
    MBOOL wakelock = MFALSE;
    this->m_pDrv = (ISP_DRV_CAM*)this->m_pIspDrv;
    irq.Clear = ISP_IRQ_CLEAR_WAIT;
    irq.Status = VS_INT_ST;
    irq.St_type = SIGNAL_INT;
    //irq.Timeout = ((1000 + (this->m_fps - 1)) / this->m_fps) * (this->SubSample+1);
    irq.Timeout = MIN_GRPFRM_TIME_MS * 2;
    irq.UserKey = 0x0;
    MUINT32 bTwin;
    MBOOL bForce = *(MBOOL *)pParam;

    //dbg log
    this->m_pDrv->getCurObjInfo(&this->m_hwModule,&cq,&page);
    CAM_FUNC_DBG("CAM_TOP_CTRL::_disable: + cq:0x%x,page:0x%x,force:%d\n",cq,page,bForce);

    bTwin = CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_CTL_TWIN_STATUS,TWIN_EN);

    if(MFALSE == bForce)
        goto DISABLE_VF;

    //
    switch(this->CAM_Path){
        case IF_CAM_A:
#if DMX_ID_SW_WORK_AROUND
            this->m_bCfg = MFALSE;
#endif
        case IF_CAM_B:
            if(CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_TG_VF_CON,VFDATA_EN) == 1){
                // Set 'CMOS_EN=0' to turn off view finder immediately. No need to wait for next vsync.
                CAM_WRITE_BITS(this->m_pDrv->getPhyObj(), CAM_TG_SEN_MODE, CMOS_EN, 0);
            }
            this->m_pDrv->setDeviceInfo(_SET_VF_OFF,(MUINT8*)&this->m_hwModule);
            //free ion handle by hw module
            this->m_pDrv->setDeviceInfo(_SET_ION_FREE_BY_HWMODULE,(MUINT8*)&this->m_hwModule);
            if(bTwin) {
                ISP_HW_MODULE twinModule = (this->m_hwModule == CAM_A)? CAM_B: CAM_A;
                CAM_FUNC_INF("flush twin ion buf twinModule(0x%x) %d\n", twinModule, 1);
                this->m_pDrv->setDeviceInfo(_SET_ION_FREE_BY_HWMODULE,(MUINT8*)&twinModule);
            }
            break;
        case IF_RAWI_CAM_A:
        case IF_RAWI_CAM_B:
            CAM_FUNC_INF("useing RAWI path\n");
            break;
        default:
            CAM_FUNC_ERR("unsupported path:0x%x\n",this->CAM_Path);
            return -1;
            break;
    }

    //force reset ISP first, current frame will be broken
    CAM_FUNC_WRN("TOP force HW abort reset!\n");
    CAM_WRITE_REG(this->m_pDrv->getPhyObj(),CAM_CTL_SW_CTL,0x0);
    CAM_WRITE_REG(this->m_pDrv->getPhyObj(),CAM_CTL_SW_CTL,0x1);
    while(CAM_READ_REG(this->m_pDrv->getPhyObj(),CAM_CTL_SW_CTL) != 0x2){
        CAM_FUNC_DBG("TOP reseting...\n");
    }
    CAM_WRITE_REG(this->m_pDrv->getPhyObj(),CAM_CTL_SW_CTL,0x4);
    CAM_WRITE_REG(this->m_pDrv->getPhyObj(),CAM_CTL_SW_CTL,0x0);
    //
    goto STOP_ISP;

DISABLE_VF:
    switch(this->CAM_Path){
        case IF_CAM_A:
#if DMX_ID_SW_WORK_AROUND
            this->m_bCfg = MFALSE;
#endif
        case IF_CAM_B:
            if(CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_TG_VF_CON,VFDATA_EN) == 1){
                MUINT32 _cnt=0;
                CAM_REG_TG_INTER_ST tg_stat;

                CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),CAM_TG_VF_CON,VFDATA_EN,0);
                do{
                    ++_cnt;
                    this->m_pDrv->waitIrq(&irq);
                    CAM_FUNC_INF("wait vsync %d time for TG idle\n",_cnt);
                    if(_cnt > 3){
                        PIPE_CHECK ppc;
                        ppc.m_pDrv = this->m_pDrv;
                        ppc.TG_CHECK(0);
                        //dump phy reg
                        CAM_FUNC_ERR("start dump CAM register\n");
                        this->m_pDrv->DumpReg(MTRUE);

                        break;
                    }
                    tg_stat.Raw = CAM_READ_REG(this->m_pDrv->getPhyObj(), CAM_TG_INTER_ST);
                    if (1 == tg_stat.Bits.TG_CAM_CS) {
                        if ((0 != tg_stat.Bits.SYN_VF_DATA_EN) || (0 != tg_stat.Bits.OUT_REQ)) {
                            CAM_FUNC_INF("WARNING: TG stat NOT IDLE: 0x%x", tg_stat.Raw);
                        }
                    }
                } while (!IS_TG_IDLE(tg_stat));
                //}while(CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_TG_INTER_ST,TG_CAM_CS) != 1);
        }
        else{
            CAM_REG_TG_INTER_ST tg_stat;
            tg_stat.Raw = CAM_READ_REG(this->m_pDrv->getPhyObj(), CAM_TG_INTER_ST);

            if (IS_TG_IDLE(tg_stat)) {
                //if(CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_TG_INTER_ST,TG_CAM_CS) == 1){
                CAM_FUNC_INF("vf_en off + tg idle, no wait vsync\n");
            }
            else {
                CAM_FUNC_ERR("Wrong TG state: 0x%x\n", CAM_READ_REG(this->m_pDrv->getPhyObj(), CAM_TG_INTER_ST));
            }
        }

        //
        this->m_pDrv->setDeviceInfo(_SET_VF_OFF,(MUINT8*)&this->m_hwModule);
        //free ion handle by hw module
        this->m_pDrv->setDeviceInfo(_SET_ION_FREE_BY_HWMODULE,(MUINT8*)&this->m_hwModule);
        if(bTwin) {
            ISP_HW_MODULE twinModule = (this->m_hwModule == CAM_A)? CAM_B: CAM_A;
            CAM_FUNC_INF("flush twin ion buf twinModule(0x%x) %d\n", twinModule, 1);
            this->m_pDrv->setDeviceInfo(_SET_ION_FREE_BY_HWMODULE,(MUINT8*)&twinModule);
        }
        //
        break;
        case IF_RAWI_CAM_A:
        case IF_RAWI_CAM_B:
        CAM_FUNC_INF("useing RAWI path\n");
        break;
        default:
        CAM_FUNC_ERR("unsupported path:0x%x\n",this->CAM_Path);
        return -1;
        break;
    }

STOP_ISP:
    CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),CAM_CTL_MISC,DB_EN,0); //disable double buffer
    //signal ctrl
    CAM_WRITE_REG(this->m_pDrv->getPhyObj(),CAM_CTL_SW_PASS1_DONE,0);
    CAM_WRITE_REG(this->m_pDrv->getPhyObj(),CAM_CTL_AB_DONE_SEL,0);
    CAM_WRITE_REG(this->m_pDrv->getPhyObj(),CAM_CTL_UNI_DONE_SEL,0);
    CAM_WRITE_REG(this->m_pDrv->getPhyObj(),CAM_CTL_DMA_EN,0x0);

    CAM_WRITE_BITS(this->m_pDrv->getPhyObj(), CAM_TG_PATH_CFG, DB_LOAD_DIS, 1);//disable TG double buffer
    //VF disable for HW abort reset.
    if(CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_TG_VF_CON,VFDATA_EN) == 1){
        this->m_pDrv->setDeviceInfo(_SET_VF_OFF,(MUINT8*)&this->m_hwModule);
    }

    /// !!!!!!!!!!! Notice !!!!!!!!!!!!
    /// Must disable TwinModule's(CAMB) DMA_EN as well
    /// Or, you'll see P1_Done (both SW and HW) keep coming...
    /// P1_Done criterion: (a) FBC_EN = 0, (b) DMA_EN = 1, (c) TG_EN = 0
    /// E.x.: Twin CAMA,CAMB + AFO CAMA,CAMB: CAMB AFO_EN = 1, AFO_FBC_CTRL = 0, CAMA TG_EN = 0 => CAMA keep coming P1_Done.
    if(0x1 == bTwin) {
        CAM_FUNC_DBG("Close TWIN DMA_EN!!!\n");
        IspDrv* ptr = NULL;
        switch(this->m_hwModule) {
            case CAM_A:
                ptr = IspDrvImp::createInstance(CAM_B);
                break;
            case CAM_B:
                ptr = IspDrvImp::createInstance(CAM_A);
                break;
            default:
                CAM_FUNC_ERR("Unsopprted m_hwModule(%d) use twin!!!\n", this->m_hwModule);
                break;
        }

        if (ptr != NULL) {
            MUINT32 tmp;
            //ptr->init("TWIN");
            ptr->writeReg(0x8,0x0);
            //ptr->uninit("TWIN");
            ptr->destroyInstance();
        }
    }

    CAM_WRITE_REG(this->m_pDrv->getPhyObj(),CAM_CTL_EN,0x1);
    //pdc function enable is within BNR
    CAM_WRITE_REG(this->m_pDrv->getPhyObj(),CAM_BNR_BPC_CON,0x0);
    CAM_WRITE_REG(this->m_pDrv->getPhyObj(),CAM_BNR_PDC_CON,0x0);

    //close  fbc
    //FBC on uni will be closed at the _disable() of uni_top
    //FBC of STT pipe will be closed at STT pipe
    CAM_WRITE_REG(this->m_pDrv->getPhyObj(),CAM_FBC_IMGO_CTL1,0x0);
    CAM_WRITE_REG(this->m_pDrv->getPhyObj(),CAM_FBC_RRZO_CTL1,0x0);
    CAM_WRITE_REG(this->m_pDrv->getPhyObj(),CAM_FBC_LCSO_CTL1,0x0);

    CAM_WRITE_BITS(this->m_pDrv->getPhyObj(), CAM_TG_PATH_CFG, DB_LOAD_DIS, 0);//enable TG double buffer
    CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),CAM_CTL_MISC,DB_EN,1); //disable double buffer


    ISP_BUFFER_CTRL_STRUCT buf_ctrl;
    MUINT32 _clrPort = 2;
    MUINT32 _dma[2] = {_imgo_,_rrzo_};

    while(_clrPort--){
        buf_ctrl.ctrl = ISP_RT_BUF_CTRL_CLEAR;
        buf_ctrl.buf_id = (_isp_dma_enum_)_dma[_clrPort];
        buf_ctrl.data_ptr = NULL;
        buf_ctrl.pExtend = 0;
        if ( MTRUE != this->m_pDrv->setDeviceInfo(_SET_BUF_CTRL,(MUINT8 *) &buf_ctrl)) {
            CAM_FUNC_ERR("ERROR:clear buf ctrl fail\n");
        }
    }

    //reset
    if(MFALSE == bForce) {
        CAM_FUNC_DBG("TOP reset\n");
        CAM_WRITE_REG(this->m_pDrv->getPhyObj(),CAM_CTL_SW_CTL,0x0);
        CAM_WRITE_REG(this->m_pDrv->getPhyObj(),CAM_CTL_SW_CTL,0x1);
#if 0
        while(CAM_READ_REG(this->m_pDrv->getPhyObj(),CAM_CTL_SW_CTL) != 0x2){
            CAM_FUNC_DBG("TOP reseting...\n");
        }
#else
        usleep(10);
#endif
        CAM_WRITE_REG(this->m_pDrv->getPhyObj(),CAM_CTL_SW_CTL,0x4);
        CAM_WRITE_REG(this->m_pDrv->getPhyObj(),CAM_CTL_SW_CTL,0x0);
    }
    //
    CAM_FUNC_INF("UNLOCK WAKE LOCK\n");
    if ( MFALSE == this->m_pDrv->setDeviceInfo(_SET_WAKE_LOCK,(MUINT8*)& wakelock)) {
        CAM_FUNC_ERR("ISPWakeLockCtrl disable fail!!");
    }


    this->m_bBusy = MFALSE;

    //register dump after stop()
    if ( MTRUE == func_cam_DbgLogEnable_VERBOSE) {
        CAM_FUNC_DBG("DUMP CAM REGISTER BEFORE START\n");
        //dump phy
        this->m_pDrv->DumpReg(MTRUE);
        //dump CQ
        this->m_pDrv->dumpCQTable();
    }

    CAM_FUNC_DBG("-\n");

    return 0;
}

MBOOL CAM_TOP_CTRL::checkBusy(  MUINTPTR param  )
{
    (void)param;
    return this->m_bBusy;
}

MINT32 CAM_TOP_CTRL::_write2CQ(void)
{
    E_ISP_CAM_CQ cq;
    MUINT32 page;
    this->m_pDrv = (ISP_DRV_CAM*)this->m_pIspDrv;
    //dbg log
    this->m_pDrv->getCurObjInfo(&this->m_hwModule,&cq,&page);
    CAM_FUNC_DBG("CAM_TOP_CTRL::_write2CQ + cq:0x%x,page:0x%x\n",cq,page);

    this->m_pDrv->cqAddModule(CAM_CTL_EN_);
    this->m_pDrv->cqAddModule(CAM_CTL_FMT_);
    this->m_pDrv->cqAddModule(CAM_CTL_SEL_);
    this->m_pDrv->cqAddModule(CAM_CTL_FBC_RCNT_INC_);
    this->m_pDrv->cqAddModule(CAM_DMA_FH_EN_);
    //
    CAM_FUNC_DBG("-\n");

    return 0;
}


MBOOL CAM_TOP_CTRL::suspend(void)
{
    MBOOL rst = MTRUE;
    MUINT32 i=0;
    MUINT32 flags[2];
    this->m_pDrv = (ISP_DRV_CAM*)this->m_pIspDrv;

    CAM_FUNC_INF("CAM_TOP_CTRL::suspend + \n");

    IspFunction_B::fbcPhyLock_IMGO[this->m_hwModule].lock();
    IspFunction_B::fbcPhyLock_RRZO[this->m_hwModule].lock();
    IspFunction_B::fbcPhyLock_UFEO[this->m_hwModule].lock();
    IspFunction_B::fbcPhyLock_AFO[this->m_hwModule].lock();
    IspFunction_B::fbcPhyLock_AAO[this->m_hwModule].lock();
    IspFunction_B::fbcPhyLock_LCSO[this->m_hwModule].lock();
    IspFunction_B::fbcPhyLock_PDO[this->m_hwModule].lock();

    //keep fbc cnt
    if(CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_FBC_IMGO_CTL1,FBC_EN)){
        this->fbc_IMGO.Raw = CAM_READ_REG(this->m_pDrv->getPhyObj(),CAM_FBC_IMGO_CTL2);
    }

    if(CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_FBC_RRZO_CTL1,FBC_EN)){
        this->fbc_RRZO.Raw = CAM_READ_REG(this->m_pDrv->getPhyObj(),CAM_FBC_RRZO_CTL2);
    }

    if(CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_FBC_UFEO_CTL1,FBC_EN)){
        this->fbc_UFEO.Raw = CAM_READ_REG(this->m_pDrv->getPhyObj(),CAM_FBC_UFEO_CTL2);
    }

    if(CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_FBC_AFO_CTL1,FBC_EN)){
        this->fbc_AFO.Raw = CAM_READ_REG(this->m_pDrv->getPhyObj(),CAM_FBC_AFO_CTL2);
    }

    if(CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_FBC_AAO_CTL1,FBC_EN)){
        this->fbc_AAO.Raw = CAM_READ_REG(this->m_pDrv->getPhyObj(),CAM_FBC_AAO_CTL2);
    }

    if(CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_FBC_LCSO_CTL1,FBC_EN)){
        this->fbc_LCSO.Raw = CAM_READ_REG(this->m_pDrv->getPhyObj(),CAM_FBC_LCSO_CTL2);
    }

    if(CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_FBC_PDO_CTL1,FBC_EN)){
        this->fbc_PDO.Raw = CAM_READ_REG(this->m_pDrv->getPhyObj(),CAM_FBC_PDO_CTL2);
    }

    //close vf
    CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),CAM_TG_VF_CON,VFDATA_EN,0);

    /* backup FBC FIFO_FULL_DROP */
    this->fbcSwP1DoneCon.Raw = CAM_READ_REG(this->m_pDrv->getPhyObj(), CAM_CTL_SW_PASS1_DONE);
    /* disable FBC FIFO_FULL_DROP */
    //no need by CVD CAM_WRITE_REG(this->m_pDrv->getPhyObj(), CAM_CTL_SW_PASS1_DONE, 0);

    CAM_FUNC_INF("TOP reset\n");
    CAM_WRITE_REG(this->m_pDrv->getPhyObj(),CAM_CTL_SW_CTL,0x0);
    CAM_WRITE_REG(this->m_pDrv->getPhyObj(),CAM_CTL_SW_CTL,0x1);
    while(CAM_READ_REG(this->m_pDrv->getPhyObj(),CAM_CTL_SW_CTL) != 0x2){
        CAM_FUNC_INF("TOP reseting...\n");
    }
    CAM_WRITE_REG(this->m_pDrv->getPhyObj(),CAM_CTL_SW_CTL,0x4);
    CAM_WRITE_REG(this->m_pDrv->getPhyObj(),CAM_CTL_SW_CTL,0x0);

    /* reset kernel: timestamp base for H/W timestamp is used */
    flags[0] = 0;
    flags[1] = 0;
    if (MFALSE == this->m_pDrv->setDeviceInfo(_SET_CAM_RESET, (MUINT8*)flags)) {
        CAM_FUNC_ERR("ISP kernel reset NG!\n");
    }

    CAM_FUNC_DBG("CAM_TOP_CTRL::suspend - \n");

    return rst;
}

MBOOL CAM_TOP_CTRL::resume(void)
{
    MUINT32 frm_cnt = 0;

    CAM_FUNC_INF("CAM_TOP_CTRL::resume + \n");

    CAM_FUNC_INF("CAM_TOP_CTRL::resume SAVE_FBC:IMGO_0x%08x RRZO_0x%08x UFEO_0x%08x AFO_0x%08x AAO_0x%08x LCSO_0x%08x PDO_0x%08x",
            this->fbc_IMGO.Raw, this->fbc_RRZO.Raw, this->fbc_UFEO.Raw, this->fbc_AFO.Raw,
            this->fbc_AAO.Raw, this->fbc_LCSO.Raw, this->fbc_PDO.Raw);

    //keep fbc cnt
    if(CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_FBC_IMGO_CTL1,FBC_EN)){
        for (frm_cnt = 0; frm_cnt < this->fbc_IMGO.Bits.FBC_CNT; frm_cnt++) {
            CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),CAM_CTL_FBC_RCNT_INC, IMGO_RCNT_INC, 1);
        }
        CAM_WRITE_REG(this->m_pDrv->getPhyObj(),CAM_FBC_IMGO_CTL2,this->fbc_IMGO.Raw);
    }

    if(CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_FBC_RRZO_CTL1,FBC_EN)){
        for (frm_cnt = 0; frm_cnt < this->fbc_RRZO.Bits.FBC_CNT; frm_cnt++) {
            CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),CAM_CTL_FBC_RCNT_INC, RRZO_RCNT_INC, 1);
        }
        CAM_WRITE_REG(this->m_pDrv->getPhyObj(),CAM_FBC_RRZO_CTL2,this->fbc_RRZO.Raw);
    }

    if(CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_FBC_UFEO_CTL1,FBC_EN)){
        for (frm_cnt = 0; frm_cnt < this->fbc_UFEO.Bits.FBC_CNT; frm_cnt++) {
            CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),CAM_CTL_FBC_RCNT_INC, UFEO_RCNT_INC, 1);
        }
        CAM_WRITE_REG(this->m_pDrv->getPhyObj(),CAM_FBC_UFEO_CTL2,this->fbc_UFEO.Raw);
    }
    if(CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_FBC_AFO_CTL1,FBC_EN)){
        for (frm_cnt = 0; frm_cnt < this->fbc_AFO.Bits.FBC_CNT; frm_cnt++) {
            if (CAM_READ_BITS(this->m_pDrv->getPhyObj(), CAM_CTL_TWIN_STATUS,TWIN_EN)) {
                CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),CAM_CTL_FBC_RCNT_INC, AFO_A_RCNT_INC, 1);
                CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),CAM_CTL_FBC_RCNT_INC, AFO_B_RCNT_INC, 1);
            } else {
                switch (this->m_hwModule) {
                    case CAM_A:
                        CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),CAM_CTL_FBC_RCNT_INC, AFO_A_RCNT_INC, 1);
                        break;
                    case CAM_B:
                        CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),CAM_CTL_FBC_RCNT_INC, AFO_B_RCNT_INC, 1);
                        break;
                    default:
                        CAM_FUNC_ERR("Unsupported module: %d\n",this->m_hwModule);
                        break;
                }
            }

        }
        CAM_WRITE_REG(this->m_pDrv->getPhyObj(),CAM_FBC_AFO_CTL2,this->fbc_AFO.Raw);
    }

    if(CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_FBC_AAO_CTL1,FBC_EN)){
        for (frm_cnt = 0; frm_cnt < this->fbc_AAO.Bits.FBC_CNT; frm_cnt++) {
            CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),CAM_CTL_FBC_RCNT_INC, AAO_RCNT_INC, 1);
        }
        CAM_WRITE_REG(this->m_pDrv->getPhyObj(),CAM_FBC_AAO_CTL2,this->fbc_AAO.Raw);
    }

    if(CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_FBC_LCSO_CTL1,FBC_EN)){
        for (frm_cnt = 0; frm_cnt < this->fbc_LCSO.Bits.FBC_CNT; frm_cnt++) {
            CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),CAM_CTL_FBC_RCNT_INC, LCSO_RCNT_INC, 1);
        }
        CAM_WRITE_REG(this->m_pDrv->getPhyObj(),CAM_FBC_LCSO_CTL2,this->fbc_LCSO.Raw);
    }

    if(CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_FBC_PDO_CTL1,FBC_EN)){
        for (frm_cnt = 0; frm_cnt < this->fbc_PDO.Bits.FBC_CNT; frm_cnt++) {
            CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),CAM_CTL_FBC_RCNT_INC, PDO_A_RCNT_INC, 1);
        }
        CAM_WRITE_REG(this->m_pDrv->getPhyObj(),CAM_FBC_PDO_CTL2,this->fbc_PDO.Raw);
    }

    IspFunction_B::fbcPhyLock_IMGO[this->m_hwModule].unlock();
    IspFunction_B::fbcPhyLock_RRZO[this->m_hwModule].unlock();
    IspFunction_B::fbcPhyLock_UFEO[this->m_hwModule].unlock();
    IspFunction_B::fbcPhyLock_AFO[this->m_hwModule].unlock();
    IspFunction_B::fbcPhyLock_AAO[this->m_hwModule].unlock();
    IspFunction_B::fbcPhyLock_LCSO[this->m_hwModule].unlock();
    IspFunction_B::fbcPhyLock_PDO[this->m_hwModule].unlock();

    CAM_FUNC_INF("CAM_TOP_CTRL::resume PHYS_FBC:IMGO_0x%08x RRZO_0x%08x UFEO_0x%08x AFO_0x%08x AAO_0x%08x LCSO_0x%08x PDO_0x%08x",
            CAM_READ_REG(this->m_pDrv->getPhyObj(), CAM_FBC_IMGO_CTL2),
            CAM_READ_REG(this->m_pDrv->getPhyObj(), CAM_FBC_RRZO_CTL2),
            CAM_READ_REG(this->m_pDrv->getPhyObj(), CAM_FBC_UFEO_CTL2),
            CAM_READ_REG(this->m_pDrv->getPhyObj(), CAM_FBC_AFO_CTL1),
            CAM_READ_REG(this->m_pDrv->getPhyObj(), CAM_FBC_AAO_CTL2),
            CAM_READ_REG(this->m_pDrv->getPhyObj(), CAM_FBC_LCSO_CTL2),
            CAM_READ_REG(this->m_pDrv->getPhyObj(), CAM_FBC_PDO_CTL2));

    /* restore FBC FIFO_FULL_DROP */
    //no need by CVD CAM_WRITE_REG(this->m_pDrv->getPhyObj(), CAM_CTL_SW_PASS1_DONE, this->fbcSwP1DoneCon.Raw);

    //vf
    CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),CAM_TG_VF_CON,VFDATA_EN,1);

    CAM_FUNC_DBG("CAM_TOP_CTRL::resume - \n");
    return MTRUE;
}


UNI_TOP_CTRL::UNI_TOP_CTRL()
{
    UNI_Source = IF_CAM_MAX;
    m_pUniDrv= NULL;
    m_pDrv = NULL;
}

MINT32 UNI_TOP_CTRL::_config( void )
{
    MUINT32 _loglevel;
    E_ISP_CAM_CQ cq;
    MUINT32 page;
    MUINT32 _tmp;
    this->m_pDrv = (ISP_DRV_CAM*)this->m_pIspDrv;
    //dbg log
    this->m_pDrv->getCurObjInfo(&this->m_hwModule,&cq,&page);
    if (page == 0) { /*reduce log, only show page 0*/
        CAM_FUNC_INF("UNI_TOP_CTRL::_config +,uni_module(0x%x),en(0x%08x),fmtSel(0x%08x),ctrlsel(0x%08x),intEn(0x%08x),cq(0x%x),page_num(0x%x)\n", \
            this->m_pUniDrv->getCurObjInfo(), \
            this->uni_top_ctl.FUNC_EN.Raw, \
            this->uni_top_ctl.FMT_SEL.Raw, \
            this->uni_top_ctl.CTRL_SEL.Raw, \
            this->uni_top_ctl.INTE_EN.Raw,  \
            cq,page);
    }

    /********************************************************************************************************************/
    //most uni setting r configured thru CQ
    /********************************************************************************************************************/

    //func en
    //UNI_WRITE_REG(this->m_pUniDrv,CAM_UNI_TOP_MOD_EN,this->uni_top_ctl.FUNC_EN.Raw);
    UNI_WRITE_REG(this->m_pDrv,CAM_UNI_TOP_MOD_EN,this->uni_top_ctl.FUNC_EN.Raw);


    //default always enabe qbin3 & hds,  GSE is binding with FullG
    UNI_WRITE_BITS(this->m_pDrv,CAM_UNI_TOP_MOD_EN,QBIN3_A_EN,1);
    UNI_WRITE_BITS(this->m_pDrv,CAM_UNI_TOP_MOD_EN,HDS_A_EN,1);

    if(CAM_READ_BITS(this->m_pDrv,CAM_CTL_FMT_SEL,RRZO_FG_MODE) == 1)
        UNI_WRITE_BITS(this->m_pDrv,CAM_UNI_TOP_MOD_EN,GSE_A_EN,1);


    //fmt sel
    //UNI_WRITE_REG(this->m_pUniDrv,CAM_UNI_TOP_FMT_SEL,this->uni_top_ctl.FMT_SEL.Raw);
    UNI_WRITE_REG(this->m_pDrv,CAM_UNI_TOP_FMT_SEL,this->uni_top_ctl.FMT_SEL.Raw);

    //ctl sel
    //UNI_WRITE_REG(this->m_pUniDrv,CAM_UNI_TOP_PATH_SEL,this->uni_top_ctl.CTRL_SEL.Raw);
    UNI_WRITE_REG(this->m_pDrv,CAM_UNI_TOP_PATH_SEL,this->uni_top_ctl.CTRL_SEL.Raw);

    //inte en
    UNI_WRITE_REG(this->m_pUniDrv,CAM_UNI_TOP_DMA_INT_EN,this->uni_top_ctl.INTE_EN.Raw);

    //db_en
    UNI_WRITE_BITS(this->m_pUniDrv,CAM_UNI_TOP_MISC,DB_EN,1);


    switch(this->UNI_Source){
        case IF_RAWI_CAM_A:
        case IF_RAWI_CAM_B:
            //input information, borrow tg crop to keep informaiton
            CAM_WRITE_REG(this->m_pDrv->getPhyObj(),CAM_TG_SEN_GRAB_PXL,(((this->m_Crop.w+this->m_Crop.x)<<16)|this->m_Crop.x));
            CAM_WRITE_REG(this->m_pDrv->getPhyObj(),CAM_TG_SEN_GRAB_LIN,(((this->m_Crop.h+this->m_Crop.y)<<16)|this->m_Crop.y));

            //hds
            _tmp = UNI_READ_BITS(this->m_pDrv,CAM_UNI_TOP_FMT_SEL,PIX_BUS_RAWI);
            UNI_WRITE_BITS(this->m_pDrv,CAM_UNI_HDS_A_MODE,HDS_DS,_tmp);

            //qbin3
            UNI_WRITE_BITS(this->m_pDrv,CAM_UNI_QBN3_A_MODE, QBN_ACC_MODE,1);//always use average
            _tmp = UNI_READ_BITS(this->m_pDrv,CAM_UNI_TOP_FMT_SEL,PIX_BUS_RAWI);
            UNI_WRITE_BITS(this->m_pDrv,CAM_UNI_QBN3_A_MODE, QBN_ACC,_tmp);
            CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),CAM_CTL_EN,TG_EN,0);
            break;
        case IF_CAM_A:
        case IF_CAM_B:
            //hds
            _tmp = (CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_TG_SEN_MODE,DBL_DATA_BUS) + \
                    CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_TG_SEN_MODE,DBL_DATA_BUS1));
            if(1 == CAM_READ_BITS(this->m_pDrv,CAM_CTL_SEL,HDS1_SEL))
                _tmp >>= (CAM_READ_BITS(this->m_pDrv,CAM_CTL_EN,BIN_EN) + CAM_READ_BITS(this->m_pDrv,CAM_CTL_EN,DBN_EN));
            UNI_WRITE_BITS(this->m_pDrv,CAM_UNI_HDS_A_MODE,HDS_DS,_tmp);

            //qbin3
            _tmp = (CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_TG_SEN_MODE,DBL_DATA_BUS) + \
                    CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_TG_SEN_MODE,DBL_DATA_BUS1));

            if(0 == CAM_READ_BITS(this->m_pDrv,CAM_CTL_SEL,FLK1_SEL))
                _tmp >>= (CAM_READ_BITS(this->m_pDrv,CAM_CTL_EN,BIN_EN) + CAM_READ_BITS(this->m_pDrv,CAM_CTL_EN,DBN_EN));

            UNI_WRITE_BITS(this->m_pDrv,CAM_UNI_QBN3_A_MODE, QBN_ACC_MODE,1);//always use average
            UNI_WRITE_BITS(this->m_pDrv,CAM_UNI_QBN3_A_MODE, QBN_ACC,_tmp);

            break;
        default:
            CAM_FUNC_ERR("unsupported source:0x%x\n",this->UNI_Source);
            return -1;
            break;
    }

    //
    CAM_FUNC_DBG("-");
    return 0;
}


MINT32 UNI_TOP_CTRL::_enable( void* pParam  )
{
    E_ISP_CAM_CQ cq;
    MUINT32 page;
    (void)pParam;
    this->m_pDrv = (ISP_DRV_CAM*)this->m_pIspDrv;
    //dbg log
    this->m_pDrv->getCurObjInfo(&this->m_hwModule,&cq,&page);
    CAM_FUNC_DBG("UNI_TOP_CTRL::_enable: + cq:0x%x,page:0x%x\n",cq,page);


    //register dump before start()
    if ( MTRUE == func_cam_DbgLogEnable_VERBOSE) {
        CAM_FUNC_DBG("DUMP CAM+UNI REGISTER BEFORE START\n");
        //dump vir
        this->m_pDrv->DumpReg(MFALSE);
    }
    //dump CQ
    if ( MTRUE == func_cam_DbgLogEnable_DEBUG) {
        this->m_pDrv->dumpCQTable();
    }

    switch(this->UNI_Source){
        case IF_RAWI_CAM_A:
        case IF_RAWI_CAM_B:
            UNI_WRITE_BITS(this->m_pUniDrv,CAM_UNI_TOP_RAWI_TRIG,TOP_RAWI_TRIG,1);
            break;
        case IF_CAM_A:
        case IF_CAM_B:
            CAM_FUNC_INF("UNI link with CAM(0x%x)\n",this->UNI_Source);
            break;
        default:
            CAM_FUNC_ERR("UNI unsupported source:0x%x\n",this->UNI_Source);
            return -1;
            break;
    }
    //
    CAM_FUNC_DBG("-");
    return 0;
}

MINT32 UNI_TOP_CTRL::ClrUniFunc(void)
{
    E_ISP_CAM_CQ cq;
    MUINT32 page;
    this->m_pDrv = (ISP_DRV_CAM*)this->m_pIspDrv;
    //dbg log
    this->m_pDrv->getCurObjInfo(&this->m_hwModule,&cq,&page);
    CAM_FUNC_DBG("UNI_TOP_CTRL::ClrUniFunc: + cq:0x%x,page:0x%x\n",cq,page);

    UNI_WRITE_REG(this->m_pDrv,CAM_UNI_TOP_MOD_EN,0);
    UNI_WRITE_REG(this->m_pDrv,CAM_UNI_TOP_DMA_EN,0);
    UNI_WRITE_REG(this->m_pDrv,CAM_UNI_DMA_FRAME_HEADER_EN,0);
    UNI_WRITE_REG(this->m_pDrv,CAM_UNI_FBC_FLKO_A_CTL1,0);
    UNI_WRITE_REG(this->m_pDrv,CAM_UNI_FBC_EISO_A_CTL1,0);
    UNI_WRITE_REG(this->m_pDrv,CAM_UNI_FBC_RSSO_A_CTL1,0);
    return 0;
}

MINT32 UNI_TOP_CTRL::_disable( void* pParam )
{
    MINT32 ret = 0;
    E_ISP_CAM_CQ cq;
    MUINT32 page;
    ISP_WAIT_IRQ_ST irq;
    (void)pParam;

    this->m_pDrv = (ISP_DRV_CAM*)this->m_pIspDrv;
    irq.Clear = ISP_IRQ_CLEAR_WAIT;

    irq.St_type = SIGNAL_INT;
    irq.Timeout = 0x1000;
    irq.UserKey = 0x0;

    //dbg log
    this->m_pDrv->getCurObjInfo(&this->m_hwModule,&cq,&page);
    CAM_FUNC_DBG("UNI_TOP_CTRL::_disable: + cq:0x%x,page:0x%x\n",cq,page);

    switch(this->UNI_Source){
        case IF_RAWI_CAM_A:
        case IF_RAWI_CAM_B:
            break;
        case IF_CAM_A:
        case IF_CAM_B:
            if(CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_TG_VF_CON,VFDATA_EN))
            {
                MUINT32 _cnt = 0;
                //wait sof for timing-align
                irq.Status = SOF_INT_ST;
                this->m_pDrv->waitIrq(&irq);
                //off db load to force register setting to be flushed into inner immediately
                UNI_WRITE_BITS(this->m_pUniDrv,CAM_UNI_TOP_MISC,DB_EN,0);
                UNI_WRITE_REG(this->m_pUniDrv,CAM_UNI_TOP_DMA_EN,0);
                UNI_WRITE_REG(this->m_pUniDrv,CAM_UNI_TOP_MOD_EN,0);
                UNI_WRITE_REG(this->m_pUniDrv,CAM_UNI_DMA_FRAME_HEADER_EN,0);
                UNI_WRITE_REG(this->m_pUniDrv,CAM_UNI_FBC_FLKO_A_CTL1,0);
                UNI_WRITE_REG(this->m_pUniDrv,CAM_UNI_FBC_EISO_A_CTL1,0);
                UNI_WRITE_REG(this->m_pUniDrv,CAM_UNI_FBC_RSSO_A_CTL1,0);
                UNI_WRITE_BITS(this->m_pUniDrv,CAM_UNI_TOP_MISC,DB_EN,1);
                //chk dmao is enough
                while( (UNI_READ_REG(this->m_pUniDrv,CAM_UNI_TOP_DMA_EN)&0xF) != 0){
                    ++_cnt;
                    irq.Status = HW_PASS1_DON_ST;
                    this->m_pDrv->waitIrq(&irq);
                    CAM_FUNC_INF("wait hw p1 done %d time for UNI FUNC/DMAO off\n",_cnt);
                    if(_cnt > 2){
                        CAM_FUNC_ERR("uni stop fail,timeout\n");
                        ret = 1;
                        goto EXIT;
                    }

                }
            }
            else
            {
                UNI_WRITE_BITS(this->m_pUniDrv,CAM_UNI_TOP_MISC,DB_EN,0);
                UNI_WRITE_REG(this->m_pUniDrv,CAM_UNI_TOP_MOD_EN,0);
                UNI_WRITE_REG(this->m_pUniDrv,CAM_UNI_TOP_DMA_EN,0);
                UNI_WRITE_REG(this->m_pUniDrv,CAM_UNI_DMA_FRAME_HEADER_EN,0);
                UNI_WRITE_REG(this->m_pUniDrv,CAM_UNI_FBC_FLKO_A_CTL1,0);
                UNI_WRITE_REG(this->m_pUniDrv,CAM_UNI_FBC_EISO_A_CTL1,0);
                UNI_WRITE_REG(this->m_pUniDrv,CAM_UNI_FBC_RSSO_A_CTL1,0);
                UNI_WRITE_BITS(this->m_pUniDrv,CAM_UNI_TOP_MISC,DB_EN,1);
            }

            //clear descriptor is to avoid CQ working interactively when uni link to new path
            this->m_pDrv->cqDelModule(UNI_CTL_EN_);
            this->m_pDrv->cqDelModule(UNI_CTL_SEL_);
            this->m_pDrv->cqDelModule(UNI_CTL_FMT_);

            //this->m_pDrv->cqDelModule(UNI_ISP_UNP2_);
            this->m_pDrv->cqDelModule(UNI_ISP_QBN3_);
            this->m_pDrv->cqDelModule(UNI_ISP_SGG3_);
            this->m_pDrv->cqDelModule(UNI_ISP_FLK_);
            this->m_pDrv->cqDelModule(UNI_ISP_SGG2_A_);
            this->m_pDrv->cqDelModule(UNI_ISP_HDS_A_);
            this->m_pDrv->cqDelModule(UNI_ISP_EIS_);
            this->m_pDrv->cqDelModule(UNI_ISP_RSS_A_);

            this->m_pDrv->cqDelModule(UNI_FBC_FLKO_);
            this->m_pDrv->cqDelModule(UNI_FBC_EISO_);
            this->m_pDrv->cqDelModule(UNI_FBC_RSSO_);
            break;
        default:
            CAM_FUNC_ERR("UNI unsupported source:0x%x\n",this->UNI_Source);
            return -1;
            break;
    }

    //reset
    CAM_FUNC_DBG("UNI reset\n");
    UNI_WRITE_REG(this->m_pUniDrv,CAM_UNI_TOP_SW_CTL,0x222);
    UNI_WRITE_REG(this->m_pUniDrv,CAM_UNI_TOP_SW_CTL,0x111);
#if 0
    while(UNI_READ_REG(this->m_pUniDrv,CAM_UNI_TOP_SW_CTL) != 0x222){
        CAM_FUNC_DBG("UNI reseting...\n");
    }
#else
    usleep(10);
#endif
    UNI_WRITE_REG(this->m_pUniDrv,CAM_UNI_TOP_SW_CTL,0x0444);
    UNI_WRITE_REG(this->m_pUniDrv,CAM_UNI_TOP_SW_CTL,0x0);
EXIT:
    CAM_FUNC_DBG("-\n");

    return ret;
}

MINT32 UNI_TOP_CTRL::_write2CQ(void)
{
    E_ISP_CAM_CQ cq;
    MUINT32 page;
    this->m_pDrv = (ISP_DRV_CAM*)this->m_pIspDrv;
    //dbg log
    this->m_pDrv->getCurObjInfo(&this->m_hwModule,&cq,&page);
    CAM_FUNC_DBG("UNI_TOP_CTRL::_write2CQ + cq:0x%x,page:0x%x\n",cq,page);

    this->m_pDrv->cqAddModule(UNI_CTL_EN_);
    this->m_pDrv->cqAddModule(UNI_CTL_SEL_);
    this->m_pDrv->cqAddModule(UNI_CTL_FMT_);

    //this->m_pDrv->cqAddModule(UNI_ISP_UNP2_);
    this->m_pDrv->cqAddModule(UNI_ISP_QBN3_);
    this->m_pDrv->cqAddModule(UNI_ISP_SGG3_);
    this->m_pDrv->cqAddModule(UNI_ISP_FLK_);
    this->m_pDrv->cqAddModule(UNI_ISP_SGG2_A_);
    this->m_pDrv->cqAddModule(UNI_ISP_HDS_A_);
    this->m_pDrv->cqAddModule(UNI_ISP_EIS_);
    this->m_pDrv->cqAddModule(UNI_ISP_RSS_A_);


    //this->m_pDrv->cqNopModule(UNI_ISP_HDS_A_);  hds is binding with tg pixel mode


    this->m_pDrv->cqNopModule(UNI_ISP_EIS_);
    this->m_pDrv->cqNopModule(UNI_ISP_RSS_A_);
    this->m_pDrv->cqNopModule(UNI_ISP_SGG3_);
    this->m_pDrv->cqNopModule(UNI_ISP_FLK_);
    this->m_pDrv->cqNopModule(UNI_ISP_SGG2_A_);

    //
    CAM_FUNC_DBG("-\n");

    return 0;
}

MBOOL UNI_TOP_CTRL::suspend(void)
{
    MBOOL rst = MTRUE;
    MUINT32 i=0;
    this->m_pDrv = (ISP_DRV_CAM*)this->m_pIspDrv;

    CAM_FUNC_INF("UNI_TOP_CTRL::suspend + \n");

    IspFunction_B::fbcPhyLock_EISO[this->m_hwModule].lock();
    IspFunction_B::fbcPhyLock_FLKO[this->m_hwModule].lock();
    IspFunction_B::fbcPhyLock_RSSO[this->m_hwModule].lock();

    //keep fbc cnt
    if(UNI_READ_BITS(this->m_pUniDrv,CAM_UNI_FBC_EISO_A_CTL1,FBC_EN)){
        this->fbc_EISO.Raw = UNI_READ_REG(this->m_pUniDrv,CAM_UNI_FBC_EISO_A_CTL2);
    }

    if(UNI_READ_BITS(this->m_pUniDrv,CAM_UNI_FBC_FLKO_A_CTL1,FBC_EN)){
        this->fbc_FLKO.Raw = UNI_READ_REG(this->m_pUniDrv,CAM_UNI_FBC_FLKO_A_CTL2);
    }

    if(UNI_READ_BITS(this->m_pUniDrv,CAM_UNI_FBC_RSSO_A_CTL1,FBC_EN)){
        this->fbc_RSSO.Raw = UNI_READ_REG(this->m_pUniDrv,CAM_UNI_FBC_RSSO_A_CTL2);
    }


    //reset
    CAM_FUNC_INF("UNI reset\n");
    UNI_WRITE_REG(this->m_pUniDrv,CAM_UNI_TOP_SW_CTL,0x222);
    UNI_WRITE_REG(this->m_pUniDrv,CAM_UNI_TOP_SW_CTL,0x111);
#if 0
    while(UNI_READ_REG(this->m_pUniDrv,CAM_UNI_TOP_SW_CTL) != 0x222){
        CAM_FUNC_DBG("UNI reseting...\n");
    }
#endif
    UNI_WRITE_REG(this->m_pUniDrv,CAM_UNI_TOP_SW_CTL,0x0444);
    UNI_WRITE_REG(this->m_pUniDrv,CAM_UNI_TOP_SW_CTL,0x0);


    CAM_FUNC_DBG("UNI_TOP_CTRL::suspend - \n");
    return rst;

}

MBOOL UNI_TOP_CTRL::resume(void)
{
    MUINT32 frm_cnt = 0;

    CAM_FUNC_INF("UNI_TOP_CTRL::resume + \n");

    CAM_FUNC_INF("UNI_TOP_CTRL::resume SAVE_FBC:EISO_0x%08x FLKO_0x%08x RSSO_0x%08x",
            this->fbc_EISO.Raw, this->fbc_FLKO.Raw, this->fbc_RSSO.Raw);

    //keep fbc cnt
    if(UNI_READ_BITS(this->m_pUniDrv,CAM_UNI_FBC_EISO_A_CTL1,FBC_EN)){
        for (frm_cnt = 0; frm_cnt < this->fbc_EISO.Bits.FBC_CNT; frm_cnt++) {
            CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),CAM_CTL_FBC_RCNT_INC, EISO_RCNT_INC, 1);
        }
        UNI_WRITE_REG(this->m_pUniDrv, CAM_UNI_FBC_EISO_A_CTL2, this->fbc_EISO.Raw);
    }

    if(UNI_READ_BITS(this->m_pUniDrv,CAM_UNI_FBC_FLKO_A_CTL1,FBC_EN)){
        for (frm_cnt = 0; frm_cnt < this->fbc_FLKO.Bits.FBC_CNT; frm_cnt++) {
            CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),CAM_CTL_FBC_RCNT_INC, FLKO_RCNT_INC, 1);
        }
        UNI_WRITE_REG(this->m_pUniDrv, CAM_UNI_FBC_FLKO_A_CTL2, this->fbc_FLKO.Raw);
    }

    if(UNI_READ_BITS(this->m_pUniDrv,CAM_UNI_FBC_RSSO_A_CTL1,FBC_EN)){
        for (frm_cnt = 0; frm_cnt < this->fbc_RSSO.Bits.FBC_CNT; frm_cnt++) {
            CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),CAM_CTL_FBC_RCNT_INC, RSSO_RCNT_INC, 1);
        }
        UNI_WRITE_REG(this->m_pUniDrv, CAM_UNI_FBC_RSSO_A_CTL2, this->fbc_RSSO.Raw);
    }

    IspFunction_B::fbcPhyLock_EISO[this->m_hwModule].unlock();
    IspFunction_B::fbcPhyLock_FLKO[this->m_hwModule].unlock();
    IspFunction_B::fbcPhyLock_RSSO[this->m_hwModule].unlock();

    CAM_FUNC_INF("UNI_TOP_CTRL::resume PHYS_FBC:EISO_0x%08x FLKO_0x%08x RSSO_0x%08x",
            UNI_READ_REG(this->m_pUniDrv, CAM_UNI_FBC_EISO_A_CTL2),
            UNI_READ_REG(this->m_pUniDrv, CAM_UNI_FBC_FLKO_A_CTL2),
            UNI_READ_REG(this->m_pUniDrv, CAM_UNI_FBC_RSSO_A_CTL2));

    CAM_FUNC_DBG("UNI_TOP_CTRL::resume - \n");
    return MTRUE;
}


MINT32 CAM_MAGIC_CTRL::_config(void)
{
    E_ISP_CAM_CQ cq;
    MUINT32 page;

    Header_IMGO fh_imgo;
    Header_RRZO fh_rrzo;
    Header_UFEO fh_ufeo;
    Header_LCSO fh_lcso;
    Header_EISO fh_eiso;
    Header_RSSO fh_rsso;
    Header_AAO  fh_aao;
    Header_PDO  fh_pdo;
    Header_AFO  fh_afo;
    Header_FLKO fh_flko;

    this->m_pDrv = (ISP_DRV_CAM*)this->m_pIspDrv;
    //dbg log
    this->m_pDrv->getCurObjInfo(&this->m_hwModule,&cq,&page);

    CAM_FUNC_DBG("CAM_MAGIC_CTRL::_config+ cq:0x%x,page:0x%x,magic:0x%x\n",cq,page,this->m_nMagic);


    //cam
    fh_imgo.Header_Enque(Header_IMGO::E_Magic,this->m_pDrv,this->m_nMagic);
    fh_rrzo.Header_Enque(Header_RRZO::E_Magic,this->m_pDrv,this->m_nMagic);
    fh_ufeo.Header_Enque(Header_UFEO::E_Magic,this->m_pDrv,this->m_nMagic);
    fh_lcso.Header_Enque(Header_LCSO::E_Magic,this->m_pDrv,this->m_nMagic);
    fh_aao.Header_Enque(Header_AAO::E_Magic,this->m_pDrv,this->m_nMagic);
    fh_pdo.Header_Enque(Header_PDO::E_Magic,this->m_pDrv,this->m_nMagic);
    fh_afo.Header_Enque(Header_AFO::E_Magic,this->m_pDrv,this->m_nMagic);


    //UNI
    fh_eiso.Header_Enque(Header_EISO::E_Magic,this->m_pDrv,this->m_nMagic);
    fh_flko.Header_Enque(Header_FLKO::E_Magic,this->m_pDrv,this->m_nMagic);
    fh_rsso.Header_Enque(Header_RSSO::E_Magic,this->m_pDrv,this->m_nMagic);


    return 0;
}

MINT32 CAM_PMX_CTRL::_config(void)
{
    MINT32 ret = 0;
    MUINT32 tmp, pbn_en;
    CAM_REG_TG_SEN_GRAB_PXL TG_W;
    CAM_REG_TG_SEN_GRAB_LIN TG_H;
    E_ISP_CAM_CQ cq;
    MUINT32 page;

    this->m_pDrv = (ISP_DRV_CAM*)this->m_pIspDrv;
    //dbg log
    this->m_pDrv->getCurObjInfo(&this->m_hwModule,&cq,&page);
    CAM_FUNC_DBG("CAM_PMX_CTRL::_config + cq:0x%x,page:0x%x\n",cq,page);

    TG_W.Raw = CAM_READ_REG(this->m_pDrv->getPhyObj(),CAM_TG_SEN_GRAB_PXL);
    TG_H.Raw = CAM_READ_REG(this->m_pDrv->getPhyObj(),CAM_TG_SEN_GRAB_LIN);

    if(CAM_READ_BITS(this->m_pDrv,CAM_CTL_SEL,PMX_SEL) == PMX_SEL_0){
        CAM_WRITE_BITS(this->m_pDrv,CAM_PMX_CROP,PMX_STR_X,0);
        tmp = TG_W.Bits.PXL_E - TG_W.Bits.PXL_S;
        tmp = tmp >> (CAM_READ_BITS(this->m_pDrv,CAM_CTL_EN,BIN_EN) + CAM_READ_BITS(this->m_pDrv,CAM_CTL_EN,DBN_EN));
        CAM_WRITE_BITS(this->m_pDrv,CAM_PMX_CROP,PMX_END_X,(tmp - 1));
        tmp = TG_H.Bits.LIN_E - TG_H.Bits.LIN_S;
        tmp = tmp >> CAM_READ_BITS(this->m_pDrv,CAM_CTL_EN,BIN_EN);
        CAM_WRITE_REG(this->m_pDrv,CAM_PMX_VSIZE,tmp);
    }
    else{
        CAM_WRITE_BITS(this->m_pDrv,CAM_CTL_EN,PKP_EN, 0);
        pbn_en = CAM_READ_BITS(this->m_pDrv,CAM_CTL_EN,PBN_EN);
        CAM_WRITE_BITS(this->m_pDrv,CAM_PMX_CROP,PMX_STR_X,0);
        if (pbn_en) {
            tmp = TG_W.Bits.PXL_E - TG_W.Bits.PXL_S;
            tmp = PBIN_WIDTH(tmp);
            CAM_WRITE_BITS(this->m_pDrv,CAM_PMX_CROP,PMX_END_X,(tmp - 1));
            tmp = TG_H.Bits.LIN_E - TG_H.Bits.LIN_S;
            tmp = PBIN_HEIGHT(tmp);
            CAM_WRITE_REG(this->m_pDrv,CAM_PMX_VSIZE,tmp);
        } else {
            tmp = TG_W.Bits.PXL_E - TG_W.Bits.PXL_S;
            CAM_WRITE_BITS(this->m_pDrv,CAM_PMX_CROP,PMX_END_X,(tmp - 1));
            tmp = TG_H.Bits.LIN_E - TG_H.Bits.LIN_S;
            CAM_WRITE_REG(this->m_pDrv,CAM_PMX_VSIZE,tmp);
        }
    }

    if (page == 0) { /*reduce log, only show page 0*/
        CAM_FUNC_INF("CAM_PMX_CTRL::_config+ pmx(0x%x_0x%x),tg(0x%x_0x%x)\n",\
            CAM_READ_REG(this->m_pDrv,CAM_PMX_CROP),\
            CAM_READ_REG(this->m_pDrv,CAM_PMX_VSIZE),\
            (TG_W.Bits.PXL_E - TG_W.Bits.PXL_S),\
            (TG_H.Bits.LIN_E - TG_H.Bits.LIN_S));
    }
    return ret;
}

MINT32 CAM_RRZ_CTRL::_config(void)
{
    MINT32 ret = 0;
    IspSize rrz_in_size;
    IspRect rrz_in_roi_with_bin;
    MUINT32 rrz_h_step;
    MUINT32 rrz_v_step;
    MUINT32 rrz_ctl = 0;
    MUINT32 dbn_en, bin_en;

    E_ISP_CAM_CQ cq;
    MUINT32 page;
    Header_RRZO fh_rrzo;
    this->m_pDrv = (ISP_DRV_CAM*)this->m_pIspDrv;
    //dbg log
    this->m_pDrv->getCurObjInfo(&this->m_hwModule,&cq,&page);

#if 0   //can't user dmx , it will have bug under twin mode. and why don't use cam_a's + cam_b's? because twin is the last operation in enque flow.
    rrz_in_size.w = CAM_READ_BITS(this->m_pDrv,CAM_DMX_CROP,DMX_END_X) - CAM_READ_BITS(this->m_pDrv,CAM_DMX_CROP,DMX_STR_X) + 1;
    rrz_in_size.h = CAM_READ_BITS(this->m_pDrv,CAM_DMX_VSIZE,DMX_HT);
#else
    {
        CAM_REG_TG_SEN_GRAB_PXL TG_W;
        CAM_REG_TG_SEN_GRAB_LIN TG_H;
        TG_W.Raw = CAM_READ_REG(this->m_pDrv->getPhyObj(),CAM_TG_SEN_GRAB_PXL);
        TG_H.Raw = CAM_READ_REG(this->m_pDrv->getPhyObj(),CAM_TG_SEN_GRAB_LIN);

        rrz_in_size.h = (TG_H.Bits.LIN_E - TG_H.Bits.LIN_S);
        rrz_in_size.w = (TG_W.Bits.PXL_E - TG_W.Bits.PXL_S);
    }
#endif

    // If DBN=1, rrz in setting must be divide by 2; if DBN = 1 and BIN = 2, imgo setting must be divide by 4
    dbn_en = CAM_READ_BITS(this->m_pDrv,CAM_CTL_EN,DBN_EN);
    bin_en = CAM_READ_BITS(this->m_pDrv,CAM_CTL_EN,BIN_EN);
    rrz_in_size.w >>= (dbn_en + bin_en);
    rrz_in_size.h >>= (bin_en);
    rrz_in_roi_with_bin.x = this->rrz_in_roi.x >> (bin_en);
    rrz_in_roi_with_bin.y = this->rrz_in_roi.y >> (bin_en);
    rrz_in_roi_with_bin.floatX = this->rrz_in_roi.floatX >> (bin_en);
    rrz_in_roi_with_bin.floatY = this->rrz_in_roi.floatY >> (bin_en);
    rrz_in_roi_with_bin.w = this->rrz_in_roi.w >> (bin_en);
    rrz_in_roi_with_bin.h = this->rrz_in_roi.h >> (bin_en);


    rrz_h_step = this->calCoefStep(rrz_in_size.w,rrz_in_roi_with_bin.w,rrz_in_roi_with_bin.x,rrz_in_roi_with_bin.floatX,this->rrz_out_size.w);
    rrz_v_step = this->calCoefStep(rrz_in_size.h,rrz_in_roi_with_bin.h,rrz_in_roi_with_bin.y,rrz_in_roi_with_bin.floatY,this->rrz_out_size.h);
    rrz_ctl = (this->getCoefTbl(rrz_in_roi_with_bin.w,this->rrz_out_size.w)<<16) | \
          (this->getCoefTbl(rrz_in_roi_with_bin.h,this->rrz_out_size.h)<<24) | \
          0x03; //enable

    if(rrz_ctl == 0){
        ret = 1;
    }

    CAM_FUNC_INF("CAM_RRZ_CTRL::_config+ cq:0x%x,page:0x%x,rrz_in_w/h(%d/%d),rrz_crop_x/y/fx/fy/w/h(%d/%d/%d/%d/%d/%d),rrz_out_w/h(%d/%d),rrz_h/v_step(%d/%d),dbn/bin(%d/%d)\n",cq,page,\
            rrz_in_size.w, \
            rrz_in_size.h, \
            this->rrz_in_roi.x, \
            this->rrz_in_roi.y, \
            this->rrz_in_roi.floatX, \
            this->rrz_in_roi.floatY, \
            this->rrz_in_roi.w, \
            this->rrz_in_roi.h, \
            this->rrz_out_size.w, \
            this->rrz_out_size.h, \
            rrz_h_step, rrz_v_step,
            dbn_en, bin_en);

    CAM_WRITE_BITS(this->m_pDrv, CAM_RRZ_MODE_CTL,RRZ_PRF_BLD, this->PicQ.PRF_BLD);
    CAM_WRITE_BITS(this->m_pDrv, CAM_RRZ_MODE_CTL,RRZ_PRF, this->PicQ.PRF);
    CAM_WRITE_BITS(this->m_pDrv, CAM_RRZ_MODE_CTL,RRZ_BLD_SL, this->PicQ.BLD_SL);
    CAM_WRITE_BITS(this->m_pDrv, CAM_RRZ_MODE_CTL,RRZ_CR_MODE, this->PicQ.CR_MODE);


    CAM_WRITE_BITS(this->m_pDrv, CAM_RRZ_MODE_TH,RRZ_TH_MD, this->PicQ.TH_MD);
    CAM_WRITE_BITS(this->m_pDrv, CAM_RRZ_MODE_TH,RRZ_TH_HI, this->PicQ.TH_HI);
    CAM_WRITE_BITS(this->m_pDrv, CAM_RRZ_MODE_TH,RRZ_TH_LO, this->PicQ.TH_LO);
    CAM_WRITE_BITS(this->m_pDrv, CAM_RRZ_MODE_TH,RRZ_TH_MD2, this->PicQ.TH_MD2);

    CAM_WRITE_REG(this->m_pDrv, CAM_RRZ_CTL, rrz_ctl);
    CAM_WRITE_REG(this->m_pDrv, CAM_RRZ_IN_IMG, ((rrz_in_size.h<<16) | rrz_in_size.w));
    CAM_WRITE_REG(this->m_pDrv, CAM_RRZ_OUT_IMG, ((this->rrz_out_size.h<<16) | this->rrz_out_size.w) );
    CAM_WRITE_REG(this->m_pDrv, CAM_RRZ_HORI_STEP, rrz_h_step);
    CAM_WRITE_REG(this->m_pDrv, CAM_RRZ_VERT_STEP, rrz_v_step);
    CAM_WRITE_REG(this->m_pDrv, CAM_RRZ_HORI_INT_OFST, rrz_in_roi_with_bin.x);
    CAM_WRITE_REG(this->m_pDrv, CAM_RRZ_HORI_SUB_OFST, rrz_in_roi_with_bin.floatX);
    CAM_WRITE_REG(this->m_pDrv, CAM_RRZ_VERT_INT_OFST, rrz_in_roi_with_bin.y);
    CAM_WRITE_REG(this->m_pDrv, CAM_RRZ_VERT_SUB_OFST, rrz_in_roi_with_bin.floatY);

    //update rmx crop
    CAM_WRITE_REG(this->m_pDrv,CAM_RMX_CROP,( ((this->rrz_out_size.w-1)<<16) | 0) );
    CAM_WRITE_BITS(this->m_pDrv,CAM_RMX_VSIZE,RMX_HT,this->rrz_out_size.h);

    fh_rrzo.Header_Enque(Header_RRZO::E_RRZ_CRP_START,this->m_pDrv,((rrz_in_roi_with_bin.y<<16)|rrz_in_roi_with_bin.x));
    fh_rrzo.Header_Enque(Header_RRZO::E_RRZ_CRP_SIZE,this->m_pDrv,((rrz_in_roi_with_bin.h<<16)|rrz_in_roi_with_bin.w));
    fh_rrzo.Header_Enque(Header_RRZO::E_RRZ_DST_SIZE,this->m_pDrv,((this->rrz_out_size.h<<16) | this->rrz_out_size.w));

    this->m_pDrv->cqApbModule(CAM_ISP_RRZ_);
    return ret;
}

MUINT32 CAM_RRZ_CTRL::calCoefStep(MUINT32 in,MUINT32 crop,MUINT32 crop_ofst,MUINT32 crop_sub_ofst,MUINT32 out)
{
    /*
       MUINT32 rrz_in_w        = rrz_cfg.rrz_in_size.w;
       MUINT32 rrz_in_crop_w   = rrz_cfg.rrz_crop.w;

       MUINT32 left_ofst = rrz_cfg.rrz_crop.x;
       MUINT32 left_sub_ofst  = rrz_cfg.rrz_crop.floatX;
       MUINT32 right_ofst      = rrz_in_w- left_ofst - rrz_in_crop_w - (left_sub_ofst?1:0);

       MUINT32 rrz_out_w = rrz_cfg.rrz_crop.w;

    //TODO:define in dual_cal.h. remove later
#define DUAL_RRZ_PREC (1<<15)

MUINT32 rrz_h_step = ( (rrz_in_w - 1 - left_ofst - right_ofst)*DUAL_RRZ_PREC ) / (rrz_out_w - 1);
     */
#define RRZ_PREC    (32768)

    // Add to fix devide by zero NE issue caused by /(out -1)
    if(out <= 1){
        out = 2;
        CAM_FUNC_INF("out <=1, reset to 2!");
    }

    MUINT32 rrz_step = ( ( in - 1 - crop_ofst - ( in- crop_ofst - crop) - (crop_sub_ofst?1:0) ) )*RRZ_PREC  / (out - 1);

    return rrz_step;
}

MUINT32 CAM_RRZ_CTRL::getCoefTbl(MUINT32 resize_in,MUINT32 resize_out)
{
    /*
       resizing ratio  suggested table
       1.0~0.9           0~3
       0.9~0.8           4~7
       0.8~0.7           8~12
       0.7~0.6          13~18
       0.6~0.5          19~24
       0.5~0.4          25~30
     */

    struct {
        MUINT32 left;
        MUINT32 right;
    }map_tbl[11] = {{1,1},{1,1},{30,30},{30,30},{25,30},{19,24},{13,18},{8,12},{4,7},{0,3},{0,3}};

    MUINT32 scale_ratio_int = (MUINT32)( (resize_out*10) / resize_in );
    MUINT32 scale_ratio_int_100 = (MUINT32)( (resize_out*100) / resize_in );
    MUINT32 table = 0;

    CAM_FUNC_DBG("+in/out(%d/%d),ratio(%d)",resize_in,resize_out,scale_ratio_int);

    if ( 0 == resize_in || 0 == resize_out ) {
        CAM_FUNC_ERR("rrz param error:0 size(%d/%d)",resize_in,resize_out);
        return 0;
    }
    if ( resize_out > resize_in ) {
        CAM_FUNC_ERR("rrz param error:not support size up");
        return 0;
    }
    if ( 25 > scale_ratio_int_100 ) {
        CAM_FUNC_ERR("rrz param error:max 0.25(%d/%d)",resize_in,resize_out);
        return 0;
    }

    if ( resize_out == resize_in ) {
        table = 0x1F; //table index 31
    }
    else {
        table = map_tbl[scale_ratio_int].left + \
            ( ( (scale_ratio_int+1)*10 - scale_ratio_int_100 )* \
              ( map_tbl[scale_ratio_int].right-map_tbl[scale_ratio_int].left ) )/10;
        table = (table > 0x1f)? (0x1f):(table);
    }

    CAM_FUNC_DBG("table(%d)",table);

    return table;
}


/*/////////////////////////////////////////////////////////////////////////////
  ISP_RAW_PIPE
/////////////////////////////////////////////////////////////////////////////*/
MINT32 CAM_RAW_PIPE::_config( void )
{
    E_ISP_CAM_CQ cq;
    MUINT32 page;
    CAM_REG_CTL_FMT_SEL fmt_sel;
    Header_IMGO fh_imgo;
    Header_RRZO fh_rrzo;
    Header_UFEO  fh_ufeo;
    Header_PDO  fh_pdo;
    CAM_REG_TG_SEN_GRAB_PXL TG_W;
    CAM_REG_TG_SEN_GRAB_LIN TG_H;
    MUINT32 pixmode = 0;
    MUINT32 qbn_pixmode = 0;
    MUINT32 tmp;
#if 0
    MUINT32 pbn_en, sepMode = 0;
#endif
    MUINT32 bTwin;
    this->m_pDrv = (ISP_DRV_CAM*)this->m_pIspDrv;
    //dbg log
    this->m_pDrv->getCurObjInfo(&this->m_hwModule,&cq,&page);
    switch(this->m_Source){
        case IF_RAWI_CAM_A:
        case IF_RAWI_CAM_B:
            pixmode = UNI_READ_BITS(this->m_pUniDrv,CAM_UNI_TOP_FMT_SEL,PIX_BUS_RAWI);
            break;
        case IF_CAM_A:
        case IF_CAM_B:
            pixmode = CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_TG_SEN_MODE,DBL_DATA_BUS) + \
                  CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_TG_SEN_MODE,DBL_DATA_BUS1);
            break;
        default:
            CAM_FUNC_ERR("unsupported source:0x%x\n",this->m_Source);
            return 1;
            break;
    }

    bTwin = CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_CTL_TWIN_STATUS,TWIN_EN);

    if (page == 0) { /*reduce log, only show page 0*/
        CAM_FUNC_INF("CAM_RAW_PIPE::_config+ cq:0x%x,page:0x%x,rrz_en:%d,tg_pix_mode:0x%x,cam source:0x%x, twin:0x%x\n",cq,page,\
            CAM_READ_BITS(this->m_pDrv,CAM_CTL_EN,RRZ_EN),\
            pixmode,\
            this->m_Source,\
            bTwin);
    }

    //default always enable
    CAM_WRITE_BITS(this->m_pDrv,CAM_CTL_EN,DMX_EN, 1);
    CAM_WRITE_BITS(this->m_pDrv,CAM_CTL_EN,BMX_EN, 1);
    CAM_WRITE_BITS(this->m_pDrv,CAM_CTL_EN,RCP_EN, 1);
    CAM_WRITE_BITS(this->m_pDrv,CAM_CTL_EN,PMX_EN, 1);
    CAM_WRITE_BITS(this->m_pDrv,CAM_CTL_DMA_EN,AMX_EN, 1);

    /// Enable RMX only TWIN_EN & RRZ_EN, or disable RMX.
    /// Remider: Must disable RMX in single cam mode or some lines will occur in 4:3 preview image
    if(bTwin == 1 && CAM_READ_BITS(this->m_pDrv,CAM_CTL_EN,RRZ_EN) == 1){
        CAM_WRITE_BITS(this->m_pDrv,CAM_CTL_EN,RMX_EN, 1);
    } else{
        CAM_WRITE_BITS(this->m_pDrv,CAM_CTL_EN,RMX_EN, 0);
    }


    CAM_WRITE_BITS(this->m_pDrv,CAM_CTL_EN,QBIN1_EN, 1);
    CAM_WRITE_BITS(this->m_pDrv,CAM_CTL_EN,QBIN2_EN, 1);

    CAM_WRITE_BITS(this->m_pDrv,CAM_CTL_EN,PKP_EN, 1);


    //xmx cropping and single_mode under tiwn mode is cal. by twin_Drv
    //here is non-twin setting
    //TG cropping must be still applied even if path is rawi
    TG_W.Raw = CAM_READ_REG(this->m_pDrv->getPhyObj(),CAM_TG_SEN_GRAB_PXL);
    TG_H.Raw = CAM_READ_REG(this->m_pDrv->getPhyObj(),CAM_TG_SEN_GRAB_LIN);
    //dmx
    CAM_WRITE_BITS(this->m_pDrv,CAM_DMX_CTL,DMX_SRAM_SIZE, DMX_SRAM);
    CAM_WRITE_BITS(this->m_pDrv,CAM_DMX_CTL,DMX_EDGE, 0xf);
    CAM_WRITE_BITS(this->m_pDrv,CAM_DMX_CTL,DMX_EDGE_SET, 0);

    CAM_WRITE_BITS(this->m_pDrv,CAM_DMX_CROP,DMX_STR_X,0);
    CAM_WRITE_BITS(this->m_pDrv,CAM_DMX_CROP,DMX_END_X,(TG_W.Bits.PXL_E - TG_W.Bits.PXL_S - 1));

    CAM_WRITE_REG(this->m_pDrv,CAM_DMX_VSIZE,(TG_H.Bits.LIN_E - TG_H.Bits.LIN_S));

    //rmx
    CAM_WRITE_BITS(this->m_pDrv,CAM_RMX_CTL,RMX_SRAM_SIZE, RMX_SRAM);
    CAM_WRITE_BITS(this->m_pDrv,CAM_RMX_CTL,RMX_EDGE, 0xf);
    CAM_WRITE_BITS(this->m_pDrv,CAM_RMX_CTL,RMX_EDGE_SET, 0);
    CAM_WRITE_BITS(this->m_pDrv,CAM_RMX_CTL,RMX_EDGE, 0xf);
    //single_mode1 & 2 are using exclusive method.   signle_mode1 is main path, signle_mode2 is from other path
    CAM_WRITE_BITS(this->m_pDrv,CAM_RMX_CTL,RMX_SINGLE_MODE_1, 1);
    CAM_WRITE_BITS(this->m_pDrv,CAM_RMX_CTL,RMX_SINGLE_MODE_2, 0);

    //removed, move into rrz_ctrl
    //CAM_WRITE_BITS(this->m_pDrv,CAM_RMX_CROP,RMX_STR_X,0);
    //CAM_WRITE_BITS(this->m_pDrv,CAM_RMX_CROP,RMX_END_X,(this->rrz_out_size.w - 1));
    //CAM_WRITE_REG(this->m_pDrv,CAM_RMX_VSIZE,this->rrz_out_size.h);

    //bmx
    CAM_WRITE_BITS(this->m_pDrv,CAM_BMX_CTL,BMX_SRAM_SIZE, BMX_SRAM);
    CAM_WRITE_BITS(this->m_pDrv,CAM_BMX_CTL,BMX_EDGE, 0xf);
    CAM_WRITE_BITS(this->m_pDrv,CAM_BMX_CTL,BMX_EDGE_SET, 0);
    CAM_WRITE_BITS(this->m_pDrv,CAM_BMX_CTL,BMX_EDGE, 0xf);
    CAM_WRITE_BITS(this->m_pDrv,CAM_BMX_CTL,BMX_SINGLE_MODE_1, 1);
    CAM_WRITE_BITS(this->m_pDrv,CAM_BMX_CTL,BMX_SINGLE_MODE_2, 0);

    //amx in single mode {MODE_1,MODE_2} = 2b10 same as bmx
    CAM_WRITE_BITS(this->m_pDrv,CAM_AMX_CTL,AMX_SRAM_SIZE, AMX_SRAM);
    CAM_WRITE_BITS(this->m_pDrv,CAM_AMX_CTL,AMX_EDGE, 0xf);
    CAM_WRITE_BITS(this->m_pDrv,CAM_AMX_CTL,AMX_EDGE_SET, 0);
    CAM_WRITE_BITS(this->m_pDrv,CAM_AMX_CTL,AMX_EDGE, 0xf);
    CAM_WRITE_BITS(this->m_pDrv,CAM_AMX_CTL,AMX_SINGLE_MODE_1, 1);
    CAM_WRITE_BITS(this->m_pDrv,CAM_AMX_CTL,AMX_SINGLE_MODE_2, 0);

    // dbn: x-dir => /2, y-dir => same; bin: x-dir => /2, y-dir => /2
    CAM_WRITE_BITS(this->m_pDrv,CAM_BMX_CROP,BMX_STR_X,0);
    CAM_WRITE_BITS(this->m_pDrv,CAM_AMX_CROP,AMX_STR_X,0);
    tmp = TG_W.Bits.PXL_E - TG_W.Bits.PXL_S;
    tmp = tmp >> (CAM_READ_BITS(this->m_pDrv,CAM_CTL_EN,BIN_EN) + CAM_READ_BITS(this->m_pDrv,CAM_CTL_EN,DBN_EN));
    CAM_WRITE_BITS(this->m_pDrv,CAM_BMX_CROP,BMX_END_X,(tmp - 1));
    CAM_WRITE_BITS(this->m_pDrv,CAM_AMX_CROP,AMX_END_X,(tmp - 1));
    tmp = TG_H.Bits.LIN_E - TG_H.Bits.LIN_S;
    tmp = tmp >> CAM_READ_BITS(this->m_pDrv,CAM_CTL_EN,BIN_EN);
    CAM_WRITE_REG(this->m_pDrv,CAM_BMX_VSIZE,tmp);
    CAM_WRITE_REG(this->m_pDrv,CAM_AMX_VSIZE,tmp);


    //pmx
    CAM_WRITE_BITS(this->m_pDrv,CAM_PMX_CTL,PMX_SRAM_SIZE, PMX_SRAM);
    CAM_WRITE_BITS(this->m_pDrv,CAM_PMX_CTL,PMX_EDGE, 0xf);
    CAM_WRITE_BITS(this->m_pDrv,CAM_PMX_CTL,PMX_EDGE_SET, 0);
    CAM_WRITE_BITS(this->m_pDrv,CAM_PMX_CTL,PMX_EDGE, 0xf);
    CAM_WRITE_BITS(this->m_pDrv,CAM_PMX_CTL,PMX_SINGLE_MODE_1, 1);
    CAM_WRITE_BITS(this->m_pDrv,CAM_PMX_CTL,PMX_SINGLE_MODE_2, 0);

    if((CAM_READ_BITS(this->m_pDrv,CAM_CTL_EN,PBN_EN) == 1) && \
            (CAM_READ_REG(this->m_pDrv,CAM_CTL_SEL) & PMX_SEL_1)){
        tmp = 1;
    }
    else
        tmp = 0;
    CAM_WRITE_BITS(this->m_pDrv,CAM_PMX_CTL,PMX_DUAL_PD_MODE, tmp);

#if 0//move to PMX_CTRL
    if(CAM_READ_BITS(this->m_pDrv,CAM_CTL_SEL,PMX_SEL) == PMX_SEL_0){
        CAM_WRITE_BITS(this->m_pDrv,CAM_PMX_CROP,PMX_STR_X,0);
        tmp = TG_W.Bits.PXL_E - TG_W.Bits.PXL_S;
        tmp = tmp >> (CAM_READ_BITS(this->m_pDrv,CAM_CTL_EN,BIN_EN) + CAM_READ_BITS(this->m_pDrv,CAM_CTL_EN,DBN_EN));
        CAM_WRITE_BITS(this->m_pDrv,CAM_PMX_CROP,PMX_END_X,(tmp - 1));
        tmp = TG_H.Bits.LIN_E - TG_H.Bits.LIN_S;
        tmp = tmp >> CAM_READ_BITS(this->m_pDrv,CAM_CTL_EN,BIN_EN);
        CAM_WRITE_REG(this->m_pDrv,CAM_PMX_VSIZE,tmp);
    }
    else{
        CAM_WRITE_BITS(this->m_pDrv,CAM_CTL_EN,PKP_EN, 0);
        pbn_en = CAM_READ_BITS(this->m_pDrv,CAM_CTL_EN,PBN_EN);
        sepMode = CAM_READ_BITS(this->m_pDrv,CAM_PBN_TYPE,PBN_SEP);
        CAM_WRITE_BITS(this->m_pDrv,CAM_PMX_CROP,PMX_STR_X,0);
        if (pbn_en) {
            tmp = TG_W.Bits.PXL_E - TG_W.Bits.PXL_S;
            tmp = (tmp >> 4) << (1-sepMode);
            CAM_WRITE_BITS(this->m_pDrv,CAM_PMX_CROP,PMX_END_X,(tmp - 1));
            tmp = TG_H.Bits.LIN_E - TG_H.Bits.LIN_S;
            tmp = (tmp >> 3) << sepMode;
            CAM_WRITE_REG(this->m_pDrv,CAM_PMX_VSIZE,tmp);
        } else {
            tmp = TG_W.Bits.PXL_E - TG_W.Bits.PXL_S;
            CAM_WRITE_BITS(this->m_pDrv,CAM_PMX_CROP,PMX_END_X,(tmp - 1));
            tmp = TG_H.Bits.LIN_E - TG_H.Bits.LIN_S;
            CAM_WRITE_REG(this->m_pDrv,CAM_PMX_VSIZE,tmp);
        }
    }
#endif
    //rcrop
    tmp = CAM_READ_BITS(this->m_pDrv,CAM_BMX_CROP,BMX_END_X) - CAM_READ_BITS(this->m_pDrv,CAM_BMX_CROP,BMX_STR_X) + 1;
    CAM_WRITE_BITS(this->m_pDrv,CAM_RCP_CROP_CON1,RCP_STR_X,0);
    CAM_WRITE_BITS(this->m_pDrv,CAM_RCP_CROP_CON1,RCP_END_X,tmp - 1);
    tmp = CAM_READ_BITS(this->m_pDrv,CAM_BMX_VSIZE,BMX_HT);
    CAM_WRITE_BITS(this->m_pDrv,CAM_RCP_CROP_CON2,RCP_STR_Y,0);
    CAM_WRITE_BITS(this->m_pDrv,CAM_RCP_CROP_CON2,RCP_END_Y,tmp - 1);


    qbn_pixmode = (pixmode >> (CAM_READ_BITS(this->m_pDrv,CAM_CTL_EN,BIN_EN) + CAM_READ_BITS(this->m_pDrv,CAM_CTL_EN,DBN_EN)));
    //bin
    CAM_WRITE_REG(this->m_pDrv->getPhyObj(),CAM_BIN_CTL, 0x00000C00);//from CY
    CAM_WRITE_REG(this->m_pDrv->getPhyObj(),CAM_BIN_FTH, 0x0C090603);
    CAM_WRITE_REG(this->m_pDrv->getPhyObj(),CAM_BIN_SPARE, 0x00000000);
    CAM_WRITE_REG(this->m_pDrv,CAM_BIN_CTL, 0x00000C00);
    CAM_WRITE_REG(this->m_pDrv,CAM_BIN_FTH, 0x0C090603);
    CAM_WRITE_REG(this->m_pDrv,CAM_BIN_SPARE, 0x00000000);

    //qbin1
    CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),CAM_QBN1_MODE, QBN_ACC_MODE,1);//always use average
    CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),CAM_QBN1_MODE, QBN_ACC,qbn_pixmode);
    //qbin2
    CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),CAM_QBN2_MODE, QBN_ACC_MODE,1);//always use average
    CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),CAM_QBN2_MODE, QBN_ACC,qbn_pixmode);
    //bin3 & hds setting is in UNI_TOP

    //image header
    fmt_sel.Raw = CAM_READ_REG(this->m_pDrv,CAM_CTL_FMT_SEL);
    fh_imgo.Header_Enque(Header_IMGO::E_PIX_ID,this->m_pDrv,fmt_sel.Bits.PIX_ID);
    fh_imgo.Header_Enque(Header_IMGO::E_FMT,this->m_pDrv,fmt_sel.Bits.IMGO_FMT);
    fh_rrzo.Header_Enque(Header_RRZO::E_PIX_ID,this->m_pDrv,fmt_sel.Bits.PIX_ID);
    fh_rrzo.Header_Enque(Header_RRZO::E_FMT,this->m_pDrv,fmt_sel.Bits.RRZO_FMT);
    fh_ufeo.Header_Enque(Header_UFEO::E_FMT,this->m_pDrv,fmt_sel.Bits.RRZO_FMT);

    fh_pdo.Header_Enque(Header_PDO::E_PMX_A_CROP,this->m_pDrv,\
            CAM_READ_REG(this->m_pDrv,CAM_PMX_CROP));

    fh_pdo.Header_Enque(Header_PDO::E_BMX_A_CROP,this->m_pDrv,\
            CAM_READ_REG(this->m_pDrv,CAM_BMX_CROP));

    return 0;
}

MINT32 CAM_RAW_PIPE::_write2CQ(void)
{
    E_ISP_CAM_CQ cq;
    MUINT32 page;
    this->m_pDrv = (ISP_DRV_CAM*)this->m_pIspDrv;
    //dbg log
    this->m_pDrv->getCurObjInfo(&this->m_hwModule,&cq,&page);

    CAM_FUNC_DBG("CAM_RAW_PIPE::_write2CQ+ cq:0x%x,page:0x%x\n",cq,page);


    this->m_pDrv->cqAddModule(CAM_ISP_DMX_);
    this->m_pDrv->cqAddModule(CAM_ISP_BMX_);
    this->m_pDrv->cqAddModule(CAM_ISP_RMX_);
    this->m_pDrv->cqAddModule(CAM_ISP_PMX_);
    this->m_pDrv->cqAddModule(CAM_ISP_RCP_);
    this->m_pDrv->cqAddModule(CAM_ISP_AMX_);

    this->m_pDrv->cqAddModule(CAM_ISP_RRZ_);
    this->m_pDrv->cqAddModule(CAM_ISP_PBN_);
    this->m_pDrv->cqAddModule(CAM_ISP_DBN_);
    this->m_pDrv->cqAddModule(CAM_ISP_BIN_);
    this->m_pDrv->cqAddModule(CAM_ISP_DBS_);

    this->m_pDrv->cqAddModule(CAM_ISP_SL2F_);
    this->m_pDrv->cqAddModule(CAM_ISP_OBC_);
    this->m_pDrv->cqAddModule(CAM_ISP_RMG_);
    this->m_pDrv->cqAddModule(CAM_ISP_BNR_);
    this->m_pDrv->cqAddModule(CAM_ISP_RMM_);
    this->m_pDrv->cqAddModule(CAM_ISP_CAC_);
    this->m_pDrv->cqAddModule(CAM_ISP_LSC_);

    this->m_pDrv->cqAddModule(CAM_ISP_RPG_);
    this->m_pDrv->cqAddModule(CAM_ISP_UFE_);
    this->m_pDrv->cqAddModule(CAM_ISP_RCP3_);
    this->m_pDrv->cqAddModule(CAM_ISP_CPG_);
    //this->m_pDrv->cqAddModule(CAM_ISP_QBN1_);
    //this->m_pDrv->cqAddModule(CAM_ISP_QBN2_);
    this->m_pDrv->cqAddModule(CAM_ISP_LCS_);
    this->m_pDrv->cqAddModule(CAM_ISP_AE_);
    this->m_pDrv->cqAddModule(CAM_ISP_AWB_);
    this->m_pDrv->cqAddModule(CAM_ISP_SGG1_);
    this->m_pDrv->cqAddModule(CAM_ISP_AF_);

    //NOP function , these func should apb when being used at tuning
    this->m_pDrv->cqNopModule(CAM_ISP_RRZ_);
    this->m_pDrv->cqNopModule(CAM_ISP_PBN_);
    this->m_pDrv->cqNopModule(CAM_ISP_DBN_);
    this->m_pDrv->cqNopModule(CAM_ISP_BIN_);
    this->m_pDrv->cqNopModule(CAM_ISP_DBS_);
    this->m_pDrv->cqNopModule(CAM_ISP_SL2F_);
    this->m_pDrv->cqNopModule(CAM_ISP_OBC_);
    this->m_pDrv->cqNopModule(CAM_ISP_RMG_);
    this->m_pDrv->cqNopModule(CAM_ISP_BNR_);
    this->m_pDrv->cqNopModule(CAM_ISP_RMM_);
    this->m_pDrv->cqNopModule(CAM_ISP_CAC_);
    this->m_pDrv->cqNopModule(CAM_ISP_LSC_);
    this->m_pDrv->cqNopModule(CAM_ISP_RPG_);
    this->m_pDrv->cqNopModule(CAM_ISP_UFE_);
    this->m_pDrv->cqNopModule(CAM_ISP_RCP3_);
    this->m_pDrv->cqNopModule(CAM_ISP_CPG_);
    //this->m_pDrv->cqNopModule(CAM_ISP_QBN1_);
    //this->m_pDrv->cqNopModule(CAM_ISP_QBN2_);
    this->m_pDrv->cqNopModule(CAM_ISP_LCS_);
    this->m_pDrv->cqNopModule(CAM_ISP_AE_);
    this->m_pDrv->cqNopModule(CAM_ISP_AWB_);
    this->m_pDrv->cqNopModule(CAM_ISP_AF_);

    return 0;
}


MBOOL CAM_RAW_PIPE::setIMG_SEL(void)
{
    E_ISP_CAM_CQ cq;
    MUINT32 page;
    Header_IMGO fh_imgo;
    this->m_pDrv = (ISP_DRV_CAM*)this->m_pIspDrv;
    //dbg log
    this->m_pDrv->getCurObjInfo(&this->m_hwModule,&cq,&page);

    switch(this->m_RawType){
        case E_FromTG:
            CAM_FUNC_DBG("CAM_RAW_PIPE::setIMG_SEL+ cq:0x%x,page:0x%x,RawData from TG\n",cq,page);
            CAM_WRITE_BITS(this->m_pDrv,CAM_CTL_SEL, IMG_SEL, 1);
            fh_imgo.Header_Enque(Header_IMGO::E_RAW_TYPE,this->m_pDrv,E_FromTG);
            break;
        case E_BeforLSC:
            CAM_FUNC_DBG("CAM_RAW_PIPE::setIMG_SEL+ cq:0x%x,page:0x%x,RawData before LSC\n",cq,page);
            CAM_WRITE_BITS(this->m_pDrv,CAM_CTL_SEL, IMG_SEL, 2);
            fh_imgo.Header_Enque(Header_IMGO::E_RAW_TYPE,this->m_pDrv,E_BeforLSC);
            break;
        case E_AFTERLSC:
            CAM_FUNC_DBG("CAM_RAW_PIPE::setIMG_SEL+ cq:0x%x,page:0x%x,RawData after LSC\n",cq,page);
            CAM_WRITE_BITS(this->m_pDrv,CAM_CTL_SEL, IMG_SEL, 0);
            fh_imgo.Header_Enque(Header_IMGO::E_RAW_TYPE,this->m_pDrv,E_AFTERLSC);
            break;
        default:
            CAM_FUNC_ERR("Raw Data support only 0:from TG,1:before LSC,2:after LSC\n");
            return MFALSE;
            break;
    }
    return MTRUE;
}

MBOOL CAM_RAW_PIPE::setEIS(void)
{
    MBOOL ret = MTRUE;
    DMA_EISO eiso;


    this->m_pDrv = (ISP_DRV_CAM*)this->m_pIspDrv;
    if(this->m_pP1Tuning){
        EIS_REG_CFG _eis_cfg;
        LMV_CFG  lmv_cfg;
        LMV_INPUT_INFO input;
        CAM_REG_TG_SEN_GRAB_PXL TG_W;
        CAM_REG_TG_SEN_GRAB_LIN TG_H;
        Header_RRZO fh_rrzo;
        MUINT32 tmp;

        MUINT32 bin = CAM_READ_BITS(this->m_pDrv,CAM_CTL_EN,BIN_EN);
        MUINT32 dbin = CAM_READ_BITS(this->m_pDrv,CAM_CTL_EN,DBN_EN);

        if(CAM_READ_BITS(this->m_pDrv,CAM_CTL_EN,RRZ_EN) != 1){
            CAM_FUNC_ERR("RRZ must be opened in Bayer domain when EIS is needed\n");
            return MFALSE;
        }

        //only hds , bin information is not needed
        input.pixMode = UNI_READ_BITS(this->m_pDrv,CAM_UNI_HDS_A_MODE,HDS_DS);


        TG_W.Raw = CAM_READ_REG(this->m_pDrv->getPhyObj(),CAM_TG_SEN_GRAB_PXL);
        input.sTGOut.w = TG_W.Bits.PXL_E - TG_W.Bits.PXL_S;
        TG_H.Raw = CAM_READ_REG(this->m_pDrv->getPhyObj(),CAM_TG_SEN_GRAB_LIN);
        input.sTGOut.h = TG_H.Bits.LIN_E - TG_H.Bits.LIN_S;


        //
        if(CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_CTL_SEL,HDS1_SEL) == HDS1_SEL_0){    //rrz have no enabled
            input.sHBINOut.w = input.sTGOut.w >> input.pixMode;
            input.sHBINOut.h = input.sTGOut.h;
        }
        else{
            tmp = fh_rrzo.GetRegInfo(Header_RRZO::E_RRZ_DST_SIZE,this->m_pDrv);
            input.sHBINOut.w = (tmp & 0xffff) >> input.pixMode;
            input.sHBINOut.h = (tmp>>16);
        }

#if 0   //rmx output will be affected at twin mode
        //rmx always cropping full size image compare to rrz output
        input.sRMXOut.w = CAM_READ_BITS(this->m_pDrv,CAM_RRZ_OUT_IMG,RRZ_OUT_WD);
        input.sRMXOut.h = CAM_READ_BITS(this->m_pDrv,CAM_RRZ_OUT_IMG,RRZ_OUT_HT);
#endif

        //this info is for GIS
#if 0 //can't use rrz . because it will be modified by twin drv under twin mode
        input.RRZ_IN_CROP.in_size_w = CAM_READ_BITS(this->m_pDrv,CAM_RRZ_IN_IMG,RRZ_IN_WD);
        input.RRZ_IN_CROP.in_size_h = CAM_READ_BITS(this->m_pDrv,CAM_RRZ_IN_IMG,RRZ_IN_HT);
#else
        input.RRZ_IN_CROP.in_size_w = (TG_W.Bits.PXL_E - TG_W.Bits.PXL_S) >> (bin + dbin);
        input.RRZ_IN_CROP.in_size_h = (TG_H.Bits.LIN_E - TG_H.Bits.LIN_S) >> bin;
#endif


        input.RRZ_IN_CROP.start_x = CAM_READ_BITS(this->m_pDrv,CAM_RRZ_HORI_INT_OFST,RRZ_HORI_INT_OFST);
        input.RRZ_IN_CROP.start_y = CAM_READ_BITS(this->m_pDrv,CAM_RRZ_VERT_INT_OFST,RRZ_VERT_INT_OFST);
        tmp = fh_rrzo.GetRegInfo(Header_RRZO::E_RRZ_CRP_SIZE,this->m_pDrv);
        input.RRZ_IN_CROP.crop_size_w = tmp & 0xffff;
        input.RRZ_IN_CROP.crop_size_h = (tmp>>16);
        tmp = fh_rrzo.GetRegInfo(Header_RRZO::E_RRZ_DST_SIZE,this->m_pDrv);
        input.sRMXOut.w = tmp & 0xffff;
        input.sRMXOut.h = (tmp>>16);


        input.bYUVFmt = MFALSE;

        CAM_FUNC_DBG("CAM_RAW_PIPE::setEIS:w/h(hbn:%d_%d,tg:%d_%d,rmx:%d_%d,rrz:%d_%d_%d_%d)\n",input.sHBINOut.w,\
                input.sHBINOut.h,input.sTGOut.w,input.sTGOut.h,\
                input.sRMXOut.w,input.sRMXOut.h,\
                input.RRZ_IN_CROP.start_x,input.RRZ_IN_CROP.start_y,input.RRZ_IN_CROP.crop_size_w,input.RRZ_IN_CROP.crop_size_h);

        this->m_pP1Tuning->p1TuningNotify((MVOID*)&input,(MVOID*)&lmv_cfg);

        _eis_cfg.bEIS_Bypass = lmv_cfg.bypassLMV;
        _eis_cfg.bEIS_EN = lmv_cfg.enLMV;
        _eis_cfg._EIS_REG.CTRL_1 = lmv_cfg.cfg_lmv_prep_me_ctrl1;
        _eis_cfg._EIS_REG.CTRL_2 = lmv_cfg.cfg_lmv_prep_me_ctrl2;
        _eis_cfg._EIS_REG.LMV_TH = lmv_cfg.cfg_lmv_lmv_th;
        _eis_cfg._EIS_REG.FL_ofs = lmv_cfg.cfg_lmv_fl_offset;
        _eis_cfg._EIS_REG.MB_ofs = lmv_cfg.cfg_lmv_mb_offset;
        _eis_cfg._EIS_REG.MB_int = lmv_cfg.cfg_lmv_mb_interval;
        _eis_cfg._EIS_REG.GMV = lmv_cfg.cfg_lmv_gmv;
        _eis_cfg._EIS_REG.ERR_CTRL = lmv_cfg.cfg_lmv_err_ctrl;
        _eis_cfg._EIS_REG.IMG_CTRL = lmv_cfg.cfg_lmv_image_ctrl;

        CAM_FUNC_DBG("-\n");

        if(_eis_cfg.bEIS_Bypass == 0){
            if(_eis_cfg.bEIS_EN){
                MUINT32 addrost,modulesize;
                ISP_DRV_REG_IO_STRUCT* pReg;
                MUINT32* ptr = (MUINT32*)&_eis_cfg._EIS_REG.CTRL_1;

                if(UNI_READ_BITS(this->m_pDrv, CAM_UNI_TOP_MOD_EN, SGG2_A_EN) != 1){
                    CAM_FUNC_ERR("need to enable SGG2 before enable EIS\n");
                    return MFALSE;
                }
                this->m_pDrv->getCQModuleInfo(UNI_ISP_EIS_,addrost,modulesize);
                pReg = (ISP_DRV_REG_IO_STRUCT*)malloc(sizeof(ISP_DRV_REG_IO_STRUCT)*modulesize);

                for(MUINT32 i=0;i<modulesize;i++){
                    pReg[i].Data = *(ptr++);
                }
                UNI_BURST_WRITE_REGS(this->m_pDrv,CAM_UNI_EIS_A_PREP_ME_CTRL1,pReg,modulesize);

                UNI_WRITE_BITS(this->m_pDrv, CAM_UNI_TOP_MOD_EN, EIS_A_EN, 1);

                eiso.m_pUniDrv = this->m_pUniDrv;
                eiso.m_pIspDrv = this->m_pIspDrv;


                eiso.config();
                eiso.write2CQ();
                eiso.enable(NULL);

                this->m_pDrv->cqApbModule(UNI_ISP_EIS_);
                free(pReg);
            }
            else{
                CAM_FUNC_ERR("eis must be opened if notification is registered\n");
                ret = MFALSE;
            }

        }
        else{
            this->m_pDrv->cqNopModule(UNI_ISP_EIS_);
        }
    }
    return ret;
}


MBOOL CAM_RAW_PIPE::setLCS(void)
{
    DMA_LCSO lcso;
    MBOOL ret = MTRUE;


    this->m_pDrv = (ISP_DRV_CAM*)this->m_pIspDrv;
    if(this->m_pP1Tuning)
    {
        CAM_REG_TG_SEN_GRAB_PXL TG_W;
        CAM_REG_TG_SEN_GRAB_LIN TG_H;
        MUINT32 pixMode,tmp;
        LCS_REG_CFG     _lcs_cfg;
        LCS_INPUT_INFO  input;
        Header_RRZO fh_rrzo;
        MUINT32 bin = CAM_READ_BITS(this->m_pDrv,CAM_CTL_EN,BIN_EN);
        MUINT32 dbin = CAM_READ_BITS(this->m_pDrv,CAM_CTL_EN,DBN_EN);

        TG_W.Raw = CAM_READ_REG(this->m_pDrv->getPhyObj(),CAM_TG_SEN_GRAB_PXL);
        TG_H.Raw = CAM_READ_REG(this->m_pDrv->getPhyObj(),CAM_TG_SEN_GRAB_LIN);

        //
        pixMode = CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_QBN2_MODE, QBN_ACC);
        input.sHBINOut.w = (TG_W.Bits.PXL_E - TG_W.Bits.PXL_S) >> pixMode;
        input.sHBINOut.h = TG_H.Bits.LIN_E - TG_H.Bits.LIN_S;

        input.sHBINOut.w = input.sHBINOut.w >> (bin + dbin);
        input.sHBINOut.h = input.sHBINOut.h >> bin ;

        input.bIsHbin = MTRUE;
        input.magic = fh_rrzo.GetRegInfo(Header_RRZO::E_Magic,this->m_pDrv);

#if 0   //rmx output will be affected at twin mode
        //
        input.sRRZOut.w = CAM_READ_BITS(this->m_pDrv,CAM_RRZ_OUT_IMG,RRZ_OUT_WD);
        input.sRRZOut.h = CAM_READ_BITS(this->m_pDrv,CAM_RRZ_OUT_IMG,RRZ_OUT_HT);
#endif

        //this info is request from lsc
#if 0 //can't use rrz . because it will be modified by twin drv under twin mode
        input.RRZ_IN_CROP.in_size_w = CAM_READ_BITS(this->m_pDrv,CAM_RRZ_IN_IMG,RRZ_IN_WD);
        input.RRZ_IN_CROP.in_size_h = CAM_READ_BITS(this->m_pDrv,CAM_RRZ_IN_IMG,RRZ_IN_HT);
#else
        input.RRZ_IN_CROP.in_size_w = (TG_W.Bits.PXL_E - TG_W.Bits.PXL_S) >> (bin + dbin);
        input.RRZ_IN_CROP.in_size_h = (TG_H.Bits.LIN_E - TG_H.Bits.LIN_S) >> bin;
#endif
        input.RRZ_IN_CROP.start_x = CAM_READ_BITS(this->m_pDrv,CAM_RRZ_HORI_INT_OFST,RRZ_HORI_INT_OFST);
        input.RRZ_IN_CROP.start_y = CAM_READ_BITS(this->m_pDrv,CAM_RRZ_VERT_INT_OFST,RRZ_VERT_INT_OFST);
        tmp = fh_rrzo.GetRegInfo(Header_RRZO::E_RRZ_CRP_SIZE,this->m_pDrv);
        input.RRZ_IN_CROP.crop_size_w = tmp & 0xffff;
        input.RRZ_IN_CROP.crop_size_h = (tmp>>16);
        tmp = fh_rrzo.GetRegInfo(Header_RRZO::E_RRZ_DST_SIZE,this->m_pDrv);
        input.sRRZOut.w = tmp & 0xffff;
        input.sRRZOut.h = (tmp>>16);


        CAM_FUNC_DBG("+");
        this->m_pP1Tuning->p1TuningNotify((MVOID*)&input,(MVOID*)&_lcs_cfg);
        CAM_FUNC_DBG("-");
        CAM_FUNC_DBG("CAM_RAW_PIPE::setLCS:in:%d_%d, LCS_REG(%d): 0x%x,0x%x,m(%d)\n",input.sRRZOut.w,input.sRRZOut.h,\
                _lcs_cfg.bLCS_Bypass,_lcs_cfg._LCS_REG.LCS_ST,_lcs_cfg._LCS_REG.LCS_AWS,input.magic);
        if(_lcs_cfg.bLCS_Bypass == 0)
        {
            if(_lcs_cfg.bLCS_EN){
                CAM_WRITE_REG(this->m_pDrv, CAM_LCS25_ST, _lcs_cfg._LCS_REG.LCS_ST );
                CAM_WRITE_REG(this->m_pDrv, CAM_LCS25_AWS, _lcs_cfg._LCS_REG.LCS_AWS );
                CAM_WRITE_REG(this->m_pDrv, CAM_LCS25_LRZR_1, _lcs_cfg._LCS_REG.LCS_LRZR_1 );
                CAM_WRITE_REG(this->m_pDrv, CAM_LCS25_LRZR_2, _lcs_cfg._LCS_REG.LCS_LRZR_2 );
                CAM_WRITE_REG(this->m_pDrv, CAM_LCS25_CON, _lcs_cfg._LCS_REG.LCS_CON );
                CAM_WRITE_REG(this->m_pDrv, CAM_LCS25_FLR, _lcs_cfg._LCS_REG.LCS_FLR );
                CAM_WRITE_REG(this->m_pDrv, CAM_LCS25_SATU_1,_lcs_cfg._LCS_REG.LCS_SATU1 );
                CAM_WRITE_REG(this->m_pDrv, CAM_LCS25_SATU_2,_lcs_cfg._LCS_REG.LCS_SATU2 );
                CAM_WRITE_REG(this->m_pDrv, CAM_LCS25_GAIN_1, _lcs_cfg._LCS_REG.LCS_GAIN_1 );
                CAM_WRITE_REG(this->m_pDrv, CAM_LCS25_GAIN_2, _lcs_cfg._LCS_REG.LCS_GAIN_2 );
                CAM_WRITE_REG(this->m_pDrv, CAM_LCS25_OFST_1,_lcs_cfg._LCS_REG.LCS_OFST_1 );
                CAM_WRITE_REG(this->m_pDrv, CAM_LCS25_OFST_2,_lcs_cfg._LCS_REG.LCS_OFST_2 );
                CAM_WRITE_REG(this->m_pDrv, CAM_LCS25_G2G_CNV_1,_lcs_cfg._LCS_REG.LCS_G2G_CNV_1 );
                CAM_WRITE_REG(this->m_pDrv, CAM_LCS25_G2G_CNV_2,_lcs_cfg._LCS_REG.LCS_G2G_CNV_2 );
                CAM_WRITE_REG(this->m_pDrv, CAM_LCS25_G2G_CNV_3,_lcs_cfg._LCS_REG.LCS_G2G_CNV_3 );
                CAM_WRITE_REG(this->m_pDrv, CAM_LCS25_G2G_CNV_4,_lcs_cfg._LCS_REG.LCS_G2G_CNV_4 );
                CAM_WRITE_REG(this->m_pDrv, CAM_LCS25_G2G_CNV_5,_lcs_cfg._LCS_REG.LCS_G2G_CNV_5 );
                CAM_WRITE_REG(this->m_pDrv, CAM_LCS25_LPF,_lcs_cfg._LCS_REG.LCS_LPF );

                CAM_WRITE_BITS(this->m_pDrv, CAM_CTL_EN, LCS_EN, 1);

                lcso.m_pIspDrv = this->m_pIspDrv;
                lcso.dma_cfg.size.xsize = CAM_READ_BITS(this->m_pDrv,CAM_LCS25_CON,LCS25_OUT_WD)*2;
                lcso.dma_cfg.size.h = CAM_READ_BITS(this->m_pDrv,CAM_LCS25_CON,LCS25_OUT_HT);
                lcso.dma_cfg.size.stride = lcso.dma_cfg.size.xsize;
                lcso.config();
                lcso.write2CQ();
                lcso.enable(NULL);

                this->m_pDrv->cqApbModule(CAM_ISP_LCS_);
            }
            else{
                CAM_FUNC_ERR("lcs must be opened if notification is registered\n");
                ret = MFALSE;
            }

        }
        else
            this->m_pDrv->cqNopModule(CAM_ISP_LCS_);

    }
    return ret;

}

MBOOL CAM_RAW_PIPE::setSGG2(void)
{
    MBOOL ret = MTRUE;
    this->m_pDrv = (ISP_DRV_CAM*)this->m_pIspDrv;
    if(this->m_pP1Tuning){
        SGG2_REG_CFG _sgg2_cfg;
        LMV_SGG_CFG lmv_sgg_cfg;


        CAM_FUNC_DBG("CAM_RAW_PIPE::setSGG2:\n");
        this->m_pP1Tuning->p1TuningNotify(NULL,(MVOID*)&lmv_sgg_cfg);

        _sgg2_cfg.bSGG2_Bypass = lmv_sgg_cfg.bSGG2_Bypass;
        _sgg2_cfg.bSGG2_EN = lmv_sgg_cfg.bSGG2_EN;
        _sgg2_cfg._SGG2_REG.PGN = lmv_sgg_cfg.PGN;
        _sgg2_cfg._SGG2_REG.GMRC_1 = lmv_sgg_cfg.GMRC_1;
        _sgg2_cfg._SGG2_REG.GMRC_2 = lmv_sgg_cfg.GMRC_2;

        CAM_FUNC_DBG("-\n");

        if(_sgg2_cfg.bSGG2_Bypass == 0){
            UNI_WRITE_REG(this->m_pDrv,CAM_UNI_SGG2_A_PGN,_sgg2_cfg._SGG2_REG.PGN);
            UNI_WRITE_REG(this->m_pDrv,CAM_UNI_SGG2_A_GMRC_1,_sgg2_cfg._SGG2_REG.GMRC_1);
            UNI_WRITE_REG(this->m_pDrv,CAM_UNI_SGG2_A_GMRC_2,_sgg2_cfg._SGG2_REG.GMRC_2);

            this->m_pDrv->cqApbModule(UNI_ISP_SGG2_A_);

        }
        else{
            this->m_pDrv->cqNopModule(UNI_ISP_SGG2_A_);


        }

        if(_sgg2_cfg.bSGG2_EN){
            UNI_WRITE_BITS(this->m_pDrv, CAM_UNI_TOP_MOD_EN, SGG2_A_EN, 1);
        }
        else{
            UNI_WRITE_BITS(this->m_pDrv, CAM_UNI_TOP_MOD_EN, SGG2_A_EN, 0);
        }

    }
    return ret;
}

MUINT32 CAM_RAW_PIPE::getCurPixMode(_isp_dma_enum_ dmao)
{
    this->m_pDrv = (ISP_DRV_CAM*)this->m_pIspDrv;
    switch(dmao){
        case _imgo_:
            switch(this->m_RawType){
                case E_FromTG:
                    if(CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_TG_SEN_MODE,DBL_DATA_BUS1) == 1)
                        return _4_pix_;
                    else if(CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_TG_SEN_MODE,DBL_DATA_BUS) == 1)
                        return _2_pix_;
                    else
                        return _1_pix_;
                    break;
                case E_BeforLSC:
                    return _1_pix_;
                    break;
                case E_AFTERLSC:
                    switch(CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_CTL_FMT_SEL,PIX_BUS_BMXO)){
                        case 0:
                            return _1_pix_;
                            break;
                        case 1:
                            return _2_pix_;
                            break;
                        case 2:
                            return _4_pix_;
                            break;
                        default:
                            CAM_FUNC_ERR("BMXO setting error\n");
                            return MFALSE;
                            break;
                    }
                    break;
                default:
                    CAM_FUNC_ERR("Raw Data support only 0:from TG,1:before LSC,2:after LSC\n");
                    return MFALSE;
                    break;
            }
            break;
        case _rrzo_:
            switch(CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_CTL_FMT_SEL,PIX_BUS_RMXO)){
                case 0:
                    return _1_pix_;
                    break;
                case 1:
                    return _2_pix_;
                    break;
                case 2:
                    return _4_pix_;
                    break;
                default:
                    CAM_FUNC_ERR("RMXO setting error\n");
                    return MFALSE;
                    break;
            }
            break;
        default:
            CAM_FUNC_ERR("unsupported dmao:0x%x,return unknown-pix\n",dmao);
            return _unkonw_pix;
            break;
    }
}

///////////////////////////////////////////////////////////////////////////////
MINT32 CAM_TWIN_PIPE::_config( void)
{
    MINT32 ret = 0;

    CAM_FUNC_INF("_config +, m_twinHwModule(%d)\n", this->m_twinHwModule);

    // 1. Set TWIN_EN of m_pMainIspDrv to be 1
    CAM_WRITE_BITS(this->m_pMainIspDrv->getPhyObj(), CAM_CTL_TWIN_STATUS, TWIN_EN, 0x1); // 0x1A004050
    CAM_WRITE_BITS(this->m_pMainIspDrv->getPhyObj(), CAM_CTL_TWIN_STATUS, TWIN_MODULE, this->m_twinHwModule);

    CAM_WRITE_BITS(this->m_pTwinIspDrv->getPhyObj(), CAM_CTL_TWIN_STATUS, TWIN_EN, 0x1); // 0x1A005050
    CAM_WRITE_BITS(this->m_pTwinIspDrv->getPhyObj(), CAM_CTL_TWIN_STATUS, TWIN_MODULE, this->m_twinHwModule);

    CAM_WRITE_BITS(this->m_pTwinIspDrv->getPhyObj(),CAM_CTL_FMT_SEL,DMX_ID,0x0);
    CAM_WRITE_BITS(this->m_pTwinIspDrv,CAM_CTL_FMT_SEL,DMX_ID,0x0);
#if 1
    //reset
    CAM_FUNC_DBG("Twin TOP reset\n");
    CAM_WRITE_REG(this->m_pTwinIspDrv->getPhyObj(),CAM_CTL_SW_CTL,0x1);
    while(CAM_READ_REG(this->m_pTwinIspDrv->getPhyObj(),CAM_CTL_SW_CTL) != 0x2){
        CAM_FUNC_DBG("Twin TOP reseting...\n");
    }
    CAM_WRITE_REG(this->m_pTwinIspDrv->getPhyObj(),CAM_CTL_SW_CTL,0x4);
    CAM_WRITE_REG(this->m_pTwinIspDrv->getPhyObj(),CAM_CTL_SW_CTL,0x0);
#endif

    CAM_WRITE_REG(this->m_pTwinIspDrv->getPhyObj(),CAM_CTL_RAW_INT_EN,\
            CQ_CODE_ERR_EN_|\
            CQ_APB_ERR_EN_ |\
            CQ_VS_ERR_EN_);


    CAM_WRITE_REG(this->m_pTwinIspDrv->getPhyObj(),CAM_CTL_RAW_INT2_EN,\
            AFO_DONE_EN_);

    return ret;
}

MINT32 CAM_TWIN_PIPE::_enable( void* pParam  )
{
    MINT32 ret = 0;
    (void)pParam;

    CAM_FUNC_DBG("_enable +, m_twinHwModule(%d), subsample(%d)\n", this->m_twinHwModule, this->m_subsample);

    if(this->m_pMainIspDrv == NULL) {
        CAM_FUNC_ERR("m_pMainDrv is null!!\n");
        return 1;
    }

    if(this->m_pTwinIspDrv == NULL) {
        CAM_FUNC_ERR("m_pTwinDrv is null!!\n");
        return 1;
    }



    // 2. CQ related setting
    //cq db setting
    CAM_WRITE_BITS(this->m_pTwinIspDrv->getPhyObj(),CAM_CQ_EN,CQ_DB_EN,1); //enable double buffer

    //subsample p1 done
    if(this->m_subsample){
        CAM_WRITE_BITS(this->m_pTwinIspDrv->getPhyObj(),CAM_CTL_SW_PASS1_DONE,DOWN_SAMPLE_EN,1);
    }
    else{
        CAM_WRITE_BITS(this->m_pTwinIspDrv->getPhyObj(),CAM_CTL_SW_PASS1_DONE,DOWN_SAMPLE_EN,0);
    }

    CAM_WRITE_BITS(this->m_pTwinIspDrv->getPhyObj(),CAM_CTL_SW_PASS1_DONE,DOWN_SAMPLE_PERIOD,this->m_subsample);

    //
    //change CQ load mode before streaming start.
    //because this bit is shared to all cq, so put here to make sure all cq timing
    if(CAM_READ_BITS(this->m_pTwinIspDrv->getPhyObj(),CAM_CTL_SW_PASS1_DONE,DOWN_SAMPLE_EN) == 1){
        //load mode = 0 if the latch-timing of the cq-baseaddress-reg is by sw p1_done
        CAM_WRITE_BITS(this->m_pTwinIspDrv->getPhyObj(),CAM_CQ_EN,CQ_DB_LOAD_MODE,0);
    }
    else{
        //load mode = 1 if the latch-timing of the cq-baseaddress-reg is by hw p1_done
        //when mode = 0 , inner reg is useless
        CAM_WRITE_BITS(this->m_pTwinIspDrv->getPhyObj(),CAM_CQ_EN,CQ_DB_LOAD_MODE,1);
    }

    // 3. special
    CAM_WRITE_REG(this->m_pTwinIspDrv->getPhyObj(),CAM_DMA_RSV1,(CAM_READ_REG(this->m_pTwinIspDrv->getPhyObj(),CAM_DMA_RSV1)&0x7fffffff));//bit31 for smi mask, blocking imgo output
    CAM_WRITE_REG(this->m_pTwinIspDrv->getPhyObj(),CAM_DMA_RSV6,0xffffffff);
    //dma performance
    /**
     * Notice!!!!!!!!!!!!!!!!! CAM_SPECIAL_FUN_EN must set or you'll encounter the following situation. But still don't know why!! (Ask SC Huang, 2015/08/12)
     * This register must set enable CQ_ULTRA_BPCI_EN and CQ_ULTRA_LSCI_EN, or
     * You'll encounter: (1) CAM's CQ cannot load into physical addr even it's in immediately trigger!!!!
     *                   (2) CAMB's CQ need to trigger by CAMA' CQ start
     */

#if 0
    // No need this. verified OK in twin in this platform is OK, 2016/03/16 JSS
    CAM_WRITE_BITS(this->m_pTwinIspDrv->getPhyObj(),CAM_SPECIAL_FUN_EN,CQ_ULTRA_BPCI_EN,1);
    CAM_WRITE_BITS(this->m_pTwinIspDrv->getPhyObj(),CAM_SPECIAL_FUN_EN,CQ_ULTRA_LSCI_EN,1);
#endif

    return 0;
}


MINT32 CAM_TWIN_PIPE::_disable(void* pParam)
{
    MINT32 ret = 0;
    (void)pParam;

    // Set TWIN_EN of m_pMainIspDrv to be 0
    CAM_WRITE_REG(this->m_pMainIspDrv->getPhyObj(), CAM_CTL_TWIN_STATUS,  0x0); // 0x1A004050
    CAM_WRITE_REG(this->m_pTwinIspDrv->getPhyObj(), CAM_CTL_TWIN_STATUS,  0x0); // 0x1A005050

    // !!Must set CAMB's SGG_SEL = 0, or run other case CAMA will enconter error
    CAM_WRITE_BITS(this->m_pTwinIspDrv->getPhyObj(), CAM_CTL_SEL, SGG_SEL, 0x0);

    // !!Must set CAMB's dmx_sel = 0, or run other case CAMA will enconter error
    CAM_WRITE_BITS(this->m_pTwinIspDrv->getPhyObj(), CAM_CTL_SEL, DMX_SEL, 0x0);

    CAM_WRITE_BITS(this->m_pTwinIspDrv->getPhyObj(),CAM_CTL_MISC,DB_EN,0); //disable double buffer
    //signal ctrl
    CAM_WRITE_REG(this->m_pTwinIspDrv->getPhyObj(),CAM_CTL_SW_PASS1_DONE,0);
    CAM_WRITE_REG(this->m_pTwinIspDrv->getPhyObj(),CAM_CTL_AB_DONE_SEL,0);
    CAM_WRITE_REG(this->m_pTwinIspDrv->getPhyObj(),CAM_CTL_UNI_DONE_SEL,0);
    CAM_WRITE_REG(this->m_pTwinIspDrv->getPhyObj(),CAM_CTL_DMA_EN,0x0);


    CAM_WRITE_REG(this->m_pTwinIspDrv->getPhyObj(),CAM_CTL_EN,0x0);
    //pdc function enable is within BNR
    CAM_WRITE_REG(this->m_pTwinIspDrv->getPhyObj(),CAM_BNR_BPC_CON,0x0);
    CAM_WRITE_REG(this->m_pTwinIspDrv->getPhyObj(),CAM_BNR_PDC_CON,0x0);

    //close  fbc
    //FBC on uni will be closed at the _disable() of uni_top
    //FBC of STT pipe will be closed at STT pipe
    CAM_WRITE_REG(this->m_pTwinIspDrv->getPhyObj(),CAM_FBC_IMGO_CTL1,0x0);
    CAM_WRITE_REG(this->m_pTwinIspDrv->getPhyObj(),CAM_FBC_RRZO_CTL1,0x0);
    CAM_WRITE_REG(this->m_pTwinIspDrv->getPhyObj(),CAM_FBC_LCSO_CTL1,0x0);


    CAM_WRITE_BITS(this->m_pTwinIspDrv->getPhyObj(),CAM_CTL_MISC,DB_EN,1); //disable double buffer

    //reset
    CAM_FUNC_DBG("TOP reset\n");
    CAM_WRITE_REG(this->m_pTwinIspDrv->getPhyObj(),CAM_CTL_SW_CTL,0x2);
    CAM_WRITE_REG(this->m_pTwinIspDrv->getPhyObj(),CAM_CTL_SW_CTL,0x1);
#if 0
    while(CAM_READ_REG(this->m_pTwinIspDrv->getPhyObj(),CAM_CTL_SW_CTL) != 0x2){
        CAM_FUNC_DBG("TOP reseting...\n");
    }
#else
    usleep(10);
#endif
    CAM_WRITE_REG(this->m_pTwinIspDrv->getPhyObj(),CAM_CTL_SW_CTL,0x4);
    CAM_WRITE_REG(this->m_pTwinIspDrv->getPhyObj(),CAM_CTL_SW_CTL,0x0);

#if PDO_SW_WORK_AROUND
    {
        Header_PDO fh_pdo;

        //clear cam_b's record when stop twin mode to avoid confused header output at single mode
        //note : cam_b's record will be kept on cam_a's spare
        CAM_WRITE_REG(this->m_pMainIspDrv->getPhyObj(),CAM_PDO_FH_SPARE_2,0);
        CAM_WRITE_REG(this->m_pMainIspDrv->getPhyObj(),CAM_PDO_FH_SPARE_3,0);
        CAM_WRITE_REG(this->m_pMainIspDrv->getPhyObj(),CAM_PDO_FH_SPARE_4,0);
        CAM_WRITE_REG(this->m_pMainIspDrv->getPhyObj(),CAM_PDO_FH_SPARE_5,0);
        CAM_WRITE_REG(this->m_pMainIspDrv->getPhyObj(),CAM_PDO_FH_SPARE_6,0);
        CAM_WRITE_REG(this->m_pMainIspDrv->getPhyObj(),CAM_PDO_FH_SPARE_7,0);
        CAM_WRITE_REG(this->m_pMainIspDrv->getPhyObj(),CAM_PDO_FH_SPARE_8,0);
        CAM_WRITE_REG(this->m_pMainIspDrv->getPhyObj(),CAM_PDO_FH_SPARE_9,0);
        CAM_WRITE_REG(this->m_pMainIspDrv->getPhyObj(),CAM_PDO_FH_SPARE_10,0);
        CAM_WRITE_REG(this->m_pMainIspDrv->getPhyObj(),CAM_PDO_FH_SPARE_11,0);
        CAM_WRITE_REG(this->m_pMainIspDrv->getPhyObj(),CAM_PDO_FH_SPARE_12,0);
        CAM_WRITE_REG(this->m_pMainIspDrv->getPhyObj(),CAM_PDO_FH_SPARE_13,0);
        CAM_WRITE_REG(this->m_pMainIspDrv->getPhyObj(),CAM_PDO_FH_SPARE_14,0);
        CAM_WRITE_REG(this->m_pMainIspDrv->getPhyObj(),CAM_PDO_FH_SPARE_15,0);
    }

#endif

    return 0;
}


MINT32 CAM_TWIN_PIPE::update_beforeRunTwin()
{
    MINT32 ret = 0;
    MUINT32  stride;
    Header_RRZO fh_rrzo;
    MUINT32 tmp;
    CAM_REG_TG_SEN_GRAB_PXL TG_W;
    CAM_REG_TG_SEN_GRAB_LIN TG_H;

    // Check if RRZ_EN =1 and RRZ_VERT_STEP == 0. return err.
    // Cause TWIN_RRZ_IN_CROP_HT is set to 0 here, but Dual_cal will return err when TWIN_RRZ_IN_CROP_HT and RRZ_VERT_STEP both = 0
    if(1 == CAM_READ_BITS(this->m_pMainIspDrv, CAM_CTL_EN, RRZ_EN) && 0 == CAM_READ_BITS(this->m_pMainIspDrv, CAM_RRZ_VERT_STEP, RRZ_VERT_STEP)) {
        CAM_FUNC_ERR("RRZ_EN(%d) RRZ_VERT_STEP(%d) is 0!!!! Cannot run twin!!!", \
                CAM_READ_BITS(this->m_pMainIspDrv, CAM_CTL_EN, RRZ_EN), \
                CAM_READ_BITS(this->m_pMainIspDrv, CAM_RRZ_VERT_STEP, RRZ_VERT_STEP));
        return 1;
    }

    //TG cropping window
    TG_H.Raw = CAM_READ_REG(this->m_pMainIspDrv->getPhyObj(),CAM_TG_SEN_GRAB_LIN);
    TG_W.Raw = CAM_READ_REG(this->m_pMainIspDrv->getPhyObj(),CAM_TG_SEN_GRAB_PXL);

    CAM_WRITE_REG(this->m_pMainIspDrv,CAM_TG_SEN_GRAB_PXL,TG_W.Raw);
    CAM_WRITE_REG(this->m_pMainIspDrv,CAM_TG_SEN_GRAB_LIN,TG_H.Raw);

    // AF related setting that need to set before run Twin_drv
    if(0x1 == CAM_READ_BITS(this->m_pMainIspDrv,CAM_CTL_EN,AF_EN))
    {
        MUINT32 sgg_sel = CAM_READ_BITS(this->m_pMainIspDrv, CAM_CTL_SEL, SGG_SEL);
        // 1. SGG SEL = 1
        CAM_WRITE_BITS(this->m_pTwinIspDrv, CAM_CTL_SEL, SGG_SEL, sgg_sel);
        //
    }

    this->m_rrz_out_wd = CAM_READ_BITS(this->m_pMainIspDrv, CAM_RRZ_OUT_IMG, RRZ_OUT_WD);

    //rrz roi
    this->m_rrz_roi.floatX = this->m_rrz_roi.floatY = 0;
    tmp = fh_rrzo.GetRegInfo(Header_RRZO::E_RRZ_CRP_START,this->m_pMainIspDrv);
    this->m_rrz_roi.x = (tmp & 0xffff);
    this->m_rrz_roi.y = (tmp>>16);

    tmp = fh_rrzo.GetRegInfo(Header_RRZO::E_RRZ_CRP_SIZE,this->m_pMainIspDrv);
    this->m_rrz_roi.w = (tmp & 0xffff);
    this->m_rrz_roi.h = (tmp>>16);

    //rrz_in
    this->m_rrz_in.h = TG_H.Bits.LIN_E - TG_H.Bits.LIN_S;
    this->m_rrz_in.w = TG_W.Bits.PXL_E - TG_W.Bits.PXL_S;


    this->m_af_vld_xstart = CAM_READ_BITS(this->m_pMainIspDrv, CAM_AF_VLD, AF_VLD_XSTART);
    this->m_af_blk_xsize = CAM_READ_BITS(this->m_pMainIspDrv, CAM_AF_BLK_0, AF_BLK_XSIZE);
    this->m_af_x_win_num = CAM_READ_BITS(this->m_pMainIspDrv, CAM_AF_BLK_1, AF_BLK_XNUM);

    this->m_lsc_lwidth = CAM_READ_BITS(this->m_pMainIspDrv, CAM_LSC_LBLOCK,LSC_SDBLK_lWIDTH);
    this->m_lsc_win_num_x = CAM_READ_BITS(this->m_pMainIspDrv, CAM_LSC_CTL2,LSC_SDBLK_XNUM);


    //
    // 2. AFO stride, set to same as CAMA
    stride = CAM_READ_BITS(this->m_pMainIspDrv,CAM_AFO_STRIDE,STRIDE);
    CAM_WRITE_BITS(this->m_pTwinIspDrv, CAM_AFO_STRIDE, STRIDE, stride);

#if PDO_SW_WORK_AROUND
    stride = CAM_READ_BITS(this->m_pMainIspDrv,CAM_PDO_STRIDE,STRIDE);
    CAM_WRITE_BITS(this->m_pTwinIspDrv, CAM_PDO_STRIDE, STRIDE, stride);
#endif

    return 0;
}


MINT32 CAM_TWIN_PIPE::update_afterRunTwin()
{
    MINT32 ret = 0;
    //MUINT32  afo_stride, afo_ofst, sgg;
    MUINT32 af[2];
    MUINT32 afo[2];
    static MUINT32 sycn_counter = 0;

    //inte en => marked
    // Don'y enable CAMB's interrupt cause it will get P1_done iRQ as well
    //CAM_WRITE_REG(this->m_pTwinIspDrv->getPhyObj(),CAM_CTL_RAW_INT_EN,CAM_READ_REG(this->m_pMainIspDrv->getPhyObj(), CAM_CTL_RAW_INT_EN));

    // Set CAMB's dmx_sel = 1
    CAM_WRITE_BITS(this->m_pTwinIspDrv, CAM_CTL_SEL, DMX_SEL, 0x1);

    /// AFO related setting, these registers need to be set where twin_drv not take care
    if(0x1 == CAM_READ_BITS(this->m_pMainIspDrv,CAM_CTL_EN,AF_EN))
    {
        // 2. AFO stride, set to same as CAMA
        //afo_stride = CAM_READ_BITS(this->m_pMainIspDrv,CAM_AFO_STRIDE,STRIDE);
        //CAM_WRITE_BITS(this->m_pTwinIspDrv, CAM_AFO_STRIDE, STRIDE, afo_stride);

#if 0   //twin drv should write offset to virreg directly
        // 3. AFO offset = AF_BLK_XSIZE x AF_BLK_XNUM
        afo_ofst = CAM_READ_BITS(this->m_pMainIspDrv,CAM_AF_BLK_1,AF_BLK_XNUM) * 16; // one af window is 16 bytes
        CAM_WRITE_BITS(this->m_pTwinIspDrv, CAM_AFO_OFST_ADDR, OFFSET_ADDR, afo_ofst);


        // 4. SGG SEL = 1
        /// MOVE before run twin_drv
        //CAM_WRITE_BITS(this->m_pTwinIspDrv, CAM_CTL_SEL, SGG_SEL, 0x1);

        // SGG_EN
        CAM_WRITE_BITS(this->m_pTwinIspDrv, CAM_CTL_EN, SGG1_EN, 0x1);

        // SGG_GAIN copy to same as CAMA
        sgg = CAM_READ_REG(this->m_pMainIspDrv,CAM_SGG1_PGN);
        CAM_WRITE_REG(this->m_pTwinIspDrv,CAM_SGG1_PGN, sgg);

        sgg = CAM_READ_REG(this->m_pMainIspDrv,CAM_SGG1_GMRC_1);
        CAM_WRITE_REG(this->m_pTwinIspDrv,CAM_SGG1_GMRC_1, sgg);

        sgg = CAM_READ_REG(this->m_pMainIspDrv,CAM_SGG1_GMRC_2);
        CAM_WRITE_REG(this->m_pTwinIspDrv,CAM_SGG1_GMRC_2, sgg);

        CAM_FUNC_DBG("afo_stride(0x%x), afo_ofst(0x%x)", afo_stride, afo_ofst);
#endif
    }

    if(0x1 == CAM_READ_BITS(this->m_pMainIspDrv,CAM_CTL_EN,LSC_EN)){
        MUINT32 addr;
        //
        addr = CAM_READ_REG(this->m_pMainIspDrv,CAM_LSCI_BASE_ADDR);
        CAM_WRITE_REG(this->m_pTwinIspDrv,CAM_LSCI_BASE_ADDR,addr);
        //
        CAM_WRITE_BITS(this->m_pTwinIspDrv,CAM_CTL_DMA_EN,LSCI_EN,1);
    }
    else{
        CAM_WRITE_BITS(this->m_pTwinIspDrv,CAM_CTL_DMA_EN,LSCI_EN,0);
    }

    if(0x1 == CAM_READ_BITS(this->m_pMainIspDrv,CAM_CTL_DMA_EN,BPCI_EN)){
        MUINT32 addr;
        //
        addr = CAM_READ_REG(this->m_pMainIspDrv,CAM_BPCI_BASE_ADDR);
        CAM_WRITE_REG(this->m_pTwinIspDrv,CAM_BPCI_BASE_ADDR,addr);
        //
        CAM_WRITE_BITS(this->m_pTwinIspDrv,CAM_CTL_DMA_EN,BPCI_EN,1);
    }
    else{
        CAM_WRITE_BITS(this->m_pTwinIspDrv,CAM_CTL_DMA_EN,BPCI_EN,0);
    }

#if PDO_SW_WORK_AROUND

    if(0x1 == CAM_READ_BITS(this->m_pMainIspDrv,CAM_CTL_DMA_EN,PDO_EN)){
        MUINT32 addr;
        //
        addr = CAM_READ_REG(this->m_pMainIspDrv,CAM_PDO_BASE_ADDR);
        CAM_WRITE_REG(this->m_pTwinIspDrv,CAM_PDO_BASE_ADDR,addr);
        //
        CAM_WRITE_BITS(this->m_pTwinIspDrv,CAM_CTL_DMA_EN,PDO_EN,1);

        //patch twin drv bug
        addr = CAM_READ_REG(this->m_pMainIspDrv,CAM_PDO_YSIZE);
        CAM_WRITE_REG(this->m_pTwinIspDrv,CAM_PDO_YSIZE,addr);
    }
    else{
        CAM_WRITE_BITS(this->m_pTwinIspDrv,CAM_CTL_DMA_EN,PDO_EN,0);
    }


#endif

    //check af & afo, af & afo must be opened at both cam at the same time is needed.
    af[0] = CAM_READ_BITS(this->m_pMainIspDrv,CAM_CTL_EN,AF_EN);
    af[1] = CAM_READ_BITS(this->m_pTwinIspDrv,CAM_CTL_EN,AF_EN);
    afo[0] = CAM_READ_BITS(this->m_pMainIspDrv,CAM_CTL_DMA_EN,AFO_EN);
    afo[1] = CAM_READ_BITS(this->m_pTwinIspDrv,CAM_CTL_DMA_EN,AFO_EN);
#if 1
    {
        ISP_DRV_REG_IO_STRUCT Regs_R[E_HEADER_MAX -1];

        CAM_BURST_READ_REGS(this->m_pMainIspDrv,CAM_AFO_FH_SPARE_2,Regs_R,E_HEADER_MAX -1);
        CAM_BURST_WRITE_REGS(this->m_pTwinIspDrv,CAM_AFO_FH_SPARE_2,Regs_R,E_HEADER_MAX -1);
    }
#endif
    sycn_counter++;
    CAM_WRITE_REG(this->m_pMainIspDrv,CAM_CTL_SPARE1,sycn_counter);
    CAM_WRITE_REG(this->m_pTwinIspDrv,CAM_CTL_SPARE1,sycn_counter);
    this->m_pMainIspDrv->cqAddModule(CAM_TWIN_COUNTER);
    this->m_pTwinIspDrv->cqAddModule(CAM_TWIN_COUNTER);
    if(af[0] != af[1]){
        ret = 1;
        CAM_FUNC_ERR("af_en must be the same under twin mode[0x%x_0x%x]\n",af[0],af[1]);
    }
    if(afo[0] != afo[1]){
        ret = 1;
        CAM_FUNC_ERR("afo_en must be the same under twin mode[0x%x_0x%x]\n",afo[0],afo[1]);
    }

#if TOP_DCM_SW_WORK_AROUND
    CAM_WRITE_REG(this->m_pMainIspDrv->getPhyObj(),CAM_CTL_TOP_DCM_DIS,0x1);
    CAM_WRITE_REG(this->m_pTwinIspDrv->getPhyObj(),CAM_CTL_TOP_DCM_DIS,0x1);
#endif

#if 1//patch twin drv bug , twin drv at some condition will not copy cam_a's af
    {
        MUINT32 afo_ysize = CAM_READ_REG(this->m_pMainIspDrv,CAM_AFO_YSIZE);
        CAM_WRITE_REG(this->m_pTwinIspDrv,CAM_AFO_YSIZE,afo_ysize);
    }
#endif



#if PDO_SW_WORK_AROUND
    Header_PDO fh_pdo;

    fh_pdo.Header_Enque(Header_PDO::E_PMX_A_CROP,this->m_pMainIspDrv,\
            CAM_READ_REG(this->m_pMainIspDrv,CAM_PMX_CROP));
    fh_pdo.Header_Enque(Header_PDO::E_PMX_B_CROP,this->m_pMainIspDrv,\
            CAM_READ_REG(this->m_pTwinIspDrv,CAM_PMX_CROP));

    fh_pdo.Header_Enque(Header_PDO::E_BMX_A_CROP,this->m_pMainIspDrv,\
            CAM_READ_REG(this->m_pMainIspDrv,CAM_BMX_CROP));
    fh_pdo.Header_Enque(Header_PDO::E_BMX_B_CROP,this->m_pMainIspDrv,\
            CAM_READ_REG(this->m_pTwinIspDrv,CAM_BMX_CROP));


    fh_pdo.Header_Enque(Header_PDO::E_IMG_PA_OFSET,this->m_pMainIspDrv,\
            CAM_READ_REG(this->m_pTwinIspDrv,CAM_PDO_OFST_ADDR));
#endif
    return ret;
}


MINT32 CAM_TUNING_CTRL::_config(void)
{
    MINT32 ret = 0;

    ETuningDrvUser TuningDrvUser = eTuningDrvUser_3A;


    this->m_pTuningDrv = (void*)TuningDrv::getInstance(this->m_SenDev);
    if(MFALSE == ((TuningDrv*)this->m_pTuningDrv)->init("CAM_TUNING_CTRL",TuningDrvUser,1)){
        CAM_FUNC_ERR("tuning drv init fail\n");
        return 1;
    }

    this->m_BQNum = ((TuningDrv*)this->m_pTuningDrv)->getBQNum();

    return 0;
}

MINT32 CAM_TUNING_CTRL::_enable( void* pParam )
{
    (void)pParam;
    //by pass, do nothing
    return 0;
}

MINT32 CAM_TUNING_CTRL::_disable( void* pParam )
{
    MINT32 ret = 0;
    ETuningDrvUser TuningDrvUser = eTuningDrvUser_3A;
    (void)pParam;

    if(MFALSE == ((TuningDrv*)this->m_pTuningDrv)->uninit("CAM_TUNING_CTRL",TuningDrvUser)){
        CAM_FUNC_ERR("tuning drv uninit fail\n");
        ret = 1;
    }
    return ret;
}


MINT32 CAM_TUNING_CTRL::update(void)
{
    MINT32 ret = 0;
    E_ISP_CAM_CQ cq;
    MUINT32 page;

    ETuningDrvUser TuningDrvUser = eTuningDrvUser_3A;
    vector <stTuningNode*> vpTuningNodes;

    MUINT32 module_adr,module_size;
    ISP_DRV_REG_IO_STRUCT* pReg_st;
    MUINT32 subSample;
    cam_reg_t* pReg;
    MBOOL _pdo_en = MFALSE;
    MBOOL _bpci_en = MFALSE;

    DMA_BPCI bpci;
    DMA_PDO pdo;
    bpci.m_pIspDrv = this->m_pIspDrv;
    pdo.m_pIspDrv = this->m_pIspDrv;
    PIPE_CHECK pipe_chk;

    DMA_LSCI lsci;
    lsci.m_pIspDrv = this->m_pIspDrv;

    this->m_pDrv = (ISP_DRV_CAM*)this->m_pIspDrv;

    //dbg log
    this->m_pDrv->getCurObjInfo(&this->m_hwModule,&cq,&page);

    CAM_FUNC_DBG("CAM_TUNING_CTRL::update+ cq:0x%x,page:0x%x,SenId:0x%x,Mag:0x%x\n",cq,page,\
            this->m_SenDev,*this->m_Magic);

    subSample = CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_CTL_SW_PASS1_DONE,DOWN_SAMPLE_PERIOD) + 1;

    if(((TuningDrv*)this->m_pTuningDrv)->getIsApplyTuning(TuningDrvUser) == MFALSE){
        CAM_FUNC_INF("tuning_drv is empty, bypass\n");
        goto EXIT;
    }
    vpTuningNodes.clear();
    if(MFALSE == ((TuningDrv*)this->m_pTuningDrv)->deTuningQueByDrv(TuningDrvUser, (MINT32*)this->m_Magic, vpTuningNodes)){
        CAM_FUNC_ERR("de tuning que fail\n");
        ret = 1;
        goto EXIT;
    }

    if(subSample != ((TuningDrv*)this->m_pTuningDrv)->getBQNum()){
        CAM_FUNC_ERR("tuning que size mismatch:%d_%d\n",subSample,((TuningDrv*)this->m_pTuningDrv)->getBQNum());
        ret = 1;
        goto EXIT;
    }
    //tuningnode at non-subsample case, always using node_0
    if(subSample == 1)
        page = 0;
    else
        page = page % subSample;

    pReg = (cam_reg_t*)vpTuningNodes[page]->pTuningRegBuf;

    //DBS
    // module en: don't need to depend to eUpdateFuncBit, directly use it from each tuning node
    if(vpTuningNodes[page]->ctlEn_CAM & DBS_EN_){
        CAM_WRITE_BITS(this->m_pDrv,CAM_CTL_EN,DBS_EN,1);
    } else{
        CAM_WRITE_BITS(this->m_pDrv,CAM_CTL_EN,DBS_EN,0);
    }

    // module tuning value
    if(vpTuningNodes[page]->eUpdateFuncBit & eIspTuningMgrFunc_DBS){

        this->m_pDrv->cqApbModule(CAM_ISP_DBS_);

        //
        this->m_pDrv->getCQModuleInfo(CAM_ISP_DBS_,module_adr,module_size);
        //
        pReg_st = (ISP_DRV_REG_IO_STRUCT*)malloc(sizeof(ISP_DRV_REG_IO_STRUCT)*module_size);
        //
        for(MUINT32 j=0;j<module_size;j++)
            pReg_st[j].Data = vpTuningNodes[page]->pTuningRegBuf[(module_adr + j*0x4)>>2];
        //
        CAM_BURST_WRITE_REGS(this->m_pDrv,CAM_DBS_SIGMA,pReg_st,module_size);

        free(pReg_st);

        CAM_FUNC_DBG("DBS:[0x%x_0x%x_0x%x],[0x%x_0x%x_0x%x] \n",\
                vpTuningNodes[page]->pTuningRegBuf[(module_adr )>>2],\
                vpTuningNodes[page]->pTuningRegBuf[(module_adr + 1*0x4 )>>2],\
                vpTuningNodes[page]->pTuningRegBuf[(module_adr + 2*0x4 )>>2],\
                this->m_pDrv->readReg((module_adr ),CAM_A),\
                this->m_pDrv->readReg((module_adr + 1*0x4 ),CAM_A),\
                this->m_pDrv->readReg((module_adr + 2*0x4 ),CAM_A));
    }
    else{
        this->m_pDrv->cqNopModule(CAM_ISP_DBS_);
    }

    //SL2F
    if(vpTuningNodes[page]->ctlEnDMA_CAM & SL2F_EN_){
        CAM_WRITE_BITS(this->m_pDrv, CAM_CTL_DMA_EN, SL2F_EN, 1);
    }else{
        CAM_WRITE_BITS(this->m_pDrv, CAM_CTL_DMA_EN, SL2F_EN, 0);
    }

    if(vpTuningNodes[page]->eUpdateFuncBit & eIspTuningMgrFunc_SL2F){

        this->m_pDrv->cqApbModule(CAM_ISP_SL2F_);

        /*Get SL2F Module Info*/
        this->m_pDrv->getCQModuleInfo(CAM_ISP_SL2F_,module_adr,module_size);
        /*Allocat a register space*/
        pReg_st = (ISP_DRV_REG_IO_STRUCT*)malloc(sizeof(ISP_DRV_REG_IO_STRUCT)*module_size);
        /*Set all the registers to the register space*/
        for(MUINT32 j=0;j<module_size;j++)
            pReg_st[j].Data = vpTuningNodes[page]->pTuningRegBuf[(module_adr + j*0x4)>>2];
        /*final update to the CQ virtual register space*/
        CAM_BURST_WRITE_REGS(this->m_pDrv,CAM_SL2F_CEN,pReg_st,module_size);

        free(pReg_st);

        CAM_FUNC_DBG("SL2F:[0x%x_0x%x_0x%x],[0x%x_0x%x_0x%x] \n",\
                vpTuningNodes[page]->pTuningRegBuf[(module_adr )>>2],\
                vpTuningNodes[page]->pTuningRegBuf[(module_adr + 1*0x4 )>>2],\
                vpTuningNodes[page]->pTuningRegBuf[(module_adr + 2*0x4 )>>2],\
                this->m_pDrv->readReg((module_adr ),CAM_A),\
                this->m_pDrv->readReg((module_adr + 1*0x4 ),CAM_A),\
                this->m_pDrv->readReg((module_adr + 2*0x4 ),CAM_A));
    }
    else{
        this->m_pDrv->cqNopModule(CAM_ISP_SL2F_);
    }

    //OBC
    if(vpTuningNodes[page]->ctlEn_CAM & OBC_EN_){
        CAM_WRITE_BITS(this->m_pDrv,CAM_CTL_EN,OBC_EN,1);
    } else{
        CAM_WRITE_BITS(this->m_pDrv,CAM_CTL_EN,OBC_EN,0);
    }

    if(vpTuningNodes[page]->eUpdateFuncBit & eIspTuningMgrFunc_OBC){

        this->m_pDrv->cqApbModule(CAM_ISP_OBC_);

        //
        this->m_pDrv->getCQModuleInfo(CAM_ISP_OBC_,module_adr,module_size);
        //
        pReg_st = (ISP_DRV_REG_IO_STRUCT*)malloc(sizeof(ISP_DRV_REG_IO_STRUCT)*module_size);
        //
        for(MUINT32 j=0;j<module_size;j++)
            pReg_st[j].Data = vpTuningNodes[page]->pTuningRegBuf[(module_adr + j*0x4)>>2];
        //
        CAM_BURST_WRITE_REGS(this->m_pDrv,CAM_OBC_OFFST0,pReg_st,module_size);

        free(pReg_st);

        CAM_FUNC_DBG("OBC:[0x%x_0x%x_0x%x],[0x%x_0x%x_0x%x] \n",\
                vpTuningNodes[page]->pTuningRegBuf[(module_adr )>>2],\
                vpTuningNodes[page]->pTuningRegBuf[(module_adr + 1*0x4 )>>2],\
                vpTuningNodes[page]->pTuningRegBuf[(module_adr + 2*0x4 )>>2],\
                this->m_pDrv->readReg((module_adr ),CAM_A),\
                this->m_pDrv->readReg((module_adr + 1*0x4 ),CAM_A),\
                this->m_pDrv->readReg((module_adr + 2*0x4 ),CAM_A));
    }
    else{
        this->m_pDrv->cqNopModule(CAM_ISP_OBC_);
    }

    //RMG
    if(vpTuningNodes[page]->ctlEn_CAM & RMG_EN_){
        CAM_WRITE_BITS(this->m_pDrv,CAM_CTL_EN,RMG_EN,1);
    } else{
        CAM_WRITE_BITS(this->m_pDrv,CAM_CTL_EN,RMG_EN,0);
    }

    if(vpTuningNodes[page]->eUpdateFuncBit & eIspTuningMgrFunc_RMG){

        this->m_pDrv->cqApbModule(CAM_ISP_RMG_);

        //
        this->m_pDrv->getCQModuleInfo(CAM_ISP_RMG_,module_adr,module_size);
        //
        pReg_st = (ISP_DRV_REG_IO_STRUCT*)malloc(sizeof(ISP_DRV_REG_IO_STRUCT)*module_size);
        //
        for(MUINT32 j=0;j<module_size;j++)
            pReg_st[j].Data = vpTuningNodes[page]->pTuningRegBuf[(module_adr + j*0x4)>>2];
        //
        CAM_BURST_WRITE_REGS(this->m_pDrv,CAM_RMG_HDR_CFG,pReg_st,module_size);

        free(pReg_st);

        CAM_FUNC_DBG("RMG:[0x%x_0x%x_0x%x],[0x%x_0x%x_0x%x] \n",\
                vpTuningNodes[page]->pTuningRegBuf[(module_adr )>>2],\
                vpTuningNodes[page]->pTuningRegBuf[(module_adr + 1*0x4 )>>2],\
                vpTuningNodes[page]->pTuningRegBuf[(module_adr + 2*0x4 )>>2],\
                this->m_pDrv->readReg((module_adr ),CAM_A),\
                this->m_pDrv->readReg((module_adr + 1*0x4 ),CAM_A),\
                this->m_pDrv->readReg((module_adr + 2*0x4 ),CAM_A));
    }
    else{
        this->m_pDrv->cqNopModule(CAM_ISP_RMG_);
    }

    //BNR
    if(vpTuningNodes[page]->ctlEn_CAM & BNR_EN_){
        /*Enable BNR before PDO_EN is enabled*/
        MUINT32 pmx_sel = CAM_READ_BITS(this->m_pDrv,CAM_CTL_SEL,PMX_SEL);
        if (pmx_sel == 0) {
            if(CAM_READ_BITS(this->m_pDrv,CAM_CTL_DMA_EN,PDO_EN)){
                //check input
                if(0 == (pReg->CAM_BNR_BPC_CON.Bits.BPC_LUT_EN * (vpTuningNodes[page]->ctlEnDMA_CAM & BPCI_EN_))){
                    CAM_FUNC_ERR("bpci & lut_en must be enabled when pdc_en is enabled\n");
                    goto BYPASS_BNR;
                }
                else{
                    if( (pReg->CAM_BNR_PDC_CON.Bits.PDC_OUT * pReg->CAM_BNR_PDC_CON.Bits.PDC_EN * CAM_READ_BITS(this->m_pDrv,CAM_CTL_DMA_EN,PDO_EN)) == 0){
                        CAM_FUNC_ERR("PDC_EN must be enabled when PDO_EN is enabled\n");
                        ret = 1;
                        goto BYPASS_BNR;
                    }

                    _bpci_en = MTRUE;
                    _pdo_en = pReg->CAM_BNR_PDC_CON.Bits.PDC_OUT;
                }
            }
#if 0
            else{//check bpci
                if((pReg->CAM_BNR_BPC_CON.Bits.BPC_LUT_EN + (vpTuningNodes[page]->ctlEnDMA_CAM & BPCI_EN_)) != 0){
                    if((pReg->CAM_BNR_BPC_CON.Bits.BPC_LUT_EN * (vpTuningNodes[page]->ctlEnDMA_CAM & BPCI_EN_)) == 0){
                        CAM_FUNC_ERR("bpci & lut_en must be enabled when bpci/lut is enabled\n");
                        ret = 1;
                        goto BYPASS_BNR;
                    }
                }
            }
#endif
        } else {//pmx_sel = 1
            /* disable bpci_en when pmx_sel=1, because bpci table is no need for dual pd.
               current bnr is not using bpci table.

            //check input
            tmp = pReg->CAM_BNR_BPC_CON.Bits.BPC_LUT_EN * (vpTuningNodes[page]->ctlEnDMA_CAM & BPCI_EN_);
            if(tmp == 0 && pReg->CAM_BNR_BPC_CON.Bits.BPC_LUT_EN){
                CAM_FUNC_ERR("bpci & lut_en must be enabled/disabled at the same time\n");
                this->m_pDrv->cqNopModule(CAM_ISP_BNR_);
                goto BYPASS_BNR;
            }
            if( (pReg->CAM_BNR_PDC_CON.Bits.PDC_OUT * pReg->CAM_BNR_PDC_CON.Bits.PDC_EN * CAM_READ_BITS(this->m_pDrv,CAM_CTL_DMA_EN,PDO_EN)) == 1){
                CAM_FUNC_ERR("PDC_EN must be disabled when pmx_sel=1\n");
                ret = 1;
                goto BYPASS_BNR;
            }
            _bpci_en = MTRUE;*/
        }
        CAM_WRITE_BITS(this->m_pDrv,CAM_CTL_EN,BNR_EN,1);
    } else {
        MUINT32 pmx_sel = CAM_READ_BITS(this->m_pDrv,CAM_CTL_SEL,PMX_SEL);
        if(CAM_READ_BITS(this->m_pDrv,CAM_CTL_DMA_EN,PDO_EN) == 1 && !pmx_sel){
            CAM_FUNC_ERR("can't disable BNR when PDO_EN is enabled\n");
            goto BYPASS_BNR;
        }
        CAM_WRITE_BITS(this->m_pDrv,CAM_CTL_EN,BNR_EN,0);
        //bpci disable at vpTuningNodes[page]->eUpdateFuncBit & eIspTuningMgrFunc_BNR
    }
    if(vpTuningNodes[page]->eUpdateFuncBit & eIspTuningMgrFunc_BNR){

        this->m_pDrv->cqApbModule(CAM_ISP_BNR_);
        //
        this->m_pDrv->getCQModuleInfo(CAM_ISP_BNR_,module_adr,module_size);
        //
        pReg_st = (ISP_DRV_REG_IO_STRUCT*)malloc(sizeof(ISP_DRV_REG_IO_STRUCT)*module_size);
        //
        for(MUINT32 j=0;j<module_size;j++)
            pReg_st[j].Data = vpTuningNodes[page]->pTuningRegBuf[(module_adr + j*0x4)>>2];
        //
        CAM_BURST_WRITE_REGS(this->m_pDrv,CAM_BNR_BPC_CON,pReg_st,module_size);

        free(pReg_st);

        if (_bpci_en) {
            MUINT32 in_h,in_v;
            CAM_REG_TG_SEN_GRAB_PXL TG_W;
            CAM_REG_TG_SEN_GRAB_LIN TG_H;

            //for pipechk
            PIPE_CHECK pipe_chk;
            //ion handle: buffer
            ISP_DEV_ION_NODE_STRUCT IonNode;
            IonNode.devNode = this->m_hwModule;
            IonNode.memID = vpTuningNodes[page]->Dmai_bufinfo[eIspTuningMgrFuncBit_BNR].FD;
            IonNode.dmaPort = _dma_bpci_;
            this->m_pDrv->setDeviceInfo(_SET_ION_HANDLE,(MUINT8*)&IonNode);

            TG_W.Raw = CAM_READ_REG(this->m_pDrv->getPhyObj(),CAM_TG_SEN_GRAB_PXL);
            TG_H.Raw = CAM_READ_REG(this->m_pDrv->getPhyObj(),CAM_TG_SEN_GRAB_LIN);

            in_h = TG_W.Bits.PXL_E - TG_W.Bits.PXL_S;
            in_v = TG_H.Bits.LIN_E - TG_H.Bits.LIN_S;

            in_h = in_h>> (CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_CTL_EN,BIN_EN) + \
                                    CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_CTL_EN,DBN_EN));
            in_v = in_v>> CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_CTL_EN,BIN_EN);

            CAM_WRITE_BITS(this->m_pDrv, CAM_BNR_BPC_TBLI2, BPC_XSIZE, in_h-1);
            CAM_WRITE_BITS(this->m_pDrv, CAM_BNR_BPC_TBLI2, BPC_YSIZE, in_v-1);

            bpci.m_pIspDrv = this->m_pIspDrv;
            bpci.dma_cfg.memBuf.base_pAddr = pReg->CAM_BPCI_BASE_ADDR.Raw;
            bpci.dma_cfg.size.xsize = pReg->CAM_BPCI_XSIZE.Bits.XSIZE;
            bpci.dma_cfg.size.h = pReg->CAM_BPCI_YSIZE.Bits.YSIZE;
            bpci.dma_cfg.size.stride = bpci.dma_cfg.size.xsize;
            bpci.write2CQ();
            bpci.config();
            bpci.enable(NULL);

            pipe_chk.m_BPCI_VA = vpTuningNodes[page]->Dmai_bufinfo[eIspTuningMgrFuncBit_BNR].VA_addr;
        }
        else{
            PIPE_CHECK pipe_chk;
            bpci.m_pIspDrv = this->m_pIspDrv;
            bpci.disable();

            pipe_chk.m_BPCI_VA = 0;
        }

        if(_pdo_en){
            pdo.m_pIspDrv = this->m_pIspDrv;
            pdo.dma_cfg.size.xsize = pReg->CAM_PDO_XSIZE.Bits.XSIZE;
            pdo.dma_cfg.size.h = pReg->CAM_PDO_YSIZE.Bits.YSIZE;
            pdo.dma_cfg.size.stride = PDO_STRIDE_ALIGN(pdo.dma_cfg.size.xsize);
            pdo.config();
        }

        CAM_FUNC_DBG("BNR:[0x%x_0x%x_0x%x],[0x%x_0x%x_0x%x] \n",\
                vpTuningNodes[page]->pTuningRegBuf[(module_adr )>>2],\
                vpTuningNodes[page]->pTuningRegBuf[(module_adr + 1*0x4 )>>2],\
                vpTuningNodes[page]->pTuningRegBuf[(module_adr + 2*0x4 )>>2],\
                this->m_pDrv->readReg((module_adr ),CAM_A),\
                this->m_pDrv->readReg((module_adr + 1*0x4 ),CAM_A),\
                this->m_pDrv->readReg((module_adr + 2*0x4 ),CAM_A));
    }
    else{
        this->m_pDrv->cqNopModule(CAM_ISP_BNR_);
    }
BYPASS_BNR:

    //PBN
    _pdo_en = MFALSE;
    if(vpTuningNodes[page]->ctlEn_CAM & PBN_EN_){
        MUINT32 pmx_sel = CAM_READ_BITS(this->m_pDrv,CAM_CTL_SEL,PMX_SEL);
        MUINT32 pbn_en = CAM_READ_BITS(this->m_pDrv,CAM_CTL_EN,PBN_EN);

        if (pbn_en !=1){
            CAM_FUNC_ERR("can't set PBN tuning when PBN_EN=0\n");
            goto BYPASS_PBN;
        }
        if (pmx_sel != 1){
            CAM_FUNC_ERR("can't enable PBN but PMX_SEL=0\n");
            goto BYPASS_PBN;
        }
        //PBN_EN at twin_drv
    }
    else{
        if (CAM_READ_BITS(this->m_pDrv,CAM_CTL_EN,PBN_EN)) {
            CAM_FUNC_ERR("can't disable PBN by tuning\n");
            goto BYPASS_PBN;
        }
    }

    if(vpTuningNodes[page]->eUpdateFuncBit & eIspTuningMgrFunc_PBN){
        DMA_PDO pdo;

        _pdo_en = CAM_READ_BITS(this->m_pDrv,CAM_CTL_DMA_EN,PDO_EN);

        /* PBN_EN will set by twin driver */
        this->m_pDrv->cqApbModule(CAM_ISP_PBN_);
        //
        this->m_pDrv->getCQModuleInfo(CAM_ISP_PBN_,module_adr,module_size);
        //
        pReg_st = (ISP_DRV_REG_IO_STRUCT*)malloc(sizeof(ISP_DRV_REG_IO_STRUCT)*module_size);
        //
        CAM_REG_TG_SEN_GRAB_LIN TG_H;
        TG_H.Raw = CAM_READ_REG(this->m_pDrv->getPhyObj(),CAM_TG_SEN_GRAB_LIN);
        pReg->CAM_PBN_VSIZE.Bits.PBN_VSIZE = TG_H.Bits.LIN_E - TG_H.Bits.LIN_S;
        //
        for(MUINT32 j=0;j<module_size;j++)
            pReg_st[j].Data = vpTuningNodes[page]->pTuningRegBuf[(module_adr + j*0x4)>>2];
        //
        CAM_BURST_WRITE_REGS(this->m_pDrv,CAM_PBN_TYPE,pReg_st,module_size);
        free(pReg_st);
        //
        if(_pdo_en){
            pdo.m_pIspDrv = this->m_pIspDrv;
            pdo.dma_cfg.size.xsize = pReg->CAM_PDO_XSIZE.Bits.XSIZE;
            pdo.dma_cfg.size.h = pReg->CAM_PDO_YSIZE.Bits.YSIZE;
            pdo.dma_cfg.size.stride = PDO_STRIDE_ALIGN(pdo.dma_cfg.size.xsize);
            pdo.config();
        }

        CAM_FUNC_DBG("PBN:[0x%x_0x%x_0x%x],[0x%x_0x%x_0x%x],PMX[0x%x_0x%x]\n",\
            vpTuningNodes[page]->pTuningRegBuf[(module_adr )>>2],\
            vpTuningNodes[page]->pTuningRegBuf[(module_adr + 1*0x4 )>>2],\
            vpTuningNodes[page]->pTuningRegBuf[(module_adr + 2*0x4 )>>2],\
            this->m_pDrv->readReg((module_adr ),CAM_A),\
            this->m_pDrv->readReg((module_adr + 1*0x4 ),CAM_A),\
            this->m_pDrv->readReg((module_adr + 2*0x4 ),CAM_A),\
            CAM_READ_REG(this->m_pDrv,CAM_PMX_CROP),\
            CAM_READ_REG(this->m_pDrv,CAM_PMX_VSIZE));

    }
    else{
        this->m_pDrv->cqNopModule(CAM_ISP_PBN_);
    }
BYPASS_PBN:

    //RMM
    if(vpTuningNodes[page]->ctlEn_CAM & RMM_EN_){
        CAM_WRITE_BITS(this->m_pDrv,CAM_CTL_EN,RMM_EN,1);
    } else{
        CAM_WRITE_BITS(this->m_pDrv,CAM_CTL_EN,RMM_EN,0);
    }

    if(vpTuningNodes[page]->eUpdateFuncBit & eIspTuningMgrFunc_RMM){

        this->m_pDrv->cqApbModule(CAM_ISP_RMM_);

        //
        this->m_pDrv->getCQModuleInfo(CAM_ISP_RMM_,module_adr,module_size);
        //
        pReg_st = (ISP_DRV_REG_IO_STRUCT*)malloc(sizeof(ISP_DRV_REG_IO_STRUCT)*module_size);
        //
        for(MUINT32 j=0;j<module_size;j++)
            pReg_st[j].Data = vpTuningNodes[page]->pTuningRegBuf[(module_adr + j*0x4)>>2];
        //
        CAM_BURST_WRITE_REGS(this->m_pDrv,CAM_RMM_OSC,pReg_st,module_size);

        free(pReg_st);

        CAM_FUNC_DBG("RMM:[0x%x_0x%x_0x%x],[0x%x_0x%x_0x%x] \n",\
                vpTuningNodes[page]->pTuningRegBuf[(module_adr )>>2],\
                vpTuningNodes[page]->pTuningRegBuf[(module_adr + 1*0x4 )>>2],\
                vpTuningNodes[page]->pTuningRegBuf[(module_adr + 2*0x4 )>>2],\
                this->m_pDrv->readReg((module_adr ),CAM_A),\
                this->m_pDrv->readReg((module_adr + 1*0x4 ),CAM_A),\
                this->m_pDrv->readReg((module_adr + 2*0x4 ),CAM_A));
    }
    else{
        this->m_pDrv->cqNopModule(CAM_ISP_RMM_);
    }
#if 1
    //LSC
    if(vpTuningNodes[page]->ctlEn_CAM & LSC_EN_) {
        if((vpTuningNodes[page]->ctlEnDMA_CAM & LSCI_EN_) == 0){
            CAM_FUNC_ERR("LSC & LSCI must be enabled at the same time\n");
            this->m_pDrv->cqNopModule(CAM_ISP_LSC_);
            goto BYPASS_LSC;
        }

        CAM_WRITE_BITS(this->m_pDrv,CAM_CTL_EN,LSC_EN,1);
        lsci.enable(NULL);
    }
    else{
        CAM_WRITE_BITS(this->m_pDrv,CAM_CTL_EN,LSC_EN,0);
        lsci.disable();
    }
    if(vpTuningNodes[page]->eUpdateFuncBit & eIspTuningMgrFunc_LSC){

        this->m_pDrv->cqApbModule(CAM_ISP_LSC_);

        //lsc
        this->m_pDrv->getCQModuleInfo(CAM_ISP_LSC_,module_adr,module_size);
        //
        pReg_st = (ISP_DRV_REG_IO_STRUCT*)malloc(sizeof(ISP_DRV_REG_IO_STRUCT)*module_size);
        //
        for(MUINT32 j=0;j<module_size;j++)
            pReg_st[j].Data = vpTuningNodes[page]->pTuningRegBuf[(module_adr + j*0x4)>>2];
        //
        CAM_BURST_WRITE_REGS(this->m_pDrv,CAM_LSC_CTL1,pReg_st,module_size);

        free(pReg_st);

        //ion handle: buffer
        ISP_DEV_ION_NODE_STRUCT IonNode;
        IonNode.devNode = this->m_hwModule;
        IonNode.memID = vpTuningNodes[page]->Dmai_bufinfo[eIspTuningMgrFuncBit_LSC].FD;
        IonNode.dmaPort = _dma_lsci_;
        this->m_pDrv->setDeviceInfo(_SET_ION_HANDLE,(MUINT8*)&IonNode);

        lsci.dma_cfg.memBuf.base_pAddr = pReg->CAM_LSCI_BASE_ADDR.Raw;
        lsci.dma_cfg.size.xsize = pReg->CAM_LSCI_XSIZE.Bits.XSIZE;
        lsci.dma_cfg.size.h = pReg->CAM_LSCI_YSIZE.Bits.YSIZE;
        lsci.dma_cfg.size.stride = lsci.dma_cfg.size.xsize;
        lsci.write2CQ();
        lsci.config();

        CAM_FUNC_DBG("LSC:[0x%x_0x%x_0x%x],[0x%x_0x%x_0x%x] \n",\
                vpTuningNodes[page]->pTuningRegBuf[(module_adr )>>2],\
                vpTuningNodes[page]->pTuningRegBuf[(module_adr + 1*0x4 )>>2],\
                vpTuningNodes[page]->pTuningRegBuf[(module_adr + 2*0x4 )>>2],\
                this->m_pDrv->readReg((module_adr ),CAM_A),\
                this->m_pDrv->readReg((module_adr + 1*0x4 ),CAM_A),\
                this->m_pDrv->readReg((module_adr + 2*0x4 ),CAM_A));
    }
    else{
        this->m_pDrv->cqNopModule(CAM_ISP_LSC_);
    }
#endif
BYPASS_LSC:

    //RPG
    if(vpTuningNodes[page]->ctlEn_CAM & RPG_EN_){
        CAM_WRITE_BITS(this->m_pDrv,CAM_CTL_EN,RPG_EN,1);
    } else{
        CAM_WRITE_BITS(this->m_pDrv,CAM_CTL_EN,RPG_EN,0);
    }


    if(vpTuningNodes[page]->eUpdateFuncBit & eIspTuningMgrFunc_RPG){

        this->m_pDrv->cqApbModule(CAM_ISP_RPG_);

        //
        this->m_pDrv->getCQModuleInfo(CAM_ISP_RPG_,module_adr,module_size);
        //
        pReg_st = (ISP_DRV_REG_IO_STRUCT*)malloc(sizeof(ISP_DRV_REG_IO_STRUCT)*module_size);
        //
        for(MUINT32 j=0;j<module_size;j++)
            pReg_st[j].Data = vpTuningNodes[page]->pTuningRegBuf[(module_adr + j*0x4)>>2];
        //
        CAM_BURST_WRITE_REGS(this->m_pDrv,CAM_RPG_SATU_1,pReg_st,module_size);

        free(pReg_st);

        CAM_FUNC_DBG("RPG:[0x%x_0x%x_0x%x],[0x%x_0x%x_0x%x] \n",\
                vpTuningNodes[page]->pTuningRegBuf[(module_adr )>>2],\
                vpTuningNodes[page]->pTuningRegBuf[(module_adr + 1*0x4 )>>2],\
                vpTuningNodes[page]->pTuningRegBuf[(module_adr + 2*0x4 )>>2],\
                this->m_pDrv->readReg((module_adr ),CAM_A),\
                this->m_pDrv->readReg((module_adr + 1*0x4 ),CAM_A),\
                this->m_pDrv->readReg((module_adr + 2*0x4 ),CAM_A));
    }
    else{
        this->m_pDrv->cqNopModule(CAM_ISP_RPG_);
    }

    //CPG
    if(vpTuningNodes[page]->ctlEn_CAM & CPG_EN_){
        CAM_WRITE_BITS(this->m_pDrv,CAM_CTL_EN,CPG_EN,1);
    } else{
        CAM_WRITE_BITS(this->m_pDrv,CAM_CTL_EN,CPG_EN,0);
    }

    if(vpTuningNodes[page]->eUpdateFuncBit & eIspTuningMgrFunc_CPG){

        this->m_pDrv->cqApbModule(CAM_ISP_CPG_);

        //
        this->m_pDrv->getCQModuleInfo(CAM_ISP_CPG_,module_adr,module_size);
        //
        pReg_st = (ISP_DRV_REG_IO_STRUCT*)malloc(sizeof(ISP_DRV_REG_IO_STRUCT)*module_size);
        //
        for(MUINT32 j=0;j<module_size;j++)
            pReg_st[j].Data = vpTuningNodes[page]->pTuningRegBuf[(module_adr + j*0x4)>>2];
        //
        CAM_BURST_WRITE_REGS(this->m_pDrv,CAM_CPG_SATU_1,pReg_st,module_size);

        free(pReg_st);

        CAM_FUNC_DBG("CPG:[0x%x_0x%x_0x%x],[0x%x_0x%x_0x%x] \n",\
                vpTuningNodes[page]->pTuningRegBuf[(module_adr )>>2],\
                vpTuningNodes[page]->pTuningRegBuf[(module_adr + 1*0x4 )>>2],\
                vpTuningNodes[page]->pTuningRegBuf[(module_adr + 2*0x4 )>>2],\
                this->m_pDrv->readReg((module_adr ),CAM_A),\
                this->m_pDrv->readReg((module_adr + 1*0x4 ),CAM_A),\
                this->m_pDrv->readReg((module_adr + 2*0x4 ),CAM_A));
    }
    else{
        this->m_pDrv->cqNopModule(CAM_ISP_CPG_);
    }


    //AWB & AE
    if(vpTuningNodes[page]->ctlEn_CAM & AA_EN_){
        if((vpTuningNodes[page]->ctlEnDMA_CAM & AAO_EN_) == 0){
            CAM_FUNC_ERR("aa & aao must be enabled at the same time\n");
            goto BYPASS_AA;
        }
        CAM_WRITE_BITS(this->m_pDrv,CAM_CTL_EN,AA_EN,1);

    }
    else {
        if(1 == CAM_READ_BITS(this->m_pDrv,CAM_FBC_AAO_CTL1,FBC_EN)) {
            CAM_FUNC_ERR("AA supported no dynamic func off under continuous mode\n");
            goto BYPASS_AA;
        }
    }

    if(vpTuningNodes[page]->eUpdateFuncBit & eIspTuningMgrFunc_AA){
        DMA_AAO aao;

        this->m_pDrv->cqApbModule(CAM_ISP_AWB_);
        this->m_pDrv->cqApbModule(CAM_ISP_AE_);

        //awb
        this->m_pDrv->getCQModuleInfo(CAM_ISP_AWB_,module_adr,module_size);
        //
        pReg_st = (ISP_DRV_REG_IO_STRUCT*)malloc(sizeof(ISP_DRV_REG_IO_STRUCT)*module_size);
        //
        for(MUINT32 j=0;j<module_size;j++)
            pReg_st[j].Data = vpTuningNodes[page]->pTuningRegBuf[(module_adr + j*0x4)>>2];
        //
        CAM_BURST_WRITE_REGS(this->m_pDrv,CAM_AWB_WIN_ORG,pReg_st,module_size);

        free(pReg_st);

        //ae
        this->m_pDrv->getCQModuleInfo(CAM_ISP_AE_,module_adr,module_size);
        //
        pReg_st = (ISP_DRV_REG_IO_STRUCT*)malloc(sizeof(ISP_DRV_REG_IO_STRUCT)*module_size);
        //
        for(MUINT32 j=0;j<module_size;j++)
            pReg_st[j].Data = vpTuningNodes[page]->pTuningRegBuf[(module_adr + j*0x4)>>2];
        //
        CAM_BURST_WRITE_REGS(this->m_pDrv,CAM_AE_HST_CTL,pReg_st,module_size);

        free(pReg_st);

        //aao , ref to the config() at statistic pipe
        aao.m_pIspDrv = this->m_pIspDrv;
        aao.dma_cfg.size.xsize = pReg->CAM_AAO_XSIZE.Bits.XSIZE;
        aao.dma_cfg.size.h = pReg->CAM_AAO_YSIZE.Bits.YSIZE;
        aao.dma_cfg.size.stride = aao.dma_cfg.size.xsize;
        aao.config();

        CAM_FUNC_DBG("AE:[0x%x_0x%x_0x%x_0x%x],[0x%x_0x%x_0x%x_0x%x] \n",\
                vpTuningNodes[page]->pTuningRegBuf[(module_adr )>>2],\
                vpTuningNodes[page]->pTuningRegBuf[(module_adr + 1*0x4 )>>2],\
                vpTuningNodes[page]->pTuningRegBuf[(module_adr + 2*0x4 )>>2],\
                vpTuningNodes[page]->pTuningRegBuf[0xA38>>2],\
                this->m_pDrv->readReg((module_adr ),CAM_A),\
                this->m_pDrv->readReg((module_adr + 1*0x4 ),CAM_A),\
                this->m_pDrv->readReg((module_adr + 2*0x4 ),CAM_A),\
                this->m_pDrv->readReg(0xA38,CAM_A));
    } else{
        this->m_pDrv->cqNopModule(CAM_ISP_AWB_);
        this->m_pDrv->cqNopModule(CAM_ISP_AE_);
    }
BYPASS_AA:

    //SGG1
    if(vpTuningNodes[page]->ctlEn_CAM & SGG1_EN_){
        CAM_WRITE_BITS(this->m_pDrv,CAM_CTL_EN,SGG1_EN,1);
    } else{
        CAM_WRITE_BITS(this->m_pDrv,CAM_CTL_EN,SGG1_EN,0);
    }

    if(vpTuningNodes[page]->eUpdateFuncBit & eIspTuningMgrFunc_SGG1){

        this->m_pDrv->cqApbModule(CAM_ISP_SGG1_);

        //
        this->m_pDrv->getCQModuleInfo(CAM_ISP_SGG1_,module_adr,module_size);
        //
        pReg_st = (ISP_DRV_REG_IO_STRUCT*)malloc(sizeof(ISP_DRV_REG_IO_STRUCT)*module_size);
        //
        for(MUINT32 j=0;j<module_size;j++)
            pReg_st[j].Data = vpTuningNodes[page]->pTuningRegBuf[(module_adr + j*0x4)>>2];
        //
        CAM_BURST_WRITE_REGS(this->m_pDrv,CAM_SGG1_PGN,pReg_st,module_size);

        free(pReg_st);

        CAM_FUNC_DBG("SGG1:[0x%x_0x%x_0x%x],[0x%x_0x%x_0x%x] \n",\
                vpTuningNodes[page]->pTuningRegBuf[(module_adr )>>2],\
                vpTuningNodes[page]->pTuningRegBuf[(module_adr + 1*0x4 )>>2],\
                vpTuningNodes[page]->pTuningRegBuf[(module_adr + 2*0x4 )>>2],\
                this->m_pDrv->readReg((module_adr ),CAM_A),\
                this->m_pDrv->readReg((module_adr + 1*0x4 ),CAM_A),\
                this->m_pDrv->readReg((module_adr + 2*0x4 ),CAM_A));
    } else {
        this->m_pDrv->cqNopModule(CAM_ISP_SGG1_);
    }

    //AF
    if(vpTuningNodes[page]->ctlEn_CAM & AF_EN_){
        if(CAM_READ_BITS(this->m_pDrv,CAM_CTL_EN,SGG1_EN) != 1){
            CAM_FUNC_ERR("need to enable SGG1 if AF/AFO is needed\n");
            goto BYPASS_AF;
        }
        if((vpTuningNodes[page]->ctlEnDMA_CAM & AFO_EN_) == 0){
            CAM_FUNC_ERR("af & afo must be enabled at the same time\n");
            goto BYPASS_AF;
        }
        CAM_WRITE_BITS(this->m_pDrv,CAM_CTL_EN,AF_EN,1);
    }
    else {
        if(1 == CAM_READ_BITS(this->m_pDrv,CAM_FBC_AFO_CTL1,FBC_EN)) {
            CAM_FUNC_ERR("AF supported no dynamic func off under continuous mode\n");
            goto BYPASS_AF;
        }
    }

    if(vpTuningNodes[page]->eUpdateFuncBit & eIspTuningMgrFunc_AF){
        DMA_AFO afo;

        this->m_pDrv->cqApbModule(CAM_ISP_AF_);

        //
        this->m_pDrv->getCQModuleInfo(CAM_ISP_AF_,module_adr,module_size);
        //
        pReg_st = (ISP_DRV_REG_IO_STRUCT*)malloc(sizeof(ISP_DRV_REG_IO_STRUCT)*module_size);
        //
        for(MUINT32 j=0;j<module_size;j++)
            pReg_st[j].Data = vpTuningNodes[page]->pTuningRegBuf[(module_adr + j*0x4)>>2];
        //
        CAM_BURST_WRITE_REGS(this->m_pDrv,CAM_AF_CON,pReg_st,module_size);

        free(pReg_st);

        //afo , ref to the config() at statistic pipe
        afo.m_pIspDrv = this->m_pIspDrv;
        afo.dma_cfg.size.xsize = pReg->CAM_AFO_XSIZE.Bits.XSIZE;
        afo.dma_cfg.size.h = pReg->CAM_AFO_YSIZE.Bits.YSIZE;

#if 1       //avoid CAM_A or CAM_B'S  afo being disabled by twin drv af touch at the corner of image
        afo.dma_cfg.size.stride = afo.dma_cfg.size.xsize + AFO_DUMMY_STRIDE;
#else
        afo.dma_cfg.size.stride = afo.dma_cfg.size.xsize;
#endif
        afo.config();


        CAM_FUNC_DBG("AF:[0x%x_0x%x_0x%x_0x%x],[0x%x_0x%x_0x%x_0x%x] \n",\
                vpTuningNodes[page]->pTuningRegBuf[(module_adr )>>2],\
                vpTuningNodes[page]->pTuningRegBuf[(module_adr + 1*0x4 )>>2],\
                vpTuningNodes[page]->pTuningRegBuf[(module_adr + 2*0x4 )>>2],\
                vpTuningNodes[page]->pTuningRegBuf[(module_adr + 3*0x4 )>>2],\
                this->m_pDrv->readReg((module_adr ),CAM_A),\
                this->m_pDrv->readReg((module_adr + 1*0x4 ),CAM_A),\
                this->m_pDrv->readReg((module_adr + 2*0x4 ),CAM_A),\
                this->m_pDrv->readReg((module_adr + 3*0x4 ),CAM_A));
    } else {
        this->m_pDrv->cqNopModule(CAM_ISP_AF_);
    }
BYPASS_AF:

    //SGG3
    if(vpTuningNodes[page]->ctlEn_UNI & SGG3_A_EN_){
        UNI_WRITE_BITS(this->m_pDrv,CAM_UNI_TOP_MOD_EN,SGG3_A_EN,1);
    } else{
        UNI_WRITE_BITS(this->m_pDrv,CAM_UNI_TOP_MOD_EN,SGG3_A_EN,0);
    }

    if(vpTuningNodes[page]->eUpdateFuncBit & eIspTuningMgrFunc_SGG3){
        if(this->m_pUniDrv == NULL){
            CAM_FUNC_ERR("UNI is not linked with current module\n");
            goto BYPASS_SGG3;
        }
        this->m_pDrv->cqApbModule(UNI_ISP_SGG3_);

        //
        this->m_pDrv->getCQModuleInfo(UNI_ISP_SGG3_,module_adr,module_size);
        //
        pReg_st = (ISP_DRV_REG_IO_STRUCT*)malloc(sizeof(ISP_DRV_REG_IO_STRUCT)*module_size);
        //
        for(MUINT32 j=0;j<module_size;j++)
            pReg_st[j].Data = vpTuningNodes[page]->pTuningRegBuf[(module_adr + j*0x4 + ISP_REG_RANGE)>>2];
        //
        UNI_BURST_WRITE_REGS(this->m_pDrv,CAM_UNI_SGG3_A_PGN,pReg_st,module_size);

        free(pReg_st);

        CAM_FUNC_DBG("SGG3:[0x%x_0x%x_0x%x],[0x%x_0x%x_0x%x] \n",\
                vpTuningNodes[page]->pTuningRegBuf[(module_adr +  ISP_REG_RANGE)>>2],\
                vpTuningNodes[page]->pTuningRegBuf[(module_adr + 1*0x4 + ISP_REG_RANGE)>>2],\
                vpTuningNodes[page]->pTuningRegBuf[(module_adr + 2*0x4 + ISP_REG_RANGE)>>2],\
                this->m_pDrv->readReg((module_adr ),UNI_A),\
                this->m_pDrv->readReg((module_adr + 1*0x4 ),UNI_A),\
                this->m_pDrv->readReg((module_adr + 2*0x4 ),UNI_A));
    }
    else{
        this->m_pDrv->cqNopModule(UNI_ISP_SGG3_);
    }
BYPASS_SGG3:

    //FLK
    if(vpTuningNodes[page]->ctlEn_UNI & FLK_A_EN_){
        if(UNI_READ_BITS(this->m_pDrv,CAM_UNI_TOP_MOD_EN,SGG3_A_EN) != 1){
            CAM_FUNC_ERR("need to enable SGG3 if FLK/FLKO is needed\n");
            goto BYPASS_FLK;
        }

        if((vpTuningNodes[page]->ctlEnDMA_UNI & FLKO_A_EN_) == 0){
            CAM_FUNC_ERR("FLK & FLKo must be enabled at the same time\n");
            goto BYPASS_FLK;
        }
        UNI_WRITE_BITS(this->m_pDrv,CAM_UNI_TOP_MOD_EN,FLK_A_EN,1);

    }
    else{
        if(this->m_pUniDrv == NULL){
            goto BYPASS_FLK;
        }
        if(1 == UNI_READ_BITS(this->m_pUniDrv,CAM_UNI_FBC_FLKO_A_CTL1,FBC_EN)){
            CAM_FUNC_ERR("FLK supported no dynamic func off under continuous mode\n");
            goto BYPASS_FLK;
        }
    }

    if(vpTuningNodes[page]->eUpdateFuncBit & eIspTuningMgrFunc_FLK){
        DMA_FLKO flko;
        cam_uni_reg_t* pUniReg = (cam_uni_reg_t*)&vpTuningNodes[page]->pTuningRegBuf[ISP_REG_RANGE>>2];
        if(this->m_pUniDrv == NULL){
            CAM_FUNC_ERR("UNI is not linked with current module\n");
            goto BYPASS_FLK;
        }

        this->m_pDrv->cqApbModule(UNI_ISP_FLK_);

        //
        this->m_pDrv->getCQModuleInfo(UNI_ISP_FLK_,module_adr,module_size);
        //
        pReg_st = (ISP_DRV_REG_IO_STRUCT*)malloc(sizeof(ISP_DRV_REG_IO_STRUCT)*module_size);
        //
        for(MUINT32 j=0;j<module_size;j++)
            pReg_st[j].Data = vpTuningNodes[page]->pTuningRegBuf[(module_adr + j*0x4 + ISP_REG_RANGE)>>2];
        //
        UNI_BURST_WRITE_REGS(this->m_pDrv,CAM_UNI_FLK_A_CON,pReg_st,module_size);

        free(pReg_st);

        //FLKo , ref to the config() at statistic pipe
        flko.m_pIspDrv = this->m_pIspDrv;
        flko.m_pUniDrv = this->m_pUniDrv;
        flko.dma_cfg.size.xsize = pUniReg->CAM_UNI_FLKO_XSIZE.Bits.XSIZE;
        flko.dma_cfg.size.h = pUniReg->CAM_UNI_FLKO_YSIZE.Bits.YSIZE;
        flko.dma_cfg.size.stride = flko.dma_cfg.size.xsize;
        flko.config();

        CAM_FUNC_DBG("FLK:[0x%x_0x%x_0x%x],[0x%x_0x%x_0x%x] \n",\
                vpTuningNodes[page]->pTuningRegBuf[(module_adr +  ISP_REG_RANGE)>>2],\
                vpTuningNodes[page]->pTuningRegBuf[(module_adr + 1*0x4 + ISP_REG_RANGE)>>2],\
                vpTuningNodes[page]->pTuningRegBuf[(module_adr + 2*0x4 + ISP_REG_RANGE)>>2],\
                this->m_pDrv->readReg((module_adr ),UNI_A),\
                this->m_pDrv->readReg((module_adr + 1*0x4 ),UNI_A),\
                this->m_pDrv->readReg((module_adr + 2*0x4 ),UNI_A));
    }
    else{
        this->m_pDrv->cqNopModule(UNI_ISP_FLK_);
    }
BYPASS_FLK:
    CAM_FUNC_DBG("CAM_TUNING_CTRL - \n");





EXIT:

    return ret;
}

MINT32 CAM_TUNING_CTRL::update_end(void)
{
    MINT32 ret = 0;
    ETuningDrvUser TuningDrvUser = eTuningDrvUser_3A;

    if(MFALSE == ((TuningDrv*)this->m_pTuningDrv)->enTuningQueByDrv(TuningDrvUser)){
        CAM_FUNC_ERR("en tuning que fail\n");
        ret = 1;
    }
    return ret;
}

/*/////////////////////////////////////////////////////////////////////////////
  ISP_YUV_PIPE
/////////////////////////////////////////////////////////////////////////////*/
MINT32 CAM_YUV_PIPE::_config( void )
{
    E_ISP_CAM_CQ cq;
    MUINT32 page;
    MUINT32 pixmode;
    CAM_REG_CTL_FMT_SEL fmt_sel;
    Header_IMGO fh_imgo;

    this->m_pDrv = (ISP_DRV_CAM*)this->m_pIspDrv;
    //dbg log
    this->m_pDrv->getCurObjInfo(&this->m_hwModule,&cq,&page);
    pixmode = CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_TG_SEN_MODE,DBL_DATA_BUS) + \
          CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_TG_SEN_MODE,DBL_DATA_BUS1);
    if (page == 0) { /*reduce log, only show page 0*/
        CAM_FUNC_INF("CAM_YUV_PIPE::_config+ cq:0x%x,page:0x%x,pix_mode:0x%x\n",cq,page,\
            pixmode);
    }


    //default always enable
    CAM_WRITE_BITS(this->m_pDrv,CAM_CTL_EN,QBIN2_EN, 1);

    //qbin2
    CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),CAM_QBN2_MODE, QBN_ACC_MODE,1);//always use average
    CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),CAM_QBN2_MODE, QBN_ACC,pixmode);
    //bin3 & hds setting is in UNI_TOP


    //image header
    fmt_sel.Raw = CAM_READ_REG(this->m_pDrv,CAM_CTL_FMT_SEL);
    fh_imgo.Header_Enque(Header_IMGO::E_PIX_ID,this->m_pDrv,fmt_sel.Bits.PIX_ID);
    fh_imgo.Header_Enque(Header_IMGO::E_FMT,this->m_pDrv,fmt_sel.Bits.IMGO_FMT);
    return 0;
}



MINT32 CAM_YUV_PIPE::_write2CQ(void)
{
    E_ISP_CAM_CQ cq;
    MUINT32 page;
    this->m_pDrv = (ISP_DRV_CAM*)this->m_pIspDrv;
    //dbg log
    this->m_pDrv->getCurObjInfo(&this->m_hwModule,&cq,&page);

    CAM_FUNC_DBG("CAM_YUV_PIPE::_write2CQ+ cq:0x%x,page:0x%x\n",cq,page);


    this->m_pDrv->cqAddModule(CAM_ISP_QBN2_);
    this->m_pDrv->cqAddModule(CAM_ISP_LCS_);
    this->m_pDrv->cqAddModule(CAM_ISP_SGG1_);
    this->m_pDrv->cqAddModule(CAM_ISP_AF_);


    //NOP function , these func should apb when being used at tuning
    this->m_pDrv->cqNopModule(CAM_ISP_LCS_);
    this->m_pDrv->cqNopModule(CAM_ISP_SGG1_);
    this->m_pDrv->cqNopModule(CAM_ISP_AF_);

    return 0;
}



MBOOL CAM_YUV_PIPE::setEIS(void)
{
    MBOOL ret = MTRUE;
    DMA_EISO eiso;

    this->m_pDrv = (ISP_DRV_CAM*)this->m_pIspDrv;
    if(this->m_pP1Tuning){
        EIS_REG_CFG _eis_cfg;
        EIS_INPUT_INFO input;

        CAM_REG_TG_SEN_GRAB_PXL TG_W;
        CAM_REG_TG_SEN_GRAB_LIN TG_H;

        input.pixMode = UNI_READ_BITS(this->m_pDrv,CAM_UNI_HDS_A_MODE,HDS_DS);


        TG_W.Raw = CAM_READ_REG(this->m_pDrv->getPhyObj(),CAM_TG_SEN_GRAB_PXL);
        input.sTGOut.w = TG_W.Bits.PXL_E - TG_W.Bits.PXL_S;
        TG_H.Raw = CAM_READ_REG(this->m_pDrv->getPhyObj(),CAM_TG_SEN_GRAB_LIN);
        input.sTGOut.h = TG_H.Bits.LIN_E - TG_H.Bits.LIN_S;

        //
        input.sHBINOut.w = input.sTGOut.w >> input.pixMode;
        input.sHBINOut.h = input.sTGOut.h;

        input.sRMXOut.w = 0;
        input.sRMXOut.h = 0;

        input.bYUVFmt = MTRUE;

        CAM_FUNC_DBG("CAM_YUV_PIPE::setEIS:w/h(%d_%d,%d_%d)\n",input.sHBINOut.w,input.sHBINOut.h,input.sTGOut.w,input.sTGOut.h);
        this->m_pP1Tuning->p1TuningNotify((MVOID*)&input,(MVOID*)&_eis_cfg);
        CAM_FUNC_DBG("-\n");

        if(_eis_cfg.bEIS_Bypass == 0){
            if(_eis_cfg.bEIS_EN){
                MUINT32 addrost,modulesize;
                ISP_DRV_REG_IO_STRUCT* pReg;
                MUINT32* ptr = (MUINT32*)&_eis_cfg._EIS_REG.CTRL_1;
                this->m_pDrv->getCQModuleInfo(UNI_ISP_EIS_,addrost,modulesize);
                pReg = (ISP_DRV_REG_IO_STRUCT*)malloc(sizeof(ISP_DRV_REG_IO_STRUCT)*modulesize);

                for(MUINT32 i=0;i<modulesize;i++){
                    pReg[i].Data = *(ptr++);
                }
                UNI_BURST_WRITE_REGS(this->m_pDrv,CAM_UNI_EIS_A_PREP_ME_CTRL1,pReg,modulesize);

                UNI_WRITE_BITS(this->m_pDrv, CAM_UNI_TOP_MOD_EN, EIS_A_EN, 1);

                eiso.m_pUniDrv = this->m_pUniDrv;
                eiso.m_pIspDrv = this->m_pIspDrv;


                eiso.config();
                eiso.write2CQ();
                eiso.enable(NULL);


                this->m_pDrv->cqApbModule(UNI_ISP_EIS_);
                free(pReg);
            }
            else{
                CAM_FUNC_ERR("eis must be opened if notification is registered\n");
                ret = MFALSE;
            }


        }
        else{
            this->m_pDrv->cqNopModule(UNI_ISP_EIS_);
        }
    }
    return ret;
}

MBOOL CAM_YUV_PIPE::setLCS(void)
{

    MBOOL ret = MTRUE;
    DMA_LCSO lcso;


    this->m_pDrv = (ISP_DRV_CAM*)this->m_pIspDrv;

    if(this->m_pP1Tuning)
    {
        CAM_REG_TG_SEN_GRAB_PXL TG_W;
        CAM_REG_TG_SEN_GRAB_LIN TG_H;
        MUINT32 pixMode;
        LCS_REG_CFG     _lcs_cfg;
        LCS_INPUT_INFO  input;
        Header_RRZO fh_rrzo;

        TG_W.Raw = CAM_READ_REG(this->m_pDrv->getPhyObj(),CAM_TG_SEN_GRAB_PXL);
        TG_H.Raw = CAM_READ_REG(this->m_pDrv->getPhyObj(),CAM_TG_SEN_GRAB_LIN);

        //
        pixMode = (CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_TG_SEN_MODE,DBL_DATA_BUS) + \
                CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_TG_SEN_MODE,DBL_DATA_BUS1));

        if(pixMode != 0){
            CAM_FUNC_ERR("LCS support no 2/4 pixmode under YUV format\n");
            return MFALSE;
        }
        input.sHBINOut.w = (TG_W.Bits.PXL_E - TG_W.Bits.PXL_S);
        input.sHBINOut.h = TG_H.Bits.LIN_E - TG_H.Bits.LIN_S;
        input.bIsHbin = MTRUE;
        input.magic = fh_rrzo.GetRegInfo(Header_RRZO::E_Magic,this->m_pDrv);
        //
        input.sRRZOut.w = 0;
        input.sRRZOut.h = 0;

        //this info is request from lsc
        input.RRZ_IN_CROP.in_size_w = 0;
        input.RRZ_IN_CROP.in_size_h = 0;

        input.RRZ_IN_CROP.start_x = 0;
        input.RRZ_IN_CROP.start_y = 0;
        input.RRZ_IN_CROP.crop_size_w = 0;
        input.RRZ_IN_CROP.crop_size_h = 0;

        CAM_FUNC_DBG("+");
        this->m_pP1Tuning->p1TuningNotify((MVOID*)&input,(MVOID*)&_lcs_cfg);
        CAM_FUNC_DBG("-");
        CAM_FUNC_DBG("CAM_YUV_PIPE::setLCS:in:%d_%d, LCS_REG(%d): 0x%x,0x%x,m(%d)\n",input.sRRZOut.w,input.sRRZOut.h,\
                _lcs_cfg.bLCS_Bypass,_lcs_cfg._LCS_REG.LCS_ST,_lcs_cfg._LCS_REG.LCS_AWS,input.magic);
        if(_lcs_cfg.bLCS_Bypass == 0)
        {
            if(_lcs_cfg.bLCS_EN){
                CAM_WRITE_REG(this->m_pDrv, CAM_LCS25_ST, _lcs_cfg._LCS_REG.LCS_ST );
                CAM_WRITE_REG(this->m_pDrv, CAM_LCS25_AWS, _lcs_cfg._LCS_REG.LCS_AWS );
                CAM_WRITE_REG(this->m_pDrv, CAM_LCS25_LRZR_1, _lcs_cfg._LCS_REG.LCS_LRZR_1 );
                CAM_WRITE_REG(this->m_pDrv, CAM_LCS25_LRZR_2, _lcs_cfg._LCS_REG.LCS_LRZR_2 );
                CAM_WRITE_REG(this->m_pDrv, CAM_LCS25_CON, _lcs_cfg._LCS_REG.LCS_CON );
                CAM_WRITE_REG(this->m_pDrv, CAM_LCS25_FLR, _lcs_cfg._LCS_REG.LCS_FLR );
                CAM_WRITE_REG(this->m_pDrv, CAM_LCS25_SATU_1,_lcs_cfg._LCS_REG.LCS_SATU1 );
                CAM_WRITE_REG(this->m_pDrv, CAM_LCS25_SATU_2,_lcs_cfg._LCS_REG.LCS_SATU2 );
                CAM_WRITE_REG(this->m_pDrv, CAM_LCS25_GAIN_1, _lcs_cfg._LCS_REG.LCS_GAIN_1 );
                CAM_WRITE_REG(this->m_pDrv, CAM_LCS25_GAIN_2, _lcs_cfg._LCS_REG.LCS_GAIN_2 );
                CAM_WRITE_REG(this->m_pDrv, CAM_LCS25_OFST_1,_lcs_cfg._LCS_REG.LCS_OFST_1 );
                CAM_WRITE_REG(this->m_pDrv, CAM_LCS25_OFST_2,_lcs_cfg._LCS_REG.LCS_OFST_2 );
                CAM_WRITE_REG(this->m_pDrv, CAM_LCS25_G2G_CNV_1,_lcs_cfg._LCS_REG.LCS_G2G_CNV_1 );
                CAM_WRITE_REG(this->m_pDrv, CAM_LCS25_G2G_CNV_2,_lcs_cfg._LCS_REG.LCS_G2G_CNV_2 );
                CAM_WRITE_REG(this->m_pDrv, CAM_LCS25_G2G_CNV_3,_lcs_cfg._LCS_REG.LCS_G2G_CNV_3 );
                CAM_WRITE_REG(this->m_pDrv, CAM_LCS25_G2G_CNV_4,_lcs_cfg._LCS_REG.LCS_G2G_CNV_4 );
                CAM_WRITE_REG(this->m_pDrv, CAM_LCS25_G2G_CNV_5,_lcs_cfg._LCS_REG.LCS_G2G_CNV_5 );
                CAM_WRITE_REG(this->m_pDrv, CAM_LCS25_LPF,_lcs_cfg._LCS_REG.LCS_LPF );

                CAM_WRITE_BITS(this->m_pDrv, CAM_CTL_EN, LCS_EN, 1);


                lcso.m_pIspDrv = this->m_pIspDrv;
                lcso.dma_cfg.size.xsize = CAM_READ_BITS(this->m_pDrv,CAM_LCS25_CON,LCS25_OUT_WD)*2;
                lcso.dma_cfg.size.h = CAM_READ_BITS(this->m_pDrv,CAM_LCS25_CON,LCS25_OUT_HT);
                lcso.dma_cfg.size.stride = lcso.dma_cfg.size.xsize;
                lcso.config();
                lcso.write2CQ();
                lcso.enable(NULL);


                this->m_pDrv->cqApbModule(CAM_ISP_LCS_);
            }
            else{
                CAM_FUNC_ERR("lcs must be opened if notification is registered\n");
                ret = MFALSE;
            }

        }
        else
            this->m_pDrv->cqNopModule(CAM_ISP_LCS_);
    }
    return ret;

}

MBOOL CAM_YUV_PIPE::setSGG2(void)
{
    MBOOL ret = MTRUE;
    this->m_pDrv = (ISP_DRV_CAM*)this->m_pIspDrv;
    if(this->m_pP1Tuning){
        SGG2_REG_CFG _sgg2_cfg;


        CAM_FUNC_DBG("CAM_YUV_PIPE::setSGG2:\n");
        this->m_pP1Tuning->p1TuningNotify(NULL,(MVOID*)&_sgg2_cfg);
        CAM_FUNC_DBG("-\n");

        if(_sgg2_cfg.bSGG2_Bypass == 0){
            UNI_WRITE_REG(this->m_pDrv,CAM_UNI_SGG2_A_PGN,_sgg2_cfg._SGG2_REG.PGN);
            UNI_WRITE_REG(this->m_pDrv,CAM_UNI_SGG2_A_GMRC_1,_sgg2_cfg._SGG2_REG.GMRC_1);
            UNI_WRITE_REG(this->m_pDrv,CAM_UNI_SGG2_A_GMRC_2,_sgg2_cfg._SGG2_REG.GMRC_2);

            this->m_pDrv->cqApbModule(UNI_ISP_SGG2_A_);
        }
        else{
            this->m_pDrv->cqNopModule(UNI_ISP_SGG2_A_);
        }
    }
    return ret;
}


MUINT32 CAM_YUV_PIPE::getCurPixMode(_isp_dma_enum_ dmao)
{
    this->m_pDrv = (ISP_DRV_CAM*)this->m_pIspDrv;
    switch(dmao){
        case _imgo_:
            if(CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_TG_SEN_MODE,DBL_DATA_BUS1) == 1)
                return _4_pix_;
            else if(CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_TG_SEN_MODE,DBL_DATA_BUS) == 1)
                return _2_pix_;
            else
                return _1_pix_;
            break;
            break;
        default:
            CAM_FUNC_ERR("unsupported dmao:0x%x,return unknown-pix\n",dmao);
            return _unkonw_pix;
            break;
    }
}
