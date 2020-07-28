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
#ifndef _ISPIO_I_POSTPROC_PIPE_H_
#define _ISPIO_I_POSTPROC_PIPE_H_


/*******************************************************************************
*
********************************************************************************/
namespace NSImageio {
namespace NSIspio   {
////////////////////////////////////////////////////////////////////////////////


/*******************************************************************************
* Pipe Interface
********************************************************************************/

class IPostProcPipe
{
public:     ////    Instantiation.
    static IPostProcPipe* createInstance();
    virtual MVOID   destroyInstance();

                    IPostProcPipe(void){};
    virtual         ~IPostProcPipe(void){};

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Command Class.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
    virtual MBOOL   init(char const* szCallerName, MUINT32 secTag=0) = 0;
    virtual MBOOL   uninit(char const* szCallerName) = 0;

    virtual MBOOL   start(void* pParam=NULL) = 0;
    virtual MBOOL   stop(void* pParam=NULL) = 0;

    virtual MBOOL   configPipe(const NSCam::NSIoPipe::FrameParams &pFrameParams, PipePackageInfo *pPipePackageInfo, char const* szCallerName) = 0;

    virtual MBOOL   dequeMdpFrameEnd(MINT32 const eDrvSce, MINT32 dequeCq=0,MINT32 const dequeBurstQIdx=0, MINT32 const dequeDupCqIdx=0, MUINT32 const moduleIdx=0)=0;
    virtual MBOOL   bufferQueCtrl(EPIPE_P2BUFQUECmd cmd, EPIPE_P2engine P2Eng, MUINT32 callerID, MINT32 p2CQ, MINT32 p2dupCQIdx, MINT32 p2burstQIdx, MINT32 frameNum, MINT32 timeout=0)=0;
    virtual MBOOL   queryCropPathNum(EDrvScenario drvScen, MUINT32& pathNum)=0;
    virtual MBOOL   queryScenarioInfo(EDrvScenario drvScen, vector<CropPathInfo>& vCropPaths)=0;
    virtual MBOOL   sendCommand(MINT32 cmd, MINTPTR arg1, MINTPTR arg2, MINTPTR arg3) = 0;

    virtual MUINT32 dequeBuf(MBOOL& isVencContained, MUINTPTR& tpipeVa, MUINTPTR& ispVirRegVa, EDrvScenario eDrvSce=eDrvScenario_P2A, MINT32 dequeCq=0,MINT32 const dequeBurstQIdx=0, MINT32 const dequeDupCqIdx=0, MUINT32 const RingBufIdx=0, MUINT32 const u4TimeoutMs = 0xFFFFFFFF) = 0;

    virtual MBOOL   getVssDipWBInfo(MUINTPTR& vsstdriaddr, MINT32 p2cqIdx, MINT32 p2RingBufIdx) = 0;
    virtual MBOOL   getDipDumpInfo(MUINT32& tdriaddr, MUINT32& cmdqaddr, MUINT32& imgiaddr, MBOOL& bDequeBufIsTheSameAaGCEDum) = 0;
    //
    virtual MUINT32 queryCQ(EDrvScenario sDrvScen, MUINT32 sensorDev, MINT32 p2burstQIdx, MINT32 frameNum, MUINT32& RingBufIndex, MUINTPTR& pVirIspAddr)=0;
    virtual MBOOL updateCQUser(MINT32 p2CQ)=0;
    virtual MBOOL   getCrzPosition(EDrvScenario drvScen, MBOOL &isSl2eAheadCrz, MBOOL &isLceAheadCrz)=0;

};


////////////////////////////////////////////////////////////////////////////////
};  //namespace NSIspio
};  //namespace NSImageio
#endif  //  _ISPIO_I_POSTPROC_PIPE_H_

