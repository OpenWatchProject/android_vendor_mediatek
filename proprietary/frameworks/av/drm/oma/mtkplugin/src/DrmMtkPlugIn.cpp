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
 * MediaTek Inc. (C) 2010. All rights reserved
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

/*
 * Copyright (C) 2010 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#define LOG_NDEBUG 0
#define LOG_TAG "DrmMtkPlugIn"
#include <utils/Log.h>

#include <DrmMtkPlugIn.h>

#include <drm/DrmMtkDef.h>
#include <drm/DrmMtkUtil.h>
#include <drm/SecureTimer.h>

#include <ByteBuffer.h>
#include <CipherFileUtil.h>
#include <CryptoHelper.h>
#include <DrmDef.h>
#include <DrmTypeDef.h>
#include <DrmUtil.h>
#include <RO.h>
#include <SecureTimerHelper.h>
#include <StrUtil.h>
#include <DrmRequestType.h>

#include <drm/DrmConstraints.h>
#include <drm/DrmConvertedStatus.h>
#include <drm/drm_framework_common.h>
#include <drm/DrmInfo.h>
#include <drm/DrmInfoEvent.h>
#include <drm/DrmInfoRequest.h>
#include <drm/DrmInfoStatus.h>
#include <drm/DrmMetadata.h>
#include <drm/DrmRights.h>
#include <drm/DrmSupportInfo.h>

#include <cutils/properties.h>

#include <dirent.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/times.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <time.h>
#include <unistd.h>

#include "ActivityManager.h"

#define _DRM_DEBUG

// the invalid offset
#define INVALID_OFFSET 0x7fffffff

// openDecryptSession decode image actions
#define ACTION_DECODE_FULL_IMAGE     0x0
#define ACTION_JUST_DECODE_BOUND     0x1
#define ACTION_JUST_DECODE_THUMBNAIL 0x2

using namespace android;


// This extern "C" is mandatory to be managed by TPlugInManager
extern "C" IDrmEngine* create()
{
    return new DrmMtkPlugIn();
}

// This extern "C" is mandatory to be managed by TPlugInManager
extern "C" void destroy(IDrmEngine* pPlugIn)
{
    delete pPlugIn;
    pPlugIn = NULL;
}

DrmMtkPlugIn::DrmMtkPlugIn() :
    DrmEngineBase()
{
    // the flag if we using "Forward Lock Only" feature
    m_bFwdLockOnly = false;

    // we check according to the property "vendor.drm.forwardlock.only"
    if (DrmUtil::sDDebug) ALOGD("DrmMtkPlugIn() : check the Forward-lock-only property");
    char value[PROPERTY_VALUE_MAX];
    bzero(value, sizeof(value));
    if (property_get("vendor.drm.forwardlock.only", value, NULL)
            && (0 == strcmp(value, "1") || 0 == strcmp(value, "yes") || 0 == strcmp(value, "true"))) {
        if (DrmUtil::sDDebug) ALOGD("DrmMtkPlugIn() : Forward-lock-only is set.");
        m_bFwdLockOnly = true;
    }

    int res = SecureTimer::instance().load();
    if (DrmUtil::sDDebug) ALOGD("SecureTimer DrmMtkPlugIn() : load securetimer result=[%d]", res);
    SecureTimer::instance().updateTimeBase();
    if (DrmUtil::sDDebug) ALOGD("SecureTimer DrmMtkPlugIn() : updateTimeBase");
}

DrmMtkPlugIn::~DrmMtkPlugIn()
{
}

//@{ M: Add by rui.hu
//To fix ALPS01752997
long str2long(const char *str)
{
    if(str == NULL)
    {
        ALOGE("str2long bad input str is NULL");
        return 0;
    }
    long ret = 0;
    sscanf(str,"%ld", &ret);
    if (DrmUtil::sDDebug) ALOGD("str2long ret = %ld", ret);
    return ret;
}

/**
     * Parse constraints from a entry
     * @param entryPtr[in] The entry to be parsed which should be not null
     * @param drmConstraints[out] A structure used to store constraints
     */
void parseConctraints(Entry *entryPtr, DrmConstraints* drmConstraints)
{
    Entry entry = *entryPtr;
    if (DrmUtil::sDDebug) ALOGD("parseConctraints() : total_count: %ld, used_count: %ld", entry.total_count,
            entry.used_count);
    //Since count type rights is at the last item
    //So there is no need to get count info from drmConstraints
    long max_repeat_count = str2long(drmConstraints->get(DrmConstraints::MAX_REPEAT_COUNT).string());
    long used_count = str2long(drmConstraints->get(DrmConstraints::REMAINING_REPEAT_COUNT).string());
    if (max_repeat_count < 0 || used_count < 0) {
       max_repeat_count = entry.total_count;
       used_count = entry.used_count;
    } else {
    max_repeat_count += entry.total_count;
    used_count += entry.used_count;
    }

    if (max_repeat_count > 0 && used_count >0) {
        drmConstraints->put(&DrmConstraints::MAX_REPEAT_COUNT, StrUtil::toString(max_repeat_count));
        drmConstraints->put(&(DrmConstraints::REMAINING_REPEAT_COUNT), StrUtil::toString(used_count));
    }

    bool isValid = SecureTimer::instance().isValid();
    time_t offset = 0; // offset = real time - device time
    if (isValid)
    {
        offset = SecureTimer::instance().getOffset();
        if (DrmUtil::sDDebug) ALOGD("parseConctraints() : secure timer offset: %ld", offset);
    }

    // get bigger start time
    if (DrmUtil::sDDebug) ALOGD("parseConctraints() : start_time %ld, start_intv %ld", entry.start_time, entry.start_intv);
    long min = DrmDef::MIN_START_TIME;
    if (entry.start_time != min || entry.start_intv != 0)
    {
        long start = entry.start_time > entry.start_intv ? entry.start_time : entry.start_intv;
        if (DrmUtil::sDDebug) ALOGD("parseConctraints() : put start_time: %ld = %ld - %ld", start - offset, start,
                offset);
        drmConstraints->put(&(DrmConstraints::LICENSE_START_TIME),
                StrUtil::toString(start - offset));
    }

    // get smaller end time
    if (DrmUtil::sDDebug) ALOGD("parseConctraints() : end_time %ld, end_intv %ld", entry.end_time, entry.end_intv);
    long max = DrmDef::MAX_END_TIME;
    if (entry.end_time != max || entry.end_intv != 0)
    {
        long end = 0;
        if (entry.end_time > 0 && entry.end_intv > 0)
        {
            end = entry.end_time < entry.end_intv ? entry.end_time : entry.end_intv;
        } else
        {
            end = entry.end_time > 0 ? entry.end_time : entry.end_intv;
        }
        if (DrmUtil::sDDebug) ALOGD("parseConctraints() : put end_time: %ld = %ld - %ld", end - offset, end, offset);
        drmConstraints->put(&(DrmConstraints::LICENSE_EXPIRY_TIME),
                StrUtil::toString(end - offset));
    }

    if (entry.interval > 0)
    {
        if (DrmUtil::sDDebug) ALOGD("parseConctraints() : interval:", entry.interval);
        drmConstraints->put(&(DrmConstraints::LICENSE_AVAILABLE_TIME),
                StrUtil::toString(entry.interval));
    }

    // put -1 if no constraints
    if ((drmConstraints->get(DrmConstraints::REMAINING_REPEAT_COUNT)).length() == 0)
    {
        drmConstraints->put(&DrmConstraints::MAX_REPEAT_COUNT, "-1");
        drmConstraints->put(&(DrmConstraints::REMAINING_REPEAT_COUNT), "-1");
    }
    if (drmConstraints->get(DrmConstraints::LICENSE_START_TIME).length() == 0)
    {
        drmConstraints->put(&(DrmConstraints::LICENSE_START_TIME), "-1");
    }
    if (drmConstraints->get(DrmConstraints::LICENSE_EXPIRY_TIME).length() == 0)
    {
        drmConstraints->put(&(DrmConstraints::LICENSE_EXPIRY_TIME), "-1");
    }

    if (drmConstraints->get(DrmConstraints::LICENSE_AVAILABLE_TIME).length() == 0)
    {
        drmConstraints->put(&(DrmConstraints::LICENSE_AVAILABLE_TIME), "-1");
    }
}
//@}


// in c++ make sure the returned pointer of DrmConstraints is deleted by caller.
// returns NULL if it fails.
DrmConstraints* DrmMtkPlugIn::onGetConstraints(int uniqueId,
        const String8* path, int action)
{
    if (DrmUtil::sDDebug) ALOGD("onGetConstraints() [%d]", uniqueId);
    DrmMetadata* drmMetadata = onGetMetadata(uniqueId, path);
    if (drmMetadata == NULL)
    {
        ALOGE("onGetConstriants() : error, failed to get metadata.");
        return NULL;
    }

    String8 cid = drmMetadata->get(String8(DrmMetaKey::META_KEY_CONTENT_URI));
    if (cid.isEmpty())
    {
        ALOGE("onGetConstraints() : error, invalid content id (cid).");
        delete drmMetadata;
        return NULL;
    }
    String8 method = drmMetadata->get(String8(DrmMetaKey::META_KEY_METHOD));
    if (method.isEmpty())
    {
        ALOGE("onGetConstraints() : error, invalid method.");
        delete drmMetadata;
        return NULL;
    }

    // for FL method, put -1 in constraints
    DrmConstraints* drmConstraints = new DrmConstraints();
    if (method == StrUtil::toString(DrmDef::METHOD_FL))
    {
        drmConstraints->put(&DrmConstraints::MAX_REPEAT_COUNT, "-1");
        drmConstraints->put(&(DrmConstraints::REMAINING_REPEAT_COUNT), "-1");
        drmConstraints->put(&(DrmConstraints::LICENSE_START_TIME), "-1");
        drmConstraints->put(&(DrmConstraints::LICENSE_EXPIRY_TIME), "-1");
        drmConstraints->put(&(DrmConstraints::LICENSE_AVAILABLE_TIME), "-1");
        delete drmMetadata;
        return drmConstraints;
    }

    //@{ M: Modify by rui.hu
    //To fix ALPS01752997
    if (DrmUtil::sDDebug) ALOGD("onGetConstraints() : the action type: %d", action);
    RO ro;
    if (!DrmUtil::restore(cid, &ro))
    {
        ALOGE("onGetConstraints() : failed to restore ro.");
        delete drmMetadata;
        delete drmConstraints;
        return NULL;
    }
    int permission = DrmDef::getPermissionFromAction(action);
    int permissionIndex = DrmDef::getPermissionIndex(permission);
    if (permissionIndex < 0)
    {
        ALOGE("traverseRights: permission index is negative[%d]", permissionIndex);
        delete drmMetadata;
        delete drmConstraints;
        return NULL;
    }

    int best = ro.rights.best[permissionIndex];
    if (DrmUtil::sDDebug) ALOGD("best = %d", best);
    Entry *entryPtr = ro.rights.entryPtr;
    int entryIndex = best;
    while (entryIndex != 0xFF)
    {
        if (DrmUtil::sDDebug) ALOGD("entryIndex = %d", entryIndex);
        if (DrmUtil::isEntryValid(&entryPtr[entryIndex]))
        {
            if (DrmUtil::sDDebug) ALOGD("parse a valid entry %d",entryIndex);
            parseConctraints(&entryPtr[entryIndex], drmConstraints);

        }
        entryIndex = entryPtr[entryIndex].next;
        if (DrmUtil::sDDebug) ALOGD("next entryIndex = %d", entryIndex);
    }
    ro.rights.dump();
    //@}
    delete drmMetadata;
    return drmConstraints;
}

// in c++ make sure the returned pointer of DrmMetadata is deleted by caller.
// returns NULL if it fails.
DrmMetadata* DrmMtkPlugIn::onGetMetadata(int uniqueId, const String8* path)
{
    if (DrmUtil::sDDebug) ALOGD("onGetMetadata() [%d]", uniqueId);
    if (NULL == path)
    {
        ALOGE("onGetMetadata() : error, the path is NULL");
        return NULL;
    }

    DrmMetadata* drmMetadata = new DrmMetadata();
    if (!DrmUtil::parseDcf(*path, drmMetadata))
    {
        ALOGE("onGetMetadata() : error, failed to parse dcf file.");
        delete drmMetadata;
        return NULL;
    }

    // "Forward Lock Only"
    String8 method = drmMetadata->get(String8(DrmMetaKey::META_KEY_METHOD));
    if (m_bFwdLockOnly && method != StrUtil::toString(DrmDef::METHOD_FL)) {
        if (DrmUtil::sDDebug) ALOGD("onGetMetadata() : Forward-lock-only is set, and it's not FL type");
        delete drmMetadata;
        return NULL;
    }

    return drmMetadata;
}

status_t DrmMtkPlugIn::onInitialize(int uniqueId)
{
    if (DrmUtil::sDDebug) ALOGD("DrmMtkPlugIn::onInitialize : %d", uniqueId);
    return DRM_NO_ERROR;
}

status_t DrmMtkPlugIn::onSetOnInfoListener(int uniqueId,
        const IDrmEngine::OnInfoListener* infoListener)
{
    if (DrmUtil::sDDebug) ALOGD("DrmMtkPlugIn::onSetOnInfoListener : %d", uniqueId);
    return DRM_NO_ERROR;
}

status_t DrmMtkPlugIn::onTerminate(int uniqueId)
{
    if (DrmUtil::sDDebug) ALOGD("DrmMtkPlugIn::onTerminate : %d", uniqueId);
    return DRM_NO_ERROR;
}

// whether the file can be handled by DRM engine. check by file surffix.
bool DrmMtkPlugIn::onCanHandle(int uniqueId, const String8& path)
{
    if (DrmUtil::sDDebug) ALOGD("onCanHandle() [%d], file path: %s ", uniqueId, path.string());
    String8 extString = path.getPathExtension();
    extString.toLower();
    return (extString == DrmDef::EXT_DRM_CONTENT
            || extString == DrmDef::EXT_DRM_MESSAGE
            || extString == DrmDef::EXT_RIGHTS_XML
            || extString == DrmDef::EXT_RIGHTS_WBXML);
}

DrmInfoStatus* DrmMtkPlugIn::onProcessDrmInfo(int uniqueId,
        const DrmInfo* drmInfo)
{
    if (DrmUtil::sDDebug) ALOGD("DrmMtkPlugIn::onProcessDrmInfo - Enter : %d", uniqueId);
    DrmInfoStatus* drmInfoStatus = NULL;

    // =====================================================================
    // new implementation for refactored OMA DRM framework: com.mediatek.drm
    // =====================================================================
    //will be remove, after remove omadrmclient. or else, it will case JE.
    if (DrmRequestType::TYPE_GET_DRM_INFO == drmInfo->getInfoType())
    {
        if (DrmUtil::sDDebug) ALOGD("DrmMtkPlugIn::onProcessDrmInfo - received TYPE_GET_DRM_INFO");
        String8 actionType = drmInfo->get(DrmRequestType::KEY_ACTION);
        String8 ACTION_GET_CONTENT_ID("getContentId");
        if (actionType == ACTION_GET_CONTENT_ID)
        {
            DrmRights rights(drmInfo->getData(), drmInfo->getMimeType());
            String8 cid = DrmMtkUtil::getContentId(rights);
            if (DrmUtil::sDDebug) ALOGD("DrmMtkPlugIn::onProcessDrmInfo - get content-id: %s", cid.string());

            // convert cid to DrmBuffer and put to drmInfoStatus
            size_t size = cid.size();
            char* value = new char[size];
            bzero(value, size);
            memcpy(value, cid.string(), size);
            DrmBuffer* valueBuffer = new DrmBuffer(value, (int)size);

            drmInfoStatus = new DrmInfoStatus(DrmInfoStatus::STATUS_OK,
                    drmInfo->getInfoType(), valueBuffer,
                    drmInfo->getMimeType());
        }
    }
    if (DrmUtil::sDDebug) ALOGD("DrmMtkPlugIn::onProcessDrmInfo - Exit");
    return drmInfoStatus;
}

// returns an empty string if it fails to save RO.
status_t DrmMtkPlugIn::onSaveRights(int uniqueId, const DrmRights& drmRights,
        const String8& rightsPath, const String8& contentPath)
{
    if (DrmUtil::sDDebug) ALOGD("SaveRights() [%d] r[%s] c[%s]", uniqueId, rightsPath.string(), contentPath.string());
    // "Forward Lock Only"
    if (m_bFwdLockOnly)
    {
        if (DrmUtil::sDDebug) ALOGD("onSaveRights() : Forward-lock-only is set, and don't save any rights.");
        return DRM_NO_ERROR;
    }

    Mutex::Autolock lock(DrmUtil::mROLock);

    // check mime in drmRights
    if (drmRights.getMimeType() != DrmDef::MIME_RIGHTS_WBXML
        && drmRights.getMimeType() != DrmDef::MIME_RIGHTS_XML)
    {
        ALOGE("onSaveRights() : error, the mime type: %s must be xml or wbxml.",
                drmRights.getMimeType().string());
        return DRM_ERROR_UNKNOWN;
    }

    // parse ro
    RO newRO;
    DrmBuffer drmBuffer = drmRights.getData();
    bool parseResult = false;
    if (drmRights.getMimeType() == DrmDef::MIME_RIGHTS_WBXML)
    {
        parseResult = DrmUtil::parse_drc(drmBuffer, &newRO);
    }
    else if (drmRights.getMimeType() == DrmDef::MIME_RIGHTS_XML)
    {
        parseResult = DrmUtil::parse_dr(drmBuffer, &newRO);
    }
    if (!parseResult)
    {
        ALOGE("onSaveRights() : error, failed to parse RO.");
        return DRM_ERROR_UNKNOWN;
    }

    // for CD, the roPath shall be specified by {rightsPath};
    // for SD, the {rightsPath} is not specified (empty) and shall get from RO structure
    String8 roPath;
    if (rightsPath.isEmpty())
    {
        roPath = DrmUtil::getROFileName(String8(newRO.rights.cid));
    }
    else
    {
        roPath = rightsPath;
        strcat(newRO.rights.cid, DrmUtil::readId().string());
    }

    // check old RO existence: merge, or save new one
    if (DrmUtil::checkExistence(roPath))
    {
        // exist, restore old ro & merge
        if (DrmUtil::sDDebug) ALOGD("onSaveRights() : the old RO file: %s existed, merge it.",
                roPath.string());
        if (!DrmUtil::merge(&newRO))
        {
            ALOGE("onSaveRights() : error, failed to merge the new RO.");
            return DRM_ERROR_UNKNOWN;
        }
    }
    else
    {
        if (DrmUtil::sDDebug) ALOGD("onSaveRights() : the old RO file: %s didn't exist, save it.",
                roPath.string());
        if (!DrmUtil::save(&newRO))
        {
            ALOGE("onSaveRights() : error, failed to save the new RO.");
            return DRM_ERROR_UNKNOWN;
        }
    }

    // Trigger to scan relate content files to update metadata, such as duration for audio/video
    // and height/width.
    // sendBroadcastMessage(String16(newRO.rights.cid));
    return DRM_NO_ERROR;
}

DrmInfo* DrmMtkPlugIn::onAcquireDrmInfo(int uniqueId,
        const DrmInfoRequest* drmInfoRequest)
{
    if (DrmUtil::sDDebug) ALOGD("DrmMtkPlugIn::onAcquireDrmInfo : %d", uniqueId);
    DrmInfo* drmInfo = NULL;

    if (NULL != drmInfoRequest)
    {
        int type = drmInfoRequest->getInfoType();
        String8 action = drmInfoRequest->get(DrmRequestType::KEY_ACTION);
        if (DrmUtil::sDDebug) ALOGD("DrmMtkPlugIn::onAcquireDrmInfo : execute action %s", action.string());

        // "SET_DRM_INFO" is used when it requires plugin to make some opeartion with given information
        if (DrmRequestType::TYPE_SET_DRM_INFO == type)
        {
            String8 result; // the result of the operation
            if (DrmRequestType::ACTION_INSTALL_DRM_MSG == action)  //should remove after download check in.
            {
                // shall be an valid path
                String8 dmPath = drmInfoRequest->get(DrmRequestType::KEY_DATA);
                result = DrmMtkUtil::installDrmMsg(dmPath) ?
                    DrmRequestType::RESULT_SUCCESS : DrmRequestType::RESULT_FAILURE;
            }

            //M: Add to avoid writing sdcard in native layer, guo, need remove
            String8 ACTION_INSTALL_DRM_MSG_BY_FD("installDrmMsgByFd");
            if(ACTION_INSTALL_DRM_MSG_BY_FD == action)
            {
                if (DrmUtil::sDDebug) ALOGD("DrmMtkPlugIn::onAcquireDrmInfo() install drm msg by fd");
                String8  dm_fd_str = drmInfoRequest->get(DrmRequestType::KEY_DM_FD);
                String8  dcf_fd_str = drmInfoRequest->get(DrmRequestType::KEY_DCF_FD);
                int dm_fd = -1;
                int dcf_fd = -1;
                //dm_fd = atoi(dm_fd_str.string());
                sscanf(dm_fd_str.string(),"%d",&dm_fd);
                //dcf_fd = atoi(dcf_fd_str.string());
                sscanf(dcf_fd_str.string(),"%d",&dcf_fd);
                if (DrmUtil::sDDebug) ALOGD("dm_fd : %s, dcf_fd : %s",dm_fd_str.string(),dcf_fd_str.string());
                if (DrmUtil::sDDebug) ALOGD("dm_fd : %d, dcf_fd : %d",dm_fd,dcf_fd);
                result =
                        DrmMtkUtil::installDrmMsg(dm_fd,dcf_fd) ?
                                DrmRequestType::RESULT_SUCCESS : DrmRequestType::RESULT_FAILURE;
            }

            if (DrmRequestType::ACTION_CONSUME_RIGHTS == action)
            {
                // shall be valid path and valid action
                String8 path = drmInfoRequest->get(DrmRequestType::KEY_DATA);
                String8 drmAction = drmInfoRequest->get(DrmRequestType::KEY_DATA_EXTRA_1);
                int actionType = atoi(drmAction.string());

                String8 uri = String8("file://") + path;
                sp<DecryptHandle> handle = new DecryptHandle();
                status_t res = onOpenDecryptSession(uniqueId, handle, uri.string());
                if (DRM_NO_ERROR == res)
                {
                    res = onConsumeRights(uniqueId, handle, actionType, false);
                    onCloseDecryptSession(uniqueId, handle);
                }

                result = (DRM_NO_ERROR == res) ?
                    DrmRequestType::RESULT_SUCCESS : DrmRequestType::RESULT_FAILURE;
            }

            // TODO OMA DRM Roadmap new added, other must be remove later {@
            if (DrmRequestType::ACTION_SHOW_DRM_DIALOG_IF_NEED == action) {
                int fd = atol(drmInfoRequest->get(DrmRequestType::KEY_FILEDESCRIPTOR).string());
                DrmMetadata* pMetaData = new DrmMetadata();
                int parseResult = -4; // error code, 0 is succuss
                if (-1 == lseek(fd, 0, SEEK_SET)) {
                    int length = atol(drmInfoRequest->get(DrmRequestType::KEY_DATA).string());
                    char* data = (char*) mmap(NULL, length, PROT_READ, MAP_SHARED, fd, 0);
                    if (data != MAP_FAILED) {
                        DrmBuffer drmBuffer(data, length);
                        parseResult = DrmUtil::parseDcf(drmBuffer, pMetaData);
                        munmap(data, length);
                    } else {
                        if (DrmUtil::sDDebug) ALOGD("onAcquireDrmInfo can't map shared fd with error: %s", strerror(errno));
                    }
                } else {
                    parseResult = DrmUtil::parseDcf(fd, pMetaData);
                }

                if (parseResult == 0) {
                    // Check rights status, when status is invalid, all can not open decryption success
                    // except decode image just for bound(height and width).
                    String8 mimeType = pMetaData->get(String8(DrmMetaKey::META_KEY_MIME));
                    int action = Action::PLAY;
                    if (0 == strncasecmp(mimeType.string(), "image/", 6)) {
                        action = Action::DISPLAY;
                    }
                    String8 method = pMetaData->get(String8(DrmMetaKey::META_KEY_METHOD));
                    String8 cid = pMetaData->get(String8(DrmMetaKey::META_KEY_CONTENT_URI));
                    String8 rightsIssuer = pMetaData->get(String8(DrmMetaKey::META_KEY_RIGHTS_ISSUER));
                    int status = DrmUtil::checkRightsStatus(method, cid, action);
                    if (status != RightsStatus::RIGHTS_VALID) {
                        String8 path = DrmUtil::getPathFromFd(fd);
                        ALOGD("onOpenDecryptSession() : start drm dialog for filepath: %s",
                              path.string());
                        startDrmDialogService(String16(cid), String16(rightsIssuer), String16(method), status,
                                              String16(path));
                        if (DrmUtil::sDDebug) ALOGD("onOpenDecryptSession() : dialog for file: %s", cid.string());
                    }
                }
                delete pMetaData;
                result = DrmRequestType::RESULT_SUCCESS;
            } else if (DrmRequestType::ACTION_INSTALL_DRM_TO_DEVICE == action) {
                // get dm path(need open it as fd) and dcf fd
                int dmFd = open(drmInfoRequest->get(DrmRequestType::KEY_DATA).string(), O_RDWR);
                int dcfFd = atol(drmInfoRequest->get(DrmRequestType::KEY_FILEDESCRIPTOR).string());
                result = DrmMtkUtil::installDrmMsg(dmFd, dcfFd) ? DrmRequestType::RESULT_SUCCESS
                        : DrmRequestType::RESULT_FAILURE;
                if (dmFd >= 0) {
                    close(dmFd); // close dm fd if open success
                }
            } else if (DrmRequestType::ACTION_MARK_AS_CONSUME_IN_APP_CLIENT == action) {
                pid_t pid = atoi(drmInfoRequest->get(DrmRequestType::KEY_DATA_1).string());
                String8 procName = DrmMtkUtil::getProcessName(pid);
                String8 cid = drmInfoRequest->get(DrmRequestType::KEY_DATA_2);
                if (DrmUtil::sDDebug) ALOGD("DrmMtkPlugIn::onAcquireDrmInfo : mark %s as consume in app client for %s", procName.string(), cid.string());
                bool ret = DrmMtkUtil::markAsConsumeInAppClient(procName, cid);
                result = ret ? DrmRequestType::RESULT_SUCCESS : DrmRequestType::RESULT_FAILURE;
            } else if (DrmRequestType::ACTION_SAVE_SECURE_TIME == action) {
                String8 offsetStr = drmInfoRequest->get(DrmRequestType::KEY_DATA);
                long offset = atol(offsetStr.string());
                if (DrmUtil::sDDebug) ALOGD("DrmMtkPlugIn::onAcquireDrmInfo : update clock with offset %ld", offset);

                if (offset == INVALID_OFFSET) {
                    if (DrmUtil::sDDebug) ALOGD("DrmMtkPlugIn::onAcquireDrmInfo - invalid offset, do not update clock");
                    SecureTimer::instance().reset();
                    SecureTimer::instance().save();
                    result = DrmRequestType::RESULT_FAILURE;
                } else {
                    int res = SecureTimer::instance().updateDRMTime((time_t)offset);
                    result = (SecureTimerHelper::NTP_SYNC_SUCCESS == res) ?
                            DrmRequestType::RESULT_SUCCESS : DrmRequestType::RESULT_FAILURE;
                }
            } else if (DrmRequestType::ACTION_CHECK_RIGHTS_STATUS_BY_FD == action) {
                int fd = atol(drmInfoRequest->get(DrmRequestType::KEY_FILEDESCRIPTOR).string());
                DrmMetadata* pMetaData = new DrmMetadata();
                int status = RightsStatus::RIGHTS_INVALID;
                int parseResult = DrmUtil::parseDcf(fd, pMetaData);
                if (parseResult == 0) { // parse success
                    String8 mimeType = pMetaData->get(String8(DrmMetaKey::META_KEY_MIME));
                    int action = Action::PLAY;
                    if (0 == strncasecmp(mimeType.string(), "image/", 6)) {
                        action = Action::DISPLAY;
                    }
                    String8 method = pMetaData->get(String8(DrmMetaKey::META_KEY_METHOD));
                    String8 cid = pMetaData->get(String8(DrmMetaKey::META_KEY_CONTENT_URI));
                    String8 rightsIssuer = pMetaData->get(String8(DrmMetaKey::META_KEY_RIGHTS_ISSUER));
                    status = DrmUtil::checkRightsStatus(method, cid, action);
                }
                delete pMetaData;
                char statusStr[32] = {0};
                sprintf(statusStr, "%d", status);
                result = String8(statusStr);
                if (DrmUtil::sDDebug) ALOGD("DrmMtkPlugIn::onAcquireDrmInfo : check rightsStatus[%d] by fd[%d]", status, fd);
            }

            if (DrmRequestType::ACTION_UPDATE_OFFSET == action)
            {
                int res = SecureTimer::instance().updateOffset();
                result = (SecureTimer::RESULT_OK == res) ?
                    DrmRequestType::RESULT_SUCCESS : DrmRequestType::RESULT_FAILURE;
            }

            if (DrmRequestType::ACTION_SAVE_DEVICE_ID == action)
            {
                String8 idValue = drmInfoRequest->get(DrmRequestType::KEY_DATA);
                if (DrmUtil::sDDebug) ALOGD("DrmMtkPlugIn::onAcquireDrmInfo : save device id %s", idValue.string());
                int res = DrmUtil::saveId(idValue);
                result = (0 == res) ?
                    DrmRequestType::RESULT_SUCCESS : DrmRequestType::RESULT_FAILURE;
            }

            if (DrmUtil::sDDebug) ALOGD("DrmMtkPlugIn::onAcquireDrmInfo : result of SET_DRM_INFO operation %s", result.string());
            // the result value is passed back to caller side.
            int length = result.length();
            char* data = new char[length];
            memcpy(data, result.string(), length);

            drmInfo = new DrmInfo(drmInfoRequest->getInfoType(),
                DrmBuffer(data, length), drmInfoRequest->getMimeType());
        }

        // "GET_DRM_INFO" is used when that it need to get information for further use
        else if (DrmRequestType::TYPE_GET_DRM_INFO == type)
        {
            String8 result; // the result it had get for caller side.
            if (DrmRequestType::ACTION_CHECK_SECURE_TIME == action)
            {
                //Fix the issue that sometimes secure timer is invalid.
                //but SecureTimer::instance().isValid() still return valid.
                time_t t = 0;
                int isValid = SecureTimer::instance().getDRMTime(t);
                if (SecureTimer::CLOCK_VALID == isValid)
                {
                    result.setTo("valid");
                } else
                {
                    result.setTo("invalid");
                }
                /*bool isValid = SecureTimer::instance().isValid();
                result = isValid ? String8("valid") : String8("invalid");*/
                if (DrmUtil::sDDebug) ALOGD("DrmMtkPlugIn::onAcquireDrmInfo : check secure timer status %s", result.string());
            }

            if (DrmRequestType::ACTION_LOAD_DEVICE_ID == action)
            {
                result = DrmUtil::readIMEI();
                if (DrmUtil::sDDebug) ALOGD("DrmMtkPlugIn::onAcquireDrmInfo : load device id %s", result.string());
            }

            if (DrmUtil::sDDebug) ALOGD("DrmMtkPlugIn::onAcquireDrmInfo : result of GET_DRM_INFO operation %s", result.string());
            // the result value is passed back to caller side.
            int length = result.length();
            char* data = new char[length];
            memcpy(data, result.string(), length);

            drmInfo = new DrmInfo(drmInfoRequest->getInfoType(),
                DrmBuffer(data, length), drmInfoRequest->getMimeType());
        }

        else // when the type is not supported by this plug-in
        {
            String8 dataString("dummy_acquistion_string");
            int length = dataString.length();
            char* data = new char[length];
            memcpy(data, dataString.string(), length);

            drmInfo = new DrmInfo(drmInfoRequest->getInfoType(),
                    DrmBuffer(data, length), drmInfoRequest->getMimeType());
        }
    }
    return drmInfo;
}

// returns an empty string if it fails.
String8 DrmMtkPlugIn::onGetOriginalMimeType(int uniqueId, const String8& path, int fd)
{
    if (DrmUtil::sDDebug) ALOGD("onGetOriginalMimeType() [%d]", uniqueId);

    DrmMetadata* drmMetadata = onGetMetadata(uniqueId, &path);
    if (drmMetadata == NULL)
    {
        ALOGE("onGetOriginalMimeType() : error, failed to get metadata.");
        return String8("");
    }
    String8 mime = drmMetadata->get(String8(DrmMetaKey::META_KEY_MIME));
    delete drmMetadata;

    if (mime.isEmpty())
    {
        ALOGE("onGetOriginalMimeType() : error, failed to get mime type.");
        return String8("");
    }
    if (DrmUtil::sDDebug) ALOGD("onGetOriginalMimeType() : result [%s]", mime.string());
    return DrmMtkUtil::toCommonMime(mime.string());
}

// check with mime type by priority.
int DrmMtkPlugIn::onGetDrmObjectType(int uniqueId, const String8& path,
        const String8& mimeType)
{
    if (DrmUtil::sDDebug) ALOGD("onGetDrmObjectType() [%d]", uniqueId);

    if (mimeType.length() > 0)
    {
        if (mimeType == DrmDef::MIME_RIGHTS_XML
            || mimeType == DrmDef::MIME_RIGHTS_WBXML)
        {
            return DrmObjectType::RIGHTS_OBJECT;
        }
        else if (mimeType == DrmDef::MIME_DRM_CONTENT
                 || mimeType == DrmDef::MIME_DRM_MESSAGE)
        {
            return DrmObjectType::CONTENT;
        }
    }

    // if mimeType not available then we check the path suffix
    if (path.length() > 0 && onCanHandle(uniqueId, path))
    {
        String8 ext = path.getPathExtension();
        ext.toLower();
        if (ext == DrmDef::EXT_RIGHTS_XML || ext == DrmDef::EXT_RIGHTS_WBXML)
        {
            return DrmObjectType::RIGHTS_OBJECT;
        }
        else if (ext == DrmDef::EXT_DRM_CONTENT
                 || ext == DrmDef::EXT_DRM_MESSAGE)
        {
            return DrmObjectType::CONTENT;
        }
    }

    return DrmObjectType::UNKNOWN;
}

int DrmMtkPlugIn::onCheckRightsStatus(int uniqueId, const String8& path,
        int action)
{
    if (DrmUtil::sDDebug) ALOGD("onCheckRightsStatus() [%d]", uniqueId);

    int result = RightsStatus::RIGHTS_INVALID;
    DrmMetadata* drmMetadata = onGetMetadata(uniqueId, &path);
    if (drmMetadata == NULL)
    {
        ALOGE("onCheckRightsStatus() : error, failed to get metadata.");
        return RightsStatus::RIGHTS_INVALID;
    }
    String8 method = drmMetadata->get(String8(DrmMetaKey::META_KEY_METHOD));
    String8 cid = drmMetadata->get(String8(DrmMetaKey::META_KEY_CONTENT_URI));
    // If action is Action::DEFAULT, use really actions: image->DISPLAY, audio/video->PLAY
    if (Action::DEFAULT == action) {
        String8 mimeType = drmMetadata->get(String8(DrmMetaKey::META_KEY_MIME));
        action = Action::PLAY;
        if (0 == strncasecmp(mimeType.string(), "image/", 6)) {
            action = Action::DISPLAY;
        }
    }
    delete drmMetadata;

    result = DrmUtil::checkRightsStatus(method, cid, action);
    if (DrmUtil::sDDebug) ALOGD("onCheckRightsStatus() : result [%d]", result);
    return result;
}

status_t DrmMtkPlugIn::onConsumeRights(int uniqueId,
        sp<DecryptHandle>& decryptHandle, int action, bool reserve)
{
    if (DrmUtil::sDDebug) ALOGD("onConsumeRights() [%d]", uniqueId);
    // "Forward Lock Only"
    if (m_bFwdLockOnly) {
        if (DrmUtil::sDDebug) ALOGD("onConsumeRights() : Forward-lock-only is set, and don't consume any rights.");
        return DRM_NO_ERROR;
    }

    if (!reserve) // if shall not reserve
    {
        if (!SecureTimer::instance().isValid())
        {
            ALOGE("onConsumeRights() : invalid secure timer.");
            return DRM_ERROR_UNKNOWN;
        }

        if (!decryptHandle)
        {
            ALOGE("onConsumeRights() : invalid decrypt handle.");
            return DRM_ERROR_UNKNOWN;
        }

        DecryptSession* pDecryptSession =
            m_decryptSessionMap.getValue(decryptHandle->decryptId);
        if (NULL == pDecryptSession)
        {
            ALOGE("onConsumeRights() : invalid decrypt session.");
            return DRM_ERROR_UNKNOWN;
        }

        int fd = pDecryptSession->m_fileDesc;
        DrmMetadata* pMetaData = new DrmMetadata();

        // Add for Decode image with open decrypt seesion with ashmem {@
        int parseResult = -1;
        if (NULL != pDecryptSession && pDecryptSession->m_data != NULL) {
            DrmBuffer drmBuffer((char*)pDecryptSession->m_data, pDecryptSession->m_dataLength);
            parseResult = DrmUtil::parseDcf(drmBuffer, pMetaData);
        } else {
            parseResult = DrmUtil::parseDcf(fd, pMetaData);
        }
        // @}

        if (parseResult < 0)
        {
            ALOGE("onConsumeRights() : failed to parse dcf file.");
            delete pMetaData;
            return DRM_ERROR_CANNOT_HANDLE;
        }
        String8 method = pMetaData->get(String8(DrmMetaKey::META_KEY_METHOD));
        String8 cid = pMetaData->get(String8(DrmMetaKey::META_KEY_CONTENT_URI));
        String8 mimeType = pMetaData->get(String8(DrmMetaKey::META_KEY_MIME));
        delete pMetaData;

        // If mark as consume in app client(only for video and image, keep same logic with
        // OmaDrmUtils.java), only consume rights when app request @{
        if (NAME_NOT_FOUND != decryptHandle->extendedData.indexOfKey(String8("clientProcName"))
            && (0 == strncasecmp(mimeType.string(), "video/", 6)
                || 0 == strncasecmp(mimeType.string(), "image/", 6))) {
            String8 procName = decryptHandle->extendedData.valueFor(String8("clientProcName"));
            if (DrmMtkUtil::isDrmConsumeInAppClient(procName) && !DrmMtkUtil::isNeedConsume(cid)) {
                if (DrmUtil::sDDebug) ALOGD("onConsumeRights: don't consume rights this time for [%s], control by itself",
                        procName.string());
                return DRM_NO_ERROR;
            }
        }
        //@}
        if (!DrmUtil::consumeRights(method, cid, action))
        {
            ALOGE("onConsumeRights() : failed to consume rights.");
            return DRM_ERROR_UNKNOWN;
        }
    }

    if (DrmUtil::sDDebug) ALOGD("onConsumeRights() : done.");
    return DRM_NO_ERROR;
}

status_t DrmMtkPlugIn::onSetPlaybackStatus(int uniqueId,
        sp<DecryptHandle>& decryptHandle, int playbackStatus, int64_t position)
{
    if (DrmUtil::sDDebug) ALOGD("DrmMtkPlugIn::onSetPlaybackStatus() : %d, playbackStatus : %d", uniqueId, playbackStatus);
    // We check rights when creating DecryptHandle, and we don't check rights when playing
    // so looks like we don't have to implement this method
    return DRM_NO_ERROR;
}

bool DrmMtkPlugIn::onValidateAction(int uniqueId, const String8& path,
        int action, const ActionDescription& description)
{
    if (DrmUtil::sDDebug) ALOGD("onValidateAction() [%d]", uniqueId);
    return (onCheckRightsStatus(uniqueId, path, action)
            == RightsStatus::RIGHTS_VALID);
}

status_t DrmMtkPlugIn::onRemoveRights(int uniqueId, const String8& path)
{
    if (DrmUtil::sDDebug) ALOGD("onRemoveRights() [%d]", uniqueId);
    Mutex::Autolock lock(DrmUtil::mROLock);

    if (-1 == remove(path.string()))
    {
        ALOGE("onRemoveRights() : error, failed to remove RO file.");
        return DRM_ERROR_UNKNOWN;
    }

    return DRM_NO_ERROR;
}

status_t DrmMtkPlugIn::onRemoveAllRights(int uniqueId)
{
    if (DrmUtil::sDDebug) ALOGD("onRemoveAllRights() [%d]", uniqueId);
    Mutex::Autolock lock(DrmUtil::mROLock);

    bool isSuccess = true;
    // check RO folder existence
    DIR* pdir = opendir((DrmDef::RO_DIRECTORY).string());
    if (NULL == pdir)
    {
        ALOGW("onRemoveAllRights() : RO directory doesn't exist. No need to remove.");
        return DRM_NO_ERROR;
    }
    else
    {
        struct dirent* pent = NULL;
        while (NULL != (pent = readdir(pdir)))
        {
            String8 file(pent->d_name);
            if (file != "." && file != "..")
            {
                String8 f = DrmDef::RO_DIRECTORY + file;
                if (-1 == remove(f.string()))
                {
                    ALOGE("onRemoveAllRights() : error, failed to remove RO file.");
                    isSuccess = false;
                }
            }
        }
    }

    closedir(pdir);
    return isSuccess ? DRM_NO_ERROR : DRM_ERROR_UNKNOWN;
}

status_t DrmMtkPlugIn::onOpenConvertSession(int uniqueId, int convertId)
{
    if (DrmUtil::sDDebug) ALOGD("DrmMtkPlugIn::onOpenConvertSession() : %d", uniqueId);
    return DRM_NO_ERROR;
}

DrmConvertedStatus* DrmMtkPlugIn::onConvertData(int uniqueId, int convertId,
        const DrmBuffer* inputData)
{
    if (DrmUtil::sDDebug) ALOGD("DrmMtkPlugIn::onConvertData() : %d", uniqueId);
    DrmBuffer* convertedData = NULL;

    if (NULL != inputData && 0 < inputData->length)
    {
        int length = inputData->length;
        char* data = NULL;
        data = new char[length];
        convertedData = new DrmBuffer(data, length);
        memcpy(convertedData->data, inputData->data, length);
    }
    return new DrmConvertedStatus(DrmConvertedStatus::STATUS_OK, convertedData,
            0 /*offset*/);
}

DrmConvertedStatus* DrmMtkPlugIn::onCloseConvertSession(int uniqueId,
        int convertId)
{
    if (DrmUtil::sDDebug) ALOGD("DrmMtkPlugIn::onCloseConvertSession() : %d", uniqueId);
    return new DrmConvertedStatus(DrmConvertedStatus::STATUS_OK, NULL, 0 /*offset*/);
}

DrmSupportInfo* DrmMtkPlugIn::onGetSupportInfo(int uniqueId)
{
    if (DrmUtil::sDDebug) ALOGD("DrmMtkPlugIn::onGetSupportInfo : %d", uniqueId);
    DrmSupportInfo* drmSupportInfo = new DrmSupportInfo();
    drmSupportInfo->addMimeType(DrmDef::MIME_DRM_CONTENT);
    drmSupportInfo->addFileSuffix(DrmDef::EXT_DRM_CONTENT);
    drmSupportInfo->addMimeType(DrmDef::MIME_DRM_MESSAGE);
    drmSupportInfo->addFileSuffix(DrmDef::EXT_DRM_MESSAGE);
    drmSupportInfo->addMimeType(DrmDef::MIME_RIGHTS_XML);
    drmSupportInfo->addFileSuffix(DrmDef::EXT_RIGHTS_XML);
    drmSupportInfo->addMimeType(DrmDef::MIME_RIGHTS_WBXML);
    drmSupportInfo->addFileSuffix(DrmDef::EXT_RIGHTS_WBXML);
    drmSupportInfo->setDescription(DrmDef::PLUGIN_DESCRIPTION);
    return drmSupportInfo;
}

// open a decrypt session for reading content from protected files.
// returns DRM_NO_ERROR if the session is successfully created.
// for this api we do not use parameter {offset} & {length}.
// you may specify them as 0.
status_t DrmMtkPlugIn::onOpenDecryptSession(int uniqueId,
        sp<DecryptHandle>& decryptHandle, int fd, off64_t offset, off64_t length)
{
    if (DrmUtil::sDDebug) ALOGD("onOpenDecryptSession() [%d], Start with file descryptor", uniqueId);


    if (fd < 0)
    {
        ALOGE("onOpenDecryptSession() : invalid input file descriptor [%d]", fd);
        return DRM_ERROR_CANNOT_HANDLE;
    }

    // Add for Decode image with open decrypt session with ashmem {@
    if (-1 == lseek(fd, 0, SEEK_SET)) {
        if (DrmUtil::sDDebug) ALOGD("onOpenDecryptSession() : using shared memory fd");
        char* data = (char*) mmap(NULL, length, PROT_READ, MAP_SHARED, fd, 0);
        if (data == MAP_FAILED) {
            ALOGE("onOpenDecryptSession() : can't map shared fd with error: %s", strerror(errno));
            return DRM_ERROR_UNKNOWN;
        }
        if (DrmUtil::sDDebug) ALOGD("onOpenDecryptSession() : using shared memory fd mapped memory: %p", data);
        DrmBuffer drmBuffer(data, length);
        status_t result = openDecryptSessionWithBuffer(uniqueId, decryptHandle, drmBuffer, offset, String8());
        return result;
    }
    // @}

    status_t result = DRM_ERROR_CANNOT_HANDLE;
    if (decryptHandle)
    {
        DrmMetadata* pMetaData = new DrmMetadata();
        int parseResult = DrmUtil::parseDcf(fd, pMetaData);
        if (parseResult < 0)
        {
            if (DrmUtil::sDDebug) ALOGE("onOpenDecryptSession(fd) : failed to parse dcf file.");
            delete pMetaData;
            return DRM_ERROR_CANNOT_HANDLE;
        }

        // Forward Lock Only
        String8 method = pMetaData->get(String8(DrmMetaKey::META_KEY_METHOD));
        if (m_bFwdLockOnly && method != StrUtil::toString(DrmDef::METHOD_FL))
        {
            if (DrmUtil::sDDebug) ALOGD("onOpenDecryptSession() : Forward-lock-only is set, and it's not FL type");
            delete pMetaData;
            return DRM_ERROR_CANNOT_HANDLE;
        }

        // Check rights status, when status is invalid, all can not open decryption success
        // except decode image just for bound(height and width).
        String8 mimeType = pMetaData->get(String8(DrmMetaKey::META_KEY_MIME));
        int action = Action::PLAY;
        if (0 == strncasecmp(mimeType.string(), "image/", 6)) {
            action = Action::DISPLAY;
        }
        String8 cid = pMetaData->get(String8(DrmMetaKey::META_KEY_CONTENT_URI));
        int status = DrmUtil::checkRightsStatus(method, cid, action);

        decryptHandle->mimeType = mimeType;
        decryptHandle->decryptApiType = DecryptApiType::CONTAINER_BASED;
        decryptHandle->status = status;

        // Get dcf key and data offset/lenght
        ByteBuffer drmkey(DrmUtil::getDcfDrmKey(pMetaData));
        int dataOffset = atoi(pMetaData->get(String8(DrmMetaKey::META_KEY_OFFSET)).string());
        int dataLength = atoi(pMetaData->get(String8(DrmMetaKey::META_KEY_DATALEN)).string());
        delete pMetaData;

        if (DrmUtil::sDDebug) ALOGD("onOpenDecryptSession() : status=%d, method=%s, offset=%lld", status, method.string(), offset);
        if (status != RightsStatus::RIGHTS_VALID)
        {
            if (DrmUtil::sDDebug) ALOGD("onOpenDecryptSession() : invalid right for drm file.");
            // Check decode action when invalid, get action from offset parameter with flag 0xfffffff,
            // if ACTION_JUST_DECODE_BOUND return decrypt session to let app can decode bound
            int decodeAction = int (offset - 0xfffffff);
            if (decodeAction == ACTION_JUST_DECODE_BOUND) {
                if (1 == strncasecmp(mimeType.string(), "image/", 6)) {
                    if (DrmUtil::sDDebug) ALOGD("onOpenDecryptSession: only support just decode bound case when rights invalid for image");
                    return DRM_ERROR_CANNOT_HANDLE;
                }
            } else {
                return DRM_ERROR_CANNOT_HANDLE;
            }
        }

        decryptHandle->decryptInfo = new DecryptInfo();
        decryptHandle->decryptInfo->decryptBufferLength = dataLength;

        if (drmkey.length() == 0)
        {
            ALOGE("onOpenDecryptSession() : invalid drm key.");
            return DRM_ERROR_CANNOT_HANDLE;
        }

        // the key buffer
        BYTE key[DrmDef::DRM_MAX_KEY_LENGTH];
        bzero(key, sizeof(key));
        memcpy(key, drmkey.buffer(), drmkey.length());

        // check decrypt session map
        int fileDesc = -1;
        if (!m_decryptSessionMap.isCreated(decryptHandle->decryptId))
        {
            // use the file descriptor: the original raw fd is owned by binder,
            // shall use dup() to get our own copy.
            fileDesc = fcntl(fd, F_DUPFD, 0);
            if (-1 == fileDesc)
            {
                ALOGE("onOpneDecryptSession() : failed to duplicate the fd, reason [%s]",
                        strerror(errno));
                return DRM_ERROR_CANNOT_HANDLE;
            }
        }
        else
        {
            ALOGE("onOpenDecryptSession() : decrypt session map error.");
            return DRM_ERROR_CANNOT_HANDLE;
        }

        lseek(fileDesc, dataOffset, SEEK_SET);

        // add to decrypt session map
        DecryptSession* pDecryptSession = new DecryptSession(fileDesc, dataOffset, dataLength, key);
        m_decryptSessionMap.addValue(decryptHandle->decryptId, pDecryptSession);
        result = DRM_NO_ERROR;
    }
    else
    {
        ALOGE("onOpenDecryptSession() : invalid decryptHandle [%p]", decryptHandle.get());
    }

    if (DrmUtil::sDDebug) ALOGD("onOpenDecryptSession() : result [%d]", result);
    return result;
}

// the uri should be begin with "file://"
status_t DrmMtkPlugIn::onOpenDecryptSession(int uniqueId,
        sp<DecryptHandle>& decryptHandle, const char* uri)
{
    if (DrmUtil::sDDebug) ALOGD("onOpenDecryptSession() [%d], Start with uri [%s]", uniqueId, uri);


    status_t result = DRM_ERROR_CANNOT_HANDLE;
    const char fileTag[] = "file://";
    if (decryptHandle && NULL != uri && strlen(uri) > sizeof(fileTag))
    {
        String8 uriTag = String8(uri);
        uriTag.toLower();

        // compare: 0 for all equal
        // check if the first 7 chars is "file://"
        if (0 != strncmp(uriTag.string(), fileTag, sizeof(fileTag) - 1))
        {
            ALOGI("onOpenDecryptSession() : uri does not start with file://");
            return DRM_ERROR_CANNOT_HANDLE;
        }
        // find the position of file-name in uri string.
        const char* filePath = strchr(uri + sizeof(fileTag) - 1, '/');
        if (NULL == filePath)
        {
            ALOGE("onOpenDecryptSession() : invalid file path.");
            return DRM_ERROR_CANNOT_HANDLE;
        }

        int fd = open(filePath, O_RDONLY);
        if (-1 == fd)
        {
            ALOGE("onOpenDecryptSession() : failed to open file to read, reason [%s]",
                    strerror(errno));
            return DRM_ERROR_CANNOT_HANDLE;
        }

        DrmMetadata* pMetaData = new DrmMetadata();
        int parseResult = DrmUtil::parseDcf(fd, pMetaData);
        if (parseResult < 0)
        {
            if (DrmUtil::sDDebug) ALOGE("onOpenDecryptSession(uri) : failed to parse dcf file.");
            delete pMetaData;
            close(fd);
            return DRM_ERROR_CANNOT_HANDLE;
        }

        // Forward Lock Only
        if (m_bFwdLockOnly)
        {
            String8 method = pMetaData->get(String8(DrmMetaKey::META_KEY_METHOD));
            if (method != StrUtil::toString(DrmDef::METHOD_FL))
            {
                if (DrmUtil::sDDebug) ALOGD("onOpenDecryptSession() : Forward-lock-only is set, and it's not FL type");
                delete pMetaData;
                close(fd);
                return DRM_ERROR_CANNOT_HANDLE;
            }
        }

        String8 data_offset =
            pMetaData->get(String8(DrmMetaKey::META_KEY_OFFSET));
        const char* pDataOffset = data_offset.string();
        int dataOffset = atoi(pDataOffset);

        String8 data_length =
            pMetaData->get(String8(DrmMetaKey::META_KEY_DATALEN));
        const char* pDataLength = data_length.string();
        int dataLength = atoi(pDataLength);

        decryptHandle->mimeType =
            pMetaData->get(String8(DrmMetaKey::META_KEY_MIME));
        decryptHandle->decryptApiType =
            DecryptApiType::CONTAINER_BASED;
        decryptHandle->status = RightsStatus::RIGHTS_VALID;

        decryptHandle->decryptInfo = new DecryptInfo();
        decryptHandle->decryptInfo->decryptBufferLength = dataLength;

        // get dcf key
        ByteBuffer drmkey(DrmUtil::getDcfDrmKey(pMetaData));
        if (drmkey.length() == 0)
        {
            ALOGE("onOpenDecryptSession() : invalid drm key.");
            delete pMetaData;
            close(fd);
            return DRM_ERROR_CANNOT_HANDLE;
        }

        delete pMetaData;

        BYTE key[DrmDef::DRM_MAX_KEY_LENGTH];
        bzero(key, sizeof(key));
        memcpy(key, drmkey.buffer(), drmkey.length());

        // check decrypt session map
        int fileDesc = -1;
        if (!m_decryptSessionMap.isCreated(decryptHandle->decryptId))
        {
            // use the file descriptor
            fileDesc = fd;
            if (DrmUtil::sDDebug) ALOGD("onOpenDecryptSession() : the file descriptor [%d] stored in session", fd);
        }
        else
        {
            ALOGE("onOpenDecryptSession() : decrypt session map error.");
            close(fd);
            return DRM_ERROR_CANNOT_HANDLE;
        }

        lseek(fileDesc, dataOffset, SEEK_SET);

        // add to decrypt session map
        DecryptSession* pDecryptSession =
            new DecryptSession(fileDesc, dataOffset, dataLength, key);
        m_decryptSessionMap.addValue(decryptHandle->decryptId, pDecryptSession);
        result = DRM_NO_ERROR;
    }
    else
    {
        ALOGE("onOpenDecryptSession() : invalid parameter: decryptHandle [%p], uri [%s]",
                decryptHandle.get(), uri);
    }

    if (DrmUtil::sDDebug) ALOGD("onOpenDecryptSession() : result [%d]", result);
    return result;
}

// Add for Decode image with open decrypt seesion with ashmem {@
status_t DrmMtkPlugIn::openDecryptSessionWithBuffer(int uniqueId, sp<DecryptHandle>& decryptHandle,
            const DrmBuffer& buffer, const off64_t offset, const String8& mimeType)
{
    if (DrmUtil::sDDebug) ALOGD("onOpenDecryptSession() [%d], Start with buffer length [%d]", uniqueId, buffer.length);
    status_t result = DRM_ERROR_CANNOT_HANDLE;

    if (decryptHandle && NULL != buffer.data && buffer.length > 0)
    {
        DrmMetadata* pMetaData = new DrmMetadata();
        int parseResult = DrmUtil::parseDcf(buffer, pMetaData);
        if (parseResult < 0)
        {
            ALOGE("onOpenDecryptSession() : failed to parse dcf file.");
            delete pMetaData;
            return DRM_ERROR_CANNOT_HANDLE;
        }

        // Forward Lock Only
        String8 method = pMetaData->get(String8(DrmMetaKey::META_KEY_METHOD));
        if (m_bFwdLockOnly && method != StrUtil::toString(DrmDef::METHOD_FL))
        {
            if (DrmUtil::sDDebug) ALOGD("onOpenDecryptSession(buffer) : Forward-lock-only is set, and it's not FL type");
            delete pMetaData;
            return DRM_ERROR_CANNOT_HANDLE;
        }

        // Check rights status, when status is invalid, all can not open decryption success
        // except decode image just for bound(height and width).
        String8 mimeType = pMetaData->get(String8(DrmMetaKey::META_KEY_MIME));
        int action = Action::PLAY;
        if (0 == strncasecmp(mimeType.string(), "image/", 6)) {
            action = Action::DISPLAY;
        }
        String8 cid = pMetaData->get(String8(DrmMetaKey::META_KEY_CONTENT_URI));
        int status = DrmUtil::checkRightsStatus(method, cid, action);

        decryptHandle->mimeType = mimeType;
        decryptHandle->decryptApiType = DecryptApiType::CONTAINER_BASED;
        decryptHandle->status = status;

        // Get dcf key and data offset/lenght
        ByteBuffer drmkey(DrmUtil::getDcfDrmKey(pMetaData));
        int dataOffset = atoi(pMetaData->get(String8(DrmMetaKey::META_KEY_OFFSET)).string());
        int dataLength = atoi(pMetaData->get(String8(DrmMetaKey::META_KEY_DATALEN)).string());
        delete pMetaData;

        if (DrmUtil::sDDebug) ALOGD("onOpenDecryptSession() : status=%d, method=%s, offset=%lld", status, method.string(), offset);
        if (status != RightsStatus::RIGHTS_VALID)
        {
            if (DrmUtil::sDDebug) ALOGD("onOpenDecryptSession() : invalid right for drm file.");
            // Check decode action when invalid, get action from offset parameter with flag 0xfffffff,
            // if ACTION_JUST_DECODE_BOUND return decrypt session to let app can decode bound
            int decodeAction = int (offset - 0xfffffff);
            if (decodeAction == ACTION_JUST_DECODE_BOUND) {
                if (1 == strncasecmp(mimeType.string(), "image/", 6)) {
                    if (DrmUtil::sDDebug) ALOGD("onOpenDecryptSession: only support image for just decode bound when rights invalid");
                    return DRM_ERROR_CANNOT_HANDLE;
                }
            } else {
                return DRM_ERROR_CANNOT_HANDLE;
            }
        }

        decryptHandle->decryptInfo = new DecryptInfo();
        decryptHandle->decryptInfo->decryptBufferLength = dataLength;

        if (drmkey.length() == 0)
        {
            ALOGE("onOpenDecryptSession() : invalid drm key.");
            return DRM_ERROR_CANNOT_HANDLE;
        }

        // the key buffer
        BYTE key[DrmDef::DRM_MAX_KEY_LENGTH];
        bzero(key, sizeof(key));
        memcpy(key, drmkey.buffer(), drmkey.length());

        // add to decrypt session map
        DecryptSession* pDecryptSession =
            new DecryptSession(buffer.data, dataOffset, dataLength, key);
        m_decryptSessionMap.addValue(decryptHandle->decryptId, pDecryptSession);
        result = DRM_NO_ERROR;
    }
    else
    {
        ALOGE("onOpenDecryptSession() : invalid parameter: decryptHandle [%p] dataLength:%d",
        decryptHandle.get(), buffer.length);
    }

    if (DrmUtil::sDDebug) ALOGD("onOpenDecryptSession() : result [%d]", result);
    return result;
}

// should call this after you've called opendecryptsession
// it always returns DRM_NO_ERROR
status_t DrmMtkPlugIn::onCloseDecryptSession(int uniqueId,
        sp<DecryptHandle>& decryptHandle)
{
    if (DrmUtil::sDDebug) ALOGD("onCloseDecryptSession() [%d]", uniqueId);
    if (decryptHandle)
    {
        // Add for Decode image with open decrypt seesion with ashmem {@
        DecryptSession* pDecryptSession = m_decryptSessionMap.getValue(decryptHandle->decryptId);
        if (NULL != pDecryptSession && pDecryptSession->m_data != NULL) {
            munmap(pDecryptSession->m_data, pDecryptSession->m_dataLength);
        }
        // @}

        m_decryptSessionMap.removeValue(decryptHandle->decryptId);
        if (NULL != decryptHandle->decryptInfo)
        {
            delete decryptHandle->decryptInfo;
            decryptHandle->decryptInfo = NULL;
        }
        decryptHandle = NULL;
    }

    return DRM_NO_ERROR;
}

status_t DrmMtkPlugIn::onInitializeDecryptUnit(int uniqueId,
        sp<DecryptHandle>& decryptHandle, int decryptUnitId,
        const DrmBuffer* headerInfo)
{
    if (DrmUtil::sDDebug) ALOGD("DrmMtkPlugIn::onInitializeDecryptUnit() : %d", uniqueId);
    return DRM_NO_ERROR;
}

status_t DrmMtkPlugIn::onDecrypt(int uniqueId, sp<DecryptHandle>& decryptHandle,
        int decryptUnitId, const DrmBuffer* encBuffer, DrmBuffer** decBuffer,
        DrmBuffer* IV)
{
    if (DrmUtil::sDDebug) ALOGD("DrmMtkPlugIn::onDecrypt() : %d", uniqueId);
    /**
     * As a workaround implementation passthru would copy the given
     * encrypted buffer as it is to decrypted buffer. Note, decBuffer
     * memory has to be allocated by the caller.
     */
    if (NULL != (*decBuffer) && 0 < (*decBuffer)->length)
    {
        memcpy((*decBuffer)->data, encBuffer->data, encBuffer->length);
        (*decBuffer)->length = encBuffer->length;
    }
    return DRM_NO_ERROR;
}

status_t DrmMtkPlugIn::onFinalizeDecryptUnit(int uniqueId,
        sp<DecryptHandle>& decryptHandle, int decryptUnitId)
{
    if (DrmUtil::sDDebug) ALOGD("DrmMtkPlugIn::onFinalizeDecryptUnit() : %d", uniqueId);
    return DRM_NO_ERROR;
}

// read some data from protected content. a valid decrypt handle and session is
// essential for this.
// the {offset} is the offset from the real beginning of the raw content
ssize_t DrmMtkPlugIn::onPread(int uniqueId, sp<DecryptHandle>& decryptHandle,
        void* buffer, ssize_t numBytes, off64_t offset)
{
    if (!decryptHandle || buffer == NULL || numBytes < 0 || offset < 0) {
        ALOGE("onPread[%d] with invalid parameter: numBytes[%d], offset[%ld]", uniqueId,
                numBytes, offset);
        return -1;
    }
    DecryptSession* pDecryptSession = m_decryptSessionMap.getValue(decryptHandle->decryptId);
    if (pDecryptSession == NULL) {
        ALOGE("onPread[%d] with invalid decrypt session", uniqueId);
        return -1;
    }
    ssize_t bytesRead = -1; // returns the actual bytes read.
    CryptoHelper cr_hlp(CryptoHelper::CIPHER_AES128CBC, pDecryptSession->m_key, 0);
    // Add for Decoding image with open decrypt seesion with ashmem {@
    if (pDecryptSession->m_data != NULL) { // share memoty data
        bytesRead = CipherFileUtil::CipherFileRead(
                (char*) (pDecryptSession->m_data), buffer, numBytes, offset,
                pDecryptSession->m_dataOffset,
                pDecryptSession->m_dataLength, cr_hlp);
    } else if (pDecryptSession->m_fileDesc > -1) { // fd data
        bytesRead = CipherFileUtil::CipherFileRead(
                pDecryptSession->m_fileDesc, buffer, numBytes, offset,
                pDecryptSession->m_dataOffset,
                pDecryptSession->m_dataLength, cr_hlp);
    } else { // error parameter, no ivalid fd or ashmem
        ALOGE("onPread with error parameter: m_fileDesc[%d] or m_data[%d] is invalid",
                pDecryptSession->m_fileDesc, pDecryptSession->m_data);
    }
    // @}
    return bytesRead;
}

