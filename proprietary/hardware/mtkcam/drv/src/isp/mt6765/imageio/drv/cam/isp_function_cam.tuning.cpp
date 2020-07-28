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

#define LOG_TAG "ifunc_cam_tuning"

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
#include "sec_mgr.h"

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
    MBOOL _pdi_en = MFALSE;
    MUINT32 _pmx_sel;

    DMA_BPCI bpci;
    DMA_PDI pdi;
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
        if (pReg_st == NULL) {
            CAM_FUNC_ERR("malloc error\n");
            ret = 1;
            goto EXIT;
        }
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
        if (pReg_st == NULL) {
            CAM_FUNC_ERR("malloc error\n");
            ret = 1;
            goto EXIT;
        }

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
        if (pReg_st == NULL) {
            CAM_FUNC_ERR("malloc error\n");
            ret = 1;
            goto EXIT;
        }

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
        if (pReg_st == NULL) {
            CAM_FUNC_ERR("malloc error\n");
            ret = 1;
            goto EXIT;
        }

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

    //for BNR, PBN, PDE check path
    _pmx_sel = CAM_READ_BITS(this->m_pDrv,CAM_CTL_SEL,PMX_SEL);
    //
    //BNR
    if(vpTuningNodes[page]->ctlEn_CAM & BNR_EN_){
        /*Enable BNR before PDO_EN is enabled*/
        if (_pmx_sel == 0) {
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
        }  else if(_pmx_sel == 2) {//check bpci
            if(CAM_READ_BITS(this->m_pDrv,CAM_CTL_DMA_EN,PDO_EN)){
                if((pReg->CAM_BNR_BPC_CON.Bits.BPC_LUT_EN * (vpTuningNodes[page]->ctlEnDMA_CAM & BPCI_EN_)) == 0){
                    CAM_FUNC_WRN("bpci & lut_en better be enabled when bpci/lut is enabled, or might effect quality\n");
                    goto BYPASS_BNR;
                }
                if( pReg->CAM_BNR_PDC_CON.Bits.PDC_EN  == 0){
                    CAM_FUNC_WRN("PDC_EN better be enabled when PDO_EN is enabled while pmx_sel=2, or might effect quality\n");
                    goto BYPASS_BNR;
                }
                _bpci_en = MTRUE;
            }
        }
        else {
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
        if(CAM_READ_BITS(this->m_pDrv,CAM_CTL_DMA_EN,PDO_EN) == 1 && _pmx_sel == 0){
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
        if (pReg_st == NULL) {
            CAM_FUNC_ERR("malloc error\n");
            ret = 1;
            goto EXIT;
        }

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

            if (NULL == pReg->CAM_LSCI_BASE_ADDR.Raw) {
                CAM_FUNC_ERR("BNR: bpci PA is null!");
            }

            if (SecMgr::SecMgr_GetSecureCamStatus(this->m_hwModule) > 0) {
                SecMgr* pSec = SecMgr::SecMgr_GetMgrObj();
                SecMgr_SecInfo secinfo;

                secinfo.type = SECMEM_BPC;
                secinfo.module = this->m_hwModule;
                secinfo.buff_size = bpci.dma_cfg.size.stride * bpci.dma_cfg.size.h;
                secinfo.buff_va = vpTuningNodes[page]->Dmai_bufinfo[eIspTuningMgrFuncBit_BNR].VA_addr;
                secinfo.memID = vpTuningNodes[page]->Dmai_bufinfo[eIspTuningMgrFuncBit_BNR].FD;
                if (pSec->SecMgr_P1RegisterShareMem(&secinfo) == MFALSE) {
                    CAM_FUNC_ERR("BPCI P1RegisterShareMem failed\n");
                }
                bpci.dma_cfg.memBuf.base_pAddr = secinfo.buff_sec_mva;

                if (pSec->SecMgr_P1MigrateTable(&secinfo) == MFALSE) {
                    CAM_FUNC_ERR("BPCI P1Migrate Table failed\n");
                }

                CAM_FUNC_INF("SECMEM_BPC size:0x%x buff sec_mva/va:0x%x_0x%llx memID:0x%x",
                             secinfo.buff_size, secinfo.buff_sec_mva, secinfo.buff_va, secinfo.memID);
            }

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
        if (CAM_READ_BITS(this->m_pDrv,CAM_CTL_EN,PBN_EN) != 1){
            CAM_FUNC_ERR("can't set PBN tuning when PBN_EN=0\n");
            goto BYPASS_PBN;
        }
        if (_pmx_sel != 1){
            CAM_FUNC_ERR("can't enable PBN when PMX_SEL!=1(%x)\n", _pmx_sel);
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
        if (pReg_st == NULL) {
            CAM_FUNC_ERR("malloc error\n");
            ret = 1;
            goto EXIT;
        }

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
    //PDE
    _pdo_en = MFALSE;
    if(vpTuningNodes[page]->ctlEnDMA_CAM & PDE_EN_){
        if (_pmx_sel != 2) {
            CAM_FUNC_ERR("can't enable PDE but PMX_SEL!=2(%x)\n", _pmx_sel);
            goto BYPASS_PDE;
        }

        if((vpTuningNodes[page]->ctlEnDMA_CAM & PDI_EN_) == 0){
            CAM_FUNC_ERR("PDE & PDI must be enabled at the same time\n");
            this->m_pDrv->cqNopModule(CAM_ISP_PDE_);
            goto BYPASS_PDE;
        }
        _pdi_en = MTRUE;

        if(CAM_READ_BITS(this->m_pDrv,CAM_CTL_DMA_EN,PDO_EN)){
            _pdo_en = MTRUE;
        }

        CAM_WRITE_BITS(this->m_pDrv,CAM_CTL_DMA_EN,PDE_EN,1);

    } else {
        if(CAM_READ_BITS(this->m_pDrv,CAM_CTL_DMA_EN,PDO_EN) == 1 && _pmx_sel == 2){
            CAM_FUNC_ERR("can't disable PDE when PDO_EN is enabled and PMX_SEL=2\n");
            goto BYPASS_PDE;
        }
        CAM_WRITE_BITS(this->m_pDrv,CAM_CTL_DMA_EN,PDE_EN,0);
    }

    if(vpTuningNodes[page]->eUpdateFuncBit & eIspTuningMgrFunc_PDE){
        DMA_PDO pdo;

        this->m_pDrv->cqApbModule(CAM_ISP_PDE_);
        //
        this->m_pDrv->getCQModuleInfo(CAM_ISP_PDE_,module_adr,module_size);
        //
        pReg_st = (ISP_DRV_REG_IO_STRUCT*)malloc(sizeof(ISP_DRV_REG_IO_STRUCT)*module_size);
        if (pReg_st == NULL) {
            CAM_FUNC_ERR("malloc error\n");
            ret = 1;
            goto EXIT;
        }

        //
        for(MUINT32 j=0;j<module_size;j++)
            pReg_st[j].Data = vpTuningNodes[page]->pTuningRegBuf[(module_adr + j*0x4)>>2];
        //
        CAM_BURST_WRITE_REGS(this->m_pDrv,CAM_PDE_TBLI1,pReg_st,module_size);

        free(pReg_st);

        //
        if(_pdo_en){
            pdo.m_pIspDrv = this->m_pIspDrv;
            pdo.dma_cfg.size.xsize = pReg->CAM_PDO_XSIZE.Bits.XSIZE;
            pdo.dma_cfg.size.h = pReg->CAM_PDO_YSIZE.Bits.YSIZE;
            pdo.dma_cfg.size.stride = PDO_STRIDE_ALIGN(pdo.dma_cfg.size.xsize);
            pdo.config();
        }

        if (_pdi_en) {
            //for pipechk
            PIPE_CHECK pipe_chk;
            //ion handle: buffer
            ISP_DEV_ION_NODE_STRUCT IonNode;
            IonNode.devNode = this->m_hwModule;
            IonNode.memID = vpTuningNodes[page]->Dmai_bufinfo[eIspTuningMgrFuncBit_PDE].FD;
            IonNode.dmaPort = _dma_pdi_;
            this->m_pDrv->setDeviceInfo(_SET_ION_HANDLE,(MUINT8*)&IonNode);

            pdi.m_pIspDrv = this->m_pIspDrv;
            pdi.dma_cfg.memBuf.base_pAddr = pReg->CAM_PDI_BASE_ADDR.Raw;
            pdi.dma_cfg.size.xsize = pReg->CAM_PDI_XSIZE.Bits.XSIZE;
            pdi.dma_cfg.size.h = pReg->CAM_PDI_YSIZE.Bits.YSIZE;
            pdi.dma_cfg.size.stride = pdi.dma_cfg.size.xsize;
            pdi.write2CQ();
            pdi.config();
            pdi.enable(NULL);

            pipe_chk.m_PDI_VA = vpTuningNodes[page]->Dmai_bufinfo[eIspTuningMgrFuncBit_PDE].VA_addr;
        }
        else{
            PIPE_CHECK pipe_chk;
            pdi.m_pIspDrv = this->m_pIspDrv;
            pdi.disable();

            pipe_chk.m_PDI_VA = 0;
        }

        CAM_FUNC_DBG("PDE:[0x%x_0x%x] PDI:[0x%x_0x%x_0x%x],[0x%x_0x%x_0x%x]\n",\
            vpTuningNodes[page]->pTuningRegBuf[(module_adr )>>2],\
            this->m_pDrv->readReg((module_adr ),CAM_A),\
            pReg->CAM_PDI_XSIZE.Raw,\
            pReg->CAM_PDI_YSIZE.Raw,\
            pReg->CAM_PDI_STRIDE.Raw,\
            CAM_READ_REG(this->m_pDrv,CAM_PDI_XSIZE),\
            CAM_READ_REG(this->m_pDrv,CAM_PDI_YSIZE),\
            CAM_READ_REG(this->m_pDrv,CAM_PDI_STRIDE));
    }
    else{
        this->m_pDrv->cqNopModule(CAM_ISP_PDE_);
    }
BYPASS_PDE:

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
        if (pReg_st == NULL) {
            CAM_FUNC_ERR("malloc error\n");
            ret = 1;
            goto EXIT;
        }

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
        if (pReg_st == NULL) {
            CAM_FUNC_ERR("malloc error\n");
            ret = 1;
            goto EXIT;
        }

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

        if (NULL == pReg->CAM_LSCI_BASE_ADDR.Raw) {
            CAM_FUNC_ERR("LSC: lsci PA is null!");
        }

        if (SecMgr::SecMgr_GetSecureCamStatus(this->m_hwModule) > 0) {
            SecMgr* pSec = SecMgr::SecMgr_GetMgrObj();
            SecMgr_SecInfo secinfo;

            secinfo.type = SECMEM_LSC;
            secinfo.module = this->m_hwModule;
            secinfo.buff_size = lsci.dma_cfg.size.stride * lsci.dma_cfg.size.h;
            secinfo.buff_va = vpTuningNodes[page]->Dmai_bufinfo[eIspTuningMgrFuncBit_LSC].VA_addr;
            secinfo.memID = vpTuningNodes[page]->Dmai_bufinfo[eIspTuningMgrFuncBit_LSC].FD;
            if (pSec->SecMgr_P1RegisterShareMem(&secinfo) == MFALSE) {
                CAM_FUNC_ERR("LSCI P1RegisterShareMem failed\n");
            }
            lsci.dma_cfg.memBuf.base_pAddr = secinfo.buff_sec_mva;

            if (pSec->SecMgr_P1MigrateTable(&secinfo) == MFALSE) {
                CAM_FUNC_ERR("LSCI P1Migrate Table failed\n");
            }

            CAM_FUNC_INF("SECMEM_LSC size:0x%x buff sec_mva/va:0x%x_0x%llx memID:0x%x",
                         secinfo.buff_size, secinfo.buff_sec_mva, secinfo.buff_va, secinfo.memID);
        }

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
        if (pReg_st == NULL) {
            CAM_FUNC_ERR("malloc error\n");
            ret = 1;
            goto EXIT;
        }

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
        if (pReg_st == NULL) {
            CAM_FUNC_ERR("malloc error\n");
            ret = 1;
            goto EXIT;
        }

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
        if (pReg_st == NULL) {
            CAM_FUNC_ERR("malloc error\n");
            ret = 1;
            goto EXIT;
        }
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
        if (pReg_st == NULL) {
            CAM_FUNC_ERR("malloc error\n");
            ret = 1;
            goto EXIT;
        }

        //
        for(MUINT32 j=0;j<module_size;j++)
            pReg_st[j].Data = vpTuningNodes[page]->pTuningRegBuf[(module_adr + j*0x4)>>2];
        //
        CAM_BURST_WRITE_REGS(this->m_pDrv,CAM_AE_GAIN2_0,pReg_st,module_size);

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

    //PS
    if(vpTuningNodes[page]->ctlEnDMA_CAM & PS_EN_){
        if((vpTuningNodes[page]->ctlEnDMA_CAM & PSO_EN_) == 0){
            CAM_FUNC_ERR("ps &pso must be enabled at the same time\n");
            goto BYPASS_PS;
        }
        CAM_WRITE_BITS(this->m_pDrv,CAM_CTL_DMA_EN,PS_EN,1);

    }
    else {
        if(1 == CAM_READ_BITS(this->m_pDrv,CAM_FBC_PSO_CTL1,FBC_EN)) {
            CAM_FUNC_ERR("PS supported no dynamic func off under continuous mode\n");
            goto BYPASS_PS;
        }
    }

    if(vpTuningNodes[page]->eUpdateFuncBit & eIspTuningMgrFunc_PS){
        DMA_PSO pso;

        this->m_pDrv->cqApbModule(CAM_ISP_PS_AE_);
        this->m_pDrv->cqApbModule(CAM_ISP_PS_AWB_);

        //ps_ae
        this->m_pDrv->getCQModuleInfo(CAM_ISP_PS_AE_,module_adr,module_size);
        //
        pReg_st = (ISP_DRV_REG_IO_STRUCT*)malloc(sizeof(ISP_DRV_REG_IO_STRUCT)*module_size);
        if (pReg_st == NULL) {
            CAM_FUNC_ERR("malloc error\n");
            ret = 1;
            goto EXIT;
        }

        //
        for(MUINT32 j=0;j<module_size;j++)
            pReg_st[j].Data = vpTuningNodes[page]->pTuningRegBuf[(module_adr + j*0x4)>>2];
        //
        CAM_BURST_WRITE_REGS(this->m_pDrv,CAM_PS_AE_YCOEF0,pReg_st,module_size);

        free(pReg_st);

        //ps_awb
        this->m_pDrv->getCQModuleInfo(CAM_ISP_PS_AWB_,module_adr,module_size);
        //
        pReg_st = (ISP_DRV_REG_IO_STRUCT*)malloc(sizeof(ISP_DRV_REG_IO_STRUCT)*module_size);
        if (pReg_st == NULL) {
            CAM_FUNC_ERR("malloc error\n");
            ret = 1;
            goto EXIT;
        }

        //
        for(MUINT32 j=0;j<module_size;j++)
            pReg_st[j].Data = vpTuningNodes[page]->pTuningRegBuf[(module_adr + j*0x4)>>2];
        //
        CAM_BURST_WRITE_REGS(this->m_pDrv,CAM_PS_AWB_WIN_ORG,pReg_st,module_size);

        free(pReg_st);


        //pso , ref to the config() at statistic pipe
        pso.m_pIspDrv = this->m_pIspDrv;
        pso.dma_cfg.size.xsize = pReg->CAM_PSO_XSIZE.Bits.XSIZE;
        pso.dma_cfg.size.h = pReg->CAM_PSO_YSIZE.Bits.YSIZE;
        pso.dma_cfg.size.stride = pso.dma_cfg.size.xsize;
        pso.config();

        CAM_FUNC_DBG("PS:[0x%x_0x%x_0x%x_0x%x],[0x%x_0x%x_0x%x_0x%x] \n",\
                vpTuningNodes[page]->pTuningRegBuf[(module_adr )>>2],\
                vpTuningNodes[page]->pTuningRegBuf[(module_adr + 1*0x4 )>>2],\
                vpTuningNodes[page]->pTuningRegBuf[(module_adr + 2*0x4 )>>2],\
                vpTuningNodes[page]->pTuningRegBuf[0xD30>>2],\
                this->m_pDrv->readReg((module_adr ),CAM_A),\
                this->m_pDrv->readReg((module_adr + 1*0x4 ),CAM_A),\
                this->m_pDrv->readReg((module_adr + 2*0x4 ),CAM_A),\
                this->m_pDrv->readReg(0xD30,CAM_A));
    } else{
        this->m_pDrv->cqNopModule(CAM_ISP_PS_AE_);
        this->m_pDrv->cqNopModule(CAM_ISP_PS_AWB_);
    }
BYPASS_PS:

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
        if (pReg_st == NULL) {
            CAM_FUNC_ERR("malloc error\n");
            ret = 1;
            goto EXIT;
        }

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

    //SGG5
    if(vpTuningNodes[page]->ctlEnDMA_CAM & SGG5_EN_){
        CAM_WRITE_BITS(this->m_pDrv,CAM_CTL_DMA_EN,SGG5_EN,1);
    } else{
        CAM_WRITE_BITS(this->m_pDrv,CAM_CTL_DMA_EN,SGG5_EN,0);
    }

    if(vpTuningNodes[page]->eUpdateFuncBit & eIspTuningMgrFunc_SGG5){

        this->m_pDrv->cqApbModule(CAM_ISP_SGG5_);

        //
        this->m_pDrv->getCQModuleInfo(CAM_ISP_SGG5_,module_adr,module_size);
        //
        pReg_st = (ISP_DRV_REG_IO_STRUCT*)malloc(sizeof(ISP_DRV_REG_IO_STRUCT)*module_size);
        if (pReg_st == NULL) {
            CAM_FUNC_ERR("malloc error\n");
            ret = 1;
            goto EXIT;
        }

        //
        for(MUINT32 j=0;j<module_size;j++)
            pReg_st[j].Data = vpTuningNodes[page]->pTuningRegBuf[(module_adr + j*0x4)>>2];
        //
        CAM_BURST_WRITE_REGS(this->m_pDrv,CAM_SGG5_PGN,pReg_st,module_size);

        free(pReg_st);

        CAM_FUNC_DBG("SGG5:[0x%x_0x%x_0x%x],[0x%x_0x%x_0x%x] \n",\
                vpTuningNodes[page]->pTuningRegBuf[(module_adr )>>2],\
                vpTuningNodes[page]->pTuningRegBuf[(module_adr + 1*0x4 )>>2],\
                vpTuningNodes[page]->pTuningRegBuf[(module_adr + 2*0x4 )>>2],\
                this->m_pDrv->readReg((module_adr ),CAM_A),\
                this->m_pDrv->readReg((module_adr + 1*0x4 ),CAM_A),\
                this->m_pDrv->readReg((module_adr + 2*0x4 ),CAM_A));
    } else {
        this->m_pDrv->cqNopModule(CAM_ISP_SGG5_);
    }

    //AF
    if(vpTuningNodes[page]->ctlEn_CAM & AF_EN_){
        capibility CamInfo;
        if(CamInfo.m_DTwin.GetDTwin()){
            CAM_FUNC_ERR("AF should be controlled by CCU under dynamic twin\n");
            goto BYPASS_AF;
        }
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
            capibility CamInfo;
            CAM_FUNC_ERR("AF supported no dynamic func off under continuous mode\n");
            if(CamInfo.m_DTwin.GetDTwin())
                CAM_FUNC_ERR("AF should be controlled by CCU under dynamic twin\n");
            goto BYPASS_AF;
        }
    }

    if(vpTuningNodes[page]->eUpdateFuncBit & eIspTuningMgrFunc_AF){
        DMA_AFO afo;
        capibility CamInfo;
        if(CamInfo.m_DTwin.GetDTwin()){
            CAM_FUNC_ERR("AF should be controlled by CCU under dynamic twin\n");
            goto BYPASS_AF;
        }

        if (pReg->CAM_AF_CON.Bits.AF_EXT_STAT_EN == 1 && CAM_READ_BITS(this->m_pDrv,CAM_CTL_DMA_EN,SGG5_EN) != 1) {
            CAM_FUNC_ERR("need to enable SGG5 if AF_ext is enabled\n");
            goto BYPASS_AF;
        }

        this->m_pDrv->cqApbModule(CAM_ISP_AF_);

        //
        this->m_pDrv->getCQModuleInfo(CAM_ISP_AF_,module_adr,module_size);
        //
        pReg_st = (ISP_DRV_REG_IO_STRUCT*)malloc(sizeof(ISP_DRV_REG_IO_STRUCT)*module_size);
        if (pReg_st == NULL) {
            CAM_FUNC_ERR("malloc error\n");
            ret = 1;
            goto EXIT;
        }

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

        CAM_FUNC_DBG("AF:[0x%x_0x%x_0x%x_0x%x],[0x%x_0x%x_0x%x_0x%x]\n",\
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
        if (pReg_st == NULL) {
            CAM_FUNC_ERR("malloc error\n");
            ret = 1;
            goto EXIT;
        }

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
        if (pReg_st == NULL) {
            CAM_FUNC_ERR("malloc error\n");
            ret = 1;
            goto EXIT;
        }

        //
        for(MUINT32 j=0;j<module_size;j++)
            pReg_st[j].Data = vpTuningNodes[page]->pTuningRegBuf[(module_adr + j*0x4 + ISP_REG_RANGE)>>2];
        //
        UNI_BURST_WRITE_REGS(this->m_pDrv,CAM_UNI_FLK_A_CON,pReg_st,module_size);

        free(pReg_st);

        //FLKo , ref to the config() at statistic pipe
        flko.m_pIspDrv = this->m_pIspDrv;
        flko.m_pUniDrv = this->m_pUniDrv;

        if(this->m_hwModule == CAM_A){
            flko.dma_cfg.size.xsize = pUniReg->CAM_UNI_FLKO_XSIZE.Bits.XSIZE;
            flko.dma_cfg.size.h = pUniReg->CAM_UNI_FLKO_YSIZE.Bits.YSIZE;
        }
        if(this->m_hwModule == CAM_B){
            flko.dma_cfg.size.xsize = pUniReg->CAM_UNI_B_FLKO_XSIZE.Bits.XSIZE;
            flko.dma_cfg.size.h = pUniReg->CAM_UNI_B_FLKO_YSIZE.Bits.YSIZE;
        }

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
    CAM_FUNC_INF("CAM_TUNING_CTRL - EN:0x%x DMA:0x%x\n",vpTuningNodes[page]->ctlEn_CAM,vpTuningNodes[page]->ctlEnDMA_CAM);

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
