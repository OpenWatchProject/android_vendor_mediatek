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

#define LOG_TAG "TuningDrv"

#include <tuning_drv_imp.h>
//include for reg size
#include "camera_isp.h"
//#include "isp_drv_cam.h" // for get E_CAM_MODULE

 //-----------------------------------------------------------------------------
#include <cutils/properties.h>              // For property_get().

#undef  DBG_LOG_TAG                        // Decide a Log TAG for current file.
#define DBG_LOG_TAG     LOG_TAG
#include "drv_log.h"                    // Note: DBG_LOG_TAG will be used in header file, so header must be included after definition.
DECLARE_DBG_LOG_VARIABLE(tuning_drv);

// Clear previous define, use our own define.
#undef LOG_VRB
#undef LOG_DBG
#undef LOG_INF
#undef LOG_WRN
#undef LOG_ERR
#undef LOG_AST
#define LOG_VRB(fmt, arg...)        do { if (tuning_drv_DbgLogEnable_VERBOSE) { BASE_LOG_VRB(fmt, ##arg); } } while(0)
#define LOG_DBG(fmt, arg...)        do { if (tuning_drv_DbgLogEnable_DEBUG  ) { BASE_LOG_DBG(fmt, ##arg); } } while(0)
#define LOG_INF(fmt, arg...)        do { if (tuning_drv_DbgLogEnable_INFO   ) { BASE_LOG_INF(fmt, ##arg); } } while(0)
#define LOG_WRN(fmt, arg...)        do { if (tuning_drv_DbgLogEnable_WARN   ) { BASE_LOG_WRN(fmt, ##arg); } } while(0)
#define LOG_ERR(fmt, arg...)        do { if (tuning_drv_DbgLogEnable_ERROR  ) { BASE_LOG_ERR(fmt, ##arg); } } while(0)
#define LOG_AST(cond, fmt, arg...)  do { if (tuning_drv_DbgLogEnable_ASSERT ) { BASE_LOG_AST(cond, fmt, ##arg); } } while(0)
//-----------------------------------------------------------------------------



#define NTUNING_MAX_SENSOR_CNT 5
#define TUNING_NODE_NUM_BASE_PER_BQ 16 // The base of tuning nodes, where tuning nodes = BQNum * TUNING_NODE_NUM_BASE_PER_BQ
#define TUNING_NODE_REG_BUF_SIZE (2 * ISP_REG_RANGE) // register size: cam + uni
#define TUNING_DUMMY_MAGIC_NUM -1
#define TUNING_INIT_NODE_IDX -1

#define GET_NEXT_TUNING_NODE_IDX(_idx_)    ( ((((MINT32)_idx_)+1)>=TUNING_NODE_NUM_BASE_PER_BQ )?(0):(((MINT32)_idx_)+1) )
#define GET_PREV_TUNING_NODE_IDX(_idx_)    ( ((((MINT32)_idx_)-1)>=0 )?(((MINT32)_idx_)-1):(TUNING_NODE_NUM_BASE_PER_BQ-1) )

//#define TUNING_NODE_LOG_DEBUG


TuningDrvImp::TuningDrvImp()
                : mInitCount(0)
                , mSensorIndex(0)
                , mBQNum(0)
                , mLock()
                , m_pIMemDrv(NULL)

{
    this->mInitCount = 0;

    // Init each variable for eTuningDrvUser_TuningUserNum
    for(int i = 0; i < eTuningDrvUser_TuningUserNum; i++)
    {
        this->mInitCountUser[i] = 0;
        this->mIsApplyTuning[i] = MFALSE;
    }

    memset(&mTuningQue, 0, sizeof(mTuningQue));
    memset(&mTuningQueVerify, 0, sizeof(mTuningQueVerify));

    LOG_DBG("getpid[0x%08x],gettid[0x%08x]", getpid() ,gettid());
}

static TuningDrvImp gTuningDrvObj[NTUNING_MAX_SENSOR_CNT];
//
TuningDrv* TuningDrv::getInstance(MUINT32 sensorIndex)
{
    if(sensorIndex >= NTUNING_MAX_SENSOR_CNT)
    {
        LOG_ERR("Unsupported sensorIndex: %d (Must in 0 ~ %d, Set it to be 0)", sensorIndex, NTUNING_MAX_SENSOR_CNT);
        sensorIndex = 0;
    }

    LOG_INF("+getInstance, sensorIndex: %d", sensorIndex);

    gTuningDrvObj[sensorIndex].mSensorIndex = sensorIndex;
    return  (TuningDrv*)&gTuningDrvObj[sensorIndex];
}



// Allocate tuning queue here, where # of tuning nodes = BQNum * 16
// Only tuning user can call init(), other user will return error
MBOOL TuningDrvImp::init(const char* userName, ETuningDrvUser user, MUINT32 BQNum)
{
    MBOOL ret = MTRUE;
    IMEM_BUF_INFO tmpTuningBufInfo;
    stTuningNode *tuningNode, *tuningNodeVerify;

    MUINT32 TuningNodeNum;
    MUINT32 i;
    MINT32 tmp=0;

    Mutex::Autolock lock(this->mLock);

    if(strlen(userName)<1)
    {
        LOG_ERR("Plz add userName if you want to use isp driver\n");
        return MFALSE;
    }

    LOG_INF(" -E. userName(%s), mInitCount(%d), curUser(%d), mInitCountUser(%d)", \
        userName, this->mInitCount, user, this->mInitCountUser[user]);

    if(BQNum <= 0)
    {
        LOG_ERR("BQNum(%d) must > 0!!!\n", BQNum);
        ret = MFALSE;
        goto EXIT;
    }

    if(user >= eTuningDrvUser_TuningUserNum)
    {
        LOG_ERR("TuningDrvUser: %d must < eTuningDrvUser_TuningUserNum: %d!!!\n", \
            user, eTuningDrvUser_TuningUserNum);
        ret = MFALSE;
        goto EXIT;
    }

    // 1st call init()
    if(this->mInitCount == 0)
    {
        this->mBQNum = BQNum;

        LOG_INF("BQNum(%d), mBQNum(%d)", BQNum, this->mBQNum);
    }
    // check whether BQNum is different to this->mBQNum
    else
    {
        if(this->mBQNum != BQNum)
        {
            LOG_WRN("mBQNum have been set!! Will remain the old value: mBQNum(%d). Your BQNum(%d)\n",\
                this->mBQNum, BQNum);
        }
    }

    // Allocate tuning queue for tuning user when 1st time call init()
    if(this->mInitCountUser[user] == 0)
    {
        // Move  mIsApplyTuning setting to first enTuningQue.
        // So p1 drv can detect this variable = MTRUE during deque/enque, not in init()
        //this->mIsApplyTuning[user] = MTRUE; // Set this->mIsApplyTuning to MTRUE

        TuningNodeNum = TUNING_NODE_NUM_BASE_PER_BQ * BQNum;

        //Init current read tuning node by p1 driver
        this->mTuningQue[user].curReadTuningNodeBaseId = TUNING_INIT_NODE_IDX;

        //Init current write tuning node by tuning user
        this->mTuningQue[user].curWriteTuningNodeBaseId = TUNING_INIT_NODE_IDX;

        this->mTuningQue[user].user = user;

        tuningNode = (stTuningNode*)malloc(sizeof(stTuningNode) * TuningNodeNum);

        ///  TuningQue that to be verifed
        this->mTuningQueVerify[user].curReadTuningNodeBaseId = TUNING_INIT_NODE_IDX;
        this->mTuningQueVerify[user].curWriteTuningNodeBaseId = 0; // init is 0
        this->mTuningQueVerify[user].user = user;
        tuningNodeVerify= (stTuningNode*)malloc(sizeof(stTuningNode) * TuningNodeNum);

        for(i = 0; i < TuningNodeNum; i++)
        {
            ///  tuningNode init and allocate
            tuningNode[i].bufSize = TUNING_NODE_REG_BUF_SIZE;
            tuningNode[i].memID = 0x0;

            tuningNode[i].pTuningRegBuf = (MUINT32*)malloc(TUNING_NODE_REG_BUF_SIZE);

            tuningNode[i].magicNum = TUNING_DUMMY_MAGIC_NUM; // magic # init value = -1
            tuningNode[i].eUpdateFuncBit = eIspTuningMgrFunc_Null;
	    tuningNode[i].ctlEn_CAM = 0;
	    tuningNode[i].ctlEn2_CAM = 0;
            tuningNode[i].ctlEnDMA_CAM = 0;
            tuningNode[i].ctlEn_UNI = 0;
            tuningNode[i].ctlEnDMA_UNI = 0;
            tuningNode[i].isVerify = MFALSE;

            memset((MUINT8*)tuningNode[i].pTuningRegBuf, 0, tuningNode[i].bufSize);
            LOG_INF("tuning user(%d), que(%d): tuningNode.pTuningRegBuf(%p)", user, i, tuningNode[i].pTuningRegBuf);


            ///  tuningNode that to be verifed
            tuningNodeVerify[i].bufSize = TUNING_NODE_REG_BUF_SIZE;
            tuningNodeVerify[i].memID = 0x0;

            tuningNodeVerify[i].pTuningRegBuf = (MUINT32*)malloc(TUNING_NODE_REG_BUF_SIZE);

            tuningNodeVerify[i].magicNum = TUNING_DUMMY_MAGIC_NUM; // magic # init value = -1
            tuningNodeVerify[i].eUpdateFuncBit = eIspTuningMgrFunc_Null;
            tuningNodeVerify[i].ctlEn_CAM = 0;
            tuningNodeVerify[i].ctlEn2_CAM = 0;
            tuningNodeVerify[i].ctlEnDMA_CAM = 0;
            tuningNodeVerify[i].ctlEn_UNI = 0;
            tuningNodeVerify[i].ctlEnDMA_UNI = 0;

            memset((MUINT8*)tuningNodeVerify[i].pTuningRegBuf, 0, tuningNodeVerify[i].bufSize);
        }


        this->mTuningQue[user].pTuningNode = tuningNode;

        this->mTuningQueVerify[user].pTuningNode = tuningNodeVerify;

    }

    tmp = android_atomic_inc(&this->mInitCount);
    tmp = android_atomic_inc(&this->mInitCountUser[user]);

EXIT:

    LOG_INF(" -X. ");
    return ret;

}


MBOOL TuningDrvImp::uninit(const char* userName, ETuningDrvUser user)
{
    MBOOL ret = MTRUE;
    IMEM_BUF_INFO tmpTuningBufInfo;
    MUINT32 TuningNodeNum = TUNING_NODE_NUM_BASE_PER_BQ * this->mBQNum;
    MUINT32 i;
    MINT32 tmp=0;

    Mutex::Autolock lock(this->mLock);

    if(strlen(userName)<1)
    {
        LOG_ERR("Plz add userName if you want to use isp driver\n");
        return MFALSE;
    }

    LOG_INF("+, userName(%s), mInitCount(%d), curUser(%d), mInitCountUser(%d)\n", userName, this->mInitCount, user, this->mInitCountUser[user]);

    if(this->mInitCount <= 0)
    {
        goto EXIT;
    }

    tmp = android_atomic_dec(&this->mInitCount);
    tmp = android_atomic_dec(&this->mInitCountUser[user]);

    // free allocate buffer
    if(this->mInitCountUser[user] == 0)
    {
        this->mIsApplyTuning[user] = MFALSE; // Set this->mIsApplyTuning to MFALSE

        // Free tuningQue
        if(this->mTuningQue[user].pTuningNode != NULL)
        {
            LOG_INF("Free Tuning Queue\n");

            //free tuning nodes
            for(i = 0; i < TuningNodeNum; i++)
                free(this->mTuningQue[user].pTuningNode[i].pTuningRegBuf);

            free(this->mTuningQue[user].pTuningNode);
            this->mTuningQue[user].pTuningNode = NULL;
            this->mTuningQue[user].curReadTuningNodeBaseId = TUNING_INIT_NODE_IDX;
            this->mTuningQue[user].curWriteTuningNodeBaseId = TUNING_INIT_NODE_IDX;

        }

        // Free verify tuningQue
        if(this->mTuningQueVerify[user].pTuningNode != NULL)
        {
            LOG_INF("Free Verify Tuning Queue\n");

            //free tuning nodes
            for(i = 0; i < TuningNodeNum; i++)
                free(this->mTuningQueVerify[user].pTuningNode[i].pTuningRegBuf);

            free(this->mTuningQueVerify[user].pTuningNode);
            this->mTuningQueVerify[user].pTuningNode = NULL;
            this->mTuningQueVerify[user].curReadTuningNodeBaseId = TUNING_INIT_NODE_IDX;
            this->mTuningQueVerify[user].curWriteTuningNodeBaseId = 0;

        }

    }

    if(this->mInitCount == 0)
    {
        LOG_INF("-");
        this->mBQNum = 1;
    }

EXIT:
    return ret;
}


MBOOL TuningDrvImp::deTuningQue(ETuningDrvUser user, MINT32* magicNum, MBOOL* isVerify, vector<stTuningNode*> &vpTuningNodes,
    MBOOL bFirst)
{
    MBOOL ret = MTRUE;
    MUINT32 i;
    MINT32 curTuningNodeId;

    Mutex::Autolock lock(mTuningQueueAccessLock[user]);
    (void)bFirst;
    vpTuningNodes.clear(); // clear tuning nodes



    if(user >= eTuningDrvUser_TuningUserNum)
    {
        LOG_ERR("TuningUser(%d) is illegal!! must < (%d)!!!", user,eTuningDrvUser_TuningUserNum);
        ret = MFALSE;
        goto EXIT;
    }

    // Not init yet
    if(this->mInitCountUser[user] < 1)
    {
        LOG_ERR("Tuning drv, user(%d) havn't been init, mInitCountUser(%d)!!!", user, this->mInitCountUser[user]);
        ret = MFALSE;
        goto EXIT;
    }

    if((this->mTuningQue[user].curWriteTuningNodeBaseId >= TUNING_NODE_NUM_BASE_PER_BQ) || \
        (this->mTuningQue[user].curWriteTuningNodeBaseId < 0))
    {
        LOG_DBG("Init tuningQue curWriteTuningNodeBaseId(%d) to 0!!", this->mTuningQue[user].curWriteTuningNodeBaseId);
        this->mTuningQue[user].curWriteTuningNodeBaseId = 0;
    }

    LOG_DBG("+,user(%d), curWriteTuningNodeBaseId(%d)", user, this->mTuningQue[user].curWriteTuningNodeBaseId);

    for(i = 0; i < this->mBQNum; i++)
    {
        curTuningNodeId = this->mTuningQue[user].curWriteTuningNodeBaseId * this->mBQNum + i;
        LOG_DBG("tuning node(%d), magic#(%d)", curTuningNodeId, magicNum[i]);
        // set magic# to tuning nodes
        this->mTuningQue[user].pTuningNode[curTuningNodeId].magicNum = magicNum[i];

        // isVerify: true means this tuning nodes can be used by 3A to verify its value after driver deque it
        if(NULL == isVerify)
            this->mTuningQue[user].pTuningNode[curTuningNodeId].isVerify = MFALSE;
        else
            this->mTuningQue[user].pTuningNode[curTuningNodeId].isVerify = isVerify[i];

        // init tuning nodes
        memset((MUINT8*)this->mTuningQue[user].pTuningNode[curTuningNodeId].pTuningRegBuf, \
            0, this->mTuningQue[user].pTuningNode[curTuningNodeId].bufSize);
        this->mTuningQue[user].pTuningNode[curTuningNodeId].eUpdateFuncBit = eIspTuningMgrFunc_Null;
        this->mTuningQue[user].pTuningNode[curTuningNodeId].ctlEn_CAM = 0;
        this->mTuningQue[user].pTuningNode[curTuningNodeId].ctlEn2_CAM = 0;
        this->mTuningQue[user].pTuningNode[curTuningNodeId].ctlEnDMA_CAM = 0;
        this->mTuningQue[user].pTuningNode[curTuningNodeId].ctlEn_UNI = 0;
        this->mTuningQue[user].pTuningNode[curTuningNodeId].ctlEnDMA_UNI = 0;

        vpTuningNodes.push_back(this->mTuningQue[user].pTuningNode + curTuningNodeId);
    }


    LOG_DBG("-. deTuningQue");
EXIT:
    return ret;
}



MBOOL TuningDrvImp::enTuningQue(ETuningDrvUser user, MBOOL bFirst)
{
    MBOOL ret = MTRUE;
    MINT32 nextWriteTuningNodeBaseId;
    MINT32 curTuningNodeId;
    MUINT32 i;
    (void)bFirst;
    Mutex::Autolock lock(mTuningQueueAccessLock[user]);

    if(user >= eTuningDrvUser_TuningUserNum)
    {
        LOG_ERR("TuningUser(%d) is illegal!! must < (%d)!!!", user, eTuningDrvUser_TuningUserNum);
        return MFALSE;
    }

    // Not init yet
    if(this->mInitCountUser[user] < 1)
    {
        LOG_ERR("Tuning drv, user(%d) havn't been init, mInitCountUser(%d)!!!", user, this->mInitCountUser[user]);
        ret = MFALSE;
        goto EXIT;
    }

    // Modify isApplyTuning to be true
    if(this->mIsApplyTuning[user] == MFALSE)
    {
        this->mIsApplyTuning[user] = MTRUE;
    }

    if((this->mTuningQue[user].curWriteTuningNodeBaseId >= TUNING_NODE_NUM_BASE_PER_BQ) || \
        (this->mTuningQue[user].curWriteTuningNodeBaseId < 0))
    {
        LOG_ERR("Error!! curWriteTuningNodeBaseId(%d) error!!", this->mTuningQue[user].curWriteTuningNodeBaseId);
        this->mTuningQue[user].curWriteTuningNodeBaseId = 0;
        ret = MFALSE;
        goto EXIT;
    }

    for(i = 0; i < this->mBQNum; i++)
    {
        curTuningNodeId = this->mTuningQue[user].curWriteTuningNodeBaseId * this->mBQNum + i;
        LOG_DBG("TuningNode(%d): magic#(%d), eUpdateFuncBit(0x%x), ctlEn_CAM(0x%x), ctlEnDMA_CAM(0x%x), ctlEn_UNI(0x%x), ctlEnDMA_UNI(0x%x)", \
            curTuningNodeId, this->mTuningQue[user].pTuningNode[curTuningNodeId].magicNum, \
            this->mTuningQue[user].pTuningNode[curTuningNodeId].eUpdateFuncBit, \
            this->mTuningQue[user].pTuningNode[curTuningNodeId].ctlEn_CAM, \
            this->mTuningQue[user].pTuningNode[curTuningNodeId].ctlEnDMA_CAM, \
            this->mTuningQue[user].pTuningNode[curTuningNodeId].ctlEn_UNI, \
            this->mTuningQue[user].pTuningNode[curTuningNodeId].ctlEnDMA_UNI \
            );
    }

    nextWriteTuningNodeBaseId = GET_NEXT_TUNING_NODE_IDX(this->mTuningQue[user].curWriteTuningNodeBaseId);

    // Check wthether nextWriteTuningNodeBaseId is used by driver. If yes, return error cause the node cannot be used right now.
    if(nextWriteTuningNodeBaseId == this->mTuningQue[user].curReadTuningNodeBaseId)
    {
        LOG_ERR("Error!! Next Write index is used by driver!! nextWriteTuningNodeBaseId(%d), curReadTuningNodeBaseId(%d)", \
            nextWriteTuningNodeBaseId, this->mTuningQue[user].curReadTuningNodeBaseId);

        ret = MFALSE;
        goto EXIT;
    }

    this->mTuningQue[user].curWriteTuningNodeBaseId = nextWriteTuningNodeBaseId;

    // Init next writing node for tuning user use
    for(i = 0; i < this->mBQNum; i++)
    {
        curTuningNodeId = this->mTuningQue[user].curWriteTuningNodeBaseId * this->mBQNum + i;

        memset((MUINT8*)this->mTuningQue[user].pTuningNode[curTuningNodeId].pTuningRegBuf, \
            0, this->mTuningQue[user].pTuningNode[curTuningNodeId].bufSize);
        this->mTuningQue[user].pTuningNode[curTuningNodeId].magicNum = TUNING_DUMMY_MAGIC_NUM;
        this->mTuningQue[user].pTuningNode[curTuningNodeId].eUpdateFuncBit = eIspTuningMgrFunc_Null;
        this->mTuningQue[user].pTuningNode[curTuningNodeId].ctlEn_CAM = 0;
        this->mTuningQue[user].pTuningNode[curTuningNodeId].ctlEn2_CAM = 0;
        this->mTuningQue[user].pTuningNode[curTuningNodeId].ctlEnDMA_CAM = 0;
        this->mTuningQue[user].pTuningNode[curTuningNodeId].ctlEn_UNI = 0;
        this->mTuningQue[user].pTuningNode[curTuningNodeId].ctlEnDMA_UNI = 0;
    }

    LOG_DBG("user(%d), curWriteTuningNodeBaseId(%d)", user, this->mTuningQue[user].curWriteTuningNodeBaseId);


EXIT:
    return ret;
}


MBOOL TuningDrvImp::deTuningQueByDrv(ETuningDrvUser user, MINT32* magicNum, vector<stTuningNode*> &vpTuningNodes)
{
    MBOOL ret = MTRUE;
    MUINT32 i;
    MINT32 curTuningNodeId, curTuningNodeVerifyId;

    Mutex::Autolock lock(mTuningQueueAccessLock[user]);

    if(user >= eTuningDrvUser_TuningUserNum)
    {
        LOG_ERR("User(%d) is illegal!! must < (%d)!!!", user,eTuningDrvUser_TuningUserNum);
        ret = MFALSE;
        goto EXIT;
    }

    // Not init yet
    if(this->mInitCountUser[user] < 1)
    {
        LOG_ERR("Tuning drv, user(%d) havn't been init, mInitCountUser(%d)!!!", user, this->mInitCountUser[user]);
        ret = MFALSE;
        goto EXIT;
    }


    if(searchTuningQue(user, magicNum) != MTRUE)
    {
        LOG_ERR("curReadTuningNodeBaseId search error");
        ret = MFALSE;
        goto EXIT;
    }


    LOG_DBG("User(%d), curReadTuningNodeBaseId(%d) \n", user, this->mTuningQue[user].curReadTuningNodeBaseId);

    for(i = 0; i < this->mBQNum; i++)
    {
        curTuningNodeId = this->mTuningQue[user].curReadTuningNodeBaseId * this->mBQNum + i;
        vpTuningNodes.push_back(this->mTuningQue[user].pTuningNode + curTuningNodeId);

        LOG_DBG("TuningNode(%d): magic#(%d), eUpdateFuncBit(0x%x), ctlEn_CAM(0x%x), ctlEnDMA_CAM(0x%x), ctlEn_UNI(0x%x), ctlEnDMA_UNI(0x%x). Verify(%d)", \
            curTuningNodeId, this->mTuningQue[user].pTuningNode[curTuningNodeId].magicNum, \
            this->mTuningQue[user].pTuningNode[curTuningNodeId].eUpdateFuncBit, \
            this->mTuningQue[user].pTuningNode[curTuningNodeId].ctlEn_CAM, \
            this->mTuningQue[user].pTuningNode[curTuningNodeId].ctlEnDMA_CAM, \
            this->mTuningQue[user].pTuningNode[curTuningNodeId].ctlEn_UNI, \
            this->mTuningQue[user].pTuningNode[curTuningNodeId].ctlEnDMA_UNI, \
            this->mTuningQue[user].pTuningNode[curTuningNodeId].isVerify \
            );


        /// If isVerify = true, add node to verify tuning queue
        if(MTRUE == this->mTuningQue[user].pTuningNode[curTuningNodeId].isVerify) {
            curTuningNodeVerifyId = this->mTuningQueVerify[user].curWriteTuningNodeBaseId;

            memcpy(this->mTuningQueVerify[user].pTuningNode[curTuningNodeVerifyId].pTuningRegBuf, \
                   this->mTuningQue[user].pTuningNode[curTuningNodeId].pTuningRegBuf, \
                   this->mTuningQue[user].pTuningNode[curTuningNodeId].bufSize);

            this->mTuningQueVerify[user].pTuningNode[curTuningNodeVerifyId].magicNum =
                this->mTuningQue[user].pTuningNode[curTuningNodeId].magicNum;
            this->mTuningQueVerify[user].pTuningNode[curTuningNodeVerifyId].eUpdateFuncBit =
                this->mTuningQue[user].pTuningNode[curTuningNodeId].eUpdateFuncBit;
            this->mTuningQueVerify[user].pTuningNode[curTuningNodeVerifyId].isVerify =
                this->mTuningQue[user].pTuningNode[curTuningNodeId].isVerify;
            this->mTuningQueVerify[user].pTuningNode[curTuningNodeVerifyId].ctlEn_CAM =
                this->mTuningQue[user].pTuningNode[curTuningNodeId].ctlEn_CAM;
            this->mTuningQueVerify[user].pTuningNode[curTuningNodeVerifyId].ctlEnDMA_CAM =
                this->mTuningQue[user].pTuningNode[curTuningNodeId].ctlEnDMA_CAM;
            this->mTuningQueVerify[user].pTuningNode[curTuningNodeVerifyId].ctlEn_UNI =
                this->mTuningQue[user].pTuningNode[curTuningNodeId].ctlEn_UNI;
            this->mTuningQueVerify[user].pTuningNode[curTuningNodeVerifyId].ctlEnDMA_UNI =
                this->mTuningQue[user].pTuningNode[curTuningNodeId].ctlEnDMA_UNI;

            // Move write id to next one
            this->mTuningQueVerify[user].curWriteTuningNodeBaseId = GET_NEXT_TUNING_NODE_IDX(this->mTuningQueVerify[user].curWriteTuningNodeBaseId);

            LOG_DBG("Add TuningNode(%d) to verify TuningNode(%d), writeNode(%d)", \
                curTuningNodeId, curTuningNodeVerifyId, \
                this->mTuningQueVerify[user].curWriteTuningNodeBaseId );
        }
    }

EXIT:
    return ret;
}

MBOOL TuningDrvImp::enTuningQueByDrv(ETuningDrvUser user)
{
    MBOOL ret = MTRUE;

    Mutex::Autolock lock(mTuningQueueAccessLock[user]);

    if(user >= eTuningDrvUser_TuningUserNum)
    {
        LOG_ERR("TuningUser(%d) is illegal!! must < (%d)!!!", user, eTuningDrvUser_TuningUserNum);
        return MFALSE;
    }

    // Not init yet
    if(this->mInitCountUser[user] < 1)
    {
        LOG_ERR("Tuning drv, user(%d) havn't been init, mInitCountUser(%d)!!!", user, this->mInitCountUser[user]);
        ret = MFALSE;
        goto EXIT;
    }

    this->mTuningQue[user].curReadTuningNodeBaseId = TUNING_INIT_NODE_IDX;


    LOG_DBG("-enTuningQueByDrv");

EXIT:
    return ret;
}

MUINT32 TuningDrvImp::getBQNum()
{
    return this->mBQNum;
}

MBOOL TuningDrvImp::getIsApplyTuning(ETuningDrvUser user)
{
    if(user >= eTuningDrvUser_TuningUserNum)
    {
        LOG_ERR("TuningUser(%d) is illegal!! must < (%d)!!!", user, eTuningDrvUser_TuningUserNum);
        return MFALSE;
    }

    LOG_DBG("isApplyTuning(%d), user(%d)", this->mIsApplyTuning[user], user);

    return this->mIsApplyTuning[user];
}



// 1. Use the 1st magic# to search readBaseIdx
// 2. Return readBaseIdx + (this->mBQNum-1) tuning nodes to driver
// 3. Need to check magic# in sub tuning modes are the same with magic# pass by driver
//      If not identical, print error return error
MBOOL TuningDrvImp::searchTuningQue(ETuningDrvUser user, MINT32* magicNum, MBOOL bPrint)
{
    MBOOL ret = MTRUE;
    MUINT32 searchCnt;
    MINT32 curSearchIdx = -1;
    MINT32 curSearchBaseIdx = -1;
    MUINT32 i;

    if(user >= eTuningDrvUser_TuningUserNum)
    {
        LOG_ERR("TuningUser(%d) is illegal!! must < (%d)!!!", user,eTuningDrvUser_TuningUserNum);
        ret = MFALSE;
        goto EXIT;
    }

    // Search back from (curWriteTuningNodeBaseId - 1) to (curWriteTuningNodeBaseId + 1)
    // Don't search curWriteTuningNodeBaseId cause this is researved for tuning user writing
    if(this->mTuningQue[user].curWriteTuningNodeBaseId < TUNING_NODE_NUM_BASE_PER_BQ && \
        this->mTuningQue[user].curWriteTuningNodeBaseId >= 0)
    {
        curSearchBaseIdx = GET_PREV_TUNING_NODE_IDX(this->mTuningQue[user].curWriteTuningNodeBaseId);

        // Search the 1st node only, and max seach count is TUNING_NODE_NUM_BASE_PER_BQ - 1 => Not search current writing node
        for(searchCnt = 0; searchCnt < (TUNING_NODE_NUM_BASE_PER_BQ - 1); searchCnt++)
        {
            curSearchIdx = curSearchBaseIdx * this->mBQNum;

            if(this->mTuningQue[user].pTuningNode[curSearchIdx].magicNum == magicNum[0])
                break;

            curSearchBaseIdx = GET_PREV_TUNING_NODE_IDX(curSearchBaseIdx);
        }


        if(searchCnt >= TUNING_NODE_NUM_BASE_PER_BQ - 1)
        {
            LOG_ERR("Error!! Search tuning node fail!!, magic#[0]: %d, curWriteTuningNodeBaseId(%d)", magicNum[0], this->mTuningQue[user].curWriteTuningNodeBaseId);
            this->mTuningQue[user].curReadTuningNodeBaseId = TUNING_INIT_NODE_IDX;
            ret = MFALSE;
            goto EXIT;
        }
        else
        {
            this->mTuningQue[user].curReadTuningNodeBaseId = curSearchBaseIdx;

            for(i = 0; i < this->mBQNum; i++)
            {
                if(this->mTuningQue[user].pTuningNode[curSearchIdx + i].magicNum != magicNum[i])
                {
                    LOG_ERR("magic#[%d]: %d not matched to tuning node(%d) magic#(%d)", i, magicNum[i], curSearchIdx + i, this->mTuningQue[user].pTuningNode[curSearchIdx + i].magicNum);
                    this->mTuningQue[user].curReadTuningNodeBaseId = TUNING_INIT_NODE_IDX;
                    ret = MFALSE;
                    goto EXIT;
                }
            }

        }
    }
    else
    {
        LOG_ERR("Error!! curWriteTuningNodeBaseId(%d) error!!", this->mTuningQue[user].curWriteTuningNodeBaseId);
        ret = MFALSE;
        goto EXIT;
    }

    LOG_DBG("Found tuning node!! magic#:(%d), curReadTuningNodeBaseId(%d), curWriteTuningNodeBaseId(%d)", \
        this->mTuningQue[user].pTuningNode[curSearchIdx].magicNum, this->mTuningQue[user].curReadTuningNodeBaseId, \
        this->mTuningQue[user].curWriteTuningNodeBaseId);

EXIT:
    return ret;
}


MBOOL TuningDrvImp::searchVerifyTuningQue(ETuningDrvUser user, MINT32 magicNum, stTuningNode* &vpTuningNodes)
{
    MBOOL ret = MTRUE;
    MUINT32 searchCnt;
    MINT32 curSearchIdx = -1;
    int i;

    if(user >= eTuningDrvUser_TuningUserNum)
    {
        LOG_WRN("TuningUser(%d) is illegal!! must < (%d)!!!", user,eTuningDrvUser_TuningUserNum);
        ret = MFALSE;
        goto EXIT;
    }

    // Search back from (curWriteTuningNodeBaseId - 1) to (curWriteTuningNodeBaseId + 1)
    // Don't search curWriteTuningNodeBaseId cause this is researved for tuning user writing
    if(this->mTuningQueVerify[user].curWriteTuningNodeBaseId < TUNING_NODE_NUM_BASE_PER_BQ && \
        this->mTuningQueVerify[user].curWriteTuningNodeBaseId >= 0)
    {
        curSearchIdx = GET_PREV_TUNING_NODE_IDX(this->mTuningQueVerify[user].curWriteTuningNodeBaseId);

        // Search the 1st node only, and max seach count is TUNING_NODE_NUM_BASE_PER_BQ - 1 => Not search current writing node
        for(searchCnt = 0; searchCnt < (TUNING_NODE_NUM_BASE_PER_BQ - 1); searchCnt++)
        {

            if(this->mTuningQueVerify[user].pTuningNode[curSearchIdx].magicNum == magicNum)
                break;

            curSearchIdx = GET_PREV_TUNING_NODE_IDX(curSearchIdx);
        }


        if(searchCnt >= TUNING_NODE_NUM_BASE_PER_BQ - 1)
        {
            LOG_WRN("Error!! Search verify tuning node fail!!, magic#: %d, curWriteTuningNodeBaseId(%d)", magicNum, this->mTuningQueVerify[user].curWriteTuningNodeBaseId);
            this->mTuningQueVerify[user].curReadTuningNodeBaseId = TUNING_INIT_NODE_IDX;
            ret = MFALSE;
            goto EXIT;
        }
        else
        {
            this->mTuningQueVerify[user].curReadTuningNodeBaseId = curSearchIdx;
        }
    }
    else
    {
        LOG_WRN("Error!! verify curWriteTuningNodeBaseId(%d) error!!", this->mTuningQueVerify[user].curWriteTuningNodeBaseId);
        ret = MFALSE;
        goto EXIT;
    }

    vpTuningNodes = this->mTuningQueVerify[user].pTuningNode + curSearchIdx;

    LOG_INF("Found verify tuning node!! magic#:(%d), curReadTuningNodeBaseId(%d), curWriteTuningNodeBaseId(%d)", \
        this->mTuningQueVerify[user].pTuningNode[curSearchIdx].magicNum, this->mTuningQueVerify[user].curReadTuningNodeBaseId, \
        this->mTuningQueVerify[user].curWriteTuningNodeBaseId);


EXIT:
    return ret;
}

MBOOL
TuningDrvImp::queryModulebyMagicNum(
    MINT32 magicNum,
    vector<MUINT32> *moduleEn)
{
    (void)magicNum;
    (void)moduleEn;

    LOG_INF("ISP_50 no spported queryModulebyMagicNum yet!!!\n");
    return MFALSE;
}


