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
#ifndef _ISPIO_PIPE_PORTS_H_
#define _ISPIO_PIPE_PORTS_H_
#include <utils/Vector.h>
//
#include <vector>
//
#include "ispio_stddef.h"
#include "ispio_pipe_scenario.h"
#include <mtkcam/drv/def/Dip_Notify_datatype.h>
#include <mtkcam/drv/def/ispio_port_index.h>
#include <mtkcam/drv/def/IPostProcDef.h>


/*******************************************************************************
*
********************************************************************************/
namespace NSImageio {
namespace NSIspio   {
////////////////////////////////////////////////////////////////////////////////


/*******************************************************************************
* Pipe Port Type.
********************************************************************************/
enum EPortType
{
    EPortType_Memory,
    EPortType_Sensor,
    EPortType_GDMA,
    EPortType_VRZ_RDMA,
    EPortType_VID_RDMA,
    EPortType_DISP_RDMA,
};

/*******************************************************************************
* Pipe Port Capbility.
********************************************************************************/
enum EPortCapbility
{
    EPortCapbility_Non  =0x00,
    EPortCapbility_Cap  =0x01,
    EPortCapbility_Rcrd =0x02,
    EPortCapbility_Disp =0x03,
};

/*******************************************************************************
* Pipe Sensor index.
********************************************************************************/
enum ESensorIndex
{
    ESensorIndex_NONE   = 0x00,
    ESensorIndex_MAIN   = 0x01,
    ESensorIndex_SUB    = 0x02,
    ESensorIndex_ATV    = 0x04,
    ESensorIndex_MAIN_2 = 0x08,
    ESensorIndex_MAIN_3D= 0x09,
};

enum EPipePass
{
    EPipePass_Dip_Phy = 0,
    EPipePass_Dip_Th0,
    EPipePass_Dip_Th1,
    EPipePass_Dip_Th2,
    EPipePass_Dip_Th3,
    EPipePass_Dip_Th4,
    EPipePass_Dip_Th5,
    EPipePass_Dip_Th6,
    EPipePass_Dip_Th7,
    EPipePass_Dip_Th8,
    EPipePass_Dip_Th9,
    EPipePass_Dip_Th10,
    EPipePass_Dip_Th11,
    EPipePass_Dip_Th12,
    EPipePass_Dip_Th13,
    EPipePass_Dip_Th14,
    EPipePass_PASS1_TG1,
    EPipePass_PASS1_TG1_D,
    EPipePass_CAMSV1_TG,
    EPipePass_CAMSV2_TG
};
/*******************************************************************************
* Pipe Port direction.
********************************************************************************/
enum EPortDirection
{
    EPortDirection_In,
    EPortDirection_Out,
};




/*******************************************************************************
* Pipe Crop Group index.
********************************************************************************/
enum ECropGroupShiftIdx
{
    ECropGroupShiftIdx_1 = 1,
    ECropGroupShiftIdx_2,
    ECropGroupShiftIdx_3,
    ECropGroupShiftIdx_4,
    ECropGroupShiftIdx_5,
    ECropGroupShiftIdx_Num
};

enum ECropGroupIndex
{
    ECropGroupIndex_NONE = 0x01,
    ECropGroupIndex_1    = (1<<ECropGroupShiftIdx_1),
    ECropGroupIndex_2    = (1<<ECropGroupShiftIdx_2),
    ECropGroupIndex_3    = (1<<ECropGroupShiftIdx_3),
    ECropGroupIndex_4    = (1<<ECropGroupShiftIdx_4),
    ECropGroupIndex_5    = (1<<ECropGroupShiftIdx_5),
};

/*******************************************************************************
* Pipe Port Capability (Descriptor).
********************************************************************************/
struct CropPathInfo
{
public:
	MUINT32 u4NumGroup;
	MUINT32 u4CropGroup;
	std::vector<MINT32> PortIdxVec;

    CropPathInfo()
    {
       u4NumGroup = 0;
	   u4CropGroup = 0;
	   PortIdxVec.resize(0);
	}
};


/*******************************************************************************
* Pipe Port Capability (Descriptor).
********************************************************************************/
struct PortCap
{
public:
    MUINT32     u4Rotation;     //  support rotation
    MUINT32     u4Flip;         //  Flip
    MUINT32     u4MaxWidth;     //  Support Maximum Image Width
    MUINT32     u4MaxHeight;    //  Support Maximum Image Height
    MUINT32     u4MinWidth;     //  Support Minmum Image Width
    MUINT32     u4MinHeight;    //  Support Minmum Image Height
    MUINT32     u4CropGroup;     //The DMA port is belong to which crop function
    PortCap()
    {
    	u4Rotation = 0;
		u4Flip = 0;
		u4MaxWidth = 0;
		u4MaxHeight = 0;
		u4MinWidth = 0;
		u4MinHeight = 0;
		u4CropGroup = 0;
    }
    PortCap(MUINT32 rot, MUINT32 flip,
		      MUINT32 maxwidth, MUINT32 maxheight,
		      MUINT32 minwidth, MUINT32 minheight,
		      MUINT32 cropgroup)
    {
    	u4Rotation = rot;
		u4Flip = flip;
		u4MaxWidth = maxwidth;
		u4MaxHeight = maxheight;
		u4MinWidth = minwidth;
		u4MinHeight = minheight;
		u4CropGroup = cropgroup;
    }

};

/*******************************************************************************
* Pipe Port ID (Descriptor).
********************************************************************************/
struct PortID
{
public:     //// fields.
    MUINT32     type     :   8;      //  EPortType
    MUINT32     index    :   8;      //  port index
    MUINT32     inout    :   1;      //  0:in/1:out
    MUINT32     capbility:   2;      // dma port capbility
    MUINT32     bOver4lane:   1;      // 0:flase/1:true
    MUINT32     pipePass;
    ESoftwareScenario swscenario;
    MUINT32     tgFps;              // fps x 10
    MUINT32     tTimeClk;           // 10 -> 1mhz.   20->2mhz
    MUINT32     MIPI_PixRate;
    //
public:     //// constructors.
    PortID(
        EPortType const _eType     = EPortType_Memory,
        MUINT32 const _index       = 0,
        MUINT32 const _inout       = 0,
        EPortCapbility const _capbility = EPortCapbility_Non,
        MUINT32 const _pipePass= EPipePass_Dip_Th0,
        ESoftwareScenario const _swscenario=eSoftwareScenario_Main_Normal_Stream
    )
    {
        type         = _eType;
        index        = _index;
        inout        = _inout;
        capbility    = _capbility;
        pipePass     = _pipePass;
        swscenario   = _swscenario;
        tgFps        = 10;
        bOver4lane   = MFALSE;
        tTimeClk     = 10;
        MIPI_PixRate = 0;
    }
    //
public:     //// operations.
    MUINT32 operator()() const
    {
        return  *reinterpret_cast<MUINT32 const*>(this);
    }
};


/*******************************************************************************
* Pipe Port Info.
********************************************************************************/
struct PortInfo : public ImgInfo, public PortID, public BufInfo
{
public:     //// constructors.
    PortInfo()
        : ImgInfo()
        , PortID()
    {
    }
    //
    PortInfo(ImgInfo const& _ImgInfo)
        : ImgInfo(_ImgInfo)
        , PortID()
    {
    }
    //
    PortInfo(PortID const& _PortID)
        : ImgInfo()
        , PortID(_PortID)
    {
    }
    //
    PortInfo(ImgInfo const& _ImgInfo, PortID const& _PortID)
        : ImgInfo(_ImgInfo)
        , PortID(_PortID)
    {
    }
    //
    PortInfo(ImgInfo const& _ImgInfo, PortID const& _PortID, BufInfo const& _BufInfo)
        : ImgInfo(_ImgInfo)
        , PortID(_PortID)
        ,BufInfo(_BufInfo)
    {
    }
    //

};


/*******************************************************************************
* IspP1PrivateData
********************************************************************************/
struct IspP1PrivateData{
    MUINT32 ResizeSrcW;
    MUINT32 ResizeSrcH;
    MUINT32 ResizeDstW;
    MUINT32 ResizeDstH;
    MUINT32 ResizeCropX;
    MUINT32 ResizeCropY;
};

/*******************************************************************************
* Pipe Package Info
********************************************************************************/
struct PipePackageInfo
{
    EDrvScenario drvScen;
    MINT32 burstQIdx;
    MINT32 dupCqIdx;
    MINT32 p2cqIdx;
    std::vector<DipModuleCfg> vModuleParams;
    MVOID* pTuningQue;
    MUINT32  pixIdP2;
    MBOOL lastframe; //last frame in enque package or not
    const android::Vector<NSCam::NSIoPipe::ExtraParam> *pExtraParam;
public:     //// constructors.
    PipePackageInfo()
        : drvScen(eDrvScenario_P2A)
        , burstQIdx(0)
        , dupCqIdx(0)
        , p2cqIdx(0)
        , vModuleParams()
        , pTuningQue(NULL)
        , pixIdP2(0)
        , lastframe(MFALSE)
        , pExtraParam(NULL)
    {
        vModuleParams.resize(0);
        //
    }
};


////////////////////////////////////////////////////////////////////////////////
};  //namespace NSIspio
};  //namespace NSImageio
#endif  //  _ISPIO_PIPE_PORTS_H_

