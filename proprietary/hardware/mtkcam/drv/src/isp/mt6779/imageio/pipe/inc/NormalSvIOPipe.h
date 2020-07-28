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
#ifndef _NORMALSVIOPIPE_H_
#define _NORMALSVIOPIPE_H_
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
#include "isp_function_normalsv.h"
#include "isp_drv_camsv.h"
#include "ResMgr.h"
#include "cam_dupCmdQMgr.h"
#include <cam_dfs.h>

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

inline MUINT32 NSVPIPE_MAP_SEN_TG(E_INPUT const tg_input)
{
    MUINT32 _CAM_TG_ = CAM_TG_NONE;

    switch(tg_input){
        case TG_CAMSV_0:
            _CAM_TG_ = CAM_SV_1;
            break;
        case TG_CAMSV_1:
            _CAM_TG_ = CAM_SV_2;
            break;
        case TG_CAMSV_2:
            _CAM_TG_ = CAM_SV_3;
            break;
        case TG_CAMSV_3:
            _CAM_TG_ = CAM_SV_4;
             break;
        case TG_CAMSV_4:
            _CAM_TG_ = CAM_SV_5;
             break;
        case TG_CAMSV_5:
            _CAM_TG_ = CAM_SV_6;
            break;
		#if 0
        case TG_CAMSV_6:
            _CAM_TG_ = CAM_SV_7;
            break;
        case TG_CAMSV_7:
            _CAM_TG_ = CAM_SV_8;
            break;
        #endif
        default:
            break;
    }
    return _CAM_TG_;
}

class NormalSvIOPipe : public ICamIOPipe
{
//
public:
    static NormalSvIOPipe*   Create(MINT8 const szUsrName[32], E_INPUT& InPut);
    static NormalSvIOPipe*   Create(MINT8 const szUsrName[32], vector<E_INPUT>& InOutPut, PortInfo const *portInfo);
                    void     Destroy(void);




                    NormalSvIOPipe();
                    virtual ~NormalSvIOPipe();

private:
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

    MINT32   enqueInBuf(PortID const portID, QBufInfo const& rQBufInfo){portID;rQBufInfo;return 0;}
    E_BUF_STATUS   dequeInBuf(PortID const portID, QTimeStampBufInfo& rQBufInfo, MUINT32 const u4TimeoutMs = 0xFFFFFFFF, CAM_STATE_NOTIFY *pNotify = NULL){portID;rQBufInfo;u4TimeoutMs;return eBuf_Pass;}

    //enque can only 1 time per-frame , otherwise, STT header will be over-written
    MBOOL   enqueOutBuf(PortID const portID, QBufInfo const& rQBufInfo, MBOOL bImdMode = 0);
    E_BUF_STATUS dequeOutBuf(PortID const portID, QTimeStampBufInfo& rQBufInfo, MUINT32 const u4TimeoutMs = 0xFFFFFFFF, CAM_STATE_NOTIFY *pNotify = NULL);

    //
    virtual MBOOL   configFrame(E_FRM_EVENT event,CAMIO_Func func,ST_CAMIO_WT_INFO *wt_info = NULL){event;func;return MFALSE;}
public:     ////    Settings.
    MBOOL   configPipe(vector<PortInfo const*>const& vInPorts, vector<PortInfo const*>const& vOutPorts, CAMIO_Func const *pInFunc, CAMIO_Func *pOutFunc = NULL);

public:     ////    original style sendCommand method
    MBOOL   sendCommand(MINT32 cmd, MINTPTR arg1, MINTPTR arg2, MINTPTR arg3);

    MINT32  registerIrq(MINT8 const szUsrName[32]);
    MBOOL   irq(Irq_t* pIrq);
    MBOOL   signalIrq(Irq_t irq);
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
            MBOOL           configDmaPort(PortInfo const* portInfo,IspDMACfg &a_dma,MUINT32 pixel_Byte,MUINT32 slave=MFALSE,MUINT32 swap=MFALSE, MUINT32 isBypassOffset=MTRUE,E_BufPlaneID planeNum=ePlane_1st);
            MBOOL           configDmaPort(BufInfo bufInfo, IspDMACfg &a_dma);
            MBOOL           getClockSet(void);

private:
    MINT8                   m_szUsrName[32];

    mutable Mutex           m_FSMLock;
    E_FSM                   m_FSM;
    ISP_HW_MODULE           m_hwMasterModule;//CAM A/B/C module for DCIF
    ISP_HW_MODULE           m_hwModule;
    ISP_HW_MODULE           m_hwModuleCamsvSlave;

    IspDrvCamsv*            m_pIspDrvCamsv;
    IspDrvCamsv*            m_pIspDrvCamsvSlave;

    NORMALSV_TG_CTRL        m_TGCtrl;
    NORMALSV_TG_CTRL        m_TGCtrlSlave;
    NORMALSV_TOP_CTRL       m_TopCtrl;
    NORMALSV_TOP_CTRL       m_TopCtrlSlave;
    BUF_CTRL_NORMALSV_IMGO  m_FbcImgo;
    BUF_CTRL_NORMALSV_IMGO  m_FbcImgoSlave;
    DMA_NORMALSV_IMGO       m_DmaImgo;
    DMA_NORMALSV_IMGO       m_DmaImgoSlave;
    NORMALSV_BUF_CTRL       m_BufCtrl;
    ResMgr*                 m_pResMgr;
    ResMgr*                 m_pSlaveResMgr;

    DupCmdQMgr*             m_pCmdQMgr;
    vector<E_INPUT>         InPutTG;
    STImgCrop               crop;
    MUINT32                 m_Subsample;
    MUINT32                 m_Data_Pattern;
    MUINT32                 m_Dcif_En;
    MUINT32                 m_UFEO_En;
    MUINT32                 m_Dcif_MasterRaw;
    MUINT32                 m_TwinMode_En;
    MUINT32                 m_Dcif_Offset;
    vector<MUINT32>         clk_gear;

    //for new bw ctrl
    T_BW_INPUT              m_dfs_info;
    CAM_DFS_Mgr*            m_cam_dfs;

};


////////////////////////////////////////////////////////////////////////////////
};  //namespace NSIspio
};  //namespace NSImageio

#endif // _NORMALSVIOPIPE_H_

