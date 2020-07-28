/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein is
 * confidential and proprietary to MediaTek Inc. and/or its licensors. Without
 * the prior written permission of MediaTek inc. and/or its licensors, any
 * reproduction, modification, use or disclosure of MediaTek Software, and
 * information contained herein, in whole or in part, shall be strictly
 * prohibited.
 *
 * MediaTek Inc. (C) 2010. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
 * ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 * NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH
 * RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 * INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES
 * TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
 * RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
 * OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN MEDIATEK
 * SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE
 * RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S
 * ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE
 * RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE
 * MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE
 * CHARGE PAID BY RECEIVER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek
 * Software") have been modified by MediaTek Inc. All revisions are subject to
 * any receiver's applicable license agreements with MediaTek Inc.
 */

#ifndef _MTK_PLATFORM_HARDWARE_INCLUDE_MTKCAM_IOPIPE_IPOSTPROCDEF_H_
#define _MTK_PLATFORM_HARDWARE_INCLUDE_MTKCAM_IOPIPE_IPOSTPROCDEF_H_
#include <utils/Vector.h>
//#include <vector>
//
#include <mtkcam/def/common.h>
#include <mtkcam/utils/imgbuf/IImageBuffer.h>
#include <mtkcam/utils/std/common.h>
#include <mtkcam/drv/iopipe/Port.h>
#include <mtkcam/drv/iopipe/PortMap.h>
#include "IPostProcFeFm.h"
#include <mtkcam/drv/def/ISyncDump.h>


/******************************************************************************
 *
 ******************************************************************************/
namespace NSCam {
namespace NSIoPipe {

/******************************************************************************
 * @define MAX_PIPE_USER_NUMBER
 *
 * @brief maximum user number of each pass2 pipe.
 *
 ******************************************************************************/
#define MAX_PIPE_USER_NUMBER     32
#define MAX_PIPE_WPEUSER_NUMBER   8


/******************************************************************************
 * @enum StreamPipeID
 *
 * @brief Enum ID for each stream pipe in pass2 control.
 *
 ******************************************************************************/
enum EStreamPipeID
{
    ESatremPipeID_None        = 0,
    EStreamPipeID_Normal,
    EStreamPipeID_WarpEG,
    EStreamPipeID_Total
};


/******************************************************************************
 * @struct MCropRect
 *
 * @brief Cropped Rectangle.
 *
 * @param[in] p_fractional: fractional part of left-top corner in pixels.
 *
 * @param[in] p_integral: integral part of left-top corner in pixels.
 *
 * @param[in] s: size (i.e. width and height) in pixels.
 *
 ******************************************************************************/
struct MCropRect
{
    typedef int                 value_type;
    MPoint                      p_fractional;       //  left-top corner
    MPoint                      p_integral;         //  left-top corner
    MSize                       s;                  //  size: width, height
    value_type                  w_fractional;       //  float width
    value_type                  h_fractional;       //  float height

public:     ////                Instantiation.

    // we don't provide copy-ctor and copy assignment on purpose
    // because we want the compiler generated versions

    inline                      MCropRect(int _w = 0, int _h = 0)
                                    : p_fractional(0, 0), p_integral(0, 0), s(_w, _h), w_fractional(0), h_fractional(0)
                                {
                                }

    inline                      MCropRect(MPoint const& topLeft, MPoint const& bottomRight)
                                    : p_fractional(0, 0), p_integral(topLeft), s(topLeft, bottomRight), w_fractional(0), h_fractional(0)
                                {
                                }

    inline                      MCropRect(MPoint const& _p, MSize const& _s)
                                    : p_fractional(0, 0), p_integral(_p), s(_s), w_fractional(0), h_fractional(0)
                                {
                                }

    inline                      MCropRect(MRect const& _rect)
                                    : p_fractional(0, 0), p_integral(_rect.leftTop()), s(_rect.size()), w_fractional(0), h_fractional(0)
                                {
                                }

};

/******************************************************************************
 * @struct MCrpRsInfo
 *
 * @brief Cropped Rectangle and Resize Information for whole pipe.
 *
 * @param[in] mCropRect: cropped rectangle.
 *
 * @param[in] mResizeDst: resized size of current dst buffer.
 *
 * @param[in] mMdpGroup: group information for mdp crop. 0 stands for MDP_CROP, 1 stands for MDP_CROP2.
 *
 ******************************************************************************/
struct MCrpRsInfo
{
    MUINT32 mFrameGroup;
    MINT32    mGroupID;
    MUINT32   mMdpGroup;
    MCropRect mCropRect;
    MSize     mResizeDst;
    MCrpRsInfo()
        : mFrameGroup(0)
        , mGroupID(0)
        , mMdpGroup(0)
        , mResizeDst(mCropRect.s){}
};

/*******************************************************************************
* @struct MCropPathInfo
*
* @brief Crop path information.
*
* @param[in] mGroupNum: number of crop group.
*
* @param[in] mGroupID: crop group id.
*
* @param[in] mvPorts: dma port in each crop group.
*
********************************************************************************/
struct MCropPathInfo
{
	MUINT32                     mGroupIdx;
	android::Vector<MUINT32>    mvPorts;
public:
    MCropPathInfo()
    {
	   mGroupIdx = 0;
	}
};

///////////////////////////////////////////////////
//test struct
struct ExtraParams
{
    unsigned int imgFmt;
    int imgw;
    int imgh;
    MUINTPTR memVA;
    MUINTPTR memPA;
    int memID;
    unsigned int memSize;
    int p2pxlID;
    ExtraParams(
        MUINT32     _imgFmt=0x0,
        MINT32      _imgw=0,
        MINT32      _imgh=0,
        MUINT32     _size = 0,
        MINT32      _memID = -1,
        MUINTPTR    _virtAddr = 0,
        MUINTPTR    _phyAddr = 0,
        MINT32      _p2pxlID=0)
        : imgFmt(_imgFmt)
        , imgw(_imgw)
        , imgh(_imgh)
        , memVA(_virtAddr)
        , memPA(_phyAddr)
        , memID(_memID)
        , memSize(_size)
        , p2pxlID(_p2pxlID)
        {
        }
};
///////////////////////////////////////////////////


/******************************************************************************
 *
 * @struct ModuleInfo
 * @brief parameter for specific hw module or dma statistic data which need be by frame set
 * @details
 *
 ******************************************************************************/
struct ModuleInfo
{
    MUINT32   moduleTag;
    MINT32   frameGroup;
    MVOID*   moduleStruct;
public:     //// constructors.

    ModuleInfo()
        : moduleTag(0x0)
        , frameGroup(0)
        , moduleStruct(NULL)
    {
    }
    //

};

/******************************************************************************
 * @struct EDIPSecureEnum
 *
 * @brief EDIPSecureEnum.
 ******************************************************************************/
enum EDIPSecureEnum
{
    EDIPSecure_NONE = 0,
    EDIPSecure_SECURE,
    EDIPSecure_PROTECT,
    EDIPSecure_MAX
};

/******************************************************************************
 * @struct EDIPSecDMAType
 *
 * @brief EDIPSecDMAType.
 ******************************************************************************/
enum EDIPSecDMAType
{
    EDIPSecDMAType_TUNE = 0,	/* additional secure buffer: no, mSecureTag = 0*/
    EDIPSecDMAType_IMAGE,	/* additional secure buffer: no, keep original mSecureTag*/
    EDIPSecDMAType_TUNE_SHARED,	/* additional secure buffer: yes, mSecureTag = 1 or 2*/
    EDIPSecDMAType_MAX
};

/******************************************************************************
 * @struct Input
 *
 * @brief Pipe input parameters.
 *
 * @param[in] mPortID: The input port ID of the pipe.
 *
 * @param[in] mBuffer: A pointer to an image buffer.
 *            Callee must lock, unlock, and signal release-fence.
 *
 * @param[in] mCropRect: Input CROP is applied BEFORE transforming and resizing.
 *
 * @param[in] mTransform: ROTATION CLOCKWISE is applied AFTER FLIP_{H|V}.
 *
 ******************************************************************************/
struct  Input
{
public:     ////                    Fields (Info)
    PortID                          mPortID;
    IImageBuffer*                   mBuffer;

public:     ////                    Fields (Operations)
    MINT32                          mTransform;
    MUINT32                         mOffsetInBytes;
    EDIPSecureEnum                  mSecureTag;
    MUINT32                         mSecHandle;
public:     ////                    Constructors.
                                    Input(
                                        PortID const&   rPortID     = PortID(),
                                        IImageBuffer*   buffer      = 0,
                                        MINT32 const    transform   = 0,
                                        MUINT32 const   offsetInBytes = 0,
                                        EDIPSecureEnum const secureTag = EDIPSecure_NONE,
                                        MUINT32 const   secHandle   = 0
                                    )
                                        : mPortID(rPortID)
                                        , mBuffer(buffer)
                                        , mTransform(transform)
                                        , mOffsetInBytes(offsetInBytes)
                                        , mSecureTag(secureTag)
                                        , mSecHandle(secHandle)
                                    {
                                    }
};


/******************************************************************************
 * @struct EDIPInfoEnum
 *
 * @brief EDIPInfoEnum.
 ******************************************************************************/
enum EDIPInfoEnum
{
    EDIPINFO_DIPVERSION,
    EDIPINFO_BATCH_FRAME_BUFFER_COUNT,
    EDIPINFO_PER_FRAME_CB_SUPPORT,
    EDIPINFO_FM_WIDTH_LIMITATION,
    EDIPINFO_MAX
};

/******************************************************************************
 * @struct EDIPInfoEnum
 *
 * @brief EDIPInfoEnum.
 ******************************************************************************/
enum EDIPHWVersionEnum
{
    EDIPHWVersion_30 = 0x30,
    EDIPHWVersion_40 = 0x40,
    EDIPHWVersion_50 = 0x50,
    EDIPHWVersion_60 = 0x60,
    EDIPHWVersion_MAX
};



/******************************************************************************
 * @struct Output
 *
 * @brief Pipe output parameters.
 *
 * @param[in] mPortID: The output port ID of the pipe.
 *
 * @param[in] mBuffer: A pointer to an image buffer.
 *            Output CROP is applied AFTER the transform.
 *            Callee must lock, unlock, and signal release-fence.
 *
 * @param[in/out] mTransform: ROTATION CLOCKWISE is applied AFTER FLIP_{H|V}.
 *            The result of transform must be set by the pipe if the request of
 *            transform is not supported by the pipe.
 *
 ******************************************************************************/
struct  Output
{
public:     ////                    Fields (Info)
    PortID                          mPortID;
    IImageBuffer*                   mBuffer;

public:     ////                    Fields (Operations)
    MINT32                          mTransform;
    MUINT32                         mOffsetInBytes;
    EDIPSecureEnum                  mSecureTag;
public:     ////                    Constructors.
                                    Output(
                                        PortID const&   rPortID     = PortID(),
                                        IImageBuffer*   buffer      = 0,
                                        MINT32 const    transform   = 0,
                                        MUINT32 const   offsetInBytes = 0,
                                        EDIPSecureEnum const secureTag = EDIPSecure_NONE
                                    )
                                        : mPortID(rPortID)
                                        , mBuffer(buffer)
                                        , mTransform(transform)
                                        , mOffsetInBytes(offsetInBytes)
                                        , mSecureTag(secureTag)
                                    {
                                    }
};

/******************************************************************************
 * @struct PQParam
 *
 * @brief PQParam.
 *
 * @param[in] CmdIdx: specific command index: EPIPE_MDP_PQPARAM_CMD
 *
 * @param[in] moduleStruct: specific structure: PQParam
 *
 * @param[in] s: WDMAPQParam  (i.e. DpPqParam) which is defined by MDP PQ owner
 * @param[in] s: WROTPQParam  (i.e. DpPqParam) which is defined by MDP PQ owner
  *
 ******************************************************************************/
struct PQParam
{
    MVOID* WDMAPQParam;
    MVOID* WROTPQParam;
   PQParam()
        : WDMAPQParam(NULL)
        , WROTPQParam(NULL)
    {
    }
};

/******************************************************************************
 * @struct CrspInfo
 *
 * @brief CrspInfo.
 *
 * @param[in] CmdIdx: specific command index: EPIPE_IMG3O_CRSPINFO_CMD
 *
 * @param[in] moduleStruct: specific structure: CrspInfo
 *
 * @param[in] s: this command will only use m_CrspInfo.p_integral.x, m_CrspInfo.p_integral.y, m_CrspInfo.s.w and m_CrspInfo.s.h
  *
 ******************************************************************************/
struct CrspInfo
{
    MCropRect m_CrspInfo;
};


/******************************************************************************
 * @struct EDIPRawHDRTypeEnum
 *
 * @brief EDIPRawHDRTypeEnum.
 ******************************************************************************/
enum EDIPRawHDRTypeEnum
{
    EDIPRawHDRType_NONE,
    EDIPRawHDRType_SENSORHDR,
    EDIPRawHDRType_ISPHDR,
    EDIPRawHDRType_MAX
};
/******************************************************************************
 * @struct EDIPTimgoDumpEnum
 *
 * @brief EDIPTimgoDumpEnum.
 ******************************************************************************/
enum EDIPTimgoDumpEnum
{
    EDIPTimgoDump_NONE,
    EDIPTimgoDump_AFTER_DGN,
    EDIPTimgoDump_AFTER_LSC,
    EDIPTimgoDump_AFTER_HLR,
    EDIPTimgoDump_AFTER_LTM,
    EDIPTimgoDump_AFTER_GGM,
    EDIPTimgoDump_AFTER_MAX
};
/******************************************************************************
 * @struct ExtraPara
 *
 * @brief ExtraPara.
 *
 * @param[in] CmdIdx: specific command index to responding modulestruct
 *
 * @param[in] moduleStruct: specific structure accorind to command index
 *
 * @param[in] s: size (i.e. width and height) in pixels.
 *
 ******************************************************************************/
enum EPostProcCmdIndex
{
    EPIPE_FE_INFO_CMD,         // type: structure => NSCam::NSIoPipe::FEInfo
    EPIPE_FM_INFO_CMD,         // type: structure => NSCam::NSIoPipe::FMInfo
    EPIPE_WPE_INFO_CMD,        // type: structure => Wrapping Engine
    EPIPE_MDP_PQPARAM_CMD,     // type: structure => PQParam: MDP PQ Param, if MW have any requirement of MDP PQ, please use this command to pass the PQ param.
    EPIPE_IMG3O_CRSPINFO_CMD,  // type: structure => PQParam: CrspInfo IMG3O CRSP used, some times the preview frame will use imgo output as imgi input., CrspInfo
    EPIPE_RAW_HDRTYPE_CMD,     // type: MUINT32 => EDIPRawHDRTypeEnum
    EPIPE_TIMGO_DUMP_SEL_CMD,  // type: MUINT32 => EDIPTimgoDumpEnum
    EPIPE_MFB_INFO_CMD,        // type: starcture => NSCam::NSIoPipe::MFBConfig
	EPIPE_MSS_INFO_CMD, 	   // type: starcture => NSCam::NSIoPipe::MSSConfig
	EPIPE_MSF_INFO_CMD,		   // type: starcture => NSCam::NSIoPipe::MSFConfig
    EPIPE_TOTAL_CMD,
};

struct ExtraParam
{
    EPostProcCmdIndex   CmdIdx;
    MVOID*   moduleStruct;
public:     //// constructors.

    ExtraParam()
        : CmdIdx(EPIPE_TOTAL_CMD)
        , moduleStruct(NULL)
    {
    }
    //
};

enum EFrameCallBackMsg
{
    EFRAMECALLBACK_NOERR,
    EFRAMECALLBACK_CONFIGERR,
    EFRAMECALLBACK_STARTMDPFAIL,
    EFRAMECALLBACK_DEQUEFAIL,
    EFRAMECALLBACK_NONE,
};

/******************************************************************************
 *
 * @struct FrameParams
 *
 * @brief Queuing parameters for the pipe.
 *      input cropping -> resizing ->
 *      output flip_{H|V} -> output rotation -> output cropping
 *
 * @param[in] mpCookie: frame callback cookie; it shouldn't be modified by the pipe.
 *
 * @param[in] mvIn: a vector of input parameters.
 *
 * @param[in] mvOut: a vector of output parameters.
 *
 * @param[in] mCropRsInfo: a array of pipe crop/resize information.
 *
 * @param[in] mvExtraParam: extra command information in this frame request.
 *
 ******************************************************************************/

struct FrameParams
{
    MUINT32 FrameNo;
    MUINT32 RequestNo;
    MUINT32 Timestamp;
    MINT32 UniqueKey;
    MINT32 IspProfile;
    MINT32 SensorDev;
    //---upper is for bit-true dump---//
    MINT32 FrameIdentify; //by User Defined
    P2_RUN_INDEX mRunIdx;
    MBOOL  NeedDump;
    MINT32 mStreamTag;
    MINT32 mSensorIdx;
    MBOOL mSecureFra;
    MVOID* mTuningData;
    MVOID* mpCookie;
    struct timeval ExpectedEndTime;
    android::Vector<Input>       mvIn;
    android::Vector<Output>      mvOut;
    android::Vector<MCrpRsInfo>  mvCropRsInfo;
    android::Vector<ModuleInfo>     mvModuleData;
    android::Vector<ExtraParam>     mvExtraParam;
    typedef bool (*PFN_FRAMECB_T)(const FrameParams& rFrmParams, EFrameCallBackMsg FrmCBMsg);  // per frame callback
    PFN_FRAMECB_T mpfnCallback; //deque call back
    FrameParams()
            : FrameNo(0)
            , RequestNo(0)
            , Timestamp(0)
            , UniqueKey(0)
            , IspProfile(-1)
            , SensorDev(0)
            , FrameIdentify(0)
            , mRunIdx(P2_RUN_UNKNOWN)
            , NeedDump(MFALSE)
            , mStreamTag(-1)
            , mSensorIdx(-1)
            , mSecureFra(0)
            , mTuningData(NULL)
            , mpCookie(NULL)
            , mvIn()
            , mvOut()
            , mvCropRsInfo()
            , mvModuleData()
            , mvExtraParam()
            , mpfnCallback(NULL)
        {
            ExpectedEndTime.tv_sec = 0;
            ExpectedEndTime.tv_usec = 0;
        }

};


enum EDipUserEnum
{
    EDipUserEnum_DEFAUT = 0, //will map to command 0
	EDipUserEnum_DSDN,  //will map to command 2
	EDipUserEnum_MFNR,  //will map to command 3
	EDipUserEnum_MSNR,  //will map to command 4
};


/*******************************************************************************
* @struct NormalStreamDipUserParam
********************************************************************************/
struct DipUserParam{
	NSCam::NSIoPipe::EDipUserEnum DipUser;
	MUINT32 UseHowManyBufNum;
	MUINT32 HowManyFrameRepeated;
};

/******************************************************************************
 *
 * @struct QParams
 *
 * @brief Queuing parameters for the pipe.
 *      input cropping -> resizing ->
 *      output flip_{H|V} -> output rotation -> output cropping
 *
 * @param[in] mpfnCallback: a pointer to a callback function.
 *      If it is NULL, the pipe must put the result into its result queue, and
 *      then a user will get the result by deque() from the pipe later.
 *      If it is not NULL, the pipe does not put the result to its result queue.
 *      The pipe must invoke a callback with the result.
 *
 * @param[in] mpCookie: callback cookie; it shouldn't be modified by the pipe.
 *
 * @param[in] mvIn: a vector of input parameters.
 *
 * @param[in] mvOut: a vector of output parameters.
 *
 * @param[in] mCropRsInfo: a array of pipe crop/resize information.
 *
 * @param[in] mFrameNo: frame number, starting from 0.
 *
 ******************************************************************************/
struct  QParams
{
    typedef MVOID                   (*PFN_CALLBACK_T)(QParams& rParams);
    PFN_CALLBACK_T                  mpfnCallback;   //deque call back
    PFN_CALLBACK_T                  mpfnEnQFailCallback;    //call back for enque fail
    PFN_CALLBACK_T                  mpfnEnQBlockCallback;   //call back for enque blocking
    MVOID*                          mpCookie;
    MBOOL                           mDequeSuccess;
    android::Vector<FrameParams>    mvFrameParams;
                                    //
                                    QParams()
                                        : mpfnCallback(NULL)
                                        , mpfnEnQFailCallback(NULL)
                                        , mpfnEnQBlockCallback(NULL)
                                        , mpCookie(NULL)
                                        , mDequeSuccess(MFALSE)
                                        , mvFrameParams()
                                    {
                                    }
};


/******************************************************************************
 *
 ******************************************************************************/
};  //namespace NSIoPipe
};  //namespace NSCam
#endif  //_MTK_PLATFORM_HARDWARE_INCLUDE_MTKCAM_IOPIPE_IPOSTPROCDEF_H_

