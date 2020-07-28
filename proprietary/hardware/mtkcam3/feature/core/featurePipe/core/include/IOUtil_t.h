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
 * MediaTek Inc. (C) 2017. All rights reserved.
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

#ifndef _MTK_CAMERA_FEATURE_PIPE_CORE_IO_UTIL_T_H_
#define _MTK_CAMERA_FEATURE_PIPE_CORE_IO_UTIL_T_H_

#include <set>
#include <map>
#include <vector>
#include <unordered_map>
#include <unordered_set>

#include "MtkHeader.h"
#include "BufferPool.h"

namespace NSCam {
namespace NSCamFeature {
namespace NSFeaturePipe {

enum StreamType {
  STREAMTYPE_PREVIEW,
  STREAMTYPE_PREVIEW_CALLBACK,
  STREAMTYPE_RECORD,
  STREAMTYPE_FD,
  STREAMTYPE_PHYSICAL,
  STREAMTYPE_ASYNC,

  STREAMTYPE_ENUM_COUNT,
};

enum OutputType {
  OUTPUT_INVALID,

  OUTPUT_STREAM_PREVIEW,
  OUTPUT_STREAM_PREVIEW_CALLBACK,
  OUTPUT_STREAM_RECORD,
  OUTPUT_STREAM_FD,
  OUTPUT_STREAM_PHYSICAL,
  OUTPUT_STREAM_ASYNC,

  OUTPUT_FULL,
  OUTPUT_NEXT_FULL,

  OUTPUT_ENUM_COUNT,
};

enum IOPolicyType {
  IOPOLICY_BYPASS,
  IOPOLICY_INOUT,
  IOPOLICY_LOOPBACK,
  IOPOLICY_INOUT_EXCLUSIVE,
  IOPOLICY_INOUT_NEXT,
  IOPOLICY_INOUT_QUEUE,
  IOPOLICY_INPLACE,

  IOPOLICY_ENUM_COUNT,
};

struct NextFullInfo
{
public:
    MSize mResize = MSize(0,0);
    MBOOL mNeedCrop = MFALSE;
    MBOOL mCanShare = MFALSE;
};

template <typename Node_T, typename ReqInfo_T>
class IORequest;

template <typename Node_T, typename ReqInfo_T>
class IOControl
{
public:
  using IORequest = IORequest<Node_T, ReqInfo_T>;

public:
  struct PolicyInfo
  {
  public:
      PolicyInfo() {}
      PolicyInfo(Node_T *node, IOPolicyType policy)
        : mNode(node), mPolicy(policy) {}

  public:
      Node_T *mNode = NULL;
      IOPolicyType mPolicy = IOPOLICY_BYPASS;
  };

  struct NextInfo
  {
  public:
    NextInfo() {}
    NextInfo(const Node_T *node);
  public:
    void addType(const OutputType &type);
    void setNext(Node_T *next);
    bool needType(const OutputType &type) const;
    Node_T* getNext() const;
    void print() const;
  private:
    const Node_T *mNode = NULL;
    Node_T *mNext = NULL;
    std::set<OutputType> mOutput;
  };

  struct BufferInfo
  {
  public:
    BufferInfo() {}
    BufferInfo(const android::sp<IIBuffer> &buffer, const NextFullInfo &info);
    MVOID get(android::sp<IIBuffer> &buffer, NextFullInfo &info) const;
    MVOID pop(android::sp<IIBuffer> &buffer, NextFullInfo &info);

  public:
    android::sp<IIBuffer> mBuffer = NULL;
    NextFullInfo mNextFullInfo;
  };

public:
  typedef typename std::list<Node_T*> NodeList;

  typedef std::set<StreamType> StreamSet;
  typedef std::map<StreamType, NodeList> PathMap;

  typedef std::map<const Node_T*, NextInfo> NextTable;
  typedef std::map<const Node_T*, BufferInfo> BufferTable;

  typedef std::list<PolicyInfo> PolicyInfoList;

  IOControl();
  ~IOControl();

  MVOID setRoot(Node_T *root);
  MVOID addStream(const StreamType &stream, const NodeList &nodes);
  MBOOL prepareMap(const StreamSet &streams, const ReqInfo_T &reqInfo, IORequest &req);
  MVOID dump(const IORequest &req) const;

private:
  MVOID dumpInfo(const NextTable &nextTable) const;
  MVOID dumpInfo(const BufferTable &bufferTable) const;

  MBOOL prepareStreamMap(const StreamType &stream, const ReqInfo_T &reqInfo, IORequest &req);
  MBOOL prepareGeneralMap(const StreamType &stream, const ReqInfo_T &reqInfo, const PolicyInfoList &policys, NextTable &nextTable);
    MBOOL prepareAsyncNext(const PolicyInfoList &policys, Node_T* &asyncNext);
  MVOID allocNextBuf(const ReqInfo_T &reqInfo, NextTable &nextTable, BufferTable &bufferTable) const;
  MVOID allocAsyncBuf(const ReqInfo_T &reqInfo, Node_T *asyncNext, BufferInfo &asyncBuf) const;
  PolicyInfoList queryPathPolicy(const StreamType &stream, const ReqInfo_T &reqInfo) const;

  MBOOL forwardCheck(const PolicyInfoList &policyInfos) const;
  MVOID backwardCalc(const StreamType &stream, const ReqInfo_T &reqInfo, const PolicyInfoList &policyInfos, NextTable &nextTable) const;

  Node_T                     *mRoot = NULL;
  PathMap                     mStreamPathMap;
};

template <typename Node_T, typename ReqInfo_T>
class IORequest
{
public:
  friend class IOControl<Node_T, ReqInfo_T>;
  using IOControl = IOControl<Node_T, ReqInfo_T>;
  using NextTable = typename IOControl::NextTable;
  using BufferTable = typename IOControl::BufferTable;
  using BufferInfo = typename IOControl::BufferInfo;

  MBOOL isFullBufHold() const;
  MBOOL needPreview(const Node_T *node) const;
  MBOOL needPreviewCallback(const Node_T *node) const;
  MBOOL needRecord(const Node_T *node) const;
  MBOOL needFD(const Node_T *node) const;
  MBOOL needPhysicalOut(const Node_T *node) const;
  MBOOL needFull(const Node_T *node) const;
  MBOOL needNextFull(const Node_T *node) const;
  //MBOOL needDualFull(const Node_T *node) const;
  MBOOL needAsync(const Node_T *node) const;
  android::sp<IIBuffer> getNextFullImg(const Node_T *node, NextFullInfo &info);
  MBOOL getNextFullImgInfo(const Node_T *node, NextFullInfo &info) const;
  android::sp<IIBuffer> getAsyncImg(const Node_T *node, NextFullInfo &info);

  MBOOL needOutputType(const Node_T *node, const OutputType &type) const;

private:
  NextTable mNextTable;
  BufferTable mBufferTable;

  Node_T *mAsyncNext = NULL;
  BufferInfo mAsyncBuffer;
};

} // namespace NSFeaturePipe
} // namespace NSCamFeature
} // namespace NSCam

#endif // _MTK_CAMERA_FEATURE_PIPE_CORE_IO_UTIL_T_H_
