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

#ifndef _TEST_IO_H_
#define _TEST_IO_H_

#include <vector>
#include <featurePipe/core/include/IOUtil.h>

namespace NSCam{
namespace NSCamFeature{
namespace NSFeaturePipe{

#define NODEA_NAME   "NodeA"
#define NODEB_NAME   "NodeB"
#define NODEC_NAME   "NodeC"
#define NODED_NAME   "NodeD"
#define NODEE_NAME   "NodeE"

#define NODEDA_NAME   "NodeDA"
#define NODEDB_NAME   "NodeDB"

class TestReqInfo{
public:
    MUINT32 mFrameNo;
    MUINT32 mFeatureMask;
    MUINT32 mMasterId;
    MUINT32 mSensorId;
    MVOID makeDebugStr() {
        str = android::String8::format("No(%d), fmask(0x%08x), sId(%d), masterId(%d)",mFrameNo, mFeatureMask, mSensorId, mMasterId);
    }
    const char* dump() const { return str.c_str();}

    TestReqInfo(MUINT32 fno, MUINT32 mask, MUINT32 mId, MUINT32 sId)
    : mFrameNo(fno)
    , mFeatureMask(mask)
    , mMasterId(mId)
    , mSensorId(sId)
    {
        makeDebugStr();
    }
private:
    android::String8 str = android::String8("");
};

struct TestOutputConfig
{
    bool record;
    bool display;
    bool extra;
    bool phy;
};

struct TestNodeConfig
{
    bool A;
    bool A_3; // A + 3DNR
    bool A_P; // A + PhysicalOut
    bool B;
    bool C;
    bool C_A; // InOut Exclusive
    bool C_I; // Inplace
    bool D;
    bool D_Q;
    bool E;
    bool DA;  // Dual In Single Out
    bool DB;  // Dual In Dual Out

    unsigned pack()
    {
        unsigned value = 0;
        int bit = 0;
        if(A)   value |= (1<<bit); bit++;
        if(A_3) value |= (1<<bit); bit++;
        if(A_P) value |= (1<<bit); bit++;
        if(B)   value |= (1<<bit); bit++;
        if(C)   value |= (1<<bit); bit++;
        if(C_A) value |= (1<<bit); bit++;
        if(C_I) value |= (1<<bit); bit++;
        if(D)   value |= (1<<bit); bit++;
        if(D_Q) value |= (1<<bit); bit++;
        if(E)   value |= (1<<bit); bit++;
        if(DA)  value |= (1<<bit); bit++;
        if(DB)  value |= (1<<bit); bit++;

        //printf("pack 0x%08X\n", value);
        return value;
    }
    void unpack(unsigned value)
    {
        int bit = 0;
        A =    (value & (1<<bit)); bit++;
        A_3 =  (value & (1<<bit)); bit++;
        A_P =  (value & (1<<bit)); bit++;
        B =    (value & (1<<bit)); bit++;
        C =    (value & (1<<bit)); bit++;
        C_A =  (value & (1<<bit)); bit++;
        C_I =  (value & (1<<bit)); bit++;
        D =    (value & (1<<bit)); bit++;
        D_Q =  (value & (1<<bit)); bit++;
        E =    (value & (1<<bit)); bit++;
        DA =   (value & (1<<bit)); bit++;
        DB =   (value & (1<<bit)); bit++;

        //printf("unpack 0x%08X\n", value);
    }
};

struct TestNodeExpect
{
    bool record;
    bool display;
    bool extra;
    bool phy;
    bool full;
    bool QFull;
    bool AFull;
    bool DFull;
};

class TestNode;

struct TestCase
{
    //config
    TestOutputConfig hasOutput;
    TestNodeConfig needNode;

    //expect
    std::vector<TestNodeExpect> nodeExpect;
};

class TestIONode
{
public:
    typedef TestIONode* NodeID_T;

    TestIONode(const char *name);
    TestIONode();
    const char* getName() const;
    IOPolicyType getIOPolicy(StreamType, const TestReqInfo& reqInfo) const;
    bool getInputBufferPool(const TestReqInfo&, android::sp<IBufferPool>& pool, NextFullInfo &info);
    void setInputBufferPool(android::sp<IBufferPool>& pool);
private:
    const char* mName;
    android::sp<IBufferPool> mInputBufferPool;
};

} // namespace NSFeaturePipe
} // namespace NSCamFeature
} // namespace NSCam

#endif // _TEST_IO_H_
