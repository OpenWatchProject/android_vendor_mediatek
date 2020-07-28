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

#ifndef _MTK_CAMERA_STREAMING_FEATURE_PIPE_TPIO_H_
#define _MTK_CAMERA_STREAMING_FEATURE_PIPE_TPIO_H_

#include "inc/tpi_session.h"
#include <list>
#include <map>
#include <set>

namespace NSCam {
namespace NSCamFeature {
namespace NSFeaturePipe {

enum class TPIOEntry { UNKNOWN, YUV, RAW, ASYNC, DISP, META };
enum class TPIOUse
{
    IN_YUV,
    IN_DEPTH,
    IN_PURE,
    OUT_DISPLAY,
    OUT_RECORD,
    INPLACE,
    DIV,
    INPUT_CROP,
    SHARED_INPUT,
};

typedef std::map<TPI_BUFFER_ID, std::list<TPI_Port>> TPI_PortMap;;

class TPIO
{
public:
  TPIO();
  TPIO(const TPI_NodeInfo &node);
  void addPrev(const TPI_PathInfo &path);
  void addNext(const TPI_PathInfo &path);
public:
  unsigned mNodeID = TPI_NODE_ID_UNKNOWN;
  TPI_NodeInfo mNodeInfo;
  MUINT32 mTravelCount = 0;
  TPI_PortMap mPrevPortMap;
  TPI_PortMap mNextPortMap;
};

typedef std::map<TPI_NODE_ID, TPIO> TPIOMap;
typedef std::set<TPIOUse> TPIOUseSet;

class TPIOGroup
{
public:
  bool mEnable = false;
  TPIO mEntry;
  std::vector<TPIO> mNodes;
  TPIOUseSet mUseSet;
  EImageFormat mFormat = eImgFmt_UNKNOWN;
  MSize mSize = MSize(0,0);
  MUINT32 mNumInBuffer = 3;
  MUINT32 mNumOutBuffer = 3;
  MUINT32 mTotalQueueCount = 0;
};

typedef std::map<TPIOEntry, TPIOGroup> TPIOGroupMap;

class TPIOFrame
{
public:
  void setID(unsigned frameID) { mFrameID = frameID; }
  void add(TPIOEntry entry, unsigned id)
  {
    mNeedEntry.insert(entry);
    mNeedNode.insert(id);
  }
  bool needEntry(TPIOEntry entry) const { return mNeedEntry.count(entry); }
  bool needNode(unsigned id) const { return mNeedNode.count(id); }

private:
  unsigned mFrameID = 0;
  std::set<TPIOEntry> mNeedEntry;
  std::set<unsigned> mNeedNode;
};

} // namespace NSFeaturePipe
} // namespace NSCamFeature
} // namespace NSCam

#endif // _MTK_CAMERA_STREAMING_FEATURE_PIPE_TPIO_H_
