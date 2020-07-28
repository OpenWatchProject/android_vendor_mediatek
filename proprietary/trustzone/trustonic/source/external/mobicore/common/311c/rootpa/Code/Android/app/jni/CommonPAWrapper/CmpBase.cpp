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
#include "CmpBase.h"

/*=================================================================================

Public methods

*/

CmpBase::CmpBase(JNIEnv* env, jobject msgs): env_(env),
                                             msgs_(msgs),
                                             cls_(NULL),
                                             objectCls_(NULL),
                                             numberOfElements_(0),
											 broken_(false)
{
    if((env_ != NULL) && (msgs_ != NULL))
    {
        cls_=env_->GetObjectClass(msgs_);
        if(NULL==cls_)
        {
            LOGE("can not get object class");
            broken_=true;
        }
    }
    else
    {
        broken_=true;
    }
}

/*
*/
CmpBase::~CmpBase()
{
    if(cls_!=NULL)
    {
        env_->DeleteLocalRef(cls_);
    }

    if(objectCls_!=NULL)
    {
        env_->DeleteLocalRef(objectCls_);
    }
}

/*
returns number of elements in the array. For the first time gets it from java
*/
int CmpBase::numberOfElements()
{
    if(!broken_ && (0 == numberOfElements_))
    {
        jmethodID mid =  env_->GetMethodID(cls_, "size", "()I");
        if(mid !=0)
        {
            numberOfElements_ = (int) env_->CallIntMethod(msgs_, mid);
        }
        else
        {
            LOGE("no size ()I method");
        }
    }
    return numberOfElements_;
}





