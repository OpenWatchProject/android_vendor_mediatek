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


typedef struct
{
    DUAL_IN_CONFIG_STRUCT*  dualInCfg;
    DUAL_OUT_CONFIG_STRUCT* dualOutCfg;
    ISP_REG_PTR_STRUCT*     dualIspReg;

}TWIN_DUAL_INPUT_PARAM;

class TwinMgr_BufCtrl{
public:
    TwinMgr_BufCtrl();
    ~TwinMgr_BufCtrl();


    MBOOL   TBC_config(CAM_TWIN_PIPE* pTwinctrl,DupCmdQMgr* pMain,DupCmdQMgr* pTwin,vector<MUINT32>* pchannel,const NSImageio::NSIspio::PortID* pInput);
    MBOOL   TBC_Start(MVOID);
    MBOOL   TBC_Stop(MVOID);

    //enable DMAO/FBC only , case of DMAO disable is covered by twin drv, case of FBC disable is coverd by ifunc_twin
    //push buffer into que only, and open FBC/DMAO based on the record of opened port at TBC_config
    MBOOL   enque_push( MUINT32 const dmaChannel, ISP_BUF_INFO_L& bufInfo);
    MINT32  deque( MUINT32 const dmaChannel ,vector<NSImageio::NSIspio::BufInfo>& bufInfo,CAM_STATE_NOTIFY *pNotify);//deque buffer at twin mode
    //set into hw after twin_drv's cal.
    MINT32  enque_pop(MVOID);
    MINT32  enque_drop(MVOID);

private:
    MBOOL   FSM_UPDATE(MUINT32 op);
    MINT32  enqueHW(MUINT32 const dmaChannel);
    //patch header's va/pa which is affected by DMAO'S offset address
    MBOOL   HEADER_ADDR_HW_LIMITATION(stISP_BUF_INFO& buf,MUINT32 offset);
    //patch header's va/pa which is for separate DMAO under twin mode
    MBOOL   HeaderUpdate(stISP_BUF_INFO& buf,ISP_HW_MODULE curModule);
public:


private:
    //que for sw enque record
    //statistic is not supported,because statistic's enque control timing is not the same with main image
    QueueMgr<ISP_BUF_INFO_L>    m_enque_IMGO;
    QueueMgr<ISP_BUF_INFO_L>    m_enque_RRZO;


    //
    DupCmdQMgr*                 m_pMainCmdQMgr; // CmdQMgr for main hwModule
    DupCmdQMgr*                 m_pTwinCmdQMgr; // CmdQMgr for twin hwModule

    //
    CAM_TWIN_PIPE*              m_pTwinCtrl;

    MBOOL                       m_bEnque;

    //MAIN/twin hwmodule's dma&fbc
    DMA_IMGO        m_Imgo[CAM_MAX];
    BUF_CTRL_IMGO   m_Imgo_FBC[CAM_MAX];
    DMA_RRZO        m_Rrzo[CAM_MAX];
    BUF_CTRL_RRZO   m_Rrzo_FBC[CAM_MAX];


    vector<MUINT32> m_OpenedChannel;

    //
    mutable Mutex               mFSMLock;
    MUINT32                     m_FSM;
    //mutex for subsample
    mutable Mutex   m_lock;//protect deque/enque behavior under subsample
};

class Twin_Resume_check{
public:
    Twin_Resume_check(){m_pCmdQ.clear();m_pTwinpipe = NULL;m_loop=0;}
public:
    MBOOL Resueme_init(vector<DupCmdQMgr*>* pvector,CAM_TWIN_PIPE* pTwinpipe);
    MBOOL Resume_check(void);
private:
    vector<DupCmdQMgr*>   m_pCmdQ;
    CAM_TWIN_PIPE*      m_pTwinpipe;
    MUINT32             m_loop;
};

class TwinMgr_IMP: public TwinMgr
{
public:
    TwinMgr_IMP();
    virtual ~TwinMgr_IMP(){};

    static TwinMgr_IMP*      createInstance(ISP_HW_MODULE hwModule); // Get static instance of TwinMgr by hwModule: CAM_A/B/...
    virtual void             destroyInstance();

    virtual MBOOL                   initPath(); // for dynamic twin, clear all result in queue

    // Must configPath() before call TwinMgr init()
    // Config path according to input params, includes:
    // (1) pixel modes of mux; (2) frontal binning enable/disable; (3) twin enable/disable
    // per-frame ctrl: support dynamic twin, can be ionvoked each frame.
    virtual MBOOL                   configPath(const TWIN_MGR_PATH_CFG_IN_PARAM &cfgInParam, TWIN_MGR_PATH_CFG_OUT_PARAM &cfgOutParam, MUINT32 navailCam);


    ///
    /// Twin 1-time related control
    //
    virtual MBOOL                   initTwin(DupCmdQMgr* pMainCmdQMgr,const NSImageio::NSIspio::PortID* pInput,MUINT32 subSample,vector<MUINT32>* pChannle);
    virtual MBOOL                   uninitTwin();
    virtual MBOOL                   configTwin(void);

    virtual MBOOL                   startTwin(void); // StartTwin: 1. init TwinDrv, 2.RunTwinDrv(), 3. twin cmdQ start
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

    virtual MBOOL                   runTwinDrv(); // Run twin drv
        //twin's dmao ctrl
    virtual MINT32                  enqueTwin( MUINT32 const dmaChannel, ISP_BUF_INFO_L& bufInfo);//enque buffer at twin mode
    virtual MINT32                  dequeTwin( MUINT32 const dmaChannel ,vector<NSImageio::NSIspio::BufInfo>& bufInfo,CAM_STATE_NOTIFY *pNotify);//deque buffer at twin mode
    virtual MBOOL                   dropEnqueTwin(TWIN_MGR_PATH_CFG_IN_PARAM m_cfgInParam, vector<MUINT32>* pLMagicNum);

    //CQ ctrl under twin mode.
    virtual MUINT32                 CQ_cfg(MINTPTR arg1, MINTPTR arg2);
    virtual MUINT32                 CQ_Trig(void);

    virtual MBOOL                   sendCommand(E_TWIN_CMD cmd,MINTPTR arg1,MINTPTR arg2,MINTPTR arg3);

private:
            MUINT32                 chkRRZOutSize(TWIN_MGR_PATH_CFG_IN_PARAM cfgInParam, TWIN_MGR_PATH_CFG_OUT_PARAM &cfgOutParam);
            MBOOL                   cvtDupCmdQRegtoDualIspReg();
            MBOOL                   prepareDualInCfg();
            //get twinmgr's information
            MBOOL                   getTwinALLCropinfo(vector<vector<ISP_HW_MODULE>>* pPath,vector<vector<STImgCrop>>* pcrop);
            MBOOL                   getTwinReg(vector<ISP_HW_MODULE>* pMaster, vector<MUINT32>* pAddr);
            E_TWIN_STATUS           getTwinStatuts(void);//this is an dynamic result after configPath()
            MUINT32                 getTwinHeaderSize(MUINT32 dmaChannel);
            vector<ISP_HW_MODULE>     getCurModule(void);
            DupCmdQMgr*             getTwinCmdQ(ISP_HW_MODULE module,E_ISP_CAM_CQ cq);

    class RACING_PROT{
        public:
            template <typename _typ>
            RACING_PROT(_typ* ptr,_typ value){
                Mutex::Autolock lock(this->lock);
                *ptr = value;
            }
        private:
            mutable Mutex   lock;
    };
    // 1. Update Twin's CQ descriptor according to Main's CQ descriptor
    // 2. Set CAMB's dmx_sel = 1

    typedef enum _E_FSM{
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
    }E_FSM;

    // FSM check current op is corrected or not
            MBOOL                   FSM_CHECK(MUINT32 op);
            MBOOL                   FSM_UPDATE(MUINT32 op);
            MBOOL                   TwinInfo_update(const TWIN_MGR_PATH_CFG_IN_PARAM& cfgInParam,MBOOL isTwin);
private:
    E_FSM m_FSM;

    ISP_HW_MODULE           m_hwModule;
    ISP_HW_MODULE           m_twinHwModule;   // Twin hwmodules
    vector<ISP_HW_MODULE>   m_occupiedModule; // phy module



    // twin
    TwinDrv*                m_pTwinDrv;
    TWIN_DUAL_INPUT_PARAM   m_DualInputParam;

    //current , only these 3 dmao is separated at twin
    QueueMgr<MBOOL>         m_isTwin_AFO;
    QueueMgr<MBOOL>         m_isTwin_IMGO;
    QueueMgr<MBOOL>         m_isTwin_RRZO;
    MBOOL                   m_lateset_AFO;  //latest para. is design for getisTwin is used between deque to next enque.
    MBOOL                   m_lateset_IMGO; //due to drv have no frame_end behavior.
    MBOOL                   m_lateset_RRZO;
    #define MAX_QUE_SIZE    (64)

    //twin's buf ctrl
    TwinMgr_BufCtrl         m_TBC;

    // Use to repsent how many hwModule used when enable twin
    typedef enum _ETwinMgr_TwinNum
    {
        eTwinMgr_none       = 0,
        eTwinMgr_TwinNum_2  = 1,
        //eTwinMgr_TwinNum_3,
        //eTwinMgr_TwinNum_4,
        eTwinMgr_TwinNum_Max
    }ETwinMgr_TwinNum;
    ETwinMgr_TwinNum        m_hwModuleNum;


    DupCmdQMgr*             m_pMainCmdQMgr; // CmdQMgr for main hwModule
    DupCmdQMgr*             m_pTwinCmdQMgr[ISP_DRV_CAM_BASIC_CQ_NUM]; // CmdQMgr for twin hwModule
    ISP_DRV_CAM*            m_pMainIspDrv;
    ISP_DRV_CAM*            m_pTwinIspDrv;

    CAM_TWIN_PIPE           m_TwinCtrl;

    //record opened port at current scenario, input information for twin_drv
    vector<MUINT32>         m_OpenedChannel;

    // mutex
    mutable Mutex           mTwinLock; // for protecting init, uninit twin related control
    mutable Mutex           mFSMLock;

    #define DISABLE_TWIN    (1)
    #define ENABLE_TWIN     (0)
    MUINT32                 m_switchLatency;    //latency to ctrl the fetch-timing of hw pipline.clear occupiedmodule until hw twin is suspended.

    T_XMXO                  m_xmx;              //xmxo result after configpath()
    CAM_TOP_CTRL            m_TopCtrl;          //for hw recover

    struct{
        MUINT32 w;
        MUINT32 h;
        MBOOL   dbn_en;
        MBOOL   bin_en;
    }m_TG_size;

    //
    Twin_Resume_check       m_TwinResume;
};


#endif
