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


    //hds setting is in UNI_TOP


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


    this->m_pDrv->cqAddModule(CAM_ISP_LCS_);
    this->m_pDrv->cqAddModule(CAM_ISP_SGG1_);
    this->m_pDrv->cqAddModule(CAM_ISP_SGG2_);
    this->m_pDrv->cqAddModule(CAM_ISP_SGG5_);
    this->m_pDrv->cqAddModule(CAM_ISP_AF_);


    //NOP function , these func should apb when being used at tuning
    this->m_pDrv->cqNopModule(CAM_ISP_LCS_);
    this->m_pDrv->cqNopModule(CAM_ISP_SGG1_);
    this->m_pDrv->cqNopModule(CAM_ISP_SGG2_);
    this->m_pDrv->cqNopModule(CAM_ISP_SGG5_);
    this->m_pDrv->cqNopModule(CAM_ISP_AF_);

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

        CAM_REG_TG_SEN_GRAB_PXL TG_W;
        CAM_REG_TG_SEN_GRAB_LIN TG_H;

        input.pixMode = CAM_READ_BITS(this->m_pDrv,CAM_HDS_MODE,HDS_DS);


        TG_W.Raw = CAM_READ_REG(this->m_pDrv->getPhyObj(),CAM_TG_SEN_GRAB_PXL);
        input.sTGOut.w = TG_W.Bits.PXL_E - TG_W.Bits.PXL_S;
        TG_H.Raw = CAM_READ_REG(this->m_pDrv->getPhyObj(),CAM_TG_SEN_GRAB_LIN);
        input.sTGOut.h = TG_H.Bits.LIN_E - TG_H.Bits.LIN_S;

        //
        input.sHBINOut.w = input.sTGOut.w >> (CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_TG_SEN_MODE,DBL_DATA_BUS) +
                    CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_TG_SEN_MODE,DBL_DATA_BUS1) + 1);//+1 is because of YUV fmt
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

                CAM_BURST_WRITE_REGS(this->m_pDrv,CAM_EIS_PREP_ME_CTRL1,pReg,modulesize);
                CAM_WRITE_BITS(this->m_pDrv, CAM_CTL_EN2, LMV_EN, 1);

                lmvo.m_pIspDrv = this->m_pIspDrv;

                lmvo.config();
                lmvo.write2CQ();
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

MBOOL CAM_YUV_PIPE::setRSS(void)
{
    return MFALSE;
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
        #if 0
        if(pixMode != 0){
            CAM_FUNC_ERR("LCS support no 2/4 pixmode under YUV format\n");
            return MFALSE;
        }
        #endif
        input.sHBINOut.w = ((TG_W.Bits.PXL_E - TG_W.Bits.PXL_S) >> (pixMode+1));
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
        LMV_SGG_CFG lmv_sgg_cfg;

        CAM_FUNC_DBG("CAM_YUV_PIPE::setSGG2:\n");
        this->m_pP1Tuning->p1TuningNotify(NULL,(MVOID*)&lmv_sgg_cfg);

        CAM_FUNC_DBG("-\n");

        if(lmv_sgg_cfg.bSGG2_Bypass == 0){
            CAM_WRITE_REG(this->m_pDrv,CAM_SGG2_PGN,lmv_sgg_cfg.PGN);
            CAM_WRITE_REG(this->m_pDrv,CAM_SGG2_GMRC_1,lmv_sgg_cfg.GMRC_1);
            CAM_WRITE_REG(this->m_pDrv,CAM_SGG2_GMRC_2,lmv_sgg_cfg.GMRC_2);

            this->m_pDrv->cqApbModule(CAM_ISP_SGG2_);
        }
        else{
            this->m_pDrv->cqNopModule(CAM_ISP_SGG2_);
        }

        if(lmv_sgg_cfg.bSGG2_EN){
            CAM_WRITE_BITS(this->m_pDrv, CAM_CTL_EN2, SGG2_EN, 1);
        }
        else{
            CAM_WRITE_BITS(this->m_pDrv, CAM_CTL_EN2, SGG2_EN, 0);
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
                return ePixMode_4;
            else if(CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_TG_SEN_MODE,DBL_DATA_BUS) == 1)
                return ePixMode_2;
            else
                return ePixMode_1;
            break;
        default:
            CAM_FUNC_ERR("unsupported dmao:0x%x,return unknown-pix\n",dmao);
            return ePixMode_NONE;
            break;
    }
}
