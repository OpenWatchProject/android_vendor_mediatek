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
#include "tuning_drv_imp.h"
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

#define CAM_CB_DURATION                      (1)
#define CAM_CB_WRITE_REG(pReg,RegName,Value) ({((cam_reg_t*)pReg)->RegName.Raw = (Value);})
#define CAM_CB_READ_REG(pReg,RegName)        ({((cam_reg_t*)pReg)->RegName.Raw;})
#define CAM_CB_DURATION_US                   (3000)


CAM_TUNING_CTRL::CAM_TUNING_CTRL()
{
    this->m_Magic         = NULL;
    this->m_SenDev        = 0xFFFFFFFF;
    this->m_pTuningDrv    = NULL;
    this->m_UpdateFuncBit = 0;
    this->m_BQNum         = 0;
    this->m_hwModule      = CAM_MAX;
    this->m_pIspDrv       = NULL;
    this->m_pDrv          = NULL;
    this->m_TG_H.Raw      = 0;
    this->m_TG_W.Raw      = 0;
    this->m_bBin          = MFALSE;
    this->m_pP1Tuning     = NULL;
    memset((void *)&this->SLKUpdateStatus, MFALSE, sizeof(this->SLKUpdateStatus));
    memset((void *)&this->m_input, 0, sizeof(this->m_input));
    memset((void *)&this->m_TuningCBinput, 0, sizeof(this->m_TuningCBinput));

    //for cb use
    this->pIspReg       = NULL;
    this->m_NotifyFrame = 0;
    this->m_CurBin      = MFALSE;
    this->m_CurQbn1Acc  = _1_pix_;

    this->pIspReg = (MUINT32*)malloc(CAM_BASE_RANGE);
    if(this->pIspReg == NULL) {
        CAM_FUNC_ERR("allocate mem for callback fail\n");
    }
    // for dumpCQ CB use
    memset((void *)&this->m_dumpInput, 0, sizeof(m_dumpInput));
    this->m_dumpInput.size = CAM_BASE_RANGE;
}

CAM_TUNING_CTRL::~CAM_TUNING_CTRL()
{
    if(this->pIspReg){
        free(this->pIspReg);
        this->pIspReg = NULL;
    }

    if(this->m_dumpInput.pReg){
        free(this->m_dumpInput.pReg);
        this->m_dumpInput.pReg = NULL;
    }
}

MINT32 CAM_TUNING_CTRL::_config(void)
{
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

MINT32 CAM_TUNING_CTRL::_disable( void * )
{
    return 0;
}

MINT32 CAM_TUNING_CTRL::_init(void)
{
    return 0;
}

MINT32 CAM_TUNING_CTRL::_uninit(void)
{
    MINT32 ret = 0;
    ETuningDrvUser TuningDrvUser = eTuningDrvUser_3A;

    if(MFALSE == ((TuningDrv*)this->m_pTuningDrv)->uninit("CAM_TUNING_CTRL",TuningDrvUser)){
        CAM_FUNC_ERR("tuning drv uninit fail\n");
        ret = 1;
    }
    return ret;
}


MINT32 CAM_TUNING_CTRL::update(MBOOL UpdateFLK, MBOOL UpdatebyCB, MINT32 CBmagic)
{
    MINT32 ret = 0;
    E_ISP_CAM_CQ cq;
    MUINT32 page = 0, cqpage = 0;

    ETuningDrvUser TuningDrvUser = eTuningDrvUser_3A;
    vector <stTuningNode*> vpTuningNodes;

    MUINT32 module_adr,module_size;
    ISP_DRV_REG_IO_STRUCT* pReg_st;
    MUINT32 subSample;
    cam_reg_t* pReg;
    MBOOL _pdo_en = MFALSE;
    MBOOL _bpci_en = MFALSE;
    MBOOL _pdi_en = MFALSE;
    MUINT32 showMagic = 0;
    DMA_BPCI bpci;
    DMA_PDI pdi;
    DMA_PDO pdo;
    bpci.m_pIspDrv    = this->m_pIspDrv;
    pdo.m_pIspDrv     = this->m_pIspDrv;
    PIPE_CHECK pipe_chk;

    DMA_LSCI lsci;
    lsci.m_pIspDrv = this->m_pIspDrv;

    //
    SecMgr* pSec = SecMgr::SecMgr_GetMgrObj();

    this->m_pDrv = (ISP_DRV_CAM*)this->m_pIspDrv;
    if(this->m_pDrv == NULL){
        CAM_FUNC_ERR("Null m_pDrv\n");
        goto EXIT;
    }

    //dbg log
    this->m_pDrv->getCurObjInfo(&this->m_hwModule,&cq,&cqpage);


    if(UpdatebyCB == MTRUE){
        showMagic = CBmagic;
    } else {
        showMagic = *this->m_Magic;
    }
    CAM_FUNC_DBG("update+ cq:0x%x,cqpage:0x%x,SenId:0x%x,Mag:%d\n",
                  cq, cqpage, this->m_SenDev, showMagic);


    if(((TuningDrv*)this->m_pTuningDrv)->getIsApplyTuning(TuningDrvUser) == MFALSE){
        CAM_FUNC_INF("tuning_drv is empty, bypass\n");
        goto EXIT;
    }

    vpTuningNodes.clear();
    this->m_UpdateFuncBit = 0;

    if(UpdatebyCB == MTRUE) {
        if(MFALSE == ((TuningDrv*)this->m_pTuningDrv)->deTuningQueByDrv(
            TuningDrvUser, (MINT32*)&CBmagic, vpTuningNodes)){
            CAM_FUNC_ERR("updateByCB de tuning que fail\n");
            ret = 1;
            goto EXIT;
        }
        CAM_FUNC_DBG("updateByCB\n");
    } else {
        if(MFALSE == ((TuningDrv*)this->m_pTuningDrv)->deTuningQueByDrv(
            TuningDrvUser, (MINT32*)this->m_Magic, vpTuningNodes)){
            CAM_FUNC_ERR("update de tuning que fail\n");
            ret = 1;
            goto EXIT;
        }
        CAM_FUNC_DBG("updateByNormalFlow\n");
    }

    subSample = ((TuningDrv*)this->m_pTuningDrv)->getBQNum();

    //tuningnode at non-subsample case, always using node_0
    if(subSample == 1){
        page = 0;
    } else {
        page = page % subSample;
    }

    pReg = (cam_reg_t*)vpTuningNodes[page]->pTuningRegBuf;
    this->m_UpdateFuncBit = vpTuningNodes[page]->eUpdateFuncBit;

    // OBC
    if(vpTuningNodes[page]->ctlEn_CAM & OBC_R1_EN_){

        switch(this->m_hwModule){
            case CAM_A:
            case CAM_B:
                break;
            default:
#if 0
                CAM_FUNC_ERR("hwModule(%d) no support OBC_R1,"
                             "OBC_R1 only support CAM_A/CAM_B only \n",
                              this->m_hwModule);
                ret = 1;
#endif
                goto BYPASS_OBC;
        }

        CAM_WRITE_BITS(this->m_pDrv,CAMCTL_R1_CAMCTL_EN,CAMCTL_OBC_R1_EN,1);
    } else {
        CAM_WRITE_BITS(this->m_pDrv,CAMCTL_R1_CAMCTL_EN,CAMCTL_OBC_R1_EN,0);
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
        CAM_BURST_WRITE_REGS(this->m_pDrv,OBC_R1_OBC_CTL,pReg_st,module_size);

        free(pReg_st);

        CAM_FUNC_DBG("OBC:[0x%x_0x%x_0x%x],[0x%x_0x%x_0x%x] \n",\
                vpTuningNodes[page]->pTuningRegBuf[(module_adr )>>2],\
                vpTuningNodes[page]->pTuningRegBuf[(module_adr + 1*0x4 )>>2],\
                vpTuningNodes[page]->pTuningRegBuf[(module_adr + 2*0x4 )>>2],\
                CAM_READ_REG(this->m_pDrv,OBC_R1_OBC_CTL),\
                CAM_READ_REG(this->m_pDrv,OBC_R1_OBC_DBS),\
                CAM_READ_REG(this->m_pDrv,OBC_R1_OBC_GRAY_BLD_0));

        if(MTRUE == func_cam_DbgLogEnable_DEBUG) {
            dumpTuningRegBuf((void *)vpTuningNodes[page], "OBC_R1", module_size, module_adr);
        }

    } else {
        this->m_pDrv->cqNopModule(CAM_ISP_OBC_);
    }
BYPASS_OBC:

    // BPC : BNR rename to BPC
    if(vpTuningNodes[page]->ctlEn_CAM & BPC_R1_EN_){
        //check input
        if(pReg->BPC_R1_BPC_PDC_CON.Bits.BPC_PDC_EN == MTRUE){
            if(pReg->BPC_R1_BPC_BPC_CON.Bits.BPC_BPC_LUT_EN == MFALSE) {
                CAM_FUNC_ERR("pdc_en & lut_en must be enabled when pdc is enabled(%d_%d)\n",
                        pReg->BPC_R1_BPC_PDC_CON.Bits.BPC_PDC_EN,
                        pReg->BPC_R1_BPC_BPC_CON.Bits.BPC_BPC_LUT_EN);
                goto BYPASS_BPC;
            }
        }

        if(pReg->BPC_R1_BPC_BPC_CON.Bits.BPC_BPC_LUT_EN == MTRUE){
            if(0 == (vpTuningNodes[page]->ctlEnDMA_CAM & BPCI_R1_EN_)){
                CAM_FUNC_ERR("bpci & lut_en must be enabled when bpci/lut is enabled(%d_0x%x)\n",
                        pReg->BPC_R1_BPC_BPC_CON.Bits.BPC_BPC_LUT_EN,vpTuningNodes[page]->ctlEnDMA_CAM);
                goto BYPASS_BPC;
            }

            if(pReg->BPCI_R1_BPCI_BASE_ADDR.Raw == 0){
                CAM_FUNC_ERR("xsize & addr can't be 0 if bpci enabled(0x%x_0x%x)\n",
                        pReg->BPCI_R1_BPCI_BASE_ADDR.Raw,pReg->BPCI_R1_BPCI_XSIZE.Bits.BPCI_XSIZE);
                goto BYPASS_BPC;
            }

            if(pReg->BPCI_R1_BPCI_XSIZE.Bits.BPCI_XSIZE == 0){
                CAM_FUNC_ERR("xsize & addr can't be 0 if bpci enabled(0x%x_0x%x)\n",
                        pReg->BPCI_R1_BPCI_BASE_ADDR.Raw,pReg->BPCI_R1_BPCI_XSIZE.Bits.BPCI_XSIZE);
                goto BYPASS_BPC;
            }
            _bpci_en = MTRUE;
        }

        CAM_WRITE_BITS(this->m_pDrv,CAMCTL_R1_CAMCTL_EN,CAMCTL_BPC_R1_EN,1);
    } else {
        CAM_WRITE_BITS(this->m_pDrv,CAMCTL_R1_CAMCTL_EN,CAMCTL_BPC_R1_EN,0);
    }
    if(vpTuningNodes[page]->eUpdateFuncBit & eIspTuningMgrFunc_BPC){

        this->m_pDrv->cqApbModule(CAM_ISP_BPC_);
        //
        this->m_pDrv->getCQModuleInfo(CAM_ISP_BPC_,module_adr,module_size);
        //
        pReg_st = (ISP_DRV_REG_IO_STRUCT*)malloc(sizeof(ISP_DRV_REG_IO_STRUCT)*module_size);
        //
        for(MUINT32 j=0;j<module_size;j++)
            pReg_st[j].Data = vpTuningNodes[page]->pTuningRegBuf[(module_adr + j*0x4)>>2];
        //
        CAM_BURST_WRITE_REGS(this->m_pDrv,BPC_R1_BPC_BPC_CON,pReg_st,module_size);

        free(pReg_st);

        if (_bpci_en) {
            MUINT32 in_h,in_v;
            REG_TG_R1_TG_SEN_GRAB_PXL TG_W;
            REG_TG_R1_TG_SEN_GRAB_LIN TG_H;

            //for pipechk
            PIPE_CHECK pipe_chk;
            //ion handle: buffer
            ISP_DEV_ION_NODE_STRUCT IonNode;
            IonNode.devNode = this->m_hwModule;
            IonNode.memID = vpTuningNodes[page]->Dmai_bufinfo[eIspTuningMgrFuncBit_BPC].FD;  // BNR rename to BPC
            IonNode.dmaPort = _dma_bpci_;
            this->m_pDrv->setDeviceInfo(_SET_ION_HANDLE,(MUINT8*)&IonNode);
            TG_W.Raw = CAM_READ_REG(this->m_pDrv->getPhyObj(), TG_R1_TG_SEN_GRAB_PXL);
            TG_H.Raw = CAM_READ_REG(this->m_pDrv->getPhyObj(), TG_R1_TG_SEN_GRAB_LIN);

            in_h = TG_W.Bits.TG_PXL_E - TG_W.Bits.TG_PXL_S;
            in_v = TG_H.Bits.TG_LIN_E - TG_H.Bits.TG_LIN_S;

            in_h = in_h >> (CAM_READ_BITS(this->m_pDrv,CAMCTL_R1_CAMCTL_EN2,CAMCTL_BIN_R1_EN) + \
                            CAM_READ_BITS(this->m_pDrv,CAMCTL_R1_CAMCTL_EN2,CAMCTL_DBN_R1_EN) + \
                            CAM_READ_BITS(this->m_pDrv,CAMCTL_R1_CAMCTL_EN2,CAMCTL_FBND_R1_EN));

            in_v = in_v >> (CAM_READ_BITS(this->m_pDrv,CAMCTL_R1_CAMCTL_EN2,CAMCTL_BIN_R1_EN) +
                            CAM_READ_BITS(this->m_pDrv,CAMCTL_R1_CAMCTL_EN2,CAMCTL_FBND_R1_EN));

            CAM_WRITE_BITS(this->m_pDrv, BPC_R1_BPC_BPC_TBLI2, BPC_XSIZE, in_h-1);
            CAM_WRITE_BITS(this->m_pDrv, BPC_R1_BPC_BPC_TBLI2, BPC_YSIZE, in_v-1);
            bpci.m_pIspDrv = this->m_pIspDrv;
            bpci.dma_cfg.memBuf.base_pAddr = pReg->BPCI_R1_BPCI_BASE_ADDR.Raw;
            bpci.dma_cfg.size.xsize = pReg->BPCI_R1_BPCI_XSIZE.Bits.BPCI_XSIZE;
            bpci.dma_cfg.size.h = pReg->BPCI_R1_BPCI_YSIZE.Bits.BPCI_YSIZE;
            bpci.dma_cfg.size.stride = bpci.dma_cfg.size.xsize;
            bpci.write2CQ(0);
            bpci.config();
            bpci.enable(NULL);

            pipe_chk.m_BPCI_VA = vpTuningNodes[page]->Dmai_bufinfo[eIspTuningMgrFuncBit_BPC].VA_addr; // BNR rename to BPC
        } else {
            PIPE_CHECK pipe_chk;
            bpci.m_pIspDrv = this->m_pIspDrv;
            bpci.disable();

            pipe_chk.m_BPCI_VA = 0;
        }

        CAM_FUNC_DBG("BPC:[0x%x_0x%x_0x%x],[0x%x_0x%x_0x%x] \n",\
                vpTuningNodes[page]->pTuningRegBuf[(module_adr )>>2],\
                vpTuningNodes[page]->pTuningRegBuf[(module_adr + 1*0x4 )>>2],\
                vpTuningNodes[page]->pTuningRegBuf[(module_adr + 2*0x4 )>>2],\
                CAM_READ_REG(this->m_pDrv,BPC_R1_BPC_BPC_CON),\
                CAM_READ_REG(this->m_pDrv,BPC_R1_BPC_BPC_BLD),\
                CAM_READ_REG(this->m_pDrv,BPC_R1_BPC_BPC_TH1));

        if(MTRUE == func_cam_DbgLogEnable_DEBUG) {
            dumpTuningRegBuf((void *)vpTuningNodes[page], "BPC_R1", module_size, module_adr);
        }

    } else {
        this->m_pDrv->cqNopModule(CAM_ISP_BPC_);
    }
BYPASS_BPC:

    // PDE
    if(vpTuningNodes[page]->ctlEn2_CAM & PDE_R1_EN_){
        /*Enable BNR before PDO_EN is enabled*/
        MUINT32 pdo_sel = CAM_READ_BITS(this->m_pDrv,CAMCTL_R1_CAMCTL_SEL,CAMCTL_PDO_SEL);
        if (pdo_sel == 1) {
            if(CAM_READ_BITS(this->m_pDrv,CAMCTL_R1_CAMCTL_DMA_EN,CAMCTL_PDO_R1_EN)){
                _pdi_en = MTRUE;
                _pdo_en = MTRUE;
            }
        }
        CAM_WRITE_BITS(this->m_pDrv,CAMCTL_R1_CAMCTL_EN2,CAMCTL_PDE_R1_EN,1);

        Header_PDO  fh_pdo;
        fh_pdo.m_hwModule = this->m_hwModule;
        fh_pdo.Header_Enque(Header_PDO::E_SENSOR_ID, this->m_pDrv, this->m_SenDev);

    } else {
        MUINT32 pdo_sel = CAM_READ_BITS(this->m_pDrv,CAMCTL_R1_CAMCTL_SEL,CAMCTL_PDO_SEL);
        if(CAM_READ_BITS(this->m_pDrv,CAMCTL_R1_CAMCTL_DMA_EN,CAMCTL_PDO_R1_EN) == 1 && pdo_sel == 1){
            CAM_FUNC_ERR("can't disable PDE when PDO_EN is enabled and PDO_SEL = 1\n");
            goto BYPASS_PDE;
        }
        CAM_WRITE_BITS(this->m_pDrv,CAMCTL_R1_CAMCTL_EN2,CAMCTL_PDE_R1_EN,0);
        //bpci disable at vpTuningNodes[page]->eUpdateFuncBit & eIspTuningMgrFunc_BPC
    }
    if(vpTuningNodes[page]->eUpdateFuncBit & eIspTuningMgrFunc_PDE){

        this->m_pDrv->cqApbModule(CAM_ISP_PDE_);
        //
        this->m_pDrv->getCQModuleInfo(CAM_ISP_PDE_,module_adr,module_size);
        //
        pReg_st = (ISP_DRV_REG_IO_STRUCT*)malloc(sizeof(ISP_DRV_REG_IO_STRUCT)*module_size);
        //
        for(MUINT32 j=0;j<module_size;j++)
            pReg_st[j].Data = vpTuningNodes[page]->pTuningRegBuf[(module_adr + j*0x4)>>2];
        //
        CAM_BURST_WRITE_REGS(this->m_pDrv,PDE_R1_PDE_TBLI1,pReg_st,module_size);

        free(pReg_st);

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
            pdi.dma_cfg.memBuf.base_pAddr = pReg->PDI_R1_PDI_BASE_ADDR.Raw;
            pdi.dma_cfg.size.xsize = pReg->PDI_R1_PDI_XSIZE.Bits.PDI_XSIZE;
            pdi.dma_cfg.size.h = pReg->PDI_R1_PDI_YSIZE.Bits.PDI_YSIZE;
            pdi.dma_cfg.size.stride = pReg->PDI_R1_PDI_STRIDE.Bits.PDI_STRIDE;
            pdi.write2CQ(0);
            pdi.config();
            pdi.enable(NULL);

            pipe_chk.m_PDI_VA = vpTuningNodes[page]->Dmai_bufinfo[eIspTuningMgrFuncBit_PDE].VA_addr;
        } else {
            PIPE_CHECK pipe_chk;
            pdi.m_pIspDrv = this->m_pIspDrv;
            pdi.disable();

            pipe_chk.m_PDI_VA = 0;
        }

        if(_pdo_en){
            pdo.m_pIspDrv = this->m_pIspDrv;
            pdo.dma_cfg.size.xsize = pReg->PDO_R1_PDO_XSIZE.Bits.PDO_XSIZE;
            pdo.dma_cfg.size.h = pReg->PDO_R1_PDO_YSIZE.Bits.PDO_YSIZE;
            pdo.dma_cfg.size.stride = PDO_STRIDE_ALIGN(pdo.dma_cfg.size.xsize);
            pdo.config();
        }
        CAM_FUNC_DBG("PDE:[0x%x_0x%x] PDI:[0x%x_0x%x_0x%x],[0x%x_0x%x_0x%x]\n",\
                vpTuningNodes[page]->pTuningRegBuf[(module_adr )>>2],\
                this->m_pDrv->readReg((module_adr ),CAM_A),\
                pReg->PDI_R1_PDI_XSIZE.Raw,\
                pReg->PDI_R1_PDI_YSIZE.Raw,\
                pReg->PDI_R1_PDI_STRIDE.Raw,\
                CAM_READ_REG(this->m_pDrv,PDI_R1_PDI_XSIZE),\
                CAM_READ_REG(this->m_pDrv,PDI_R1_PDI_YSIZE),\
                CAM_READ_REG(this->m_pDrv,PDI_R1_PDI_STRIDE));

        if(MTRUE == func_cam_DbgLogEnable_DEBUG) {
            dumpTuningRegBuf((void *)vpTuningNodes[page], "PDE_R1", module_size, module_adr);
        }

    } else {
        this->m_pDrv->cqNopModule(CAM_ISP_PDE_);
    }
BYPASS_PDE:

    // PBN
    _pdo_en = MFALSE;
    if(vpTuningNodes[page]->ctlEn2_CAM & PBN_R1_EN_){
        switch(this->m_hwModule){
            case CAM_A:
            case CAM_B:
                break;
            default:
#if 0
                CAM_FUNC_ERR("hwModule(%d) no support PBN_R1,"
                             "PBN_R1 only support CAM_A/CAM_B only \n",
                              this->m_hwModule);
                ret = 1;
#endif
                goto BYPASS_PBN;
        }

        if (CAM_READ_BITS(this->m_pDrv,CAMCTL_R1_CAMCTL_EN2,CAMCTL_PBN_R1_EN) != 1){
            CAM_FUNC_ERR("can't set PBN tuning when PBN_EN=0\n");
            goto BYPASS_PBN;
        }
        if ((CAM_READ_REG(this->m_pDrv,CAMCTL_R1_CAMCTL_SEL) & PDO_SEL_1 ) != 0){
            CAM_FUNC_ERR("can't enable PBN but PDO_SEL=1\n");
            goto BYPASS_PBN;
        }
        //PBN_EN at twin_drv
    } else {
        if (CAM_READ_BITS(this->m_pDrv,CAMCTL_R1_CAMCTL_EN2,CAMCTL_PBN_R1_EN)) {
            CAM_FUNC_ERR("can't disable PBN by tuning\n");
            goto BYPASS_PBN;
        }
    }

    if(vpTuningNodes[page]->eUpdateFuncBit & eIspTuningMgrFunc_PBN){
        DMA_PDO pdo;

        _pdo_en = CAM_READ_BITS(this->m_pDrv,CAMCTL_R1_CAMCTL_DMA_EN,CAMCTL_PDO_R1_EN);

        /* PBN_EN will set by twin driver */
        this->m_pDrv->cqApbModule(CAM_ISP_PBN_);
        //
        this->m_pDrv->getCQModuleInfo(CAM_ISP_PBN_,module_adr,module_size);
        //
        pReg_st = (ISP_DRV_REG_IO_STRUCT*)malloc(sizeof(ISP_DRV_REG_IO_STRUCT)*module_size);
        //
        REG_TG_R1_TG_SEN_GRAB_LIN TG_H;
        TG_H.Raw = CAM_READ_REG(this->m_pDrv->getPhyObj(),TG_R1_TG_SEN_GRAB_LIN);
        pReg->PBN_R1_PBN_PBN_VSIZE.Bits.PBN_PBN_VSIZE = TG_H.Bits.TG_LIN_E - TG_H.Bits.TG_LIN_S;
        //
        for(MUINT32 j=0;j<module_size;j++)
            pReg_st[j].Data = vpTuningNodes[page]->pTuningRegBuf[(module_adr + j*0x4)>>2];
        //
        CAM_BURST_WRITE_REGS(this->m_pDrv,PBN_R1_PBN_PBN_TYPE,pReg_st,module_size);
        free(pReg_st);
        //
        if(_pdo_en){
            pdo.m_pIspDrv = this->m_pIspDrv;
            pdo.dma_cfg.size.xsize = pReg->PDO_R1_PDO_XSIZE.Bits.PDO_XSIZE;
            pdo.dma_cfg.size.h = pReg->PDO_R1_PDO_YSIZE.Bits.PDO_YSIZE;
            pdo.dma_cfg.size.stride = PDO_STRIDE_ALIGN(pdo.dma_cfg.size.xsize);
            pdo.config();
        }

        CAM_FUNC_DBG("PBN:[0x%x_0x%x_0x%x],[0x%x_0x%x_0x%x]\n",\
            vpTuningNodes[page]->pTuningRegBuf[(module_adr )>>2],\
            vpTuningNodes[page]->pTuningRegBuf[(module_adr + 1*0x4 )>>2],\
            vpTuningNodes[page]->pTuningRegBuf[(module_adr + 2*0x4 )>>2],\
            CAM_READ_REG(this->m_pDrv,PBN_R1_PBN_PBN_TYPE),\
            CAM_READ_REG(this->m_pDrv,PBN_R1_PBN_PBN_LST),\
            CAM_READ_REG(this->m_pDrv,PBN_R1_PBN_PBN_VSIZE));

        if(MTRUE == func_cam_DbgLogEnable_DEBUG) {
            dumpTuningRegBuf((void *)vpTuningNodes[page], "PBN_R1", module_size, module_adr);
        }

    } else {
        this->m_pDrv->cqNopModule(CAM_ISP_PBN_);
    }
BYPASS_PBN:

    // LSC
    if(vpTuningNodes[page]->ctlEn_CAM & LSC_R1_EN_) {
        if((vpTuningNodes[page]->ctlEnDMA_CAM & LSCI_R1_EN_) == 0){
            CAM_FUNC_ERR("LSC & LSCI must be enabled at the same time\n");
            this->m_pDrv->cqNopModule(CAM_ISP_LSC_);
            goto BYPASS_LSC;
        }
        CAM_WRITE_BITS(this->m_pDrv,CAMCTL_R1_CAMCTL_EN,CAMCTL_LSC_R1_EN,1);
        lsci.enable(NULL);
    } else {
        CAM_WRITE_BITS(this->m_pDrv,CAMCTL_R1_CAMCTL_EN,CAMCTL_LSC_R1_EN,0);
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
        CAM_BURST_WRITE_REGS(this->m_pDrv,LSC_R1_LSC_CTL1,pReg_st,module_size);

        free(pReg_st);

        //ion handle: buffer
        ISP_DEV_ION_NODE_STRUCT IonNode;
        IonNode.devNode = this->m_hwModule;
        IonNode.memID = vpTuningNodes[page]->Dmai_bufinfo[eIspTuningMgrFuncBit_LSC].FD;
        IonNode.dmaPort = _dma_lsci_;
        this->m_pDrv->setDeviceInfo(_SET_ION_HANDLE,(MUINT8*)&IonNode);

        lsci.dma_cfg.memBuf.base_pAddr = pReg->LSCI_R1_LSCI_BASE_ADDR.Raw;
        lsci.dma_cfg.size.xsize = pReg->LSCI_R1_LSCI_XSIZE.Bits.LSCI_XSIZE;
        lsci.dma_cfg.size.h = pReg->LSCI_R1_LSCI_YSIZE.Bits.LSCI_YSIZE;
        lsci.dma_cfg.size.stride = lsci.dma_cfg.size.xsize;
        lsci.write2CQ(0);
        lsci.config();

        CAM_FUNC_DBG("LSC:[0x%x_0x%x_0x%x],[0x%x_0x%x_0x%x] \n",\
                vpTuningNodes[page]->pTuningRegBuf[(module_adr )>>2],\
                vpTuningNodes[page]->pTuningRegBuf[(module_adr + 1*0x4 )>>2],\
                vpTuningNodes[page]->pTuningRegBuf[(module_adr + 2*0x4 )>>2],\
                CAM_READ_REG(this->m_pDrv,LSC_R1_LSC_CTL1),\
                CAM_READ_REG(this->m_pDrv,LSC_R1_LSC_CTL2),\
                CAM_READ_REG(this->m_pDrv,LSC_R1_LSC_CTL3));

        if(MTRUE == func_cam_DbgLogEnable_DEBUG) {
            dumpTuningRegBuf((void *)vpTuningNodes[page], "LSC_R1", module_size, module_adr);
        }

    } else {
        this->m_pDrv->cqNopModule(CAM_ISP_LSC_);
    }
BYPASS_LSC:

    // AWB & AE
    if(vpTuningNodes[page]->ctlEn_CAM & AA_R1_EN_){
        if((vpTuningNodes[page]->ctlEnDMA_CAM & AAO_R1_EN_) == 0){
            CAM_FUNC_ERR("aa & aao must be enabled at the same time\n");
            goto BYPASS_AA;
        }
        CAM_WRITE_BITS(this->m_pDrv,CAMCTL_R1_CAMCTL_EN,CAMCTL_AA_R1_EN,1);

        Header_AAO   fh_aao;
        fh_aao.m_hwModule = this->m_hwModule;
        fh_aao.Header_Enque(Header_AAO::E_SENSOR_ID, this->m_pDrv, this->m_SenDev);

    } else {
        if(1 == CAM_READ_BITS(this->m_pDrv,FBC_R1_FBC_AAO_R1_CTL1,FBC_AAO_R1_FBC_EN)) {
            CAM_FUNC_ERR("AA supported no dynamic func off under continuous mode\n");
            goto BYPASS_AA;
        }
    }

    if(vpTuningNodes[page]->eUpdateFuncBit & eIspTuningMgrFunc_AA){
        DMA_AAO aao;

        this->m_pDrv->cqApbModule(CAM_ISP_AWB_);
        this->m_pDrv->cqApbModule(CAM_ISP_AE_);
        this->m_pDrv->cqApbModule(CAM_ISP_AE2_);

        //awb
        this->m_pDrv->getCQModuleInfo(CAM_ISP_AWB_,module_adr,module_size);
        //
        pReg_st = (ISP_DRV_REG_IO_STRUCT*)malloc(sizeof(ISP_DRV_REG_IO_STRUCT)*module_size);
        //
        for(MUINT32 j=0;j<module_size;j++)
            pReg_st[j].Data = vpTuningNodes[page]->pTuningRegBuf[(module_adr + j*0x4)>>2];
        //
        CAM_BURST_WRITE_REGS(this->m_pDrv,AA_R1_AA_AWB_WIN_ORG,pReg_st,module_size);

        if(MTRUE == func_cam_DbgLogEnable_DEBUG) {
            dumpTuningRegBuf((void *)vpTuningNodes[page], "AA_R1_AA_AWB_WIN_ORG", module_size, module_adr);
        }

        free(pReg_st);

        //ae
        this->m_pDrv->getCQModuleInfo(CAM_ISP_AE_,module_adr,module_size);
        //
        pReg_st = (ISP_DRV_REG_IO_STRUCT*)malloc(sizeof(ISP_DRV_REG_IO_STRUCT)*module_size);
        //
        for(MUINT32 j=0;j<module_size;j++)
            pReg_st[j].Data = vpTuningNodes[page]->pTuningRegBuf[(module_adr + j*0x4)>>2];
        //
        CAM_BURST_WRITE_REGS(this->m_pDrv,AA_R1_AA_AE_GAIN2_0,pReg_st,module_size);

        if(MTRUE == func_cam_DbgLogEnable_DEBUG) {
            dumpTuningRegBuf((void *)vpTuningNodes[page], "AA_R1_AA_AE_GAIN2_0", module_size, module_adr);
        }

        free(pReg_st);

        //ae2
        this->m_pDrv->getCQModuleInfo(CAM_ISP_AE2_,module_adr,module_size);
        //
        pReg_st = (ISP_DRV_REG_IO_STRUCT*)malloc(sizeof(ISP_DRV_REG_IO_STRUCT)*module_size);
        //
        for(MUINT32 j=0;j<module_size;j++)
            pReg_st[j].Data = vpTuningNodes[page]->pTuningRegBuf[(module_adr + j*0x4)>>2];
        //
        CAM_BURST_WRITE_REGS(this->m_pDrv,AA_R1_AA_AE_SPARE, pReg_st,module_size);

        if(MTRUE == func_cam_DbgLogEnable_DEBUG) {
            dumpTuningRegBuf((void *)vpTuningNodes[page], "AA_R1_AA_AE_SPARE", module_size, module_adr);
        }

        free(pReg_st);

        //aao , ref to the config() at statistic pipe
        aao.m_pIspDrv = this->m_pIspDrv;
        aao.dma_cfg.size.xsize = pReg->AAO_R1_AAO_XSIZE.Bits.AAO_XSIZE;
        aao.dma_cfg.size.h = pReg->AAO_R1_AAO_YSIZE.Bits.AAO_YSIZE;
        aao.dma_cfg.size.stride = aao.dma_cfg.size.xsize;
        aao.config();

        CAM_FUNC_DBG("AA:[0x%x_0x%x_0x%x],[0x%x_0x%x_0x%x] \n",\
                vpTuningNodes[page]->pTuningRegBuf[(module_adr )>>2],\
                vpTuningNodes[page]->pTuningRegBuf[(module_adr + 1*0x4 )>>2],\
                vpTuningNodes[page]->pTuningRegBuf[(module_adr + 2*0x4 )>>2],\
                CAM_READ_REG(this->m_pDrv,AA_R1_AA_AWB_WIN_ORG),\
                CAM_READ_REG(this->m_pDrv,AA_R1_AA_AWB_WIN_SIZE),\
                CAM_READ_REG(this->m_pDrv,AA_R1_AA_AWB_WIN_PIT));
    } else {
        this->m_pDrv->cqNopModule(CAM_ISP_AWB_);
        this->m_pDrv->cqNopModule(CAM_ISP_AE_);
        this->m_pDrv->cqNopModule(CAM_ISP_AE2_);
    }
BYPASS_AA:

    // AF :SGG1/SGG5 merge into AF
    if(vpTuningNodes[page]->ctlEn2_CAM & AF_R1_EN_){
        capibility CamInfo;
        if(CamInfo.m_DTwin.GetDTwin()){
            CAM_FUNC_ERR("AF should be controlled by CCU under dynamic twin\n");
            goto BYPASS_AF;
        }

        if((vpTuningNodes[page]->ctlEnDMA_CAM & AFO_R1_EN_) == 0){
            CAM_FUNC_ERR("af & afo must be enabled at the same time\n");
            goto BYPASS_AF;
        }
        CAM_WRITE_BITS(this->m_pDrv,CAMCTL_R1_CAMCTL_EN2,CAMCTL_AF_R1_EN,1);

        Header_AFO   fh_afo;
        fh_afo.m_hwModule = this->m_hwModule;
        fh_afo.Header_Enque(Header_AFO::E_SENSOR_ID, this->m_pDrv, this->m_SenDev);

    } else {
        if(1 == CAM_READ_BITS(this->m_pDrv,FBC_R1_FBC_AFO_R1_CTL1,FBC_AFO_R1_FBC_EN)) {
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

        this->m_pDrv->cqApbModule(CAM_ISP_AF_);

        //
        this->m_pDrv->getCQModuleInfo(CAM_ISP_AF_,module_adr,module_size);
        //
        pReg_st = (ISP_DRV_REG_IO_STRUCT*)malloc(sizeof(ISP_DRV_REG_IO_STRUCT)*module_size);
        //
        for(MUINT32 j=0;j<module_size;j++)
            pReg_st[j].Data = vpTuningNodes[page]->pTuningRegBuf[(module_adr + j*0x4)>>2];
        //
        CAM_BURST_WRITE_REGS(this->m_pDrv,AF_R1_AF_CON,pReg_st,module_size);

        free(pReg_st);

        //afo , ref to the config() at statistic pipe
        afo.m_pIspDrv = this->m_pIspDrv;
        afo.dma_cfg.size.xsize = pReg->AFO_R1_AFO_XSIZE.Bits.AFO_XSIZE;
        afo.dma_cfg.size.h = pReg->AFO_R1_AFO_YSIZE.Bits.AFO_YSIZE;

#if 1       //avoid CAM_A or CAM_B'S  afo being disabled by twin drv af touch at the corner of image
        afo.dma_cfg.size.stride = afo.dma_cfg.size.xsize + AFO_DUMMY_STRIDE;
#else
        afo.dma_cfg.size.stride = afo.dma_cfg.size.xsize;
#endif
        afo.config();


        CAM_FUNC_DBG("AF:[0x%x_0x%x_0x%x_0x%x],[0x%x_0x%x_0x%x_0x%x] \n",\
                vpTuningNodes[page]->pTuningRegBuf[(module_adr )>>2],\
                vpTuningNodes[page]->pTuningRegBuf[(module_adr + 1*0x4 )>>2],\
                vpTuningNodes[page]->pTuningRegBuf[(module_adr + 4*0x4 )>>2],\
                vpTuningNodes[page]->pTuningRegBuf[(module_adr + 5*0x4 )>>2],\
                CAM_READ_REG(this->m_pDrv,AF_R1_AF_CON),\
                CAM_READ_REG(this->m_pDrv,AF_R1_AF_CON2),\
                CAM_READ_REG(this->m_pDrv,AF_R1_AF_SIZE),\
                CAM_READ_REG(this->m_pDrv,AF_R1_AF_VLD));

        if(MTRUE == func_cam_DbgLogEnable_DEBUG) {
            dumpTuningRegBuf((void *)vpTuningNodes[page], "AF_R1", module_size, module_adr);
        }

    } else {
        this->m_pDrv->cqNopModule(CAM_ISP_AF_);
    }
BYPASS_AF:

    if(UpdateFLK) {
    // FLK : SGG3 merge into FLK.
    if(vpTuningNodes[page]->ctlEn_CAM & FLK_R1_EN_){

         switch(this->m_hwModule){
            case CAM_A:
            case CAM_B:
                break;
            default:
#if 0
                CAM_FUNC_ERR("hwModule(%d) no support FLK_R1,"
                             "FLK_R1 only support CAM_A/CAM_B only \n",
                              this->m_hwModule);
                ret = 1;
#endif
                goto BYPASS_FLK;
        }

        if((vpTuningNodes[page]->ctlEnDMA_CAM & FLKO_R1_EN_) == 0){
            CAM_FUNC_ERR("FLK & FLKo must be enabled at the same time\n");
            goto BYPASS_FLK;
        }
        CAM_WRITE_BITS(this->m_pDrv,CAMCTL_R1_CAMCTL_EN,CAMCTL_FLK_R1_EN,1);

        Header_FLKO  fh_flko;
        fh_flko.m_hwModule = this->m_hwModule;
        fh_flko.Header_Enque(Header_FLKO::E_SENSOR_ID, this->m_pDrv, this->m_SenDev);
    } else {
#if 0
        if(this->m_pDrv == NULL){
            goto BYPASS_FLK;
        }
#endif
        if (1 == CAM_READ_BITS(this->m_pDrv,FBC_R1_FBC_FLKO_R1_CTL1,FBC_FLKO_R1_FBC_EN)){
            CAM_FUNC_ERR("FLK supported no dynamic func off under continuous mode\n");
            goto BYPASS_FLK;
        }
    }

    if(vpTuningNodes[page]->eUpdateFuncBit & eIspTuningMgrFunc_FLK){
        DMA_FLKO flko;
        cam_reg_t* pReg = (cam_reg_t*)&vpTuningNodes[page]->pTuningRegBuf[0];

        this->m_pDrv->cqApbModule(CAM_ISP_FLK_A_);

        //
        this->m_pDrv->getCQModuleInfo(CAM_ISP_FLK_A_,module_adr,module_size);
        //
        pReg_st = (ISP_DRV_REG_IO_STRUCT*)malloc(sizeof(ISP_DRV_REG_IO_STRUCT)*module_size);
        //
        for(MUINT32 j=0;j<module_size;j++)
            pReg_st[j].Data = vpTuningNodes[page]->pTuningRegBuf[(module_adr + j*0x4)>>2];
        //
        CAM_BURST_WRITE_REGS(this->m_pDrv,FLK_R1_FLK_CON,pReg_st,module_size);

        free(pReg_st);

        //FLKo , ref to the config() at statistic pipe
        flko.m_pIspDrv = this->m_pIspDrv;
        flko.dma_cfg.size.xsize = pReg->FLKO_R1_FLKO_XSIZE.Bits.FLKO_XSIZE;
        flko.dma_cfg.size.h = pReg->FLKO_R1_FLKO_YSIZE.Bits.FLKO_YSIZE;
        flko.dma_cfg.size.stride = flko.dma_cfg.size.xsize;
        flko.config();

        CAM_FUNC_DBG("FLK:[0x%x_0x%x_0x%x],[0x%x_0x%x_0x%x] \n",\
                vpTuningNodes[page]->pTuningRegBuf[module_adr>>2],\
                vpTuningNodes[page]->pTuningRegBuf[(module_adr + 1*0x4)>>2],\
                vpTuningNodes[page]->pTuningRegBuf[(module_adr + 2*0x4)>>2],\
                CAM_READ_REG(this->m_pDrv,FLK_R1_FLK_CON),\
                CAM_READ_REG(this->m_pDrv,FLK_R1_FLK_OFST),\
                CAM_READ_REG(this->m_pDrv,FLK_R1_FLK_SIZE));

        if(MTRUE == func_cam_DbgLogEnable_DEBUG) {
            dumpTuningRegBuf((void *)vpTuningNodes[page], "FLK_R1", module_size, module_adr);
        }

    } else {
        this->m_pDrv->cqNopModule(CAM_ISP_FLK_A_);
    }
    }
BYPASS_FLK:

    // MOBC_R2
    if(vpTuningNodes[page]->ctlEn_CAM & MOBC_R2_EN_) {
        switch(this->m_hwModule){
           case CAM_C:
               break;
           default:
#if 0
               CAM_FUNC_ERR("hwModule(%d) no support MOBC_R2,"
                            " MOBC_R2 only support CAM_C only \n",
                            this->m_hwModule);
               ret = 1;
#endif
               goto BYPASS_MOBC_R2;
       }

       CAM_WRITE_BITS(this->m_pDrv,CAMCTL_R1_CAMCTL_EN,CAMCTL_MOBC_R2_EN,1);
    } else {
        CAM_WRITE_BITS(this->m_pDrv,CAMCTL_R1_CAMCTL_EN,CAMCTL_MOBC_R2_EN,0);
    }
    if(vpTuningNodes[page]->eUpdateFuncBit & eIspTuningMgrFunc_MOBC_R2){

        this->m_pDrv->cqApbModule(CAM_ISP_MOBC_R2_);

        //
        this->m_pDrv->getCQModuleInfo(CAM_ISP_MOBC_R2_,module_adr,module_size);
        //
        pReg_st = (ISP_DRV_REG_IO_STRUCT*)malloc(sizeof(ISP_DRV_REG_IO_STRUCT)*module_size);
        //
        for(MUINT32 j=0;j<module_size;j++)
            pReg_st[j].Data = vpTuningNodes[page]->pTuningRegBuf[(module_adr + j*0x4)>>2];
        //

        CAM_BURST_WRITE_REGS(this->m_pDrv,MOBC_R2_MOBC_OFFST0,pReg_st,module_size);

        free(pReg_st);

        CAM_FUNC_DBG("MOBC_R2:[0x%x_0x%x_0x%x],[0x%x_0x%x_0x%x] \n",\
                vpTuningNodes[page]->pTuningRegBuf[(module_adr )>>2],\
                vpTuningNodes[page]->pTuningRegBuf[(module_adr + 1*0x4 )>>2],\
                vpTuningNodes[page]->pTuningRegBuf[(module_adr + 2*0x4 )>>2],\
                CAM_READ_REG(this->m_pDrv,MOBC_R2_MOBC_OFFST0),\
                CAM_READ_REG(this->m_pDrv,MOBC_R2_MOBC_OFFST1),\
                CAM_READ_REG(this->m_pDrv,MOBC_R2_MOBC_OFFST2));

        if(MTRUE == func_cam_DbgLogEnable_DEBUG) {
            dumpTuningRegBuf((void *)vpTuningNodes[page], "MOBC_R2", module_size, module_adr);
        }

    } else {
        this->m_pDrv->cqNopModule(CAM_ISP_MOBC_R2_);
    }
BYPASS_MOBC_R2:

    // WB
    if(vpTuningNodes[page]->ctlEn_CAM & WB_R1_EN_) {
        CAM_WRITE_BITS(this->m_pDrv,CAMCTL_R1_CAMCTL_EN,CAMCTL_WB_R1_EN,1);
    } else {
        CAM_WRITE_BITS(this->m_pDrv,CAMCTL_R1_CAMCTL_EN,CAMCTL_WB_R1_EN,0);
    }
    if(vpTuningNodes[page]->eUpdateFuncBit & eIspTuningMgrFunc_WB){

        this->m_pDrv->cqApbModule(CAM_ISP_WB_);

        //
        this->m_pDrv->getCQModuleInfo(CAM_ISP_WB_,module_adr,module_size);
        //
        pReg_st = (ISP_DRV_REG_IO_STRUCT*)malloc(sizeof(ISP_DRV_REG_IO_STRUCT)*module_size);
        //
        for(MUINT32 j=0;j<module_size;j++)
            pReg_st[j].Data = vpTuningNodes[page]->pTuningRegBuf[(module_adr + j*0x4)>>2];
        //
        CAM_BURST_WRITE_REGS(this->m_pDrv,WB_R1_WB_GAIN_1,pReg_st,module_size);

        free(pReg_st);

        CAM_FUNC_DBG("WB:[0x%x_0x%x_0x%x],[0x%x_0x%x_0x%x] \n",\
                vpTuningNodes[page]->pTuningRegBuf[(module_adr )>>2],\
                vpTuningNodes[page]->pTuningRegBuf[(module_adr + 1*0x4 )>>2],\
                vpTuningNodes[page]->pTuningRegBuf[(module_adr + 2*0x4 )>>2],\
                CAM_READ_REG(this->m_pDrv,WB_R1_WB_GAIN_1),\
                CAM_READ_REG(this->m_pDrv,WB_R1_WB_GAIN_2),\
                CAM_READ_REG(this->m_pDrv,WB_R1_WB_HLC));

        if(MTRUE == func_cam_DbgLogEnable_DEBUG) {
            dumpTuningRegBuf((void *)vpTuningNodes[page], "WB_R1", module_size, module_adr);
        }

    } else {
        this->m_pDrv->cqNopModule(CAM_ISP_WB_);
    }
//BYPASS_WB:

    // DGN
    if(vpTuningNodes[page]->ctlEn_CAM & DGN_R1_EN_) {
        CAM_WRITE_BITS(this->m_pDrv,CAMCTL_R1_CAMCTL_EN,CAMCTL_DGN_R1_EN,1);
    } else {
        CAM_WRITE_BITS(this->m_pDrv,CAMCTL_R1_CAMCTL_EN,CAMCTL_DGN_R1_EN,0);
    }
    if(vpTuningNodes[page]->eUpdateFuncBit & eIspTuningMgrFunc_DGN){

        this->m_pDrv->cqApbModule(CAM_ISP_DGN_);

        //
        this->m_pDrv->getCQModuleInfo(CAM_ISP_DGN_,module_adr,module_size);
        //
        pReg_st = (ISP_DRV_REG_IO_STRUCT*)malloc(sizeof(ISP_DRV_REG_IO_STRUCT)*module_size);
        //
        for(MUINT32 j=0;j<module_size;j++)
            pReg_st[j].Data = vpTuningNodes[page]->pTuningRegBuf[(module_adr + j*0x4)>>2];
        //
        CAM_BURST_WRITE_REGS(this->m_pDrv,DGN_R1_DGN_GN0,pReg_st,module_size);

        free(pReg_st);

        CAM_FUNC_DBG("DGN:[0x%x_0x%x_0x%x],[0x%x_0x%x_0x%x] \n",\
                vpTuningNodes[page]->pTuningRegBuf[(module_adr )>>2],\
                vpTuningNodes[page]->pTuningRegBuf[(module_adr + 1*0x4 )>>2],\
                vpTuningNodes[page]->pTuningRegBuf[(module_adr + 2*0x4 )>>2],\
                CAM_READ_REG(this->m_pDrv,DGN_R1_DGN_GN0),\
                CAM_READ_REG(this->m_pDrv,DGN_R1_DGN_GN1),\
                CAM_READ_REG(this->m_pDrv,DGN_R1_DGN_GN2));

        if(MTRUE == func_cam_DbgLogEnable_DEBUG) {
            dumpTuningRegBuf((void *)vpTuningNodes[page], "DGN_R1", module_size, module_adr);
        }

    } else {
        this->m_pDrv->cqNopModule(CAM_ISP_DGN_);
    }
//BYPASS_DGN:

    // TSFS
    if(vpTuningNodes[page]->ctlEn2_CAM & TSFS_R1_EN_){

        if((vpTuningNodes[page]->ctlEnDMA_CAM & TSFSO_R1_EN_) == 0){
            CAM_FUNC_ERR("TSFS & TSFSO must be enabled at the same time\n");
            goto BYPASS_TSFS;
        }
        CAM_WRITE_BITS(this->m_pDrv,CAMCTL_R1_CAMCTL_EN2,CAMCTL_TSFS_R1_EN,1);

        Header_TSFSO  fh_tsfso;
        fh_tsfso.m_hwModule = this->m_hwModule;
        fh_tsfso.Header_Enque(Header_TSFSO::E_SENSOR_ID, this->m_pDrv, this->m_SenDev);

    } else {
        if (1 == CAM_READ_BITS(this->m_pDrv,FBC_R1_FBC_TSFSO_R1_CTL1,FBC_TSFSO_R1_FBC_EN)){
            CAM_FUNC_ERR("TSFS supported no dynamic func off under continuous mode\n");
            goto BYPASS_TSFS;
        }
    }

    if(vpTuningNodes[page]->eUpdateFuncBit & eIspTuningMgrFunc_TSFS){

        DMA_TSFSO tsfso;

        cam_reg_t* pReg = (cam_reg_t*)&vpTuningNodes[page]->pTuningRegBuf[0];

        this->m_pDrv->cqApbModule(CAM_ISP_TSFS_);

        //
        this->m_pDrv->getCQModuleInfo(CAM_ISP_TSFS_,module_adr,module_size);
        //
        pReg_st = (ISP_DRV_REG_IO_STRUCT*)malloc(sizeof(ISP_DRV_REG_IO_STRUCT)*module_size);
        //
        for(MUINT32 j=0;j<module_size;j++)
            pReg_st[j].Data = vpTuningNodes[page]->pTuningRegBuf[(module_adr + j*0x4)>>2];
        //
        CAM_BURST_WRITE_REGS(this->m_pDrv,TSFS_R1_TSFS_ORG,pReg_st,module_size);

        free(pReg_st);

        //TSFS , ref to the config() at statistic pipe
        tsfso.m_pIspDrv = this->m_pIspDrv;
        tsfso.dma_cfg.size.xsize = pReg->TSFSO_R1_TSFSO_XSIZE.Bits.TSFSO_XSIZE;
        tsfso.dma_cfg.size.h = pReg->TSFSO_R1_TSFSO_YSIZE.Bits.TSFSO_YSIZE;
        tsfso.dma_cfg.size.stride = tsfso.dma_cfg.size.xsize;
        tsfso.config();

        CAM_FUNC_DBG("TSFS:[0x%x_0x%x_0x%x],[0x%x_0x%x_0x%x] \n",\
                vpTuningNodes[page]->pTuningRegBuf[module_adr>>2],\
                vpTuningNodes[page]->pTuningRegBuf[(module_adr + 1*0x4)>>2],\
                vpTuningNodes[page]->pTuningRegBuf[(module_adr + 2*0x4)>>2],\
                CAM_READ_REG(this->m_pDrv,TSFS_R1_TSFS_ORG),\
                CAM_READ_REG(this->m_pDrv,TSFS_R1_TSFS_SIZE),\
                CAM_READ_REG(this->m_pDrv,TSFS_R1_TSFS_PIT));

        if(MTRUE == func_cam_DbgLogEnable_DEBUG) {
            dumpTuningRegBuf((void *)vpTuningNodes[page], "TSFS_R1", module_size, module_adr);
        }

    } else {
        this->m_pDrv->cqNopModule(CAM_ISP_TSFS_);
    }
BYPASS_TSFS:

    //DM_R1
    if(vpTuningNodes[page]->ctlEn2_CAM & DM_R1_EN_){

        switch(this->m_hwModule){
           case CAM_A:
           case CAM_B:
               break;
           default:
#if 0
               CAM_FUNC_ERR("hwModule(%d) no support DM_R1,"
                            "DM_R1 only support CAM_A/CAM_B only \n",
                            this->m_hwModule);
               ret = 1;
#endif
               goto BYPASS_DM_R1;
        }

        CAM_WRITE_BITS(this->m_pDrv, CAMCTL_R1_CAMCTL_EN2, CAMCTL_DM_R1_EN, 1);
    } else {
        CAM_WRITE_BITS(this->m_pDrv, CAMCTL_R1_CAMCTL_EN2, CAMCTL_DM_R1_EN, 0);
    }

    if(vpTuningNodes[page]->eUpdateFuncBit & eIspTuningMgrFunc_DM){

        this->m_pDrv->cqApbModule(CAM_ISP_DM_);

        /*Get DM Module Info*/
        this->m_pDrv->getCQModuleInfo(CAM_ISP_DM_,module_adr,module_size);
        /*Allocat a register space*/
        pReg_st = (ISP_DRV_REG_IO_STRUCT*)malloc(sizeof(ISP_DRV_REG_IO_STRUCT)*module_size);
        /*Set all the registers to the register space*/
        for(MUINT32 j=0;j<module_size;j++)
            pReg_st[j].Data = vpTuningNodes[page]->pTuningRegBuf[(module_adr + j*0x4)>>2];
        /*final update to the CQ virtual register space*/
        CAM_BURST_WRITE_REGS(this->m_pDrv,DM_R1_DM_INTP_CRS,pReg_st,module_size);

        free(pReg_st);

        CAM_WRITE_BITS(this->m_pDrv, DM_R1_DM_TILE_EDGE, DM_TILE_EDGE, 0XF);//temp workaround

        CAM_FUNC_DBG("DM_R1:[0x%x_0x%x_0x%x],[0x%x_0x%x_0x%x] \n",\
                vpTuningNodes[page]->pTuningRegBuf[(module_adr )>>2],\
                vpTuningNodes[page]->pTuningRegBuf[(module_adr + 1*0x4 )>>2],\
                vpTuningNodes[page]->pTuningRegBuf[(module_adr + 2*0x4 )>>2],\
                CAM_READ_REG(this->m_pDrv,DM_R1_DM_INTP_CRS),\
                CAM_READ_REG(this->m_pDrv,DM_R1_DM_INTP_NAT),\
                CAM_READ_REG(this->m_pDrv,DM_R1_DM_INTP_AUG));

        if(MTRUE == func_cam_DbgLogEnable_DEBUG) {
            dumpTuningRegBuf((void *)vpTuningNodes[page], "DM_R1", module_size, module_adr);
        }

    } else {
        this->m_pDrv->cqNopModule(CAM_ISP_DM_);
    }
BYPASS_DM_R1:

    //FLC_R1
    if(vpTuningNodes[page]->ctlEn3_CAM & FLC_R1_EN_){
        CAM_WRITE_BITS(this->m_pDrv, CAMCTL_R1_CAMCTL_EN3, CAMCTL_FLC_R1_EN, 1);
    } else {
        CAM_WRITE_BITS(this->m_pDrv, CAMCTL_R1_CAMCTL_EN3, CAMCTL_FLC_R1_EN, 0);
    }

    if(vpTuningNodes[page]->eUpdateFuncBit & eIspTuningMgrFunc_FLC){

        this->m_pDrv->cqApbModule(CAM_ISP_FLC_);

        /*Get FLC Module Info*/
        this->m_pDrv->getCQModuleInfo(CAM_ISP_FLC_,module_adr,module_size);
        /*Allocat a register space*/
        pReg_st = (ISP_DRV_REG_IO_STRUCT*)malloc(sizeof(ISP_DRV_REG_IO_STRUCT)*module_size);
        /*Set all the registers to the register space*/
        for(MUINT32 j=0;j<module_size;j++)
            pReg_st[j].Data = vpTuningNodes[page]->pTuningRegBuf[(module_adr + j*0x4)>>2];
        /*final update to the CQ virtual register space*/
        CAM_BURST_WRITE_REGS(this->m_pDrv,FLC_R1_FLC_OFST_RB,pReg_st,module_size);

        free(pReg_st);

        CAM_FUNC_DBG("FLC_R1:[0x%x_0x%x_0x%x],[0x%x_0x%x_0x%x] \n",\
                vpTuningNodes[page]->pTuningRegBuf[(module_adr )>>2],\
                vpTuningNodes[page]->pTuningRegBuf[(module_adr + 1*0x4 )>>2],\
                vpTuningNodes[page]->pTuningRegBuf[(module_adr + 2*0x4 )>>2],\
                CAM_READ_REG(this->m_pDrv,FLC_R1_FLC_OFST_RB),\
                CAM_READ_REG(this->m_pDrv,FLC_R1_FLC_OFST_G),\
                CAM_READ_REG(this->m_pDrv,FLC_R1_FLC_GN_RB));

        if(MTRUE == func_cam_DbgLogEnable_DEBUG) {
            dumpTuningRegBuf((void *)vpTuningNodes[page], "FLC_R1", module_size, module_adr);
        }

    } else {
        this->m_pDrv->cqNopModule(CAM_ISP_FLC_);
    }
//BYPASS_FLC_R1:


    //CCM_R1
    if(vpTuningNodes[page]->ctlEn2_CAM & CCM_R1_EN_){
        switch(this->m_hwModule){
           case CAM_A:
           case CAM_B:
               break;
           default:
#if 0
               CAM_FUNC_ERR("hwModule(%d) no support CCM_R1,"
                            "CCM_R1 only support CAM_A/CAM_B only \n",
                            this->m_hwModule);
               ret = 1;
#endif
               goto BYPASS_CCM_R1;
        }

        // CCM_R1_CCM_CTRL:CCM_IN_FMT must = 0 ,P1 only support 0
        const int CCM_R1_CCM_CTRL_INX = 6;
        const int CCM_IN_FMT_POSITION = 1L << 7;

        if((vpTuningNodes[page]->pTuningRegBuf[(module_adr + CCM_R1_CCM_CTRL_INX*0x4)>>2] &
            CCM_IN_FMT_POSITION) != 0){

            CAM_FUNC_ERR("CM_R1_CCM update Tuning Failed "
                         "CM_R1_CCM_CTRL:CCM_IN_FMT must = 0 P1 only support 0");
            goto BYPASS_CCM_R1;
        }
        CAM_WRITE_BITS(this->m_pDrv, CAMCTL_R1_CAMCTL_EN2, CAMCTL_CCM_R1_EN, 1);
    } else {
        CAM_WRITE_BITS(this->m_pDrv, CAMCTL_R1_CAMCTL_EN2, CAMCTL_CCM_R1_EN, 0);
    }

    if(vpTuningNodes[page]->eUpdateFuncBit & eIspTuningMgrFunc_CCM){

        this->m_pDrv->cqApbModule(CAM_ISP_CCM_);

        /*Get CCM Module Info*/
        this->m_pDrv->getCQModuleInfo(CAM_ISP_CCM_,module_adr,module_size);
        /*Allocat a register space*/
        pReg_st = (ISP_DRV_REG_IO_STRUCT*)malloc(sizeof(ISP_DRV_REG_IO_STRUCT)*module_size);
        /*Set all the registers to the register space*/
        for(MUINT32 j=0;j<module_size;j++)
            pReg_st[j].Data = vpTuningNodes[page]->pTuningRegBuf[(module_adr + j*0x4)>>2];
        /*final update to the CQ virtual register space*/
        CAM_BURST_WRITE_REGS(this->m_pDrv,CCM_R1_CCM_CNV_1,pReg_st,module_size);

        free(pReg_st);

        CAM_FUNC_DBG("CCM_R1:[0x%x_0x%x_0x%x],[0x%x_0x%x_0x%x] \n",\
                vpTuningNodes[page]->pTuningRegBuf[(module_adr )>>2],\
                vpTuningNodes[page]->pTuningRegBuf[(module_adr + 1*0x4 )>>2],\
                vpTuningNodes[page]->pTuningRegBuf[(module_adr + 2*0x4 )>>2],\
                CAM_READ_REG(this->m_pDrv,CCM_R1_CCM_CNV_1),\
                CAM_READ_REG(this->m_pDrv,CCM_R1_CCM_CNV_2),\
                CAM_READ_REG(this->m_pDrv,CCM_R1_CCM_CNV_3));

        if(MTRUE == func_cam_DbgLogEnable_DEBUG) {
            dumpTuningRegBuf((void *)vpTuningNodes[page], "CCM_R1", module_size, module_adr);
        }

    } else {
        this->m_pDrv->cqNopModule(CAM_ISP_CCM_);
    }
BYPASS_CCM_R1:

    //GGM_R1
    if(vpTuningNodes[page]->ctlEn2_CAM & GGM_R1_EN_){
        switch(this->m_hwModule){
           case CAM_A:
           case CAM_B:
               break;
           default:
#if 0
               CAM_FUNC_ERR("hwModule(%d) no support GGM_R1,"
                            "GGM_R1 only support CAM_A/CAM_B only \n",
                            this->m_hwModule);
               ret = 1;
#endif
               goto BYPASS_GGM_R1;
        }
        CAM_WRITE_BITS(this->m_pDrv, CAMCTL_R1_CAMCTL_EN2, CAMCTL_GGM_R1_EN, 1);
    } else {
        CAM_WRITE_BITS(this->m_pDrv, CAMCTL_R1_CAMCTL_EN2, CAMCTL_GGM_R1_EN, 0);
    }

    if(vpTuningNodes[page]->eUpdateFuncBit & eIspTuningMgrFunc_GGM){

        this->m_pDrv->cqApbModule(CAM_ISP_GGM_);
        this->m_pDrv->cqApbModule(CAM_ISP_GGM_PINGPONG_);

        /*Get GGM Module Info*/
        this->m_pDrv->getCQModuleInfo(CAM_ISP_GGM_,module_adr,module_size);
        /*Allocat a register space*/
        pReg_st = (ISP_DRV_REG_IO_STRUCT*)malloc(sizeof(ISP_DRV_REG_IO_STRUCT)*module_size);
        /*Set all the registers to the register space*/
        for(MUINT32 j=0;j<module_size;j++)
            pReg_st[j].Data = vpTuningNodes[page]->pTuningRegBuf[(module_adr + j*0x4)>>2];
        /*final update to the CQ virtual register space*/
        CAM_BURST_WRITE_REGS(this->m_pDrv,GGM_R1_GGM_LUT,pReg_st,module_size);

        free(pReg_st);

        /* PINGPONG switch sram when updated. */
        CAM_WRITE_REG(this->m_pDrv, GGM_R1_GGM_SRAM_PINGPONG, 1);
        CAM_FUNC_DBG("After tuningMgr set GGM_R1 PingPong\n");

        CAM_FUNC_DBG("GGM_R1:[0x%x_0x%x_0x%x],[0x%x_0x%x_0x%x] \n",\
                vpTuningNodes[page]->pTuningRegBuf[(module_adr )>>2],\
                vpTuningNodes[page]->pTuningRegBuf[(module_adr + 192*0x4 )>>2],\
                vpTuningNodes[page]->pTuningRegBuf[(module_adr + 193*0x4 )>>2],\
                CAM_READ_REG(this->m_pDrv,GGM_R1_GGM_LUT),\
                CAM_READ_REG(this->m_pDrv,GGM_R1_GGM_CTRL),\
                CAM_READ_REG(this->m_pDrv,GGM_R1_GGM_SRAM_PINGPONG));

        if(MTRUE == func_cam_DbgLogEnable_DEBUG) {
            dumpTuningRegBuf((void *)vpTuningNodes[page], "GGM_R1", module_size, module_adr);
        }

    } else {
        this->m_pDrv->cqNopModule(CAM_ISP_GGM_);
        this->m_pDrv->cqNopModule(CAM_ISP_GGM_PINGPONG_);
    }
BYPASS_GGM_R1:

    //GGM_R2
    if(vpTuningNodes[page]->ctlEn3_CAM & GGM_R2_EN_){
        switch(this->m_hwModule){
           case CAM_A:
           case CAM_B:
               break;
           default:
#if 0
               CAM_FUNC_ERR("hwModule(%d) no support GGM_R2,"
                            "GGM_R2 only support CAM_A/CAM_B only \n",
                            this->m_hwModule);
               ret = 1;
#endif
               goto BYPASS_GGM_R2;
        }
        CAM_WRITE_BITS(this->m_pDrv, CAMCTL_R1_CAMCTL_EN3, CAMCTL_GGM_R2_EN, 1);
    } else {
        CAM_WRITE_BITS(this->m_pDrv, CAMCTL_R1_CAMCTL_EN3, CAMCTL_GGM_R2_EN, 0);
    }

    if(vpTuningNodes[page]->eUpdateFuncBit & eIspTuningMgrFunc_GGM_R2){

        this->m_pDrv->cqApbModule(CAM_ISP_GGM_R2_);
        this->m_pDrv->cqApbModule(CAM_ISP_GGM_R2_PINGPONG_);

        /*Get GGM_R2 Module Info*/
        this->m_pDrv->getCQModuleInfo(CAM_ISP_GGM_R2_,module_adr,module_size);
        /*Allocat a register space*/
        pReg_st = (ISP_DRV_REG_IO_STRUCT*)malloc(sizeof(ISP_DRV_REG_IO_STRUCT)*module_size);
        /*Set all the registers to the register space*/
        for(MUINT32 j=0;j<module_size;j++)
            pReg_st[j].Data = vpTuningNodes[page]->pTuningRegBuf[(module_adr + j*0x4)>>2];
        /*final update to the CQ virtual register space*/
        CAM_BURST_WRITE_REGS(this->m_pDrv,GGM_R2_GGM_LUT,pReg_st,module_size);

        free(pReg_st);

        /* PINGPONG switch sram when updated. */
        CAM_WRITE_REG(this->m_pDrv, GGM_R2_GGM_SRAM_PINGPONG, 1);
        CAM_FUNC_DBG("After tuningMgr set GGM_R2 PingPong\n");

        CAM_FUNC_DBG("GGM_R2:[0x%x_0x%x_0x%x],[0x%x_0x%x_0x%x] \n",\
                vpTuningNodes[page]->pTuningRegBuf[(module_adr )>>2],\
                vpTuningNodes[page]->pTuningRegBuf[(module_adr + 192*0x4 )>>2],\
                vpTuningNodes[page]->pTuningRegBuf[(module_adr + 193*0x4 )>>2],\
                CAM_READ_REG(this->m_pDrv,GGM_R2_GGM_LUT),\
                CAM_READ_REG(this->m_pDrv,GGM_R2_GGM_CTRL),\
                CAM_READ_REG(this->m_pDrv,GGM_R2_GGM_SRAM_PINGPONG));

        if(MTRUE == func_cam_DbgLogEnable_DEBUG) {
            dumpTuningRegBuf((void *)vpTuningNodes[page], "GGM_R2", module_size, module_adr);
        }

    } else {
        this->m_pDrv->cqNopModule(CAM_ISP_GGM_R2_);
        this->m_pDrv->cqNopModule(CAM_ISP_GGM_R2_PINGPONG_);
    }
BYPASS_GGM_R2:


    //G2C_R1
    if(vpTuningNodes[page]->ctlEn2_CAM & G2C_R1_EN_){
        switch(this->m_hwModule){
           case CAM_A:
           case CAM_B:
               break;
           default:
#if 0
               CAM_FUNC_ERR("hwModule(%d) no support G2C_R1,"
                            "G2C_R1 only support CAM_A/CAM_B only \n",
                            this->m_hwModule);
               ret = 1;
#endif
               goto BYPASS_G2C_R1;
        }
        CAM_WRITE_BITS(this->m_pDrv, CAMCTL_R1_CAMCTL_EN2, CAMCTL_G2C_R1_EN, 1);
    } else {
        CAM_WRITE_BITS(this->m_pDrv, CAMCTL_R1_CAMCTL_EN2, CAMCTL_G2C_R1_EN, 0);
    }

    if(vpTuningNodes[page]->eUpdateFuncBit & eIspTuningMgrFunc_G2C){

        this->m_pDrv->cqApbModule(CAM_ISP_G2C_);

        /*Get G2C Module Info*/
        this->m_pDrv->getCQModuleInfo(CAM_ISP_G2C_,module_adr,module_size);
        /*Allocat a register space*/
        pReg_st = (ISP_DRV_REG_IO_STRUCT*)malloc(sizeof(ISP_DRV_REG_IO_STRUCT)*module_size);
        /*Set all the registers to the register space*/
        for(MUINT32 j=0;j<module_size;j++)
            pReg_st[j].Data = vpTuningNodes[page]->pTuningRegBuf[(module_adr + j*0x4)>>2];
        /*final update to the CQ virtual register space*/
        CAM_BURST_WRITE_REGS(this->m_pDrv,G2C_R1_G2C_CONV_0A,pReg_st,module_size);

        free(pReg_st);

        CAM_FUNC_DBG("G2C_R1:[0x%x_0x%x_0x%x],[0x%x_0x%x_0x%x] \n",\
                vpTuningNodes[page]->pTuningRegBuf[(module_adr )>>2],\
                vpTuningNodes[page]->pTuningRegBuf[(module_adr + 1*0x4 )>>2],\
                vpTuningNodes[page]->pTuningRegBuf[(module_adr + 2*0x4 )>>2],\
                CAM_READ_REG(this->m_pDrv,G2C_R1_G2C_CONV_0A),\
                CAM_READ_REG(this->m_pDrv,G2C_R1_G2C_CONV_0B),\
                CAM_READ_REG(this->m_pDrv,G2C_R1_G2C_CONV_1A));

        if(MTRUE == func_cam_DbgLogEnable_DEBUG) {
            dumpTuningRegBuf((void *)vpTuningNodes[page], "G2C_R1", module_size, module_adr);
        }

    } else {
        this->m_pDrv->cqNopModule(CAM_ISP_G2C_);
    }
BYPASS_G2C_R1:

    //G2C_R2
    if(vpTuningNodes[page]->ctlEn3_CAM & G2C_R2_EN_){
        switch(this->m_hwModule){
           case CAM_A:
           case CAM_B:
               break;
           default:
#if 0
               CAM_FUNC_ERR("hwModule(%d) no support G2C_R2,"
                            "G2C_R2 only support CAM_A/CAM_B only \n",
                            this->m_hwModule);
               ret = 1;
#endif
               goto BYPASS_G2C_R2;
        }
        CAM_WRITE_BITS(this->m_pDrv, CAMCTL_R1_CAMCTL_EN3, CAMCTL_G2C_R2_EN, 1);
    } else {
        CAM_WRITE_BITS(this->m_pDrv, CAMCTL_R1_CAMCTL_EN3, CAMCTL_G2C_R2_EN, 0);
    }

    if(vpTuningNodes[page]->eUpdateFuncBit & eIspTuningMgrFunc_G2C_R2){

        this->m_pDrv->cqApbModule(CAM_ISP_G2C_R2_);

        /*Get G2C_R2 Module Info*/
        this->m_pDrv->getCQModuleInfo(CAM_ISP_G2C_R2_,module_adr,module_size);
        /*Allocat a register space*/
        pReg_st = (ISP_DRV_REG_IO_STRUCT*)malloc(sizeof(ISP_DRV_REG_IO_STRUCT)*module_size);
        /*Set all the registers to the register space*/
        for(MUINT32 j=0;j<module_size;j++)
            pReg_st[j].Data = vpTuningNodes[page]->pTuningRegBuf[(module_adr + j*0x4)>>2];
        /*final update to the CQ virtual register space*/
        CAM_BURST_WRITE_REGS(this->m_pDrv,G2C_R2_G2C_CONV_0A,pReg_st,module_size);

        free(pReg_st);

        CAM_FUNC_DBG("G2C_R2:[0x%x_0x%x_0x%x],[0x%x_0x%x_0x%x] \n",\
                vpTuningNodes[page]->pTuningRegBuf[(module_adr )>>2],\
                vpTuningNodes[page]->pTuningRegBuf[(module_adr + 1*0x4 )>>2],\
                vpTuningNodes[page]->pTuningRegBuf[(module_adr + 2*0x4 )>>2],\
                CAM_READ_REG(this->m_pDrv,G2C_R2_G2C_CONV_0A),\
                CAM_READ_REG(this->m_pDrv,G2C_R2_G2C_CONV_0B),\
                CAM_READ_REG(this->m_pDrv,G2C_R2_G2C_CONV_1A));

        if(MTRUE == func_cam_DbgLogEnable_DEBUG) {
            dumpTuningRegBuf((void *)vpTuningNodes[page], "G2C_R2", module_size, module_adr);
        }

    } else {
        this->m_pDrv->cqNopModule(CAM_ISP_G2C_R2_);
    }
BYPASS_G2C_R2:

    //YNRS_R1 & YNRS_TABLE
    if(vpTuningNodes[page]->ctlEn3_CAM & YNRS_R1_EN_){
        switch(this->m_hwModule){
           case CAM_A:
           case CAM_B:
               break;
           default:
#if 0
               CAM_FUNC_ERR("hwModule(%d) no support YNRS_R1,"
                            "YNRS_R1 only support CAM_A/CAM_B only \n",
                            this->m_hwModule);
               ret = 1;
#endif
               goto BYPASS_YNRS_R1;
        }
        CAM_WRITE_BITS(this->m_pDrv, CAMCTL_R1_CAMCTL_EN3, CAMCTL_YNRS_R1_EN, 1);
    } else {
        CAM_WRITE_BITS(this->m_pDrv, CAMCTL_R1_CAMCTL_EN3, CAMCTL_YNRS_R1_EN, 0);
    }

    if(vpTuningNodes[page]->eUpdateFuncBit & eIspTuningMgrFunc_YNRS) {

        this->m_pDrv->cqApbModule(CAM_ISP_YNRS_);
        this->m_pDrv->cqApbModule(CAM_ISP_YNRS_PINGPONG_);
        /*Get YNRS Module Info*/
        this->m_pDrv->getCQModuleInfo(CAM_ISP_YNRS_,module_adr,module_size);
        /*Allocat a register space*/
        pReg_st = (ISP_DRV_REG_IO_STRUCT*)malloc(sizeof(ISP_DRV_REG_IO_STRUCT)*module_size);
        /*Set all the registers to the register space*/
        for(MUINT32 j=0;j<module_size;j++)
            pReg_st[j].Data = vpTuningNodes[page]->pTuningRegBuf[(module_adr + j*0x4)>>2];
        /*final update to the CQ virtual register space*/
        CAM_BURST_WRITE_REGS(this->m_pDrv,YNRS_R1_YNRS_TBL,pReg_st,module_size);

        free(pReg_st);

        /* PINGPONG switch sram when updated. */
        CAM_WRITE_REG(this->m_pDrv, YNRS_R1_YNRS_SRAM_PINGPONG, 1);
        CAM_FUNC_DBG("After tuningMgr set YNRS PingPong\n");

        CAM_FUNC_DBG("YNRS_R1_TABLE:[0x%x_0x%x_0x%x],[0x%x_0x%x_0x%x] \n",\
                vpTuningNodes[page]->pTuningRegBuf[(module_adr )>>2],\
                vpTuningNodes[page]->pTuningRegBuf[(module_adr + 256*0x4 )>>2],\
                vpTuningNodes[page]->pTuningRegBuf[(module_adr + 257*0x4 )>>2],\
                CAM_READ_REG(this->m_pDrv,YNRS_R1_YNRS_TBL),\
                CAM_READ_REG(this->m_pDrv,YNRS_R1_YNRS_CON1),\
                CAM_READ_REG(this->m_pDrv,YNRS_R1_YNRS_CON2));

        if(MTRUE == func_cam_DbgLogEnable_DEBUG) {
            dumpTuningRegBuf((void *)vpTuningNodes[page], "YNRS_R1", module_size, module_adr);
        }

    } else {
        this->m_pDrv->cqNopModule(CAM_ISP_YNRS_);
        this->m_pDrv->cqNopModule(CAM_ISP_YNRS_PINGPONG_);
    }
BYPASS_YNRS_R1:

    //RSS_R2
    if(vpTuningNodes[page]->ctlEn3_CAM & RSS_R2_EN_){
        switch(this->m_hwModule){
           case CAM_A:
           case CAM_B:
               break;
           default:
#if 0
               CAM_FUNC_ERR("hwModule(%d) no support RSS_R2,"
                            "RSS_R2 only support CAM_A/CAM_B only \n",
                            this->m_hwModule);
               ret = 1;
#endif
               goto BYPASS_RSS_R2;
        }
        CAM_WRITE_BITS(this->m_pDrv, CAMCTL_R1_CAMCTL_EN3, CAMCTL_RSS_R2_EN, 1);
    } else {
        CAM_WRITE_BITS(this->m_pDrv, CAMCTL_R1_CAMCTL_EN3, CAMCTL_RSS_R2_EN, 0);
    }

    if(vpTuningNodes[page]->eUpdateFuncBit & eIspTuningMgrFunc_RSS_R2){
        DMA_RSSO rsso;
        this->m_pDrv->cqApbModule(CAM_ISP_RSS_R2_);

        /*Get RSS_R2 Module Info*/
        this->m_pDrv->getCQModuleInfo(CAM_ISP_RSS_R2_,module_adr,module_size);
        /*Allocat a register space*/
        pReg_st = (ISP_DRV_REG_IO_STRUCT*)malloc(sizeof(ISP_DRV_REG_IO_STRUCT)*module_size);
        /*Set all the registers to the register space*/
        for(MUINT32 j=0;j<module_size;j++)
            pReg_st[j].Data = vpTuningNodes[page]->pTuningRegBuf[(module_adr + j*0x4)>>2];
        /*final update to the CQ virtual register space*/
        CAM_BURST_WRITE_REGS(this->m_pDrv,RSS_R2_RSS_CONTROL,pReg_st,module_size);

        free(pReg_st);

        //rsso , ref to the config() at statistic pipe
        rsso.m_pIspDrv = this->m_pIspDrv;
        rsso.dma_cfg.size.xsize = pReg->RSSO_R1_RSSO_XSIZE.Bits.RSSO_XSIZE;
        rsso.dma_cfg.size.h = pReg->RSSO_R1_RSSO_YSIZE.Bits.RSSO_YSIZE;
        rsso.dma_cfg.size.stride = rsso.dma_cfg.size.xsize;
        rsso.config();

        CAM_FUNC_DBG("RSS_R2:[0x%x_0x%x_0x%x],[0x%x_0x%x_0x%x] \n",\
                vpTuningNodes[page]->pTuningRegBuf[(module_adr )>>2],\
                vpTuningNodes[page]->pTuningRegBuf[(module_adr + 1*0x4 )>>2],\
                vpTuningNodes[page]->pTuningRegBuf[(module_adr + 2*0x4 )>>2],\
                CAM_READ_REG(this->m_pDrv,RSS_R2_RSS_CONTROL),\
                CAM_READ_REG(this->m_pDrv,RSS_R2_RSS_IN_IMG),\
                CAM_READ_REG(this->m_pDrv,RSS_R2_RSS_OUT_IMG));

        if(MTRUE == func_cam_DbgLogEnable_DEBUG) {
            dumpTuningRegBuf((void *)vpTuningNodes[page], "RSS_R2", module_size, module_adr);
        }

    } else {
        this->m_pDrv->cqNopModule(CAM_ISP_RSS_R2_);
    }
BYPASS_RSS_R2:

   //SLK
    if(vpTuningNodes[page]->ctlEn_CAM & SLK_R1_EN_){

       switch(this->m_hwModule){
          case CAM_A:
          case CAM_B:
              break;
          default:
#if 0
              CAM_FUNC_ERR("hwModule(%d) no support SLK_R1,"
                           "SLK_R1 only support CAM_A/CAM_B only \n",
                           this->m_hwModule);
              ret = 1;
#endif
              goto BYPASS_SLK;
       }

       CAM_WRITE_BITS(this->m_pDrv, CAMCTL_R1_CAMCTL_EN, CAMCTL_SLK_R1_EN, 1);
    } else {
       CAM_WRITE_BITS(this->m_pDrv, CAMCTL_R1_CAMCTL_EN, CAMCTL_SLK_R1_EN, 0);
    }

    if(vpTuningNodes[page]->eUpdateFuncBit & eIspTuningMgrFunc_SLK){
       this->SLKUpdateStatus[SLK_R1_Update] = MTRUE;
       this->m_pDrv->cqApbModule(CAM_ISP_SLK_CEN);
       this->m_pDrv->cqApbModule(CAM_ISP_SLK_XOFF);
       this->m_pDrv->cqApbModule(CAM_ISP_SLK_CON);
       this->m_pDrv->cqApbModule(CAM_ISP_SLK_SIZE);

       /*Get SLK Module Info*/
       this->m_pDrv->getCQModuleInfo(CAM_ISP_SLK_CEN,module_adr,module_size);
       /*Allocat a register space*/
       pReg_st = (ISP_DRV_REG_IO_STRUCT*)malloc(sizeof(ISP_DRV_REG_IO_STRUCT)*module_size);
       /*Set all the registers to the register space*/
       for(MUINT32 j=0;j<module_size;j++)
           pReg_st[j].Data = vpTuningNodes[page]->pTuningRegBuf[(module_adr + j*0x4)>>2];
       /*final update to the CQ virtual register space*/
       CAM_BURST_WRITE_REGS(this->m_pDrv,SLK_R1_SLK_CEN,pReg_st,module_size);

       if(MTRUE == func_cam_DbgLogEnable_DEBUG) {
            dumpTuningRegBuf((void *)vpTuningNodes[page], "SLK_R1_SLK_CEN", module_size, module_adr);
       }

       free(pReg_st);

       //SLK_R1_CON
       this->m_pDrv->getCQModuleInfo(CAM_ISP_SLK_CON,module_adr,module_size);
       //
       pReg_st = (ISP_DRV_REG_IO_STRUCT*)malloc(sizeof(ISP_DRV_REG_IO_STRUCT)*module_size);
       //
       for(MUINT32 j=0;j<module_size;j++)
           pReg_st[j].Data = vpTuningNodes[page]->pTuningRegBuf[(module_adr + j*0x4)>>2];
       //
       CAM_BURST_WRITE_REGS(this->m_pDrv,SLK_R1_SLK_SLP_CON0,pReg_st,module_size);

       if(MTRUE == func_cam_DbgLogEnable_DEBUG) {
            dumpTuningRegBuf((void *)vpTuningNodes[page], "SLK_R1_SLK_SLP_CON0", module_size, module_adr);
       }

       free(pReg_st);

       CAM_FUNC_DBG("SLK:[0x%x_0x%x_0x%x],[0x%x_0x%x_0x%x] \n",\
               vpTuningNodes[page]->pTuningRegBuf[(module_adr )>>2],\
               vpTuningNodes[page]->pTuningRegBuf[(module_adr + 1*0x4 )>>2],\
               vpTuningNodes[page]->pTuningRegBuf[(module_adr + 2*0x4 )>>2],\
               CAM_READ_REG(this->m_pDrv,SLK_R1_SLK_CEN),\
               CAM_READ_REG(this->m_pDrv,SLK_R1_SLK_RR_CON0),\
               CAM_READ_REG(this->m_pDrv,SLK_R1_SLK_RR_CON1));
    } else {
       this->SLKUpdateStatus[SLK_R1_Update] = MFALSE;
       this->m_pDrv->cqNopModule(CAM_ISP_SLK_CEN);
       this->m_pDrv->cqNopModule(CAM_ISP_SLK_XOFF);
       this->m_pDrv->cqNopModule(CAM_ISP_SLK_CON);
       this->m_pDrv->cqNopModule(CAM_ISP_SLK_SIZE);
    }
BYPASS_SLK:

    //SLK_R2
    if(vpTuningNodes[page]->ctlEn2_CAM & SLK_R2_EN_){

       switch(this->m_hwModule){
          case CAM_A:
          case CAM_B:
              break;
          default:
#if 0
              CAM_FUNC_ERR("hwModule(%d) no support SLK_R2,"
                           "SLK_R2 only support CAM_A/CAM_B only \n",
                           this->m_hwModule);
              ret = 1;
#endif
              goto BYPASS_SLK_R2;
       }

       CAM_WRITE_BITS(this->m_pDrv, CAMCTL_R1_CAMCTL_EN2, CAMCTL_SLK_R2_EN, 1);
    } else {
       CAM_WRITE_BITS(this->m_pDrv, CAMCTL_R1_CAMCTL_EN2, CAMCTL_SLK_R2_EN, 0);
    }

    if(vpTuningNodes[page]->eUpdateFuncBit & eIspTuningMgrFunc_SLK_R2){
       this->SLKUpdateStatus[SLK_R2_Update] = MTRUE;
       this->m_pDrv->cqApbModule(CAM_ISP_SLK_R2_CEN);
       this->m_pDrv->cqApbModule(CAM_ISP_SLK_R2_XOFF);
       this->m_pDrv->cqApbModule(CAM_ISP_SLK_R2_CON);
       this->m_pDrv->cqApbModule(CAM_ISP_SLK_R2_SIZE);

       /*Get SLK_R2_CEN Module Info*/
       this->m_pDrv->getCQModuleInfo(CAM_ISP_SLK_R2_CEN,module_adr,module_size);
       /*Allocat a register space*/
       pReg_st = (ISP_DRV_REG_IO_STRUCT*)malloc(sizeof(ISP_DRV_REG_IO_STRUCT)*module_size);
       /*Set all the registers to the register space*/
       for(MUINT32 j=0;j<module_size;j++)
           pReg_st[j].Data = vpTuningNodes[page]->pTuningRegBuf[(module_adr + j*0x4)>>2];
       /*final update to the CQ virtual register space*/
       CAM_BURST_WRITE_REGS(this->m_pDrv,SLK_R2_SLK_CEN,pReg_st,module_size);

       if(MTRUE == func_cam_DbgLogEnable_DEBUG) {
            dumpTuningRegBuf((void *)vpTuningNodes[page], "SLK_R2_SLK_CEN", module_size, module_adr);
       }

       free(pReg_st);

       //SLK_R2_CON
       this->m_pDrv->getCQModuleInfo(CAM_ISP_SLK_R2_CON,module_adr,module_size);
       //
       pReg_st = (ISP_DRV_REG_IO_STRUCT*)malloc(sizeof(ISP_DRV_REG_IO_STRUCT)*module_size);
       //
       for(MUINT32 j=0;j<module_size;j++)
           pReg_st[j].Data = vpTuningNodes[page]->pTuningRegBuf[(module_adr + j*0x4)>>2];
       //
       CAM_BURST_WRITE_REGS(this->m_pDrv,SLK_R2_SLK_SLP_CON0,pReg_st,module_size);

       if(MTRUE == func_cam_DbgLogEnable_DEBUG) {
            dumpTuningRegBuf((void *)vpTuningNodes[page], "SLK_R2_SLK_SLP_CON0", module_size, module_adr);
       }

       free(pReg_st);

       CAM_FUNC_DBG("SLK_R2:[0x%x_0x%x_0x%x],[0x%x_0x%x_0x%x] \n",\
               vpTuningNodes[page]->pTuningRegBuf[(module_adr )>>2],\
               vpTuningNodes[page]->pTuningRegBuf[(module_adr + 1*0x4 )>>2],\
               vpTuningNodes[page]->pTuningRegBuf[(module_adr + 2*0x4 )>>2],\
               CAM_READ_REG(this->m_pDrv,SLK_R2_SLK_CEN),\
               CAM_READ_REG(this->m_pDrv,SLK_R2_SLK_RR_CON0),\
               CAM_READ_REG(this->m_pDrv,SLK_R2_SLK_RR_CON1));
    } else {
       this->SLKUpdateStatus[SLK_R2_Update] = MFALSE;
       this->m_pDrv->cqNopModule(CAM_ISP_SLK_R2_CEN);
       this->m_pDrv->cqNopModule(CAM_ISP_SLK_R2_XOFF);
       this->m_pDrv->cqNopModule(CAM_ISP_SLK_R2_CON);
       this->m_pDrv->cqNopModule(CAM_ISP_SLK_R2_SIZE);
    }
BYPASS_SLK_R2:

    //BPCI_R1
    if(vpTuningNodes[page]->ctlEnDMA_CAM & BPCI_R1_EN_){
       CAM_WRITE_BITS(this->m_pDrv, CAMCTL_R1_CAMCTL_DMA_EN, CAMCTL_BPCI_R1_EN, 1);
    } else {
       CAM_WRITE_BITS(this->m_pDrv, CAMCTL_R1_CAMCTL_DMA_EN, CAMCTL_BPCI_R1_EN, 0);
    }

    if(vpTuningNodes[page]->eUpdateFuncBit & eIspTuningMgrFunc_BPCI){

       this->m_pDrv->cqApbModule(CAM_DMA_BPCI_);

       /*Get SLK Module Info*/
       this->m_pDrv->getCQModuleInfo(CAM_DMA_BPCI_,module_adr,module_size);
       /*Allocat a register space*/
       pReg_st = (ISP_DRV_REG_IO_STRUCT*)malloc(sizeof(ISP_DRV_REG_IO_STRUCT)*module_size);
       /*Set all the registers to the register space*/
       for(MUINT32 j=0;j<module_size;j++)
           pReg_st[j].Data = vpTuningNodes[page]->pTuningRegBuf[(module_adr + j*0x4)>>2];
       /*final update to the CQ virtual register space*/
       CAM_BURST_WRITE_REGS(this->m_pDrv,BPCI_R1_BPCI_BASE_ADDR,pReg_st,module_size);

       free(pReg_st);

       CAM_FUNC_DBG("BPCI_R1:[0x%x_0x%x_0x%x],[0x%x_0x%x_0x%x] \n",\
               vpTuningNodes[page]->pTuningRegBuf[(module_adr )>>2],\
               vpTuningNodes[page]->pTuningRegBuf[(module_adr + 1*0x4 )>>2],\
               vpTuningNodes[page]->pTuningRegBuf[(module_adr + 2*0x4 )>>2],\
               CAM_READ_REG(this->m_pDrv,BPCI_R1_BPCI_BASE_ADDR),\
               CAM_READ_REG(this->m_pDrv,BPCI_R1_BPCI_OFST_ADDR),\
               CAM_READ_REG(this->m_pDrv,BPCI_R1_BPCI_DRS));

       if(MTRUE == func_cam_DbgLogEnable_DEBUG) {
            dumpTuningRegBuf((void *)vpTuningNodes[page], "CAM_DMA_BPCI_", module_size, module_adr);
       }

    } else {
       this->m_pDrv->cqNopModule(CAM_DMA_BPCI_);
    }
    //BYPASS_BPCI_R1:

    if (UpdatebyCB == MFALSE){ // avoid tuning CB overwrite LCES_CB value.
        //LCES
        if(vpTuningNodes[page]->ctlEn_CAM & LCES_R1_EN_){
           CAM_WRITE_BITS(this->m_pDrv, CAMCTL_R1_CAMCTL_EN, CAMCTL_LCES_R1_EN, 1);
        } else {
           CAM_WRITE_BITS(this->m_pDrv, CAMCTL_R1_CAMCTL_EN, CAMCTL_LCES_R1_EN, 0);
        }

        if(vpTuningNodes[page]->eUpdateFuncBit & eIspTuningMgrFunc_LCES) {

           this->m_pDrv->cqApbModule(CAM_ISP_LCS_LOG_);
           this->m_pDrv->cqApbModule(CAM_ISP_LCS_FLC0_);

           /*Get LCS_LOG Module Info*/
           this->m_pDrv->getCQModuleInfo(CAM_ISP_LCS_LOG_,module_adr,module_size);
           /*Allocat a register space*/
           pReg_st = (ISP_DRV_REG_IO_STRUCT*)malloc(sizeof(ISP_DRV_REG_IO_STRUCT)*module_size);
           /*Set all the registers to the register space*/
           for(MUINT32 j=0;j<module_size;j++)
               pReg_st[j].Data = vpTuningNodes[page]->pTuningRegBuf[(module_adr + j*0x4)>>2];
           /*final update to the CQ virtual register space*/
           CAM_BURST_WRITE_REGS(this->m_pDrv,LCES_R1_LCES_LOG,pReg_st,module_size);

           if(MTRUE == func_cam_DbgLogEnable_DEBUG) {
                dumpTuningRegBuf((void *)vpTuningNodes[page], "LCES_R1_LCES_LOG", module_size, module_adr);
           }

           free(pReg_st);


           /*Get LCS_FLC0 Module Info*/
           this->m_pDrv->getCQModuleInfo(CAM_ISP_LCS_FLC0_,module_adr,module_size);
           /*Allocat a register space*/
           pReg_st = (ISP_DRV_REG_IO_STRUCT*)malloc(sizeof(ISP_DRV_REG_IO_STRUCT)*module_size);
           /*Set all the registers to the register space*/
           for(MUINT32 j=0;j<module_size;j++)
               pReg_st[j].Data = vpTuningNodes[page]->pTuningRegBuf[(module_adr + j*0x4)>>2];
           /*final update to the CQ virtual register space*/
           CAM_BURST_WRITE_REGS(this->m_pDrv,LCES_R1_LCES_FLC0,pReg_st,module_size);

           if(MTRUE == func_cam_DbgLogEnable_DEBUG) {
                dumpTuningRegBuf((void *)vpTuningNodes[page], "LCES_R1_LCES_FLC0", module_size, module_adr);
           }

           free(pReg_st);

           CAM_FUNC_DBG("LCS_LOG/LCS_FLC0:[0x%x_0x%x_0x%x],[0x%x_0x%x_0x%x] \n",\
                   vpTuningNodes[page]->pTuningRegBuf[(module_adr )>>2],\
                   vpTuningNodes[page]->pTuningRegBuf[(module_adr + 5*0x4 )>>2],\
                   vpTuningNodes[page]->pTuningRegBuf[(module_adr + 6*0x4 )>>2],\
                   CAM_READ_REG(this->m_pDrv,LCES_R1_LCES_LOG),\
                   CAM_READ_REG(this->m_pDrv,LCES_R1_LCES_FLC0),\
                   CAM_READ_REG(this->m_pDrv,LCES_R1_LCES_FLC1));
        } else {
           this->m_pDrv->cqNopModule(CAM_ISP_LCS_LOG_);
           this->m_pDrv->cqNopModule(CAM_ISP_LCS_FLC0_);
        }
    //BYPASS_LCS:
    }

    //LTMS : only enable module, don't update tuning register
    if(vpTuningNodes[page]->ctlEn_CAM & LTMS_R1_EN_){
       CAM_WRITE_BITS(this->m_pDrv, CAMCTL_R1_CAMCTL_EN, CAMCTL_LTMS_R1_EN, 1);
    } else {
       CAM_WRITE_BITS(this->m_pDrv, CAMCTL_R1_CAMCTL_EN, CAMCTL_LTMS_R1_EN, 0);
    }

    // LTM :
    if(vpTuningNodes[page]->ctlEn_CAM & LTM_R1_EN_){
       CAM_WRITE_BITS(this->m_pDrv, CAMCTL_R1_CAMCTL_EN, CAMCTL_LTM_R1_EN, 0);
       CAM_FUNC_DBG("LTM enable  do disable %d \n",
            CAM_READ_BITS(this->m_pDrv, CAMCTL_R1_CAMCTL_EN, CAMCTL_LTM_R1_EN));
    } else {
       CAM_WRITE_BITS(this->m_pDrv, CAMCTL_R1_CAMCTL_EN, CAMCTL_LTM_R1_EN, 0);
       CAM_FUNC_DBG("LTM disable do disable %d \n",
            CAM_READ_BITS(this->m_pDrv, CAMCTL_R1_CAMCTL_EN, CAMCTL_LTM_R1_EN));
    }

    if(vpTuningNodes[page]->eUpdateFuncBit & eIspTuningMgrFunc_LTM){
       this->m_pDrv->cqApbModule(CAM_ISP_LTM_SECTION1);
       this->m_pDrv->cqApbModule(CAM_ISP_LTM_SECTION2);

       /*Get LTM_R1_LTM_MAX_DIV Module Info*/
       this->m_pDrv->cqApbModule(CAM_ISP_LTM_MAX_DIV);

       this->m_pDrv->getCQModuleInfo(CAM_ISP_LTM_MAX_DIV,module_adr,module_size);
       /*Allocat a register space*/
       pReg_st = (ISP_DRV_REG_IO_STRUCT*)malloc(sizeof(ISP_DRV_REG_IO_STRUCT)*module_size);
       /*Set all the registers to the register space*/
       for(MUINT32 j=0;j<module_size;j++)
           pReg_st[j].Data = vpTuningNodes[page]->pTuningRegBuf[(module_adr + j*0x4)>>2];
       /*final update to the CQ virtual register space*/
       CAM_BURST_WRITE_REGS(this->m_pDrv,LTM_R1_LTM_MAX_DIV,pReg_st,module_size); //dicuss with Tuning


       free(pReg_st);

       CAM_FUNC_DBG("LTM_MAX_DIV:[0x%x],[0x%x] \n",\
                vpTuningNodes[page]->pTuningRegBuf[(module_adr )>>2],\
                CAM_READ_REG(this->m_pDrv,LTM_R1_LTM_MAX_DIV));

       if(MTRUE == func_cam_DbgLogEnable_DEBUG) {
            dumpTuningRegBuf((void *)vpTuningNodes[page], "LTM_R1_LTM_MAX_DIV", module_size, module_adr);
       }

       this->setLTM();

    } else {
       this->m_pDrv->cqNopModule(CAM_ISP_LTM_SECTION1);
       this->m_pDrv->cqNopModule(CAM_ISP_LTM_MAX_DIV);
       this->m_pDrv->cqNopModule(CAM_ISP_LTM_SECTION2);
    }

    // LTMTC_R1_CURVE : no need crtl enable.
    // eIspTuningMgrFunc_LTM & eIspTuningMgrFunc_LTMTC_R1_CURVE
    // must enable simultaneously
    if(vpTuningNodes[page]->eUpdateFuncBit & eIspTuningMgrFunc_LTMTC_R1_CURVE){
       MUINT32 module_adr_LTMTC_CURVE_1 = 0;
       /*Get CAM_ISP_LTMTC_CURVE Module Info*/
       this->m_pDrv->cqApbModule(CAM_ISP_LTMTC_CURVE_1);
       this->m_pDrv->cqApbModule(CAM_ISP_LTMTC_CURVE_2);
       this->m_pDrv->cqApbModule(CAM_ISP_LTM_PINGPONG_);

       this->m_pDrv->getCQModuleInfo(CAM_ISP_LTMTC_CURVE_1,module_adr,module_size);

       module_adr_LTMTC_CURVE_1 = module_adr;
       /*Allocat a register space*/
       pReg_st = (ISP_DRV_REG_IO_STRUCT*)malloc(sizeof(ISP_DRV_REG_IO_STRUCT)*module_size);
       /*Set all the registers to the register space*/
       for(MUINT32 j=0;j<module_size;j++)
           pReg_st[j].Data = vpTuningNodes[page]->pTuningRegBuf[(module_adr + j*0x4)>>2];
       /*final update to the CQ virtual register space*/
       CAM_BURST_WRITE_REGS(this->m_pDrv,LTMTC_R1_LTMTC_CURVE,pReg_st,module_size);

       free(pReg_st);

       if(MTRUE == func_cam_DbgLogEnable_DEBUG) {
           dumpTuningRegBuf((void *)vpTuningNodes[page], "LTMTC_R1_LTMTC_CURVE_1", module_size, module_adr);
       }

       /* Divided LTMTC_R1_LTMTC_CURVE[1782] due to CQ max write number only 1024 */
       this->m_pDrv->getCQModuleInfo(CAM_ISP_LTMTC_CURVE_2, module_adr, module_size);

       /*Allocat a register space*/
       pReg_st = (ISP_DRV_REG_IO_STRUCT*)malloc(sizeof(ISP_DRV_REG_IO_STRUCT) * module_size);
       /*Set all the registers to the register space*/
       for(MUINT32 j=0;j<module_size;j++)
           pReg_st[j].Data = vpTuningNodes[page]->pTuningRegBuf[(module_adr + j*0x4)>>2];

       /*final update to the CQ virtual register space*/
       CAM_BURST_WRITE_REGS_DIVIDED_ARRAY(this->m_pDrv,
                                          LTMTC_R1_LTMTC_CURVE,
                                          pReg_st,module_size,
                                         (module_adr-module_adr_LTMTC_CURVE_1));
       free(pReg_st);


       if(MTRUE == func_cam_DbgLogEnable_DEBUG) {
            dumpTuningRegBuf((void *)vpTuningNodes[page], "LTMTC_R1_LTMTC_CURVE_2", module_size, module_adr);
       }

       /* PINGPONG switch sram when updated. */
       CAM_WRITE_REG(this->m_pDrv, LTM_R1_LTM_SRAM_PINGPONG, 1);
       CAM_FUNC_DBG("After tuningMgr set LTM PingPong\n");

    } else {
       this->m_pDrv->cqNopModule(CAM_ISP_LTMTC_CURVE_1);
       this->m_pDrv->cqNopModule(CAM_ISP_LTMTC_CURVE_2);
       this->m_pDrv->cqNopModule(CAM_ISP_LTM_PINGPONG_);
    }

    //for secure fow, pre-enalbe LTM and LTMS
    if(pSec){
        if(pSec->SecMgr_GetSecureCamStatus(this->m_hwModule) > 0){
            PRE_REG_STATE status = SET_NONE;
            status = pSec->SecMgr_GetPreRegStatus(this->m_hwModule);
            //
            if(status == SET_TUNING){
                vector<MUINT32> preg_addr;
                preg_addr.clear();
                pSec->SecMgr_GetPreReg(&preg_addr,this->m_hwModule);
                for(MUINT32 i=0;i<preg_addr.size();i++)
                    CAM_FUNC_DBG("LTM/LTMS addr:0x%x",preg_addr.at(i));
                //
                CAM_WRITE_REG(this->m_pDrv,LTM_R1_LTM_TILE_SIZE,vpTuningNodes[page]->pTuningRegBuf[(preg_addr.at(0))>>2]);
                CAM_WRITE_REG(this->m_pDrv,LTM_R1_LTM_TILE_EDGE,vpTuningNodes[page]->pTuningRegBuf[(preg_addr.at(1))>>2]);
                CAM_WRITE_REG(this->m_pDrv,LTM_R1_LTM_TILE_CROP,vpTuningNodes[page]->pTuningRegBuf[(preg_addr.at(2))>>2]);
                CAM_WRITE_REG(this->m_pDrv,LTMS_R1_LTMS_BLK_NUM,vpTuningNodes[page]->pTuningRegBuf[(preg_addr.at(3))>>2]);
                CAM_WRITE_REG(this->m_pDrv,LTMS_R1_LTMS_BLK_SZ,vpTuningNodes[page]->pTuningRegBuf[(preg_addr.at(4))>>2]);
                CAM_WRITE_REG(this->m_pDrv,LTMS_R1_LTMS_IN_SIZE,vpTuningNodes[page]->pTuningRegBuf[(preg_addr.at(5))>>2]);
                CAM_WRITE_REG(this->m_pDrv,LTMS_R1_LTMS_OUT_SIZE,vpTuningNodes[page]->pTuningRegBuf[(preg_addr.at(6))>>2]);
                CAM_WRITE_REG(this->m_pDrv,LTMS_R1_LTMS_ACT_WINDOW_X,vpTuningNodes[page]->pTuningRegBuf[(preg_addr.at(7))>>2]);
                CAM_WRITE_REG(this->m_pDrv,LTMS_R1_LTMS_ACT_WINDOW_Y,vpTuningNodes[page]->pTuningRegBuf[(preg_addr.at(8))>>2]);
                CAM_WRITE_REG(this->m_pDrv,LTMS_R1_LTMS_OUT_DATA_NUM,vpTuningNodes[page]->pTuningRegBuf[(preg_addr.at(9))>>2]);
                CAM_WRITE_REG(this->m_pDrv,LTMS_R1_LTMS_SRAM_CFG,vpTuningNodes[page]->pTuningRegBuf[(preg_addr.at(10))>>2]);
                CAM_WRITE_REG(this->m_pDrv,LTMS_R1_LTMS_SRAM_CFG,vpTuningNodes[page]->pTuningRegBuf[(preg_addr.at(11))>>2]);
                CAM_WRITE_REG(this->m_pDrv,LTMS_R1_LTMS_SRAM_CFG,vpTuningNodes[page]->pTuningRegBuf[(preg_addr.at(12))>>2]);
                CAM_WRITE_REG(this->m_pDrv,LTMS_R1_LTMS_SRAM_CFG,vpTuningNodes[page]->pTuningRegBuf[(preg_addr.at(13))>>2]);

                CAM_WRITE_BITS(this->m_pDrv, CAMCTL_R1_CAMCTL_EN, CAMCTL_LTMS_R1_EN, 1);
                CAM_WRITE_BITS(this->m_pDrv, CAMCTL_R1_CAMCTL_EN, CAMCTL_LTM_R1_EN, 1);
                CAM_WRITE_BITS(this->m_pDrv, CAMCTL_R1_CAMCTL_DMA_EN, CAMCTL_LTMSO_R1_EN, 1);
            }else if(status == SET_DEFAULT){
                PreSet_Reg pReg;
                MUINT32 in_w = (CAM_READ_REG(this->m_pDrv, MRG_R4_MRG_CROP) >> 16) + 1;
                MUINT32 in_h = CAM_READ_REG(this->m_pDrv, MRG_R4_MRG_VSIZE);
                MUINT32 resize_mode =  CAM_READ_BITS(this->m_pDrv, QBIN_R4_QBIN_CTL, QBIN_ACC);

                CAM_FUNC_DBG("in_w:0x%x in_h:0x%x resize_mode:0x%x",in_w,in_h,resize_mode);
                if(pSec->SecMgr_SetPreReg(&pReg,in_w,in_h,resize_mode,this->m_hwModule)){
                    //
                    CAM_WRITE_REG(this->m_pDrv,LTM_R1_LTM_TILE_SIZE, pReg.LTM_R1_LTM_TILE_SIZE);
                    CAM_WRITE_REG(this->m_pDrv,LTM_R1_LTM_TILE_EDGE, pReg.LTM_R1_LTM_TILE_EDGE);
                    CAM_WRITE_REG(this->m_pDrv,LTM_R1_LTM_TILE_CROP, pReg.LTM_R1_LTM_TILE_CROP);
                    CAM_WRITE_REG(this->m_pDrv,LTMS_R1_LTMS_BLK_NUM, pReg.LTMS_R1_LTMS_BLK_NUM);
                    CAM_WRITE_REG(this->m_pDrv,LTMS_R1_LTMS_BLK_SZ, pReg.LTMS_R1_LTMS_BLK_SZ);
                    //protected
                    CAM_WRITE_REG(this->m_pDrv,LTMS_R1_LTMS_IN_SIZE, pReg.LTMS_R1_LTMS_IN_SIZE);
                    CAM_WRITE_REG(this->m_pDrv,LTMS_R1_LTMS_OUT_SIZE, pReg.LTMS_R1_LTMS_OUT_SIZE);
                    CAM_WRITE_REG(this->m_pDrv,LTMS_R1_LTMS_ACT_WINDOW_X, pReg.LTMS_R1_LTMS_ACT_WINDOW_X);
                    CAM_WRITE_REG(this->m_pDrv,LTMS_R1_LTMS_ACT_WINDOW_Y, pReg.LTMS_R1_LTMS_ACT_WINDOW_Y);

                    CAM_WRITE_REG(this->m_pDrv,LTMS_R1_LTMS_OUT_DATA_NUM, pReg.LTMS_R1_LTMS_OUT_DATA_NUM);
                    CAM_WRITE_REG(this->m_pDrv,LTMS_R1_LTMS_SRAM_CFG, pReg.LTMS_R1_LTMS_SRAM_CFG);

                    CAM_WRITE_BITS(this->m_pDrv, CAMCTL_R1_CAMCTL_EN, CAMCTL_LTMS_R1_EN, 1);
                    CAM_WRITE_BITS(this->m_pDrv, CAMCTL_R1_CAMCTL_EN, CAMCTL_LTM_R1_EN, 1);
                    CAM_WRITE_BITS(this->m_pDrv, CAMCTL_R1_CAMCTL_DMA_EN, CAMCTL_LTMSO_R1_EN, 1);

                    CAM_WRITE_REG(this->m_pDrv->getPhyObj(),LTMSO_R1_LTMSO_XSIZE, pReg.LTMSO_R1_LTMSO_XSIZE);
                    CAM_WRITE_REG(this->m_pDrv->getPhyObj(),LTMSO_R1_LTMSO_YSIZE, pReg.LTMSO_R1_LTMSO_YSIZE);
                    CAM_WRITE_REG(this->m_pDrv->getPhyObj(),LTMSO_R1_LTMSO_STRIDE, pReg.LTMSO_R1_LTMSO_STRIDE);
                }else{
                    CAM_FUNC_ERR("SetPreReg failed, in_w:0x%x in_h:0x%x mode:0x%x",in_w,in_h,resize_mode);
                }
           }
        }
    }

    //HLR_R1
    if(vpTuningNodes[page]->ctlEn2_CAM & HLR_R1_EN_){
        CAM_WRITE_BITS(this->m_pDrv, CAMCTL_R1_CAMCTL_EN2, CAMCTL_HLR_R1_EN, 1);
    } else {
        CAM_WRITE_BITS(this->m_pDrv, CAMCTL_R1_CAMCTL_EN2, CAMCTL_HLR_R1_EN, 0);
    }

    if(vpTuningNodes[page]->eUpdateFuncBit & eIspTuningMgrFunc_HLR){

        this->m_pDrv->cqApbModule(CAM_ISP_HLR_);
        this->m_pDrv->cqApbModule(CAM_CTL_HLR_LKMSB_);

        /*Get HLR Module Info*/
        this->m_pDrv->getCQModuleInfo(CAM_ISP_HLR_,module_adr,module_size);
        /*Allocat a register space*/
        pReg_st = (ISP_DRV_REG_IO_STRUCT*)malloc(sizeof(ISP_DRV_REG_IO_STRUCT)*module_size);
        /*Set all the registers to the register space*/
        for(MUINT32 j=0;j<module_size;j++)
            pReg_st[j].Data = vpTuningNodes[page]->pTuningRegBuf[(module_adr + j*0x4)>>2];
        /*final update to the CQ virtual register space*/
        CAM_BURST_WRITE_REGS(this->m_pDrv,HLR_R1_HLR_EST_Y0,pReg_st,module_size);

        free(pReg_st);

        CAM_FUNC_DBG("HLR_R1:[0x%x_0x%x_0x%x],[0x%x_0x%x_0x%x] \n",\
                vpTuningNodes[page]->pTuningRegBuf[(module_adr )>>2],\
                vpTuningNodes[page]->pTuningRegBuf[(module_adr + 1*0x4 )>>2],\
                vpTuningNodes[page]->pTuningRegBuf[(module_adr + 2*0x4 )>>2],\
                CAM_READ_REG(this->m_pDrv,HLR_R1_HLR_EST_Y0),\
                CAM_READ_REG(this->m_pDrv,HLR_R1_HLR_EST_Y1),\
                CAM_READ_REG(this->m_pDrv,HLR_R1_HLR_EST_Y2));

        if(MTRUE == func_cam_DbgLogEnable_DEBUG) {
            dumpTuningRegBuf((void *)vpTuningNodes[page], "HLR_R1", module_size, module_adr);
        }

        /*Get HLR_LKMSB Module Info*/
        this->m_pDrv->getCQModuleInfo(CAM_CTL_HLR_LKMSB_,module_adr,module_size);
        /*Allocat a register space*/
        pReg_st = (ISP_DRV_REG_IO_STRUCT*)malloc(sizeof(ISP_DRV_REG_IO_STRUCT)*module_size);
        /*Set all the registers to the register space*/
        for(MUINT32 j=0;j<module_size;j++)
            pReg_st[j].Data = vpTuningNodes[page]->pTuningRegBuf[(module_adr + j*0x4)>>2];
        /*final update to the CQ virtual register space*/
        CAM_BURST_WRITE_REGS(this->m_pDrv,CAMCTL_R1_CAMCTL_HLR_LKMSB,pReg_st,module_size);

        free(pReg_st);

        CAM_FUNC_DBG("HLR_R1_LKMSB:[0x%x],[0x%x] \n",\
                     vpTuningNodes[page]->pTuningRegBuf[(module_adr )>>2],\
                     CAM_READ_REG(this->m_pDrv,CAMCTL_R1_CAMCTL_HLR_LKMSB));

        if(MTRUE == func_cam_DbgLogEnable_DEBUG) {
            dumpTuningRegBuf((void *)vpTuningNodes[page], "HLR_R1_LKMSB", module_size, module_adr);
        }

    } else {
        this->m_pDrv->cqNopModule(CAM_ISP_HLR_);
        this->m_pDrv->cqNopModule(CAM_CTL_HLR_LKMSB_);
    }

    CAM_FUNC_DBG("update- "
                 "cq:0x%x cqpage:0x%x page:0x%x SenId:0x%x Mag:%d \n"
                 "eUpdateFuncBit:0x%08x "
                 "ctlEn_CAM:0x%08x, ctlEn2_CAM:0x%08x, "
                 "ctlEn3_CAM:0x%08x, ctlEn4_CAM:0x%08x "
                 "ctlEnDMA_CAM:0x%08x, ctlEnDMA2_CAM:0x%08x "
                 "ctlEn_UNI:0x%08x, ctlEnDMA_UNI:0x%08x ",
                 cq, cqpage, page, this->m_SenDev, showMagic,
                 vpTuningNodes[page]->eUpdateFuncBit,
                 vpTuningNodes[page]->ctlEn_CAM, vpTuningNodes[page]->ctlEn2_CAM,
                 vpTuningNodes[page]->ctlEn3_CAM, vpTuningNodes[page]->ctlEn4_CAM,
                 vpTuningNodes[page]->ctlEnDMA_CAM, vpTuningNodes[page]->ctlEnDMA2_CAM,
                 vpTuningNodes[page]->ctlEn_UNI, vpTuningNodes[page]->ctlEnDMA_UNI);


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

MINT32 CAM_TUNING_CTRL::setNotify(E_SET_NOTIFY_OP op)
{
    E_ISP_CAM_CQ cq;
    MUINT32 page = 0;

    this->m_pDrv = (ISP_DRV_CAM*)this->m_pIspDrv;
    this->m_pDrv->getCurObjInfo(&this->m_hwModule,&cq,&page);

    switch(op){
        case E_Notify_Config:
            this->m_CurBin = CAM_READ_BITS(this->m_pDrv->getPhyObj(), CAMCTL_R1_CAMCTL_EN2, CAMCTL_BIN_R1_EN);
            this->m_CurQbn1Acc = (CAM_C == this->m_hwModule) ? _1_pix_ : CAM_READ_BITS(this->m_pDrv, QBIN_R1_QBIN_CTL, QBIN_ACC);
            XOR_WRITE_BITS(this->m_hwModule,this->m_pDrv, QBIN_R1_QBIN_CTL, QBIN_ACC, this->m_CurQbn1Acc);
            break;
        case E_Notify_Update:
            this->m_CurBin = CAM_READ_BITS(this->m_pDrv, CAMCTL_R1_CAMCTL_EN2, CAMCTL_BIN_R1_EN);
            this->m_CurQbn1Acc = (CAM_C == this->m_hwModule) ? _1_pix_ : CAM_READ_BITS(this->m_pDrv, QBIN_R1_QBIN_CTL, QBIN_ACC);
            break;
        default:
            CAM_FUNC_ERR("unsupport cmd(%d)", op);
            break;
    }

    return 0;
}

MINT32 CAM_TUNING_CTRL::configNotify(void)
{
    E_ISP_CAM_CQ cq;
    MUINT32 page;

    this->m_pDrv = (ISP_DRV_CAM*)this->m_pIspDrv;
    //dbg log
    this->m_pDrv->getCurObjInfo(&this->m_hwModule,&cq,&page);

    this->m_CurBin = CAM_READ_BITS(this->m_pDrv,CAMCTL_R1_CAMCTL_EN2, CAMCTL_BIN_R1_EN);
    this->m_CurQbn1Acc = (CAM_C == this->m_hwModule) ? _1_pix_ : CAM_READ_BITS(this->m_pDrv,QBIN_R1_QBIN_CTL,QBIN_ACC);
    this->m_NotifyFrame = 0;//reset to 0. first enque will update this

    this->m_TG_W.Raw = CAM_READ_REG(this->m_pDrv->getPhyObj(),TG_R1_TG_SEN_GRAB_PXL);
    this->m_TG_H.Raw = CAM_READ_REG(this->m_pDrv->getPhyObj(),TG_R1_TG_SEN_GRAB_LIN);

    CAM_FUNC_DBG("m_CurBin=%d, m_bBin=%d,"
                "m_CurQbn1Acc=%d"
                "m_NotifyFrame=%d",
                this->m_CurBin, this->m_bBin,
                this->m_CurQbn1Acc,
                this->m_NotifyFrame);

    return 0;
}

MINT32 CAM_TUNING_CTRL::configCB(MBOOL bForce)
{
    MINT32 ret = 0;
    MUINT32 Qbn1Acc = 0;

    this->m_pDrv = (ISP_DRV_CAM*)this->m_pIspDrv;

    if(bForce){
        this->m_NotifyFrame = Dynamic_bin_Notify_Frame;
    }

    if(this->m_CurBin != this->m_bBin){
        this->m_NotifyFrame = Dynamic_bin_Notify_Frame;
    }

    Qbn1Acc = (CAM_C == this->m_hwModule) ? _1_pix_ : CAM_READ_BITS(this->m_pDrv,QBIN_R1_QBIN_CTL,QBIN_ACC);

    if(this->m_CurQbn1Acc != Qbn1Acc){
        this->m_NotifyFrame = Dynamic_bin_Notify_Frame;
    }

    CAM_FUNC_DBG("m_CurBin=%d, m_bBin=%d,"
                "m_CurQbn1Acc=%d, Qbn1Acc=%d,"
                "m_NotifyFrame=%d",
                this->m_CurBin, this->m_bBin,
                this->m_CurQbn1Acc, Qbn1Acc,
                this->m_NotifyFrame);
    return ret;
}

MBOOL
CAM_TUNING_CTRL::setAWB(void)
{
    MBOOL ret = MTRUE;
    char _str[128] = {'\0'};
    MUINT32 Qbn1Acc = 0;
    this->m_pDrv = (ISP_DRV_CAM*)this->m_pIspDrv;
    snprintf(_str, 127, __FUNCTION__);

    if (!this->pIspReg){
        CAM_FUNC_ERR("pIspReg = NULL\n");
        return MFALSE;
    }

    if(this->m_pP1Tuning){
        Tuning_CFG     _tuning_cfg;

        RectData<MUINT32> Bin;
        this->m_input.Magic = CAM_READ_REG(this->m_pDrv,CAMDMATOP1_R1_CAMDMATOP1_DMA_MAGIC_NUM);

        //input for CB, set "before" BIN_EN/QBN1_ACC/RMB_ACC changing
        this->m_input.TgOut_W = this->m_SrcSize.w;
        this->m_input.TgOut_H = this->m_SrcSize.h;
        // this->m_input.TgOut_W = this->m_TG_W.Bits.TG_PXL_E - this->m_TG_W.Bits.TG_PXL_S;
        // this->m_input.TgOut_H = this->m_TG_H.Bits.TG_LIN_E - this->m_TG_H.Bits.TG_LIN_S;
        this->m_input.CurBinOut_W = this->m_input.TgOut_W >> (this->m_CurBin + CAM_READ_BITS(this->m_pDrv,CAMCTL_R1_CAMCTL_EN2,CAMCTL_DBN_R1_EN) \
                                                                         + CAM_READ_BITS(this->m_pDrv,CAMCTL_R1_CAMCTL_EN2,CAMCTL_FBND_R1_EN));
        this->m_input.CurBinOut_H = this->m_input.TgOut_H >> (this->m_CurBin + CAM_READ_BITS(this->m_pDrv,CAMCTL_R1_CAMCTL_EN2,CAMCTL_FBND_R1_EN));
        this->m_input.CurQBNOut_W = this->m_input.CurBinOut_W >> ((CAM_C == this->m_hwModule) ? _1_pix_ : CAM_READ_BITS(this->m_pDrv,QBIN_R1_QBIN_CTL,QBIN_ACC));

        //set input of CB fun, set "after" BIN_EN/QBN1_ACC/RMB_ACC changing
        this->m_input.Bin_MD = CAM_READ_BITS(this->m_pDrv,CAMCTL_R1_CAMCTL_EN2,CAMCTL_BIN_R1_EN);//0: average, 1: sum
        this->m_input.TarBin_EN = this->m_bBin;
        GET_FULL_BIN(this->m_pDrv, Bin,2);
        this->m_input.TarBinOut_W = this->m_input.TgOut_W >> Bin.H;
        this->m_input.TarBinOut_H = this->m_input.TgOut_H >> Bin.V;
        this->m_input.TarQBNOut_W = this->m_input.TarBinOut_W >> Qbn1Acc;

        _tuning_cfg.Bypass.Raw = 0xFFFFFFFF;
        _tuning_cfg.pIspReg = this->pIspReg;
        _tuning_cfg.dma_lsci.memID = _tuning_cfg.dma_bpci.memID = 0;
        _tuning_cfg.dma_lsci.va = _tuning_cfg.dma_bpci.va = 0;

        CAM_FUNC_DBG("%s:tg:%dx%d,bin_en:%d_%d,bin_md:%d,bin_size=%dx%d_%dx%d,magic:%d,Notify:%d,qbin:%d_%d,rmb:%d_%d\n",\
            __FUNCTION__, this->m_input.TgOut_W, this->m_input.TgOut_H, \
            this->m_CurBin, this->m_input.TarBin_EN, this->m_input.Bin_MD, \
            this->m_input.CurBinOut_W, this->m_input.CurBinOut_H, \
            this->m_input.TarBinOut_W, this->m_input.TarBinOut_H,\
            this->m_input.Magic, this->m_NotifyFrame, \
            this->m_input.CurQBNOut_W, this->m_input.TarQBNOut_W,\
            this->m_input.CurRMBOut_W, this->m_input.TarRMBOut_W);

        this->CBTimeChk(_str, MTRUE);
        this->m_pP1Tuning->p1TuningNotify((MVOID*)&this->m_input,(MVOID*)&_tuning_cfg);
        this->CBTimeChk(_str, MFALSE);

        CAM_FUNC_DBG("%s: CTL_EN:%x, bypass:%x\n", \
            __FUNCTION__, CAM_READ_REG(this->m_pDrv,CAMCTL_R1_CAMCTL_EN),_tuning_cfg.Bypass.Raw);

        //AWB
        if(_tuning_cfg.Bypass.Bits.AA != 1 && CAM_READ_BITS(this->m_pDrv,CAMCTL_R1_CAMCTL_EN,CAMCTL_AA_R1_EN)) {
            CAM_WRITE_REG(this->m_pDrv, WB_R1_WB_GAIN_1,   CAM_CB_READ_REG(this->pIspReg, WB_R1_WB_GAIN_1));
            CAM_WRITE_REG(this->m_pDrv, WB_R1_WB_GAIN_2,   CAM_CB_READ_REG(this->pIspReg, WB_R1_WB_GAIN_2));
            CAM_WRITE_REG(this->m_pDrv, AA_R1_AA_AWB_L0_X, CAM_CB_READ_REG(this->pIspReg, AA_R1_AA_AWB_L0_X));
            CAM_WRITE_REG(this->m_pDrv, AA_R1_AA_AWB_L0_Y, CAM_CB_READ_REG(this->pIspReg, AA_R1_AA_AWB_L0_Y));
        }
    }
    return ret;
}


MBOOL CAM_TUNING_CTRL::setTuning(void)
{
    MBOOL ret = MTRUE;
    capibility CamInfo;
    MBOOL TuningCBupdate = MTRUE;
    char _str[128] = {'\0'};
    REG_TG_R1_TG_SEN_GRAB_PXL TG_W;
    REG_TG_R1_TG_SEN_GRAB_LIN TG_H;
    MUINT32 tmp = 0;
    RectData<MUINT32> Bin;
    Header_RRZO fh_rrzo;

    this->m_pDrv = (ISP_DRV_CAM*)this->m_pIspDrv;
    snprintf(_str, 127, __FUNCTION__);

    if (!this->m_NotifyFrame){
        return ret;
    }

    if (!this->pIspReg){
        CAM_FUNC_ERR("pIspReg = NULL\n");
        return MFALSE;
    }

    if(this->m_pP1Tuning){
        RectData<MUINT32> Bin;

        this->m_TuningCBinput.Magic = CAM_READ_REG(this->m_pDrv,
                    CAMDMATOP1_R1_CAMDMATOP1_DMA_MAGIC_NUM);

        this->m_TuningCBinput.m_hwModule = tuningHwModuleMap(this->m_hwModule);

        //input for CB, set "before" BIN_EN/QBN1_ACC/RMB_ACC changing
        this->m_TuningCBinput.TgOut_W = this->m_SrcSize.w;
        this->m_TuningCBinput.TgOut_H = this->m_SrcSize.h;

        this->m_TuningCBinput.FrontalBin =
            CAM_READ_BITS(this->m_pDrv, CAMCTL_R1_CAMCTL_EN2, CAMCTL_FBND_R1_EN);

        TG_W.Raw = CAM_READ_REG(this->m_pDrv, TG_R1_TG_SEN_GRAB_PXL);
        TG_H.Raw = CAM_READ_REG(this->m_pDrv, TG_R1_TG_SEN_GRAB_LIN);
        GET_FULL_BIN(this->m_pDrv, Bin, 2);

        this->m_TuningCBinput.RRZ_IN_CROP.src_w   = (TG_W.Bits.TG_PXL_E - TG_W.Bits.TG_PXL_S) >> Bin.H;
        this->m_TuningCBinput.RRZ_IN_CROP.src_h   = (TG_H.Bits.TG_LIN_E - TG_H.Bits.TG_LIN_S) >> Bin.V;
        this->m_TuningCBinput.RRZ_IN_CROP.crop_x  = CAM_READ_BITS(this->m_pDrv, RRZ_R1_RRZ_HORI_INT_OFST, RRZ_HORI_INT_OFST);
        this->m_TuningCBinput.RRZ_IN_CROP.crop_y  = CAM_READ_BITS(this->m_pDrv, RRZ_R1_RRZ_VERT_INT_OFST, RRZ_VERT_INT_OFST);
        tmp = fh_rrzo.GetRegInfo(Header_RRZO::E_RRZ_CRP_SIZE,this->m_pDrv);
        this->m_TuningCBinput.RRZ_IN_CROP.crop_w  = tmp & 0xffff;
        this->m_TuningCBinput.RRZ_IN_CROP.crop_h  = (tmp>>16);
        tmp = fh_rrzo.GetRegInfo(Header_RRZO::E_RRZ_DST_SIZE,this->m_pDrv);
        this->m_TuningCBinput.RRZ_IN_CROP.tar_w   = tmp & 0xffff;
        this->m_TuningCBinput.RRZ_IN_CROP.tar_h   = (tmp>>16);

        //input for AF
        if (!CamInfo.m_DTwin.GetDTwin()){
            CAM_CB_WRITE_REG(this->pIspReg, AF_R1_AF_VLD, CAM_READ_REG(this->m_pDrv,AF_R1_AF_VLD));
            CAM_CB_WRITE_REG(this->pIspReg, AF_R1_AF_BLK_0, CAM_READ_REG(this->m_pDrv,AF_R1_AF_BLK_0));
        }

        CAM_FUNC_DBG("%s: tg:%dx%d frontbin_en:%d "
                     "rrz:(src_w src_h crop_x crop_y crop_w crop_h tar_w tar_h) "
                     "(%dx%d_%dx%d_%dx%d_%dx%d) "
                     "magic:%d Notify:%d\n",
                     __FUNCTION__,
                     this->m_TuningCBinput.TgOut_W, this->m_TuningCBinput.TgOut_H, this->m_TuningCBinput.FrontalBin,
                     this->m_TuningCBinput.RRZ_IN_CROP.src_w,  this->m_TuningCBinput.RRZ_IN_CROP.src_h,
                     this->m_TuningCBinput.RRZ_IN_CROP.crop_x,  this->m_TuningCBinput.RRZ_IN_CROP.crop_y,
                     this->m_TuningCBinput.RRZ_IN_CROP.crop_w, this->m_TuningCBinput.RRZ_IN_CROP.crop_h,
                     this->m_TuningCBinput.RRZ_IN_CROP.tar_w,  this->m_TuningCBinput.RRZ_IN_CROP.tar_h,
                     this->m_TuningCBinput.Magic, this->m_NotifyFrame);

        CAM_FUNC_DBG("Before tuningCB: "
                     "m_UpdateFuncBit:0x%08x Magic:%d "
                     "CTL_EN:0x%08x CTL_EN2:0x%08x CTL_EN3:0x%08x "
                     "CTL_EN4:0x%08x CTL_DMA_EN:0x%08x CTL_DMA_EN2:0x%08x ",
                     this->m_UpdateFuncBit, this->m_TuningCBinput.Magic,
                     CAM_READ_REG(this->m_pDrv, CAMCTL_R1_CAMCTL_EN),
                     CAM_READ_REG(this->m_pDrv, CAMCTL_R1_CAMCTL_EN2),
                     CAM_READ_REG(this->m_pDrv, CAMCTL_R1_CAMCTL_EN3),
                     CAM_READ_REG(this->m_pDrv, CAMCTL_R1_CAMCTL_EN4),
                     CAM_READ_REG(this->m_pDrv, CAMCTL_R1_CAMCTL_DMA_EN),
                     CAM_READ_REG(this->m_pDrv, CAMCTL_R1_CAMCTL_DMA2_EN));

        this->CBTimeChk(_str, MTRUE);
        this->m_pP1Tuning->p1TuningNotify((MVOID*)&this->m_TuningCBinput, NULL);
        this->CBTimeChk(_str, MFALSE);

        if(this->update(MTRUE, TuningCBupdate, this->m_TuningCBinput.Magic) != 0) {
            CAM_FUNC_ERR("setTuning update failed !!!\n");
        }

        if(this->setSLK() == MFALSE) {
            CAM_FUNC_ERR("setSLK in setTuning failed !!!\n");
        }

        CAM_FUNC_DBG("After  tuningCB & update: "
             "m_UpdateFuncBit:0x%08x Magic:%d "
             "CTL_EN:0x%08x CTL_EN2:0x%08x CTL_EN3:0x%08x "
             "CTL_EN4:0x%08x CTL_DMA_EN:0x%08x CTL_DMA_EN2:0x%08x ",
             this->m_UpdateFuncBit, this->m_TuningCBinput.Magic,
             CAM_READ_REG(this->m_pDrv, CAMCTL_R1_CAMCTL_EN),
             CAM_READ_REG(this->m_pDrv, CAMCTL_R1_CAMCTL_EN2),
             CAM_READ_REG(this->m_pDrv, CAMCTL_R1_CAMCTL_EN3),
             CAM_READ_REG(this->m_pDrv, CAMCTL_R1_CAMCTL_EN4),
             CAM_READ_REG(this->m_pDrv, CAMCTL_R1_CAMCTL_DMA_EN),
             CAM_READ_REG(this->m_pDrv, CAMCTL_R1_CAMCTL_DMA2_EN));
    }
    return ret;
}

MBOOL CAM_TUNING_CTRL::dumpCQReg(void)
{
    MBOOL ret = MTRUE;
    MUINT32 addrOffset = 0 , moduleSize = 0, dumpcq = 0;
    static MUINT32* pIspRegMap = NULL;
    static MUINT32 _dpFrameCnt = 0, _preDumpcq = 0;
    char value[PROPERTY_VALUE_MAX] = {'\0'};
    char _str[128] = {'\0'};
    this->m_pDrv = (ISP_DRV_CAM*)this->m_pIspDrv;
    snprintf(_str, 127, __FUNCTION__);

    if(this->m_pP1Tuning){
        //adb control dump
        //1. set debug.isp.dumpcq.framecnt > 0
        //2. set debug.isp.dumpcq.enable > 0
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
            pIspRegMap = (MUINT32*)calloc(this->m_dumpInput.size/sizeof(MUINT32), sizeof(MUINT32));
            if(pIspRegMap == NULL) {
                CAM_FUNC_ERR("allocate mem for dump fail\n");
                return MFALSE;
            }
            this->m_dumpInput.pReg = (void *)pIspRegMap;
        }

        this->m_dumpInput.magic = CAM_READ_REG(this->m_pDrv,CAMDMATOP1_R1_CAMDMATOP1_DMA_MAGIC_NUM);

        CAM_FUNC_INF("DumpCQ m_UpdateFuncBit = 0x%lx\n", this->m_UpdateFuncBit);

        this->CBTimeChk(_str, MTRUE);
        if(this->m_UpdateFuncBit & eIspTuningMgrFunc_OBC){

            this->m_pDrv->getCQModuleInfo(CAM_ISP_OBC_, addrOffset, moduleSize);
            CAM_PTR_READ_REGS(this->m_pDrv,OBC_R1_OBC_CTL,&pIspRegMap,moduleSize);
        }

        if(this->m_UpdateFuncBit & eIspTuningMgrFunc_BPC){

            this->m_pDrv->getCQModuleInfo(CAM_ISP_BPC_, addrOffset, moduleSize);
            CAM_PTR_READ_REGS(this->m_pDrv,BPC_R1_BPC_BPC_CON,&pIspRegMap,moduleSize);
            this->m_pDrv->getCQModuleInfo(CAM_DMA_BPCI_, addrOffset, moduleSize);
            CAM_PTR_READ_REGS(this->m_pDrv,BPCI_R1_BPCI_BASE_ADDR,&pIspRegMap,moduleSize);
        }

        if(this->m_UpdateFuncBit & eIspTuningMgrFunc_PDE){

            this->m_pDrv->getCQModuleInfo(CAM_ISP_PDE_, addrOffset, moduleSize);
            CAM_PTR_READ_REGS(this->m_pDrv,PDE_R1_PDE_TBLI1,&pIspRegMap,moduleSize);
            this->m_pDrv->getCQModuleInfo(CAM_DMA_PDO_, addrOffset, moduleSize);
            CAM_PTR_READ_REGS(this->m_pDrv,PDO_R1_PDO_XSIZE,&pIspRegMap,moduleSize);
            this->m_pDrv->getCQModuleInfo(CAM_DMA_PDO_BA, addrOffset, moduleSize);
            CAM_PTR_READ_REGS(this->m_pDrv,PDO_R1_PDO_BASE_ADDR,&pIspRegMap,moduleSize);
            this->m_pDrv->getCQModuleInfo(CAM_DMA_PDI_, addrOffset, moduleSize);
            CAM_PTR_READ_REGS(this->m_pDrv,PDI_R1_PDI_BASE_ADDR,&pIspRegMap,moduleSize);
        }
        if(this->m_UpdateFuncBit & eIspTuningMgrFunc_PBN){

            this->m_pDrv->getCQModuleInfo(CAM_ISP_PBN_, addrOffset, moduleSize);
            CAM_PTR_READ_REGS(this->m_pDrv,PBN_R1_PBN_PBN_TYPE,&pIspRegMap,moduleSize);
            this->m_pDrv->getCQModuleInfo(CAM_DMA_PDO_, addrOffset, moduleSize);
            CAM_PTR_READ_REGS(this->m_pDrv,PDO_R1_PDO_XSIZE,&pIspRegMap,moduleSize);
            this->m_pDrv->getCQModuleInfo(CAM_DMA_PDO_BA, addrOffset, moduleSize);
            CAM_PTR_READ_REGS(this->m_pDrv,PDO_R1_PDO_BASE_ADDR,&pIspRegMap,moduleSize);
        }
        if(this->m_UpdateFuncBit & eIspTuningMgrFunc_LSC){

            this->m_pDrv->getCQModuleInfo(CAM_ISP_LSC_, addrOffset, moduleSize);
            CAM_PTR_READ_REGS(this->m_pDrv,LSC_R1_LSC_CTL1,&pIspRegMap,moduleSize);
            this->m_pDrv->getCQModuleInfo(CAM_DMA_LSCI_, addrOffset, moduleSize);
            CAM_PTR_READ_REGS(this->m_pDrv,LSCI_R1_LSCI_BASE_ADDR,&pIspRegMap,moduleSize);
        }
        if (this->m_UpdateFuncBit & eIspTuningMgrFunc_AA) {

            this->m_pDrv->getCQModuleInfo(CAM_ISP_AWB_, addrOffset, moduleSize);
            CAM_PTR_READ_REGS(this->m_pDrv,AA_R1_AA_AWB_WIN_ORG,&pIspRegMap,moduleSize);
            this->m_pDrv->getCQModuleInfo(CAM_ISP_AE_, addrOffset, moduleSize);
            CAM_PTR_READ_REGS(this->m_pDrv,AA_R1_AA_AE_GAIN2_0,&pIspRegMap,moduleSize);
            this->m_pDrv->getCQModuleInfo(CAM_DMA_AAO_, addrOffset, moduleSize);
            CAM_PTR_READ_REGS(this->m_pDrv,AAO_R1_AAO_XSIZE,&pIspRegMap,moduleSize);
            this->m_pDrv->getCQModuleInfo(CAM_DMA_AAO_BA, addrOffset, moduleSize);
            CAM_PTR_READ_REGS(this->m_pDrv,AAO_R1_AAO_BASE_ADDR,&pIspRegMap,moduleSize);
        }
        if (this->m_UpdateFuncBit & eIspTuningMgrFunc_AF) {

            this->m_pDrv->getCQModuleInfo(CAM_ISP_AF_, addrOffset, moduleSize);
            CAM_PTR_READ_REGS(this->m_pDrv,AF_R1_AF_CON,&pIspRegMap,moduleSize);
            this->m_pDrv->getCQModuleInfo(CAM_DMA_AFO_, addrOffset, moduleSize);
            CAM_PTR_READ_REGS(this->m_pDrv,AFO_R1_AFO_XSIZE,&pIspRegMap,moduleSize);
            this->m_pDrv->getCQModuleInfo(CAM_DMA_AFO_BA, addrOffset, moduleSize);
            CAM_PTR_READ_REGS(this->m_pDrv,AFO_R1_AFO_BASE_ADDR,&pIspRegMap,moduleSize);
        }
        if (this->m_UpdateFuncBit & eIspTuningMgrFunc_FLK) {

            this->m_pDrv->getCQModuleInfo(CAM_ISP_FLK_A_, addrOffset, moduleSize);
            CAM_PTR_READ_REGS(this->m_pDrv,FLK_R1_FLK_CON,&pIspRegMap,moduleSize);
            this->m_pDrv->getCQModuleInfo(CAM_DMA_FLKO_, addrOffset, moduleSize);
            CAM_PTR_READ_REGS(this->m_pDrv,FLKO_R1_FLKO_XSIZE,&pIspRegMap,moduleSize);
            this->m_pDrv->getCQModuleInfo(CAM_DMA_FLKO_BA, addrOffset, moduleSize);
            CAM_PTR_READ_REGS(this->m_pDrv,FLKO_R1_FLKO_BASE_ADDR,&pIspRegMap,moduleSize);
        }
        if (this->m_UpdateFuncBit & eIspTuningMgrFunc_MOBC_R2) {

            this->m_pDrv->getCQModuleInfo(CAM_ISP_MOBC_R2_, addrOffset, moduleSize);
            CAM_PTR_READ_REGS(this->m_pDrv,MOBC_R2_MOBC_OFFST0,&pIspRegMap,moduleSize);
        }

        if (this->m_UpdateFuncBit & eIspTuningMgrFunc_WB) {

            this->m_pDrv->getCQModuleInfo(CAM_ISP_WB_, addrOffset, moduleSize);
            CAM_PTR_READ_REGS(this->m_pDrv,WB_R1_WB_GAIN_1,&pIspRegMap,moduleSize);
        }
        if (this->m_UpdateFuncBit & eIspTuningMgrFunc_DGN) {

            this->m_pDrv->getCQModuleInfo(CAM_ISP_DGN_, addrOffset, moduleSize);
            CAM_PTR_READ_REGS(this->m_pDrv,DGN_R1_DGN_GN0,&pIspRegMap,moduleSize);
        }
        if (this->m_UpdateFuncBit & eIspTuningMgrFunc_TSFS) {

            this->m_pDrv->getCQModuleInfo(CAM_ISP_TSFS_, addrOffset, moduleSize);
            CAM_PTR_READ_REGS(this->m_pDrv,TSFS_R1_TSFS_ORG,&pIspRegMap,moduleSize);
            this->m_pDrv->getCQModuleInfo(CAM_DMA_TSFSO_, addrOffset, moduleSize);
            CAM_PTR_READ_REGS(this->m_pDrv,TSFSO_R1_TSFSO_XSIZE,&pIspRegMap,moduleSize);
            this->m_pDrv->getCQModuleInfo(CAM_DMA_TSFSO_BA_, addrOffset, moduleSize);
            CAM_PTR_READ_REGS(this->m_pDrv,TSFSO_R1_TSFSO_BASE_ADDR,&pIspRegMap,moduleSize);
        }
        if (this->m_UpdateFuncBit & eIspTuningMgrFunc_DM) {

            this->m_pDrv->getCQModuleInfo(CAM_ISP_DM_, addrOffset, moduleSize);
            CAM_PTR_READ_REGS(this->m_pDrv,DM_R1_DM_INTP_CRS,&pIspRegMap,moduleSize);
        }
        if (this->m_UpdateFuncBit & eIspTuningMgrFunc_FLC) {

            this->m_pDrv->getCQModuleInfo(CAM_ISP_FLC_, addrOffset, moduleSize);
            CAM_PTR_READ_REGS(this->m_pDrv,FLC_R1_FLC_OFST_RB,&pIspRegMap,moduleSize);
        }
        if (this->m_UpdateFuncBit & eIspTuningMgrFunc_CCM) {

            this->m_pDrv->getCQModuleInfo(CAM_ISP_CCM_, addrOffset, moduleSize);
            CAM_PTR_READ_REGS(this->m_pDrv,CCM_R1_CCM_CNV_1,&pIspRegMap,moduleSize);
        }
        if (this->m_UpdateFuncBit & eIspTuningMgrFunc_GGM) {

            this->m_pDrv->getCQModuleInfo(CAM_ISP_GGM_, addrOffset, moduleSize);
            CAM_PTR_READ_REGS(this->m_pDrv,GGM_R1_GGM_LUT,&pIspRegMap,moduleSize);
            this->m_pDrv->getCQModuleInfo(CAM_ISP_GGM_PINGPONG_, addrOffset, moduleSize);
            CAM_PTR_READ_REGS(this->m_pDrv,GGM_R1_GGM_SRAM_PINGPONG,&pIspRegMap,moduleSize);
        }
        if (this->m_UpdateFuncBit & eIspTuningMgrFunc_GGM_R2) {

            this->m_pDrv->getCQModuleInfo(CAM_ISP_GGM_R2_, addrOffset, moduleSize);
            CAM_PTR_READ_REGS(this->m_pDrv,GGM_R2_GGM_LUT,&pIspRegMap,moduleSize);
            this->m_pDrv->getCQModuleInfo(CAM_ISP_GGM_R2_PINGPONG_, addrOffset, moduleSize);
            CAM_PTR_READ_REGS(this->m_pDrv,GGM_R2_GGM_SRAM_PINGPONG,&pIspRegMap,moduleSize);

        }
        if (this->m_UpdateFuncBit & eIspTuningMgrFunc_G2C) {

            this->m_pDrv->getCQModuleInfo(CAM_ISP_G2C_, addrOffset, moduleSize);
            CAM_PTR_READ_REGS(this->m_pDrv,G2C_R1_G2C_CONV_0A,&pIspRegMap,moduleSize);
        }
        if (this->m_UpdateFuncBit & eIspTuningMgrFunc_G2C_R2) {

            this->m_pDrv->getCQModuleInfo(CAM_ISP_G2C_R2_, addrOffset, moduleSize);
            CAM_PTR_READ_REGS(this->m_pDrv,G2C_R2_G2C_CONV_0A,&pIspRegMap,moduleSize);
        }
        if (this->m_UpdateFuncBit & eIspTuningMgrFunc_YNRS) {

            this->m_pDrv->getCQModuleInfo(CAM_ISP_YNRS_, addrOffset, moduleSize);
            CAM_PTR_READ_REGS(this->m_pDrv,YNRS_R1_YNRS_CON1,&pIspRegMap,moduleSize);
            this->m_pDrv->getCQModuleInfo(CAM_ISP_YNRS_PINGPONG_, addrOffset, moduleSize);
            CAM_PTR_READ_REGS(this->m_pDrv,YNRS_R1_YNRS_SRAM_PINGPONG,&pIspRegMap,moduleSize);
        }
        if (this->m_UpdateFuncBit & eIspTuningMgrFunc_RSS_R2) {

            this->m_pDrv->getCQModuleInfo(CAM_ISP_RSS_R2_, addrOffset, moduleSize);
            CAM_PTR_READ_REGS(this->m_pDrv,RSS_R2_RSS_CONTROL,&pIspRegMap,moduleSize);
            this->m_pDrv->getCQModuleInfo(CAM_DMA_RSSO_A_, addrOffset, moduleSize);
            CAM_PTR_READ_REGS(this->m_pDrv,RSSO_R1_RSSO_XSIZE,&pIspRegMap,moduleSize);
            this->m_pDrv->getCQModuleInfo(CAM_DMA_RSSO_A_BA, addrOffset, moduleSize);
            CAM_PTR_READ_REGS(this->m_pDrv,RSSO_R1_RSSO_BASE_ADDR,&pIspRegMap,moduleSize);
        }
        if (this->m_UpdateFuncBit & eIspTuningMgrFunc_SLK) {

            this->m_pDrv->getCQModuleInfo(CAM_ISP_SLK_CEN, addrOffset, moduleSize);
            CAM_PTR_READ_REGS(this->m_pDrv,SLK_R1_SLK_CEN,&pIspRegMap,moduleSize);
            this->m_pDrv->getCQModuleInfo(CAM_ISP_SLK_XOFF, addrOffset, moduleSize);
            CAM_PTR_READ_REGS(this->m_pDrv,SLK_R1_SLK_XOFF,&pIspRegMap,moduleSize);
            this->m_pDrv->getCQModuleInfo(CAM_ISP_SLK_CON, addrOffset, moduleSize);
            CAM_PTR_READ_REGS(this->m_pDrv,SLK_R1_SLK_SLP_CON0,&pIspRegMap,moduleSize);
            this->m_pDrv->getCQModuleInfo(CAM_ISP_SLK_SIZE, addrOffset, moduleSize);
            CAM_PTR_READ_REGS(this->m_pDrv,SLK_R1_SLK_SIZE,&pIspRegMap,moduleSize);
        }
        if (this->m_UpdateFuncBit & eIspTuningMgrFunc_SLK_R2) {

            this->m_pDrv->getCQModuleInfo(CAM_ISP_SLK_R2_CEN, addrOffset, moduleSize);
            CAM_PTR_READ_REGS(this->m_pDrv,SLK_R2_SLK_CEN,&pIspRegMap,moduleSize);
            this->m_pDrv->getCQModuleInfo(CAM_ISP_SLK_R2_XOFF, addrOffset, moduleSize);
            CAM_PTR_READ_REGS(this->m_pDrv,SLK_R2_SLK_XOFF,&pIspRegMap,moduleSize);
            this->m_pDrv->getCQModuleInfo(CAM_ISP_SLK_R2_CON, addrOffset, moduleSize);
            CAM_PTR_READ_REGS(this->m_pDrv,SLK_R2_SLK_SLP_CON0,&pIspRegMap,moduleSize);
            this->m_pDrv->getCQModuleInfo(CAM_ISP_SLK_R2_SIZE, addrOffset, moduleSize);
            CAM_PTR_READ_REGS(this->m_pDrv,SLK_R2_SLK_SIZE,&pIspRegMap,moduleSize);
        }
        if (this->m_UpdateFuncBit & eIspTuningMgrFunc_BPCI) {

            this->m_pDrv->getCQModuleInfo(CAM_DMA_BPCI_, addrOffset, moduleSize);
            CAM_PTR_READ_REGS(this->m_pDrv,BPCI_R1_BPCI_BASE_ADDR,&pIspRegMap,moduleSize);
        }

        if (this->m_UpdateFuncBit & eIspTuningMgrFunc_LCES) {

            this->m_pDrv->getCQModuleInfo(CAM_ISP_LCS_LOG_, addrOffset, moduleSize);
            CAM_PTR_READ_REGS(this->m_pDrv,LCES_R1_LCES_LOG,&pIspRegMap,moduleSize);
            this->m_pDrv->getCQModuleInfo(CAM_ISP_LCS_FLC0_, addrOffset, moduleSize);
            CAM_PTR_READ_REGS(this->m_pDrv,LCES_R1_LCES_FLC0,&pIspRegMap,moduleSize);
            this->m_pDrv->getCQModuleInfo(CAM_ISP_LCS_SIZE, addrOffset, moduleSize);
            CAM_PTR_READ_REGS(this->m_pDrv,LCES_R1_LCES_IN_SIZE,&pIspRegMap,moduleSize);
            this->m_pDrv->getCQModuleInfo(CAM_ISP_LCS_LRZR, addrOffset, moduleSize);
            CAM_PTR_READ_REGS(this->m_pDrv,LCES_R1_LCES_LRZRX,&pIspRegMap,moduleSize);
        }

        if (this->m_UpdateFuncBit & eIspTuningMgrFunc_LTM) {

            this->m_pDrv->getCQModuleInfo(CAM_ISP_LTM_MAX_DIV, addrOffset, moduleSize);
            CAM_PTR_READ_REGS(this->m_pDrv,LTM_R1_LTM_MAX_DIV,&pIspRegMap,moduleSize);
        }

        if (this->m_UpdateFuncBit & eIspTuningMgrFunc_LTMTC_R1_CURVE) {
            MUINT32 module_adr_LTMTC_CURVE_1 = 0;
            this->m_pDrv->getCQModuleInfo(CAM_ISP_LTMTC_CURVE_1, addrOffset, moduleSize);
            CAM_PTR_READ_REGS(this->m_pDrv,LTMTC_R1_LTMTC_CURVE,&pIspRegMap,moduleSize);

            module_adr_LTMTC_CURVE_1 = addrOffset;
            this->m_pDrv->getCQModuleInfo(CAM_ISP_LTMTC_CURVE_2, addrOffset, moduleSize);
            CAM_PTR_READ_REGS_DIVIDED_ARRAY(this->m_pDrv, LTMTC_R1_LTMTC_CURVE,
                &pIspRegMap, moduleSize, (addrOffset-module_adr_LTMTC_CURVE_1));

            this->m_pDrv->getCQModuleInfo(CAM_ISP_LTM_PINGPONG_, addrOffset, moduleSize);
            CAM_PTR_READ_REGS(this->m_pDrv,LTM_R1_LTM_SRAM_PINGPONG,&pIspRegMap,moduleSize);
        }
        if (this->m_UpdateFuncBit & eIspTuningMgrFunc_HLR) {

            this->m_pDrv->getCQModuleInfo(CAM_ISP_HLR_, addrOffset, moduleSize);
            CAM_PTR_READ_REGS(this->m_pDrv,HLR_R1_HLR_EST_Y0,&pIspRegMap,moduleSize);

            this->m_pDrv->getCQModuleInfo(CAM_CTL_HLR_LKMSB_, addrOffset, moduleSize);
            CAM_PTR_READ_REGS(this->m_pDrv,CAMCTL_R1_CAMCTL_HLR_LKMSB,&pIspRegMap,moduleSize);
        }

        /* default dump: QBIN */
        this->m_pDrv->getCQModuleInfo(CAM_ISP_QBIN_, addrOffset, moduleSize);
        CAM_PTR_READ_REGS(this->m_pDrv,QBIN_R1_QBIN_CTL,&pIspRegMap,moduleSize);
        this->m_pDrv->getCQModuleInfo(CAM_ISP_QBIN_R2_, addrOffset, moduleSize);
        CAM_PTR_READ_REGS(this->m_pDrv,QBIN_R2_QBIN_CTL,&pIspRegMap,moduleSize);
        this->m_pDrv->getCQModuleInfo(CAM_ISP_QBIN_R4_, addrOffset, moduleSize);
        CAM_PTR_READ_REGS(this->m_pDrv,QBIN_R4_QBIN_CTL,&pIspRegMap,moduleSize);
        this->m_pDrv->getCQModuleInfo(CAM_ISP_QBIN_R5_, addrOffset, moduleSize);
        CAM_PTR_READ_REGS(this->m_pDrv,QBIN_R5_QBIN_CTL,&pIspRegMap,moduleSize);
        /* default dump: RRZ */
        this->m_pDrv->getCQModuleInfo(CAM_ISP_RRZ_, addrOffset, moduleSize);
        CAM_PTR_READ_REGS(this->m_pDrv,RRZ_R1_RRZ_CTL,&pIspRegMap,moduleSize);
        this->m_pDrv->getCQModuleInfo(CAM_DMA_RRZO_BA, addrOffset, moduleSize);
        CAM_PTR_READ_REGS(this->m_pDrv,RRZO_R1_RRZO_BASE_ADDR,&pIspRegMap,moduleSize);
        this->m_pDrv->getCQModuleInfo(CAM_DMA_RRZO_X_, addrOffset, moduleSize);
        CAM_PTR_READ_REGS(this->m_pDrv,RRZO_R1_RRZO_XSIZE,&pIspRegMap,moduleSize);
        this->m_pDrv->getCQModuleInfo(CAM_DMA_RRZO_Y_, addrOffset, moduleSize);
        CAM_PTR_READ_REGS(this->m_pDrv,RRZO_R1_RRZO_YSIZE,&pIspRegMap,moduleSize);
        this->m_pDrv->getCQModuleInfo(CAM_DMA_RRZO_STRIDE_, addrOffset, moduleSize);
        CAM_PTR_READ_REGS(this->m_pDrv,RRZO_R1_RRZO_STRIDE,&pIspRegMap,moduleSize);
        this->m_pDrv->getCQModuleInfo(CAM_DMA_RRZO_CROP_, addrOffset, moduleSize);
        CAM_PTR_READ_REGS(this->m_pDrv,RRZO_R1_RRZO_CROP,&pIspRegMap,moduleSize);
        /* default dump: C42 */
        this->m_pDrv->getCQModuleInfo(CAM_ISP_C42_, addrOffset, moduleSize);
        CAM_PTR_READ_REGS(this->m_pDrv,C42_R1_C42_CON,&pIspRegMap,moduleSize);
        this->m_pDrv->getCQModuleInfo(CAM_ISP_C42_R2_, addrOffset, moduleSize);
        CAM_PTR_READ_REGS(this->m_pDrv,C42_R2_C42_CON,&pIspRegMap,moduleSize);
        /* default dump: CRZ */
        this->m_pDrv->getCQModuleInfo(CAM_ISP_CRZ_, addrOffset, moduleSize);
        CAM_PTR_READ_REGS(this->m_pDrv,CRZ_R1_CRZ_CONTROL,&pIspRegMap,moduleSize);
        this->m_pDrv->getCQModuleInfo(CAM_DMA_CRZO_BA_, addrOffset, moduleSize);
        CAM_PTR_READ_REGS(this->m_pDrv,CRZO_R1_CRZO_BASE_ADDR,&pIspRegMap,moduleSize);
        this->m_pDrv->getCQModuleInfo(CAM_DMA_CRZO, addrOffset, moduleSize);
        CAM_PTR_READ_REGS(this->m_pDrv,CRZO_R1_CRZO_XSIZE,&pIspRegMap,moduleSize);
        this->m_pDrv->getCQModuleInfo(CAM_ISP_CRZ_R2_, addrOffset, moduleSize);
        CAM_PTR_READ_REGS(this->m_pDrv,CRZ_R2_CRZ_CONTROL,&pIspRegMap,moduleSize);
        this->m_pDrv->getCQModuleInfo(CAM_DMA_CRZO_R2_BA_, addrOffset, moduleSize);
        CAM_PTR_READ_REGS(this->m_pDrv,CRZO_R2_CRZO_BASE_ADDR,&pIspRegMap,moduleSize);
        this->m_pDrv->getCQModuleInfo(CAM_DMA_CRZO_R2_, addrOffset, moduleSize);
        CAM_PTR_READ_REGS(this->m_pDrv,CRZO_R2_CRZO_XSIZE,&pIspRegMap,moduleSize);
        /* default dump: CRSP, CRP */
        this->m_pDrv->getCQModuleInfo(CAM_ISP_CRSP_, addrOffset, moduleSize);
        CAM_PTR_READ_REGS(this->m_pDrv,CRSP_R1_CRSP_CTRL,&pIspRegMap,moduleSize);
        this->m_pDrv->getCQModuleInfo(CAM_ISP_CRSP_R2_, addrOffset, moduleSize);
        CAM_PTR_READ_REGS(this->m_pDrv,CRSP_R2_CRSP_CTRL,&pIspRegMap,moduleSize);
        this->m_pDrv->getCQModuleInfo(CAM_ISP_CRP_R4_, addrOffset, moduleSize);
        CAM_PTR_READ_REGS(this->m_pDrv,CRP_R4_CRP_X_POS,&pIspRegMap,moduleSize);
        /* default dump: CTL_EN, CTL_SEL */
        this->m_pDrv->getCQModuleInfo(CAM_CTL_EN_, addrOffset, moduleSize);
        CAM_PTR_READ_REGS(this->m_pDrv,CAMCTL_R1_CAMCTL_EN,&pIspRegMap,moduleSize);
        this->m_pDrv->getCQModuleInfo(CAM_CTL_SEL_, addrOffset, moduleSize);
        CAM_PTR_READ_REGS(this->m_pDrv,CAMCTL_R1_CAMCTL_SEL,&pIspRegMap,moduleSize);

        this->CBTimeChk(_str, MFALSE);

        CAM_FUNC_INF("Start dump reg for ISP P1 settings %d.\n", func_cam_DbgLogEnable_DEBUG);

        if ( func_cam_DbgLogEnable_DEBUG == MTRUE) {
            char _tmpchr[16] = "\0";
            char _chr[256] = "\0";
            CAM_FUNC_INF("###################\n");
            CAM_FUNC_INF("Start dump reg for ISP P1 settings.\n");
            for (MUINT32 i=0x0;i<CAM_BASE_RANGE;i+=0x20) {
                _chr[0] = '\0';
                sprintf(_chr,"0x%x: ",0x10000 + i);
                for (MUINT32 j=i;j<(i+0x20);j+=0x4) {
                    if (j >= CAM_BASE_RANGE) {
                        break;
                    }
                    _tmpchr[0] = '\0';
                    sprintf(_tmpchr,"0x%8x - ",pIspRegMap[(j>>2)]);
                    strncat(_chr,_tmpchr, strlen(_tmpchr));
                }
                CAM_FUNC_INF("%s\n",_chr);
            }
            CAM_FUNC_INF("###################\n");
        }

        this->m_dumpInput.pReg = (void *)pIspRegMap;

        this->CBTimeChk(_str, MTRUE);
        this->m_pP1Tuning->p1TuningNotify((MVOID*)&this->m_dumpInput, NULL);
        this->CBTimeChk(_str, MFALSE);

        //free
        if (!_dpFrameCnt && this->m_dumpInput.pReg){//last frame -> free
            free(this->m_dumpInput.pReg);
            pIspRegMap = NULL;
            this->m_dumpInput.pReg = NULL;
            this->m_dumpInput.magic = 0;
        }

        //dump CQ for debug
        if ( MTRUE == func_cam_DbgLogEnable_DEBUG) {
            this->m_pDrv->dumpCQTable();
        }

        if ( MTRUE == func_cam_DbgLogEnable_DEBUG) {
            this->m_pDrv->DumpReg(this->m_hwModule);
        }
    }

    CAM_FUNC_DBG("-\n");
    return ret;
}

MBOOL CAM_TUNING_CTRL::setSLK(void)
{
    MBOOL ret = MTRUE;
    RRZ_PARAMETER rCropRz;
    REG_TG_R1_TG_SEN_GRAB_PXL TG_W;
    REG_TG_R1_TG_SEN_GRAB_LIN TG_H;

    this->m_pDrv = (ISP_DRV_CAM*)this->m_pIspDrv;

    MINT32 SL2_CENTR_X  = 0;
    MINT32 SL2_CENTR_Y  = 0;

    MINT32 SL2_HRZ_COM = 2048; // 11bits MSB=1, only affect by RRZ, algo confirm.
    MINT32 SL2_VRZ_COM = 2048; // 11bits MSB=1, only affect by RRZ, algo confirm.
    MUINT32 SL2_TPIPE_WD = 0, SL2_TPIPE_HT = 0;

    MUINT32 tmp = 0;
    RectData<MUINT32> Bin;
    Header_RRZO fh_rrzo;

    GET_FULL_BIN(this->m_pDrv, Bin, 2);

    TG_W.Raw = CAM_READ_REG(this->m_pDrv, TG_R1_TG_SEN_GRAB_PXL);
    TG_H.Raw = CAM_READ_REG(this->m_pDrv, TG_R1_TG_SEN_GRAB_LIN);

    rCropRz.src_w  = (TG_W.Bits.TG_PXL_E - TG_W.Bits.TG_PXL_S) >> Bin.H;
    rCropRz.src_h  = (TG_H.Bits.TG_LIN_E - TG_H.Bits.TG_LIN_S) >> Bin.V;
    rCropRz.src_x  = CAM_READ_BITS(this->m_pDrv, RRZ_R1_RRZ_HORI_INT_OFST, RRZ_HORI_INT_OFST);
    rCropRz.src_y  = CAM_READ_BITS(this->m_pDrv, RRZ_R1_RRZ_VERT_INT_OFST, RRZ_VERT_INT_OFST);
    tmp            = fh_rrzo.GetRegInfo(Header_RRZO::E_RRZ_CRP_SIZE,this->m_pDrv);
    rCropRz.crop_w = tmp & 0xffff;
    rCropRz.crop_h = (tmp>>16);
    tmp            = fh_rrzo.GetRegInfo(Header_RRZO::E_RRZ_DST_SIZE,this->m_pDrv);
    rCropRz.tar_w  = tmp & 0xffff;
    rCropRz.tar_h  = (tmp>>16);

    if (rCropRz.tar_w == 0 || rCropRz.tar_h == 0) {
        CAM_FUNC_DBG("Unexpected rrz info: rrz width(%d), rrz hight(%d)",
            rCropRz.tar_w, rCropRz.tar_h);
        return MFALSE;
    }

    if (rCropRz.crop_w == 0 || rCropRz.crop_h == 0) {
        CAM_FUNC_DBG("Unexpected rrz crop info: rrz crop width(%d), rrz crop hight(%d)",
            rCropRz.crop_w, rCropRz.crop_h);
        return MFALSE;
    }

    SL2_CENTR_X   = CAM_READ_BITS(this->m_pDrv, SLK_R1_SLK_CEN, SLK_CENTR_X);
    SL2_CENTR_Y   = CAM_READ_BITS(this->m_pDrv, SLK_R1_SLK_CEN, SLK_CENTR_Y);

    CAM_FUNC_DBG("TG_W.Raw:%d, TG_H.Raw:%d "
                 "rCropRz.src_w:%d rCropRz.src_h:%d "
                 "rCropRz.src_x:%d rCropRz.src_y:%d "
                 "rCropRz.crop_w:%d rCropRz.crop_h:%d "
                 "rCropRz.tar_w:%d rCropRz.tar_h:%d "
                 "SL2_CENTR_X:%d SL2_CENTR_Y:%d ",
                 TG_W.Raw, TG_H.Raw,
                 rCropRz.src_w, rCropRz.src_h,
                 rCropRz.src_x, rCropRz.src_y,
                 rCropRz.crop_w,rCropRz.crop_h,
                 rCropRz.tar_w, rCropRz.tar_h,
                 SL2_CENTR_X, SL2_CENTR_Y);

    // decide to update SLK module.
    if((this->SLKUpdateStatus[SLK_R1_Update] == MTRUE) ||
       (this->SLKUpdateStatus[SLK_R2_Update] == MTRUE)) {
        // decide to go throught RRZ module
        if((CAM_READ_REG(this->m_pDrv, CAMCTL_R1_CAMCTL_SEL2) & DM_R1_SEL_1) ==
            DM_R1_SEL_1) {
            // go throught RRZ module
            MFLOAT fRzRtoX = (MFLOAT)rCropRz.tar_w / (MFLOAT)rCropRz.crop_w;
            MFLOAT fRzRtoY = (MFLOAT)rCropRz.tar_h / (MFLOAT)rCropRz.crop_h;
            SL2_CENTR_X    = (SL2_CENTR_X - rCropRz.src_x);
            SL2_CENTR_Y    = (SL2_CENTR_Y - rCropRz.src_y);
            SL2_HRZ_COM    = (MINT32)((MFLOAT)SL2_HRZ_COM / fRzRtoX);
            SL2_VRZ_COM    = (MINT32)((MFLOAT)SL2_VRZ_COM / fRzRtoY);
            SL2_TPIPE_WD   =  rCropRz.tar_w;
            SL2_TPIPE_HT   =  rCropRz.tar_h;

            CAM_FUNC_DBG("setSLK use formula value to update "
                         "fRzRtoX=%f fRzRtoY=%f "
                         "SL2_CENTR_X=%d, SL2_CENTR_Y=%d "
                         "SL2_HRZ_COM=%d, SL2_VRZ_COM=%d "
                         "SL2_TPIPE_WD=%d, SL2_TPIPE_HT=%d "
                         "SLKUpdateStatus[0] = %d, SLKUpdateStatus[1] = %d ",
                         fRzRtoX, fRzRtoY,
                         SL2_CENTR_X, SL2_CENTR_Y,
                         SL2_HRZ_COM, SL2_VRZ_COM,
                         SL2_TPIPE_WD,SL2_TPIPE_HT,
                         this->SLKUpdateStatus[SLK_R1_Update],
                         this->SLKUpdateStatus[SLK_R2_Update]);
        } else {
            // don't go throught RRZ module
            SL2_TPIPE_WD   =  rCropRz.src_w;
            SL2_TPIPE_HT   =  rCropRz.src_h;

            CAM_FUNC_DBG("setSLK use default value to update "
                         "SL2_CENTR_X=%d, SL2_CENTR_Y=%d "
                         "SL2_HRZ_COM=%d, SL2_VRZ_COM=%d "
                         "SL2_TPIPE_WD=%d, SL2_TPIPE_HT=%d "
                         "SLKUpdateStatus[0] = %d, SLKUpdateStatus[1] = %d ",
                         SL2_CENTR_X, SL2_CENTR_Y,
                         SL2_HRZ_COM, SL2_VRZ_COM,
                         SL2_TPIPE_WD,SL2_TPIPE_HT,
                         this->SLKUpdateStatus[SLK_R1_Update],
                         this->SLKUpdateStatus[SLK_R2_Update]);
        }
        if(this->SLKUpdateStatus[SLK_R1_Update] == MTRUE) {

            CAM_FUNC_DBG("SLK_R1 "
                         "SL2_CENTR_X=%d, SL2_CENTR_Y=%d "
                         "SL2_HRZ_COM=%d, SL2_VRZ_COM=%d "
                         "SL2_TPIPE_WD=%d, SL2_TPIPE_HT=%d ",
                         SL2_CENTR_X, SL2_CENTR_Y,
                         SL2_HRZ_COM, SL2_VRZ_COM,
                         SL2_TPIPE_WD,SL2_TPIPE_HT);

            CAM_WRITE_BITS(this->m_pDrv, SLK_R1_SLK_CEN,  SLK_CENTR_X,  SL2_CENTR_X);
            CAM_WRITE_BITS(this->m_pDrv, SLK_R1_SLK_CEN,  SLK_CENTR_Y,  SL2_CENTR_Y);
            CAM_WRITE_BITS(this->m_pDrv, SLK_R1_SLK_RZ,   SLK_HRZ_COMP, SL2_HRZ_COM);
            CAM_WRITE_BITS(this->m_pDrv, SLK_R1_SLK_RZ,   SLK_VRZ_COMP, SL2_VRZ_COM);
            CAM_WRITE_BITS(this->m_pDrv, SLK_R1_SLK_SIZE, SLK_TPIPE_WD, SL2_TPIPE_WD);
            CAM_WRITE_BITS(this->m_pDrv, SLK_R1_SLK_SIZE, SLK_TPIPE_HT, SL2_TPIPE_HT);
        }

        if(this->SLKUpdateStatus[SLK_R2_Update] == MTRUE) {
            CAM_FUNC_DBG("SLK_R2 "
                         "SL2_CENTR_X=%d, SL2_CENTR_Y=%d "
                         "SL2_HRZ_COM=%d, SL2_VRZ_COM=%d "
                         "SL2_TPIPE_WD=%d, SL2_TPIPE_HT=%d ",
                         SL2_CENTR_X, SL2_CENTR_Y,
                         SL2_HRZ_COM, SL2_VRZ_COM,
                         SL2_TPIPE_WD,SL2_TPIPE_HT);

            CAM_WRITE_BITS(this->m_pDrv, SLK_R2_SLK_CEN,  SLK_CENTR_X,  SL2_CENTR_X);
            CAM_WRITE_BITS(this->m_pDrv, SLK_R2_SLK_CEN,  SLK_CENTR_Y,  SL2_CENTR_Y);
            CAM_WRITE_BITS(this->m_pDrv, SLK_R2_SLK_RZ,   SLK_HRZ_COMP, SL2_HRZ_COM);
            CAM_WRITE_BITS(this->m_pDrv, SLK_R2_SLK_RZ,   SLK_VRZ_COMP, SL2_VRZ_COM);
            CAM_WRITE_BITS(this->m_pDrv, SLK_R2_SLK_SIZE, SLK_TPIPE_WD, SL2_TPIPE_WD);
            CAM_WRITE_BITS(this->m_pDrv, SLK_R2_SLK_SIZE, SLK_TPIPE_HT, SL2_TPIPE_HT);
        }
    } else {
        CAM_FUNC_DBG("SLK don't update "
                     "SLKUpdateStatus[0] = %d, SLKUpdateStatus[1] = %d ",
                     this->SLKUpdateStatus[SLK_R1_Update],
                     this->SLKUpdateStatus[SLK_R2_Update]);
    }

    return ret;
}

MBOOL CAM_TUNING_CTRL::setNotifyDone(void)
{
    MBOOL ret = MTRUE;

    this->m_pDrv = (ISP_DRV_CAM*)this->m_pIspDrv;

    if (this->m_NotifyFrame) {
        this->m_NotifyFrame--;
    }
    this->m_CurBin = CAM_READ_BITS(this->m_pDrv, CAMCTL_R1_CAMCTL_EN2, CAMCTL_BIN_R1_EN);
    this->m_CurQbn1Acc = (CAM_C == this->m_hwModule) ? _1_pix_ : CAM_READ_BITS(this->m_pDrv, QBIN_R1_QBIN_CTL, QBIN_ACC);

    return ret;
}

void CAM_TUNING_CTRL::updateTuningDrv(MUINT32 senDev)
{
    this->m_SenDev = senDev;
    this->m_pTuningDrv = (void*)TuningDrv::getInstance(this->m_SenDev);;
}

inline void CAM_TUNING_CTRL::CBTimeChk(char *str, MBOOL bStartT)
{
    struct timespec ts;
    div_t divResult;
    static MUINT32 TimehourS, TimeMinS, TimeSecS, TimeUsS;
    MUINT32 Timehour = 0, TimeMin = 0, TimeSec = 0, TimeUs = 0;

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

void CAM_TUNING_CTRL::dumpTuningRegBuf(
    void *vpTuningNodesPage,
    const char *moduleName,
    MUINT32 module_size,
    MUINT32 module_adr)
{
    const MUINT32 STR_UNIT = 11; //ex:0x12345678-
    const MUINT32 __MAX_DUMP_STR_LENGTH__ = STR_UNIT * 16;

    string mainstr;
    string substr;
    char _tmp[16] = {"\0"};

    stTuningNode *lvpTuningNodesPage = (stTuningNode *)vpTuningNodesPage;

    for(MUINT32 i = 0; i < module_size; i++) {
           sprintf(_tmp,"0x%08x-",
                lvpTuningNodesPage->pTuningRegBuf[(module_adr + i * 0x4)>>2]);
           mainstr += _tmp;
    }

    CAM_FUNC_INF("%s :cur size: %lu registerNum:%lu \n", moduleName,
        (unsigned long)mainstr.size(),
        (unsigned long)mainstr.size() / STR_UNIT);


    MUINT32 maxStringSectionSize = mainstr.size() / __MAX_DUMP_STR_LENGTH__;

    for(MUINT32 stringSectionInx = 0; stringSectionInx <= maxStringSectionSize ; stringSectionInx++){
       substr.assign(mainstr.substr(__MAX_DUMP_STR_LENGTH__ * stringSectionInx, __MAX_DUMP_STR_LENGTH__));
       CAM_FUNC_INF("%s\n", substr.c_str());
    }

    substr.clear();
    mainstr.clear();

}

MBOOL CAM_TUNING_CTRL::setLTM(void)
{
    // CCU define parameter depend on platform.
    // this function setting and formulat all come from CCU.
    // must ask CCU owner
    const MUINT32 CCU_LTM_BLK_X_NUM = 12 , CCU_LTM_BLK_Y_NUM = 9;

    E_ISP_CAM_CQ cq;
    MUINT32 page;
    MUINT32 full_w = 0,full_h = 0;
    MUINT32 blk_w = 0, blk_h = 0;
    MUINT32 x_base = 0, x_sb  = 0, y_base  = 0, y_sb = 0, th_base = 0, th_sb = 0;
    MUINT32 temp = 0 ,temp2 = 0;
    MUINT32 ret = MTRUE;
    REG_TG_R1_TG_SEN_GRAB_PXL TG_W;
    REG_TG_R1_TG_SEN_GRAB_LIN TG_H;

    this->m_pDrv = (ISP_DRV_CAM*)this->m_pIspDrv;


    //dbg log
    this->m_pDrv->getCurObjInfo(&this->m_hwModule,&cq,&page);
    CAM_FUNC_INF("+ cq:0x%x,page:0x%x\n",cq,page);

    TG_W.Raw = CAM_READ_REG(this->m_pDrv, TG_R1_TG_SEN_GRAB_PXL);
    TG_H.Raw = CAM_READ_REG(this->m_pDrv, TG_R1_TG_SEN_GRAB_LIN);

    full_w = TG_W.Bits.TG_PXL_E - TG_W.Bits.TG_PXL_S;
    full_h = TG_H.Bits.TG_LIN_E - TG_H.Bits.TG_LIN_S;

    full_w = full_w >> (CAM_READ_BITS(this->m_pDrv,CAMCTL_R1_CAMCTL_EN2,CAMCTL_BIN_R1_EN) + \
                        CAM_READ_BITS(this->m_pDrv,CAMCTL_R1_CAMCTL_EN2,CAMCTL_DBN_R1_EN) + \
                        CAM_READ_BITS(this->m_pDrv,CAMCTL_R1_CAMCTL_EN2,CAMCTL_FBND_R1_EN));

    full_h = full_h >> (CAM_READ_BITS(this->m_pDrv,CAMCTL_R1_CAMCTL_EN2,CAMCTL_BIN_R1_EN) +
                        CAM_READ_BITS(this->m_pDrv,CAMCTL_R1_CAMCTL_EN2,CAMCTL_FBND_R1_EN));

    blk_w = full_w / CCU_LTM_BLK_X_NUM + ((full_w % CCU_LTM_BLK_X_NUM) ? 1 : 0);
    blk_h = full_h / CCU_LTM_BLK_Y_NUM + ((full_h % CCU_LTM_BLK_Y_NUM) ? 1 : 0);

    //LTM_X_ALPHA_BASE LTM_X_ALPHA_BASE_SHIFT_BIT
    temp = ((full_w + 256) >> 9);
    x_sb = 4;
    while(temp != 0)
    {
        temp  = (temp >> 1);
        x_sb++;
    }

    x_base = ((CCU_LTM_BLK_X_NUM << (x_sb + 9)) + (full_w >> 1)) / full_w;

    //LTM_Y_ALPHA_BASE LTM_Y_ALPHA_BASE_SHIFT_BIT

    temp = ((full_h + 256) >> 9);
    y_sb = 4;
    while(temp != 0)
    {
        temp = (temp >> 1);
        y_sb++;
    }

    y_base = ((CCU_LTM_BLK_Y_NUM << (y_sb + 9)) + (full_h >> 1)) / full_h;

    CAM_FUNC_DBG("full_w:0x%08x, full_h:0x%08x, blk_w:0x%08x, blk_h:0x%08x, "
                 "x_base:0x%08x, x_sb:0x%08x y_base:0x%08x, y_sb:0x%08x, "
                 "th_base:0x%08x, th_sb:0x%08x",
                 full_w, full_h, blk_w, blk_h,
                 x_base, x_sb, y_base, y_sb,
                 th_base, th_sb);

    CAM_WRITE_BITS(this->m_pDrv, LTM_R1_LTM_CTRL,    LTM_LUM_MAP_MODE, 1);
    CAM_WRITE_BITS(this->m_pDrv, LTM_R1_LTM_BLK_NUM, LTM_BLK_X_NUM,    CCU_LTM_BLK_X_NUM);
    CAM_WRITE_BITS(this->m_pDrv, LTM_R1_LTM_BLK_NUM, LTM_BLK_Y_NUM,    CCU_LTM_BLK_Y_NUM);
    CAM_WRITE_BITS(this->m_pDrv, LTM_R1_LTM_BLK_SZ,  LTM_BLK_WIDTH,    blk_w);
    CAM_WRITE_BITS(this->m_pDrv, LTM_R1_LTM_BLK_SZ,  LTM_BLK_HEIGHT,   blk_h);

    CAM_WRITE_BITS(this->m_pDrv, LTM_R1_LTM_BLK_DIVX, LTM_X_ALPHA_BASE,           x_base);
    CAM_WRITE_BITS(this->m_pDrv, LTM_R1_LTM_BLK_DIVX, LTM_X_ALPHA_BASE_SHIFT_BIT, x_sb);

    CAM_WRITE_BITS(this->m_pDrv, LTM_R1_LTM_BLK_DIVY, LTM_Y_ALPHA_BASE,           y_base);
    CAM_WRITE_BITS(this->m_pDrv, LTM_R1_LTM_BLK_DIVY, LTM_Y_ALPHA_BASE_SHIFT_BIT, y_sb);

    CAM_WRITE_BITS(this->m_pDrv, LTM_R1_LTM_CLIP, LTM_GAIN_TH, 8);

    CAM_WRITE_BITS(this->m_pDrv, LTM_R1_LTM_TILE_NUM, LTM_TILE_BLK_X_NUM_START,  0);
    CAM_WRITE_BITS(this->m_pDrv, LTM_R1_LTM_TILE_NUM, LTM_TILE_BLK_X_NUM_END,
        ((full_w - 1) / blk_w));
    CAM_WRITE_BITS(this->m_pDrv, LTM_R1_LTM_TILE_CNTX, LTM_TILE_BLK_X_CNT_START, 0);

    temp2 = CAM_READ_BITS(this->m_pDrv, LTM_R1_LTM_TILE_NUM, LTM_TILE_BLK_X_NUM_END);
    CAM_WRITE_BITS(this->m_pDrv, LTM_R1_LTM_TILE_CNTX, LTM_TILE_BLK_X_CNT_END,
        ((full_w - 1) - (blk_w * temp2)));

    CAM_WRITE_BITS(this->m_pDrv, LTM_R1_LTM_TILE_NUM, LTM_TILE_BLK_Y_NUM_START,  0);
    CAM_WRITE_BITS(this->m_pDrv, LTM_R1_LTM_TILE_NUM, LTM_TILE_BLK_Y_NUM_END,
        ((full_h - 1) / blk_h));
    CAM_WRITE_BITS(this->m_pDrv, LTM_R1_LTM_TILE_CNTY, LTM_TILE_BLK_Y_CNT_START, 0);

    temp2 = CAM_READ_BITS(this->m_pDrv, LTM_R1_LTM_TILE_NUM, LTM_TILE_BLK_Y_NUM_END);
    CAM_WRITE_BITS(this->m_pDrv, LTM_R1_LTM_TILE_CNTY, LTM_TILE_BLK_Y_CNT_END,
        ((full_h - 1) - (blk_h * temp2)));


    CAM_WRITE_BITS(this->m_pDrv, LTM_R1_LTM_CFG,   LTM_LTM_ENGINE_EN,  1);
    CAM_WRITE_BITS(this->m_pDrv, LTM_R1_LTM_CFG,   LTM_LTM_CG_DISABLE, 0);
    CAM_WRITE_BITS(this->m_pDrv, LTM_R1_LTM_CFG,   LTM_CHKSUM_EN,      0);
    CAM_WRITE_BITS(this->m_pDrv, LTM_R1_LTM_CFG,   LTM_CHKSUM_SEL,     0);
    CAM_WRITE_BITS(this->m_pDrv, LTM_R1_LTM_RESET, LTM_LTM_RESET,      0);
    CAM_WRITE_BITS(this->m_pDrv, LTM_R1_LTM_INTEN, LTM_IF_END_INT_EN,  0);
    CAM_WRITE_BITS(this->m_pDrv, LTM_R1_LTM_INTEN, LTM_OF_END_INT_EN,  0);

//    CAM_WRITE_BITS(this->m_pDrv, LTM_R1_LTM_INTSTA,       LTM_IF_END_INT,    0);
//    CAM_WRITE_BITS(this->m_pDrv, LTM_R1_LTM_INTSTA,       LTM_OF_END_INT,    0);
//    CAM_WRITE_BITS(this->m_pDrv, LTM_R1_LTM_STATUS,       LTM_IF_UNFINISH,   0);
//    CAM_WRITE_BITS(this->m_pDrv, LTM_R1_LTM_STATUS,       LTM_OF_UNFINISH,   0);
//    CAM_WRITE_BITS(this->m_pDrv, LTM_R1_LTM_STATUS,       LTM_HANDSHAKE,     0);
//    CAM_WRITE_BITS(this->m_pDrv, LTM_R1_LTM_INPUT_COUNT,  LTM_INP_PIX_CNT,   0);
//    CAM_WRITE_BITS(this->m_pDrv, LTM_R1_LTM_INPUT_COUNT,  LTM_INP_LINE_CNT,  0);
//    CAM_WRITE_BITS(this->m_pDrv, LTM_R1_LTM_OUTPUT_COUNT, LTM_OUTP_PIX_CNT,  0);
//    CAM_WRITE_BITS(this->m_pDrv, LTM_R1_LTM_OUTPUT_COUNT, LTM_OUTP_LINE_CNT, 0);
//    CAM_WRITE_BITS(this->m_pDrv, LTM_R1_LTM_CHKSUM,       LTM_CHKSUM,        0);

    CAM_WRITE_BITS(this->m_pDrv, LTM_R1_LTM_TILE_SIZE, LTM_TILE_HSIZE,   full_w);
    CAM_WRITE_BITS(this->m_pDrv, LTM_R1_LTM_TILE_SIZE, LTM_TILE_VSIZE,   full_h);
    CAM_WRITE_BITS(this->m_pDrv, LTM_R1_LTM_TILE_EDGE, LTM_TILE_EDGE,        15);
    CAM_WRITE_BITS(this->m_pDrv, LTM_R1_LTM_TILE_CROP, LTM_CROP_H,            2); // Use HW default value
    CAM_WRITE_BITS(this->m_pDrv, LTM_R1_LTM_TILE_CROP, LTM_CROP_V,            2); // Use HW default value
    CAM_WRITE_BITS(this->m_pDrv, LTM_R1_LTM_DUMMY_REG, LTM_DUMMY_REG,         0);
    CAM_WRITE_BITS(this->m_pDrv, LTM_R1_LTM_SRAM_CFG,  LTM_FORCE_SRAM_EN,     0);
    CAM_WRITE_BITS(this->m_pDrv, LTM_R1_LTM_SRAM_CFG,  LTM_FORCE_SRAM_APB,    0);
    CAM_WRITE_BITS(this->m_pDrv, LTM_R1_LTM_SRAM_CFG,  LTM_FORCE_SRAM_INT,    0);

//    CAM_WRITE_BITS(this->m_pDrv, LTM_R1_LTM_SRAM_STATUS,  LTM_RB_SRAM_IDX,     0);
//    CAM_WRITE_BITS(this->m_pDrv, LTM_R1_LTM_SRAM_STATUS,  LTM_RB_SRAM_IDX_OUT, 0);

    CAM_WRITE_BITS(this->m_pDrv, LTM_R1_LTM_ATPG,  LTM_ltm_atpg_ob,           0);
    CAM_WRITE_BITS(this->m_pDrv, LTM_R1_LTM_ATPG,  LTM_ltm_atpg_ct,           0);

    CAM_WRITE_BITS(this->m_pDrv, LTM_R1_LTM_SHADOW_CTRL,  LTM_BYPASS_SHADOW,  0);
    CAM_WRITE_BITS(this->m_pDrv, LTM_R1_LTM_SHADOW_CTRL,  LTM_FORCE_COMMIT,   0);

    CAM_WRITE_BITS(this->m_pDrv, LTM_R1_LTM_SELRGB_GRAD0,  LTM_SELRGB_GRAD0,  0);
    CAM_WRITE_BITS(this->m_pDrv, LTM_R1_LTM_SELRGB_GRAD0,  LTM_SELRGB_GRAD1,  0);

    CAM_WRITE_BITS(this->m_pDrv, LTM_R1_LTM_SELRGB_GRAD1,  LTM_SELRGB_GRAD2,  0);
    CAM_WRITE_BITS(this->m_pDrv, LTM_R1_LTM_SELRGB_GRAD1,  LTM_SELRGB_GRAD3,  0);

    CAM_WRITE_BITS(this->m_pDrv, LTM_R1_LTM_SELRGB_GRAD2,  LTM_SELRGB_GRAD4,  0);
    CAM_WRITE_BITS(this->m_pDrv, LTM_R1_LTM_SELRGB_GRAD2,  LTM_SELRGB_GRAD5,  0);

    CAM_WRITE_BITS(this->m_pDrv, LTM_R1_LTM_SELRGB_GRAD3,  LTM_SELRGB_GRAD6,  0);
    CAM_WRITE_BITS(this->m_pDrv, LTM_R1_LTM_SELRGB_GRAD3,  LTM_SELRGB_GRAD7,  0);

    CAM_WRITE_BITS(this->m_pDrv, LTM_R1_LTM_SELRGB_TH0,  LTM_SELRGB_TH0,     64);
    CAM_WRITE_BITS(this->m_pDrv, LTM_R1_LTM_SELRGB_TH0,  LTM_SELRGB_TH1,     64);

    CAM_WRITE_BITS(this->m_pDrv, LTM_R1_LTM_SELRGB_TH1,  LTM_SELRGB_TH2,     64);
    CAM_WRITE_BITS(this->m_pDrv, LTM_R1_LTM_SELRGB_TH1,  LTM_SELRGB_TH3,     64);

    CAM_WRITE_BITS(this->m_pDrv, LTM_R1_LTM_SELRGB_TH2,  LTM_SELRGB_TH4,     64);
    CAM_WRITE_BITS(this->m_pDrv, LTM_R1_LTM_SELRGB_TH2,  LTM_SELRGB_TH5,     64);

    CAM_WRITE_BITS(this->m_pDrv, LTM_R1_LTM_SELRGB_TH3,  LTM_SELRGB_TH6,     64);
    CAM_WRITE_BITS(this->m_pDrv, LTM_R1_LTM_SELRGB_TH3,  LTM_SELRGB_TH7,     64);

    CAM_WRITE_BITS(this->m_pDrv, LTM_R1_LTM_SELRGB_SLP0,  LTM_SELRGB_SLP0,    0);
    CAM_WRITE_BITS(this->m_pDrv, LTM_R1_LTM_SELRGB_SLP1,  LTM_SELRGB_SLP1,    0);
    CAM_WRITE_BITS(this->m_pDrv, LTM_R1_LTM_SELRGB_SLP2,  LTM_SELRGB_SLP2,    0);
    CAM_WRITE_BITS(this->m_pDrv, LTM_R1_LTM_SELRGB_SLP3,  LTM_SELRGB_SLP3,    0);
    CAM_WRITE_BITS(this->m_pDrv, LTM_R1_LTM_SELRGB_SLP4,  LTM_SELRGB_SLP4,    0);
    CAM_WRITE_BITS(this->m_pDrv, LTM_R1_LTM_SELRGB_SLP5,  LTM_SELRGB_SLP5,    0);
    CAM_WRITE_BITS(this->m_pDrv, LTM_R1_LTM_SELRGB_SLP6,  LTM_SELRGB_SLP6,    0);

    CAM_WRITE_BITS(this->m_pDrv, LTM_R1_LTM_OUT_STR,      LTM_OUT_STR,       16);
    CAM_FUNC_INF("-:\n");
    return ret;
}

NSCam::NSIoPipe::NSCamIOPipe::ENPipe_CAM
CAM_TUNING_CTRL::tuningHwModuleMap(
    const ISP_HW_MODULE hwModule)
{
    NSCam::NSIoPipe::NSCamIOPipe::ENPipe_CAM lModule;
    switch(hwModule){
        case CAM_A:
            lModule = NSCam::NSIoPipe::NSCamIOPipe::ENPipe_CAM_A;
            break;
        case CAM_B:
            lModule = NSCam::NSIoPipe::NSCamIOPipe::ENPipe_CAM_B;
            break;
        case CAM_C:
            lModule = NSCam::NSIoPipe::NSCamIOPipe::ENPipe_CAM_C;
            break;
        default:
            lModule = NSCam::NSIoPipe::NSCamIOPipe::ENPipe_CAM_MAX;
            break;
    }
    return lModule;
};

