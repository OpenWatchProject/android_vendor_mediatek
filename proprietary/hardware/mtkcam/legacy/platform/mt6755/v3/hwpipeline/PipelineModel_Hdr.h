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

#ifndef _MTK_PLATFORM_HARDWARE_MTKCAM_V3_HWPIPELINE_PIPELINEMODEL_HDR_H_
#define _MTK_PLATFORM_HARDWARE_MTKCAM_V3_HWPIPELINE_PIPELINEMODEL_HDR_H_
//
//#include <v3/pipeline/IPipelineModelMgr.h>
#include <mtkcam/v3/hwpipeline/NodeId.h>
#include <mtkcam/v3/hwpipeline/StreamId.h>
#include <mtkcam/v3/pipeline/IPipelineModel.h>

/******************************************************************************
 *
 ******************************************************************************/
namespace NSCam {
namespace v3 {


/******************************************************************************
 *
 ******************************************************************************/
class PipelineModel_Hdr
    : public virtual IPipelineModel
{
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Definitions.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////                    Definitions.

    struct Duration
    {
        MINT64 minDuration;
        MINT64 stallDuration;
        void set(MINT64 min, MINT64 stall) {
            minDuration = min;
            stallDuration = stall;
        };
    };
    struct  PipeConfigParams
    {
        android::sp<IMetaStreamInfo>pMeta_Control;

        android::sp<IImageStreamInfo>
                                    pImage_Raw;

        struct Duration             mImage_Raw_Duration;
        android::sp<IImageStreamInfo>
                                    pImage_Jpeg_Stall;

        struct Duration             mImage_Jpeg_Duration;
        android::Vector <
            android::sp<IImageStreamInfo>
                        >           vImage_Yuv_NonStall;
        android::Vector <Duration>
                                    vImage_Yuv_Duration;

        MUINT32                     mOperation_mode;
    };

	class PipelineModelCallback
        : public virtual android::RefBase
    {
    public:     ////            Operations.
        virtual MVOID           updateFrame(
                                    MUINT32 const RequestNo,
                                    MUINT32 const frameNo
                                    //MINTPTR const userId,
                                    //ssize_t const /*nOutMetaLeft*/,
                                    //android::Vector<android::sp<IMetaStreamBuffer> > /*vOutMeta*/
                                )                                           = 0;
    };

	struct HdrRequest
	{
        /**
         * @param[in] frame number.
         *  The caller must promise its content.
         *  The callee can not modify it.
         */
        MUINT32                     frameNo;

        /**
         * @param[in] A pointer to the callback.
         *  The caller must promise its content.
         *  The callee can not modify it.
         */
        //PipelineModelCallback*      pPipelineModelCallback;

        /**
         * @param[in,out] input image stream buffers, if any.
         *  The caller must promise the number of buffers and each content.
         *  The callee will update each buffer's users.
         */
        android::KeyedVector<
            StreamId_T,
            android::sp<IImageStreamBuffer>
                            >       vIImageBuffers;

        /**
         * @param[in,out] output image stream buffers.
         *  The caller must promise the number of buffers and each content.
         *  The callee will update each buffer's users.
         */
        android::KeyedVector<
            StreamId_T,
            android::sp<IImageStreamBuffer>
                            >       vOImageBuffers;

        /**
         * @param[in,out] input meta stream buffers.
         *  The caller must promise the number of buffers and each content.
         *  The callee will update each buffer's users.
         */
        android::KeyedVector<
            StreamId_T,
            android::sp<IMetaStreamBuffer>
                            >       vIMetaBuffers;
	};

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  IPipelineModel Interfaces.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////                    Attributes.
    static  PipelineModel_Hdr*      create(
                                        MINT32 const openId,
                                        android::wp<IPipelineModelMgr::IAppCallback> pAppCallback
                                        );
    static  char const*             magicName() { return "PipelineModel_Hdr"; }

    /**
     * Configure.
     *
     * @param[in] rAppParams
     *
     * @return
     *      0 indicates success; otherwise failure.
     */
    virtual MERROR                  configure(
                                        PipeConfigParams const& rConfigParams,
                                        android::sp<IPipelineModel> pOldPipeline = NULL
                                    )                                       = 0;
    virtual void                    onLastStrongRef(const void* id)     = 0;
};


/******************************************************************************
 *
 ******************************************************************************/
};  //namespace v3
};  //namespace NSCam
#endif  //_MTK_PLATFORM_HARDWARE_MTKCAM_V3_HWPIPELINE_PIPELINEMODEL_HDR_H_

