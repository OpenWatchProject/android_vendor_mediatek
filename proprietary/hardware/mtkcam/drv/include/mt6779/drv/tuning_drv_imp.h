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


#include <tuning_drv.h>



enum EIspTuningMgrFuncBit  // need mapping to ETuningMgrFunc / gIspTuningFuncBitMapp
{
    eIspTuningMgrFuncBit_OBC,
    eIspTuningMgrFuncBit_BPC,       // BNR rename to BPC
    eIspTuningMgrFuncBit_LSC,
    eIspTuningMgrFuncBit_AA,
    eIspTuningMgrFuncBit_AF,
    eIspTuningMgrFuncBit_FLK,
    eIspTuningMgrFuncBit_PBN,
    eIspTuningMgrFuncBit_PDE,

    // new module
    eIspTuningMgrFuncBit_MOBC_R2,
    eIspTuningMgrFuncBit_WB,
    eIspTuningMgrFuncBit_DGN,
    eIspTuningMgrFuncBit_TSFS,     // PS rename TSFS

    // new module - YUV DM path
    eIspTuningMgrFuncBit_DM,
    eIspTuningMgrFuncBit_FLC,
    eIspTuningMgrFuncBit_CCM,
    eIspTuningMgrFuncBit_GGM,
    eIspTuningMgrFuncBit_GGM_R2,
    eIspTuningMgrFuncBit_G2C,
    eIspTuningMgrFuncBit_G2C_R2,
    eIspTuningMgrFuncBit_YNRS,
    eIspTuningMgrFuncBit_RSS_R2,
    eIspTuningMgrFuncBit_SLK,
    eIspTuningMgrFuncBit_SLK_R2,
    eIspTuningMgrFuncBit_BPCI,
    eIspTuningMgrFuncBit_LCES,
    eIspTuningMgrFuncBit_LTMS,
    eIspTuningMgrFuncBit_LTM,
    eIspTuningMgrFuncBit_LTMTC_R1_CURVE,
    eIspTuningMgrFuncBit_HLR,
    eIspTuningMgrFuncBit_Num   //can't over 32
};

enum EIspTuningMgrFunc  // need mapping to ETuningMgrFunc
{
    eIspTuningMgrFunc_Null    =  0x00000000,
    eIspTuningMgrFunc_OBC     = (1 << eIspTuningMgrFuncBit_OBC),    //   28   04C0~052C
    eIspTuningMgrFunc_BPC     = (1 << eIspTuningMgrFuncBit_BPC),    //   58   03C0~04A4
    eIspTuningMgrFunc_LSC     = (1 << eIspTuningMgrFuncBit_LSC),    //   11   05C0~05E8
    eIspTuningMgrFunc_AA      = (1 << eIspTuningMgrFuncBit_AA),     //  103   0A00~0B98
    eIspTuningMgrFunc_AF      = (1 << eIspTuningMgrFuncBit_AF),     //  102   1380~1514
    eIspTuningMgrFunc_FLK     = (1 << eIspTuningMgrFuncBit_FLK),    //   13   0C40~0C70
    eIspTuningMgrFunc_PBN     = (1 << eIspTuningMgrFuncBit_PBN),    //    3   1780~1788
    eIspTuningMgrFunc_PDE     = (1 << eIspTuningMgrFuncBit_PDE),    //    2   1700~1704

     // new module
    eIspTuningMgrFunc_MOBC_R2 = (1 << eIspTuningMgrFuncBit_MOBC_R2), //   8   3A00~3A1C
    eIspTuningMgrFunc_WB      = (1 << eIspTuningMgrFuncBit_WB),      //   3   0600~0608
    eIspTuningMgrFunc_DGN     = (1 << eIspTuningMgrFuncBit_DGN),     //   5   0580~0590
    eIspTuningMgrFunc_TSFS    = (1 << eIspTuningMgrFuncBit_TSFS),    //   7   1644~165C
    // new module - YUV DM path
    eIspTuningMgrFunc_DM      = (1 << eIspTuningMgrFuncBit_DM),      //  26   1800~1864
    eIspTuningMgrFunc_FLC     = (1 << eIspTuningMgrFuncBit_FLC),     //   4   2DC0~2DCC
    eIspTuningMgrFunc_CCM     = (1 << eIspTuningMgrFuncBit_CCM),     //   9   1880~18A0
    eIspTuningMgrFunc_GGM     = (1 << eIspTuningMgrFuncBit_GGM),     // 194   18C0~1BC4
    eIspTuningMgrFunc_GGM_R2  = (1 << eIspTuningMgrFuncBit_GGM_R2),  // 194   3600~3904
    eIspTuningMgrFunc_G2C     = (1 << eIspTuningMgrFuncBit_G2C),     //   6   1C00~1C14
    eIspTuningMgrFunc_G2C_R2  = (1 << eIspTuningMgrFuncBit_G2C_R2),  //   6   2D40~2D54
    eIspTuningMgrFunc_YNRS    = (1 << eIspTuningMgrFuncBit_YNRS),    // 296   3000~349C
    eIspTuningMgrFunc_RSS_R2  = (1 << eIspTuningMgrFuncBit_RSS_R2),  //   9   3940~3960
    eIspTuningMgrFunc_SLK     = (1 << eIspTuningMgrFuncBit_SLK),     //  12   17C0~17EC
    eIspTuningMgrFunc_SLK_R2  = (1 << eIspTuningMgrFuncBit_SLK_R2),  //  12   2980~29AC

    eIspTuningMgrFunc_BPCI    = (1 << eIspTuningMgrFuncBit_BPCI),    //  6    41D0~41E4
    eIspTuningMgrFunc_LCES    = (1 << eIspTuningMgrFuncBit_LCES),    //  1    080C
    eIspTuningMgrFunc_LTMS    = (1 << eIspTuningMgrFuncBit_LTMS),    //  1
    eIspTuningMgrFunc_LTM     = (1 << eIspTuningMgrFuncBit_LTM),     //  1
    eIspTuningMgrFunc_LTMTC_R1_CURVE = (1 << eIspTuningMgrFuncBit_LTMTC_R1_CURVE),     //  1
    eIspTuningMgrFunc_HLR     = (1 << eIspTuningMgrFuncBit_HLR),     //  1
    eIspTuningMgrFunc_Num     = (1 << eIspTuningMgrFuncBit_Num)
};

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
        virtual MBOOL deTuningQue(ETuningDrvUser user, MINT32* magicNum, MBOOL* isVerify, vector<stTuningNode*> &vpTuningNodes, MBOOL bFirst = MTRUE);
        virtual MBOOL enTuningQue(ETuningDrvUser user, MBOOL bFirst = MTRUE);
        virtual MBOOL deTuningQueByDrv(ETuningDrvUser user, MINT32* magicNum, vector<stTuningNode*> &vpTuningNodes);
        virtual MBOOL enTuningQueByDrv(ETuningDrvUser user);
        virtual MUINT32 getBQNum(); // get mBQNum value
        virtual MBOOL getIsApplyTuning(ETuningDrvUser user);
        virtual MBOOL uninit(const char* userName, ETuningDrvUser user);
        virtual MBOOL searchVerifyTuningQue(ETuningDrvUser user, MINT32 magicNum, stTuningNode* &vpTuningNodes);
        virtual MBOOL queryModulebyMagicNum(MINT32 magicNum, vector<MUINT32> *moduleEn);

    private:
        virtual MBOOL searchTuningQue(ETuningDrvUser user, MINT32* magicNum, MBOOL bPrint = MTRUE);

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
        ETuningDrvUser m_TuningDrvUser;
};

#endif

