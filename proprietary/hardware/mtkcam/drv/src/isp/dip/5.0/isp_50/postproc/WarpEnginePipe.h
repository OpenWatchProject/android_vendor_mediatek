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
#ifndef _ISPIO_WARPENGINE_PIPE_H_
#define _ISPIO_WARPENGINE_PIPE_H_
//
#include <vector>
#include <map>
#include <list>
//
using namespace std;
//

#include <imem_drv.h>

#include "isp_datatypes.h"
#include "mdp_mgr.h"


#include "IWarpEnginePipe.h"
#include <mtkcam/drv/iopipe/PostProc/IHalWpePipe.h>

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

struct WPECropInfo
{
        NSImageio::NSIspio::STImgCrop   crop1;           // image crop info. (ring buffer use curz to run crop)
        NSImageio::NSIspio::STImgResize resize1;
        NSImageio::NSIspio::STImgCrop   crop2;           // image crop info. (ring buffer use curz to run crop)
        NSImageio::NSIspio::STImgResize resize2;
        NSImageio::NSIspio::STImgCrop   crop3;           // image crop info. (ring buffer use curz to run crop)
        NSImageio::NSIspio::STImgResize resize3;
};

/*******************************************************************************
*
********************************************************************************/
namespace NSImageio {
namespace NSIspio   {
////////////////////////////////////////////////////////////////////////////////

/*******************************************************************************
*
********************************************************************************/
class WarpEnginePipe : public IWarpEnginePipe
{
public:     ////    Constructor/Destructor.
                    WarpEnginePipe();

                    virtual ~ WarpEnginePipe();

public:     ////    Instantiation.
    virtual MBOOL   init(ISP_WPE_MODULE_IDX module_idx, MUINT32 secTag);
    virtual MBOOL   uninit(ISP_WPE_MODULE_IDX module_idx);
    virtual MINT32 configwpe(WpePackage wpepackage, MBOOL configUVplane);
    virtual MINT32 configWpeCommandData(NSCam::NSIoPipe::WPEConfig wpeconfig, MBOOL configUVplane);
    virtual MINT32 configTpipeData(NSCam::NSIoPipe::WPEConfig wpeconfig, MBOOL configUVplane);
    virtual MBOOL  wpeconfigMdpOutPort(NSImageio::NSIspio::PortInfo const* oImgInfo, MdpRotDMACfg &a_rotDma, NSCam::NSIoPipe::EDIPSecureEnum mSecureTag);
    virtual MINT32 HandleExtraCommand(QParams rParams, NSCam::NSIoPipe::NSWpe::WPEQParams *wpeqpararms);
    virtual MINT32 start(MINT32  wpeDupIdx, MINT32  wpeBurstIdx, MUINT32 wpeDupCmdIdx);
    virtual MINT32 dequeue(MINT32  wpeDupIdx, MINT32  wpeBurstIdx, MUINT32 wpeDupCmdIdx);
    //virtual MBOOL  bufferQueCtrl(EPIPE_P2BUFQUECmd cmd, EPIPE_P2engine p2Eng, MUINT32 callerID, MINT32 p2CQ, MINT32 p2dupCQIdx, MINT32 p2burstQIdx, MINT32 frameNum, MINT32 timeout);

private:


        MINT32                   mWeapEngInitCount;   //Record the user count
private:
        //Wpe Driver
        static WpeDrv*             m_pWpeDrv;
        Mutex                      mMutex_EQUser;  //protect the enque sequence!!
        Mutex                      mModuleMtx;

        MINT32                     mTotalCmdNum;
        Mutex                      mWpeCmdLock;
        MINT32                     m_wpeEQDupIdx;
        MINT32                     m_wpeEQBurstIdx;
        ISP_WPE_MODULE_IDX         module_idx;
        MUINT32                    m_wpeCallerID;
        MUINT32                    m_wpeDupCmdIdx;

        DipIMemDrv*                m_pIMemDrv ; /* IMEM */
        MdpMgr*                    m_pMdpMgr; /* MdpMgr */
        volatile MBOOL             m_WarpEngInit;
        MINT32                     m_iWpeEnqueReq;
        MINT32                     m_bSaveReqToFile;
        MINT32                     mDumpCount;

        IMEM_BUF_INFO              gWPEMdpBufInfo;
        IMEM_BUF_INFO              gWPETpipeBufInfo;
        IMEM_BUF_INFO              wCmdBufInfo;
        char                       m_UserName[MAX_USER_NAME_SIZE];
        WPE_Config                 ***wCmdData;
        MDPMGR_CFG_STRUCT          ***gpWPEMdpMgrCfgData;
        WPE_Config                 gCmdData[2][2][4];
        MDPMGR_CFG_STRUCT          gMDPCfgData[2][2][4];
    //
};


////////////////////////////////////////////////////////////////////////////////
};  //namespace NSIspio
};  //namespace NSImageio
#endif  //  _ISPIO_POSTPROC_PIPE_H_

