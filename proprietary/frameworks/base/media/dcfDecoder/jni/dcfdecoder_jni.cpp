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

//#define LOG_NDEBUG 0
#define LOG_TAG "DRM_DCF_DECODER_JNI"
#include <utils/Log.h>

#include <jni.h>
#include <nativehelper/JNIHelp.h>
#include <fcntl.h>
#include <sys/stat.h>


#include <drm_framework_common.h>
#include <DrmManagerClient.h>
#include <DrmInfoRequest.h>
#include <DrmMtkUtil.h>

#include <binder/IPCThreadState.h>

#define DECRYPT_BUF_LEN 4096

// openDecryptSession decode image actions
#define ACTION_DECODE_FULL_IMAGE     0x0
#define ACTION_JUST_DECODE_BOUND     0x1
#define ACTION_JUST_DECODE_THUMBNAIL 0x2

using namespace android;

static unsigned char* decryptDcfSource(JNIEnv* env, DrmManagerClient *drmManagerClient,
        sp<DecryptHandle> decryptHandle, int* length, int fileSizeIn) {
    if (NULL == env) {
        ALOGE("decryptDcfSource: invalid JNIEnv.");
        return NULL;
    }
    if (NULL == drmManagerClient || NULL == decryptHandle.get()) {
        ALOGE("decryptDcfSource: drmManagerClient or decryptHandle is NULL");
        return NULL;
    }

    int fileLength = 0;

    if (decryptHandle->decryptInfo) {
        fileLength = decryptHandle->decryptInfo->decryptBufferLength;
    } else {
        ALOGD("decryptDcfSource: file length from para %d Bytes", fileLength);
        fileLength = fileSizeIn;
    }

    ALOGD("decryptDcfSource: file length %d Bytes", fileLength);
    if (fileLength <= 0) {
        ALOGE("decryptDcfSource: illegal file length.%d", fileLength);
        return NULL;
    }

    //allocate buffer to hold decrypted data
    // just use the length of the encrypted data
    unsigned char *decryptedBuf = (unsigned char*)malloc(fileLength);
    memset(decryptedBuf, 0, fileLength);

    unsigned char buffer[DECRYPT_BUF_LEN];
    memset(buffer, 0, sizeof(buffer));

    int readSize = 0;
    int resultSize = 0; // also the offset of data
    while (readSize < fileLength) {
        memset(buffer, 0, sizeof(buffer));
        int size = (fileLength - readSize) > DECRYPT_BUF_LEN ?
            DECRYPT_BUF_LEN : (fileLength - readSize);
        int readLength = drmManagerClient->pread(decryptHandle, buffer, size, resultSize);
        if (readLength <= 0) {
            ALOGE("decryptDcfSource: failed to read valid decrypted data.");
            break;
        }
        memcpy(decryptedBuf + resultSize, buffer, readLength);
        readSize += size;
        resultSize += readLength;
    }
    if (resultSize <= 0) {
        ALOGE("decryptDcfSource: failed to read decrypted data.");
        free(decryptedBuf);
        return NULL;
    }

    if (NULL != length) {
        *length = resultSize;
        ALOGD("decryptDcfSource: returned buffer length %d", resultSize);
    }
    return decryptedBuf;
}

static jbyteArray nativeForceDecryptFile(JNIEnv* env, jobject thiz, jstring filePath,
        jboolean consume) {
    ALOGD("nativeForceDecryptFile ---->");
    if (NULL == env || NULL == thiz) {
        ALOGE("nativeForceDecryptFile: invalid JNI env or thiz.");
        return NULL;
    }
    if (NULL == filePath) {
        ALOGE("nativeForceDecryptFile: invalid filePath.");
        return NULL;
    }

    //get file name
    //!!!!!! whether pathname needs to be freed when returned ?!!!!!!!!
    const char* pathname = env->GetStringUTFChars(filePath, NULL);
    if (NULL == pathname) {
        ALOGE("nativeForceDecryptFile: failed to get file path name.");
        return NULL;
    }
    ALOGD("nativeForceDecryptFile: attempt to decrypt file [%s]", pathname);

    int fd = open(pathname, O_RDONLY);
    if (fd < 0) {
        ALOGE("nativeForceDecryptFile: failed to open file to read.");
        return NULL;
    }

    // open decryptSession...
    DrmManagerClient *drmManagerClient = new DrmManagerClient();
    if (NULL == drmManagerClient) {
        ALOGE("nativeForceDecryptFile: failed to create DrmManagerClient.");
        close(fd);
        return NULL;
    }
    sp<DecryptHandle> decryptHandle = drmManagerClient->openDecryptSession(fd, 0, 0, NULL);
    if (decryptHandle.get() == NULL) {
        ALOGE("nativeForceDecryptFile: failed to open decrypt session.");
        delete drmManagerClient;
        close(fd);
        return NULL;
    }

    struct stat sb;
    int fileSizeIn = 0;
    if (fstat(fd, &sb) == 0 && sb.st_size > 0) {
        fileSizeIn = (int) sb.st_size;
    }

    int length = 0;
    unsigned char* decryptedBuf = decryptDcfSource(env, drmManagerClient, decryptHandle,
        &length, fileSizeIn);

    if (NULL == decryptedBuf) {
        ALOGE("nativeForceDecryptFile: failed to decrypt DCF.");
        drmManagerClient->closeDecryptSession(decryptHandle);
        delete drmManagerClient;
        close(fd);
        return NULL;
    }
    ALOGD("nativeForceDecryptFile: result length [%d]", length);

    if (consume == JNI_TRUE) {
        ALOGD("nativeForceDecryptFile: consume rights.");
        drmManagerClient->consumeRights(decryptHandle, Action::DISPLAY, false);
    }

    drmManagerClient->closeDecryptSession(decryptHandle);
    delete drmManagerClient;
    close(fd);

    // the result
    jbyteArray result = env->NewByteArray(length);
    env->SetByteArrayRegion(result, 0, length, (const jbyte*)decryptedBuf);
    free(decryptedBuf);

    return result;
}

static jbyteArray nativeDecryptDcfFile(JNIEnv* env, jobject thiz, jobject fileDescriptor,
        jint size, jint action) {
    if (NULL == env || NULL == thiz) {
        ALOGE("nativeForceDecryptFile: invalid JNI env or thiz.");
        return NULL;
    }
    ALOGD("nativeDecryptDcfFile: size = %d, action = %d", size , action);

    // get file descriptor in jni
    int fd = (fileDescriptor == NULL) ? -1 : jniGetFDFromFileDescriptor(env, fileDescriptor);
    if (fd < 0) {
        ALOGE("nativeDecryptDcfFile: failed to open file to read.");
        return NULL;
    }

    // open decryptSession...
    DrmManagerClient *drmManagerClient = new DrmManagerClient();
    if (NULL == drmManagerClient) {
        ALOGE("nativeDecryptDcfFile: failed to create DrmManagerClient.");
        return NULL;
    }
    // Set a flag with parameter offset to tell drm plugin don't show renew/expire/secure time
    // dialog. flag value: 0xfffffff or action value.
    // ACTION_DECODE_FULL_IMAGE: show renew/expire/secure time invalid dialog
    // ACTION_JUST_DECODE_BOUND: return decrypt session to decode bound
    // ACTION_JUST_DECODE_THUMBNAIL: don't show dialog and don't return decrypt session
    off64_t offset = 0xfffffff + action;
    ALOGD("nativeDecryptDcfFile: openDecryptSession with offset = %lld", (long long) offset);
    sp<DecryptHandle> decryptHandle = drmManagerClient->openDecryptSession(fd, offset, size, NULL);
    if (NULL == decryptHandle.get()) {
        ALOGE("nativeDecryptDcfFile: failed to openDecryptSession, trigger showDrmDialogIfNeed");
        if (action == ACTION_DECODE_FULL_IMAGE) {
            // If OMA DRM file, try to trigger check whether need show drm dialog
            const int infoType = 2021; // DrmRequestType::TYPE_SET_DRM_INFO
            const String8 mimeType = String8("application/vnd.oma.drm.content");
            DrmInfoRequest* drmInfoRequest = new DrmInfoRequest(infoType, mimeType);
            drmInfoRequest->put(String8("action"), String8("showDrmDialogIfNeed"));
            // FileDescriptor
            char fdStr[32] = {0};
            sprintf(fdStr, "FileDescriptor[%d]", fd);
            drmInfoRequest->put(String8("FileDescriptorKey"), String8(fdStr));
            // Data length
            char sizeStr[32] = {0};
            sprintf(sizeStr, "%d", size);
            drmInfoRequest->put(String8("data"), String8(sizeStr));
            drmManagerClient->acquireDrmInfo(drmInfoRequest);
            delete drmInfoRequest; drmInfoRequest = NULL;
        }
        delete drmManagerClient; drmManagerClient = NULL;
        return NULL;
    }

    int length = 0;

    struct stat sb;
    int fileSizeIn = 0;
    if (fstat(fd, &sb) == 0 && sb.st_size > 0) {
        fileSizeIn = (int) sb.st_size;
    }

    unsigned char* decryptedBuf = decryptDcfSource(env, drmManagerClient, decryptHandle,
        &length, fileSizeIn);
    if (NULL == decryptedBuf) {
        ALOGE("nativeDecryptDcfFile: failed to decrypt DCF.");
        drmManagerClient->closeDecryptSession(decryptHandle);
        delete drmManagerClient;
        return NULL;
    }
    ALOGD("nativeDecryptDcfFile: result length [%d]", length);

    if (action == ACTION_DECODE_FULL_IMAGE) {
        ALOGD("nativeDecryptDcfFile: consume rights.");
        String8 procName = DrmMtkUtil::getProcessName(IPCThreadState::self()->getCallingPid());
        decryptHandle->extendedData.add(String8("clientProcName"), procName);
        drmManagerClient->consumeRights(decryptHandle, Action::DISPLAY, false);
    }

    drmManagerClient->closeDecryptSession(decryptHandle);
    delete drmManagerClient;

    // the result
    jbyteArray result = env->NewByteArray(length);
    env->SetByteArrayRegion(result, 0, length, (const jbyte*)decryptedBuf);
    free(decryptedBuf);

    return result;
}

///////////////////////////////////////////////////////////////////////////////
//JNI register
///////////////////////////////////////////////////////////////////////////////

static const JNINativeMethod nativeMethods[] = {
    {"nativeForceDecryptFile", "(Ljava/lang/String;Z)[B",
                                        (void*)nativeForceDecryptFile},

    {"nativeDecryptDcfFile", "(Ljava/io/FileDescriptor;II)[B",
                                        (void*)nativeDecryptDcfFile}
};

static int registerNativeMethods(JNIEnv* env) {
    int result = -1;

    /* look up the class */
    jclass clazz = env->FindClass("com/mediatek/dcfdecoder/DcfDecoder");

    if (NULL != clazz) {
        if (env->RegisterNatives(clazz, nativeMethods, sizeof(nativeMethods)
                / sizeof(nativeMethods[0])) == JNI_OK) {
            result = 0;
        }
    }
    return result;
}

jint JNI_OnLoad(JavaVM* vm, void* /* reserved */) {
    JNIEnv* env = NULL;
    jint result = -1;

    if (vm->GetEnv((void**) &env, JNI_VERSION_1_4) == JNI_OK) {
        if (NULL != env && registerNativeMethods(env) == 0) {
            result = JNI_VERSION_1_4;
        }
    }
    return result;
}
