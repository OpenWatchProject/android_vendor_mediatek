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

#include "gtest/gtest.h"
#include <utils/RefBase.h>
#include <string>
#include <iostream>

#include "../P2_Param.h"
#include "TestP2_Common.h"
#include <mtkcam/utils/std/DebugTimer.h>

using android::sp;
using android::RefBase;

using namespace NSCam;
using namespace NSCam::v3;
using namespace NSCam::Utils;

namespace P2
{
//********************************************
// Test Utils
//********************************************


//********************************************
// Test Cases
//********************************************
TEST(P2InIDMap, fourCam)
{
    MY_UTLOG("+");
    MUINT32 mainID = 0;
    std::vector<MUINT32> subIDList = {2,4,8};
    std::vector<MUINT32> testSIDList = {0,2,4,8};
    std::vector<ID_IMG> testImgIDList = {IN_FULL , IN_RESIZED, IN_LCSO , IN_RSSO, IN_RESIZED_YUV1 , IN_FULL_YUV};

    sp<P2InIDMap> idMap = new P2InIDMap(mainID, subIDList);
    MY_UTLOG("start test");
    DebugTimer timer;
    timer.start();
    for(int i = 0; i < 99999 ; i++)
    {
        MUINT32 sID = testSIDList[i % testSIDList.size()];
        ID_IMG imgID = testImgIDList[i % testImgIDList.size()];
        ID_IMG findID = idMap->getImgID(sID, imgID);
        if(findID == NO_IMG)
        {
            printf("ID not found in sId(%d) imgID(%d)", sID, imgID);
        }
    }
    timer.stop();
    printf("total time = %d us", timer.getElapsedU());
    MY_UTLOG("end test");
    MY_UTLOG("-");
}

} // namespace P2
