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

#include <gtest/gtest.h>
#include <featurePipe/core/include/WaitQueue.h>
#include <iostream>

#define TEST_QUEUE_COUNT 10

namespace NSCam{
namespace NSCamFeature{
namespace NSFeaturePipe{

TEST(WaitHub, BasicSignal)
{
  WaitHub hub("testHub");
  WaitQueue<int> queue;
  unsigned signal;

  hub.addWaitQueue(&queue);
  queue.enque(123);
}

#define TEST_ENQUE_COUNT 10
TEST(WaitQueue, Basic)
{
  WaitQueue<int> queue;
  int i, val;

  EXPECT_FALSE(queue.deque(val));
  EXPECT_FALSE(queue.isReady());

  queue.setWaitHub(NULL);

  for( i = 0; i < TEST_ENQUE_COUNT; ++i )
  {
    queue.enque(i);
    EXPECT_TRUE(queue.isReady());
    EXPECT_TRUE(queue.deque(val));
    EXPECT_FALSE(queue.isReady());
    EXPECT_EQ(val, i);
  }

  for( i = 0; i < 2*TEST_ENQUE_COUNT; ++i )
  {
    queue.enque(i);
  }
  EXPECT_EQ(queue.size(), (unsigned)2*TEST_ENQUE_COUNT);
  for( i = 0; i < TEST_ENQUE_COUNT; ++i )
  {
    EXPECT_TRUE(queue.deque(val));
  }
  EXPECT_TRUE(queue.isReady());
  EXPECT_EQ(queue.size(), (unsigned)TEST_ENQUE_COUNT);
  for( i = 0; i < TEST_ENQUE_COUNT; ++i )
  {
    queue.enque(i);
  }
  EXPECT_EQ(queue.size(), (unsigned)2*TEST_ENQUE_COUNT);
  for( i = 0; i < 2*TEST_ENQUE_COUNT; ++i )
  {
    EXPECT_TRUE(queue.deque(val));
  }
  EXPECT_FALSE(queue.isReady());
  EXPECT_EQ(queue.size(), (unsigned)0);
}

TEST(WaitHub, AddWaitQueue)
{
  WaitHub hub("TestHub");
  WaitQueue<int> queue[TEST_QUEUE_COUNT];
  int i;

  hub.addWaitQueue(NULL);
  for( i = 0; i < TEST_QUEUE_COUNT; ++i )
  {
    hub.addWaitQueue(&queue[i]);
  }

  hub.addWaitQueue(NULL);
  for( i = 0; i < TEST_QUEUE_COUNT; ++i )
  {
    hub.addWaitQueue(&queue[i]);
  }
}

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
    IWaitQueue::Index operator()(const Data &data)
    {
      return IWaitQueue::Index(data.mID, data.mID);
    }
    static unsigned getID(const Data &data)
    {
      return data.mID;
    }
  };

};

TEST(WaitQueue, Priority)
{
  PriorityWaitQueue<Data, Data::IndexConverter> queue;
  Data val, data[5];

  for( unsigned i = 0; i < 5; ++i )
  {
    data[i] = Data(i, i*i + i );
  }

  EXPECT_FALSE(queue.deque(val));
  queue.enque(data[2]);
  queue.enque(data[4]);
  queue.enque(data[0]);
  queue.enque(data[3]);
  queue.enque(data[1]);
  EXPECT_EQ(queue.size(), (unsigned)5);

  for( unsigned i = 0; i < 5; ++i )
  {
    EXPECT_TRUE(queue.deque(val));
    EXPECT_EQ(val.mID, data[i].mID);
    EXPECT_EQ(val.mVal, data[i].mVal);
  }

  EXPECT_EQ(queue.size(), (unsigned)0);
  EXPECT_FALSE(queue.deque(val));

  queue.enque(data[2]);
  queue.enque(data[4]);
  queue.enque(data[0]);
  queue.enque(data[3]);
  queue.enque(data[1]);
  EXPECT_FALSE(queue.deque(30, val));
  unsigned order[] = { 3, 4, 1, 0, 2 };
  for( unsigned i = 0; i < 5; ++i )
  {
    EXPECT_TRUE(queue.deque(order[i], val) );
    EXPECT_EQ(val.mID, order[i]);
    EXPECT_EQ(val.mVal, data[order[i]].mVal);
  }
}

TEST(WaitHub, Priority)
{
  unsigned id;
  Data val1, val2;
  PriorityWaitQueue<Data, Data::IndexConverter> queue1;
  PriorityWaitQueue<Data, Data::IndexConverter> queue2;
  WaitHub hub("priority hub");

  hub.addWaitQueue(&queue1);
  hub.addWaitQueue(&queue2);
  queue1.enque(Data(10, 27));
  queue2.enque(Data(10, 33));
  EXPECT_TRUE(hub.waitAllQueueSync(id));
  EXPECT_EQ(id, (unsigned)10);
  EXPECT_TRUE(queue1.deque(id, val1));
  EXPECT_TRUE(queue2.deque(id, val2));
  EXPECT_EQ(id, val1.mID);
  EXPECT_EQ(id, val2.mID);

  queue1.enque(Data(30, 123));
  queue1.enque(Data(7, 11));
  queue2.enque(Data(30, 99));
  EXPECT_TRUE(hub.waitAllQueueSync(id));
  EXPECT_EQ(id, (unsigned)30);

  queue2.enque(Data(70, 1));
  EXPECT_TRUE(hub.waitAllQueueSync(id));
  EXPECT_EQ(id, (unsigned)30);
  queue2.enque(Data(7, 9));
  EXPECT_TRUE(hub.waitAllQueueSync(id));
  EXPECT_EQ(id, (unsigned)7);
}

} // namespace NSFeaturePipe
} // namespace NSCamFeature
} // namespace NSCam
