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
#include "logging.h"
#include "CmpCommands.h"
#include "JniHelpers.h"
CmpCommands::CmpCommands(JNIEnv* env, jobject commands): CmpBase(env, commands), jObjectArray_(NULL)
{
    if(!broken_)
    {
        createObjectArray();
    }
}

CmpCommands::~CmpCommands()
{

}


/*
Trusting that the caller has reserved long enough array
*/
bool CmpCommands::getCommands(CmpMessage* objectArray)
{
    if(!broken_)
    {
        if(NULL==objectCls_)
        {
            objectCls_ = env_->FindClass("com/gd/mobicore/pa/ifc/CmpCommand"); // element in array
        }
        if(objectCls_ != NULL)
        {
            jmethodID midToByteArray=env_->GetMethodID(objectCls_, "toByteArray", "()[B");
            if(NULL==midToByteArray){
                LOGE("<<CmpCommands::getCommands returning false, method toByteArray not found");
                return false;
            }
            jmethodID midIgnoreError=env_->GetMethodID(objectCls_, "ignoreError", "()Z");
            if(NULL==midIgnoreError){
                LOGE("<<CmpCommands::getCommands returning false, method ignoreError not found");
                return false;
            }

            jbyteArray jba=NULL;
            jobject arrayElement=NULL;
            for(int i=0; i<numberOfElements(); i++)
            {
                arrayElement = env_->GetObjectArrayElement(jObjectArray_, i);
                jba = (jbyteArray) env_->CallObjectMethod(arrayElement, midToByteArray);

                JniHelpers helper(env_);
                objectArray[i].contentP=helper.jByteArrayToCByteArray(jba, &(objectArray[i].length));
                objectArray[i].hdr.ignoreError=(JNI_TRUE==env_->CallBooleanMethod(arrayElement, midIgnoreError));
                env_->DeleteLocalRef(jba);
                env_->DeleteLocalRef(arrayElement);
            }
        return true;
        }
    }
    return false;
}


/*

*/
void CmpCommands::createObjectArray()
{
    if(broken_) return;

    jmethodID mid = env_->GetMethodID(cls_, "toArray", "()[Ljava/lang/Object;");
    if (mid != 0)
    {
	    jObjectArray_ = (jobjectArray) env_->CallObjectMethod(msgs_, mid);
        if(NULL==jObjectArray_)
        {
	        LOGE("error in getting jObjectArray_");
            broken_= true;
        }
	}
    else
    {
	    LOGE("Sorry, but the method toArray()[Ljava/lang/Object cannot be found!");
        broken_= true;
    }
    return;
}
