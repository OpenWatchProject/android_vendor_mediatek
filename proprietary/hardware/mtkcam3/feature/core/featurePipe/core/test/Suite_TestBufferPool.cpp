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
#include "TestBufferPool.h"
#include "TestTool.h"

namespace NSCam{
namespace NSCamFeature{
namespace NSFeaturePipe{

#define ALLOC_COUNT 10
TEST(TestBufferPool, Alloc)
{
  android::sp<TestBufferPool> pool;
  SmartTestBuffer buffer[ALLOC_COUNT];
  unsigned i, available, total;

  total = 0;
  available = 0;

  pool = TestBufferPool::create(256);
  EXPECT_TRUE(pool != NULL);

  for( i = 0; i < ALLOC_COUNT/2; ++i )
  {
    ++total;
    ++available;
    EXPECT_TRUE(pool->allocate());
    EXPECT_EQ(pool->peakPoolSize(), total);
    EXPECT_EQ(pool->peakAvailableSize(), available);
  }

  buffer[0] = pool->request();
  --available;
  EXPECT_TRUE(buffer[0] != NULL);
  EXPECT_EQ(pool->peakPoolSize(), total);
  EXPECT_EQ(pool->peakAvailableSize(), available);

  buffer[0] = NULL;
  ++available;
  EXPECT_TRUE(buffer[0] == NULL);
  EXPECT_EQ(pool->peakPoolSize(), total);
  EXPECT_EQ(pool->peakAvailableSize(), available);

  for( i = 0; i < ALLOC_COUNT/2; ++i )
  {
    buffer[i] = pool->request();
    --available;
    EXPECT_TRUE(buffer[i] != NULL);
    EXPECT_EQ(pool->peakPoolSize(), total);
    EXPECT_EQ(pool->peakAvailableSize(), available);
  }

  EXPECT_EQ(pool->allocate(2), (unsigned)2);
  total+=2;
  available+=2;

  while( total < ALLOC_COUNT )
  {
    ++total;
    ++available;
    EXPECT_TRUE(pool->allocate());
  }
  EXPECT_EQ(pool->peakPoolSize(), total);
  EXPECT_EQ(pool->peakAvailableSize(), available);

  for( i = ALLOC_COUNT/2; i < total; ++i )
  {
    buffer[i] = pool->request();
    --available;
    EXPECT_TRUE(buffer[i] != NULL);
  }
  EXPECT_EQ(pool->peakPoolSize(), total);
  EXPECT_EQ(pool->peakAvailableSize(), available);

  for( i = 0; i < ALLOC_COUNT; ++i )
  {
    buffer[i] = NULL;
    ++available;
  }
  EXPECT_EQ(pool->peakAvailableSize(), available);
  TestBufferPool::destroy(pool);

  pool = TestBufferPool::create(256);
  EXPECT_TRUE(pool != NULL);
  EXPECT_EQ(pool->peakPoolSize(), (unsigned)0);
  EXPECT_EQ(pool->peakAvailableSize(), (unsigned)0);
  EXPECT_EQ(pool->allocate(3), (unsigned)3);
  EXPECT_EQ(pool->peakPoolSize(), (unsigned)3);
  EXPECT_EQ(pool->peakAvailableSize(), (unsigned)3);
  TestBufferPool::destroy(pool);
}

#define UNIQUE_COUNT 10
TEST(BufferPool, Unique)
{
  android::sp<TestBufferPool> pool;
  SmartTestBuffer buffer[UNIQUE_COUNT];
  TestBufferPool::CONTAINER_TYPE poolContents;
  int i, j;
  bool found;

  pool = TestBufferPool::create(256);
  EXPECT_TRUE(pool != NULL);
  for( i = 0; i < UNIQUE_COUNT; ++i )
  {
    EXPECT_TRUE(pool->allocate());
  }
  ASSERT_EQ(pool->peakAvailableSize(), (unsigned)UNIQUE_COUNT);
  for( i = 0; i < UNIQUE_COUNT; ++i )
  {
    buffer[i] = pool->request();
    EXPECT_TRUE(buffer[i] != NULL);
    for( j = 0; j < i; ++j )
    {
      EXPECT_TRUE(buffer[i]->mBuffer != buffer[j]->mBuffer);
    }
  }

  poolContents = pool->getPoolContents();
  EXPECT_EQ(poolContents.size(), (unsigned)10);

  for( i = 0; i < UNIQUE_COUNT; ++i )
  {
    found = false;
    for( j = 0; j < UNIQUE_COUNT; ++j )
    {
      if( buffer[i] == poolContents[j] )
      {
        found = true;
        break;
      }
    }
    EXPECT_TRUE(found);
  }
  TestBufferPool::destroy(pool);
}

#define EXT_COUNT 10
TEST(BufferPool, Add)
{
  SmartTestBuffer buffer[UNIQUE_COUNT];
  SmartTestBuffer extBuffers[10];
  android::sp<TestBufferPool> pool, extPool;
  TestBufferPool::CONTAINER_TYPE poolContents;
  int i, j;
  bool found;

  pool = TestBufferPool::create(256);
  extPool = TestBufferPool::create(256);
  EXPECT_TRUE(pool != NULL);
  EXPECT_TRUE(extPool != NULL);

  ASSERT_EQ(extPool->allocate(EXT_COUNT), (unsigned)EXT_COUNT);
  for( i = 0; i < EXT_COUNT; ++i )
  {
    extBuffers[i] = extPool->request();
    EXPECT_TRUE(extBuffers[i] != NULL);
  }

  for( i = 0; i < EXT_COUNT; ++i )
  {
    pool->add(extBuffers[i]->mBuffer);
  }
  ASSERT_EQ(pool->peakAvailableSize(), (unsigned)EXT_COUNT);

  for( i = 0; i < EXT_COUNT; ++i )
  {
    buffer[i] = pool->request();
    EXPECT_TRUE(buffer[i] != NULL);
  }

  for( i = 0; i < EXT_COUNT; ++i )
  {
    found = false;
    for( j = 0; j < EXT_COUNT; ++j )
    {
      if( buffer[i]->mBuffer == extBuffers[j]->mBuffer )
      {
        found = true;
        break;
      }
    }
    EXPECT_TRUE(found);
  }
  TestBufferPool::destroy(pool);
  TestBufferPool::destroy(extPool);
}

#define AUTO_ALLOCATE_COUNT 10
#define AUTO_FREE_COUNT 3
TEST(BufferPool, Auto)
{
  SmartTestBuffer buffer[AUTO_ALLOCATE_COUNT];
  android::sp<TestBufferPool> pool;
  int i;

  // test auto allocate check available buffer first
  pool = TestBufferPool::create(256);
  pool->setAutoAllocate(AUTO_ALLOCATE_COUNT);
  EXPECT_EQ(pool->peakPoolSize(), (unsigned)0);
  EXPECT_EQ(pool->peakAvailableSize(), (unsigned)0);
  for( i = 0; i < AUTO_ALLOCATE_COUNT; ++i )
  {
    SmartTestBuffer temp;
    temp = pool->request();
  }
  // since temp always get returned
  // there should only be one buffer auto allocated
  EXPECT_EQ(pool->peakPoolSize(), (unsigned)1);
  EXPECT_EQ(pool->peakAvailableSize(), (unsigned)1);

  // test auto allocate work up to max
  for( i = 0; i < AUTO_ALLOCATE_COUNT; ++i )
  {
    buffer[i] = pool->request();
  }
  EXPECT_EQ(pool->peakPoolSize(), (unsigned)AUTO_ALLOCATE_COUNT);
  EXPECT_EQ(pool->peakAvailableSize(), (unsigned)0);
  for( i = 0; i < AUTO_ALLOCATE_COUNT; ++i )
  {
    buffer[i] = NULL;
  }
  EXPECT_EQ(pool->peakPoolSize(), (unsigned)AUTO_ALLOCATE_COUNT);
  EXPECT_EQ(pool->peakAvailableSize(), (unsigned)AUTO_ALLOCATE_COUNT);
  // test auto free work
  pool->setAutoFree(1);
  EXPECT_EQ(pool->peakPoolSize(), (unsigned)1);
  EXPECT_EQ(pool->peakAvailableSize(), (unsigned)1);

  // test auto free work per buffer release
  for( i = 0; i < AUTO_ALLOCATE_COUNT; ++i )
  {
    buffer[i] = pool->request();
  }
  EXPECT_EQ(pool->peakPoolSize(), (unsigned)AUTO_ALLOCATE_COUNT);
  EXPECT_EQ(pool->peakAvailableSize(), (unsigned)0);
  pool->setAutoFree(0);
  for( i = 0; i < AUTO_ALLOCATE_COUNT; ++i )
  {
    buffer[i] = NULL;
    EXPECT_EQ(pool->peakPoolSize(), (unsigned)(AUTO_ALLOCATE_COUNT - i - 1));
    EXPECT_EQ(pool->peakAvailableSize(), (unsigned)0);
  }

  // intetionally NO explict buffer release and pool destroy
  // test if implict release/destroy work
}

} // namespace NSFeaturePipe
} // namespace NSCamFeature
} // namespace NSCam
