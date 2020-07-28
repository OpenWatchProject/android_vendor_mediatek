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

#include "StreamingFeatureNode.h"

#define PIPE_CLASS_TAG "Node"
#define PIPE_TRACE TRACE_STREAMING_FEATURE_NODE
#include <featurePipe/core/include/PipeLog.h>

namespace NSCam {
namespace NSCamFeature {
namespace NSFeaturePipe {
const char* StreamingFeatureDataHandler::ID2Name(DataID id)
{
#define MAKE_NAME_CASE(name)  \
    case name: return #name;

    switch(id)
    {
    case ID_ROOT_ENQUE:             return "root_enque";
    case ID_ROOT_TO_P2A:            return "root_to_p2a";
    case ID_ROOT_TO_RSC:            return "root_to_rsc";
    case ID_P2A_TO_FD_DSIMG:        return "p2a_to_fd";
    case ID_P2A_TO_VFB_DSIMG:       return "p2a_to_vfb";
    case ID_P2A_TO_WARP_FULLIMG:    return "p2a_to_warp";
    case ID_P2A_TO_P2B_FULLIMG:     return "p2a_to_p2b";
    case ID_P2A_TO_EIS_P2DONE:      return "p2a_to_eis_done";
    case ID_P2A_TO_EIS_FM:          return "p2a_to_eis_fm";
    case ID_P2A_TO_HELPER:          return "p2a_to_helper";
    case ID_WARP_TO_MDP_FULLIMG:    return "warp_to_mdp";
    case ID_WARP_TO_HELPER:         return "warp_to_helper";
    case ID_MDP_TO_P2B_FULLIMG:     return "gpu_to_p2b";
    case ID_MDP_TO_HELPER:          return "mdp_to_helper";
    case ID_FD_TO_VFB_FACE:         return "fd_to_vfb";
    case ID_VFB_TO_P2B:             return "vfb_to_p2b";
    case ID_VFB_TO_WARP:            return "vfb_to_warp";
    case ID_EIS_TO_VFB_WARP:        return "eis_to_vfb";
    case ID_EIS_TO_WARP:            return "eis_to_warp";
    case ID_P2A_TO_VENDOR_FULLIMG:  return "p2a_to_vendor";
    case ID_VENDOR_TO_VMDP_FULLIMG: return "vendor_to_vmdp";
    case ID_VMDP_TO_NEXT_FULLIMG:   return "vmdp_to_next";
    case ID_VMDP_TO_HELPER:         return "vmdp_to_helper";
    case ID_RSC_TO_HELPER:          return "rsc_to_helper";
    case ID_RSC_TO_EIS:             return "rsc_to_eis";

    case ID_P2A_TO_FOV_FEFM:        return "p2a_to_fov_fefm";
    case ID_P2A_TO_FOV_FULLIMG:     return "p2a_to_fov_fullimg";
    case ID_FOV_TO_FOV_WARP:        return "fov_to_fov_warp";
    case ID_FOV_TO_EIS_WARP:        return "fov_to_eis_warp";
    case ID_FOV_WARP_TO_HELPER:     return "fovwrp_to_helper";
    case ID_FOV_WARP_TO_VENDOR:     return "fovwrp_to_vendor";
    case ID_P2A_TO_FOV_WARP:        return "p2a_to_fov_warp";
    case ID_FOV_TO_EIS_FULLIMG:     return "fov_to_eis_fullimg";
    case ID_P2A_TO_N3DP2:           return "p2a_to_n3dp2";
    case ID_N3DP2_TO_N3D:           return "n3dp2_to_n3d";
    case ID_N3D_TO_HELPER:          return "n3d_to_helper";
    case ID_N3D_TO_VMDP:            return "n3d_to_vmdp";
    case ID_P2A_TO_VFOV_FULLIMG:    return "p2a_to_vfov";
    case ID_VFOV_TO_VENDOR:         return "vfov_to_vendor";
    case ID_VFOV_TO_VMDP_FULLIMG:   return "vfov_to_vmdp";
    case ID_RSC_TO_P2A:             return "rsc_to_p2a";
    default:                        return "unknown";
    };
#undef MAKE_NAME_CASE
}

NodeSignal::NodeSignal()
    : mSignal(0)
    , mStatus(0)
{
}

NodeSignal::~NodeSignal()
{
}

MVOID NodeSignal::setSignal(Signal signal)
{
    android::Mutex::Autolock lock(mMutex);
    mSignal |= signal;
    mCondition.broadcast();
}

MVOID NodeSignal::clearSignal(Signal signal)
{
    android::Mutex::Autolock lock(mMutex);
    mSignal &= ~signal;
}

MBOOL NodeSignal::getSignal(Signal signal)
{
    android::Mutex::Autolock lock(mMutex);
    return (mSignal & signal);
}

MVOID NodeSignal::waitSignal(Signal signal)
{
    android::Mutex::Autolock lock(mMutex);
    while( !(mSignal & signal) )
    {
        mCondition.wait(mMutex);
    }
}

MVOID NodeSignal::setStatus(Status status)
{
    android::Mutex::Autolock lock(mMutex);
    mStatus |= status;
}

MVOID NodeSignal::clearStatus(Status status)
{
    android::Mutex::Autolock lock(mMutex);
    mStatus &= ~status;
}

MBOOL NodeSignal::getStatus(Status status)
{
    android::Mutex::Autolock lock(mMutex);
    return (mStatus & status);
}

StreamingFeatureDataHandler::~StreamingFeatureDataHandler()
{
}

StreamingFeatureNode::StreamingFeatureNode(const char *name)
    : CamThreadNode(name)
    , mSensorIndex(-1)
    , mNodeDebugLV(0)
    , mPipeUsage()
{
}

StreamingFeatureNode::~StreamingFeatureNode()
{
}

MBOOL StreamingFeatureNode::onInit()
{
    mNodeDebugLV = getFormattedPropertyValue("debug.%s", this->getName());
    return MTRUE;
}

MVOID StreamingFeatureNode::setSensorIndex(MUINT32 sensorIndex)
{
    mSensorIndex = sensorIndex;
}

MVOID StreamingFeatureNode::setPipeUsage(const StreamingFeaturePipeUsage &usage)
{
    mPipeUsage = usage;
}

MVOID StreamingFeatureNode::setNodeSignal(const android::sp<NodeSignal> &nodeSignal)
{
    mNodeSignal = nodeSignal;
}

MBOOL StreamingFeatureNode::dumpNddData(TuningUtils::FILE_DUMP_NAMING_HINT *hint, IImageBuffer *buffer, MUINT32 portIndex)
{
    if( hint && buffer )
    {
        char fileName[256] = {0};
        extract(hint, buffer);

        if( portIndex == NSImageio::NSIspio::EPortIndex_IMG3O )
        {
            genFileName_YUV(fileName, sizeof(fileName), hint, NSCam::TuningUtils::YUV_PORT_IMG3O);
        }

        MY_LOGD("dump to: %s", fileName);
        buffer->saveToFile(fileName);
    }
    return MTRUE;
}

MBOOL StreamingFeatureNode::dumpData(const RequestPtr &request, const ImgBuffer &buffer, const char *fmt, ...)
{
    TRACE_FUNC_ENTER();
    MBOOL ret = MFALSE;
    if( buffer != NULL && fmt )
    {
        char name[256];
        va_list ap;
        va_start(ap, fmt);
        if( 0 >= vsnprintf(name, sizeof(name), fmt, ap) )
        {
            strncpy(name, "NA", sizeof(name));
            name[sizeof(name)-1] = 0;
        }
        va_end(ap);
        ret = dumpNamedData(request, buffer->getImageBufferPtr(), name);
    }
    TRACE_FUNC_EXIT();
    return ret;
}

MBOOL StreamingFeatureNode::dumpData(const RequestPtr &request, const BasicImg &buffer, const char *fmt, ...)
{
    TRACE_FUNC_ENTER();
    MBOOL ret = MFALSE;
    if( buffer.mBuffer != NULL && fmt )
    {
        char name[256];
        va_list ap;
        va_start(ap, fmt);
        if( 0 >= vsnprintf(name, sizeof(name), fmt, ap) )
        {
            strncpy(name, "NA", sizeof(name));
            name[sizeof(name)-1] = 0;
        }
        va_end(ap);
        ret = dumpNamedData(request, buffer.mBuffer->getImageBufferPtr(), name);
    }
    TRACE_FUNC_EXIT();
    return ret;
}

MBOOL StreamingFeatureNode::dumpData(const RequestPtr &request, IImageBuffer *buffer, const char *fmt, ...)
{
    TRACE_FUNC_ENTER();
    MBOOL ret = MFALSE;
    if( buffer && fmt )
    {
        char name[256];
        va_list ap;
        va_start(ap, fmt);
        if( 0 >= vsnprintf(name, sizeof(name), fmt, ap) )
        {
            strncpy(name, "NA", sizeof(name));
            name[sizeof(name)-1] = 0;
        }
        va_end(ap);
        ret = dumpNamedData(request, buffer, name);
    }
    TRACE_FUNC_EXIT();
    return ret;
}

MBOOL StreamingFeatureNode::dumpNamedData(const RequestPtr &request, IImageBuffer *buffer, const char *name)
{
    TRACE_FUNC_ENTER();
    MBOOL ret = MFALSE;
    if( buffer && name )
    {
        MUINT32 stride, pbpp, ibpp, width, height, size;
        stride = buffer->getBufStridesInBytes(0);
        pbpp = buffer->getPlaneBitsPerPixel(0);
        ibpp = buffer->getImgBitsPerPixel();
        size = buffer->getBufSizeInBytes(0);
        pbpp = pbpp ? pbpp : 8;
        width = stride * 8 / pbpp;
        width = width ? width : 1;
        ibpp = ibpp ? ibpp : 8;
        height = size / width;
        if( buffer->getPlaneCount() == 1 )
        {
          height = height * 8 / ibpp;
        }

        char path[256];
        snprintf(path, sizeof(path), "/data/vendor/dump/%04d_r%04d_%s_%dx%d_%dx%d.bin", request->mRequestNo, request->mRecordNo, name,
            buffer->getImgSize().w, buffer->getImgSize().h, width, height);

        TRACE_FUNC("dump to %s", path);
        buffer->saveToFile(path);
        ret = MTRUE;
    }
    TRACE_FUNC_EXIT();
    return ret;
}

MUINT32 StreamingFeatureNode::dumpData(const char *buffer, MUINT32 size, const char *filename)
{
    uint32_t writeCount = 0;
    int fd = ::open(filename, O_RDWR | O_CREAT | O_TRUNC, S_IRWXU);
    if( fd < 0 )
    {
        MY_LOGE("Cannot create file [%s]", filename);
    }
    else
    {
        for( int cnt = 0, nw = 0; writeCount < size; ++cnt )
        {
            nw = ::write(fd, buffer + writeCount, size - writeCount);
            if( nw < 0 )
            {
                MY_LOGE("Cannot write to file [%s]", filename);
                break;
            }
            writeCount += nw;
        }
        ::close(fd);
    }
    return writeCount;
}

MBOOL StreamingFeatureNode::loadData(IImageBuffer *buffer, const char *filename)
{
    MBOOL ret = MFALSE;
    if( buffer )
    {
        loadData((char*)buffer->getBufVA(0), 0, filename);
        ret = MTRUE;
    }
    return MFALSE;
}

MUINT32 StreamingFeatureNode::loadData(char *buffer, size_t size, const char *filename)
{
    uint32_t readCount = 0;
    int fd = ::open(filename, O_RDONLY);
    if( fd < 0 )
    {
        MY_LOGE("Cannot open file [%s]", filename);
    }
    else
    {
        if( size == 0 )
        {
            off_t readSize = ::lseek(fd, 0, SEEK_END);
            size = (readSize < 0) ? 0 : readSize;
            ::lseek(fd, 0, SEEK_SET);
        }
        for( int cnt = 0, nr = 0; readCount < size; ++cnt )
        {
            nr = ::read(fd, buffer + readCount, size - readCount);
            if( nr < 0 )
            {
                MY_LOGE("Cannot read from file [%s]", filename);
                break;
            }
            readCount += nr;
        }
        ::close(fd);
    }
    return readCount;
}

} // namespace NSFeaturePipe
} // namespace NSCamFeature
} // namespace NSCam
