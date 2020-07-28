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

#define LOG_TAG "ifunc_cam_YUV"

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
#include <math.h>                           /* floor */
#include "imageio_log.h"                    // Note: DBG_LOG_TAG/LEVEL will be used in header file, so header must be included after definition.
#undef   DBG_LOG_TAG                        // Decide a Log TAG for current file.
#ifndef USING_MTK_LDVT   // Not using LDVT.
#define  DBG_LOG_TAG        ""
#else
#define  DBG_LOG_TAG        LOG_TAG
#endif

using namespace NSCam::NSIoPipe::NSCamIOPipe;
using namespace NSImageio::NSIspio;

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

#define GetPlaneMax(module) ({\
    MUINT32 PlaneNum = 1;\
    switch(module){\
    case CAM_CRZ_CTRL::CRZ_R1:\
        switch(CAM_READ_BITS(this->m_pDrv,CAMCTL_R1_CAMCTL_FMT_SEL,CAMCTL_CRZO_R1_FMT)){\
        case CRZO_FMT_YUV422_1P: PlaneNum = ePlane_1st; break;\
        case CRZO_FMT_YUV422_2P: PlaneNum = ePlane_2nd; break;\
        default: break;\
        }\
        break;\
    case CAM_CRZ_CTRL::CRZ_R2:\
        switch(CAM_READ_BITS(this->m_pDrv,CAMCTL_R1_CAMCTL_FMT_SEL,CAMCTL_CRZO_R2_FMT)){\
        case CRZO_R2_FMT_YUV422_1P: PlaneNum = ePlane_1st; break;\
        case CRZO_R2_FMT_YUV422_2P: PlaneNum = ePlane_2nd; break;\
        case CRZO_R2_FMT_YUV420_2P: PlaneNum = ePlane_2nd; break;\
        default: break;\
        }\
        break;\
    default: break;\
    }\
    PlaneNum;\
})
#define isYUV422(damo) ({\
    MBOOL ret = MFALSE;\
    switch(damo){\
    case _yuvo_:\
        switch(CAM_READ_BITS(this->m_pDrv,CAMCTL_R1_CAMCTL_FMT2_SEL,CAMCTL_YUVO_R1_FMT)){\
        case YUVO_FMT_YUV422_1P:\
        case YUVO_FMT_YUV422_2P:\
        case YUVO_FMT_YUV422_3P: ret = MTRUE; break;\
        default: break;\
        }\
        break;\
    case _crzo_r2_:\
        switch(CAM_READ_BITS(this->m_pDrv,CAMCTL_R1_CAMCTL_FMT_SEL,CAMCTL_CRZO_R2_FMT)){\
        case CRZO_R2_FMT_YUV422_1P:\
        case CRZO_R2_FMT_YUV422_2P: ret = MTRUE; break;\
        default: break;\
        }\
        break;\
    default: break;\
    }\
    ret;\
})

/*/////////////////////////////////////////////////////////////////////////////
  ISP_YUV_PIPE
/////////////////////////////////////////////////////////////////////////////*/
MINT32 CAM_YUV_PIPE::_config( void )
{
    E_ISP_CAM_CQ cq;
    MUINT32 page;
    MUINT32 pixmode;
    REG_CAMCTL_R1_CAMCTL_FMT_SEL fmt_sel;
    Header_IMGO fh_imgo;

    this->m_pDrv = (ISP_DRV_CAM*)this->m_pIspDrv;
    //dbg log
    this->m_pDrv->getCurObjInfo(&this->m_hwModule,&cq,&page);
    pixmode = CAM_READ_BITS(this->m_pDrv->getPhyObj(),TG_R1_TG_SEN_MODE,TG_DBL_DATA_BUS) + \
          CAM_READ_BITS(this->m_pDrv->getPhyObj(),TG_R1_TG_SEN_MODE,TG_DBL_DATA_BUS1);
    if (page == 0) { /*reduce log, only show page 0*/
        CAM_FUNC_INF("CAM_YUV_PIPE::_config+ cq:0x%x,page:0x%x,pix_mode:0x%x\n",cq,page,\
            pixmode);
    }


    //hds setting is in UNI_TOP


    //image header
    fmt_sel.Raw = CAM_READ_REG(this->m_pDrv,CAMCTL_R1_CAMCTL_FMT_SEL);
    fh_imgo.Header_Enque(Header_IMGO::E_PIX_ID,this->m_pDrv,fmt_sel.Bits.CAMCTL_PIX_ID);
    fh_imgo.Header_Enque(Header_IMGO::E_FMT,this->m_pDrv,fmt_sel.Bits.CAMCTL_IMGO_R1_FMT);
    return 0;
}

MINT32 CAM_YUV_PIPE::_write2CQ(MUINT32 BurstIndex)
{
    E_ISP_CAM_CQ cq;
    MUINT32 page;
    this->m_pDrv = (ISP_DRV_CAM*)this->m_pIspDrv;
    //dbg log
    this->m_pDrv->getCurObjInfo(&this->m_hwModule,&cq,&page);

    CAM_FUNC_DBG("CAM_YUV_PIPE::_write2CQ+ cq:0x%x,page:0x%x\n",cq,page);

    if (BurstIndex > 0) {
        /*For SMVR, only add module at normal page*/
        goto EXIT;
    }

    this->m_pDrv->cqAddModule(CAM_ISP_LCS_SIZE);
    this->m_pDrv->cqAddModule(CAM_ISP_LCS_LRZR);
    this->m_pDrv->cqAddModule(CAM_ISP_GSE_);
    this->m_pDrv->cqAddModule(CAM_ISP_AF_);

    //NOP function , these func should apb when being used at tuning
    this->m_pDrv->cqNopModule(CAM_ISP_LCS_SIZE);
    this->m_pDrv->cqNopModule(CAM_ISP_LCS_LRZR);
    this->m_pDrv->cqNopModule(CAM_ISP_GSE_);
    this->m_pDrv->cqNopModule(CAM_ISP_AF_);
EXIT:

    return 0;
}

MBOOL CAM_YUV_PIPE::setLMV(void)
{
    MBOOL ret = MTRUE;
    DMA_EISO lmvo;
    this->m_pDrv = (ISP_DRV_CAM*)this->m_pIspDrv;
    if(this->m_pP1Tuning){
        LMV_CFG lmv_cfg;
        LMV_INPUT_INFO input;

        REG_TG_R1_TG_SEN_GRAB_PXL TG_W;
        REG_TG_R1_TG_SEN_GRAB_LIN TG_H;

        input.pixMode = CAM_READ_BITS(this->m_pDrv,HDS_R1_HDS_HDS_MODE,HDS_DS_MODE);

        TG_W.Raw = CAM_READ_REG(this->m_pDrv->getPhyObj(),TG_R1_TG_SEN_GRAB_PXL);
        input.sTGOut.w = TG_W.Bits.TG_PXL_E - TG_W.Bits.TG_PXL_S;
        TG_H.Raw = CAM_READ_REG(this->m_pDrv->getPhyObj(),TG_R1_TG_SEN_GRAB_LIN);
        input.sTGOut.h = TG_H.Bits.TG_LIN_E - TG_H.Bits.TG_LIN_S;

        input.sHBINOut.w = input.sTGOut.w >> (CAM_READ_BITS(this->m_pDrv->getPhyObj(),TG_R1_TG_SEN_MODE,TG_DBL_DATA_BUS) +
                     CAM_READ_BITS(this->m_pDrv->getPhyObj(),TG_R1_TG_SEN_MODE,TG_DBL_DATA_BUS1) + 1);//+1 is because of YUV fmt
        input.sHBINOut.h = input.sTGOut.h;

        input.sRMXOut.w = 0;
        input.sRMXOut.h = 0;

        input.bYUVFmt = MTRUE;

        CAM_FUNC_DBG("CAM_YUV_PIPE::setLMV:w/h(%d_%d,%d_%d)\n",input.sHBINOut.w,input.sHBINOut.h,input.sTGOut.w,input.sTGOut.h);
        this->m_pP1Tuning->p1TuningNotify((MVOID*)&input,(MVOID*)&lmv_cfg);

        CAM_FUNC_DBG("-\n");

        if(lmv_cfg.bypassLMV == 0){
            if(lmv_cfg.enLMV){
                MUINT32 addrost,modulesize;
                ISP_DRV_REG_IO_STRUCT* pReg;
                MUINT32* ptr = (MUINT32*)&lmv_cfg.cfg_lmv_prep_me_ctrl1;
                this->m_pDrv->getCQModuleInfo(CAM_ISP_LMV_,addrost,modulesize);
                pReg = (ISP_DRV_REG_IO_STRUCT*)malloc(sizeof(ISP_DRV_REG_IO_STRUCT)*modulesize);

                for(MUINT32 i=0;i<modulesize;i++){
                    pReg[i].Data = *(ptr++);
                }

                CAM_BURST_WRITE_REGS(this->m_pDrv,LMV_R1_LMV_LMV_PREP_ME_CTRL1,pReg,modulesize);
                CAM_WRITE_BITS(this->m_pDrv, CAMCTL_R1_CAMCTL_EN2, CAMCTL_LMV_R1_EN, 1);

                lmvo.m_pIspDrv = this->m_pIspDrv;

                lmvo.config();
                lmvo.write2CQ(0);
                lmvo.enable(NULL);


                this->m_pDrv->cqApbModule(CAM_ISP_LMV_);
                free(pReg);
            }
            else{
                CAM_FUNC_ERR("eis must be opened if notification is registered\n");
                ret = MFALSE;
            }
        }
        else{
            this->m_pDrv->cqNopModule(CAM_ISP_LMV_);
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
        REG_TG_R1_TG_SEN_GRAB_PXL TG_W;
        REG_TG_R1_TG_SEN_GRAB_LIN TG_H;
        MUINT32 pixMode;
        LCES_REG_CFG     _lces_cfg;
        LCES_INPUT_INFO  input;
        Header_RRZO fh_rrzo;

        TG_W.Raw = CAM_READ_REG(this->m_pDrv->getPhyObj(),TG_R1_TG_SEN_GRAB_PXL);
        TG_H.Raw = CAM_READ_REG(this->m_pDrv->getPhyObj(),TG_R1_TG_SEN_GRAB_LIN);

        pixMode = (CAM_READ_BITS(this->m_pDrv->getPhyObj(),TG_R1_TG_SEN_MODE,TG_DBL_DATA_BUS) + \
        CAM_READ_BITS(this->m_pDrv->getPhyObj(),TG_R1_TG_SEN_MODE,TG_DBL_DATA_BUS1));

        input.sHBINOut.w = ((TG_W.Bits.TG_PXL_E - TG_W.Bits.TG_PXL_S) >> (pixMode+1));
        input.sHBINOut.h = TG_H.Bits.TG_LIN_E - TG_H.Bits.TG_LIN_S;
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
        this->m_pP1Tuning->p1TuningNotify((MVOID*)&input,(MVOID*)&_lces_cfg);
        CAM_FUNC_DBG("-");
        CAM_FUNC_DBG("CAM_YUV_PIPE::setLCS:in:%d_%d, LCS_REG(%d): m(%d)\n",input.sRRZOut.w,input.sRRZOut.h,\
                _lces_cfg.bLCES_Bypass,input.magic);
        if(_lces_cfg.bLCES_Bypass == 0)
        {
            if(_lces_cfg.bLCES_EN){
                CAM_WRITE_REG(this->m_pDrv, LCES_R1_LCES_IN_SIZE, _lces_cfg._LCES_REG.LCES_IN_SIZE );
                CAM_WRITE_REG(this->m_pDrv, LCES_R1_LCES_OUT_SIZE, _lces_cfg._LCES_REG.LCES_OUT_SIZE );
                CAM_WRITE_REG(this->m_pDrv, LCES_R1_LCES_LRZRX, _lces_cfg._LCES_REG.LCES_LRZRX );
                CAM_WRITE_REG(this->m_pDrv, LCES_R1_LCES_LRZRY, _lces_cfg._LCES_REG.LCES_LRZRY );
                CAM_WRITE_BITS(this->m_pDrv, CAMCTL_R1_CAMCTL_EN, CAMCTL_LCES_R1_EN, 1);

                lcso.m_pIspDrv = this->m_pIspDrv;
                lcso.dma_cfg.size.xsize = CAM_READ_BITS(this->m_pDrv,LCES_R1_LCES_OUT_SIZE,LCES_OUT_WD)*2;
                lcso.dma_cfg.size.h = CAM_READ_BITS(this->m_pDrv,LCES_R1_LCES_OUT_SIZE,LCES_OUT_HT);
                lcso.dma_cfg.size.stride = lcso.dma_cfg.size.xsize;
                lcso.config();
                lcso.write2CQ(0);
                lcso.enable(NULL);

                this->m_pDrv->cqApbModule(CAM_ISP_LCS_SIZE);
                this->m_pDrv->cqApbModule(CAM_ISP_LCS_LRZR);
            }
            else{
                CAM_FUNC_ERR("lcs must be opened if notification is registered\n");
                ret = MFALSE;
            }

        }
        else {
            this->m_pDrv->cqNopModule(CAM_ISP_LCS_SIZE);
            this->m_pDrv->cqNopModule(CAM_ISP_LCS_LRZR);
        }
    }
    return ret;

}

MBOOL CAM_YUV_PIPE::setGSE(void)
{
    MBOOL ret = MTRUE;

    this->m_pDrv = (ISP_DRV_CAM*)this->m_pIspDrv;
    if(this->m_pP1Tuning){
        LMV_SGG_CFG lmv_sgg_cfg;

        CAM_FUNC_DBG("CAM_YUV_PIPE::setGSE:\n");
        this->m_pP1Tuning->p1TuningNotify(NULL,(MVOID*)&lmv_sgg_cfg);
        CAM_FUNC_DBG("-\n");

        if(lmv_sgg_cfg.bSGG2_Bypass == 0){
            CAM_WRITE_REG(this->m_pDrv,GSE_R1_GSE_GSE_00,lmv_sgg_cfg.PGN);
            CAM_WRITE_REG(this->m_pDrv,GSE_R1_GSE_GSE_01,lmv_sgg_cfg.GMRC_1);
            CAM_WRITE_REG(this->m_pDrv,GSE_R1_GSE_GSE_02,lmv_sgg_cfg.GMRC_2);
            this->m_pDrv->cqApbModule(CAM_ISP_GSE_);
        }
        else{
            this->m_pDrv->cqNopModule(CAM_ISP_GSE_);
        }

        if(lmv_sgg_cfg.bSGG2_EN){
            CAM_WRITE_BITS(this->m_pDrv, CAMCTL_R1_CAMCTL_EN2, CAMCTL_GSE_R1_EN, 1);
        }
        else{
            CAM_WRITE_BITS(this->m_pDrv, CAMCTL_R1_CAMCTL_EN2, CAMCTL_GSE_R1_EN, 0);
        }
    }

    return ret;
}

MUINT32 CAM_YUV_PIPE::getCurPixMode(_isp_dma_enum_ dmao)
{
    this->m_pDrv = (ISP_DRV_CAM*)this->m_pIspDrv;
    switch(dmao){
        case _imgo_:
            if(CAM_READ_BITS(this->m_pDrv->getPhyObj(),TG_R1_TG_SEN_MODE,TG_DBL_DATA_BUS1) == 1)
                return ePixMode_4;
            else if(CAM_READ_BITS(this->m_pDrv->getPhyObj(),TG_R1_TG_SEN_MODE,TG_DBL_DATA_BUS) == 1)
                return ePixMode_2;
            else
                return ePixMode_1;
            break;
        default:
            CAM_FUNC_ERR("unsupported dmao:0x%x,return unknown-pix\n",dmao);
            return ePixMode_NONE;
            break;
    }
    return 0;
}

CAM_CRZ_CTRL::CAM_CRZ_CTRL(E_CRZ_RX RX, CAM_RAW_PIPE* _this, MBOOL bIsCB)
{
    mFgVertFirst = MFALSE;
    m_bIsCB = bIsCB;
    memset((void*)&crz_in, 0, sizeof(crz_in));
    memset((void*)&crz_in_roi, 0, sizeof(crz_in_roi));
    memset((void*)&crz_out_size, 0, sizeof(crz_out_size));
    m_pMainPipe = _this;
    m_pDrv = _this->m_pDrv;
    mCRZ_RX = RX;
    mNameStr = "CRZ_R" + std::to_string(RX);
}

inline MVOID CAM_CRZ_CTRL::updateCrzInSize()
{
    E_ISP_CAM_CQ cq;
    MUINT32 page = 0;

    this->m_pDrv = this->m_pMainPipe->m_pDrv;
    this->m_pDrv->getCurObjInfo(&this->m_hwModule,&cq,&page);

    if (isThroughRRZ(this->m_pDrv)) {
        this->crz_in.w = this->m_pMainPipe->m_input.CrzIn_W = (CAM_READ_BITS(this->m_pDrv, MRG_R13_MRG_CROP, MRG_END_X) - \
                                                               CAM_READ_BITS(this->m_pDrv, MRG_R13_MRG_CROP, MRG_STR_X) + 1);
        this->crz_in.h = this->m_pMainPipe->m_input.CrzIn_H = CAM_READ_BITS(this->m_pDrv, MRG_R13_MRG_VSIZE, MRG_HT);
    }
    else {
        this->crz_in.w = this->m_pMainPipe->m_input.CrzIn_W = this->m_pMainPipe->m_input.CurBinOut_W;
        this->crz_in.h = this->m_pMainPipe->m_input.CrzIn_H = this->m_pMainPipe->m_input.CurBinOut_H;
    }

    CAM_FUNC_DBG("%s: updateCrzInSize: isThroughRRZ_%d W_0x%x H_0x%x",this->name_Str(),
        isThroughRRZ(this->m_pDrv),this->m_pMainPipe->m_input.CrzIn_W,this->m_pMainPipe->m_input.CrzIn_H);
}

inline MBOOL CAM_CRZ_CTRL::CalOffset(
    E_CRZ_ALGO          Algo,
    MBOOL               IsWidth,
    MUINT32             CoeffStep,
    MUINT32              Offset,
    MUINT32*            pLumaInt,
    MUINT32*            pLumaSub,
    MUINT32*            pChromaInt,
    MUINT32*            pChromaSub
)
{
    MUINT32 OffsetInt, OffsetSub;
    CAM_FUNC_DBG("%s: - E. Algo: %d, IsWidth: %d, CoeffStep: %d, Offset: %d.",this->name_Str(), Algo, IsWidth, CoeffStep, Offset);

    OffsetInt = floor(Offset);
    OffsetSub = kRzUint[Algo] * (Offset - floor(Offset));

    // Calculate pChromaInt/pChromaSub according current algorithm.
    if (Algo == ALGO_6_TAP) {
        *pLumaInt = OffsetInt;
        *pLumaSub = OffsetSub;

        if (IsWidth) {
            *pChromaInt = floor(Offset / (2.0)); // Because format is YUV422, so the width of chroma is half of Y.
            *pChromaSub = kRzUint[Algo] * 2 * (Offset / (2.0) - floor(Offset / (2.0)));
        }
        else {
            *pChromaInt = (*pLumaInt);
            *pChromaSub = (*pLumaSub);
        }
    }
    else {
        *pLumaInt = (OffsetInt * CoeffStep + OffsetSub * CoeffStep / kRzUint[Algo]) / kRzUint[Algo];
        *pLumaSub = (OffsetInt * CoeffStep + OffsetSub * CoeffStep / kRzUint[Algo]) % kRzUint[Algo];
        *pChromaInt = (*pLumaInt);
        *pChromaSub = (*pLumaSub);
    }

    //
    CAM_FUNC_DBG("%s: - X. LumaInt/Sub(%d, %d), ChromaInt/Sub(%d, %d).",this->name_Str(), *pLumaInt, *pLumaSub, *pChromaInt, *pChromaSub);
    //
    return MTRUE;
}

MBOOL CAM_CRZ_CTRL::calCoefStep(
    MUINT32                 SizeIn_H,
    MUINT32                 SizeIn_V,
    MUINT32                 u4CroppedSize_H,
    MUINT32                 u4CroppedSize_V,
    MUINT32                 SizeOut_H,
    MUINT32                 SizeOut_V,
    E_CRZ_ALGO              *pAlgo_H,
    E_CRZ_ALGO              *pAlgo_V,
    MUINT32                 *pTable_H,
    MUINT32                 *pTable_V,
    MUINT32                 *pCoeffStep_H,
    MUINT32                 *pCoeffStep_V
)
{
    MUINT32 Mm1_H, Mm1_V, Nm1_H, Nm1_V;
    MUINT32 u4SupportingWidth_4tap  = 0;
    MUINT32 u4SupportingWidth_4Ntap = 0;
    MUINT32 u4SupportingWidth_Ntap  = 0;
    MBOOL Result = MTRUE;   // MTRUE: success. MFALSE: fail.
    MUINT32 u4SizeToBeChecked_H = 0;
    MUINT32 u4SizeToBeChecked_V = 0;

    //
    CAM_FUNC_DBG("%s: - E. Mode: %d. eRzName: %d. In/Crop/Out: (%d, %d)/(%d, %d)/(%d, %d).",this->name_Str(), SizeIn_H, SizeIn_V, u4CroppedSize_H, u4CroppedSize_V, SizeOut_H, SizeOut_V);

    Mm1_H = u4CroppedSize_H - 1;
    Mm1_V = u4CroppedSize_V - 1;
    Nm1_H = SizeOut_H - 1;
    Nm1_V = SizeOut_V - 1;

    //Calculate Algo/CoeffStep/Table.
    mFgVertFirst = MFALSE;

    if ((u4CroppedSize_H  * RZ_4_TAP_RATIO_MAX) >= SizeOut_H && \
        (SizeOut_H * RZ_4_TAP_RATIO_MIN) >  u4CroppedSize_H ) {
// 4-tap
        if (((u4CroppedSize_H  * RZ_4_TAP_RATIO_MAX) >= SizeOut_H) && \
            ((u4CroppedSize_H  * RZ_4_TAP_RATIO_MAX) <= SizeOut_H)) {
            mFgVertFirst = MTRUE;
        }

        //     Decide Algorithm.
        *pAlgo_H = ALGO_6_TAP;
        //     Calculate CoefStep.
        *pCoeffStep_H = (MUINT32)((Mm1_H * kRzUint[*pAlgo_H] + (Nm1_H >> 1)) / Nm1_H);
        //     Find Table.
        if ((*pCoeffStep_H) <= 32768) {
            (*pTable_H) = 27;
        }
        else {
            for ((*pTable_H) = RZ_TABLE_OFFSET; (*pTable_H) < (RZ_TABLE_OFFSET+RZ_TABLE_AMOUNT); (*pTable_H)++) {
                if ((*pCoeffStep_H) <= kRzTbl[(*pTable_H) - RZ_TABLE_OFFSET]) {
                    break;
                }
            }
            // When Table exceed RZ_TABLE_AMOUNT, use last table.
            if ((*pTable_H) >= (RZ_TABLE_OFFSET+RZ_TABLE_AMOUNT)) {
                (*pTable_H) = 26;
            }
        }
    }
    else if ((SizeOut_H * RZ_4N_TAP_RATIO_MAX) <= u4CroppedSize_H &&
             (SizeOut_H * RZ_4N_TAP_RATIO_MIN) >= u4CroppedSize_H ) {
// 4N-tap
        //     Decide Algorithm.
        *pAlgo_H = ALGO_N_TAP;
        //     Calculate CoefStep.
        *pCoeffStep_H = (MUINT32)((Nm1_H * kRzUint[*pAlgo_H] + Mm1_H - 1) / Mm1_H);
        //     Find Table.
        (*pTable_H) = RZ_N_TAP_TABLE;
    }
    else if ((SizeOut_H * RZ_N_TAP_RATIO_MAX) <  u4CroppedSize_H && \
             (SizeOut_H * RZ_N_TAP_RATIO_MIX) >= u4CroppedSize_H) {
// N-tap
        //     Decide Algorithm.
        *pAlgo_H = ALGO_N_TAP;
        //     Calculate CoefStep.
        *pCoeffStep_H = (MUINT32)((Nm1_H * kRzUint[*pAlgo_H] + Mm1_H - 1) / Mm1_H);
        //     Find Table.
        (*pTable_H) = RZ_N_TAP_TABLE;
    }
    else {
        CAM_FUNC_ERR("%s: [Error]Not support ratio. u4CroppedSize_H: %d, SizeOut_H: %d.",this->name_Str(), u4CroppedSize_H, SizeOut_H);
        Result = MFALSE;
        goto lbExit;
    }

    // Calculate vertical part.
    //     Calculate Algo/CoeffStep/Table.
    if ( ((u4CroppedSize_V  * RZ_4_TAP_RATIO_MAX) >= SizeOut_V && \
            (SizeOut_V * RZ_4_TAP_RATIO_MIN) >  u4CroppedSize_V )) {
// 4-tap
        //     Decide Algorithm.
        *pAlgo_V = ALGO_6_TAP;
        //     Calculate CoefStep.
        *pCoeffStep_V = (MUINT32)((Mm1_V * kRzUint[*pAlgo_V] + (Nm1_V >> 1)) / Nm1_V);
        //     Find Table.
        if ((*pCoeffStep_V) <= 32768) {
            (*pTable_V) = 27;
        }
        else {
            for ((*pTable_V) = RZ_TABLE_OFFSET; (*pTable_V) < (RZ_TABLE_OFFSET+RZ_TABLE_AMOUNT); (*pTable_V)++) {
                if ((*pCoeffStep_V) <= kRzTbl[(*pTable_V) - RZ_TABLE_OFFSET]) {
                    //(*pTable_H) = (*pTable_H) + RZ_TABLE_OFFSET;
                    break;
                }
            }

            // When Table exceed RZ_TABLE_AMOUNT, use last table.
            if ((*pTable_V) >= (RZ_TABLE_OFFSET+RZ_TABLE_AMOUNT)) {
                (*pTable_V) = 26;
            }

        }
    }
    else if ((SizeOut_V * RZ_4N_TAP_RATIO_MAX) <= u4CroppedSize_V && \
        (SizeOut_V * RZ_4N_TAP_RATIO_MIN) >= u4CroppedSize_V ) {
// 4N-tap
        //     Decide Algorithm.
        *pAlgo_V = ALGO_N_TAP;
        //     Calculate CoefStep.
        *pCoeffStep_V = (MUINT32)((Nm1_V * kRzUint[*pAlgo_V] + Mm1_V - 1) / Mm1_V);
        //     Find Table.
        (*pTable_V) = RZ_N_TAP_TABLE;
    }
    else if ((SizeOut_V * RZ_N_TAP_RATIO_MAX) <  u4CroppedSize_V && \
             (SizeOut_V * RZ_N_TAP_RATIO_MIX) >= u4CroppedSize_V) {
// N-tap
        //     Decide Algorithm.
        *pAlgo_V = ALGO_N_TAP;
        //     Calculate CoefStep.
        *pCoeffStep_V = (MUINT32)((Nm1_V * kRzUint[*pAlgo_V] + Mm1_V - 1) / Mm1_V);
        //     Find Table.
        (*pTable_V) = RZ_N_TAP_TABLE;
    }
    else {
        CAM_FUNC_ERR("%s: [Error]Not support ratio. u4CroppedSize_V: %d, SizeOut_V: %d.",this->name_Str(), u4CroppedSize_V, SizeOut_V);
        Result = MFALSE;
        goto lbExit;
    }

lbExit:
    CAM_FUNC_DBG("%s: - X. Result: %d. Algo(%d, %d) Table(%d, %d) CoeffStep(%d, %d).",this->name_Str(), Result, *pAlgo_H, *pAlgo_V, *pTable_H, *pTable_V, *pCoeffStep_H, *pCoeffStep_V);
    return Result;
}

MINT32 CAM_CRZ_CTRL::_config()
{
    E_CRZ_ALGO Algo_H, Algo_V;
    MUINT32 Table_H, Table_V;
    MUINT32 CoeffStep_H, CoeffStep_V;
    MUINT32 LumaInt_H,LumaInt_V;
    MUINT32 LumaSub_H,LumaSub_V;
    MUINT32 ChromaInt_H,ChromaInt_V;
    MUINT32 ChromaSub_H,ChromaSub_V;
    MINT32 Result = -1;   // MTRUE: success. MFALSE: fail.

    this->updateCrzInSize(); //rrz out size might change by setRRZ, need update here for crz

    CAM_FUNC_DBG("%s: m_bIsCB_%d In(%d, %d). Crop(%d, %d). Out(%d, %d). CropStart(%d, %d).",
        this->name_Str() ,m_bIsCB, crz_in.w, crz_in.h, crz_in_roi.w, crz_in_roi.h, crz_out_size.w, crz_out_size.h, crz_in_roi.x, crz_in_roi.y);

    if(!m_bIsCB){
        /*CRZ cfg depends on setCRZ_RX CB, because at enque phrase there is no info for MW to know CRZ is throught RRZ or not.*/
        Result = 0;
        goto EXIT;
    }
    //Width
    if (crz_in_roi.w == 0) {
        crz_in_roi.x = 0;
        crz_in_roi.w = crz_in.w;
    }
    else if (crz_in_roi.x < 0 || crz_in_roi.w > crz_in.w || (crz_in_roi.x+crz_in_roi.w) > crz_in.w) {
        CAM_FUNC_ERR("%s: Error crop. InWidth: %d, crz_in_roi.Width::Start: %d, Size: %d",this->name_Str(), crz_in.w, crz_in_roi.x, crz_in_roi.w);
        goto EXIT;
    }

    //Height
    if (crz_in_roi.h == 0) {
        crz_in_roi.y = 0;
        crz_in_roi.h = crz_in.h;
    }
    else if (crz_in_roi.y < 0 || crz_in_roi.h > crz_in.h || (crz_in_roi.y+crz_in_roi.h) > crz_in.h) {
        CAM_FUNC_ERR("%s: Error crop. InHeight: %d. crz_in_roi.Height::Start: %d, Size: %d",this->name_Str(),crz_in.h, crz_in_roi.y, crz_in_roi.h);
        goto EXIT;
    }

    if (crz_out_size.w > crz_in_roi.w || crz_out_size.h > crz_in_roi.h) {
        CAM_FUNC_ERR("%s: CRZ can't support size up. crz_in_roi(%d/%d), crz_out_size: (%d/%d)",
            this->name_Str(), crz_in_roi.w, crz_in_roi.h, crz_out_size.w, crz_out_size.h);
        goto EXIT;
    }

    Result = calCoefStep(
                crz_in.w,
                crz_in.h,
                crz_in_roi.w,
                crz_in_roi.h,
                crz_out_size.w,
                crz_out_size.h,
                &Algo_H,
                &Algo_V,
                &Table_H,
                &Table_V,
                &CoeffStep_H,
                &CoeffStep_V
             );

    if (!Result) {
        CAM_FUNC_ERR("%s:[Error]CalAlgoAndCStep fail.",this->name_Str());
        goto EXIT;
    }

    if (!CalOffset(Algo_H, MTRUE, CoeffStep_H, crz_in_roi.x, &LumaInt_H, &LumaSub_H, &ChromaInt_H, &ChromaSub_H)) {
        CAM_FUNC_ERR("%s: CalOffset H fail.",this->name_Str());
        goto EXIT;
    }

    if (!CalOffset(Algo_V, MFALSE, CoeffStep_V, crz_in_roi.y, &LumaInt_V, &LumaSub_V, &ChromaInt_V, &ChromaSub_V)) {
        CAM_FUNC_ERR("%s: CalOffset V fail.",this->name_Str());
        goto EXIT;
    }

    // check value
    if((Table_H > MASK_TABLE_SELECT)||(Table_V > MASK_TABLE_SELECT)) {
        CAM_FUNC_ERR("%s: Table(%d/%d) is out of %d",this->name_Str(),Table_H,Table_V,MASK_TABLE_SELECT);
        goto EXIT;
    }

    if((crz_in.w > MASK_IMAGE_SIZE)||(crz_out_size.w > MASK_IMAGE_SIZE) || \
       (crz_in.h > MASK_IMAGE_SIZE)||(crz_out_size.h > MASK_IMAGE_SIZE)) {
        CAM_FUNC_ERR("%s: crz_in(%d/%d) crz_out_size(%d/%d) is out of %d",this->name_Str(),crz_in.w,crz_in.h,crz_out_size.w,crz_out_size.h,MASK_IMAGE_SIZE);
        goto EXIT;
    }

    if((CoeffStep_H > MASK_COEFF_STEP)||(CoeffStep_V > MASK_COEFF_STEP)) {
        CAM_FUNC_ERR("%s: CoeffStep(%d/%d) is out of %d",this->name_Str(),CoeffStep_H,CoeffStep_V,MASK_COEFF_STEP);
        goto EXIT;
    }

    if((LumaInt_H > MASK_INT_OFFSET || ChromaInt_H > MASK_INT_OFFSET) || \
       (LumaSub_H > MASK_SUB_OFFSET || ChromaSub_H > MASK_SUB_OFFSET) || \
       (LumaInt_V > MASK_INT_OFFSET || ChromaInt_V > MASK_INT_OFFSET) || \
       (LumaSub_V > MASK_SUB_OFFSET || ChromaSub_V > MASK_SUB_OFFSET)) {
        CAM_FUNC_ERR("%s: LumaInt(%d/%d) or ChromaInt(%d/%d) is out of %d",this->name_Str(),LumaInt_H,LumaInt_V,ChromaInt_H,ChromaInt_V,MASK_INT_OFFSET);
        CAM_FUNC_ERR("%s: LumaSub(%d/%d) or ChromaSub(%d/%d) is out of %d",this->name_Str(),LumaSub_H,LumaSub_V,ChromaSub_H,ChromaSub_V,MASK_SUB_OFFSET);
        goto EXIT;
    }

    switch(this->id()){
    case CRZ_R1:
        {
            Header_CRZO_R1 fh_crzo_r1;
            CAM_WRITE_BITS(this->m_pDrv,CRZ_R1_CRZ_CONTROL, CRZ_HORI_EN, MTRUE);
            CAM_WRITE_BITS(this->m_pDrv,CRZ_R1_CRZ_CONTROL, CRZ_HORI_ALGO, Algo_H);
            CAM_WRITE_BITS(this->m_pDrv,CRZ_R1_CRZ_CONTROL, CRZ_TRNC_HORI, 0);
            CAM_WRITE_BITS(this->m_pDrv,CRZ_R1_CRZ_CONTROL, CRZ_HORI_TBL_SEL, Table_H);
            CAM_WRITE_BITS(this->m_pDrv,CRZ_R1_CRZ_IN_IMG, CRZ_IN_WD, crz_in.w);
            CAM_WRITE_BITS(this->m_pDrv,CRZ_R1_CRZ_OUT_IMG, CRZ_OUT_WD, crz_out_size.w);
            CAM_WRITE_BITS(this->m_pDrv,CRZ_R1_CRZ_HORI_STEP, CRZ_HORI_STEP, CoeffStep_H);
            CAM_WRITE_BITS(this->m_pDrv,CRZ_R1_CRZ_LUMA_HORI_INT_OFST,CRZ_LUMA_HORI_INT_OFST, LumaInt_H);
            CAM_WRITE_BITS(this->m_pDrv,CRZ_R1_CRZ_LUMA_HORI_SUB_OFST,CRZ_LUMA_HORI_SUB_OFST, LumaSub_H);
            CAM_WRITE_BITS(this->m_pDrv,CRZ_R1_CRZ_CHRO_HORI_INT_OFST,CRZ_CHRO_HORI_INT_OFST, ChromaInt_H);
            CAM_WRITE_BITS(this->m_pDrv,CRZ_R1_CRZ_CHRO_HORI_SUB_OFST,CRZ_CHRO_HORI_SUB_OFST, ChromaSub_H);
            CAM_WRITE_BITS(this->m_pDrv,CRZ_R1_CRZ_CONTROL, CRZ_VERT_EN, MTRUE);
            CAM_WRITE_BITS(this->m_pDrv,CRZ_R1_CRZ_CONTROL, CRZ_VERT_FIRST, mFgVertFirst);
            CAM_WRITE_BITS(this->m_pDrv,CRZ_R1_CRZ_CONTROL, CRZ_VERT_ALGO, Algo_V);
            CAM_WRITE_BITS(this->m_pDrv,CRZ_R1_CRZ_CONTROL, CRZ_TRNC_VERT, 0);
            CAM_WRITE_BITS(this->m_pDrv,CRZ_R1_CRZ_CONTROL, CRZ_VERT_TBL_SEL, Table_V);
            CAM_WRITE_BITS(this->m_pDrv,CRZ_R1_CRZ_IN_IMG, CRZ_IN_HT,crz_in.h);
            CAM_WRITE_BITS(this->m_pDrv,CRZ_R1_CRZ_OUT_IMG, CRZ_OUT_HT, crz_out_size.h);
            CAM_WRITE_BITS(this->m_pDrv,CRZ_R1_CRZ_VERT_STEP, CRZ_VERT_STEP, CoeffStep_V);
            CAM_WRITE_BITS(this->m_pDrv,CRZ_R1_CRZ_LUMA_VERT_INT_OFST,CRZ_LUMA_VERT_INT_OFST, LumaInt_V);
            CAM_WRITE_BITS(this->m_pDrv,CRZ_R1_CRZ_LUMA_VERT_SUB_OFST,CRZ_LUMA_VERT_SUB_OFST, LumaSub_V);
            CAM_WRITE_BITS(this->m_pDrv,CRZ_R1_CRZ_CHRO_VERT_INT_OFST,CRZ_CHRO_VERT_INT_OFST, ChromaInt_V);
            CAM_WRITE_BITS(this->m_pDrv,CRZ_R1_CRZ_CHRO_VERT_SUB_OFST,CRZ_CHRO_VERT_SUB_OFST, ChromaSub_V);
            CAM_WRITE_BITS(this->m_pDrv,CAMCTL_R1_CAMCTL_EN3, CAMCTL_CRZ_R1_EN, MTRUE);
            fh_crzo_r1.Header_Enque(Header_CRZO_R1::E_CRZ_CROP_START,this->m_pDrv,((crz_in_roi.y << 16) | crz_in_roi.x));
            fh_crzo_r1.Header_Enque(Header_CRZO_R1::E_CRZ_CROP_SIZE,this->m_pDrv,((crz_in_roi.h << 16) | crz_in_roi.w));
            fh_crzo_r1.Header_Enque(Header_CRZO_R1::E_CRZ_DST_SIZE,this->m_pDrv,((crz_out_size.h << 16) | crz_out_size.w));
        }
        break;
    case CRZ_R2:
        {
            Header_CRZO_R2 fh_crzo_r2;
            CAM_WRITE_BITS(this->m_pDrv,CRZ_R2_CRZ_CONTROL, CRZ_HORI_EN, MTRUE);
            CAM_WRITE_BITS(this->m_pDrv,CRZ_R2_CRZ_CONTROL, CRZ_HORI_ALGO, Algo_H);
            CAM_WRITE_BITS(this->m_pDrv,CRZ_R2_CRZ_CONTROL, CRZ_TRNC_HORI, 0);
            CAM_WRITE_BITS(this->m_pDrv,CRZ_R2_CRZ_CONTROL, CRZ_HORI_TBL_SEL, Table_H);
            CAM_WRITE_BITS(this->m_pDrv,CRZ_R2_CRZ_IN_IMG, CRZ_IN_WD, crz_in.w);
            CAM_WRITE_BITS(this->m_pDrv,CRZ_R2_CRZ_OUT_IMG, CRZ_OUT_WD, crz_out_size.w);
            CAM_WRITE_BITS(this->m_pDrv,CRZ_R2_CRZ_HORI_STEP, CRZ_HORI_STEP, CoeffStep_H);
            CAM_WRITE_BITS(this->m_pDrv,CRZ_R2_CRZ_LUMA_HORI_INT_OFST,CRZ_LUMA_HORI_INT_OFST, LumaInt_H);
            CAM_WRITE_BITS(this->m_pDrv,CRZ_R2_CRZ_LUMA_HORI_SUB_OFST,CRZ_LUMA_HORI_SUB_OFST, LumaSub_H);
            CAM_WRITE_BITS(this->m_pDrv,CRZ_R2_CRZ_CHRO_HORI_INT_OFST,CRZ_CHRO_HORI_INT_OFST, ChromaInt_H);
            CAM_WRITE_BITS(this->m_pDrv,CRZ_R2_CRZ_CHRO_HORI_SUB_OFST,CRZ_CHRO_HORI_SUB_OFST, ChromaSub_H);
            CAM_WRITE_BITS(this->m_pDrv,CRZ_R2_CRZ_CONTROL, CRZ_VERT_EN, MTRUE);
            CAM_WRITE_BITS(this->m_pDrv,CRZ_R2_CRZ_CONTROL, CRZ_VERT_FIRST, mFgVertFirst);
            CAM_WRITE_BITS(this->m_pDrv,CRZ_R2_CRZ_CONTROL, CRZ_VERT_ALGO, Algo_V);
            CAM_WRITE_BITS(this->m_pDrv,CRZ_R2_CRZ_CONTROL, CRZ_TRNC_VERT, 0);
            CAM_WRITE_BITS(this->m_pDrv,CRZ_R2_CRZ_CONTROL, CRZ_VERT_TBL_SEL, Table_V);
            CAM_WRITE_BITS(this->m_pDrv,CRZ_R2_CRZ_IN_IMG, CRZ_IN_HT,crz_in.h);
            CAM_WRITE_BITS(this->m_pDrv,CRZ_R2_CRZ_OUT_IMG, CRZ_OUT_HT, crz_out_size.h);
            CAM_WRITE_BITS(this->m_pDrv,CRZ_R2_CRZ_VERT_STEP, CRZ_VERT_STEP, CoeffStep_V);
            CAM_WRITE_BITS(this->m_pDrv,CRZ_R2_CRZ_LUMA_VERT_INT_OFST,CRZ_LUMA_VERT_INT_OFST, LumaInt_V);
            CAM_WRITE_BITS(this->m_pDrv,CRZ_R2_CRZ_LUMA_VERT_SUB_OFST,CRZ_LUMA_VERT_SUB_OFST, LumaSub_V);
            CAM_WRITE_BITS(this->m_pDrv,CRZ_R2_CRZ_CHRO_VERT_INT_OFST,CRZ_CHRO_VERT_INT_OFST, ChromaInt_V);
            CAM_WRITE_BITS(this->m_pDrv,CRZ_R2_CRZ_CHRO_VERT_SUB_OFST,CRZ_CHRO_VERT_SUB_OFST, ChromaSub_V);
            CAM_WRITE_BITS(this->m_pDrv,CAMCTL_R1_CAMCTL_EN3, CAMCTL_CRZ_R2_EN, MTRUE);
            fh_crzo_r2.Header_Enque(Header_CRZO_R1::E_CRZ_CROP_START,this->m_pDrv,((crz_in_roi.y << 16) | crz_in_roi.x));
            fh_crzo_r2.Header_Enque(Header_CRZO_R1::E_CRZ_CROP_SIZE,this->m_pDrv,((crz_in_roi.h << 16) | crz_in_roi.w));
            fh_crzo_r2.Header_Enque(Header_CRZO_R1::E_CRZ_DST_SIZE,this->m_pDrv,((crz_out_size.h << 16) | crz_out_size.w));
        }
        break;
    default:
        CAM_FUNC_ERR("%s: unsupport module(%d)",this->name_Str(),this->id());
        break;
    }

    Result = 0;

EXIT:
    //
    return Result;
}

MINT32 CAM_MRG_R13_CTRL::_config(void)
{
    MINT32 ret = 0;
    MSize tmp;
    capibility CamInfo(this->m_hwModule);
    tCAM_rst rst;

    CamInfo.GetCapibility(0, ENPipeQueryCmd_UNKNOWN, NormalPipe_InputInfo(), rst, E_CAM_SRAM_MRG);

    if (isThroughRRZ(this->m_pRaw_p->m_pDrv)) {
        tmp.w = CAM_READ_BITS(this->m_pRaw_p->m_pDrv, RRZ_R1_RRZ_OUT_IMG, RRZ_OUT_WD);
        tmp.h = CAM_READ_BITS(this->m_pRaw_p->m_pDrv, RRZ_R1_RRZ_OUT_IMG, RRZ_OUT_HT);
    }
    else {
        RectData<MUINT32> Bin;
        GET_SEP_BIN(this->m_pRaw_p->m_pDrv,Bin);
        tmp.w = ((CAM_READ_BITS(this->m_pRaw_p->m_pDrv,SEP_R1_SEP_CROP,SEP_END_X) - \
                 CAM_READ_BITS(this->m_pRaw_p->m_pDrv,SEP_R1_SEP_CROP,SEP_STR_X)) + 1) >> Bin.H;
        tmp.h = CAM_READ_BITS(this->m_pRaw_p->m_pDrv,SEP_R1_SEP_VSIZE,SEP_HT)  >> Bin.V;
    }

    /*R13*/
    CAM_WRITE_BITS(this->m_pRaw_p->m_pDrv, CAMCTL_R1_CAMCTL_EN3, CAMCTL_MRG_R13_EN, 1);
    CAM_WRITE_BITS(this->m_pRaw_p->m_pDrv, MRG_R13_MRG_CTL, MRG_SRAM_SIZE, rst.dlp.module.sram_mrg_r13);
    CAM_WRITE_BITS(this->m_pRaw_p->m_pDrv, MRG_R13_MRG_CTL, MRG_EDGE, 0xf);
    CAM_WRITE_BITS(this->m_pRaw_p->m_pDrv, MRG_R13_MRG_CTL, MRG_EDGE_SET, 0);
    CAM_WRITE_BITS(this->m_pRaw_p->m_pDrv, MRG_R13_MRG_CTL, MRG_EDGE, 0xf);

    //single_mode1 & 2 are using exclusive method.   signle_mode1 is main path, signle_mode2 is from other path
    CAM_WRITE_BITS(this->m_pRaw_p->m_pDrv, MRG_R13_MRG_CTL, MRG_SIG_MODE1, 1);
    CAM_WRITE_BITS(this->m_pRaw_p->m_pDrv, MRG_R13_MRG_CTL, MRG_SIG_MODE2, 0);

    // MRG cropping
    CAM_WRITE_BITS(this->m_pRaw_p->m_pDrv, MRG_R13_MRG_CROP, MRG_STR_X, 0);
    CAM_WRITE_BITS(this->m_pRaw_p->m_pDrv, MRG_R13_MRG_CROP, MRG_END_X, tmp.w - 1);
    CAM_WRITE_BITS(this->m_pRaw_p->m_pDrv, MRG_R13_MRG_VSIZE, MRG_HT, tmp.h);

    CAM_WRITE_BITS(this->m_pRaw_p->m_pDrv, MRG_R14_MRG_CTL, MRG_SRAM_SIZE, rst.dlp.module.sram_mrg_r14);
    CAM_WRITE_BITS(this->m_pRaw_p->m_pDrv, MRG_R14_MRG_CTL, MRG_EDGE, 0xf);
    CAM_WRITE_BITS(this->m_pRaw_p->m_pDrv, MRG_R14_MRG_CTL, MRG_EDGE_SET, 0);
    CAM_WRITE_BITS(this->m_pRaw_p->m_pDrv, MRG_R14_MRG_CTL, MRG_EDGE, 0xf);

    //single_mode1 & 2 are using exclusive method.   signle_mode1 is main path, signle_mode2 is from other path
    CAM_WRITE_BITS(this->m_pRaw_p->m_pDrv, MRG_R14_MRG_CTL, MRG_SIG_MODE1, 1);
    CAM_WRITE_BITS(this->m_pRaw_p->m_pDrv, MRG_R14_MRG_CTL, MRG_SIG_MODE2, 0);

    // MRG cropping
    CAM_WRITE_BITS(this->m_pRaw_p->m_pDrv, MRG_R14_MRG_CROP, MRG_STR_X, 0);
    CAM_WRITE_BITS(this->m_pRaw_p->m_pDrv, MRG_R14_MRG_CROP, MRG_END_X, tmp.w - 1);
    CAM_WRITE_BITS(this->m_pRaw_p->m_pDrv, MRG_R14_MRG_VSIZE, MRG_HT, tmp.h);

    return 0;
}

MINT32 CAM_CRP_R4_CTRL::_config(void)
{
    MINT32 ret = 0;
    MUINT32 tmp = 0;

    if (isThroughRRZ(this->m_pRaw_p->m_pDrv)) {
        tmp = CAM_READ_BITS(this->m_pRaw_p->m_pDrv, RRZ_R1_RRZ_OUT_IMG, RRZ_OUT_WD);
        CAM_WRITE_BITS(this->m_pRaw_p->m_pDrv,CRP_R4_CRP_X_POS,CRP_XSTART,0);
        CAM_WRITE_BITS(this->m_pRaw_p->m_pDrv,CRP_R4_CRP_X_POS,CRP_XEND,tmp - 1);

        tmp = CAM_READ_BITS(this->m_pRaw_p->m_pDrv, RRZ_R1_RRZ_OUT_IMG, RRZ_OUT_HT);

        CAM_WRITE_BITS(this->m_pRaw_p->m_pDrv,CRP_R4_CRP_Y_POS,CRP_YSTART,0);
        CAM_WRITE_BITS(this->m_pRaw_p->m_pDrv,CRP_R4_CRP_Y_POS,CRP_YEND,tmp - 1);
    }
    else {
        RectData<MUINT32> Bin;
        GET_SEP_BIN(this->m_pRaw_p->m_pDrv,Bin);
        E_PIX pix_mode = (E_PIX)(CAM_READ_BITS(this->m_pRaw_p->m_pDrv,CAMCTL_R1_CAMCTL_FMT_SEL,CAMCTL_PIX_BUS_SEPO) >> Bin.H);

        tmp = (CAM_READ_BITS(this->m_pRaw_p->m_pDrv,SEP_R1_SEP_CROP,SEP_END_X) - \
               CAM_READ_BITS(this->m_pRaw_p->m_pDrv,SEP_R1_SEP_CROP,SEP_STR_X)) + 1;

        tmp = tmp >> (Bin.H + (MUINT32)pix_mode);
        CAM_WRITE_BITS(this->m_pRaw_p->m_pDrv,CRP_R4_CRP_X_POS,CRP_XSTART,0);
        CAM_WRITE_BITS(this->m_pRaw_p->m_pDrv,CRP_R4_CRP_X_POS,CRP_XEND,tmp - 1);

        tmp = CAM_READ_BITS(this->m_pRaw_p->m_pDrv,SEP_R1_SEP_VSIZE,SEP_HT) >> Bin.V;

        CAM_WRITE_BITS(this->m_pRaw_p->m_pDrv,CRP_R4_CRP_Y_POS,CRP_YSTART,0);
        CAM_WRITE_BITS(this->m_pRaw_p->m_pDrv,CRP_R4_CRP_Y_POS,CRP_YEND,tmp - 1);
    }

    CAM_WRITE_BITS(this->m_pRaw_p->m_pDrv, CAMCTL_R1_CAMCTL_EN3, CAMCTL_CRP_R4_EN, 1);

    return ret;
}

MINT32 CAM_CRSP_CTRL::_config(void)
{
    MSize tmp;

    switch(mCRSP_RX) {
        case CRSP_R1:
            tmp.w = CAM_READ_BITS(this->m_pRaw_p->m_pDrv,CRP_R4_CRP_X_POS,CRP_XEND) - \
                    CAM_READ_BITS(this->m_pRaw_p->m_pDrv,CRP_R4_CRP_X_POS,CRP_XSTART);
            tmp.h = CAM_READ_BITS(this->m_pRaw_p->m_pDrv,CRP_R4_CRP_Y_POS,CRP_YEND) - \
                    CAM_READ_BITS(this->m_pRaw_p->m_pDrv,CRP_R4_CRP_Y_POS,CRP_YSTART);

            CAM_WRITE_BITS(this->m_pRaw_p->m_pDrv, CRSP_R1_CRSP_CTRL, CRSP_HORI_EN, 0);
            CAM_WRITE_BITS(this->m_pRaw_p->m_pDrv, CRSP_R1_CRSP_CTRL, CRSP_VERT_EN, 1);
            CAM_WRITE_BITS(this->m_pRaw_p->m_pDrv, CRSP_R1_CRSP_CTRL, CRSP_CROP_EN, 1);
            CAM_WRITE_BITS(this->m_pRaw_p->m_pDrv, CRSP_R1_CRSP_OUT_IMG, CRSP_WD, tmp.w);
            CAM_WRITE_BITS(this->m_pRaw_p->m_pDrv, CRSP_R1_CRSP_OUT_IMG, CRSP_HT, tmp.h);
            CAM_WRITE_BITS(this->m_pRaw_p->m_pDrv, CRSP_R1_CRSP_STEP_OFST, CRSP_STEP_X, 4);
            CAM_WRITE_BITS(this->m_pRaw_p->m_pDrv, CRSP_R1_CRSP_STEP_OFST, CRSP_STEP_Y, 4);
            CAM_WRITE_BITS(this->m_pRaw_p->m_pDrv, CRSP_R1_CRSP_STEP_OFST, CRSP_OFST_X, 0);
            CAM_WRITE_BITS(this->m_pRaw_p->m_pDrv, CRSP_R1_CRSP_STEP_OFST, CRSP_OFST_Y, 1);
            CAM_WRITE_BITS(this->m_pRaw_p->m_pDrv, CRSP_R1_CRSP_CROP_X, CRSP_CROP_XSTART, 0);
            CAM_WRITE_BITS(this->m_pRaw_p->m_pDrv, CRSP_R1_CRSP_CROP_Y, CRSP_CROP_YSTART, 0);
            CAM_WRITE_BITS(this->m_pRaw_p->m_pDrv, CRSP_R1_CRSP_CROP_X, CRSP_CROP_XEND, tmp.w -1);
            CAM_WRITE_BITS(this->m_pRaw_p->m_pDrv, CRSP_R1_CRSP_CROP_Y, CRSP_CROP_YEND, tmp.h -1);
            CAM_WRITE_BITS(this->m_pRaw_p->m_pDrv, CAMCTL_R1_CAMCTL_EN3, CAMCTL_CRSP_R1_EN, 1);
            break;
        case CRSP_R2:
            tmp.w = CAM_READ_BITS(this->m_pRaw_p->m_pDrv,CRZ_R2_CRZ_OUT_IMG, CRZ_OUT_WD);
            tmp.h = CAM_READ_BITS(this->m_pRaw_p->m_pDrv,CRZ_R2_CRZ_OUT_IMG, CRZ_OUT_HT);

            CAM_WRITE_BITS(this->m_pRaw_p->m_pDrv, CRSP_R2_CRSP_CTRL, CRSP_HORI_EN, 0);
            CAM_WRITE_BITS(this->m_pRaw_p->m_pDrv, CRSP_R2_CRSP_CTRL, CRSP_VERT_EN, 1);
            CAM_WRITE_BITS(this->m_pRaw_p->m_pDrv, CRSP_R2_CRSP_CTRL, CRSP_CROP_EN, 1);
            CAM_WRITE_BITS(this->m_pRaw_p->m_pDrv, CRSP_R2_CRSP_OUT_IMG, CRSP_WD, tmp.w);
            CAM_WRITE_BITS(this->m_pRaw_p->m_pDrv, CRSP_R2_CRSP_OUT_IMG, CRSP_HT, tmp.h);
            CAM_WRITE_BITS(this->m_pRaw_p->m_pDrv, CRSP_R2_CRSP_STEP_OFST, CRSP_STEP_X, 4);
            CAM_WRITE_BITS(this->m_pRaw_p->m_pDrv, CRSP_R2_CRSP_STEP_OFST, CRSP_STEP_Y, 4);
            CAM_WRITE_BITS(this->m_pRaw_p->m_pDrv, CRSP_R2_CRSP_STEP_OFST, CRSP_OFST_X, 0);
            CAM_WRITE_BITS(this->m_pRaw_p->m_pDrv, CRSP_R2_CRSP_STEP_OFST, CRSP_OFST_Y, 1);
            CAM_WRITE_BITS(this->m_pRaw_p->m_pDrv, CRSP_R2_CRSP_CROP_X, CRSP_CROP_XSTART, 0);
            CAM_WRITE_BITS(this->m_pRaw_p->m_pDrv, CRSP_R2_CRSP_CROP_Y, CRSP_CROP_YSTART, 0);
            CAM_WRITE_BITS(this->m_pRaw_p->m_pDrv, CRSP_R2_CRSP_CROP_X, CRSP_CROP_XEND, tmp.w -1);
            CAM_WRITE_BITS(this->m_pRaw_p->m_pDrv, CRSP_R2_CRSP_CROP_Y, CRSP_CROP_YEND, tmp.h -1);
            CAM_WRITE_BITS(this->m_pRaw_p->m_pDrv, CAMCTL_R1_CAMCTL_EN3, CAMCTL_CRSP_R2_EN, 1);
            break;
        default:
            CAM_FUNC_ERR("%s: unsupport module(%d)", this->name_Str(), this->id());
            break;
    }

    return 0;
}

/*/////////////////////////////////////////////////////////////////////////////
  ISP_DYUV_PIPE
/////////////////////////////////////////////////////////////////////////////*/

MBOOL CAM_DYUV_PIPE::_config()
{
    E_ISP_CAM_CQ cq;
    MUINT32 page = 0;

    this->m_pDrv = this->m_pMainPipe->m_pDrv;
    this->m_pDrv->getCurObjInfo(&this->m_hwModule,&cq,&page);

    if (page == 0) {
        CAM_FUNC_DBG("ISP_DYUV_PIPE::_config+ cq:0x%x,page:0x%x\n",cq,page);
    }

    if (this->m_hwModule == CAM_C) {
        CAM_FUNC_WRN("CAM_C is not support DYUV\n");
        return MFALSE;
    }

    CAM_WRITE_BITS(this->m_pDrv, CAMCTL_R1_CAMCTL_EN2, CAMCTL_CCM_R1_EN, 1);
    CAM_WRITE_BITS(this->m_pDrv, CAMCTL_R1_CAMCTL_EN2, CAMCTL_GGM_R1_EN, 1);
    CAM_WRITE_BITS(this->m_pDrv, CAMCTL_R1_CAMCTL_EN3, CAMCTL_GGM_R2_EN, 1);
    CAM_WRITE_BITS(this->m_pDrv, CAMCTL_R1_CAMCTL_EN2, CAMCTL_G2C_R1_EN, 1);
    CAM_WRITE_BITS(this->m_pDrv, CAMCTL_R1_CAMCTL_EN3, CAMCTL_G2C_R2_EN, 1);
    CAM_WRITE_BITS(this->m_pDrv, CAMCTL_R1_CAMCTL_EN2, CAMCTL_DM_R1_EN, 1);
    CAM_WRITE_BITS(this->m_pDrv, CAMCTL_R1_CAMCTL_EN2, CAMCTL_C42_R1_EN, 1);
    CAM_WRITE_BITS(this->m_pDrv, CAMCTL_R1_CAMCTL_EN3, CAMCTL_C42_R2_EN, 1);
    CAM_WRITE_BITS(this->m_pDrv, CAMCTL_R1_CAMCTL_EN3, CAMCTL_PLNW_R3_EN, 1);
    CAM_WRITE_BITS(this->m_pDrv, CAMCTL_R1_CAMCTL_EN3, CAMCTL_PLNW_R2_EN, 1);
    CAM_WRITE_BITS(this->m_pDrv, CAMCTL_R1_CAMCTL_EN3, CAMCTL_PLNW_R1_EN, 1);
    CAM_WRITE_BITS(this->m_pDrv, CAMCTL_R1_CAMCTL_EN3, CAMCTL_PAK_R3_EN, 1);
    CAM_WRITE_BITS(this->m_pDrv, CAMCTL_R1_CAMCTL_EN3, CAMCTL_PAK_R4_EN, 1);
    CAM_WRITE_BITS(this->m_pDrv, CAMCTL_R1_CAMCTL_EN3, CAMCTL_PAK_R5_EN, 1);
    CAM_WRITE_BITS(this->m_pDrv, CAMCTL_R1_CAMCTL_EN, CAMCTL_BS_R1_EN, 1);
    CAM_WRITE_BITS(this->m_pDrv, CAMCTL_R1_CAMCTL_EN, CAMCTL_BS_R2_EN, 1);
    CAM_WRITE_BITS(this->m_pDrv, CAMCTL_R1_CAMCTL_EN, CAMCTL_BS_R3_EN, 1);

    /*DM_R1*/
    CAM_WRITE_BITS(this->m_pDrv, DM_R1_DM_TILE_EDGE, DM_TILE_EDGE, 0XF);

    /*C42*/
    CAM_WRITE_BITS(this->m_pDrv, C42_R1_C42_CON, C42_TDR_EDGE, 0xF);
    CAM_WRITE_BITS(this->m_pDrv, C42_R1_C42_CON, C42_FILT_DIS, 0x1);
    CAM_WRITE_BITS(this->m_pDrv, C42_R2_C42_CON, C42_TDR_EDGE, 0xF);
    CAM_WRITE_BITS(this->m_pDrv, C42_R2_C42_CON, C42_FILT_DIS, 0x1);

    this->m_MrgR13.config(this);
    this->m_CrpR4.config(this);

    /*CRSP_R1*/
    if(!isYUV422(_yuvo_)) {
        unique_ptr<CAM_CRSP_CTRL> pCrspCtl(new CAM_CRSP_CTRL(CAM_CRSP_CTRL::CRSP_R1));
        pCrspCtl->config(this);
    }

    /*CRSP_R2*/
    if(!isYUV422(_crzo_r2_)) {
        unique_ptr<CAM_CRSP_CTRL> pCrspCtl(new CAM_CRSP_CTRL(CAM_CRSP_CTRL::CRSP_R2));
        pCrspCtl->config(this);
    }

    return MTRUE;
}

MBOOL CAM_DYUV_PIPE::_write2CQ()
{
    E_ISP_CAM_CQ cq;
    MUINT32 page;

    this->m_pDrv = this->m_pMainPipe->m_pDrv;
    this->m_pDrv->getCurObjInfo(&this->m_hwModule,&cq,&page);
    CAM_FUNC_DBG("CAM_DYUV_PIPE::_write2CQ+ cq:0x%x,page:0x%x\n",cq,page);

    XOR_ADD_MODULE(this->m_hwModule,this->m_pDrv,CAM_ISP_CRZ_);
    XOR_ADD_MODULE(this->m_hwModule,this->m_pDrv,CAM_ISP_CRZ_R2_);
    XOR_ADD_MODULE(this->m_hwModule,this->m_pDrv,CAM_ISP_MRG_R13_);
    XOR_ADD_MODULE(this->m_hwModule,this->m_pDrv,CAM_ISP_DM_);
    XOR_ADD_MODULE(this->m_hwModule,this->m_pDrv,CAM_ISP_CCM_);
    XOR_ADD_MODULE(this->m_hwModule,this->m_pDrv,CAM_ISP_GGM_);
    XOR_ADD_MODULE(this->m_hwModule,this->m_pDrv,CAM_ISP_GGM_PINGPONG_);
    XOR_ADD_MODULE(this->m_hwModule,this->m_pDrv,CAM_ISP_YNRS_);
    XOR_ADD_MODULE(this->m_hwModule,this->m_pDrv,CAM_ISP_YNRS_PINGPONG_);
    XOR_ADD_MODULE(this->m_hwModule,this->m_pDrv,CAM_ISP_FLC_);
    XOR_ADD_MODULE(this->m_hwModule,this->m_pDrv,CAM_ISP_G2C_);
    XOR_ADD_MODULE(this->m_hwModule,this->m_pDrv,CAM_ISP_C42_);
    XOR_ADD_MODULE(this->m_hwModule,this->m_pDrv,CAM_ISP_CRSP_);
    XOR_ADD_MODULE(this->m_hwModule,this->m_pDrv,CAM_ISP_CRP_R4_);
    XOR_ADD_MODULE(this->m_hwModule,this->m_pDrv,CAM_ISP_PAK_R3_);
    XOR_ADD_MODULE(this->m_hwModule,this->m_pDrv,CAM_ISP_PAK_R4_);
    XOR_ADD_MODULE(this->m_hwModule,this->m_pDrv,CAM_ISP_PAK_R5_);
    XOR_ADD_MODULE(this->m_hwModule,this->m_pDrv,CAM_ISP_MRG_R14_);
    XOR_ADD_MODULE(this->m_hwModule,this->m_pDrv,CAM_ISP_GGM_R2_);
    XOR_ADD_MODULE(this->m_hwModule,this->m_pDrv,CAM_ISP_GGM_R2_PINGPONG_);
    XOR_ADD_MODULE(this->m_hwModule,this->m_pDrv,CAM_ISP_SLK_R2_CEN);
    XOR_ADD_MODULE(this->m_hwModule,this->m_pDrv,CAM_ISP_SLK_R2_CON);
    XOR_ADD_MODULE(this->m_hwModule,this->m_pDrv,CAM_ISP_G2C_R2_);
    XOR_ADD_MODULE(this->m_hwModule,this->m_pDrv,CAM_ISP_C42_R2_);
    XOR_ADD_MODULE(this->m_hwModule,this->m_pDrv,CAM_ISP_CRSP_R2_);
    XOR_ADD_MODULE(this->m_hwModule,this->m_pDrv,CAM_ISP_RSS_R2_);
    XOR_ADD_MODULE(this->m_hwModule,this->m_pDrv,CAM_ISP_SLK_CEN);
    XOR_ADD_MODULE(this->m_hwModule,this->m_pDrv,CAM_ISP_SLK_CON);
    XOR_ADD_MODULE(this->m_hwModule,this->m_pDrv,CAM_ISP_SLK_XOFF);
    XOR_ADD_MODULE(this->m_hwModule,this->m_pDrv,CAM_ISP_SLK_SIZE);
    XOR_ADD_MODULE(this->m_hwModule,this->m_pDrv,CAM_ISP_SLK_R2_CEN);
    XOR_ADD_MODULE(this->m_hwModule,this->m_pDrv,CAM_ISP_SLK_R2_CON);
    XOR_ADD_MODULE(this->m_hwModule,this->m_pDrv,CAM_ISP_SLK_R2_XOFF);
    XOR_ADD_MODULE(this->m_hwModule,this->m_pDrv,CAM_ISP_SLK_R2_SIZE);

    //NOP function , these func should apb when being used at tuning
    this->m_pDrv->cqNopModule(CAM_ISP_DM_);
    this->m_pDrv->cqNopModule(CAM_ISP_FLC_);
    this->m_pDrv->cqNopModule(CAM_ISP_CCM_);
    this->m_pDrv->cqNopModule(CAM_ISP_YNRS_);
    this->m_pDrv->cqNopModule(CAM_ISP_YNRS_PINGPONG_);
    this->m_pDrv->cqNopModule(CAM_ISP_GGM_);
    this->m_pDrv->cqNopModule(CAM_ISP_GGM_PINGPONG_);
    this->m_pDrv->cqNopModule(CAM_ISP_GGM_R2_);
    this->m_pDrv->cqNopModule(CAM_ISP_GGM_R2_PINGPONG_);
    this->m_pDrv->cqNopModule(CAM_ISP_G2C_);
    this->m_pDrv->cqNopModule(CAM_ISP_G2C_R2_);
    this->m_pDrv->cqNopModule(CAM_ISP_SLK_R2_CEN);
    this->m_pDrv->cqNopModule(CAM_ISP_SLK_R2_CON);
    this->m_pDrv->cqNopModule(CAM_ISP_RSS_R2_);
    this->m_pDrv->cqNopModule(CAM_ISP_SLK_CEN);
    this->m_pDrv->cqNopModule(CAM_ISP_SLK_CON);
    this->m_pDrv->cqNopModule(CAM_ISP_SLK_XOFF);
    this->m_pDrv->cqNopModule(CAM_ISP_SLK_SIZE);
    this->m_pDrv->cqNopModule(CAM_ISP_SLK_R2_CEN);
    this->m_pDrv->cqNopModule(CAM_ISP_SLK_R2_CON);
    this->m_pDrv->cqNopModule(CAM_ISP_SLK_R2_XOFF);
    this->m_pDrv->cqNopModule(CAM_ISP_SLK_R2_SIZE);

    return MTRUE;
}

MBOOL CAM_DYUV_PIPE::dynamicPak(MUINT32 hwYuvoFmt, Pak_Func _Func)
{
    E_ISP_CAM_CQ cq;
    MUINT32 page;

    this->m_pDrv->getCurObjInfo(&this->m_hwModule,&cq,&page);

    if(this->m_hwModule == CAM_C) {
        CAM_FUNC_ERR("CAM_C is not support DYUV\n");
        return MFALSE;
    }

    CAM_WRITE_BITS(this->m_pDrv, PAK_R3_PAK_CONT, PAK_SWAP_ODR, _Func.Bits.PAK_R3_ODR);
    CAM_WRITE_BITS(this->m_pDrv, PAK_R3_PAK_CONT, PAK_UV_SIGN, 0);
    CAM_WRITE_BITS(this->m_pDrv, PAK_R3_PAK_CONT, PAK_YUV_BIT, _Func.Bits.PAK_BIT);
    CAM_WRITE_BITS(this->m_pDrv, PAK_R3_PAK_CONT, PAK_YUV_DNG, _Func.Bits.PAK_DNG);
    CAM_WRITE_BITS(this->m_pDrv, PAK_R3_PAK_CONT, PAK_OBIT, 0);
    CAM_WRITE_BITS(this->m_pDrv, PAK_R4_PAK_CONT, PAK_SWAP_ODR, _Func.Bits.PAK_R4_ODR);
    CAM_WRITE_BITS(this->m_pDrv, PAK_R4_PAK_CONT, PAK_UV_SIGN, 0);
    CAM_WRITE_BITS(this->m_pDrv, PAK_R4_PAK_CONT, PAK_YUV_BIT, _Func.Bits.PAK_BIT);
    CAM_WRITE_BITS(this->m_pDrv, PAK_R4_PAK_CONT, PAK_YUV_DNG, _Func.Bits.PAK_DNG);
    CAM_WRITE_BITS(this->m_pDrv, PAK_R4_PAK_CONT, PAK_OBIT, 0);

    CAM_WRITE_BITS(this->m_pDrv, CAMCTL_R1_CAMCTL_FMT2_SEL, CAMCTL_YUVO_R1_FMT, hwYuvoFmt);

    CAM_FUNC_DBG("[%d_%d]ISP_DYUV_PIPE::dynamicPak- YUVO_FMT:0x%x, PAK_R3:0x%x_0x%x, PAK_R4:0x%x_0x%x\n",
        cq, page, hwYuvoFmt,
        CAM_READ_REG(this->m_pDrv,PAK_R3_PAK_CONT),
        CAM_READ_REG(this->m_pDrv,PAK_R3_PAK_CONT),
        CAM_READ_REG(this->m_pDrv,PAK_R4_PAK_CONT),
        CAM_READ_REG(this->m_pDrv,PAK_R4_PAK_CONT));

    return MTRUE;
}

MUINT32 CAM_DYUV_PIPE::getCurPixMode(_isp_dma_enum_ dmao)
{
    MUINT32 pix_mode = CAM_READ_BITS(this->m_pDrv,CAMCTL_R1_CAMCTL_FMT_SEL,CAMCTL_PIX_BUS_SEPO);
    RectData<MUINT32> Bin;
    GET_SEP_BIN(this->m_pDrv, Bin);

    switch(dmao){
        case _yuvo_:
        case _crzo_:
        case _crzbo_:
        case _crzo_r2_:
        case _crzbo_r2_:
            pix_mode = pix_mode >> Bin.H;
            break;
        default:
            CAM_FUNC_ERR("unsupported dmao:0x%x,return unknown-pix\n",dmao);
            return ePixMode_1;
            break;
    }
    return MAP_CAM_PIXMODE((E_PIX)pix_mode,CAM_FUNC_ERR);
}

MVOID CAM_DYUV_PIPE::updateMrgInSize()
{
    MSize tmp;
    E_ISP_CAM_CQ cq;
    MUINT32 page = 0;

    this->m_pDrv = this->m_pMainPipe->m_pDrv;
    this->m_pDrv->getCurObjInfo(&this->m_hwModule,&cq,&page);

    if (isThroughRRZ(this->m_pDrv)) {
        tmp.w = CAM_READ_BITS(this->m_pDrv, RRZ_R1_RRZ_OUT_IMG, RRZ_OUT_WD);
        tmp.h = CAM_READ_BITS(this->m_pDrv, RRZ_R1_RRZ_OUT_IMG, RRZ_OUT_HT);
        // MRG_R13 cropping
        CAM_WRITE_BITS(this->m_pDrv, MRG_R13_MRG_CROP, MRG_STR_X, 0);
        CAM_WRITE_BITS(this->m_pDrv, MRG_R13_MRG_CROP, MRG_END_X, tmp.w - 1);
        CAM_WRITE_BITS(this->m_pDrv, MRG_R13_MRG_VSIZE, MRG_HT, tmp.h);
    }

    CAM_FUNC_DBG("updateMrgInSize: isThroughRRZ_%d Size_0x%x_0x%x ",isThroughRRZ(this->m_pDrv),tmp.w,tmp.h);
}

MBOOL CAM_DYUV_PIPE::setCRZ(MUINT32 RX)
{
    MBOOL ret = MTRUE;
    CRZ_REG_CFG _crz_cfg;
    E_ISP_CAM_CQ cq;
    MUINT32 page = 0;
    unique_ptr<CAM_CRZ_CTRL> pCrzCtl(new CAM_CRZ_CTRL((CAM_CRZ_CTRL::E_CRZ_RX)RX, this->m_pMainPipe, MTRUE));

    this->m_pDrv = m_pMainPipe->m_pDrv;
    this->m_pDrv->getCurObjInfo(&this->m_hwModule,&cq,&page);
    CAM_FUNC_DBG("[setCRZ] %s: cq:0x%x,page:0x%x\n",pCrzCtl->name_Str(),cq,page);

    if(this->m_hwModule == CAM_C) {
        CAM_FUNC_ERR("simple raw_c doesn't support crz");
        goto EXIT;
    }

    _crz_cfg.bCRZ_Bypass = MTRUE;
    pCrzCtl->updateCrzInSize(); //rrz out size might change by setRRZ, need update here for m_input
    this->m_pMainPipe->m_pP1Tuning->p1TuningNotify((MVOID*)&m_pMainPipe->m_input,(MVOID*)&_crz_cfg);

    CAM_FUNC_INF("setCRZ[%ld]: bypass_crz(%d) rrz_out_w/h(%ld/%ld) crz_in_w/h(%ld/%ld) crz_src_x/y/w/h(%ld/%ld/%ld/%ld) crz_tar_w/h(%ld/%ld)\n",\
                    pCrzCtl->id(), _crz_cfg.bCRZ_Bypass,\
                    m_pMainPipe->m_input.CurRrzCfg.tar_w, m_pMainPipe->m_input.CurRrzCfg.tar_h,\
                    m_pMainPipe->m_input.CrzIn_W, m_pMainPipe->m_input.CrzIn_H,\
                    _crz_cfg.src_x, _crz_cfg.src_y, _crz_cfg.src_w, _crz_cfg.src_h,\
                    _crz_cfg.tar_w, _crz_cfg.tar_h);

    if (_crz_cfg.bCRZ_Bypass == MFALSE) {
        MUINT32 PlaneNum = 0;
        MBOOL cap = MFALSE;
        capibility CamInfo(this->m_hwModule);
        tCAM_rst rst;
        ENPipeQueryCmd Qcmd = (ENPipeQueryCmd)(ENPipeQueryCmd_CROP_X_PIX | ENPipeQueryCmd_X_BYTE);
        E_CamPixelMode pixMode = (E_CamPixelMode)this->getCurPixMode(_crzo_);
        unique_ptr<DMAO_B> pCrzoCtl[ePlane_max];

        pCrzCtl->crz_in_roi.x   = _crz_cfg.src_x;
        pCrzCtl->crz_in_roi.y   = _crz_cfg.src_y;
        pCrzCtl->crz_in_roi.w   = _crz_cfg.src_w;
        pCrzCtl->crz_in_roi.h   = _crz_cfg.src_h;
        pCrzCtl->crz_out_size.w = _crz_cfg.tar_w;
        pCrzCtl->crz_out_size.h = _crz_cfg.tar_h;

        switch(pCrzCtl->id()) {
            case CAM_CRZ_CTRL::CRZ_R1:
                {
                    Header_CRZO_R1 fh_crzo_r1;
                    pCrzoCtl[ePlane_1st].reset((DMAO_B*)new DMA_CRZO_R1());
                    pCrzoCtl[ePlane_1st]->dma_cfg.size.h       = pCrzCtl->crz_out_size.h;
                    pCrzoCtl[ePlane_1st]->dma_cfg.size.stride  = CAM_READ_REG(this->m_pDrv,CRZO_R1_CRZO_STRIDE);
                    pCrzoCtl[ePlane_1st]->dma_cfg.lIspColorfmt = (EImageFormat)fh_crzo_r1.GetRegInfo(Header_CRZO_R1::E_SW_FMT,this->m_pDrv);
                    if (MFALSE == CamInfo.GetCapibility(EPortIndex_CRZO,
                                                        Qcmd,
                                                        NormalPipe_InputInfo(pCrzoCtl[ePlane_1st]->dma_cfg.lIspColorfmt, pCrzCtl->crz_out_size.w, pixMode),
                                                        rst)) {
                        CAM_FUNC_ERR("%s size err(0x%x)\n", pCrzCtl->name_Str(), pCrzCtl->crz_out_size.w);
                        ret = MFALSE;
                    }
                }
                break;
            case CAM_CRZ_CTRL::CRZ_R2:
                {
                    Header_CRZO_R2 fh_crzo_r2;
                    pCrzoCtl[ePlane_1st].reset((DMAO_B*)new DMA_CRZO_R2());
                    pCrzoCtl[ePlane_1st]->dma_cfg.size.h       = pCrzCtl->crz_out_size.h;
                    pCrzoCtl[ePlane_1st]->dma_cfg.size.stride  = CAM_READ_REG(this->m_pDrv,CRZO_R2_CRZO_STRIDE);
                    pCrzoCtl[ePlane_1st]->dma_cfg.lIspColorfmt = (EImageFormat)fh_crzo_r2.GetRegInfo(Header_CRZO_R2::E_SW_FMT,this->m_pDrv);
                    if (MFALSE == CamInfo.GetCapibility(EPortIndex_CRZO_R2,
                                                        Qcmd,
                                                        NormalPipe_InputInfo(pCrzoCtl[ePlane_1st]->dma_cfg.lIspColorfmt, pCrzCtl->crz_out_size.w, pixMode),
                                                        rst)) {
                        CAM_FUNC_ERR("%s size err(0x%x)\n", pCrzCtl->name_Str(), pCrzCtl->crz_out_size.w);
                        ret = MFALSE;
                    }
                }
                break;
            default:
                CAM_FUNC_ERR("%s: module is not supported(%d)\n",pCrzCtl->name_Str(),pCrzCtl->id());
                break;
        }

        pCrzoCtl[ePlane_1st]->dma_cfg.size.xsize = rst.xsize_byte[0];
        pCrzoCtl[ePlane_1st]->m_pIspDrv = (IspDrvVir*)this->m_pDrv;
        pCrzoCtl[ePlane_1st]->config();
        pCrzoCtl[ePlane_1st]->write2CQ(0);
        pCrzoCtl[ePlane_1st]->enable(NULL);

        if (GetPlaneMax(pCrzCtl->id()) == ePlane_2nd) {
            switch(pCrzCtl->id()) {
                case CAM_CRZ_CTRL::CRZ_R1:
                    {
                        pCrzoCtl[ePlane_2nd].reset((DMAO_B*)new DMA_CRZBO_R1());
                        pCrzoCtl[ePlane_2nd]->dma_cfg.size.xsize  = rst.xsize_byte[1];
                        pCrzoCtl[ePlane_2nd]->dma_cfg.size.stride = CAM_READ_REG(this->m_pDrv,CRZBO_R1_CRZBO_STRIDE);
                        pCrzoCtl[ePlane_2nd]->dma_cfg.size.h      = pCrzoCtl[ePlane_1st]->dma_cfg.size.h;
                    }
                    break;
                case CAM_CRZ_CTRL::CRZ_R2:
                    {
                        pCrzoCtl[ePlane_2nd].reset((DMAO_B*)new DMA_CRZBO_R2());
                        pCrzoCtl[ePlane_2nd]->dma_cfg.size.xsize  = rst.xsize_byte[1];
                        pCrzoCtl[ePlane_2nd]->dma_cfg.size.stride = CAM_READ_REG(this->m_pDrv,CRZBO_R2_CRZBO_STRIDE);
                        pCrzoCtl[ePlane_2nd]->dma_cfg.size.h      = pCrzoCtl[ePlane_1st]->dma_cfg.size.h;
                    }
                    break;
                default:
                    CAM_FUNC_ERR("%s: module is not supported(%d)\n",pCrzCtl->name_Str(),pCrzCtl->id());
                    break;
            }
            pCrzoCtl[ePlane_2nd]->m_pIspDrv = (IspDrvVir*)this->m_pDrv;
            pCrzoCtl[ePlane_2nd]->config();
            pCrzoCtl[ePlane_2nd]->write2CQ(0);
            pCrzoCtl[ePlane_2nd]->enable(NULL);
        }

        pCrzCtl->config();

        /*CRSP_R2*/
        if (pCrzCtl->id() == CAM_CRZ_CTRL::CRZ_R2) {
            if (!isYUV422(_crzo_r2_)) {
                unique_ptr<CAM_CRSP_CTRL> pCrspCtl(new CAM_CRSP_CTRL(CAM_CRSP_CTRL::CRSP_R2));
                pCrspCtl->config(this);
            }
        }
    }

EXIT:

    return ret;
}

