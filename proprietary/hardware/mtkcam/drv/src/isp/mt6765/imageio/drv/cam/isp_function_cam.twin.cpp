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


///////////////////////////////////////////////////////////////////////////////
MINT32 CAM_TWIN_PIPE::_config( void)
{
    MINT32 ret = 0;
    E_ISP_CAM_CQ cq;
    MUINT32 page;

    this->m_pDrv = (ISP_DRV_CAM*)this->m_pIspDrv;

    this->m_pDrv->getCurObjInfo(&this->m_hwModule,&cq,&page);
    ((ISP_DRV_CAM*)this->m_pTwinIspDrv)->getCurObjInfo(&this->m_twinHwModule,&cq,&page);

    CAM_FUNC_INF("_config +, m_twinHwModule(%d),cq(%d),page(%d)\n", this->m_twinHwModule,cq,page);

    // 1. Set TWIN_EN of m_pDrv to be 1
    CAM_WRITE_BITS(this->m_pDrv, CAM_CTL_TWIN_STATUS, TWIN_EN, 0x1); // 0x1A004050
    CAM_WRITE_BITS(this->m_pDrv, CAM_CTL_TWIN_STATUS, TWIN_MODULE, this->m_twinHwModule);

    CAM_WRITE_BITS(this->m_pTwinIspDrv, CAM_CTL_TWIN_STATUS, TWIN_EN, 0x1); // 0x1A005050
    CAM_WRITE_BITS(this->m_pTwinIspDrv, CAM_CTL_TWIN_STATUS, TWIN_MODULE, this->m_twinHwModule);

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

    //inte en => marked
    // Don'y enable CAMB's interrupt cause it will get P1_done iRQ as well,  only err r enabled
    CAM_WRITE_REG(this->m_pTwinIspDrv,CAM_CTL_RAW_INT_EN,\
            CQ_CODE_ERR_EN_|\
            CQ_APB_ERR_EN_ |\
            CQ_VS_ERR_EN_);

    //only afo is needed, because the waiting event of afo's buf_ctrl is afo_done
    CAM_WRITE_REG(this->m_pTwinIspDrv,CAM_CTL_RAW_INT2_EN,\
            AFO_DONE_EN_);

    //reset dma list
    this->m_dma_imgo_L.clear();
    this->m_dma_rrzo_L.clear();


    //cq db setting
    CAM_WRITE_BITS(this->m_pTwinIspDrv,CAM_CQ_EN,CQ_DB_EN,1); //enable double buffer

    //subsample p1 done
    if(this->m_subsample){
        CAM_WRITE_BITS(this->m_pTwinIspDrv,CAM_CTL_SW_PASS1_DONE,DOWN_SAMPLE_EN,1);
    }
    else{
        CAM_WRITE_BITS(this->m_pTwinIspDrv,CAM_CTL_SW_PASS1_DONE,DOWN_SAMPLE_EN,0);
    }

    CAM_WRITE_BITS(this->m_pTwinIspDrv,CAM_CTL_SW_PASS1_DONE,DOWN_SAMPLE_PERIOD,this->m_subsample);

    //
    //change CQ load mode before streaming start.
    //because this bit is shared to all cq, so put here to make sure all cq timing
    if(CAM_READ_BITS(this->m_pTwinIspDrv,CAM_CTL_SW_PASS1_DONE,DOWN_SAMPLE_EN) == 1){
        //load mode = 0 if the latch-timing of the cq-baseaddress-reg is by sw p1_done
        CAM_WRITE_BITS(this->m_pTwinIspDrv,CAM_CQ_EN,CQ_DB_LOAD_MODE,0);
    }
    else{
        //load mode = 1 if the latch-timing of the cq-baseaddress-reg is by hw p1_done
        //when mode = 0 , inner reg is useless
        CAM_WRITE_BITS(this->m_pTwinIspDrv,CAM_CQ_EN,CQ_DB_LOAD_MODE,1);
    }

    {
        DMA_LSCI lsci;
        DMA_BPCI bpci;
        DMA_IMGO imgo;
        DMA_RRZO rrzo;
        DMA_AFO afo;

        lsci.m_pIspDrv = this->m_pTwinIspDrv;
        bpci.m_pIspDrv = this->m_pTwinIspDrv;
        imgo.m_pIspDrv = this->m_pTwinIspDrv;
        rrzo.m_pIspDrv = this->m_pTwinIspDrv;
        afo.m_pIspDrv = this->m_pTwinIspDrv;

        //config once for fifosize/ultra/pre-ultra
        lsci.config();
        bpci.config();
        imgo.config();
        rrzo.config();
        afo.config();

        //config once
        CAM_WRITE_BITS(this->m_pTwinIspDrv->getPhyObj(), CAM_SPECIAL_FUN_EN, CQ_ULTRA_BPCI_EN, 1);
        CAM_WRITE_BITS(this->m_pTwinIspDrv->getPhyObj(), CAM_SPECIAL_FUN_EN, CQ_ULTRA_LSCI_EN, 1);
    }

    return ret;
}

MINT32 CAM_TWIN_PIPE::_enable( void* pParam  )
{
    MINT32 ret = 0;
    E_ISP_CAM_CQ cq;
    MUINT32 page;
    (void)pParam;

    this->m_pDrv = (ISP_DRV_CAM*)this->m_pIspDrv;

    this->m_pDrv->getCurObjInfo(&this->m_hwModule,&cq,&page);
    ((ISP_DRV_CAM*)this->m_pTwinIspDrv)->getCurObjInfo(&this->m_twinHwModule,&cq,&page);

    CAM_FUNC_INF("_enable +, m_twinHwModule(%d), subsample(%d),cq(%d),page(%d)\n", this->m_twinHwModule, this->m_subsample,cq,page);

#if TOP_DCM_SW_WORK_AROUND
    CAM_WRITE_REG(this->m_pTwinIspDrv->getPhyObj(),CAM_CTL_TOP_DCM_DIS,0x1);
#endif


    // special
    CAM_WRITE_REG(this->m_pTwinIspDrv->getPhyObj(),CAM_DMA_RSV1,(CAM_READ_REG(this->m_pTwinIspDrv->getPhyObj(),CAM_DMA_RSV1)&0x7fffffff));//bit31 for smi mask, blocking imgo output
    CAM_WRITE_REG(this->m_pTwinIspDrv->getPhyObj(),CAM_DMA_RSV6,0xffffffff);


    return 0;
}


MINT32 CAM_TWIN_PIPE::_disable(void* pParam)
{
    MINT32 ret = 0;
    E_ISP_CAM_CQ cq;
    MUINT32 page;
    (void)pParam;

    this->m_pDrv = (ISP_DRV_CAM*)this->m_pIspDrv;

    this->m_pDrv->getCurObjInfo(&this->m_hwModule,&cq,&page);
    ((ISP_DRV_CAM*)this->m_pTwinIspDrv)->getCurObjInfo(&this->m_twinHwModule,&cq,&page);

    CAM_FUNC_INF("_disable +, m_twinHwModule(%d),cq(%d),page(%d)\n", this->m_twinHwModule,cq,page);

    CAM_FUNC_DBG("flush twin ion buf twinModule(0x%x)\n", this->m_twinHwModule);
    this->m_pDrv->setDeviceInfo(_SET_ION_FREE_BY_HWMODULE,(MUINT8*)&this->m_twinHwModule);

    if(CAM_READ_BITS(this->m_pDrv->getPhyObj(), CAM_CTL_TWIN_STATUS,  TWIN_EN)== 0){
        //do not access cam_b's register directly, current camb may be running with a sesnor.
        CAM_FUNC_INF("donothing for Twin's diable\n");
        return ret;
    }

    // Set TWIN_EN of m_pDrv to be 0
    CAM_WRITE_REG(this->m_pDrv->getPhyObj(), CAM_CTL_TWIN_STATUS,  0x0); // 0x1A004050
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
    CAM_WRITE_REG(this->m_pTwinIspDrv->getPhyObj(),CAM_FBC_UFEO_CTL1,0x0);


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

    return 0;
}

MINT32 CAM_TWIN_PIPE::_write2CQ( void )
{
    E_ISP_CAM_CQ cq;
    MUINT32 page;

    this->m_pDrv = (ISP_DRV_CAM*)this->m_pIspDrv;

    this->m_pDrv->getCurObjInfo(&this->m_hwModule,&cq,&page);
    ((ISP_DRV_CAM*)this->m_pTwinIspDrv)->getCurObjInfo(&this->m_twinHwModule,&cq,&page);

    CAM_FUNC_VRB("_write2CQ +, m_twinHwModule(%d),cq(%d),page(%d)\n", this->m_twinHwModule,cq,page);

    //
    this->m_pDrv->cqAddModule(CAM_TWIN_INFO);

    //
    this->m_pTwinIspDrv->cqAddModule(CAM_TWIN_INFO);
    this->m_pTwinIspDrv->cqAddModule(CAM_CTL_INT_EN_);
    this->m_pTwinIspDrv->cqAddModule(CAM_CTL_INT2_EN_);
    this->m_pTwinIspDrv->cqAddModule(CAM_CTL_SW_DONE_SEL_);
    this->m_pTwinIspDrv->cqAddModule(CAM_CQ_EN_);
    return 0;
}

MINT32 CAM_TWIN_PIPE::update_cq(void)
{
    MUINT32* main_IspDescript_vir;
    MUINT32* main_IspVirReg_phy;
    MUINT32* twin_IspDescript_vir;
    MUINT32* twin_IspVirReg_phy;
    MUINT32 tmp;
    E_ISP_CAM_CQ cq;
    MUINT32 page;
    MUINT32 _tmp2;
    #define _STRUCT_OFST(_stype, _field) ((MUINT32)(MUINTPTR)&(((_stype*)0)->_field))


    this->m_pDrv = (ISP_DRV_CAM*)this->m_pIspDrv;

    this->m_pDrv->getCurObjInfo(&this->m_hwModule,&cq,&page);
    ((ISP_DRV_CAM*)this->m_pTwinIspDrv)->getCurObjInfo(&this->m_twinHwModule,&cq,&page);

    CAM_FUNC_VRB("update_cq +, m_twinHwModule(%d),cq(%d),page(%d)\n", this->m_twinHwModule,cq,page);

    // Update Twin's CQ descriptor according to Main's CQ descriptor
    main_IspDescript_vir = this->m_pDrv->getCQDescBufVirAddr();
    main_IspVirReg_phy = this->m_pDrv->getIspVirRegPhyAddr();

    twin_IspDescript_vir = this->m_pTwinIspDrv->getCQDescBufVirAddr();
    twin_IspVirReg_phy = this->m_pTwinIspDrv->getIspVirRegPhyAddr();

    // Copy Main's CQ descriptor to Twin's
    // Copy each CQ module separatly
    if(CAM_A == this->m_hwModule){
        for(int i = 0 ; i < CAM_CAM_MODULE_MAX ; i++){
            // twin module is CAM_B: addr + 0x1000 of CAM_A
            tmp = *(main_IspDescript_vir + 2*i);
            _tmp2 = (tmp&CQ_DES_RANGE);
            //copy only non-dummy descriptor && do not copy cq's baseaddress, baseaddress is for master cam's link-list
            if((_tmp2>0x4000) && (_tmp2 != CQ_DUMMY_REG) && \
                (_tmp2 != (_STRUCT_OFST(cam_reg_t, CAM_CQ_THR0_BASEADDR)+0x4000))){//UNI r excluded
#if TWIN_CQ_SW_WORKAROUND
                //only copy reg within master cam
                if(((tmp&CQ_DES_RANGE)<0x5000)){
                    *(twin_IspDescript_vir + 2*i) = tmp + 0x1000;
                    *(twin_IspDescript_vir + (2*i + 1)) = (MUINT32)((MUINT8*)(MUINTPTR)*(main_IspDescript_vir + (2*i + 1)) - (MUINT8*)main_IspVirReg_phy) + (MUINTPTR)twin_IspVirReg_phy;
                }
#else
                *(twin_IspDescript_vir + 2*i) = tmp + 0x1000;
                *(twin_IspDescript_vir + (2*i + 1)) = (MUINT32)((MUINT8*)(MUINTPTR)*(main_IspDescript_vir + (2*i + 1)) - (MUINT8*)main_IspVirReg_phy) + (MUINTPTR)twin_IspVirReg_phy;
#endif
            }
        }
    }
    else if(CAM_B == this->m_hwModule){
        for(int i = 0 ; i < CAM_CAM_MODULE_MAX ; i++){
            // twin module is CAM_A: addr + 0x1000 of CAM_B
            tmp = *(main_IspDescript_vir + 2*i);
            _tmp2 = (tmp&CQ_DES_RANGE);
            //copy only non-dummy descriptor && do not copy cq's baseaddress, baseaddress is for master cam's link-list
            if((_tmp2>0x5000) && (_tmp2 != (CQ_DUMMY_REG+0x1000)) && \
                (_tmp2 != (_STRUCT_OFST(cam_reg_t, CAM_CQ_THR0_BASEADDR)+0x5000))){//UNI r excluded
                *(twin_IspDescript_vir + 2*i) = tmp - 0x1000;
                *(twin_IspDescript_vir + (2*i + 1)) = (MUINT32)((MUINT8*)(MUINTPTR)*(main_IspDescript_vir + (2*i + 1)) - (MUINT8*)main_IspVirReg_phy) + (MUINTPTR)twin_IspVirReg_phy;
            }
        }
    }

#if 0
    for(int i = 0; i < CAM_CAM_MODULE_MAX; i++)
    {
        CAM_FUNC_INF("0x%x(0x%x), 0x%x(0x%x)", main_IspDescript_vir + 2*i, *(main_IspDescript_vir + 2*i), \
                main_IspDescript_vir + (2*i + 1), *(main_IspDescript_vir + (2*i + 1)));
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
    DMA_RRZO rrzo;
    DMA_IMGO imgo;
    E_ISP_CAM_CQ cq;
    MUINT32 page;
    MUINT32 dbn_en, bin_en;

    this->m_pDrv = (ISP_DRV_CAM*)this->m_pIspDrv;

    this->m_pDrv->getCurObjInfo(&this->m_hwModule,&cq,&page);
    ((ISP_DRV_CAM*)this->m_pTwinIspDrv)->getCurObjInfo(&this->m_twinHwModule,&cq,&page);

    CAM_FUNC_DBG("update_beforeRunTwin +, m_twinHwModule(%d),cq(%d),page(%d)\n", this->m_twinHwModule,cq,page);

    // Check if RRZ_EN =1 and RRZ_VERT_STEP == 0. return err.
    // Cause TWIN_RRZ_IN_CROP_HT is set to 0 here, but Dual_cal will return err when TWIN_RRZ_IN_CROP_HT and RRZ_VERT_STEP both = 0
    if(1 == CAM_READ_BITS(this->m_pDrv, CAM_CTL_EN, RRZ_EN) && 0 == CAM_READ_BITS(this->m_pDrv, CAM_RRZ_VERT_STEP, RRZ_VERT_STEP)) {
        CAM_FUNC_ERR("RRZ_EN(%d) RRZ_VERT_STEP(%d) is 0!!!! Cannot run twin!!!", \
                CAM_READ_BITS(this->m_pDrv, CAM_CTL_EN, RRZ_EN), \
                CAM_READ_BITS(this->m_pDrv, CAM_RRZ_VERT_STEP, RRZ_VERT_STEP));
        return 1;
    }

    //TG cropping window
    TG_H.Raw = CAM_READ_REG(this->m_pDrv->getPhyObj(),CAM_TG_SEN_GRAB_LIN);
    TG_W.Raw = CAM_READ_REG(this->m_pDrv->getPhyObj(),CAM_TG_SEN_GRAB_PXL);

    CAM_WRITE_REG(this->m_pDrv,CAM_TG_SEN_GRAB_PXL,TG_W.Raw);
    CAM_WRITE_REG(this->m_pDrv,CAM_TG_SEN_GRAB_LIN,TG_H.Raw);

    // AF related setting that need to set before run Twin_drv
    if(0x1 == CAM_READ_BITS(this->m_pDrv,CAM_CTL_EN,AF_EN))
    {
        MUINT32 sgg_sel = CAM_READ_BITS(this->m_pDrv, CAM_CTL_SEL, SGG_SEL);
        // 1. SGG SEL = 1
        CAM_WRITE_BITS(this->m_pTwinIspDrv, CAM_CTL_SEL, SGG_SEL, sgg_sel);
        //
    }

    this->m_rrz_out_wd = CAM_READ_BITS(this->m_pDrv, CAM_RRZ_OUT_IMG, RRZ_OUT_WD);

    //rrz roi
    this->m_rrz_roi.floatX = this->m_rrz_roi.floatY = 0;
    tmp = fh_rrzo.GetRegInfo(Header_RRZO::E_RRZ_CRP_START,this->m_pDrv);
    this->m_rrz_roi.x = (tmp & 0xffff);
    this->m_rrz_roi.y = (tmp>>16);

    tmp = fh_rrzo.GetRegInfo(Header_RRZO::E_RRZ_CRP_SIZE,this->m_pDrv);
    this->m_rrz_roi.w = (tmp & 0xffff);
    this->m_rrz_roi.h = (tmp>>16);

    //rrz_in
    this->m_rrz_in.h = TG_H.Bits.LIN_E - TG_H.Bits.LIN_S;
    this->m_rrz_in.w = TG_W.Bits.PXL_E - TG_W.Bits.PXL_S;

    // If DBN=1, rrz in setting must be divide by 2; if DBN = 1 and BIN = 2, imgo setting must be divide by 4
    dbn_en = CAM_READ_BITS(this->m_pDrv,CAM_CTL_EN,DBN_EN);
    bin_en = CAM_READ_BITS(this->m_pDrv,CAM_CTL_EN,BIN_EN);
    this->m_rrz_in.w >>= (dbn_en + bin_en);
    this->m_rrz_in.h >>= (bin_en);

    this->m_af_vld_xstart = CAM_READ_BITS(this->m_pDrv, CAM_AF_VLD, AF_VLD_XSTART);
    this->m_af_blk_xsize = CAM_READ_BITS(this->m_pDrv, CAM_AF_BLK_0, AF_BLK_XSIZE);
    this->m_af_x_win_num = CAM_READ_BITS(this->m_pDrv, CAM_AF_BLK_1, AF_BLK_XNUM);

    this->m_lsc_lwidth = CAM_READ_BITS(this->m_pDrv, CAM_LSC_LBLOCK,LSC_SDBLK_lWIDTH);
    this->m_lsc_win_num_x = CAM_READ_BITS(this->m_pDrv, CAM_LSC_CTL2,LSC_SDBLK_XNUM);


    //
    // 2. AFO stride, set to same as CAMA
    stride = CAM_READ_BITS(this->m_pDrv,CAM_AFO_STRIDE,STRIDE);
    CAM_WRITE_BITS(this->m_pTwinIspDrv, CAM_AFO_STRIDE, STRIDE, stride);


    //imgo/rrzo baseaddr 0-init,otherwise, twin drv will be run-in C-model mode
    if(CAM_READ_BITS(this->m_pDrv,CAM_CTL_DMA_EN,IMGO_EN) ){
        imgo.m_pIspDrv = (IspDrvVir*)this->m_pDrv;
        imgo.dma_cfg.memBuf.base_pAddr = 0x0;
        imgo.Header_Addr = 0x0;
        imgo.setBaseAddr();
        imgo.m_pIspDrv = (IspDrvVir*)this->m_pTwinIspDrv;
        imgo.setBaseAddr();
    }
    if(CAM_READ_BITS(this->m_pDrv,CAM_CTL_DMA_EN,RRZO_EN) ){
        rrzo.m_pIspDrv = (IspDrvVir*)this->m_pDrv;
        rrzo.dma_cfg.memBuf.base_pAddr = 0x0;
        rrzo.Header_Addr = 0x0;
        rrzo.setBaseAddr();
        rrzo.m_pIspDrv = (IspDrvVir*)this->m_pTwinIspDrv;
        rrzo.setBaseAddr();
    }

    return 0;
}


MINT32 CAM_TWIN_PIPE::update_afterRunTwin()
{
    MINT32 ret = 0;
    //MUINT32  afo_stride, afo_ofst, sgg;
    MUINT32 af[2];
    MUINT32 afo[2];
    static MUINT32 sycn_counter = 0;
    E_ISP_CAM_CQ cq;
    MUINT32 page;

    this->m_pDrv = (ISP_DRV_CAM*)this->m_pIspDrv;

    this->m_pDrv->getCurObjInfo(&this->m_hwModule,&cq,&page);
    ((ISP_DRV_CAM*)this->m_pTwinIspDrv)->getCurObjInfo(&this->m_twinHwModule,&cq,&page);

    CAM_FUNC_DBG("update_afterRunTwin +, m_twinHwModule(%d),cq(%d),page(%d)\n", this->m_twinHwModule,cq,page);

    //inte en => marked
    // Don'y enable CAMB's interrupt cause it will get P1_done iRQ as well
    //CAM_WRITE_REG(this->m_pTwinIspDrv->getPhyObj(),CAM_CTL_RAW_INT_EN,CAM_READ_REG(this->m_pDrv->getPhyObj(), CAM_CTL_RAW_INT_EN));

    // Set CAMB's dmx_sel = 1
    CAM_WRITE_BITS(this->m_pTwinIspDrv, CAM_CTL_SEL, DMX_SEL, 0x1);

    /// AFO related setting, these registers need to be set where twin_drv not take care
    if(0x1 == CAM_READ_BITS(this->m_pDrv,CAM_CTL_EN,AF_EN))
    {
        // 2. AFO stride, set to same as CAMA
        //afo_stride = CAM_READ_BITS(this->m_pDrv,CAM_AFO_STRIDE,STRIDE);
        //CAM_WRITE_BITS(this->m_pTwinIspDrv, CAM_AFO_STRIDE, STRIDE, afo_stride);

#if 0   //twin drv should write offset to virreg directly
        // 3. AFO offset = AF_BLK_XSIZE x AF_BLK_XNUM
        afo_ofst = CAM_READ_BITS(this->m_pDrv,CAM_AF_BLK_1,AF_BLK_XNUM) * 16; // one af window is 16 bytes
        CAM_WRITE_BITS(this->m_pTwinIspDrv, CAM_AFO_OFST_ADDR, OFFSET_ADDR, afo_ofst);


        // 4. SGG SEL = 1
        /// MOVE before run twin_drv
        //CAM_WRITE_BITS(this->m_pTwinIspDrv, CAM_CTL_SEL, SGG_SEL, 0x1);

        // SGG_EN
        CAM_WRITE_BITS(this->m_pTwinIspDrv, CAM_CTL_EN, SGG1_EN, 0x1);

        // SGG_GAIN copy to same as CAMA
        sgg = CAM_READ_REG(this->m_pDrv,CAM_SGG1_PGN);
        CAM_WRITE_REG(this->m_pTwinIspDrv,CAM_SGG1_PGN, sgg);

        sgg = CAM_READ_REG(this->m_pDrv,CAM_SGG1_GMRC_1);
        CAM_WRITE_REG(this->m_pTwinIspDrv,CAM_SGG1_GMRC_1, sgg);

        sgg = CAM_READ_REG(this->m_pDrv,CAM_SGG1_GMRC_2);
        CAM_WRITE_REG(this->m_pTwinIspDrv,CAM_SGG1_GMRC_2, sgg);

        CAM_FUNC_DBG("afo_stride(0x%x), afo_ofst(0x%x)", afo_stride, afo_ofst);
#endif
    }

    if(0x1 == CAM_READ_BITS(this->m_pDrv,CAM_CTL_EN,LSC_EN)){
        MUINT32 addr;
        //
        addr = CAM_READ_REG(this->m_pDrv,CAM_LSCI_BASE_ADDR);
        CAM_WRITE_REG(this->m_pTwinIspDrv,CAM_LSCI_BASE_ADDR,addr);
        //
        CAM_WRITE_BITS(this->m_pTwinIspDrv,CAM_CTL_DMA_EN,LSCI_EN,1);
    }
    else{
        CAM_WRITE_BITS(this->m_pTwinIspDrv,CAM_CTL_DMA_EN,LSCI_EN,0);
    }

    if(0x1 == CAM_READ_BITS(this->m_pDrv,CAM_CTL_DMA_EN,BPCI_EN)){
        MUINT32 addr;
        //
        addr = CAM_READ_REG(this->m_pDrv,CAM_BPCI_BASE_ADDR);
        CAM_WRITE_REG(this->m_pTwinIspDrv,CAM_BPCI_BASE_ADDR,addr);
        //
        CAM_WRITE_BITS(this->m_pTwinIspDrv,CAM_CTL_DMA_EN,BPCI_EN,1);
    }
    else{
        CAM_WRITE_BITS(this->m_pTwinIspDrv,CAM_CTL_DMA_EN,BPCI_EN,0);
    }

    //check af & afo, af & afo must be opened at both cam at the same time is needed.
    //afo support no twin fbc ctrl .
    //afo can't be off  because of afo's control timing is not sync with main image
    af[0] = CAM_READ_BITS(this->m_pDrv,CAM_CTL_EN,AF_EN);
    af[1] = CAM_READ_BITS(this->m_pTwinIspDrv,CAM_CTL_EN,AF_EN);
    afo[0] = CAM_READ_BITS(this->m_pDrv,CAM_CTL_DMA_EN,AFO_EN);
    afo[1] = CAM_READ_BITS(this->m_pTwinIspDrv,CAM_CTL_DMA_EN,AFO_EN);

    {//write cam_b's header register from cam_A
        ISP_DRV_REG_IO_STRUCT Regs_R[E_HEADER_MAX -1];

        CAM_BURST_READ_REGS(this->m_pDrv,CAM_AFO_FH_SPARE_2,Regs_R,E_HEADER_MAX -1);
        CAM_BURST_WRITE_REGS(this->m_pTwinIspDrv,CAM_AFO_FH_SPARE_2,Regs_R,E_HEADER_MAX -1);
    }

    sycn_counter++;
    CAM_WRITE_REG(this->m_pDrv,CAM_CTL_SPARE1,sycn_counter);
    CAM_WRITE_REG(this->m_pTwinIspDrv,CAM_CTL_SPARE1,sycn_counter);

    if(af[0] != af[1]){
        ret = 1;
        CAM_FUNC_ERR("af_en must be the same under twin mode[0x%x_0x%x]\n",af[0],af[1]);
    }
    if(afo[0] != afo[1]){
        ret = 1;
        CAM_FUNC_ERR("afo_en must be the same under twin mode[0x%x_0x%x]\n",afo[0],afo[1]);
    }


#if 1//patch twin drv bug , twin drv at some condition will not copy cam_a's af
    {
        MUINT32 afo_ysize = CAM_READ_REG(this->m_pDrv,CAM_AFO_YSIZE);
        CAM_WRITE_REG(this->m_pTwinIspDrv,CAM_AFO_YSIZE,afo_ysize);
    }
#endif


    this->update_afterRunTwin_Dma();
    //update twin's cq descriptor
    this->update_cq();

    return ret;
}


MINT32 CAM_TWIN_PIPE::update_afterRunTwin_Dma()
{
    L_T_TWIN_DMA channel;
    T_TWIN_DMA dma;

    Mutex::Autolock lock(this->m_muList);

    dma.bFrameEnd = MFALSE;

    //afo support no twin fbc ctrl .
    //afo can't be off  because of afo's control timing is not sync with main image

    //imgo
    channel.clear();

    if(CAM_READ_BITS(this->m_pTwinIspDrv,CAM_CTL_DMA_EN,IMGO_EN) ){
        capibility CamInfo;
        if(CamInfo.m_DTwin.GetDTwin() == MFALSE)
            dma.module = CAM_B;
        else
            dma.module = CAM_A_TWIN;

        dma.channel = _imgo_;
        dma.offsetAdr = CAM_READ_REG(this->m_pTwinIspDrv,CAM_IMGO_OFST_ADDR);

        channel.push_back(dma);

        CAM_WRITE_BITS(this->m_pTwinIspDrv,CAM_FBC_IMGO_CTL1,FBC_EN,1);
    }
    else{
        CAM_WRITE_BITS(this->m_pTwinIspDrv,CAM_FBC_IMGO_CTL1,FBC_EN,0);
    }

    if(CAM_READ_BITS(this->m_pDrv,CAM_CTL_DMA_EN,IMGO_EN) ){
        dma.module = CAM_A;
        dma.channel = _imgo_;
        dma.offsetAdr = CAM_READ_REG(this->m_pDrv,CAM_IMGO_OFST_ADDR);

        channel.push_back(dma);

        CAM_WRITE_BITS(this->m_pDrv,CAM_FBC_IMGO_CTL1,FBC_EN,1);
    }
    else{
        CAM_WRITE_BITS(this->m_pDrv,CAM_FBC_IMGO_CTL1,FBC_EN,0);
    }

    if(CAM_READ_BITS(this->m_pDrv,CAM_CTL_DMA_EN,IMGO_EN) != (CAM_READ_BITS(this->m_pTwinIspDrv,CAM_CTL_DMA_EN,IMGO_EN) )){
        //currently , because of no p1_don/p1_sof at slave cam, arbitrary cropping is not suppoeted!!
        //it will be failed at waitbufready()!
        CAM_FUNC_ERR("support no arbitrary cropping\n");
        return 1;
    }
    //for frame end token
    dma.bFrameEnd = MTRUE;
    channel.push_back(dma);

    this->m_dma_imgo_L.push_back(channel);

    {//write cam_b's header register from cam_A
        ISP_DRV_REG_IO_STRUCT Regs_R[E_HEADER_MAX -1];

        CAM_BURST_READ_REGS(this->m_pDrv,CAM_IMGO_FH_SPARE_2,Regs_R,E_HEADER_MAX -1);
        CAM_BURST_WRITE_REGS(this->m_pTwinIspDrv,CAM_IMGO_FH_SPARE_2,Regs_R,E_HEADER_MAX -1);
    }

    //////////////////////////////////////////////////////////////////////////////
    //rrzo
    dma.bFrameEnd = MFALSE;
    channel.clear();

    if(CAM_READ_BITS(this->m_pTwinIspDrv,CAM_CTL_DMA_EN,RRZO_EN) ){
        capibility CamInfo;
        if(CamInfo.m_DTwin.GetDTwin() == MFALSE)
            dma.module = CAM_B;
        else
            dma.module = CAM_A_TWIN;

        dma.channel = _rrzo_;
        dma.offsetAdr = CAM_READ_REG(this->m_pTwinIspDrv,CAM_RRZO_OFST_ADDR);

        channel.push_back(dma);

        CAM_WRITE_BITS(this->m_pTwinIspDrv,CAM_FBC_RRZO_CTL1,FBC_EN,1);
    }
    else{
        CAM_WRITE_BITS(this->m_pTwinIspDrv,CAM_FBC_RRZO_CTL1,FBC_EN,0);
    }

    if(CAM_READ_BITS(this->m_pDrv,CAM_CTL_DMA_EN,RRZO_EN) ){
        dma.module = CAM_A;
        dma.channel = _rrzo_;
        dma.offsetAdr = CAM_READ_REG(this->m_pDrv,CAM_RRZO_OFST_ADDR);

        channel.push_back(dma);

        CAM_WRITE_BITS(this->m_pDrv,CAM_FBC_RRZO_CTL1,FBC_EN,1);
    }
    else{
        CAM_WRITE_BITS(this->m_pDrv,CAM_FBC_RRZO_CTL1,FBC_EN,0);
    }

#if (Arbitrary_Crop == 0)
    if((CAM_READ_BITS(this->m_pDrv,CAM_CTL_DMA_EN,RRZO_EN) == MFALSE) &&
        (CAM_READ_BITS(this->m_pTwinIspDrv,CAM_CTL_DMA_EN,RRZO_EN) == MTRUE)){
        //currently , because of no p1_don/p1_sof at slave cam, arbitrary cropping is not suppoeted!!
        //it will be failed at waitbufready()!
        CAM_FUNC_ERR("support no arbitrary cropping\n");
    }
#endif

    //for frame end token
    dma.bFrameEnd = MTRUE;
    channel.push_back(dma);

    this->m_dma_rrzo_L.push_back(channel);

    {//write cam_b's header register from cam_A
        ISP_DRV_REG_IO_STRUCT Regs_R[E_HEADER_MAX -1];

        CAM_BURST_READ_REGS(this->m_pDrv,CAM_RRZO_FH_SPARE_2,Regs_R,E_HEADER_MAX -1);
        CAM_BURST_WRITE_REGS(this->m_pTwinIspDrv,CAM_RRZO_FH_SPARE_2,Regs_R,E_HEADER_MAX -1);
    }

    return 0;
}

MINT32 CAM_TWIN_PIPE::get_RunTwinRst(MUINT32 dmachannel,vector<L_T_TWIN_DMA>& channel,MBOOL bDeque)
{
    vector<L_T_TWIN_DMA>* pList = NULL;
    Mutex::Autolock lock(this->m_muList);

    switch(dmachannel){
        case _imgo_:
            pList = &this->m_dma_imgo_L;
            break;
        case _rrzo_:
            pList = &this->m_dma_rrzo_L;
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
            channel.push_back(pList->at(i));
        }
    }
    else{//enque is from latest data
        if(pList->size() < (this->m_subsample+1) ){
            CAM_FUNC_ERR("logic error: %d_%d\n",(MUINT32)pList->size(),(this->m_subsample+1));
            return 1;
        }

        //
        for(MUINT32 i= 0;i<(this->m_subsample+1);i++){
            channel.push_back(pList->at(i + (pList->size()-(this->m_subsample+1))));
        }
    }

    return 0;
}


MINT32 CAM_TWIN_PIPE::clr_RunTwinRst(MUINT32 dmachannel)
{
    vector<L_T_TWIN_DMA>* pList = NULL;
    Mutex::Autolock lock(this->m_muList);

    switch(dmachannel){
        case _imgo_:
            pList = &this->m_dma_imgo_L;
            break;
        case _rrzo_:
            pList = &this->m_dma_rrzo_L;
            break;
        default:
            CAM_FUNC_ERR("unsupported dma:%d\n",dmachannel);
            return 1;
            break;
    }

    //remove record
    if(this->m_subsample){
        vector<L_T_TWIN_DMA>::iterator it;
        for(it=pList->begin();it<(pList->begin() + this->m_subsample);it++){
            it->clear();
        }
        pList->erase(pList->begin(),pList->begin() + this->m_subsample);
    }
    else{
        pList->begin()->clear();
        pList->erase(pList->begin());
    }

    return 0;
}


MINT32 CAM_TWIN_PIPE::suspend( void )
{
    E_ISP_CAM_CQ cq;
    MUINT32 page;

    this->m_pDrv = (ISP_DRV_CAM*)this->m_pIspDrv;

    this->m_pDrv->getCurObjInfo(&this->m_hwModule,&cq,&page);
    ((ISP_DRV_CAM*)this->m_pTwinIspDrv)->getCurObjInfo(&this->m_twinHwModule,&cq,&page);

    CAM_FUNC_DBG("suspend +, m_twinHwModule(%d),cq(%d),page(%d)\n", this->m_twinHwModule,cq,page);

    //
    CAM_WRITE_BITS(this->m_pDrv, CAM_CTL_TWIN_STATUS,TWIN_EN,  0x0);
    CAM_WRITE_BITS(this->m_pTwinIspDrv, CAM_CTL_TWIN_STATUS,TWIN_EN,  0x0);

    // !!Must set CAMB's SGG_SEL = 0, or run other case CAMA will enconter error
    CAM_WRITE_BITS(this->m_pTwinIspDrv, CAM_CTL_SEL, SGG_SEL, 0x0);

    // !!Must set CAMB's dmx_sel = 0, or run other case CAMA will enconter error
    CAM_WRITE_BITS(this->m_pTwinIspDrv, CAM_CTL_SEL, DMX_SEL, 0x0);

    // !!Must set CAMB's dmx_id = 1,
    CAM_WRITE_BITS(this->m_pTwinIspDrv,CAM_CTL_FMT_SEL,DMX_ID,0x1);


#if TOP_DCM_SW_WORK_AROUND
    CAM_WRITE_REG(this->m_pTwinIspDrv->getPhyObj(),CAM_CTL_TOP_DCM_DIS,0x0);
#endif

    //need to disable rcnt_inc.
    //because we need this page to switch dmx, but we don't want to have impact on fbc
    m_suspend_Rcnt = CAM_READ_REG(this->m_pTwinIspDrv,CAM_CTL_FBC_RCNT_INC);
    CAM_WRITE_REG(this->m_pTwinIspDrv,CAM_CTL_FBC_RCNT_INC,0);

    //close fbc to prevent fbc counter abnormal
    if(CAM_READ_BITS(this->m_pTwinIspDrv,CAM_CTL_DMA_EN,IMGO_EN))
        CAM_WRITE_BITS(this->m_pTwinIspDrv,CAM_FBC_IMGO_CTL1,FBC_EN,0);

    if(CAM_READ_BITS(this->m_pTwinIspDrv,CAM_CTL_DMA_EN,RRZO_EN))
        CAM_WRITE_BITS(this->m_pTwinIspDrv,CAM_FBC_RRZO_CTL1,FBC_EN,0);

    if(CAM_READ_BITS(this->m_pTwinIspDrv,CAM_CTL_DMA_EN,AFO_EN))
        CAM_WRITE_BITS(this->m_pTwinIspDrv,CAM_FBC_AFO_CTL1,FBC_EN,0);

    return 0;
}

MINT32 CAM_TWIN_PIPE::resume( void )
{
    MINT32 ret = 0;
    E_ISP_CAM_CQ cq;
    MUINT32 page;

    this->m_pDrv = (ISP_DRV_CAM*)this->m_pIspDrv;

    this->m_pDrv->getCurObjInfo(&this->m_hwModule,&cq,&page);
    ((ISP_DRV_CAM*)this->m_pTwinIspDrv)->getCurObjInfo(&this->m_twinHwModule,&cq,&page);

    CAM_FUNC_DBG("resume +, m_twinHwModule(%d),cq(%d),page(%d)\n", this->m_twinHwModule,cq,page);

    //
    CAM_WRITE_BITS(this->m_pDrv, CAM_CTL_TWIN_STATUS,TWIN_EN,  0x1);
    CAM_WRITE_BITS(this->m_pTwinIspDrv, CAM_CTL_TWIN_STATUS,TWIN_EN,  0x1);

    // !!Must set CAMB's SGG_SEL = 0, or run other case CAMA will enconter error
    CAM_WRITE_BITS(this->m_pTwinIspDrv, CAM_CTL_SEL, SGG_SEL, 0x1);

    // !!Must set CAMB's dmx_sel = 0, or run other case CAMA will enconter error
    CAM_WRITE_BITS(this->m_pTwinIspDrv, CAM_CTL_SEL, DMX_SEL, 0x1);

    // !!Must set CAMB's dmx_id = 1,
    CAM_WRITE_BITS(this->m_pTwinIspDrv,CAM_CTL_FMT_SEL,DMX_ID,0x0);


#if TOP_DCM_SW_WORK_AROUND
        CAM_WRITE_REG(this->m_pTwinIspDrv->getPhyObj(),CAM_CTL_TOP_DCM_DIS,0x1);
#endif


    //need to enable rcnt_inc. due to disabled at susepnd
    CAM_WRITE_REG(this->m_pTwinIspDrv,CAM_CTL_FBC_RCNT_INC,this->m_suspend_Rcnt);

    //need to enable FBC
    if(CAM_READ_BITS(this->m_pTwinIspDrv,CAM_CTL_DMA_EN,IMGO_EN))
        CAM_WRITE_BITS(this->m_pTwinIspDrv,CAM_FBC_IMGO_CTL1,FBC_EN,1);

    if(CAM_READ_BITS(this->m_pTwinIspDrv,CAM_CTL_DMA_EN,RRZO_EN))
        CAM_WRITE_BITS(this->m_pTwinIspDrv,CAM_FBC_RRZO_CTL1,FBC_EN,1);

    if(CAM_READ_BITS(this->m_pTwinIspDrv,CAM_CTL_DMA_EN,AFO_EN))
        CAM_WRITE_BITS(this->m_pTwinIspDrv,CAM_FBC_AFO_CTL1,FBC_EN,1);
    return ret;
}

MINT32 CAM_TWIN_PIPE::resume_check(void)
{
    if(CAM_READ_BITS(this->m_pTwinIspDrv, CAM_CTL_TWIN_STATUS,TWIN_EN) == 0){
        this->resume();
    }

    return 0;
}

MINT32 CAM_TWIN_PIPE::updateTwinInfo(MBOOL bEn)
{
    MINT32 ret = 0;
    E_ISP_CAM_CQ cq;
    MUINT32 page;

    this->m_pDrv = (ISP_DRV_CAM*)this->m_pIspDrv;

    this->m_pDrv->getCurObjInfo(&this->m_hwModule,&cq,&page);
    ((ISP_DRV_CAM*)this->m_pTwinIspDrv)->getCurObjInfo(&this->m_twinHwModule,&cq,&page);

    CAM_FUNC_DBG("updateTwinInfo +, m_twinHwModule(%d),cq(%d),page(%d)\n", this->m_twinHwModule,cq,page);


    this->m_pDrv = (ISP_DRV_CAM*)this->m_pIspDrv;

    CAM_WRITE_BITS(this->m_pDrv, CAM_CTL_TWIN_STATUS,TWIN_EN,  bEn);
    CAM_WRITE_BITS(this->m_pTwinIspDrv, CAM_CTL_TWIN_STATUS,TWIN_EN,  bEn);

    return ret;
}

//set 0:stop pipeline. set 1:restart pipeline
MBOOL CAM_TWIN_PIPE::HW_recover(MUINT32 step)
{
    switch(step){
        case 0:
            {
                MBOOL rst = MTRUE;
                MUINT32 i=0 , flags[2];

                CAM_FUNC_INF("CAM_TWIN_PIPE::stop & reset + \n");

                IspFunction_B::fbcPhyLock_IMGO[this->m_twinHwModule].lock();
                IspFunction_B::fbcPhyLock_RRZO[this->m_twinHwModule].lock();
                IspFunction_B::fbcPhyLock_UFEO[this->m_twinHwModule].lock();
                IspFunction_B::fbcPhyLock_AFO[this->m_twinHwModule].lock();

                //keep fbc cnt
                if(CAM_READ_BITS(this->m_pTwinIspDrv->getPhyObj(),CAM_FBC_IMGO_CTL1,FBC_EN)){
                    this->fbc_IMGO.Raw = CAM_READ_REG(this->m_pTwinIspDrv->getPhyObj(),CAM_FBC_IMGO_CTL2);
                }

                if(CAM_READ_BITS(this->m_pTwinIspDrv->getPhyObj(),CAM_FBC_RRZO_CTL1,FBC_EN)){
                    this->fbc_RRZO.Raw = CAM_READ_REG(this->m_pTwinIspDrv->getPhyObj(),CAM_FBC_RRZO_CTL2);
                }

                if(CAM_READ_BITS(this->m_pTwinIspDrv->getPhyObj(),CAM_FBC_UFEO_CTL1,FBC_EN)){
                    this->fbc_UFEO.Raw = CAM_READ_REG(this->m_pTwinIspDrv->getPhyObj(),CAM_FBC_UFEO_CTL2);
                }

                if(CAM_READ_BITS(this->m_pTwinIspDrv->getPhyObj(),CAM_FBC_AFO_CTL1,FBC_EN)){
                    this->fbc_AFO.Raw = CAM_READ_REG(this->m_pTwinIspDrv->getPhyObj(),CAM_FBC_AFO_CTL2);
                }


                CAM_FUNC_INF("TOP reset\n");
                CAM_WRITE_REG(this->m_pTwinIspDrv->getPhyObj(),CAM_CTL_SW_CTL,0x0);
                CAM_WRITE_REG(this->m_pTwinIspDrv->getPhyObj(),CAM_CTL_SW_CTL,0x1);
                while(CAM_READ_REG(this->m_pTwinIspDrv->getPhyObj(),CAM_CTL_SW_CTL) != 0x2){
                    CAM_FUNC_INF("TOP reseting...\n");
                }
                CAM_WRITE_REG(this->m_pTwinIspDrv->getPhyObj(),CAM_CTL_SW_CTL,0x4);
                CAM_WRITE_REG(this->m_pTwinIspDrv->getPhyObj(),CAM_CTL_SW_CTL,0x0);


                CAM_FUNC_DBG("CAM_TWIN_PIPE::- \n");

                return rst;
            }
            break;
        case 1:
            {
                MUINT32 frm_cnt = 0;

                CAM_FUNC_INF("CAM_TWIN_PIPE::restart + \n");

                CAM_FUNC_INF("CAM_TWIN_PIPE::restart SAVE_FBC:IMGO_0x%08x RRZO_0x%08x UFEO_0x%08x AFO_0x%08x",
                        this->fbc_IMGO.Raw, this->fbc_RRZO.Raw, this->fbc_UFEO.Raw, this->fbc_AFO.Raw);

                //keep fbc cnt
                if(CAM_READ_BITS(this->m_pTwinIspDrv->getPhyObj(),CAM_FBC_IMGO_CTL1,FBC_EN)){
                    for (frm_cnt = 0; frm_cnt < this->fbc_IMGO.Bits.FBC_CNT; frm_cnt++) {
                        CAM_WRITE_BITS(this->m_pTwinIspDrv->getPhyObj(),CAM_CTL_FBC_RCNT_INC, IMGO_RCNT_INC, 1);
                    }
                    CAM_WRITE_REG(this->m_pTwinIspDrv->getPhyObj(),CAM_FBC_IMGO_CTL2,this->fbc_IMGO.Raw);
                }

                if(CAM_READ_BITS(this->m_pTwinIspDrv->getPhyObj(),CAM_FBC_RRZO_CTL1,FBC_EN)){
                    for (frm_cnt = 0; frm_cnt < this->fbc_RRZO.Bits.FBC_CNT; frm_cnt++) {
                        CAM_WRITE_BITS(this->m_pTwinIspDrv->getPhyObj(),CAM_CTL_FBC_RCNT_INC, RRZO_RCNT_INC, 1);
                    }
                    CAM_WRITE_REG(this->m_pTwinIspDrv->getPhyObj(),CAM_FBC_RRZO_CTL2,this->fbc_RRZO.Raw);
                }

                if(CAM_READ_BITS(this->m_pTwinIspDrv->getPhyObj(),CAM_FBC_UFEO_CTL1,FBC_EN)){
                    for (frm_cnt = 0; frm_cnt < this->fbc_UFEO.Bits.FBC_CNT; frm_cnt++) {
                        CAM_WRITE_BITS(this->m_pTwinIspDrv->getPhyObj(),CAM_CTL_FBC_RCNT_INC, UFEO_RCNT_INC, 1);
                    }
                    CAM_WRITE_REG(this->m_pTwinIspDrv->getPhyObj(),CAM_FBC_UFEO_CTL2,this->fbc_UFEO.Raw);
                }
                if(CAM_READ_BITS(this->m_pTwinIspDrv->getPhyObj(),CAM_FBC_AFO_CTL1,FBC_EN)){
                    for (frm_cnt = 0; frm_cnt < this->fbc_AFO.Bits.FBC_CNT; frm_cnt++) {
                        if (CAM_READ_BITS(this->m_pTwinIspDrv->getPhyObj(), CAM_CTL_TWIN_STATUS,TWIN_EN)) {
                            CAM_WRITE_BITS(this->m_pTwinIspDrv->getPhyObj(),CAM_CTL_FBC_RCNT_INC, AFO_A_RCNT_INC, 1);
                            CAM_WRITE_BITS(this->m_pTwinIspDrv->getPhyObj(),CAM_CTL_FBC_RCNT_INC, AFO_B_RCNT_INC, 1);
                        } else {
                            switch (this->m_twinHwModule) {
                                case CAM_A:
                                    CAM_WRITE_BITS(this->m_pTwinIspDrv->getPhyObj(),CAM_CTL_FBC_RCNT_INC, AFO_A_RCNT_INC, 1);
                                    break;
                                case CAM_B:
                                    CAM_WRITE_BITS(this->m_pTwinIspDrv->getPhyObj(),CAM_CTL_FBC_RCNT_INC, AFO_B_RCNT_INC, 1);
                                    break;
                                default:
                                    CAM_FUNC_ERR("Unsupported module: %d\n",this->m_twinHwModule);
                                    break;
                            }
                        }

                    }
                    CAM_WRITE_REG(this->m_pTwinIspDrv->getPhyObj(),CAM_FBC_AFO_CTL2,this->fbc_AFO.Raw);
                }

                IspFunction_B::fbcPhyLock_IMGO[this->m_twinHwModule].unlock();
                IspFunction_B::fbcPhyLock_RRZO[this->m_twinHwModule].unlock();
                IspFunction_B::fbcPhyLock_UFEO[this->m_twinHwModule].unlock();
                IspFunction_B::fbcPhyLock_AFO[this->m_twinHwModule].unlock();

                CAM_FUNC_INF("CAM_TWIN_PIPE::restart PHYS_FBC:IMGO_0x%08x RRZO_0x%08x UFEO_0x%08x AFO_0x%08x",
                        CAM_READ_REG(this->m_pTwinIspDrv->getPhyObj(), CAM_FBC_IMGO_CTL2),
                        CAM_READ_REG(this->m_pTwinIspDrv->getPhyObj(), CAM_FBC_RRZO_CTL2),
                        CAM_READ_REG(this->m_pTwinIspDrv->getPhyObj(), CAM_FBC_UFEO_CTL2),
                        CAM_READ_REG(this->m_pTwinIspDrv->getPhyObj(), CAM_FBC_AFO_CTL1));


                CAM_FUNC_DBG("CAM_TWIN_PIPE::- \n");
                return MTRUE;
            }
            break;
        default:
            CAM_FUNC_ERR("unsupported:%d\n",step);
            return MFALSE;
            break;
    }
}

MBOOL CAM_TWIN_PIPE::GetTwinRegAddr(ISP_HW_MODULE module,MUINT32* pAddr)
{
    ISP_DRV_CAM* ptr = (ISP_DRV_CAM*)ISP_DRV_CAM::createInstance(CAM_A,ISP_DRV_CQ_THRE0,0,"GetTwinAddr");
    MBOOL ret = MTRUE;

    switch(module){
        case CAM_A:
            *pAddr = CAM_REG_ADDR(ptr,CAM_CTL_TWIN_STATUS);
            break;
        default:
            ret = MFALSE;
            BASE_LOG_ERR("module:%d is not supported as a master cam.\n",module);
            break;
    }

    ptr->destroyInstance();
    return ret;
}


