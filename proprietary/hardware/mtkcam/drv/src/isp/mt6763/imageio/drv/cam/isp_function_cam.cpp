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
#include "Cam_Notify_datatype.h"
#include "tuning_drv.h"
#include "cam_capibility.h"

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

#define CAM_CB_DURATION                      (1)
#define CAM_CB_WRITE_REG(pReg,RegName,Value) ({((cam_reg_t*)pReg)->RegName.Raw = (Value);})
#define CAM_CB_READ_REG(pReg,RegName)        ({((cam_reg_t*)pReg)->RegName.Raw;})
#define CAM_CB_DURATION_US                   (3000)

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
    capibility CamInfo;
    tCAM_rst rst;
    this->m_pDrv = (ISP_DRV_CAM*)this->m_pIspDrv;
    NSCam::NSIoPipe::NSCamIOPipe::Normalpipe_PIXMODE pixMode = NSCam::NSIoPipe::NSCamIOPipe::_UNKNOWN_PIX_MODE;

    //dbg log
    this->m_pDrv->getCurObjInfo(&this->m_hwModule,&cq,&page);
    if (page == 0) { /*reduce log, only show page 0*/
        CAM_FUNC_INF("CAM_TG_CTRL::_config +:xcropping:(0x%x_0x%x_0x%lx_0x%lx),is_continuous(0x%x),SubSample(0x%x),pixmode(0x%x),data-pattern(0x%x)\n", \
            this->m_Crop.x, \
            this->m_Crop.y, \
            this->m_Crop.w, \
            this->m_Crop.h,  \
            this->m_continuous,  \
            this->m_SubSample,\
            this->m_PixMode,\
            this->m_Datapat);
    }

    switch(this->m_PixMode) {
        case _1_pix_:
            pixMode = NSCam::NSIoPipe::NSCamIOPipe::_1_PIX_MODE;
            break;
        case _2_pix_:
            pixMode = NSCam::NSIoPipe::NSCamIOPipe::_2_PIX_MODE;
            break;
        case _4_pix_:
            pixMode = NSCam::NSIoPipe::NSCamIOPipe::_4_PIX_MODE;
            break;
        default:
            CAM_FUNC_ERR("unsupported pix mode:%d\n", this->m_PixMode);
            break;
    }

    CamInfo.GetCapibility(0, NSCam::NSIoPipe::NSCamIOPipe::ENPipeQueryCmd_UNKNOWN, NSCam::NSIoPipe::NSCamIOPipe::NormalPipe_InputInfo(\
                                         (NSCam::EImageFormat)0, 0, pixMode), rst, E_CAM_pipeline_min_size);

    if((this->m_Crop.w - this->m_Crop.x) < rst.pipelineMinSize.w)
        CAM_FUNC_ERR("TG size is smaller than pipeline min width size(%d)", rst.pipelineMinSize.w);

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
#if TEMP_SW_TIMESTAMP
    CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),CAM_TG_SEN_MODE,TIME_STP_EN,0);
#else
    CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),CAM_TG_SEN_MODE,TIME_STP_EN,1);
#endif




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
        case _tg_yuv_:
            this->m_Crop.w *= 2;
            break;
        case _tg_quad_code_:
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
    cam_top_ctl.bVBN = 0;
    DBG_LOG_CONFIG(imageio, func_cam);
}

MINT32 CAM_TOP_CTRL::_config( void )
{
    char value[PROPERTY_VALUE_MAX] = {'\0'};
    MUINT32 _loglevel = 0, i = 0;
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
    property_get("vendor.debug.isp", value, "0");
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
    while((CAM_READ_REG(this->m_pDrv->getPhyObj(),CAM_CTL_SW_CTL) != 0x2) && i++ < 10000){
        CAM_FUNC_INF("TOP reseting...CAM_CTL_SW_CTL:0x%x\n", CAM_READ_REG(this->m_pDrv->getPhyObj(),CAM_CTL_SW_CTL));
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
    CAM_WRITE_REG(this->m_pDrv,CAM_CTL_RAW_INT_EN,this->cam_top_ctl.INTE_EN.Raw);

    //dma inte en
    CAM_WRITE_REG(this->m_pDrv->getPhyObj(),CAM_CTL_RAW_INT2_EN,this->cam_top_ctl.DMA_INTE_EN.Raw);
    CAM_WRITE_REG(this->m_pDrv,CAM_CTL_RAW_INT2_EN,this->cam_top_ctl.DMA_INTE_EN.Raw);

    //db_en
    CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),CAM_CTL_MISC,DB_EN,1);

    //reset FH
    CAM_WRITE_REG(this->m_pDrv->getPhyObj(),CAM_DMA_FRAME_HEADER_EN,0);
    CAM_WRITE_REG(this->m_pDrv,CAM_DMA_FRAME_HEADER_EN,0);

    //reset dma en
    CAM_WRITE_REG(this->m_pDrv->getPhyObj(),CAM_CTL_DMA_EN,0);
    CAM_WRITE_REG(this->m_pDrv,CAM_CTL_DMA_EN,0);

    CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),CAM_CTL_DMA_EN,VBN_EN,this->cam_top_ctl.bVBN);
    CAM_WRITE_BITS(this->m_pDrv,CAM_CTL_DMA_EN,VBN_EN,this->cam_top_ctl.bVBN);

    //subsample p1 done
    if(this->SubSample){
        CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),CAM_CTL_SW_PASS1_DONE,DOWN_SAMPLE_EN,1);
        CAM_WRITE_BITS(this->m_pDrv,CAM_CTL_SW_PASS1_DONE,DOWN_SAMPLE_EN,1);
    }
    else{
        CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),CAM_CTL_SW_PASS1_DONE,DOWN_SAMPLE_EN,0);
        CAM_WRITE_BITS(this->m_pDrv,CAM_CTL_SW_PASS1_DONE,DOWN_SAMPLE_EN,0);
    }

    CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),CAM_CTL_SW_PASS1_DONE,DOWN_SAMPLE_PERIOD,this->SubSample);
    CAM_WRITE_BITS(this->m_pDrv,CAM_CTL_SW_PASS1_DONE,DOWN_SAMPLE_PERIOD,this->SubSample);


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

    {
        UniDrvImp* pDrvUni = (UniDrvImp*)UniDrvImp::createInstance(UNI_A);
        if (pDrvUni->init("UNI_DefConfig", (static_cast<IspDrvImp*>(this->m_pDrv->getPhyObj()))) == MFALSE) {
            pDrvUni->destroyInstance();
            CAM_FUNC_ERR("UNI_A init failed\n");
            return -1;
        }
        if (UNI_READ_BITS(pDrvUni, CAM_UNI_TOP_MOD_EN, RLB_A_EN) == 0) {
            /*
             * Read RLB_A_EN bit first to prevent 2nd time write to this bit
             * casues race condition between:
             * 1) CPU read-then-write(UNI_WRITE_BITS), and 2) CQ load to this reg
             */
            CAM_FUNC_INF("UNI 1st set RLB");
            UNI_WRITE_BITS(pDrvUni, CAM_UNI_TOP_MOD_EN, RLB_A_EN, 1);
        }
        pDrvUni->uninit("UNI_DefConfig");
        pDrvUni->destroyInstance();
    }


#if TOP_DCM_SW_WORK_AROUND
        CAM_WRITE_REG(this->m_pDrv->getPhyObj(),CAM_CTL_TOP_DCM_DIS,0x1);
#endif

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

#if 0 //move to init in isp_drv.cpp
    //lock pmic for seninf, need to review this mechanism.[power issue]
    if ( MFALSE == this->m_pDrv->setDeviceInfo(_SET_WAKE_LOCK,(MUINT8*)& wakelock)) {
        CAM_FUNC_ERR("ISPWakeLockCtrl enable fail!!");
    }
#endif


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

        if(this->m_hwModule == ((UNI_READ_BITS(pUni, CAM_UNI_TOP_PATH_SEL, HDS2_A_SEL) == 0) ? CAM_A : CAM_B)) {
            //eiso
            _tmp = UNI_READ_BITS(pUni,CAM_UNI_FBC_EISO_A_CTL1,FBC_EN);
            CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),CAM_CTL_SW_PASS1_DONE,EISO_FIFO_FULL_DROP,_tmp);

            //rsso
            _tmp = UNI_READ_BITS(pUni,CAM_UNI_FBC_RSSO_A_CTL1,FBC_EN);
            CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),CAM_CTL_SW_PASS1_DONE,RSSO_FIFO_FULL_DROP,_tmp);

            CAM_FUNC_INF("uni is linked with cur module:0x%x\n", this->m_hwModule);
        }
        #endif
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
            done_sel |= ((dma_en & PSO_EN_)?(PSO_A_):(0));
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
            done_sel |= ((dma_en & PSO_EN_)?(PSO_B_):(0));
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
#if (TEMP_SW_TIMESTAMP == 0)
                {
                    CAM_TIMESTAMP* pTime = CAM_TIMESTAMP::getInstance(this->m_hwModule,this->m_pDrv);
                    pTime->TimeStamp_reset();
                }
#endif
                //
                this->m_pDrv->setDeviceInfo(_SET_VF_ON,(MUINT8*)&this->m_hwModule);


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
    MUINT32 bTwin, i = 0;
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
                //CMOS_EN is changed immediately. No need to wait for one vsync.
                CAM_WRITE_BITS(this->m_pDrv->getPhyObj(), CAM_TG_SEN_MODE, CMOS_EN, 0);
            }

            //free ion handle by hw module
            this->m_pDrv->setDeviceInfo(_SET_ION_FREE_BY_HWMODULE,(MUINT8*)&this->m_hwModule);
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
    while((CAM_READ_REG(this->m_pDrv->getPhyObj(),CAM_CTL_SW_CTL) != 0x2) && i++ < 10000){
        CAM_FUNC_INF("TOP reseting...CAM_CTL_SW_CTL:0x%x\n", CAM_READ_REG(this->m_pDrv->getPhyObj(),CAM_CTL_SW_CTL));
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

                this->m_pDrv->setDeviceInfo(_SET_VF_OFF,(MUINT8*)&this->m_hwModule);
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
    CAM_WRITE_REG(this->m_pDrv->getPhyObj(),CAM_FBC_UFEO_CTL1,0x0);

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
#if 0 //move to init in isp_drv.cpp
    if ( MFALSE == this->m_pDrv->setDeviceInfo(_SET_WAKE_LOCK,(MUINT8*)& wakelock)) {
        CAM_FUNC_ERR("ISPWakeLockCtrl disable fail!!");
    }
#endif

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
    #if (P1_STANDBY_DTWIN_SUPPORT == 1)
    this->m_pDrv->cqAddModule(CAM_CTL_INT_EN_);
    this->m_pDrv->cqAddModule(CAM_CTL_INT2_EN_);
    this->m_pDrv->cqAddModule(CAM_CTL_SW_DONE_SEL_);
    #endif
    //
    CAM_FUNC_DBG("-\n");

    return 0;
}


//set 0:stop pipeline. set 1:restart pipeline
MBOOL CAM_TOP_CTRL::HW_recover(MUINT32 step)
{
    switch(step){
        case 0:
            {
                MBOOL rst = MTRUE;
                MUINT32 i=0 , flags[2], page;
                E_ISP_CAM_CQ cq;

                this->m_pDrv = (ISP_DRV_CAM*)this->m_pIspDrv;
                this->m_pDrv->getCurObjInfo(&this->m_hwModule,&cq,&page);

                CAM_FUNC_INF("CAM_TOP_CTRL::stop & reset + \n");

                IspFunction_B::fbcPhyLock_IMGO[this->m_hwModule].lock();
                IspFunction_B::fbcPhyLock_RRZO[this->m_hwModule].lock();
                IspFunction_B::fbcPhyLock_UFEO[this->m_hwModule].lock();
                IspFunction_B::fbcPhyLock_AFO[this->m_hwModule].lock();
                IspFunction_B::fbcPhyLock_AAO[this->m_hwModule].lock();
                IspFunction_B::fbcPhyLock_LCSO[this->m_hwModule].lock();
                IspFunction_B::fbcPhyLock_PDO[this->m_hwModule].lock();
                IspFunction_B::fbcPhyLock_PSO[this->m_hwModule].lock();

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

                if(CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_FBC_PSO_CTL1,FBC_EN)){
                    this->fbc_PSO.Raw = CAM_READ_REG(this->m_pDrv->getPhyObj(),CAM_FBC_PSO_CTL2);
                }

                //close vf
                this->m_pDrv->setDeviceInfo(_SET_VF_OFF,(MUINT8*)&this->m_hwModule);


                CAM_FUNC_INF("TOP reset\n");
                CAM_WRITE_REG(this->m_pDrv->getPhyObj(),CAM_CTL_SW_CTL,0x0);
                CAM_WRITE_REG(this->m_pDrv->getPhyObj(),CAM_CTL_SW_CTL,0x1);
                while((CAM_READ_REG(this->m_pDrv->getPhyObj(),CAM_CTL_SW_CTL) != 0x2) && i++ < 10000){
                    CAM_FUNC_INF("TOP reseting...CAM_CTL_SW_CTL:0x%x\n", CAM_READ_REG(this->m_pDrv->getPhyObj(),CAM_CTL_SW_CTL));
                }
                CAM_WRITE_REG(this->m_pDrv->getPhyObj(),CAM_CTL_SW_CTL,0x4);
                CAM_WRITE_REG(this->m_pDrv->getPhyObj(),CAM_CTL_SW_CTL,0x0);


                CAM_FUNC_DBG("CAM_TOP_CTRL::- \n");

                return rst;
            }
            break;
        case 1:
            {
                MUINT32 frm_cnt = 0;

                CAM_FUNC_INF("CAM_TOP_CTRL::restart + \n");
                CAM_FUNC_INF("CAM_TOP_CTRL::restart SAVE_FBC:IMGO_0x%08x RRZO_0x%08x UFEO_0x%08x AFO_0x%08x AAO_0x%08x LCSO_0x%08x PDO_0x%08x PSO_0x%08x",
                        this->fbc_IMGO.Raw, this->fbc_RRZO.Raw, this->fbc_UFEO.Raw, this->fbc_AFO.Raw,
                        this->fbc_AAO.Raw, this->fbc_LCSO.Raw, this->fbc_PDO.Raw, this->fbc_PSO.Raw);

                //keep fbc cnt
                if(CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_FBC_IMGO_CTL1,FBC_EN)){
                    for (frm_cnt = 0; frm_cnt < this->fbc_IMGO.Bits.FBC_CNT; frm_cnt++) {
                        CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),CAM_CTL_FBC_RCNT_INC, IMGO_RCNT_INC, 1);
                    }
                }

                if(CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_FBC_RRZO_CTL1,FBC_EN)){
                    for (frm_cnt = 0; frm_cnt < this->fbc_RRZO.Bits.FBC_CNT; frm_cnt++) {
                        CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),CAM_CTL_FBC_RCNT_INC, RRZO_RCNT_INC, 1);
                    }
                }

                if(CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_FBC_UFEO_CTL1,FBC_EN)){
                    for (frm_cnt = 0; frm_cnt < this->fbc_UFEO.Bits.FBC_CNT; frm_cnt++) {
                        CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),CAM_CTL_FBC_RCNT_INC, UFEO_RCNT_INC, 1);
                    }
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
                }

                if(CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_FBC_AAO_CTL1,FBC_EN)){
                    for (frm_cnt = 0; frm_cnt < this->fbc_AAO.Bits.FBC_CNT; frm_cnt++) {
                        CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),CAM_CTL_FBC_RCNT_INC, AAO_RCNT_INC, 1);
                    }
                }

                if(CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_FBC_LCSO_CTL1,FBC_EN)){
                    for (frm_cnt = 0; frm_cnt < this->fbc_LCSO.Bits.FBC_CNT; frm_cnt++) {
                        CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),CAM_CTL_FBC_RCNT_INC, LCSO_RCNT_INC, 1);
                    }
                }

                if(CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_FBC_PDO_CTL1,FBC_EN)){
                    for (frm_cnt = 0; frm_cnt < this->fbc_PDO.Bits.FBC_CNT; frm_cnt++) {
                        CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),CAM_CTL_FBC_RCNT_INC, PDO_A_RCNT_INC, 1);
                    }
                }

                if(CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_FBC_PSO_CTL1,FBC_EN)){
                    for (frm_cnt = 0; frm_cnt < this->fbc_PSO.Bits.FBC_CNT; frm_cnt++) {
                        CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),CAM_CTL_FBC_RCNT_INC, PSO_RCNT_INC, 1);
                    }
                }

                IspFunction_B::fbcPhyLock_IMGO[this->m_hwModule].unlock();
                IspFunction_B::fbcPhyLock_RRZO[this->m_hwModule].unlock();
                IspFunction_B::fbcPhyLock_UFEO[this->m_hwModule].unlock();
                IspFunction_B::fbcPhyLock_AFO[this->m_hwModule].unlock();
                IspFunction_B::fbcPhyLock_AAO[this->m_hwModule].unlock();
                IspFunction_B::fbcPhyLock_LCSO[this->m_hwModule].unlock();
                IspFunction_B::fbcPhyLock_PDO[this->m_hwModule].unlock();
                IspFunction_B::fbcPhyLock_PSO[this->m_hwModule].unlock();

                CAM_FUNC_INF("CAM_TOP_CTRL::restart PHYS_FBC:IMGO_0x%08x RRZO_0x%08x UFEO_0x%08x AFO_0x%08x AAO_0x%08x LCSO_0x%08x PDO_0x%08x PSO_0x%08x",
                        CAM_READ_REG(this->m_pDrv->getPhyObj(), CAM_FBC_IMGO_CTL2),
                        CAM_READ_REG(this->m_pDrv->getPhyObj(), CAM_FBC_RRZO_CTL2),
                        CAM_READ_REG(this->m_pDrv->getPhyObj(), CAM_FBC_UFEO_CTL2),
                        CAM_READ_REG(this->m_pDrv->getPhyObj(), CAM_FBC_AFO_CTL2),
                        CAM_READ_REG(this->m_pDrv->getPhyObj(), CAM_FBC_AAO_CTL2),
                        CAM_READ_REG(this->m_pDrv->getPhyObj(), CAM_FBC_LCSO_CTL2),
                        CAM_READ_REG(this->m_pDrv->getPhyObj(), CAM_FBC_PDO_CTL2),
                        CAM_READ_REG(this->m_pDrv->getPhyObj(), CAM_FBC_PSO_CTL2));

                /* restore FBC FIFO_FULL_DROP */
                //no need by CVD CAM_WRITE_REG(this->m_pDrv->getPhyObj(), CAM_CTL_SW_PASS1_DONE, this->fbcSwP1DoneCon.Raw);
#if (TEMP_SW_TIMESTAMP == 0)
                {
                    CAM_TIMESTAMP* pTime = CAM_TIMESTAMP::getInstance(this->m_hwModule,this->m_pDrv);
                    pTime->TimeStamp_reset();
                }
#endif
                //vf
                this->m_pDrv->setDeviceInfo(_SET_VF_ON,(MUINT8*)&this->m_hwModule);

                CAM_FUNC_DBG("CAM_TOP_CTRL::- \n");
                return MTRUE;
            }
            break;
        default:
            CAM_FUNC_ERR("unsupported:%d\n",step);
            return MFALSE;
            break;
    }
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
    Header_PSO fh_pso;

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
    fh_pso.Header_Enque(Header_PSO::E_Magic,this->m_pDrv,this->m_nMagic);


    //UNI
    fh_eiso.Header_Enque(Header_EISO::E_Magic,this->m_pDrv,this->m_nMagic);
    fh_flko.Header_Enque(Header_FLKO::E_Magic,this->m_pDrv,this->m_nMagic);
    fh_rsso.Header_Enque(Header_RSSO::E_Magic,this->m_pDrv,this->m_nMagic);


    return 0;
}



MINT32 CAM_RRZ_CTRL::_config(void)
{
    MINT32 ret = 0;
    IspSize rrz_in_size;
    IspRect rrz_in_roi_with_bin;
    MUINT32 rrz_h_step;
    MUINT32 rrz_v_step;
    MUINT32 rrz_ctl = 0;
    MUINT32 dbn_en, bin_en, vbn_en;

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
    vbn_en = CAM_READ_BITS(this->m_pDrv,CAM_CTL_DMA_EN,VBN_EN);
    bin_en = CAM_READ_BITS(this->m_pDrv,CAM_CTL_EN,BIN_EN);
    rrz_in_size.w >>= (dbn_en + bin_en);
    rrz_in_size.h >>= (vbn_en + bin_en);
    rrz_in_roi_with_bin.x = this->rrz_in_roi.x;
    rrz_in_roi_with_bin.y = this->rrz_in_roi.y;
    rrz_in_roi_with_bin.floatX = this->rrz_in_roi.floatX;
    rrz_in_roi_with_bin.floatY = this->rrz_in_roi.floatY;
    rrz_in_roi_with_bin.w = this->rrz_in_roi.w;
    rrz_in_roi_with_bin.h = this->rrz_in_roi.h;


    rrz_h_step = this->calCoefStep(rrz_in_size.w,rrz_in_roi_with_bin.w,rrz_in_roi_with_bin.x,rrz_in_roi_with_bin.floatX,this->rrz_out_size.w);
    rrz_v_step = this->calCoefStep(rrz_in_size.h,rrz_in_roi_with_bin.h,rrz_in_roi_with_bin.y,rrz_in_roi_with_bin.floatY,this->rrz_out_size.h);
    rrz_ctl = (this->getCoefTbl(rrz_in_roi_with_bin.w,this->rrz_out_size.w)<<16) | \
          (this->getCoefTbl(rrz_in_roi_with_bin.h,this->rrz_out_size.h)<<24) | \
          0x03; //enable

    if(rrz_ctl == 0){
        ret = 1;
    }

    CAM_FUNC_INF("CAM_RRZ_CTRL::_config+ cq:0x%x,page:0x%x,rrz_in_w/h(%ld/%ld),rrz_crop_x/y/fx/fy/w/h(%d/%d/%d/%d/%ld/%ld),rrz_out_w/h(%ld/%ld),rlb_offset(%d),rrz_h/v_step(%d/%d),vbn/dbn/bin(%d/%d/%d)\n",cq,page,\
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
            this->rrz_rlb_offset, \
            rrz_h_step, rrz_v_step,
            vbn_en, dbn_en, bin_en);

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
    CAM_WRITE_BITS(this->m_pDrv,CAM_RRZ_RLB_AOFST,RRZ_RLB_AOFST,this->rrz_rlb_offset);

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
       >=1.0            31
       0.99~0.9          0~3
       0.9~0.8           4~7
       0.8~0.7           8~11
       0.7~0.6          12~15
       0.6~0.5          16~19
       0.5~0.4          20~23
       0.4~0.3          24~27
       0.3~0.25         28~30
     */

    struct {
        MUINT32 left;
        MUINT32 right;
    }map_tbl[10] = {{1,1},{1,1},{28,30},{24,27},{20,23},{16,19},{12,15},{8,11},{4,7},{0,3}};

    MUINT32 scale_ratio_int = (MUINT32)( (resize_out*10) / resize_in );
    MUINT32 scale_ratio_int_100 = (MUINT32)( (resize_out*100) / resize_in );
    MUINT32 ref = (scale_ratio_int_100 < 30) ? 25 : scale_ratio_int*10;
    MUINT32 div = (scale_ratio_int_100 < 30) ? 5 : 10;
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
        /* +1: for filtering averagely each interval*/
        table = map_tbl[scale_ratio_int].right - \
              ( ( scale_ratio_int_100 - ref )* \
                ( map_tbl[scale_ratio_int].right - map_tbl[scale_ratio_int].left + 1 ) ) / div;
        table = (table > 0x1f)? (0x1f):(table);
    }

    CAM_FUNC_DBG("table(%d)",table);

    return table;
}

MINT32 CAM_XMX_CTRL::config(CAM_RAW_PIPE* _this)
{
    MINT32 ret = 0;
    this->m_pRaw_p = _this;
    this->m_hwModule = _this->m_hwModule;

    ret = this->_config();

    return ret;
}

MINT32 CAM_RCP_CTRL::_config(void)
{
    MINT32 ret = 0;
    MUINT32 tmp;

    CAM_WRITE_BITS(this->m_pRaw_p->m_pDrv,CAM_CTL_EN,RCP_EN, 1);


    //rcp crp
    tmp = CAM_READ_BITS(this->m_pRaw_p->m_pDrv,CAM_DMX_CROP,DMX_END_X) - CAM_READ_BITS(this->m_pRaw_p->m_pDrv,CAM_DMX_CROP,DMX_STR_X) + 1;
    tmp = tmp >> (CAM_READ_BITS(this->m_pRaw_p->m_pDrv,CAM_CTL_EN,BIN_EN) + CAM_READ_BITS(this->m_pRaw_p->m_pDrv,CAM_CTL_EN,DBN_EN));
    CAM_WRITE_BITS(this->m_pRaw_p->m_pDrv,CAM_RCP_CROP_CON1,RCP_STR_X,0);
    CAM_WRITE_BITS(this->m_pRaw_p->m_pDrv,CAM_RCP_CROP_CON1,RCP_END_X,tmp - 1);

    tmp = CAM_READ_BITS(this->m_pRaw_p->m_pDrv,CAM_DMX_VSIZE,DMX_HT);
    tmp = tmp >> (CAM_READ_BITS(this->m_pRaw_p->m_pDrv,CAM_CTL_EN,BIN_EN) + CAM_READ_BITS(this->m_pRaw_p->m_pDrv,CAM_CTL_DMA_EN,VBN_EN));
    CAM_WRITE_BITS(this->m_pRaw_p->m_pDrv,CAM_RCP_CROP_CON2,RCP_STR_Y,0);
    CAM_WRITE_BITS(this->m_pRaw_p->m_pDrv,CAM_RCP_CROP_CON2,RCP_END_Y,tmp - 1);

    return ret;
}

//unlike other module, RCP3's counting unit is CLK, not PIX.
MINT32 CAM_RCP3_CTRL::_config()
{
    MINT32 ret = 0;
    MUINT32 src_size;
    MUINT32 h_bin_status = 0;
    MUINT32 v_bin_status = 0;
    E_PIX_MODE pix_mode = _1_pix_;

    CAM_WRITE_BITS(this->m_pRaw_p->m_pDrv,CAM_CTL_EN,RCP3_EN, 1);

    //rcp3 crp
    switch(CAM_READ_BITS(this->m_pRaw_p->m_pDrv,CAM_CTL_SEL,RCP3_SEL))
    {
        case E_rcp3_0:
        case E_rcp3_1:
        case E_rcp3_3:
            h_bin_status = (CAM_READ_BITS(this->m_pRaw_p->m_pDrv,CAM_CTL_EN,BIN_EN) + CAM_READ_BITS(this->m_pRaw_p->m_pDrv,CAM_CTL_EN,DBN_EN));
            v_bin_status = (CAM_READ_BITS(this->m_pRaw_p->m_pDrv,CAM_CTL_EN,BIN_EN) + CAM_READ_BITS(this->m_pRaw_p->m_pDrv,CAM_CTL_DMA_EN,VBN_EN));
            pix_mode = _1_pix_;
            break;
        case E_rcp3_2:
            h_bin_status = CAM_READ_BITS(this->m_pRaw_p->m_pDrv,CAM_CTL_EN,DBN_EN);
            v_bin_status = CAM_READ_BITS(this->m_pRaw_p->m_pDrv,CAM_CTL_DMA_EN,VBN_EN);
            pix_mode = (E_PIX_MODE)(CAM_READ_BITS(this->m_pRaw_p->m_pDrv,CAM_CTL_FMT_SEL,PIX_BUS_DMXO) >> \
                CAM_READ_BITS(this->m_pRaw_p->m_pDrv,CAM_CTL_EN,DBN_EN));
            break;
        default:
            CAM_FUNC_ERR("unsupported sel(%d), rcp3 fail\n",CAM_READ_BITS(this->m_pRaw_p->m_pDrv,CAM_CTL_SEL,RCP3_SEL));
            ret = 1;
            goto EXIT;
            break;

    }

    src_size = CAM_READ_BITS(this->m_pRaw_p->m_pDrv,CAM_DMX_CROP,DMX_END_X) - CAM_READ_BITS(this->m_pRaw_p->m_pDrv,CAM_DMX_CROP,DMX_STR_X) + 1;
    src_size = (src_size >> (h_bin_status + (MUINT32)pix_mode));//in case of rcp3_2 + bin_on
    CAM_WRITE_BITS(this->m_pRaw_p->m_pDrv,CAM_RCP3_CROP_CON1,RCP_STR_X,0);
    CAM_WRITE_BITS(this->m_pRaw_p->m_pDrv,CAM_RCP3_CROP_CON1,RCP_END_X,src_size - 1);

    src_size = CAM_READ_BITS(this->m_pRaw_p->m_pDrv,CAM_DMX_VSIZE,DMX_HT);
    src_size = (src_size >> v_bin_status);
    CAM_WRITE_BITS(this->m_pRaw_p->m_pDrv,CAM_RCP3_CROP_CON2,RCP_STR_Y,0);
    CAM_WRITE_BITS(this->m_pRaw_p->m_pDrv,CAM_RCP3_CROP_CON2,RCP_END_Y,src_size - 1);

EXIT:
    return ret;
}

MINT32 CAM_BMX_CTRL::_config(void)
{
    MINT32 ret = 0;
    MUINT32 tmp;
    capibility CamInfo;
    tCAM_rst rst;
    MUINT32 scaling_ratio = 0;
    NSCam::NSIoPipe::NSCamIOPipe::Normalpipe_PIXMODE pixMode = NSCam::NSIoPipe::NSCamIOPipe::_UNKNOWN_PIX_MODE;

    switch(this->m_pRaw_p->m_xmxo.pix_mode_dmxi) {
        case _1_pix_:
            pixMode = NSCam::NSIoPipe::NSCamIOPipe::_1_PIX_MODE;
            break;
        case _2_pix_:
            pixMode = NSCam::NSIoPipe::NSCamIOPipe::_2_PIX_MODE;
            break;
        case _4_pix_:
            pixMode = NSCam::NSIoPipe::NSCamIOPipe::_4_PIX_MODE;
            break;
        default:
            CAM_FUNC_ERR("unsupported pix mode:%d\n", this->m_pRaw_p->m_xmxo.pix_mode_dmxi);
            break;
    }

    CamInfo.GetCapibility(0,NSCam::NSIoPipe::NSCamIOPipe::ENPipeQueryCmd_UNKNOWN,NSCam::NSIoPipe::NSCamIOPipe::NormalPipe_InputInfo(),rst,E_CAM_SRAM_BMX);
    CamInfo.GetCapibility(0, NSCam::NSIoPipe::NSCamIOPipe::ENPipeQueryCmd_UNKNOWN, NSCam::NSIoPipe::NSCamIOPipe::NormalPipe_InputInfo(\
                                          (NSCam::EImageFormat)0, 0, pixMode), rst, E_CAM_pipeline_min_size);

    CAM_WRITE_BITS(this->m_pRaw_p->m_pDrv,CAM_CTL_EN,BMX_EN, 1);
    CAM_WRITE_BITS(this->m_pRaw_p->m_pDrv,CAM_CTL_DMA_EN,QBIN4_EN, 1);

    //bmx ctrl
    CAM_WRITE_BITS(this->m_pRaw_p->m_pDrv,CAM_BMX_CTL,BMX_SRAM_SIZE, rst.xmx.sram_bmx);
    CAM_WRITE_BITS(this->m_pRaw_p->m_pDrv,CAM_BMX_CTL,BMX_EDGE, 0xf);
    CAM_WRITE_BITS(this->m_pRaw_p->m_pDrv,CAM_BMX_CTL,BMX_EDGE_SET, 0);
    CAM_WRITE_BITS(this->m_pRaw_p->m_pDrv,CAM_BMX_CTL,BMX_EDGE, 0xf);
    CAM_WRITE_BITS(this->m_pRaw_p->m_pDrv,CAM_BMX_CTL,BMX_SINGLE_MODE_1, 1);
    CAM_WRITE_BITS(this->m_pRaw_p->m_pDrv,CAM_BMX_CTL,BMX_SINGLE_MODE_2, 0);

    //qbn4
    //due to dynamic twin, qbn need to make sure the consistancy of data volume
    if ((this->m_pRaw_p->m_TG_W.Bits.PXL_E - this->m_pRaw_p->m_TG_W.Bits.PXL_S) > rst.pipelineMinSize.w) {
        scaling_ratio++; //for reduce through-put
        //condition for size of dmx after twin-path is controlled by twin_drv
    } else {
        CAM_FUNC_ERR("TG size is smaller than minWidthConstraint(%d), then Qbin4 don't shrink width", rst.pipelineMinSize.w);
    }
#if 0   //because of quality issue , remove this logic under single-path. under twin-path , QBN_ACC will be >1 only when TG_h need to > amx's sram*4.
    {//else{
        //qbn acc design loigc is changed, 0 -> no scaling, 1->2x scaling down, 2->4x scaling down
        tmp = this->m_pRaw_p->m_TG_W.Bits.PXL_E - this->m_pRaw_p->m_TG_W.Bits.PXL_S;
        tmp = tmp >> (CAM_READ_BITS(this->m_pRaw_p->m_pDrv,CAM_CTL_EN,BIN_EN) + \
        CAM_READ_BITS(this->m_pRaw_p->m_pDrv,CAM_CTL_EN,DBN_EN)+scaling_ratio);
        while(tmp>rst.xmx.line_bmx)
            tmp = ( tmp >> (++scaling_ratio) );
    }
#endif
    //W phy is for query , vir is for twin
    CAM_WRITE_BITS(this->m_pRaw_p->m_pDrv->getPhyObj(),CAM_QBN4_MODE, QBN_ACC_MODE,1);//always use average
    CAM_WRITE_BITS(this->m_pRaw_p->m_pDrv->getPhyObj(),CAM_QBN4_MODE, QBN_ACC,scaling_ratio);
    CAM_WRITE_BITS(this->m_pRaw_p->m_pDrv,CAM_QBN4_MODE, QBN_ACC_MODE,1);//always use average
    CAM_WRITE_BITS(this->m_pRaw_p->m_pDrv,CAM_QBN4_MODE, QBN_ACC,scaling_ratio);

    //bmx crp
    CAM_WRITE_BITS(this->m_pRaw_p->m_pDrv,CAM_BMX_CROP,BMX_STR_X,0);
    tmp = this->m_pRaw_p->m_TG_W.Bits.PXL_E - this->m_pRaw_p->m_TG_W.Bits.PXL_S;
    tmp = tmp >> (CAM_READ_BITS(this->m_pRaw_p->m_pDrv,CAM_CTL_EN,BIN_EN) + \
        CAM_READ_BITS(this->m_pRaw_p->m_pDrv,CAM_CTL_EN,DBN_EN) + \
        scaling_ratio);

    CAM_WRITE_BITS(this->m_pRaw_p->m_pDrv,CAM_BMX_CROP,BMX_END_X,(tmp - 1));

    tmp = this->m_pRaw_p->m_TG_H.Bits.LIN_E - this->m_pRaw_p->m_TG_H.Bits.LIN_S;
    tmp = tmp >> (CAM_READ_BITS(this->m_pRaw_p->m_pDrv,CAM_CTL_EN,BIN_EN) + CAM_READ_BITS(this->m_pRaw_p->m_pDrv,CAM_CTL_DMA_EN,VBN_EN));
    CAM_WRITE_REG(this->m_pRaw_p->m_pDrv,CAM_BMX_VSIZE,tmp);

    CAM_WRITE_BITS(this->m_pRaw_p->m_pDrv,CAM_CTL_FMT_SEL,PIX_BUS_BMXO, this->m_pRaw_p->m_xmxo.pix_mode_bmxo);
    return ret;
}

MINT32 CAM_PMX_CTRL::_config(void)
{
    MINT32 ret = 0;
    MUINT32 tmp;
    capibility CamInfo;
    tCAM_rst rst;

    CamInfo.GetCapibility(0,NSCam::NSIoPipe::NSCamIOPipe::ENPipeQueryCmd_UNKNOWN,NSCam::NSIoPipe::NSCamIOPipe::NormalPipe_InputInfo(),rst,E_CAM_SRAM_PMX);

    CAM_WRITE_BITS(this->m_pRaw_p->m_pDrv,CAM_CTL_EN,PMX_EN, 1);

    CAM_WRITE_BITS(this->m_pRaw_p->m_pDrv,CAM_PMX_CTL,PMX_SRAM_SIZE, rst.xmx.sram_pmx);
    CAM_WRITE_BITS(this->m_pRaw_p->m_pDrv,CAM_PMX_CTL,PMX_EDGE, 0xf);
    CAM_WRITE_BITS(this->m_pRaw_p->m_pDrv,CAM_PMX_CTL,PMX_EDGE_SET, 0);
    CAM_WRITE_BITS(this->m_pRaw_p->m_pDrv,CAM_PMX_CTL,PMX_EDGE, 0xf);
    CAM_WRITE_BITS(this->m_pRaw_p->m_pDrv,CAM_PMX_CTL,PMX_SINGLE_MODE_1, 1);
    CAM_WRITE_BITS(this->m_pRaw_p->m_pDrv,CAM_PMX_CTL,PMX_SINGLE_MODE_2, 0);

    if((CAM_READ_BITS(this->m_pRaw_p->m_pDrv,CAM_CTL_EN,PBN_EN) == 1) && \
            (CAM_READ_REG(this->m_pRaw_p->m_pDrv,CAM_CTL_SEL) & PMX_SEL_1)){
        tmp = 1;
        //if pmx_sel = 1 , pix mode must = 2pix
        if(this->m_pRaw_p->m_xmxo.pix_mode_dmxi != _2_pix_){
            CAM_FUNC_ERR("support only 2pix mode at this path(%d)\n",this->m_pRaw_p->m_xmxo.pix_mode_dmxi);
            return 1;
        }
    }
    else
        tmp = 0;
    CAM_WRITE_BITS(this->m_pRaw_p->m_pDrv,CAM_PMX_CTL,PMX_DUAL_PD_MODE, tmp);

    if(CAM_READ_BITS(this->m_pRaw_p->m_pDrv,CAM_CTL_SEL,PMX_SEL) == PMX_SEL_0){
        CAM_WRITE_BITS(this->m_pRaw_p->m_pDrv,CAM_PMX_CROP,PMX_STR_X,0);
        tmp = this->m_pRaw_p->m_TG_W.Bits.PXL_E - this->m_pRaw_p->m_TG_W.Bits.PXL_S;
        tmp = tmp >> (CAM_READ_BITS(this->m_pRaw_p->m_pDrv,CAM_CTL_EN,BIN_EN) + CAM_READ_BITS(this->m_pRaw_p->m_pDrv,CAM_CTL_EN,DBN_EN));
        CAM_WRITE_BITS(this->m_pRaw_p->m_pDrv,CAM_PMX_CROP,PMX_END_X,(tmp - 1));

        tmp = this->m_pRaw_p->m_TG_H.Bits.LIN_E - this->m_pRaw_p->m_TG_H.Bits.LIN_S;
        tmp = tmp >> (CAM_READ_BITS(this->m_pRaw_p->m_pDrv,CAM_CTL_EN,BIN_EN) + CAM_READ_BITS(this->m_pRaw_p->m_pDrv,CAM_CTL_DMA_EN,VBN_EN));
        CAM_WRITE_REG(this->m_pRaw_p->m_pDrv,CAM_PMX_VSIZE,tmp);
    }
    else{
        CAM_WRITE_BITS(this->m_pRaw_p->m_pDrv,CAM_PMX_CROP,PMX_STR_X,0);

        if (CAM_READ_BITS(this->m_pRaw_p->m_pDrv,CAM_CTL_EN,PBN_EN)) {
            tmp = this->m_pRaw_p->m_TG_W.Bits.PXL_E - this->m_pRaw_p->m_TG_W.Bits.PXL_S;
            tmp = PBIN_WIDTH(tmp);
            CAM_WRITE_BITS(this->m_pRaw_p->m_pDrv,CAM_PMX_CROP,PMX_END_X,(tmp - 1));

            tmp = ((this->m_pRaw_p->m_TG_H.Bits.LIN_E - this->m_pRaw_p->m_TG_H.Bits.LIN_S) >> CAM_READ_BITS(this->m_pRaw_p->m_pDrv,CAM_CTL_DMA_EN,VBN_EN));
            tmp = PBIN_HEIGHT(tmp);
            CAM_WRITE_REG(this->m_pRaw_p->m_pDrv,CAM_PMX_VSIZE,tmp);
        } else {
            tmp = this->m_pRaw_p->m_TG_W.Bits.PXL_E - this->m_pRaw_p->m_TG_W.Bits.PXL_S;
            CAM_WRITE_BITS(this->m_pRaw_p->m_pDrv,CAM_PMX_CROP,PMX_END_X,(tmp - 1));

            tmp = ((this->m_pRaw_p->m_TG_H.Bits.LIN_E - this->m_pRaw_p->m_TG_H.Bits.LIN_S) >> CAM_READ_BITS(this->m_pRaw_p->m_pDrv,CAM_CTL_DMA_EN,VBN_EN));
            CAM_WRITE_REG(this->m_pRaw_p->m_pDrv,CAM_PMX_VSIZE,tmp);
        }
    }

    return ret;
}

MINT32 CAM_DMX_CTRL::_config(void)
{
    MINT32 ret=0;
    capibility CamInfo;
    tCAM_rst rst;

    CamInfo.GetCapibility(0,NSCam::NSIoPipe::NSCamIOPipe::ENPipeQueryCmd_UNKNOWN,NSCam::NSIoPipe::NSCamIOPipe::NormalPipe_InputInfo(),rst,E_CAM_SRAM_DMX);

    CAM_WRITE_BITS(this->m_pRaw_p->m_pDrv,CAM_CTL_EN,DMX_EN, 1);

    CAM_WRITE_BITS(this->m_pRaw_p->m_pDrv,CAM_DMX_CTL,DMX_SRAM_SIZE, rst.xmx.sram_dmx);
    CAM_WRITE_BITS(this->m_pRaw_p->m_pDrv,CAM_DMX_CTL,DMX_EDGE, 0xf);
    CAM_WRITE_BITS(this->m_pRaw_p->m_pDrv,CAM_DMX_CTL,DMX_EDGE_SET, 0);

    CAM_WRITE_BITS(this->m_pRaw_p->m_pDrv,CAM_DMX_CROP,DMX_STR_X,0);
    CAM_WRITE_BITS(this->m_pRaw_p->m_pDrv,CAM_DMX_CROP,DMX_END_X,(this->m_pRaw_p->m_TG_W.Bits.PXL_E - this->m_pRaw_p->m_TG_W.Bits.PXL_S - 1));

    CAM_WRITE_REG(this->m_pRaw_p->m_pDrv,CAM_DMX_VSIZE,(this->m_pRaw_p->m_TG_H.Bits.LIN_E - this->m_pRaw_p->m_TG_H.Bits.LIN_S));

    CAM_WRITE_BITS(this->m_pRaw_p->m_pDrv,CAM_CTL_FMT_SEL,PIX_BUS_DMXO, this->m_pRaw_p->m_xmxo.pix_mode_dmxo);
    CAM_WRITE_BITS(this->m_pRaw_p->m_pDrv,CAM_CTL_FMT_SEL,PIX_BUS_DMXI, this->m_pRaw_p->m_xmxo.pix_mode_dmxi);
    return ret;
}

MINT32 CAM_RMX_CTRL::_config(void)
{
    MINT32 ret=0;
    capibility CamInfo;
    tCAM_rst rst;

    CamInfo.GetCapibility(0,NSCam::NSIoPipe::NSCamIOPipe::ENPipeQueryCmd_UNKNOWN,NSCam::NSIoPipe::NSCamIOPipe::NormalPipe_InputInfo(),rst,E_CAM_SRAM_RMX);

    CAM_WRITE_BITS(this->m_pRaw_p->m_pDrv,CAM_CTL_EN,RMX_EN, 1);

    CAM_WRITE_BITS(this->m_pRaw_p->m_pDrv,CAM_RMX_CTL,RMX_SRAM_SIZE, rst.xmx.sram_rmx);
    CAM_WRITE_BITS(this->m_pRaw_p->m_pDrv,CAM_RMX_CTL,RMX_EDGE, 0xf);
    CAM_WRITE_BITS(this->m_pRaw_p->m_pDrv,CAM_RMX_CTL,RMX_EDGE_SET, 0);
    CAM_WRITE_BITS(this->m_pRaw_p->m_pDrv,CAM_RMX_CTL,RMX_EDGE, 0xf);
    //single_mode1 & 2 are using exclusive method.   signle_mode1 is main path, signle_mode2 is from other path
    CAM_WRITE_BITS(this->m_pRaw_p->m_pDrv,CAM_RMX_CTL,RMX_SINGLE_MODE_1, 1);
    CAM_WRITE_BITS(this->m_pRaw_p->m_pDrv,CAM_RMX_CTL,RMX_SINGLE_MODE_2, 0);

    //removed, move into rrz_ctrl
    //CAM_WRITE_BITS(this->m_pRaw_p->m_pDrv,CAM_RMX_CROP,RMX_STR_X,0);
    //CAM_WRITE_BITS(this->m_pRaw_p->m_pDrv,CAM_RMX_CROP,RMX_END_X,(this->m_pRaw_p->rrz_out_size.w - 1));
    //CAM_WRITE_REG(this->m_pRaw_p->m_pDrv,CAM_RMX_VSIZE,this->m_pRaw_p->rrz_out_size.h);

    CAM_WRITE_BITS(this->m_pRaw_p->m_pDrv,CAM_CTL_FMT_SEL,PIX_BUS_RMXO, this->m_pRaw_p->m_xmxo.pix_mode_rmxo);

    if(this->m_pRaw_p->m_pUniDrv){
        UNI_WRITE_BITS(this->m_pRaw_p->m_pDrv,CAM_UNI_HDS_A_MODE,HDS_DS,this->m_pRaw_p->m_xmxo.pix_mode_rmxo);
    }
    return ret;
}

MINT32 CAM_AMX_CTRL::_config(void)
{
    MINT32 ret=0;
    capibility CamInfo;
    tCAM_rst rst;
    MUINT32 tmp;
    MUINT32 scaling_ratio = 0;
    NSCam::NSIoPipe::NSCamIOPipe::Normalpipe_PIXMODE pixMode = NSCam::NSIoPipe::NSCamIOPipe::_UNKNOWN_PIX_MODE;

    switch(this->m_pRaw_p->m_xmxo.pix_mode_dmxi) {
        case _1_pix_:
            pixMode = NSCam::NSIoPipe::NSCamIOPipe::_1_PIX_MODE;
            break;
        case _2_pix_:
            pixMode = NSCam::NSIoPipe::NSCamIOPipe::_2_PIX_MODE;
            break;
        case _4_pix_:
            pixMode = NSCam::NSIoPipe::NSCamIOPipe::_4_PIX_MODE;
            break;
        default:
            CAM_FUNC_ERR("unsupported pix mode:%d\n", this->m_pRaw_p->m_xmxo.pix_mode_dmxi);
            break;
    }

    CamInfo.GetCapibility(0,NSCam::NSIoPipe::NSCamIOPipe::ENPipeQueryCmd_UNKNOWN,NSCam::NSIoPipe::NSCamIOPipe::NormalPipe_InputInfo(),rst,E_CAM_SRAM_AMX);
    CamInfo.GetCapibility(0, NSCam::NSIoPipe::NSCamIOPipe::ENPipeQueryCmd_UNKNOWN, NSCam::NSIoPipe::NSCamIOPipe::NormalPipe_InputInfo(\
                                          (NSCam::EImageFormat)0, 0, pixMode), rst, E_CAM_pipeline_min_size);

    CAM_WRITE_BITS(this->m_pRaw_p->m_pDrv,CAM_CTL_DMA_EN,AMX_EN, 1);
    CAM_WRITE_BITS(this->m_pRaw_p->m_pDrv,CAM_CTL_EN,QBIN1_EN, 1);


    CAM_WRITE_BITS(this->m_pRaw_p->m_pDrv,CAM_AMX_CTL,AMX_SRAM_SIZE, rst.xmx.sram_amx);
    CAM_WRITE_BITS(this->m_pRaw_p->m_pDrv,CAM_AMX_CTL,AMX_EDGE, 0xf);
    CAM_WRITE_BITS(this->m_pRaw_p->m_pDrv,CAM_AMX_CTL,AMX_EDGE_SET, 0);
    CAM_WRITE_BITS(this->m_pRaw_p->m_pDrv,CAM_AMX_CTL,AMX_EDGE, 0xf);
    CAM_WRITE_BITS(this->m_pRaw_p->m_pDrv,CAM_AMX_CTL,AMX_SINGLE_MODE_1, 1);
    CAM_WRITE_BITS(this->m_pRaw_p->m_pDrv,CAM_AMX_CTL,AMX_SINGLE_MODE_2, 0);


    //qbin1
    //due to dynamic twin, qbn need to make sure the consistancy of data volume
    if ((this->m_pRaw_p->m_TG_W.Bits.PXL_E - this->m_pRaw_p->m_TG_W.Bits.PXL_S) > rst.pipelineMinSize.w) {
        scaling_ratio++; //for reduce through-put
        //condition for size of dmx after twin-path is controlled by twin_drv
    } else {
        CAM_FUNC_ERR("TG size is smaller than minWidthConstraint(%d), then Qbin1 don't shrink width", rst.pipelineMinSize.w);
    }
#if 0   //because of quality issue , remove this logic under single-path. under twin-path , QBN_ACC will be >1 only when TG_h need to > amx's sram*4.
    {//else{
        //qbn acc design loigc is changed, 0 -> no scaling, 1->2x scaling down, 2->4x scaling down
        tmp = this->m_pRaw_p->m_TG_W.Bits.PXL_E - this->m_pRaw_p->m_TG_W.Bits.PXL_S;
        tmp = tmp >> (CAM_READ_BITS(this->m_pRaw_p->m_pDrv,CAM_CTL_EN,BIN_EN) + \
        CAM_READ_BITS(this->m_pRaw_p->m_pDrv,CAM_CTL_EN,DBN_EN)+scaling_ratio);
        while(tmp>rst.xmx.line_amx)
            tmp = ( tmp >> (++scaling_ratio) );
    }
#endif
    //W phy is for query , vir is for twin
    CAM_WRITE_BITS(this->m_pRaw_p->m_pDrv,CAM_QBN1_MODE, QBN_ACC_MODE,1);//always use average
    CAM_WRITE_BITS(this->m_pRaw_p->m_pDrv->getPhyObj(),CAM_QBN1_MODE, QBN_ACC_MODE,1);//always use average
    CAM_WRITE_BITS(this->m_pRaw_p->m_pDrv,CAM_QBN1_MODE, QBN_ACC,scaling_ratio);
    CAM_WRITE_BITS(this->m_pRaw_p->m_pDrv->getPhyObj(),CAM_QBN1_MODE, QBN_ACC,scaling_ratio);

    //amx cropping
    // dbn: x-dir => /2, y-dir => same; bin: x-dir => /2, y-dir => /2
    CAM_WRITE_BITS(this->m_pRaw_p->m_pDrv,CAM_AMX_CROP,AMX_STR_X,0);
    tmp = this->m_pRaw_p->m_TG_W.Bits.PXL_E - this->m_pRaw_p->m_TG_W.Bits.PXL_S;
    tmp = tmp >> (CAM_READ_BITS(this->m_pRaw_p->m_pDrv,CAM_CTL_EN,BIN_EN) + \
        CAM_READ_BITS(this->m_pRaw_p->m_pDrv,CAM_CTL_EN,DBN_EN) + \
        scaling_ratio);
    CAM_WRITE_BITS(this->m_pRaw_p->m_pDrv,CAM_AMX_CROP,AMX_END_X,(tmp - 1));

    tmp = this->m_pRaw_p->m_TG_H.Bits.LIN_E - this->m_pRaw_p->m_TG_H.Bits.LIN_S;
    tmp = tmp >> (CAM_READ_BITS(this->m_pRaw_p->m_pDrv,CAM_CTL_EN,BIN_EN) + CAM_READ_BITS(this->m_pRaw_p->m_pDrv,CAM_CTL_DMA_EN,VBN_EN));
    CAM_WRITE_REG(this->m_pRaw_p->m_pDrv,CAM_AMX_VSIZE,tmp);

    return ret;
}

/*/////////////////////////////////////////////////////////////////////////////
  ISP_RAW_PIPE
/////////////////////////////////////////////////////////////////////////////*/
inline void CAM_RAW_PIPE::CBTimeChk(char *str, MBOOL bStartT)
{
    struct timespec ts;
    div_t divResult;
    static MUINT32 TimehourS, TimeMinS, TimeSecS, TimeUsS;
    MUINT32 Timehour, TimeMin, TimeSec, TimeUs;

    clock_gettime(CLOCK_REALTIME, &ts);
    TimeUs = ts.tv_nsec/1000;//ms
    TimeSec  = ts.tv_sec%86400;//day=60*60*24=86400 sec

    divResult = div(TimeSec, 3600);
    Timehour = divResult.quot;
    TimeSec = divResult.rem;

    divResult = div(TimeSec, 60);
    TimeMin = divResult.quot;
    TimeSec = divResult.rem;

    if (bStartT) {
        TimehourS = Timehour;
        TimeMinS = TimeMin;
        TimeSecS = TimeSec;
        TimeUsS = TimeUs;
        CAM_FUNC_DBG("%s(m:%d)+[%02d:%02d:%02d.%06d]",
                    str, this->m_dumpInput.magic, Timehour, TimeMin, TimeSec, TimeUs);
    } else {
        CAM_FUNC_DBG("%s(m:%d)-[%02d:%02d:%02d.%06d], diff[%02d:%02d:%02d.%06d]",
                     str, this->m_dumpInput.magic, Timehour, TimeMin, TimeSec, TimeUs,
                    (Timehour-TimehourS), (TimeMin-TimeMinS),
                    (TimeSec-TimeSecS), (TimeUs-TimeUsS));

        Timehour -= TimehourS;
        TimeMin  -= TimeMinS;
        TimeSec  -= TimeSecS;
        TimeUs   -= TimeUsS;
        if((Timehour+TimeMin+TimeSec) || (TimeUs > CAM_CB_DURATION_US)){
            CAM_FUNC_WRN("%s(m:%d) takes too many time: [%02d:%02d:%02d.%06d]",
                    str, this->m_dumpInput.magic, Timehour, TimeMin, TimeSec, TimeUs);
        }
    }
}

MBOOL CAM_RAW_PIPE::dumpCQReg(void)
{
    MBOOL ret = MTRUE;
    MUINT32 i, j, addrOffset, moduleSize, mode, _addr, dumpcq = 0, _dma_en = 0;
    MUINT32* pIspRegMap = NULL;
    CQ_DUMP_INFO input;
    static MUINT32 _dpFrameCnt = 0, _preDumpcq = 0;
    char value[PROPERTY_VALUE_MAX] = {'\0'};
    char _str[128] = {'\0'};

    this->m_pDrv = (ISP_DRV_CAM*)this->m_pIspDrv;
    snprintf(_str, 127, __FUNCTION__);

    if(this->m_pP1Tuning){
        /* adb control dump
          1. set debug.isp.dumpcq.framecnt > 0
          2. set debug.isp.dumpcq.enable > 0
        */
        property_get("vendor.debug.isp.dumpcq.enable", value, "0");
        dumpcq = atoi(value);

        //"dumpcq.enable" 0 ->1, update _dpFrameCnt
        if(_preDumpcq == 0 && dumpcq == 1){
            //get dump frame cnt
            property_get("vendor.debug.isp.dumpcq.framecnt", value, "0");
            _dpFrameCnt = atoi(value);
        }
        //update "dumpcq.enable" value to _preDumpcq
        _preDumpcq = dumpcq;

        if(_dpFrameCnt == 0)
            return MTRUE;

        CAM_FUNC_INF("DumpCQ %d frames\n", _dpFrameCnt);
        _dpFrameCnt--;

        //allocate
        if(this->m_dumpInput.pReg == NULL){//first dump -> allocate
            pIspRegMap = (MUINT32*)calloc(this->m_dumpInput.size, sizeof(MUINT32));
            if(pIspRegMap == NULL) {
                CAM_FUNC_ERR("allocate mem for dump fail\n");
                return MFALSE;
            }
            this->m_dumpInput.pReg = (void *)pIspRegMap;
        }

        _dma_en = CAM_READ_REG(this->m_pDrv,CAM_CTL_DMA_EN);

        if (_dma_en & RRZO_EN_) {
           Header_RRZO fh_rrzo;
           m_dumpInput.magic = fh_rrzo.GetRegInfo(Header_RRZO::E_Magic, this->m_pDrv);
        } else if (_dma_en & IMGO_EN_) {
           m_dumpInput.magic = CAM_READ_REG(this->m_pDrv,CAM_IMGO_FH_SPARE_2);//Header_IMGO::E_Magic
        } else {
           CAM_FUNC_ERR("Can't get magic#. dma_en(x%x)\n", _dma_en);
        }

        for( i= 0;i < CAM_NEXT_Thread_;i++){
            ret = this->m_pDrv->readCQModuleInfo(i, &this->m_dumpInput.pReg, this->m_dumpInput.size);
            if (ret == MFALSE){
                CAM_FUNC_ERR("module(%d),getCqInfo fail\n",i);
                break;
            }
        }

        CBTimeChk(_str, MTRUE);
        this->m_pP1Tuning->p1TuningNotify((MVOID*)&this->m_dumpInput, NULL);
        CBTimeChk(_str, MFALSE);

        //free
        if (!_dpFrameCnt && this->m_dumpInput.pReg){//last frame -> free
            free(this->m_dumpInput.pReg);
            pIspRegMap = NULL;
            this->m_dumpInput.pReg = NULL;
            this->m_dumpInput.magic = 0;
        }

        //dump CQ for debug
        if ( MTRUE == func_cam_DbgLogEnable_DEBUG){
            this->m_pDrv->dumpCQTable();
        }
    }

    CAM_FUNC_DBG("-\n");
    return ret;
}

CAM_RAW_PIPE::CAM_RAW_PIPE()
{
    m_pUniDrv = NULL;
    m_pP1Tuning = NULL;
    m_pDrv = NULL;
    m_RawType = E_FromTG;
    m_DataPat = _raw_normal_;
    m_Source = IF_CAM_MAX;
    m_pDrv = NULL;
    m_bTwin = MFALSE;
    m_TG_H.Raw = 0;
    m_TG_W.Raw = 0;

    m_xmxo.pix_mode_dmxi = _1_pix_;
    m_xmxo.pix_mode_dmxo = _1_pix_;
    m_xmxo.pix_mode_bmxo = _1_pix_;
    m_xmxo.pix_mode_rmxo = _1_pix_;
    m_xmxo.pix_mode_amxo = _1_pix_;
    m_xmxo.pix_mode_pmxo = _1_pix_;

    memset((void *)&m_dumpInput, 0, sizeof(m_dumpInput));
    m_dumpInput.size = CAM_BASE_RANGE;
}

MINT32 CAM_RAW_PIPE::_config( void )
{
    E_ISP_CAM_CQ cq;
    MUINT32 page;
    CAM_REG_CTL_FMT_SEL fmt_sel;
    Header_IMGO fh_imgo;
    Header_RRZO fh_rrzo;
    Header_UFEO  fh_ufeo;
    Header_PDO  fh_pdo;
    MUINT32 qbn_en = 0;

    this->m_pDrv = (ISP_DRV_CAM*)this->m_pIspDrv;
    //dbg log
    this->m_pDrv->getCurObjInfo(&this->m_hwModule,&cq,&page);

    this->m_bTwin = CAM_READ_BITS(this->m_pDrv,CAM_CTL_TWIN_STATUS,TWIN_EN);


    CAM_FUNC_DBG("CAM_RAW_PIPE::_config+ cq:0x%x,page:0x%x,rrz_en:%d,tg_pix_mode:0x%x,cam source:0x%x, twin:0x%x\n",cq,page,\
        CAM_READ_BITS(this->m_pDrv,CAM_CTL_EN,RRZ_EN),\
        this->m_xmxo.pix_mode_dmxi,\
        this->m_Source,\
        this->m_bTwin);


    //xmx cropping and single_mode under tiwn mode is cal. by twin_Drv
    //here is non-twin setting
    //TG cropping must be still applied even if path is rawi
    this->m_TG_W.Raw = CAM_READ_REG(this->m_pDrv->getPhyObj(),CAM_TG_SEN_GRAB_PXL);
    this->m_TG_H.Raw = CAM_READ_REG(this->m_pDrv->getPhyObj(),CAM_TG_SEN_GRAB_LIN);


    //dmx
    this->m_Dmx.config(this);

    //pmx, path ctrl is at DPD_Ctrl

    //rmx
    this->m_Rmx.config(this);

    //bmx
    this->m_Bmx.config(this);

    //amx in single mode {MODE_1,MODE_2} = 2b10 same as bmx
    this->m_Amx.config(this);

    //rcrop
    this->m_Rcp.config(this);

    //rcrop3
    this->m_Rcp3.config(this);


    //bin
    CAM_WRITE_REG(this->m_pDrv->getPhyObj(),CAM_BIN_CTL, 0x00000C00);//from CY
    CAM_WRITE_REG(this->m_pDrv->getPhyObj(),CAM_BIN_FTH, 0x0C090603);
    CAM_WRITE_REG(this->m_pDrv->getPhyObj(),CAM_BIN_SPARE, 0x00000000);
    CAM_WRITE_REG(this->m_pDrv,CAM_BIN_CTL, 0x00000C00);
    CAM_WRITE_REG(this->m_pDrv,CAM_BIN_FTH, 0x0C090603);
    CAM_WRITE_REG(this->m_pDrv,CAM_BIN_SPARE, 0x00000000);

    switch (this->m_DataPat) {
    case _raw_quad_code_:
        CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),CAM_VBN_OFST,VBN_OFST, 0);
        CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),CAM_VBN_GAIN,VBN_GAIN, 256);
        CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),CAM_VBN_TYPE,VBN_DIAG_SEL_EN, 0);
        CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),CAM_VBN_TYPE,VBN_TYPE, 0);

        CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),CAM_DBN_OFST,DBN_OFST, 0);
        CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),CAM_DBN_GAIN,DBN_GAIN, 256);
        break;
    case _raw_dual_pix_:
        // tbd: dbn setting
    case _raw_normal_:
    default:
        break;
    }


    //hds setting is in UNI_TOP

    //image header
    fmt_sel.Raw = CAM_READ_REG(this->m_pDrv,CAM_CTL_FMT_SEL);
    fh_imgo.Header_Enque(Header_IMGO::E_PIX_ID,this->m_pDrv,fmt_sel.Bits.PIX_ID);
    fh_imgo.Header_Enque(Header_IMGO::E_FMT,this->m_pDrv,fmt_sel.Bits.IMGO_FMT);
    qbn_en = CAM_READ_BITS(this->m_pDrv,CAM_CTL_EN,DBN_EN);
    fh_imgo.Header_Enque(Header_IMGO::E_SRC_SIZE,this->m_pDrv,\
        ((qbn_en<<31) || \
        ((this->m_TG_H.Bits.LIN_E - this->m_TG_H.Bits.LIN_S) << 16) || \
        (this->m_TG_W.Bits.PXL_E - this->m_TG_W.Bits.PXL_S )));

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
    this->m_pDrv->cqAddModule(CAM_ISP_RCP_);
    this->m_pDrv->cqAddModule(CAM_ISP_RCP3_);
    this->m_pDrv->cqAddModule(CAM_ISP_AMX_);

    this->m_pDrv->cqAddModule(CAM_ISP_RRZ_);
    this->m_pDrv->cqAddModule(CAM_ISP_PBN_);
    this->m_pDrv->cqAddModule(CAM_ISP_DBN_);
    this->m_pDrv->cqAddModule(CAM_ISP_BIN_);
    this->m_pDrv->cqAddModule(CAM_ISP_DBS_);

    //
    this->m_pDrv->cqAddModule(CAM_ISP_QBN1_);
    this->m_pDrv->cqAddModule(CAM_ISP_QBN4_);

    this->m_pDrv->cqAddModule(CAM_ISP_SL2F_);
    this->m_pDrv->cqAddModule(CAM_ISP_HLR_);
    this->m_pDrv->cqAddModule(CAM_ISP_SL2J_);
    this->m_pDrv->cqAddModule(CAM_ISP_OBC_);
    this->m_pDrv->cqAddModule(CAM_ISP_RMG_);
    this->m_pDrv->cqAddModule(CAM_ISP_BNR_);
    this->m_pDrv->cqAddModule(CAM_ISP_RMM_);
    this->m_pDrv->cqAddModule(CAM_ISP_CAC_);
    this->m_pDrv->cqAddModule(CAM_ISP_LSC_);

    this->m_pDrv->cqAddModule(CAM_ISP_RPG_);
    this->m_pDrv->cqAddModule(CAM_ISP_UFE_);
    this->m_pDrv->cqAddModule(CAM_ISP_CPG_);
    this->m_pDrv->cqAddModule(CAM_ISP_LCS_);
    this->m_pDrv->cqAddModule(CAM_ISP_AE_);
    this->m_pDrv->cqAddModule(CAM_ISP_AWB_);
    this->m_pDrv->cqAddModule(CAM_ISP_SGG1_);
    this->m_pDrv->cqAddModule(CAM_ISP_SGG2_);
    this->m_pDrv->cqAddModule(CAM_ISP_SGG5_);
    this->m_pDrv->cqAddModule(CAM_ISP_AF_);
    this->m_pDrv->cqAddModule(CAM_ISP_PS_AE_);
    this->m_pDrv->cqAddModule(CAM_ISP_PS_AWB_);

    //NOP function , these func should apb when being used at tuning
    this->m_pDrv->cqNopModule(CAM_ISP_RRZ_);
    this->m_pDrv->cqNopModule(CAM_ISP_PBN_);
    this->m_pDrv->cqNopModule(CAM_ISP_DBN_);
    this->m_pDrv->cqNopModule(CAM_ISP_BIN_);
    this->m_pDrv->cqNopModule(CAM_ISP_DBS_);
    this->m_pDrv->cqNopModule(CAM_ISP_SL2F_);
    this->m_pDrv->cqNopModule(CAM_ISP_HLR_);
    this->m_pDrv->cqNopModule(CAM_ISP_SL2J_);
    this->m_pDrv->cqNopModule(CAM_ISP_OBC_);
    this->m_pDrv->cqNopModule(CAM_ISP_RMG_);
    this->m_pDrv->cqNopModule(CAM_ISP_BNR_);
    this->m_pDrv->cqNopModule(CAM_ISP_RMM_);
    this->m_pDrv->cqNopModule(CAM_ISP_CAC_);
    this->m_pDrv->cqNopModule(CAM_ISP_LSC_);
    this->m_pDrv->cqNopModule(CAM_ISP_RPG_);
    this->m_pDrv->cqNopModule(CAM_ISP_UFE_);
    this->m_pDrv->cqNopModule(CAM_ISP_CPG_);
    this->m_pDrv->cqNopModule(CAM_ISP_LCS_);
    this->m_pDrv->cqNopModule(CAM_ISP_AE_);
    this->m_pDrv->cqNopModule(CAM_ISP_AWB_);
    this->m_pDrv->cqNopModule(CAM_ISP_SGG2_);
    this->m_pDrv->cqNopModule(CAM_ISP_SGG5_);
    this->m_pDrv->cqNopModule(CAM_ISP_AF_);
    this->m_pDrv->cqNopModule(CAM_ISP_PS_AE_);
    this->m_pDrv->cqNopModule(CAM_ISP_PS_AWB_);

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
            switch(this->m_DataPat) {
            case _raw_quad_code_:
                CAM_WRITE_BITS(this->m_pDrv,CAM_CTL_SEL, IMG_SEL, 1);
                break;
            case _raw_dual_pix_:
            case _raw_normal_:
            default:
                CAM_WRITE_BITS(this->m_pDrv,CAM_CTL_SEL, IMG_SEL, 2);
                break;
            }
            CAM_WRITE_BITS(this->m_pDrv,CAM_CTL_SEL, RCP3_SEL, 2);
            fh_imgo.Header_Enque(Header_IMGO::E_RAW_TYPE,this->m_pDrv,E_FromTG);
            break;
        case E_BeforLSC:
            CAM_FUNC_DBG("CAM_RAW_PIPE::setIMG_SEL+ cq:0x%x,page:0x%x,RawData before LSC\n",cq,page);
            if (this->m_DataPat == _raw_quad_code_) {
                CAM_FUNC_ERR("CAM_RAW_PIPE::setIMG_SEL do quad_code es not support this path: %d\n", (int)this->m_RawType);
                return MFALSE;
            }
            CAM_WRITE_BITS(this->m_pDrv,CAM_CTL_SEL, IMG_SEL, 2);
            CAM_WRITE_BITS(this->m_pDrv,CAM_CTL_SEL, RCP3_SEL, 0);
            fh_imgo.Header_Enque(Header_IMGO::E_RAW_TYPE,this->m_pDrv,E_BeforLSC);
            break;
        case E_AFTERLSC:
            CAM_FUNC_DBG("CAM_RAW_PIPE::setIMG_SEL+ cq:0x%x,page:0x%x,RawData after LSC\n",cq,page);
            if (this->m_DataPat == _raw_quad_code_) {
                CAM_FUNC_ERR("CAM_RAW_PIPE::setIMG_SEL do quad_code es not support this path: %d\n", (int)this->m_RawType);
                return MFALSE;
            }
            CAM_WRITE_BITS(this->m_pDrv,CAM_CTL_SEL, IMG_SEL, 2);
            CAM_WRITE_BITS(this->m_pDrv,CAM_CTL_SEL, RCP3_SEL, 3);
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
        MUINT32 vbn = CAM_READ_BITS(this->m_pDrv,CAM_CTL_DMA_EN,VBN_EN);

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
        input.RRZ_IN_CROP.in_size_h = (TG_H.Bits.LIN_E - TG_H.Bits.LIN_S) >> (bin + vbn);
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

                if(CAM_READ_BITS(this->m_pDrv, CAM_CTL_DMA_EN, SGG2_EN) != 1){
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
        MUINT32 tmp, qbinRatio;
        LCS_REG_CFG     _lcs_cfg;
        LCS_INPUT_INFO  input;
        Header_RRZO fh_rrzo;
        MUINT32 bin = CAM_READ_BITS(this->m_pDrv,CAM_CTL_EN,BIN_EN);
        MUINT32 dbin = CAM_READ_BITS(this->m_pDrv,CAM_CTL_EN,DBN_EN);
        MUINT32 vbn = CAM_READ_BITS(this->m_pDrv,CAM_CTL_DMA_EN,VBN_EN);

        TG_W.Raw = CAM_READ_REG(this->m_pDrv->getPhyObj(),CAM_TG_SEN_GRAB_PXL);
        input.sTGOut.w = TG_W.Bits.PXL_E - TG_W.Bits.PXL_S;
        TG_H.Raw = CAM_READ_REG(this->m_pDrv->getPhyObj(),CAM_TG_SEN_GRAB_LIN);
        input.sTGOut.h = TG_H.Bits.LIN_E - TG_H.Bits.LIN_S;

        //
        input.sHBINOut.w = (TG_W.Bits.PXL_E - TG_W.Bits.PXL_S) >> (CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_QBN1_MODE, QBN_ACC));//acc is changed to represent scaling ratio
        input.sHBINOut.h = TG_H.Bits.LIN_E - TG_H.Bits.LIN_S;

        input.sHBINOut.w = input.sHBINOut.w >> (bin + dbin);
        input.sHBINOut.h = input.sHBINOut.h >> (bin + vbn);

        input.bIsHbin = CAM_READ_BITS(this->m_pDrv,CAM_CTL_EN,BIN_EN);
        input.magic = fh_rrzo.GetRegInfo(Header_RRZO::E_Magic,this->m_pDrv);

        input.bIsDbin = dbin;
        input.bIsbin = bin;
        input.bIsVbn = vbn;

         qbinRatio = CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_QBN1_MODE, QBN_ACC);
         input.bQbinRatio = (qbinRatio == 0) ? 1 : ((qbinRatio == 1) ? 2 : 4); //return qbin ratio 1x, 2x or 4x to 3A

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
        input.RRZ_IN_CROP.in_size_h = (TG_H.Bits.LIN_E - TG_H.Bits.LIN_S) >> (bin + vbn);
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
        CAM_FUNC_DBG("CAM_RAW_PIPE::setLCS:in:%d_%d, LCS_REG(%d_%d): 0x%x,0x%x,0x%x,m(%d)\n",input.sHBINOut.w,input.sHBINOut.h,\
                _lcs_cfg.bLCS_Bypass,_lcs_cfg.bLCS_EN,_lcs_cfg._LCS_REG.LCS_ST,_lcs_cfg._LCS_REG.LCS_AWS,_lcs_cfg._LCS_REG.LCS_CON,input.magic);
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
                lcso.dma_cfg.size.stride = _lcs_cfg.u4LCSO_Stride;
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
            CAM_WRITE_REG(this->m_pDrv,CAM_SGG2_PGN,_sgg2_cfg._SGG2_REG.PGN);
            CAM_WRITE_REG(this->m_pDrv,CAM_SGG2_GMRC_1,_sgg2_cfg._SGG2_REG.GMRC_1);
            CAM_WRITE_REG(this->m_pDrv,CAM_SGG2_GMRC_2,_sgg2_cfg._SGG2_REG.GMRC_2);

            this->m_pDrv->cqApbModule(CAM_ISP_SGG2_);

        }
        else{
            this->m_pDrv->cqNopModule(CAM_ISP_SGG2_);


        }

        if(_sgg2_cfg.bSGG2_EN){
            CAM_WRITE_BITS(this->m_pDrv, CAM_CTL_DMA_EN, SGG2_EN, 1);
        }
        else{
            CAM_WRITE_BITS(this->m_pDrv, CAM_CTL_DMA_EN, SGG2_EN, 0);
        }

    }

    return ret;
}

MBOOL CAM_RAW_PIPE::setRSS(void)
{
    MBOOL ret = MTRUE;
    DMA_RSSO rsso;

    this->m_pDrv = (ISP_DRV_CAM*)this->m_pIspDrv;

    if(this->m_pP1Tuning){
        RSS_CFG _rss_cfg;
        RSS_INPUT_INFO input;
        CAM_REG_TG_SEN_GRAB_PXL TG_W;
        CAM_REG_TG_SEN_GRAB_LIN TG_H;
        Header_RRZO fh_rrzo;
        MUINT32 tmp, pixMode;

        MUINT32 bin = CAM_READ_BITS(this->m_pDrv,CAM_CTL_EN,BIN_EN);
        MUINT32 dbin = CAM_READ_BITS(this->m_pDrv,CAM_CTL_EN,DBN_EN);
        MUINT32 hds1_sel = CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_CTL_SEL,HDS1_SEL);

        if(CAM_READ_BITS(this->m_pDrv,CAM_CTL_EN,RRZ_EN) != 1){
            CAM_FUNC_ERR("RRZ must be opened in Bayer domain when RSS is needed\n");
            return MFALSE;
        }

        //only hds , bin information is not needed
        pixMode = UNI_READ_BITS(this->m_pDrv,CAM_UNI_HDS_A_MODE,HDS_DS);

        if(!hds1_sel){ //rrz is not enabled. data is from tg
            TG_W.Raw = CAM_READ_REG(this->m_pDrv->getPhyObj(),CAM_TG_SEN_GRAB_PXL);
            TG_H.Raw = CAM_READ_REG(this->m_pDrv->getPhyObj(),CAM_TG_SEN_GRAB_LIN);
            input.tg_out_w = TG_W.Bits.PXL_E - TG_W.Bits.PXL_S;
            input.tg_out_h = TG_H.Bits.LIN_E - TG_H.Bits.LIN_S;
            input.rss_in_w = input.tg_out_w >> pixMode;
            input.rss_in_h = input.tg_out_h;
            input.rrz_out_w = input.tg_out_w;
            input.rrz_out_h = input.tg_out_h;
        } else { //from rrz
            tmp = fh_rrzo.GetRegInfo(Header_RRZO::E_RRZ_DST_SIZE,this->m_pDrv);
            input.tg_out_w = 0;
            input.tg_out_h = 0;
            input.rss_in_w = (tmp & 0xffff) >> pixMode;
            input.rss_in_h = (tmp>>16);
            input.rrz_out_w = (tmp & 0xffff);
            input.rrz_out_h = (tmp>>16);
        }

        input.rss_scale_up_factor = 100; //default:100, un-support scale up
        input.bYUVFmt = MFALSE;

        CAM_FUNC_INF("%s:hds1_sel:%d, pixmode=%d, rssin.w:%d, rssin.h:%d, tg.w=%d, tg.h=%d, rrzout.w:%d rrzout.h:%d\n",\
            __PRETTY_FUNCTION__, hds1_sel, pixMode, \
            input.rss_in_w, input.rss_in_h, input.tg_out_w, input.tg_out_h, \
            input.rrz_out_w, input.rrz_out_h);

        // pre-config rss input image size
        _rss_cfg.cfg_rss_in_img = (input.rss_in_h << 16) | (input.rss_in_w);

        this->m_pP1Tuning->p1TuningNotify((MVOID*)&input,(MVOID*)&_rss_cfg);

        if(_rss_cfg.bypassRSS== 0){
            if(_rss_cfg.enRSS){
#define RSS_DRV_RZ_UINT (1048576) //2^20
                MUINT32 Mm1_H, Mm1_V, Nm1_H, Nm1_V;
                CAM_UNI_REG_RSS_A_CONTROL ctrl;
                CAM_UNI_REG_RSS_A_HORI_STEP hori_step;
                CAM_UNI_REG_RSS_A_VERT_STEP vert_step;

                //rss ctrl
                ctrl.Bits.RSS_HORI_EN = _rss_cfg.cfg_rss_ctrl_hori_en & 0x1;
                ctrl.Bits.RSS_VERT_EN = _rss_cfg.cfg_rss_ctrl_vert_en & 0x1;
                ctrl.Bits.RSS_OUTPUT_WAIT_EN = _rss_cfg.cfg_rss_ctrl_output_wait_en & 0x1;
                ctrl.Bits.RSS_VERT_FIRST = _rss_cfg.cfg_rss_ctrl_vert_first & 0x1;
                ctrl.Bits.RSS_HORI_TBL_SEL = _rss_cfg.cfg_rss_ctrl_hori_tbl_sel & 0x1F;
                ctrl.Bits.RSS_VERT_TBL_SEL = _rss_cfg.cfg_rss_ctrl_vert_tbl_sel & 0x1F;

                //rss step and ofst
                Mm1_H = (_rss_cfg.cfg_rss_in_img & 0xFFFF) - 1;
                Mm1_V = (_rss_cfg.cfg_rss_in_img >> 16) - 1;
                Nm1_H = (_rss_cfg.cfg_rss_out_img & 0xFFFF) - 1;
                Nm1_V = (_rss_cfg.cfg_rss_out_img >> 16) - 1;
                hori_step.Raw = (Mm1_H)?(MUINT32)((Nm1_H * RSS_DRV_RZ_UINT + Mm1_H - 1) / Mm1_H):0;
                vert_step.Raw = (Mm1_V)?(MUINT32)((Nm1_V * RSS_DRV_RZ_UINT + Mm1_V - 1) / Mm1_V):0;

                UNI_WRITE_REG(this->m_pDrv, CAM_UNI_RSS_A_CONTROL, ctrl.Raw);
                UNI_WRITE_REG(this->m_pDrv, CAM_UNI_RSS_A_IN_IMG, _rss_cfg.cfg_rss_in_img);
                UNI_WRITE_REG(this->m_pDrv, CAM_UNI_RSS_A_OUT_IMG, _rss_cfg.cfg_rss_out_img);
                UNI_WRITE_REG(this->m_pDrv, CAM_UNI_RSS_A_HORI_STEP, hori_step.Raw);
                UNI_WRITE_REG(this->m_pDrv, CAM_UNI_RSS_A_VERT_STEP, vert_step.Raw);
                UNI_WRITE_REG(this->m_pDrv, CAM_UNI_RSS_A_HORI_INT_OFST, 0);
                UNI_WRITE_REG(this->m_pDrv, CAM_UNI_RSS_A_HORI_SUB_OFST, 0);
                UNI_WRITE_REG(this->m_pDrv, CAM_UNI_RSS_A_VERT_INT_OFST, 0);
                UNI_WRITE_REG(this->m_pDrv, CAM_UNI_RSS_A_VERT_SUB_OFST, 0);

                UNI_WRITE_BITS(this->m_pDrv, CAM_UNI_TOP_MOD_EN, RSS_A_EN, 1);

                rsso.m_pUniDrv = this->m_pUniDrv;
                rsso.m_pIspDrv = this->m_pIspDrv;

                rsso.dma_cfg.size.xsize = (_rss_cfg.cfg_rss_out_img & 0xFFFF);
                rsso.dma_cfg.size.h = (_rss_cfg.cfg_rss_out_img >> 16) + 1; //+1 is for APL data that is appended to last line.
                rsso.dma_cfg.size.stride = (_rss_cfg.cfg_rss_out_img & 0xFFFF);

                rsso.config();
                rsso.write2CQ();
                rsso.enable(NULL);

                this->m_pDrv->cqApbModule(UNI_ISP_RSS_A_);
#undef RSS_DRV_RZ_UINT
            }
            else{
                CAM_FUNC_ERR("rss must be opened if notification is registered\n");
                ret = MFALSE;
            }

        }
        else{
            this->m_pDrv->cqNopModule(UNI_ISP_RSS_A_);
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
                    {
                        MUINT32 dmxo = CAM_READ_BITS(this->m_pDrv,CAM_CTL_FMT_SEL,PIX_BUS_DMXO);
                        MUINT32 dbn_en = CAM_READ_BITS(this->m_pDrv,CAM_CTL_EN,DBN_EN);

                        /* Due to RCP3_sel is set to 2 in E_FromTG case, therefore pix mode should ref to DMXO not TG */
                        if((dmxo - dbn_en) == _4_pix_)
                            return _4_pix_;
                        else if((dmxo - dbn_en) == _2_pix_)
                            return _2_pix_;
                        else
                            return _1_pix_;
                    }
                    break;
                case E_BeforLSC:
                    return _1_pix_;
                    break;
                case E_AFTERLSC:
                    switch(CAM_READ_BITS(this->m_pDrv,CAM_CTL_FMT_SEL,PIX_BUS_BMXO)){
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
            switch(CAM_READ_BITS(this->m_pDrv,CAM_CTL_FMT_SEL,PIX_BUS_RMXO)){
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
            return _1_pix_;
            break;
    }
}


MINT32 CAM_DPD_CTRL::_config( void )
{
    E_ISP_CAM_CQ cq;
    MUINT32 page;

    this->m_pDrv = (ISP_DRV_CAM*)this->m_pIspDrv;
    //dbg log
    this->m_pDrv->getCurObjInfo(&this->m_hwModule,&cq,&page);

    //Dual PD path is configured from statisticpipe not from twin_drv. Therefore need to reference TG pixel mode.
    this->m_xmxo.pix_mode_dmxi = (E_PIX_MODE)(CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_TG_SEN_MODE,DBL_DATA_BUS) + \
        CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_TG_SEN_MODE,DBL_DATA_BUS1));

    this->m_bTwin = CAM_READ_BITS(this->m_pDrv,CAM_CTL_TWIN_STATUS,TWIN_EN);

    CAM_FUNC_DBG("CAM_DPD_CTRL::_config+ cq:0x%x,page:0x%x,tg_pix_mode:0x%x,cam source:0x%x, twin:0x%x\n",cq,page,\
            this->m_xmxo.pix_mode_dmxi,\
            CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_CTL_SEL,DMX_SEL),\
            this->m_bTwin);


    //TG cropping must be still applied even if path is rawi
    this->m_TG_W.Raw = CAM_READ_REG(this->m_pDrv->getPhyObj(),CAM_TG_SEN_GRAB_PXL);
    this->m_TG_H.Raw = CAM_READ_REG(this->m_pDrv->getPhyObj(),CAM_TG_SEN_GRAB_LIN);


    if(CAM_READ_REG(this->m_pDrv,CAM_CTL_SEL) & PMX_SEL_1){
        switch(this->m_Density){
            case _level_1:
                CAM_WRITE_BITS(this->m_pDrv,CAM_CTL_EN,PBN_EN,1);
                CAM_WRITE_BITS(this->m_pDrv,CAM_CTL_EN,PKP_EN,0);
                CAM_WRITE_BITS(this->m_pDrv,CAM_CTL_DMA_EN,PSB_EN,0);
                CAM_WRITE_BITS(this->m_pDrv,CAM_CTL_DMA_EN,PCP_EN,0);
                break;
            case _level_2:
                CAM_WRITE_BITS(this->m_pDrv,CAM_CTL_EN,PBN_EN,0);
                CAM_WRITE_BITS(this->m_pDrv,CAM_CTL_EN,PKP_EN,0);
                CAM_WRITE_BITS(this->m_pDrv,CAM_CTL_DMA_EN,PSB_EN,1);
                CAM_WRITE_BITS(this->m_pDrv,CAM_CTL_DMA_EN,PCP_EN,1);//cropping is applied via tuning
            default:
                CAM_FUNC_ERR("unsupported Density:%d\n",this->m_Density);
                return 1;
                break;
        }
    }
    else{
        CAM_WRITE_BITS(this->m_pDrv,CAM_CTL_DMA_EN,PCP_EN,0);
        CAM_WRITE_BITS(this->m_pDrv,CAM_CTL_EN,PKP_EN,1);
        CAM_WRITE_BITS(this->m_pDrv,CAM_CTL_DMA_EN,PSB_EN,0);
        CAM_WRITE_BITS(this->m_pDrv,CAM_CTL_EN,PBN_EN,0);
    }

    //pmx need to be enabled even if data path is from bnr, pdc's linebuf is used from pmx
    this->m_Pmx.config((CAM_RAW_PIPE*)this);

    return 0;
}

MINT32 CAM_DPD_CTRL::_write2CQ(void)
{
    this->m_pDrv->cqAddModule(CAM_ISP_PMX_);

    if(CAM_READ_BITS(this->m_pDrv,CAM_CTL_DMA_EN,PCP_EN) == 1)
        this->m_pDrv->cqAddModule(CAM_ISP_PCP_CROP_);

    //descriptor of psb/pbn is applied at tuning

    return 0;
}
