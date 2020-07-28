/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2010. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
 * AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek Software")
 * have been modified by MediaTek Inc. All revisions are subject to any receiver's
 * applicable license agreements with MediaTek Inc.
 */

#undef LOG_NDEBUG 
#undef NDEBUG
#define LOG_TAG "EM-Wifi-JNI"
#define MTK_LOG_ENABLE 1
#include <cutils/log.h>

#include <stdio.h>
#include <unistd.h>

#include "jni.h"
#include <nativehelper/JNIHelp.h>
#include "type.h"
#include "android_runtime/AndroidRuntime.h"
#include "WiFi_EM_API.h"
#include <inttypes.h>

using namespace android;

#define UNIMPLEMENT  	-2
#define STATUS_ERROR  -1
#define STATUS_OK     0
static CAdapter *s_adapter;
static COID *s_oid;
static int sRefCount = 0;
pthread_mutex_t g_mutex = PTHREAD_MUTEX_INITIALIZER;

static int com_mediatek_engineermode_wifi_EMWifi_setTestMode(JNIEnv *env,
        jobject thiz) {
    UNUSED(env, thiz);
    ALOGE("JNI, Enter setTestMode succeed");
    return s_adapter->setTestMode();
}

static int com_mediatek_engineermode_wifi_EMWifi_setNormalMode(JNIEnv *env,
        jobject thiz) {
    UNUSED(env, thiz);
    ALOGE("JNI, Enter setNormalMode succeed");
    return s_adapter->setNormalMode();
}

static int com_mediatek_engineermode_wifi_EMWifi_setStandBy(JNIEnv *env,
        jobject thiz) {
    UNUSED(env, thiz);
    ALOGE("JNI, Enter setStandBy succeed");
    return s_adapter->setStandBy();
}

static int com_mediatek_engineermode_wifi_EMWifi_setEEPRomSize(JNIEnv *env,
        jobject thiz, jlong i4EepromSz) {
    UNUSED(env, thiz);
    ALOGE("JNI Set the eep_rom size is size = (%d)", ((int) i4EepromSz));
    return s_adapter->setEEPRomSize((INT_32)i4EepromSz);
}


static int com_mediatek_engineermode_wifi_EMWifi_readTxPowerFromEEPromEx(
        JNIEnv *env, jobject thiz, jlong i4ChnFreg, jlong i4Rate,
        jlongArray PowerStatus, jint arrayLength) {
    UNUSED(thiz);

    jlong valBuff[8] = {0};
    INT_32 pi4TxPwrGain = -1;
    INT_32 pi4Eerp = -1;
    INT_32 pi4ThermoVal = -1;
    INT_32 index = -1;

    if (PowerStatus == NULL) {
        ALOGE("NULL java array of TxPower from EE Prom Ex");
        return -2;
    }

    if (arrayLength != 3) {
        ALOGE("Error length pass to the array");
        return STATUS_ERROR;
    } else {
        index = s_adapter->readTxPowerFromEEPromEx((INT_32)i4ChnFreg, (INT_32)i4Rate,
                &pi4TxPwrGain, &pi4Eerp, &pi4ThermoVal);
        if (index == 0) {
            valBuff[0] = pi4TxPwrGain;
            valBuff[1] = pi4Eerp;
            valBuff[2] = pi4ThermoVal;
        } else {
            ALOGE("Error to call readTxPowerFromEEPromEx in native");
        }
    }
    ALOGE("get the readTxPowerFromEEPromEx value, pi4TxPwrGain = (%d), pi4Eerp = (%d), pi4ThermoVal = (%d)",
            ((int) pi4TxPwrGain), ((int) pi4Eerp), ((int) pi4ThermoVal));

    env->SetLongArrayRegion(PowerStatus, 0, 3, valBuff);
    return index;
}

static int com_mediatek_engineermode_wifi_EMWifi_setEEPromCKSUpdated(
        JNIEnv *env, jobject thiz) {
    UNUSED(env, thiz);
    ALOGE("JNI, Enter setEEPromCKSUpdated succeed");
    return s_adapter->setEEPromCKSUpdated();
}

static int com_mediatek_engineermode_wifi_EMWifi_getPacketRxStatus(JNIEnv *env,
        jobject thiz, jlongArray i4Init, jint arraylen) {
    UNUSED(thiz);

    ALOGE("JNI, Enter getPacketRxStatus succeed");
    INT_32 index = -1;
    jlong valBuff[4] = {0};
    INT_32 pi4RxOk, pi4RxCrcErr,pi4RxRssi;

    if (arraylen != 3) {
        ALOGE("Wrong array length for getPacketRxStatus");
        return STATUS_ERROR;
    }

    if (i4Init == NULL) {
        ALOGE("NULL java array of getPacketRxStatus");
        return -2;
    }

    index = s_adapter->getPacketRxStatus(&pi4RxOk, &pi4RxCrcErr, &pi4RxRssi);
    if (index == 0) {
        valBuff[0] = pi4RxOk;
        valBuff[1] = pi4RxCrcErr;
        valBuff[2] = pi4RxRssi;
        ALOGE("JNI, getPacketRxStatus value pi4RxOk = (%d), pi4RxCrcErr = (%d),pi4RxRssi = (%0x)",
              (int) pi4RxOk, (int) pi4RxCrcErr, (int) pi4RxRssi);
    } else {
        ALOGE("Native, get getPacketRxStatus failed");
    }
    env->SetLongArrayRegion(i4Init, 0, 3, valBuff);
    return index;
}


static int com_mediatek_engineermode_wifi_EMWifi_setChannel(JNIEnv *env,
        jobject thiz, jlong i4ChFreqKHz) {
    UNUSED(env, thiz);
    ALOGE("JNI, Enter setChannel succeed, country i4ChFreqKHz = %d",
            (int) i4ChFreqKHz);
    INT_32 index = -1;
    index = s_adapter->setChannel((INT_32)i4ChFreqKHz);
    return index;
}


static int com_mediatek_engineermode_wifi_EMWifi_readEEPRom16(JNIEnv *env,
        jobject thiz, jlong u4Offset, jlongArray pu4Value) {
    UNUSED(thiz);
    UINT_32 value = 0;
    INT_32 index = -1;
    jlong targVal = 0;

    if (pu4Value == NULL) {
        ALOGE("NULL java array of readEEPRom16");
        return -2;
    }

    index = s_adapter->readEEPRom16(u4Offset, &value);
    targVal = value;

    ALOGE("JNI, Enter readEEPRom16 succeed, u4Offset = %d, pu4Value = %d",
            (int) u4Offset, (int) value);
    env->SetLongArrayRegion(pu4Value, 0, 1, &targVal);
    return index;
}

static int com_mediatek_engineermode_wifi_EMWifi_writeEEPRom16(JNIEnv *env,
        jobject thiz, jlong u4Offset, jlong u4Value) {
    UNUSED(env, thiz);
    INT_32 index = -1;

    ALOGE("JNI, Enter writeEEPRom16 succeed, u4Offset = %d, u4Value = %d",
            (int) u4Offset, (int) u4Value);
    index = s_adapter->writeEEPRom16(u4Offset, u4Value);

    return index;
}

static int com_mediatek_engineermode_wifi_EMWifi_eepromReadByteStr(JNIEnv *env,
        jobject thiz, jlong u4Addr, jlong u4Length, jbyteArray paucStr) {
    UNUSED(thiz);
    INT_32 index = -1;
    jbyte byteBuffer[512] = {0};

    index = s_adapter->eepromReadByteStr(u4Addr, u4Length, (char *)byteBuffer);

    if (index == 0) {
        env->SetByteArrayRegion(paucStr, 0, sizeof(byteBuffer), byteBuffer);
        ALOGE("JNI, Enter eepromReadByteStr succeed, paucStr = %s", byteBuffer);
    } else {
        ALOGE("Native, eepromReadByteStr call failed");
    }

    return index;
}

static int com_mediatek_engineermode_wifi_EMWifi_eepromWriteByteStr(JNIEnv *env,
        jobject thiz, jlong u4Addr, jlong u4Length, jstring paucStr) {
    UNUSED(thiz);
    INT_32 index = -1;
    INT_32 len = env->GetStringUTFLength(paucStr);
    const CHAR *str = env->GetStringUTFChars(paucStr, NULL);

    CHAR *name = new CHAR[len + 1];
    if (name) {
        memcpy(name, str, len);
        name[len] = '\0';

        index = s_adapter->eepromWriteByteStr(u4Addr, u4Length, name);

        ALOGE("JNI, Enter eepromWriteByteStr succeed, name = %s", name);

        delete[] name;
    }

    ALOGE("JNI, Enter eepromWriteByteStr succeed, u4Addr = %d, u4Length = %d, str = %s",
            (int) u4Addr, (int) u4Length, str);
    env->ReleaseStringUTFChars(paucStr, str);

    return index;
}

static int com_mediatek_engineermode_wifi_EMWifi_setATParam(JNIEnv *env,
        jobject thiz, jlong u4FuncIndex, jlong pu4FuncData) {
    UNUSED(env, thiz);
    ALOGE("JNI, Enter setATParam succeed, u4FuncIndex = %d, pu4FuncData = %d",
            (unsigned int) u4FuncIndex, (unsigned int) pu4FuncData);
    return s_adapter->SetATParam(u4FuncIndex, pu4FuncData);
}

static int com_mediatek_engineermode_wifi_EMWifi_getATParam(JNIEnv *env,
        jobject thiz, jlong u4FuncIndex, jlongArray pu4FuncData) {
    UNUSED(thiz);
    INT_32 index = -1;
    UINT_32 value[2] = {0};
    jlong targVal[2] = {0};

    if (pu4FuncData == NULL) {
        ALOGE("NULL java array of getATParam");
        return -2;
    }

    index = s_adapter->GetATParam(u4FuncIndex, value);
    targVal[0] = value[0];
    targVal[1] = value[1];
    ALOGE("JNI, Enter getATParam succeed, u4FuncIndex = %d, pu4FuncData = %d, pu4FuncData2 = %d",
       (unsigned int) u4FuncIndex, (unsigned int) value[0],(unsigned int) value[1]);
    env->SetLongArrayRegion(pu4FuncData, 0, 2, targVal);
	return index;
}

static int com_mediatek_engineermode_wifi_EMWifi_setPnpPower(JNIEnv *env,
        jobject thiz, jlong i4PowerMode) {
    UNUSED(env, thiz);
    ALOGE("JNI, Enter setPnpPower succeed, i4PowerMode = %d", (int) i4PowerMode);
    return s_adapter->setPnpPower(i4PowerMode);
}

static int com_mediatek_engineermode_wifi_EMWifi_writeMCR32(JNIEnv *env,
        jobject thiz, jlong offset, jlong value) {
    UNUSED(env, thiz);
    ALOGE("JNI, Enter writeMCR32 succeed, offset = %d, value = %d",
            (int) offset, (int) value);
    return s_adapter->writeMCR32(offset, value);
}

static int com_mediatek_engineermode_wifi_EMWifi_readMCR32(JNIEnv *env,
        jobject thiz, jlong offset, jlongArray value) {
    UNUSED(thiz);
    jlong targVal = 0;
    UINT_32 val = 0;
    INT_32 index = -1;

    if (value == NULL) {
        ALOGE("NULL java array of readMCR32");
        return -2;
    }
    index = s_adapter->readMCR32(offset, &val);
    targVal = val;
    ALOGE("JNI, Enter writeMCR32 succeed, offset = %d, value = %d",
            (int) offset, (int) val);
    env->SetLongArrayRegion(value, 0, 1, &targVal);
    return index;
}

static int com_mediatek_engineermode_wifi_EMWifi_initial(JNIEnv *env,
        jobject thiz/*, jlong chipID*/) {
    UNUSED(env, thiz);
    const char *ifname = "wlan0";
    const char *wrapperName="local";
    char desc[NAMESIZE];
    UINT_32 chipID = 0x6620;

    pthread_mutex_lock(&g_mutex);
    ALOGE("JNI, Enter initial, sRefCount: %d", sRefCount);
    if (++sRefCount <= 1) {
        s_oid = new CLocalOID(ifname);
        sprintf(desc, "%x", chipID & DEVID_IDMSK);
        em_printf(MSG_DEBUG, (char*)"CWrapper name is %s desc is %s\n", ifname, desc);
        s_adapter = new CMT66xx(ifname, desc, s_oid, wrapperName, chipID);
    }
    ALOGE("JNI, Exit initial");
    pthread_mutex_unlock(&g_mutex);

    return chipID;
}

static int com_mediatek_engineermode_wifi_EMWifi_unInitial(JNIEnv *env,
        jobject thiz) {
    UNUSED(env, thiz);

    pthread_mutex_lock(&g_mutex);
    ALOGE("JNI, Entener Uninitialize, sRefCount: %d", sRefCount);
    if (--sRefCount <= 0) {
        delete s_oid;
        s_adapter->CloseDevice(); /* if reference equal to 0 or less than 0, will delete s_adapter */
        s_adapter = NULL;
        s_oid = NULL;
    }
    pthread_mutex_unlock(&g_mutex);

    return ERROR_RFTEST_SUCCESS;
}


// Added by mtk54046 @ 2012-01-05 for get support channel list
static int com_mediatek_engineermode_wifi_EMWifi_getSupportChannelList(JNIEnv *env,
        jobject thiz, jlongArray pau4Channel) {
    #define MAX_CHANNEL_NUM 75
    UINT_32 value[MAX_CHANNEL_NUM] = {0};
    jlong buffer[MAX_CHANNEL_NUM] = {0};
    INT_32 index;
    UNUSED(thiz);
    if (pau4Channel == NULL) {
        ALOGE("NULL java array of getSupportChannelList");
        return -2;
    }
    index = s_adapter->getSupportChannelList(value);
    if (!index) {
        ALOGE("JNI, Enter getSupportChannelList succeed, channel list length is %d. value[1] is %d, value[2] is %d",
                (int)value[0], (int)value[1], (int)value[2]);
        buffer[0] = value[0];
        for (unsigned int i=1; i<= value[0]; i++) {
            buffer[i] = value[i];
        }
    }
    env->SetLongArrayRegion(pau4Channel, 0, value[0] + 1, buffer);
    return index;
}

static int com_mediatek_engineermode_wifi_EMWifi_doCTIATestSet(JNIEnv *env,
        jobject thiz, jlong u4Id, jlong u4Value) {
    INT_32 index;
    UNUSED(env, thiz);
    ALOGE("JNI Set ID is %" PRId64 ", value is %" PRId64, u4Id, u4Value);
    index = s_adapter->sw_cmd(1, u4Id, (UINT_32*)&u4Value);
    if (!index) {
        ALOGE("doCTIATestSet succeed, ID is %" PRId64, u4Id);
    }
    return index;
}

static int com_mediatek_engineermode_wifi_EMWifi_doCTIATestGet(JNIEnv *env,
        jobject thiz, jlong u4Id, jlongArray pu4Value) {
    INT_32 index;
    UINT_32 value = 0;
    jlong targVal = 0;
    UNUSED(thiz);
    if (pu4Value == NULL) {
        ALOGE("NULL java array of doCTIATestGet");
        return -2;
    }
    ALOGE("JNI Get ID is %" PRId64 ", value is %u", u4Id, value);
    index = s_adapter->sw_cmd(0, u4Id, &value);
    if (!index) {
        ALOGE("doCTIATestGet succeed, ID is %" PRId64 ", value is %u", u4Id, value);
    }
    targVal = value;
    env->SetLongArrayRegion(pu4Value, 0, 1, &targVal);
    return index;
}

static bool com_mediatek_engineermode_wifi_EMWifi_isWifiLogUiEnable(JNIEnv *, jobject,
        jint module) {
    return s_adapter->isWifiLogUiEnable(module) > 0;
}

static int com_mediatek_engineermode_wifi_EMWifi_getWifiLogLevel(JNIEnv *, jobject,
        jint module) {
    return s_adapter->getWifiLogLevel(module);
}

static bool com_mediatek_engineermode_wifi_EMWifi_setWifiLogLevel(JNIEnv *, jobject,
        jint module, jint level) {
    return s_adapter->setWifiLogLevel(module, level) == 0;
}

static jstring com_mediatek_engineermode_wifi_EMWifi_getFwManifestVersion(JNIEnv *env, jobject) {
    char manifest[256];
    if (s_adapter->getFwManifestVersion(manifest) != 0)
        return NULL;
    return env->NewStringUTF(manifest);
}

static bool com_mediatek_engineermode_wifi_EMWifi_isAntSwapSupport(JNIEnv *, jobject) {
    return s_adapter->isAntSwapSupport() > 0;
}

//method register to vm
static JNINativeMethod gMethods[] = {
    { "initial", "()I", (void*) com_mediatek_engineermode_wifi_EMWifi_initial },
    { "unInitial", "()I", (void*) com_mediatek_engineermode_wifi_EMWifi_unInitial },
    { "setTestMode", "()I", (void*) com_mediatek_engineermode_wifi_EMWifi_setTestMode },
    { "setNormalMode", "()I", (void*) com_mediatek_engineermode_wifi_EMWifi_setNormalMode },
    { "setStandBy", "()I", (void*) com_mediatek_engineermode_wifi_EMWifi_setStandBy },
    { "setEEPRomSize", "(J)I", (void*) com_mediatek_engineermode_wifi_EMWifi_setEEPRomSize },
    { "readTxPowerFromEEPromEx", "(JJ[JI)I", (void*) com_mediatek_engineermode_wifi_EMWifi_readTxPowerFromEEPromEx },
    { "setEEPromCKSUpdated", "()I", (void*) com_mediatek_engineermode_wifi_EMWifi_setEEPromCKSUpdated },
    { "getPacketRxStatus", "([JI)I", (void*) com_mediatek_engineermode_wifi_EMWifi_getPacketRxStatus },
    { "setChannel", "(J)I", (void*) com_mediatek_engineermode_wifi_EMWifi_setChannel },
    { "readEEPRom16", "(J[J)I", (void*) com_mediatek_engineermode_wifi_EMWifi_readEEPRom16 },
    { "writeEEPRom16", "(JJ)I", (void*) com_mediatek_engineermode_wifi_EMWifi_writeEEPRom16 },
    { "eepromReadByteStr", "(JJ[B)I", (void*) com_mediatek_engineermode_wifi_EMWifi_eepromReadByteStr },
    { "eepromWriteByteStr", "(JJLjava/lang/String;)I", (void*) com_mediatek_engineermode_wifi_EMWifi_eepromWriteByteStr },
    { "setATParam", "(JJ)I", (void*) com_mediatek_engineermode_wifi_EMWifi_setATParam },
    { "getATParam", "(J[J)I", (void*) com_mediatek_engineermode_wifi_EMWifi_getATParam },
    { "setPnpPower", "(J)I", (void*) com_mediatek_engineermode_wifi_EMWifi_setPnpPower },
    { "writeMCR32", "(JJ)I", (void*) com_mediatek_engineermode_wifi_EMWifi_writeMCR32 },
    { "readMCR32", "(J[J)I", (void*) com_mediatek_engineermode_wifi_EMWifi_readMCR32 },
    { "getSupportChannelList", "([J)I", (void*) com_mediatek_engineermode_wifi_EMWifi_getSupportChannelList },
    { "doCTIATestSet", "(JJ)I", (void*) com_mediatek_engineermode_wifi_EMWifi_doCTIATestSet },
    { "doCTIATestGet", "(J[J)I", (void*) com_mediatek_engineermode_wifi_EMWifi_doCTIATestGet },
    { "isWifiLogUiEnable", "(I)Z", (void*) com_mediatek_engineermode_wifi_EMWifi_isWifiLogUiEnable },
    { "getWifiLogLevel", "(I)I", (void*) com_mediatek_engineermode_wifi_EMWifi_getWifiLogLevel },
    { "setWifiLogLevel", "(II)Z", (void*) com_mediatek_engineermode_wifi_EMWifi_setWifiLogLevel },
    { "getFwManifestVersion", "()Ljava/lang/String;",
         (void*) com_mediatek_engineermode_wifi_EMWifi_getFwManifestVersion },
    { "isAntSwapSupport", "()Z", (void*) com_mediatek_engineermode_wifi_EMWifi_isAntSwapSupport },
};


static const char* const kClassPathName = "com/mediatek/engineermode/wifi/EMWifi";

int register_com_mediatek_engineermode_wifi_EMWifi(JNIEnv *env) {
     return AndroidRuntime::registerNativeMethods(env, kClassPathName, gMethods,
         NELEM(gMethods));
}

jint JNI_OnLoad(JavaVM *vm, void *reserved) {
    JNIEnv *env = NULL;
    jint result = -1;

    UNUSED(reserved);

    if (vm->GetEnv((void **) &env, JNI_VERSION_1_4) != JNI_OK) {
        ALOGE("ERROR: GetEnv failed");
        goto bail;
    }

    assert(env != NULL);

    if (register_com_mediatek_engineermode_wifi_EMWifi(env) < 0) {
        ALOGE("ERROR: Wi-Fi native for engineermode registration failed\n");
        goto bail;
    }

    result = JNI_VERSION_1_4;

    bail:
        return result;
}
