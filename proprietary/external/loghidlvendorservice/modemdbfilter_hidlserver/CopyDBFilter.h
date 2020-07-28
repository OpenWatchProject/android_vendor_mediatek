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

/*******************************************************************************
 *
 * Filename:
 * ---------
 *   CopyDBFilter.h
 *
 * Description:
 * ------------
 *   HIDL service's header file of ICopyDBFilter interface
 *
 * Author:
 * -------
 *   Bo.Shang (MTK80204) 04/16/2019
 *
 *------------------------------------------------------------------------------
 * $Revision:$
 * $Modtime:$
 * $Log:$
 *******************************************************************************/

#pragma once

#include <vendor/mediatek/hardware/modemdbfilter/1.0/ICopyDBFilter.h>
#include <hidl/MQDescriptor.h>
#include <hidl/Status.h>
#include <log/log.h>
#include <android-base/unique_fd.h>
#include <sys/types.h>
#include <sys/stat.h>

#ifdef LOG_TAG
#undef LOG_TAG
#define LOG_TAG "modemdbfilter_service"
#endif

#define HIDL_SERVER_NAME "ModemDBFilterHidlServer"
#define PATH_MAX_LEN 256

namespace vendor {
namespace mediatek {
namespace hardware {
namespace modemdbfilter {
namespace V1_0 {
namespace implementation {

using ::android::hardware::hidl_array;
using ::android::hardware::hidl_memory;
using ::android::hardware::hidl_string;
using ::android::hardware::hidl_vec;
using ::android::hardware::hidl_handle;
using ::android::hardware::Return;
using ::android::hardware::Void;
using ::android::base::unique_fd;
using ::android::sp;
using ::vendor::mediatek::hardware::modemdbfilter::V1_0::IFileListCallback;
using ::vendor::mediatek::hardware::modemdbfilter::V1_0::IFileDataCallback;
using ::vendor::mediatek::hardware::modemdbfilter::V1_0::ICopyDBFilter;

class ModemDBFilterService : public ICopyDBFilter {
public:
    ModemDBFilterService(const char* name);
    ~ModemDBFilterService();
    // Methods from ::vendor::mediatek::hardware::modemdbfilter::V1_0::ICopyDBFilter follow.
    virtual Return<bool> getFileListAndSize(const hidl_string& path, const sp<IFileListCallback>& fileListCB) override;
    virtual Return<bool> getFileData(const hidl_string& path, const sp<IFileDataCallback>& fileDataCB) override;

private:
    bool isExist(const char * pathname);

    int searchFiles(const char * folder, bool mRecursive = false);
    int countFiles(const char * folder, bool mRecursive = false);
    int insertFileToVec(const char * folder, bool mRecursive = false);
    int isdir(const char * file);

    unique_fd openFile(const char * pathname);
    void freeHidlHandle(const hidl_handle& hd);
    void releaseVector();

    char mServerName[64];
    hidl_vec<hidl_string> fileListVector;
    sp<ICopyDBFilter> mDBFilterCallback;

};


}  // namespace implementation
}  // namespace V1_0
}  // namespace modemdbfilter
}  // namespace hardware
}  // namespace mediatek
}  // namespace vendor
