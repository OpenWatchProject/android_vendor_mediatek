/*
 * Copyright (c) 2013-2017 TRUSTONIC LIMITED
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

#ifndef __PROXY_CLIENT_H__
#define __PROXY_CLIENT_H__

#include <functional>
#include <memory>

#include "proxy_common.h"

namespace trustonic {

class ProxyClient {
    struct Impl;
    const std::unique_ptr<Impl> pimpl_;
public:
    ProxyClient();
    ~ProxyClient();

    void registerPlatformContext(void* globalContext, void* localContext);
    bool open();
    void close();
    void registerCloseCallback(std::function<void()> cleanupCb);
    bool isOpen() const;
    int hasConnections() const;
    class ICaller {
    public:
        virtual ~ICaller() {}
        virtual bool isClientOperation() const = 0;
        virtual bool isOpenSession() const = 0;
        virtual bool isCloseSession() const = 0;
        virtual uint32_t sessionId() const = 0;
        virtual void setSignalled() = 0;
        virtual bool isSignalled() const = 0;
    };
    int call(
            ICaller& caller,
            trustonic::RpcMethod method,
            const ::google::protobuf::MessageLite& request,
            ::google::protobuf::MessageLite* response,
            std::function<void()> message_sent_cb = nullptr);
    int forceCloseConnection(ICaller& caller);
};

} // namespace trustonic

#endif // __PROXY_CLIENT_H__
