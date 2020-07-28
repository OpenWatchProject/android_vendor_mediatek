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
 * MediaTek Inc. (C) 2018. All rights reserved.
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

#ifndef _MTK_CAMERA_CAPTURE_FEATURE_PIPE_ROOT_NODE_H_
#define _MTK_CAMERA_CAPTURE_FEATURE_PIPE_ROOT_NODE_H_

#include "CaptureFeatureNode.h"
#include <utils/KeyedVector.h>

#ifdef SUPPORT_MFNR
// ALGORITHM
#include <MTKBss.h>
#include <mtkcam/utils/hw/IFDContainer.h>

//Transfer FD to BSS_FD
struct MTKBSSFDInfo {
    BssFaceMetadata facedata;
    BssFace         faces[15];
    BssFaceInfo     posInfo[15];
    MTKBSSFDInfo ()
    {
        memset(&facedata,   0, sizeof(BssFaceMetadata));
        memset(&faces[0],   0, 15*sizeof(BssFace));
        memset(&posInfo[0], 0, 15*sizeof(BssFaceInfo));
    };
    ~MTKBSSFDInfo () {};

    void parser(FD_DATATYPE& mtkFace)
    {

#define MFLLBSS_COPY(param1, param2, element) \
        do { \
            param1.element = param2.element; \
        } while (0)
#define MFLLBSS_COPY_ARRAY(param1, param2, array, size) \
        do { \
            for (int i = 0 ; i  < size ; i++) \
                param1.array[i] = param2.array[i]; \
        } while (0)
#define MFLLBSS_COPY_ARRAY2(param1, param2, array, M, N) \
        do { \
            for (int j = 0 ; j  < N ; j++) \
                for (int i = 0 ; i  < M ; i++) \
                    param1.array[i][j] = param2.array[i][j] ;\
        } while (0)

#define MFLLBSS_FACEMETA_COPY(element)              MFLLBSS_COPY(facedata, mtkFace.facedata, element)
#define MFLLBSS_FACEMETA_COPY_ARRAY(array, size)    MFLLBSS_COPY_ARRAY(facedata, mtkFace.facedata, array, size)
#define MFLLBSS_FACEMETA_COPY_ARRAY2(array, M, N)   MFLLBSS_COPY_ARRAY2(facedata, mtkFace.facedata, array, M, N)
        //BssFaceMetadata facedata
        MFLLBSS_FACEMETA_COPY(number_of_faces);
        //
        facedata.faces = &faces[0];
        facedata.posInfo = &posInfo[0];
        MFLLBSS_FACEMETA_COPY_ARRAY(faces_type, 15);
        MFLLBSS_FACEMETA_COPY_ARRAY2(motion, 15, 2);
        //
        MFLLBSS_FACEMETA_COPY(ImgWidth);
        MFLLBSS_FACEMETA_COPY(ImgHeight);
        //
        MFLLBSS_FACEMETA_COPY(gmIndex);
        MFLLBSS_FACEMETA_COPY(gmData);
        MFLLBSS_FACEMETA_COPY(gmSize);
        MFLLBSS_FACEMETA_COPY(genderNum);
        MFLLBSS_FACEMETA_COPY(poseNum);
        MFLLBSS_FACEMETA_COPY(landmarkNum);
        MFLLBSS_FACEMETA_COPY_ARRAY(leyex0, 15);
        MFLLBSS_FACEMETA_COPY_ARRAY(leyey0, 15);
        MFLLBSS_FACEMETA_COPY_ARRAY(leyex1, 15);
        MFLLBSS_FACEMETA_COPY_ARRAY(leyey1, 15);
        MFLLBSS_FACEMETA_COPY_ARRAY(reyex0, 15);
        MFLLBSS_FACEMETA_COPY_ARRAY(reyey0, 15);
        MFLLBSS_FACEMETA_COPY_ARRAY(reyex1, 15);
        MFLLBSS_FACEMETA_COPY_ARRAY(reyey1, 15);
        MFLLBSS_FACEMETA_COPY_ARRAY(nosex, 15);
        MFLLBSS_FACEMETA_COPY_ARRAY(nosey, 15);
        MFLLBSS_FACEMETA_COPY_ARRAY(mouthx0, 15);
        MFLLBSS_FACEMETA_COPY_ARRAY(mouthy0, 15);
        MFLLBSS_FACEMETA_COPY_ARRAY(mouthx1, 15);
        MFLLBSS_FACEMETA_COPY_ARRAY(mouthy1, 15);
        MFLLBSS_FACEMETA_COPY_ARRAY(leyeux, 15);
        MFLLBSS_FACEMETA_COPY_ARRAY(leyeuy, 15);
        MFLLBSS_FACEMETA_COPY_ARRAY(leyedx, 15);
        MFLLBSS_FACEMETA_COPY_ARRAY(leyedy, 15);
        MFLLBSS_FACEMETA_COPY_ARRAY(reyeux, 15);
        MFLLBSS_FACEMETA_COPY_ARRAY(reyeuy, 15);
        MFLLBSS_FACEMETA_COPY_ARRAY(reyedx, 15);
        MFLLBSS_FACEMETA_COPY_ARRAY(reyedy, 15);
        MFLLBSS_FACEMETA_COPY_ARRAY(fa_cv, 15);
        MFLLBSS_FACEMETA_COPY_ARRAY(fld_rip, 15);
        MFLLBSS_FACEMETA_COPY_ARRAY(fld_rop, 15);
        MFLLBSS_FACEMETA_COPY_ARRAY2(YUVsts, 15, 5);
        MFLLBSS_FACEMETA_COPY_ARRAY(fld_GenderLabel, 15);
        MFLLBSS_FACEMETA_COPY_ARRAY(fld_GenderInfo, 15);
        MFLLBSS_FACEMETA_COPY_ARRAY(GenderLabel, 15);
        MFLLBSS_FACEMETA_COPY_ARRAY(oGenderLabel, 15);
        MFLLBSS_FACEMETA_COPY_ARRAY(GenderCV, 15);
        MFLLBSS_FACEMETA_COPY_ARRAY(RaceLabel, 15);
        MFLLBSS_FACEMETA_COPY_ARRAY2(RaceCV, 15, 4);
        //
        MFLLBSS_FACEMETA_COPY_ARRAY2(poseinfo, 15, 10);
        //
        MFLLBSS_FACEMETA_COPY(timestamp);
        //
        MFLLBSS_COPY(facedata.CNNFaces, mtkFace.facedata.CNNFaces, PortEnable);
        MFLLBSS_COPY(facedata.CNNFaces, mtkFace.facedata.CNNFaces, IsTrueFace);
        MFLLBSS_COPY(facedata.CNNFaces, mtkFace.facedata.CNNFaces, CnnResult0);
        MFLLBSS_COPY(facedata.CNNFaces, mtkFace.facedata.CNNFaces, CnnResult1);

        //BssFace         faces[15];
        for (size_t f = 0; f < 15 ; f++) {
            MFLLBSS_COPY_ARRAY(faces[f], mtkFace.facedata.faces[f], rect, 4);
            MFLLBSS_COPY(faces[f], mtkFace.facedata.faces[f], score);
            MFLLBSS_COPY(faces[f], mtkFace.facedata.faces[f], id);
            MFLLBSS_COPY_ARRAY(faces[f], mtkFace.facedata.faces[f], left_eye, 2);
            MFLLBSS_COPY_ARRAY(faces[f], mtkFace.facedata.faces[f], right_eye, 2);
            MFLLBSS_COPY_ARRAY(faces[f], mtkFace.facedata.faces[f], mouth, 2);
        }
        //BssFaceInfo     posInfo[15];
        for (size_t p = 0; p < 15 ; p++) {
            MFLLBSS_COPY(posInfo[p], mtkFace.facedata.posInfo[p], rop_dir);
            MFLLBSS_COPY(posInfo[p], mtkFace.facedata.posInfo[p], rip_dir);
        }

#undef MFLLBSS_FACEMETA_COPY_ARRAY2
#undef MFLLBSS_FACEMETA_COPY_ARRAY
#undef MFLLBSS_FACEMETA_COPY
#undef MFLLBSS_COPY_ARRAY2
#undef MFLLBSS_COPY_ARRAY
#undef MFLLBSS_COPY
    }
};
#endif

namespace NSCam {
namespace NSCamFeature {
namespace NSFeaturePipe {
namespace NSCapture {

class RootNode : public CaptureFeatureNode
{
#ifdef SUPPORT_MFNR
    /*
     *  Tuning Param for EIS.
     *  Should not be configure by customer
     */
    static const int MFC_GMV_CONFX_TH = 25;
    static const int MFC_GMV_CONFY_TH = 25;
    static const int MAX_GMV_CNT = MAX_FRAME_NUM;

    struct GMV{
        MINT32 x = 0;
        MINT32 y = 0;
    };
#endif
public:
    RootNode(NodeID_T nid, const char *name, MINT32 policy = SCHED_NORMAL, MINT32 priority = DEFAULT_CAMTHREAD_PRIORITY);
    virtual ~RootNode();

public:
    virtual MBOOL onData(DataID id, const RequestPtr& pRequest);
    virtual MBOOL onAbort(RequestPtr &data);
    virtual MERROR evaluate(NodeID_T nodeId, CaptureFeatureInferenceData& rInference);

protected:
    virtual MBOOL onInit();
    virtual MBOOL onThreadStart();
    virtual MBOOL onThreadLoop();
    virtual MBOOL onThreadStop();

    MBOOL onRequestProcess(RequestPtr&);
    MVOID onRequestFinish(const RequestPtr&);
private:

#ifdef SUPPORT_MFNR
    MBOOL doBss(Vector<RequestPtr>& rvToDoRequests);
    MVOID updateMetadata(Vector<RequestPtr>& rvOrderedRequests, size_t i4SkipFrmCnt, MetadataID_T metaId);
    MVOID updateFaceData(RequestPtr pGoldenRequest, RequestPtr pFirstPipelineRequest);
#endif
    MVOID reorder(Vector<RequestPtr>& rvRequests, Vector<RequestPtr>& rvOrderedRequests, size_t i4SkipFrmCnt);

private:

    mutable Mutex                       mLock;
    mutable Condition                   mWaitCondition;
    bool                                mbWait = false;
    WaitQueue<RequestPtr>               mRequests;
    Vector<RequestPtr>                  mvPendingRequests;
    Vector<RequestPtr>                  mvRestoredRequests;
    Vector<RequestPtr>                  mvBypassBSSRequest;

    class FDContainerWraper;
    UniquePtr<FDContainerWraper>        mFDContainerWraperPtr;

    mutable Mutex                       mNvramChunkLock;
    MINT32                              mDebugLevel;
    MINT32                              mMfnrQueryIndex;
    MINT32                              mMfnrDecisionIso;
    MINT32                              mDebugDump;
    MINT32                              mEnableBSSOrdering;
    MINT32                              mDebugDrop;
    MINT32                              mDebugLoadIn;
};

} // NSCapture
} // namespace NSFeaturePipe
} // namespace NSCamFeature
} // namespace NSCam

#endif // _MTK_CAMERA_CAPTURE_FEATURE_PIPE_ROOT_NODE_H_
