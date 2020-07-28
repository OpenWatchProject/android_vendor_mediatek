/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2010. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
 * AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek Software")
 * have been modified by MediaTek Inc. All revisions are subject to any receiver's
 * applicable license agreements with MediaTek Inc.
 */

/********************************************************************************************
 *     LEGAL DISCLAIMER
 *
 *     (Header of MediaTek Software/Firmware Release or Documentation)
 *
 *     BY OPENING OR USING THIS FILE, BUYER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 *     THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE") RECEIVED
 *     FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO BUYER ON AN "AS-IS" BASIS
 *     ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES, EXPRESS OR IMPLIED,
 *     INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR
 *     A PARTICULAR PURPOSE OR NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY
 *     WHATSOEVER WITH RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 *     INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND BUYER AGREES TO LOOK
 *     ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. MEDIATEK SHALL ALSO
 *     NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE RELEASES MADE TO BUYER'S SPECIFICATION
 *     OR TO CONFORM TO A PARTICULAR STANDARD OR OPEN FORUM.
 *
 *     BUYER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND CUMULATIVE LIABILITY WITH
 *     RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION,
 *     TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE
 *     FEES OR SERVICE CHARGE PAID BY BUYER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 *     THE TRANSACTION CONTEMPLATED HEREUNDER SHALL BE CONSTRUED IN ACCORDANCE WITH THE LAWS
 *     OF THE STATE OF CALIFORNIA, USA, EXCLUDING ITS CONFLICT OF LAWS PRINCIPLES.
 ************************************************************************************************/
#define LOG_TAG "CqTuningMgr"
#define MTK_LOG_ENABLE 1

#include <cutils/log.h>
#include <utils/threads.h>  // For Mutex::Autolock.
#include <cutils/atomic.h>
#include <semaphore.h>

#include "cq_tuning_mgr_imp.h"

/**************************************************************************
 *                      D E F I N E S / M A C R O S                       *
 **************************************************************************/

/**************************************************************************
 *     E N U M / S T R U C T / T Y P E D E F    D E C L A R A T I O N     *
 **************************************************************************/

/**************************************************************************
 *                 E X T E R N A L    R E F E R E N C E S                 *
 **************************************************************************/

/**************************************************************************
 *                         G L O B A L    D A T A                         *
 **************************************************************************/

static CqTuningMgrImp gCqTuningMgrObj[NCQTUNING_MAX_SENSOR_CNT];


/**************************************************************************
 *       P R I V A T E    F U N C T I O N    D E C L A R A T I O N        *
 **************************************************************************/
//-----------------------------------------------------------------------------
CqTuningMgrImp::CqTuningMgrImp()
    : mpCamIOPipe(NULL)
    , mSensorIndex(0)
    , mSubsample(0)
    , mTgIdx(0)
    , m_hwModule(CAM_MAX)
    , m_pCmdQMgr_AE(NULL)
    , m_pCmdQMgr_AF(NULL)
    , m_TwinMgr(NULL)
    , bInitDone(MFALSE)
{
    DBG_LOG_CONFIG(tuning, cqmgr);
    this->mInitCount = 0;
}

//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
CqTuningMgr* CqTuningMgr::getInstance(MUINT32 sensorIndex)
{
    if(sensorIndex >= NCQTUNING_MAX_SENSOR_CNT)
    {
        BASE_LOG_ERR("Unsupported sensorIndex: %d (Must in 0 ~ %d, Reset it to be 0)", sensorIndex, NCQTUNING_MAX_SENSOR_CNT);
        sensorIndex = 0;
    }

    BASE_LOG_INF("+getInstance, sensorIndex: %d", sensorIndex);

    gCqTuningMgrObj[sensorIndex].mSensorIndex = sensorIndex;
    return  (CqTuningMgr*)&gCqTuningMgrObj[sensorIndex];
}

//-----------------------------------------------------------------------------
MBOOL CqTuningMgrImp::init(const char* userName)
{
    MBOOL ret = MTRUE;
    int i;
    IHalSensorList*     pHalSensorList = NULL;
    IHalSensor*         mHalSensor = NULL;
    SensorDynamicInfo   seninfo;
    ISP_DRV_CAM         *mAFIspVirDrv;
    MUINT32             _ctl_en, _ctl_dma_en;

    Mutex::Autolock lock(this->mLock);

    if(strlen(userName)<1) {
        LOG_ERR("Plz add userName\n");
        return MFALSE;
    }

    LOG_INF("userName: %s, mInitCount: %d", userName, this->mInitCount);

    if(this->mInitCount > 0)
        goto EXIT;


    // Get TG idx
    pHalSensorList = IHalSensorList::get();
    mHalSensor = pHalSensorList->createSensor(LOG_TAG, this->mSensorIndex);
    if (mHalSensor == NULL){
        LOG_ERR("createSensor fail \n");
        ret = MFALSE;
        goto EXIT;
    }
    mHalSensor->querySensorDynamicInfo(pHalSensorList->querySensorDevIdx(this->mSensorIndex), &seninfo);
    this->mTgIdx = seninfo.TgInfo;
    mHalSensor->destroyInstance(LOG_TAG);
    this->m_hwModule = (this->mTgIdx == CAM_TG_1)? CAM_A : CAM_B;

    //map reg map for R/W Macro, in order to calculate addr-offset by reg name
    this->m_RegLock.lock();
    if (this->m_pIspRegMap) {
        LOG_WRN("m_pIspRegMap is not null");
        free(this->m_pIspRegMap);
    }
    this->m_pIspRegMap = (MUINT32*)malloc(REG_SIZE);
    memset(this->m_pIspRegMap, 0, REG_SIZE);
    this->m_RegLock.unlock();

    // get CamIOPipe obj
    this->mpCamIOPipe = ICamIOPipe::getInstance(ICamIOPipe::CAMIO,(this->mTgIdx == CAM_TG_1)? \
        (NSImageio::NSIspio::ICamIOPipe::TG_A) : (NSImageio::NSIspio::ICamIOPipe::TG_B));
    if (!this->mpCamIOPipe) {
        LOG_ERR("mpCamIOPipe::getInstance() fail, must configPipe first\n");
        ret = MFALSE;
        goto EXIT;
    }
    //ctl_en query from cq0, and get subsmaple for CQ1
    if(this->mpCamIOPipe->sendCommand(EPIPECmd_GET_TOP_INFO,(MINTPTR)&_ctl_en,(MINTPTR)&_ctl_dma_en,(MINTPTR)&this->mSubsample) == MFALSE){
        LOG_ERR("get top info fail\n");
        ret = MFALSE;
        goto EXIT;
    }

    //attach cmdq1 & cmdq5, fps is unknown, tmp set 1
    if( (this->m_pCmdQMgr_AF = DupCmdQMgr::CmdQMgr_attach(1,this->mSubsample,this->m_hwModule,ISP_DRV_CQ_THRE5)) == NULL){
        LOG_ERR("CQ 5 init fail\n");
        this->m_pCmdQMgr_AF = NULL;
        ret = MFALSE;
        goto EXIT;
    }
    mAFIspVirDrv = this->m_pCmdQMgr_AF->CmdQMgr_GetCurCycleObj(0)[0];

    if(!this->mSubsample){
        if( (this->m_pCmdQMgr_AE = DupCmdQMgr::CmdQMgr_attach(1,this->mSubsample,this->m_hwModule,ISP_DRV_CQ_THRE1)) == NULL){
            LOG_ERR("CQ 1 init fail\n");
            this->m_pCmdQMgr_AE = NULL;
            ret = MFALSE;
            goto EXIT;
        }
    }

    //twin
    this->m_TwinMgr = TwinMgr::createInstance(this->m_hwModule);
    if(this->m_TwinMgr->getIsTwin() == MTRUE)
        this->m_TwinMgr->initTwin(m_pCmdQMgr_AF);
    //
    if(this->m_TwinMgr->getIsTwin()){
        CAM_WRITE_REG(mAFIspVirDrv,CAM_TG_SEN_GRAB_PXL, CAM_READ_REG(mAFIspVirDrv->getPhyObj(),CAM_TG_SEN_GRAB_PXL)); //must read phy
        CAM_WRITE_REG(mAFIspVirDrv,CAM_CTL_FMT_SEL,CAM_READ_REG(mAFIspVirDrv->getPhyObj(),CAM_CTL_FMT_SEL));
        CAM_WRITE_REG(mAFIspVirDrv,CAM_CTL_SEL,CAM_READ_REG(mAFIspVirDrv->getPhyObj(),CAM_CTL_SEL));
        CAM_WRITE_REG(mAFIspVirDrv,CAM_CTL_EN, _ctl_en);
        CAM_WRITE_REG(mAFIspVirDrv,CAM_CTL_DMA_EN, _ctl_dma_en);
    }

    //map cqModule info of "mIspCQModuleInfo"
    LOG_DBG("query mCQTuningMap");
    for(MUINT32 i=0;i<E_TUNING_MODULE_MAX;i++){
        mAFIspVirDrv->getCQModuleInfo(getCQModule(i), mCQTuningMap[i].offset,mCQTuningMap[i].count);
        LOG_DBG("[%d]module(%d) 0x%x_0x%x",i,getCQModule(i),mCQTuningMap[i].offset,mCQTuningMap[i].count);
    }

    LOG_DBG("tg(%x)hwModule(%x)subSample(%d)",this->mTgIdx,this->m_hwModule,this->mSubsample);

    this-> bInitDone = MTRUE;

EXIT:
    android_atomic_inc(&this->mInitCount);
    LOG_DBG("X:\n");
    return ret;

}

//-----------------------------------------------------------------------------
MBOOL CqTuningMgrImp::uninit(const char* userName)
{
    MBOOL ret = MTRUE;

    Mutex::Autolock lock(this->mLock);

    if(strlen(userName)<1)
    {
        LOG_ERR("Plz add userName\n");
        return MFALSE;
    }

    LOG_INF("userName: %s, mInitCount: %d", userName, this->mInitCount);

    if(this->mInitCount == 0)
        goto EXIT;

    android_atomic_dec(&this->mInitCount);

    // Not the last one.
    if(this->mInitCount > 0) {
        goto EXIT;
    }

    if(this->m_pIspRegMap != NULL)
    {
        this->m_RegLock.lock();
        free((void*)this->m_pIspRegMap);
        this->m_pIspRegMap = NULL;
        this->m_RegLock.unlock();
    }

    this->mTgIdx = 0;
    this->m_hwModule = CAM_MAX;
    this->mpCamIOPipe = NULL;
    this->m_TwinMgr = NULL;
    this->bInitDone = MFALSE;

    //stop & detach
    if(this->m_pCmdQMgr_AF){
        if(this->m_pCmdQMgr_AF->CmdQMgr_stop()){
            LOG_ERR("CQ_AF stop fail\n");
            return MFALSE;
        }
        if(this->m_pCmdQMgr_AF->CmdQMgr_detach()){
            LOG_ERR("CQ_AF uninit fail\n");
            return MFALSE;
        }
        this->m_pCmdQMgr_AF = NULL;
    }

    if(this->m_pCmdQMgr_AE){
        if(this->m_pCmdQMgr_AE->CmdQMgr_stop()){
            LOG_ERR("CQ_AE stop fail\n");
            return MFALSE;
        }
        if(this->m_pCmdQMgr_AE->CmdQMgr_detach()){
            LOG_ERR("CQ_AE uninit fail\n");
            return MFALSE;
        }
        this->m_pCmdQMgr_AE = NULL;
    }

EXIT:
    LOG_DBG("X:\n");
    return ret;

}

//-----------------------------------------------------------------------------
MBOOL CqTuningMgrImp::update(CQ_TUNING_CMD_ST cmd)
{
    MBOOL ret = MTRUE;
    E_ISP_CAM_CQ mCq5=ISP_DRV_CQ_NONE, mCq1=ISP_DRV_CQ_NONE;
    list<ECQ_TUNING_USER>::iterator it;
    list<ISPIO_REG_CFG> ispRegCfgCq5;
    list<ISPIO_REG_CFG> ispRegCfgCq1;
    MUINT32 moduleEn5=0;
    EISPCQ_TUNING_MODULE cqModule;
    Header_AAO  fh_aao;
    Header_AAO  fh_afo;

    Mutex::Autolock lock(this->mLock);

    if(!this->bInitDone){
        ret = MFALSE;
        LOG_ERR("Must init CqTuningMgr first");
        goto EXIT;
    }

    LOG_DBG("+serial(0x%x)",cmd.serial);

    for(it = cmd.module->begin(); it != cmd.module->end();it++){
        cqModule = getCqTuningModule(*it);
        switch(cqModule)
        {
            case E_TUNING_AF:
                if(this->m_pCmdQMgr_AF == NULL){
                    LOG_ERR("NULL m_pCmdQMgr_AF, init first");
                    ret = MFALSE;
                    goto EXIT;
                }

                mCq5 = ISP_DRV_CQ_THRE5;
                moduleEn5|= AF_EN_;
                addRegCfg(&ispRegCfgCq5, mCQTuningMap[cqModule].offset, mCQTuningMap[cqModule].count);
                break;

            case E_TUNING_SGG1:
                if(this->m_pCmdQMgr_AF == NULL){
                    LOG_ERR("NULL m_pCmdQMgr_AF, init first");
                    ret = MFALSE;
                    goto EXIT;
                }

                mCq5 = ISP_DRV_CQ_THRE5;
                moduleEn5|= SGG1_EN_;
                addRegCfg(&ispRegCfgCq5, mCQTuningMap[cqModule].offset, mCQTuningMap[cqModule].count);
                break;

            case E_TUNING_SGG5:
                if(this->m_pCmdQMgr_AF == NULL){
                    LOG_ERR("NULL m_pCmdQMgr_AF, init first");
                    ret = MFALSE;
                    goto EXIT;
                }

                mCq5 = ISP_DRV_CQ_THRE5;
                moduleEn5|= SGG5_EN_;
                addRegCfg(&ispRegCfgCq5, mCQTuningMap[cqModule].offset, mCQTuningMap[cqModule].count);
                break;

            case E_TUNING_AFO:
                if(this->m_pCmdQMgr_AF == NULL){
                    LOG_ERR("NULL m_pCmdQMgr_AF, init first");
                    ret = MFALSE;
                    goto EXIT;
                }

                mCq5 = ISP_DRV_CQ_THRE5;
                moduleEn5|= AFO_EN_;

                //for frame header stride, spare reg (SPEARE_3) must same to AFO's Header_Enque
                CQTuningMgrWriteReg(_REG_OFST_(this,CAM_AFO_FH_SPARE_3), \
                                    AFO_STRIDE_DUMMY(CQTuningMgrReadReg(_REG_OFST_(this,CAM_AFO_XSIZE))));
                fh_afo.Header_Enque(Header_AFO::E_STRIDE,\
                                    this->m_pCmdQMgr_AF->CmdQMgr_GetCurCycleObj(0)[0],\
                                    CQTuningMgrReadReg(_REG_OFST_(this,CAM_AFO_XSIZE)));

                //strdie
                CQTuningMgrWriteReg(_REG_OFST_(this,CAM_AFO_STRIDE), \
                                    AFO_STRIDE_DUMMY(CQTuningMgrReadReg(_REG_OFST_(this,CAM_AFO_XSIZE))));

                //xsize-=1, ysize-=1
                CQTuningMgrWriteReg(_REG_OFST_(this,CAM_AFO_XSIZE),CQTuningMgrReadReg(_REG_OFST_(this,CAM_AFO_XSIZE))-1);
                CQTuningMgrWriteReg(_REG_OFST_(this,CAM_AFO_YSIZE),CQTuningMgrReadReg(_REG_OFST_(this,CAM_AFO_YSIZE))-1);
                addRegCfg(&ispRegCfgCq5, mCQTuningMap[cqModule].offset, mCQTuningMap[cqModule].count);
                break;

            case E_TUNING_OB:
                if(this->m_pCmdQMgr_AE == NULL){
                    LOG_ERR("only supported under non-high-speed mode");
                    ret = MFALSE;
                    goto EXIT;
                }

                mCq1 = ISP_DRV_CQ_THRE1;
                addRegCfg(&ispRegCfgCq1, mCQTuningMap[cqModule].offset, mCQTuningMap[cqModule].count);
                //FH
                CQTuningMgrWriteReg(mCQTuningMap[E_TUNING_FH_AAO_OB].offset, CQTuningMgrReadReg(mCQTuningMap[cqModule].offset));
                addRegCfg(&ispRegCfgCq1, mCQTuningMap[E_TUNING_FH_AAO_OB].offset, mCQTuningMap[E_TUNING_FH_AAO_OB].count);
                fh_aao.Header_Enque(Header_AAO::E_OB,\
                                    this->m_pCmdQMgr_AE->CmdQMgr_GetCurCycleObj(0)[0],\
                                    CQTuningMgrReadReg(mCQTuningMap[E_TUNING_FH_AAO_OB].offset));
                break;

            case E_TUNING_LSC_0:
                if(this->m_pCmdQMgr_AE== NULL){
                    LOG_ERR("only supported under non-high-speed mode");
                    ret = MFALSE;
                    goto EXIT;
                }

                mCq1 = ISP_DRV_CQ_THRE1;
                addRegCfg(&ispRegCfgCq1, mCQTuningMap[cqModule].offset, mCQTuningMap[cqModule].count);
                //LSCI_ratio_1
                CQTuningMgrWriteReg(mCQTuningMap[E_TUNING_LSC_1].offset, CQTuningMgrReadReg(mCQTuningMap[E_TUNING_LSC_1].offset));
                addRegCfg(&ispRegCfgCq1, mCQTuningMap[E_TUNING_LSC_1].offset, mCQTuningMap[E_TUNING_LSC_1].count);
                //FH
                CQTuningMgrWriteReg(mCQTuningMap[E_TUNING_FH_AAO_LSC].offset, CQTuningMgrReadReg(mCQTuningMap[cqModule].offset));
                addRegCfg(&ispRegCfgCq1, mCQTuningMap[E_TUNING_FH_AAO_LSC].offset, mCQTuningMap[E_TUNING_FH_AAO_LSC].count);
                fh_aao.Header_Enque(Header_AAO::E_LSC,\
                                    this->m_pCmdQMgr_AE->CmdQMgr_GetCurCycleObj(0)[0],\
                                    CQTuningMgrReadReg(mCQTuningMap[E_TUNING_FH_AAO_LSC].offset));
                break;

            default:
                LOG_ERR("update: Unsupported module(%x)!!", cqModule);
                ret = MFALSE;
                goto EXIT;
                break;
        }
    }

    if (mCq5 == ISP_DRV_CQ_THRE5){
        LOG_DBG("update: cq(%d)module(0x%x)serial(0x%x)",mCq5, moduleEn5, cmd.serial);
        //af magic
        CQTuningMgrWriteReg(mCQTuningMap[E_TUNING_FH_AFO_MAGIC].offset, cmd.serial);
        addRegCfg(&ispRegCfgCq5, mCQTuningMap[E_TUNING_FH_AFO_MAGIC].offset,mCQTuningMap[E_TUNING_FH_AFO_MAGIC].count);
        fh_afo.Header_Enque(Header_AFO::E_Magic,\
                    this->m_pCmdQMgr_AF->CmdQMgr_GetCurCycleObj(0)[0],\
                    CQTuningMgrReadReg(mCQTuningMap[E_TUNING_FH_AFO_MAGIC].offset));
        //twin counter, only add descriptor, twin counter will update by update_afterRunTwin
        if(MTRUE == this->m_TwinMgr->getIsTwin()){
            addRegCfg(&ispRegCfgCq5, mCQTuningMap[E_TUNING_TWIN_COUNTER].offset,mCQTuningMap[E_TUNING_TWIN_COUNTER].count);
        }
        if(cqmgr_DbgLogEnable_VERBOSE)
            dumpTuning(cmd, ISP_DRV_CQ_THRE5);

        ret = this->update2Drv(mCq5, &ispRegCfgCq5, moduleEn5);
    }
    else if(mCq1 == ISP_DRV_CQ_THRE1) {
        LOG_DBG("update: cq(%d)serial(0x%x)", mCq1, cmd.serial);
        //serial
        CQTuningMgrWriteReg(mCQTuningMap[E_TUNING_FH_AAO_SERIAL].offset, cmd.serial);
        addRegCfg(&ispRegCfgCq1, mCQTuningMap[E_TUNING_FH_AAO_SERIAL].offset,mCQTuningMap[E_TUNING_FH_AAO_SERIAL].count);
        fh_aao.Header_Enque(Header_AAO::E_Serial,\
                    this->m_pCmdQMgr_AE->CmdQMgr_GetCurCycleObj(0)[0],\
                    CQTuningMgrReadReg(mCQTuningMap[E_TUNING_FH_AAO_SERIAL].offset));
        //
        if(cqmgr_DbgLogEnable_VERBOSE)
            dumpTuning(cmd, ISP_DRV_CQ_THRE1);

        ret = this->update2Drv(mCq1, &ispRegCfgCq1, 0);
    }

EXIT:
    LOG_DBG("X:");
    return ret;

}
//-----------------------------------------------------------------------------
MBOOL CqTuningMgrImp::update2Drv(E_ISP_CAM_CQ cq,list<ISPIO_REG_CFG>* input, MUINT32 module_en)
{
    list<ISPIO_REG_CFG>::iterator it;
    MUINT32 targetIdx;
    MBOOL ret = MTRUE;
    MUINT32 cam_en = 0x0, dma_en = 0x0;
    //
    if(input == NULL){
        LOG_ERR("update reg list is null\n");
        return MFALSE;
    }

    switch (cq){
        case ISP_DRV_CQ_THRE5:
        {
            ISP_DRV_CAM *mAFIspVirDrv = NULL;

            if(module_en == 0){
                LOG_INF("no update\n");
                return MTRUE;
            }
            if(this->m_pCmdQMgr_AF == NULL){
                LOG_ERR("not init yet, m_pCmdQMgr_AF is null\n");
                return MFALSE;
            }

            //
            ret = this->m_pCmdQMgr_AF->CmdQMgr_Cfg(0,(MUINTPTR)input);
            if(this->m_TwinMgr->getIsTwin()){
                ret += this->m_TwinMgr->CQ_cfg(cq,0,(MUINTPTR)input);
            }

            /* If twin is enable, update top
            *  Compute the CQ data via twin_drv (dual_isp_driver) -- runTwin()*/
            if(MTRUE == this->m_TwinMgr->getIsTwin()){
                mAFIspVirDrv = this->m_pCmdQMgr_AF->CmdQMgr_GetCurCycleObj(0)[0];
                if(module_en & AF_EN_)
                    cam_en |= AF_EN_;
                if(module_en & SGG1_EN_)
                    cam_en |= SGG1_EN_;
                if(module_en & SGG5_EN_)
                    dma_en |= SGG5_EN_;
                if(module_en & AFO_EN_)
                    dma_en |= AFO_EN_;

                /* CAM_CTL_EN and CAM_CTL_DMA_EN only update by CQ0, write to cq5 just for runTwinDrv
                 * NOTE: mask rrz_en to prevent runTwinDrv err */
                CAM_WRITE_REG(mAFIspVirDrv,CAM_CTL_DMA_EN, (dma_en|CAM_READ_REG(mAFIspVirDrv,CAM_CTL_DMA_EN)));
                CAM_WRITE_REG(mAFIspVirDrv,CAM_CTL_EN, ((~RRZ_EN_) & (cam_en|CAM_READ_REG(mAFIspVirDrv,CAM_CTL_EN))));

                //runTwin
                if(MFALSE == this->m_TwinMgr->runTwinDrv(TWN_SCENARIO_AF_FAST_P2)){
                    LOG_ERR("Run Twin drv fail\n");
                    return MFALSE;
                }
            }

            //
            if(ret == 0){
                ret = this->m_pCmdQMgr_AF->CmdQMgr_start();
                if(this->m_TwinMgr->getIsTwin())
                    ret += this->m_TwinMgr->CQ_Trig(cq);
            }
            else{
                LOG_ERR("thread5 CFG fail\n");
                return MFALSE;
            }
            ret = MTRUE;
        }
            break;
        case ISP_DRV_CQ_THRE1:
            if(this->m_pCmdQMgr_AE == NULL){
                LOG_ERR("only supported under non-high-speed mode\n");
                return MFALSE;
            }
            //
            ret = this->m_pCmdQMgr_AE->CmdQMgr_Cfg(0,(MUINTPTR)input);
            if(this->m_TwinMgr->getIsTwin())
                ret += this->m_TwinMgr->CQ_cfg(cq, 0,(MUINTPTR)input);
            if(ret == 0){
                ret = this->m_pCmdQMgr_AE->CmdQMgr_start();
                if(this->m_TwinMgr->getIsTwin())
                    ret += this->m_TwinMgr->CQ_Trig(cq);
            }
            else{
                LOG_ERR("thread1 CFG fail\n");
            }
            ret = MTRUE;
            break;
        default:
            LOG_ERR("no surpported cq:0x%x\n",cq);
            break;

    }

    return ret;
}
//-----------------------------------------------------------------------------
MBOOL CqTuningMgrImp::addRegCfg(list<ISPIO_REG_CFG>* RegCfg, MUINT32 addr, MUINT32 count)
{
    MUINT32 ret = MTRUE;
    ISPIO_REG_CFG Regio;

    if (RegCfg == NULL) {
        LOG_ERR("reg cfg list is null");
        return MFALSE;
    }

    for (MUINT32 i=0;i<count;i++){
        Regio.Addr = addr + i*0x4;
        Regio.Data = CQTuningMgrReadReg(Regio.Addr);
        RegCfg->push_back(Regio);
    }

    return ret;
}

//-----------------------------------------------------------------------------
MUINT32 CqTuningMgrImp::CQTuningMgrReadReg(MUINT32 addr)
{
    MBOOL ret = MTRUE;
    MUINT32 shift;
    MUINT32 legal_range;
    int i = 0;

    Mutex::Autolock lock(this->m_RegLock);

    if (this->m_pIspRegMap == NULL){
        LOG_ERR("plz init CqTuningMgr first");
        goto EXIT;
    }

    shift = 0x0;
    // check wether addr is < upper bound
    legal_range = CAM_BASE_RANGE;

    if(addr >= legal_range){
        LOG_ERR("over range(0x%x), legal_range(0x%x)\n", addr, legal_range);
        ret = MFALSE;
        goto EXIT;
    }

    return (this->m_pIspRegMap[(addr + shift) >> 2]);

EXIT:
    return ret;

}


//-----------------------------------------------------------------------------
MBOOL CqTuningMgrImp::CQTuningMgrWriteReg(MUINT32 addr, MUINT32 data)
{
    MBOOL ret = MTRUE;
    MUINT32 shift;
    MUINT32 legal_range;
    int i = 0;

    Mutex::Autolock lock(this->m_RegLock);

    LOG_DBG("Addr(0x%8x), data(0x%8x)", addr, data);

    if (this->m_pIspRegMap == NULL){
        LOG_ERR("plz init CqTuningMgr first");
        goto EXIT;
    }

    shift = 0x0;
    // check wether addr is < upper bound
    legal_range = CAM_BASE_RANGE;

    if(addr >= legal_range){
        LOG_ERR("over range(0x%x), legal_range(0x%x)\n", addr, legal_range);
        ret = MFALSE;
        goto EXIT;
    }

    this->m_pIspRegMap[(addr + shift) >> 2] = data;

EXIT:
    return ret;

}

//-----------------------------------------------------------------------------

MBOOL CqTuningMgrImp::dumpTuning(CQ_TUNING_CMD_ST cmd, E_ISP_CAM_CQ mCq)
{
    MBOOL ret = MTRUE;
    list<ECQ_TUNING_USER>::iterator it;
    MUINT32 nStart, nEnd;
    EISPCQ_TUNING_MODULE cqModule;

    if(cmd.module== NULL){
        LOG_ERR("module is null, can't dump tuning");
        return MFALSE;
    }

    Mutex::Autolock lock(this->m_RegLock);

    LOG_INF("+Cq%d, serial:0x%x",mCq,cmd.serial);

    for(it = cmd.module->begin(); it != cmd.module->end();it++){
        char str[256] = {'\0'};
        MUINT32 strLeng = sizeof(str)-1, totalStrLeng=0;

        cqModule = getCqTuningModule(*it);

        LOG_INF("Module_%d_%d: Addr(0x%x)Count(%d)\n", (*it), \
            cqModule, mCQTuningMap[cqModule].offset, mCQTuningMap[cqModule].count);

        for(MUINT32 j=0;j<mCQTuningMap[cqModule].count;j++){
            char _tmp[128];
            MUINT addr =  mCQTuningMap[cqModule].offset + j*0x4;
            MUINT32 curLeng = sprintf(_tmp,"[0x%x] 0x%8x - ", addr, this->m_pIspRegMap[addr>>2]);

            if(strLeng - totalStrLeng < curLeng){
                LOG_INF("%s", str);
                str[0] = '\0';
                totalStrLeng = 0;
            }
            strncat(str,_tmp,(sizeof(str) - totalStrLeng));
            totalStrLeng += curLeng;
        }
        LOG_INF("%s", str);
    }

    //Extra
    if(mCq == ISP_DRV_CQ_THRE5){
        nStart= E_TUNING_FH_AFO_MAGIC;
        nEnd = E_TUNING_LSC_1;
    } else{
        nStart= E_TUNING_LSC_1;
        nEnd = E_TUNING_MODULE_MAX;
    }

    for(MUINT32 i = nStart; i < nEnd; i++){
        char str[256] = {'\0'};
        MUINT32 strLeng = sizeof(str)-1, totalStrLeng=0;

        LOG_INF("Module_%d: Addr(0x%x)Count(%d)\n", i, \
            mCQTuningMap[i].offset, mCQTuningMap[i].count);

        for(MUINT32 j=0;j<mCQTuningMap[i].count;j++){
            char _tmp[128];
            MUINT addr =  mCQTuningMap[i].offset + j*0x4;
            MUINT32 curLeng = sprintf(_tmp,"[0x%x] 0x%8x - ", addr, this->m_pIspRegMap[addr>>2]);

            if(strLeng - totalStrLeng < curLeng){
                LOG_INF("%s", str);
                str[0] = '\0';
                totalStrLeng = 0;
            }
            strncat(str,_tmp,(sizeof(str) - totalStrLeng));
            totalStrLeng += curLeng;
        }
        LOG_INF("%s", str);
    }
    return ret;
}


