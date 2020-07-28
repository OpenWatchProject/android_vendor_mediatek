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

CAM_ULOG_DECLARE_MODULE_ID(MOD_FPIPE_STREAMING);

using namespace NSCam::NSIoPipe;

#define MAKE_NAME_FROM_VA(name, len, fmt)         \
    if( name && len > 0)                          \
    {                                             \
        va_list ap;                               \
        va_start(ap, fmt);                        \
        if( 0 >= vsnprintf(name, len, fmt, ap) )  \
        {                                         \
            strncpy(name, "NA", len);             \
            name[len-1] = 0;                      \
        }                                         \
        va_end(ap);                               \
    }

namespace NSCam {
namespace NSCamFeature {
namespace NSFeaturePipe {
const char* StreamingFeatureDataHandler::ID2Name(DataID id)
{
#define MAKE_NAME_CASE(name)  \
    case name: return #name;

    switch(id)
    {
    case ID_ROOT_ENQUE:                     return "root_enque";
    case ID_ROOT_TO_P2A:                    return "root_to_p2a";
    case ID_ROOT_TO_P2SM:                   return "root_to_p2sm";
    case ID_ROOT_TO_RSC:                    return "root_to_rsc";
    case ID_ROOT_TO_DEPTH:                  return "root_to_depth";
    case ID_ROOT_TO_TOF:                    return "root_to_tof";
    case ID_ROOT_TO_EIS:                    return "root_to_eis";
    case ID_P2A_TO_P2NR:                    return "p2a_to_p2nr";
    case ID_P2A_TO_WARP_FULLIMG:            return "p2a_to_warp";
    case ID_P2A_TO_EIS_P2DONE:              return "p2a_to_eis_done";
    case ID_P2A_TO_PMDP:                    return "p2a_to_p2amdp";
    case ID_P2A_TO_HELPER:                  return "p2a_to_helper";
    case ID_P2SM_TO_HELPER:                 return "p2sm_to_helper";
    case ID_P2NR_TO_VNR:                    return "p2nr_to_vnr";
    case ID_VNR_TO_NEXT_FULLIMG:            return "vnr_to_next_fullimg";
    case ID_PMDP_TO_HELPER:                 return "p2amdp_to_helper";
    case ID_BOKEH_TO_HELPER:                return "bokeh_to_helper";
    case ID_WARP_TO_HELPER:                 return "warp_to_helper";
    case ID_ASYNC_TO_HELPER:                return "async_to_helper";
    case ID_HELPER_TO_ASYNC:                return "helper_to_async";
    case ID_EIS_TO_WARP:                    return "eis_to_warp";
    case ID_P2A_TO_VENDOR_FULLIMG:          return "p2a_to_vendor";
    case ID_BOKEH_TO_VENDOR_FULLIMG:        return "bokeh_to_vendor";
    case ID_VENDOR_TO_NEXT:                 return "vendor_to_next";
    case ID_VMDP_TO_NEXT_FULLIMG:           return "vmdp_to_next";
    case ID_VMDP_TO_HELPER:                 return "vmdp_to_helper";
    case ID_RSC_TO_HELPER:                  return "rsc_to_helper";
    case ID_RSC_TO_EIS:                     return "rsc_to_eis";
    case ID_DEPTH_TO_BOKEH:                 return "depth_to_bokeh";
    case ID_DEPTH_TO_VENDOR:                return "depth_to_vendor";
    case ID_TOF_TO_NEXT:                    return "tof_to_next";
    case ID_RSC_TO_P2A:                     return "rsc_to_p2a";
    default:                                return "unknown";
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
    , mDebugScanLine(NULL)
{
}

StreamingFeatureNode::~StreamingFeatureNode()
{
    if( mDebugScanLine )
    {
        mDebugScanLine->destroyInstance();
        mDebugScanLine = NULL;
    }
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

MBOOL StreamingFeatureNode::dumpNddData(TuningUtils::FILE_DUMP_NAMING_HINT *hint, const BasicImg &img, TuningUtils::YUV_PORT port, const char *str)
{
    return img.mBuffer != NULL ? dumpNddData(hint, img.mBuffer->getImageBufferPtr(), port, str) : MFALSE;
}

MBOOL StreamingFeatureNode::dumpNddData(TuningUtils::FILE_DUMP_NAMING_HINT *hint, const ImgBuffer &img, TuningUtils::YUV_PORT port, const char *str)
{
    return img != NULL ? dumpNddData(hint, img->getImageBufferPtr(), port, str) : MFALSE;
}

MBOOL StreamingFeatureNode::dumpNddData(TuningUtils::FILE_DUMP_NAMING_HINT *hint, IImageBuffer *buffer, TuningUtils::YUV_PORT portIndex, const char *pUserStr)
{
    if( hint && buffer )
    {
        char fileName[256] = {0};
        extract(hint, buffer);
        genFileName_YUV(fileName, sizeof(fileName), hint, portIndex, pUserStr);

        MY_LOGD("dump to: %s", fileName);
        buffer->saveToFile(fileName);
    }
    return MTRUE;
}

IOPolicyType StreamingFeatureNode::getIOPolicy(StreamType /*stream*/, const StreamingReqInfo &/*reqInfo*/) const
{
    return IOPOLICY_BYPASS;
}

MBOOL StreamingFeatureNode::getInputBufferPool(const StreamingReqInfo &/*reqInfo*/, android::sp<IBufferPool>& /*pool*/, NextFullInfo &/*info*/)
{
    return MFALSE;
}

MVOID StreamingFeatureNode::drawScanLine(IImageBuffer *buffer)
{
    if( mDebugScanLine == NULL)
    {
        mDebugScanLine = DebugScanLine::createInstance();
    }

    if( mDebugScanLine )
    {
        mDebugScanLine->drawScanLine(buffer->getImgSize().w, buffer->getImgSize().h, (void*)(buffer->getBufVA(0)), buffer->getBufSizeInBytes(0), buffer->getBufStridesInBytes(0));
    }
}

MVOID StreamingFeatureNode::enableDumpMask(MUINT32 defaultMask, const std::vector<DumpFilter> &vFilter, const char *postFix)
{
    std::string prop = "vendor.debug.mask.";
    if(postFix != NULL)
    {
        prop += std::string(postFix);
    }
    else
    {
        prop += std::string(this->getName());
    }
    mDebugDumpMask = defaultMask;
    MINT32 tmp = getPropertyValue(prop.c_str(), -1);
    if(tmp != -1)
    {
        mDebugDumpMask = tmp;
        for(const DumpFilter &filter : vFilter)
        {
            MY_LOGI("%s %d=%s", this->getName(), (1 << filter.index), filter.name);
        }
    }
    MY_LOGI("%s current dumpMask(0x%x) prop(%s)", this->getName(),mDebugDumpMask, prop.c_str());
}

MBOOL StreamingFeatureNode::allowDump(MUINT8 maskIndex) const
{
    return (mDebugDumpMask & (1 << maskIndex));
}

MBOOL StreamingFeatureNode::syncAndDump(const RequestPtr &request, const BasicImg &img, const char *fmt, ...)
{
    TRACE_FUNC_ENTER();
    MBOOL ret = MFALSE;
    if( img.mBuffer != NULL && fmt )
    {
        IImageBuffer *buffer = img.mBuffer->getImageBufferPtr();
        if( buffer != NULL )
        {
            char name[256];
            MAKE_NAME_FROM_VA(name, sizeof(name), fmt)         \
            buffer->syncCache(eCACHECTRL_INVALID);
            ret = dumpNamedData(request, buffer, name);
        }
    }

    TRACE_FUNC_EXIT();
    return ret;
}

MBOOL StreamingFeatureNode::syncAndDump(const RequestPtr &request, const ImgBuffer &img, const char *fmt, ...)
{
    TRACE_FUNC_ENTER();
    MBOOL ret = MFALSE;
    if( img != NULL && fmt )
    {
        IImageBuffer *buffer = img->getImageBufferPtr();
        if( buffer != NULL )
        {
            char name[256];
            MAKE_NAME_FROM_VA(name, sizeof(name), fmt)         \
            buffer->syncCache(eCACHECTRL_INVALID);
            ret = dumpNamedData(request, buffer, name);
        }
    }

    TRACE_FUNC_EXIT();
    return ret;
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
        MINT format = buffer->getImgFormat();
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
        snprintf(path, sizeof(path), "/data/vendor/dump/%04d_r%04d_%s_%dx%d_%dx%d.%s.bin",
                request->mRequestNo, request->mRecordNo, name,
                buffer->getImgSize().w, buffer->getImgSize().h, width, height, Fmt2Name(format));

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
