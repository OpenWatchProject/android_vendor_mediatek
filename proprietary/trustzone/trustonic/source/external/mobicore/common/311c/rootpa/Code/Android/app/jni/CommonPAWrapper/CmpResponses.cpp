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
#include <stdlib.h>
#include "JniHelpers.h"
#include "CmpResponses.h"

CmpResponses::CmpResponses(JNIEnv* env, jobject responses): CmpBase(env, responses)
{
    if(!broken_)
    {
        jmethodID mid = env_->GetMethodID(cls_, "clear", "()V");
        if(mid != NULL)
        {
            env_->CallVoidMethod(msgs_, mid);
        }
        else
        {
    	    LOGE("CmpResponses::CmpResponses: Sorry, but the method clear()V cannot be found!");
        }
    }
}

CmpResponses::~CmpResponses()
{

}


int CmpResponses::update(CmpMessage* responses, int numberOfResponses)
{
    int ret=ROOTPA_OK;
    if(broken_) return ROOTPA_ERROR_INTERNAL;

    jmethodID mid=env_->GetMethodID(cls_, "add", "(Ljava/lang/Object;)Z");
    if(NULL==mid)
    {
        LOGE("do not find add(Ljava/lang/Object)Z from List");
        return ROOTPA_ERROR_INTERNAL;
    }

    jobject responseObject=NULL;
    for(int i=0; i<numberOfResponses; i++)
    {
        responseObject=createCmpResponseObject(responses[i]);
        if(responseObject != NULL)
        {
            if(((bool) env_->CallBooleanMethod(msgs_, mid, responseObject))==false)
            {
                ret=ROOTPA_ERROR_INTERNAL;
                LOGE("adding cmp response object %d failed", i);
            }
            env_->DeleteLocalRef(responseObject);
        }
        else
        {
            ret=ROOTPA_ERROR_INTERNAL;
            LOGE("creating cmp response object failed");
        }
    }
    return ret;
}



jobject CmpResponses::createCmpResponseObject(CmpMessage msg)
{
    jobject newObject=NULL;
    if(!broken_)
    {
        objectCls_=env_->FindClass("com/gd/mobicore/pa/ifc/CmpResponse");
        if(objectCls_!=NULL)
        {
            jmethodID constructor=NULL;
            JniHelpers helper(env_);
            jbyteArray rsp = helper.byteArrayToJByteArray(msg.contentP, msg.length);
            if(rsp != NULL)
            {
                constructor = env_->GetMethodID(objectCls_, "<init>", "([B)V");
            }
            else
            {
                constructor = env_->GetMethodID(objectCls_, "<init>", "()V");
                LOGE("CmpResponses::createCmpResponseObject no response received, using empty response object");
            }

            if(constructor != NULL)
            {

                if(rsp != NULL)
                {
                    newObject = env_->NewObject(objectCls_, constructor, rsp);
                }
                else
                {
                    newObject = env_->NewObject(objectCls_, constructor);
                }

                if(NULL==newObject)
                {
                    LOGE("CmpResponses::createCmpResponseObject creating new object failed %p %p", objectCls_, constructor);
                }

            }
            else
            {
                LOGE("CmpResponses::createCmpResponseObject creating constructor failed");
            }

            if(rsp != NULL)
            {
                env_->DeleteLocalRef(rsp);
            }

        }
        else
        {
            LOGE("CmpResponses::createCmpResponseObject did not find java side class /com/gd/mobicore/pa/ifc/CmpResponse");
        }
    }

    if(objectCls_!=NULL)
    {
        env_->DeleteLocalRef(objectCls_);
        objectCls_=NULL;
    }

    return newObject;
}


