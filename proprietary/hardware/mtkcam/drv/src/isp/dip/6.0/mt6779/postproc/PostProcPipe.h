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

#include <mtkcam/drv/def/mfbcommon.h>
#include <mtkcam/drv/def/diperror.h>
#include "dip_aee.h"
#include <camera_mfb.h>

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
    //EDrvScenario u4DrvScenId;
    MUINT32 u4DrvScenId;
    MUINT32 u4CropGroup;
    MINT32  u4PortID[DMA_PORT_TYPE_NUM];
    Scen_Map_CropPathInfo_STRUCT()
    {
       u4DrvScenId = NSCam::NSIoPipe::NSPostProc::ENormalStreamTag_Normal;
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

    Scen_Map_CropPathInfo_STRUCT(MUINT32 drvScenId,
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

typedef struct
{
    bool mfbi_en;
    bool mfbi_b_en;
    bool mfb2i_en;
    bool mfb2i_b_en;
    bool mfb3i_en;
    bool mfb4i_en;
    bool mfbo_en;
    bool mfbo_b_en;
    bool mfb2o_en;
}MFB_DRV_DMA_EN_STRUCT;

enum MFBERRCODE {
    MFBERR_NOERROR      = 0,
    MFBERR_BUFFER_EXIST = -1,
    MFBERR_BUFFER_PARAM = -2,
    MFBERR_SRZ_SETTING  = -3,
    MFBERR_TUNNING      = -4
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
    virtual MBOOL   start(MINT32 const p2CQ, MUINT32 const RingBufIdx, MINT32 const burstQIdx, MUINT32 const frameNum);
    virtual MBOOL   stop(void* pParam = NULL);

public:     ////    Buffer Quening.
    virtual MBOOL   checkFrameParamIsTheSame(const NSCam::NSIoPipe::FrameParams &pCurrFrameParams,const NSCam::NSIoPipe::FrameParams &pPrevFrameParams);
    virtual MUINT32 checkDipHWConstrain();
    virtual MBOOL   configPipe(const NSCam::NSIoPipe::FrameParams &pFrameParams, PipePackageInfo *pPipePackageInfo, char const* szCallerName);
    virtual MBOOL   dumpFailFrameSetting(const NSCam::NSIoPipe::FrameParams &pFrameParams, int p2CQ, int p2RingBufIdx,  bool lastframe, char const* szCallerName);
    virtual MBOOL   dequeMdpFrameEnd();

public:     ////    enqueue/dequeue control in ihalpipewarpper
    //virtual MBOOL   bufferQueCtrl(EPIPE_P2BUFQUECmd cmd, EPIPE_P2engine p2Eng, MUINT32 callerID, MINT32 p2CQ, MINT32 p2dupCQIdx, MINT32 p2burstQIdx, MINT32 frameNum, MINT32 timeout=0);

    virtual MBOOL   queryCropPathNum(MUINT32 drvScen, MUINT32& pathNum);
    virtual MBOOL   queryScenarioInfo(MUINT32 drvScen, vector<CropPathInfo>& vCropPaths);

public:     ////    original style sendCommand method
    virtual MBOOL   sendCommand(MINT32 cmd, MINTPTR arg1, MINTPTR arg2, MINTPTR arg3);
    virtual MUINT32   dequeBuf(MBOOL& isVencContained, MUINTPTR& tpipeVa, MUINTPTR& ispVirRegVa, MINT32 dequeCq=0,MUINT32 const RingBufIdx=0);

    virtual MBOOL   getVssDipWBInfo(MUINTPTR& vsstdriaddr, MUINTPTR& vssispviraddr, MINT32 p2cqIdx, MINT32 p2RingBufIdx);
    virtual MBOOL   getDipDumpInfo(MUINT32& tdriaddr, MUINT32& cmdqaddr, MUINT32& imgiaddr, MBOOL& bDequeBufIsTheSameAaGCEDum);
public:
    //
    //virtual MUINT32 queryCQ(EDrvScenario sDrvScen, MUINT32 sensorDev, MINT32 p2burstQIdx, MINT32 frameNum, MUINT32& RingBufIndex, MUINTPTR& pVirIspAddr);
    virtual MUINT32 queryCQ(MUINT32 StreamTag, MUINT32 sensorDev, MINT32 p2burstQIdx, MINT32 frameNum, MUINT32& RingBufIndex, MUINTPTR& pVirIspAddr);
    virtual MBOOL updateCQUser(MINT32 p2CQ);
public:     ////
    virtual MBOOL   getCrzPosition(MUINT32 drvScen, MBOOL &isSl2eAheadCrz, MBOOL &isLceAheadCrz);

    DIP_ISP_PIPE        ispDipPipe;
    DIP_BUF_CTRL        ispBufCtrl;
    //DIP_MDP_PIPE        ispMdpPipe;

//
private:
    MUINT32 VirPortIdxMapToHWPortIdx(MUINT32 index);

    MBOOL HandleExtraCommand(PipePackageInfo *pPipePackageInfo);
    MBOOL PrintPipePortInfo(const NSCam::NSIoPipe::FrameParams &pFrameParams);
    MBOOL PrintMMPathInfo(const NSCam::NSIoPipe::FrameParams &pFrameParams);
    DipRawPxlID PixelIDMapping(MUINT32 pixIdP2);
    MUINT32 checkDMAConstrain(IImageBuffer* pImgBuf, MUINT32 index, char* pAssertBuf);
    MUINT32 checkTuningModuleConstrain(char* pAssertBuf);
    MUINT32 getDMAConstrainErrCode(IImageBuffer* pImgBuf);
    MUINT32 getTuningModuleConstrainErrCode();

    //MBOOL getCropFunctionEnable(EDrvScenario drvScen, MINT32 portID, MUINT32& CropGroup);
    MBOOL getCropFunctionEnable(MUINT32 StreamTag, MINT32 portID, MUINT32& CropGroup);
    MBOOL configMdpOutPort(NSCam::NSIoPipe::Output const* oImgInfo, MdpRotDMACfg &a_rotDma, NSCam::EImageFormat eImgFmt);
    MBOOL configInDmaPort(NSCam::NSIoPipe::Input const* portInfo,IspDMACfg &a_dma,MUINT32 pixel_Byte,MUINT32 swap, MUINT32 isBypassOffset,EIMAGE_STRIDE planeNum, NSCam::EImageFormat eImgFmt);
    MBOOL configOutDmaPort(NSCam::NSIoPipe::Output const* portInfo,IspDMACfg &a_dma,MUINT32 pixel_Byte,MUINT32 swap, MUINT32 isBypassOffset,EIMAGE_STRIDE planeNum, NSCam::EImageFormat eImgFmt);
    MBOOL configSrz(SrzCfg* pSrz_cf, _SRZ_SIZE_INFO_* pSrz_size_info, EDipModule srzEnum);
    MBOOL configSrz5(SrzCfg *pSrz_cfg, NSCam::NSIoPipe::SRZSizeInfo SRZ5Info, unsigned int wkaround_h);
    MINT32 configMFBTPipe(NSCam::NSIoPipe::MFBConfig* pMfbConfig, SrzCfg pSrz_cfg, MFB_DRV_DMA_EN_STRUCT mfb_dma_en);
    void configMFB(NSCam::NSIoPipe::MFBConfig *pMfbConfig, MFB_Config *MfbRegConfig);
    MBOOL doUVSwap(MUINTPTR *uv1, MUINTPTR *uv2);

    IspDrvShell*             m_pIspDrvShell;
    DipCmdQMgr*             m_dipCmdQMgr;

    DipRingBuffer*          m_pDipRingBuf;
    DipWorkingBuffer*       m_pDipWorkingBuf;
    DipWorkingBuffer*       m_pDequeDipWorkingBuf;
    //volatile MINT32          mp2CQDupIdx; //

    MINT8                    m_szUsrName[32];
    //
    //jpeg
    //MINT32  jpg_WorkingMem_memId; //no used
    //MUINT32 jpg_WorkingMem_Size;  //no used
    //MUINT32 *pJpgConfigVa;        //no used

    //MUINT32 cq1_size,cq1_phy;     //no used
    //MUINT8 *cq1_vir;              //no used
    //MINT32 cq1_memId;             //no used
    //MUINT32 cq2_size,cq2_phy;     //no used
    //MUINT8 *cq2_vir;              //no used
    //MINT32 cq2_memId;             //no used
    //MUINT32 cq3_size,cq3_phy;     //no used
    //MUINT8 *cq3_vir;              //no used
    //MINT32 cq3_memId;             //no used

    char *pPortLogBuf;
    char *pMaxLogBuf;
    //
    //MBOOL   m_isImgPlaneByImgi;   //no used
    //
    //MBOOL   m_Nr3dEn;             //no used
    //MBOOL   m_Nr3dDmaSel;         //no used
    //
    MBOOL   m_CrzEn;
    //MBOOL   m_SeeeEn;  //no used
    //for img3o crsp only, preview will use rrzo or imgo as input, when the input is imgo. the original definition of mOffsetInBytes will have wrong. the hw main isp pipeline size is refer imgi input.
    NSCam::NSIoPipe::CrspInfo* m_Img3oCropInfo;
    MUINT32 m_RawHDRType;
    MUINT32 m_TimgoDumpSel;
    MFB_Config m_MfbRegStruct;
    MBOOL   m_isMfbDL;
    //Property
    int m_bSmartTileEn;
    int m_bMix4En;
    int m_dipConfigPipeLog;
    int m_mtkcamLogLevel;
    int m_mmpathEn;
    //DMA Format and ImageBuffer
    NSCam::NSIoPipe::Input *portInfo_imgi;
    NSCam::NSIoPipe::Input *portInfo_imgbi;
    NSCam::NSIoPipe::Input *portInfo_imgci;
    NSCam::NSIoPipe::Input *portInfo_vipi;
    NSCam::NSIoPipe::Input *portInfo_vip2i;
    NSCam::NSIoPipe::Input *portInfo_vip3i;
    NSCam::NSIoPipe::Input *portInfo_ufdi;
    NSCam::NSIoPipe::Input *portInfo_lcei;
    NSCam::NSIoPipe::Input *portInfo_dmgi;
    NSCam::NSIoPipe::Input *portInfo_depi;
    NSCam::NSIoPipe::Input *portInfo_regi;
    NSCam::NSIoPipe::Output *portInfo_img2o;
    NSCam::NSIoPipe::Output *portInfo_img2bo;
    NSCam::NSIoPipe::Output *portInfo_img3o;
    NSCam::NSIoPipe::Output *portInfo_img3bo;
    NSCam::NSIoPipe::Output *portInfo_img3co;
    NSCam::NSIoPipe::Output *portInfo_feo;
    NSCam::NSIoPipe::Output *portInfo_timgo;
    NSCam::NSIoPipe::Output *portInfo_dceso;
    NSCam::NSIoPipe::Output *portInfo_wroto;
    NSCam::NSIoPipe::Output *portInfo_wdmao;
    NSCam::NSIoPipe::Output *portInfo_jpego;
    NSCam::NSIoPipe::Output *portInfo_venco;

    NSCam::EImageFormat eImgFmt_imgi;
    NSCam::EImageFormat eImgFmt_imgbi;
    NSCam::EImageFormat eImgFmt_imgci;
    NSCam::EImageFormat eImgFmt_vipi;
    NSCam::EImageFormat eImgFmt_vip2i;
    NSCam::EImageFormat eImgFmt_vip3i;
    NSCam::EImageFormat eImgFmt_ufdi;
    NSCam::EImageFormat eImgFmt_lcei;
    NSCam::EImageFormat eImgFmt_dmgi;
    NSCam::EImageFormat eImgFmt_depi;
    NSCam::EImageFormat eImgFmt_regi;
    NSCam::EImageFormat eImgFmt_img2o;
    NSCam::EImageFormat eImgFmt_img2bo;
    NSCam::EImageFormat eImgFmt_img3o;
    NSCam::EImageFormat eImgFmt_img3bo;
    NSCam::EImageFormat eImgFmt_img3co;
    NSCam::EImageFormat eImgFmt_feo;
    NSCam::EImageFormat eImgFmt_timgo;
    NSCam::EImageFormat eImgFmt_dceso;
    NSCam::EImageFormat eImgFmt_wroto;
    NSCam::EImageFormat eImgFmt_wdmao;
    NSCam::EImageFormat eImgFmt_jpego;
    NSCam::EImageFormat eImgFmt_venco;
    dip_x_reg_t *pTuningDipReg;
    MINT32 m_imgi_planeNum;
    MDPMGR_CFG_STRUCT *pMdpCfg;
};


////////////////////////////////////////////////////////////////////////////////
};  //namespace NSIspio
};  //namespace NSImageio
#endif  //  _ISPIO_POSTPROC_PIPE_H_

