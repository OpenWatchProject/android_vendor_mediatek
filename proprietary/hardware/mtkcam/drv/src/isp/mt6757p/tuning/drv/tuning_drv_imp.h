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
#ifndef _TUNING_DRV_IMP_H_
#define _TUNING_DRV_IMP_H_

#include <mtkcam/def/common.h>
#include <utils/threads.h>
#include <cutils/atomic.h>
#include <imem_drv.h>   // For IMemDrv


#include "tuning_drv.h"

/**************************************************************************
 *                   C L A S S    D E C L A R A T I O N                   *
 **************************************************************************/
class TuningDrvImp: public TuningDrv
{
    friend TuningDrv* TuningDrv::getInstance(MUINT32 sensorIndex);

    public:
        TuningDrvImp();
        ~TuningDrvImp(){}; // remember add {}, or you'll get undefinded to vtable build error!!

    public:
        //static TuningDrv* getInstance(MUINT32 sensorIndex);
        virtual MBOOL init(const char* userName, ETuningDrvUser user, MUINT32 BQNum = 1); // allocate stTuningQue in init function
        virtual MBOOL deTuningQue(ETuningDrvUser user, MINT32* magicNum, MBOOL* isVerify, vector<stTuningNode*> &vpTuningNodes);
        virtual MBOOL enTuningQue(ETuningDrvUser user);
        virtual MBOOL deTuningQueByDrv(ETuningDrvUser user, MINT32* magicNum, vector<stTuningNode*> &vpTuningNodes);
        virtual MBOOL enTuningQueByDrv(ETuningDrvUser user);
        virtual MUINT32 getBQNum(); // get mBQNum value
        virtual MBOOL getIsApplyTuning(ETuningDrvUser user);
        virtual MBOOL uninit(const char* userName, ETuningDrvUser user);
        virtual MBOOL searchVerifyTuningQue(ETuningDrvUser user, MINT32 magicNum, stTuningNode* &vpTuningNodes);

    private:
        virtual MBOOL searchTuningQue(ETuningDrvUser user, MINT32* magicNum);

    private:
        volatile MINT32 mInitCount;
        volatile MINT32 mInitCountUser[eTuningDrvUser_TuningUserNum];
        MUINT32 mSensorIndex;
        MUINT32 mBQNum;

        mutable Mutex mLock;
        // Assign a mutex for user (tuning user & p1 driver)
        mutable Mutex mTuningQueueAccessLock[eTuningDrvUser_TuningUserNum];

        IMemDrv* m_pIMemDrv;
        stTuningQue mTuningQue[eTuningDrvUser_TuningUserNum]; // Tuning queue for each tuning user
        stTuningQue mTuningQueVerify[eTuningDrvUser_TuningUserNum]; // Verify Tuning queue for each tuning user
        MBOOL mIsApplyTuning[eTuningDrvUser_TuningUserNum]; // Record whether is apply tuning for current tuning_drv user
};

#endif

