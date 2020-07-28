/*
 * Copyright (c) 2016-2017 TRUSTONIC LIMITED
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

#include "dynamic_log.h"
#include "proxy_internal.h"

using namespace trustonic;

// Missing std::make_unique
Proxy::Proxy(): pimpl_(new Impl(*this)) {}

// Destructor needs the size of Impl
Proxy::~Proxy() {}

void Proxy::TEEC_TT_RegisterPlatformContext(
        void*                   globalContext,
        void*                   localContext) {
    pimpl_->jvm = static_cast<JavaVM*>(globalContext);
    pimpl_->application_context = static_cast<jobject>(localContext);
}

struct TestData {
    bool        using_proxy = false;
    bool        using_proxy_protocol_v3 = false;
};

TEEC_Result Proxy::TEEC_TT_TestEntry(
        void*                   buff,
        size_t                  /*len*/,
        uint32_t*               /*tag*/) {
    ENTER();
    TestData* test_data = static_cast<TestData*>(buff);
    test_data->using_proxy = true;
    test_data->using_proxy_protocol_v3 = true;
    EXIT(TEEC_SUCCESS);
}

bool Proxy::getBindInformation(
    BindInformation*        bind_information) {
    ::strncpy(bind_information->package_name,
              "com.trustonic.teeproxyservice",
              sizeof(bind_information->package_name));
    ::strncpy(bind_information->starter_class,
              "TeeProxyService",
              sizeof(bind_information->starter_class));
    return true;
}
