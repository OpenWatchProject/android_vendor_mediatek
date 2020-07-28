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
#include "isp_drv_cam.h" // for get E_CAM_MODULE

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

enum EIspTuningMgrFuncBit  // need mapping to ETuningMgrFunc / gIspTuningFuncBitMapp
{
    //eIspTuningMgrFuncBit_DMX,                // 3    4580~4588
    //eIspTuningMgrFuncBit_PBN,                // 3    4BB0~4BB8
    //eIspTuningMgrFuncBit_DBN,                // 2    4BA0~4BA4
    //eIspTuningMgrFuncBit_BIN,                // 3    4B80~4B88
    eIspTuningMgrFuncBit_DBS,                // 6    4B40~4B54
    eIspTuningMgrFuncBit_OBC,                // 8    45F0~460C
    eIspTuningMgrFuncBit_RMG,                // 2    45A0~45A4
    eIspTuningMgrFuncBit_BNR,                // 32   4620~469C
    eIspTuningMgrFuncBit_RMM,                // 9    45C0~45E0
    eIspTuningMgrFuncBit_CAC,                // 2    4AF0~4AF4
    //eIspTuningMgrFuncBit_BMX,                // 3    4780~4788
    eIspTuningMgrFuncBit_LSC,                // 8    46A0~46BC
    eIspTuningMgrFuncBit_RCP,                // 2    48F0~48F4
    eIspTuningMgrFuncBit_RPG,                // 6    46C0~46D4
    //eIspTuningMgrFuncBit_RRZ,                // 11   46E0~4708
    //eIspTuningMgrFuncBit_RMX,                // 3    4740~4748
    eIspTuningMgrFuncBit_UFE,                // 1    47C0
    //eIspTuningMgrFuncBit_PMX,                // 3    4B00~4B08
    eIspTuningMgrFuncBit_RCP3,               // 2    4BC0~4BC4
    eIspTuningMgrFuncBit_CPG,                // 6    4AD0~4AE4
    //eIspTuningMgrFuncBit_QBN1,               // 1    4AC0
    //eIspTuningMgrFuncBit_QBN2,               // 1    4910
    /* LCS is set in notify() of NormalPipe */
    //eIspTuningMgrFuncBit_LCS,                // 6    47E0~47F4
    eIspTuningMgrFuncBit_AA,                 // 67   4920~4A34
    eIspTuningMgrFuncBit_SGG1,               // 3    4900~4908
    eIspTuningMgrFuncBit_AF,                 // 16   4800~483C
    eIspTuningMgrFuncBit_UNP2,               // 1    3500
    //eIspTuningMgrFuncBit_QBN3,               // 1    3510
    eIspTuningMgrFuncBit_SGG3,               // 3    3520~3528
    eIspTuningMgrFuncBit_FLK,                // 4    3530~353C
    /* EIS & RSS are set in notify() of NormalPipe */
    //eIspTuningMgrFuncBit_SGG2,             // 3    3590~3598
    //eIspTuningMgrFuncBit_HDS,              // 1    35A0
    //eIspTuningMgrFuncBit_EIS,                // 9    3550~3570
    //eIspTuningMgrFuncBit_RSS,              // 9    35C0~35E0
    eIspTuningMgrFuncBit_SL2F,                //10   4C40~4C64
    eIspTuningMgrFuncBit_PBN,                // 3    4BB0~4BB8
    eIspTuningMgrFuncBit_PS,                 // 12   4D10~4D30
    eIspTuningMgrFuncBit_SGG5,               // 3    4760~4768
    eIspTuningMgrFuncBit_HLR,
    eIspTuningMgrFuncBit_SL2J,
    eIspTuningMgrFuncBit_PDE,                // 1
    eIspTuningMgrFuncBit_Num   //can't over 32
};

enum EIspTuningMgrFunc  // need mapping to ETuningMgrFunc
{
    eIspTuningMgrFunc_Null  = 0x00000000,
    eIspTuningMgrFunc_DBS   =   (1 << eIspTuningMgrFuncBit_DBS),
    eIspTuningMgrFunc_OBC   =   (1 << eIspTuningMgrFuncBit_OBC),
    eIspTuningMgrFunc_RMG   =   (1 << eIspTuningMgrFuncBit_RMG),
    eIspTuningMgrFunc_BNR   =   (1 << eIspTuningMgrFuncBit_BNR),
    eIspTuningMgrFunc_RMM   =   (1 << eIspTuningMgrFuncBit_RMM),
    eIspTuningMgrFunc_CAC   =   (1 << eIspTuningMgrFuncBit_CAC),
    eIspTuningMgrFunc_LSC   =   (1 << eIspTuningMgrFuncBit_LSC),
    eIspTuningMgrFunc_RCP   =   (1 << eIspTuningMgrFuncBit_RCP),
    eIspTuningMgrFunc_RPG   =   (1 << eIspTuningMgrFuncBit_RPG),
    eIspTuningMgrFunc_UFE   =   (1 << eIspTuningMgrFuncBit_UFE),
    eIspTuningMgrFunc_RCP3  =   (1 << eIspTuningMgrFuncBit_RCP3),
    eIspTuningMgrFunc_CPG   =   (1 << eIspTuningMgrFuncBit_CPG),
    eIspTuningMgrFunc_AA    =   (1 << eIspTuningMgrFuncBit_AA),
    eIspTuningMgrFunc_SGG1  =   (1 << eIspTuningMgrFuncBit_SGG1),
    eIspTuningMgrFunc_AF    =   (1 << eIspTuningMgrFuncBit_AF),
    eIspTuningMgrFunc_UNP2  =   (1 << eIspTuningMgrFuncBit_UNP2),
    eIspTuningMgrFunc_SGG3  =   (1 << eIspTuningMgrFuncBit_SGG3),
    eIspTuningMgrFunc_FLK   =   (1 << eIspTuningMgrFuncBit_FLK),
    eIspTuningMgrFunc_SL2F   =  (1 << eIspTuningMgrFuncBit_SL2F),
    eIspTuningMgrFunc_PBN   =   (1 << eIspTuningMgrFuncBit_PBN),
    eIspTuningMgrFunc_PS    =   (1 << eIspTuningMgrFuncBit_PS),
    eIspTuningMgrFunc_SGG5  =   (1 << eIspTuningMgrFuncBit_SGG5),
    eIspTuningMgrFunc_HLR   =   (1 << eIspTuningMgrFuncBit_HLR),
    eIspTuningMgrFunc_SL2J  =   (1 << eIspTuningMgrFuncBit_SL2J),
    eIspTuningMgrFunc_PDE   =   (1 << eIspTuningMgrFuncBit_PDE),
    eIspTuningMgrFunc_Num   =   (1 << eIspTuningMgrFuncBit_Num)
};


typedef struct stTuningNode_t
{
    MUINT32* pTuningRegBuf;
    MINT32 bufSize;
    MINT32 memID;
    MINT32 magicNum;
    EIspTuningMgrFunc eUpdateFuncBit; // record update engine
    MUINT32 ctlEn_CAM; // used to en/disable update engine
    MUINT32 ctlEnDMA_CAM; // used to en/disable update engine
    MUINT32 ctlEn_UNI; // used to en/disable update engine
    MUINT32 ctlEnDMA_UNI; // used to en/disable update engine

    struct{
        MUINT32     FD;
        MUINTPTR    VA_addr;
    }Dmai_bufinfo[eIspTuningMgrFuncBit_Num];          //for input dam buf va & fd only

    MBOOL isVerify; // if this tuning node need to be verifed by user after drv deque
}stTuningNode;


#if 0
typedef struct
{
    EIspTuningMgrFuncBit eTuningFuncBit;
    MINT32 ctlEn_CAM_Bit;
    MINT32 ctlEnDMA_CAM_Bit;
    MINT32 ctlEn_UNI_Bit;
    MINT32 ctlEnDMA_UNI_Bit;
    //
    E_CAM_MODULE eTuningCqFunc1;  // cq function: module1
    E_CAM_MODULE eTuningCqFunc2;  // cq function: module2
    E_CAM_MODULE eTuningCqFunc3;  // cq function: dma
}ISP_TURNING_FUNC_BIT_MAPPING;
#endif

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
        virtual MBOOL deTuningQue(ETuningDrvUser user, MINT32* magicNum, MBOOL* isVerify, vector<stTuningNode*> &vpTuningNodes) = 0; // deTuningQue by tuning user
        virtual MBOOL enTuningQue(ETuningDrvUser user) = 0; // enTuningQue by tuning user
        virtual MBOOL deTuningQueByDrv(ETuningDrvUser user, MINT32* magicNum, vector<stTuningNode*> &vpTuningNodes) = 0; // deTuningQue by p1 driver
        virtual MBOOL enTuningQueByDrv(ETuningDrvUser user) = 0; // enTuningQue by p1 driver
        virtual MUINT32 getBQNum() = 0; // get mBQNum value
        virtual MBOOL getIsApplyTuning(ETuningDrvUser user) = 0;

        virtual MBOOL uninit(const char* userName, ETuningDrvUser user) = 0;
        virtual MBOOL searchVerifyTuningQue(ETuningDrvUser user, MINT32 magicNum, stTuningNode* &vpTuningNodes) = 0;

    private:
        virtual MBOOL searchTuningQue(ETuningDrvUser user, MINT32* magicNum) = 0;
};

#endif

