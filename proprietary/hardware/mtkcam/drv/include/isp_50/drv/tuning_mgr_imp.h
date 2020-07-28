/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2010. All rights reserved.
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
TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE
 *     FEES OR SERVICE CHARGE PAID BY BUYER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 *     THE TRANSACTION CONTEMPLATED HEREUNDER SHALL BE CONSTRUED IN ACCORDANCE WITH THE LAWS
 *     OF THE STATE OF CALIFORNIA, USA, EXCLUDING ITS CONFLICT OF LAWS PRINCIPLES.
 ************************************************************************************************/
#ifndef _TUNING_MGR_IMP_H_
#define _TUNING_MGR_IMP_H_


#include <mtkcam/def/common.h>
#include <isp_reg.h>
#include <tuning_mgr.h>
#include <tuning_drv_imp.h>
//#include <imem_buffer.h> // Jessy TODO: Verify this


//-----------------------------------------------------------------------------
using namespace std;
using namespace NSCam;
using namespace android;
//-----------------------------------------------------------------------------

/**************************************************************************
 *                      D E F I N E S / M A C R O S                       *
 **************************************************************************/


 //-----------------------------------------------------------------------------
#include <cutils/properties.h>              // For property_get().

#undef  DBG_LOG_TAG                        // Decide a Log TAG for current file.
#define DBG_LOG_TAG     LOG_TAG
#include "drv_log.h"                    // Note: DBG_LOG_TAG will be used in header file, so header must be included after definition.
DECLARE_DBG_LOG_VARIABLE(tuning_mgr);

// Clear previous define, use our own define.
#undef LOG_VRB
#undef LOG_DBG
#undef LOG_INF
#undef LOG_WRN
#undef LOG_ERR
#undef LOG_AST
#define LOG_VRB(fmt, arg...)        do { if (tuning_mgr_DbgLogEnable_VERBOSE) { BASE_LOG_VRB(fmt, ##arg); } } while(0)
#define LOG_DBG(fmt, arg...)        do { if (tuning_mgr_DbgLogEnable_DEBUG  ) { BASE_LOG_DBG(fmt, ##arg); } } while(0)
//#define LOG_DBG(fmt, arg...)        do { if (tuning_mgr_DbgLogEnable_INFO  ) { BASE_LOG_INF(fmt, ##arg); } } while(0)
#define LOG_INF(fmt, arg...)        do { if (tuning_mgr_DbgLogEnable_INFO   ) { BASE_LOG_INF(fmt, ##arg); } } while(0)
#define LOG_WRN(fmt, arg...)        do { if (tuning_mgr_DbgLogEnable_WARN   ) { BASE_LOG_WRN(fmt, ##arg); } } while(0)
#define LOG_ERR(fmt, arg...)        do { if (tuning_mgr_DbgLogEnable_ERROR  ) { BASE_LOG_ERR(fmt, ##arg); } } while(0)
#define LOG_AST(cond, fmt, arg...)  do { if (tuning_mgr_DbgLogEnable_ASSERT ) { BASE_LOG_AST(cond, fmt, ##arg); } } while(0)
//-----------------------------------------------------------------------------



/**************************************************************************
 *     E N U M / S T R U C T / T Y P E D E F    D E C L A R A T I O N     *
 **************************************************************************/
typedef struct{
    ETuningMgrFunc eTuningMgrUpdateFunc;
    MUINT32* pCurWriteTuningQue; // the queue of featureio path be used currently
    MINT32  MagicNum;
}TUNING_MGR_UPDATE_INFO;

#define getIspTuningMgrFunc(hw) ({\
    EIspTuningMgrFunc f;\
    switch (hw) {\
        case eTuningMgrFunc_DBS:\
            f = eIspTuningMgrFunc_DBS;\
            break;\
        case eTuningMgrFunc_OBC:\
            f = eIspTuningMgrFunc_OBC;\
            break;\
        case eTuningMgrFunc_RMG:\
            f = eIspTuningMgrFunc_RMG;\
            break;\
        case eTuningMgrFunc_BNR:\
            f = eIspTuningMgrFunc_BNR;\
            break;\
        case eTuningMgrFunc_RMM:\
            f = eIspTuningMgrFunc_RMM;\
            break;\
        case eTuningMgrFunc_CAC:\
            f = eIspTuningMgrFunc_CAC;\
            break;\
        case eTuningMgrFunc_LSC:\
            f = eIspTuningMgrFunc_LSC;\
            break;\
        case eTuningMgrFunc_RCP:\
            f = eIspTuningMgrFunc_RCP;\
            break;\
        case eTuningMgrFunc_RPG:\
            f = eIspTuningMgrFunc_RPG;\
            break;\
        case eTuningMgrFunc_UFE:\
            f = eIspTuningMgrFunc_UFE;\
            break;\
        case eTuningMgrFunc_RCP3:\
            f = eIspTuningMgrFunc_RCP3;\
            break;\
        case eTuningMgrFunc_CPG:\
            f = eIspTuningMgrFunc_CPG;\
            break;\
        case eTuningMgrFunc_AA:\
            f = eIspTuningMgrFunc_AA;\
            break;\
        case eTuningMgrFunc_SGG1:\
            f = eIspTuningMgrFunc_SGG1;\
            break;\
        case eTuningMgrFunc_AF:\
            f = eIspTuningMgrFunc_AF;\
            break;\
        case eTuningMgrFunc_UNP2:\
            f = eIspTuningMgrFunc_UNP2;\
            break;\
        case eTuningMgrFunc_SGG3:\
            f = eIspTuningMgrFunc_SGG3;\
            break;\
        case eTuningMgrFunc_FLK:\
            f = eIspTuningMgrFunc_FLK;\
            break;\
        case eTuningMgrFunc_SL2F:\
            f = eIspTuningMgrFunc_SL2F;\
            break;\
        case eTuningMgrFunc_PBN:\
            f = eIspTuningMgrFunc_PBN;\
            break;\
        case eTuningMgrFunc_PS:\
            f = eIspTuningMgrFunc_PS;\
            break;\
        case eTuningMgrFunc_SGG5:\
            f = eIspTuningMgrFunc_SGG5;\
            break;\
        case eTuningMgrFunc_HLR:\
            f = eIspTuningMgrFunc_HLR;\
            break;\
        case eTuningMgrFunc_SL2J:\
            f = eIspTuningMgrFunc_SL2J;\
            break;\
        case eTuningMgrFunc_PDE:\
            f = eIspTuningMgrFunc_PDE;\
            break;\
        case eTuningMgrFunc_CPN:\
            f = eIspTuningMgrFunc_CPN;\
            break;\
        case eTuningMgrFunc_DCPN:\
            f = eIspTuningMgrFunc_DCPN;\
            break;\
        case eTuningMgrFunc_ADBS:\
            f = eIspTuningMgrFunc_ADBS;\
            break;\
        case eTuningMgrFunc_SCM:\
            f = eIspTuningMgrFunc_SCM;\
            break;\
        default:\
            f = eIspTuningMgrFunc_Null;\
            break;\
    }\
    f;\
})
/**************************************************************************
 *                 E X T E R N A L    R E F E R E N C E S                 *
 **************************************************************************/

/**************************************************************************
 *        P U B L I C    F U N C T I O N    D E C L A R A T I O N         *
 **************************************************************************/

/**************************************************************************
 *                   C L A S S    D E C L A R A T I O N                   *
 **************************************************************************/
class TuningMgrImp : public TuningMgr
{
    friend  TuningMgr* TuningMgr::getInstance(MUINT32 sensorIndex);
    public:
        TuningMgrImp();
        ~TuningMgrImp();
    //
    public:
        virtual MBOOL   init(const char* userName="", MUINT32 BQNum = 1);
        virtual MBOOL   uninit(const char* userName="");
        //
        virtual MBOOL updateEngineFD(ETuningMgrFunc engine, MUINT32 BQIdx, MINT32 memID,MUINTPTR va);
        virtual MBOOL updateEngine(ETuningMgrFunc engine, MBOOL ctlEn, MUINT32 BQIdx = 0);
        virtual MBOOL enableEngine(ETuningMgrFunc engine, MBOOL ctlEn, MUINT32 BQIdx = 0);
        virtual MBOOL tuningEngine(ETuningMgrFunc engine, MBOOL update, MUINT32 BQIdx = 0);

        virtual MBOOL dequeBuffer(MINT32* magicNum, MBOOL* isVerify = NULL, TUNING_MGR_TYPE_ENUM type = TUNING_MGR_TYPE_SETTUNING); // magic# of tuning node, isVerify: if tuning node need to verify by 3A lately
        virtual MBOOL enqueBuffer(TUNING_MGR_TYPE_ENUM type = TUNING_MGR_TYPE_SETTUNING);

        virtual MBOOL dumpTuningInfo(MINT32 magicNum, MINT32 uniqueKey);
		virtual MBOOL queryTopControlStatus(ETuningMgrFunc engine, MBOOL *enableStatus, MUINT32 BQIdx);

    public:
        virtual MBOOL tuningMgrWriteRegs(TUNING_MGR_REG_IO_STRUCT*  pRegIo , MINT32 cnt, MUINT32 BQIdx = 0);
        virtual MBOOL tuningMgrWriteReg(MUINT32 addr, MUINT32 data, MUINT32 BQIdx = 0);
        virtual MUINT32 tuningMgrReadReg(MUINT32 addr, MUINT32 BQIdx = 0);
        virtual MBOOL tuningMgrReadRegs(TUNING_MGR_REG_IO_STRUCT*  pRegIo, MINT32 cnt, MUINT32 BQIdx);

        // UNI
        virtual MBOOL tuningMgrWriteRegs_Uni(TUNING_MGR_REG_IO_STRUCT*  pRegIo, MINT32 cnt, MUINT32 BQIdx = 0);
        virtual MBOOL tuningMgrWriteReg_Uni(MUINT32 addr, MUINT32 data, MUINT32 BQIdx = 0);
        virtual MUINT32 tuningMgrReadReg_Uni(MUINT32 addr, MUINT32 BQIdx = 0);
        //
    private:
        //
        TuningDrv   *pTuningDrv;
        //
        mutable Mutex       mLock;
        mutable Mutex       mQueueInfoLock;
        volatile MINT32     mInitCount;
        MUINT32             mSensorIndex;
        MUINT32             mBQNum;
        vector<stTuningNode*> mvTuningNodes;
        stTuningNode*       mpVerifyTuningNode;
        MUINT32             mCtlEn_CAM; // used to en/disable update engine
        MUINT32             mCtlEn2_CAM; // used to en/disable update engine
        MUINT32             mCtlEnDMA_CAM; // used to en/disable update engine
        MUINT32             mCtlEn_UNI; // used to en/disable update engine
        MUINT32             mCtlEnDMA_UNI; // used to en/disable update engine

        ETuningDrvUser  mTuningDrvUser;

        char m_UserName[MAX_USER_NUMBER][MAX_USER_NAME_SIZE]; //support only one user to enque/deque, use this for debug
        //
};

//-----------------------------------------------------------------------------
#endif // _TUNING_MGR_IMP_H_

