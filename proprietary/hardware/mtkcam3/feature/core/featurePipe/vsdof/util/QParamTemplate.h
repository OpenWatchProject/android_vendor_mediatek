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

#ifndef _MTK_CAMERA_FEATURE_PIPE_CORE_QPARAM_TEMPLATE_H_
#define _MTK_CAMERA_FEATURE_PIPE_CORE_QPARAM_TEMPLATE_H_

#include <utils/Vector.h>
#include <utils/KeyedVector.h>
#include <mtkcam/utils/metadata/IMetadata.h>
#include <mtkcam3/feature/effectHalBase/EffectRequest.h>
#include <mtkcam/def/common.h>
#include <mtkcam/utils/std/Trace.h>
#include <mtkcam/drv/iopipe/Port.h>
#include <mtkcam/drv/iopipe/PostProc/IHalPostProcPipe.h>
#include <mtkcam/drv/iopipe/PostProc/INormalStream.h>

namespace NSCam {
namespace NSCamFeature {
namespace NSFeaturePipe {

using namespace android;
using namespace NSIoPipe;
using namespace NSCam::NSIoPipe::NSPostProc;

/*******************************************************************************
* Enum Definition
********************************************************************************/
enum eCropGroup
{
    eCROP_CRZ=1,
    eCROP_WDMA=2,
    eCROP_WROT=3
};

/*******************************************************************************
* Class Definition
********************************************************************************/

/**
 * @class QParamTemplateGenerator
 * @brief Store the location information of mvIn, mvOut, mvCropRsInfo for each frame
 **/
class QParamTemplateGenerator
{
private:
    QParamTemplateGenerator();
    MBOOL checkValid();
    MVOID debug(QParams& rQParam);
public:
    QParamTemplateGenerator(MUINT32 frameID, MUINT32 iSensorIdx, ENormalStreamTag streamTag);
    QParamTemplateGenerator& addCrop(eCropGroup groupID, MPoint startLoc, MSize cropSize, MSize resizeDst, bool isMDPCrop=false);
    QParamTemplateGenerator& addInput(PortID portID);
    QParamTemplateGenerator& addOutput(PortID portID, MINT32 transform=0);
    QParamTemplateGenerator& addModuleInfo(MUINT32 moduleTag, MVOID* moduleStruct);
    QParamTemplateGenerator& addExtraParam(EPostProcCmdIndex cmdIdx, MVOID* param);
    MBOOL generate(QParams& rQParamTmpl);

public:
    MUINT32 miFrameID;
    FrameParams mPass2EnqueFrame;
};

/**
 * @class QParamTemplateFiller
 * @brief Fill the corresponding input/output/tuning buffer and configure crop information
 **/
class QParamTemplateFiller
{

public:
    struct FrameInfo
    {
        MUINT32 FrameNo;
        MUINT32 RequestNo;
        MUINT32 Timestamp;
        MINT32 UniqueKey;
        MINT32 IspProfile;
        MINT32 SensorID;
    };
    QParamTemplateFiller(QParams& target);
    QParamTemplateFiller& insertTuningBuf(MUINT frameID, MVOID* pTuningBuf);
    QParamTemplateFiller& insertInputBuf(MUINT frameID, PortID portID, IImageBuffer* pImggBuf);
    QParamTemplateFiller& insertInputBuf(MUINT frameID, PortID portID, sp<EffectFrameInfo> pFrameInfo);
    QParamTemplateFiller& insertOutputBuf(MUINT frameID, PortID portID, IImageBuffer* pImggBuf);
    QParamTemplateFiller& insertOutputBuf(MUINT frameID, PortID portID, sp<EffectFrameInfo> pFrameInfo);
    QParamTemplateFiller& setCrop(MUINT frameID, eCropGroup groupID, MPoint startLoc, MSize cropSize, MSize resizeDst, bool isMDPCrop=false);
    QParamTemplateFiller& setCropResize(MUINT frameID, eCropGroup groupID, MSize resizeDst);
    QParamTemplateFiller& setExtOffset(MUINT frameID, PortID portID, MINT32 offsetInBytes);
    QParamTemplateFiller& setInfo(MUINT frameID, const FrameInfo& frameInfo, MINT8 dumpFlag);
    QParamTemplateFiller& delOutputPort(MUINT frameID, PortID portID, eCropGroup cropGID);
    QParamTemplateFiller& delInputPort(MUINT frameID, PortID portID);
    /**
     * Validate the template filler status
     *
     * @return
     *  -  true if successful; otherwise false.
     **/
    MBOOL validate();
public:
    QParams& mTargetQParam;
    MBOOL mbSuccess;

};

} //NSFeaturePipe
} //NSCamFeature
} //NSCam


#endif
