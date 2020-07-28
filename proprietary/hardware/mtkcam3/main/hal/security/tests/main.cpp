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
 * MediaTek Inc. (C) 2019. All rights reserved.
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

#define LOG_TAG "securecamera_test"

#include <mtkcam/utils/std/ULog.h>

#include "InteractiveTest.h"

#include <gtest/gtest.h>

CAM_ULOG_DECLARE_MODULE_ID(MOD_CAMERA_HAL_SERVER);

// ------------------------------------------------------------------------

void showUsage(const std::string& programName)
{
    std::printf("Usage: %s [OPTION]\n\n", programName.c_str());
    std::printf("DESCRIPTION\n");
    std::printf("\t-n\t\t\texecute interactive tests.\n");
    std::printf("\t-g [--help|-h|-?] \texecute Google tests.\n");
    std::printf("\t\t\t\t--help, -h or -? lists the supported flags and their usage\n");
}

// ------------------------------------------------------------------------

int main(int argc, char **argv)
{
    // initialize ULog
    // NOTE: Can be only used by main()
    NSCam::Utils::ULog::ULogInitializer ulogInit;

    if (argc == 1)
    {
        showUsage(argv[0]);
        return EXIT_SUCCESS;
    }

    int opt;
    int ret = EXIT_SUCCESS;
    while ((opt = getopt(argc, argv, "ngh")) != -1)
    {
        switch (opt)
        {
            case 'n':
                ret = NSCam::tests::InteractiveTest().run();
                break;
            case 'g':
                {
                    // parse the command line and remove all
                    // recognized Google Test flags
                    // must call this function before calling RUN_ALL_TESTS()
                    ::testing::InitGoogleTest(&argc, argv);

                    ret = RUN_ALL_TESTS();
                }
                break;
            case 'h':
            default:
                showUsage(argv[0]);
        }

        // exit the while loop statement when finding gtest flag
        if (opt == 'g')
            break;
    }

    return ret;
}
