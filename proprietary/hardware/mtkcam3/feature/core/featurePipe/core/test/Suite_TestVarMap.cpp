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
#include <utils/RefBase.h>
#include <iostream>
#include <mtkcam3/feature/featurePipe/util/VarMap.h>

namespace NSCam{
namespace NSCamFeature{
namespace NSFeaturePipe{

class Buffer : public virtual android::RefBase
{
public:
  Buffer()
  {
  }

  virtual ~Buffer()
  {
  }
};

typedef android::sp<Buffer> BufferSP;
#define INT_TYPE int

#define TEST_INT_VAL 10
#define TEST_CHAR_VAL 'a'

TEST(VarMap, tryGet)
{
  VarMap map;
  int val1, val2;
  EXPECT_FALSE(map.tryGet<int>("no name", val1));
  EXPECT_FALSE(map.tryGet<int>("no name", val1));

  EXPECT_TRUE(map.set<int>("myint", TEST_INT_VAL));
  val1 = -1;
  val2 = -1;
  EXPECT_TRUE(map.tryGet<int>("myint", val1));
  EXPECT_TRUE(map.tryGet<int>("myint", val2));
  EXPECT_EQ(val1, TEST_INT_VAL);
  EXPECT_EQ(val2, TEST_INT_VAL);
  EXPECT_FALSE(map.tryGet<int>("no name", val1));
}

TEST(VarMap, get)
{
  VarMap map;
  int val1, val2;

  EXPECT_EQ(TEST_INT_VAL, map.get<int>("no name", TEST_INT_VAL));
  EXPECT_TRUE(map.set<int>("myint", TEST_INT_VAL));
  EXPECT_EQ(TEST_INT_VAL, map.get<int>("myint", -1));
  EXPECT_EQ(-1, map.get<int>("no name", -1));

  EXPECT_TRUE(map.set<int>("myint", TEST_INT_VAL+1));
  EXPECT_EQ(TEST_INT_VAL+1, map.get<int>("myint", -1));
}

TEST(VarMap, type)
{
  VarMap map;

  int intVal;
  char charVal;

  // test same name differenct type
  EXPECT_TRUE(map.set<char>("samename", TEST_CHAR_VAL));
  EXPECT_TRUE(map.set<int>("samename", TEST_INT_VAL));
  charVal = 0;
  EXPECT_TRUE(map.tryGet<char>("samename", charVal));
  intVal = 0;
  EXPECT_TRUE(map.tryGet<int>("samename", intVal));
  EXPECT_EQ(charVal, TEST_CHAR_VAL);
  EXPECT_EQ(intVal, TEST_INT_VAL);
  EXPECT_NE((int)charVal, (int)intVal);

  // test sp
  android::sp<Buffer> bufferA;
  android::sp<Buffer> bufferB;

  bufferA = new Buffer();
  ASSERT_TRUE(bufferA != NULL);
  EXPECT_TRUE(map.set<android::sp<Buffer> >("mybuffer", bufferA));
  EXPECT_TRUE(bufferB == NULL);
  EXPECT_TRUE(map.tryGet<android::sp<Buffer> >("mybuffer", bufferB));
  EXPECT_TRUE(bufferB == bufferA);

  // test typedef
  BufferSP bufferC;
  EXPECT_TRUE(map.tryGet<BufferSP>("mybuffer", bufferC));
  EXPECT_TRUE(bufferC == bufferA);

  // test define
  INT_TYPE intTypeVal;
  EXPECT_TRUE(map.set<int>("myintdef", TEST_INT_VAL));
  intTypeVal = 0;
  EXPECT_TRUE(map.tryGet<INT_TYPE>("myintdef", intTypeVal));
  EXPECT_EQ(intTypeVal, TEST_INT_VAL);
}

} // namespace NSFeaturePipe
} // namespace NSCamFeature
} // namespace NSCam
