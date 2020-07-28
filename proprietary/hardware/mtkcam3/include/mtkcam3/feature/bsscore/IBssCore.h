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

#ifndef _IBSS_CORE_H_
#define _IBSS_CORE_H_

#include <utils/RefBase.h> // android::RefBase
#include <vector> // std::vector
#include <utils/Vector.h>
#include <utils/Mutex.h>
#include <map>

#include <mtkcam/utils/metadata/hal/mtk_platform_metadata_tag.h>
#include <mtkcam/utils/metadata/client/mtk_metadata_tag.h>
#include <mtkcam/utils/metadata/IMetadata.h>

#include <utils/KeyedVector.h>

#include <mtkcam3/feature/featurePipe/ICaptureFeaturePipe.h>


using android::sp;
using std::vector;
using namespace android;
using namespace NSCam;
using namespace NSCamFeature::NSFeaturePipe::NSCapture;

namespace BSScore {

struct ZipOutData {
    MINT32 imgWidth;
    MINT32 imgHeight;
    MINT32 imgFormat;
    MINT32 imgPlaneCount;
    MINT32 imgStride[3];
    MINT32 imgSize[3];
    MINT32 frameIndex;  //MTK_CONTROL_CAPTURE_HINT_FOR_ISP_FRAME_INDEX
    ZipOutData () : imgWidth{}, imgHeight{}, imgFormat{}, imgPlaneCount{}, imgStride{}, imgSize{}, frameIndex{} {};
};

struct BssRequest {
    MSize ImgSize;
    MINTPTR BufVA;
    size_t BufStridesInBytes;
    MINT32 imgFormat;
};

typedef android::sp<ICaptureFeatureRequest> RequestPtr;

class IBssCore {

public:
/* interfaces */
    static std::shared_ptr<IBssCore> createInstance();
    virtual MVOID Init(MINT32 SensorIndex) = 0;
    virtual MBOOL doBss(Vector<RequestPtr>& rvReadyRequests, Vector<MUINT32>& BSSOrder, int& frameNumToSkip) = 0;
    virtual const std::map<MINT32, MINT32>& getExifDataMap() = 0;

    /**
     *  Qeury BSS zip output buffer size.
     *
     *  @param input              - IImageBuffer that is used to do bss zip
     *  @param pHalMeta           - Hal metadata of input
     */
    virtual MUINT32 getZipOutSize(IImageBuffer* input, IMetadata* pHalMeta);

     /**
     *  Do BSS zip
     *
     *  @param input              - IImageBuffer that is used to do bss zip
     *  @param output             - IImageBuffer that is used to save zip result
     *  @param ImgOutSize         - size query from getZipOutSize
     *  @param pHalMeta           - Hal metadata of input
     *  @param isFirst            - is first frame to do BSS zip
     */
    virtual MVOID   doZip(IImageBuffer* input, IImageBuffer* output, MUINT32 ImgOutSize, IMetadata* pHalMeta, MBOOL isFirst);

protected:
    virtual ~IBssCore(){};

};// class IBssCore

} // namespace BSScore

#endif//_IBSS_CORE_H_
