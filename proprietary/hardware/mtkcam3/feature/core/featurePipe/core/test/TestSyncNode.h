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

#ifndef _TEST_SYNC_NODE_H_
#define _TEST_SYNC_NODE_H_

#include "TestExtThread.h"
#include <featurePipe/core/include/CamThreadNode.h>

namespace NSCam {
namespace NSCamFeature {
namespace NSFeaturePipe {

enum TestSyncDataID {
  ID_INVALID,
  ID_ROOT_ENQUE,
  ID_A2B,
  ID_B_OUT,
  ID_NEXT,
};

class TestSyncDataHandler
{
public:
  typedef TestSyncDataID DataID;
public:
  virtual ~TestSyncDataHandler();
  virtual MBOOL onData(DataID, MUINT32 num) { return MFALSE; }
  static const char* ID2Name(DataID id);
  static const bool supportSeq = false;
};

class TestSyncNodeUsage
{
public:
  TestSyncNodeUsage();
  TestSyncNodeUsage(long min, long max, MBOOL ext, long extMin, long extMax);

public:
  long  mMin = 0;
  long  mMax = 0;
  MBOOL mExt = MFALSE;
  long  mExtMin = 0;
  long  mExtMax = 0;
};

class TestSyncNode : public TestSyncDataHandler, public CamThreadNode<TestSyncDataHandler>
{
public:
  typedef CamGraph<TestSyncNode> Graph_T;
  typedef TestSyncDataHandler Handler_T;

public:
  TestSyncNode(const char *name);
  virtual ~TestSyncNode();

  virtual MBOOL onData(DataID id, MUINT32 num);

public:
  MVOID configUsage(const TestSyncNodeUsage &usage);

protected:  // ICamThreadNode members
  virtual MBOOL onInit();
  virtual MBOOL onUninit();
  virtual MBOOL onThreadLoop();
  virtual MBOOL onThreadStart();
  virtual MBOOL onThreadStop();

private:
  WaitQueue<MUINT32> mRequests;

private:
  TestSyncNodeUsage mUsage;

  class EnqueData
  {
  public:
      EnqueData(TestSyncNode *node, MUINT32 data);

      TestSyncNode *mNode;
      MUINT32 mData;
  };

  static void onEnqueCB(const EnqueData &data);
  TestExtThread<EnqueData> mExtThread;

}; // class TestSyncNode

} // namespace NSFeaturePipe
} // namespace NSCamFeature
} // namespace NSCam

#endif // _TEST_SYNC_NODE_H_
