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

#define LOG_TAG "ifunc_cam_twin"

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

DECLARE_DBG_LOG_VARIABLE(func_twin);
//EXTERN_DBG_LOG_VARIABLE(func_cam);

// Clear previous define, use our own define.
#undef CAM_FUNC_VRB
#undef CAM_FUNC_DBG
#undef CAM_FUNC_INF
#undef CAM_FUNC_WRN
#undef CAM_FUNC_ERR
#undef CAM_FUNC_AST
#define CAM_FUNC_VRB(fmt, arg...)        do { if (func_twin_DbgLogEnable_VERBOSE) { BASE_LOG_VRB(fmt, ##arg); } } while(0)
#define CAM_FUNC_DBG(fmt, arg...)        do {\
    if (func_twin_DbgLogEnable_DEBUG  ) { \
        BASE_LOG_DBG("[0x%x]:" fmt,this->m_hwModule, ##arg); \
    }\
} while(0)

#define CAM_FUNC_INF(fmt, arg...)        do {\
    if (func_twin_DbgLogEnable_INFO  ) { \
        BASE_LOG_INF("[0x%x]:" fmt,this->m_hwModule, ##arg); \
    }\
} while(0)

#define CAM_FUNC_WRN(fmt, arg...)        do { if (func_twin_DbgLogEnable_WARN   ) { BASE_LOG_WRN(fmt, ##arg); } } while(0)

#define CAM_FUNC_ERR(fmt, arg...)        do {\
    if (func_twin_DbgLogEnable_ERROR  ) { \
        BASE_LOG_ERR("[0x%x]:" fmt,this->m_hwModule, ##arg); \
    }\
} while(0)

#define CAM_FUNC_AST(cond, fmt, arg...)  do { if (func_twin_DbgLogEnable_ASSERT ) { BASE_LOG_AST(cond, fmt, ##arg); } } while(0)

CAM_TWIN_PIPE::CAM_TWIN_PIPE()
{
    m_subsample = 0;
    m_crz_r1_out_wd = m_crz_r2_out_wd = m_rss_r2_out_wd = m_rrz_out_wd = 0;
    m_af_vld_xstart = m_af_blk_xsize = m_af_x_win_num =0;
    m_lsc_lwidth = m_lsc_win_num_x = 0;
    m_rlb_oft = 0;
    m_pTwinCB = NULL;
    m_ConfigDMAEnSt = 0;
    m_ConfigDMA2EnSt = 0;
    DBG_LOG_CONFIG(imageio, func_twin);
}

inline MBOOL CAM_TWIN_PIPE::slaveNumCheck(MUINT32 &slave_num)
{
    ISP_DRV_CAM* pTwinIspDrv = NULL;
    ISP_HW_MODULE twinHwModule = PHY_CAM;
    E_ISP_CAM_CQ cq;
    MUINT32 page = 0;
    MBOOL ret = MFALSE;

    slave_num = this->m_pTwinIspDrv_v.size();
    if(!slave_num){
        CAM_FUNC_ERR("TwinIspDrv vector is empty\n");
        goto EXIT;
    } else if (slave_num > _SLAVE_CAM_MAX_){
        CAM_FUNC_ERR("Only support to %dRAW(%d)\n", (_SLAVE_CAM_MAX_+1), slave_num);
        goto EXIT;
    }
    for(MUINT32 i = 0; i < slave_num; i++)
    {
        pTwinIspDrv = this->m_pTwinIspDrv_v.at(i);
        pTwinIspDrv->getCurObjInfo(&twinHwModule,&cq,&page);
        if (twinHwModule != CAM_B) {
            CAM_FUNC_ERR("Only CamB can be TwinModule(%d/%d)", slave_num, twinHwModule);
            goto EXIT;
        }
    }
    if((this->m_hwModule != CAM_A)){
        CAM_FUNC_ERR("CAM(%d) can't be master cam\n",this->m_hwModule);
        goto EXIT;
    }
    ret = MTRUE;
EXIT:
    return ret;
}

///////////////////////////////////////////////////////////////////////////////
MINT32 CAM_TWIN_PIPE::_config( void)
{
    MINT32 ret = 0;
    E_ISP_CAM_CQ cq;
    MUINT32 page, slave_num;
    ISP_HW_MODULE twinHwModule;
    ISP_DRV_CAM* pTwinIspDrv;
    std::string dbg_str = "twinHwModule: ";

    this->m_pDrv = (ISP_DRV_CAM*)this->m_pIspDrv;
    this->m_pDrv->getCurObjInfo(&this->m_hwModule,&cq,&page);

    if(!slaveNumCheck(slave_num))
        return 1;

    CAM_FUNC_DBG("+, slavenum:%d,cq:%d,page:%d\n", slave_num, cq, page);

    CAM_WRITE_BITS(this->m_pDrv, CAMCTL_R1_CAMCTL_TWIN_STATUS, TWIN_EN, 0x1);
    CAM_WRITE_BITS(this->m_pDrv, CAMCTL_R1_CAMCTL_EN3, CAMCTL_MRG_R11_EN, 1);
    CAM_WRITE_BITS(this->m_pDrv, CAMCTL_R1_CAMCTL_EN3, CAMCTL_MRG_R12_EN, 1);

    if(this->m_bDYUV)
        CAM_WRITE_BITS(this->m_pDrv, CAMCTL_R1_CAMCTL_EN3, CAMCTL_MRG_R14_EN, 1);

    //reset dma list
    for(MUINT32 i = 0; i < (MUINT32)TWIN_DMAO_MAX; i++) {
        this->m_dma_L[i].clear();
    }

    for(MUINT32 i = 0; i < slave_num; i++)
    {
        pTwinIspDrv = this->m_pTwinIspDrv_v.at(i);

        pTwinIspDrv->getCurObjInfo(&twinHwModule,&cq,&page);
        CAM_FUNC_VRB("[%d_%d] twinHwModule:%d,cq:%d,page:%d\n",slave_num,i,twinHwModule,cq,page);
        dbg_str += std::to_string(twinHwModule) + ",";

        CAM_WRITE_BITS(pTwinIspDrv,CAMCTL_R1_CAMCTL_MISC,CAMCTL_DB_EN,1);

#ifdef DCIF_STT_SW_WORK_AROUND
        CAM_WRITE_BITS(pTwinIspDrv,CAMCTL_R1_CAMCTL_MISC,CAMCTL_DB_BYPASS_CAMCTL,1);
#endif

        // 1. Set TWIN_EN of m_pDrv to be 1
        CAM_WRITE_BITS(pTwinIspDrv, CAMCTL_R1_CAMCTL_TWIN_STATUS, TWIN_EN, 0x1);
        // Enable last P1 done
        CAM_WRITE_BITS(pTwinIspDrv, CAMCTL_R1_CAMCTL_MISC,CAMCTL_PASS1_DONE_SEL,0);

        //cq db setting
        CAM_WRITE_BITS(pTwinIspDrv,CAMCQ_R1_CAMCQ_CQ_EN,CAMCQ_CQ_DB_EN,1); //enable double buffer

        //subsample p1 done
        if(this->m_subsample){
            CAM_WRITE_BITS(pTwinIspDrv,CAMCTL_R1_CAMCTL_SW_PASS1_DONE,CAMCTL_DOWN_SAMPLE_EN,1);
        }
        else{
            CAM_WRITE_BITS(pTwinIspDrv,CAMCTL_R1_CAMCTL_SW_PASS1_DONE,CAMCTL_DOWN_SAMPLE_EN,0);
        }

        CAM_WRITE_BITS(pTwinIspDrv,CAMCTL_R1_CAMCTL_SW_PASS1_DONE,CAMCTL_DOWN_SAMPLE_PERIOD,this->m_subsample);

        //
        //change CQ load mode before streaming start.
        //because this bit is shared to all cq, so put here to make sure all cq timing

        if(CAM_READ_BITS(pTwinIspDrv,CAMCTL_R1_CAMCTL_SW_PASS1_DONE,CAMCTL_DOWN_SAMPLE_EN) == 1){
            //load mode = 0 if the latch-timing of the cq-baseaddress-reg is by sw p1_done
            CAM_WRITE_BITS(pTwinIspDrv,CAMCQ_R1_CAMCQ_CQ_EN,CAMCQ_CQ_DB_LOAD_MODE,0);
        }
        else{
            //load mode = 1 if the latch-timing of the cq-baseaddress-reg is by hw p1_done
            //when mode = 0 , inner reg is useless
            CAM_WRITE_BITS(pTwinIspDrv,CAMCQ_R1_CAMCQ_CQ_EN,CAMCQ_CQ_DB_LOAD_MODE,1);
        }

        if (CAM_READ_BITS(this->m_pDrv, CAMCTL_R1_CAMCTL_SEL2, CAMCTL_DCIF_MAIN_EN)){ //is DCIF
            CAM_WRITE_BITS(pTwinIspDrv, CAMCTL_R1_CAMCTL_SEL2, CAMCTL_DCIF_EN_SEL, 0);
            CAM_WRITE_BITS(pTwinIspDrv, CAMCTL_R1_CAMCTL_SEL2, CAMCTL_DCIF_MAIN_SEL, 0);
            CAM_WRITE_BITS(pTwinIspDrv, CAMCTL_R1_CAMCTL_SEL2, CAMCTL_DCIF_MAIN_EN, 1);
            CAM_WRITE_BITS(pTwinIspDrv, CAMCTL_R1_CAMCTL_MISC, CAMCTL_CAMSV_DC_MODE, 1);
        }
    }
    // recode CRZO_R1/YUVO_R1/CRZO_R2 enable status.
    this->m_ConfigDMAEnSt = CAM_READ_REG(this->m_pDrv, CAMCTL_R1_CAMCTL_DMA_EN);
    this->m_ConfigDMA2EnSt = CAM_READ_REG(this->m_pDrv, CAMCTL_R1_CAMCTL_DMA2_EN);

    CAM_FUNC_INF("-,slavenum:%d,cq:%d,page:%d,%s\n",slave_num,cq,page,dbg_str.c_str());

    return ret;
}

MINT32 CAM_TWIN_PIPE::_enable( void* pParam  )
{
    MINT32 ret = 0;
    E_ISP_CAM_CQ cq;
    MUINT32 page, slave_num;
    ISP_DRV_CAM* pTwinIspDrv;
    ISP_HW_MODULE twinHwModule;
    (void)pParam;
    std::string dbg_str = "twinHwModule: ";

    this->m_pDrv = (ISP_DRV_CAM*)this->m_pIspDrv;
    this->m_pDrv->getCurObjInfo(&this->m_hwModule,&cq,&page);

    if(!slaveNumCheck(slave_num))
        return 1;

    CAM_FUNC_DBG("+, slavenum:%d,subsample:%d,cq:%d,page:%d\n",slave_num,this->m_subsample,cq,page);

    for(MUINT32 i = 0; i < slave_num; i++)
    {
        pTwinIspDrv = this->m_pTwinIspDrv_v.at(i);
        pTwinIspDrv->getCurObjInfo(&twinHwModule,&cq,&page);
        CAM_FUNC_VRB("[%d_%d] twinHwModule:%d,cq:%d,page:%d\n",slave_num,i,twinHwModule,cq,page);
        dbg_str += std::to_string(twinHwModule) + ",";

        // special, w via vir
        CAM_WRITE_REG(pTwinIspDrv,CAMDMATOP1_R1_CAMDMATOP1_DMA_RSV11,(CAM_READ_REG(pTwinIspDrv->getPhyObj(),CAMDMATOP1_R1_CAMDMATOP1_DMA_RSV11)&0x7fffffff));//bit31 for smi mask, blocking imgo output
        CAM_WRITE_REG(pTwinIspDrv,CAMDMATOP1_R1_CAMDMATOP1_DMA_RSV16,0xffffffff);

        if (CAM_READ_BITS(this->m_pDrv, CAMCTL_R1_CAMCTL_SEL2, CAMCTL_DCIF_MAIN_EN)){ //is DCIF
            CAM_WRITE_BITS(pTwinIspDrv, CAMCTL_R1_CAMCTL_SEL2, CAMCTL_DCIF_MAIN_EN, 1);
        }
    }

    CAM_FUNC_INF("-,slavenum:%d,subsample:%d,cq:%d,page:%d,%s\n",slave_num,this->m_subsample,cq,page,dbg_str.c_str());

    return 0;
}

MINT32 CAM_TWIN_PIPE::_disable(void* pParam)
{
    MINT32 ret = 0;
    E_ISP_CAM_CQ cq;
    MUINT32 page, slave_num;
    ISP_DRV_CAM* pTwinIspDrv;
    ISP_HW_MODULE twinHwModule;
    (void)pParam;
    std::string dbg_str = "twinHwModule: ";

    this->m_pDrv = (ISP_DRV_CAM*)this->m_pIspDrv;
    this->m_pDrv->getCurObjInfo(&this->m_hwModule,&cq,&page);

    if(CAM_READ_BITS(this->m_pDrv->getPhyObj(), CAMCTL_R1_CAMCTL_TWIN_STATUS, TWIN_EN)== 0){
        //do not access cam_b's register directly, current camb may be running with a sesnor.
        CAM_FUNC_INF("do nothing for Twin's diable, %d\n", this->m_pTwinIspDrv_v.size());
        return ret;
    }

    if(!slaveNumCheck(slave_num))
        return 1;

    CAM_FUNC_DBG("+, slavenum:%d,cq:%d,page:%d\n", slave_num, cq, page);

    for(MUINT32 i = 0; i < slave_num; i++)
    {
        pTwinIspDrv = this->m_pTwinIspDrv_v.at(i);
        pTwinIspDrv->getCurObjInfo(&twinHwModule,&cq,&page);
        CAM_FUNC_VRB("[%d_%d] twinHwModule:%d,cq:%d,page:%d\n",slave_num,i,twinHwModule,cq,page);
        dbg_str += std::to_string(twinHwModule) + ",";

        CAM_FUNC_DBG("[%d_%d] flush twin ion buf twinModule(0x%x)\n",slave_num,i,twinHwModule);
        this->m_pDrv->setDeviceInfo(_SET_ION_FREE_BY_HWMODULE,(MUINT8*)&twinHwModule);

        // Set TWIN_EN of m_pDrv to be 0
        CAM_WRITE_REG(this->m_pDrv->getPhyObj(), CAMCTL_R1_CAMCTL_TWIN_STATUS,  0x0); // 0x1A004050
        CAM_WRITE_REG(pTwinIspDrv->getPhyObj(), CAMCTL_R1_CAMCTL_TWIN_STATUS,  0x0); // 0x1A005050

        // !!Must set CAMB's CRP_R1_SEL = 0, or run other case CAMA will enconter error
        CAM_WRITE_BITS(pTwinIspDrv->getPhyObj(), CAMCTL_R1_CAMCTL_SEL, CAMCTL_CRP_R1_SEL, 0x0);

        // Enable last P1 done, but ref itself
        CAM_WRITE_BITS(pTwinIspDrv->getPhyObj(), CAMCTL_R1_CAMCTL_SEL, CAMCTL_SEP_SEL, 0x0);
        CAM_WRITE_BITS(this->m_pDrv, CAMCTL_R1_CAMCTL_MISC, CAMCTL_LAST_PASS1_DONE_CTL, GetP1DoneRef(this->m_hwModule));
        CAM_WRITE_BITS(pTwinIspDrv, CAMCTL_R1_CAMCTL_MISC, CAMCTL_LAST_PASS1_DONE_CTL, GetP1DoneRef(twinHwModule));
        CAM_WRITE_BITS(this->m_pDrv->getPhyObj(), CAMCTL_R1_CAMCTL_MISC, CAMCTL_LAST_PASS1_DONE_CTL, GetP1DoneRef(this->m_hwModule));
        CAM_WRITE_BITS(pTwinIspDrv->getPhyObj(), CAMCTL_R1_CAMCTL_MISC, CAMCTL_LAST_PASS1_DONE_CTL, GetP1DoneRef(twinHwModule));

        CAM_WRITE_BITS(pTwinIspDrv->getPhyObj(),CAMCTL_R1_CAMCTL_MISC,CAMCTL_DB_EN,0); //disable double buffer
        //signal ctrl
        CAM_WRITE_REG(pTwinIspDrv->getPhyObj(),CAMCTL_R1_CAMCTL_SW_PASS1_DONE,0);
        CAM_WRITE_REG(pTwinIspDrv->getPhyObj(),CAMCTL_R1_CAMCTL_DONE_SEL,0);
        CAM_WRITE_REG(pTwinIspDrv->getPhyObj(),CAMCTL_R1_CAMCTL_DMA_EN,0x0);


        CAM_WRITE_REG(pTwinIspDrv->getPhyObj(),CAMCTL_R1_CAMCTL_EN_CTL,0x0);
        CAM_WRITE_REG(pTwinIspDrv->getPhyObj(),CAMCTL_R1_CAMCTL_EN_CTL,0x0);
        //pdc function enable is within BNR
        CAM_WRITE_REG(pTwinIspDrv,BPC_R1_BPC_BPC_CON,0x0);
        CAM_WRITE_REG(pTwinIspDrv,BPC_R1_BPC_PDC_CON,0x0);

        //close  fbc
        //FBC on uni will be closed at the _disable() of uni_top
        //FBC of STT pipe will be closed at STT pipe
        CAM_WRITE_REG(pTwinIspDrv->getPhyObj(),FBC_R1_FBC_IMGO_R1_CTL1,0x0);
        CAM_WRITE_REG(pTwinIspDrv->getPhyObj(),FBC_R1_FBC_RRZO_R1_CTL1,0x0);
        CAM_WRITE_REG(pTwinIspDrv->getPhyObj(),FBC_R1_FBC_UFEO_R1_CTL1,0x0);
        CAM_WRITE_REG(pTwinIspDrv->getPhyObj(),FBC_R1_FBC_UFGO_R1_CTL1,0x0);
        CAM_WRITE_REG(pTwinIspDrv->getPhyObj(),FBC_R1_FBC_YUVO_R1_CTL1,0x0);
        CAM_WRITE_REG(pTwinIspDrv->getPhyObj(),FBC_R1_FBC_YUVBO_R1_CTL1,0x0);
        CAM_WRITE_REG(pTwinIspDrv->getPhyObj(),FBC_R1_FBC_YUVCO_R1_CTL1,0x0);
        CAM_WRITE_REG(pTwinIspDrv->getPhyObj(),FBC_R1_FBC_CRZO_R1_CTL1,0x0);
        CAM_WRITE_REG(pTwinIspDrv->getPhyObj(),FBC_R1_FBC_CRZBO_R1_CTL1,0x0);
        CAM_WRITE_REG(pTwinIspDrv->getPhyObj(),FBC_R1_FBC_CRZO_R2_CTL1,0x0);
        CAM_WRITE_REG(pTwinIspDrv->getPhyObj(),FBC_R1_FBC_CRZBO_R2_CTL1,0x0);

        CAM_WRITE_BITS(pTwinIspDrv->getPhyObj(),CAMCTL_R1_CAMCTL_MISC,CAMCTL_DB_EN,1); //disable double buffer

        //reset
        SW_RESET(pTwinIspDrv->getPhyObj());

    }

    CAM_FUNC_INF("-, slavenum:%d,cq:%d,page:%d,%s\n",slave_num,cq,page,dbg_str.c_str());

    return 0;
}

MINT32 CAM_TWIN_PIPE::_write2CQ(MUINT32 burstIndex)
{
    E_ISP_CAM_CQ cq;
    MUINT32 page, slave_num;
    ISP_DRV_CAM* pTwinIspDrv;

    this->m_pDrv = (ISP_DRV_CAM*)this->m_pIspDrv;
    this->m_pDrv->getCurObjInfo(&this->m_hwModule,&cq,&page);
    //
    CAM_FUNC_VRB("m_HwModule(%d),cq(%d),page(%d)\n", this->m_hwModule,cq,page);

    if (!slaveNumCheck(slave_num)) {
        return 1;
    }

    for (MUINT32 i = 0; i < slave_num; i++) {
        pTwinIspDrv = this->m_pTwinIspDrv_v.at(i);
        pTwinIspDrv->cqAddModule(CAM_CTL_FBC_RCNT_INC_);
    }

    return 0;
}

MINT32 CAM_TWIN_PIPE::DoneCollector(MBOOL En, E_INPUT InPut)
{
    E_ISP_CAM_CQ cq;
    MUINT32 page = 0, slave_num = 0;
    En;
    InPut;

    this->m_pDrv = (ISP_DRV_CAM*)this->m_pIspDrv;
    this->m_pDrv->getCurObjInfo(&this->m_hwModule,&cq,&page);

    if (!slaveNumCheck(slave_num))
        return 1;

    for (MUINT32 cnt = 0; cnt < slave_num; cnt++) {
        ISP_HW_MODULE twinHwModule;
        ISP_DRV_CAM* pTwinIspDrv;
        MUINT32 P1DoneRef = 0;

        pTwinIspDrv = this->m_pTwinIspDrv_v.at(cnt);
        pTwinIspDrv->getCurObjInfo(&twinHwModule,&cq,&page);

        P1DoneRef = CAM_READ_BITS(this->m_pDrv, CAMCTL_R1_CAMCTL_MISC, CAMCTL_LAST_PASS1_DONE_CTL);

        /* if slave cam has no WDMA enabled, this slave cam will has no p1 done, last P1 done should ref. master cam*/
        if ((CAM_READ_REG(pTwinIspDrv, CAMCTL_R1_CAMCTL_DMA_EN) & WDMA_EN_MASK) || \
            (CAM_READ_REG(pTwinIspDrv, CAMCTL_R1_CAMCTL_DMA2_EN) & WDMA2_EN_MASK)) {
            P1DoneRef |= GetP1DoneRef(twinHwModule);
        }

        CAM_WRITE_BITS(this->m_pDrv, CAMCTL_R1_CAMCTL_MISC, CAMCTL_LAST_PASS1_DONE_CTL, P1DoneRef);
        CAM_WRITE_BITS(pTwinIspDrv, CAMCTL_R1_CAMCTL_MISC, CAMCTL_LAST_PASS1_DONE_CTL, P1DoneRef);

    }

    return 0;
}

MINT32 CAM_TWIN_PIPE::update_cq(void)
{
    #define _STRUCT_OFST(_stype, _field) ((MUINT32)(MUINTPTR)&(((_stype*)0)->_field))
    #define DEBUG_CQ_UPDATE 0
    MINT32 ret = 0;
    MUINT32* main_IspDescript_vir;
    MUINT32* main_IspVirReg_phy;
    MUINT32* twin_IspDescript_vir;
    MUINT32* twin_IspVirReg_phy;
    MUINT32 tmp = 0, _tmp2 = 0, page = 0, slave_num = 0;
    E_ISP_CAM_CQ cq;
    ISP_DRV_CAM* pTwinIspDrv;
    ISP_HW_MODULE twinHwModule;

    this->m_pDrv = (ISP_DRV_CAM*)this->m_pIspDrv;
    this->m_pDrv->getCurObjInfo(&this->m_hwModule,&cq,&page);

    if(!slaveNumCheck(slave_num))
        return 1;

    main_IspDescript_vir = this->m_pDrv->getCQDescBufVirAddr();
    main_IspVirReg_phy = this->m_pDrv->getIspVirRegPhyAddr();

    for(MUINT32 i = 0; i < slave_num; i++)
    {
        MINT32 slave_ofset;
        MUINT32 master_base, master_ofst;

        pTwinIspDrv = this->m_pTwinIspDrv_v.at(i);
        pTwinIspDrv->getCurObjInfo(&twinHwModule,&cq,&page);
        CAM_FUNC_VRB("[%d_%d] twinHwModule:%d,cq:%d,page:%d\n",slave_num,i,twinHwModule,cq,page);

        // Update Twin's CQ descriptor according to Main's CQ descriptor
        twin_IspDescript_vir = pTwinIspDrv->getCQDescBufVirAddr();
        twin_IspVirReg_phy = pTwinIspDrv->getIspVirRegPhyAddr();

        //ofset & base
        master_ofst = (this->m_hwModule - CAM_A)*CAM_BASE_RANGE;
        master_base = 0x0 + master_ofst;

        switch(twinHwModule)
        {
            case CAM_A:
                slave_ofset = (-CAM_BASE_RANGE * 2);//master cam is camb
                break;
            case CAM_B:
                slave_ofset = CAM_BASE_RANGE * 2;
                break;
            case CAM_C:
                slave_ofset = CAM_BASE_RANGE * 2;
                slave_ofset *= ((this->m_hwModule == CAM_A)? 2:1);
                break;
            default:
                CAM_FUNC_ERR("CAM%d not supported slave module(%d)\n", this->m_hwModule, twinHwModule);
                return 1;
                break;
        }

        //plz refer to ISP_DRV_CQ_CMD_DESC_STRUCT:
        //osft_addr_msb[b'31-29], osft_addr_lsb[b'15-0]
        slave_ofset = (((slave_ofset&0x70000)<<(29-16)) | (slave_ofset&CQ_DES_RANGE));

#if DEBUG_CQ_UPDATE
        CAM_FUNC_INF("dump slave(%d) descrptor:\n", twinHwModule);
#endif

        // Copy Main's CQ descriptor to Twin's
        // Copy each CQ module separatly
        for(int j = 0; j < CAM_CAM_MODULE_MAX; j++)
        {
            tmp = *(main_IspDescript_vir + 2*j);
            _tmp2 = (tmp&CQ_DES_RANGE);
            //copy only non-dummy descriptor && do not copy cq's baseaddress, baseaddress is for master cam's link-list
            if((_tmp2 > master_base) && (_tmp2 != (CQ_DUMMY_REG+master_ofst)) && \
                (_tmp2 != (_STRUCT_OFST(cam_reg_t, CAMCQ_R1_CAMCQ_CQ_THR0_BASEADDR)+master_base)))
            {//UNI r excluded
#if TWIN_CQ_SW_WORKAROUND
                //only copy reg within master cam
                if((tmp&CQ_DES_RANGE) < (master_base+CAM_BASE_RANGE)){
                    *(twin_IspDescript_vir + 2*j) = tmp + slave_ofset;
                    *(twin_IspDescript_vir + (2*j + 1)) = (MUINT32)((MUINT8*)(MUINTPTR)*(main_IspDescript_vir + (2*j + 1)) - (MUINT8*)main_IspVirReg_phy) + (MUINTPTR)twin_IspVirReg_phy;
                }
#else
                *(twin_IspDescript_vir + 2*j) = tmp + slave_ofset;
                *(twin_IspDescript_vir + (2*j + 1)) = (MUINT32)((MUINT8*)(MUINTPTR)*(main_IspDescript_vir + (2*j + 1)) - (MUINT8*)main_IspVirReg_phy) + (MUINTPTR)twin_IspVirReg_phy;
#endif

            }

#if DEBUG_CQ_UPDATE
            CAM_FUNC_INF("0x%x(0x%x), 0x%x(0x%x)\n", twin_IspDescript_vir + 2*j, *(twin_IspDescript_vir + 2*j), \
                    twin_IspDescript_vir + (2*j + 1), *(twin_IspDescript_vir + (2*j + 1)));
#endif

        }

        /*  descriptors of slave cams should add and update by update_cq().
                  If the registers of master cam, written directly to phy., and it can be programmed by TWIN_PIPE
                  => Add cqModule here
            */
        pTwinIspDrv->cqAddModule(CAM_CQ_EN_);
        pTwinIspDrv->cqAddModule(CAM_CTL_MISC_);//CAMCTL_DB_EN
        //LafiteEP_TODO: RSV need redeign: pTwinIspDrv->cqAddModule(CAM_DMA_RSV1_);//smi mask
        //LafiteEP_TODO: RSV need redeign: pTwinIspDrv->cqAddModule(CAM_DMA_RSV6_);//smi mask

        //Only 1st slave cam needs to addCQ
        if( i == 0){
        }

#if DEBUG_CQ_UPDATE
        CAM_FUNC_INF("dump master(%d) descrptor:\n", this->m_hwModule);
        for(int j = 0; j < CAM_CAM_MODULE_MAX; j++)
        {
            CAM_FUNC_INF("0x%x(0x%x), 0x%x(0x%x)\n", main_IspDescript_vir + 2*j, *(main_IspDescript_vir + 2*j), \
                    main_IspDescript_vir + (2*j + 1), *(main_IspDescript_vir + (2*j + 1)));
        }
#endif
    }

    return ret;
}

MINT32 CAM_TWIN_PIPE::update_cq_suspend(void)
{
    MINT32 ret = 0;
    E_ISP_CAM_CQ cq;
    MUINT32 page, slave_num;
    ISP_DRV_CAM* pTwinIspDrv;
    ISP_HW_MODULE twinHwModule;

    this->m_pDrv = (ISP_DRV_CAM*)this->m_pIspDrv;
    this->m_pDrv->getCurObjInfo(&this->m_hwModule,&cq,&page);

    if(!slaveNumCheck(slave_num))
        return 1;

    for(MUINT32 i = 0; i < slave_num; i++)
    {
        pTwinIspDrv = this->m_pTwinIspDrv_v.at(i);
        pTwinIspDrv->getCurObjInfo(&twinHwModule,&cq,&page);
        CAM_FUNC_VRB("[%d_%d] twinHwModule:%d,cq:%d,page:%d\n",slave_num,i,twinHwModule,cq,page);

        pTwinIspDrv->cqAddModule(CAM_CTL_EN_);
        pTwinIspDrv->cqAddModule(CAM_CTL_SEL_);
        pTwinIspDrv->cqAddModule(CAM_TWIN_INFO);
        pTwinIspDrv->cqAddModule(CAM_CTL_FMT_);
        pTwinIspDrv->cqAddModule(CAM_CTL_FBC_RCNT_INC_);
        pTwinIspDrv->cqAddModule(CAM_FBC_IMGO_);
        pTwinIspDrv->cqAddModule(CAM_FBC_RRZO_);
        pTwinIspDrv->cqAddModule(CAM_FBC_AFO_);
        pTwinIspDrv->cqAddModule(CAM_CQ_EN_);
        pTwinIspDrv->cqAddModule(CAM_CTL_MISC_);//CAMCTL_DB_EN
        //LafiteEP_TODO: RSV need redeign: pTwinIspDrv->cqAddModule(CAM_DMA_RSV1_);//smi mask
        //LafiteEP_TODO: RSV need redeign: pTwinIspDrv->cqAddModule(CAM_DMA_RSV6_);//smi mask
    }

    return ret;
}

MINT32 CAM_TWIN_PIPE::update_beforeRunTwin()
{
#define UPDATE_TWIN_EN(CLASS,EN)\
    do{ \
        unique_ptr<CLASS> ptr(new CLASS());\
        ptr->m_pIspDrv = (IspDrvVir*)pTwinIspDrv;\
        if (EN) {ptr->enable(NULL);} else {ptr->disable(NULL);}\
    } while(0)

    MINT32 ret = 0;
    MUINT32  stride = 0, tmp = 0, page = 0, slave_num = 0;
    Header_RRZO fh_rrzo;
    REG_TG_R1_TG_SEN_GRAB_PXL TG_W;
    REG_TG_R1_TG_SEN_GRAB_LIN TG_H;
    E_ISP_CAM_CQ cq;
    ISP_DRV_CAM* pTwinIspDrv;
    ISP_HW_MODULE twinHwModule = PHY_CAM;
    std::string dbg_str = "twinHwModule: ";
    RectData<MUINT32> Bin;

    this->m_pDrv = (ISP_DRV_CAM*)this->m_pIspDrv;
    this->m_pDrv->getCurObjInfo(&this->m_hwModule,&cq,&page);

    if(!slaveNumCheck(slave_num))
        return 1;

    CAM_FUNC_DBG("+, slavenum:%d,cq:%d,page:%d\n", slave_num, cq, page);

    // Check if RRZ_EN =1 and RRZ_VERT_STEP == 0. return err.
    // Cause TWIN_RRZ_IN_CROP_HT is set to 0 here, but Dual_cal will return err when TWIN_RRZ_IN_CROP_HT and RRZ_VERT_STEP both = 0
    if(1 == CAM_READ_BITS(this->m_pDrv, CAMCTL_R1_CAMCTL_EN2, CAMCTL_RRZ_R1_EN) && 0 == CAM_READ_BITS(this->m_pDrv, RRZ_R1_RRZ_VERT_STEP, RRZ_VERT_STEP)) {
        CAM_FUNC_ERR("RRZ_EN(%d) RRZ_VERT_STEP(%d) is 0!!!! Cannot run twin!!!", \
                CAM_READ_BITS(this->m_pDrv, CAMCTL_R1_CAMCTL_EN2, CAMCTL_RRZ_R1_EN), \
                CAM_READ_BITS(this->m_pDrv, RRZ_R1_RRZ_VERT_STEP, RRZ_VERT_STEP));
        return 1;
    }

    //rlb
    this->m_rlb_oft = CAM_READ_BITS(this->m_pDrv,RRZ_R1_RRZ_RLB_AOFST,RRZ_RLB_AOFST);

    /*rrz*/
    this->m_rrz_out_wd = CAM_READ_BITS(this->m_pDrv, RRZ_R1_RRZ_OUT_IMG, RRZ_OUT_WD);

    //rrz roi
    this->m_rrz_roi.floatX = this->m_rrz_roi.floatY = 0;
    tmp = fh_rrzo.GetRegInfo(Header_RRZO::E_RRZ_CRP_START,this->m_pDrv);
    this->m_rrz_roi.x = (tmp & 0xffff);
    this->m_rrz_roi.y = (tmp>>16);

    tmp = fh_rrzo.GetRegInfo(Header_RRZO::E_RRZ_CRP_SIZE,this->m_pDrv);
    this->m_rrz_roi.w = (tmp & 0xffff);
    this->m_rrz_roi.h = (tmp>>16);

    // rrz_in
    // If DBN=1, rrz in setting must be divide by 2; if DBN = 1 and BIN = 2, imgo setting must be divide by 4
    GET_SEP_BIN(this->m_pDrv,Bin);
    this->m_rrz_in.w = (CAM_READ_BITS(this->m_pDrv,SEP_R1_SEP_CROP,SEP_END_X) - \
                     CAM_READ_BITS(this->m_pDrv,SEP_R1_SEP_CROP,SEP_STR_X) + 1) >> Bin.H;
    this->m_rrz_in.h = CAM_READ_BITS(this->m_pDrv,SEP_R1_SEP_VSIZE,SEP_HT) >> Bin.V;

    if (this->m_bDYUV) {
        Header_CRZO_R1 fh_crzo_r1;
        Header_CRZO_R2 fh_crzo_r2;

        /*crz_r1*/
        this->m_crz_r1_out_wd = CAM_READ_BITS(this->m_pDrv, CRZ_R1_CRZ_OUT_IMG, CRZ_OUT_WD);

        //crz_r1 roi
        this->m_crz_r1_roi.floatX = this->m_crz_r1_roi.floatY = 0;
        tmp = fh_crzo_r1.GetRegInfo(Header_CRZO_R1::E_CRZ_CROP_START,this->m_pDrv);
        this->m_crz_r1_roi.x = (tmp & 0xffff);
        this->m_crz_r1_roi.y = (tmp >> 16);

        tmp = fh_crzo_r1.GetRegInfo(Header_CRZO_R1::E_CRZ_CROP_SIZE,this->m_pDrv);
        this->m_crz_r1_roi.w = (tmp & 0xffff);
        this->m_crz_r1_roi.h = (tmp >> 16);

        //crz_r1_in
        if (isThroughRRZ(this->m_pDrv)) {
            this->m_crz_r1_in.w = ((CAM_READ_BITS(this->m_pDrv, MRG_R13_MRG_CROP, MRG_END_X) - \
                                    CAM_READ_BITS(this->m_pDrv, MRG_R13_MRG_CROP, MRG_STR_X)) + 1);
            this->m_crz_r1_in.h = CAM_READ_BITS(this->m_pDrv, MRG_R13_MRG_VSIZE, MRG_HT);
        }
        else {
            this->m_crz_r1_in.w = this->m_rrz_in.w;
            this->m_crz_r1_in.h = this->m_rrz_in.h;
        }

        /*crz_r2*/
        this->m_crz_r2_out_wd = CAM_READ_BITS(this->m_pDrv, CRZ_R2_CRZ_OUT_IMG, CRZ_OUT_WD);

        //crz_r2 roi
        this->m_crz_r2_roi.floatX = this->m_crz_r2_roi.floatY = 0;
        tmp = fh_crzo_r2.GetRegInfo(Header_CRZO_R2::E_CRZ_CROP_START,this->m_pDrv);
        this->m_crz_r2_roi.x = (tmp & 0xffff);
        this->m_crz_r2_roi.y = (tmp >> 16);

        tmp = fh_crzo_r2.GetRegInfo(Header_CRZO_R2::E_CRZ_CROP_SIZE,this->m_pDrv);
        this->m_crz_r2_roi.w = (tmp & 0xffff);
        this->m_crz_r2_roi.h = (tmp >> 16);

        //crz_r2_in
        if (isThroughRRZ(this->m_pDrv)) {
            this->m_crz_r2_in.w = ((CAM_READ_BITS(this->m_pDrv, MRG_R13_MRG_CROP, MRG_END_X) - \
                                    CAM_READ_BITS(this->m_pDrv, MRG_R13_MRG_CROP, MRG_STR_X)) + 1);
            this->m_crz_r2_in.h = CAM_READ_BITS(this->m_pDrv, MRG_R13_MRG_VSIZE, MRG_HT);
        }
        else {
            this->m_crz_r2_in.w = this->m_rrz_in.w;
            this->m_crz_r2_in.h = this->m_rrz_in.h;
        }
    }

    //af
    this->m_af_vld_xstart = CAM_READ_BITS(this->m_pDrv, AF_R1_AF_VLD, AF_VLD_XSTART);
    this->m_af_blk_xsize = CAM_READ_BITS(this->m_pDrv, AF_R1_AF_BLK_0, AF_BLK_XSIZE);
    this->m_af_x_win_num = CAM_READ_BITS(this->m_pDrv, AF_R1_AF_BLK_1, AF_BLK_XNUM);

    //lsc
    this->m_lsc_lwidth = CAM_READ_BITS(this->m_pDrv, LSC_R1_LSC_LBLOCK,LSC_SDBLK_lWIDTH);
    this->m_lsc_win_num_x = CAM_READ_BITS(this->m_pDrv, LSC_R1_LSC_CTL2,LSC_SDBLK_XNUM);

    //Salve cam modules
    //TG cropping window
    TG_H.Raw = CAM_READ_REG(this->m_pDrv->getPhyObj(),TG_R1_TG_SEN_GRAB_LIN);
    TG_W.Raw = CAM_READ_REG(this->m_pDrv->getPhyObj(),TG_R1_TG_SEN_GRAB_PXL);
    //for twin drv to check size
    CAM_WRITE_REG(this->m_pDrv,TG_R1_TG_SEN_GRAB_PXL,TG_W.Raw);
    CAM_WRITE_REG(this->m_pDrv,TG_R1_TG_SEN_GRAB_LIN,TG_H.Raw);
    for(MUINT32 i = 0; i < slave_num; i++)
    {
        pTwinIspDrv = this->m_pTwinIspDrv_v.at(i);
        pTwinIspDrv->getCurObjInfo(&twinHwModule,&cq,&page);

        //for easy debug
        CAM_WRITE_REG(pTwinIspDrv->getPhyObj(),TG_R1_TG_SEN_GRAB_PXL,TG_W.Raw);
        CAM_WRITE_REG(pTwinIspDrv->getPhyObj(),TG_R1_TG_SEN_GRAB_LIN,TG_H.Raw);

        CAM_FUNC_VRB("[%d_%d] twinHwModule:%d,cq:%d,page:%d\n",slave_num,i,twinHwModule,cq,page);
        dbg_str += std::to_string(twinHwModule) + ",";

        // AF related setting that need to set before run Twin_drv
        if(0x1 == CAM_READ_BITS(this->m_pDrv,CAMCTL_R1_CAMCTL_EN2,CAMCTL_AF_R1_EN))
        {
            MBOOL crp_r1_sel = CAM_READ_BITS(this->m_pDrv, CAMCTL_R1_CAMCTL_SEL, CAMCTL_CRP_R1_SEL);
            CAM_WRITE_BITS(pTwinIspDrv, CAMCTL_R1_CAMCTL_SEL, CAMCTL_CRP_R1_SEL, crp_r1_sel);
        }

        if (CAM_READ_BITS(this->m_pDrv,CAMCTL_R1_CAMCTL_DMA_EN,CAMCTL_IMGO_R1_EN)){
            UPDATE_TWIN_EN(DMA_IMGO,MTRUE);
        } else {
            UPDATE_TWIN_EN(DMA_IMGO,MFALSE);
        }

        if (CAM_READ_BITS(this->m_pDrv,CAMCTL_R1_CAMCTL_DMA_EN,CAMCTL_RRZO_R1_EN)){
            UPDATE_TWIN_EN(DMA_RRZO,MTRUE);
        } else {
            UPDATE_TWIN_EN(DMA_RRZO,MFALSE);
        }

        if(CAM_READ_BITS(this->m_pDrv,CAMCTL_R1_CAMCTL_DMA_EN,CAMCTL_UFEO_R1_EN)){
            UPDATE_TWIN_EN(DMA_UFEO,MTRUE);
            UPDATE_TWIN_EN(BUF_CTRL_UFEO,MTRUE);
        } else {
            UPDATE_TWIN_EN(DMA_UFEO,MFALSE);
            UPDATE_TWIN_EN(BUF_CTRL_UFEO,MFALSE);
        }

        if(CAM_READ_BITS(this->m_pDrv,CAMCTL_R1_CAMCTL_DMA_EN,CAMCTL_UFGO_R1_EN)){
            UPDATE_TWIN_EN(DMA_UFGO,MTRUE);
            UPDATE_TWIN_EN(BUF_CTRL_UFGO,MTRUE);
        } else {
            UPDATE_TWIN_EN(DMA_UFGO,MFALSE);
            UPDATE_TWIN_EN(BUF_CTRL_UFGO,MFALSE);
        }

        if (this->m_bDYUV) {
            if(CAM_READ_BITS(this->m_pDrv,CAMCTL_R1_CAMCTL_DMA2_EN,CAMCTL_YUVO_R1_EN)){
                UPDATE_TWIN_EN(DMA_YUVO,MTRUE);
            } else {
                UPDATE_TWIN_EN(DMA_YUVO,MFALSE);
            }

            if(CAM_READ_BITS(this->m_pDrv,CAMCTL_R1_CAMCTL_DMA_EN,CAMCTL_YUVBO_R1_EN)){
                UPDATE_TWIN_EN(DMA_YUVBO,MTRUE);
                UPDATE_TWIN_EN(BUF_CTRL_YUVBO,MTRUE);
            } else {
                UPDATE_TWIN_EN(DMA_YUVBO,MFALSE);
                UPDATE_TWIN_EN(BUF_CTRL_YUVBO,MFALSE);
            }

            if(CAM_READ_BITS(this->m_pDrv,CAMCTL_R1_CAMCTL_DMA_EN,CAMCTL_YUVCO_R1_EN)){
                UPDATE_TWIN_EN(DMA_YUVCO,MTRUE);
                UPDATE_TWIN_EN(BUF_CTRL_YUVCO,MTRUE);
            } else {
                UPDATE_TWIN_EN(DMA_YUVCO,MFALSE);
                UPDATE_TWIN_EN(BUF_CTRL_YUVCO,MFALSE);
            }

            if(CAM_READ_BITS(this->m_pDrv,CAMCTL_R1_CAMCTL_DMA_EN,CAMCTL_CRZO_R1_EN)){
                UPDATE_TWIN_EN(DMA_CRZO_R1,MTRUE);
            } else {
                UPDATE_TWIN_EN(DMA_CRZO_R1,MFALSE);
            }

            if(CAM_READ_BITS(this->m_pDrv,CAMCTL_R1_CAMCTL_DMA_EN,CAMCTL_CRZBO_R1_EN)){
                UPDATE_TWIN_EN(DMA_CRZBO_R1,MTRUE);
                UPDATE_TWIN_EN(BUF_CTRL_CRZBO_R1,MTRUE);
            } else {
                UPDATE_TWIN_EN(DMA_CRZBO_R1,MFALSE);
                UPDATE_TWIN_EN(BUF_CTRL_CRZBO_R1,MFALSE);
            }

            if(CAM_READ_BITS(this->m_pDrv,CAMCTL_R1_CAMCTL_DMA2_EN,CAMCTL_CRZO_R2_EN)){
                UPDATE_TWIN_EN(DMA_CRZO_R2,MTRUE);
            } else {
                UPDATE_TWIN_EN(DMA_CRZO_R2,MFALSE);
            }

            if(CAM_READ_BITS(this->m_pDrv,CAMCTL_R1_CAMCTL_DMA2_EN,CAMCTL_CRZBO_R2_EN)){
                UPDATE_TWIN_EN(DMA_CRZBO_R2,MTRUE);
                UPDATE_TWIN_EN(BUF_CTRL_CRZBO_R2,MTRUE);
            } else {
                UPDATE_TWIN_EN(DMA_CRZBO_R2,MFALSE);
                UPDATE_TWIN_EN(BUF_CTRL_CRZBO_R2,MFALSE);
            }
        }
    }

    /*
        *under dynamic twin, if uni is linked with CAM_B 1st ,
        * master cam will have no uni info on vir cq under twin mode
        * for dual_cal check only
      */
    CAM_WRITE_BITS(this->m_pDrv,CAMCTL_R1_CAMCTL_EN4,CAMCTL_RLB_R1_EN,1);

    //this is for twin drv's sw request, avoid twin drv running at RTL-verification mode
    this->update_before_twindrv_request();

    CAM_FUNC_DBG("-,slavenum:%d,cq:%d,page:%d,%s\n",slave_num,cq,page,dbg_str.c_str());

#undef UPDATE_TWIN_EN
    return 0;
}
MVOID  CAM_TWIN_PIPE::update_before_twindrv_request(void)
{
#define UPDATE_TWIN_BA(DMAO_C)\
    do{\
        unique_ptr<DMAO_C> ptr(new DMAO_C());\
        ptr->m_pIspDrv = (IspDrvVir*)this->m_pDrv;\
        ptr->dma_cfg.memBuf.base_pAddr = 0x0;\
        ptr->Header_Addr = 0x0;\
        ptr->setBaseAddr();\
        ptr->m_pIspDrv = (IspDrvVir*)pTwinIspDrv;\
        ptr->setBaseAddr();\
    } while(0)

    MUINT32 slave_num = 0,stride = 0, page = 0;
    E_ISP_CAM_CQ cq;
    ISP_DRV_CAM* pTwinIspDrv;
    ISP_HW_MODULE twinHwModule = PHY_CAM;

    if(!slaveNumCheck(slave_num))
        return;

    //partial merge , they will be disabled by dual_cal, so drv need enable them per-frame
    CAM_WRITE_BITS(this->m_pDrv, CAMCTL_R1_CAMCTL_EN3, CAMCTL_MRG_R11_EN, 1);
    CAM_WRITE_BITS(this->m_pDrv, CAMCTL_R1_CAMCTL_EN3, CAMCTL_MRG_R12_EN, 1);

    for(MUINT32 i = 0; i < slave_num; i++)
    {
        pTwinIspDrv = this->m_pTwinIspDrv_v.at(i);
        pTwinIspDrv->getCurObjInfo(&twinHwModule,&cq,&page);

        // 2. AFO stride, set to same as CAMA
        stride = CAM_READ_BITS(this->m_pDrv,AFO_R1_AFO_STRIDE,AFO_STRIDE);
        CAM_WRITE_BITS(pTwinIspDrv, AFO_R1_AFO_STRIDE, AFO_STRIDE, stride);

        //imgo/rrzo baseaddr 0-init,otherwise, twin drv will be run-in C-model mode
        //DMA_EN control of slave cam move in TWIN_PIPE
        if(CAM_READ_BITS(this->m_pDrv,CAMCTL_R1_CAMCTL_DMA_EN,CAMCTL_IMGO_R1_EN)){
            UPDATE_TWIN_BA(DMA_IMGO);
        }
        if(CAM_READ_BITS(this->m_pDrv,CAMCTL_R1_CAMCTL_DMA_EN,CAMCTL_RRZO_R1_EN)){
            UPDATE_TWIN_BA(DMA_RRZO);
        }
        if(CAM_READ_BITS(this->m_pDrv,CAMCTL_R1_CAMCTL_DMA_EN,CAMCTL_UFEO_R1_EN)){
            UPDATE_TWIN_BA(DMA_UFEO);
        }
        if(CAM_READ_BITS(this->m_pDrv,CAMCTL_R1_CAMCTL_DMA_EN,CAMCTL_UFGO_R1_EN)){
            UPDATE_TWIN_BA(DMA_UFGO);
        }
        if(CAM_READ_BITS(this->m_pDrv,CAMCTL_R1_CAMCTL_DMA2_EN,CAMCTL_YUVO_R1_EN)){
            UPDATE_TWIN_BA(DMA_YUVO);
        }
        if(CAM_READ_BITS(this->m_pDrv,CAMCTL_R1_CAMCTL_DMA_EN,CAMCTL_YUVBO_R1_EN)){
            UPDATE_TWIN_BA(DMA_YUVBO);
        }
        if(CAM_READ_BITS(this->m_pDrv,CAMCTL_R1_CAMCTL_DMA_EN,CAMCTL_YUVCO_R1_EN)){
            UPDATE_TWIN_BA(DMA_YUVCO);
        }
        if(CAM_READ_BITS(this->m_pDrv,CAMCTL_R1_CAMCTL_DMA_EN,CAMCTL_CRZO_R1_EN)){
            UPDATE_TWIN_BA(DMA_CRZO_R1);
        }
        if(CAM_READ_BITS(this->m_pDrv,CAMCTL_R1_CAMCTL_DMA_EN,CAMCTL_CRZBO_R1_EN)){
            UPDATE_TWIN_BA(DMA_CRZBO_R1);
        }
        if(CAM_READ_BITS(this->m_pDrv,CAMCTL_R1_CAMCTL_DMA2_EN,CAMCTL_CRZO_R2_EN)){
            UPDATE_TWIN_BA(DMA_CRZO_R2);
        }
        if(CAM_READ_BITS(this->m_pDrv,CAMCTL_R1_CAMCTL_DMA2_EN,CAMCTL_CRZBO_R2_EN)){
            UPDATE_TWIN_BA(DMA_CRZBO_R2);
        }
    }

#undef UPDATE_TWIN_BA
}

MINT32 CAM_TWIN_PIPE::update_afterRunTwin()
{
    MINT32              ret = 0;
    MUINT32             af[2], afo[2], page, slave_num, P1DoneCtl = 0, srcTg;
    static MUINT32      sycn_counter = 0;
    E_ISP_CAM_CQ        cq;
    ISP_DRV_CAM*        pTwinIspDrv;
    ISP_HW_MODULE       twinHwModule;
    capibility          CamInfo;
    MBOOL               bDTwin = CamInfo.m_DTwin.GetDTwin();
    std::string         dbg_str = "twinHwModule: ";

    REG_CAMCTL_R1_CAMCTL_EN      mEn,sEn;
    REG_CAMCTL_R1_CAMCTL_EN2     mEn2,sEn2;
    REG_CAMCTL_R1_CAMCTL_DMA_EN  mDEn,sDEn;

    this->m_pDrv = (ISP_DRV_CAM*)this->m_pIspDrv;
    this->m_pDrv->getCurObjInfo(&this->m_hwModule,&cq,&page);

    if(!slaveNumCheck(slave_num))
        return 1;

    CAM_FUNC_DBG("+, slavenum:%d,cq:%d,page:%d\n", slave_num, cq, page);

    mEn.Raw = CAM_READ_REG(this->m_pDrv,CAMCTL_R1_CAMCTL_EN);
    mEn2.Raw = CAM_READ_REG(this->m_pDrv,CAMCTL_R1_CAMCTL_EN2);
    mDEn.Raw = CAM_READ_REG(this->m_pDrv,CAMCTL_R1_CAMCTL_DMA_EN);
    srcTg = CAM_READ_BITS(this->m_pDrv,CAMCTL_R1_CAMCTL_TWIN_STATUS,SRC_TG);
    for(MUINT32 i = 0; i < slave_num; i++)
    {
        pTwinIspDrv = this->m_pTwinIspDrv_v.at(i);
        pTwinIspDrv->getCurObjInfo(&twinHwModule,&cq,&page);
        sEn.Raw = CAM_READ_REG(pTwinIspDrv,CAMCTL_R1_CAMCTL_EN);
        sEn2.Raw = CAM_READ_REG(pTwinIspDrv,CAMCTL_R1_CAMCTL_EN2);
        sDEn.Raw = CAM_READ_REG(pTwinIspDrv,CAMCTL_R1_CAMCTL_DMA_EN);

        CAM_FUNC_VRB("[%d_%d] twinHwModule:%d,cq:%d,page:%d\n",slave_num,i,twinHwModule,cq,page);
        dbg_str += std::to_string(twinHwModule) + ",";

        //int_en & int2_en
        // Don't enable CAMB's interrupt cause it will get P1_done iRQ as well,  only err r enabled
        CAM_WRITE_REG(pTwinIspDrv,CAMCTL_R1_CAMCTL_INT_EN,\
                CQ_CODE_ERR_EN_|\
                CQ_APB_ERR_EN_ |\
                CQ_VS_ERR_EN_);

        //only afo is needed, because the waiting event of afo's buf_ctrl is afo_done
        CAM_WRITE_BITS(pTwinIspDrv,CAMCTL_R1_CAMCTL_INT2_EN,CAMCTL_AFO_R1_DONE_EN,1);

        // Set SEP_SEL = 1, means slave cam
        CAM_WRITE_BITS(pTwinIspDrv, CAMCTL_R1_CAMCTL_SEL, CAMCTL_SEP_SEL, 0x1);

        // Set SRC TG
        CAM_WRITE_BITS(pTwinIspDrv, CAMCTL_R1_CAMCTL_TWIN_STATUS, SRC_TG, srcTg);

        //LSC
        if(0x1 == mEn.Bits.CAMCTL_LSC_R1_EN){
            MUINT32 addr;
            //
            addr = CAM_READ_REG(this->m_pDrv,LSCI_R1_LSCI_BASE_ADDR);
            CAM_WRITE_REG(pTwinIspDrv,LSCI_R1_LSCI_BASE_ADDR,addr);
            //
            CAM_WRITE_BITS(pTwinIspDrv,CAMCTL_R1_CAMCTL_DMA_EN,CAMCTL_LSCI_R1_EN,1);
        }
        else{
            CAM_WRITE_BITS(pTwinIspDrv,CAMCTL_R1_CAMCTL_DMA_EN,CAMCTL_LSCI_R1_EN,0);
        }

        //BPCI
        if(0x1 == mDEn.Bits.CAMCTL_BPCI_R1_EN ){
            MUINT32 addr;
            //
            addr = CAM_READ_REG(this->m_pDrv,BPCI_R1_BPCI_BASE_ADDR);
            CAM_WRITE_REG(pTwinIspDrv,BPCI_R1_BPCI_BASE_ADDR,addr);
            //
            CAM_WRITE_BITS(pTwinIspDrv,CAMCTL_R1_CAMCTL_DMA_EN,CAMCTL_BPCI_R1_EN,1);
        }
        else{
            CAM_WRITE_BITS(pTwinIspDrv,CAMCTL_R1_CAMCTL_DMA_EN,CAMCTL_BPCI_R1_EN,0);
        }

        //check af & afo, af & afo must be opened at both cam at the same time is needed.
        //afo support no twin fbc ctrl .
        //afo can't be off  because of afo's control timing is not sync with main image
        af[0] = mEn2.Bits.CAMCTL_AF_R1_EN;
        af[1] = sEn2.Bits.CAMCTL_AF_R1_EN;
        afo[0] = mDEn.Bits.CAMCTL_AFO_R1_EN;
        afo[1] = sDEn.Bits.CAMCTL_AFO_R1_EN;

        if(!bDTwin){//write cam_b's header register from cam_A
            ISP_DRV_REG_IO_STRUCT Regs_R[E_HEADER_MAX -1];

            CAM_BURST_READ_REGS(this->m_pDrv,AFO_R1_AFO_FH_SPARE_2,Regs_R,E_HEADER_MAX -1);
            CAM_BURST_WRITE_REGS(pTwinIspDrv,AFO_R1_AFO_FH_SPARE_2,Regs_R,E_HEADER_MAX -1);
        }

        sycn_counter++;
        CAM_WRITE_REG(this->m_pDrv,CAMDMATOP1_R1_CAMDMATOP1_DMA_TWIN_SYNC,sycn_counter);
        CAM_WRITE_REG(pTwinIspDrv,CAMDMATOP1_R1_CAMDMATOP1_DMA_TWIN_SYNC,sycn_counter);

        if(af[0] != af[1]){
            ret = 1;
            CAM_FUNC_ERR("af_en must be the same under twin mode[0x%x_0x%x]\n",af[0],af[1]);
        }
        if(afo[0] != afo[1]){
            ret = 1;
            CAM_FUNC_ERR("afo_en must be the same under twin mode[0x%x_0x%x]\n",afo[0],afo[1]);
        }

#if 1//patch twin drv bug , twin drv at some condition will not copy cam_a's af
        if(!bDTwin){
            MUINT32 afo_ysize = CAM_READ_REG(this->m_pDrv,AFO_R1_AFO_YSIZE);
            CAM_WRITE_REG(pTwinIspDrv,AFO_R1_AFO_YSIZE,afo_ysize);
        }
#endif
        //magic#
        CAM_WRITE_REG(pTwinIspDrv,CAMDMATOP1_R1_CAMDMATOP1_DMA_MAGIC_NUM, CAM_READ_REG(this->m_pDrv,CAMDMATOP1_R1_CAMDMATOP1_DMA_MAGIC_NUM));


        //patch time stamp
        CAM_WRITE_BITS(pTwinIspDrv,CAMCTL_R1_CAMCTL_SEL,CAMCTL_RAW_SEL,3);
        switch(this->m_hwModule){
            case CAM_A:
                CAM_WRITE_BITS(pTwinIspDrv,CAMCTL_R1_CAMCTL_SEL2,CAMCTL_RAW_TG_SEL,FROM_TG_A);
                break;
            default:
                CAM_FUNC_ERR("twin is not supported by this module_%d\n",this->m_hwModule);
                ret = MFALSE;
                break;
        }
    }

    this->update_afterRunTwin_Dma();

    //update twin's cq descriptor
    this->update_cq();

    //update dfs according twin's overhead
    if(this->m_pTwinCB){
        V_CAM_THRU_PUT vThru;
        T_CAM_THRU_PUT Thru;
        REG_SEP_R1_SEP_CROP sep;
        sep.Raw = CAM_READ_REG(this->m_pDrv,SEP_R1_SEP_CROP);
        vThru.clear();

        Thru.SEP_W = (sep.Bits.SEP_END_X - sep.Bits.SEP_STR_X)+1;
        Thru.SEP_H = CAM_READ_REG(this->m_pDrv,SEP_R1_SEP_VSIZE);
        Thru.bBin = mEn2.Bits.CAMCTL_DBN_R1_EN;
        vThru.push_back(Thru);
        for(MUINT32 i = 0; i < slave_num; i++){
            pTwinIspDrv = this->m_pTwinIspDrv_v.at(i);
            Thru.bBin = CAM_READ_BITS(pTwinIspDrv,CAMCTL_R1_CAMCTL_EN2,CAMCTL_DBN_R1_EN);
            sep.Raw = CAM_READ_REG(pTwinIspDrv,SEP_R1_SEP_CROP);
            Thru.SEP_W = (sep.Bits.SEP_END_X - sep.Bits.SEP_STR_X)+1;
            Thru.SEP_H = CAM_READ_REG(pTwinIspDrv,SEP_R1_SEP_VSIZE);
            vThru.push_back(Thru);
        }
        this->m_pTwinCB->p1TuningNotify((MVOID*)&vThru,NULL);
    }


    CAM_FUNC_DBG("-,slavenum:%d,cq:%d,page:%d,%s\n",slave_num,cq,page,dbg_str.c_str());
    return ret;
}

MUINT32 CAM_TWIN_PIPE::update_afterRunTwin_rlb(MUINT32 rlb_oft_prv,MUINT32 rrz_wd_prv,MUINT32 rrz_wd)
{
    MUINT32 rlb_oft = 0, start_oft = 0;
    capibility CamInfo;
    tCAM_rst rst;

    /* rlb, order by [Master1-Slave1-Salve2...] - [Master2-Slave1-...]
       * [output] rlb_oft of cur slave cam
       * [ex.] when current slave cam is Slave2 of Master1,
       * rlb_oft_prv  is rlb_ofst of prv cam = Slave1's rlb_ofst
       * rrz_wd_prv   is rrz_wd   of prv cam = Slave1's rrz_wd
       * -------------------------------------------------------
       * rlb_oft      is rlb_ofst of cur slave cam = Slave2's rlb_ofst
       * rrz_wd       is rrz_wd   of cur slave cam = Slave2's rrz_wd
       */
    CamInfo.GetCapibility(0,NSCam::NSIoPipe::NSCamIOPipe::ENPipeQueryCmd_UNKNOWN,\
        NSCam::NSIoPipe::NSCamIOPipe::NormalPipe_InputInfo(),rst,E_CAM_BS_Alignment);

    start_oft = rlb_oft_prv + rrz_wd_prv;
    if((start_oft % rst.bs_info.bs_alignment) != 0){
        CAM_FUNC_DBG("RLB_OFFSET must be %d alignment:%d\n",rst.bs_info.bs_alignment,start_oft);
        rlb_oft = start_oft + (rst.bs_info.bs_alignment - (start_oft % rst.bs_info.bs_alignment));
    } else
        rlb_oft = start_oft;

    CAM_FUNC_DBG("+,[PrvCam]rlb_oft/rrz_wd(%d/%d)[Slave]rlb_oft/rrz_wd(%d/%d)\n",
        rlb_oft_prv, rrz_wd_prv, rlb_oft,rrz_wd);

    CamInfo.GetCapibility(0,NSCam::NSIoPipe::NSCamIOPipe::ENPipeQueryCmd_UNKNOWN,\
        NSCam::NSIoPipe::NSCamIOPipe::NormalPipe_InputInfo(),rst,E_CAM_BS_Max_size);

    if((rlb_oft + rrz_wd) > rst.bs_info.bs_max_size){
        CAM_FUNC_ERR("rrz_hor over sram size:%d_%d_%d\n",rlb_oft,rrz_wd,rst.bs_info.bs_max_size);
        rlb_oft = 0;
    }

    return rlb_oft;
}

inline ISP_HW_MODULE CAM_TWIN_PIPE::slaveModuleMap(MUINT32 slave_num, ISP_HW_MODULE MhwModule, ISP_HW_MODULE ShwModule)
{
    ISP_HW_MODULE virHwModule = CAM_MAX;
    capibility CamInfo;
    MBOOL bDTwin = CamInfo.m_DTwin.GetDTwin();

    if(!bDTwin)
        return ShwModule;

    if((1 == slave_num) && (CAM_A == MhwModule))
        virHwModule = CAM_A_TWIN_B;
    else
        CAM_FUNC_ERR("combination of master(%d)slave(%d)slave#(%d) is not supported\n",MhwModule, ShwModule, slave_num);

    CAM_FUNC_VRB("virHwModule:%d\n", virHwModule);
    return virHwModule;
}

MINT32 CAM_TWIN_PIPE::update_afterRunTwin_Dma()
{
    L_T_TWIN_DMA channel;
    T_TWIN_DMA dma;
    E_ISP_CAM_CQ cq;
    MUINT32 page, slave_num, rlb_oft = 0, rrz_wd_prv = 0, rrz_wd_slave = 0;
    ISP_DRV_CAM* pTwinIspDrv;
    ISP_HW_MODULE twinHwModule;

    Mutex::Autolock lock(this->m_muList);

    if(!slaveNumCheck(slave_num))
        return 1;

    //afo support no twin fbc ctrl .
    //afo can't be off  because of afo's control timing is not sync with main image

    //rlb
    rlb_oft = this->m_rlb_oft;

    CAM_WRITE_BITS(this->m_pDrv,RRZ_R1_RRZ_RLB_AOFST,RRZ_RLB_AOFST, rlb_oft);
    rrz_wd_prv = CAM_READ_BITS(this->m_pDrv,RRZ_R1_RRZ_OUT_IMG,RRZ_OUT_WD);

    //imgo
    {
        unique_ptr<DMA_IMGO> imgo(new DMA_IMGO());
        unique_ptr<DMA_UFEO> ufeo(new DMA_UFEO());
        unique_ptr<BUF_CTRL_IMGO> fbc_imgo(new BUF_CTRL_IMGO());
        unique_ptr<BUF_CTRL_UFEO> fbc_ufeo(new BUF_CTRL_UFEO());

        dma.bFrameEnd = MFALSE;
        channel.clear();

        for(MUINT32 i = 0; i < slave_num; i++)
        {
            pTwinIspDrv = this->m_pTwinIspDrv_v.at(i);
            pTwinIspDrv->getCurObjInfo(&twinHwModule,&cq,&page);

            fbc_imgo->m_pIspDrv = (IspDrvVir*)pTwinIspDrv;
            fbc_ufeo->m_pIspDrv = (IspDrvVir*)pTwinIspDrv;
            imgo->m_pIspDrv = pTwinIspDrv;
            ufeo->m_pIspDrv = pTwinIspDrv;

            //If the source of imgo of master is directly from TG (img_sel=0), slave cam shouldn't enable IMGO
            if(CAM_READ_BITS(this->m_pDrv,CAMCTL_R1_CAMCTL_SEL, CAMCTL_IMG_SEL) == IMG_SEL_0){
                //
                if(CAM_READ_BITS(pTwinIspDrv,CAMCTL_R1_CAMCTL_DMA_EN,CAMCTL_IMGO_R1_EN)){
                    CAM_FUNC_DBG("force disable slave(%d)'s imgo when master(%d) is pure IMGO\n",
                        twinHwModule, this->m_hwModule);
                    imgo->disable(NULL);
                    ufeo->disable(NULL);
                }
                //
                if (!CAM_READ_BITS(this->m_pDrv,CAMCTL_R1_CAMCTL_DMA_EN,CAMCTL_IMGO_R1_EN)){
                    CAM_FUNC_WRN("IMGO disabled with IMG_SEL_0\n");
                }
            }

            if(CAM_READ_BITS(pTwinIspDrv,CAMCTL_R1_CAMCTL_DMA_EN,CAMCTL_IMGO_R1_EN) ){
                dma.module = slaveModuleMap(slave_num, this->m_hwModule, twinHwModule);
                if(dma.module == CAM_MAX)
                    return 1;

                dma.channel = _imgo_;
                dma.offsetAdr[NSImageio::NSIspio::ePlane_1st] = CAM_READ_REG(pTwinIspDrv,IMGO_R1_IMGO_OFST_ADDR);

                fbc_imgo->enable(NULL);

                if(CAM_READ_BITS(pTwinIspDrv,CAMCTL_R1_CAMCTL_DMA_EN,CAMCTL_UFEO_R1_EN)){
                    fbc_ufeo->enable(NULL);
                    dma.offsetAdr[NSImageio::NSIspio::ePlane_2nd] = CAM_READ_REG(pTwinIspDrv,UFEO_R1_UFEO_OFST_ADDR);
                    ufeo->update_AU();
                    ufeo->enable(NULL);
                }
                else{
                    fbc_ufeo->disable(NULL);
                    dma.offsetAdr[NSImageio::NSIspio::ePlane_2nd] = 0;
                }
                imgo->enable(NULL);
                channel.push_back(dma);
            }
            else{
                fbc_imgo->disable(NULL);
                fbc_ufeo->disable(NULL);
                ufeo->disable(NULL);
                imgo->disable(NULL);
            }

            if((CAM_READ_BITS(this->m_pDrv,CAMCTL_R1_CAMCTL_SEL, CAMCTL_IMG_SEL) != IMG_SEL_0) && //because force diable slave's imgo when master's img_sel=0
               (CAM_READ_BITS(this->m_pDrv,CAMCTL_R1_CAMCTL_DMA_EN,CAMCTL_IMGO_R1_EN) != CAM_READ_BITS(pTwinIspDrv,CAMCTL_R1_CAMCTL_DMA_EN,CAMCTL_IMGO_R1_EN)) ){
                //currently , because of no p1_don/p1_sof at slave cam, arbitrary cropping is not suppoeted!!
                //it will be failed at waitbufready()!
                CAM_FUNC_ERR("imgo support no arbitrary cropping(%d_%d)\n",
                    CAM_READ_BITS(this->m_pDrv,CAMCTL_R1_CAMCTL_DMA_EN,CAMCTL_IMGO_R1_EN), CAM_READ_BITS(pTwinIspDrv,CAMCTL_R1_CAMCTL_DMA_EN,CAMCTL_IMGO_R1_EN));
                return 1;
            }
        }
        //m_dma_*_L & fbc ctl
        fbc_imgo->m_pIspDrv = (IspDrvVir*)this->m_pDrv;
        fbc_ufeo->m_pIspDrv = (IspDrvVir*)this->m_pDrv;
        ufeo->m_pIspDrv = this->m_pDrv;
        imgo->m_pIspDrv = this->m_pDrv;
        if(CAM_READ_BITS(this->m_pDrv,CAMCTL_R1_CAMCTL_DMA_EN,CAMCTL_IMGO_R1_EN) ){
            dma.module = this->m_hwModule;
            dma.channel = _imgo_;
            dma.offsetAdr[NSImageio::NSIspio::ePlane_1st] = CAM_READ_REG(this->m_pDrv,IMGO_R1_IMGO_OFST_ADDR);

            fbc_imgo->enable(NULL);

            if(CAM_READ_BITS(this->m_pDrv,CAMCTL_R1_CAMCTL_DMA_EN,CAMCTL_UFEO_R1_EN)){
                 fbc_ufeo->enable(NULL);
                 ufeo->enable(NULL);
                 dma.offsetAdr[NSImageio::NSIspio::ePlane_2nd] = CAM_READ_REG(this->m_pDrv,UFEO_R1_UFEO_OFST_ADDR);
                 ufeo->update_AU();
            }
            else{
                 fbc_ufeo->disable(NULL);
                 dma.offsetAdr[NSImageio::NSIspio::ePlane_2nd] = 0;
            }
            imgo->enable(NULL);
            channel.push_back(dma);
        }
        else{
            fbc_imgo->disable(NULL);
            fbc_ufeo->disable(NULL);
            ufeo->disable(NULL);
            imgo->disable(NULL);
        }

        //for frame end token
        dma.bFrameEnd = MTRUE;
        channel.push_back(dma);

        // avoid memory leak when m_dma_L don't call clear()
        if((this->m_ConfigDMAEnSt & IMGO_R1_EN_) == IMGO_R1_EN_) {
            this->m_dma_L[TWIN_IMGO_R1].push_back(channel);
            CAM_FUNC_DBG("TWIN_IMGO_R1 push_back!!!");
        } else {
            CAM_FUNC_DBG("NO TWIN_IMGO_R1 push_back!!!");
        }

        //write cam_b's header register from cam_A
        for(MUINT32 i = 0; i < slave_num; i++)
        {
            pTwinIspDrv = this->m_pTwinIspDrv_v.at(i);
            ISP_DRV_REG_IO_STRUCT Regs_R[E_HEADER_MAX -1];

            CAM_BURST_READ_REGS(this->m_pDrv,IMGO_R1_IMGO_FH_SPARE_2,Regs_R,E_HEADER_MAX -1);
            CAM_BURST_WRITE_REGS(pTwinIspDrv,IMGO_R1_IMGO_FH_SPARE_2,Regs_R,E_HEADER_MAX -1);

            if(CAM_READ_BITS(pTwinIspDrv,CAMCTL_R1_CAMCTL_DMA_EN,CAMCTL_UFEO_R1_EN)){
                CAM_BURST_READ_REGS(this->m_pDrv,UFEO_R1_UFEO_FH_SPARE_2,Regs_R,E_HEADER_MAX -1);
                CAM_BURST_WRITE_REGS(pTwinIspDrv,UFEO_R1_UFEO_FH_SPARE_2,Regs_R,E_HEADER_MAX -1);
            }
        }
    }
    //////////////////////////////////////////////////////////////////////////////
    //rrzo
    {
        unique_ptr<DMA_RRZO> rrzo(new DMA_RRZO());
        unique_ptr<DMA_UFGO> ufgo(new DMA_UFGO());
        unique_ptr<BUF_CTRL_RRZO> fbc_rrzo(new BUF_CTRL_RRZO());
        unique_ptr<BUF_CTRL_UFGO> fbc_ufgo(new BUF_CTRL_UFGO());

        dma.bFrameEnd = MFALSE;
        channel.clear();

        for(MUINT32 i = 0; i < slave_num; i++)
        {
            pTwinIspDrv = this->m_pTwinIspDrv_v.at(i);
            pTwinIspDrv->getCurObjInfo(&twinHwModule,&cq,&page);

            fbc_rrzo->m_pIspDrv = (IspDrvVir*)pTwinIspDrv;
            fbc_ufgo->m_pIspDrv = (IspDrvVir*)pTwinIspDrv;
            rrzo->m_pIspDrv = pTwinIspDrv;
            ufgo->m_pIspDrv = pTwinIspDrv;

            if(CAM_READ_BITS(pTwinIspDrv,CAMCTL_R1_CAMCTL_DMA_EN,CAMCTL_RRZO_R1_EN) ){
                dma.module = slaveModuleMap(slave_num, this->m_hwModule, twinHwModule);
                if(dma.module == CAM_MAX)
                    return 1;

                dma.channel = _rrzo_;
                dma.offsetAdr[NSImageio::NSIspio::ePlane_1st] = CAM_READ_REG(pTwinIspDrv,RRZO_R1_RRZO_OFST_ADDR);

                fbc_rrzo->enable(NULL);
                //CAMCTL_UFGO_R1_EN control by dual_cal driver
                if(CAM_READ_BITS(pTwinIspDrv,CAMCTL_R1_CAMCTL_DMA_EN,CAMCTL_UFGO_R1_EN)){
                    fbc_ufgo->enable(NULL);
                    dma.offsetAdr[NSImageio::NSIspio::ePlane_2nd] = CAM_READ_REG(pTwinIspDrv,UFGO_R1_UFGO_OFST_ADDR);
                    ufgo->update_AU();
                    ufgo->enable(NULL);
                }
                else{
                    fbc_ufgo->disable(NULL);
                    dma.offsetAdr[NSImageio::NSIspio::ePlane_2nd] = 0;
                }
                rrzo->enable(NULL);
                channel.push_back(dma);
            }
            else{
                fbc_rrzo->disable(NULL);
                fbc_ufgo->disable(NULL);
                rrzo->disable(NULL);
                ufgo->disable(NULL);
            }

#if (Arbitrary_Crop == 0)
            if(CAM_READ_BITS(this->m_pDrv,CAMCTL_R1_CAMCTL_DMA_EN,CAMCTL_RRZO_R1_EN) != CAM_READ_BITS(pTwinIspDrv,CAMCTL_R1_CAMCTL_DMA_EN,CAMCTL_RRZO_R1_EN) ){
                //currently , because of no p1_don/p1_sof at slave cam, arbitrary cropping is not suppoeted!!
                //it will be failed at waitbufready()!
                CAM_FUNC_WRN("rrzo support no arbitrary cropping(%d_%d), it's ok if crop left side\n",
                    CAM_READ_BITS(this->m_pDrv,CAMCTL_R1_CAMCTL_DMA_EN,CAMCTL_RRZO_R1_EN), CAM_READ_BITS(pTwinIspDrv,CAMCTL_R1_CAMCTL_DMA_EN,CAMCTL_RRZO_R1_EN));
            }
#endif

            //rlb
            if(CAM_READ_BITS(pTwinIspDrv,CAMCTL_R1_CAMCTL_DMA_EN,CAMCTL_RRZO_R1_EN)){
                rrz_wd_slave = CAM_READ_BITS(pTwinIspDrv,RRZ_R1_RRZ_OUT_IMG,RRZ_OUT_WD);
                rlb_oft = update_afterRunTwin_rlb(rlb_oft, rrz_wd_prv, rrz_wd_slave);
                if(!rlb_oft){
                    return 1;
                }
                CAM_WRITE_BITS(pTwinIspDrv,RRZ_R1_RRZ_RLB_AOFST,RRZ_RLB_AOFST, rlb_oft);
                rrz_wd_prv = rrz_wd_slave;
            }
        }
        //m_dma_*_L & fbc ctl
        fbc_rrzo->m_pIspDrv = (IspDrvVir*)this->m_pDrv;
        fbc_ufgo->m_pIspDrv = (IspDrvVir*)this->m_pDrv;
        ufgo->m_pIspDrv = this->m_pDrv;
        rrzo->m_pIspDrv = this->m_pDrv;
        if(CAM_READ_BITS(this->m_pDrv,CAMCTL_R1_CAMCTL_DMA_EN,CAMCTL_RRZO_R1_EN) ){
            dma.module = this->m_hwModule;
            dma.channel = _rrzo_;
            dma.offsetAdr[NSImageio::NSIspio::ePlane_1st] = CAM_READ_REG(this->m_pDrv,RRZO_R1_RRZO_OFST_ADDR);

            fbc_rrzo->enable(NULL);

            if(CAM_READ_BITS(this->m_pDrv,CAMCTL_R1_CAMCTL_DMA_EN,CAMCTL_UFGO_R1_EN)){
                 fbc_ufgo->enable(NULL);
                 dma.offsetAdr[NSImageio::NSIspio::ePlane_2nd] = CAM_READ_REG(this->m_pDrv,UFGO_R1_UFGO_OFST_ADDR);
                 ufgo->update_AU();
                 ufgo->enable(NULL);
            }
            else{
                 fbc_ufgo->disable(NULL);
                 dma.offsetAdr[NSImageio::NSIspio::ePlane_2nd] = 0;
            }

            rrzo->enable(NULL);
            channel.push_back(dma);
        }
        else{
            fbc_rrzo->disable(NULL);
            fbc_ufgo->disable(NULL);
            ufgo->disable(NULL);
            rrzo->disable();
        }

        //for frame end token
        dma.bFrameEnd = MTRUE;
        channel.push_back(dma);

        this->m_dma_L[TWIN_RRZO_R1].push_back(channel);

        //write cam_b's header register from cam_A
        for(MUINT32 i = 0; i < slave_num; i++)
        {
            pTwinIspDrv = this->m_pTwinIspDrv_v.at(i);
            ISP_DRV_REG_IO_STRUCT Regs_R[E_HEADER_MAX -1];

            CAM_BURST_READ_REGS(this->m_pDrv,RRZO_R1_RRZO_FH_SPARE_2,Regs_R,E_HEADER_MAX -1);
            CAM_BURST_WRITE_REGS(pTwinIspDrv,RRZO_R1_RRZO_FH_SPARE_2,Regs_R,E_HEADER_MAX -1);

            if(CAM_READ_BITS(pTwinIspDrv,CAMCTL_R1_CAMCTL_DMA_EN,CAMCTL_UFGO_R1_EN)){
                CAM_BURST_READ_REGS(this->m_pDrv,UFGO_R1_UFGO_FH_SPARE_2,Regs_R,E_HEADER_MAX -1);
                CAM_BURST_WRITE_REGS(pTwinIspDrv,UFGO_R1_UFGO_FH_SPARE_2,Regs_R,E_HEADER_MAX -1);
            }
        }
    }

    if (MFALSE == this->m_bDYUV) {
        goto BYPASS_DYUV;
    }

    //////////////////////////////////////////////////////////////////////////////
    //yuvo
    {
        unique_ptr<DMAO_B> dyuv_dmao[ePlane_max];
        unique_ptr<CAM_BUF_CTRL> fbc_dyuv_dmao[ePlane_max];

        dyuv_dmao[ePlane_1st].reset((DMAO_B*) new DMA_YUVO());
        dyuv_dmao[ePlane_2nd].reset((DMAO_B*) new DMA_YUVBO());
        dyuv_dmao[ePlane_3rd].reset((DMAO_B*) new DMA_YUVCO());
        fbc_dyuv_dmao[ePlane_1st].reset((CAM_BUF_CTRL*) new BUF_CTRL_YUVO());
        fbc_dyuv_dmao[ePlane_2nd].reset((CAM_BUF_CTRL*) new BUF_CTRL_YUVBO());
        fbc_dyuv_dmao[ePlane_3rd].reset((CAM_BUF_CTRL*) new BUF_CTRL_YUVCO());

        dma.bFrameEnd = MFALSE;
        channel.clear();

        for (MUINT32 i = 0; i < slave_num; i++) {
            pTwinIspDrv = this->m_pTwinIspDrv_v.at(i);
            pTwinIspDrv->getCurObjInfo(&twinHwModule,&cq,&page);

            for (MUINT32 PlaneNum = ePlane_1st; PlaneNum <= ePlane_3rd; PlaneNum++){
                dyuv_dmao[PlaneNum]->m_pIspDrv = (IspDrvVir*)pTwinIspDrv;
                fbc_dyuv_dmao[PlaneNum]->m_pIspDrv = (IspDrvVir*)pTwinIspDrv;
            }

            if(CAM_READ_BITS(pTwinIspDrv,CAMCTL_R1_CAMCTL_DMA2_EN,CAMCTL_YUVO_R1_EN)){
                dma.module = slaveModuleMap(slave_num, this->m_hwModule, twinHwModule);

                if(dma.module == CAM_MAX)
                    return 1;

                dma.channel = _yuvo_;
                dma.offsetAdr[ePlane_1st] = CAM_READ_REG(pTwinIspDrv,YUVO_R1_YUVO_OFST_ADDR);

                //
                fbc_dyuv_dmao[ePlane_1st]->enable(NULL);

                if(CAM_READ_BITS(pTwinIspDrv,CAMCTL_R1_CAMCTL_DMA_EN,CAMCTL_YUVBO_R1_EN)){
                    fbc_dyuv_dmao[ePlane_2nd]->enable(NULL);
                    dma.offsetAdr[ePlane_2nd] = CAM_READ_REG(pTwinIspDrv,YUVBO_R1_YUVBO_OFST_ADDR);
                    dyuv_dmao[ePlane_2nd]->enable(NULL);
                }
                else{
                    fbc_dyuv_dmao[ePlane_2nd]->disable(NULL);
                    dma.offsetAdr[ePlane_2nd] = 0;
                }
                if(CAM_READ_BITS(pTwinIspDrv,CAMCTL_R1_CAMCTL_DMA_EN,CAMCTL_YUVCO_R1_EN)){
                    fbc_dyuv_dmao[ePlane_3rd]->enable(NULL);
                    dma.offsetAdr[ePlane_3rd] = CAM_READ_REG(pTwinIspDrv,YUVCO_R1_YUVCO_OFST_ADDR);
                    dyuv_dmao[ePlane_3rd]->enable(NULL);
                }
                else{
                    fbc_dyuv_dmao[ePlane_3rd]->disable(NULL);
                    dma.offsetAdr[ePlane_3rd] = 0;
                }
                dyuv_dmao[ePlane_1st]->enable(NULL);
                channel.push_back(dma);
            }
            else{
                for (MUINT32 PlaneNum = ePlane_1st; PlaneNum <= ePlane_3rd; PlaneNum++){
                    dyuv_dmao[PlaneNum]->disable(NULL);
                    fbc_dyuv_dmao[PlaneNum]->disable(NULL);
                }
            }
        }

        //m_dma_*_L & fbc ctl
        for (MUINT32 PlaneNum = ePlane_1st; PlaneNum <= ePlane_3rd; PlaneNum++){
            dyuv_dmao[PlaneNum]->m_pIspDrv = (IspDrvVir*)this->m_pDrv;
            fbc_dyuv_dmao[PlaneNum]->m_pIspDrv = (IspDrvVir*)this->m_pDrv;
        }

        if(CAM_READ_BITS(this->m_pDrv,CAMCTL_R1_CAMCTL_DMA2_EN,CAMCTL_YUVO_R1_EN)){
            dma.module = this->m_hwModule;
            dma.channel = _yuvo_;
            dma.offsetAdr[ePlane_1st] = CAM_READ_REG(this->m_pDrv,YUVO_R1_YUVO_OFST_ADDR);

            fbc_dyuv_dmao[ePlane_1st]->enable(NULL);

            if(CAM_READ_BITS(this->m_pDrv,CAMCTL_R1_CAMCTL_DMA_EN,CAMCTL_YUVBO_R1_EN)){
                 fbc_dyuv_dmao[ePlane_2nd]->enable(NULL);
                 dma.offsetAdr[ePlane_2nd] = CAM_READ_REG(this->m_pDrv,YUVBO_R1_YUVBO_OFST_ADDR);
                 dyuv_dmao[ePlane_2nd]->enable(NULL);
            }
            else{
                 fbc_dyuv_dmao[ePlane_2nd]->disable(NULL);
                 dma.offsetAdr[ePlane_2nd] = 0;
            }
            if(CAM_READ_BITS(this->m_pDrv,CAMCTL_R1_CAMCTL_DMA_EN,CAMCTL_YUVCO_R1_EN)){
                 fbc_dyuv_dmao[ePlane_3rd]->enable(NULL);
                 dma.offsetAdr[ePlane_3rd] = CAM_READ_REG(this->m_pDrv,YUVCO_R1_YUVCO_OFST_ADDR);
                 dyuv_dmao[ePlane_3rd]->enable(NULL);
            }
            else{
                 fbc_dyuv_dmao[ePlane_3rd]->disable(NULL);
                 dma.offsetAdr[ePlane_3rd] = 0;
            }
            dyuv_dmao[ePlane_1st]->enable(NULL);
            channel.push_back(dma);
        }
        else{
            for (MUINT32 PlaneNum = ePlane_1st; PlaneNum <= ePlane_3rd; PlaneNum++){
                dyuv_dmao[PlaneNum]->disable(NULL);
                fbc_dyuv_dmao[PlaneNum]->disable(NULL);
            }
        }

        //for frame end token
        dma.bFrameEnd = MTRUE;
        channel.push_back(dma);

        if((m_ConfigDMA2EnSt & YUVO_R1_EN_) == YUVO_R1_EN_) {
            this->m_dma_L[TWIN_YUVO_R1].push_back(channel);
            CAM_FUNC_DBG("TWIN_YUVO_R1 push_back!!!");
        } else {
            CAM_FUNC_DBG("NO TWIN_YUVO_R1 push_back!!!");
        }

        //write cam_b's header register from cam_A
        for (MUINT32 i = 0; i < slave_num; i++) {
            pTwinIspDrv = this->m_pTwinIspDrv_v.at(i);
            ISP_DRV_REG_IO_STRUCT Regs_R[E_HEADER_MAX -1];

            CAM_BURST_READ_REGS(this->m_pDrv,YUVO_R1_YUVO_FH_SPARE_2,Regs_R,E_HEADER_MAX -1);
            CAM_BURST_WRITE_REGS(pTwinIspDrv,YUVO_R1_YUVO_FH_SPARE_2,Regs_R,E_HEADER_MAX -1);

            if (CAM_READ_BITS(pTwinIspDrv,CAMCTL_R1_CAMCTL_DMA_EN,CAMCTL_YUVBO_R1_EN)){
                CAM_BURST_READ_REGS(this->m_pDrv,YUVBO_R1_YUVBO_FH_SPARE_2,Regs_R,E_HEADER_MAX -1);
                CAM_BURST_WRITE_REGS(pTwinIspDrv,YUVBO_R1_YUVBO_FH_SPARE_2,Regs_R,E_HEADER_MAX -1);
            }
            if (CAM_READ_BITS(pTwinIspDrv,CAMCTL_R1_CAMCTL_DMA_EN,CAMCTL_YUVCO_R1_EN)){
                CAM_BURST_READ_REGS(this->m_pDrv,YUVCO_R1_YUVCO_FH_SPARE_2,Regs_R,E_HEADER_MAX -1);
                CAM_BURST_WRITE_REGS(pTwinIspDrv,YUVCO_R1_YUVCO_FH_SPARE_2,Regs_R,E_HEADER_MAX -1);
            }
        }
    }

    //////////////////////////////////////////////////////////////////////////////
    //crzo_r1
    {
        unique_ptr<DMAO_B> dyuv_dmao[ePlane_max];
        unique_ptr<CAM_BUF_CTRL> fbc_dyuv_dmao[ePlane_max];

        dyuv_dmao[ePlane_1st].reset((DMAO_B*) new DMA_CRZO_R1());
        dyuv_dmao[ePlane_2nd].reset((DMAO_B*) new DMA_CRZBO_R1());
        fbc_dyuv_dmao[ePlane_1st].reset((CAM_BUF_CTRL*) new BUF_CTRL_CRZO_R1());
        fbc_dyuv_dmao[ePlane_2nd].reset((CAM_BUF_CTRL*) new BUF_CTRL_CRZBO_R1());

        dma.bFrameEnd = MFALSE;
        channel.clear();

        for (MUINT32 i = 0; i < slave_num; i++) {
            pTwinIspDrv = this->m_pTwinIspDrv_v.at(i);
            pTwinIspDrv->getCurObjInfo(&twinHwModule,&cq,&page);

            for (MUINT32 PlaneNum = ePlane_1st; PlaneNum <= ePlane_2nd; PlaneNum++){
                dyuv_dmao[PlaneNum]->m_pIspDrv = (IspDrvVir*)pTwinIspDrv;
                fbc_dyuv_dmao[PlaneNum]->m_pIspDrv = (IspDrvVir*)pTwinIspDrv;
            }

            if(CAM_READ_BITS(pTwinIspDrv,CAMCTL_R1_CAMCTL_DMA_EN,CAMCTL_CRZO_R1_EN) ){
                dma.module = slaveModuleMap(slave_num, this->m_hwModule, twinHwModule);

                if(dma.module == CAM_MAX)
                    return 1;

                dma.channel = _crzo_;
                dma.offsetAdr[ePlane_1st] = CAM_READ_REG(pTwinIspDrv,CRZO_R1_CRZO_OFST_ADDR);

                fbc_dyuv_dmao[ePlane_1st]->enable(NULL);
                //CAMCTL_UFGO_R1_EN control by dual_cal driver
                if(CAM_READ_BITS(pTwinIspDrv,CAMCTL_R1_CAMCTL_DMA_EN,CAMCTL_CRZBO_R1_EN)){
                    fbc_dyuv_dmao[ePlane_2nd]->enable(NULL);
                    dma.offsetAdr[ePlane_2nd] = CAM_READ_REG(pTwinIspDrv,CRZBO_R1_CRZBO_OFST_ADDR);
                    dyuv_dmao[ePlane_2nd]->enable(NULL);
                }
                else{
                    fbc_dyuv_dmao[ePlane_2nd]->disable(NULL);
                    //dma.uf_offsetAdr = 0;
                }
                dyuv_dmao[ePlane_1st]->enable(NULL);
                channel.push_back(dma);
            }
            else{
                for (MUINT32 PlaneNum = ePlane_1st; PlaneNum <= ePlane_2nd; PlaneNum++){
                    dyuv_dmao[PlaneNum]->disable(NULL);
                    fbc_dyuv_dmao[PlaneNum]->disable(NULL);
                }
            }

#if (Arbitrary_Crop == 0)
            if(CAM_READ_BITS(this->m_pDrv,CAMCTL_R1_CAMCTL_DMA_EN,CAMCTL_CRZO_R1_EN) != CAM_READ_BITS(pTwinIspDrv,CAMCTL_R1_CAMCTL_DMA_EN,CAMCTL_CRZO_R1_EN) ){
                //currently , because of no p1_don/p1_sof at slave cam, arbitrary cropping is not suppoeted!!
                //it will be failed at waitbufready()!
                CAM_FUNC_WRN("rrzo support no arbitrary cropping(%d_%d), it's ok if crop left side\n",
                    CAM_READ_BITS(this->m_pDrv,CAMCTL_R1_CAMCTL_DMA_EN,CAMCTL_CRZO_R1_EN), CAM_READ_BITS(pTwinIspDrv,CAMCTL_R1_CAMCTL_DMA_EN,CAMCTL_CRZO_R1_EN));
            }
#endif
        }
        //m_dma_*_L & fbc ctl
        for (MUINT32 PlaneNum = ePlane_1st; PlaneNum <= ePlane_2nd; PlaneNum++) {
            dyuv_dmao[PlaneNum]->m_pIspDrv = (IspDrvVir*)this->m_pDrv;
            fbc_dyuv_dmao[PlaneNum]->m_pIspDrv = (IspDrvVir*)this->m_pDrv;
        }

        if(CAM_READ_BITS(this->m_pDrv,CAMCTL_R1_CAMCTL_DMA_EN,CAMCTL_CRZO_R1_EN) ){
            dma.module = this->m_hwModule;
            dma.channel = _crzo_;
            dma.offsetAdr[ePlane_1st] = CAM_READ_REG(this->m_pDrv,CRZO_R1_CRZO_OFST_ADDR);

            fbc_dyuv_dmao[ePlane_1st]->enable(NULL);

            if(CAM_READ_BITS(this->m_pDrv,CAMCTL_R1_CAMCTL_DMA_EN,CAMCTL_CRZBO_R1_EN)){
                 fbc_dyuv_dmao[ePlane_2nd]->enable(NULL);
                 dma.offsetAdr[ePlane_2nd] = CAM_READ_REG(this->m_pDrv,CRZBO_R1_CRZBO_OFST_ADDR);
                 dyuv_dmao[ePlane_2nd]->enable(NULL);
            }
            else{
                 fbc_dyuv_dmao[ePlane_2nd]->disable(NULL);
                 dma.offsetAdr[ePlane_2nd] = 0;
            }
            dyuv_dmao[ePlane_1st]->enable(NULL);
            channel.push_back(dma);
        }
        else{
            for (MUINT32 PlaneNum = ePlane_1st; PlaneNum <= ePlane_2nd; PlaneNum++){
                dyuv_dmao[PlaneNum]->disable(NULL);
                fbc_dyuv_dmao[PlaneNum]->disable(NULL);
            }
        }

        //for frame end token
        dma.bFrameEnd = MTRUE;
        channel.push_back(dma);

        if ((m_ConfigDMAEnSt & CRZO_R1_EN_) == CRZO_R1_EN_) {
            this->m_dma_L[TWIN_CRZO_R1].push_back(channel);
            CAM_FUNC_DBG("TWIN_CRZO_R1 push_back!!!");
        } else {
            CAM_FUNC_DBG("NO TWIN_CRZO_R1 push_back!!!");
        }
        //write cam_b's header register from cam_A
        for(MUINT32 i = 0; i < slave_num; i++)
        {
            pTwinIspDrv = this->m_pTwinIspDrv_v.at(i);
            ISP_DRV_REG_IO_STRUCT Regs_R[E_HEADER_MAX -1];

            CAM_BURST_READ_REGS(this->m_pDrv,CRZO_R1_CRZO_FH_SPARE_2,Regs_R,E_HEADER_MAX -1);
            CAM_BURST_WRITE_REGS(pTwinIspDrv,CRZO_R1_CRZO_FH_SPARE_2,Regs_R,E_HEADER_MAX -1);

            if(CAM_READ_BITS(pTwinIspDrv,CAMCTL_R1_CAMCTL_DMA_EN,CAMCTL_CRZBO_R1_EN)){
                CAM_BURST_READ_REGS(this->m_pDrv,CRZBO_R1_CRZBO_FH_SPARE_2,Regs_R,E_HEADER_MAX -1);
                CAM_BURST_WRITE_REGS(pTwinIspDrv,CRZBO_R1_CRZBO_FH_SPARE_2,Regs_R,E_HEADER_MAX -1);
            }
        }
    }

    //////////////////////////////////////////////////////////////////////////////
    //crzo_r2
    {
        unique_ptr<DMAO_B> dyuv_dmao[ePlane_max];
        unique_ptr<CAM_BUF_CTRL> fbc_dyuv_dmao[ePlane_max];

        dyuv_dmao[ePlane_1st].reset((DMAO_B*) new DMA_CRZO_R2());
        dyuv_dmao[ePlane_2nd].reset((DMAO_B*) new DMA_CRZBO_R2());
        fbc_dyuv_dmao[ePlane_1st].reset((CAM_BUF_CTRL*) new BUF_CTRL_CRZO_R2());
        fbc_dyuv_dmao[ePlane_2nd].reset((CAM_BUF_CTRL*) new BUF_CTRL_CRZBO_R2());

        dma.bFrameEnd = MFALSE;
        channel.clear();

        for (MUINT32 i = 0; i < slave_num; i++) {
            pTwinIspDrv = this->m_pTwinIspDrv_v.at(i);
            pTwinIspDrv->getCurObjInfo(&twinHwModule,&cq,&page);

            for (MUINT32 PlaneNum = ePlane_1st; PlaneNum <= ePlane_2nd; PlaneNum++) {
                dyuv_dmao[PlaneNum]->m_pIspDrv = (IspDrvVir*)pTwinIspDrv;
                fbc_dyuv_dmao[PlaneNum]->m_pIspDrv = (IspDrvVir*)pTwinIspDrv;
            }

            if(CAM_READ_BITS(pTwinIspDrv,CAMCTL_R1_CAMCTL_DMA2_EN,CAMCTL_CRZO_R2_EN) ){
                dma.module = slaveModuleMap(slave_num, this->m_hwModule, twinHwModule);

                if(dma.module == CAM_MAX)
                    return 1;

                dma.channel = _crzo_r2_;
                dma.offsetAdr[ePlane_1st] = CAM_READ_REG(pTwinIspDrv,CRZO_R2_CRZO_OFST_ADDR);

                fbc_dyuv_dmao[ePlane_1st]->enable(NULL);
                //CAMCTL_UFGO_R1_EN control by dual_cal driver
                if(CAM_READ_BITS(pTwinIspDrv,CAMCTL_R1_CAMCTL_DMA2_EN,CAMCTL_CRZBO_R2_EN)){
                    fbc_dyuv_dmao[ePlane_2nd]->enable(NULL);
                    dma.offsetAdr[ePlane_2nd] = CAM_READ_REG(pTwinIspDrv,CRZBO_R2_CRZBO_OFST_ADDR);
                    dyuv_dmao[ePlane_2nd]->enable(NULL);
                }
                else{
                    fbc_dyuv_dmao[ePlane_2nd]->disable(NULL);
                    dma.offsetAdr[ePlane_2nd] = 0;
                }
                dyuv_dmao[ePlane_1st]->enable(NULL);
                channel.push_back(dma);
            }
            else{
                for (MUINT32 PlaneNum = ePlane_1st; PlaneNum <= ePlane_2nd; PlaneNum++){
                    dyuv_dmao[PlaneNum]->disable(NULL);
                    fbc_dyuv_dmao[PlaneNum]->disable(NULL);
                }
            }

#if (Arbitrary_Crop == 0)
            if(CAM_READ_BITS(this->m_pDrv,CAMCTL_R1_CAMCTL_DMA2_EN,CAMCTL_CRZO_R2_EN) != CAM_READ_BITS(pTwinIspDrv,CAMCTL_R1_CAMCTL_DMA2_EN,CAMCTL_CRZO_R2_EN) ){
                //currently , because of no p1_don/p1_sof at slave cam, arbitrary cropping is not suppoeted!!
                //it will be failed at waitbufready()!
                CAM_FUNC_WRN("rrzo support no arbitrary cropping(%d_%d), it's ok if crop left side\n",
                    CAM_READ_BITS(this->m_pDrv,CAMCTL_R1_CAMCTL_DMA2_EN,CAMCTL_CRZO_R2_EN), CAM_READ_BITS(pTwinIspDrv,CAMCTL_R1_CAMCTL_DMA2_EN,CAMCTL_CRZO_R2_EN));
            }
#endif
        }
        //m_dma_*_L & fbc ctl
        for (MUINT32 PlaneNum = ePlane_1st; PlaneNum <= ePlane_2nd; PlaneNum++) {
            dyuv_dmao[PlaneNum]->m_pIspDrv = (IspDrvVir*)this->m_pDrv;
            fbc_dyuv_dmao[PlaneNum]->m_pIspDrv = (IspDrvVir*)this->m_pDrv;
        }

        if(CAM_READ_BITS(this->m_pDrv,CAMCTL_R1_CAMCTL_DMA2_EN,CAMCTL_CRZO_R2_EN) ){
            dma.module = this->m_hwModule;
            dma.channel = _crzo_r2_;
            dma.offsetAdr[ePlane_1st] = CAM_READ_REG(this->m_pDrv,CRZO_R2_CRZO_OFST_ADDR);

            fbc_dyuv_dmao[ePlane_1st]->enable(NULL);

            if(CAM_READ_BITS(this->m_pDrv,CAMCTL_R1_CAMCTL_DMA2_EN,CAMCTL_CRZBO_R2_EN)){
                 fbc_dyuv_dmao[ePlane_2nd]->enable(NULL);
                 dma.offsetAdr[ePlane_2nd] = CAM_READ_REG(this->m_pDrv,CRZBO_R2_CRZBO_OFST_ADDR);
                 dyuv_dmao[ePlane_2nd]->enable(NULL);
            }
            else{
                 fbc_dyuv_dmao[ePlane_2nd]->disable(NULL);
                 dma.offsetAdr[ePlane_2nd] = 0;
            }
            dyuv_dmao[ePlane_1st]->enable(NULL);
            channel.push_back(dma);
        }
        else{
            for (MUINT32 PlaneNum = ePlane_1st; PlaneNum <= ePlane_2nd; PlaneNum++){
                dyuv_dmao[PlaneNum]->disable(NULL);
                fbc_dyuv_dmao[PlaneNum]->disable(NULL);
            }
        }

        //for frame end token
        dma.bFrameEnd = MTRUE;
        channel.push_back(dma);

        if((m_ConfigDMA2EnSt & CRZO_R2_EN_) == CRZO_R2_EN_) {
            this->m_dma_L[TWIN_CRZO_R2].push_back(channel);
            CAM_FUNC_DBG("TWIN_CRZO_R2 push_back!!!");
        } else {
            CAM_FUNC_DBG("NO TWIN_CRZO_R2 push_back!!!");
        }

        //write cam_b's header register from cam_A
        for(MUINT32 i = 0; i < slave_num; i++)
        {
            pTwinIspDrv = this->m_pTwinIspDrv_v.at(i);
            ISP_DRV_REG_IO_STRUCT Regs_R[E_HEADER_MAX -1];

            CAM_BURST_READ_REGS(this->m_pDrv,CRZO_R2_CRZO_FH_SPARE_2,Regs_R,E_HEADER_MAX -1);
            CAM_BURST_WRITE_REGS(pTwinIspDrv,CRZO_R2_CRZO_FH_SPARE_2,Regs_R,E_HEADER_MAX -1);

            if(CAM_READ_BITS(pTwinIspDrv,CAMCTL_R1_CAMCTL_DMA2_EN,CAMCTL_CRZBO_R2_EN)){
                CAM_BURST_READ_REGS(this->m_pDrv,CRZBO_R2_CRZBO_FH_SPARE_2,Regs_R,E_HEADER_MAX -1);
                CAM_BURST_WRITE_REGS(pTwinIspDrv,CRZBO_R2_CRZBO_FH_SPARE_2,Regs_R,E_HEADER_MAX -1);
            }
        }
    }

BYPASS_DYUV:

    //write cam_b's header register from cam_A
    this->update_afterRunTwin_FH(slave_num, this->m_pTwinIspDrv_v);

    return 0;
}

MINT32 CAM_TWIN_PIPE::update_afterRunTwin_FH(MUINT32 slave_num, vector<ISP_DRV_CAM *> &v_slaveDrv)
{
    MUINT32 value;
    ISP_DRV_CAM* pTwinIspDrv;
    Header_RRZO fh_rrzo;

    this->m_pDrv = (ISP_DRV_CAM*)this->m_pIspDrv;

    /*RAW*/
    for(MUINT32 i = 0; i < slave_num; i++)
    {
        pTwinIspDrv = this->m_pTwinIspDrv_v.at(i);
        if(CAM_READ_BITS(v_slaveDrv.at(i),CAMCTL_R1_CAMCTL_DMA_EN,CAMCTL_IMGO_R1_EN)){
            Header_IMGO fh_imgo;

            #if 1
            value = CAM_READ_BITS(this->m_pDrv,IMGO_R1_IMGO_YSIZE,IMGO_YSIZE);
            CAM_WRITE_BITS(pTwinIspDrv,IMGO_R1_IMGO_YSIZE,IMGO_YSIZE,value);
            #endif

            value = fh_imgo.GetRegInfo(Header_IMGO::E_IMG_PA,(IspDrv *)this->m_pDrv);
            fh_imgo.Header_Enque(Header_IMGO::E_IMG_PA,pTwinIspDrv,value);

            value = fh_imgo.GetRegInfo(Header_IMGO::E_ENQUE_SOF,(IspDrv *)this->m_pDrv);
            fh_imgo.Header_Enque(Header_IMGO::E_ENQUE_SOF,pTwinIspDrv,value);

            if(CAM_READ_BITS(v_slaveDrv.at(i),CAMCTL_R1_CAMCTL_DMA_EN,CAMCTL_UFEO_R1_EN)){
                Header_UFEO fh_ufeo;

                value = fh_ufeo.GetRegInfo(Header_UFEO::E_IMG_PA,(IspDrv *)this->m_pDrv);
                fh_ufeo.Header_Enque(Header_UFEO::E_IMG_PA,pTwinIspDrv,value);

                value = fh_ufeo.GetRegInfo(Header_UFEO::E_ENQUE_SOF,(IspDrv *)this->m_pDrv);
                fh_ufeo.Header_Enque(Header_UFEO::E_ENQUE_SOF,pTwinIspDrv,value);
            }
        }

        #if 1
        value = CAM_READ_BITS(this->m_pDrv,RRZO_R1_RRZO_YSIZE,RRZO_YSIZE);
        CAM_WRITE_BITS(pTwinIspDrv,RRZO_R1_RRZO_YSIZE,RRZO_YSIZE,value);
        #endif

        value = fh_rrzo.GetRegInfo(Header_RRZO::E_IMG_PA,(IspDrv *)this->m_pDrv);
        fh_rrzo.Header_Enque(Header_RRZO::E_IMG_PA,pTwinIspDrv,value);

        value = fh_rrzo.GetRegInfo(Header_RRZO::E_IS_UFG_FMT,(IspDrv *)this->m_pDrv);
        fh_rrzo.Header_Enque(Header_RRZO::E_IS_UFG_FMT,pTwinIspDrv,value);

        value = fh_rrzo.GetRegInfo(Header_RRZO::E_ENQUE_SOF,(IspDrv *)this->m_pDrv);
        fh_rrzo.Header_Enque(Header_RRZO::E_ENQUE_SOF,pTwinIspDrv,value);

        if(CAM_READ_BITS(v_slaveDrv.at(i),CAMCTL_R1_CAMCTL_DMA_EN,CAMCTL_UFGO_R1_EN)){
            Header_UFGO fh_ufgo;

            value = fh_ufgo.GetRegInfo(Header_UFGO::E_IMG_PA,(IspDrv *)this->m_pDrv);
            fh_ufgo.Header_Enque(Header_UFGO::E_IMG_PA,pTwinIspDrv,value);

            value = fh_ufgo.GetRegInfo(Header_UFGO::E_ENQUE_SOF,(IspDrv *)this->m_pDrv);
            fh_ufgo.Header_Enque(Header_UFGO::E_ENQUE_SOF,pTwinIspDrv,value);
        }
    }

    /*DYUV*/
    for(MUINT32 i = 0; i < slave_num; i++) {
        pTwinIspDrv = this->m_pTwinIspDrv_v.at(i);

        /*yuvo*/
        if(CAM_READ_BITS(v_slaveDrv.at(i),CAMCTL_R1_CAMCTL_DMA2_EN,CAMCTL_YUVO_R1_EN)) {
            Header_YUVO fh_yuvo;
            value = fh_yuvo.GetRegInfo(Header_YUVO::E_IMG_PA,(IspDrv *)this->m_pDrv);
            fh_yuvo.Header_Enque(Header_YUVO::E_IMG_PA,pTwinIspDrv,value);

            value = fh_yuvo.GetRegInfo(Header_YUVO::E_ENQUE_SOF,(IspDrv *)this->m_pDrv);
            fh_yuvo.Header_Enque(Header_YUVO::E_ENQUE_SOF,pTwinIspDrv,value);

            if(CAM_READ_BITS(v_slaveDrv.at(i),CAMCTL_R1_CAMCTL_DMA_EN,CAMCTL_YUVBO_R1_EN)) {
                Header_YUVBO fh_yuvbo;
                value = fh_yuvbo.GetRegInfo(Header_YUVBO::E_IMG_PA,(IspDrv *)this->m_pDrv);
                fh_yuvbo.Header_Enque(Header_YUVBO::E_IMG_PA,pTwinIspDrv,value);

                value = fh_yuvbo.GetRegInfo(Header_YUVBO::E_ENQUE_SOF,(IspDrv *)this->m_pDrv);
                fh_yuvbo.Header_Enque(Header_YUVBO::E_ENQUE_SOF,pTwinIspDrv,value);

            }
            if(CAM_READ_BITS(v_slaveDrv.at(i),CAMCTL_R1_CAMCTL_DMA_EN,CAMCTL_YUVCO_R1_EN)) {
                Header_YUVCO fh_yuvco;
                value = fh_yuvco.GetRegInfo(Header_YUVCO::E_IMG_PA,(IspDrv *)this->m_pDrv);
                fh_yuvco.Header_Enque(Header_YUVCO::E_IMG_PA,pTwinIspDrv,value);

                value = fh_yuvco.GetRegInfo(Header_YUVCO::E_ENQUE_SOF,(IspDrv *)this->m_pDrv);
                fh_yuvco.Header_Enque(Header_YUVCO::E_ENQUE_SOF,pTwinIspDrv,value);
            }
        }

        /*crzo_r1*/
        if(CAM_READ_BITS(v_slaveDrv.at(i),CAMCTL_R1_CAMCTL_DMA_EN,CAMCTL_CRZO_R1_EN)){
            Header_CRZO_R1 fh_crzo_r1;

            value = fh_crzo_r1.GetRegInfo(Header_CRZO_R1::E_IMG_PA,(IspDrv *)this->m_pDrv);
            fh_crzo_r1.Header_Enque(Header_CRZO_R1::E_IMG_PA,pTwinIspDrv,value);

            value = fh_crzo_r1.GetRegInfo(Header_CRZO_R1::E_SW_FMT,(IspDrv *)this->m_pDrv);
            fh_crzo_r1.Header_Enque(Header_CRZO_R1::E_SW_FMT,pTwinIspDrv,value);

            value = fh_crzo_r1.GetRegInfo(Header_CRZO_R1::E_ENQUE_SOF,(IspDrv *)this->m_pDrv);
            fh_crzo_r1.Header_Enque(Header_CRZO_R1::E_ENQUE_SOF,pTwinIspDrv,value);

            if(CAM_READ_BITS(v_slaveDrv.at(i),CAMCTL_R1_CAMCTL_DMA_EN,CAMCTL_CRZBO_R1_EN)){
                Header_CRZBO_R1 fh_crzbo_r1;

                value = fh_crzbo_r1.GetRegInfo(Header_CRZBO_R1::E_IMG_PA,(IspDrv *)this->m_pDrv);
                fh_crzbo_r1.Header_Enque(Header_CRZBO_R1::E_IMG_PA,pTwinIspDrv,value);

                value = fh_crzbo_r1.GetRegInfo(Header_CRZBO_R1::E_ENQUE_SOF,(IspDrv *)this->m_pDrv);
                fh_crzbo_r1.Header_Enque(Header_CRZBO_R1::E_ENQUE_SOF,pTwinIspDrv,value);
            }
        }

        /*crzo_r2*/
        if(CAM_READ_BITS(v_slaveDrv.at(i),CAMCTL_R1_CAMCTL_DMA2_EN,CAMCTL_CRZO_R2_EN)){
            Header_CRZO_R2 fh_crzo_r2;

            value = fh_crzo_r2.GetRegInfo(Header_CRZO_R2::E_IMG_PA,(IspDrv *)this->m_pDrv);
            fh_crzo_r2.Header_Enque(Header_CRZO_R2::E_IMG_PA,pTwinIspDrv,value);

            value = fh_crzo_r2.GetRegInfo(Header_CRZO_R2::E_SW_FMT,(IspDrv *)this->m_pDrv);
            fh_crzo_r2.Header_Enque(Header_CRZO_R2::E_SW_FMT,pTwinIspDrv,value);

            value = fh_crzo_r2.GetRegInfo(Header_CRZO_R2::E_ENQUE_SOF,(IspDrv *)this->m_pDrv);
            fh_crzo_r2.Header_Enque(Header_CRZO_R2::E_ENQUE_SOF,pTwinIspDrv,value);

            if(CAM_READ_BITS(v_slaveDrv.at(i),CAMCTL_R1_CAMCTL_DMA2_EN,CAMCTL_CRZBO_R2_EN)){
                Header_CRZBO_R2 fh_crzbo_r2;

                value = fh_crzbo_r2.GetRegInfo(Header_CRZBO_R2::E_IMG_PA,(IspDrv *)this->m_pDrv);
                fh_crzbo_r2.Header_Enque(Header_CRZBO_R2::E_IMG_PA,pTwinIspDrv,value);

                value = fh_crzbo_r2.GetRegInfo(Header_CRZBO_R2::E_ENQUE_SOF,(IspDrv *)this->m_pDrv);
                fh_crzbo_r2.Header_Enque(Header_CRZBO_R2::E_ENQUE_SOF,pTwinIspDrv,value);
            }
        }
    }

BYPASS_DYUV:

    return 0;
}

MINT32 CAM_TWIN_PIPE::get_RunTwinRst(MUINT32 dmachannel,vector<L_T_TWIN_DMA>& channel,MBOOL bDeque)
{
    vector<L_T_TWIN_DMA>* pList = NULL;
    Mutex::Autolock lock(this->m_muList);
    MUINT32 _port = 0;

    switch(dmachannel){
        case _imgo_:
        case _rrzo_:
        case _yuvo_:
        case _crzo_:
        case _crzo_r2_:
            _port = MAP_TWIN_DMAO(dmachannel, MFALSE, CAM_FUNC);
            pList = &this->m_dma_L[_port];
            break;
        default:
            CAM_FUNC_ERR("unsupported dma:%d\n",dmachannel);
            return 1;
            break;
    }
    if(pList->size() == 0){
        CAM_FUNC_ERR("get no twin result\n");
        return 1;
    }

    channel.clear();
    if(bDeque){
        //deque is from oldest data
        for(MUINT32 i=0;i<(this->m_subsample+1);i++){
            //smvr480 channel.push_back(pList->at(i));
            channel.push_back(pList->at(0));
        }
    }
    else{//enque is from latest data
        #if 0
        if(pList->size() < (this->m_subsample+1) ){
            CAM_FUNC_ERR("logic error: %d_%d\n",(MUINT32)pList->size(),(this->m_subsample+1));
            return 1;
        }
        #endif

        //
        for(MUINT32 i= 0;i<(this->m_subsample+1);i++){
            //smvr480 channel.push_back(pList->at(i + (pList->size()-(this->m_subsample+1))));
            channel.push_back(pList->at(pList->size() - 1));
        }
    }

    return 0;
}

MINT32 CAM_TWIN_PIPE::clr_RunTwinRst(MUINT32 dmachannel)
{
    vector<L_T_TWIN_DMA>* pList = NULL;
    Mutex::Autolock lock(this->m_muList);
    MUINT32 _port = 0;

    switch(dmachannel){
        case _imgo_:
        case _rrzo_:
        case _yuvo_:
        case _crzo_:
        case _crzo_r2_:
            _port = MAP_TWIN_DMAO(dmachannel, MFALSE, CAM_FUNC);
            pList = &this->m_dma_L[_port];
            break;
        default:
            CAM_FUNC_ERR("unsupported dma:%d\n",dmachannel);
            return 1;
            break;
    }

    //remove record
    #if 0 //smvr480
    if(this->m_subsample){
        vector<L_T_TWIN_DMA>::iterator it;
        for(it=pList->begin();it<(pList->begin() + this->m_subsample);it++){
            it->clear();
        }
        pList->erase(pList->begin(),pList->begin() + this->m_subsample);
    }
    else
    #endif
    {
        pList->begin()->clear();
        pList->erase(pList->begin());
    }

    return 0;
}

MINT32 CAM_TWIN_PIPE::suspend( void )
{
    E_ISP_CAM_CQ cq;
    MUINT32 page, slave_num;
    ISP_DRV_CAM* pTwinIspDrv;
    ISP_HW_MODULE twinHwModule;
    std::string dbg_str = "twinHwModule: ";

    this->m_pDrv = (ISP_DRV_CAM*)this->m_pIspDrv;
    this->m_pDrv->getCurObjInfo(&this->m_hwModule,&cq,&page);

    if(!slaveNumCheck(slave_num))
        return 1;

    CAM_FUNC_DBG("+, slavenum:%d,cq:%d,page:%d\n", slave_num, cq, page);

    CAM_WRITE_BITS(this->m_pDrv, CAMCTL_R1_CAMCTL_TWIN_STATUS, SLAVE_CAM_NUM, 0);
    CAM_WRITE_BITS(this->m_pDrv, CAMCTL_R1_CAMCTL_TWIN_STATUS, TWIN_EN,  0x0);
    //
    for(MUINT32 i = 0; i < slave_num; i++)
    {
        pTwinIspDrv = this->m_pTwinIspDrv_v.at(i);
        pTwinIspDrv->getCurObjInfo(&twinHwModule,&cq,&page);

        CAM_FUNC_VRB("[%d_%d] twinHwModule:%d,cq:%d,page:%d\n",slave_num,i,twinHwModule,cq,page);
        dbg_str += std::to_string(twinHwModule) + ",";

        //update twin status
        CAM_WRITE_BITS(pTwinIspDrv, CAMCTL_R1_CAMCTL_TWIN_STATUS,TWIN_EN,  0x0);
        CAM_WRITE_BITS(pTwinIspDrv, CAMCTL_R1_CAMCTL_TWIN_STATUS, SLAVE_CAM_NUM,0x0);
        if(CAM_READ_BITS(this->m_pDrv, CAMCTL_R1_CAMCTL_TWIN_STATUS,TWIN_MODULE) == twinHwModule){
             CAM_WRITE_BITS(this->m_pDrv, CAMCTL_R1_CAMCTL_TWIN_STATUS,TWIN_MODULE, CAM_MAX);
             CAM_WRITE_BITS(pTwinIspDrv, CAMCTL_R1_CAMCTL_TWIN_STATUS,TWIN_MODULE, CAM_MAX);
        }

        // !!Must set CRP_R1_SEL = 0, or run other case CAMA will enconter error
        CAM_WRITE_BITS(pTwinIspDrv, CAMCTL_R1_CAMCTL_SEL, CAMCTL_CRP_R1_SEL, 0x0);

        //Enable Last P1 done, but ref itself
        CAM_WRITE_BITS(pTwinIspDrv, CAMCTL_R1_CAMCTL_SEL, CAMCTL_SEP_SEL, 0x0);
        CAM_WRITE_BITS(this->m_pDrv, CAMCTL_R1_CAMCTL_MISC, CAMCTL_LAST_PASS1_DONE_CTL, GetP1DoneRef(this->m_hwModule));
        CAM_WRITE_BITS(pTwinIspDrv, CAMCTL_R1_CAMCTL_MISC, CAMCTL_LAST_PASS1_DONE_CTL, GetP1DoneRef(twinHwModule));

        //need to disable rcnt_inc.
        //because we need this page to switch dmx, but we don't want to have impact on fbc

#if (BMX_AMX_DTWIN_WROK_AROUND == 1)
        CAM_WRITE_REG(pTwinIspDrv,CAMCTL_R1_CAMCTL_EN, 0);
        CAM_WRITE_REG(pTwinIspDrv,CAMCTL_R1_CAMCTL_EN2, 0);
        CAM_WRITE_REG(pTwinIspDrv,CAMCTL_R1_CAMCTL_EN3, 0);
        CAM_WRITE_REG(pTwinIspDrv,CAMCTL_R1_CAMCTL_EN4, 0);
#endif

        CAM_WRITE_REG(pTwinIspDrv,CAMCTL_R1_CAMCTL_FBC_RCNT_INC,0);

        //close fbc to prevent fbc counter abnormal
        if(CAM_READ_BITS(pTwinIspDrv,CAMCTL_R1_CAMCTL_DMA_EN,CAMCTL_IMGO_R1_EN))
            CAM_WRITE_BITS(pTwinIspDrv,FBC_R1_FBC_IMGO_R1_CTL1,FBC_IMGO_R1_FBC_EN,0);

        if(CAM_READ_BITS(pTwinIspDrv,CAMCTL_R1_CAMCTL_DMA_EN,CAMCTL_RRZO_R1_EN))
            CAM_WRITE_BITS(pTwinIspDrv,FBC_R1_FBC_RRZO_R1_CTL1,FBC_RRZO_R1_FBC_EN,0);

        if(CAM_READ_BITS(pTwinIspDrv,CAMCTL_R1_CAMCTL_DMA_EN,CAMCTL_AFO_R1_EN))
            CAM_WRITE_BITS(pTwinIspDrv,FBC_R1_FBC_AFO_R1_CTL1,FBC_AFO_R1_FBC_EN,0);

        if(CAM_READ_BITS(pTwinIspDrv,CAMCTL_R1_CAMCTL_DMA2_EN,CAMCTL_YUVO_R1_EN))
            CAM_WRITE_BITS(pTwinIspDrv,FBC_R1_FBC_YUVO_R1_CTL1,FBC_YUVO_R1_FBC_EN,0);

        if(CAM_READ_BITS(pTwinIspDrv,CAMCTL_R1_CAMCTL_DMA_EN,CAMCTL_CRZO_R1_EN))
            CAM_WRITE_BITS(pTwinIspDrv,FBC_R1_FBC_CRZO_R1_CTL1,FBC_CRZO_R1_FBC_EN,0);

        if(CAM_READ_BITS(pTwinIspDrv,CAMCTL_R1_CAMCTL_DMA2_EN,CAMCTL_CRZO_R2_EN))
            CAM_WRITE_BITS(pTwinIspDrv,FBC_R1_FBC_CRZO_R2_CTL1,FBC_CRZO_R2_FBC_EN,0);
    }

    //for suspending at first 3 pages, slave cam's cq descriptor
    this->update_cq_suspend();

    CAM_FUNC_DBG("-,slavenum:%d,cq:%d,page:%d,%s\n",slave_num,cq,page,dbg_str.c_str());

    return 0;
}

MINT32 CAM_TWIN_PIPE::resume( void )
{
    MINT32 ret = 0;
    E_ISP_CAM_CQ cq;
    MUINT32 page, slave_num, P1DoneCtl = 0;
    ISP_DRV_CAM* pTwinIspDrv;
    ISP_DRV_CAM* pTwinIspDrvPrv = NULL;
    ISP_HW_MODULE twinHwModule, twinHwModulePrv = CAM_MAX;
    std::string dbg_str = "twinHwModule: ";

    this->m_pDrv = (ISP_DRV_CAM*)this->m_pIspDrv;
    this->m_pDrv->getCurObjInfo(&this->m_hwModule,&cq,&page);

    if(!slaveNumCheck(slave_num))
        return 1;

    CAM_FUNC_DBG("+, slavenum:%d,cq:%d,page:%d\n", slave_num, cq, page);
    CAM_WRITE_BITS(this->m_pDrv, CAMCTL_R1_CAMCTL_TWIN_STATUS,TWIN_EN,  0x1);
    CAM_WRITE_BITS(this->m_pDrv, CAMCTL_R1_CAMCTL_TWIN_STATUS,SLAVE_CAM_NUM, slave_num);

    for(MUINT32 i = 0; i < slave_num; i++)
    {
        pTwinIspDrv = this->m_pTwinIspDrv_v.at(i);
        pTwinIspDrv->getCurObjInfo(&twinHwModule,&cq,&page);

        CAM_FUNC_VRB("[%d_%d] twinHwModule:%d,cq:%d,page:%d\n",slave_num,i,twinHwModule,cq,page);
        dbg_str += std::to_string(twinHwModule) + ",";

        CAM_WRITE_BITS(pTwinIspDrv->getPhyObj(),CAMCTL_R1_CAMCTL_MISC,CAMCTL_DB_EN,1);

        //update twin status
        CAM_WRITE_BITS(pTwinIspDrv, CAMCTL_R1_CAMCTL_TWIN_STATUS, TWIN_EN,  0x1);
        CAM_WRITE_BITS(pTwinIspDrv, CAMCTL_R1_CAMCTL_TWIN_STATUS, SLAVE_CAM_NUM, slave_num);
        CAM_WRITE_BITS(pTwinIspDrv, CAMCTL_R1_CAMCTL_TWIN_STATUS, MASTER_MODULE, this->m_hwModule);

        // special, w via vir
        CAM_WRITE_REG(pTwinIspDrv,CAMDMATOP1_R1_CAMDMATOP1_DMA_RSV11,(CAM_READ_REG(pTwinIspDrv->getPhyObj(),CAMDMATOP1_R1_CAMDMATOP1_DMA_RSV11)&0x7fffffff));//bit31 for smi mask, blocking imgo output
        CAM_WRITE_REG(pTwinIspDrv,CAMDMATOP1_R1_CAMDMATOP1_DMA_RSV16,0xffffffff);

        if(i == 0){
            CAM_WRITE_BITS(this->m_pDrv, CAMCTL_R1_CAMCTL_TWIN_STATUS, TWIN_MODULE, twinHwModule);
            CAM_WRITE_BITS(pTwinIspDrv, CAMCTL_R1_CAMCTL_TWIN_STATUS,  TWIN_MODULE, twinHwModule);
        }

        // !!Must set CAMB's CRP_R1_SEL = 0, or run other case CAMA will enconter error
        CAM_WRITE_BITS(pTwinIspDrv, CAMCTL_R1_CAMCTL_SEL, CAMCTL_CRP_R1_SEL, 0x1);

        // Enable Last P1 Done, and ref Master and Slave
        CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),CAMCTL_R1_CAMCTL_MISC,CAMCTL_PASS1_DONE_SEL,0);
        P1DoneCtl = (CAM_READ_BITS(this->m_pDrv->getPhyObj(), CAMCTL_R1_CAMCTL_MISC, CAMCTL_LAST_PASS1_DONE_CTL) | GetP1DoneRef(twinHwModule));
        CAM_WRITE_BITS(this->m_pDrv, CAMCTL_R1_CAMCTL_MISC, CAMCTL_LAST_PASS1_DONE_CTL, P1DoneCtl);
        CAM_WRITE_BITS(pTwinIspDrv, CAMCTL_R1_CAMCTL_MISC, CAMCTL_LAST_PASS1_DONE_CTL, P1DoneCtl);

#if (BMX_AMX_DTWIN_WROK_AROUND == 1)
        CAM_WRITE_REG(pTwinIspDrv,CAMCTL_R1_CAMCTL_EN, 0);
#endif

        //need to enable FBC
        if(CAM_READ_BITS(pTwinIspDrv,CAMCTL_R1_CAMCTL_DMA_EN,CAMCTL_IMGO_R1_EN))
            CAM_WRITE_BITS(pTwinIspDrv,FBC_R1_FBC_IMGO_R1_CTL1,FBC_IMGO_R1_FBC_EN,1);

        if(CAM_READ_BITS(pTwinIspDrv,CAMCTL_R1_CAMCTL_DMA_EN,CAMCTL_RRZO_R1_EN))
            CAM_WRITE_BITS(pTwinIspDrv,FBC_R1_FBC_RRZO_R1_CTL1,FBC_RRZO_R1_FBC_EN,1);

        if(CAM_READ_BITS(pTwinIspDrv,CAMCTL_R1_CAMCTL_DMA_EN,CAMCTL_AFO_R1_EN))
            CAM_WRITE_BITS(pTwinIspDrv,FBC_R1_FBC_AFO_R1_CTL1,FBC_AFO_R1_FBC_EN,1);

        if(CAM_READ_BITS(pTwinIspDrv,CAMCTL_R1_CAMCTL_DMA2_EN,CAMCTL_YUVO_R1_EN))
            CAM_WRITE_BITS(pTwinIspDrv,FBC_R1_FBC_YUVO_R1_CTL1,FBC_YUVO_R1_FBC_EN,1);

        if(CAM_READ_BITS(pTwinIspDrv,CAMCTL_R1_CAMCTL_DMA_EN,CAMCTL_CRZO_R1_EN))
            CAM_WRITE_BITS(pTwinIspDrv,FBC_R1_FBC_CRZO_R1_CTL1,FBC_CRZO_R1_FBC_EN,1);

        if(CAM_READ_BITS(pTwinIspDrv,CAMCTL_R1_CAMCTL_DMA2_EN,CAMCTL_CRZO_R2_EN))
            CAM_WRITE_BITS(pTwinIspDrv,FBC_R1_FBC_CRZO_R2_CTL1,FBC_CRZO_R2_FBC_EN,1);

        //
        pTwinIspDrvPrv = pTwinIspDrv;
        twinHwModulePrv = twinHwModule;
    }
    CAM_FUNC_DBG("-,slavenum:%d,cq:%d,page:%d,%s\n",slave_num,cq,page,dbg_str.c_str());

    return ret;
}

MINT32 CAM_TWIN_PIPE::update_TwinEn(MBOOL bEn)
{
    MINT32 ret = 0;
    E_ISP_CAM_CQ cq;
    MUINT32 page, slave_num = 0;
    ISP_DRV_CAM* pTwinIspDrv;
    ISP_DRV_CAM* pTwinIspDrvPrv = NULL;
    ISP_HW_MODULE twinHwModule, twinHwModulePrv = CAM_MAX;
    std::string dbg_str;

    DMA_LSCI lsci;
    DMA_BPCI bpci;
    DMA_IMGO imgo;
    DMA_RRZO rrzo;
    DMA_AFO afo;
    DMA_YUVO yuvo;
    DMA_CRZO_R1 crzo_r1;
    DMA_CRZO_R2 crzo_r2;

    this->m_pDrv = (ISP_DRV_CAM*)this->m_pIspDrv;
    this->m_pDrv->getCurObjInfo(&this->m_hwModule,&cq,&page);

    CAM_FUNC_DBG("+,slavenum:%d,en:%d,cq:%d,page:%d\n",
        this->m_pTwinIspDrv_v.size(),bEn,cq,page);
    CAM_WRITE_BITS(this->m_pDrv, CAMCTL_R1_CAMCTL_TWIN_STATUS, TWIN_EN, bEn);
    CAM_WRITE_BITS(this->m_pDrv, CAMCTL_R1_CAMCTL_TWIN_STATUS, MASTER_MODULE, this->m_hwModule);
    if(!bEn) {
        //if bEn == 0, this->m_pTwinIspDrv_v.size() should be 0, skip slaveNumCheck()
        CAM_WRITE_BITS(this->m_pDrv, CAMCTL_R1_CAMCTL_TWIN_STATUS, SLAVE_CAM_NUM, 0);
    }
    else {
        if(!slaveNumCheck(slave_num))
            return 1;

        CAM_WRITE_BITS(this->m_pDrv, CAMCTL_R1_CAMCTL_TWIN_STATUS, SLAVE_CAM_NUM, slave_num);
        dbg_str += ",twinHwModule: ";
    }

    //update TWIN_STATUS & DMA ultra setting
    for(MUINT32 i = 0; i < slave_num; i++)
    {
        pTwinIspDrv = this->m_pTwinIspDrv_v.at(i);
        pTwinIspDrv->getCurObjInfo(&twinHwModule,&cq,&page);

        CAM_FUNC_DBG("[%d_%d] twinHwModule:%d,cq:%d,page:%d\n",slave_num,i,twinHwModule,cq,page);
        dbg_str += std::to_string(twinHwModule) + ",";

        CAM_WRITE_BITS(pTwinIspDrv, CAMCTL_R1_CAMCTL_TWIN_STATUS, TWIN_EN, bEn);
        if(!bEn) {
            CAM_WRITE_BITS(pTwinIspDrv, CAMCTL_R1_CAMCTL_TWIN_STATUS, SLAVE_CAM_NUM, 0);
            CAM_WRITE_BITS(pTwinIspDrv, CAMCTL_R1_CAMCTL_TWIN_STATUS, MASTER_MODULE, CAM_MAX);
            CAM_WRITE_BITS(pTwinIspDrv, CAMCTL_R1_CAMCTL_TWIN_STATUS, TWIN_MODULE, CAM_MAX);
        } else {
            CAM_WRITE_BITS(pTwinIspDrv, CAMCTL_R1_CAMCTL_TWIN_STATUS, SLAVE_CAM_NUM, slave_num);
            CAM_WRITE_BITS(pTwinIspDrv, CAMCTL_R1_CAMCTL_TWIN_STATUS, MASTER_MODULE, this->m_hwModule);
            CAM_WRITE_BITS(this->m_pDrv, CAMCTL_R1_CAMCTL_TWIN_STATUS, TWIN_MODULE, twinHwModule);
            CAM_WRITE_BITS(pTwinIspDrv, CAMCTL_R1_CAMCTL_TWIN_STATUS,  TWIN_MODULE, twinHwModule);

            //config once for fifosize/ultra/pre-ultra
            lsci.m_pIspDrv = pTwinIspDrv;
            bpci.m_pIspDrv = pTwinIspDrv;
            imgo.m_pIspDrv = pTwinIspDrv;
            rrzo.m_pIspDrv = pTwinIspDrv;
            afo.m_pIspDrv = pTwinIspDrv;
            yuvo.m_pIspDrv = pTwinIspDrv;
            crzo_r1.m_pIspDrv = pTwinIspDrv;
            crzo_r2.m_pIspDrv = pTwinIspDrv;

            lsci.config();
            bpci.config();
            imgo.config();
            rrzo.config();
            afo.config();
            yuvo.config();
            crzo_r1.config();
            crzo_r2.config();
        }
        pTwinIspDrvPrv = pTwinIspDrv;
        twinHwModulePrv = twinHwModule;
    }
    CAM_FUNC_DBG("-,slavenum:%d,en:%d,cq:%d,page:%d %s\n",slave_num,bEn,cq,page,dbg_str.c_str());

    return ret;
}

MINT32 CAM_TWIN_PIPE::updateTwinInfo(MBOOL bEn, E_TWIN_UPDATE_OP op)
{
    MINT32 ret = 0;

    switch (op){
        case E_ENABLE_TWIN:
            ret = update_TwinEn(bEn);
            break;
        case E_BEFORE_TWIN:
            ret = update_beforeRunTwin();
            break;
        case E_AFTER_TWIN:
            ret = update_afterRunTwin();
            break;
        default:
            CAM_FUNC_ERR("not supported op(%d)\n", op);
            ret = 1;
            break;
    }

    return ret;
}


//set 0:stop pipeline. set 1:restart pipeline
MBOOL CAM_TWIN_PIPE::HW_recover(MUINT32 step)
{
    MBOOL rst = MTRUE;
    E_ISP_CAM_CQ cq;
    MUINT32 page, slave_num;
    ISP_DRV_CAM* pTwinIspDrv;
    ISP_HW_MODULE twinHwModule;
    string _str;

    this->m_pDrv = (ISP_DRV_CAM*)this->m_pIspDrv;
    this->m_pDrv->getCurObjInfo(&this->m_hwModule,&cq,&page);

    if(!slaveNumCheck(slave_num))
        return 1;

    for(MUINT32 i = 0; i < slave_num; i++)
    {
        pTwinIspDrv = this->m_pTwinIspDrv_v.at(i);
        pTwinIspDrv->getCurObjInfo(&twinHwModule,&cq,&page);

        switch(step){
            case 0:
                {
                    MUINT32 i=0 , flags[2];

                    CAM_FUNC_INF("Stop&Reset +, [%d_%d]twinHwModule(%d)\n",slave_num,i,twinHwModule);

                    for (MUINT32 i = 0; i < (MUINT32)TWIN_DMAO_MAX; i++){
                        MUINT32 _port = MAP_TWIN_DMAO(i,MTRUE,CAM_FUNC);
                        IspFunction_B::fbcPhyLock[twinHwModule][_port].lock();
                    }

                    //keep fbc cnt
                    SAVE_FBC(pTwinIspDrv->getPhyObj(),IMGO_R1,_str);
                    SAVE_FBC(pTwinIspDrv->getPhyObj(),RRZO_R1,_str);
                    SAVE_FBC(pTwinIspDrv->getPhyObj(),UFEO_R1,_str);
                    SAVE_FBC(pTwinIspDrv->getPhyObj(),UFGO_R1,_str);
                    SAVE_FBC(pTwinIspDrv->getPhyObj(),AFO_R1,_str);
                    SAVE_FBC(pTwinIspDrv->getPhyObj(),YUVO_R1,_str);
                    SAVE_FBC(pTwinIspDrv->getPhyObj(),YUVBO_R1,_str);
                    SAVE_FBC(pTwinIspDrv->getPhyObj(),YUVCO_R1,_str);
                    SAVE_FBC(pTwinIspDrv->getPhyObj(),CRZO_R1,_str);
                    SAVE_FBC(pTwinIspDrv->getPhyObj(),CRZO_R2,_str);
                    SAVE_FBC(pTwinIspDrv->getPhyObj(),CRZBO_R1,_str);
                    SAVE_FBC(pTwinIspDrv->getPhyObj(),CRZBO_R2,_str);

                    SW_RESET(pTwinIspDrv->getPhyObj());

                    CAM_FUNC_INF("Stop&Reset-, [%d_%d]twinHwModule(%d), SAVE_FBC: %s\n", slave_num,i,twinHwModule,_str.c_str());
                }
                break;
            case 1:
                {
                    MUINT32 frm_cnt = 0;

                    CAM_FUNC_INF("Restart +,[%d_%d]twinHwModule(%d)\n",slave_num,i,twinHwModule);

                    //keep fbc cnt
                    PULL_RCNT_INC(pTwinIspDrv->getPhyObj(),IMGO_R1,_str);
                    PULL_RCNT_INC(pTwinIspDrv->getPhyObj(),RRZO_R1,_str);
                    PULL_RCNT_INC(pTwinIspDrv->getPhyObj(),UFEO_R1,_str);
                    PULL_RCNT_INC(pTwinIspDrv->getPhyObj(),UFGO_R1,_str);
                    PULL_RCNT_INC(pTwinIspDrv->getPhyObj(),AFO_R1,_str);
                    PULL_RCNT_INC(pTwinIspDrv->getPhyObj(),YUVO_R1,_str);
                    PULL_RCNT_INC(pTwinIspDrv->getPhyObj(),YUVBO_R1,_str);
                    PULL_RCNT_INC(pTwinIspDrv->getPhyObj(),YUVCO_R1,_str);
                    PULL_RCNT_INC(pTwinIspDrv->getPhyObj(),CRZO_R1,_str);
                    PULL_RCNT_INC(pTwinIspDrv->getPhyObj(),CRZO_R2,_str);
                    PULL_RCNT_INC(pTwinIspDrv->getPhyObj(),CRZBO_R1,_str);
                    PULL_RCNT_INC(pTwinIspDrv->getPhyObj(),CRZBO_R2,_str);

                    for (MUINT32 i = 0; i < (MUINT32)TWIN_DMAO_MAX; i++){
                        MUINT32 _port = MAP_TWIN_DMAO(i,MTRUE,CAM_FUNC);
                        IspFunction_B::fbcPhyLock[twinHwModule][_port].unlock();
                    }
                    CAM_FUNC_INF("Restart-, [%d_%d]twinHwModule:(%d), PHYS_FBC: %s\n",slave_num,i,twinHwModule, _str.c_str());
                }
                break;
            default:
                CAM_FUNC_ERR("unsupported:%d, [%d_%d]twinHwModule(%d)\n",step,slave_num,i, twinHwModule);
                return MFALSE;
                break;
        }
    }

    return rst;
}

MBOOL CAM_TWIN_PIPE::GetTwinRegAddr(ISP_HW_MODULE module,MUINT32* pAddr)
{
    ISP_DRV_CAM* ptr = NULL;
    MBOOL ret = MTRUE;

    switch(module){
        case CAM_A:
        case CAM_B:
        case CAM_C:
            ptr = (ISP_DRV_CAM*)ISP_DRV_CAM::createInstance(module,ISP_DRV_CQ_THRE2,0,"GetTwinAddr");
            *pAddr = CAM_REG_ADDR(ptr,CAMCTL_R1_CAMCTL_TWIN_STATUS);
            break;
        default:
            ret = MFALSE;
            BASE_LOG_ERR("module:%d is not supported as a master cam.\n",module);
            break;
    }

    if(ptr)
        ptr->destroyInstance();

    return ret;
}

