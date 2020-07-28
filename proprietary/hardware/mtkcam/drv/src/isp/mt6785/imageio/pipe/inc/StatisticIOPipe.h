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
#ifndef _STATISTICIOPIPE_H_
#define _STATISTICIOPIPE_H_
//
#include <vector>
#include <map>
#include <list>

//
using namespace std;
//
#include <ispio_pipe_ports.h>
#include <ispio_pipe_buffer.h>
//
#include <Cam_Notify_datatype.h>
#include <ICamIOPipe.h>
#include "isp_function_cam.h"
#include "cam_dupCmdQMgr.h"
#include "uni_mgr.h"
#include "twin_mgr.h"

//
/*******************************************************************************
*
********************************************************************************/
namespace NSImageio {
namespace NSIspio   {
////////////////////////////////////////////////////////////////////////////////


/*******************************************************************************
*
********************************************************************************/
class STTIOPipe : public ICamIOPipe
{
//
public:
    static STTIOPipe*   Create(MINT8 const szUsrName[32], const E_INPUT& InPut);
                void    Destroy(void);




                    STTIOPipe();
                    virtual ~STTIOPipe();
private:
    typedef enum _E_FSM{
        op_unknown  = 0,
        op_init     = 1,
        op_cfg      = 2,
        op_start    = 3,
        op_stop     = 4,
        op_uninit   = 5,
        op_cmd      = 6,
        op_endeq    = 7,
        op_suspend  = 8,
        op_resume   = 9
    }E_FSM;

    class ResMgr{
        public:
            ResMgr(){
                m_occupied = MFALSE;
                m_User[0] = '\0';
                m_hwModule = PHY_CAM;

            }

            MBOOL   Register_Res(char Name[32]);
            MBOOL   Release_Res(void);


            char*   Get_Res(void);                        // if [0] = '\0' -> available, else -> occupied
        public:
            ISP_HW_MODULE   m_hwModule;
        private:
            MBOOL           m_occupied;
            char            m_User[32];
            mutable Mutex   mLock;
    };

    class TWIN_STT{
        public:
            TWIN_STT(){
                memset((void*)m_pTwinCQMgr,0,sizeof(DupCmdQMgr*)*ISP_DRV_CAM_BASIC_CQ_NUM*PHY_CAM);
                m_hwModule = PHY_CAM;
            }
        public:
            DupCmdQMgr*             m_pTwinCQMgr[ISP_DRV_CAM_BASIC_CQ_NUM][PHY_CAM]; //twinCQMgr, to simulate twinmgr's behavior

            DMA_AFO                 m_AFO[PHY_CAM];      //add afo &afo's fbc is for enque/deque is multi-thread. using mutex will have performance defect(enque is blocking by deque)
            BUF_CTRL_AFO            m_AFO_FBC[PHY_CAM];
        private:
            ISP_HW_MODULE           m_hwModule;
        public:
            MBOOL           clear(void);
    };

public:     ////    Instantiation.
    virtual MBOOL   init();
    virtual MBOOL   uninit();

public:     ////    Operations.
    virtual MBOOL   start();
    virtual MBOOL   stop(MBOOL bForce = MFALSE);

    virtual MBOOL   suspend(E_SUSPEND_MODE sMode);
    virtual MBOOL   resume(E_SUSPEND_MODE sMode);

    virtual MBOOL   abortDma(PortID port);

public:     ////    Buffer Quening.

    virtual MBOOL   enqueInBuf(PortID const portID, QBufInfo const& rQBufInfo){portID;rQBufInfo;return MFALSE;}
    virtual MBOOL   dequeInBuf(PortID const portID, QTimeStampBufInfo& rQBufInfo, MUINT32 const u4TimeoutMs = 0xFFFFFFFF){portID;rQBufInfo;u4TimeoutMs;return MFALSE;}

    //enque can only 1 time per-frame , otherwise, STT header will be over-written
    virtual MINT32  enqueOutBuf(PortID const portID, QBufInfo const& rQBufInfo, MBOOL bImdMode = 0);
    virtual E_BUF_STATUS dequeOutBuf(PortID const portID, QTimeStampBufInfo& rQBufInfo, MUINT32 const u4TimeoutMs = 0xFFFFFFFF, CAM_STATE_NOTIFY *pNotify = NULL);

    //
    virtual MBOOL   configFrame(E_FRM_EVENT event,CAMIO_Func func);

public:     ////    Settings.
    virtual MBOOL   configPipe(vector<PortInfo const*>const& vInPorts, vector<PortInfo const*>const& vOutPorts, CAMIO_Func const *pInFunc, CAMIO_Func *pOutFunc = NULL);

public:     ////    original style sendCommand method
    virtual MBOOL   sendCommand(MINT32 cmd, MINTPTR arg1, MINTPTR arg2, MINTPTR arg3);

    virtual MINT32  registerIrq(MINT8 const szUsrName[32]){szUsrName;return 1;}
    virtual MBOOL   irq(Irq_t* pIrq){pIrq;return MFALSE;}
    virtual MBOOL   signalIrq(Irq_t irq){irq;return MFALSE;}
    virtual MUINT32* getIspReg(MUINT32 path){path;return NULL;}
    virtual MUINT32 ReadReg(MUINT32 addr,MBOOL bCurPhy){bCurPhy;addr;return 0;}
    virtual MUINT32 UNI_ReadReg(MUINT32 addr,MBOOL bCurPhy){bCurPhy;addr;return 0;}
//
private:
            _isp_dma_enum_  PortID_MAP(MUINT32 PortID);

            MBOOL           FSM_CHECK(MUINT32 op);
            MBOOL           FSM_UPDATE(MUINT32 op);
            MBOOL           HeaderUpdate(stISP_BUF_INFO &bufUpdate, stISP_BUF_INFO const& buf,
                                         MUINT32 dmaChannel, MUINT32 SalveIdx);
            MUINT32         AlignResizeHeaderSize(MUINT32 headerSize, MUINT32 align_num);
            inline MBOOL    bufInfoCopy(stISP_BUF_INFO *bufInfoUpdate, stISP_BUF_INFO *bufInfo);
private:
    ResMgr                  m_ResMgr;

    DupCmdQMgr*             m_pCmdQMgr[ISP_DRV_CAM_BASIC_CQ_NUM];

    mutable Mutex           m_FSMLock;
    E_FSM                   m_FSM;

    ISP_HW_MODULE           m_hwModule;

    MBOOL                   m_byPassAAO;
    DMA_AAO                 m_AAO;
    BUF_CTRL_AAO            m_AAO_FBC;

    MBOOL                   m_byPassAFO;
    DMA_AFO                 m_AFO;
    BUF_CTRL_AFO            m_AFO_FBC;

    MBOOL                   m_byPassPDO;
    DMA_PDO                 m_PDO;
    BUF_CTRL_PDO            m_PDO_FBC;
    CAM_DPD_CTRL            m_DPD_CTRL;

    MBOOL                   m_byPassFLKO;
    DMA_FLKO                m_FLKO;
    BUF_CTRL_FLKO           m_FLKO_FBC;

    MBOOL                   m_byPassPSO;
    DMA_PSO                 m_PSO;
    BUF_CTRL_PSO            m_PSO_FBC;
    CAM_SPS_CTRL            m_SPS_CTRL;

    MRect                   m_Xmx[2];   //0:pmx,1:1:bmx

    //DMA_FLKO                m_FLKO;
    //BUF_CTRL_FLKO           m_FLKO_FBC;

    TWIN_STT                m_Twin;
    MUINT32                 m_subsample;
    MUINT32                 m_DualPDDensity;

};


////////////////////////////////////////////////////////////////////////////////
};  //namespace NSIspio
};  //namespace NSImageio
#endif


