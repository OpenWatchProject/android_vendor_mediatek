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

#ifndef _TEST_NODE_H_
#define _TEST_NODE_H_

#include "TestRequest.h"
#include <featurePipe/core/include/CamThreadNode.h>
#include <featurePipe/core/include/SeqUtil.h>

namespace NSCam {
namespace NSCamFeature {
namespace NSFeaturePipe {

enum TestDataID {
  ID_INVALID,
  ROOT_ENQUE,
  A2B,
  A2C,
  B2D,
  C2D,
  B_OUT,
  D_OUT,
};

class TestDataHandler : virtual public CamNodeULogHandler
{
public:
  typedef TestDataID DataID;
  //typedef TestSeqConverter SeqConverter;
public:
  virtual ~TestDataHandler();
  virtual MBOOL onData(DataID, const TestRequestPtr &) { return MFALSE; }
  virtual MBOOL onData(DataID, const TestBuffer &)     { return MFALSE; }
  static const char* ID2Name(DataID id);

  template<typename T>
  static unsigned getSeq(const T &data)
  {
    return data.mRequest->mID;
  }

  static unsigned getSeq(const TestRequestPtr &data)
  {
    return data->mID;
  }

  static const bool supportSeq = true;
};

class TestNode : public TestDataHandler, public CamThreadNode<TestDataHandler>
{
public:
  typedef CamGraph<TestNode> Graph_T;
  typedef TestDataHandler Handler_T;

public:
  TestNode(const char *name, Graph_T *graph);
  virtual ~TestNode();

  MVOID setDelayNS(long ns);
  long getDelayNS() const;

protected:
  void simulateDelay();

protected:
  virtual MBOOL onInit() = 0;
  virtual MBOOL onUninit() = 0;
  virtual MBOOL onThreadLoop() = 0;
  virtual MBOOL onThreadStart()    { return MTRUE; }
  virtual MBOOL onThreadStop()     { return MTRUE; }

private:
  long mDelayNS;
};

} // namespace NSFeaturePipe
} // namespace NSCamFeature
} // namespace NSCam

#endif // _TEST_NODE_H_
