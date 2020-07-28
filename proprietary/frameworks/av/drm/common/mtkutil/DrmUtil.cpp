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
 * MediaTek Inc. (C) 2013. All rights reserved.
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

#define LOG_NDEBUG 0
#define LOG_TAG "DrmMtkUtil/DrmUtil"
#include <utils/Log.h>

#include <drm/drm_framework_common.h>
#include <drm/DrmMtkDef.h>
#include <drm/SecureTimer.h>

#include <RO.h>
#include <DrmDef.h>
#include <DrmTypeDef.h>
#include <ByteBuffer.h>
#include <CipherFileUtil.h>
#include <CryptoHelper.h>
#include <DrmUtil.h>
#include <FileUtil.h>
#include <StrUtil.h>
#include <WBUtil.h>

#include <libxml/parser.h>
#include <libxml/tree.h>
#include <openssl/md5.h>
#include <openssl/sha.h>
#include <openssl/hmac.h>
#include <openssl/evp.h>
#include <openssl/base64.h>
#include <openssl/bio.h>
#include <openssl/buffer.h>
#include <openssl/rc4.h>

#include <utils/List.h>
#include <utils/String8.h>
#include <utils/threads.h>

#include <ctype.h>
#include <dirent.h>
#include <fcntl.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <time.h>

#include <cutils/properties.h>

#define MAC_BYTES 6

#define LOG_TAG "DrmMtkUtil/DrmUtil"

using namespace android;

// ==== local constants defination =============================================

// the old style content-id length for FL, using 18-char signature, is something like:
// MTK_FL_CID:XXXXXXXXXXXXXXXXXX
// so we have a 28-char length string
// static const int OLD_STYLE_FL_CID_LENGTH = 28;

// static const int VER_01_0 = 10; // the value indicating Rights Object version
// static const char* RO_VERSION_01_0 = "VER_01_0"; // the version string tag, placed at the beginning of RO file
// static const int SIZE_VER_01_0  = 9; // the length of string tag

// constants for device id's .dat file.
const static String8 ID_LOCATION("/data/drm/");
const static String8 ID_DIR("/data/drm/id/");
const static String8 ID_FILE("/data/drm/id/id.dat");

// ==== local constants defination end =========================================

Mutex DrmUtil::mROLock;
Mutex DrmUtil::mDCFLock;
char value[PROPERTY_VALUE_MAX];
bool DrmUtil::sDDebug = (property_get("ro.build.type", value, NULL) && !strcasecmp(value, "eng")) ||
        property_get("vendor.debug.drm.logd", value, NULL) == 1;
bool DrmUtil::sVDebug = property_get("vendor.debug.drm.logv", value, NULL) == 1;

#if 0
static void convertRO(Rights* rights, OldRights* oldRights)
{
    bzero(rights->cid, sizeof(rights->cid));
    memcpy(rights->cid, oldRights->cid, sizeof(oldRights->cid));

    memcpy(rights->key, oldRights->key, sizeof(oldRights->key));
    rights->permission = oldRights->permission;
    memcpy(rights->num, oldRights->num, sizeof(oldRights->num));
    memcpy(rights->best, oldRights->best, sizeof(oldRights->best));
}


// upon success, returns the length of line it has found;
// or 0 indicating no end-of-line was found and the file position remains unchanged;
// -1 for error (invalid data)
static int get_line(int fd, char* str, size_t str_size)
{
    off_t old_pos = lseek(fd, 0, SEEK_CUR);
    if (DrmUtil::sVDebug) ALOGV("get_line: check next headers line at [%ld]", old_pos);

    if (-1 == read(fd, str, str_size))
    {
        ALOGE("get_line: failed to read data from fd[%d], reason [%s]", fd, strerror(errno));
        return -1;
    }

    // go through the buffer to find the first "end-of-line" charactor
    // the buffer shall have length at least DrmDef::DRM_MAX_META_LENGTH
    int i = 0;
    bool has_find = false;
    while ((size_t)i < str_size && str[i] != 0x00)
    {
        if (str[i] == 0x0a)
        {
            has_find = true;

            i++;
            old_pos += i; // move to the position to check next line
            break;
        }

        i++;
    }
    if (-1 == lseek(fd, old_pos, SEEK_SET)) {
        ALOGE("get_line: fd lseek error, reason [%s]", strerror(errno));
        return -1;
    }

    if (i == 0)
    {
        ALOGE("get_line: failed to find next headers line with invalid 0x00");
        return -1;
    }

    // clear the bytes after 0x0a
    memset(str + i, 0, str_size - (size_t)i);
    return has_find ? i : 0;
}
// Add for Decode image with open decrypt seesion with ashmem
static int get_line(char* data, char* str, size_t str_size)
{
    // go through the buffer to find the first "end-of-line" charactor
    // the buffer shall have length at least DrmDef::DRM_MAX_META_LENGTH
    int i = 0;
    bool has_find = false;
    memcpy(str, data, str_size);

    while ((size_t)i < str_size && str[i] != 0x00)
    {
        if (str[i] == 0x0a)
        {
            has_find = true;

            i++;
            break;
        }

        i++;
    }

    if (i == 0)
    {
        ALOGE("get_line: failed to find next headers line with invalid 0x00");
        return -1;
    }

    // clear the bytes after 0x0a
    memset(str + i, 0, str_size - (size_t)i);
    return has_find ? i : 0;
}

static int readUintVarFd(int fd)
{
    int value = 0;
    do
    {
        char c = 0;
        if (-1 == read(fd, &c, 1))
        {
            ALOGE("readUintVarFd: failed to read from file.");
            return -1;
        }

        value <<= 7;
        value |= (c & 0x7f);
        if ((c & 0x80) == 0)
        {
            break;
        }
    }
    while (true);
    return value;
}

// Add for Decode image with open decrypt seesion with ashmem
static int readUintVarBuffer(char* data, size_t* forward)
{
    *forward = 0;
    int value = 0;
    do
    {
        char c = 0;
        c = data[0];

        data = (char*)data + 1;
        (*forward) += 1;

        value <<= 7;
        value |= (c & 0x7f);
        if ((c & 0x80) == 0)
        {
            break;
        }
    }while (true);
    return value;
}
#endif

bool DrmUtil::checkDir(String8 dir)
{
    if (sVDebug) ALOGV("checkDir: [%s]", dir.string());
    struct stat stFileInfo;
    if (stat(dir.string(), &stFileInfo) == 0)
    {
        return true; // directory exist
    } else
    {
        if (sDDebug) ALOGD("checkDir: %s is not existence, then create it.", dir.string());
        CreateMulvPath((char*) dir.string());
        return true;
    }
}

bool DrmUtil::checkExistence(String8 filePath)
{
    if (sVDebug) ALOGV("checkExistence: [%s]", filePath.string());
    struct stat stFileInfo;
    if (stat(filePath.string(), &stFileInfo) == -1)
    {
        ALOGE("checkExistence failed for %s, reason [%s]", filePath.string(), strerror(errno));
        return false;
    }
    return true;
}

#if 0
// returns the RO file full path name which the name is the hash valud of content-id.
String8 DrmUtil::getROFileName(String8 cid)
{
    String8 hashValue = hash((char*)cid.string(), (int)cid.length());
    String8 ret = DrmDef::RO_DIRECTORY + hashValue;
    if (sVDebug) ALOGV("getROFileName: cid [%s] -> file name [%s]", cid.string(), ret.string());
    return ret;
}

// ro should be an clear ro
bool DrmUtil::restore(String8 cid, RO* ro)
{
    if (sVDebug) ALOGV("restore: cid[%s]", cid.string());

    String8 ro_file = getROFileName(cid);
    FILE* fp = NULL;
    if (!FileUtil::fopenx(ro_file.string(), "rb", &fp))
    {
        ALOGE("restore: failed to open ro file[%s].", ro_file.string());
        return false;
    }

    if (!restoreRights(fp, ro))
    {
        ALOGE("restore: failed to restore rights from ro file.");
        FileUtil::fclosex(fp);
        if (checkExistence(ro_file))
        {
            if (sDDebug) ALOGD("restore: delete existing ro file.");
            remove(ro_file.string());
        }
        return false;
    }

    if (!restoreEntry(ro_file, fp, ro))
    {
        ALOGE("restore: failed to restore entries from ro file.");
        FileUtil::fclosex(fp);
        if (checkExistence(ro_file))
        {
            if (sDDebug) ALOGD("restore: delete existing ro file.");
            remove(ro_file.string());
        }
        return false;
    }

    FileUtil::fclosex(fp);
    return true;
}

// overwrite original ro file
bool DrmUtil::save(RO* ro)
{
    if (sVDebug) ALOGV("save ---->");

    checkDir(DrmDef::RO_DIRECTORY);
    String8 ro_file = getROFileName(String8(ro->rights.cid));
    // save the {ro_file}_tmp file first
    String8 ro_file_tmp = ro_file + String8("_tmp");
    FILE* fp = NULL;
    if (!FileUtil::fopenx(ro_file_tmp.string(), "wb", &fp))
    {
        ALOGE("save: failed to open ro file to write.");
        return false;
    }

    chmod(ro_file_tmp.string(), 0770); // now on Jelly Bean, because of the mask, it will be 700

    if (!saveRights(fp, &ro->rights))
    {
        ALOGE("save: failed to save rights to ro file.");
        FileUtil::fclosex(fp);
        return false;
    }

    if (!saveEntry(fp, ro))
    {
        ALOGE("save: failed to save entries to ro file.");
        FileUtil::fclosex(fp);
        return false;
    }

    if (0 != fsync(fileno(fp)))
    {
        if (sDDebug) ALOGD("save: does not sync the file, reason [%s]", strerror(errno));
    }
    FileUtil::fclosex(fp);

    // rename {ro_file}_tmp to {ro_file}
    if (0 != rename(ro_file_tmp.string(), ro_file.string()))
    {
        ALOGE("save: failed to rename [%s], reason [%s]",
                ro_file_tmp.string(), strerror(errno));
        return false;
    }
    if (sVDebug) ALOGV("save <----");
    return true;
}

// merge with original ro file
bool DrmUtil::merge(RO* newRO)
{
    if (sVDebug) ALOGV("merge ---->");

    for (int i = DrmDef::PERMISSION_PLAY_INDEX;
         i <= DrmDef::PERMISSION_PRINT_INDEX; i++)
    {
        if (newRO->rights.num[i] != 0)
        {
            int new_entry_index = newRO->rights.best[i];
            while (new_entry_index != 0xFF)
            {
                Entry* new_entry = &newRO->rights.entryPtr[new_entry_index];
                if (!mergeEntry(newRO, new_entry_index, i))
                {
                    ALOGE("merge: failed to merge entry.");
                    return false;
                }
                new_entry_index = new_entry->next;
            }
        }
    }
    if (sVDebug) ALOGV("merge <----");
    return true;
}

int DrmUtil::checkRightsStatus(String8 method, String8 cid, int action)
{
    if (method.isEmpty() || cid.isEmpty())
    {
        ALOGE("checkRightsStatus: invalid method or cid.");
        return RightsStatus::RIGHTS_INVALID;
    }

    int result = RightsStatus::RIGHTS_INVALID;
    switch (action)
    {
        case Action::RINGTONE:
        case OmaDrmAction::WALLPAPER:
        {
            if (method == StrUtil::toString(DrmDef::METHOD_FL))
            {
                if (cid.length() <= OLD_STYLE_FL_CID_LENGTH)
                {
                    // old style content-id, GB2, GB, does not check device id (come from mota)
                    result = RightsStatus::RIGHTS_VALID;
                }
                else
                {
                    // now we need to check if the device Id matches.
                    String8 id = readId();
                    result = (0 == strncmp(id.string(), cid.string(), id.length()))
                             ? RightsStatus::RIGHTS_VALID : RightsStatus::RIGHTS_INVALID;
                }
            }
            else
            {
                result = RightsStatus::RIGHTS_INVALID;
            }
            break;
        }

        case Action::TRANSFER:
        {
            if (method == StrUtil::toString(DrmDef::METHOD_SD))
            {
                result = RightsStatus::RIGHTS_VALID;
            }
            else
            {
                result = RightsStatus::RIGHTS_INVALID;
            }
            break;
        }

        case OmaDrmAction::PRINT:
        {
            // OMA DRM could not print
            result = RightsStatus::RIGHTS_INVALID;
            break;
        }
        case Action::PLAY:
        case Action::DISPLAY:
        case Action::EXECUTE:
        {
            if (method == StrUtil::toString(DrmDef::METHOD_FL))
            {
                if (cid.length() <= OLD_STYLE_FL_CID_LENGTH)
                {
                    // old style content-id, GB2, GB, does not check device id (come from mota)
                    result = RightsStatus::RIGHTS_VALID;
                }
                else
                {
                    // now we need to check if the device Id matches.
                    String8 id = readId();
                    result = (0 == strncmp(id.string(), cid.string(), id.length()))
                             ? RightsStatus::RIGHTS_VALID : RightsStatus::RIGHTS_INVALID;
                }
            }
            else
            {
                // check secure timer
                if (!SecureTimer::instance().isValid())
                {
                    result = OmaDrmRightsStatus::SECURE_TIMER_INVALID;
                    ALOGE("checkRightsStatus: secure time is invalid");
                    break;
                }
                bool rightsValid = traverseRights(cid, DrmDef::getPermissionFromAction(action),
                        Mode::CheckRightsStatus);
                if (rightsValid)
                {
                    result = RightsStatus::RIGHTS_VALID;
                }
                else
                {
                    result = RightsStatus::RIGHTS_INVALID;
                }
            }
            break;
        }

        default:
        {
            ALOGW("checkRightsStatus: unsupport action.");
            result = RightsStatus::RIGHTS_INVALID;
            break;
        }
    }
    if (sDDebug) ALOGD("checkRightsStatus: method[%s], cid[%s], action[%d], rights status[%d]",
            method.string(), cid.string(), action, result);
    return result;
}

bool DrmUtil::consumeRights(String8 method, String8 cid, int action)
{
    if (method.isEmpty() || cid.isEmpty())
    {
        ALOGE("consumeRights: invalid method[%s] or cid[%s]", method.string(), cid.string());
        return false;
    }

    // check according to method
    if (method == StrUtil::toString(DrmDef::METHOD_FL))
    {
        return true;
    }

    bool result =
        traverseRights(cid, DrmDef::getPermissionFromAction(action), Mode::ConsumeRights);

    if (sDDebug) ALOGD("consumeRights: method[%s], cid[%s], action[%d], result[%d]",
            method.string(), cid.string(), action, result);
    return result;
}

bool DrmUtil::consumeRights(String8 path, int action)
{
    if (sDDebug) ALOGD("consumeRights: path[%s], action[%d]", path.string(), action);

    DrmMetadata* drmMetadata = new DrmMetadata();
    if (!parseDcf(path, drmMetadata))
    {
        ALOGE("consumeRights: failed to get metadata.");
        delete drmMetadata;
        return false;
    }

    String8 method = drmMetadata->get(String8(DrmMetaKey::META_KEY_METHOD));
    String8 cid = drmMetadata->get(String8(DrmMetaKey::META_KEY_CONTENT_URI));
    bool result = consumeRights(method, cid, action);

    delete drmMetadata;
    return result;
}

// only rights valid, then you can getConstraints
bool DrmUtil::getConstraints(String8 cid, int action, Entry& outEntry)
{
    if (cid.isEmpty())
    {
        ALOGE("getConstraints: invalid cid");
        return false;
    }

    bool result = traverseRights(cid, DrmDef::getPermissionFromAction(action),
            Mode::GetConstraints, outEntry);
    if (sDDebug) ALOGD("getConstraints: cid[%s], action[%d], result[%d]", cid.string(), action, result);
    return result;
}

bool DrmUtil::parse_dr(DrmBuffer& drmBuffer, RO* ro)
{
    if (sVDebug) ALOGV("parse_dr ---->");
    xmlDocPtr doc = getXmlDocPtr(drmBuffer);
    if (NULL == doc)
    {
        ALOGE("parse_dr: Loading xml file failed.");
        return false;
    }

    // get version, must be 1.0
    xmlNodePtr versionPtr = getNode(doc, DrmDef::PATH_VERSION);
    if (versionPtr == NULL)
    {
        ALOGE("parse_dr: parse version failed.");
        xmlFreeDoc(doc);
        xmlCleanupParser();
        return false;
    }
    char* value = (char*)xmlNodeGetContent(versionPtr);
    if (0 != strcmp(value, "1.0")) // not 1.0
    {
        ALOGE("parse_dr: version [%s] should be 1.0", value);
        xmlFree(value);
        xmlFreeDoc(doc);
        xmlCleanupParser();
        return false;
    }
    xmlFree(value);

    // get cid
    xmlNodePtr uidPtr = getNode(doc, DrmDef::PATH_UID);
    if (uidPtr == NULL)
    {
        ALOGE("parse_dr: parse cid failed.");
        xmlFreeDoc(doc);
        xmlCleanupParser();
        return false;
    }
    value = (char*)xmlNodeGetContent(uidPtr);
    String8 content_uri = StrUtil::getContentUri(String8(value));
    if (content_uri.length() == 0) {
        ALOGE("parse_dr: content_uri shall not be empty.");
        xmlFree(value);
        xmlFreeDoc(doc);
        xmlCleanupParser();
        return false;
    }
    content_uri = StrUtil::trimLRSpace(content_uri);
    if (sVDebug) ALOGV("parse_dr: cid[%s]", content_uri.string());
    snprintf(ro->rights.cid, sizeof(ro->rights.cid), "%s", content_uri.string());
    xmlFree(value);

    // get KeyValue, CD has no key
    xmlNodePtr keyvaluePtr = getNode(doc, DrmDef::PATH_KEYVALUE);
    if (keyvaluePtr == NULL)
    {
        if (sVDebug) ALOGV("parse_dr: No KeyValue, should be CD case.");
    }
    else
    {
        value = (char*)xmlNodeGetContent(keyvaluePtr);
        if (sVDebug) ALOGV("parse_dr: base64 encoded key value [%s]", value);
        // base64 decode key
        ByteBuffer encodedKey(value);
        ByteBuffer decodedKey(base64_decode_bytebuffer(encodedKey));
        if (sVDebug) ALOGV("parse_dr: decoded key value [%s]", decodedKey.buffer());
        if(decodedKey.buffer() == NULL)
        {
            ALOGE("decoded key failed");
            xmlFree(value);
            xmlFreeDoc(doc);
            xmlCleanupParser();
            return false;
        }
        memcpy(ro->rights.key, decodedKey.buffer(), DrmDef::DRM_MAX_KEY_LENGTH);
        xmlFree(value);
    }

    ro->rights.entryPtr = new Entry[DrmDef::PERMISSION_TOTAL_INDEX];
    // check  permission
    for (int i = DrmDef::PERMISSION_PLAY_INDEX;
         i <= DrmDef::PERMISSION_PRINT_INDEX; i++)
    {
        xmlNodePtr permissionPtr =
            getNode(doc, DrmDef::getPermissionPath(i));
        if (permissionPtr == NULL)
        {
            if (sDDebug) ALOGD("parse_dr: No permission[%s]", DrmDef::getPermissionStr(i).string());
            continue; // check next permission
        }

        ro->rights.num[i] = 1;
        ro->rights.best[i] = i;
        ro->rights.permission |= DrmDef::getPermission(i);
        ro->rights.entryPtr[i].type = DrmDef::CONSTRAINT_UNINIT;
        ro->rights.entryPtr[i].next = 0xFF;

        // check count
        xmlNodePtr countPtr =
            getNode(doc, DrmDef::getPermissionPath(i) + DrmDef::PATH_CONSTRAINT_COUNT);
        if (countPtr != NULL)
        {
            ro->rights.entryPtr[i].type |= DrmDef::CONSTRAINT_COUNT;
            xmlChar* count = xmlNodeGetContent(countPtr);
            long countLong = atol((char*)count);
            ro->rights.entryPtr[i].total_count = countLong;
            ro->rights.entryPtr[i].used_count = countLong;
            xmlFree(count);
        }

        // check datetime
        xmlNodePtr startPtr =
            getNode(doc, DrmDef::getPermissionPath(i) + DrmDef::PATH_CONSTRAINT_DATETIME_START);
        if (startPtr != NULL)
        {
            ro->rights.entryPtr[i].type |= DrmDef::CONSTRAINT_DATETIME;
            xmlChar* start = xmlNodeGetContent(startPtr);
            if (!StrUtil::datetimeStrToTick((char*)start, ro->rights.entryPtr[i].start_time))
            {
                ALOGE("parse_dr: DT[%s] is invalid", (char*)start);
                xmlFree(start);
                xmlFreeDoc(doc);
                xmlCleanupParser();
                return false;
            }
            xmlFree(start);
        }

        xmlNodePtr endPtr =
            getNode(doc, DrmDef::getPermissionPath(i) + DrmDef::PATH_CONSTRAINT_DATETIME_END);
        if (endPtr != NULL)
        {
            ro->rights.entryPtr[i].type |= DrmDef::CONSTRAINT_DATETIME;
            xmlChar* end = xmlNodeGetContent(endPtr);
            if (!StrUtil::datetimeStrToTick((char*)end, ro->rights.entryPtr[i].end_time))
            {
                ALOGE("parse_dr: DT[%s] is invalid", (char*)end);
                xmlFree(end);
                xmlFreeDoc(doc);
                xmlCleanupParser();
                return false;
            }
            xmlFree(end);
        }

        // check interval
        xmlNodePtr intervalPtr =
            getNode(doc, DrmDef::getPermissionPath(i) + DrmDef::PATH_CONSTRAINT_INTERVAL);
        if (intervalPtr != NULL)
        {
            ro->rights.entryPtr[i].type |= DrmDef::CONSTRAINT_INTERVAL;
            xmlChar* interval = xmlNodeGetContent(intervalPtr);
            if (!StrUtil::intervalStrToTick((char*)interval, ro->rights.entryPtr[i].interval))
            {
                ALOGE("parse_dr: interval[%s] is invalid", (char*)interval);
                xmlFree(interval);
                xmlFreeDoc(doc);
                xmlCleanupParser();
                return false;
            }
            xmlFree(interval);
        }

        if (ro->rights.entryPtr[i].type == DrmDef::CONSTRAINT_UNINIT)
        {
            ro->rights.entryPtr[i].type = DrmDef::CONSTRAINT_NONE;
        }

        // check if entry valid
        if (!isEntryValid(&ro->rights.entryPtr[i]))
        {
            ro->rights.num[i] = 0;
            ro->rights.best[i] = 0xFF;
            ALOGW("parse_dr: remove invalid entry for[%s].", DrmDef::getPermissionStr(i).string());
        }
    }

    xmlFreeDoc(doc);
    xmlCleanupParser();
    if (sVDebug) ALOGV("parse_dr <----");
    return true;
}

bool DrmUtil::parse_drc(DrmBuffer& drmBuffer, RO* ro)
{
    if (sVDebug) ALOGV("parse_drc ---->");

    if (!WBUtil::parseWbXml(drmBuffer.data, drmBuffer.length, ro))
    {
        ALOGE("parse_drc: failed to parse WB xml.");
        return false;
    }
    if (sVDebug) ALOGV("parse_drc <----");
    return true;
}

bool DrmUtil::parseDcf(String8 path, DrmMetadata* metadataPtr)
{
    if (sDDebug) ALOGD("parseDcf: path[%s]", path.string());

    if (!checkExistence(path))
    {
        ALOGE("parseDcf: file does not exist.");
        return false;
    }

    int fd = open(path.string(), O_RDONLY);
    if (-1 == fd)
    {
        ALOGE("parseDcf: failed to open file to read, reason [%s]", strerror(errno));
        return false;
    }

    int result = parseDcf(fd, metadataPtr);
    if (sVDebug) ALOGV("parseDcf: path[%s], result[%d]", path.string(), result);
    close(fd);
    return result < 0 ? false : true;
}

// note: the file descriptor shall be an valid one
//       and we do not close file descriptor inside this function.
// returns -1 for file operation failure.
// returns -2 for incorrect file length (comparing to {headersLen} + {dataLen} + index)
// returns -3 for non-DCF format
// returns 0 for correct DCF format

// Add for Decode image with open decrypt seesion with ashmem
int DrmUtil::parseDcf(const DrmBuffer& drmBuffer, DrmMetadata* metadataPtr)
{
    char* data = (char*) drmBuffer.data;
    if (NULL == data)
    {
        ALOGE("parseDcf: invalid parameter with drm buffer is NULL");
        return -1;
    }

    size_t cur = 0;
    // check first 3 bytes
    char tmp[3];
    bzero(tmp, sizeof(tmp));
    if (memcpy(tmp, data, sizeof(tmp)) == NULL)
    {
        ALOGE("parseDcf: buffer read error, reason [%s]", strerror(errno));
        return -1;
    }
    if (sVDebug) ALOGV("parseDcf: first 3 bytes [%x][%x][%x]", tmp[0], tmp[1], tmp[2]);

    // version
    if (tmp[0] != 1)
    {
        ALOGE("parseDcf: not dcf type, dcf version value [%d]", tmp[0]);
        return -3;
    }

    // mime type length
    int mime_len = tmp[1];
    if (mime_len > DrmDef::DRM_MAX_MIME_LENGTH - 1)
    {
        ALOGE("parseDcf: too long, and invalid mime type length[%d]", mime_len);
        return -3;
    }
    cur += 3;
    data = data + 3;
    // mime type
    char dcfMime[DrmDef::DRM_MAX_MIME_LENGTH];
    bzero(dcfMime, sizeof(dcfMime));
    if (memcpy(dcfMime, data, (size_t)mime_len) == NULL)
    {
        ALOGE("parseDcf: drm buffer read error for mime type, reason [%s]", strerror(errno));
        return -1;
    }
    String8 mime_type(dcfMime, (size_t)mime_len);
    if (sVDebug) ALOGV("parseDcf: mime type[%s]", mime_type.string());

    // mime type shall contain '/'
    if (mime_type.find("/") == -1)
    {
        ALOGE("parseDcf: not dcf type, invalid mime type[%s]", mime_type.string());
        return -3;
    }
    // put mime-type
    String8 mimeKey(DrmMetaKey::META_KEY_MIME);
    metadataPtr->put(&mimeKey, mime_type.string());
    data = data + mime_len;
    cur += mime_len;

    // content-id
    int cid_len = tmp[2];
    char cid_str[DrmDef::DRM_MAX_META_LENGTH];
    bzero(cid_str, sizeof(cid_str));
    if (NULL == memcpy(cid_str, data, (size_t)cid_len))
    {
        ALOGE("parseDcf: drm buffer read error for cid, reason [%s]", strerror(errno));
        return -1;
    }
    String8 cid(cid_str, (size_t)cid_len);
    if (sVDebug) ALOGV("parseDcf: cid[%s]", cid.string());

    // content-id shall not be empty
    String8 content_uri = StrUtil::getContentUri(cid);
    if (content_uri.isEmpty())
    {
        ALOGE("parseDcf: not dcf type, empty cid");
        return -3;
    }

    // put content-id
    String8 contentUriKey(DrmMetaKey::META_KEY_CONTENT_URI);
    metadataPtr->put(&contentUriKey, content_uri.string());
    data = data + cid_len;
    cur += cid_len;

    // check cid for method -> FL/CD/SD/FLSD
    // the string for cid of FL, CD, FLSD is specially defined.
    bool isSD = false;
    String8 methodKey(DrmMetaKey::META_KEY_METHOD);
    int method = DrmDef::METHOD_FL;
    if (strstr(cid_str, DrmDef::MTK_FL_CID.string()) != NULL)
    {
        if (sVDebug) ALOGV("parseDcf: method FL");
        method = DrmDef::METHOD_FL;
    }
    else if (strstr(cid_str, DrmDef::MTK_CD_CID.string()) != NULL)
    {
        if (sVDebug) ALOGV("parseDcf: method CD");
        method = DrmDef::METHOD_CD;
    }
    else if (strstr(cid_str, DrmDef::MTK_FLDCF_CID.string()) != NULL)
    {
        if (sVDebug) ALOGV("parseDcf: method FLSD");
        method = DrmDef::METHOD_FLDCF;
    }
    else // otherwise SD case
    {
        if (sVDebug) ALOGV("parseDcf: method SD");
        method = DrmDef::METHOD_SD;
        isSD = true;
    }
    metadataPtr->put(&methodKey, StrUtil::toString(method));

    // {headersLen} uintvar
    size_t forward = 0;
    int headersLen = readUintVarBuffer(data, &forward);
    data += forward;
    cur += forward;
    if (sVDebug) ALOGV("parseDcf: {headersLen} [%d] forward:[%d]", headersLen, forward);
    // {dataLen} unitvar
    int dataLen = readUintVarBuffer(data, &forward);
    if (sVDebug) ALOGV("parseDcf: {dataLen} [%d] forward:[%d]", dataLen, forward);
    cur += forward;

    // set {dataLenFile} for SD/FLSD
    if (strstr(cid_str, DrmDef::MTK_FL_CID.string()) == NULL
        && strstr(cid_str, DrmDef::MTK_CD_CID.string()) == NULL)
    {
        // for SD/FLSD, we check for validation of headersLen or dataLen
        if (headersLen <= 0 || dataLen <= 0)
        {
            ALOGE("parseDcf: not dcf type, invalid headersLen [%d] or dataLen [%d]",
                    headersLen, dataLen);
            return -3;
        }

        String8 dataLenKey(DrmMetaKey::META_KEY_DATALEN);
        metadataPtr->put(&dataLenKey, StrUtil::toString(dataLen));
    }

    // headers start position
    data = drmBuffer.data + cur;

    // validate from the header start position, check the {headersLen} & {dataLen}
    if (headersLen > 0 && dataLen > 0)
    {
        // the header length, the data length, and the header's offset shall equals total file length.
        if ((headersLen + dataLen + cur) != drmBuffer.length)
        {
            ALOGE("parseDcf: headersLen[%d] / dataLen[%d] incorrect value.", headersLen, dataLen);
            off_t contentOffset = headersLen;
            String8 contentOffsetKey(DrmMetaKey::META_KEY_OFFSET);
            metadataPtr->put(&contentOffsetKey, StrUtil::toString((long)contentOffset));

            String8 contentLengthKey(DrmMetaKey::META_KEY_DATALEN);
            metadataPtr->put(&contentLengthKey, StrUtil::toString(dataLen));

            return -2;
        }
    }

    // if (sVDebug) ALOGV("parseDcf: header starts at [%ld]", cur);

    // headers
    // the {headersLen} shall be correct!
    int amount = headersLen;
    while (amount > 0)
    {
        char str[DrmDef::DRM_MAX_META_LENGTH]; // contain \r\n NULL
        bzero(str, sizeof(str));

        // find a line from headers
        //int readNum = get_line(fd, str, sizeof(str));
        if (sVDebug) ALOGV("parseDcf: headers: to enter get_line");
        int readNum = get_line(data, str, sizeof(str));
        if (sVDebug) ALOGV("parseDcf: headers: readNum[%d]", readNum);
        if (-1 == readNum)
        {
            ALOGE("parseDcf: headers: fails by invalid data.");
            return -3;
        }
        String8 line(str, (size_t)readNum);

        if (sVDebug) ALOGV("parseDcf: readNum[%ld] read heads:[%s]", readNum, line.string());
        // work around for specical case:
        //   dcf file's last header is not ended with 0D0A
        if (0 == readNum)
        {
            readNum = amount;

            bzero(str, sizeof(str));
            memcpy(str, data, (size_t)readNum);
            //data = data + readNum;
            // correct the {line} string
            line.setTo(str, (size_t)readNum);
        }
        // special case: the last header is not ended with 0D0A, and the retrieved
        //   string is longer than that
        if (readNum > amount)
        {
            readNum = amount;
            line.setTo(str, (size_t)readNum); // adjust the string already read
        }
        data = data + readNum;

        line = StrUtil::trimRCRLF(line);
        if (sVDebug) ALOGV("parseDcf: headers: find a line [%s]", line.string());

        // find token in the line and put meta data
        for (int i = DrmDef::META_RIGHTS_ISSUER_INDEX;
             i <= DrmDef::META_ICON_URI_INDEX; i++)
        {
            String8 tokenStr = DrmDef::getMetaTokenStr(i);
            int index = line.find(tokenStr);
            if (index >= 0)
            {
                line.setTo(&line.string()[index + tokenStr.length()]);
                line = StrUtil::trimLRSpace(line);
                String8 metaDataStrKey = DrmDef::getMetaDataStr(i);
                metadataPtr->put(&metaDataStrKey, line.string());
            }
        }

        // next
        amount -= readNum;
    }

    // headers end position: shall be {index} + {headersLen}
    off_t index_end = (off_t) (data - drmBuffer.data);
    if (sVDebug) ALOGV("parseDcf: header ends at [%ld] -> content part offset", index_end);

    // put content part offset
    String8 offsetKey(DrmMetaKey::META_KEY_OFFSET);
    metadataPtr->put(&offsetKey, StrUtil::toString((long)index_end));

    off_t dataLenFile = (off_t) drmBuffer.length - index_end;
    if (sVDebug) ALOGV("parseDcf: content part length [%ld]", dataLenFile);

    // set {dataLenFile} for FL/CD
    if (strstr(cid_str, DrmDef::MTK_FL_CID.string()) != NULL
        || strstr(cid_str, DrmDef::MTK_CD_CID.string()) != NULL)
    {
        String8 dataLenKey(DrmMetaKey::META_KEY_DATALEN);
        metadataPtr->put(&dataLenKey, StrUtil::toString(dataLenFile));
    }

    // verify {dataLen} for SD
    if (isSD && dataLenFile != (off_t)dataLen)
    {
        ALOGE("parseDcf: {dataLen} [%d] not equal to {dataLenFile} [%ld]",
                dataLen, dataLenFile);
        return -2;
    }

    // mark as DCF file
    String8 isDrmKey(DrmMetaKey::META_KEY_IS_DRM);
    metadataPtr->put(&isDrmKey, "1");

    if (sDDebug) ALOGD("parseDcf: drmBuffer[%p], cid[%s], method[%d], mimetype[%s], headerLen[%d],"
            " dataLen[%d], contentLen[%d]", drmBuffer.data, cid.string(), method, mime_type.string(),
            headersLen, dataLen, dataLenFile);
    return 0;
}

int DrmUtil::parseDcf(int fd, DrmMetadata* metadataPtr)
{
    if (sVDebug) ALOGV("parseDcf: fd[%d]", fd);

    if (-1 == lseek(fd, 0, SEEK_SET))
    {
        ALOGE("parseDcf: fd[%d] lseek error, reason [%s]", fd, strerror(errno));
        return -1;
    }

    // check first 3 bytes
    char tmp[3];
    bzero(tmp, sizeof(tmp));
    if (-1 == read(fd, tmp, sizeof(tmp)))
    {
        ALOGE("parseDcf: fd[%d] read error, reason [%s]", fd, strerror(errno));
        return -1;
    }
    if (sVDebug) ALOGV("parseDcf: first 3 bytes [%x][%x][%x]", tmp[0], tmp[1], tmp[2]);

    // version
    if (tmp[0] != 1)
    {
        if (sDDebug) ALOGE("parseDcf: not dcf type, dcf version value [%d]", tmp[0]);
        return -3;
    }

    // mime type length
    int mime_len = tmp[1];
    if (mime_len > DrmDef::DRM_MAX_MIME_LENGTH - 1)
    {
        ALOGE("parseDcf: too long, and invalid mime type length [%d]", mime_len);
        return -3;
    }

    // mime type
    char dcfMime[DrmDef::DRM_MAX_MIME_LENGTH];
    bzero(dcfMime, sizeof(dcfMime));
    if (-1 == read(fd, dcfMime, (size_t)mime_len))
    {
        ALOGE("parseDcf: fd read error for mime type, reason [%s]", strerror(errno));
        return -1;
    }
    String8 mime_type(dcfMime, (size_t)mime_len);
    if (sVDebug) ALOGV("parseDcf: mime type [%s]", mime_type.string());

    // mime type shall contain '/'
    if (mime_type.find("/") == -1)
    {
        ALOGE("parseDcf: not dcf type, invalid mime type string [%s]", mime_type.string());
        return -3;
    }

    // put mime-type
    String8 mimeKey(DrmMetaKey::META_KEY_MIME);
    metadataPtr->put(&mimeKey, mime_type.string());

    // content-id
    int cid_len = tmp[2];
    char cid_str[DrmDef::DRM_MAX_META_LENGTH];
    bzero(cid_str, sizeof(cid_str));
    if (-1 == read(fd, cid_str, (size_t)cid_len))
    {
        ALOGE("parseDcf: fd read error for content-id, reason [%s]",
                strerror(errno));
        return -1;
    }
    String8 cid(cid_str, (size_t)cid_len);
    if (sVDebug) ALOGV("parseDcf: cid[%s]", cid.string());

    // content-id shall not be empty
    String8 content_uri = StrUtil::getContentUri(cid);
    if (content_uri.isEmpty())
    {
        if (sDDebug) ALOGE("parseDcf: not dcf type, empty cid");
        return -3;
    }

    // put content-id
    String8 contentUriKey(DrmMetaKey::META_KEY_CONTENT_URI);
    metadataPtr->put(&contentUriKey, content_uri.string());

    // check cid for method -> FL/CD/SD/FLDCF
    // the string for cid of FL, CD, FLSD is specially defined.
    bool isSD = false;
    String8 methodKey(DrmMetaKey::META_KEY_METHOD);
    int method = DrmDef::METHOD_FL;
    if (strstr(cid_str, DrmDef::MTK_FL_CID.string()) != NULL)
    {
        if (sVDebug) ALOGV("parseDcf: method FL");
        method = DrmDef::METHOD_FL;
    }
    else if (strstr(cid_str, DrmDef::MTK_CD_CID.string()) != NULL)
    {
        if (sVDebug) ALOGV("parseDcf: method CD");
        method = DrmDef::METHOD_CD;
    }
    else if (strstr(cid_str, DrmDef::MTK_FLDCF_CID.string()) != NULL)
    {
        if (sVDebug) ALOGV("parseDcf: method FLSD");
        method = DrmDef::METHOD_FLDCF;
    }
    else // otherwise SD case
    {
        if (sVDebug) ALOGV("parseDcf: method SD");
        method = DrmDef::METHOD_SD;
        isSD = true;
    }
    metadataPtr->put(&methodKey, StrUtil::toString(method));

    // {headersLen} uintvar
    int headersLen = readUintVarFd(fd);
    if (sVDebug) ALOGV("parseDcf: {headersLen} [%d]", headersLen);
    // {dataLen} unitvar
    int dataLen = readUintVarFd(fd);
    if (sVDebug) ALOGV("parseDcf: {dataLen} [%d]", dataLen);

    // set {dataLenFile} for SD/FLSD
    if (strstr(cid_str, DrmDef::MTK_FL_CID.string()) == NULL
        && strstr(cid_str, DrmDef::MTK_CD_CID.string()) == NULL)
    {
        // for SD/FLSD, we check for validation of headersLen or dataLen
        if (headersLen <= 0 || dataLen <= 0)
        {
            ALOGE("parseDcf: not dcf type, invalid headersLen [%d] or dataLen [%d]",
                    headersLen, dataLen);
            return -3;
        }

        String8 dataLenKey(DrmMetaKey::META_KEY_DATALEN);
        metadataPtr->put(&dataLenKey, StrUtil::toString(dataLen));
    }

    // headers start position
    off_t index = lseek(fd, 0, SEEK_CUR);
    if (-1 == index)
    {
        ALOGE("parseDcf: fd lseek error, reason [%s]", strerror(errno));
        return -1;
    }
    if (sVDebug) ALOGV("parseDcf: header starts at [%ld]", index);

    // validate from the header start position, check the {headersLen} & {dataLen}
    if (headersLen > 0 && dataLen > 0)
    {
        off_t index_file_end = lseek(fd, 0, SEEK_END);
        if (-1 == index_file_end)
        {
            ALOGE("parseDcf: fd lseek error, reason [%s]", strerror(errno));
            return -1;
        }
        if (sVDebug) ALOGV("parseDcf: file length [%ld]", index_file_end);

        if (-1 == lseek(fd, index, SEEK_SET))
        {
            ALOGE("parseDcf: fd lseek error, reason [%s]", strerror(errno));
            return -1;
        }

        // the header length, the data length, and the header's offset shall equals total file length.
        if ((headersLen + dataLen + index) != index_file_end)
        {
            ALOGE("parseDcf: {headersLen} / {dataLen} incorrect value.");
            off_t contentOffset = index + headersLen;
            String8 contentOffsetKey(DrmMetaKey::META_KEY_OFFSET);
            metadataPtr->put(&contentOffsetKey, StrUtil::toString((long)contentOffset));

            String8 contentLengthKey(DrmMetaKey::META_KEY_DATALEN);
            metadataPtr->put(&contentLengthKey, StrUtil::toString(dataLen));

            return -2;
        }
    }

    // headers
    // the {headersLen} shall be correct!
    int amount = headersLen;
    while (amount > 0)
    {
        off_t offset = lseek(fd, 0, SEEK_CUR);
        if (-1 == offset)
        {
            ALOGE("parseDcf: headers: fd lseek error, reason [%s]", strerror(errno));
            return -1;
        }

        char str[DrmDef::DRM_MAX_META_LENGTH]; // contain \r\n NULL
        bzero(str, sizeof(str));

        // find a line from headers
        int readNum = get_line(fd, str, sizeof(str));
        if (-1 == readNum)
        {
            ALOGE("parseDcf: headers: fails by invalid data.");
            return -3;
        }
        String8 line(str, (size_t)readNum);

        // work around for specical case:
        //   dcf file's last header is not ended with 0D0A
        if (0 == readNum)
        {
            readNum = amount;

            bzero(str, sizeof(str));
            ssize_t result = read(fd, str, (size_t)readNum);
            if (-1 == result || result != (ssize_t)readNum)
            {
                ALOGE("parseDcf: headers: failed to read data from fd, reason [%s]",
                        strerror(errno));
                return -1;
            }
            // correct the {line} string
            line.setTo(str, (size_t)readNum);
        }
        // special case: the last header is not ended with 0D0A, and the retrieved
        //   string is longer than that
        if (readNum > amount)
        {
            readNum = amount;
            line.setTo(str, (size_t)readNum); // adjust the string already read
        }

        line = StrUtil::trimRCRLF(line);
        if (sVDebug) ALOGV("parseDcf: headers: find a line [%s]", line.string());

        // find token in the line and put meta data
        for (int i = DrmDef::META_RIGHTS_ISSUER_INDEX;
             i <= DrmDef::META_ICON_URI_INDEX; i++)
        {
            String8 tokenStr = DrmDef::getMetaTokenStr(i);
            int index = line.find(tokenStr);
            if (index >= 0)
            {
                line.setTo(&line.string()[index + tokenStr.length()]);
                line = StrUtil::trimLRSpace(line);
                String8 metaDataStrKey = DrmDef::getMetaDataStr(i);
                metadataPtr->put(&metaDataStrKey, line.string());
            }
        }

        // next
        amount -= readNum;
    }

    // headers end position: shall be {index} + {headersLen}
    off_t index_end = index + headersLen;
    if (sVDebug) ALOGV("parseDcf: header ends at [%ld] -> content part offset", index_end);

    // put content part offset
    String8 offsetKey(DrmMetaKey::META_KEY_OFFSET);
    metadataPtr->put(&offsetKey, StrUtil::toString((long)index_end));

    // find the end of file, and length of content part
    off_t end = lseek(fd, 0, SEEK_END);
    if (-1 == end)
    {
        ALOGE("parseDcf: fd lseek error, reason [%s]", strerror(errno));
        return -1;
    }
    if (sVDebug) ALOGV("parseDcf: file ends at [%ld]", end);
    off_t dataLenFile = end - index_end;
    if (sVDebug) ALOGV("parseDcf: content part length [%ld]", dataLenFile);

    // set {dataLenFile} for FL/CD
    if (strstr(cid_str, DrmDef::MTK_FL_CID.string()) != NULL
        || strstr(cid_str, DrmDef::MTK_CD_CID.string()) != NULL)
    {
        String8 dataLenKey(DrmMetaKey::META_KEY_DATALEN);
        metadataPtr->put(&dataLenKey, StrUtil::toString(dataLenFile));
    }

    // verify {dataLen} for SD
    if (isSD && dataLenFile != (off_t)dataLen)
    {
        ALOGE("parseDcf: {dataLen} [%d] not equal to {dataLenFile} [%ld]", dataLen, dataLenFile);
        return -2;
    }

    // mark as DCF file
    String8 isDrmKey(DrmMetaKey::META_KEY_IS_DRM);
    metadataPtr->put(&isDrmKey, "1");
    if (sDDebug) ALOGD("parseDcf: fd[%d], cid[%s], method[%d], mimetype[%s], headerLen[%d],"
            " dataLen[%d], contentLen[%d], fileLen[%d]", fd, cid.string(), method,
            mime_type.string(), headersLen, dataLen, dataLenFile, end);
    return 0;
}

bool DrmUtil::checkDcf(String8 path, char* mime)
{
    if (!checkExistence(path))
    {
        ALOGE("checkDcf: file[%s] does not exist", path.string());
        return false;
    }

    int fd = open(path.string(), O_RDONLY);
    if (-1 == fd)
    {
        ALOGE("checkDcf: failed to open file[%s], reason [%s]",path.string(), strerror(errno));
        return false;
    }

    int result = checkDcf(fd, mime);
    if (sDDebug) ALOGD("checkDcf: path[%s], result[%d]", path.string(), result);
    close(fd);
    return result < 0 ? false : true;
}

// return -1 to indicate not dcf file.
// 0 for OMA DRM v1 dcf file type. other positive value reserved for future use.
// the mime type string, if it's a dcf file, is return in {mime}
// {mime} can be NULL; the buffer shall be large enough to contain the string
// note: fd should be an opened file descriptor, and valid.
//       and we do not close it inside this function
int DrmUtil::checkDcf(int fd, char* mime)
{
    if (sVDebug) ALOGV("checkDcf: fd[%d]", fd);

    Mutex::Autolock lock(mDCFLock);

    if (-1 == lseek(fd, 0, SEEK_SET))
    {
        ALOGE("checkDcf: fd[%d] lseek error, reason [%s]", fd, strerror(errno));
        return -1;
    }

    // check first 3 bytes
    char tmp[3];
    bzero(tmp, sizeof(tmp));
    if (-1 == read(fd, tmp, sizeof(tmp)))
    {
        ALOGE("checkDcf: fd[%d] read error, reason [%s]", fd, strerror(errno));
        return -1;
    }
    if (sVDebug) ALOGV("checkDcf: first 3 bytes [%x][%x][%x]", tmp[0], tmp[1], tmp[2]);

    if (tmp[0] != 1)
    {
        ALOGE("checkDcf: not dcf type, dcf version value [%d]", tmp[0]);
        return -1;
    }

    int mime_len = tmp[1];
    if (mime_len > DrmDef::DRM_MAX_MIME_LENGTH - 1)
    {
        ALOGE("checkDcf: invalid mime type length [%d]", mime_len);
        return -1;
    }

    char dcfMime[DrmDef::DRM_MAX_MIME_LENGTH];
    bzero(dcfMime, sizeof(dcfMime));
    if (-1 == read(fd, dcfMime, (size_t)mime_len))
    {
        ALOGE("checkDcf: fd[%d] read error for mime type, reason [%s]", fd, strerror(errno));
        return -1;
    }

    String8 mime_type(dcfMime, (size_t)mime_len);
    if (sVDebug) ALOGV("checkDcf: mime type[%s]", mime_type.string());

    if (mime_type.find("/") == -1)
    {
        ALOGE("checkDcf: not dcf type, invalid mime type string [%s]",
                mime_type.string());
        return -1;
    }

    if (NULL != mime) {
        memcpy(mime, dcfMime, mime_len);
    }
    if (sDDebug) ALOGD("checkDcf: fd[%d], mime type[%s]", fd, mime_type.string());
    // simply check the content, and we consider this file is a dcf type.
    return 0;
}


ByteBuffer DrmUtil::getDcfDrmKey(DrmMetadata* pMetaData)
{
    // content uri (cid)
    String8 cid = pMetaData->get(String8(DrmMetaKey::META_KEY_CONTENT_URI));
    if (cid.isEmpty())
    {
        ALOGE("getDcfDrmKey: failed to get content uri (cid).");
        return ByteBuffer(); // an empty one
    }

    // method of dcf file
    String8 method = pMetaData->get(String8(DrmMetaKey::META_KEY_METHOD));
    if (method.isEmpty())
    {
        ALOGE("getDcfDrmKey: failed to get method.");
        return ByteBuffer();
    }

    // the dcf was converted from FL / CD drm messages
    if (method == StrUtil::toString(DrmDef::METHOD_FL)
        || method == StrUtil::toString(DrmDef::METHOD_CD))
    {
        return getDrmKey();
    }
    else // or SD/FLSD case
    {
        Mutex::Autolock lock(mROLock);
        RO ro;
        if (!restore(cid, &ro))
        {
            ALOGE("getDcfDrmKey: failed to restore rights.");
            return ByteBuffer();
        }
        return ByteBuffer(ro.rights.key, DrmDef::DRM_MAX_KEY_LENGTH);
    }
}
#endif


// return the device unique 16 bytes drm key
ByteBuffer DrmUtil::getDrmKey()
{
    char drmkey[DrmDef::DRM_MAX_ID_LENGTH + 1];
    bzero(drmkey, sizeof(drmkey));

    String8 imei = readIMEI();

    char imei_encrypt_key[DrmDef::DRM_MAX_KEY_LENGTH];
    bzero(imei_encrypt_key, sizeof(imei_encrypt_key));

    // get an fixed key to encrypt imei
    long s = 0x3D4FAD6A;
    long v = ~(0xA9832DC6 ^ s);
    for (int i = 0; i < 4; i++)
    {
        imei_encrypt_key[i] = (char) (0xFF & (v >> (i * 8)));
    }
    v = 0x16F0D768 ^ s;
    for (int i = 0; i < 4; i++)
    {
        imei_encrypt_key[i + 4] = (char) (0xFF & (v >> (i * 8)));
    }
    v = ~(0x278FB1EA ^ s);
    for (int i = 0; i < 4; i++)
    {
        imei_encrypt_key[i + 8] = (char) (0xFF & (v >> (i * 8)));
    }
    v = 0x5F3C54EC ^ s;
    for (int i = 0; i < 4; i++)
    {
        imei_encrypt_key[i + 12] = (char) (0xFF & (v >> (i * 8)));
    }

    unsigned int devIdLen = imei.length();
    if (devIdLen > DrmDef::DRM_MAX_ID_LENGTH)
    {
        devIdLen = DrmDef::DRM_MAX_ID_LENGTH;
        ALOGE("getDrmKey, imei length[%d] > drm key length[%d]", devIdLen, DrmDef::DRM_MAX_ID_LENGTH);
    }

    // encrypt imei to get an unique key
    RC4_KEY k;
    RC4_set_key(&k, DrmDef::DRM_MAX_KEY_LENGTH, (unsigned char*)imei_encrypt_key);
    RC4(&k, (unsigned long)devIdLen, (unsigned char*)imei.string(), (unsigned char*)drmkey);

    return ByteBuffer(drmkey, DrmDef::DRM_MAX_KEY_LENGTH);
}

ByteBuffer DrmUtil::base64_decode_bytebuffer(ByteBuffer& buf)
{
    EVP_ENCODE_CTX ectx;
    unsigned char* out = (unsigned char*)malloc(buf.length());
    int outlen = 0;
    int tlen = 0;

    int decode_result = 0;
    EVP_DecodeInit(&ectx);
    EVP_DecodeUpdate(&ectx, out, &outlen, (const unsigned char*)buf.buffer(), buf.length());
    tlen += outlen;
    decode_result = EVP_DecodeFinal(&ectx, out + tlen, &outlen);
    if(decode_result < 0)
    {
        ALOGE("EVP_DecodeFinal failed:decode_result = %d",decode_result);
    }
    tlen += outlen;

    ByteBuffer result((char*)out, tlen);
    free(out);
    return result;
}

void DrmUtil::base64_decrypt_buffer(unsigned char* input, unsigned char* output, int inputLength, int& outputLength)
{
    EVP_ENCODE_CTX ectx;
    unsigned char* out = (unsigned char*)malloc(inputLength);
    int outlen = 0;
    int tlen = 0;

    EVP_DecodeInit(&ectx);
    EVP_DecodeUpdate(&ectx, out, &outlen, (const unsigned char*)input, inputLength);
    tlen += outlen;
    EVP_DecodeFinal(&ectx, out + tlen, &outlen);
    tlen += outlen;

    memcpy(output, out, tlen);
    outputLength = tlen;
    free(out);
}

void DrmUtil::rc4_encrypt_buffer(unsigned char* toBuffer, unsigned char* fromBuffer, int size)
{
    ByteBuffer drmkey(getDrmKey());
    RC4_KEY k;
    RC4_set_key(&k, DrmDef::DRM_MAX_KEY_LENGTH, (unsigned char*)drmkey.buffer());
    RC4(&k, size, fromBuffer, toBuffer);
}

void DrmUtil::rc4_decrypt_buffer(unsigned char* toBuffer, unsigned char* fromBuffer, int size)
{
    rc4_encrypt_buffer(toBuffer, fromBuffer, size);
}

String8 DrmUtil::hash(char* seed, int seedLen)
{
    if (sVDebug) ALOGV("hash ----> seed [%s], seedLen [%d]", seed, seedLen);

    if (seedLen <= 0 || NULL == seed)
    {
        ALOGE("hash: invalid seed length or seed.");
    }

    char hashResult[17]; // 16 + 1: the result of MD5 hash is 128 bit
    bzero(hashResult, sizeof(hashResult));

    MD5((unsigned char*)seed, seedLen, (unsigned char*)hashResult);
    if (sVDebug) ALOGV("hash: result of MD5 [%s]", hashResult);

    String8 hexHashResult = StrUtil::toHexStr(hashResult, sizeof(hashResult) - 1);
    if (sVDebug) ALOGV("hash: result in hex format [%s]", hexHashResult.string());

    return hexHashResult;
}

void DrmUtil::CreateMulvPath(char* muldir)
{
    if (sDDebug) ALOGD("CreateMulvPath() : %s", muldir);
    char str[PATH_MAX];
    bzero(str, sizeof(str));

    // klocwork issue:
    // The function strncpy is used to copy a string of characters to a buffer of memory.
    // Among its parameters is an argument that limits the size of written data. if strncpy
    // copies data to an array of fixed size(buf), normally the limit should be sizeof(buf) -1.
    // the -1 is important because a trailing zero is counted as a byte. If the size parameter
    // is greater than the size of the output buffer, a buffer overflow may result
    strncpy(str, muldir, sizeof(str) - 1);

    size_t len = strlen(str);
    unsigned int i = 0;
    for (i = 0; i < len; i++)
    {
        if (str[i] == '/')
        {
            str[i] = '\0';
            if (access(str, F_OK) != 0) // if one level of directory does not exist. F_OK for existence check.
            {
                mkdir(str, 0770); // make directory for one level
            }
            str[i] = '/';
        }
    }

    if (len > 0 && access(str, F_OK) != 0) // some cases, it does not end with '/'
    {
        mkdir(str, 0770);
    }

    return;
}

#if 0
bool DrmUtil::restoreRights(FILE* fp, RO* ro)
{
    FileUtil::fseekx(fp, getRightsPosition(), SEEK_SET);
    // first we check the beginning of the RO file for RO_VERSION_01_0 string.
    char ver[SIZE_VER_01_0];
    bzero(ver, SIZE_VER_01_0);
    if (!FileUtil::freadx(ver, strlen(RO_VERSION_01_0), fp))
    {
        ALOGE("restoreRights: failed to read possible rights object verison from file.");
        return false;
    }

    if (sVDebug) ALOGV("restoreRights: compare [%s] with [%s]", ver, RO_VERSION_01_0);
    int version = 0;
    if (0 == strcmp(ver, RO_VERSION_01_0))
    {
        version = VER_01_0;
    }

    size_t ro_size = (0 == version) ? sizeof(struct OldRights) : sizeof(struct Rights);
    unsigned char* encryptedRightsBuffer = new unsigned char[ro_size];
    bzero(encryptedRightsBuffer, ro_size);

    // if old RO struct, then go back to the beginning of file
    if (0 == version)
    {
        if (sDDebug) ALOGD("restoreRights: old-style rights object.");
        FileUtil::fseekx(fp, 0, SEEK_SET);
    }
    if (!FileUtil::freadx(encryptedRightsBuffer, ro_size, fp))
    {
        ALOGE("restoreRights: failed to read rights data from file.");
        delete[] encryptedRightsBuffer;
        return false;
    }

    // old RO struct compatibility, and we decrypt according to the amount of data
    OldRights old_rights;
    unsigned char* output = (0 == version) ? (unsigned char*)&old_rights : (unsigned char*)&ro->rights;
    rc4_decrypt_buffer(output, encryptedRightsBuffer, ro_size);

    // if old RO struct, copy the value to the new type of struct
    if (0 == version)
    {
        convertRO(&ro->rights, &old_rights);
    }

    ro->rights.entryPtr = NULL;

    delete[] encryptedRightsBuffer;
    return true;
}

bool DrmUtil::restoreEntry(String8 ro_file, FILE* fp, RO* ro)
{
    unsigned char encryptedEntryBuffer[sizeof(struct Entry)];
    bzero(encryptedEntryBuffer, sizeof(encryptedEntryBuffer));

    // get total entry number
    // the current fp shall points to the beginning of entries. (after call restoreRights)
    long old_pos = 0;
    FileUtil::ftellx(fp, old_pos);
    if (sVDebug) ALOGV("restoreEntry: entries starts at [%ld]", old_pos);
    struct stat stFileInfo;
    if (-1 == fstat(fileno(fp), &stFileInfo))
    {
        ALOGE("restoreEntry: failed to stat file info, reason [%s]", strerror(errno));
        return false;
    }
    if (sVDebug) ALOGV("restoreEntry: size of rights object [%ld]", (long)(stFileInfo.st_size));

    long cnt = (long)(stFileInfo.st_size) - old_pos;
    if (sVDebug) ALOGV("restoreEntry: entries data amount [%ld]", cnt);

    int totalEntryNumber = cnt / sizeof(struct Entry);
    if (sVDebug) ALOGV("restoreEntry: total entry number [%d]", totalEntryNumber);
    if ((cnt % sizeof(struct Entry)) != 0)
    {
        ALOGE("restoreEntry: invalid amount of entries data.");
        totalEntryNumber = -1;
    }

    if (totalEntryNumber < 0)
    {
        ALOGE("restoreEntry: invalid total entry number [%d]", totalEntryNumber);
        return false;
    }

    ro->rights.entryPtr = new Entry[totalEntryNumber];

    long entryBeginPosition = 0;
    FileUtil::ftellx(fp, entryBeginPosition);

    int valid_entry_num = 0;
    for (int i = DrmDef::PERMISSION_PLAY_INDEX; i <= DrmDef::PERMISSION_PRINT_INDEX; i++)
    {
        valid_entry_num += ro->rights.num[i];
        if (ro->rights.num[i] != 0)
        {
            if (sVDebug) ALOGV("restoreEntry: permission [%s] entry num [%d]",
                    DrmDef::getPermissionStr(i).string(), ro->rights.num[i]);
            int entryIndex = ro->rights.best[i];
            while (entryIndex != 0xFF)
            {
                long pos = old_pos + entryIndex * sizeof(struct Entry);
                if (sVDebug) ALOGV("restoreEntry: restore entry #[%d] from position [%ld]",
                        entryIndex, pos);
                if (!FileUtil::fseekx(fp, pos, SEEK_SET))
                {
                    ALOGE("restoreEntry: failed to seek to [%ld]", pos);
                    return false;
                }

                bzero(encryptedEntryBuffer, sizeof(encryptedEntryBuffer));
                if (!FileUtil::freadx(encryptedEntryBuffer, sizeof(encryptedEntryBuffer), fp))
                {
                    ALOGE("restoreEntry: failed to read entry data from file.");
                    return false;
                }

                Entry* entry = &ro->rights.entryPtr[entryIndex];
                rc4_decrypt_buffer((unsigned char*)entry, encryptedEntryBuffer, sizeof(encryptedEntryBuffer));

                entryIndex = entry->next;
            }
        }
    }

    if (sDDebug) ALOGD("restoreEntry: valid entry num [%d]", valid_entry_num);
    return true;
}

bool DrmUtil::saveRights(FILE* fp, Rights* rights)
{
    unsigned char encryptedRightsBuffer[sizeof(struct Rights)];
    bzero(encryptedRightsBuffer, sizeof(encryptedRightsBuffer));
    rc4_encrypt_buffer(encryptedRightsBuffer, (unsigned char*)rights, sizeof(struct Rights));

    FileUtil::fseekx(fp, getRightsPosition(), SEEK_SET);

    // now we write extra RO version information at the beginning of the Right Object file
    if (!FileUtil::fwritex((char*)RO_VERSION_01_0, strlen(RO_VERSION_01_0), fp))
    {
        ALOGE("saveRights: failed to write rights object version to file.");
        return false;
    }
    if (!FileUtil::fwritex(encryptedRightsBuffer, sizeof(encryptedRightsBuffer), fp))
    {
        ALOGE("saveRights: failed to write rights data to file.");
        return false;
    }

    return true;
}

bool DrmUtil::saveEntry(FILE* fp, RO* ro)
{
    unsigned char encryptedEntryBuffer[sizeof(struct Entry)];
    unsigned char inputEntryBuffer[sizeof(struct Entry)];
    bzero(encryptedEntryBuffer, sizeof(encryptedEntryBuffer));

    // the saveEntry is called after saveRights, and the fp shall point to the begin position of entries.
    long entryBeginPosition = 0;
    FileUtil::ftellx(fp, entryBeginPosition);
    if (sDDebug) ALOGD("saveEntry: entries starts at [%ld]", entryBeginPosition);

    for (int i = DrmDef::PERMISSION_PLAY_INDEX; i <= DrmDef::PERMISSION_PRINT_INDEX; i++)
    {
        if (ro->rights.num[i] != 0)
        {
            int entryIndex = ro->rights.best[i];
            while (entryIndex != 0xFF)
            {
                Entry* entry = &ro->rights.entryPtr[entryIndex];
                bzero(inputEntryBuffer, sizeof(inputEntryBuffer));
                memcpy (inputEntryBuffer, entry, sizeof(struct Entry));

                bzero(encryptedEntryBuffer, sizeof(encryptedEntryBuffer));
                rc4_encrypt_buffer(encryptedEntryBuffer, inputEntryBuffer, sizeof(struct Entry));

                long pos = entryBeginPosition + entryIndex * sizeof(struct Entry);
                if (sDDebug) ALOGD("saveEntry: save entry #[%d] to position [%ld]", entryIndex, pos);

                FileUtil::fseekx(fp, pos, SEEK_SET);
                if (!FileUtil::fwritex(encryptedEntryBuffer, sizeof(encryptedEntryBuffer), fp))
                {
                    ALOGE("saveEntry: failed to write entry data to file.");
                    return false;
                }

                entryIndex = entry->next;
            }
        }
    }

    return true;
}

int DrmUtil::getRightsPosition()
{
    return 0;
}

bool DrmUtil::mergeEntry(RO* newRO, int new_entry_index, int permission)
{
    if (sDDebug) ALOGD("mergeEntry ----> new_entry_index [%d], permission [%d]",
            new_entry_index, permission);

    Entry* new_entry = &newRO->rights.entryPtr[new_entry_index];

    // restore old RO from file, which has the same cid
    RO oldRO;
    if (!restore(String8(newRO->rights.cid), &oldRO))
    {
        ALOGE("mergeEntry: failed to restore old RO from file.");
        return false;
    }

    // update permission
    oldRO.rights.permission |= newRO->rights.permission;

    // get old entry number & 1st available old entry index
    int old_avIndex = 0;
    int old_totalEntryNum = 0;
    getFirstAvailableEntryIndex(&oldRO, &old_avIndex, &old_totalEntryNum);
    if (sVDebug) ALOGV("mergeEntry: old_avIndex [%d], old_totalEntryNum [%d]",
            old_avIndex, old_totalEntryNum);

    bool isNewEntryDT = new_entry->type == DrmDef::CONSTRAINT_DATETIME;
    if (isNewEntryDT)
    {
        if (sDDebug) ALOGD("mergeEntry: new entry is date-time type.");
    }

    bool isNewEntryCount = new_entry->type == DrmDef::CONSTRAINT_COUNT;
    if (isNewEntryCount)
    {
        if (sDDebug) ALOGD("mergeEntry: new entry is count type.");
    }


    bool isNewEntryInterval = new_entry->type == DrmDef::CONSTRAINT_INTERVAL;
    if (isNewEntryInterval)
    {
       ALOGD("mergeEntry: new entry is interval type.");
    }

    bool isNewEntryOthers = isNewEntryDT == false && isNewEntryCount == false;
    if (isNewEntryOthers)
    {
        if (sDDebug) ALOGD("mergeEntry: new entry is other type.");
    }

    int compare_pre_index = -1;
    int dt_start_index = -1;
    int dt_end_index = -1;
    int dt_start_pre_index = -1;
    bool determine_start_flag = false;
    int dt_end_pre_index = -1;
    bool determine_end_flag = false;
    int dt_status = 0;
    bool has_dt = false;

    // overwrite new entry into old entry list
    if (sVDebug) ALOGV("mergeEntry: overwrite new entry into old entry list ---->");
    Entry* p = NULL;
    int p_index = 0;
    if (old_avIndex != -1) // overwite the old one.
    {
        p_index = old_avIndex;
        memcpy(&oldRO.rights.entryPtr[old_avIndex], new_entry, sizeof(struct Entry));
        p = &oldRO.rights.entryPtr[old_avIndex];
    }
    else
    {
        p_index = old_totalEntryNum;
        Entry* old_entry_ptr = oldRO.rights.entryPtr;
        oldRO.rights.entryPtr = new Entry[old_totalEntryNum + 1];
        memcpy(oldRO.rights.entryPtr, old_entry_ptr,
               old_totalEntryNum * sizeof(struct Entry));
        memcpy(&oldRO.rights.entryPtr[old_totalEntryNum], new_entry,
               sizeof(struct Entry));
        p = &oldRO.rights.entryPtr[old_totalEntryNum];
        delete[] old_entry_ptr;
        old_entry_ptr = NULL;
    }

    // iterate old entry, merge new entry
    int old_entry_index = oldRO.rights.best[permission];
    if (old_entry_index == 0xFF)
    {
        if (sVDebug) ALOGV("mergeEntry: add new entry as 1st position.");
        oldRO.rights.num[permission]++;
        oldRO.rights.best[permission] = p_index;
        p->next = 0xFF;
    }
    while (old_entry_index != 0xFF)
    {
        Entry* old_entry = &oldRO.rights.entryPtr[old_entry_index];
        if (sVDebug) ALOGV("mergeEntry: check old entry with index [%d]", old_entry_index);
        old_entry->dump();

        if (sVDebug) ALOGV("mergeEntry: check new entry");
        new_entry->dump();

        // merge new entry into old entry list
        if (isNewEntryOthers) {
           if (old_entry->type == DrmDef::CONSTRAINT_INTERVAL && isNewEntryInterval) {
                ALOGV("mergeEntry: Interval type, merge to old entry.");
                old_entry->interval += new_entry->interval;
                if (old_entry->start_intv > 0 && old_entry->end_intv > 0) {
                    old_entry->end_intv += new_entry->interval;
                }
            }
            else {
            if (compareEntry(new_entry, old_entry) > 0) {
                if (sVDebug) ALOGV("mergeEntry: other entry type, insert before old entry.");
                p->next = old_entry_index;
                if (compare_pre_index == -1) {
                    oldRO.rights.best[permission] = p_index;
                }
                else {
                    oldRO.rights.entryPtr[compare_pre_index].next = p_index;
                }
                oldRO.rights.num[permission]++;
                break;
            }
            else if (old_entry->next == 0xFF) {
                if (sVDebug) ALOGV("mergeEntry: other entry type, insert after last old entry.");
                p->next = 0xFF;
                oldRO.rights.entryPtr[old_entry_index].next = p_index;
                oldRO.rights.num[permission]++;
                break;
            }
        }
        }
        else if (isNewEntryCount) {
            if (old_entry->type == DrmDef::CONSTRAINT_COUNT) {
                if (sVDebug) ALOGV("mergeEntry: count type, merge to old entry.");
                old_entry->total_count += new_entry->total_count;
                old_entry->used_count += new_entry->used_count;
                break;
            }
            else if (old_entry->next == 0xFF) {
                if (sVDebug) ALOGV("mergeEntry: count type, insert after last old entry.");
                oldRO.rights.num[permission]++;
                p->next = 0xFF;
                oldRO.rights.entryPtr[old_entry_index].next = p_index;
                break;
            }
        }
        else if (isNewEntryDT) {
            if (old_entry->type == DrmDef::CONSTRAINT_DATETIME) {
                has_dt = true;
                // check new entry start_time
                if (!determine_start_flag) {
                    if (new_entry->start_time < old_entry->start_time) {  // start_index
                        if (sVDebug) ALOGV("mergeEntry: date-time type, determine dt start index 1.");
                        dt_start_index = old_entry_index;
                        dt_status &= 0x0F;
                        dt_status |= 0x10;
                        determine_start_flag = true;
                    }
                    else if (old_entry->start_time <= new_entry->start_time
                             && new_entry->start_time <= old_entry->end_time) {
                        if (sVDebug) ALOGV("mergeEntry: date-time type, determine dt start index 2.");
                        dt_start_index = old_entry_index;
                        dt_status &= 0x0F;
                        dt_status |= 0x20;
                        determine_start_flag = true;
                    }
                    else if (new_entry->start_time > old_entry->end_time) {
                        if (sVDebug) ALOGV("mergeEntry: date-time type, found dt start index 3.");
                        dt_start_index = old_entry_index;
                        dt_status &= 0x0F;
                        dt_status |= 0x80;
                    }
                }
                // check new entry end_time
                if (!determine_end_flag) {
                    if (new_entry->end_time < old_entry->start_time) {
                        if (sVDebug) ALOGV("mergeEntry: date-time type, determine dt end index 1.");
                        dt_end_index = old_entry_index;
                        dt_status &= 0xF0;
                        dt_status |= 0x01;
                        determine_end_flag = true;
                    }
                    else if (old_entry->start_time <= new_entry->end_time
                             && new_entry->end_time <= old_entry->end_time) {  // end_index
                        if (sVDebug) ALOGV("mergeEntry: date-time type, determine dt end index 2.");
                        dt_end_index = old_entry_index;
                        dt_status &= 0xF0;
                        dt_status |= 0x02;
                        determine_end_flag = true;
                    }
                    else if (new_entry->end_time > old_entry->end_time) {
                        if (sVDebug) ALOGV("mergeEntry: date-time type, found dt end index 3.");
                        dt_end_index = old_entry_index;
                        dt_status &= 0xF0;
                        dt_status |= 0x08;
                    }
                }
            }
            else if (!has_dt) {
                int result = compareEntry(new_entry, old_entry);
                if (result > 0) {
                    if (sVDebug) ALOGV("mergeEntry: No date-time in old entry list, found place to insert.");
                    dt_status = 0xFF; // no dt in old entry list, insert
                    break;
                }
            }
        }
        // merge new entry into old entry list
        compare_pre_index = old_entry_index;
        if (!determine_start_flag) {
            dt_start_pre_index = old_entry_index;
        }
        if (!determine_end_flag) {
            dt_end_pre_index = old_entry_index;
        }
        old_entry_index = old_entry->next;
    }

    // merge DT
    if (dt_status == 0x11) {  // insert after [dt_start_pre_index]
        int count = getEntryDistance(&oldRO,
                &oldRO.rights.entryPtr[dt_start_index],
                &oldRO.rights.entryPtr[dt_end_index]);
        Entry* startEntry = &oldRO.rights.entryPtr[dt_start_index];
        if (count == 0) {  // insert before current entry
            oldRO.rights.num[permission]++;
            p->next = dt_start_index;
            if (dt_start_pre_index == -1) {
                oldRO.rights.best[permission] = p_index;
            }
            else {
                oldRO.rights.entryPtr[dt_start_pre_index].next = p_index;
            }
        }
        else if (count > 0) {  // merge & delete
            int count = getEntryDistance(&oldRO,
                    &oldRO.rights.entryPtr[dt_start_index],
                    &oldRO.rights.entryPtr[dt_end_index]);
            oldRO.rights.num[permission] -= (count - 1);
            startEntry->start_time = new_entry->start_time;
            startEntry->end_time = new_entry->end_time;
            startEntry->next = dt_end_index;
        }
    }
    else if (dt_status == 0x12 || dt_status == 0x18) {  // merge
        int count = getEntryDistance(&oldRO,
                &oldRO.rights.entryPtr[dt_start_index],
                &oldRO.rights.entryPtr[dt_end_index]);
        oldRO.rights.num[permission] -= count;
        Entry* startEntry = &oldRO.rights.entryPtr[dt_start_index];
        startEntry->start_time = new_entry->start_time;
        if (dt_status == 0x12)
            startEntry->end_time = oldRO.rights.entryPtr[dt_end_index].end_time;
        else if (dt_status == 0x18)
            startEntry->end_time = new_entry->end_time;
        startEntry->next = oldRO.rights.entryPtr[dt_end_index].next;
    }
    else if (dt_status == 0x21) {
        int count = getEntryDistance(&oldRO,
                &oldRO.rights.entryPtr[dt_start_index],
                &oldRO.rights.entryPtr[dt_end_index]);
        oldRO.rights.num[permission] -= (count - 1);
        Entry* startEntry = &oldRO.rights.entryPtr[dt_start_index];
        startEntry->end_time = new_entry->end_time;
        startEntry->next = dt_end_index;
    }
    else if (dt_status == 0x22) {
        int count = getEntryDistance(&oldRO,
                &oldRO.rights.entryPtr[dt_start_index],
                &oldRO.rights.entryPtr[dt_end_index]);
        oldRO.rights.num[permission] -= count;
        Entry* startEntry = &oldRO.rights.entryPtr[dt_start_index];
        startEntry->end_time = oldRO.rights.entryPtr[dt_end_index].end_time;
        startEntry->next = oldRO.rights.entryPtr[dt_end_index].next;
    }
    else if (dt_status == 0x28) {
        int count = getEntryDistance(&oldRO,
                &oldRO.rights.entryPtr[dt_start_index],
                &oldRO.rights.entryPtr[dt_end_index]);
        oldRO.rights.num[permission] -= count;
        Entry* startEntry = &oldRO.rights.entryPtr[dt_start_index];
        startEntry->end_time = new_entry->end_time;
        ALOGW("mergeEntry: should like 0x18.");
        startEntry->next = oldRO.rights.entryPtr[dt_end_index].next;
    }
    else if (dt_status == 0x81) {  // actually 0x81 -> 0x11
        ALOGW("mergeEntry: shouldn't have 0x81 case.");
    }
    else if (dt_status == 0x82) {  // actually 0x82 -> 0x12
        ALOGW("mergeEntry: shouldn't have 0x82 case.");
    }
    else if (dt_status == 0x88) {
        // insert as last
        oldRO.rights.num[permission]++;
        // ALPS01426919,avoid dead loop linked list.
        p->next = oldRO.rights.entryPtr[dt_end_index].next;
        Entry* startEntry = &oldRO.rights.entryPtr[dt_start_index];
        startEntry->next = p_index;
        ALOGW("mergeEntry: should like 0x18.");
    }
    else if (dt_status == 0xFF) {  // no DT in old entry list, insert before current old entry
        oldRO.rights.num[permission]++;
        p->next = old_entry_index;
        if (compare_pre_index == -1) {
            oldRO.rights.best[permission] = p_index;
        }
        else {
            oldRO.rights.entryPtr[compare_pre_index].next = p_index;
        }
    }

    // save old RO to file
    save(&oldRO);

    if (sVDebug) ALOGV("mergeEntry: merge new entry index [%d]", new_entry_index);
    return true;
}

int DrmUtil::compareEntry(Entry* e1, Entry* e2)
{
    if (e1->type == DrmDef::CONSTRAINT_NONE) {
        return 1;
    }
    if (e2->type == DrmDef::CONSTRAINT_NONE) {
        return -1;
    }

    if ((e1->type & DrmDef::CONSTRAINT_DATETIME) != 0)
    {
        if ((e2->type & DrmDef::CONSTRAINT_DATETIME) == 0)  // e2 has not dt
        {
            return 1;
        }
        else  // both e1, e2 has dt
        {
            if (e1->end_time < e2->end_time)
            {
                return 1;
            }
            else if (e1->end_time > e2->end_time)
            {
                return -1;
            }
            else  // e1/e2 dt end_time equal
            {
                if (((e1->type & DrmDef::CONSTRAINT_INTERVAL) != 0)
                    && ((e2->type & DrmDef::CONSTRAINT_INTERVAL) == 0))  // e1 has interval, e2 no interval
                {
                    return 1;
                }
                else if (((e1->type & DrmDef::CONSTRAINT_INTERVAL) == 0)
                         && ((e2->type & DrmDef::CONSTRAINT_INTERVAL) != 0))  // e1 no interval, e2 has interval
                {
                    return -1;
                }
                else  // e1/e2 both has/no interval
                {
                    if ((e1->type & DrmDef::CONSTRAINT_COUNT) != 0)  // e1 has count
                    {
                        return -1;
                    }
                    else // e1 no count
                    {
                        return 1;
                    }
                }
            }
        }
    }
    else if ((e1->type & DrmDef::CONSTRAINT_INTERVAL) != 0)
    {
        if ((e2->type & DrmDef::CONSTRAINT_DATETIME) != 0) // e2 has dt
        {
            return -1;
        }
        else if ((e2->type & DrmDef::CONSTRAINT_COUNT) != 0) // e2 has count
        {
            return 1;
        }
        else  // e2 has interval
        {
            return 0; // both only interval -> equal
        }
    }
    else if ((e1->type & DrmDef::CONSTRAINT_COUNT) != 0)
    {
        if (((e2->type & DrmDef::CONSTRAINT_DATETIME) != 0)
            || ((e2->type & DrmDef::CONSTRAINT_INTERVAL) != 0))
        {
            return -1;
        }
        else // e2 has count
        {
            return 0; // both only cout -> equal
        }
    }
    return 1;
}

void DrmUtil::getFirstAvailableEntryIndex(RO* ro, int* avIndex, int* entryNum)
{
    if (sVDebug) ALOGV("getFirstAvailableEntryIndex ---->");

    // get entry number
    String8 ro_file = getROFileName(String8(ro->rights.cid));
    int totalEntryNumber = getEntryNum(ro_file);
    *entryNum = totalEntryNumber;

    List<int> indexList;
    for (int i = DrmDef::PERMISSION_PLAY_INDEX;
         i <= DrmDef::PERMISSION_PRINT_INDEX; i++)
    {
        int entry_index = ro->rights.best[i];
        while (entry_index != 0xFF)
        {
            Entry* entry = &ro->rights.entryPtr[entry_index];
            indexList.push_back(entry_index);
            entry_index = entry->next;
        }
    }

    *avIndex = -1;
    for (int index = 0; index < totalEntryNumber; index++)
    {
        List<int>::iterator it;
        bool containFlag = false;
        for (it = indexList.begin(); it != indexList.end(); ++it)
        {
            if (*it == index)
            {
                containFlag = true;
                break;
            }
        }
        if (!containFlag)
        {
            *avIndex = index;
            break;
        }
    }
    if (sDDebug) ALOGD("getFirstAvailableEntryIndex: available [%d], total entry num [%d]",
            *avIndex, *entryNum);
}

int DrmUtil::getEntryDistance(RO* ro, Entry* e1, Entry* e2)
{
    Entry* p = e1;
    int count = 0;
    while (p != e2)
    {
        p = &ro->rights.entryPtr[p->next];
        count++;
    }
    return count;
}

bool DrmUtil::traverseRights(String8 cid, int permission, int mode)
{
    Entry entry;
    return traverseRights(cid, permission, mode, entry);
}

bool DrmUtil::traverseRights(String8 cid, int permission, int mode, Entry& outEntry)
{
    if (sDDebug) ALOGD("traverseRights: cid[%s], permission[%d], mode[%d]", cid.string(), permission, mode);

    Mutex::Autolock lock(mROLock);

    RO ro;
    if (!restore(cid, &ro))
    {
        ALOGE("traverseRights: failed to restore ro.");
        return false;
    }

    // check rights
    int permissionIndex = DrmDef::getPermissionIndex(permission);
    if (permissionIndex < 0) {
        ALOGE("traverseRights: permission index is negative[%d]", permissionIndex);
        return false;
    }
    int index = ro.rights.best[permissionIndex];
    if (index == 0xFF)
    {
        ALOGE("traverseRights: No permission [%s]",
                DrmDef::getPermissionStr(permissionIndex).string());
        return false;
    }

    bool isDirty = false;
    bool result = false;
    int preValidIndex = -1;
    while (index != 0xFF)
    {
        if (sVDebug) ALOGV("traverseRights: check index [%d]", index);
        Entry* entry = &(ro.rights.entryPtr[index]);
        if (isEntryValid(entry))
        {
            preValidIndex = index;
            switch (mode)
            {
                case Mode::ConsumeRights:
                {
                    if (!isEntryUsable(entry)) {
                        // check next entry
                        if (sDDebug) ALOGD("traverseRights: entry not usable for consume, check next");
                        index = entry->next;
                        result = false;
                        continue;
                    }

                    // now consume the entry
                    if (0 != (entry->type & DrmDef::CONSTRAINT_COUNT)
                        && entry->used_count > 0
                        && entry->total_count > 0)
                    {
                        entry->used_count--;
                        if (sDDebug) ALOGD("traverseRights: consumed count [%ld]->[%ld]",
                                entry->used_count + 1, entry->used_count);
                        isDirty = true;
                    }

                    if (0 != (entry->type & DrmDef::CONSTRAINT_INTERVAL)
                        && entry->start_intv == 0)
                    {
                        // the secure timer shall be in valid state becasue "checkRightsStatus" has verified it
                        time_t t = 0;
                        int timeResult = SecureTimer::instance().getDRMTime(t);
                        timeResult = timeResult;

                        entry->start_intv = t; // set current trusted time as start time
                        entry->end_intv = entry->start_intv + entry->interval;
                        if (sDDebug) ALOGD("traverseRights: consumed interval [%s]->[%s]",
                                StrUtil::toTimeStr((time_t*)(&entry->start_intv)).string(),
                                StrUtil::toTimeStr((time_t*)(&entry->end_intv)).string());
                        isDirty = true;
                    }

                    result = true; // consume done
                }
                break;

                case Mode::CheckRightsStatus:
                {
                    result = isEntryUsable(entry);
                }
                break;

                case Mode::GetConstraints:
                {
                    outEntry = *entry;
                    result = true;
                }
                break;

                default:
                break;
            }

            if (result) {
                break; // break loop
            }
        }
        else // check to see if the next entry is valid
        {
            // if a current entry is no longer valid, we mark the ro to be saved (isDirty)
            isDirty = true;
            ro.rights.num[permissionIndex]--;

            // and we jump across this invalid entry
            if (sDDebug) ALOGD("traverseRights: rights become dirty, previous valid entry index [%d]",
                    preValidIndex);
            if (preValidIndex == -1)
            {
                ro.rights.best[permissionIndex] = entry->next;
            }
            else
            {
                ro.rights.entryPtr[preValidIndex].next = entry->next;
            }
        }

        // check next entry
        index = entry->next;
    }

    if (isDirty)
    {
        if (sDDebug) ALOGD("traverseRights: rights become dirty, save it.");
        save(&ro);
    }

    if (sDDebug) ALOGD("traverseRights: result [%d]", result);
    return result;
}


xmlNodePtr DrmUtil::getNode(xmlDocPtr doc, String8 path)
{
    if (sVDebug) ALOGV("getNode ----> [%s]", path.string());

    xmlNodePtr cur = NULL;
    bool first = true;
    while (true)
    {
        String8 base = path.walkPath(&path);
        if (base.length() == 0)
        {
            if (sVDebug) ALOGV("getNode: zero length, break");
            break;
        }

        if (first)
        {
            cur = xmlDocGetRootElement(doc);
        }
        else
        {
            cur = cur->xmlChildrenNode;
            while (cur != NULL)
            {
                if (strcmp((char*)(cur->name), base.string()) != 0)
                {
                    cur = cur->next;
                }
                else
                {
                    break;
                }
            }
        }

        if (cur == NULL || strcmp((char*)(cur->name), base.string()) != 0)
        {
            ALOGE("getNode: cur->name[%s], base[%s]",
                    cur == NULL ? "cur=NULL" : (char*) (cur->name), base.string());
            return NULL;
        }
        first = false;
    }
    return cur;
}

xmlDocPtr DrmUtil::getXmlDocPtr(DrmBuffer& drmBuffer)
{
    xmlDocPtr doc = xmlParseMemory(drmBuffer.data, drmBuffer.length);
    return doc;
}

xmlDocPtr DrmUtil::getXmlDocPtr(String8 filePath, int offset, int len)
{
    xmlDocPtr doc;

    if (offset > 0 && len > 0)
    { // mms package with drm ro
        if (sVDebug) ALOGV("getXmlDocPtr: Gonna call xmlParseMemory.");
        FILE* fp;
        if (!FileUtil::fopenx(filePath.string(), "r+b", &fp))
        {
            ALOGE("getXmlDocPtr: open file failed.");
            return NULL;
        }

        char* mmsROBuffer = new char[len + 1];
        bzero(mmsROBuffer, len + 1);
        FileUtil::fseekx(fp, offset, SEEK_SET);
        if (!FileUtil::freadx(mmsROBuffer, len, fp))
        {
            ALOGE("getXmlDocPtr: read file failed.");
            FileUtil::fclosex(fp);
            delete[] mmsROBuffer;
            return NULL;
        }
        FileUtil::fclosex(fp);

        doc = xmlParseMemory(mmsROBuffer, len);
        delete[] mmsROBuffer;
    }
    else
    {
        if (sVDebug) ALOGV("getXmlDocPtr: Gonna call xmlParseFile.");
        doc = xmlParseFile(filePath.string());
    }

    return doc;
}


int DrmUtil::readUintVar(FILE* fp)
{
    int value = 0;
    do
    {
        char c = 0;
        if (!FileUtil::freadx(&c, 1, fp))
        {
            ALOGE("readUintVar: failed to read from file.");
            return -1;
        }

        value <<= 7;
        value |= (c & 0x7F);
        if ((c & 0x80) == 0)
        {
            break;
        }
    }
    while (true);
    return value;
}


// only when entry expired, then we think it's invalid.
// future entry, we think it's still valid.
bool DrmUtil::isEntryValid(Entry* entry)
{
    if (NULL == entry)
    {
        ALOGE("isEntryValid: invalid entry parameter.");
        return false;
    }
    if (sVDebug) ALOGV("isEntryValid: entry->type[%d]", entry->type);
    if ((entry->type & DrmDef::CONSTRAINT_COUNT) != 0)
    {
        if (sDDebug) ALOGD("isEntryValid: entry has constraint type [count]");
        if (entry->used_count <= 0)
        {
            if (sDDebug) ALOGD("isEntryValid: left used_count [%ld], entry invalid.",
                    entry->used_count);
            return false;
        }
    }

    if ((entry->type & DrmDef::CONSTRAINT_DATETIME) != 0)
    {
        long max = DrmDef::MAX_END_TIME;
        long min = DrmDef::MIN_START_TIME;
        if (entry->start_time == min
            && entry->end_time == max)
        {
            if (sDDebug) ALOGD("isEntryValid: both start_time & end time not specified, entry invalid.");
            return false;
        }
    }

    long lt = 0;
    if ((entry->type & DrmDef::CONSTRAINT_DATETIME) != 0
        || (entry->type & DrmDef::CONSTRAINT_INTERVAL) != 0)
    {
        if (sDDebug) ALOGD("isEntryValid: entry has constraint type [date_time]/[interval], check secure timer");
        if (!SecureTimer::instance().isValid())
        {
            if (sDDebug) ALOGD("isEntryValid: secure timer invalid. we assume entry to be valid.");
            return true;
        }

        int result = SecureTimer::instance().getDRMTime(lt);
    }

    if ((entry->type & DrmDef::CONSTRAINT_DATETIME) != 0)
    {
        if (sDDebug) ALOGD("isEntryValid: entry has constraint type [date_time]");
        if (entry->end_time > 0 && entry->end_time <= lt)
        {
            if (sDDebug) ALOGD("isEntryValid: end_time [%ld] had expired, entry invalid.",
                    entry->end_time);
            return false;
        }
    }

    if ((entry->type & DrmDef::CONSTRAINT_INTERVAL) != 0)
    {
        if (sDDebug) ALOGD("isEntryValid: entry has constraint type [interval]");
        if ((entry->start_intv > 0
            && entry->interval > 0
            && (entry->start_intv + entry->interval) <= lt)
            || (entry->start_intv > 0 && entry->start_intv > lt))
        {
            if (sDDebug) ALOGD("isEntryValid: start_intv [%ld] interval [%ld] had expired, entry invalid.",
                    entry->start_intv, entry->interval);
            return false;
        }
    }

    return true;
}

// check if the entry is usable for current constraints
bool DrmUtil::isEntryUsable(Entry* entry)
{
    if (sVDebug) ALOGV("isEntryUsable ---->");

    if ((entry->type & DrmDef::CONSTRAINT_COUNT) != 0)
    {
        if (sDDebug) ALOGD("isEntryUsable: entry has constraints type [count]");
        if (entry->used_count <= 0)
        {
            if (sDDebug) ALOGD("isEntryUsable: left used_count [%ld], entry not usable.",
                    entry->used_count);
            return false;
        }
    }

    if ((entry->type & DrmDef::CONSTRAINT_DATETIME) != 0)
    {
        long max = DrmDef::MAX_END_TIME;
        long min = DrmDef::MIN_START_TIME;
        if (entry->start_time == min
            && entry->end_time == max)
        {
            if (sDDebug) ALOGD("isEntryUsable: both start_time & end time not specified, entry not usable.");
            return false;
        }
    }

    long lt = 0;
    if ((entry->type & DrmDef::CONSTRAINT_DATETIME) != 0
        || (entry->type & DrmDef::CONSTRAINT_INTERVAL) != 0)
    {
        if (sDDebug) ALOGD("isEntryUsable: entry has constraints type [date_time]/[interval], check secure timer");
        if (!SecureTimer::instance().isValid())
        {
            ALOGE("isEntryUsable: secure timer invalid. entry not usable.");
            return false;
        }

        int result = SecureTimer::instance().getDRMTime(lt);
    }

    if ((entry->type & DrmDef::CONSTRAINT_DATETIME) != 0)
    {
        if (sDDebug) ALOGD("isEntryUsable: entry has constraints type [date_time]");
        if (entry->end_time > 0 && entry->end_time <= lt)
        {
            if (sDDebug) ALOGD("isEntryUsable: end_time [%ld] has expired, entry not usable.",
                    entry->end_time);
            return false;
        }

        if (entry->start_time > 0 && entry->start_time > lt)
        {
            if (sDDebug) ALOGD("isEntryUsable: start_time [%ld] not reached, entry not usable.",
                    entry->start_time);
            return false;
        }
    }

    if ((entry->type & DrmDef::CONSTRAINT_INTERVAL) != 0)
    {
        if (sDDebug) ALOGD("isEntryUsable: entry has constraints type [interval], start_intv [%ld] interval [%ld] lt [%ld]", entry->start_intv, entry->interval, lt);
        if(entry->interval == 0)
        {
            if (sDDebug) ALOGD("isEntryUsable: entry->interval is 0, entry not usable");
            return false;
        }
        if ((entry->start_intv > 0
            && entry->interval > 0
            && (entry->start_intv + entry->interval) <= lt)
            || (entry->start_intv > 0 && entry->start_intv > lt))
        {
            if (sDDebug) ALOGD("isEntryUsable: start_intv [%ld] interval [%ld] has expired, entry not usable.",
                    entry->start_intv, entry->interval);
            return false;
        }
    }

    return true;
}

// -1 that no entry valid
int DrmUtil::getEntryNum(String8 ro_file)
{
    if (sDDebug) ALOGD("getEntryNum ----> ro_file [%s]", ro_file.string());

    struct stat stFileInfo;
    if (-1 == stat(ro_file.string(), &stFileInfo))
    {
        ALOGE("getEntryNum: failed to get ro_file info, reason [%s]", strerror(errno));
        return -1;
    }

    // file size shall be valid.
    if (stFileInfo.st_size < sizeof(struct OldRights)) // now check old rights struct's size
    {
        ALOGE("getEntryNum: invalid ro_file file size [%lld]", stFileInfo.st_size);
        return -1;
    }

    FILE* fp = NULL;
    if (!FileUtil::fopenx(ro_file.string(), "rb", &fp))
    {
        ALOGE("getEntryNum: failed to open ro_file.");
        return -1;
    }

    FileUtil::fseekx(fp, getRightsPosition(), SEEK_SET);

    // first we check the beginning of the RO file for RO_VERSION_01_0 string.
    char ver[SIZE_VER_01_0];
    bzero(ver, SIZE_VER_01_0);
    if (!FileUtil::freadx(ver, strlen(RO_VERSION_01_0), fp))
    {
        FileUtil::fclosex(fp);
        ALOGE("getEntryNum: failed to read possible rights object verison from file.");
        return -1;
    }
    FileUtil::fclosex(fp);
    if (sVDebug) ALOGV("getEntryNum: compare version [%s] with [%s]", ver, RO_VERSION_01_0);
    int version = 0;
    if (0 == strcmp(ver, RO_VERSION_01_0))
    {
        version = VER_01_0;
    }

    size_t ro_size =
            (0 == version) ? sizeof(struct OldRights) : (sizeof(struct Rights) + strlen(RO_VERSION_01_0));
    int totalEntryNum =
            (stFileInfo.st_size - ro_size) / sizeof(struct Entry);
    if (sDDebug) ALOGD("getEntryNum: total entry num [%d], Rights size [%d], Entry size [%d]",
            totalEntryNum, ro_size, sizeof(struct Entry));

    if (0 != (stFileInfo.st_size - ro_size) % sizeof(struct Entry))
    {
        ALOGE("getEntryNum: the ro_file file size is not correct.");
        return -1;
    }

    return totalEntryNum;
}

#endif

String8 DrmUtil::readIMEI()
{
    if (sVDebug) ALOGV("readIMEI ---->");
    // for getDrmKey() purpose, if the id is empty "",
    // we use a "000000000000000" instead
    String8 id = readId();
    if (id.isEmpty())
    {
        getMAC(id);
        if (sDDebug) ALOGD("readIMEI: empty id, get mac address[%s] as new id", id.string());
        saveId(id);
        return id;
    }
    return id;
}

String8 DrmUtil::readId()
{
    if (sVDebug) ALOGV("readId ---->");
    static String8 result;
    if(!result.isEmpty())
    {
        if (sDDebug) ALOGD("readId: Use cached id[%s]", result.string());
        return result;
    }

    FILE* fp = fopen(ID_FILE.string(), "rb");
    if (NULL == fp)
    {
        ALOGE("readId: failed to open id data file, reason [%s].", strerror(errno));
        String8 result("");
        getMAC(result);
        if (sDDebug) ALOGD("readId: get mac address[%s] as new id", result.string());
        saveId(result);
        return result;
    }

    BYTE data[DrmDef::DRM_MAX_ID_LENGTH];
    bzero(data, sizeof(data));

    fseek(fp, 0, SEEK_SET);
    if (0 >= fread(data, sizeof(BYTE), sizeof(data) - 1, fp))
    {
        fclose(fp);
        return String8(""); // id.dat file error, empty string returned
    }

    fclose(fp);
    result.clear();
    result.setTo((const char*)data, strlen((const char*)data));
    if (sDDebug) ALOGD("readId: [%s]", result.string());
    return result;
}

int DrmUtil::saveId(const String8& id)
{
    if (sVDebug) ALOGV("saveId: new id[%s]", id.string());

    // Remove the original one first
    remove(ID_FILE.string());

    // write to file
    checkDir(ID_DIR);
    FILE* fp = fopen(ID_FILE.string(), "wb");
    if (NULL == fp)
    {
        ALOGE("saveId: failed to open data file, reason [%s].",
                strerror(errno));
        return -1;
    }

    // mod 755 so that the drm manager process can access it.
    chmod(ID_DIR.string(), 0755);
    chmod(ID_LOCATION.string(), 0755);

    fseek(fp, 0, SEEK_SET);
    size_t id_len = id.length();
    if (sVDebug) ALOGV("saveId: the length of id [%d]", (int) id_len);

    if (id_len != fwrite(id.string(), 1, id_len, fp))
    {
        ALOGE("saveId: failed to write to data file, reason [%s].", strerror(errno));
        fclose(fp);
        remove(ID_FILE.string());
        return -1;
    }

    fclose(fp);
    chmod(ID_FILE.string(), 0644);
    return 0;
}

// returns seconds west of greenwhich of current time zone.
int DrmUtil::getTZOffset()
{
    tzset(); // update time zone setting information
    struct timezone tz;
    gettimeofday(NULL, &tz);

    int minute = tz.tz_minuteswest;
    if (sVDebug) ALOGV("getTZOffset: [%d] minutes west of greenwhich", minute);
    return minute * 60;
}

/**
 * MAC address like 00:11:ee:ff:aa:10
 */
void DrmUtil::getMAC(String8& mac)
{
    // generate a random id as mac, don't use nvram to read mac, because from O, Google don't allow
    // vendor so link libbinder.so
    srand(time(NULL)); // initialize seed
    int number = rand();
    number++;
    String8 result("");
    int i = 0;
    for (; i < 10; ++i)
    {
        mac.appendFormat("%02x", rand() % 128);
    }
    if (sDDebug) ALOGD("generate a random id [%s]", mac.string());
}

String8 DrmUtil::getPathFromFd(int fd)
{
    char buffer[256];
    char linkto[4096];
    memset(buffer, 0, 256);
    memset(linkto, 0, 4096);
    snprintf(buffer, sizeof(buffer), "/proc/%d/fd/%d", gettid(), fd);
    int len = 0;
    len = readlink(buffer, linkto, sizeof(linkto));
    if(len > 4096)
    {
        ALOGE("The file path is too long : %d", len);
        String8 path;
        return path;
    }
    return String8::format("%s",linkto);

}
