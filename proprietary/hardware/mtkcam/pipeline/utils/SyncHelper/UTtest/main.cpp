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
#define LOG_TAG "SyncHelper_test"
#include <future>
#include <unistd.h>

#include "mtkcam/pipeline/utils/SyncHelper/ISyncHelperBase.h"

using namespace NSCam::v3::Utils::Imp;

/******************************************************************************
 *
 ******************************************************************************/

int main()
{

    android::sp<ISyncHelperBase> syncBase = ISyncHelperBase::createInstance();
    std::future<int> feature1, feature2, feature3;

    auto sensorThread = [=] (int camId, uint64_t timeIncrement, uint64_t syncTolerance,
        std::vector<int> syncCams) {

        SyncParam param;
        int ret = 0;
        param.mCamId = camId;
        param.mSyncCams = syncCams;
        param.mSyncTolerance = syncTolerance;
        printf("CamID = %d start!\n", camId);

        syncBase->start(camId);

        for (int i = 0; i<10 ; i++) {
            ret = 0;
            param.mReslutTimeStamp = timeIncrement*i;
            sleep(10);

            syncBase->syncEnqHW(param);

            printf("CamID = %d, FrameNo = %d, timeStemp = %lu, Ret = %d\n", camId, i, timeIncrement*i, ret);
        }

        for (int i = 0; i<10 ; i++) {
            ret = 0;
            param.mReslutTimeStamp = timeIncrement*i;
            sleep(10);
            syncBase->syncResultCheck(param);

            printf("CamID = %d, FrameNo = %d, timeStemp = %lu, Ret = %d\n", camId, i, timeIncrement*i, param.mSyncResult);
        }

        syncBase->stop(camId);
        return ret;

    };

    std::vector<int> s1 { 2, 3 };
    std::vector<int> s2 { 1, 3 };
    std::vector<int> s3 { 1, 2 };


    feature1 = std::async(std::launch::async, sensorThread, 1, 10, 1, s1);

    feature2 = std::async(std::launch::async, sensorThread, 2, 10, 1, s2);

    feature3 = std::async(std::launch::async, sensorThread, 3, 10, 1, s3);

    feature1.get();
    feature2.get();
    feature3.get();

    printf("Press any key to exit...\n");
    getchar();

    return 0;
}
