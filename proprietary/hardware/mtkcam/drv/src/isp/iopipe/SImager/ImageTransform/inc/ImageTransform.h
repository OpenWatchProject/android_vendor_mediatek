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
#ifndef _MTK_PLATFORM_HARDWARE_MTKCAM_CORE_IOPIPE_SIMAGER_IMAGE_TRANSFORM_H_
#define _MTK_PLATFORM_HARDWARE_MTKCAM_CORE_IOPIPE_SIMAGER_IMAGE_TRANSFORM_H_
//
#include <mtkcam/utils/imgbuf/IImageBuffer.h>
#include <mtkcam/drv/iopipe/SImager/IImageTransform.h>
#include <mtkcam/drv/def/IPostProcDef.h>
//
#include <mtkcam/aaa/IHalISP.h>
#include <mtkcam/drv/iopipe/PostProc/INormalStream.h>
#include <semaphore.h>
#include <atomic>
#include <string>

class DpIspStream;
#include "DpDataType.h"
#include "DpIspStream.h"

using namespace NS3Av3;
/*******************************************************************************
*
********************************************************************************/
namespace NSCam {
namespace NSIoPipe {
namespace NSSImager {

using NSCam::NSIoPipe::NSPostProc::INormalStream;
////////////////////////////////////////////////////////////////////////////////

/*******************************************************************************
* Interface of Image Transform
********************************************************************************/
class ImageTransform
{
public:  ////    Constructor/Destructor.
    ImageTransform(const char* sUserName="", MINT iSensorIdx = -1);
    virtual           ~ImageTransform();

public:     ////    Attributes
    virtual MINT32    getLastErrorCode() const    { return mi4ErrorCode; }


public:     ////    Operations.
    /*
     *  Image transform , the functionality is such as bitblt function
     *
     *  Params:
     *      rSrcBufInfo
     *      [I] The image buffer info of the input image
     *
     *
     *      rDstBufInfo
     *      [I] The image buffer info of the output image
     *
     *      rROI
     *      [I] The crop of region of the input image
     *
     *      u4Rotation
     *      [I] The rotation operation
     *
     *      u4Flip
     *      [I] The flip operation
     *
     *      bLastOut
     *      [I] indicate last out for 1-in-2-out case,
     *          because only 1-in-1-out for 1 port MDP, shouldn't set dre command to "UnInitialize" for first out
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
                          MBOOL const bLastOut = MTRUE,
                          struct timeval *endTime = NULL,
                          MBOOL const bNeedDump = MTRUE
                      );

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
                      MBOOL const bNeedDump = MTRUE
                      );

    virtual MBOOL     setPQParameter(
                          const IImageTransform::PQParam& p
                          __attribute__((unused))
                      );
    virtual MBOOL     setSensorIndex(MINT32 iSensorIdx);
    virtual MVOID     setDumpInfo(const IImageTransform::DumpInfos& info);

    virtual
    const
    map<int,IImageTransform::PQParam>&
                      getPQParameter() const;

    virtual MBOOL     executeByPassISP(
                          IImageBuffer const *pSrcBuf,
                          IImageBuffer const * pDstBuf_0,
                          IImageBuffer const * pDstBuf_1,
                          MRect const rROI_0,
                          MRect const rROI_1,
                          MUINT32 const u4Transform_0,
                          MUINT32 const u4Transform_1,
                          MUINT32 const u4TimeOutInMs,
                          struct timeval *endTime = NULL,
                          MBOOL const bNeedDump = MTRUE
                      );
    virtual MBOOL     executeByPassISP(
                          IImageBuffer const *pSrcBuf,
                          IImageBuffer const * pDstBuf_0,
                          IImageBuffer const * pDstBuf_1,
                          MRect const rROI_0,
                          MUINT32 const u4Transform_0,
                          MUINT32 const u4TimeOutInMs,
                          struct timeval *endTime = NULL,
                          MBOOL const bNeedDump = MTRUE
                      );
protected:     ////
    MBOOL             convertTransform(
                          MUINT32 const u4Transform,
                          MUINT32 & u4Rotation,
                          MUINT32 & u4Flip
                      );

    MBOOL             mapDpFormat(
                          NSCam::EImageFormat const fmt,
                          DpColorFormat* dp_fmt
                      );

    MBOOL             configPort(
                          MUINT32 const port,
                          IImageBuffer const *pBufInfo
                      );

    MBOOL             enqueBuffer(
                          MUINT32 const port,
                          IImageBuffer const *pBufInfo
                      );

    MBOOL             dequeDstBuffer(
                          MUINT32 const port,
                          IImageBuffer const *pBufInfo
                      );

    MBOOL             enquePQParam(
                          MUINT32                           port,
                          const IImageTransform::PQParam&   cfg
                      );

private:
    struct EnqueCookie
    {
    public:
        EnqueCookie(ImageTransform* pUser, void* pData, int iTaskID)
        : mpUser(pUser), mpTuningData(pData), miTaskID(iTaskID), mbResult(MFALSE) {}
        MVOID updateResult(bool bResult) {mbResult = bResult;}
        MBOOL isSuccess() {return mbResult;}
    public:
        ImageTransform* mpUser;
        void* mpTuningData;
        std::vector<DpPqParam*> mvDpPqParam;
        int miTaskID;
        bool mbResult;
    };
private:
    MINT32            mi4ErrorCode;
    DpIspStream*      mpStream;

    MBOOL             mSupportCZ;
    MBOOL             mSupportDRE;
    MBOOL             mSupportHFG;

    map<int,IImageTransform::PQParam> mvPQParam;

    IHalISP*          mpIspHal;
    INormalStream*    mpNormalStream;
    MINT32            miSensorIndex;
    sem_t             mSemaphore;
    static std::atomic<int> miTaskID;
    std::string       msUserName;
    MINT32            miUniqueKey;
    IImageTransform::DumpInfos mDumpInfo;
    MBOOL             mbAddIMG3ODump;
    MBOOL             mbEnableLog = MFALSE;
private:
    //
    MBOOL        lock(MUINT32 const u4TimeOutInMs = 0xFFFFFFFF);
    MBOOL        unlock();
    // use to parse PQParam into DpPqParam
    MBOOL        parseDpPQParam(const IImageTransform::PQParam&  cfg, DpPqParam& rOutput);
    /**
     * @brief NormalStream success callback function
     * @param [in] rParams dequeued QParams
     */
    static MVOID onP2Callback(QParams& rParams);
    static MVOID onP2FailedCallback(QParams& rParams);

};


////////////////////////////////////////////////////////////////////////////////
};  //namespace NSSImager
};  //namespace NSIoPipe
};  //namespace NSCam

#endif  //  _MTK_PLATFORM_HARDWARE_MTKCAM_CORE_IOPIPE_SIMAGER_IMAGE_TRANSFORM_H_

