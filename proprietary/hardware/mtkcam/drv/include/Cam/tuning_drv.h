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


#ifndef _TUNING_DRV_H_
#define _TUNING_DRV_H_

#include <vector>
#include <map>
#include <list>
#include <isp_drv_cam.h> // for get E_CAM_MODULE

using namespace std;
using namespace android;

/**************************************************************************
 *     E N U M / S T R U C T / T Y P E D E F    D E C L A R A T I O N     *
 **************************************************************************/

// for tuning function:
enum ETuningDrvUser
{
    eTuningDrvUser_3A,
    eTuningDrvUser_TuningUserNum, // Record the number of tuning user
};



typedef struct stTuningNode_t
{
    MUINT32* pTuningRegBuf;
    MINT32 bufSize;
    MINT32 memID;
    MINT32 magicNum;
    MUINT32 eUpdateFuncBit; // EIspTuningMgrFunc
    MUINT32 eUpdateFuncBit2; // EIspTuningMgrFunc //for FRZ/RRZ and new tuning nodes
    MUINT32 ctlEn_CAM;      // used to en/disable update engine
    MUINT32 ctlEn2_CAM;     // used to en/disable update engine
    MUINT32 ctlEn3_CAM;     // used to en/disable update engine
    MUINT32 ctlEn4_CAM;     // used to en/disable update engine
    MUINT32 ctlEnDMA_CAM;   // used to en/disable update engine
    MUINT32 ctlEnDMA2_CAM;  // used to en/disable update engine
    MUINT32 ctlEn_UNI;      // used to en/disable update engine
    MUINT32 ctlEnDMA_UNI;   // used to en/disable update engine

    struct{
        MUINT32     FD;
        MUINTPTR    VA_addr;
    }Dmai_bufinfo[32];          //for input dam buf va & fd only

    MBOOL isVerify; // if this tuning node need to be verifed by user after drv deque
}stTuningNode;


typedef struct stTuningQue_t
{
    stTuningNode* pTuningNode;
    MINT32 curWriteTuningNodeBaseId; // Current write tuning node base id by tuning user. Base represnts the first node in mBQNum tuning nodes
    MINT32 curReadTuningNodeBaseId; // Current read tuning node base id by p1 driver. Base represnts the first node in mBQNum tuning nodes
    ETuningDrvUser user;
}stTuningQue;



/**************************************************************************
 *                   C L A S S    D E C L A R A T I O N                   *
 **************************************************************************/
/**
 * Abstract class of tuning_drv
 */
class TuningDrv
{
    protected:
        TuningDrv() {};
        virtual ~TuningDrv() {};

    public:
        static TuningDrv* getInstance(MUINT32 sensorIndex);
        virtual MBOOL init(const char* userName, ETuningDrvUser user, MUINT32 BQNum) = 0; // allocate stTuningQue in init function
        virtual MBOOL deTuningQue(ETuningDrvUser user, MINT32* magicNum, MBOOL* isVerify, vector<stTuningNode*> &vpTuningNodes, MBOOL bFirst = MTRUE) = 0; // deTuningQue by tuning user
        virtual MBOOL enTuningQue(ETuningDrvUser user, MBOOL bFirst = MTRUE) = 0; // enTuningQue by tuning user
        virtual MBOOL deTuningQueByDrv(ETuningDrvUser user, MINT32* magicNum, vector<stTuningNode*> &vpTuningNodes) = 0; // deTuningQue by p1 driver
        virtual MBOOL enTuningQueByDrv(ETuningDrvUser user) = 0; // enTuningQue by p1 driver
        virtual MUINT32 getBQNum() = 0; // get mBQNum value
        virtual MBOOL getIsApplyTuning(ETuningDrvUser user) = 0;

        virtual MBOOL uninit(const char* userName, ETuningDrvUser user) = 0;
        virtual MBOOL searchVerifyTuningQue(ETuningDrvUser user, MINT32 magicNum, stTuningNode* &vpTuningNodes) = 0;
        virtual MBOOL queryModulebyMagicNum(MINT32 magicNum, vector<MUINT32> *moduleEn)
                      {(void)magicNum;(void)moduleEn;return 0;}
    private:
        virtual MBOOL searchTuningQue(ETuningDrvUser user, MINT32* magicNum, MBOOL bPrint = MTRUE) = 0;
};

#endif

