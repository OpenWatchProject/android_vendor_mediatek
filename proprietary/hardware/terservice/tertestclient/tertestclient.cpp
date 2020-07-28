/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2010. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
 * AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek Software")
 * have been modified by MediaTek Inc. All revisions are subject to any receiver's
 * applicable license agreements with MediaTek Inc.
 */
#include <sys/types.h>
#include <unistd.h>
#include <grp.h>
#include <utils/String8.h>
#include <utils/Log.h>
//#include <binder/IPCThreadState.h>
//#include <binder/ProcessState.h>
#include <binder/IServiceManager.h>
//#include <private/android_filesystem_config.h>
#include "ITerService.h"
 
using namespace android;
 
int main(int argc, char** argv)
{
    status_t result = NAME_NOT_FOUND;
    status_t result2 = NO_ERROR;

    const String16 name("terservice");
    sp<ITerService> terservice = 0;

    bool bServiceEnabled = false;
    bool bDataReady=false;
    String8 mMccMnc("");

    printf("[tertestclient] started...\n");

#ifndef _LIBCPP_NO_EXCEPTIONS
    try {
#endif  // _LIBCPP_NO_EXCEPTIONS
        result = getService(name, &terservice);
#ifndef _LIBCPP_NO_EXCEPTIONS
    } catch (const std::exception& e) {
        printf("[tertestclient] getService failed. exit. (%s)", e.what());
        return 0;
    }
#endif  // _LIBCPP_NO_EXCEPTIONS

    if (argc==2)
    {
        printf("argv[0]=%s\n", argv[0]);
        printf("argv[1]=%s\n", argv[1]);

        // to turn on off service (0: off, 1: on)
        int param1 = atoi(argv[1]);

        if (param1 == 0) {
            printf("[tertestclient] test service OFF\n");
            terservice->setEarlyReadServiceEnable(false);
            printf("[tertestclient] setEarlyReadServiceEnable (false)\n");
        } else if (param1 == 1) {
            printf("[tertestclient] test service ON\n");
            terservice->setEarlyReadServiceEnable(true);
            printf("[tertestclient] setEarlyReadServiceEnable (true)\n");
        }
    }

    if (argc>1)
    {
        printf("[tertestclient] ended.\n");
        return 0;
    }

    if (result == NO_ERROR) {
        printf("[tertestclient] test 1\n");
        bServiceEnabled = terservice->isEarlyReadServiceEnabled();
        printf("[tertestclient] isEarlyReadServiceEnabled= %d\n",( bServiceEnabled ? 1 : 0 ));

        printf("[tertestclient] test 2\n");
        bDataReady = terservice->isEarlyDataReady();
        printf("[tertestclient] isEarlyDataReady= %d\n",( bDataReady ? 1 : 0 ));

        printf("[tertestclient] test 3\n");
        result2 = terservice->getSimMccMnc(&mMccMnc);
        printf("[tertestclient] getSimMccMnc(%s) length(%zu)\n", mMccMnc.string(), mMccMnc.length());

    }
    else
    {
        printf("[tertestclient] get service ret code= %d, %d\n", result, result2);
    }

//    sp<IServiceManager> sm = defaultServiceManager();
//    ITerService service = sm->getService(String16("lotteryserver"));
//    RLOGI("[TerService] service manager: %p/n",sm.get());
//
//    if (binder == 0) {
//           RLOGW("[TerService] getService return 0");
//           return;
//    }

    printf("[tertestclient] ended.\n");

}
