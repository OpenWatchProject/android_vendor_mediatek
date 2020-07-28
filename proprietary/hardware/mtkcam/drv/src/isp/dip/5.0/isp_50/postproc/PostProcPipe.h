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
#include <IPostProcPipe.h>
#include <isp_function_dip.h>
#include "dip_CmdQMgr.h"

#include "isp_drv_dip_platform.h"

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

/******************************************************************************
 *
 * @struct TransformMapping
 * @brief mapping relationship about transformation(rotation and flip).
 * @details
 *
 ******************************************************************************/
struct TransformMapping
{
    MINT32                              eTransform; //eTransform in ImageFormat.h
    NSImageio::NSIspio::EImageRotation  eImgRot;
    NSImageio::NSIspio::EImageFlip      eImgFlip;   //TODO, need judge vflip/hflip
};

struct CropInfo
{
	STImgCrop   crop1;           // image crop info. (ring buffer use curz to run crop)
	STImgResize resize1;
	STImgCrop   crop2;           // image crop info. (ring buffer use curz to run crop)
	STImgResize resize2;
	STImgCrop   crop3;           // image crop info. (ring buffer use curz to run crop)
	STImgResize resize3;
};


enum DipRawPxlID
{
    DipRawPxlID_B   = 0,  // B Gb Gr R
    DipRawPxlID_Gb,       // Gb B R Gr
    DipRawPxlID_Gr,       // Gr R B Gb
    DipRawPxlID_R         // R Gr Gb B
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
    virtual MBOOL   init(char const* szCallerName, MUINT32 secTag);
    virtual MBOOL   uninit(char const* szCallerName);

public:     ////    Operations.
    virtual MBOOL   start(void* pParam = NULL);
    virtual MBOOL   stop(void* pParam = NULL);

public:     ////    Buffer Quening.
    virtual MBOOL   configPipe(const NSCam::NSIoPipe::FrameParams &pFrameParams, PipePackageInfo *pPipePackageInfo, char const* szCallerName);
    virtual MBOOL   dequeMdpFrameEnd(MINT32 const eDrvSce, MINT32 dequeCq=0,MINT32 const dequeBurstQIdx=0, MINT32 const dequeDupCqIdx=0, MUINT32 const moduleIdx=0);

public:     ////    enqueue/dequeue control in ihalpipewarpper
    virtual MBOOL   bufferQueCtrl(EPIPE_P2BUFQUECmd cmd, EPIPE_P2engine p2Eng, MUINT32 callerID, MINT32 p2CQ, MINT32 p2dupCQIdx, MINT32 p2burstQIdx, MINT32 frameNum, MINT32 timeout=0);

    virtual MBOOL   queryCropPathNum(EDrvScenario drvScen, MUINT32& pathNum);
    virtual MBOOL   queryScenarioInfo(EDrvScenario drvScen, vector<CropPathInfo>& vCropPaths);

public:     ////    original style sendCommand method
    virtual MBOOL   sendCommand(MINT32 cmd, MINTPTR arg1, MINTPTR arg2, MINTPTR arg3);
    virtual MUINT32 dequeBuf(MBOOL& isVencContained, MUINTPTR& tpipeVa, MUINTPTR& ispVirRegVa, EDrvScenario eDrvSce=eDrvScenario_P2A, MINT32 dequeCq=0,MINT32 const dequeBurstQIdx=0, MINT32 const dequeDupCqIdx=0, MUINT32 const RingBufIdx=0, MUINT32 const u4TimeoutMs = 0xFFFFFFFF);

    virtual MBOOL   getVssDipWBInfo(MUINTPTR& vsstdriaddr, MINT32 p2cqIdx, MINT32 p2RingBufIdx);
    virtual MBOOL   getDipDumpInfo(MUINT32& tdriaddr, MUINT32& cmdqaddr, MUINT32& imgiaddr, MBOOL& bDequeBufIsTheSameAaGCEDum);
public:
    //
    virtual MUINT32 queryCQ(EDrvScenario sDrvScen, MUINT32 sensorDev, MINT32 p2burstQIdx, MINT32 frameNum, MUINT32& RingBufIndex, MUINTPTR& pVirIspAddr);
    virtual MBOOL updateCQUser(MINT32 p2CQ);
public:     ////
    virtual MBOOL   getCrzPosition(EDrvScenario drvScen, MBOOL &isSl2eAheadCrz, MBOOL &isLceAheadCrz);

    DIP_ISP_PIPE        ispDipPipe;
    DIP_BUF_CTRL        ispBufCtrl;
    DIP_MDP_PIPE        ispMdpPipe;

//
private:
    MBOOL HandleExtraCommand(PipePackageInfo *pPipePackageInfo);
    MBOOL PrintPipePortInfo(const NSCam::NSIoPipe::FrameParams &pFrameParams);
    DipRawPxlID PixelIDMapping(MUINT32 pixIdP2);

    MBOOL getCropFunctionEnable(EDrvScenario drvScen, MINT32 portID, MUINT32& CropGroup);
    MBOOL configMdpOutPort(NSCam::NSIoPipe::Output const* oImgInfo, MdpRotDMACfg &a_rotDma, NSCam::EImageFormat eImgFmt);
    MBOOL configInDmaPort(NSCam::NSIoPipe::Input const* portInfo,IspDMACfg &a_dma,MUINT32 pixel_Byte,MUINT32 swap, MUINT32 isBypassOffset,EIMAGE_STRIDE planeNum, NSCam::EImageFormat eImgFmt);
    MBOOL configOutDmaPort(NSCam::NSIoPipe::Output const* portInfo,IspDMACfg &a_dma,MUINT32 pixel_Byte,MUINT32 swap, MUINT32 isBypassOffset,EIMAGE_STRIDE planeNum, NSCam::EImageFormat eImgFmt);
    MBOOL configSrz(SrzCfg* pSrz_cf, _SRZ_SIZE_INFO_* pSrz_size_info, EDipModule srzEnum);
    MBOOL doUVSwap(MUINTPTR *uv1, MUINTPTR *uv2);

    IspDrvShell*             m_pIspDrvShell;
    DipCmdQMgr*             m_dipCmdQMgr;

    DipRingBuffer*          m_pDipRingBuf;
    DipWorkingBuffer*       m_pDipWorkingBuf;

    DipWorkingBuffer*       m_pDequeDipWorkingBuf;
    volatile MINT32          mp2CQDupIdx; //

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
    MINT32 cq3_memId;

    char *pPortLogBuf;
    char *pMaxLogBuf;
    //
    MBOOL   m_isImgPlaneByImgi;
    //
    MBOOL   m_Nr3dEn;
    MBOOL   m_Nr3dDmaSel;
    //
    MBOOL   m_CrzEn;
    MBOOL   m_SeeeEn;
    //for img3o crsp only, preview will use rrzo or imgo as input, when the input is imgo. the original definition of mOffsetInBytes will have wrong. the hw main isp pipeline size is refer imgi input.
    NSCam::NSIoPipe::CrspInfo* m_Img3oCropInfo;
    
};


////////////////////////////////////////////////////////////////////////////////
};  //namespace NSIspio
};  //namespace NSImageio
#endif  //  _ISPIO_POSTPROC_PIPE_H_

