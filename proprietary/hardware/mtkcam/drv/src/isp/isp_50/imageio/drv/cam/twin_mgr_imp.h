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
 **************************************************/
#ifndef __TWIN_MGR_IMP_H__
#define __TWIN_MGR_IMP_H__

#include "twin_mgr.h"
#include <Cam/buf_que_ctrl.h>
#include "twin_drv.h"
#include "twin_drv_reg.h"
#include "dual_isp_config.h"


typedef struct {
    DUAL_IN_CONFIG_STRUCT*  dualInCfg;
    DUAL_OUT_CONFIG_STRUCT* dualOutCfg;
    ISP_REG_PTR_STRUCT*     dualIspReg;
} TWIN_DUAL_INPUT_PARAM;

class TwinMgr_IMP;
class TwinMgr_BufCtrl : public Path_BufCtrl
{
public:
            TwinMgr_BufCtrl();
            ~TwinMgr_BufCtrl() {};

    virtual MBOOL   PBC_config(void* pPathMgr, vector<DupCmdQMgr*>* pSlave,list<MUINT32>* pchannel,const NSImageio::NSIspio::PortID* pInput);
    virtual MBOOL   PBC_Start(MVOID);
    virtual MBOOL   PBC_Stop(MVOID);

    //enable DMAO/FBC only , case of DMAO disable is covered by twin drv, case of FBC disable is coverd by ifunc_twin
    //push buffer into que only, and open FBC/DMAO based on the record of opened port at TBC_config
    virtual MBOOL   enque_push( MUINT32 const dmaChannel, ISP_BUF_INFO_L& bufInfo, vector<ISP_HW_MODULE>* pTwinVMod);
    virtual MINT32  deque( MUINT32 const dmaChannel ,vector<NSImageio::NSIspio::BufInfo>& bufInfo,CAM_STATE_NOTIFY *pNotify);//deque buffer at twin mode
    //set into hw after twin_drv's cal.
    virtual MINT32  enque_pop(MVOID);
    virtual MBOOL   enque_UF_patch(vector<T_UF>* pUF);    //patch UF's control , due to enqueHW() is run after runtwin().

private:
    MINT32  enqueHW(MUINT32 const dmaChannel);
    //patch header's va/pa which is for separate DMAO under twin mode
    MBOOL   HeaderUpdate(stISP_BUF_INFO& buf, ISP_HW_MODULE curModule);
public:

private:
    //que for sw enque record
    //statistic is not supported,because statistic's enque control timing is not the same with main image
    QueueMgr<ISP_BUF_INFO_L>    m_enque_IMGO;
    QueueMgr<ISP_BUF_INFO_L>    m_enque_RRZO;

    QueueMgr<stISP_BUF_INFO>    m_enqRecImgo;   //keep enque's buffer condition(plane number) for deque
    QueueMgr<stISP_BUF_INFO>    m_enqRecRrzo;   //keep enque's buffer condition(plane number) for deque

    TwinMgr_IMP                 *m_pTwinMgrImp;
    DupCmdQMgr*                 m_pMainCmdQMgr; // CmdQMgr for main hwModule
    vector<DupCmdQMgr*>         m_pActiveTwinCQMgr; // CmdQMgr for twin hwModule, fixed size CAM_MAX

    CAM_TWIN_PIPE*              m_pTwinCtrl;

    //MAIN/twin hwmodule's dma&fbc
    DMA_IMGO        m_Imgo[CAM_MAX];
    BUF_CTRL_IMGO   m_Imgo_FBC[CAM_MAX];
    DMA_RRZO        m_Rrzo[CAM_MAX];
    BUF_CTRL_RRZO   m_Rrzo_FBC[CAM_MAX];

    DMA_UFEO        m_Ufeo[CAM_MAX];
    BUF_CTRL_UFEO   m_Ufeo_FBC[CAM_MAX];
    DMA_UFGO        m_Ufgo[CAM_MAX];
    BUF_CTRL_UFGO   m_Ufgo_FBC[CAM_MAX];

    //mutex for subsample
    mutable Mutex   m_lock; //protect deque/enque behavior under subsample
};

class TwinFlowCheck {
public:
            TwinFlowCheck(){m_pCmdQ.clear();m_pTwinpipe = NULL;m_loop=0; m_twinFlowState=E_TWIN_FLOW_NONE;m_hwModule=CAM_MAX;}
    MBOOL   resumeInit(vector<DupCmdQMgr*>* pvector,CAM_TWIN_PIPE* pTwinpipe);
    MBOOL   suspendInit(vector<DupCmdQMgr*>* pvector,CAM_TWIN_PIPE* pTwinpipe);
    MBOOL   perFrameCheck(void);


    typedef enum _E_TWIN_FLOW_STATE {
        E_TWIN_FLOW_NONE = 0,
        E_TWIN_FLOW_SUSPEND,
        E_TWIN_FLOW_RESUME,
        E_TWIN_FLOW_MAX
    } E_TWIN_FLOW_STATE;

private:
    vector<DupCmdQMgr*>     m_pCmdQ;
    CAM_TWIN_PIPE*          m_pTwinpipe;
    MUINT32                 m_loop;
    ISP_HW_MODULE           m_hwModule;
    E_TWIN_FLOW_STATE       m_twinFlowState;
};

class TwinMgr_IMP: public TwinMgr {
public:
    friend class TwinMgr_BufCtrl;
                                    TwinMgr_IMP();
    virtual                         ~TwinMgr_IMP() {};

    static TwinMgr_IMP*             createInstance(ISP_HW_MODULE hwModule); // Get static instance of TwinMgr by hwModule: CAM_A/B/...
    virtual void                    destroyInstance();

    virtual MINT32                  init(DupCmdQMgr* pMainCmdQMgr); // for dynamic twin, clear all result in queue
    virtual MINT32                  uninit(void);
    virtual MINT32                  start(void* pParam);
    virtual MINT32                  stop(void* pParam);

    virtual MINT32                  config(struct CamPathPass1Parameter* p_parameter);
    virtual MINT32                  setP1Update(void);
    virtual MINT32                  setP1RrzCfg(list<IspRrzCfg>* pRrz_cfg_L);
    virtual MINT32                  setP1ImgoCfg(list<DMACfg>* pImgo_cfg_L);
    virtual MINT32                  setP1TuneCfg(list<IspP1TuningCfg>* pTune_L);
    virtual MINT32                  setP1Notify(void);
    virtual MINT32                  enqueueBuf(MUINT32 const dmaChannel, ISP_BUF_INFO_L& bufInfo);
    virtual MINT32                  dequeueBuf(MUINT32 const dmaChannel ,vector<NSImageio::NSIspio::BufInfo>& bufInfo, CAM_STATE_NOTIFY *pNotify = NULL);
    virtual MBOOL                   P1Notify_Mapping(MUINT32 cmd,P1_TUNING_NOTIFY* pObj);
    virtual MBOOL                   HW_recover(E_CAMPATH_STEP op);
    virtual MBOOL                   updateFrameTime(MUINT32 timeInMs, MUINT32 reqOffset);
    virtual MBOOL                   updatePath(UpdatePathParam pathParm);

    // Must configPath() before call TwinMgr init()
    // Config path according to input params, includes:
    // (1) pixel modes of mux; (2) frontal binning enable/disable; (3) twin enable/disable
    // per-frame ctrl: support dynamic twin, can be ionvoked each frame.
    virtual MBOOL                   pathControl(TWIN_MGR_PATH_CFG_IN_PARAM &cfgInParam);

    /// Twin 1-time related control
    virtual MBOOL                   initTwin(DupCmdQMgr* pMainCmdQMgr,const NSImageio::NSIspio::PortID* pInput,MUINT32 subSample,list<MUINT32>* pChannle);
    virtual MBOOL                   uninitTwin();

    virtual MBOOL                   startTwin(MBOOL bStreaming = MTRUE); // StartTwin: 1. init TwinDrv, 2.RunTwinDrv(), 3. twin cmdQ start
    virtual MBOOL                   stopTwin(); // Uninit TwinDrv

    virtual MBOOL                   suspend(void);                  // suspend master & slave cam by semaphore
    virtual MBOOL                   resume(void);                   // resume master & slave cam by post semaphore

    virtual MBOOL                   recoverTwin(E_CAMPATH_STEP step);   //stop & reset master & salve cam / restart master & slave cam

    //per-frame control
    virtual MBOOL                   suspendTwin(E_SUSPEND_OP op);
    virtual MBOOL                   resumeTwin(void);
    virtual MBOOL                   updateTwin(); // Pass CQ baseaddr and related info to twin_drv and re-compute CQ data => EPIPECmd_SET_P1_UPDATE
    virtual MBOOL                   getIsTwin(MUINT32 dmaChannel,MBOOL bDeque = MFALSE); // Get m_isTwin is TRUE or FALSE,bDeque can only be true if function is involked at deque()
    virtual MBOOL                   popTwinRst(MUINT32 dmaChannel); // for dynamic twin, need to pop result after deque . this result is pushed perframe
    virtual MBOOL                   getIsTwin(MBOOL bDeque = MFALSE); // this is for camio only,bDeque can only be true if function is involked at deque()
    virtual MBOOL                   getIsInitTwin();
    virtual MBOOL                   getIsStartTwin();

    virtual MBOOL                   runTwinDrv(); // Run twin drv
        //twin's dmao ctrl
    virtual MINT32                  enqueTwin( MUINT32 const dmaChannel, ISP_BUF_INFO_L& bufInfo);//enque buffer at twin mode
    virtual MINT32                  dequeTwin( MUINT32 const dmaChannel ,vector<NSImageio::NSIspio::BufInfo>& bufInfo,CAM_STATE_NOTIFY *pNotify);//deque buffer at twin mode

    //CQ ctrl under twin mode.
    virtual MUINT32                 CQ_cfg(MINTPTR arg1, MINTPTR arg2);
    virtual MUINT32                 CQ_Trig(void);

    virtual MBOOL                   sendCommand(E_TWIN_CMD cmd,MINTPTR arg1,MINTPTR arg2,MINTPTR arg3);

private:
            MUINT32                 chkRRZOutSize(TWIN_MGR_PATH_CFG_IN_PARAM cfgInParam, TWIN_MGR_PATH_CFG_OUT_PARAM &cfgOutParam);
            MBOOL                   cvtDupCmdQRegtoDualIspReg(vector<ISP_DRV_CAM *> &rTwinDrvs);
            MBOOL                   prepareDualInCfg();
            //get twinmgr's information
            MBOOL                   getTwinALLCropinfo(vector<vector<ISP_HW_MODULE>>* pPath,vector<vector<STImgCrop>>* pcrop);
            MBOOL                   getTwinReg(vector<ISP_HW_MODULE>* pMaster, vector<MUINT32>* pAddr);
            E_TWIN_STATUS           getTwinStatuts(void);//this is an dynamic result after configPath()
            MUINT32                 getTwinHeaderSize(MUINT32 dmaChannel);
            list<ISP_HW_MODULE>     getCurModule(void);
            DupCmdQMgr*             getTwinCmdQ(ISP_HW_MODULE module,E_ISP_CAM_CQ cq);

    class RACING_PROT {
    public:
        template <typename _typ>
        RACING_PROT(_typ* ptr,_typ value) {
            Mutex::Autolock lock(this->lock);
            *ptr = value;
        }
    private:
        mutable Mutex   lock;
    };

    // 1. Update Twin's CQ descriptor according to Main's CQ descriptor
    // 2. Set CAMB's dmx_sel = 1
    typedef enum _E_FSM {
        op_unknown  = 0,
        op_pathctrl = 1,
        op_init     = 2,
        op_cfg      = 3,
        op_startTwin= 4,
        op_stopTwin = 5,
        op_uninit   = 6,
        op_twininfo = 7,
        op_runtwin  = 8,
        op_suspendTwin  = 9,    //this status is stand for sw ran into suspend, not HW is already suspended
    } E_FSM;

    // FSM check current op is corrected or not
            MBOOL                   FSM_CHECK(MUINT32 op);
            MBOOL                   FSM_UPDATE(MUINT32 op);

            MBOOL                   twinInfoUpdate(const TWIN_MGR_PATH_CFG_IN_PARAM& cfgInParam,MBOOL isTwin);
private:
    E_FSM                   m_FSM;

    list<ISP_HW_MODULE>     m_occupiedModule;   //phyical hw module: CAM_A/B/C
    vector<ISP_HW_MODULE>   m_twinVHWModules;   //virtual hw module: CAM_A/B/C, CAM_A/B_TWIN_C, CAM_A_TRIPLE_X
    vector<ISP_HW_MODULE>   m_appliedTwinVMod; // CQ0 CmdQMgr of currently running

    TwinDrv*                m_pTwinDrv;
    TWIN_DUAL_INPUT_PARAM   m_DualInputParam;

    // current , only these 3 dmao is separated at twin
    QueueMgr<MBOOL>         m_isTwin_AFO;
    QueueMgr<MBOOL>         m_isTwin_IMGO;
    QueueMgr<MBOOL>         m_isTwin_RRZO;
    MBOOL                   m_lateset_AFO;  //latest para. is design for getisTwin is used between deque to next enque.
    MBOOL                   m_lateset_IMGO; //due to drv have no frame_end behavior.
    MBOOL                   m_lateset_RRZO;
    MBOOL                   m_latestIsTwin;

#define MAX_QUE_SIZE    (64)

    //twin's buf ctrl
    TwinMgr_BufCtrl         m_TBC;

    // Use to repsent how many hwModule used when enable twin
    typedef enum _ETwinMgr_TwinNum {
        eTwinMgr_none       = 0,
        eTwinMgr_TwinNum_2  = 1,
        eTwinMgr_TwinNum_3,
        //eTwinMgr_TwinNum_4,
        eTwinMgr_TwinNum_Max
    } ETwinMgr_TwinNum;
    ETwinMgr_TwinNum        m_hwModuleNum;

    DupCmdQMgr*             m_pMainCmdQMgr; // CmdQMgr for main hwModule
    vector<DupCmdQMgr*>     m_pActiveTwinCQMgrs[ISP_DRV_CAM_BASIC_CQ_NUM];// cq0 depth: CAM_MAX; cq1/cq11 depth: 1~2

    ISP_DRV_CAM*            m_pMainIspDrv;

    CAM_TWIN_PIPE           m_TwinCtrl;

    //record opened port at current scenario, input information for twin_drv
    list<MUINT32>           m_OpenedChannel;

    // mutex
    mutable Mutex           mTwinLock; // for protecting init, uninit twin related control
    mutable Mutex           mFSMLock;

#define DISABLE_TWIN    (1)
#define ENABLE_TWIN     (0)

    T_XMXO                  m_xmx;              //xmxo result after configpath()
    CAM_TOP_CTRL            m_TopCtrl;          //for hw recover

    struct {
        MUINT32 w;
        MUINT32 h;
        MBOOL   dbn_en;
        MBOOL   bin_en;
    } m_TG_size;

    TwinFlowCheck           m_TwinFlowCtrl;
};

static inline ISP_HW_MODULE ModuleVirtToPhy (ISP_HW_MODULE vir_mod)
{
    ISP_HW_MODULE phy_mod;

    switch (vir_mod) {
    case CAM_A:
        phy_mod = CAM_A;
        break;
    case CAM_B:
    case CAM_A_TRIPLE_B:
        phy_mod = CAM_B;
        break;
    case CAM_C:
    case CAM_A_TWIN_C:
    case CAM_B_TWIN_C:
    case CAM_A_TRIPLE_C:
        phy_mod = CAM_C;
        break;
    default:
        phy_mod = CAM_MAX;
        break;
    }

    return phy_mod;
}

// header addr constraint
#define ALIGN_HEAD_SIZE(_n)     (((_n + 15) >> 4) << 4)


#endif
