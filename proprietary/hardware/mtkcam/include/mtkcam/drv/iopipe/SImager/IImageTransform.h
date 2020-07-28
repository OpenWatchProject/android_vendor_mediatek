/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2010. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
 * AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek Software")
 * have been modified by MediaTek Inc. All revisions are subject to any receiver's
 * applicable license agreements with MediaTek Inc.
 */

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
#ifndef _MTK_PLATFORM_HARDWARE_MTKCAM_CORE_IOPIPE_SIMAGER_IIMAGE_TRANSFORM_H_
#define _MTK_PLATFORM_HARDWARE_MTKCAM_CORE_IOPIPE_SIMAGER_IIMAGE_TRANSFORM_H_
//
//
#include <mtkcam/utils/imgbuf/IImageBuffer.h>
#include <map>
/*******************************************************************************
*
********************************************************************************/
namespace NSCam {
namespace NSIoPipe {
namespace NSSImager {
////////////////////////////////////////////////////////////////////////////////



/*******************************************************************************
* Interface of Image Transform
********************************************************************************/
class IImageTransform
{
protected:  ////    Constructor/Destructor.
    virtual         ~IImageTransform() {}

public:     ////    Attributes
    virtual MINT32      getLastErrorCode() const = 0;

public:     ////    Instantiation.
    static IImageTransform* createInstance(const char* sUserName= "", MINT32 sensorIndex = -1);
    virtual MVOID   destroyInstance() = 0;

public:     ////    Operations.
    /*
     *  Image transform , the functionality is such as bitblt function
     *
     *  Params:
     *      pSrcBuf
     *      [I] The image buffer info of the input image
     *
     *      pDstBuf_0
     *      [I] The image buffer of the output image 0
     *
     *      pDstBuf_1
     *      [I] The image buffer of the output image 1
     *
     *      rROI
     *      [I] The crop of region of the input image
     *
     *      u4Transform_0
     *      [I] The image transform operation for output image 0
     *
     *      u4TimeOutInMs
     *      [I] The time out of the operation
     *
     *  Return:
     *      MTRUE indicates success; MFALSE indicates failure, and an error code
     *      can be retrived by getLastErrorCode().
     */
    virtual  MBOOL    execute(
                          IImageBuffer const *pSrcBuf,
                          IImageBuffer const * pDstBuf_0,
                          IImageBuffer const * pDstBuf_1,
                          MRect const rROI,
                          MUINT32 const u4Transform_0,
                          MUINT32 const u4TimeOutInMs,
                          struct timeval *endTime = NULL,
                          MBOOL bNeedDump = MFALSE
                          ) = 0;

    virtual  MBOOL    execute(
                          IImageBuffer const *pSrcBuf,
                          IImageBuffer const * pDstBuf_0,
                          IImageBuffer const * pDstBuf_1,
                          MRect const rROI_0,
                          MRect const rROI_1,
                          MUINT32 const u4Transform_0,
                          MUINT32 const u4Transform_1,
                          MUINT32 const u4TimeOutInMs,
                          struct timeval *endTime = NULL,
                          MBOOL bNeedDump = MFALSE
                          ) = 0;

    virtual MBOOL     setSensorIndex(MINT32 iSensorIdx) = 0;

/*******************************************************************************
* PQ Support
*
* If you don't need to apply PQ, you don't need to invoke setPQParameter. If
* you invoked setPQParameter, the PQ parameter will be always applied while
* invoking execute everytimes.
*
* Note: You DON'T need to create PQ setting before invoking execute if you
*       created a new IImageTransform instance.
*******************************************************************************/
public:
    enum PQType
    {
        Unknown = 0,
        ClearZoom = 1 << 1,
        DRE       = 1 << 2,
        HFG       = 1 << 3,
        // future extensible
    };

    enum class Mode
    {
        Unknown = 0,
        Preview,            // preview usage
        Capture_Single,     // single capture usage
        Capture_Multi,      // contiguous capture usage

        Capture = Capture_Single,
    };

    struct CZConfig
    {
        void        *p_customSetting;

        // Constructors
        CZConfig() : p_customSetting(NULL) {};
    };

    struct DREParam
    {
        enum CMD
        {
            DRE_Default  = 1   << 0,  // preview usage, use N-1 or N-2 histgram
            DRE_Generate = 1   << 1,  // capture usage, generate "userId" histgram
            DRE_Apply    = 1   << 2,  // capture usage, apply    "userId" histgram
            DRE_OFF      = 1   << 3   // no us
        };
        enum class HisType
        {
            His_Unkown   = 0,
            His_One_Time = 1 << 0,
            His_Conti    = 1 << 1
        };
        MUINT32     cmd;    //CMD
        HisType     type;
        MUINT64     userId;
        void        *pBuffer;
        void        *p_customSetting;
        MUINT32     customIdx;
        //
        DREParam() : cmd(CMD::DRE_OFF), type(HisType::His_Unkown), userId(0LL), pBuffer(NULL),
                     p_customSetting(NULL), customIdx(0) {};
    };

    struct HFGConfig
    {
        void        *p_lowerSetting;
        void        *p_upperSetting;

        // Constructors
        HFGConfig() : p_lowerSetting(NULL), p_upperSetting(NULL) {};
    };

    struct PQParam
    {
        MUINT32     type;       // PQ type
        MBOOL       enable;     // Enable or disable PQ
        MUINT32     portIdx;    // PQ can only be applied w/ a port. This attribute
        // describes which index of destination port to
        // be specified. E.g.: portIdx = 0 --> dst_buf_0
        Mode        mode;       // Mode
        MUINT32     sensorId;   // The current ID of opened sensor
        MUINT32     iso;        // The real ISO
        MUINT32     timestamp;  // Timestamp, for debug
        MUINT32     frameNo;    // Frame number, for debug
        MUINT32     requestNo;  // Request number, for debug
        void        *p_mdpSetting; //for debug exif
        int         lv_value;   // LV
        void         *p_faceInfor;
        char        userString[8] = "";

        /* Features */
        CZConfig    cz;         // ClearZoom
        DREParam    dre;        // DRE (CALTM)
        HFGConfig   hfg;        // HFG

        // future extensible

        PQParam() : type(PQType::Unknown), enable(MFALSE), portIdx(9999), mode(Mode::Unknown),
        sensorId(0), iso(0), timestamp(0), frameNo(0), requestNo(0), p_mdpSetting(nullptr),
        lv_value(0), p_faceInfor(nullptr) {};
    };

    struct DumpInfos
    {
        DumpInfos(){}
        DumpInfos(
            MINT32 iRequestNo,
            MINT32 iFrameNo,
            MINT32 iTimestamp
        )
        : FrameNo(iFrameNo)
        , RequestNo(iRequestNo)
        , Timestamp(iTimestamp)
        {}

        MINT32 FrameNo = -1;
        MINT32 RequestNo = -1;
        MINT32 Timestamp = -1;
    };

public:
    /*
     *  Before invoke IImageTransform::execute, caller can decide to set PQ
     *  parameter.
     *
     *  Params:
     *      p
     *      [I]   PQ parameter. See struct PQParam
     *
     *  Return:
     *      Not all platforms support the certain PQ parameter, if this PQ is
     *      not supported, this method returns MFALSE.
     */
    virtual MBOOL     setPQParameter(const PQParam& p) = 0;


    /*
     *  To get the PQ parameters which has been set as enabled or not.
     *
     *  Params:
     *      N/A
     *
     *  Return:
     *      The parameter, if there's no PQParam has been set before, or not
     *      supports, this method returns a PQParam with PQParam::type as
     *      PQType::Unknown and PQParam::param will be all zero. (default PQParam)
     */
    virtual const std::map<int,IImageTransform::PQParam>& getPQParameter() const = 0;
     /*
     *  config the required information for dumping buffer
     *
     *  Params:
     *      N/A
     *
     */
    virtual MVOID setDumpInfo(const DumpInfos& info) = 0;
};


////////////////////////////////////////////////////////////////////////////////
};  //namespace NSSImager
};  //namespace NSIoPipe
};  //namespace NSCam
#endif  //  _MTK_PLATFORM_HARDWARE_MTKCAM_CORE_IOPIPE_SIMAGER_IIMAGE_TRANSFORM_H_

