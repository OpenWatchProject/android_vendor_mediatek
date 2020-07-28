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

#ifndef _MTK_HARDWARE_MTKCAM3_INCLUDE_MTKCAM3_PIPELINE_PIPELINE_TYPES_H_
#define _MTK_HARDWARE_MTKCAM3_INCLUDE_MTKCAM3_PIPELINE_PIPELINE_TYPES_H_
//
#include <inttypes.h>
#include <stdint.h>

#include <sstream>
#include <unordered_set>

#include <utils/String8.h>

#include <mtkcam3/pipeline/stream/IStreamInfo.h> // StreamId_T
#include "IPipelineDAG.h" // NodeId_T, Set<>

/******************************************************************************
 *
 ******************************************************************************/
namespace NSCam::v3::pipeline::NSPipelineContext {


/**
 * A request may consist of multiple frames with the following order:
 *      pre-dummy frame(s) -> main frame -> sub frame(s) -> post-dummy frame(s)
 * where
 *      the main frame must exist, and the others are optional.
 */
enum class GroupFrameType : uint32_t
{
    MAIN = 0,
    SUB,
    PRESUB,
    PREDUMMY,
    POSTDUMMY,
};


static inline std::string toString(GroupFrameType const o)
{
    std::string os;

    switch (o)
    {
    case GroupFrameType::MAIN:
        os += "MAIN";
        break;
    case GroupFrameType::SUB:
        os += "SUB";
        break;
    case GroupFrameType::PRESUB:
        os += "PRESUB";
        break;
    case GroupFrameType::PREDUMMY:
        os += "PREDUMMY";
        break;
    case GroupFrameType::POSTDUMMY:
        os += "POSTDUMMY";
        break;
    default:
        os += "?";
        break;
    }
    return os;
}


/******************************************************************************
 *
 ******************************************************************************/
enum class StreamAttribute : uint32_t
{
    // bit 0~3: buffer source
    BUFFER_SOURCE_USER          = 0x0,  // Buffers are set externally by the user.
    BUFFER_SOURCE_POOL          = 0x1,  // Buffers are acquired from the pool.
    BUFFER_SOURCE_RUNTIME       = 0x2,  // Buffers are run-time allocated.
    BUFFER_SOURCE_PROVIDER      = 0x4,  // Buffers are acquired from the provider.
    BUFFER_SOURCE_MASK          = 0x0f,

    // bit 4: image or meta
    TYPE_IMAGE                  = 0x0,
    TYPE_META                   = 0x10,
    TYPE_MASK                   = 0x10,

    // bit 5: app or hal
    BEHAVIOR_HAL                = 0x0,
    BEHAVIOR_APP                = 0x20,
    BEHAVIOR_MASK               = 0x20,

    /**
     * combination
     */
    HAL_IMAGE                   = BEHAVIOR_HAL | TYPE_IMAGE,
    APP_IMAGE                   = BEHAVIOR_APP | TYPE_IMAGE,
    HAL_META                    = BEHAVIOR_HAL | TYPE_META,
    APP_META                    = BEHAVIOR_APP | TYPE_META,
    // hal image
    HAL_IMAGE_POOL              = HAL_IMAGE | BUFFER_SOURCE_POOL,
    HAL_IMAGE_RUNTIME           = HAL_IMAGE | BUFFER_SOURCE_RUNTIME,
    HAL_IMAGE_PROVIDER          = HAL_IMAGE | BUFFER_SOURCE_PROVIDER,
    // app image
    APP_IMAGE_USER              = APP_IMAGE | BUFFER_SOURCE_USER,
    APP_IMAGE_PROVIDER          = APP_IMAGE | BUFFER_SOURCE_PROVIDER,
    // hal meta
    HAL_META_USER               = HAL_META | BUFFER_SOURCE_USER,
    HAL_META_RUNTIME            = HAL_META | BUFFER_SOURCE_RUNTIME,
    // app meta
    APP_META_USER               = APP_META | BUFFER_SOURCE_USER,
    APP_META_RUNTIME            = APP_META | BUFFER_SOURCE_RUNTIME,

};

/* DEPRECATED - DO NOT USE !!! */
#define eStreamType_IMG_HAL_POOL        ((uint32_t)StreamAttribute::HAL_IMAGE_POOL)
#define eStreamType_IMG_HAL_RUNTIME     ((uint32_t)StreamAttribute::HAL_IMAGE_RUNTIME)
#define eStreamType_IMG_HAL_PROVIDER    ((uint32_t)StreamAttribute::HAL_IMAGE_PROVIDER)
#define eStreamType_IMG_APP             ((uint32_t)StreamAttribute::APP_IMAGE_USER)
#define eStreamType_META_HAL            ((uint32_t)StreamAttribute::HAL_META_USER)
#define eStreamType_META_APP            ((uint32_t)StreamAttribute::APP_META_USER)


static inline std::string toString(const StreamAttribute o)
{
    using ns = StreamAttribute;

    std::string os;

    os += "[";
    {
        switch ( (uint32_t)(o) & (uint32_t)(ns::TYPE_MASK) )
        {
        case (uint32_t)(ns::TYPE_IMAGE):            os += "IMAGE"; break;
        case (uint32_t)(ns::TYPE_META):             os += "META "; break;
        default:                                    os += "?    "; break;
        }
    }
    os += "-";
    {
        switch ( (uint32_t)(o) & (uint32_t)(ns::BEHAVIOR_MASK) )
        {
        case (uint32_t)(ns::BEHAVIOR_HAL):          os += "HAL"; break;
        case (uint32_t)(ns::BEHAVIOR_APP):          os += "APP"; break;
        default:                                    os += "?  "; break;
        }
    }
    os += "-";
    {
        switch ( (uint32_t)(o) & (uint32_t)(ns::BUFFER_SOURCE_MASK) )
        {
        case (uint32_t)(ns::BUFFER_SOURCE_USER):    os += "USER    "; break;
        case (uint32_t)(ns::BUFFER_SOURCE_POOL):    os += "POOL    "; break;
        case (uint32_t)(ns::BUFFER_SOURCE_RUNTIME): os += "RUNTIME "; break;
        case (uint32_t)(ns::BUFFER_SOURCE_PROVIDER):os += "PROVIDER"; break;
        default:                                    os += "?       "; break;
        }
    }
    os += "]";

    return os;
}


/******************************************************************************
 *
 ******************************************************************************/
using StreamId_T = NSCam::v3::StreamId_T;
using StreamSet = NSCam::v3::pipeline::Set<StreamId_T>;
using NodeSet = NSCam::v3::pipeline::NodeSet;


/******************************************************************************
 *
 ******************************************************************************/
struct NodeEdge
{
    NodeId_T        src;
    NodeId_T        dst;
};


static inline bool operator==(NodeEdge const& lhs, NodeEdge const& rhs) {
    return lhs.src == rhs.src && lhs.dst == rhs.dst;
}


/******************************************************************************
 *
 ******************************************************************************/
struct NodeEdgeSet
    : public Set<NodeEdge>
{
    auto        addEdge(NodeId_T const src, NodeId_T const dst) -> NodeEdgeSet&
                {
                    Set<NodeEdge>::add(NodeEdge{src, dst});
                    return *this;
                }
};


static inline android::String8 toString(const NSCam::v3::pipeline::NSPipelineContext::NodeEdgeSet& o)
{
    android::String8 os;
    os += "{ ";
    for (size_t i = 0; i < o.size(); i++) {
        auto const& v = o[i];
        os += android::String8::format("(%#" PRIxPTR " -> %#" PRIxPTR ") ", v.src, v.dst);
    }
    os += "}";
    return os;
}


/******************************************************************************
 *
 ******************************************************************************/
static inline android::String8 toString(const NSCam::v3::pipeline::NSPipelineContext::NodeSet& o)
{
    android::String8 os;
    os += "{ ";
    for (size_t i = 0; i < o.size(); i++) {
        auto const& v = o[i];
        os += android::String8::format("%#" PRIxPTR " ", v);
    }
    os += "}";
    return os;
}


/******************************************************************************
 *
 ******************************************************************************/
struct IOMap
{
    StreamSet       vIn;
    StreamSet       vOut;
    //
    auto            isEmpty() const -> bool     { return vIn.size() == 0 && vOut.size() == 0; }
    auto            sizeIn() const -> size_t    { return vIn.size(); }
    auto            sizeOut() const -> size_t   { return vOut.size(); }

    auto            addIn(StreamId_T const stream) -> IOMap&
                    {
                        vIn.add(stream);
                        return *this;
                    }

    auto            addOut(StreamId_T const stream) -> IOMap&
                    {
                        vOut.add(stream);
                        return *this;
                    }

    auto            addIn(StreamSet const& stream) -> IOMap&
                    {
                        vIn.add(stream);
                        return *this;
                    }

    auto            addOut(StreamSet const& stream) -> IOMap&
                    {
                        vOut.add(stream);
                        return *this;
                    }

};


static inline android::String8 toString(const NSCam::v3::pipeline::NSPipelineContext::IOMap& o)
{
    android::String8 os;
    auto const& iomap = o;
    os += "( ";
    for (size_t j = 0; j < iomap.vIn.size(); j++) {
        auto const& streamId = iomap.vIn[j];
        os += android::String8::format("%#" PRIx64 " ", streamId);
    }
    os += "-> ";
    for (size_t j = 0; j < iomap.vOut.size(); j++) {
        auto const& streamId = iomap.vOut[j];
        os += android::String8::format("%#" PRIx64 " ", streamId);
    }
    os += ")";
    return os;
}


/******************************************************************************
 *
 ******************************************************************************/
struct IOMapSet
    : public android::Vector<IOMap>
{
    auto            add(IOMap const& map) -> IOMapSet&
                    {
                        this->push_back(map);
                        return *this;
                    }

    static auto     empty() -> const IOMapSet
                    {
                        return IOMapSet();
                    }

};


static inline android::String8 toString(const NSCam::v3::pipeline::NSPipelineContext::IOMapSet& o)
{
    android::String8 os;
    os += "{ ";
    for (size_t i = 0; i < o.size(); i++) {
        auto const& iomap = o[i];
        os += toString(iomap);
    }
    os += " }";
    return os;
}


/******************************************************************************
 *
 ******************************************************************************/
struct TrackFrameResultParams
{
    /**
     * The image stream Ids which are needed to be kept track of.
     */
    std::unordered_set<StreamId_T>  imageStreams;

    /**
     * The meta stream Ids which are needed to be kept track of.
     */
    std::unordered_set<StreamId_T>  metaStreams;

    /**
     * true indicates the result of this frame is allowed to be recycled when needed.
     * For example, this result will be recycled if no empty stream buffers are
     * available for (preview) streaming.
     */
    bool                            isRecyclable = true;

};


static inline std::string toString(TrackFrameResultParams const& o)
{
    std::ostringstream oss;
    oss << "{";

    oss << std::hex;
    oss << " .image=[";
    for (auto const& s : o.imageStreams) { oss << " " << s; }
    oss << " ]";
    oss << " .meta=[";
    for (auto const& s : o.metaStreams) { oss << " " << s; }
    oss << " ]";
    oss << std::dec;

    oss << " .isRecyclable=" << std::to_string(o.isRecyclable);
    oss << " }";
    return oss.str();
}


/******************************************************************************
 *
 ******************************************************************************/
};  //namespace NSCam::v3::pipeline::NSPipelineContext
#endif  //_MTK_HARDWARE_MTKCAM3_INCLUDE_MTKCAM3_PIPELINE_PIPELINE_TYPES_H_

