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
#ifndef _CQ_TUNING_MGR_IMP_H_
#define _CQ_TUNING_MGR_IMP_H_


#include <mtkcam/def/common.h>
#include <isp_reg.h>
#include <mtkcam/drv/IHalSensor.h>
#include <ispio_pipe_ports.h>
#include <ispio_pipe_buffer.h>
#include <ICamIOPipe.h>


#include <list>

#include "cq_tuning_mgr.h"
#include "isp_drv_cam.h"
#include "isp_drv_stddef.h"

#include "CamIOPipe.h"
#include "cam_dupCmdQMgr.h"
#include "twin_mgr.h"

//-----------------------------------------------------------------------------
using namespace std;
using namespace NSCam;
using namespace android;

using namespace NSImageio;
using namespace NSIspio;
//-----------------------------------------------------------------------------

/**************************************************************************
 *                      D E F I N E S / M A C R O S                       *
 **************************************************************************/


 //-----------------------------------------------------------------------------
#include <cutils/properties.h>              // For property_get().

#undef  DBG_LOG_TAG                        // Decide a Log TAG for current file.
#define DBG_LOG_TAG     LOG_TAG
#include "drv_log.h"                    // Note: DBG_LOG_TAG will be used in header file, so header must be included after definition.
DECLARE_DBG_LOG_VARIABLE(cqmgr);

// Clear previous define, use our own define.
#undef LOG_VRB
#undef LOG_DBG
#undef LOG_INF
#undef LOG_WRN
#undef LOG_ERR
#undef LOG_AST
#define LOG_VRB(fmt, arg...)        do { if (cqmgr_DbgLogEnable_VERBOSE) { BASE_LOG_VRB("[0x%x]:" fmt,this->mSensorIndex, ##arg); } } while(0)
#define LOG_DBG(fmt, arg...)        do { if (cqmgr_DbgLogEnable_DEBUG  ) { BASE_LOG_DBG("[0x%x]:" fmt,this->mSensorIndex, ##arg); } } while(0)
#define LOG_INF(fmt, arg...)        do { if (cqmgr_DbgLogEnable_INFO   ) { BASE_LOG_INF("[0x%x]:" fmt,this->mSensorIndex, ##arg); } } while(0)
#define LOG_WRN(fmt, arg...)        do { if (cqmgr_DbgLogEnable_WARN   ) { BASE_LOG_WRN("[0x%x]:" fmt,this->mSensorIndex, ##arg); } } while(0)
#define LOG_ERR(fmt, arg...)        do { if (cqmgr_DbgLogEnable_ERROR  ) { BASE_LOG_ERR("[0x%x]:" fmt,this->mSensorIndex, ##arg); } } while(0)
#define LOG_AST(cond, fmt, arg...)  do { if (cqmgr_DbgLogEnable_ASSERT ) { BASE_LOG_AST(cond,"[0x%x]:" fmt,this->mSensorIndex, ##arg); } } while(0)

//-----------------------------------------------------------------------------



/**************************************************************************
 *     E N U M / S T R U C T / T Y P E D E F    D E C L A R A T I O N     *
 **************************************************************************/
typedef enum { /*plz refer ECQ_TUNING_USER*/
    E_TUNING_AF = 0,
    E_TUNING_AFO,
    E_TUNING_SGG1,
    E_TUNING_SGG5,
    E_TUNING_OB,
    E_TUNING_LSC_0,
    // extra
    E_TUNING_FH_AFO_MAGIC,
    //
    E_TUNING_LSC_1,
    E_TUNING_FH_AAO_SERIAL,
    E_TUNING_FH_AAO_OB,
    E_TUNING_FH_AAO_LSC,
    //
    E_TUNING_TWIN_COUNTER,
    E_TUNING_MODULE_MAX
} EISPCQ_TUNING_MODULE;

struct CQ_TUNING_MAP_STRUCT{
    EISPCQ_TUNING_MODULE  moduleid;
    MUINT32            offset;
    MUINT32            count;
    CQ_TUNING_MAP_STRUCT(){
        moduleid = E_TUNING_MODULE_MAX;
        offset = 0;
        count = 0;
    };
};

 #define getCqTuningModule(hw) ({\
    EISPCQ_TUNING_MODULE f;\
    switch (hw) {\
        case EAF_TUNING_AF:\
            f = E_TUNING_AF;\
            break;\
        case EAF_TUNING_AFO:\
            f = E_TUNING_AFO;\
            break;\
        case EAF_TUNING_SGG1:\
            f = E_TUNING_SGG1;\
            break;\
        case EAF_TUNING_SGG5:\
            f = E_TUNING_SGG5;\
            break;\
        case EAA_TUNING_OB:\
            f = E_TUNING_OB;\
            break;\
        case EAA_TUNING_LSC:\
            f = E_TUNING_LSC_0;\
            break;\
        default:\
            f = E_TUNING_MODULE_MAX;\
            break;\
    }\
    f;\
})

/*for mapping cqModule info of "mIspCQModuleInfo"*/
#define getCQModule(hw) ({\
    E_CAM_MODULE f;\
    switch (hw) {\
        case E_TUNING_AF:\
            f = CAM_ISP_AF_;\
            break;\
        case E_TUNING_AFO:\
            f = CAM_DMA_AFO_;\
            break;\
        case E_TUNING_SGG1:\
            f = CAM_ISP_SGG1_;\
            break;\
        case E_TUNING_SGG5:\
            f = CAM_ISP_SGG1_;\
            break;\
        case E_TUNING_OB:\
            f = CAM_ISP_OBC_GAIN_;\
            break;\
        case E_TUNING_LSC_0:\
            f = CAM_ISP_LSC_RATIO_0;\
            break;\
        case E_TUNING_FH_AFO_MAGIC:\
            f = CAM_DMA_FH_AFO_SPARE_MAGIC_;\
            break;\
        case E_TUNING_LSC_1:\
            f = CAM_ISP_LSC_RATIO_1;\
            break;\
        case E_TUNING_FH_AAO_SERIAL:\
            f = CAM_DMA_FH_AAO_SPARE_SERIAL_;\
            break;\
        case E_TUNING_FH_AAO_OB:\
            f = CAM_DMA_FH_AAO_SPARE_OB_;\
            break;\
        case E_TUNING_FH_AAO_LSC:\
            f = CAM_DMA_FH_AAO_SPARE_LSC_;\
            break;\
        case E_TUNING_TWIN_COUNTER:\
            f = CAM_TWIN_COUNTER;\
            break;\
        default:\
            f = CAM_CAM_MODULE_MAX;\
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
class CqTuningMgrImp : public CqTuningMgr
{
    friend  CqTuningMgr* CqTuningMgr::getInstance(MUINT32 sensorIndex);

    public:
        CqTuningMgrImp();
        ~CqTuningMgrImp(){};
    //
    public:
        virtual MBOOL   init(const char* userName="");
        virtual MBOOL   uninit(const char* userName="");
        //
        virtual MBOOL   CQTuningMgrWriteReg(MUINT32 addr, MUINT32 data);
        virtual MBOOL   update(CQ_TUNING_CMD_ST cmd);
        //
    private:
        MUINT32 CQTuningMgrReadReg(MUINT32 addr);
        MBOOL   dumpTuning(CQ_TUNING_CMD_ST cmd, E_ISP_CAM_CQ mCq);
        MBOOL   addRegCfg(list<ISPIO_REG_CFG>* RegCfg, MUINT32 addr, MUINT32 count);
        MBOOL   update2Drv(E_ISP_CAM_CQ cq,list<ISPIO_REG_CFG>* input, MUINT32 module_en);

    private:

        #define _REG_OFST_(CQTuningMgrObj, RegName)({                                               \
            MUINT32 addrOffset;                                                                      \
            addrOffset = (unsigned long)(&(((cam_reg_t*)(CQTuningMgrObj->m_pIspRegMap))->RegName)) -  \
                   (unsigned long) ((cam_reg_t*)(CQTuningMgrObj->m_pIspRegMap));                      \
            addrOffset;                                                                              \
        })

        NSImageio::NSIspio::ICamIOPipe *mpCamIOPipe;
        struct CQ_TUNING_MAP_STRUCT mCQTuningMap [E_TUNING_MODULE_MAX];
        //
        mutable Mutex       mLock;//lock update
        mutable Mutex       m_RegLock;//lock for r/w m_pIspRegMap
        volatile MINT32     mInitCount;
        MUINT32             mSensorIndex;
        MUINT32             mSubsample;
        MUINT32             mTgIdx;
        ISP_HW_MODULE       m_hwModule;

        DupCmdQMgr*         m_pCmdQMgr_AE;  //for ae smoothing function
        DupCmdQMgr*         m_pCmdQMgr_AF;  //for af fast fetch function
        TwinMgr*            m_TwinMgr;

    private:
        MBOOL bInitDone;
};

//-----------------------------------------------------------------------------
#endif // _CQ_TUNING_MGR_IMP_H_

