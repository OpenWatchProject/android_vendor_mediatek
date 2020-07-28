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

#define LOG_TAG "mtkcam-debug"
//
#include <unistd.h>
//
//c++
#include <string>
#include <vector>
//
//system
#include <log/log.h>
//
#include <android/hardware/camera/provider/2.4/ICameraProvider.h>
//
using ::android::hardware::camera::provider::V2_4::ICameraProvider;
using ::android::hardware::hidl_handle;
using ::android::hardware::hidl_string;
using ::android::hardware::hidl_vec;
//
/******************************************************************************
 *
 ******************************************************************************/
#define MY_LOGV(fmt, arg...)        ALOGV("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGD(fmt, arg...)        ALOGD("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGI(fmt, arg...)        ALOGI("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGW(fmt, arg...)        ALOGW("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGE(fmt, arg...)        ALOGE("[%s]{#%d:%s} " fmt, __FUNCTION__, __LINE__, __FILE__, ##arg)


/******************************************************************************
 *
 ******************************************************************************/
int main(int argc __unused, char **argv __unused)
{
    MY_LOGD("+ argc:%d", argc);
    int fd = STDOUT_FILENO;

    try {
        //  ICameraProvider
        ::android::sp<ICameraProvider> pProvider = ICameraProvider::getService("internal/0");
        if ( pProvider == nullptr ) {
            MY_LOGE("ICameraProvider::getService return nullptr");
            return -1;
        }


        //  debug(handle, "hidl_options")
        std::vector<hidl_string> options;
        dprintf(fd, "mtkcam-debug");
        for (int i = 1; i < argc; ++i) {
            options.push_back(argv[i]);
            //MY_LOGD(" %s", argv[i]);
            dprintf(fd, " \"%s\"", argv[i]);
        }
        dprintf(fd, "\n");
        hidl_vec<hidl_string> hidl_options;
        hidl_options.setToExternal(options.data(), options.size(), false);


        //  debug("handle", hidl_options)
        native_handle_t* raw_handle = native_handle_create(1, 0);
        if ( raw_handle == nullptr ) {
            MY_LOGE("fail on native_handle_create");
            return -1;
        }

        {
            raw_handle->data[0] = fd;
            hidl_handle handle(raw_handle);

            auto ret = pProvider->debug(handle, hidl_options);
            if ( ! ret.isOk() ) {
                MY_LOGE("Transaction error in ICameraProvider::debug: %s", ret.description().c_str());
            }
        }
        native_handle_delete(raw_handle);
    }
    catch (std::exception&) {
        MY_LOGW("an exception caught");
    }

    MY_LOGD("-");
    return 0;
}

