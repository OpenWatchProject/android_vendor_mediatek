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
#ifndef _CAMSVIOPIPE_H_
#define _CAMSVIOPIPE_H_
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
#include <ICamIOPipe.h>
#include "isp_function_camsv.h"
#include "isp_drv_camsv.h"

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




class CamsvIOPipe : public ICamIOPipe
{
//
public:
    static CamsvIOPipe*   Create(MINT8 const szUsrName[32], const E_INPUT& InPut);
                  void    Destroy(void);




                    CamsvIOPipe();
                    virtual ~CamsvIOPipe();

private:
    //
    class ResMgr
    {
        public:
            MBOOL   m_occupied;
            char    m_User[32];
            MUINT32 source;
            ResMgr(){
                m_occupied = MFALSE;
                m_User[0] = '\0';
                source = PHY_CAM;
            }
    };

    typedef enum _E_FSM{
        op_unknown  = 0,
        op_init     = 1,
        op_cfg      = 2,
        op_start    = 3,
        op_stop     = 4,
        op_uninit   = 5,
        op_cmd      = 6,
        op_endeq    = 7
    }E_FSM;

public:     ////    Instantiation.
    MBOOL   init();
    MBOOL   uninit();

    MBOOL   suspend(E_SUSPEND_MODE suspendMode);
    MBOOL   resume(E_SUSPEND_MODE suspendMode);

public:     ////    Operations.
    MBOOL   start();
    MBOOL   stop(MBOOL bForce = MFALSE);
    MBOOL   abortDma(PortID port);

public:     ////    Buffer Quening.

    MBOOL   enqueInBuf(PortID const portID, QBufInfo const& rQBufInfo){portID;rQBufInfo;return MFALSE;}
    MBOOL   dequeInBuf(PortID const portID, QTimeStampBufInfo& rQBufInfo, MUINT32 const u4TimeoutMs = 0xFFFFFFFF){portID;rQBufInfo;u4TimeoutMs;return MFALSE;}

    //enque can only 1 time per-frame , otherwise, STT header will be over-written
    MBOOL   enqueOutBuf(PortID const portID, QBufInfo const& rQBufInfo, MBOOL bImdMode = 0);
    E_BUF_STATUS dequeOutBuf(PortID const portID, QTimeStampBufInfo& rQBufInfo, MUINT32 const u4TimeoutMs = 0xFFFFFFFF, CAM_STATE_NOTIFY *pNotify = NULL);

    //
    virtual MBOOL   configFrame(E_FRM_EVENT event,CAMIO_Func func){event;func;return MFALSE;}
public:     ////    Settings.
    MBOOL   configPipe(vector<PortInfo const*>const& vInPorts, vector<PortInfo const*>const& vOutPorts, CAMIO_Func const *pInFunc, CAMIO_Func *pOutFunc = NULL);

public:     ////    original style sendCommand method
    MBOOL   sendCommand(MINT32 cmd, MINTPTR arg1, MINTPTR arg2, MINTPTR arg3);

    MINT32  registerIrq(MINT8 const szUsrName[32]){szUsrName;return 1;}
    MBOOL   irq(Irq_t* pIrq);//{pIrq;return MFALSE;}
    MBOOL   signalIrq(Irq_t irq){irq;return MFALSE;}
    MUINT32* getIspReg(MUINT32 path){path;return NULL;}
    MUINT32 ReadReg(MUINT32 addr,MBOOL bCurPhy){bCurPhy;addr;return 0;}
    MUINT32 UNI_ReadReg(MUINT32 addr,MBOOL bCurPhy){bCurPhy;addr;return 0;}
//
private:
            _isp_dma_enum_  PortID_MAP(MUINT32 PortID);

            MBOOL           FSM_CHECK(MUINT32 op);
            MBOOL           FSM_UPDATE(MUINT32 op);

            MINT32          SrcFmtToHwVal_TG( ImgInfo::EImgFmt_t imgFmt);
            MINT32          SrcFmtToHwVal_TG_SW( ImgInfo::EImgFmt_t imgFmt);
            MINT32          getOutPxlByteNFmt(ImgInfo::EImgFmt_t imgFmt, MINT32* pPixel_byte, MINT32* pFmt);
            MBOOL           configDmaPort(PortInfo const* portInfo,IspDMACfg &a_dma,MUINT32 pixel_Byte,MUINT32 swap=MFALSE, MUINT32 isBypassOffset=MTRUE,E_BufPlaneID planeNum=ePlane_1st);

private:
    MINT8                   m_szUsrName[32];
    MBOOL                   m_occupied;

    mutable Mutex           m_FSMLock;
    E_FSM                   m_FSM;
    ISP_HW_MODULE           m_hwModule;

    IspDrvCamsv*            m_pIspDrvCamsv;

    CAMSV_TG_CTRL           m_TGCtrl;
    CAMSV_TOP_CTRL          m_TopCtrl;
    BUF_CTRL_CAMSV_IMGO     m_FbcImgo;
    DMA_CAMSV_IMGO          m_DmaImgo;
    CAMSV_BUF_CTRL          m_BufCtrl;
    ResMgr                  m_ResMgr;
};


////////////////////////////////////////////////////////////////////////////////
};  //namespace NSIspio
};  //namespace NSImageio

#endif // _CAMSVIOPIPE_H_


