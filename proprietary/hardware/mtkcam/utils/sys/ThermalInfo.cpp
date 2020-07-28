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
#define  LOG_TAG "ThermalInfo"
#include <mtkcam/utils/sys/ThermalInfo.h>
#include <mtkcam/utils/std/Log.h>
#include <mtkcam/utils/std/ULog.h>
#include <utils/Errors.h>

#include <functional>
#include <memory>
#include <regex>
#include <stdio.h>
#include <fstream>
#include <string>

CAM_ULOG_DECLARE_MODULE_ID(MOD_UTILITY);

#define MY_LOGV(fmt, arg...)        CAM_ULOGMV("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGD(fmt, arg...)        CAM_ULOGMD("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGI(fmt, arg...)        CAM_ULOGMI("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGW(fmt, arg...)        CAM_ULOGMW("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGE(fmt, arg...)        CAM_ULOGME("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGA(fmt, arg...)        CAM_LOGA("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGF(fmt, arg...)        CAM_LOGF("[%s] " fmt, __FUNCTION__, ##arg)

namespace NSCam {
namespace NSThermalInfo {

int getSystemThermalValue()
{
    /* Usually, we cat /proc/mtktz/mtktsAP
     * represents the Temperature Value in Celsius
     * Terminal wouls show like below format
     * $ adb shell cat /proc/mtktz/mtktsAP
     * 26000 <= This is Celsius Value
     * ma_len=1
     * 1 (1000000,1) (1000000,1) (1000000,1)
     */

    const char nodepath[30] = "/proc/mtktz/mtktsAP";
    int termalVal = -EINVAL;
    std::string readstr;

    std::ifstream fp(nodepath, std::ifstream::binary|std::ifstream::in);
    if (!fp.is_open())
    {
        MY_LOGE("Open File Node Error");
        return -ENOENT;
    } else {
        fp >> readstr; //only need 1st value

        termalVal = stoi(readstr);

        fp.close();
    }
    return termalVal;
}

int setThermalMode(int Mode)
{
    /* Usually, $ echo 1 >  /proc/xxx
     * represents set Thermla Unstrick Mode
     * Terminal wouls show like below format
     * $ adb shell "echo 1 > /proc/xxx
     */
    const char nodepath[50] = "/proc/driver/cl_apu_status";
    std::ofstream fp(nodepath, std::ofstream::binary|std::ofstream::out);
    if (!fp.is_open())
    {
        MY_LOGE("Open File Node(%s) Error", nodepath);
        return -ENOENT;
    }
    else
    {
        if (Mode == 0 || Mode == 1) {
            fp << Mode;
            fp.close();
        } else {
            fp.close();
            return -EINVAL;
        }
    }

    return 0;
}

};
};
