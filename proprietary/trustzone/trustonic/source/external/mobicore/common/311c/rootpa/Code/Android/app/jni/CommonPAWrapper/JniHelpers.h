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

#ifndef JNIHELPERS_H
#define JNIHELPERS_H

#include <stdlib.h>
#include <jni.h>
#include "logging.h"


class JniHelpers
{
    public:
        JniHelpers(JNIEnv* envP);
        JniHelpers(JNIEnv* envP, jobject* keysP, jobject* valuesP, jbyteArray* productIdP);
        virtual ~JniHelpers();
        int setVersion(char* fieldName, int version);
        int setProductId(char* productId);
        int setByteArray(jbyteArray* targetArrayP, uint8_t* sourceArrayP , uint32_t length);
        int setBooleanToArray(jbooleanArray* targetArrayP, bool source);
        int setIntToArray(jintArray* targetArrayP, int index, int source);
        jbyteArray byteArrayToJByteArray(uint8_t* dataP, uint32_t length);
        uint8_t* jByteArrayToCByteArray(jbyteArray jBytes, uint32_t* outLength);
    private:

        bool broken_;

        JNIEnv* envP_;
        jobject* keysP_;
        jobject* valuesP_;
        jbyteArray* productIdP_;

// classes and methods

        jclass listCls_;
        jclass intCls_;
        jclass stringCls_;
        jmethodID stringConstructur_;
        jmethodID intConstructor_;
        jmethodID listAdd_;
};


#endif // JNIHELPERS_H
