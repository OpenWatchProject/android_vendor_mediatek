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

#define LOG_TAG "ifunc_cam_uni"

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


    //default always enabe  hds,  GSE is binding with FullG
    UNI_WRITE_BITS(this->m_pDrv,CAM_UNI_TOP_MOD_EN,HDS_A_EN,1);

    //default always enabe for rrz sram share needs
    UNI_WRITE_BITS(this->m_pDrv,CAM_UNI_TOP_MOD_EN,RLB_A_EN,1);

    //avoid YUV / Bayer format switch
    if (CAM_READ_BITS(this->m_pDrv,CAM_CTL_SEL,HDS1_SEL) == 1) {
        if(CAM_READ_BITS(this->m_pDrv,CAM_CTL_FMT_SEL,RRZO_FG_MODE) == 1){
            CAM_WRITE_BITS(this->m_pDrv,CAM_CTL_DMA_EN,GSE_EN,1);
        }
        else {
            CAM_WRITE_BITS(this->m_pDrv,CAM_CTL_DMA_EN,GSE_EN,0);
        }
    }
    else {
        CAM_WRITE_BITS(this->m_pDrv,CAM_CTL_DMA_EN,GSE_EN,0);
    }


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

            CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),CAM_CTL_EN,TG_EN,0);
            break;
        case IF_CAM_A:
        case IF_CAM_B:
            //hds
            _tmp = CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAM_CTL_FMT_SEL,PIX_BUS_RMXO);
            UNI_WRITE_BITS(this->m_pDrv,CAM_UNI_HDS_A_MODE,HDS_DS,_tmp);

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

    /* clear all bits except RLB_A_EN */
    UNI_WRITE_BITS(this->m_pDrv,CAM_UNI_TOP_MOD_EN,UNP2_A_EN,0);
    UNI_WRITE_BITS(this->m_pDrv,CAM_UNI_TOP_MOD_EN,SGG3_A_EN,0);
    UNI_WRITE_BITS(this->m_pDrv,CAM_UNI_TOP_MOD_EN,FLK_A_EN,0);
    UNI_WRITE_BITS(this->m_pDrv,CAM_UNI_TOP_MOD_EN,EIS_A_EN,0);
    UNI_WRITE_BITS(this->m_pDrv,CAM_UNI_TOP_MOD_EN,HDS_A_EN,0);
    UNI_WRITE_BITS(this->m_pDrv,CAM_UNI_TOP_MOD_EN,RSS_A_EN,0);
    UNI_WRITE_BITS(this->m_pDrv,CAM_UNI_TOP_MOD_EN,ADL_A_EN,0);
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
                /* clear all bits except RLB_A_EN */
                UNI_WRITE_BITS(this->m_pUniDrv,CAM_UNI_TOP_MOD_EN,UNP2_A_EN,0);
                UNI_WRITE_BITS(this->m_pUniDrv,CAM_UNI_TOP_MOD_EN,SGG3_A_EN,0);
                UNI_WRITE_BITS(this->m_pUniDrv,CAM_UNI_TOP_MOD_EN,FLK_A_EN,0);
                UNI_WRITE_BITS(this->m_pUniDrv,CAM_UNI_TOP_MOD_EN,EIS_A_EN,0);
                UNI_WRITE_BITS(this->m_pUniDrv,CAM_UNI_TOP_MOD_EN,HDS_A_EN,0);
                UNI_WRITE_BITS(this->m_pUniDrv,CAM_UNI_TOP_MOD_EN,RSS_A_EN,0);
                UNI_WRITE_BITS(this->m_pUniDrv,CAM_UNI_TOP_MOD_EN,ADL_A_EN,0);
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
                /* clear all bits except RLB_A_EN */
                UNI_WRITE_BITS(this->m_pUniDrv,CAM_UNI_TOP_MOD_EN,UNP2_A_EN,0);
                UNI_WRITE_BITS(this->m_pUniDrv,CAM_UNI_TOP_MOD_EN,SGG3_A_EN,0);
                UNI_WRITE_BITS(this->m_pUniDrv,CAM_UNI_TOP_MOD_EN,FLK_A_EN,0);
                UNI_WRITE_BITS(this->m_pUniDrv,CAM_UNI_TOP_MOD_EN,EIS_A_EN,0);
                UNI_WRITE_BITS(this->m_pUniDrv,CAM_UNI_TOP_MOD_EN,HDS_A_EN,0);
                UNI_WRITE_BITS(this->m_pUniDrv,CAM_UNI_TOP_MOD_EN,RSS_A_EN,0);
                UNI_WRITE_BITS(this->m_pUniDrv,CAM_UNI_TOP_MOD_EN,ADL_A_EN,0);
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
            this->m_pDrv->cqDelModule(UNI_ISP_SGG3_);
            this->m_pDrv->cqDelModule(UNI_ISP_FLK_);
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
    this->m_pDrv->cqAddModule(UNI_ISP_SGG3_);
    this->m_pDrv->cqAddModule(UNI_ISP_FLK_);
    this->m_pDrv->cqAddModule(UNI_ISP_HDS_A_);
    this->m_pDrv->cqAddModule(UNI_ISP_EIS_);
    this->m_pDrv->cqAddModule(UNI_ISP_RSS_A_);


    //this->m_pDrv->cqNopModule(UNI_ISP_HDS_A_);  hds is binding with tg pixel mode


    this->m_pDrv->cqNopModule(UNI_ISP_EIS_);
    this->m_pDrv->cqNopModule(UNI_ISP_RSS_A_);
    this->m_pDrv->cqNopModule(UNI_ISP_SGG3_);
    this->m_pDrv->cqNopModule(UNI_ISP_FLK_);

    //
    CAM_FUNC_DBG("-\n");

    return 0;
}


//set 0:stop pipeline. set 1:restart pipeline
MBOOL UNI_TOP_CTRL::HW_recover(MUINT32 step)
{
    switch(step){
        case 0:
            {
                MBOOL rst = MTRUE;
                MUINT32 i=0;
                this->m_pDrv = (ISP_DRV_CAM*)this->m_pIspDrv;

                CAM_FUNC_INF("UNI_TOP_CTRL::stop & reset + \n");

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

                while((UNI_READ_REG(this->m_pUniDrv,CAM_UNI_TOP_SW_CTL) != 0x222) && i++ < 10000){
                    CAM_FUNC_DBG("UNI reseting...CAM_UNI_TOP_SW_CTL:0x%x\n", UNI_READ_REG(this->m_pUniDrv,CAM_UNI_TOP_SW_CTL));
                }

                UNI_WRITE_REG(this->m_pUniDrv,CAM_UNI_TOP_SW_CTL,0x0444);
                UNI_WRITE_REG(this->m_pUniDrv,CAM_UNI_TOP_SW_CTL,0x0);


                CAM_FUNC_DBG("UNI_TOP_CTRL::- \n");
                return rst;

            }
            break;
        case 1:
            {
                MUINT32 frm_cnt = 0;

                CAM_FUNC_INF("UNI_TOP_CTRL::restart + \n");

                CAM_FUNC_INF("UNI_TOP_CTRL::restart SAVE_FBC:EISO_0x%08x FLKO_0x%08x RSSO_0x%08x",
                        this->fbc_EISO.Raw, this->fbc_FLKO.Raw, this->fbc_RSSO.Raw);

                //keep fbc cnt
                if(UNI_READ_BITS(this->m_pUniDrv,CAM_UNI_FBC_EISO_A_CTL1,FBC_EN)){
                    for (frm_cnt = 0; frm_cnt < this->fbc_EISO.Bits.FBC_CNT; frm_cnt++) {
                        CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),CAM_CTL_FBC_RCNT_INC, EISO_RCNT_INC, 1);
                    }
                }

                if(UNI_READ_BITS(this->m_pUniDrv,CAM_UNI_FBC_FLKO_A_CTL1,FBC_EN)){
                    for (frm_cnt = 0; frm_cnt < this->fbc_FLKO.Bits.FBC_CNT; frm_cnt++) {
                        CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),CAM_CTL_FBC_RCNT_INC, FLKO_RCNT_INC, 1);
                    }
                }

                if(UNI_READ_BITS(this->m_pUniDrv,CAM_UNI_FBC_RSSO_A_CTL1,FBC_EN)){
                    for (frm_cnt = 0; frm_cnt < this->fbc_RSSO.Bits.FBC_CNT; frm_cnt++) {
                        CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),CAM_CTL_FBC_RCNT_INC, RSSO_RCNT_INC, 1);
                    }
                }

                IspFunction_B::fbcPhyLock_EISO[this->m_hwModule].unlock();
                IspFunction_B::fbcPhyLock_FLKO[this->m_hwModule].unlock();
                IspFunction_B::fbcPhyLock_RSSO[this->m_hwModule].unlock();

                CAM_FUNC_INF("UNI_TOP_CTRL::restart PHYS_FBC:EISO_0x%08x FLKO_0x%08x RSSO_0x%08x",
                        UNI_READ_REG(this->m_pUniDrv, CAM_UNI_FBC_EISO_A_CTL2),
                        UNI_READ_REG(this->m_pUniDrv, CAM_UNI_FBC_FLKO_A_CTL2),
                        UNI_READ_REG(this->m_pUniDrv, CAM_UNI_FBC_RSSO_A_CTL2));

                CAM_FUNC_DBG("UNI_TOP_CTRL::- \n");
                return MTRUE;
            }
            break;
        default:
            break;
    }

    return MTRUE;;
}

MINT32 UNI_TOP_CTRL::SwitchOut(void)
{
    E_ISP_CAM_CQ cq;
    MUINT32 page;

    this->m_pDrv = (ISP_DRV_CAM*)this->m_pIspDrv;
    this->m_pDrv->getCurObjInfo(&this->m_hwModule,&cq,&page);
    if(cq == 0) /* print once */
        CAM_FUNC_INF("+ cq:0x%x,page:0x%x\n", cq, page);

    UNI_WRITE_BITS(this->m_pDrv, CAM_UNI_TOP_MOD_EN, HDS_A_EN, 0);
    UNI_WRITE_BITS(this->m_pDrv, CAM_UNI_TOP_MOD_EN, EIS_A_EN, 0);
    UNI_WRITE_BITS(this->m_pDrv, CAM_UNI_TOP_MOD_EN, RSS_A_EN, 0);
    UNI_WRITE_BITS(this->m_pDrv, CAM_UNI_TOP_DMA_EN, EISO_A_EN, 0);
    UNI_WRITE_BITS(this->m_pDrv, CAM_UNI_TOP_DMA_EN, RSSO_A_EN, 0);
    UNI_WRITE_BITS(this->m_pDrv, CAM_UNI_DMA_FRAME_HEADER_EN, FRAME_HEADER_EN_EISO, 0);
    UNI_WRITE_BITS(this->m_pDrv, CAM_UNI_DMA_FRAME_HEADER_EN, FRAME_HEADER_EN_RSSO_A, 0);
    UNI_WRITE_REG(this->m_pDrv, CAM_UNI_FBC_EISO_A_CTL1, 0);
    UNI_WRITE_REG(this->m_pDrv, CAM_UNI_FBC_RSSO_A_CTL1, 0);
    #if (P1_STANDBY_DTWIN_SUPPORT == 1)
    CAM_WRITE_BITS(this->m_pDrv, CAM_CTL_SW_PASS1_DONE,EISO_FIFO_FULL_DROP, 0);
    CAM_WRITE_BITS(this->m_pDrv, CAM_CTL_SW_PASS1_DONE,RSSO_FIFO_FULL_DROP, 0);
    #endif

    return 0;
}

MINT32 UNI_TOP_CTRL::SwitchOutTrig(void)
{
    MINT32 ret = 0;
    E_ISP_CAM_CQ cq;
    MUINT32 page;

    this->m_pDrv = (ISP_DRV_CAM*)this->m_pIspDrv;
    this->m_pDrv->getCurObjInfo(&this->m_hwModule,&cq,&page);
    if(cq == 0) /* print once */
        CAM_FUNC_INF("+ cq:0x%x,page:0x%x, UNI_Source=%d\n", cq, page, this->UNI_Source);

    switch(this->UNI_Source){
        case IF_CAM_A:
        case IF_CAM_B:
            //clear descriptor is to avoid CQ working interactively when uni link to new path
            this->m_pDrv->cqDelModule(UNI_CTL_EN_);
            this->m_pDrv->cqDelModule(UNI_CTL_SEL_);
            this->m_pDrv->cqDelModule(UNI_CTL_FMT_);

            this->m_pDrv->cqDelModule(UNI_ISP_HDS_A_);
            this->m_pDrv->cqDelModule(UNI_ISP_EIS_);
            this->m_pDrv->cqDelModule(UNI_ISP_RSS_A_);

            this->m_pDrv->cqDelModule(UNI_FBC_EISO_);
            this->m_pDrv->cqDelModule(UNI_FBC_RSSO_);

            this->m_pDrv->cqDelModule(UNI_DMA_EISO_BA);
            this->m_pDrv->cqDelModule(UNI_DMA_EISO_);
            this->m_pDrv->cqDelModule(UNI_DMA_FH_EISO_);
            this->m_pDrv->cqDelModule(UNI_DMA_FH_EISO_SPARE_);

            this->m_pDrv->cqDelModule(UNI_DMA_RSSO_A_BA);
            this->m_pDrv->cqDelModule(UNI_DMA_RSSO_A_);
            this->m_pDrv->cqDelModule(UNI_DMA_FH_RSSO_A_);
            this->m_pDrv->cqDelModule(UNI_DMA_FH_RSSO_A_SPARE_);
            this->m_pDrv->cqDelModule(UNI_DMA_FH_EN_);

            CAM_WRITE_BITS(this->m_pDrv, CAM_CTL_FBC_RCNT_INC, EISO_RCNT_INC, 0);
            CAM_WRITE_BITS(this->m_pDrv, CAM_CTL_FBC_RCNT_INC, RSSO_RCNT_INC, 0);

#if (FBC_DRPGRP_SW_WORK_AROUND == 0)
            #if (P1_STANDBY_DTWIN_SUPPORT == 1)
            #else
            CAM_WRITE_BITS(this->m_pDrv->getPhyObj(), CAM_CTL_SW_PASS1_DONE,EISO_FIFO_FULL_DROP, 0);
            CAM_WRITE_BITS(this->m_pDrv->getPhyObj(), CAM_CTL_SW_PASS1_DONE,RSSO_FIFO_FULL_DROP, 0);
            #endif
#endif
            break;
        default:
            CAM_FUNC_ERR("Unsupported source(0x%x) in uni-switch\n",this->UNI_Source);
            return -1;
            break;
    }

    return ret;
}

MINT32 UNI_TOP_CTRL::SwitchIn(MBOOL enableEIS, MBOOL enableRSS)
{
    E_ISP_CAM_CQ cq;
    MUINT32 page;

    this->m_pDrv = (ISP_DRV_CAM*)this->m_pIspDrv;
    this->m_pDrv->getCurObjInfo(&this->m_hwModule,&cq,&page);
    if(cq == 0) /* print once */
        CAM_FUNC_INF("+ cq:0x%x,page:0x%x\n", cq, page);

    UNI_WRITE_REG(this->m_pDrv, CAM_UNI_TOP_MOD_EN, UNI_READ_REG(this->m_pUniDrv, CAM_UNI_TOP_MOD_EN));
    UNI_WRITE_REG(this->m_pDrv, CAM_UNI_TOP_DMA_EN, UNI_READ_REG(this->m_pUniDrv, CAM_UNI_TOP_DMA_EN));
    UNI_WRITE_REG(this->m_pDrv, CAM_UNI_TOP_PATH_SEL, UNI_READ_REG(this->m_pUniDrv, CAM_UNI_TOP_PATH_SEL));
    UNI_WRITE_REG(this->m_pDrv, CAM_UNI_DMA_FRAME_HEADER_EN, UNI_READ_REG(this->m_pUniDrv, CAM_UNI_DMA_FRAME_HEADER_EN));

    UNI_WRITE_BITS(this->m_pDrv, CAM_UNI_TOP_MOD_EN, HDS_A_EN, 1);

    /* path sel */
    UNI_WRITE_REG(this->m_pDrv,CAM_UNI_TOP_PATH_SEL,this->uni_top_ctl.CTRL_SEL.Raw);

    if(enableEIS) {
        UNI_WRITE_BITS(this->m_pDrv, CAM_UNI_TOP_MOD_EN, EIS_A_EN, 1);
        UNI_WRITE_BITS(this->m_pDrv, CAM_UNI_TOP_DMA_EN, EISO_A_EN, 1);
        UNI_WRITE_BITS(this->m_pDrv, CAM_UNI_DMA_FRAME_HEADER_EN, FRAME_HEADER_EN_EISO, 1);
        UNI_WRITE_BITS(this->m_pDrv,CAM_UNI_FBC_EISO_A_CTL1,FBC_MODE,1);
        UNI_WRITE_BITS(this->m_pDrv,CAM_UNI_FBC_EISO_A_CTL1,FBC_EN,1);

        this->m_pDrv->cqAddModule(UNI_ISP_EIS_);
        this->m_pDrv->cqAddModule(UNI_FBC_EISO_);
        this->m_pDrv->cqAddModule(UNI_DMA_EISO_BA);
        this->m_pDrv->cqAddModule(UNI_DMA_EISO_);
        this->m_pDrv->cqAddModule(UNI_DMA_FH_EISO_);
        this->m_pDrv->cqAddModule(UNI_DMA_FH_EISO_SPARE_);

        UNI_WRITE_REG(this->m_pUniDrv, CAM_UNI_FBC_EISO_A_CTL2, 0);
    }

    if(enableRSS) {
        UNI_WRITE_BITS(this->m_pDrv, CAM_UNI_TOP_MOD_EN, RSS_A_EN, 1);
        UNI_WRITE_BITS(this->m_pDrv, CAM_UNI_TOP_DMA_EN, RSSO_A_EN, 1);
        UNI_WRITE_BITS(this->m_pDrv, CAM_UNI_DMA_FRAME_HEADER_EN, FRAME_HEADER_EN_RSSO_A, 1);
        UNI_WRITE_BITS(this->m_pDrv,CAM_UNI_FBC_RSSO_A_CTL1,FBC_MODE,1);
        UNI_WRITE_BITS(this->m_pDrv,CAM_UNI_FBC_RSSO_A_CTL1,FBC_EN,1);

        this->m_pDrv->cqAddModule(UNI_ISP_RSS_A_);
        this->m_pDrv->cqAddModule(UNI_FBC_RSSO_);
        this->m_pDrv->cqAddModule(UNI_DMA_RSSO_A_BA);
        this->m_pDrv->cqAddModule(UNI_DMA_RSSO_A_);
        this->m_pDrv->cqAddModule(UNI_DMA_FH_RSSO_A_);
        this->m_pDrv->cqAddModule(UNI_DMA_FH_RSSO_A_SPARE_);

        UNI_WRITE_REG(this->m_pUniDrv, CAM_UNI_FBC_RSSO_A_CTL2, 0);
    }
    #if (FBC_DRPGRP_SW_WORK_AROUND == 0)
    #if (P1_STANDBY_DTWIN_SUPPORT == 1)
    CAM_WRITE_BITS(this->m_pDrv,CAM_CTL_SW_PASS1_DONE,EISO_FIFO_FULL_DROP, (enableEIS?1:0));
    CAM_WRITE_BITS(this->m_pDrv,CAM_CTL_SW_PASS1_DONE,RSSO_FIFO_FULL_DROP,(enableRSS?1:0));
    #endif
    #endif

    this->m_pDrv->cqAddModule(UNI_ISP_HDS_A_);
    this->m_pDrv->cqAddModule(UNI_CTL_EN_);
    this->m_pDrv->cqAddModule(UNI_CTL_SEL_);
    this->m_pDrv->cqAddModule(UNI_CTL_FMT_);
    this->m_pDrv->cqAddModule(UNI_DMA_FH_EN_);

    return 0;
}

MINT32 UNI_TOP_CTRL::SwitchInTrig(void)
{
    E_ISP_CAM_CQ cq;
    MUINT32 page;

    this->m_pDrv = (ISP_DRV_CAM*)this->m_pIspDrv;
    this->m_pDrv->getCurObjInfo(&this->m_hwModule,&cq,&page);
    if(cq == 0) /* print once */
        CAM_FUNC_INF("+ cq:0x%x,page:0x%x\n", cq, page);

#if 0//(FBC_DRPGRP_SW_WORK_AROUND == 0)
    if(this->m_hwModule == ((UNI_READ_BITS(this->m_pUniDrv, CAM_UNI_TOP_PATH_SEL, HDS2_A_SEL) == 0) ? CAM_A : CAM_B)) {
        MUINT32 _tmp;

        CAM_FUNC_INF("hwModule=0x%x, enable EIS/RSS SW_PASS1_DONE\n", this->m_hwModule);
        //eiso
        _tmp = UNI_READ_BITS(this->m_pUniDrv,CAM_UNI_FBC_EISO_A_CTL1,FBC_EN);
        #if (P1_STANDBY_DTWIN_SUPPORT == 1)
        #else
        CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),CAM_CTL_SW_PASS1_DONE,EISO_FIFO_FULL_DROP,_tmp);
        #endif

        //rsso
        _tmp = UNI_READ_BITS(this->m_pUniDrv,CAM_UNI_FBC_RSSO_A_CTL1,FBC_EN);
        #if (P1_STANDBY_DTWIN_SUPPORT == 1)
        #else
        CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),CAM_CTL_SW_PASS1_DONE,RSSO_FIFO_FULL_DROP,_tmp);
        #endif
    }
#endif

    return 0;
}

MVOID UNI_TOP_CTRL::ClearFLKSettingOnCQ(void)
{
    E_ISP_CAM_CQ cq;
    MUINT32 page;

    this->m_pDrv = (ISP_DRV_CAM*)this->m_pIspDrv;
    this->m_pDrv->getCurObjInfo(&this->m_hwModule,&cq,&page);
    if(cq == 0) /* print once */
        CAM_FUNC_INF("+ cq:0x%x,page:0x%x\n", cq, page);

    UNI_WRITE_BITS(this->m_pDrv, CAM_UNI_TOP_MOD_EN, SGG3_A_EN, 0);
    UNI_WRITE_BITS(this->m_pDrv, CAM_UNI_TOP_MOD_EN, FLK_A_EN, 0);
    UNI_WRITE_BITS(this->m_pDrv, CAM_UNI_TOP_DMA_EN, FLKO_A_EN, 0);
    UNI_WRITE_BITS(this->m_pDrv, CAM_UNI_DMA_FRAME_HEADER_EN, FRAME_HEADER_EN_FLKO, 0);
}

MVOID UNI_TOP_CTRL::ClrLMVFunc(void)
{
    E_ISP_CAM_CQ cq;
    MUINT32 page;
    this->m_pDrv = (ISP_DRV_CAM*)this->m_pIspDrv;
    //dbg log
    this->m_pDrv->getCurObjInfo(&this->m_hwModule,&cq,&page);
    CAM_FUNC_DBG("UNI_TOP_CTRL::ClrLMVFunc: + cq:0x%x,page:0x%x\n",cq,page);

    UNI_WRITE_BITS(this->m_pDrv, CAM_UNI_TOP_MOD_EN, HDS_A_EN, 0);
    UNI_WRITE_BITS(this->m_pDrv, CAM_UNI_TOP_MOD_EN, EIS_A_EN, 0);
    UNI_WRITE_BITS(this->m_pDrv, CAM_UNI_TOP_MOD_EN, RSS_A_EN, 0);
    UNI_WRITE_BITS(this->m_pDrv, CAM_UNI_TOP_DMA_EN, EISO_A_EN, 0);
    UNI_WRITE_BITS(this->m_pDrv, CAM_UNI_TOP_DMA_EN, RSSO_A_EN, 0);
    UNI_WRITE_BITS(this->m_pDrv, CAM_UNI_DMA_FRAME_HEADER_EN, FRAME_HEADER_EN_EISO, 0);
    UNI_WRITE_BITS(this->m_pDrv, CAM_UNI_DMA_FRAME_HEADER_EN, FRAME_HEADER_EN_RSSO_A, 0);
    UNI_WRITE_REG(this->m_pDrv, CAM_UNI_FBC_EISO_A_CTL1, 0);
    UNI_WRITE_REG(this->m_pDrv, CAM_UNI_FBC_RSSO_A_CTL1, 0);
}

MVOID UNI_TOP_CTRL::LMVDisable(void)
{
    MINT32 ret = 0;
    E_ISP_CAM_CQ cq;
    MUINT32 page;

    this->m_pDrv = (ISP_DRV_CAM*)this->m_pIspDrv;

    //dbg log
    this->m_pDrv->getCurObjInfo(&this->m_hwModule,&cq,&page);
    CAM_FUNC_DBG("UNI_TOP_CTRL::_disable: + cq:0x%x,page:0x%x\n",cq,page);

    switch(this->UNI_Source){
        case IF_RAWI_CAM_A:
        case IF_RAWI_CAM_B:
            break;
        case IF_CAM_A:
        case IF_CAM_B:
            UNI_WRITE_BITS(this->m_pUniDrv,CAM_UNI_TOP_MISC,DB_EN,0);

            UNI_WRITE_BITS(this->m_pUniDrv, CAM_UNI_TOP_MOD_EN, HDS_A_EN, 0);
            UNI_WRITE_BITS(this->m_pUniDrv, CAM_UNI_TOP_MOD_EN, EIS_A_EN, 0);
            UNI_WRITE_BITS(this->m_pUniDrv, CAM_UNI_TOP_MOD_EN, RSS_A_EN, 0);
            UNI_WRITE_BITS(this->m_pUniDrv, CAM_UNI_TOP_DMA_EN, EISO_A_EN, 0);
            UNI_WRITE_BITS(this->m_pUniDrv, CAM_UNI_TOP_DMA_EN, RSSO_A_EN, 0);
            UNI_WRITE_BITS(this->m_pUniDrv, CAM_UNI_DMA_FRAME_HEADER_EN, FRAME_HEADER_EN_EISO, 0);
            UNI_WRITE_BITS(this->m_pUniDrv, CAM_UNI_DMA_FRAME_HEADER_EN, FRAME_HEADER_EN_RSSO_A, 0);
            UNI_WRITE_REG(this->m_pUniDrv, CAM_UNI_FBC_EISO_A_CTL1, 0);
            UNI_WRITE_REG(this->m_pUniDrv, CAM_UNI_FBC_RSSO_A_CTL1, 0);

            UNI_WRITE_BITS(this->m_pUniDrv,CAM_UNI_TOP_MISC,DB_EN,1);

            this->m_pDrv->cqDelModule(UNI_ISP_HDS_A_);
            this->m_pDrv->cqDelModule(UNI_ISP_EIS_);
            this->m_pDrv->cqDelModule(UNI_ISP_RSS_A_);

            this->m_pDrv->cqDelModule(UNI_FBC_EISO_);
            this->m_pDrv->cqDelModule(UNI_FBC_RSSO_);

            this->m_pDrv->cqDelModule(UNI_DMA_EISO_BA);
            this->m_pDrv->cqDelModule(UNI_DMA_EISO_);
            this->m_pDrv->cqDelModule(UNI_DMA_FH_EISO_);
            this->m_pDrv->cqDelModule(UNI_DMA_FH_EISO_SPARE_);

            this->m_pDrv->cqDelModule(UNI_DMA_RSSO_A_BA);
            this->m_pDrv->cqDelModule(UNI_DMA_RSSO_A_);
            this->m_pDrv->cqDelModule(UNI_DMA_FH_RSSO_A_);
            this->m_pDrv->cqDelModule(UNI_DMA_FH_RSSO_A_SPARE_);
            this->m_pDrv->cqDelModule(UNI_DMA_FH_EN_);

            break;
        default:
            CAM_FUNC_ERR("UNI unsupported source:0x%x\n",this->UNI_Source);
            break;
    }
}

