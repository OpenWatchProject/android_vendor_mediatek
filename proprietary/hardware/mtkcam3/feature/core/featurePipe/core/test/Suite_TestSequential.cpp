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

#include <gtest/gtest.h>
#include <utils/RefBase.h>
#include <vector>
#include <map>
#include <featurePipe/core/include/SeqUtil.h>

namespace NSCam{
namespace NSCamFeature{
namespace NSFeaturePipe{

class Data
{
public:
  Data()
  {
    mID = 0;
    mVal = 0;
  }

  Data(unsigned id, int val)
  {
    mID = id;
    mVal = val;
  }

  unsigned mID;
  int mVal;

  class IndexConverter
  {
  public:
    unsigned operator()(const Data &data) const
    {
        return data.mID;
    }
  };

  static NSCam::Utils::ULog::RequestTypeId getULogRequestTypeId()
  {
    return NSCam::Utils::ULog::REQ_STR_FPIPE_REQUEST;
  }

  NSCam::Utils::ULog::RequestSerial getULogRequestSerial() const
  {
    return mID;
  }
};

TEST(SequentialQueue, Basic)
{
  Data val;
  SequentialQueue<Data, Data::IndexConverter> queue;

  queue.enque(Data(0, 0));
  EXPECT_TRUE(queue.deque(val));
  EXPECT_EQ(0u, val.mID);

  queue.enque(Data(2, 2));
  EXPECT_FALSE(queue.deque(val));

  queue.enque(Data(3, 3));
  EXPECT_FALSE(queue.deque(val));

  queue.enque(Data(1, 1));
  EXPECT_TRUE(queue.deque(val));
  EXPECT_EQ(1u, val.mID);

  EXPECT_TRUE(queue.deque(val));
  EXPECT_EQ(2u, val.mID);

  EXPECT_TRUE(queue.deque(val));
  EXPECT_EQ(3u, val.mID);

  EXPECT_FALSE(queue.deque(val));
}

class Handler : virtual public CamNodeULogHandler
{
public:
  typedef unsigned DataID;
  virtual ~Handler(){}
  virtual MBOOL onData(int id, const Data& data)
  {
    map[id].push_back(data.mID);
    return MTRUE;
  }
  bool isSorted(int id)
  {
    return std::is_sorted(map[id].begin(), map[id].end());
  }

  size_t size(int id)
  {
    return map[id].size();
  }

  static const char* ID2Name(DataID id)
  {
#define MAKE_NAME_CASE(name) \
    case name: return #name;

    switch(id)
    {
    MAKE_NAME_CASE(0);
    MAKE_NAME_CASE(1);
    };
    return "UNKNOWN";
#undef MAKE_NAME_CASE
  }

  static unsigned getSeq(const Data& data)
  {
    return data.mID;
  }

  static const bool supportSeq = true;

private:
  std::map<int, std::vector<unsigned>> map;
};

typedef SequentialHandler<Handler> SeqDataHandler;

TEST(SequentialHandler, Order)
{
  int id_0 = 0;
  int id_1 = 1;
  Handler handler;
  SeqDataHandler seqhandler;

  for(unsigned i=0;i<100;i++)
  {
    EXPECT_TRUE(seqhandler.onData(id_0, Data(i, i), &handler));
    EXPECT_TRUE(handler.isSorted(id_0));
    EXPECT_EQ(i, handler.size(id_0)-1);

    EXPECT_TRUE(seqhandler.onData(id_1, Data(i, i), &handler));
    EXPECT_TRUE(handler.isSorted(id_1));
    EXPECT_EQ(i, handler.size(id_1)-1);
  }
}

TEST(SequentialHandler, Unorder)
{
  int id_0 = 0;
  int id_1 = 1;
  int id_0_data[] = {2, 1, 0, 3, 4, 6, 5};
  unsigned id_0_size[] = {0, 0, 3, 4, 5, 5, 7};
  int id_1_data[] = {0, 1, 3, 2, 6, 5, 4};
  unsigned id_1_size[] = {1, 2, 2, 4, 4, 4, 7};
  const int test_len = sizeof(id_0_data)/sizeof(id_0_data[0]);

  Handler handler;
  SeqDataHandler seqhandler;

  for(unsigned i=0;i<test_len;++i)
  {
    EXPECT_TRUE(seqhandler.onData(id_0, Data(id_0_data[i], id_0_data[i]), &handler));
    EXPECT_TRUE(handler.isSorted(id_0));
    EXPECT_EQ(id_0_size[i], handler.size(id_0));

    EXPECT_TRUE(seqhandler.onData(id_1, Data(id_1_data[i], id_1_data[i]), &handler));
    EXPECT_TRUE(handler.isSorted(id_1));
    EXPECT_EQ(id_1_size[i], handler.size(id_1));
  }
}

TEST(SequentialHandler, StartSeq)
{
  int id_0 = 0;
  int id_1 = 1;
  const int start_seq = 2;
  int id_0_data[] = {2, 3, 4, 6, 5};
  unsigned id_0_size[] = {1, 2, 3, 3, 5};
  int id_1_data[] = {3, 2, 6, 5, 4};
  unsigned id_1_size[] = {0, 2, 2, 2, 5};
  const int test_len = sizeof(id_0_data)/sizeof(id_0_data[0]);

  Handler handler;
  SeqDataHandler seqhandler(start_seq);

  for(unsigned i=0;i<test_len;++i)
  {
    EXPECT_TRUE(seqhandler.onData(id_0, Data(id_0_data[i], id_0_data[i]), &handler));
    EXPECT_TRUE(handler.isSorted(id_0));
    EXPECT_EQ(id_0_size[i], handler.size(id_0));

    EXPECT_TRUE(seqhandler.onData(id_1, Data(id_1_data[i], id_1_data[i]), &handler));
    EXPECT_TRUE(handler.isSorted(id_1));
    EXPECT_EQ(id_1_size[i], handler.size(id_1));
  }
}

} // namespace NSFeaturePipe
} // namespace NSCamFeature
} // namespace NSCam
