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
 * MediaTek Inc. (C) 2016. All rights reserved.
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
#include "VendorExample.h"
#include <mtkcam/def/Errors.h>
#include <mtkcam/drv/iopipe/PostProc/IHalPostProcPipe.h>
#include "MDPWrapper.h"

using NSCam::NSIoPipe::Output;
using NSCam::NSIoPipe::MCropRect;
using NSCam::NSIoPipe::PORT_WDMAO;

#undef PIPE_MODULE_TAG
#undef PIPE_CLASS_TAG
#undef PIPE_TRACE

#define PIPE_MODULE_TAG "VendorExample"
#define PIPE_CLASS_TAG "VendorExample"
#define PIPE_TRACE TRACE_VENDOR_NODE
#include <featurePipe/core/include/PipeLog.h>

namespace NSCam {
namespace NSCamFeature {
namespace NSFeaturePipe {

wp<VendorExample> VendorExample::mThiz = NULL;
static Mutex vendorExampleCreateLock;

sp<VendorExample>
VendorExample::
getInstance(void)
{
    TRACE_FUNC_ENTER();
    Mutex::Autolock _l(vendorExampleCreateLock);
    sp<VendorExample> inst = mThiz.promote();

    if (inst.get() == NULL) {
        MY_LOGD("First used. New VendorExample()");
        mThiz = new VendorExample();
        inst = mThiz.promote();
    }
    TRACE_FUNC_EXIT();
    return inst;
}

VendorExample::
VendorExample(void)
{
    TRACE_FUNC_ENTER();

    TRACE_FUNC_EXIT();
}

VendorExample::
~VendorExample(void)
{
    TRACE_FUNC_ENTER();
    TRACE_FUNC_EXIT();
}

MERROR
VendorExample::init(void)
{
    TRACE_FUNC_ENTER();
    MERROR ret = OK;
    TRACE_FUNC_EXIT();
    return ret;
}

MERROR
VendorExample::uninit(void)
{
    TRACE_FUNC_ENTER();
    MERROR ret = OK;
    TRACE_FUNC_EXIT();
    return ret;
}

MERROR
VendorExample::processVendor(const RequestPtr &request, const ImgBuffer &master, const ImgBuffer &slave, const ImgBuffer &out)
{
    TRACE_FUNC_ENTER();
    MERROR ret = OK;
    MY_LOGD("process request(%p), master(%p), slave(%p), out(%p)", request.get(), master.get(), slave.get(), out.get());
    // use MDP to copy master to out
    MDPWrapper::OUTPUT_ARRAY outputs;

    MCropRect cropRect;
    cropRect.s = master->getImageBufferPtr()->getImgSize();

    Output output;
    output.mPortID = PORT_WDMAO;
    output.mBuffer = out->getImageBufferPtr();
    output.mBuffer->setExtParam(cropRect.s);

    outputs.push_back(MDPWrapper::MDPOutput(output, cropRect));

    MDPWrapper mdp;
    mdp.process(master->getImageBufferPtr(), outputs);
    TRACE_FUNC_EXIT();
    return ret;
}

} // namespace NSFeaturePipe
} // namespace NSCamFeature
} // namespace NSCam
