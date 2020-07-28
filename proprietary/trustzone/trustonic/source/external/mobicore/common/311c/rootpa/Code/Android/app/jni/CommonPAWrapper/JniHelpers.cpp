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
#include "JniHelpers.h"
#include "rootpaErrors.h"

JniHelpers::JniHelpers(JNIEnv* envP):broken_(false),
                                     envP_(envP),
                                     keysP_(NULL),
                                     valuesP_(NULL),
                                     productIdP_(NULL),
                                     listCls_(NULL),
                                     intCls_(NULL),
                                     stringCls_(NULL),
                                     stringConstructur_(NULL),
                                     intConstructor_(NULL),
                                     listAdd_(NULL)
{}

JniHelpers::~JniHelpers()
{
    if(listCls_!=NULL)
    {
        envP_->DeleteLocalRef(listCls_);
    }

    if(intCls_!=NULL)
    {
        envP_->DeleteLocalRef(intCls_);
    }

    if(stringCls_!=NULL)
    {
        envP_->DeleteLocalRef(stringCls_);
    }
}


JniHelpers::JniHelpers(JNIEnv* envP,jobject* keysP, jobject* valuesP, jbyteArray* productIdP):broken_(false),
                                                                                              envP_(envP),
                                                                                              keysP_(keysP),
                                                                                              valuesP_(valuesP),
                                                                                              productIdP_(productIdP),
                                                                                              listCls_(NULL),
                                                                                              intCls_(NULL),
                                                                                              stringCls_(NULL),
                                                                                              stringConstructur_(NULL),
                                                                                              intConstructor_(NULL),
                                                                                              listAdd_(NULL)
{
    listCls_=envP_->FindClass("java/util/List");
    if(NULL == listCls_)
    {
        LOGE("JniHelpers::JniHelpers no listCls_");
        broken_=true;
        return;
    }

    listAdd_=envP_->GetMethodID(listCls_, "add", "(Ljava/lang/Object;)Z");
    if(NULL == listAdd_)
    {
        LOGE("JniHelpers::JniHelpers no listAdd_");
        broken_=true;
        return;
    }

    intCls_=envP_->FindClass("java/lang/Integer");
    if(NULL == intCls_)
    {
        LOGE("JniHelpers::JniHelpers no intCls_");
        broken_=true;
        return;
    }

    intConstructor_=envP_->GetMethodID(intCls_, "<init>", "(I)V");
    if(NULL == intConstructor_)
    {
        LOGE("JniHelpers::JniHelpers no intConstructor_");
        broken_=true;
        return;
    }

    stringCls_=envP_->FindClass("java/lang/String");
    if(NULL == stringCls_)
    {
        LOGE("JniHelpers::JniHelpers no stringCls_");
        broken_=true;
        return;
    }

    stringConstructur_=envP_->GetMethodID(stringCls_, "<init>", "([B)V");
    if(NULL == stringConstructur_)
    {
        LOGE("JniHelpers::JniHelpers no stringConstructur_");
        broken_=true;
        return;
    }
}

int JniHelpers::setVersion(char* fieldName, int version)
{
    if(broken_) return ROOTPA_ERROR_INTERNAL;

    jbyteArray fName=byteArrayToJByteArray((uint8_t*)fieldName, strlen(fieldName));
    if(NULL == fName)
    {
        LOGE("JniHelpers::setVersion no fName");
        broken_=true;
        return ROOTPA_ERROR_INTERNAL;
    }

    jobject newStringObject = envP_->NewObject(stringCls_, stringConstructur_, fName);
    if(NULL == newStringObject)
    {
        LOGE("JniHelpers::setVersion no newStringObject");
        broken_=true;
        envP_->DeleteLocalRef(fName);
        return ROOTPA_ERROR_INTERNAL;
    }
    envP_->DeleteLocalRef(fName);

    if(envP_->CallBooleanMethod(*keysP_, listAdd_, newStringObject)==JNI_FALSE)
    {
        LOGE("JniHelpers::setVersion can not add key");
        broken_=true;
        envP_->DeleteLocalRef(newStringObject);
        return ROOTPA_ERROR_INTERNAL;
    }
    envP_->DeleteLocalRef(newStringObject);

    jobject newIntObject = envP_->NewObject(intCls_, intConstructor_, version);
    if(NULL == newIntObject)
    {
        LOGE("JniHelpers::setVersion no newIntObject");
        broken_=true;
        return ROOTPA_ERROR_INTERNAL;
    }

    if(envP_->CallBooleanMethod(*valuesP_, listAdd_, newIntObject)==JNI_FALSE)
    {
        LOGE("JniHelpers::setVersion can not add value");
        broken_=true;
        envP_->DeleteLocalRef(newIntObject);
        return ROOTPA_ERROR_INTERNAL;
    }
    envP_->DeleteLocalRef(newIntObject);

    return ROOTPA_OK;
}

int JniHelpers::setProductId(char* productId)
{
    return setByteArray(productIdP_,(uint8_t*)productId, strlen(productId));
}

int JniHelpers::setByteArray(jbyteArray* targetArrayP, uint8_t* sourceArrayP , uint32_t length)
{
    if(broken_) return ROOTPA_ERROR_INTERNAL;
    if(NULL==targetArrayP || NULL == sourceArrayP || 0 == length)  return ROOTPA_ERROR_ILLEGAL_ARGUMENT;
    envP_->SetByteArrayRegion(*targetArrayP, 0, length, (jbyte*) sourceArrayP);

    return ROOTPA_OK;
}

int JniHelpers::setBooleanToArray(jbooleanArray* targetArrayP, bool source)
{
    if(broken_) return ROOTPA_ERROR_INTERNAL;
    if(NULL==targetArrayP )  return ROOTPA_ERROR_ILLEGAL_ARGUMENT;
    envP_->SetBooleanArrayRegion(*targetArrayP, 0, 1 , (jboolean*) &source);

    return ROOTPA_OK;
}

int  JniHelpers::setIntToArray(jintArray* targetArrayP, int index, int source)
{
    if(broken_) return ROOTPA_ERROR_INTERNAL;
    if(NULL==targetArrayP )  return ROOTPA_ERROR_ILLEGAL_ARGUMENT;
    envP_->SetIntArrayRegion(*targetArrayP, index, 1 , (jint*) &source);

    return ROOTPA_OK;
}

jbyteArray JniHelpers::byteArrayToJByteArray(uint8_t* dataP, uint32_t length)
{
    jbyteArray jbArray = NULL;
    if (envP_->EnsureLocalCapacity(1) == JNI_OK)
    {

        if ((length > 0) && (dataP != NULL))
        {
            jbArray = envP_->NewByteArray(length);
            if (jbArray != NULL)
            {
                envP_->SetByteArrayRegion(jbArray, 0, length, (jbyte*) dataP);
            }
        }
    }

    return jbArray;
}

/**
* Retrieves a uint8_t* jbyteArray (java) object. Result needs to be freed with delete[] afterwards.
* @param jBytes jbyteArray (java) of primitive byte data
* @param outLength* (OUT PARAMETER) length of the returned uint8_t* byte array
* @return uint8_t* byte array. Needs to be freed with delete[] afterwards.
*/
uint8_t* JniHelpers::jByteArrayToCByteArray(jbyteArray jBytes, uint32_t* outLength)
{
    *outLength=0;
    uint8_t* cBytes = NULL;
    if ((jBytes != NULL) && !broken_)
    {
        *outLength = envP_->GetArrayLength(jBytes);
        cBytes = new uint8_t[*outLength];
        jbyte* p_jcResult = envP_->GetByteArrayElements(jBytes, JNI_FALSE);
        for (unsigned int i = 0; i < *outLength; i++)
        {
            cBytes[i] = (uint8_t) p_jcResult[i] & (0x00ff);
        }

        envP_->ReleaseByteArrayElements(jBytes, p_jcResult, JNI_FALSE);
    }
    else
    {
        LOGE("jByteArrayToCByteArray: Input is NULL or something else is broken. Cannot return byte array");
    }
    return cBytes;
}


