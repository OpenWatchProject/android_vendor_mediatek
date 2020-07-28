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
    m_rrz_out_wd = 0;
    m_af_vld_xstart = m_af_blk_xsize = m_af_x_win_num =0;
    m_lsc_lwidth = m_lsc_win_num_x = 0;
    m_rlb_oft = 0;
    m_pTwinCB = NULL;
    DBG_LOG_CONFIG(imageio, func_twin);
}

inline MINT32 CAM_TWIN_PIPE::slaveNumCheck(MUINT32 &slave_num)
{
    slave_num = this->m_pTwinIspDrv_v.size();
    if(!slave_num){
        CAM_FUNC_ERR("TwinIspDrv vector is empty\n");
        return 1;
    } else if (slave_num > _SLAVE_CAM_MAX_){
        CAM_FUNC_ERR("Only support to %dRAW(%d)\n", (_SLAVE_CAM_MAX_+1), slave_num);
        return 1;
    }
    //
    if(this->m_hwModule == CAM_C && slave_num){
        CAM_FUNC_ERR("CAM_C can't be the master cam\n");
        return 1;
    }

    return 0;
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

    if(slaveNumCheck(slave_num))
        return 1;

    CAM_FUNC_DBG("+, slavenum:%d,cq:%d,page:%d\n", slave_num, cq, page);

    //
    CAM_WRITE_BITS(this->m_pDrv, CAM_CTL_TWIN_STATUS, TWIN_EN, 0x1); // 0x1A004050

    //reset dma list
    this->m_dma_imgo_L.clear();
    this->m_dma_rrzo_L.clear();

    for(MUINT32 i = 0; i < slave_num; i++)
    {
        pTwinIspDrv = this->m_pTwinIspDrv_v.at(i);

        pTwinIspDrv->getCurObjInfo(&twinHwModule,&cq,&page);
        CAM_FUNC_VRB("[%d_%d] twinHwModule:%d,cq:%d,page:%d\n",slave_num,i,twinHwModule,cq,page);
        dbg_str += std::to_string(twinHwModule) + ",";

        //disable db due to dynamic twin(cam will have time-sharing,mater cam will enable db_en), via vir
        CAM_WRITE_BITS(pTwinIspDrv,CAM_CTL_MISC,DB_EN,0);

        // 1. Set TWIN_EN of m_pDrv to be 1
        CAM_WRITE_BITS(pTwinIspDrv, CAM_CTL_TWIN_STATUS, TWIN_EN, 0x1); // 0x1A005050

        CAM_WRITE_BITS(pTwinIspDrv,CAM_CTL_FMT_SEL,DMX_ID,this->m_hwModule);

#if 0//dynamical twin, can't reset here
        //reset
        CAM_FUNC_DBG("Twin TOP reset\n");
        CAM_WRITE_REG(pTwinIspDrv->getPhyObj(),CAM_CTL_SW_CTL,0x1);
        while(CAM_READ_REG(pTwinIspDrv->getPhyObj(),CAM_CTL_SW_CTL) != 0x2){
            CAM_FUNC_DBG("Twin TOP reseting...\n");
        }
        CAM_WRITE_REG(pTwinIspDrv->getPhyObj(),CAM_CTL_SW_CTL,0x4);
        CAM_WRITE_REG(pTwinIspDrv->getPhyObj(),CAM_CTL_SW_CTL,0x0);
#endif

        //cq db setting
        CAM_WRITE_BITS(pTwinIspDrv,CAM_CQ_EN,CQ_DB_EN,1); //enable double buffer

        //subsample p1 done
        if(this->m_subsample){
            CAM_WRITE_BITS(pTwinIspDrv,CAM_CTL_SW_PASS1_DONE,DOWN_SAMPLE_EN,1);
        }
        else{
            CAM_WRITE_BITS(pTwinIspDrv,CAM_CTL_SW_PASS1_DONE,DOWN_SAMPLE_EN,0);
        }

        CAM_WRITE_BITS(pTwinIspDrv,CAM_CTL_SW_PASS1_DONE,DOWN_SAMPLE_PERIOD,this->m_subsample);

        //
        //change CQ load mode before streaming start.
        //because this bit is shared to all cq, so put here to make sure all cq timing
        if(CAM_READ_BITS(pTwinIspDrv,CAM_CTL_SW_PASS1_DONE,DOWN_SAMPLE_EN) == 1){
            //load mode = 0 if the latch-timing of the cq-baseaddress-reg is by sw p1_done
            CAM_WRITE_BITS(pTwinIspDrv,CAM_CQ_EN,CQ_DB_LOAD_MODE,0);
        }
        else{
            //load mode = 1 if the latch-timing of the cq-baseaddress-reg is by hw p1_done
            //when mode = 0 , inner reg is useless
            CAM_WRITE_BITS(pTwinIspDrv,CAM_CQ_EN,CQ_DB_LOAD_MODE,1);
        }
    }

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

    if(slaveNumCheck(slave_num))
        return 1;

    CAM_FUNC_DBG("+, slavenum:%d,subsample:%d,cq:%d,page:%d\n",slave_num,this->m_subsample,cq,page);

    for(MUINT32 i = 0; i < slave_num; i++)
    {
        pTwinIspDrv = this->m_pTwinIspDrv_v.at(i);
        pTwinIspDrv->getCurObjInfo(&twinHwModule,&cq,&page);
        CAM_FUNC_VRB("[%d_%d] twinHwModule:%d,cq:%d,page:%d\n",slave_num,i,twinHwModule,cq,page);
        dbg_str += std::to_string(twinHwModule) + ",";

        // special, w via vir
        CAM_WRITE_REG(pTwinIspDrv,CAM_DMA_RSV1,(CAM_READ_REG(pTwinIspDrv->getPhyObj(),CAM_DMA_RSV1)&0x7fffffff));//bit31 for smi mask, blocking imgo output
        CAM_WRITE_REG(pTwinIspDrv,CAM_DMA_RSV6,0xffffffff);

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

    if(CAM_READ_BITS(this->m_pDrv->getPhyObj(), CAM_CTL_TWIN_STATUS, TWIN_EN)== 0){
        //do not access cam_b's register directly, current camb may be running with a sesnor.
        CAM_FUNC_INF("do nothing for Twin's diable, %d\n", this->m_pTwinIspDrv_v.size());
        return ret;
    }

    if(slaveNumCheck(slave_num))
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
        CAM_WRITE_REG(this->m_pDrv->getPhyObj(), CAM_CTL_TWIN_STATUS,  0x0); // 0x1A004050
        CAM_WRITE_REG(pTwinIspDrv->getPhyObj(), CAM_CTL_TWIN_STATUS,  0x0); // 0x1A005050

        // !!Must set CAMB's SGG_SEL = 0, or run other case CAMA will enconter error
        CAM_WRITE_BITS(pTwinIspDrv->getPhyObj(), CAM_CTL_SEL, SGG_SEL, 0x0);

        // !!Must set CAMB's dmx_sel = 0, or run other case CAMA will enconter error
        CAM_WRITE_BITS(pTwinIspDrv->getPhyObj(), CAM_CTL_SEL, DMX_SEL, 0x0);

        switch(twinHwModule){
            case CAM_A:
                CAM_WRITE_BITS(pTwinIspDrv->getPhyObj(),CAM_CTL_FMT_SEL,DMX_ID,0x0);
                break;
            case CAM_B:
                CAM_WRITE_BITS(pTwinIspDrv->getPhyObj(),CAM_CTL_FMT_SEL,DMX_ID,0x1);
                break;
            case CAM_C:
                CAM_WRITE_BITS(pTwinIspDrv->getPhyObj(),CAM_CTL_FMT_SEL,DMX_ID,0x2);
                break;
            default:
                CAM_FUNC_ERR("unsupported slave:%d\n",twinHwModule);
                break;
        }

        CAM_WRITE_BITS(pTwinIspDrv->getPhyObj(),CAM_CTL_MISC,DB_EN,0); //disable double buffer
        //signal ctrl
        CAM_WRITE_REG(pTwinIspDrv->getPhyObj(),CAM_CTL_SW_PASS1_DONE,0);
        CAM_WRITE_REG(pTwinIspDrv->getPhyObj(),CAM_CTL_AB_DONE_SEL,0);
        CAM_WRITE_REG(pTwinIspDrv->getPhyObj(),CAM_CTL_UNI_DONE_SEL,0);
        CAM_WRITE_REG(pTwinIspDrv->getPhyObj(),CAM_CTL_DMA_EN,0x0);


        CAM_WRITE_REG(pTwinIspDrv->getPhyObj(),CAM_CTL_EN,0x0);
        CAM_WRITE_REG(pTwinIspDrv->getPhyObj(),CAM_CTL_EN2,0x0);
        //pdc function enable is within BNR
        CAM_WRITE_REG(pTwinIspDrv->getPhyObj(),CAM_BNR_BPC_CON,0x0);
        CAM_WRITE_REG(pTwinIspDrv->getPhyObj(),CAM_BNR_PDC_CON,0x0);

        //close  fbc
        //FBC on uni will be closed at the _disable() of uni_top
        //FBC of STT pipe will be closed at STT pipe
        CAM_WRITE_REG(pTwinIspDrv->getPhyObj(),CAM_FBC_IMGO_CTL1,0x0);
        CAM_WRITE_REG(pTwinIspDrv->getPhyObj(),CAM_FBC_RRZO_CTL1,0x0);
        CAM_WRITE_REG(pTwinIspDrv->getPhyObj(),CAM_FBC_UFEO_CTL1,0x0);
        CAM_WRITE_REG(pTwinIspDrv->getPhyObj(),CAM_FBC_UFGO_CTL1,0x0);

        CAM_WRITE_BITS(pTwinIspDrv->getPhyObj(),CAM_CTL_MISC,DB_EN,1); //disable double buffer

        //reset
        CAM_FUNC_DBG("TOP reset\n");
        CAM_WRITE_REG(pTwinIspDrv->getPhyObj(),CAM_CTL_SW_CTL,0x2);
        CAM_WRITE_REG(pTwinIspDrv->getPhyObj(),CAM_CTL_SW_CTL,0x1);
#if 0
        while(CAM_READ_REG(pTwinIspDrv->getPhyObj(),CAM_CTL_SW_CTL) != 0x2){
            CAM_FUNC_DBG("TOP reseting...\n");
        }
#else
        usleep(10);
#endif
        CAM_WRITE_REG(pTwinIspDrv->getPhyObj(),CAM_CTL_SW_CTL,0x4);
        CAM_WRITE_REG(pTwinIspDrv->getPhyObj(),CAM_CTL_SW_CTL,0x0);
    }

    CAM_FUNC_INF("-, slavenum:%d,cq:%d,page:%d,%s\n",slave_num,cq,page,dbg_str.c_str());

    return 0;
}

MINT32 CAM_TWIN_PIPE::_write2CQ( void )
{
    E_ISP_CAM_CQ cq;
    MUINT32 page, slave_num;

    this->m_pDrv = (ISP_DRV_CAM*)this->m_pIspDrv;
    this->m_pDrv->getCurObjInfo(&this->m_hwModule,&cq,&page);
    //
    CAM_FUNC_VRB("m_HwModule(%d),cq(%d),page(%d)\n", this->m_hwModule,cq,page);

    return 0;
}

MINT32 CAM_TWIN_PIPE::update_cq(void)
{
    MINT32 ret = 0;
    MUINT32* main_IspDescript_vir;
    MUINT32* main_IspVirReg_phy;
    MUINT32* twin_IspDescript_vir;
    MUINT32* twin_IspVirReg_phy;
    MUINT32 tmp, _tmp2;
    E_ISP_CAM_CQ cq;
    MUINT32 page, slave_num;
    ISP_DRV_CAM* pTwinIspDrv;
    ISP_HW_MODULE twinHwModule;

    #define _STRUCT_OFST(_stype, _field) ((MUINT32)(MUINTPTR)&(((_stype*)0)->_field))

    this->m_pDrv = (ISP_DRV_CAM*)this->m_pIspDrv;
    this->m_pDrv->getCurObjInfo(&this->m_hwModule,&cq,&page);

    if(slaveNumCheck(slave_num))
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
        master_base = 0x4000 + master_ofst;

        switch(twinHwModule)
        {
            case CAM_A:
                slave_ofset = (-CAM_BASE_RANGE);//master cam is camb
                break;
            case CAM_B:
                slave_ofset = CAM_BASE_RANGE;
                break;
            case CAM_C:
                slave_ofset = CAM_BASE_RANGE;
                slave_ofset *= ((this->m_hwModule == CAM_A)? 2:1);
                break;
            default:
                CAM_FUNC_ERR("CAM%d not supported slave module(%d)\n", this->m_hwModule, twinHwModule);
                return 1;
                break;
        }

#if 0//debug
        CAM_FUNC_INF("dump slave(%d) descrptor:", twinHwModule);
#endif

        // Copy Main's CQ descriptor to Twin's
        // Copy each CQ module separatly
        for(int j = 0; j < CAM_CAM_MODULE_MAX; j++)
        {
            tmp = *(main_IspDescript_vir + 2*j);
            _tmp2 = (tmp&CQ_DES_RANGE);
            //copy only non-dummy descriptor && do not copy cq's baseaddress, baseaddress is for master cam's link-list
            if((_tmp2 > master_base) && (_tmp2 != (CQ_DUMMY_REG+master_ofst)) && \
                (_tmp2 != (_STRUCT_OFST(cam_reg_t, CAM_CQ_THR0_BASEADDR)+master_base)))
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
            #if 0//debug log
            CAM_FUNC_INF("0x%x(0x%x), 0x%x(0x%x)", twin_IspDescript_vir + 2*j, *(twin_IspDescript_vir + 2*j), \
                    twin_IspDescript_vir + (2*j + 1), *(twin_IspDescript_vir + (2*j + 1)));
            #endif

        }

        /*  descriptors of slave cams should add and update by update_cq().
            If the registers of master cam, written directly to phy., and it can be programmed by TWIN_PIPE
            => Add cqModule here
        */
        pTwinIspDrv->cqAddModule(CAM_CQ_EN_);
        pTwinIspDrv->cqAddModule(CAM_CTL_MISC_);//DB_EN
        pTwinIspDrv->cqAddModule(CAM_DMA_RSV1_);//smi mask
        pTwinIspDrv->cqAddModule(CAM_DMA_RSV6_);//smi mask

        //Only 1st slave cam needs to addCQ
        if( i == 0){
            pTwinIspDrv->cqAddModule(UNI_CAM_XMX_SET_);
            pTwinIspDrv->cqAddModule(UNI_CAM_XMX_CLR_);
        }

#if 0
        CAM_FUNC_INF("dump master(%d) descrptor:", this->m_hwModule);
        for(int j = 0; j < CAM_CAM_MODULE_MAX; j++)
        {
            CAM_FUNC_INF("0x%x(0x%x), 0x%x(0x%x)", main_IspDescript_vir + 2*j, *(main_IspDescript_vir + 2*j), \
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

    if(slaveNumCheck(slave_num))
        return 1;

    for(MUINT32 i = 0; i < slave_num; i++)
    {
        pTwinIspDrv = this->m_pTwinIspDrv_v.at(i);
        pTwinIspDrv->getCurObjInfo(&twinHwModule,&cq,&page);
        CAM_FUNC_VRB("[%d_%d] twinHwModule:%d,cq:%d,page:%d\n",slave_num,i,twinHwModule,cq,page);

        pTwinIspDrv->cqAddModule(CAM_CTL_EN_);
        pTwinIspDrv->cqAddModule(CAM_CTL_EN2_);
        pTwinIspDrv->cqAddModule(CAM_CTL_SEL_);
        pTwinIspDrv->cqAddModule(CAM_TWIN_INFO);
        pTwinIspDrv->cqAddModule(CAM_CTL_FMT_);
        pTwinIspDrv->cqAddModule(CAM_CTL_FBC_RCNT_INC_);
        pTwinIspDrv->cqAddModule(CAM_FBC_IMGO_);
        pTwinIspDrv->cqAddModule(CAM_FBC_RRZO_);
        pTwinIspDrv->cqAddModule(CAM_FBC_AFO_);

        pTwinIspDrv->cqAddModule(CAM_CQ_EN_);
        pTwinIspDrv->cqAddModule(CAM_CTL_MISC_);//DB_EN
        pTwinIspDrv->cqAddModule(CAM_DMA_RSV1_);//smi mask
        pTwinIspDrv->cqAddModule(CAM_DMA_RSV6_);//smi mask
    }

    return ret;
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
    DMA_UFEO ufeo;
    DMA_UFGO ufgo;
    BUF_CTRL_UFEO fbc_ufeo;
    BUF_CTRL_UFGO fbc_ufgo;

    E_ISP_CAM_CQ cq;
    MUINT32 page, slave_num;
    MUINT32 dbn_en, bin_en;
    ISP_DRV_CAM* pTwinIspDrv;
    ISP_HW_MODULE twinHwModule = PHY_CAM;
    std::string dbg_str = "twinHwModule: ";

    this->m_pDrv = (ISP_DRV_CAM*)this->m_pIspDrv;
    this->m_pDrv->getCurObjInfo(&this->m_hwModule,&cq,&page);

    if(slaveNumCheck(slave_num))
        return 1;

    CAM_FUNC_DBG("+, slavenum:%d,cq:%d,page:%d\n", slave_num, cq, page);

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

    //rlb
    this->m_rlb_oft = CAM_READ_BITS(this->m_pDrv,CAM_RRZ_RLB_AOFST,RRZ_RLB_AOFST);

    //rrz
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

    //af
    this->m_af_vld_xstart = CAM_READ_BITS(this->m_pDrv, CAM_AF_VLD, AF_VLD_XSTART);
    this->m_af_blk_xsize = CAM_READ_BITS(this->m_pDrv, CAM_AF_BLK_0, AF_BLK_XSIZE);
    this->m_af_x_win_num = CAM_READ_BITS(this->m_pDrv, CAM_AF_BLK_1, AF_BLK_XNUM);

    //lsc
    this->m_lsc_lwidth = CAM_READ_BITS(this->m_pDrv, CAM_LSC_LBLOCK,LSC_SDBLK_lWIDTH);
    this->m_lsc_win_num_x = CAM_READ_BITS(this->m_pDrv, CAM_LSC_CTL2,LSC_SDBLK_XNUM);


    //Salve cam modules
    for(MUINT32 i = 0; i < slave_num; i++)
    {
        pTwinIspDrv = this->m_pTwinIspDrv_v.at(i);
        pTwinIspDrv->getCurObjInfo(&twinHwModule,&cq,&page);
        CAM_FUNC_VRB("[%d_%d] twinHwModule:%d,cq:%d,page:%d\n",slave_num,i,twinHwModule,cq,page);
        dbg_str += std::to_string(twinHwModule) + ",";

        // AF related setting that need to set before run Twin_drv
        if(0x1 == CAM_READ_BITS(this->m_pDrv,CAM_CTL_EN,AF_EN))
        {
            MUINT32 sgg_sel = CAM_READ_BITS(this->m_pDrv, CAM_CTL_SEL, SGG_SEL);
            // 1. SGG SEL = 1
            CAM_WRITE_BITS(pTwinIspDrv, CAM_CTL_SEL, SGG_SEL, sgg_sel);
        }

        imgo.m_pIspDrv = (IspDrvVir*)pTwinIspDrv;
        if(CAM_READ_BITS(this->m_pDrv,CAM_CTL_DMA_EN,IMGO_EN) ){
            imgo.enable(NULL);
        } else {
            imgo.disable(NULL);
        }

        rrzo.m_pIspDrv = (IspDrvVir*)pTwinIspDrv;
        if(CAM_READ_BITS(this->m_pDrv,CAM_CTL_DMA_EN,RRZO_EN) ){
            rrzo.enable(NULL);
        } else {
            rrzo.disable(NULL);
        }
        //UFEO
        ufeo.m_pIspDrv = (IspDrvVir*)pTwinIspDrv;
        fbc_ufeo.m_pIspDrv = (IspDrvVir*)pTwinIspDrv;
        if(CAM_READ_BITS(this->m_pDrv,CAM_CTL_DMA_EN,UFEO_EN)){
            ufeo.enable(NULL);
            fbc_ufeo.enable(NULL);
        } else {
            ufeo.disable(NULL);
            fbc_ufeo.disable(NULL);
        }
        //UFGO
        ufgo.m_pIspDrv = (IspDrvVir*)pTwinIspDrv;
        fbc_ufgo.m_pIspDrv = (IspDrvVir*)pTwinIspDrv;
        if(CAM_READ_BITS(this->m_pDrv,CAM_CTL_DMA_EN,UFGO_EN)){
            ufgo.enable(NULL);
            fbc_ufgo.enable(NULL);
        } else {
            ufgo.disable(NULL);
            fbc_ufgo.disable(NULL);
        }


        if(i > 0)
            continue;

        /* NOTE: "CAM_UNI_TOP_CTL" CAN'T add to CQ
          XMX_IN_SEL of "CAM_UNI_TOP_CTL", ONLY for dual_cal check
          XMX_IN_SEL of "CAM_UNI_TOP_CAM_XMX_SET" & "CAM_UNI_TOP_CAM_XMX_CLR"
            will add to CQ of slave cam (ONLY 1st slave cam can program CAM_UNI_TOP_CTL)
        */
        switch(this->m_hwModule)
        {
            case CAM_A:
                if(slave_num == 2){//a+b+c
                    //
                    UNI_WRITE_BITS(pTwinIspDrv,CAM_UNI_TOP_CAM_XMX_SET, RAW_A_XMX_IN_SEL, 1);
                    UNI_WRITE_BITS(pTwinIspDrv,CAM_UNI_TOP_CAM_XMX_SET, RAW_B_XMX_IN_SEL, 1);
                    UNI_WRITE_BITS(pTwinIspDrv,CAM_UNI_TOP_CAM_XMX_SET, RAW_C_XMX_IN_SEL, 1);
                } else { // 2raw
                    switch (twinHwModule)
                    {
                        case CAM_B:
                            //
                            UNI_WRITE_BITS(pTwinIspDrv,CAM_UNI_TOP_CAM_XMX_SET, RAW_A_XMX_IN_SEL, 1);
                            break;
                        case CAM_C:
                            //
                            UNI_WRITE_BITS(pTwinIspDrv,CAM_UNI_TOP_CAM_XMX_CLR, RAW_A_XMX_IN_SEL, 1);//means 0
                            UNI_WRITE_BITS(pTwinIspDrv,CAM_UNI_TOP_CAM_XMX_CLR, RAW_C_XMX_IN_SEL, 1);//means 0
                            break;
                        default:
                            CAM_FUNC_ERR("master(%d) is not supported slave(%d)\n", this->m_hwModule, twinHwModule);
                            ret = 1;
                            break;
                    }
                }
                break;
            case CAM_B:
                if(slave_num == 2){//b+c+a
                    UNI_WRITE_BITS(pTwinIspDrv,CAM_UNI_TOP_CAM_XMX_SET, RAW_A_XMX_IN_SEL, 1);
                    UNI_WRITE_BITS(pTwinIspDrv,CAM_UNI_TOP_CAM_XMX_SET, RAW_B_XMX_IN_SEL, 1);
                    UNI_WRITE_BITS(pTwinIspDrv,CAM_UNI_TOP_CAM_XMX_SET, RAW_C_XMX_IN_SEL, 1);
                } else {// 2raw
                    switch (twinHwModule)
                    {
                        case CAM_C:
                            //
                            UNI_WRITE_BITS(pTwinIspDrv,CAM_UNI_TOP_CAM_XMX_SET, RAW_B_XMX_IN_SEL, 1);
                            UNI_WRITE_BITS(pTwinIspDrv,CAM_UNI_TOP_CAM_XMX_SET, RAW_C_XMX_IN_SEL, 1);
                            break;
                        default:
                            CAM_FUNC_ERR("master(%d) is not supported slave(%d)\n", this->m_hwModule, twinHwModule);
                            ret = 1;
                            break;
                    }
                }
                break;
            default:
                CAM_FUNC_ERR("Not supported master cam(%d) in twin\n", this->m_hwModule);
                ret = 1;
                break;
        }
    }

    /* under dynamic twin, if uni is linked with CAM_B 1st ,
     * master cam will have no uni info on vir cq under twin mode
     * for dual_cal check only
    **/
    UNI_WRITE_BITS(this->m_pDrv,CAM_UNI_TOP_MOD_EN,RLB_A_EN,1);

    //this is for twin drv's sw request, avoid twin drv running at RTL-verification mode
    this->update_before_twindrv_request();

    CAM_FUNC_DBG("-,slavenum:%d,cq:%d,page:%d,%s\n",slave_num,cq,page,dbg_str.c_str());
    return 0;
}

MVOID  CAM_TWIN_PIPE::update_before_twindrv_request(void)
{
    DMA_RRZO rrzo;
    DMA_IMGO imgo;
    DMA_UFEO ufeo;
    DMA_UFGO ufgo;
    MUINT32 slave_num,stride,page;
    E_ISP_CAM_CQ cq;
    ISP_DRV_CAM* pTwinIspDrv;
    ISP_HW_MODULE twinHwModule = PHY_CAM;

    if(slaveNumCheck(slave_num))
        return;

    for(MUINT32 i = 0; i < slave_num; i++)
    {
        pTwinIspDrv = this->m_pTwinIspDrv_v.at(i);
        pTwinIspDrv->getCurObjInfo(&twinHwModule,&cq,&page);

        // 2. AFO stride, set to same as CAMA
        stride = CAM_READ_BITS(this->m_pDrv,CAM_AFO_STRIDE,STRIDE);
        CAM_WRITE_BITS(pTwinIspDrv, CAM_AFO_STRIDE, STRIDE, stride);

        //imgo/rrzo baseaddr 0-init,otherwise, twin drv will be run-in C-model mode
        //DMA_EN control of slave cam move in TWIN_PIPE
        if(CAM_READ_BITS(this->m_pDrv,CAM_CTL_DMA_EN,IMGO_EN) ){
            imgo.m_pIspDrv = (IspDrvVir*)this->m_pDrv;
            imgo.dma_cfg.memBuf.base_pAddr = 0x0;
            imgo.Header_Addr = 0x0;
            imgo.setBaseAddr();
            imgo.m_pIspDrv = (IspDrvVir*)pTwinIspDrv;
            imgo.setBaseAddr();
        }
        if(CAM_READ_BITS(this->m_pDrv,CAM_CTL_DMA_EN,RRZO_EN) ){
            rrzo.m_pIspDrv = (IspDrvVir*)this->m_pDrv;
            rrzo.dma_cfg.memBuf.base_pAddr = 0x0;
            rrzo.Header_Addr = 0x0;
            rrzo.setBaseAddr();
            rrzo.m_pIspDrv = (IspDrvVir*)pTwinIspDrv;
            rrzo.setBaseAddr();
        }
        if(CAM_READ_BITS(this->m_pDrv,CAM_CTL_DMA_EN,UFEO_EN)){
            ufeo.dma_cfg.memBuf.base_pAddr = 0x0;
            ufeo.Header_Addr = 0x0;
            ufeo.m_pIspDrv = (IspDrvVir*)this->m_pDrv;
            ufeo.setBaseAddr();
            ufeo.m_pIspDrv = (IspDrvVir*)pTwinIspDrv;
            ufeo.setBaseAddr();
        }
        if(CAM_READ_BITS(this->m_pDrv,CAM_CTL_DMA_EN,UFGO_EN)){
            ufgo.dma_cfg.memBuf.base_pAddr = 0x0;
            ufgo.Header_Addr = 0x0;
            ufgo.m_pIspDrv = (IspDrvVir*)this->m_pDrv;
            ufgo.setBaseAddr();
            ufgo.m_pIspDrv = (IspDrvVir*)pTwinIspDrv;
            ufgo.setBaseAddr();
        }

        /* NOTE: "CAM_UNI_TOP_CTL" CAN'T add to CQ
          XMX_IN_SEL of "CAM_UNI_TOP_CTL", ONLY for dual_cal check
          XMX_IN_SEL of "CAM_UNI_TOP_CAM_XMX_SET" & "CAM_UNI_TOP_CAM_XMX_CLR"
            will add to CQ of slave cam (ONLY 1st slave cam can program CAM_UNI_TOP_CTL)
        */
        switch(this->m_hwModule)
        {
            case CAM_A:
                if(slave_num == 2){//a+b+c
                    //for dual_cal check only
                    UNI_WRITE_BITS(this->m_pDrv,CAM_UNI_TOP_CTL, RAW_A_XMX_IN_SEL, 1);
                    UNI_WRITE_BITS(this->m_pDrv,CAM_UNI_TOP_CTL, RAW_B_XMX_IN_SEL, 1);
                    UNI_WRITE_BITS(this->m_pDrv,CAM_UNI_TOP_CTL, RAW_C_XMX_IN_SEL, 1);
                } else { // 2raw
                    switch (twinHwModule)
                    {
                        case CAM_B:
                            //for dual_cal check only
                            UNI_WRITE_BITS(this->m_pDrv,CAM_UNI_TOP_CTL, RAW_A_XMX_IN_SEL, 1);
                            break;
                        case CAM_C:
                            //for dual_cal check only
                            UNI_WRITE_BITS(this->m_pDrv,CAM_UNI_TOP_CTL, RAW_A_XMX_IN_SEL, 0);
                            UNI_WRITE_BITS(this->m_pDrv,CAM_UNI_TOP_CTL, RAW_C_XMX_IN_SEL, 0);
                            break;
                        default:
                            CAM_FUNC_ERR("master(%d) is not supported slave(%d)\n", this->m_hwModule, twinHwModule);
                            break;
                    }
                }
                break;
            case CAM_B:
                if(slave_num == 2){//b+c+a
                    //for dual_cal check only
                    UNI_WRITE_BITS(this->m_pDrv,CAM_UNI_TOP_CTL, RAW_A_XMX_IN_SEL, 1);
                    UNI_WRITE_BITS(this->m_pDrv,CAM_UNI_TOP_CTL, RAW_B_XMX_IN_SEL, 1);
                    UNI_WRITE_BITS(this->m_pDrv,CAM_UNI_TOP_CTL, RAW_C_XMX_IN_SEL, 1);
                } else {// 2raw
                    switch (twinHwModule)
                    {
                        case CAM_C:
                            //for dual_cal check only
                            UNI_WRITE_BITS(this->m_pDrv,CAM_UNI_TOP_CTL, RAW_B_XMX_IN_SEL, 1);
                            UNI_WRITE_BITS(this->m_pDrv,CAM_UNI_TOP_CTL, RAW_C_XMX_IN_SEL, 1);
                            break;
                        default:
                            CAM_FUNC_ERR("master(%d) is not supported slave(%d)\n", this->m_hwModule, twinHwModule);
                            break;
                    }
                }
                break;
            default:
                CAM_FUNC_ERR("Not supported master cam(%d) in twin\n", this->m_hwModule);
                break;
        }
    }
}

MINT32 CAM_TWIN_PIPE::update_afterRunTwin()
{
    MINT32              ret = 0;
    //MUINT32  afo_stride, afo_ofst, sgg;
    MUINT32             af[2];
    MUINT32             afo[2];
    static MUINT32      sycn_counter = 0;
    E_ISP_CAM_CQ        cq;
    MUINT32             page, slave_num;
    ISP_DRV_CAM*        pTwinIspDrv;
    ISP_HW_MODULE       twinHwModule;
    capibility          CamInfo;
    MBOOL               bDTwin = CamInfo.m_DTwin.GetDTwin();
    std::string         dbg_str = "twinHwModule: ";
    CAM_REG_CTL_EN      mEn,sEn;
    CAM_REG_CTL_DMA_EN  mDEn,sDEn;

    this->m_pDrv = (ISP_DRV_CAM*)this->m_pIspDrv;
    this->m_pDrv->getCurObjInfo(&this->m_hwModule,&cq,&page);
    if(slaveNumCheck(slave_num))
        return 1;

    CAM_FUNC_DBG("+, slavenum:%d,cq:%d,page:%d\n", slave_num, cq, page);
    mEn.Raw = CAM_READ_REG(this->m_pDrv,CAM_CTL_EN);
    mDEn.Raw = CAM_READ_REG(this->m_pDrv,CAM_CTL_DMA_EN);
    for(MUINT32 i = 0; i < slave_num; i++)
    {
        pTwinIspDrv = this->m_pTwinIspDrv_v.at(i);
        pTwinIspDrv->getCurObjInfo(&twinHwModule,&cq,&page);
        sEn.Raw = CAM_READ_REG(pTwinIspDrv,CAM_CTL_EN);
        sDEn.Raw = CAM_READ_REG(pTwinIspDrv,CAM_CTL_DMA_EN);

        CAM_FUNC_VRB("[%d_%d] twinHwModule:%d,cq:%d,page:%d\n",slave_num,i,twinHwModule,cq,page);
        dbg_str += std::to_string(twinHwModule) + ",";

        //int_en & int2_en
        // Don't enable CAMB's interrupt cause it will get P1_done iRQ as well,  only err r enabled
        CAM_WRITE_REG(pTwinIspDrv,CAM_CTL_RAW_INT_EN,\
                CQ_CODE_ERR_EN_|\
                CQ_APB_ERR_EN_ |\
                CQ_VS_ERR_EN_);

        //only afo is needed, because the waiting event of afo's buf_ctrl is afo_done
        CAM_WRITE_BITS(pTwinIspDrv,CAM_CTL_RAW_INT2_EN,AFO_DONE_EN,1);

        //DMX_ID
        CAM_WRITE_BITS(pTwinIspDrv, CAM_CTL_FMT_SEL, DMX_ID, this->m_hwModule);

        // Set CAMB's dmx_sel = 1, means slave cam
        CAM_WRITE_BITS(pTwinIspDrv, CAM_CTL_SEL, DMX_SEL, 0x1);

        /// AFO related setting, these registers need to be set where twin_drv not take care
        if(0x1 == mEn.Bits.AF_EN)
        {
            // 2. AFO stride, set to same as CAMA
            //afo_stride = CAM_READ_BITS(this->m_pDrv,CAM_AFO_STRIDE,STRIDE);
            //CAM_WRITE_BITS(pTwinIspDrv, CAM_AFO_STRIDE, STRIDE, afo_stride);

#if 0   //twin drv should write offset to virreg directly
            // 3. AFO offset = AF_BLK_XSIZE x AF_BLK_XNUM
            afo_ofst = CAM_READ_BITS(this->m_pDrv,CAM_AF_BLK_1,AF_BLK_XNUM) * 16; // one af window is 16 bytes
            CAM_WRITE_BITS(pTwinIspDrv, CAM_AFO_OFST_ADDR, OFFSET_ADDR, afo_ofst);


            // 4. SGG SEL = 1
            /// MOVE before run twin_drv
            //CAM_WRITE_BITS(pTwinIspDrv, CAM_CTL_SEL, SGG_SEL, 0x1);

            // SGG_EN
            CAM_WRITE_BITS(pTwinIspDrv, CAM_CTL_EN, SGG1_EN, 0x1);

            // SGG_GAIN copy to same as CAMA
            sgg = CAM_READ_REG(this->m_pDrv,CAM_SGG1_PGN);
            CAM_WRITE_REG(pTwinIspDrv,CAM_SGG1_PGN, sgg);

            sgg = CAM_READ_REG(this->m_pDrv,CAM_SGG1_GMRC_1);
            CAM_WRITE_REG(pTwinIspDrv,CAM_SGG1_GMRC_1, sgg);

            sgg = CAM_READ_REG(this->m_pDrv,CAM_SGG1_GMRC_2);
            CAM_WRITE_REG(pTwinIspDrv,CAM_SGG1_GMRC_2, sgg);

            CAM_FUNC_DBG("afo_stride(0x%x), afo_ofst(0x%x)", afo_stride, afo_ofst);
#endif
        }

        //LSC
        if(0x1 == mEn.Bits.LSC_EN){
            MUINT32 addr;
            //
            addr = CAM_READ_REG(this->m_pDrv,CAM_LSCI_BASE_ADDR);
            CAM_WRITE_REG(pTwinIspDrv,CAM_LSCI_BASE_ADDR,addr);
            //
            CAM_WRITE_BITS(pTwinIspDrv,CAM_CTL_DMA_EN,LSCI_EN,1);
        }
        else{
            CAM_WRITE_BITS(pTwinIspDrv,CAM_CTL_DMA_EN,LSCI_EN,0);
        }

        //BPCI
        if(0x1 == mDEn.Bits.BPCI_EN ){
            MUINT32 addr;
            //
            addr = CAM_READ_REG(this->m_pDrv,CAM_BPCI_BASE_ADDR);
            CAM_WRITE_REG(pTwinIspDrv,CAM_BPCI_BASE_ADDR,addr);
            //
            CAM_WRITE_BITS(pTwinIspDrv,CAM_CTL_DMA_EN,BPCI_EN,1);
        }
        else{
            CAM_WRITE_BITS(pTwinIspDrv,CAM_CTL_DMA_EN,BPCI_EN,0);
        }

        //check af & afo, af & afo must be opened at both cam at the same time is needed.
        //afo support no twin fbc ctrl .
        //afo can't be off  because of afo's control timing is not sync with main image
        af[0] = mEn.Bits.AF_EN;
        af[1] = sEn.Bits.AF_EN;
        afo[0] = mDEn.Bits.AFO_EN;
        afo[1] = sDEn.Bits.AFO_EN;

        if(!bDTwin){//write cam_b's header register from cam_A
            ISP_DRV_REG_IO_STRUCT Regs_R[E_HEADER_MAX -1];

            CAM_BURST_READ_REGS(this->m_pDrv,CAM_AFO_FH_SPARE_2,Regs_R,E_HEADER_MAX -1);
            CAM_BURST_WRITE_REGS(pTwinIspDrv,CAM_AFO_FH_SPARE_2,Regs_R,E_HEADER_MAX -1);
        }

        sycn_counter++;
        CAM_WRITE_REG(this->m_pDrv,CAM_CTL_SPARE1,sycn_counter);
        CAM_WRITE_REG(pTwinIspDrv,CAM_CTL_SPARE1,sycn_counter);

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
            MUINT32 afo_ysize = CAM_READ_REG(this->m_pDrv,CAM_AFO_YSIZE);
            CAM_WRITE_REG(pTwinIspDrv,CAM_AFO_YSIZE,afo_ysize);
        }
#endif

        //magic#
        CAM_WRITE_REG(pTwinIspDrv,CAM_MAGIC_NUM, CAM_READ_REG(this->m_pDrv,CAM_MAGIC_NUM));
    }

    this->update_afterRunTwin_Dma();
    //update twin's cq descriptor
    this->update_cq();

    //update dfs according twin's overhead
    if(this->m_pTwinCB){
        V_CAM_THRU_PUT vThru;
        T_CAM_THRU_PUT Thru;
        CAM_REG_DMX_CROP dmx;
        dmx.Raw = CAM_READ_REG(this->m_pDrv,CAM_DMX_CROP);
        vThru.clear();

        Thru.DMX_W = (dmx.Bits.DMX_END_X - dmx.Bits.DMX_STR_X)+1;
        Thru.DMX_H = CAM_READ_REG(this->m_pDrv,CAM_DMX_VSIZE);
        Thru.bBin = mEn.Bits.BIN_EN;
        vThru.push_back(Thru);
        for(MUINT32 i = 0; i < slave_num; i++){
            pTwinIspDrv = this->m_pTwinIspDrv_v.at(i);
            Thru.bBin = CAM_READ_BITS(pTwinIspDrv,CAM_CTL_EN,BIN_EN);
            dmx.Raw = CAM_READ_REG(pTwinIspDrv,CAM_DMX_CROP);
            Thru.DMX_W = (dmx.Bits.DMX_END_X - dmx.Bits.DMX_STR_X)+1;
            Thru.DMX_H = CAM_READ_REG(pTwinIspDrv,CAM_DMX_VSIZE);
            vThru.push_back(Thru);
        }
        this->m_pTwinCB->p1TuningNotify((MVOID*)&vThru,NULL);
    }
    CAM_FUNC_DBG("-,slavenum:%d,cq:%d,page:%d,%s\n",slave_num,cq,page,dbg_str.c_str());
    return ret;
}

MUINT32 CAM_TWIN_PIPE::update_afterRunTwin_rlb(MUINT32 rlb_oft_prv,MUINT32 rrz_wd_prv,MUINT32 rrz_wd)
{
    MUINT32 rlb_oft = 0, start_oft;
    capibility CamInfo;
    tCAM_rst rst;

    /* rlb, order by [Master1-Slave1-Salve2...] - [Master2-Slave1-...]
       [output] rlb_oft of cur slave cam

       [ex.] when current slave cam is Slave2 of Master1,
       rlb_oft_prv  is rlb_ofst of prv cam = Slave1's rlb_ofst
       rrz_wd_prv   is rrz_wd   of prv cam = Slave1's rrz_wd
       -------------------------------------------------------
       rlb_oft      is rlb_ofst of cur slave cam = Slave2's rlb_ofst
       rrz_wd       is rrz_wd   of cur slave cam = Slave2's rrz_wd
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

    switch(slave_num){
        case 1:
            switch(MhwModule){
                case CAM_A:
                    virHwModule = ((ShwModule == CAM_B)? CAM_A_TWIN_B : CAM_A_TWIN_C);
                    break;
                case CAM_B:
                    virHwModule = ((ShwModule == CAM_C)? CAM_B_TWIN_C : CAM_MAX);
                    break;
                default:
                    break;
            }
            break;
        case 2:
            if (MhwModule != CAM_A)
                break;
            virHwModule = ((ShwModule == CAM_B)? CAM_A_TRIPLE_B : CAM_A_TRIPLE_C);
            break;
        default:
            break;
    }

    if(virHwModule == CAM_MAX)
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
    BUF_CTRL_IMGO fbc_imgo;
    BUF_CTRL_UFEO fbc_ufeo;
    BUF_CTRL_RRZO fbc_rrzo;
    BUF_CTRL_UFGO fbc_ufgo;
    DMA_IMGO imgo;
    DMA_RRZO rrzo;
    DMA_UFEO ufeo;
    DMA_UFGO ufgo;

    Mutex::Autolock lock(this->m_muList);

    if(slaveNumCheck(slave_num))
        return 1;

    //afo support no twin fbc ctrl .
    //afo can't be off  because of afo's control timing is not sync with main image

    //rlb
    rlb_oft = this->m_rlb_oft;
    CAM_WRITE_BITS(this->m_pDrv,CAM_RRZ_RLB_AOFST,RRZ_RLB_AOFST, rlb_oft);
    rrz_wd_prv = CAM_READ_BITS(this->m_pDrv,CAM_RRZ_OUT_IMG,RRZ_OUT_WD);

    //imgo
    dma.bFrameEnd = MFALSE;
    channel.clear();

    for(MUINT32 i = 0; i < slave_num; i++)
    {
        pTwinIspDrv = this->m_pTwinIspDrv_v.at(i);
        pTwinIspDrv->getCurObjInfo(&twinHwModule,&cq,&page);

        //
        fbc_imgo.m_pIspDrv = (IspDrvVir*)pTwinIspDrv;
        fbc_ufeo.m_pIspDrv = (IspDrvVir*)pTwinIspDrv;
        imgo.m_pIspDrv = pTwinIspDrv;
        ufeo.m_pIspDrv = pTwinIspDrv;

        //If the source of imgo of master is directly from TG (img_sel=1), slave cam shouldn't enable IMGO
        if(CAM_READ_REG(this->m_pDrv,CAM_CTL_SEL) & IMG_SEL_1 ){
            //
            if(CAM_READ_BITS(pTwinIspDrv,CAM_CTL_DMA_EN,IMGO_EN)){
                CAM_FUNC_DBG("force disable slave(%d)'s imgo when master(%d) is pure IMGO",
                    twinHwModule, this->m_hwModule);
                imgo.disable(NULL);
                ufeo.disable(NULL);
            }
            //
            if (!CAM_READ_BITS(this->m_pDrv,CAM_CTL_DMA_EN,IMGO_EN)){
                CAM_FUNC_WRN("IMGO disabled with IMG_SEL_1\n");
            }
        }

        if(CAM_READ_BITS(pTwinIspDrv,CAM_CTL_DMA_EN,IMGO_EN) ){
            dma.module = slaveModuleMap(slave_num, this->m_hwModule, twinHwModule);
            if(dma.module == CAM_MAX)
                return 1;

            dma.channel = _imgo_;
            dma.offsetAdr = CAM_READ_REG(pTwinIspDrv,CAM_IMGO_OFST_ADDR);

            //
            fbc_imgo.enable(NULL);

            if(CAM_READ_BITS(pTwinIspDrv,CAM_CTL_DMA_EN,UFEO_EN)){
                fbc_ufeo.enable(NULL);
                dma.uf_offsetAdr = CAM_READ_REG(pTwinIspDrv,CAM_UFEO_OFST_ADDR);
                ufeo.update_AU();
            }
            else{

                fbc_ufeo.disable(NULL);
                dma.uf_offsetAdr = 0;
            }

            channel.push_back(dma);
        }
        else{
            fbc_imgo.disable(NULL);
            fbc_ufeo.disable(NULL);
            ufeo.disable(NULL);
        }

        if(!(CAM_READ_REG(this->m_pDrv,CAM_CTL_SEL) & IMG_SEL_1) && //because force diable slave's imgo when master's img_sel=1
           (CAM_READ_BITS(this->m_pDrv,CAM_CTL_DMA_EN,IMGO_EN) != CAM_READ_BITS(pTwinIspDrv,CAM_CTL_DMA_EN,IMGO_EN)) ){
            //currently , because of no p1_don/p1_sof at slave cam, arbitrary cropping is not suppoeted!!
            //it will be failed at waitbufready()!
            CAM_FUNC_ERR("imgo support no arbitrary cropping(%d_%d)\n",
                CAM_READ_BITS(this->m_pDrv,CAM_CTL_DMA_EN,IMGO_EN), CAM_READ_BITS(pTwinIspDrv,CAM_CTL_DMA_EN,IMGO_EN));
            return 1;
        }
    }
    //m_dma_*_L & fbc ctl
    fbc_imgo.m_pIspDrv = (IspDrvVir*)this->m_pDrv;
    fbc_ufeo.m_pIspDrv = (IspDrvVir*)this->m_pDrv;
    ufeo.m_pIspDrv = this->m_pDrv;
    if(CAM_READ_BITS(this->m_pDrv,CAM_CTL_DMA_EN,IMGO_EN) ){
        dma.module = this->m_hwModule;
        dma.channel = _imgo_;
        dma.offsetAdr = CAM_READ_REG(this->m_pDrv,CAM_IMGO_OFST_ADDR);


        fbc_imgo.enable(NULL);

        if(CAM_READ_BITS(this->m_pDrv,CAM_CTL_DMA_EN,UFEO_EN)){
             fbc_ufeo.enable(NULL);
             dma.uf_offsetAdr = CAM_READ_REG(this->m_pDrv,CAM_UFEO_OFST_ADDR);
             ufeo.update_AU();
        }
        else{
             fbc_ufeo.disable(NULL);
             dma.uf_offsetAdr = 0;
        }

        channel.push_back(dma);
    }
    else{
        fbc_imgo.disable(NULL);
        fbc_ufeo.disable(NULL);
        ufeo.disable(NULL);
    }

    //for frame end token
    dma.bFrameEnd = MTRUE;
    channel.push_back(dma);

    this->m_dma_imgo_L.push_back(channel);

    //write cam_b's header register from cam_A
    for(MUINT32 i = 0; i < slave_num; i++)
    {
        pTwinIspDrv = this->m_pTwinIspDrv_v.at(i);
        ISP_DRV_REG_IO_STRUCT Regs_R[E_HEADER_MAX -1];

        CAM_BURST_READ_REGS(this->m_pDrv,CAM_IMGO_FH_SPARE_2,Regs_R,E_HEADER_MAX -1);
        CAM_BURST_WRITE_REGS(pTwinIspDrv,CAM_IMGO_FH_SPARE_2,Regs_R,E_HEADER_MAX -1);

        if(CAM_READ_BITS(pTwinIspDrv,CAM_CTL_DMA_EN,UFEO_EN)){
            CAM_BURST_READ_REGS(this->m_pDrv,CAM_UFEO_FH_SPARE_2,Regs_R,E_HEADER_MAX -1);
            CAM_BURST_WRITE_REGS(pTwinIspDrv,CAM_UFEO_FH_SPARE_2,Regs_R,E_HEADER_MAX -1);
        }
    }

    //////////////////////////////////////////////////////////////////////////////
    //rrzo
    dma.bFrameEnd = MFALSE;
    channel.clear();

    for(MUINT32 i = 0; i < slave_num; i++)
    {
        pTwinIspDrv = this->m_pTwinIspDrv_v.at(i);
        pTwinIspDrv->getCurObjInfo(&twinHwModule,&cq,&page);

        //
        fbc_rrzo.m_pIspDrv = (IspDrvVir*)pTwinIspDrv;
        fbc_ufgo.m_pIspDrv = (IspDrvVir*)pTwinIspDrv;
        rrzo.m_pIspDrv = pTwinIspDrv;
        ufgo.m_pIspDrv = pTwinIspDrv;

        if(CAM_READ_BITS(pTwinIspDrv,CAM_CTL_DMA_EN,RRZO_EN) ){
            dma.module = slaveModuleMap(slave_num, this->m_hwModule, twinHwModule);
            if(dma.module == CAM_MAX)
                return 1;

            dma.channel = _rrzo_;
            dma.offsetAdr = CAM_READ_REG(pTwinIspDrv,CAM_RRZO_OFST_ADDR);


            fbc_rrzo.enable(NULL);
            //ufgo_en control by dual_cal driver
            if(CAM_READ_BITS(pTwinIspDrv,CAM_CTL_DMA_EN,UFGO_EN)){
                fbc_ufgo.enable(NULL);
                dma.uf_offsetAdr = CAM_READ_REG(pTwinIspDrv,CAM_UFGO_OFST_ADDR);
                ufgo.update_AU();
            }
            else{
                fbc_ufgo.disable(NULL);
                dma.uf_offsetAdr = 0;
            }

            channel.push_back(dma);
        }
        else{
            fbc_rrzo.disable(NULL);
            fbc_ufgo.disable(NULL);
            rrzo.disable(NULL);
            ufgo.disable(NULL);
        }

#if (Arbitrary_Crop == 0)
        if(CAM_READ_BITS(this->m_pDrv,CAM_CTL_DMA_EN,RRZO_EN) != CAM_READ_BITS(pTwinIspDrv,CAM_CTL_DMA_EN,RRZO_EN) ){
            //currently , because of no p1_don/p1_sof at slave cam, arbitrary cropping is not suppoeted!!
            //it will be failed at waitbufready()!
            CAM_FUNC_WRN("rrzo support no arbitrary cropping(%d_%d), it's ok if crop left side\n",
                CAM_READ_BITS(this->m_pDrv,CAM_CTL_DMA_EN,RRZO_EN), CAM_READ_BITS(pTwinIspDrv,CAM_CTL_DMA_EN,RRZO_EN));
        }
#endif

        //rlb
        if(CAM_READ_BITS(pTwinIspDrv,CAM_CTL_DMA_EN,RRZO_EN)){
            rrz_wd_slave = CAM_READ_BITS(pTwinIspDrv,CAM_RRZ_OUT_IMG,RRZ_OUT_WD);
            rlb_oft = update_afterRunTwin_rlb(rlb_oft, rrz_wd_prv, rrz_wd_slave);
            if(!rlb_oft){
                return 1;
            }
            CAM_WRITE_BITS(pTwinIspDrv,CAM_RRZ_RLB_AOFST,RRZ_RLB_AOFST, rlb_oft);
            rrz_wd_prv = rrz_wd_slave;
        }
    }
    //m_dma_*_L & fbc ctl
    fbc_rrzo.m_pIspDrv = (IspDrvVir*)this->m_pDrv;
    fbc_ufgo.m_pIspDrv = (IspDrvVir*)this->m_pDrv;
    ufgo.m_pIspDrv = this->m_pDrv;
    if(CAM_READ_BITS(this->m_pDrv,CAM_CTL_DMA_EN,RRZO_EN) ){
        dma.module = this->m_hwModule;
        dma.channel = _rrzo_;
        dma.offsetAdr = CAM_READ_REG(this->m_pDrv,CAM_RRZO_OFST_ADDR);


        fbc_rrzo.enable(NULL);

        if(CAM_READ_BITS(this->m_pDrv,CAM_CTL_DMA_EN,UFGO_EN)){
             fbc_ufgo.enable(NULL);
             dma.uf_offsetAdr = CAM_READ_REG(this->m_pDrv,CAM_UFGO_OFST_ADDR);
             ufgo.update_AU();
        }
        else{
             fbc_ufgo.disable(NULL);
             dma.uf_offsetAdr = 0;
        }

        channel.push_back(dma);
    }
    else{
        fbc_rrzo.disable(NULL);
        fbc_ufgo.disable(NULL);
        ufgo.disable(NULL);
    }

    //for frame end token
    dma.bFrameEnd = MTRUE;
    channel.push_back(dma);

    this->m_dma_rrzo_L.push_back(channel);

    //write cam_b's header register from cam_A
    for(MUINT32 i = 0; i < slave_num; i++)
    {
        pTwinIspDrv = this->m_pTwinIspDrv_v.at(i);
        ISP_DRV_REG_IO_STRUCT Regs_R[E_HEADER_MAX -1];

        CAM_BURST_READ_REGS(this->m_pDrv,CAM_RRZO_FH_SPARE_2,Regs_R,E_HEADER_MAX -1);
        CAM_BURST_WRITE_REGS(pTwinIspDrv,CAM_RRZO_FH_SPARE_2,Regs_R,E_HEADER_MAX -1);

        if(CAM_READ_BITS(pTwinIspDrv,CAM_CTL_DMA_EN,UFGO_EN)){
            CAM_BURST_READ_REGS(this->m_pDrv,CAM_UFGO_FH_SPARE_2,Regs_R,E_HEADER_MAX -1);
            CAM_BURST_WRITE_REGS(pTwinIspDrv,CAM_UFGO_FH_SPARE_2,Regs_R,E_HEADER_MAX -1);
        }
    }

    //write cam_b's header register from cam_A
    this->update_afterRunTwin_FH(slave_num);

    return 0;
}

MINT32 CAM_TWIN_PIPE::update_afterRunTwin_FH(MUINT32 slave_num)
{
    Header_RRZO fh_rrzo;
    MUINT32 value;
    ISP_DRV_CAM* pTwinIspDrv;

    for(MUINT32 i = 0; i < slave_num; i++)
    {
        pTwinIspDrv = this->m_pTwinIspDrv_v.at(i);
        if(CAM_READ_BITS(pTwinIspDrv,CAM_CTL_DMA_EN,IMGO_EN)){
            Header_IMGO fh_imgo;

            //
            value = fh_imgo.GetRegInfo(Header_IMGO::E_IMG_PA,(IspDrv *)this->m_pDrv);
            fh_imgo.Header_Enque(Header_IMGO::E_IMG_PA,pTwinIspDrv,value);

            //
            value = fh_imgo.GetRegInfo(Header_IMGO::E_ENQUE_SOF,(IspDrv *)this->m_pDrv);
            fh_imgo.Header_Enque(Header_IMGO::E_ENQUE_SOF,pTwinIspDrv,value);

            if(CAM_READ_BITS(pTwinIspDrv,CAM_CTL_DMA_EN,UFEO_EN)){
                Header_UFEO fh_ufeo;

                //
                value = fh_ufeo.GetRegInfo(Header_UFEO::E_IMG_PA,(IspDrv *)this->m_pDrv);
                fh_ufeo.Header_Enque(Header_UFEO::E_IMG_PA,pTwinIspDrv,value);

                //
                value = fh_ufeo.GetRegInfo(Header_UFEO::E_ENQUE_SOF,(IspDrv *)this->m_pDrv);
                fh_ufeo.Header_Enque(Header_UFEO::E_ENQUE_SOF,pTwinIspDrv,value);
            }
        }

        //
        value = fh_rrzo.GetRegInfo(Header_RRZO::E_IMG_PA,(IspDrv *)this->m_pDrv);
        fh_rrzo.Header_Enque(Header_RRZO::E_IMG_PA,pTwinIspDrv,value);

        //
        value = fh_rrzo.GetRegInfo(Header_RRZO::E_IS_UFG_FMT,(IspDrv *)this->m_pDrv);
        fh_rrzo.Header_Enque(Header_RRZO::E_IS_UFG_FMT,pTwinIspDrv,value);

        //
        value = fh_rrzo.GetRegInfo(Header_RRZO::E_ENQUE_SOF,(IspDrv *)this->m_pDrv);
        fh_rrzo.Header_Enque(Header_RRZO::E_ENQUE_SOF,pTwinIspDrv,value);

        if(CAM_READ_BITS(pTwinIspDrv,CAM_CTL_DMA_EN,UFGO_EN)){
            Header_UFGO fh_ufgo;

            //
            value = fh_ufgo.GetRegInfo(Header_UFGO::E_IMG_PA,(IspDrv *)this->m_pDrv);
            fh_ufgo.Header_Enque(Header_UFGO::E_IMG_PA,pTwinIspDrv,value);

            //
            value = fh_ufgo.GetRegInfo(Header_UFGO::E_ENQUE_SOF,(IspDrv *)this->m_pDrv);
            fh_ufgo.Header_Enque(Header_UFGO::E_ENQUE_SOF,pTwinIspDrv,value);
        }
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
    MUINT32 page, slave_num;
    ISP_DRV_CAM* pTwinIspDrv;
    ISP_HW_MODULE twinHwModule;
    std::string dbg_str = "twinHwModule: ";

    this->m_pDrv = (ISP_DRV_CAM*)this->m_pIspDrv;
    this->m_pDrv->getCurObjInfo(&this->m_hwModule,&cq,&page);

    if(slaveNumCheck(slave_num))
        return 1;

    CAM_FUNC_DBG("+, slavenum:%d,cq:%d,page:%d\n", slave_num, cq, page);

    CAM_WRITE_BITS(this->m_pDrv, CAM_CTL_TWIN_STATUS, SLAVE_CAM_NUM, 0);
    CAM_WRITE_BITS(this->m_pDrv, CAM_CTL_TWIN_STATUS,TWIN_EN,  0x0);

    //
    for(MUINT32 i = 0; i < slave_num; i++)
    {
        pTwinIspDrv = this->m_pTwinIspDrv_v.at(i);
        pTwinIspDrv->getCurObjInfo(&twinHwModule,&cq,&page);

        CAM_FUNC_VRB("[%d_%d] twinHwModule:%d,cq:%d,page:%d\n",slave_num,i,twinHwModule,cq,page);
        dbg_str += std::to_string(twinHwModule) + ",";

        //update twin status
        CAM_WRITE_BITS(pTwinIspDrv, CAM_CTL_TWIN_STATUS,TWIN_EN,  0x0);
        CAM_WRITE_BITS(pTwinIspDrv, CAM_CTL_TWIN_STATUS, SLAVE_CAM_NUM,0x0);
        if(CAM_READ_BITS(this->m_pDrv, CAM_CTL_TWIN_STATUS,TWIN_MODULE) == twinHwModule){
            CAM_WRITE_BITS(this->m_pDrv, CAM_CTL_TWIN_STATUS,TWIN_MODULE, CAM_MAX);
            CAM_WRITE_BITS(pTwinIspDrv, CAM_CTL_TWIN_STATUS,TWIN_MODULE, CAM_MAX);
        } else {
            CAM_WRITE_BITS(this->m_pDrv, CAM_CTL_TWIN_STATUS,TRIPLE_MODULE, CAM_MAX);
            CAM_WRITE_BITS(pTwinIspDrv, CAM_CTL_TWIN_STATUS,TRIPLE_MODULE, CAM_MAX);
        }

        // !!Must set SGG_SEL = 0, or run other case CAMA will enconter error
        CAM_WRITE_BITS(pTwinIspDrv, CAM_CTL_SEL, SGG_SEL, 0x0);

        // !!Must set dmx_sel = 0, or run other case CAMA will enconter error
        CAM_WRITE_BITS(pTwinIspDrv, CAM_CTL_SEL, DMX_SEL, 0x0);

        // !!Must set dmx_id, CAMB=1, CAMC=2
        switch(twinHwModule){
            case CAM_A:
                CAM_WRITE_BITS(pTwinIspDrv,CAM_CTL_FMT_SEL,DMX_ID,0x0);
                break;
            case CAM_B:
                CAM_WRITE_BITS(pTwinIspDrv,CAM_CTL_FMT_SEL,DMX_ID,0x1);
                break;
            case CAM_C:
                CAM_WRITE_BITS(pTwinIspDrv,CAM_CTL_FMT_SEL,DMX_ID,0x2);
                break;
            default:
                CAM_FUNC_ERR("unsupported slave:%d\n",twinHwModule);
                break;
        }

        //need to disable rcnt_inc.
        //because we need this page to switch dmx, but we don't want to have impact on fbc

#if (BMX_AMX_DTWIN_WROK_AROUND == 1)
        CAM_WRITE_REG(pTwinIspDrv,CAM_CTL_EN, 1);
        CAM_WRITE_REG(pTwinIspDrv,CAM_CTL_EN2, 0);
#endif

        CAM_WRITE_REG(pTwinIspDrv,CAM_CTL_FBC_RCNT_INC,0);

        //close fbc to prevent fbc counter abnormal
        if(CAM_READ_BITS(pTwinIspDrv,CAM_CTL_DMA_EN,IMGO_EN))
            CAM_WRITE_BITS(pTwinIspDrv,CAM_FBC_IMGO_CTL1,FBC_EN,0);

        if(CAM_READ_BITS(pTwinIspDrv,CAM_CTL_DMA_EN,RRZO_EN))
            CAM_WRITE_BITS(pTwinIspDrv,CAM_FBC_RRZO_CTL1,FBC_EN,0);

        if(CAM_READ_BITS(pTwinIspDrv,CAM_CTL_DMA_EN,AFO_EN))
            CAM_WRITE_BITS(pTwinIspDrv,CAM_FBC_AFO_CTL1,FBC_EN,0);
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
    MUINT32 page, slave_num;
    ISP_DRV_CAM* pTwinIspDrv;
    ISP_DRV_CAM* pTwinIspDrvPrv = NULL;
    ISP_HW_MODULE twinHwModule, twinHwModulePrv = CAM_MAX;
    std::string dbg_str = "twinHwModule: ";

    this->m_pDrv = (ISP_DRV_CAM*)this->m_pIspDrv;
    this->m_pDrv->getCurObjInfo(&this->m_hwModule,&cq,&page);

    if(slaveNumCheck(slave_num))
        return 1;

    CAM_FUNC_DBG("+, slavenum:%d,cq:%d,page:%d\n", slave_num, cq, page);

    CAM_WRITE_BITS(this->m_pDrv, CAM_CTL_TWIN_STATUS,TWIN_EN,  0x1);
    CAM_WRITE_BITS(this->m_pDrv, CAM_CTL_TWIN_STATUS,SLAVE_CAM_NUM, slave_num);

    for(MUINT32 i = 0; i < slave_num; i++)
    {
        pTwinIspDrv = this->m_pTwinIspDrv_v.at(i);
        pTwinIspDrv->getCurObjInfo(&twinHwModule,&cq,&page);

        CAM_FUNC_VRB("[%d_%d] twinHwModule:%d,cq:%d,page:%d\n",slave_num,i,twinHwModule,cq,page);
        dbg_str += std::to_string(twinHwModule) + ",";

        //disable db due to dynamic twin(cam will have time-sharing,mater cam will enable db_en)
        CAM_WRITE_BITS(pTwinIspDrv->getPhyObj(),CAM_CTL_MISC,DB_EN,0);

        //update twin status
        CAM_WRITE_BITS(pTwinIspDrv, CAM_CTL_TWIN_STATUS, TWIN_EN,  0x1);
        CAM_WRITE_BITS(pTwinIspDrv, CAM_CTL_TWIN_STATUS, SLAVE_CAM_NUM, slave_num);
        CAM_WRITE_BITS(pTwinIspDrv, CAM_CTL_TWIN_STATUS, MASTER_MODULE, this->m_hwModule);

        // special, w via vir
        CAM_WRITE_REG(pTwinIspDrv,CAM_DMA_RSV1,(CAM_READ_REG(pTwinIspDrv->getPhyObj(),CAM_DMA_RSV1)&0x7fffffff));//bit31 for smi mask, blocking imgo output
        CAM_WRITE_REG(pTwinIspDrv,CAM_DMA_RSV6,0xffffffff);

        if(i == 0){
            CAM_WRITE_BITS(this->m_pDrv, CAM_CTL_TWIN_STATUS, TWIN_MODULE, twinHwModule);
            CAM_WRITE_BITS(pTwinIspDrv, CAM_CTL_TWIN_STATUS,  TWIN_MODULE, twinHwModule);
            //
            CAM_WRITE_BITS(this->m_pDrv, CAM_CTL_TWIN_STATUS, TRIPLE_MODULE, CAM_MAX);
            CAM_WRITE_BITS(pTwinIspDrv, CAM_CTL_TWIN_STATUS,  TRIPLE_MODULE, CAM_MAX);
        } else {
            CAM_WRITE_BITS(pTwinIspDrv, CAM_CTL_TWIN_STATUS,    TWIN_MODULE,   twinHwModulePrv);
            //
            CAM_WRITE_BITS(this->m_pDrv, CAM_CTL_TWIN_STATUS,   TRIPLE_MODULE, twinHwModule);
            CAM_WRITE_BITS(pTwinIspDrv, CAM_CTL_TWIN_STATUS,    TRIPLE_MODULE, twinHwModule);
            CAM_WRITE_BITS(pTwinIspDrvPrv, CAM_CTL_TWIN_STATUS, TRIPLE_MODULE, twinHwModule);
        }

        // !!Must set CAMB's SGG_SEL = 0, or run other case CAMA will enconter error
        CAM_WRITE_BITS(pTwinIspDrv, CAM_CTL_SEL, SGG_SEL, 0x1);

        // !!Must set CAMB's dmx_sel = 0, or run other case CAMA will enconter error
        CAM_WRITE_BITS(pTwinIspDrv, CAM_CTL_SEL, DMX_SEL, 0x1);

        // !!Must set dmx_id, same to mater cam,
        CAM_WRITE_BITS(pTwinIspDrv,CAM_CTL_FMT_SEL,DMX_ID,this->m_hwModule);

#if (BMX_AMX_DTWIN_WROK_AROUND == 1)
        CAM_WRITE_REG(pTwinIspDrv,CAM_CTL_EN, 0);
#endif

        //need to enable FBC
        if(CAM_READ_BITS(pTwinIspDrv,CAM_CTL_DMA_EN,IMGO_EN))
            CAM_WRITE_BITS(pTwinIspDrv,CAM_FBC_IMGO_CTL1,FBC_EN,1);

        if(CAM_READ_BITS(pTwinIspDrv,CAM_CTL_DMA_EN,RRZO_EN))
            CAM_WRITE_BITS(pTwinIspDrv,CAM_FBC_RRZO_CTL1,FBC_EN,1);

        if(CAM_READ_BITS(pTwinIspDrv,CAM_CTL_DMA_EN,AFO_EN))
            CAM_WRITE_BITS(pTwinIspDrv,CAM_FBC_AFO_CTL1,FBC_EN,1);

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

    this->m_pDrv = (ISP_DRV_CAM*)this->m_pIspDrv;
    this->m_pDrv->getCurObjInfo(&this->m_hwModule,&cq,&page);

    CAM_FUNC_DBG("+,slavenum:%d,en:%d,cq:%d,page:%d\n",
        this->m_pTwinIspDrv_v.size(),bEn,cq,page);

    CAM_WRITE_BITS(this->m_pDrv, CAM_CTL_TWIN_STATUS, TWIN_EN, bEn);
    if(!bEn) {
        //if bEn == 0, this->m_pTwinIspDrv_v.size() should be 0, skip slaveNumCheck()
        CAM_WRITE_BITS(this->m_pDrv, CAM_CTL_TWIN_STATUS, SLAVE_CAM_NUM, 0);
        CAM_WRITE_BITS(this->m_pDrv, CAM_CTL_TWIN_STATUS, MASTER_MODULE, this->m_hwModule);
        CAM_WRITE_BITS(this->m_pDrv, CAM_CTL_TWIN_STATUS, TWIN_MODULE, CAM_MAX);
        CAM_WRITE_BITS(this->m_pDrv, CAM_CTL_TWIN_STATUS, TRIPLE_MODULE, CAM_MAX);
    }
    else {
        if(slaveNumCheck(slave_num))
            return 1;

        CAM_WRITE_BITS(this->m_pDrv, CAM_CTL_TWIN_STATUS, SLAVE_CAM_NUM, slave_num);
        CAM_WRITE_BITS(this->m_pDrv, CAM_CTL_TWIN_STATUS, MASTER_MODULE, this->m_hwModule);
        dbg_str += ",twinHwModule: ";
    }
    //update TWIN_STATUS & DMA ultra setting
    for(MUINT32 i = 0; i < slave_num; i++)
    {
        pTwinIspDrv = this->m_pTwinIspDrv_v.at(i);
        pTwinIspDrv->getCurObjInfo(&twinHwModule,&cq,&page);

        CAM_FUNC_DBG("[%d_%d] twinHwModule:%d,cq:%d,page:%d\n",slave_num,i,twinHwModule,cq,page);
        dbg_str += std::to_string(twinHwModule) + ",";

        CAM_WRITE_BITS(pTwinIspDrv, CAM_CTL_TWIN_STATUS, TWIN_EN, bEn);
        if(!bEn) {
            CAM_WRITE_BITS(pTwinIspDrv, CAM_CTL_TWIN_STATUS, SLAVE_CAM_NUM, 0);
            CAM_WRITE_BITS(pTwinIspDrv, CAM_CTL_TWIN_STATUS, MASTER_MODULE, CAM_MAX);
            CAM_WRITE_BITS(pTwinIspDrv, CAM_CTL_TWIN_STATUS, TWIN_MODULE, CAM_MAX);
            CAM_WRITE_BITS(pTwinIspDrv, CAM_CTL_TWIN_STATUS, TRIPLE_MODULE, CAM_MAX);
        } else {
            CAM_WRITE_BITS(pTwinIspDrv, CAM_CTL_TWIN_STATUS, SLAVE_CAM_NUM, slave_num);
            CAM_WRITE_BITS(pTwinIspDrv, CAM_CTL_TWIN_STATUS, MASTER_MODULE, this->m_hwModule);

            if(i == 0){
                CAM_WRITE_BITS(this->m_pDrv, CAM_CTL_TWIN_STATUS, TWIN_MODULE, twinHwModule);
                CAM_WRITE_BITS(pTwinIspDrv, CAM_CTL_TWIN_STATUS,  TWIN_MODULE, twinHwModule);
                //
                CAM_WRITE_BITS(this->m_pDrv, CAM_CTL_TWIN_STATUS, TRIPLE_MODULE, CAM_MAX);
                CAM_WRITE_BITS(pTwinIspDrv, CAM_CTL_TWIN_STATUS,  TRIPLE_MODULE, CAM_MAX);
            } else {
                CAM_WRITE_BITS(pTwinIspDrv, CAM_CTL_TWIN_STATUS,    TWIN_MODULE,   twinHwModulePrv);
                //
                CAM_WRITE_BITS(this->m_pDrv, CAM_CTL_TWIN_STATUS,   TRIPLE_MODULE, twinHwModule);
                CAM_WRITE_BITS(pTwinIspDrv, CAM_CTL_TWIN_STATUS,    TRIPLE_MODULE, twinHwModule);
                CAM_WRITE_BITS(pTwinIspDrvPrv, CAM_CTL_TWIN_STATUS, TRIPLE_MODULE, twinHwModule);
            }

            //config once for fifosize/ultra/pre-ultra
            lsci.m_pIspDrv = pTwinIspDrv;
            bpci.m_pIspDrv = pTwinIspDrv;
            imgo.m_pIspDrv = pTwinIspDrv;
            rrzo.m_pIspDrv = pTwinIspDrv;
            afo.m_pIspDrv = pTwinIspDrv;

            lsci.config();
            bpci.config();
            imgo.config();
            rrzo.config();
            afo.config();
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

    this->m_pDrv = (ISP_DRV_CAM*)this->m_pIspDrv;
    this->m_pDrv->getCurObjInfo(&this->m_hwModule,&cq,&page);

    if(slaveNumCheck(slave_num))
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

                    IspFunction_B::fbcPhyLock[twinHwModule][_imgo_].lock();
                    IspFunction_B::fbcPhyLock[twinHwModule][_rrzo_].lock();
                    IspFunction_B::fbcPhyLock[twinHwModule][_ufeo_].lock();
                    IspFunction_B::fbcPhyLock[twinHwModule][_ufgo_].lock();
                    IspFunction_B::fbcPhyLock[twinHwModule][_afo_].lock();

                    //keep fbc cnt
                    if(CAM_READ_BITS(pTwinIspDrv->getPhyObj(),CAM_FBC_IMGO_CTL1,FBC_EN)){
                        this->fbc_IMGO.Raw = CAM_READ_REG(pTwinIspDrv->getPhyObj(),CAM_FBC_IMGO_CTL2);
                    }

                    if(CAM_READ_BITS(pTwinIspDrv->getPhyObj(),CAM_FBC_RRZO_CTL1,FBC_EN)){
                        this->fbc_RRZO.Raw = CAM_READ_REG(pTwinIspDrv->getPhyObj(),CAM_FBC_RRZO_CTL2);
                    }

                    if(CAM_READ_BITS(pTwinIspDrv->getPhyObj(),CAM_FBC_UFEO_CTL1,FBC_EN)){
                        this->fbc_UFEO.Raw = CAM_READ_REG(pTwinIspDrv->getPhyObj(),CAM_FBC_UFEO_CTL2);
                    }

                    if(CAM_READ_BITS(pTwinIspDrv->getPhyObj(),CAM_FBC_UFGO_CTL1,FBC_EN)){
                        this->fbc_UFGO.Raw = CAM_READ_REG(pTwinIspDrv->getPhyObj(),CAM_FBC_UFGO_CTL2);
                    }

                    if(CAM_READ_BITS(pTwinIspDrv->getPhyObj(),CAM_FBC_AFO_CTL1,FBC_EN)){
                        this->fbc_AFO.Raw = CAM_READ_REG(pTwinIspDrv->getPhyObj(),CAM_FBC_AFO_CTL2);
                    }

                    CAM_FUNC_INF("TOP reset\n");
                    CAM_WRITE_REG(pTwinIspDrv->getPhyObj(),CAM_CTL_SW_CTL,0x0);
                    CAM_WRITE_REG(pTwinIspDrv->getPhyObj(),CAM_CTL_SW_CTL,0x1);
                    while(CAM_READ_REG(pTwinIspDrv->getPhyObj(),CAM_CTL_SW_CTL) != 0x2){
                        CAM_FUNC_INF("TOP reseting...\n");
                    }
                    CAM_WRITE_REG(pTwinIspDrv->getPhyObj(),CAM_CTL_SW_CTL,0x4);
                    CAM_WRITE_REG(pTwinIspDrv->getPhyObj(),CAM_CTL_SW_CTL,0x0);

                    CAM_FUNC_DBG("Stop&Reset-, [%d_%d]twinHwModule(%d)\n", slave_num,i,twinHwModule);

                }
                break;
            case 1:
                {
                    MUINT32 frm_cnt = 0;

                    CAM_FUNC_INF("Restart +,[%d_%d]twinHwModule(%d)\n",slave_num,i,twinHwModule);

                    CAM_FUNC_INF("Restart,SAVE_FBC:IMGO_0x%08x RRZO_0x%08x UFEO_0x%08x UFGO_0x%08x AFO_0x%08x",
                            this->fbc_IMGO.Raw, this->fbc_RRZO.Raw, this->fbc_UFEO.Raw, this->fbc_UFGO.Raw, this->fbc_AFO.Raw);

                    //keep fbc cnt
                    if(CAM_READ_BITS(pTwinIspDrv->getPhyObj(),CAM_FBC_IMGO_CTL1,FBC_EN)){
                        for (frm_cnt = 0; frm_cnt < this->fbc_IMGO.Bits.FBC_CNT; frm_cnt++) {
                            CAM_WRITE_BITS(pTwinIspDrv->getPhyObj(),CAM_CTL_FBC_RCNT_INC, IMGO_RCNT_INC, 1);
                        }
                        CAM_WRITE_REG(pTwinIspDrv->getPhyObj(),CAM_FBC_IMGO_CTL2,this->fbc_IMGO.Raw);
                    }

                    if(CAM_READ_BITS(pTwinIspDrv->getPhyObj(),CAM_FBC_RRZO_CTL1,FBC_EN)){
                        for (frm_cnt = 0; frm_cnt < this->fbc_RRZO.Bits.FBC_CNT; frm_cnt++) {
                            CAM_WRITE_BITS(pTwinIspDrv->getPhyObj(),CAM_CTL_FBC_RCNT_INC, RRZO_RCNT_INC, 1);
                        }
                        CAM_WRITE_REG(pTwinIspDrv->getPhyObj(),CAM_FBC_RRZO_CTL2,this->fbc_RRZO.Raw);
                    }

                    if(CAM_READ_BITS(pTwinIspDrv->getPhyObj(),CAM_FBC_UFEO_CTL1,FBC_EN)){
                        for (frm_cnt = 0; frm_cnt < this->fbc_UFEO.Bits.FBC_CNT; frm_cnt++) {
                            CAM_WRITE_BITS(pTwinIspDrv->getPhyObj(),CAM_CTL_FBC_RCNT_INC, UFEO_RCNT_INC, 1);
                        }
                        CAM_WRITE_REG(pTwinIspDrv->getPhyObj(),CAM_FBC_UFEO_CTL2,this->fbc_UFEO.Raw);
                    }

                    if(CAM_READ_BITS(pTwinIspDrv->getPhyObj(),CAM_FBC_UFGO_CTL1,FBC_EN)){
                        for (frm_cnt = 0; frm_cnt < this->fbc_UFGO.Bits.FBC_CNT; frm_cnt++) {
                            CAM_WRITE_BITS(pTwinIspDrv->getPhyObj(),CAM_CTL_FBC_RCNT_INC, UFGO_RCNT_INC, 1);
                        }
                        CAM_WRITE_REG(pTwinIspDrv->getPhyObj(),CAM_FBC_UFGO_CTL2,this->fbc_UFGO.Raw);
                    }

                    if(CAM_READ_BITS(pTwinIspDrv->getPhyObj(),CAM_FBC_AFO_CTL1,FBC_EN)) {
                        for (frm_cnt = 0; frm_cnt < this->fbc_AFO.Bits.FBC_CNT; frm_cnt++) {
                            switch (twinHwModule) {
                                case CAM_B:
                                    CAM_WRITE_BITS(pTwinIspDrv->getPhyObj(),CAM_CTL_FBC_RCNT_INC, AFO_B_RCNT_INC, 1);
                                    break;
                                case CAM_C:
                                    CAM_WRITE_BITS(pTwinIspDrv->getPhyObj(),CAM_CTL_FBC_RCNT_INC, AFO_C_RCNT_INC, 1);
                                    break;
                                default:
                                    CAM_FUNC_ERR("Unsupported module: %d\n",twinHwModule);
                                    break;
                            }
                        }
                    }
                    IspFunction_B::fbcPhyLock[twinHwModule][_imgo_].unlock();
                    IspFunction_B::fbcPhyLock[twinHwModule][_rrzo_].unlock();
                    IspFunction_B::fbcPhyLock[twinHwModule][_ufeo_].unlock();
                    IspFunction_B::fbcPhyLock[twinHwModule][_ufgo_].unlock();
                    IspFunction_B::fbcPhyLock[twinHwModule][_afo_].unlock();


                    CAM_FUNC_INF("Restart, PHYS_FBC:IMGO_0x%08x RRZO_0x%08x UFEO_0x%08x UFGO_0x%08x AFO_0x%08x",
                            CAM_READ_REG(pTwinIspDrv->getPhyObj(), CAM_FBC_IMGO_CTL2),
                            CAM_READ_REG(pTwinIspDrv->getPhyObj(), CAM_FBC_RRZO_CTL2),
                            CAM_READ_REG(pTwinIspDrv->getPhyObj(), CAM_FBC_UFEO_CTL2),
                            CAM_READ_REG(pTwinIspDrv->getPhyObj(), CAM_FBC_UFGO_CTL2),
                            CAM_READ_REG(pTwinIspDrv->getPhyObj(), CAM_FBC_AFO_CTL2));

                    CAM_FUNC_DBG("Restart-, [%d_%d]twinHwModule:%d-\n",slave_num,i,twinHwModule);
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
            ptr = (ISP_DRV_CAM*)ISP_DRV_CAM::createInstance(module,ISP_DRV_CQ_THRE0,0,"GetTwinAddr");
            *pAddr = CAM_REG_ADDR(ptr,CAM_CTL_TWIN_STATUS);
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

