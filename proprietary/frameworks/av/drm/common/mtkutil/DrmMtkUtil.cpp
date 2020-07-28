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

#define LOG_NDEBUG 0
#define LOG_TAG "DrmMtkUtil"
#include <utils/Log.h>

#include <drm/drm_framework_common.h>
#include <drm/DrmMetadata.h>
#include <drm/DrmRights.h>
#include <drm/DrmManagerClient.h>
#include <drm/DrmMtkUtil.h>
#include <drm/DrmMtkDef.h>

#include <RO.h>
#include <DrmDef.h>
#include <DrmTypeDef.h>
#include <ByteBuffer.h>
#include <CipherFileUtil.h>
#include <CryptoHelper.h>
#include <DrmUtil.h>
#include <DrmMessageParser.h>
#include <FileUtil.h>
#include <StrUtil.h>
#include <WBUtil.h>
#include <Cta5FileFactory.h>
#include <Cta5File.h>

#include <libxml/parser.h>
#include <libxml/tree.h>
#include <openssl/md5.h>
#include <openssl/sha.h>
#include <openssl/hmac.h>
#include <openssl/evp.h>
#include <openssl/bio.h>
#include <openssl/buffer.h>
#include <openssl/rc4.h>

#include <cutils/properties.h>
#include <utils/List.h>
#include <utils/String8.h>
#include <utils/threads.h>

#include <ctype.h>
#include <dirent.h>
#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

#define LINE_MAX_SIZE 600

#define BLOCK_SIZE 2032

#define BASE64_LINE_SIZE 76
#define BASE64_DECRYPT_SIZE 4096

#define BASE64_STYLE_DEFAULT 0
#define BASE64_STYLE_LINE_SEPARATE 1
#define MAX_HEADER_LINES 5

using namespace android;

#if 0
int DrmMtkUtil::correctDcf(const String8& dmPath) {
    if (DrmUtil::sDDebug) ALOGD("correctDcf: to be corrected: [%s]", dmPath.string());

    if (!DrmUtil::checkExistence(dmPath))
    {
        ALOGE("correctDcf: file does not exist.");
        return false;
    }

    int fd = open(dmPath.string(), O_RDONLY);
    if (-1 == fd)
    {
        ALOGE("correctDcf: failed to open file to read, reason [%s]",
                strerror(errno));
        return false;
    }

    DrmMetadata* pMetaData = new DrmMetadata();
    int parseResult = DrmUtil::parseDcf(fd, pMetaData);
    if (DrmUtil::sDDebug) ALOGD("correctDcf: result [%d]", parseResult);
    close(fd);

    if (0 == parseResult) {
        if (DrmUtil::sDDebug) ALOGD("correctDcf: correct DCF format.");
        delete pMetaData;
        return 0;
    }

    if (-2 == parseResult) {
        if (DrmUtil::sDDebug) ALOGD("correctDcf: incorrect DCF file length, try to truncate it.");

        String8 data_offset = pMetaData->get(String8(DrmMetaKey::META_KEY_OFFSET));
        const char* pDataOffset = data_offset.string();
        int dataOffset = atoi(pDataOffset);

        String8 data_length = pMetaData->get(String8(DrmMetaKey::META_KEY_DATALEN));
        const char* pDataLength = data_length.string();
        int dataLength = atoi(pDataLength);

        int desiredFileLength = dataOffset + dataLength;
        delete pMetaData;
        int result = truncate(dmPath.string(), (off_t)desiredFileLength);
        if (-1 == result) {
            ALOGE("correctDcf : failed to truncate file to desired length, reason [%s]",
                    strerror(errno));
            return -1;
        }
        return 0;
    }
    delete pMetaData;

    return -1;
}

bool DrmMtkUtil::installDrmMsg(const String8& dmPath) {
    if (DrmUtil::sDDebug) ALOGD("installDrmMsg: DRM message file: [%s]", dmPath.string());

    if (DrmUtil::checkDcf(dmPath, NULL)) {
        // the file's already a DCF format in the beginning.
        // however the DCF format may not be correct
        if (DrmUtil::sDDebug) ALOGD("installDrmMsg: already a DCF format, correct it");
        int correctResult = correctDcf(dmPath);
        if (0 != correctResult) {
            ALOGE("installDrmMsg: invalid dm file!");
        }
        return (0 == correctResult);
    }

    FILE* fp_dm = NULL;
    FILE* fp_dcf = NULL;
    String8 dcfPath;
    // After each boundary line, each body part then consist of a header area,
    // a blank line, and a body area.
    String8 boundary;             // boundary
    Vector<String8> headerLines;  // header lines
    String8 encoding;             // the encoding of the content
    String8 mime;                 // the mime type of the content
    int dmMethod = DrmDef::METHOD_NONE;
    bool ret = false;
    if (!FileUtil::fopenx(dmPath.string(), "r+", &fp_dm)) {
        ALOGE("installDrmMsg: failed to open the dm file [%s]!", dmPath.string());
        goto Exit;
    }

    // the first line should be non-empty, and is the boundary (delimiter string)
    if (!getNextNELineTrimR(fp_dm, boundary)) {
        ALOGE("installDrmMsg: failed to get the first boundary line!");
        goto Exit;
    }
    if (DrmUtil::sDDebug) ALOGD("installDrmMsg: first boundary line: [%s]", boundary.string());

    // header area
    ret = getLines(fp_dm, MAX_HEADER_LINES, true /*trim*/, headerLines);
    if (!ret) {
        ALOGE("installDrmMsg: failed to get headers lines!" );
        goto Exit;
    }

    // parse header to get content encoding, mime and drmMethod
    ret = parseHeaders(headerLines, encoding, mime, dmMethod);
    if (!ret) {
        ALOGE("installDrmMsg: No valid Content-Type or Content-Transfer-Encoding header!");
        goto Exit;
    }

    // Forward Lock Only
    // we check according to the property "vendor.drm.forwardlock.only"
    if (isForwardlockSet()) {
        if (DrmUtil::sDDebug) ALOGD("installDrmMsg: Forward-lock-only has been set!");
        if (dmMethod != DrmDef::METHOD_FL) {
            ALOGE("installDrmMsg: Objects with method=[%d] will be abandoned!", dmMethod);
            ret = false;
            goto Exit;
        }
    }

    // generate xx.dcf.tmp file
    getDcfTmpFileName(dmPath, dcfPath);
    if (!FileUtil::fopenx(dcfPath.string(), "w", &fp_dcf)) {
        ALOGE("installDrmMsg: failed to open file [%s]!", dcfPath.string());
        goto Exit;
    }

    if (dmMethod == DrmDef::METHOD_FL) {
        ret = installFl(fp_dm, boundary, encoding, mime, fp_dcf);
    } else if (dmMethod == DrmDef::METHOD_CD) {
        ret = installCd(fp_dm, boundary, fp_dcf);
    } else if (dmMethod == DrmDef::METHOD_FLDCF) {
        ret = installFlDcf(fp_dm, boundary, encoding, fp_dcf);
    } else {
        ALOGE("installDrmMsg: invalid method [%d]!", dmMethod);
        ret = false;
    }

Exit:
    if (fp_dcf) {
        FileUtil::fclosex(fp_dcf);
    }
    if (fp_dm) {
        FileUtil::fclosex(fp_dm);
    }

    if (DrmUtil::sDDebug) ALOGD("installDrmMsg: succeed=[%d]", ret);
    return ret;
}

String8 DrmMtkUtil::getContentId(const DrmRights& rights) {
    // check mime in drmRights
    if (rights.getMimeType() != DrmDef::MIME_RIGHTS_WBXML
        && rights.getMimeType() != DrmDef::MIME_RIGHTS_XML) {
        ALOGE("getContentId: the MIME type [%s] must be xml or wbxml.",
                rights.getMimeType().string());
        return String8("");
    }

    // parse ro
    RO newRO;
    DrmBuffer drmBuffer = rights.getData();
    bool parseResult = false;
    if (rights.getMimeType() == DrmDef::MIME_RIGHTS_WBXML) {
        parseResult = DrmUtil::parse_drc(drmBuffer, &newRO);
    } else if (rights.getMimeType() == DrmDef::MIME_RIGHTS_XML) {
        parseResult = DrmUtil::parse_dr(drmBuffer, &newRO);
    }
    if (!parseResult) {
        ALOGE("getContentId: failed to parse RO.");
        return String8("");
    }

    String8 result = String8(newRO.rights.cid);
    return result;
}

// get next non-empty line which contains specified string.
// used to find the boundary line; for install DRM message
// we assume that the boundary line shall be shorter than (LINE_MAX_SIZE)
bool DrmMtkUtil::getNextNELineContain(FILE* fp, String8& line, String8 contained) {
    while (true) {
        char str[LINE_MAX_SIZE];
        bzero(str, sizeof(str));

        if (!FileUtil::fgetsx(fp, str, sizeof(str))) {
            ALOGE("getNextNELineContain: failed to get a string from file.");
            return false;
        }
        if (strstr(str, contained.string()) != NULL) {
            line.setTo(str);
            return true;
        }
    }

    return false;
}

// get next non-empty line
// for install DRM message
// this result {line} will contain CR LF
// note: use a LINE_MAX_SIZE buffer for {line}
int DrmMtkUtil::getNextNELine(FILE* fp, char* line) {
    while (true) {
        char str[LINE_MAX_SIZE];
        bzero(str, LINE_MAX_SIZE);

        long startIndex = 0;
        if (!FileUtil::ftellx(fp, startIndex)) {
            ALOGE("getNextNELine: failed to tell start index!");
            return -1;
        }

        if (!FileUtil::fgetsx(fp, str, LINE_MAX_SIZE)) {
            ALOGE("getNextNELine: failed to get a string from file.");
            return -1;
        }

        long endIndex = 0;
        if (!FileUtil::ftellx(fp, endIndex)) {
            ALOGE("getNextNELine: failed to tell end index!");
            return -1;
        }
        long readLength = endIndex - startIndex;

        // 2 kinds of line separator: if CR+LF (Dos case), the line size should be longer than 2;
        // if LF only (Xnix case), and when the line size is just 2, the first character should not be CR
        if (readLength > 2 || (readLength == 2 && str[0] != 0x0D)) {
            memcpy(line, str, readLength);
            return readLength;
        }
    }

    return -1; // otherwise -1 to indicate failure
}

// get next non-empty line, not including CR LF
// returns false if the result line is empty.
bool DrmMtkUtil::getNextNELineTrimR(FILE* fp, String8& line) {
    if (!getLineX(fp, line)) {
        return false;
    }
    line = StrUtil::trimRCRLF(line);
    return !line.isEmpty();
}

// get next line (extended, could including CR LF)
// used for install DRM message
bool DrmMtkUtil::getLineX(FILE* fp, String8& line) {
    char str[LINE_MAX_SIZE];
    bzero(str, sizeof(str));

    if (!FileUtil::fgetsx(fp, str, sizeof(str))) {
        ALOGE("getLineX: failed to get a line from DRM message file.");
        return false;
    }

    line.setTo(str);
    return true;
}

// get next line, not including CR LF
// used for install DRM message
bool DrmMtkUtil::getLineXTrimR(FILE* fp, String8& line) {
    if (!getLineX(fp, line)) {
        return false;
    }
    line = StrUtil::trimRCRLF(line);
    return true;
}

bool DrmMtkUtil::getFLSignature(FILE* fp, int offset, int dataLen __attribute__((unused)),
                                char* sig) {
    int seed = (long)time(NULL) % (128 - 16);
    sig[0] = seed;
    if (!FileUtil::fseekx(fp, offset + seed, SEEK_SET)) {
        ALOGE("getFLSignature: failed to seek to [%d]", offset + seed);
        return false;
    }
    if (DrmUtil::sVDebug) ALOGV("getFLSignature: offset [%d] + seed [%d] = [%d]", offset, seed, offset + seed);
    if (!FileUtil::freadx(&sig[1], 16, fp)) {
        ALOGE("getFLSignature: failed to read 16 bytes from file.");
        return false;
    }
    for (int i = 0; i < 17; i++) {
        // make sure that all the bytes are not '\0'
        if (0 == sig[i]) {
            sig[i] = 48; // replace with a '0' digit
        }
    }
    return true;
}

// two cases: dm file ends with ".mp3.dm", download manager changes "xxx.mp3.dm" to "xxx.mp3.dcf"
// dm file ends with ".mp3", download manager add ".dcf", changes "xxx.mp3" to "xxx.mp3.dcf"
void DrmMtkUtil::getDcfTmpFileName(String8 dm, String8& dcfTmp) {
    // the download manager has changed the name (drm msg file) to /xxx/xxx.mp3.dcf
    String8 fileName = dm.getPathLeaf(); // just file name. --> xxx.mp3.dcf
    String8 fileNameWithoutExt = fileName.getBasePath(); // file name without extension. --> xxx.mp3

    String8 dcf = (dm.getPathDir() + "/" + fileNameWithoutExt + ".dcf.tmp");
    if (DrmUtil::sVDebug) ALOGV("getDcfTmpFileName: [%s]->[%s]", dm.string(), dcf.string());
    dcfTmp = dcf;
}

// after install drm message file, change the name back to an original one: /xxx/xxx.mp3.dcf
void DrmMtkUtil::renameDcfTmp(String8 dcfTmp) {
    if (rename(dcfTmp.string(), dcfTmp.getBasePath().string()) != 0) {
        ALOGE("renameDcfTmp: failed to rename [%s]", dcfTmp.string());
    }
}

int DrmMtkUtil::saveIMEI(const String8& imei) {
    return DrmUtil::saveId(imei);
}

int DrmMtkUtil::saveId(const String8& id) {
    return DrmUtil::saveId(id);
}

String8 DrmMtkUtil::loadId() {
    return DrmUtil::readId();
}

bool DrmMtkUtil::isDcf(int fd) {
    if (fd < 0) {
        ALOGE("isDcf: invalid file descriptor %d", fd);
        return false;
    }
    int dupClearFd = dup(fd);
    int result = DrmUtil::checkDcf(fd, NULL);

    /*
     * add cta check
     */
    if (-1 == dupClearFd)
    {
        ALOGE("[ERROR]isDcf() : failed to dup fd, reason [%s]", strerror(errno));
        return false;
    }

    bool isCta5 = Cta5File::isCta5File(dupClearFd);
    close(dupClearFd);
    if (DrmUtil::sDDebug) ALOGD("isDcf: result [%d], isCta5:result [%d]", result, isCta5);
    return (isCta5 || (result < 0 ? false : true));
}

bool DrmMtkUtil::isDcf(const String8& path) {
    bool isDcf = (0 == strcasecmp(path.getPathExtension().string(), ".dcf"));
    int fd_in = open(path.string(), O_RDONLY);
    if (-1 == fd_in)
    {
        ALOGE("[ERROR]isDcf() : failed to dup fd, reason [%s]", strerror(errno));
        return false;
    }

    bool isCta5 = Cta5File::isCta5File(fd_in);
    close(fd_in);
    if (DrmUtil::sDDebug) ALOGD("isDcf: result [%d], isCta5:result [%d]", isDcf, isCta5);
    return (isDcf || isCta5);
}

bool DrmMtkUtil::isDcf(const String8& path, int fd) {
    if (path.length() > 0) {
        return isDcf(path);
    }

    // when path is not available, we'd use file descriptor instead
    return isDcf(fd);
}

String8 DrmMtkUtil::getDcfMime(int fd) {
    char mime[DrmDef::DRM_MAX_MIME_LENGTH];
    bzero(mime, sizeof(mime));
    int result = DrmUtil::checkDcf(fd, mime);
    return result < 0 ? String8("") : String8(mime);
}

String8 DrmMtkUtil::getDcfMime(const String8& path) {
    char mime[DrmDef::DRM_MAX_MIME_LENGTH];
    bzero(mime, sizeof(mime));
    bool result = DrmUtil::checkDcf(path, mime);
    return result ? String8(mime) : String8("");
}
#endif
typedef struct {
    const char* mime;
    const char* cmMime; // common mime
} MimeMap;

String8 DrmMtkUtil::toCommonMime(const char* mime) {
    MimeMap mimeMaps [] = {
        {"audio/mp3", "audio/mpeg"},
        {"audio/m4a", "audio/mp4"},
        {"audio/3gpp2", "audio/mp4"},
        {"audio/iMelody", "audio/midi"},
        {"audio/imy", "audio/imelody"},
        {"audio/mid", "audio/midi"},
        {"audio/rtx", "audio/midi"},
        {"audio/smf", "audio/midi"},
        {"audio/xmf", "audio/midi"},
        {"audio/mobile-xmf", "audio/midi"},
        {"image/bmp", "image/x-ms-bmp"},
        {"image/wbmp", "image/vnd.wap.wbmp"},
        {"image/jpg", "image/jpeg"},
        {"video/m4v", "video/mp4"},
        {"video/3gp", "video/3gpp"},
    };

    const char* cmMime = NULL;
    for (size_t i = 0; i < sizeof(mimeMaps) / sizeof(MimeMap); i++) {
        if (!strcasecmp(mime, mimeMaps[i].mime)) {
            cmMime = mimeMaps[i].cmMime;
            break;
        }
    }

    if (cmMime == NULL) {
        return String8(mime); // no match, not converted
    } else {
        if (DrmUtil::sDDebug) ALOGD("toCommonMime() : convert mime: [%s]->[%s]", mime, cmMime);
        return String8(cmMime);
    }
}

String8 DrmMtkUtil::getProcessName(pid_t pid) {
    // get the proc name by pid, via reading /proc/<pid>/cmdline file
    char path[30];
    #if 0
    bzero(path, sizeof(path));
    strcat(path, "/proc/");

    char proc_id[10];
    bzero(proc_id, sizeof(proc_id));
    snprintf(proc_id, sizeof(proc_id), "%d", pid);
    strcat(path, proc_id);

    strcat(path, "/cmdline");
    #endif
    snprintf(path, sizeof(path), "/proc/%d/cmdline", pid);
    ALOGD("getProcessName() : path : [%s]", path);
    String8 result("unknown process name");
    FILE* fp = fopen(path, "r");
    if (NULL != fp) {
        char proc_name[100];
        bzero(proc_name, sizeof(proc_name));
        if (NULL != fgets(proc_name, sizeof(proc_name), fp)) {
            result.setTo(proc_name);
        }
        fclose(fp);
    } /* else {
        ALOGW("getProcessName() : failed to open [%s] to read, reason [%s]",
                path, strerror(errno));
    } */

    if (DrmUtil::sDDebug) ALOGD("getProcessName() : pid to process: [%s]->[%s]", path, result.string());
    return result;
}

bool DrmMtkUtil::isTrustedClient(const String8& procName) {
    return DrmMtkDefender::isDrmTrustedClient(procName);
}
//this function should be remove
bool DrmMtkUtil::isTrustedVideoClient(const String8& procName) {
    return DrmTrustedVideoApp::IsDrmTrustedVideoApp(procName);
}

bool DrmMtkUtil::isDrmConsumeInAppClient(const String8& procName) {
    return DrmMtkDefender::isDrmConsumeInAppClient(procName);
}

bool DrmMtkUtil::markAsConsumeInAppClient(const String8& procName, const String8& cid) {
    return DrmMtkDefender::markAsConsumeInAppClient(procName, cid);
}

bool DrmMtkUtil::isNeedConsume(const String8& cid) {
    return DrmMtkDefender::isNeedConsume(cid);
}

long DrmMtkUtil::getContentSize(DecryptHandle* handle) {
    return getContentLength(handle, NULL);
}

long DrmMtkUtil::getContentLength(DecryptHandle* handle,
                                  DrmManagerClient* client __attribute__((unused))) {
    if (NULL != handle && NULL != handle->decryptInfo) {
        if (DrmUtil::sDDebug) ALOGD("getContentLength: return encrypted length: %d",
                handle->decryptInfo->decryptBufferLength);
        return handle->decryptInfo->decryptBufferLength;
    } else {
        if (DrmUtil::sDDebug) ALOGD("getContentLength: return 0 instead.");
        return 0;
    }
}

#if 0
bool DrmMtkUtil::getLines(FILE* fp, int lineNo, bool trim, Vector<String8>& headers) {
    String8 line;
    for (int i = 0; i < lineNo; i++) {
        if (!getLineX(fp, line)) {
            return false;
        }
        if (trim) {
            line = StrUtil::trimRCRLF(line);
        }
        // when find the blank line, the header area finishes.
        if (line.isEmpty()) {
            if (DrmUtil::sDDebug) ALOGD("installDrmMsg:[getLines] found a blank line.");
            break;
        }
        if (DrmUtil::sDDebug) ALOGD("installDrmMsg:[getLines] lines[%d]=[%s]", i, line.string());

        // add each non-empty line to vector
        headers.push(line);
    }

    return true;
}

bool DrmMtkUtil::installRo(FILE * fp, long offset, long length, char* cid) {
    if (DrmUtil::sDDebug) ALOGD("installDrmMsg:[installRo] start:[%ld], length:[%ld]", offset, length);

    // read RO data to buffer -> CD's RO must be xml, not wbxml
    char* buf = new char[length];
    bzero(buf, length);

    if (!FileUtil::fseekx(fp, offset, SEEK_SET)) {
        ALOGE("installDrmMsg:[installRo] failed to seek to the start[%ld] of RO!", offset);
        delete[] buf;
        return false;
    }

    if (!FileUtil::freadx(buf, length, fp)) {
        ALOGE("installDrmMsg:[installRo] failed to read [%ld] bytes!", length);
        delete[] buf;
        return false;
    }

    Mutex::Autolock lock(DrmUtil::mROLock);

    // create DrmBuffer for saveRights
    DrmBuffer drmBuffer(buf, length);
    if (DrmUtil::sVDebug) ALOGV("installDrmMsg:[installRo] CD's RO buf: [%s]", buf);

    // Irdeto's implementation don't have "Content-ID" in the 2nd part of dm
    // so we always parse the RO and get the cid
    // some may define "Content-ID" in header area, but we ignore
    RO ro;
    if (!DrmUtil::parse_dr(drmBuffer, &ro)) {
        ALOGE("installDrmMsg:[installRo] failed to parse RO!");
        delete[] buf;
        return false;
    }
    delete[] buf;

    // for Combined delievery we need to add unique device Id after original cid
    // so that the same CD file was moved to and shown "invalid rights" when
    // we downloaded the same CD file again on another phone.
    strcat(ro.rights.cid, DrmUtil::readId().string());
    strcpy(cid, ro.rights.cid);

    // RO file's path name
    String8 roPath = DrmUtil::getROFileName(String8(cid));
    // check old RO existence: merge, or save new one
    if (DrmUtil::checkExistence(roPath)) {
        // exist, restore old ro & merge
        if (DrmUtil::sDDebug) ALOGD("installDrmMsg:[installRo] merge RO to [%s]", roPath.string());
        if (!DrmUtil::merge(&ro)) {
            ALOGE("installDrmMsg: failed to merge the new RO!");
            return false;
        }
    } else {
        if (DrmUtil::sDDebug) ALOGD("installDrmMsg:[installRo] save RO to [%s]", roPath.string());
        if (!DrmUtil::save(&ro)) {
            ALOGE("installDrmMsg:[installRo] failed to save the new RO!");
            return false;
        }
    }
    return true;
}

// get RO start and end index
bool DrmMtkUtil::getRoIndex(FILE * fp, const String8 boundary, long& startIndex, long& endIndex) {
    // the current position is at the start of rights object
    char line[LINE_MAX_SIZE];
    bzero(line, sizeof(line));

    // get something from RO part, but not all of the RO
    int lineLength = getNextNELine(fp, line);
    if (lineLength < 0) {
        ALOGE("installDrmMsg:[getRoIndex] failed to get something from RO part.");
        return false;
    }

    // get RO start index
    long pos = 0;
    if (!FileUtil::ftellx(fp, pos)) {
        ALOGE("installDrmMsg:[getRoIndex] failed to get the start of RO!");
        return false;
    }
    startIndex = pos - lineLength;
    if (DrmUtil::sDDebug) ALOGD("installDrmMsg:[getRoIndex] RO start offset: [%ld]", startIndex);

    // get RO end index
    String8 nextBoundary;
    if (!getNextNELineContain(fp, nextBoundary, boundary)) {
        ALOGE("installDrmMsg:[getRoIndex] failed to find the boundary line after RO!");
        return false;
    }
    if (!FileUtil::ftellx(fp, pos)) {
        ALOGE("installDrmMsg:[getRoIndex] failed to get the end offset of boundary after RO!");
        return false;
    }
    endIndex = pos - nextBoundary.length();
    if (DrmUtil::sDDebug) ALOGD("installDrmMsg:[getRoIndex] RO end offset: [%ld]", endIndex);

    return true;
}

// get content start and end index(without CR/LF)
bool DrmMtkUtil::getContentIndex(FILE* fp, const String8 boundary, long& startIndex, long& endIndex) {
    char line[LINE_MAX_SIZE];
    bzero(line, sizeof(line));

    // get something from content part, but not all of content
    int lineLength = getNextNELine(fp, line);
    if (lineLength < 0) {
        ALOGE("installDrmMsg:[getContentIndex] failed to get something from content!");
        return false;
    }

    long pos = 0;
    if (!FileUtil::ftellx(fp, pos)) {
        ALOGE("installDrmMsg:[getContentIndex] failed to get the start of content!");
        return false;
    }
    startIndex = pos - lineLength;
    if (DrmUtil::sDDebug) ALOGD("installDrmMsg:[getContentIndex] content start offset: [%ld]", startIndex);

    // find the final boundary line (closing delimiter line)
    // the line should be {bounary} + "--"
    String8 lastBoundary;
    if (!getNextNELineContain(fp, lastBoundary, boundary + String8("--"))) {
        ALOGE("installDrmMsg:[getContentIndex] failed to find the last boundary!");
        return false;
    }
    if (DrmUtil::sDDebug) ALOGD("installDrmMsg:[getContentIndex] last boundary line: [%s]", lastBoundary.string());

    if (!FileUtil::ftellx(fp, pos)) {
        ALOGE("installDrmMsg:[getContentIndex] failed to get the end offset of last boundary!");
        return false;
    }

    // at the end of the last boundary
    long lastBoundaryIndex = pos - lastBoundary.length();
    if (DrmUtil::sDDebug) ALOGD("installDrmMsg:[getContentIndex] content end offset: [%ld]", lastBoundaryIndex);
    if (!FileUtil::fseekx(fp, lastBoundaryIndex, SEEK_SET)) {
        ALOGE("installDrmMsg:[getContentIndex] failed to seek to the end of content!");
        return false;
    }

    // in some cases, there's CR LF at the end of content part
    // we need to adjust content length for CR LF
    // then we get the end position of data (without CR LF)
    while (true) {
        if (!FileUtil::fseekx(fp, -2, SEEK_CUR)) {
            ALOGE("installDrmMsg:[getContentIndex] failed to seek back 2 bytes!");
            return false;
        }

        char tmp[2];
        bzero(tmp, sizeof(tmp));
        if (!FileUtil::freadx(tmp, 2, fp)) {
            ALOGE("installDrmMsg:[getContentIndex] failed to read 2 bytes!");
            return false;
        }

        // it's not ends with 0D 0A
        if (!((tmp[0] == 0x0D) && (tmp[1] == 0x0A))) {
            if (DrmUtil::sDDebug) ALOGD("installDrmMsg:[getContentIndex] CR LF not founded.");
            break;
        }

        if (!FileUtil::fseekx(fp, -2, SEEK_CUR)) {
            ALOGE("installDrmMsg:[getContentIndex] failed to seek back 2 more bytes!");
            return false;
        }
    }

    if (!FileUtil::ftellx(fp, endIndex)) {
        if (DrmUtil::sDDebug) ALOGD("installDrmMsg:[getContentIndex] failed to get end offset of content after removing CR LF!");
        return false;
    }
    if (DrmUtil::sDDebug) ALOGD("installDrmMsg:[getContentIndex] content end offset(remove CR LF): [%ld]", endIndex);

    return true;
}

bool DrmMtkUtil::decryptBase64(FILE * fp, long offset, long& length) {
    long readIndex = offset;
    long writeIndex = offset;

    String8 line;
    // firstly try to get one line to check the style of the base64 encoded
    // data: end-of-line at each 76 characters, or it is continuous?
    if (!getLineXTrimR(fp, line)) {
        ALOGE("installDrmMsg:[decryptBase64] failed to read one line(without CR LF) from dm file!");
        return false;
    }
    int base64Style = BASE64_STYLE_DEFAULT;
    if (line.length() == BASE64_LINE_SIZE
        || (line.length() == (BASE64_LINE_SIZE - 4) && offset > (BASE64_LINE_SIZE - 4))) {
        // the data part is 76, that the content part is separated each 76 characters
        // special case for alps00353872: the first line of content part,
        // however, is 72 characters.
        if (DrmUtil::sDDebug) ALOGD("installDrmMsg:[decryptBase64] BASE64_STYLE_LINE_SEPARATE.");
        base64Style = BASE64_STYLE_LINE_SEPARATE;
    }

    unsigned char buffer[BASE64_DECRYPT_SIZE];
    long readNum = 0;
    while (readNum < length) {
        if (!FileUtil::fseekx(fp, readIndex, SEEK_SET)) {
            ALOGE("installDrmMsg:[decryptBase64] failed to seek to read position [%ld]!", readIndex);
            return false;
        }
        bzero(buffer, sizeof(buffer));
        size_t size = 0;

        if (BASE64_STYLE_LINE_SEPARATE == base64Style) {
            if (!getLineX(fp, line)) {
                ALOGE("installDrmMsg:[decryptBase64] failed to read one content line(with CR LF)! \
                    haveRead:[%ld]", readNum);
                return false;
            }
            readNum += line.length();

            // remove the CR LF for each 76 characters
            line = StrUtil::trimRCRLF(line);
            size = line.length();
            memcpy(buffer, line.string(), size);
        } else {
            size = (size_t)(length - readNum) > sizeof(buffer) ?
                sizeof(buffer) : (size_t)(length - readNum);
            if (!FileUtil::freadx(buffer, size, fp)) {
                ALOGE("installDrmMsg:[decryptBase64] failed to read data from content! \
                    haveRead:[%ld]", readNum);
                return false;
            }
            readNum += size;
        }

        // pass the buffer to base64 decoding
        int outputCnt = 0;
        DrmUtil::base64_decrypt_buffer(buffer, buffer, size, outputCnt);
        if (DrmUtil::sDDebug) ALOGD("installDrmMsg:[decryptBase64] input size [%d] output size [%d]",
                size, outputCnt);

        // save the read position we shall start for next round
        if (!FileUtil::ftellx(fp, readIndex)){
            ALOGE("installDrmMsg:[decryptBase64] failed to get the next read position!");
            return false;
        }

        // seek to the write position
        if (!FileUtil::fseekx(fp, writeIndex, SEEK_SET)) {
            ALOGE("installDrmMsg:[decryptBase64] failed to seek to the write position [%ld]!", writeIndex);
            return false;
        }
        // write the decoded data back to the dm file
        if (!FileUtil::fwritex(buffer, (size_t)outputCnt, fp)) {
            ALOGE("installDrmMsg:[decryptBase64] failed to write decoded data back to dm file!");
            return false;
        }
        // step forward write position
        writeIndex += outputCnt;
        if (DrmUtil::sDDebug) ALOGD("installDrmMsg:[decryptBase64] write back [%d] bytes. endPosition [%ld].", outputCnt, writeIndex);
    }

    // update content length to binary length
    length = writeIndex - offset;
    return true;
}


/**
 * Generate a signature with current time and a random
 */
static bool getFlCidSuffix(char* sig, int len) {
    unsigned int seed = (unsigned int) time(NULL);
    srand(seed);
    unsigned int number = (unsigned int) rand();
    snprintf(sig, len, "%u", seed);
    snprintf(sig + strlen(sig), len, "%u", number);
    return true;
}

bool DrmMtkUtil::installFl(FILE * fp_dm, const String8 boundary, const String8 encoding,
        const String8 mime, FILE * fp_dcf) {
    // preInstall: get content startIndex and length
    long contentOffset = 0;
    long contentLength = 0;
    bool ret = preInstall(fp_dm, boundary, encoding, contentOffset, contentLength);
    if (!ret) {
        if (DrmUtil::sDDebug) ALOGD("installDrmMsg:[FL] preInstall failed!");
        return false;
    }

    // get FL signature
#define MAX_SIG_LEN 50
    char sig[MAX_SIG_LEN];
    bzero(sig, sizeof(sig));
    getFlCidSuffix(sig, sizeof(sig));

    // get cid
    char dcfCid[DrmDef::DRM_MAX_META_LENGTH];
    bzero(dcfCid, sizeof(dcfCid));
    // now we add the device id to FL's content id meta data
    // empty -> MTK_FL_cid:deviceId + signature
    String8 deviceId = DrmUtil::readId();
    strcpy(dcfCid, DrmDef::MTK_FL_CID.string());
    strcpy(dcfCid + DrmDef::MTK_FL_CID.length(), deviceId.string());
    strncpy(dcfCid + DrmDef::MTK_FL_CID.length() + deviceId.length(), sig, strlen(sig));
    if (DrmUtil::sDDebug) ALOGD("installDrmMsg:[FL] cid=[%s], deviceId=[%s], sig=[%s]", dcfCid, deviceId.string(), sig);

    // install
    ret = installContent(fp_dm, contentOffset, contentLength, mime, dcfCid, DrmDef::MTK_FL_HEADER, fp_dcf);
    if (!ret) {
        ALOGE("installDrmMsg:[FL] install failed!");
        return false;
    }

    return true;
}

bool DrmMtkUtil::installFlDcf(FILE* fp_dm, const String8 boundary, const String8 encoding,  FILE* fp_dcf) {
    // preInstall: get content startIndex and length
    long contentOffset = 0;
    long contentLength = 0;
    bool ret = preInstall(fp_dm, boundary, encoding, contentOffset, contentLength);
    if (!ret) {
        if (DrmUtil::sDDebug) ALOGD("installDrmMsg:[FLDCF] preInstall failed!");
        return false;
    }

    // seek back to the start of content
    if (!FileUtil::fseekx(fp_dm, contentOffset, SEEK_SET)) {
        ALOGE("installDrmMsg:[FLDCF] failed to seek to the start of content [%ld]!", contentOffset);
        return false;
    }

    char in[BLOCK_SIZE];
    long readNum = 0;
    while (readNum < contentLength) {
        bzero(in, sizeof(in));
        long in_size = (long)sizeof(in);
        long remainSize = contentLength - readNum;
        long readSize = remainSize > in_size ? in_size : remainSize;
        if (DrmUtil::sVDebug) ALOGV("installDrmMsg:[FLDCF] readSize [%ld], remainSize [%ld]", readSize, remainSize);

        if (!FileUtil::freadx(in, readSize, fp_dm)) {
            ALOGE("installDrmMsg:[FLDCF] failed to read [%ld] bytes! remainSize [%ld]", readSize, remainSize);
            return false;
        }

        bool writeResult = false;
        // the first line of dcf contains "cid:...",
        // we need to change it to "MTK_FLDCF_cid" so that we can identify FLSD files.
        if (readNum == 0) {
            char* ptr = strstr(in, "cid");
            if (ptr == NULL) {
                ALOGE("installDrmMsg:[FLDCF] failed to find [cid]!");
                return false;
            }

            in[2] = in[2] + DrmDef::MTK_FLDCF_.length();
            writeResult = FileUtil::fwritex(in, ptr - in, fp_dcf);
            if (!writeResult) {
                ALOGE("installDrmMsg:[FLDCF] failed to write data before cid[in=%s]!", in);
                return false;
            }
            writeResult = FileUtil::fwritex((void*)DrmDef::MTK_FLDCF_.string(),
                                            DrmDef::MTK_FLDCF_.length(),
                                            fp_dcf);
            if (!writeResult) {
                ALOGE("installDrmMsg:[FLDCF] failed to write MTK_FLDCF_!");
                return false;
            }
            writeResult = FileUtil::fwritex(ptr, readSize - (ptr - in), fp_dcf);
            if (!writeResult) {
                ALOGE("installDrmMsg:[FLDCF] failed to write data after cid[in=%s]!", in);
                return false;
            }
        } else {
            writeResult = FileUtil::fwritex(in, readSize, fp_dcf);
            if (!writeResult) {
                ALOGE("installDrmMsg:[FLDCF] failed to write data! written [%ld]", readNum);
                return false;
            }
        }

        readNum += readSize;
        if (DrmUtil::sVDebug) ALOGV("installDrmMsg:[FLDCF] readNum [%ld], contentLen [%ld]", readNum, contentLength);
    }

    return true;
}

bool DrmMtkUtil::installContent(FILE* fp_dm, long contentOffset, long contentLength,
        const String8 mime, const char* cid, const String8 headers, FILE* fp_dcf) {
    char info[3];
    info[0] = 1;
    info[1] = mime.length();
    info[2] = strlen(cid);

    if (!FileUtil::fwritex(info, sizeof(info), fp_dcf)) {
        ALOGE("installDrmMsg:[FL/CD] failed to write tag to file!");
        return false;
    }

    // write mime
    if (!FileUtil::fwritex(const_cast<char*>(mime.string()), info[1], fp_dcf)) {
        ALOGE("installDrmMsg:[FL/CD] failed to write mime!");
        return false;
    }

    // write conent-id
    if (!FileUtil::fwritex(const_cast<char*>(cid), info[2], fp_dcf)) {
        ALOGE("installDrmMsg:[FL/CD] failed to write content id!");
        return false;
    }

    // write headersLen
    char headersLen[1];
    headersLen[0] = headers.length();
    if (!FileUtil::fwritex(headersLen, sizeof(headersLen), fp_dcf)) {
        ALOGE("installDrmMsg:[FL/CD] failed to write header length!");
        return false;
    }

    // skip dataLen for 1 byte
    if (!FileUtil::fseekx(fp_dcf, 1, SEEK_CUR)){
        ALOGE("installDrmMsg:[FL/CD] failed to skip 1 byte for data length!");
        return false;
    }

    // write headers
    if (!FileUtil::fwritex(const_cast<char*>(headers.string()), headersLen[0], fp_dcf)) {
        ALOGE("installDrmMsg:[FL/CD] failed to write headers!");
        return false;
    }

    int dcfDataLen = ((contentLength % DrmDef::DRM_DCF_BLK_LENGTH) == 0) ?
             DrmDef::DRM_DCF_IV_LENGTH + contentLength + DrmDef::DRM_DCF_BLK_LENGTH :
             DrmDef::DRM_DCF_IV_LENGTH + (contentLength / DrmDef::DRM_DCF_BLK_LENGTH + 1) * DrmDef::DRM_DCF_BLK_LENGTH;
    if (DrmUtil::sDDebug) ALOGD("installDrmMsg:[FL/CD] dcf data length: [%d]", dcfDataLen);

    // init IV
    String8 iv("4074BCC61602B5BE");
    ByteBuffer drmKey(DrmUtil::getDrmKey());
    if (DrmUtil::sDDebug) ALOGD("installDrmMsg:[FL/CD] drmkey=[%s]", drmKey.buffer());

    clearerr(fp_dm);
    // seek back the start of content
    if (!FileUtil::fseekx(fp_dm, contentOffset, SEEK_SET)) {
        ALOGE("installDrmMsg:[FL/CD] failed to seek to the start of content!");
        return false;
    }

    // encrypt the content with AES128CBC method
    CryptoHelper helper(CryptoHelper::CIPHER_AES128CBC, (BYTE*)drmKey.buffer(), 1);
    int result = CipherFileUtil::CipherFileInstall(fp_dm, contentOffset, contentLength,
        fp_dcf, helper, (BYTE*)iv.string());
    if (DrmUtil::sDDebug) ALOGD("installDrmMsg:[FL/CD] CipherFileInstall result: [%d]", result);
    // if cipher installation fails
    if (-1 == result) {
        ALOGE("installDrmMsg:[FL/CD] CipherFileInstall failed!");
        return false;
    }

    return true;
}

bool DrmMtkUtil::installCd(FILE* fp_dm, const String8 boundary, FILE* fp_dcf) {
    if (DrmUtil::sDDebug) ALOGD("installDrmMsg:[CD] Rights Object>>>");

    // get RO startIndex and endIndex
    long roStartIndex = 0;
    long roEndIndex = 0;
    bool ret = getRoIndex(fp_dm, boundary, roStartIndex, roEndIndex);
    if (!ret) {
        ALOGE("installDrmMsg:[CD] failed to get the start and end of RO!");
        return false;
    }
    long roLength = roEndIndex - roStartIndex;
    if (DrmUtil::sDDebug) ALOGD("installDrmMsg:[CD] RO start:[%ld], end:[%ld], length:[%ld]", roStartIndex, roEndIndex, roLength);

    // install RO, cid holds the value parsed out from RO
    char cid[DrmDef::DRM_MAX_META_LENGTH];
    bzero(cid, sizeof(cid));
    ret = installRo(fp_dm, roStartIndex, roLength, cid);
    if (DrmUtil::sDDebug) ALOGD("installDrmMsg:[CD] parsed cid=[%s]", cid);
    if (!ret) {
        ALOGE("installDrmMsg:[CD] failed to install RO!");
        return false;
    }

    // step forward to the boundary before content
    String8 nextBoundary;
    if (!getNextNELineContain(fp_dm, nextBoundary, boundary)) {
        ALOGE("installDrmMsg:[CD] failed to find the boundary between RO and content!");
        return false;
    }

    // then it's the header area of content part
    if (DrmUtil::sDDebug) ALOGD("installDrmMsg:[CD] content part>>>");
    Vector<String8> headerLines;
    ret = getLines(fp_dm, MAX_HEADER_LINES, true/*trim*/, headerLines);
    if (!ret) {
        ALOGE("installDrmMsg:[CD] failed to get content headers lines!" );
        return false;
    }

    // parse header to get content encoding, mime and drmMethod
    String8 encoding;// the encoding of the content
    String8 mime;    // the mime type of the content
    int dmMethod = DrmDef::METHOD_NONE;
    ret = parseHeaders(headerLines, encoding, mime, dmMethod);
    if (!ret) {
        ALOGE("installDrmMsg:[CD] No valid Content-Type or Content-Transfer-Encoding header!");
        return false;
    }

    // preInstall: get content startIndex and length
    long contentOffset = 0;
    long contentLength = 0;
    ret = preInstall(fp_dm, boundary, encoding, contentOffset, contentLength);
    if (!ret) {
        if (DrmUtil::sDDebug) ALOGD("installDrmMsg:[CD] preInstall failed!");
        return false;
    }

    // append  MTK_CD_CID to cid
    char dcfCid[DrmDef::DRM_MAX_META_LENGTH];
    bzero(dcfCid, sizeof(dcfCid));
    strcpy(dcfCid, DrmDef::MTK_CD_CID.string());
    // we've get "Content-ID" already in {cd_cid}
    strncpy(dcfCid + DrmDef::MTK_CD_CID.length(), cid, strlen(cid));
    if (DrmUtil::sDDebug) ALOGD("installDrmMsg:[CD] cid=[%s]", dcfCid);

    // install
    ret = installContent(fp_dm, contentOffset, contentLength, mime, dcfCid, DrmDef::MTK_CD_HEADER, fp_dcf);
    if (!ret) {
        ALOGE("installDrmMsg:[CD] install failed!");
        return false;
    }

    return true;
}

bool DrmMtkUtil::isForwardlockSet() {
    char value[PROPERTY_VALUE_MAX];
    bzero(value, sizeof(value));
    if (property_get("vendor.drm.forwardlock.only", value, NULL)
            && (0 == strcmp(value, "1") || 0 == strcmp(value, "yes") || 0 == strcmp(value, "true"))) {
        return true;
    }
    return false;
}

bool DrmMtkUtil::parseHeaders(const Vector<String8> headers, String8& encoding, String8& mime, int& drmMethod) {
    bool isTypeValid = false;
    bool isEncodingValid = false;
    int size = headers.size();
    if (DrmUtil::sDDebug) ALOGD("installDrmMsg:[parseHeaders] size=[%d]", size);
    for (int i = 0; i < size; i++) {
        String8 line = headers.itemAt(i);
        if (DrmUtil::sDDebug) ALOGD("installDrmMsg:[parseHeaders] headers[%d]=[%s]", i, line.string());
        // parse content type header
        if (!isTypeValid) {
            ContentType content_type_value(line.string());
            isTypeValid = content_type_value.IsValid();
            if (isTypeValid) {
                mime = content_type_value.mediaType;
                if (mime.compare(DrmDef::MIME_RIGHTS_XML) == 0) {
                    drmMethod = DrmDef::METHOD_CD;
                } else if (mime.compare(DrmDef::MIME_DRM_CONTENT) == 0) {
                    drmMethod = DrmDef::METHOD_FLDCF;
                } else { // otherwise it should be FL case. (e.g. image/jpeg)
                    drmMethod = DrmDef::METHOD_FL;
                }
                if (DrmUtil::sDDebug) ALOGD("installDrmMsg:[parseHeaders] mime=[%s], method=%d", mime.string(), drmMethod);

                if (isTypeValid && isEncodingValid) {
                    break;
                } else {
                    continue;
                }
            }
        }

        // parse content encoding header
        if (!isEncodingValid) {
            ContentTransferEncoding encoding_value(line.string());
            isEncodingValid = encoding_value.IsValid();
            if (isEncodingValid) {
                encoding = encoding_value.mechanism;
                if (DrmUtil::sDDebug) ALOGD("installDrmMsg:[parseHeaders] encoding=[%s]", encoding.string());

                if (isTypeValid && isEncodingValid) {
                    break;
                } else {
                    continue;
                }
            }
        }
    }

    return (isTypeValid && isEncodingValid);
}

// get the startIndex and length of binary content
// (converted base64 to binary if the content is base64 encoded)
// fp points to the end of conent after it return
bool DrmMtkUtil::preInstall(FILE* fp, const String8 boundary, const String8 encoding,
        long& contentOffset, long& contentLength) {
    long contentEnd = 0;
    bool ret = getContentIndex(fp, boundary, contentOffset, contentEnd);
    if (!ret) {
        if (DrmUtil::sDDebug) ALOGD("installDrmMsg:[preInstall] failed to get the start and end of content(without CR LF)!");
        return false;
    }
    contentLength = contentEnd - contentOffset;
    if (DrmUtil::sDDebug) ALOGD("installDrmMsg:[preInstall] content length: [%ld]", contentLength);

    // convert base64 -> binary
    if (encoding.compare(DrmDef::ENCODING_BASE64) == 0) {
        // clear any error tag of this FILE stream
        clearerr(fp);
        if (!FileUtil::fseekx(fp, contentOffset, SEEK_SET)) {
            ALOGE("installDrmMsg:[preInstall] failed to seek to the start of content!");
            return false;
        }

        // contentLength will be updated to binary length
        ret = decryptBase64(fp, contentOffset, contentLength);
        if (!ret) {
            ALOGE("installDrmMsg:[preInstall] failed to decrypt base64 content!");
            return false;
        }
        if (DrmUtil::sDDebug) ALOGD("installDrmMsg:[preInstall] content length(after decrypt base64): [%ld]", contentLength);
    }

    return true;
}

//M: Add to avoid writing sdcard in native layer
/*
 * copyFile
 *
 * Description:
 *   Copy the content from fd_in to fd_out
 *
 * Parameters:
 *   fd_dm (in) - The src file descriptor
 *   fd_dcf (out) - The dst file descriptor
 *   length (in) - The length to be copyed
 *
 * Returns:
 *   true indicates copy successfully
 *   false indicates copy fail
 */
bool DrmMtkUtil::copyFile(int fd_in, int fd_out, int length) {
    if (DrmUtil::sDDebug) ALOGD("copyFile: need copy length[%d]", length);
    if (-1 == lseek(fd_in, 0, SEEK_SET) || -1 == lseek(fd_out, 0, SEEK_SET)) {
        ALOGE("copyFile: failed to seek fd_in/fd_out to positin 0");
        return false;
    }

    ssize_t readSize = 0;
    ssize_t copiedLen = 0;
#define BUF_SIZE 1024
    char buf[BUF_SIZE];
    while (copiedLen <= length) {
        if (BUF_SIZE != (readSize = read(fd_in, buf, BUF_SIZE))) {
            ALOGE("copyFile: failed read data from fd_in, reason [%s]", strerror(errno));
            return false;
        }
        if (BUF_SIZE != write(fd_out, buf, readSize)) {
            ALOGE("copyFile: failed write data to fd_out, reason [%s]", strerror(errno));
            return false;
        }
        copiedLen += readSize;
    }
    // read remind data small than BUFSIZE
    readSize = length - copiedLen;
    if (readSize > 0) {
        if (readSize != read(fd_in, buf, readSize)) {
            ALOGE("copyFile: failed read remind data from fd_in, reason [%s]", strerror(errno));
            return false;
        }
        if (-1 == write(fd_out, buf, readSize)) {
            ALOGE("copyFile: failed write remind data to fd_out, reason [%s]", strerror(errno));
            return false;
        }
    }
    fsync(fd_out);
    if (DrmUtil::sDDebug) ALOGD("copyFile: first copy len[%d], remind copy len[%d]", copiedLen, readSize);
    return true;
}


/*
 * correctDcf
 *
 * Description:
 *   Modify the bad dcf file to a correct dcf file
 *
 * Parameters:
 *   fd_dm (in) - The dm file descriptor
 *   fd_dcf (out) - The dcf file descriptor
 *
 * Returns:
 *   true indicates converting successfully
 *   false indicates converting fail
 */
bool DrmMtkUtil::correctDcf(int fd_dm, int fd_dcf) {
    if (DrmUtil::sDDebug) ALOGD("correctDcf: to be corrected: fd_dm [%d], fd_dcf [%d]", fd_dm, fd_dcf);

    if (fd_dm < 0) {
        ALOGE("correctDcf: file does not exist.");
        return false;
    }

    DrmMetadata* pMetaData = new DrmMetadata();
    int parseResult = DrmUtil::parseDcf(fd_dm, pMetaData);
    if (DrmUtil::sDDebug) ALOGD("correctDcf: result [%d]", parseResult);

    if (0 == parseResult) {
        if (DrmUtil::sDDebug) ALOGD("correctDcf: correct DCF format.");
        delete pMetaData;
        return true;
    }

    if (-2 == parseResult) {
        if (DrmUtil::sDDebug) ALOGD("correctDcf: incorrect DCF file length, try to truncate it.");

        String8 data_offset = pMetaData->get(String8(DrmMetaKey::META_KEY_OFFSET));
        const char* pDataOffset = data_offset.string();
        int dataOffset = atoi(pDataOffset);

        String8 data_length = pMetaData->get(String8(DrmMetaKey::META_KEY_DATALEN));
        const char* pDataLength = data_length.string();
        int dataLength = atoi(pDataLength);

        int desiredFileLength = dataOffset + dataLength;
        delete pMetaData;
        bool result = copyFile(fd_dm, fd_dcf,desiredFileLength);
        if (false == result) {
            ALOGE("correctDcf : failed to truncate file to desired length, reason [%s]",
                    strerror(errno));
            return false;
        }
        return true;
    }
    delete pMetaData;

    return false;
}

/*
 * installDrmMsg
 *
 * Description:
 *   Convert a dm file to a dcf format file
 *
 * Parameters:
 *   fd_dm (in) - The dm file descriptor
 *   fd_dcf (out) - The dcf file descriptor
 *
 * Returns:
 *   true indicates converting successfully
 *   false indicates converting failly
 */
bool DrmMtkUtil::installDrmMsg(int fd_dm, int fd_dcf) {
    if (DrmUtil::sDDebug) ALOGD("installDrmMsg: DRM message file: fd_dm[%d], fd_dcf[%d]", fd_dm,fd_dcf);
    if(fd_dm < 0 || fd_dcf < 0) {
        ALOGE("installDrmMsg() bad file descriptor");
        return false;
    }

    if (DrmUtil::checkDcf(fd_dm, NULL) == 0) {
        // the file's already a DCF format in the beginning.
        // however the DCF format may not be correct
        if (DrmUtil::sDDebug) ALOGD("installDrmMsg: already a DCF format, correct it");
        bool correctResult = correctDcf(fd_dm, fd_dcf);
        if (true != correctResult) {
            ALOGE("installDrmMsg: invalid dm file!");
            ftruncate(fd_dcf, (off_t)0);
        }
        return correctResult;
    }

    FILE* fp_dm = NULL;
    FILE* fp_dcf = NULL;
    int fd_dm_dup = dup(fd_dm);
    int fd_dcf_dup = dup(fd_dcf);
    // After each boundary line, each body part then consist of a header area,
    // a blank line, and a body area.
    String8 boundary;             // boundary
    Vector<String8> headerLines;  // header lines
    String8 encoding;             // the encoding of the content
    String8 mime;                 // the mime type of the content
    int dmMethod = DrmDef::METHOD_NONE;
    bool ret = false;
    fp_dm = fdopen(fd_dm_dup, "r+");
    if (fp_dm == NULL) {
        ALOGE("installDrmMsg: failed to open the dm file [%d],reason [%s]",
                fd_dm_dup,strerror(errno));
        goto Exit;
    }

    // the first line should be non-empty, and is the boundary (delimiter string)
    if (!getNextNELineTrimR(fp_dm, boundary)) {
        ALOGE("installDrmMsg: failed to get the first boundary line!");
        goto Exit;
    }
    if (DrmUtil::sDDebug) ALOGD("installDrmMsg: first boundary line: [%s]", boundary.string());

    // header area
    ret = getLines(fp_dm, MAX_HEADER_LINES, true /*trim*/, headerLines);
    if (!ret) {
        ALOGE("installDrmMsg: failed to get headers lines!" );
        goto Exit;
    }

    // parse header to get content encoding, mime and drmMethod
    ret = parseHeaders(headerLines, encoding, mime, dmMethod);
    if (!ret) {
        ALOGE("installDrmMsg: No valid Content-Type or Content-Transfer-Encoding header!");
        goto Exit;
    }

    // Forward Lock Only
    // we check according to the property "vendor.drm.forwardlock.only"
    if (isForwardlockSet()) {
        if (DrmUtil::sDDebug) ALOGD("installDrmMsg: Forward-lock-only has been set!");
        if (dmMethod != DrmDef::METHOD_FL) {
            ALOGE("installDrmMsg: Objects with method=[%d] will be abandoned!", dmMethod);
            ret = false;
            goto Exit;
        }
    }

    //get FILE pointer from fd
    fp_dcf = fdopen(fd_dcf_dup,"w");
    if (fp_dcf == NULL) {
        ALOGE("installDrmMsg: failed to open dcf file [%d],reason [%s]",
                fd_dcf_dup,strerror(errno));
        goto Exit;
    }

    if (dmMethod == DrmDef::METHOD_FL) {
        ret = installFl(fp_dm, boundary, encoding, mime, fp_dcf);
    } else if (dmMethod == DrmDef::METHOD_CD) {
        ret = installCd(fp_dm, boundary, fp_dcf);
    } else if (dmMethod == DrmDef::METHOD_FLDCF) {
        ret = installFlDcf(fp_dm, boundary, encoding, fp_dcf);
    } else {
        ALOGE("installDrmMsg: invalid method [%d]!", dmMethod);
        ret = false;
    }
Exit:
    // close open files fp_dm and fp_dcf
    FileUtil::fclosex(fp_dm);
    FileUtil::fclosex(fp_dcf);
    if (DrmUtil::sDDebug) ALOGD("installDrmMsg: done [%d]", ret);
    return ret;
}
#endif