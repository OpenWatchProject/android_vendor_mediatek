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
 *   HIDL client's header file of ICopyDBFilter interface
 *
 * Author:
 * -------
 *   Bo.Shang (MTK80204) 04/18/2019
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
#include <sys/stat.h>
#include <android-base/file.h>
//#include <selinux/android.h>
#include <cutils/properties.h>
#include <sys/system_properties.h>
#include <vector>
#include <string>
#include "GlobalDefs.h"

namespace vendor {
namespace mediatek {
namespace hardware {
namespace modemdbfilter {
namespace V1_0 {
namespace implementation {

using ::android::hardware::hidl_array;
using ::android::hardware::hidl_death_recipient;
using ::android::hardware::hidl_handle;
using ::android::hardware::hidl_memory;
using ::android::hardware::hidl_string;
using ::android::hardware::hidl_vec;
using ::android::hardware::Return;
using ::android::hardware::Void;
using ::android::hidl::base::V1_0::IBase;
using ::android::sp;
using ::android::wp;
using ::android::OK;
using ::std::string;
using ::std::vector;
using ::vendor::mediatek::hardware::modemdbfilter::V1_0::IFileListCallback;
using ::vendor::mediatek::hardware::modemdbfilter::V1_0::IFileDataCallback;
using ::vendor::mediatek::hardware::modemdbfilter::V1_0::ICopyDBFilter;

typedef ::std::vector<string> StringVector;
typedef ::std::vector<string>::iterator StringVectorIter;

class CopyModemDBFilterClient;

class FileDataCallback: public IFileDataCallback
{
public:
    FileDataCallback(CopyModemDBFilterClient* client) : mHidlClient(client){}
    ~FileDataCallback(){}
    virtual Return<bool> sendFileFDToClient(const hidl_handle& hd) override;
private:
    CopyModemDBFilterClient* mHidlClient;
};

class FileListCallback: public IFileListCallback
{
public:
    FileListCallback(CopyModemDBFilterClient* client) : mHidlClient(client){}
    ~FileListCallback(){}
    virtual Return<bool> sendFileListToClient(const hidl_vec<hidl_string>& data) override;
private:
    CopyModemDBFilterClient* mHidlClient;
};

class DBFilterHidlServerDeathRecipient : public hidl_death_recipient {
public:
    DBFilterHidlServerDeathRecipient(CopyModemDBFilterClient *client) : mHidlClient(client) {}
    virtual void serviceDied(uint64_t cookie, const wp<IBase>& who) override;
private:
    CopyModemDBFilterClient *mHidlClient;
};

class CopyModemDBFilterClient {
public:
    CopyModemDBFilterClient(const char* name);
    ~CopyModemDBFilterClient();

    bool copyDBFilterFile();
    bool insertFileListData(const hidl_vec<hidl_string>& data);
    bool saveDataToFile(const hidl_handle& hd);
    void handleHidlDeath();

private:
    bool mEnableFileKeyWord;
    bool mNeedCopyCustomFilter;
    bool mNeedCopyDefaultFilter;
    bool mNeedCopyMDDB;
    char mServerName[64];
    char mFileKeyWord[64];
    char mFilePathToSave[PATH_MAX_LEN];
    int mLogFD;
    StringVector filesVector;

    sp<ICopyDBFilter>  mHidlServer;
    sp<DBFilterHidlServerDeathRecipient> mHidlServerDeathRecipient;
    sp<FileListCallback> mFileListCB;
    sp<FileDataCallback> mFileDataCB;

    bool cmpProperty(const char * propName,const char * expectValue, const char * defaultValue);
    bool copyVectorFiles(const char * srcRoot, const char * desRoot);
    bool copyFilter();
    bool copyCustomFilter();
    bool copyModemDB();
    bool createDataFolder();
    bool int_hidl();
    bool isExist(const char *path, mode_t mode);
    bool isNeedDoCopy();
    bool isModemSingleBinLoad();
    bool makeDir(const char *path, mode_t mode);
    bool makeDirs(const char *path, mode_t mode);
    bool setFileNameFlag(const char * path, const char *newPath);
    bool writeLogToFile(const char *log);

    int closeLogFD();
    int openFile(const char * pathname);

    void clearVectorAndKeyword();
    void createClientStartFile();
    void enableKeyword(const char * key);
    void waitVoldDecryptData();

};


}  // namespace implementation
}  // namespace V1_0
}  // namespace modemdbfilter
}  // namespace hardware
}  // namespace mediatek
}  // namespace vendor
