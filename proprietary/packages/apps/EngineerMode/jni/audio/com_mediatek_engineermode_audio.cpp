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

#define MTK_LOG_ENABLE 1
#include "jni.h"
#include "android_runtime/AndroidRuntime.h"
#undef LOG_NDEBUG
#undef NDEBUG

#ifdef LOG_TAG
#undef LOG_TAG
#define LOG_TAG "emAudio-JNI"
#endif

#include <media/AudioSystem.h>
#include <cutils/log.h>
#include "AudioParamParser.h"

#include <algorithm>
#include <map>

#include "AudioToolkit.h"
#include <sys/stat.h>

#include "AudioDef.h"
extern "C" {
#define MAX_DUMP_FILE_READ_SIZE (700000)
#define calc_time_diff(x,y) ((x.tv_sec - y.tv_sec) * 1000 + (double)(x.tv_nsec - y.tv_nsec) * 1e-6)

using namespace android;

static String8 keySetBuffer = String8("SetBuffer=");
static String8 keyGetBuffer = String8("GetBuffer=");
static String8 keySetCmd = String8("SetCmd=");
static String8 keyGetCmd = String8("GetCmd=");
static std::map<jint, String8> createBuffterMap()
{
    std::map<int, String8> key_map;
    key_map[0x500] = String8("GetATTDisplayInfoTc1");
    key_map[0x501] = String8("GetATTDisplayInfoTc1BtNrec");
    key_map[0x6] = String8("GetAudioCustomParamFromNvRam");
    key_map[0x100] = String8("GetVolumeVer1ParamFromNvRam");
    key_map[0x42] = String8("GetNBSpeechParamFromNvRam");
    key_map[0x40] = String8("GetWBSpeechParamFromNvRam");
    key_map[0xC0] = String8("GetMagiConSpeechParamFromNvRam");
    key_map[0xD0] = String8("GetHACSpeechParamFromNvRam");
    return key_map;
}

static std::map<jint, String8> createCmdMap()
{
    std::map<int, String8> key_map;
    key_map[0x5] = String8("GetAudioCustomDataSize");
    key_map[0x2] = String8("GetSpeechOutFirIdxFromNvRam");
    key_map[0x10] = String8("GetSpeechNormalOutFirIdxFromNvRam");
    key_map[0x11] = String8("GetSpeechHeadsetOutFirIdxFromNvRam");
    key_map[0x12] = String8("GetSpeechHandfreeOutFirIdxFromNvRam");
    key_map[0x5F] = String8("GetDumpAEECheck");
    key_map[0x64] = String8("GetDumpAudioStreamOut");
    key_map[0x66] = String8("GetDumpAudioMixerBuf");
    key_map[0x68] = String8("GetDumpAudioTrackBuf");
    key_map[0x6A] = String8("GetDumpA2DPStreamOut");
    key_map[0x6C] = String8("GetDumpAudioStreamIn");
    key_map[0x6E] = String8("GetDumpIdleVM");
    key_map[0xA1] = String8("GetDumpApSpeechEPL");
    key_map[0xA3] = String8("GetMagiASRTestEnable");
    key_map[0xA5] = String8("GetAECRecTestEnable");
    return key_map;
}

static String8 getKey(std::map<jint, String8> key_map, jint id)
{
    std::map<jint, String8>::const_iterator finder = key_map.find(id);
    if (finder != key_map.end())
    {
        return finder->second;
    }
    else
    {
        return String8("");
    }
}
static const std::map<jint, String8> get_buffer_mappings = createBuffterMap();
static const std::map<jint, String8> get_cmd_mappings = createCmdMap();

JNIEXPORT jstring JNICALL Java_com_mediatek_engineermode_audio_AudioTuningJni_getCategory(JNIEnv *env, jclass, jstring param1, jstring param2) {
    ALOGD("Enter getCategory\n");
    const char *param1Jni = env->GetStringUTFChars(param1, NULL);
    const char *param2Jni = env->GetStringUTFChars(param2, NULL);
    ALOGD("Param is %s, %s\n", param1Jni, param2Jni);
    char* result = appOpsGetInstance()->utilNativeGetCategory(param1Jni, param2Jni);
    ALOGD("Result is %s\n", result);
    jstring javeResult = env->NewStringUTF(result);
    free(result);
    return javeResult;
}

JNIEXPORT jstring JNICALL Java_com_mediatek_engineermode_audio_AudioTuningJni_getParams(JNIEnv *env, jclass, jstring param1, jstring param2,
        jstring param3) {
    ALOGD("Enter getParams\n");
    const char *param1Jni = env->GetStringUTFChars(param1, NULL);
    const char *param2Jni = env->GetStringUTFChars(param2, NULL);
    const char *param3Jni = env->GetStringUTFChars(param3, NULL);
    ALOGD("Param is %s, %s, %s\n", param1Jni, param2Jni, param3Jni);
    char* result = appOpsGetInstance()->utilNativeGetParam(param1Jni, param2Jni, param3Jni);
    ALOGD("Result is %s\n", result);
    jstring javeResult = env->NewStringUTF(result);
    free(result);
    return javeResult;
}

JNIEXPORT jstring JNICALL Java_com_mediatek_engineermode_audio_AudioTuningJni_getChecklist(JNIEnv *env, jclass, jstring param1, jstring param2,
        jstring param3) {
    ALOGD("Enter getChecklist\n");
    const char *param1Jni = env->GetStringUTFChars(param1, NULL);
    const char *param2Jni = env->GetStringUTFChars(param2, NULL);
    const char *param3Jni = env->GetStringUTFChars(param3, NULL);
    ALOGD("Param is %s, %s, %s\n", param1Jni, param2Jni, param3Jni);
    const char* result = appOpsGetInstance()->utilNativeGetChecklist(param1Jni, param2Jni,
            param3Jni);
    ALOGD("Result is %s\n", result);
    jstring javeResult = env->NewStringUTF(result);
    return javeResult;
}

JNIEXPORT jboolean JNICALL Java_com_mediatek_engineermode_audio_AudioTuningJni_setParams(JNIEnv *env, jclass, jstring param1, jstring param2,
        jstring param3, jstring param4) {
    ALOGD("Enter setParams\n");
    const char *param1Jni = env->GetStringUTFChars(param1, NULL);
    const char *param2Jni = env->GetStringUTFChars(param2, NULL);
    const char *param3Jni = env->GetStringUTFChars(param3, NULL);
    const char *param4Jni = env->GetStringUTFChars(param4, NULL);
    ALOGD("Param is %s, %s, %s, %s\n", param1Jni, param2Jni, param3Jni, param4Jni);
    APP_STATUS result = appOpsGetInstance()->utilNativeSetParam(param1Jni, param2Jni, param3Jni,
            param4Jni);
    if (result == APP_NO_ERROR) {
        ALOGD("set success\n");
        return JNI_TRUE;
    } else {
        ALOGD("set fail\n");
        return JNI_FALSE;
    }
}

JNIEXPORT jboolean JNICALL Java_com_mediatek_engineermode_audio_AudioTuningJni_saveToWork(JNIEnv *env, jclass, jstring param) {
    ALOGD("Enter saveToWork\n");
    const char *paramJni = env->GetStringUTFChars(param, NULL);
    ALOGD("Param is %s\n", paramJni);

    APP_STATUS result = appOpsGetInstance()->utilNativeSaveXml(paramJni);
    if (result == APP_NO_ERROR) {
        ALOGD("set success\n");
        return JNI_TRUE;
    } else {
        ALOGD("set success\n");
        return JNI_FALSE;
    }
}

static void xmlChangedCallback(AppHandle *appHandle, const char *audioTypeName) {
    ALOGD("XML changed! (appHandle = %p, audioType = %s)\n", appHandle, audioTypeName);
}

JNIEXPORT jboolean JNICALL Java_com_mediatek_engineermode_audio_AudioTuningJni_registerXmlChangedCallback(JNIEnv *, jclass) {
    AppOps* appOps = appOpsGetInstance();
    appOps->appSetAudioTypeLoadingList(EM_AUDIO_TYPE_LOADING_LIST);
    AppHandle * appHandle = appOps->appHandleGetInstance();
    appOps->appHandleRegXmlChangedCb(appHandle, xmlChangedCallback);
    ALOGD("enter audio_registerXmlChangedCallback\n");
    return true;
}

JNIEXPORT jboolean JNICALL Java_com_mediatek_engineermode_audio_AudioTuningJni_CustXmlEnableChanged(JNIEnv *, jclass, jint value) {
    AppOps* appOps = appOpsGetInstance();
    AppHandle * appHandle = appOps->appHandleGetInstance();
    appOps->appHandleCustXmlEnableChanged(appHandle, value);
    ALOGD("enter audio_CustXmlEnableChanged=%d\n", value);
    return true;
}
static void print_hex_buffer(size_t len, void* ptr) {
    char *pp;
    char *tempbuf = NULL;
    int sum;
    int printlen = len;
    if (printlen > 128) {
        ALOGD("%s: Truncate length to 128 byte", __FUNCTION__);
        printlen = 128;
    }

    tempbuf = new char[printlen * 16];
    pp = (char*) ptr;
    sum = 0;
    for (int i = 0; i < printlen; ++i) {
        sum += sprintf(tempbuf + sum, "0x%02x ", pp[i]);
    }
    ALOGD("print_hex_buffer: sum=%d, %s", sum, tempbuf);
    delete[] tempbuf;
}

static String8 PrintEncodedString(String8 strKey, size_t len, void *ptr) {
    String8 returnValue = String8("");
    size_t sz_Needed;
    size_t sz_enc;
    char *buf_enc = NULL;
    bool bPrint = false;

    ALOGD("%s in, len = %d", __FUNCTION__, (int)len);
    print_hex_buffer(len, ptr);

    sz_Needed = Base64_OutputSize(true, len);
    buf_enc = new char[sz_Needed + 1];
    if (buf_enc == NULL) {
        ALOGE("%s(), buf_enc allocate fail", __FUNCTION__);
        return returnValue;
    }
    buf_enc[sz_Needed] = 0;

    sz_enc = Base64_Encode((unsigned char *) ptr, buf_enc, len);

    if (sz_enc != sz_Needed) {
        ALOGE("%s, Encode Error!!after encode (%s), len(%d), sz_Needed(%d), sz_enc(%d)",
                __FUNCTION__, buf_enc, (int)len, (int)sz_Needed, (int)sz_enc);
    } else {
        bPrint = true;
        ALOGD("%s(), after encode (%s), len(%d), sz_enc(%d)", __FUNCTION__, buf_enc,
        (int)len, (int)sz_enc);
    }

    if (bPrint) {
        String8 StrVal = String8(buf_enc, sz_enc);
        returnValue += strKey;
        returnValue += StrVal;
//        returnValue += String8(";");
    }

    delete[] buf_enc;

    return returnValue;
}

JNIEXPORT jint JNICALL Java_com_mediatek_engineermode_audio_AudioTuningJni_setAudioCommand(JNIEnv *, jclass, jint par1, jint par2) {

    int iPara[2];
    iPara[0] = par1;
    iPara[1] = par2;

    String8 strPara = PrintEncodedString(keySetCmd, sizeof(iPara), iPara);
    return AudioSystem::setParameters(0, strPara);
}

#define LEN_STR 8
JNIEXPORT jint JNICALL Java_com_mediatek_engineermode_audio_AudioTuningJni_getAudioCommand(JNIEnv *, jclass, jint par1) {
    String8 returnValue;
    char cmd[8];
    jint par2;

    sprintf(cmd, "%d", par1);
    String8 cmd_key = getKey(get_cmd_mappings, par1);
    returnValue = AudioSystem::getParameters(0, cmd_key);
    par2 = atoi(returnValue.string() + cmd_key.size() + 1);
    ALOGD("%s: cmd:%s %d, returnValue: %s, *par2 = %d", __FUNCTION__, cmd_key.string(), par1,
            returnValue.string(), par2);
    return par2;
}

JNIEXPORT jint JNICALL Java_com_mediatek_engineermode_audio_AudioTuningJni_setAudioData(JNIEnv *env, jclass, jint par1, jint len, jbyteArray aptr) {
    jbyte *ptr;
    jint ret;
    ptr = env->GetByteArrayElements(aptr, NULL);

    ALOGD("%s in: par1 = %d, len = %d", __FUNCTION__, par1, len);
    size_t sz_in = (sizeof(par1) + sizeof(len) + len);

    unsigned char *buf = new unsigned char[sz_in];
    if (NULL == buf) {
        ALOGE("Fail to allocate memory !!");
        return NO_MEMORY;
    }
    int *iBuf = (int*) buf;
    *iBuf = par1;
    *(iBuf + 1) = len;

    unsigned char *cptr;
    cptr = buf + sizeof(par1) + sizeof(len);
    memcpy(cptr, ptr, len);

    String8 strPara = PrintEncodedString(keySetBuffer, sz_in, buf);
    ALOGD("%s: strPara = %s", __FUNCTION__, strPara.string());

    delete[] buf;

    ret = AudioSystem::setParameters(0, strPara);

    env->ReleaseByteArrayElements(aptr, ptr, 0);
    return ret;
}
static status_t GetDecodedData(String8 strPara, size_t len, void *ptr) {
    size_t sz_in = strPara.size();
    size_t sz_needed = Base64_OutputSize(false, sz_in);
    size_t sz_dec;
    status_t ret = NO_ERROR;

    if (sz_in <= 0)
        return NO_ERROR;

    ALOGD("%s in, len = %d", __FUNCTION__, (int)len);
    unsigned char *buf_dec = new unsigned char[sz_needed];
    sz_dec = Base64_Decode(strPara.string(), buf_dec, sz_in);

    if (sz_dec > sz_needed || sz_dec <= sz_needed - 3) {
        ALOGE("%s,Decode Error!!after decode(%s), sz_in(%d), sz_needed(%d), sz_dec(%d)",
                __FUNCTION__, buf_dec, (int)sz_in, (int)sz_needed, (int)sz_dec);
    } else {
        // sz_needed-3 < sz_dec <= sz_needed
        ALOGD("%s(), after decode, sz_in(%d), sz_dec(%d) len(%d) sizeof(ret)=%d",
                __FUNCTION__, (int)sz_in, (int)sz_dec, (int)len,(int)sizeof(ret));
        print_hex_buffer(sz_dec, buf_dec);
    }

    if ((len == 0) || (len == sz_dec - sizeof(ret))) {
        if (len) {
            ret = (status_t) * (buf_dec);
            unsigned char *buff = (buf_dec + 4);
            memcpy(ptr, buff, len);
        } else {
            const char * IntPtr = (char *) buf_dec;
            ret = atoi(IntPtr);
            ALOGD("%s len = 0 ret(%d)", __FUNCTION__, ret);
        }
    } else {
        ALOGD("%s decoded buffer isn't right format", __FUNCTION__);
    }

    if (buf_dec != NULL) {
        delete[] buf_dec;
    }

    return ret;
}
JNIEXPORT jint JNICALL Java_com_mediatek_engineermode_audio_AudioTuningJni_getAudioData(JNIEnv *env, jclass, jint par1, jint len, jbyteArray aptr) {
    jbyte *ptr;
    jint ret;
    ptr = env->GetByteArrayElements(aptr, NULL);

    String8 buffer_key = getKey(get_buffer_mappings, par1);
    String8 returnValue = AudioSystem::getParameters(0, buffer_key);
    ALOGD("%s  AudioSystem::getParameters(%s %d) returnValue = %s", __FUNCTION__,
            buffer_key.string(), par1, returnValue.string());

    String8 newval; //remove "GetBuffer="
    newval.appendFormat("%s", returnValue.string() + buffer_key.size() + 1);
    ALOGD("%s(), newval = %s", __FUNCTION__, newval.string());

    ret = GetDecodedData(newval, len, ptr);
    env->ReleaseByteArrayElements(aptr, ptr, 0);

    return ret;
}
JNIEXPORT jint JNICALL Java_com_mediatek_engineermode_audio_AudioTuningJni_setEmParameter(JNIEnv *env, jclass, jbyteArray aptr, jint len) {
    jint par1 = 0x43;
    jbyte *ptr;
    jint ret;
    ptr = env->GetByteArrayElements(aptr, NULL);

    ALOGD("%s in: par1 = %d, len = %d", __FUNCTION__, par1, len);
    size_t sz_in = (sizeof(par1) + sizeof(len) + len);

    unsigned char *buf = new unsigned char[sz_in];
    if (NULL == buf) {
        ALOGE("Fail to allocate memory !!");
        return NO_MEMORY;
    }
    int *iBuf = (int*) buf;
    *iBuf = par1;
    *(iBuf + 1) = len;

    unsigned char *cptr;
    cptr = buf + sizeof(par1) + sizeof(len);
    memcpy(cptr, ptr, len);

    String8 strPara = PrintEncodedString(keySetBuffer, sz_in, buf);
    ALOGD("%s: strPara = %s", __FUNCTION__, strPara.string());

    delete[] buf;

    ret = AudioSystem::setParameters(0, strPara);

    env->ReleaseByteArrayElements(aptr, ptr, 0);

    return ret;
}

JNIEXPORT jint JNICALL Java_com_mediatek_engineermode_audio_AudioTuningJni_getEmParameter(JNIEnv *env, jclass, jbyteArray aptr, jint len) {
    jbyte *ptr;
    jint ret;
    ptr = env->GetByteArrayElements(aptr, NULL);

    String8 buffer_key = getKey(get_buffer_mappings, 0x42);
    String8 returnValue = AudioSystem::getParameters(0, buffer_key);
    ALOGD("%s  AudioSystem::getParameters(%s) returnValue = %s", __FUNCTION__,
            buffer_key.string(), returnValue.string());

    String8 newval; //remove "GetBuffer="
    newval.appendFormat("%s", returnValue.string() + buffer_key.size() + 1);
    ALOGD("%s(), newval = %s", __FUNCTION__, newval.string());

    ret = GetDecodedData(newval, len, ptr);
    env->ReleaseByteArrayElements(aptr, ptr, 0);

    return ret;
}

static jint createDirectory(const char *pC) {
    char tmp[128];
    int i = 0;
    while (*pC) {
        tmp[i] = *pC;
        if (*pC == '/' && i) {
            tmp[i] = '\0';
            if (access(tmp, F_OK) != 0) {
                if (mkdir(tmp, 0770) == -1) {
                    ALOGE("AudioDumpPCM: mkdir error! %s\n", (char *)strerror(errno));
                    return -1;
                }
            }
            tmp[i] = '/';
        }
        i++;
        pC++;
    }

    return 0;
}

static pthread_t copyAudioDumpThread;
static pthread_mutex_t exec_lock = PTHREAD_MUTEX_INITIALIZER;
static jboolean sCancelCopy = false;
static jboolean sNeedDetach = false;
JavaVM *g_VM;

static void *copyAudioHalDumpFiles(void *p)
{
    if (p == NULL) {
        ALOGE("ERROR: get callback failed\n");
        return NULL;
    }
    jobject jcallback = (jobject)p;
    JNIEnv *env;
    if (g_VM->GetEnv((void**) &env, JNI_VERSION_1_4) == JNI_EDETACHED) {
        //主动附加到jvm环境中，获取到env
        if (g_VM->AttachCurrentThread(&env, NULL) != 0) {
            env->DeleteGlobalRef(jcallback);
            jcallback = NULL;
            return NULL;
        }
        sNeedDetach = JNI_TRUE;
    }

    //通过强转后的jcallback 获取到要回调的类
    jclass javaClass = env->GetObjectClass(jcallback);
    if (javaClass == 0) {
        ALOGW("Unable to find class");
        g_VM->DetachCurrentThread();
        env->DeleteGlobalRef(jcallback);
        jcallback = NULL;
       if(sNeedDetach) {
            g_VM->DetachCurrentThread();
        }
        return NULL;
    }

    ALOGD("get callback class");

   //获取要回调的方法ID
    jmethodID javaCallbackId = env->GetMethodID(javaClass, "onCopyProgressChanged",
            "(Ljava/lang/String;FF)V");
    if (javaCallbackId == NULL) {
        ALOGD("Unable to find method:onCopyProgressChanged");
        env->DeleteGlobalRef(jcallback);
        jcallback = NULL;
       if(sNeedDetach) {
            g_VM->DetachCurrentThread();
        }
        return NULL;
    }

    char *restOfStr = NULL;
    String8 fileListStr = getAudioHalDumpFileList();
    ALOGD("%s(), getDumpFileList return %s\n", __FUNCTION__, fileListStr.string());

    char* fileList = strdup(fileListStr);
    char* fileName = strtok_r(fileList, ",", &restOfStr);
    char* fileSizeStr = NULL;
    size_t fileSize = 0;
    struct timespec startTime, endTime;

    createDirectory(SD_DUMP_PATH);
    ALOGD("%s(), createDirectory %s\n", __FUNCTION__, SD_DUMP_PATH);

    while (fileName != NULL) {
        fileSizeStr = strtok_r(NULL, ",", &restOfStr);
        if (fileSizeStr == NULL) {
            ALOGW("%s(), no file size info... (%s)\n", __FUNCTION__, fileName);
            break;
        }
        if (sscanf(fileSizeStr, "%zu", &fileSize) == 0) {
            ALOGW("%s(), wrong file size info... (%s)\n", __FUNCTION__, fileName);
            break;
        }

        ALOGD("%s(), file name = %s, size = %zu\n", __FUNCTION__, fileName, fileSize);

        clock_gettime(CLOCK_REALTIME, &startTime);

        // Fopen
        String8 outputFilePath = String8(SD_DUMP_PATH) + String8(fileName);
        FILE* outFp = fopen(outputFilePath.string(), "w");
        if (outFp == NULL) {
            ALOGW("%s(), Cannot open file %s\n", __FUNCTION__, outputFilePath.string());
            continue;
        }

        unsigned char *buffer = NULL;
        size_t readIdx = 0;
        size_t readSize = 0;
        sCancelCopy = false;
        while (readIdx < fileSize - 1) {

            if(sCancelCopy == true) {
                pthread_mutex_lock(&exec_lock);
                ALOGD("%s() !!Cancel copy file %s!!!\n", __FUNCTION__, fileName);
                sCancelCopy = false;
                pthread_mutex_unlock(&exec_lock);

                if (outFp){
                    fclose(outFp);
                    outFp = NULL;
                }
                free(fileList);
                env->DeleteGlobalRef(jcallback);
                jcallback = NULL;
                if(sNeedDetach) {
                    g_VM->DetachCurrentThread();
                }
                return NULL;
            }
            // read buffer
            readSize = fileSize - readIdx;
            if (readSize > MAX_DUMP_FILE_READ_SIZE) {
                readSize = MAX_DUMP_FILE_READ_SIZE;
            }

            size_t retSize = readAudioHalDumpFileContent(fileName, &buffer, readIdx, readSize);

            // write buffer to file
            if (retSize == 0) {
                ALOGD("%s(), Read data size 0\n", __FUNCTION__);

                if (buffer) {
                    delete[] buffer;
                    buffer = NULL;
                }
                break;
            }

            ALOGD("%s(), read data from %zu to %zu\n", __FUNCTION__, readIdx, readIdx
            + retSize);

            // write file to sdcard
            size_t writeSize = fwrite(buffer, 1, retSize, outFp);
            if (writeSize != retSize) {
                ALOGW("%s(), write size(%zu) but the size is not %zu\n", __FUNCTION__,
                 writeSize, retSize);
            } else {
                ALOGD("%s(), Write %zu/%zu bytes to dump file (%s)\n", __FUNCTION__,
                 readIdx + retSize, fileSize, outputFilePath.string());
                 // call JAVA interface to update UI
                 env->CallVoidMethod(jcallback, javaCallbackId, env->NewStringUTF(fileName),
                         ((float)(readIdx+retSize)/(1024*1024)), (float)(fileSize)/(1024*1024));
            }

            if (buffer) {
                delete[] buffer;
                buffer = NULL;
            }

            readIdx += retSize;
        }

        if (outFp){
            fclose(outFp);
            outFp = NULL;
        }

        clock_gettime(CLOCK_REALTIME, &endTime);
        double time = calc_time_diff(endTime, startTime);
        ALOGD("%s,Copy %s(size: %zu KB)spend %f ms(speed: %f MB/s,MAX_READ_SIZE: %d)\n",
         __FUNCTION__, fileName, readIdx/(1024), time, (readIdx/(1024*1024)) / (time*1000),
        MAX_DUMP_FILE_READ_SIZE);

        fileName = strtok_r(NULL, ",", &restOfStr);
    }
    ALOGD("copy all AudioHalDumpFiles");
    env->CallVoidMethod(jcallback, javaCallbackId, env->NewStringUTF("SUCCESS"),0, 0);
    free(fileList);

    env->DeleteGlobalRef(jcallback);
    jcallback = NULL;
    if(sNeedDetach) {
        g_VM->DetachCurrentThread();
    }
    return NULL;
}

JNIEXPORT void JNICALL Java_com_mediatek_engineermode_audio_AudioTuningJni_copyAudioHalDumpFilesToSdcard(JNIEnv *env, jclass,jobject jcallback)
{
    env->GetJavaVM(&g_VM);
    jobject callback = env->NewGlobalRef(jcallback);
    pthread_create(&copyAudioDumpThread, 0, copyAudioHalDumpFiles, callback);
}

JNIEXPORT void JNICALL Java_com_mediatek_engineermode_audio_AudioTuningJni_cancleCopyAudioHalDumpFile (JNIEnv *, jclass) {
    pthread_mutex_lock(&exec_lock);
    ALOGD("%s() Cancel copy file\n", __FUNCTION__);
    sCancelCopy = true;
    pthread_mutex_unlock(&exec_lock);
}

static void *delAudioHalDumpFiles(void *p) {
    if (p == NULL) {
        ALOGE("ERROR: get callback failed\n");
        return NULL;
    }
    jobject jcallback = (jobject)p;
    JNIEnv *env;
    if (g_VM->GetEnv((void**) &env, JNI_VERSION_1_4) == JNI_EDETACHED) {
        //主动附加到jvm环境中，获取到env
        if (g_VM->AttachCurrentThread(&env, NULL) != 0) {
            env->DeleteGlobalRef(jcallback);
            jcallback = NULL;
            return NULL;
        }
        sNeedDetach = JNI_TRUE;
    }

    //通过强转后的jcallback 获取到要回调的类
    jclass javaClass = env->GetObjectClass(jcallback);
    if (javaClass == 0) {
        ALOGW("Unable to find class");
        g_VM->DetachCurrentThread();
        env->DeleteGlobalRef(jcallback);
        jcallback = NULL;
        if(sNeedDetach) {
            g_VM->DetachCurrentThread();
        }
        return NULL;
    }

   //获取要回调的方法ID
    jmethodID javaCallbackId = env->GetMethodID(javaClass, "onDeleteProgressChanged",
            "(Ljava/lang/String;)V");
    if (javaCallbackId == NULL) {
        ALOGD("Unable to find method:onDeleteProgressChanged");
        env->DeleteGlobalRef(jcallback);
        jcallback = NULL;
        if(sNeedDetach) {
            g_VM->DetachCurrentThread();
        }
        return NULL;
    }

    ALOGD("delAudioHalDumpFiles start");

    char *restOfStr = NULL;
    String8 fileListStr = getAudioHalDumpFileList();
    char* fileList = strdup(fileListStr);
    char* fileName = strtok_r(fileList, ",", &restOfStr);

    ALOGD("%s(), getDumpFileList return %s\n", __FUNCTION__, fileListStr.string());

    while (fileName != NULL) {
        char* fileSizeStr = strtok_r(NULL, ",", &restOfStr);
        if (fileSizeStr == NULL) {
            ALOGW("%s(), no file size info... (%s)\n", __FUNCTION__, fileName);
            break;
        }

         // call JAVA interface to update UI
        env->CallVoidMethod(jcallback, javaCallbackId, env->NewStringUTF(fileName));
        ALOGD("del all AudioHalDumpFiles %s", fileName);
        delAudioHalDumpFile(fileName);
        fileName = strtok_r(NULL, ",", &restOfStr);
    }

    env->CallVoidMethod(jcallback, javaCallbackId, env->NewStringUTF("SUCCESS"));
    free(fileList);
    ALOGD("delAudioHalDumpFiles done");
    env->DeleteGlobalRef(jcallback);
    jcallback = NULL;
    if(sNeedDetach) {
        g_VM->DetachCurrentThread();
    }
    return NULL;
}

JNIEXPORT void JNICALL Java_com_mediatek_engineermode_audio_AudioTuningJni_delAudioHalDumpFiles(JNIEnv *env, jclass, jobject jcallback) {
    if(g_VM == NULL) {
        env->GetJavaVM(&g_VM);
    }
    jobject callback = env->NewGlobalRef(jcallback);
    pthread_create(&copyAudioDumpThread, 0, delAudioHalDumpFiles, callback);
}

JNIEXPORT jboolean JNICALL Java_com_mediatek_engineermode_audio_AudioTuningJni_isFeatureSupported(JNIEnv *env, jclass, jstring param) {
    const char* foName = env->GetStringUTFChars(param, NULL);//"MTK_TTY_SUPPORT";
    printf("Get %s FO = %s(%d)\n", foName, appGetFeatureOptionValue(foName), appIsFeatureOptionEnabled(foName));
    return appIsFeatureOptionEnabled(foName);
}

}// extern "C"