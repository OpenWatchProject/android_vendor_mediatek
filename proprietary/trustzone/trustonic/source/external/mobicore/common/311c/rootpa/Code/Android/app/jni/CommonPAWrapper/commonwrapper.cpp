/*
 * Copyright (c) 2013 TRUSTONIC LIMITED
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the TRUSTONIC LIMITED nor the names of its
 *    contributors may be used to endorse or promote products derived from
 *    this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
 * TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
#include <string.h>

#include "com_gd_mobicore_pa_jni_CommonPAWrapper.h"
#include "CmpCommands.h"
#include "CmpResponses.h"
#include "JniHelpers.h"

#include "rootpaErrors.h"
#include "logging.h"
#include "provisioningagent.h"

#define CERT_PATH "/system/etc/security/cacerts"
#define HARDCODED_STORAGEPATH "/data/data/com.gd.mobicore.pa"

JavaVM* jvmP_ = NULL;
const jint VERSION=JNI_VERSION_1_2;

/* Original params list : (JavaVM* jvm, void* reserved)*/
JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM* jvm, void*)
{
// remember JVM pointer:
	jvmP_ = jvm;
    LOGD("JNI_OnLoad jvmP_ set %ld\n", (long int) jvmP_);
	return VERSION;
}

JNIEXPORT jint JNICALL Java_com_gd_mobicore_pa_jni_CommonPAWrapper_openSession(JNIEnv *, jobject)
{
    return (jint) openSessionToCmtl();
}

JNIEXPORT void JNICALL Java_com_gd_mobicore_pa_jni_CommonPAWrapper_closeSession(JNIEnv *, jobject)
{
    closeSessionToCmtl();
}

/* Original params list : (JNIEnv* env, jobject, jint uid, jobject inCommands, jobject outResults)*/
JNIEXPORT jint JNICALL Java_com_gd_mobicore_pa_jni_CommonPAWrapper_executeCmpCommands
  (JNIEnv* env, jobject, jint, jobject inCommands, jobject outResults)
{
    LOGD(">>Java_com_gd_mobicore_pa_jni_CommonPAWrapper_executeCmpCommands\n");
    int ret=ROOTPA_OK;
    uint32_t internalError=0;

    CmpCommands inCmd(env, inCommands);
    int numberOfCommands=inCmd.numberOfElements();

    if(0==numberOfCommands){
        LOGD("No commands received, returning ROOTPA_COMMAND_NOT_SUPPORTED\n");
        return ROOTPA_COMMAND_NOT_SUPPORTED;
    }

    CmpMessage* commands = new CmpMessage[numberOfCommands];
    if(NULL==commands) return ROOTPA_ERROR_OUT_OF_MEMORY;
    memset(commands, 0, numberOfCommands*sizeof(CmpMessage));

    CmpMessage* responses = new CmpMessage[numberOfCommands];
    if(NULL==responses)
    {
        delete [] commands;
        return ROOTPA_ERROR_OUT_OF_MEMORY;
    }
    memset(responses, 0, numberOfCommands*sizeof(CmpMessage));

    if(inCmd.getCommands(commands)==false)
    {
        LOGE("getting commands on C side of the wrapper failed\n");
        ret=ROOTPA_ERROR_INTERNAL;
    }
    else
    {
        ret=executeCmpCommands(numberOfCommands, commands, responses, &internalError);
        CmpResponses outRsp(env, outResults);
        if(ret!=ROOTPA_OK)
        {
            LOGE("call to executeCmpCommands failed %d %d\n", ret, internalError);
            (void) outRsp.update(responses, numberOfCommands); // don't overwrite the return code but still try to copy the results
        }
        else
        {
            ret=outRsp.update(responses, numberOfCommands);
        }
    }

// cleanup

    for(int i=0; i<numberOfCommands; i++)
    {
        delete [] commands[i].contentP;
        free(responses[i].contentP); // this is reserved with malloc
    }
    delete [] commands;
    delete [] responses;
    LOGD("<<Java_com_gd_mobicore_pa_jni_CommonPAWrapper_executeCmpCommands %d\n", ret);

    return ret;
}

#define VERSION_FIELD_TAG "TAG"
#define VERSION_FIELD_TAG1ALL "TAG1ALL"
#define VERSION_FIELD_MCI "MCI"
#define VERSION_FIELD_SO "SO"
#define VERSION_FIELD_MCLF "MCLF"
#define VERSION_FIELD_CONT "CONT"
#define VERSION_FIELD_MCCONF "MCCONF"
#define VERSION_FIELD_TLAPI "TLAPI"
#define VERSION_FIELD_DRAPI "DRAPI"
#define VERSION_FIELD_CMP "CMP"


JNIEXPORT jint JNICALL Java_com_gd_mobicore_pa_jni_CommonPAWrapper_getVersion
  (JNIEnv* env, jobject, jbyteArray productId, jobject keys, jobject values)
{
    LOGD(">>Java_com_gd_mobicore_pa_jni_CommonPAWrapper_getVersion %x %x %x\n", productId, keys, values);
    int ret=ROOTPA_OK;
    int tag=0;
    mcVersionInfo_t version;

    ret=getVersion(&tag, &version);
    if(ROOTPA_OK == ret)
    {
        JniHelpers jniHelp(env, &keys, &values, &productId);

        ret=jniHelp.setVersion((char*) VERSION_FIELD_TAG, tag);
        if(ROOTPA_OK == ret)
        {
            ret=jniHelp.setProductId((char*) version.productId);
            if(ret != ROOTPA_OK) return ret;
            ret=jniHelp.setVersion((char*) VERSION_FIELD_MCI, version.versionMci);
            if(ret != ROOTPA_OK) return ret;
            ret=jniHelp.setVersion((char*) VERSION_FIELD_SO, version.versionSo);
            if(ret != ROOTPA_OK) return ret;
            ret=jniHelp.setVersion((char*) VERSION_FIELD_MCLF, version.versionMclf);
            if(ret != ROOTPA_OK) return ret;
            ret=jniHelp.setVersion((char*) VERSION_FIELD_CONT, version.versionContainer);
            if(ret != ROOTPA_OK) return ret;
            ret=jniHelp.setVersion((char*) VERSION_FIELD_MCCONF, version.versionMcConfig);
            if(ret != ROOTPA_OK) return ret;
            ret=jniHelp.setVersion((char*) VERSION_FIELD_TLAPI, version.versionTlApi);
            if(ret != ROOTPA_OK) return ret;
            ret=jniHelp.setVersion((char*) VERSION_FIELD_DRAPI, version.versionDrApi);
            if(ret != ROOTPA_OK) return ret;
            ret=jniHelp.setVersion((char*) VERSION_FIELD_CMP, version.versionCmp);
            if(tag!=2)
            {
                LOGE("Java_com_gd_mobicore_pa_jni_CommonPAWrapper_getVersion unknown tag %d, version information may be wrong\n", tag);
                ret=ROOTPA_ERROR_INTERNAL;
            }
        }
    }
    LOGD("<<Java_com_gd_mobicore_pa_jni_CommonPAWrapper_getVersion %x %x %x\n", productId, keys, values);
    return ret;
}

JNIEXPORT jint JNICALL Java_com_gd_mobicore_pa_jni_CommonPAWrapper_getSuid
  (JNIEnv* env, jobject, jbyteArray suid)
{
    int ret=ROOTPA_OK;
    mcSuid_t mySuid;

    ret=getSuid(&mySuid);
    if(ROOTPA_OK == ret)
    {
        JniHelpers jniHelp(env);
        ret=jniHelp.setByteArray(&suid, (uint8_t*)&mySuid, sizeof(mySuid));
    }
    return ret;
}

JNIEXPORT jint JNICALL Java_com_gd_mobicore_pa_jni_CommonPAWrapper_isRootContainerRegistered
  (JNIEnv* envP, jobject, jbooleanArray result)
{
    int ret=ROOTPA_OK;
    bool isRegistered;

    ret=isRootContainerRegistered(&isRegistered);
    if(ROOTPA_OK == ret)
    {
        JniHelpers jniHelp(envP);
        ret=jniHelp.setBooleanToArray(&result, isRegistered);
    }
    return ret;
}

JNIEXPORT jint JNICALL Java_com_gd_mobicore_pa_jni_CommonPAWrapper_isSpContainerRegistered
  (JNIEnv* envP, jobject, jint spid, jbooleanArray result)
{
    int ret=ROOTPA_OK;
    bool isRegistered;

    ret=isSpContainerRegistered((mcSpid_t) spid, &isRegistered);
    if(ROOTPA_OK == ret)
    {
        JniHelpers jniHelp(envP);
        ret=jniHelp.setBooleanToArray(&result, isRegistered);
    }
    return ret;
}

JNIEXPORT jint JNICALL Java_com_gd_mobicore_pa_jni_CommonPAWrapper_getSPContainerState
  (JNIEnv* envP, jobject, jint spid, jintArray stateArray)
{
    LOGD(">>Java_com_gd_mobicore_pa_jni_CommonPAWrapper_getSpContainerState\n");
    int ret=ROOTPA_OK;
    int state;

    ret=getSpContainerState((mcSpid_t) spid, (mcContainerState_t*) &state);
    if(ROOTPA_OK == ret)
    {
        JniHelpers jniHelp(envP);
        ret=jniHelp.setIntToArray(&stateArray, 0, state);
    }
    LOGD("<<Java_com_gd_mobicore_pa_jni_CommonPAWrapper_getSpContainerState\n");
    return ret;
}

const int CONTAINER_STATE_IDX=0;
const int NUMBER_OF_TLTS_IDX=1;
//const int NUMBER_OF_ELEMENTS=2;

JNIEXPORT jint JNICALL Java_com_gd_mobicore_pa_jni_CommonPAWrapper_getSPContainerStructure
  (JNIEnv* envP, jobject, jint spid, jintArray ints, jobjectArray uuidArray, jintArray trustletStates)
{
    LOGD(">>Java_com_gd_mobicore_pa_jni_CommonPAWrapper_getSPContainerStructure\n");
    int ret=ROOTPA_OK;

    SpContainerStructure spContainerStructure;

    ret=getSpContainerStructure((mcSpid_t) spid, &spContainerStructure);
    if(ROOTPA_OK == ret)
    {
        JniHelpers jniHelp(envP);

        ret=jniHelp.setIntToArray(&ints, CONTAINER_STATE_IDX, spContainerStructure.state);
        if(ROOTPA_OK==ret)
        {
            ret=jniHelp.setIntToArray(&ints, NUMBER_OF_TLTS_IDX, spContainerStructure.nbrOfTlts);
            if(ROOTPA_OK==ret)
            {
                for(int i=0; i<spContainerStructure.nbrOfTlts;i++)
                {
                    LOGD("TLT %d/%d %.2x%.2x%.2x%.2x-%.2x%.2x-%.2x%.2x-%.2x%.2x-%.2x%.2x%.2x%.2x%.2x%.2x",i, spContainerStructure.nbrOfTlts, spContainerStructure.tltContainers[i].uuid.value[0],
                                                                                                             spContainerStructure.tltContainers[i].uuid.value[1],
                                                                                                             spContainerStructure.tltContainers[i].uuid.value[2],
                                                                                                             spContainerStructure.tltContainers[i].uuid.value[3],
                                                                                                             spContainerStructure.tltContainers[i].uuid.value[4],
                                                                                                             spContainerStructure.tltContainers[i].uuid.value[5],
                                                                                                             spContainerStructure.tltContainers[i].uuid.value[6],
                                                                                                             spContainerStructure.tltContainers[i].uuid.value[7],
                                                                                                             spContainerStructure.tltContainers[i].uuid.value[8],
                                                                                                             spContainerStructure.tltContainers[i].uuid.value[9],
                                                                                                             spContainerStructure.tltContainers[i].uuid.value[10],
                                                                                                             spContainerStructure.tltContainers[i].uuid.value[11],
                                                                                                             spContainerStructure.tltContainers[i].uuid.value[12],
                                                                                                             spContainerStructure.tltContainers[i].uuid.value[13],
                                                                                                             spContainerStructure.tltContainers[i].uuid.value[14],
                                                                                                             spContainerStructure.tltContainers[i].uuid.value[15]);

                    ret=jniHelp.setIntToArray(&trustletStates, i, spContainerStructure.tltContainers[i].state);
                    jbyteArray uuid = jniHelp.byteArrayToJByteArray(spContainerStructure.tltContainers[i].uuid.value, UUID_LENGTH);
                    envP->SetObjectArrayElement(uuidArray, i, (jobject) uuid);
                    envP->DeleteLocalRef(uuid);
                }
            }
            else
            {
                LOGE("..._jni_CommonPAWrapper_getSPContainerStructure copy number of trustlets failed\n");
            }
        }
        else
        {
            LOGE("..._jni_CommonPAWrapper_getSPContainerStructure copy container state failed\n");
        }
    }
    else
    {
        LOGE("..._jni_CommonPAWrapper_getSPContainerStructure getSpContainerStructure failed\n");
    }


    LOGD("<<Java_com_gd_mobicore_pa_jni_CommonPAWrapper_getSPContainerStructure %d\n", ret);
    return ret;
}

jmethodID provisioningStateCallback_=NULL;
jmethodID getSystemInfoCallback_=NULL;
jmethodID trustletInstallCallback_=NULL;
jobject obj_=NULL;

void stateUpdateCallback(ProvisioningState state, rootpaerror_t error, tltInfo_t* tltInfoP)
{
    LOGD(">>stateUpdateCallback %d %d\n", state, error);

    JNIEnv* envP=NULL;

    // it is enough to call this only once for each thread but since this is
    // the best place to call it we call it every time

    jint res = jvmP_->AttachCurrentThread(&envP, NULL);

    if(NULL==obj_ ||NULL==provisioningStateCallback_ || NULL== envP || res != JNI_OK)
    {
        LOGE("obj=%ld, provisioningStateCallback==%ld, envP==%ld res==%d", (long int) obj_, (long int) provisioningStateCallback_, (long int) envP, res);
    }
    else if(PROVISIONING_STATE_INSTALL_TRUSTLET == state)
    {
        if(tltInfoP)
        {
            LOGD("installing trustled");
            JniHelpers jniHelp(envP);
            jbyteArray trustlet=jniHelp.byteArrayToJByteArray(tltInfoP->trustletP, tltInfoP->trustletSize);
            envP->CallVoidMethod(obj_, trustletInstallCallback_, trustlet);
        }
        else
        {
	        envP->CallVoidMethod(obj_, provisioningStateCallback_, ERROR_STATE, ROOTPA_ERROR_INTERNAL);
        }
    }
    else
    {
        envP->CallVoidMethod(obj_, provisioningStateCallback_, state, error);
    }

    if( obj_!=NULL && (PROVISIONING_STATE_THREAD_EXITING == state) )
    {
        LOGD("deleting global reference to obj_ (envP %ld)", envP);
        if(envP!=NULL) envP->DeleteGlobalRef(obj_);
        obj_=NULL;
    }

    // doing this in every round in order to make sure what is attached will be detached and that
    // envP is correctly updated at every round (it seems to work also inside the above if statement,
    // but calling AttachCurrentThread to already attached thread is is supposed to be no-op. It seems
    // to update the envP though.)
    // If the thread is not detached there will be a crash when the thread exists
    jvmP_->DetachCurrentThread();

    LOGD("<<stateUpdateCallback\n");
}

void storeCallbackMethodIds(JNIEnv* envP)
{
    LOGD(">>storeCallbackMethodIds\n");

    jclass cls = envP->GetObjectClass(obj_);
    if(NULL==cls)
    {
        LOGE("storeCallbackMethodIds cls(obj_)==NULL");
        return;
    }
    provisioningStateCallback_ = envP->GetMethodID(cls, "provisioningStateCallback","(II)V");
    if(NULL==provisioningStateCallback_)
    {
        LOGE("storeCallbackMethodIds provisioningStateCallback_==NULL");
    }


    getSystemInfoCallback_ = envP->GetMethodID(cls, "getSystemInfo","()[Ljava/lang/String;");
    if(NULL==getSystemInfoCallback_)
    {
        LOGE("storeCallbackMethodIds getSystemInfoCallback_==NULL");
    }

    trustletInstallCallback_ = envP->GetMethodID(cls, "trustletInstallCallback","([B)V");
    if(NULL==trustletInstallCallback_)
    {
        LOGE("storeCallbackMethodIds trustletInstallCallback_==NULL");
    }

    if(cls!=NULL)
    {
        envP->DeleteLocalRef(cls);
    }

    LOGD("<<storeCallbackMethodIds\n");
}

/*
This function has to be called before any communication with SE is done (or actually,
before any xml parsing is done.
*/
void setFilesPath(JNIEnv* envP, jobject obj)
{
    LOGD(">>setFilesPath\n");

    jclass cls = envP->GetObjectClass(obj);
    if(NULL==cls)
    {
        LOGE("setFilesPath cls(obj)==NULL");
        return;
    }
    jmethodID getFilesDirPath = envP->GetMethodID(cls, "getFilesDirPath","()Ljava/lang/String;");
    if(NULL==getFilesDirPath)
    {
        setPaths(HARDCODED_STORAGEPATH, CERT_PATH);
        LOGE("<<setFilesPath getFilesDirPath==NULL, used hardcoded paths");
        return;
    }

    jobject jpath = envP->CallObjectMethod(obj, getFilesDirPath);
    if(jpath!=NULL)
    {
        const char* pathP = envP->GetStringUTFChars((jstring)jpath, NULL);
        setPaths(pathP, CERT_PATH);
        if(NULL == pathP)
        {
            LOGE("setFilesPath pathP==NULL");
        }

//        LOGD("path: %s\n", pathP);
        envP->ReleaseStringUTFChars((jstring)jpath, pathP);
    }
    else
    {
        LOGE("setFilesPath jpath==NULL, using hardcoded paths");
        setPaths(HARDCODED_STORAGEPATH, CERT_PATH);
    }

    if(cls!=NULL)
    {
        envP->DeleteLocalRef(cls);
    }

    LOGD("<<setFilesPath\n");
}

const int IMEI_ESN_INDEX=com_gd_mobicore_pa_jni_CommonPAWrapper_IMEI_ESN_INDEX;
const int MNO_INDEX=com_gd_mobicore_pa_jni_CommonPAWrapper_MNO_INDEX;
const int BRAND_INDEX=com_gd_mobicore_pa_jni_CommonPAWrapper_BRAND_INDEX;
const int MANUFACTURER_INDEX=com_gd_mobicore_pa_jni_CommonPAWrapper_MANUFACTURER_INDEX;
const int HARDWARE_INDEX=com_gd_mobicore_pa_jni_CommonPAWrapper_HARDWARE_INDEX;
const int MODEL_INDEX=com_gd_mobicore_pa_jni_CommonPAWrapper_MODEL_INDEX;
const int VERSION_INDEX=com_gd_mobicore_pa_jni_CommonPAWrapper_VERSION_INDEX;

void copyElement(JNIEnv* envP, char** target, jstring source)
{
    if(source != NULL)
    {
        const char* tmp=envP->GetStringUTFChars(source, NULL);
        *target=(char*)malloc(strlen(tmp)+1);
        if(*target!=NULL)
        {
            strcpy(*target, tmp);
        }
        envP->ReleaseStringUTFChars(source, tmp);
    }
    else
    {
        *target=NULL;
    }
}

rootpaerror_t getSystemInfoCallback(osInfo_t* osSpecificInfoP)
{
    LOGD(">>getSystemInfoCallback\n");
    rootpaerror_t ret=ROOTPA_OK;

    if(NULL==osSpecificInfoP) return ROOTPA_ERROR_INTERNAL;

    memset(osSpecificInfoP, 0, sizeof(osInfo_t));

    JNIEnv* envP=NULL;
    jint res = jvmP_->AttachCurrentThread(&envP, NULL);


    if(NULL==obj_ ||NULL==getSystemInfoCallback_ || NULL== envP || res != JNI_OK)
    {
        ret=ROOTPA_ERROR_INTERNAL;
        LOGE("obj=%ld, getSystemInfoCallback_==%ld, envP==%ld res==%d", (long int) obj_, (long int) getSystemInfoCallback_, (long int) envP, res);
    }
    else
    {
        jobjectArray systemInfo = (jobjectArray) envP->CallObjectMethod(obj_, getSystemInfoCallback_);
        if(systemInfo!=NULL)
        {
            jstring imeiEsn=(jstring) envP->GetObjectArrayElement(systemInfo, IMEI_ESN_INDEX);
            jstring mno=(jstring) envP->GetObjectArrayElement(systemInfo, MNO_INDEX);
            jstring brand=(jstring) envP->GetObjectArrayElement(systemInfo, BRAND_INDEX);
            jstring manufacturer=(jstring) envP->GetObjectArrayElement(systemInfo, MANUFACTURER_INDEX);
            jstring hw=(jstring) envP->GetObjectArrayElement(systemInfo, HARDWARE_INDEX);
            jstring model=(jstring) envP->GetObjectArrayElement(systemInfo, MODEL_INDEX);
            jstring version=(jstring) envP->GetObjectArrayElement(systemInfo, VERSION_INDEX);

            copyElement(envP, &osSpecificInfoP->imeiEsnP, imeiEsn);
            copyElement(envP, &osSpecificInfoP->mnoP, mno);
            copyElement(envP, &osSpecificInfoP->brandP, brand);
            copyElement(envP, &osSpecificInfoP->manufacturerP, manufacturer);
            copyElement(envP, &osSpecificInfoP->hardwareP, hw);
            copyElement(envP, &osSpecificInfoP->modelP, model);
            copyElement(envP, &osSpecificInfoP->versionP, version);

            envP->DeleteLocalRef(systemInfo);
            if(imeiEsn!=NULL) envP->DeleteLocalRef(imeiEsn);
            if(mno!=NULL) envP->DeleteLocalRef(mno);
            if(brand!=NULL) envP->DeleteLocalRef(brand);
            if(manufacturer!=NULL) envP->DeleteLocalRef(manufacturer);
            if(hw!=NULL) envP->DeleteLocalRef(hw);
            if(model!=NULL) envP->DeleteLocalRef(model);
            if(version!=NULL) envP->DeleteLocalRef(version);
        }
    }

    // doing this in every round in order to make sure what is attached will be detached and that
    // envP is correctly updated at every round (it seems to work also inside the above if, but
    // calling AttachCurrentThread to already attched thread is is supposed to be no-op. It seems to
    // update the envP though.)
    // If the thread is not detached there will be a crash when the thread exists
    jvmP_->DetachCurrentThread();

    LOGD("<<getSystemInfoCallback %d\n", ret);
    return ret;
}

/* Original params list : (JNIEnv* envP, jobject obj, jint uid, jint spid, jbyteArray seAddress)*/
JNIEXPORT jint JNICALL Java_com_gd_mobicore_pa_jni_CommonPAWrapper_doProvisioning
  (JNIEnv* envP, jobject obj, jint, jint spid, jbyteArray seAddress)
{
    LOGD(">>Java_com_gd_mobicore_pa_jni_CommonPAWrapper_doProvisioning %ld %ld\n", (long int) stateUpdateCallback, (long int) getSystemInfoCallback);
    setFilesPath(envP, obj);
    int ret=ROOTPA_OK;

    if(seAddress)
    {
        uint32_t length=0;
        JniHelpers jniHelp(envP);
        char*  addrP=(char*)jniHelp.jByteArrayToCByteArray(seAddress, &length);
        ret=setSeAddress(addrP, length);
        delete[] addrP;
    }

    if(ROOTPA_OK==ret)
    {
        obj_= envP->NewGlobalRef(obj);
        storeCallbackMethodIds(envP);
        ret=doProvisioning(spid, stateUpdateCallback, getSystemInfoCallback);
    }
    LOGD("<<Java_com_gd_mobicore_pa_jni_CommonPAWrapper_doProvisioning %d\n", ret);
    return ret;
}


JNIEXPORT jint JNICALL Java_com_gd_mobicore_pa_jni_CommonPAWrapper_installTrustlet
(JNIEnv* envP, jobject obj,
jint spid,
jbyteArray uuid,
jint requestDataType,
jbyteArray tltOrKeyData,
jint minTltVersion,
jbyteArray tltPukHash,
jint memoryType,
jint numberOfInstances,
jint flags,
jbyteArray seAddress)
{
    LOGD(">>Java_com_gd_mobicore_pa_jni_CommonPAWrapper_installTrustlet %ld %ld\n", (long int) stateUpdateCallback, (long int) getSystemInfoCallback);
    setFilesPath(envP, obj);
    int ret=ROOTPA_OK;
    JniHelpers jniHelp(envP);

    if(seAddress)
    {
        uint32_t length=0;
        char*  addrP=(char*)jniHelp.jByteArrayToCByteArray(seAddress, &length);
        ret=setSeAddress(addrP, length);
        delete[] addrP;
    }

    if(ROOTPA_OK==ret)
    {
        obj_= envP->NewGlobalRef(obj);
        storeCallbackMethodIds(envP);
        trustletInstallationData_t tltData;
        tltData.dataP=(uint8_t*) jniHelp.jByteArrayToCByteArray(tltOrKeyData, &tltData.dataLength);
        tltData.dataType=(TltInstallationRequestDataType) requestDataType;
        tltData.minTltVersion=minTltVersion;
        tltData.tltPukHashP=(uint8_t*) jniHelp.jByteArrayToCByteArray(tltPukHash, &tltData.tltPukHashLength);
        tltData.memoryType=memoryType;
        tltData.numberOfInstances=numberOfInstances;
        tltData.flags=flags;

        uint32_t uuidLength=0;
        uint8_t* uuidP=(uint8_t*) jniHelp.jByteArrayToCByteArray(uuid, &uuidLength);
        if(UUID_LENGTH != uuidLength){
            LOGD("<<Java_com_gd_mobicore_pa_jni_CommonPAWrapper_installTrustlet, wrong uuidLength %d, not installing\n", uuidLength);
            delete[] tltData.dataP;
            delete[] tltData.tltPukHashP;
            free(uuidP);
            return ROOTPA_ERROR_ILLEGAL_ARGUMENT;
        }
        memcpy(tltData.uuid.value, uuidP, UUID_LENGTH);
        free(uuidP);
        ret=installTrustlet(spid, stateUpdateCallback, getSystemInfoCallback, &tltData);
    }
    LOGD("<<Java_com_gd_mobicore_pa_jni_CommonPAWrapper_installTrustlet %d\n", ret);
    return ret;
}



JNIEXPORT jint JNICALL Java_com_gd_mobicore_pa_jni_CommonPAWrapper_unregisterRootContainer(JNIEnv* envP, jobject obj,  jbyteArray seAddress)
{
    LOGD(">>Java_com_gd_mobicore_pa_jni_CommonPAWrapper_unregisterRootContainer\n");
    setFilesPath(envP, obj);
    int ret=ROOTPA_OK;
    JniHelpers jniHelp(envP);

    if(seAddress)
    {
        uint32_t length=0;
        char*  addrP=(char*)jniHelp.jByteArrayToCByteArray(seAddress, &length);
        ret=setSeAddress(addrP, length);
        delete[] addrP;
    }

    if(ROOTPA_OK==ret)
    {
        obj_= envP->NewGlobalRef(obj);
        storeCallbackMethodIds(envP);
        ret=unregisterRootContainer(stateUpdateCallback, getSystemInfoCallback);
    }

    LOGD("<<Java_com_gd_mobicore_pa_jni_CommonPAWrapper_unregisterRootContainer\n");

    return ret;
}

char* addTrailingZero(uint8_t* vP, uint32_t length)
{
    char* newVP = new char[length+1];
    if(NULL!=newVP)
    {
        memcpy(newVP, vP, length);
        newVP[length]=0;
    }
    delete [] vP;
    return newVP;
}

/* Original params list : (JNIEnv* envP, jobject obj, jbyteArray variable_name, jbyteArray value)*/
JNIEXPORT void JNICALL Java_com_gd_mobicore_pa_jni_CommonPAWrapper_setEnvironmentVariable(JNIEnv* envP, jobject, jbyteArray variable_name, jbyteArray value)
{
    LOGD(">>Java_com_gd_mobicore_pa_jni_CommonPAWrapper_setEnvironmentVariable");
    JniHelpers jniHelp(envP);
    uint32_t length=0;
    char* envVarP=NULL;
    char* envValP=NULL;
    uint8_t*  vP=jniHelp.jByteArrayToCByteArray(variable_name, &length);

    if(NULL==vP)
    {
        LOGE("Java_com_gd_mobicore_pa_jni_CommonPAWrapper_setEnvironmentVariable, FAILURE: can not get variable\n");
        return;
    }

    envVarP = addTrailingZero(vP, length);
    if(value!=NULL)
    {
        vP=jniHelp.jByteArrayToCByteArray(value, &length);
        if(NULL!=vP)
        {
            envValP = addTrailingZero(vP, length);
            if(envVarP && envValP)
            {
                LOGD("setting environment variable, %s %s", envVarP, envValP);
                if(setenv(envVarP, envValP, 1)!=0)
                {
                    LOGE("Java_com_gd_mobicore_pa_jni_CommonPAWrapper_setEnvironmentVariable, setenv %s FAILURE\n", envVarP);
                }
            }
        }
        else
        {
            LOGE("Java_com_gd_mobicore_pa_jni_CommonPAWrapper_setEnvironmentVariable, FAILURE: can not get value\n");
        }
    }
    else
    {
        LOGD("unsetting environment variable, %s", envVarP);
        if(unsetenv(envVarP)!=0)
        {
            LOGE("Java_com_gd_mobicore_pa_jni_CommonPAWrapper_setEnvironmentVariable, unsetenv %s FAILURE\n", envVarP);
        }
    }

    delete[] envVarP;
    delete[] envValP;
    LOGD("<<Java_com_gd_mobicore_pa_jni_CommonPAWrapper_setEnvironmentVariable");
}

/* Original params list : (JNIEnv* envP, jobject obj, jint spid, jbyteArray uuid, jbyteArray taBin)*/
JNIEXPORT jint JNICALL Java_com_gd_mobicore_pa_jni_CommonPAWrapper_storeTA(JNIEnv* envP, jobject, jint spid, jbyteArray uuid, jbyteArray taBin)
{
    LOGD(">>Java_com_gd_mobicore_pa_jni_CommonPAWrapper_storeTA");
    int ret=ROOTPA_OK;
    JniHelpers jniHelp(envP);

    uint32_t uuidLength=0;
    uint8_t* uuidP=(uint8_t*) jniHelp.jByteArrayToCByteArray(uuid, &uuidLength);

    if(UUID_LENGTH != uuidLength){
        LOGD("<<Java_com_gd_mobicore_pa_jni_CommonPAWrapper_storeTA, wrong uuidLength %d, not installing\n", uuidLength);
        free(uuidP);
        return ROOTPA_ERROR_ILLEGAL_ARGUMENT;
    }
    mcUuid_t mcUuid;
    memcpy(mcUuid.value, uuidP, UUID_LENGTH);
    free(uuidP);

    uint32_t taBinLength=0;
    uint8_t* taBinP=(uint8_t*) jniHelp.jByteArrayToCByteArray(taBin, &taBinLength);
    if(0==taBinLength){
        LOGD("<<Java_com_gd_mobicore_pa_jni_CommonPAWrapper_storeTA, no taBin\n", taBinLength);
        free(taBinP);
        return ROOTPA_ERROR_ILLEGAL_ARGUMENT;
    }

    ret=storeTA((mcSpid_t)spid, &mcUuid, taBinP, taBinLength);
    free(taBinP);

    LOGD("<<Java_com_gd_mobicore_pa_jni_CommonPAWrapper_storeTA %d", ret);
    return ret;
}

