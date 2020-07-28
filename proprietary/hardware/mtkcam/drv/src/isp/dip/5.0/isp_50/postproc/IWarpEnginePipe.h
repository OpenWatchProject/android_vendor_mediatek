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
#ifndef _ISPIO_I_WARPENGINE_PIPE_H_
#define _ISPIO_I_WARPENGINE_PIPE_H_
#include <wpe_drv.h>
#include <ispio_pipe_ports.h>
#include <ispio_pipe_buffer.h>

/**
    WPE hw module
*/
enum ISP_WPE_MODULE_IDX{
        WPE_MODULE_IDX_WPE_A = 0,
        WPE_MODULE_IDX_WPE_B,
        WPE_MODULE_IDX_MAX
};


struct WpePackage
{
        QParams rParams;             //image buffer information
        MINT32  wpeBurstIdx;
        MINT32  wpeDupIdx;          //duplicate index(pinpon here)
        ISP_WPE_MODULE_IDX wpeModuleIdx;
        MUINT32 callerID;
        MUINT32 m_wpeDupCmdIdx;
        WpePackage()
                : wpeBurstIdx(0)
                , wpeDupIdx(0)
                , wpeModuleIdx(WPE_MODULE_IDX_WPE_A)
                , callerID(0x0)
                , m_wpeDupCmdIdx(0)
        {}
};

#define MAX_WPE_BURST_IDX_NUM   4
#define MAX_WPE_DUP_IDX_NUM      2

/*******************************************************************************
*
********************************************************************************/
namespace NSImageio {
namespace NSIspio   {

////////////////////////////////////////////////////////////////////////////////


/*******************************************************************************
* Pipe Interface
********************************************************************************/

class IWarpEnginePipe
{
public:     ////    Instantiation.
    static IWarpEnginePipe* createInstance();
    virtual MVOID   destroyInstance();

                    IWarpEnginePipe(void){};
    virtual         ~IWarpEnginePipe(void){};

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Command Class.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
    virtual MBOOL   init(ISP_WPE_MODULE_IDX module_idx, MUINT32 secTag =0) = 0;
    virtual MBOOL   uninit(ISP_WPE_MODULE_IDX module_idx) = 0;

    virtual MINT32 configwpe(WpePackage wpepackage, MBOOL configUVplane) = 0;
    virtual MINT32 configWpeCommandData(NSCam::NSIoPipe::WPEConfig wpeconfig, MBOOL configUVplane)= 0;
    virtual MINT32 configTpipeData(NSCam::NSIoPipe::WPEConfig wpeconfig, MBOOL configUVplane)= 0;
    virtual MINT32 start(MINT32  wpeDupIdx, MINT32  wpeBurstIdx, MUINT32 wpeDupCmdIdx)= 0;
    virtual MINT32 dequeue(MINT32  wpeDupIdx, MINT32  wpeBurstIdx, MUINT32 wpeDupCmdIdx)= 0;
    //virtual MBOOL  bufferQueCtrl(EPIPE_P2BUFQUECmd cmd, EPIPE_P2engine p2Eng, MUINT32 callerID, MINT32 p2CQ, MINT32 p2dupCQIdx, MINT32 p2burstQIdx, MINT32 frameNum, MINT32 timeout)= 0;
};


////////////////////////////////////////////////////////////////////////////////
};  //namespace NSIspio
};  //namespace NSImageio
#endif  //  _ISPIO_I_POSTPROC_PIPE_H_

