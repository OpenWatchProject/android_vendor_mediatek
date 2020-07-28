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
#ifndef _ISPIO_POSTPROC_PIPE_H_
#define _ISPIO_POSTPROC_PIPE_H_
//
#include <vector>
#include <map>
#include <list>
//
using namespace std;
//
//
#include <ispio_pipe_ports.h>
#include <ispio_pipe_buffer.h>
//
#include "cam_path_dip.h"
#include <IPostProcPipe.h>
#include <isp_function_dip.h>
#include "dip_CmdQMgr.h"
/*******************************************************************************
*
********************************************************************************/
namespace NSImageio {
namespace NSIspio   {
////////////////////////////////////////////////////////////////////////////////

#define DMA_PORT_TYPE_NUM   9
#define DMA_OUT_PORT_NUM    7


/*******************************************************************************
*
********************************************************************************/

struct Scen_Map_CropPathInfo_STRUCT{
    EDrvScenario u4DrvScenId;
    MUINT32 u4CropGroup;
    MINT32  u4PortID[DMA_PORT_TYPE_NUM];
    Scen_Map_CropPathInfo_STRUCT()
    {
       u4DrvScenId = eDrvScenario_P2A;
       u4CropGroup = 0;
       u4PortID[0] = EPortIndex_IMG2O;
       u4PortID[1] = EPortIndex_IMG2O;
       u4PortID[2] = EPortIndex_IMG2O;
       u4PortID[3] = EPortIndex_IMG2O;
       u4PortID[4] = EPortIndex_IMG2O;
       u4PortID[5] = EPortIndex_IMG2O;
       u4PortID[6] = EPortIndex_IMG2O;
       u4PortID[7] = EPortIndex_IMG2O;
       u4PortID[8] = EPortIndex_IMG2O;
    }

    Scen_Map_CropPathInfo_STRUCT(EDrvScenario drvScenId,
        MUINT32 cropGroup, MINT32 portid[])
    {
       u4DrvScenId = drvScenId;
       u4CropGroup = cropGroup;
       u4PortID[0] = portid[0];
       u4PortID[1] = portid[1];
       u4PortID[2] = portid[2];
       u4PortID[3] = portid[3];
       u4PortID[4] = portid[4];
       u4PortID[5] = portid[5];
       u4PortID[6] = portid[6];
       u4PortID[7] = portid[7];
       u4PortID[8] = portid[8];
    }
};

/*******************************************************************************
*
********************************************************************************/
class PostProcPipe : public IPostProcPipe
{
public:     ////    Constructor/Destructor.
                    PostProcPipe();

                    virtual ~PostProcPipe();

public:     ////    Instantiation.
    virtual MBOOL   init(char const* szCallerName);
    virtual MBOOL   uninit(char const* szCallerName);

public:     ////    Operations.
    virtual MBOOL   start(void* pParam);
    virtual MBOOL   stop(void* pParam);

public:     ////    Buffer Quening.
    virtual MBOOL   dequeMdpFrameEnd(MINT32 const eDrvSce, MINT32 dequeCq=0,MINT32 const dequeBurstQIdx=0, MINT32 const dequeDupCqIdx=0, MUINT32 const moduleIdx=0);
    virtual MBOOL   enqueInBuf(PortID const portID, QBufInfo const& rQBufInfo,MINT32 enqueCq,MINT32 dupCqIdx);
    virtual MUINT32  dequeInBuf(PortID const portID, QTimeStampBufInfo& rQBufInfo,EDrvScenario eDrvSce=eDrvScenario_P2A, MINT32 dequeCq=0,MINT32 const dequeBurstQIdx=0, MINT32 const dequeDupCqIdx=0, MUINT32 const moduleIdx=0, MUINT32 const u4TimeoutMs = 0xFFFFFFFF);
    //
    virtual MBOOL   enqueOutBuf(PortID const portID, QBufInfo const& rQBufInfo,MINT32 enqueCq,MINT32 dupCqIdx);
    virtual MUINT32   dequeOutBuf(PortID const portID, QTimeStampBufInfo& rQBufInfo,EDrvScenario eDrvSce=eDrvScenario_P2A, MINT32 dequeCq=0,MINT32 const dequeBurstQIdx=0, MINT32 const dequeDupCqIdx=0, MUINT32 const moduleIdx=0, MUINT32 const u4TimeoutMs = 0xFFFFFFFF);

    virtual MBOOL   queryCropPathNum(EDrvScenario drvScen, MUINT32& pathNum);

    virtual MBOOL   queryScenarioInfo(EDrvScenario drvScen, vector<CropPathInfo>& vCropPaths);

public:     ////    Settings.
    virtual MBOOL   configPipe(vector<PortInfo const*>const& vInPorts, vector<PortInfo const*>const& vOutPorts, PipePackageInfo *pPipePackageInfo);

public:     ////    original style sendCommand method
    virtual MBOOL   sendCommand(MINT32 cmd, MINTPTR arg1, MINTPTR arg2, MINTPTR arg3);

public:     ////    enqueue/dequeue control in ihalpipewarpper
    virtual MBOOL   bufferQueCtrl(EPIPE_P2BUFQUECmd cmd, EPIPE_P2engine p2Eng, MUINT32 callerID, MINT32 p2CQ, MINT32 p2dupCQIdx, MINT32 p2burstQIdx, MINT32 frameNum, MINT32 timeout=0);
public:
    //
    virtual MUINT32 queryCQ(EDrvScenario sDrvScen, MUINT32 sensorDev, MINT32 p2burstQIdx, MINT32 frameNum);
    virtual MBOOL updateCQUser(MINT32 p2CQ);
public:     ////
    virtual MBOOL   getCrzPosition(EDrvScenario drvScen, MBOOL &isSl2eAheadCrz, MBOOL &isLceAheadCrz);
//
private:
    MBOOL HandleExtraCommand(PipePackageInfo *pPipePackageInfo);
    MBOOL getCropFunctionEnable(EDrvScenario drvScen, MINT32 portID, MUINT32& CropGroup);
    MBOOL configMdpOutPort(PortInfo const* oImgInfo, MdpRotDMACfg &a_rotDma);
    MBOOL configDmaPort(PortInfo const* portInfo,IspDMACfg &a_dma,MUINT32 pixel_Byte,MUINT32 swap, MUINT32 isBypassOffset,EIMAGE_STRIDE planeNum);

    IspDrvShell*             m_pIspDrvShell;
    DipCmdQMgr*             m_dipCmdQMgr;
    CamPathPass2             m_CamPathPass2;
    CamPathPass2Parameter    m_camPass2Param;
    vector<BufInfo>          m_vBufImgi;
    vector<BufInfo>          m_vBufVipi;
    vector<BufInfo>          m_vBufVip2i;
    vector<BufInfo>          m_vBufDispo;
    vector<BufInfo>          m_vBufVido;
    MINT8                    m_szUsrName[32];
    //
    //jpeg
    MINT32  jpg_WorkingMem_memId;
    MUINT32 jpg_WorkingMem_Size;
    MUINT32 *pJpgConfigVa;

    MUINT32 cq1_size,cq1_phy;
    MUINT8 *cq1_vir;
    MINT32 cq1_memId;
    MUINT32 cq2_size,cq2_phy;
    MUINT8 *cq2_vir;
    MINT32 cq2_memId;
    MUINT32 cq3_size,cq3_phy;
    MUINT8 *cq3_vir;
//    MINT32 cq3_memId;
    //
    EPipePass   m_pipePass;
    //
    MINT32  m_pass2_CQ;
    //
    MBOOL   m_isImgPlaneByImgi;
    //
    MBOOL   m_Nr3dEn;
    MBOOL   m_Nr3dDmaSel;
    //
    MBOOL   m_CrzEn;
    MBOOL   m_SeeeEn;
    //
};


////////////////////////////////////////////////////////////////////////////////
};  //namespace NSIspio
};  //namespace NSImageio
#endif  //  _ISPIO_POSTPROC_PIPE_H_

