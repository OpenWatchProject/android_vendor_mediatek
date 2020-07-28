/*
 * Copyright (c) 2013-2018 TRUSTONIC LIMITED
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

#include <algorithm>
#include <memory>
#include <mutex>
#include <string>
#include <vector>

#include <string.h>     // strncpy
#include <sys/mman.h>   // mmap, munmap
#include <sys/socket.h>
#include <sys/un.h>

#include "dynamic_log.h"
#include "proxy_common.h"
#include "proxy_client.h"
#include "tee_bind_jni.h"       // JavaProcess

using namespace trustonic;

class ClientConnection {
    std::mutex comm_mutex_;
    std::mutex comm_queue_mutex_;
    ProtocolVersion version_;
    std::string path_;
    bool is_broken_ = false;
    std::unique_ptr<CommonConnection> conn_;
    uint32_t session_id_ = 0;
public:
    ClientConnection(ProtocolVersion version, const std::string& path):
        version_(version), path_(path) {}
    ~ClientConnection() {
        if (conn_) {
            LOG_D("socket %d disconnected", conn_->fd());
            conn_.reset();
        }
    }
    bool open() {
        std::unique_ptr<CommonConnection> conn(
                    new CommonConnection(path_.c_str(), version_));
        if (conn->connect()) {
            return false;
        }
        conn_ = std::move(conn);
        LOG_D("socket %d connected", conn_->fd());
        return true;
    }
    bool isBroken() const {
        return is_broken_;
    }
    void setSessionId(uint32_t session_id) {
        session_id_ = session_id;
    }
    uint32_t sessionId() const {
        return session_id_;
    }
    /*
     * Return codes:
     * -1: error, can be a protocol (isBroken() is true) or a called method error
     *  0: no error
     */
    int call(RpcMethod method, const ::google::protobuf::MessageLite& request,
             ::google::protobuf::MessageLite* response, bool* signal_received,
             std::function<void()> message_sent_cb) {
        // Trick to ensure round-robin access to two callers
        comm_queue_mutex_.lock();
        std::lock_guard<std::mutex> comm_lock(comm_mutex_);
        comm_queue_mutex_.unlock();
        LOG_D("socket %d session %x call %s", conn_->fd(), session_id_,
              methodToString(method));

        // Serialize request
        std::string buffer;
        if (!request.SerializeToString(&buffer)) {
            LOG_E("Failed to serialize");
            is_broken_ = true;
            return -1;
        }

        // Send request
        if (conn_->sendRequest(method, buffer, signal_received)) {
            is_broken_ = true;
            return -1;
        }

        // Receive intermediate response
        int method_rc;
        int length;
        if (message_sent_cb) {
            std::string unused;
            length = conn_->receiveResponse(method, &unused, &method_rc, signal_received);
            if (length != 0) {
                is_broken_ = true;
                return -1;
            }
            message_sent_cb();
        }

        // Receive response
        length = conn_->receiveResponse(method, &buffer, &method_rc, signal_received);
        if (length < 0) {
            is_broken_ = true;
            return -1;
        }

        // Parse response regardless of whether a method rc was returned
        if ((length > 0) && !response->ParseFromString(buffer)) {
            LOG_E("Failed to parse");
            is_broken_ = true;
            return -1;
        }

        // Method errors do not require specific method action
        if (method_rc) {
            errno = method_rc;
            return -1;
        }

        LOG_D("socket %d session %x call done", conn_->fd(), session_id_);
        return 0;
    }
};

class SessionConnectionsList {
    std::mutex mutex_;
    std::vector<std::shared_ptr<ClientConnection>> connections_;
public:
    std::shared_ptr<ClientConnection> find(uint32_t id) {
        std::lock_guard<std::mutex> lock(mutex_);
        for (auto& connection: connections_) {
            if (connection->sessionId() == id) {
                LOG_D("found connection %p for session %x", connection.get(), id);
                return connection;
            }
        }
        errno = ENXIO;
        LOG_D("found no connection for session %x", id);
        return nullptr;
    }

    void add(const std::shared_ptr<ClientConnection>& connection) {
        std::lock_guard<std::mutex> lock(mutex_);
        connections_.push_back(connection);
        LOG_D("added connection %p for session %x", connection.get(),
              connection->sessionId());
    }

    int remove(uint32_t id) {
        std::lock_guard<std::mutex> lock(mutex_);
        auto it = std::find_if(connections_.begin(), connections_.end(),
                               [id](const std::shared_ptr<ClientConnection>& connection) {
            return connection->sessionId() == id;
        });
        if (it == connections_.end()) {
            LOG_D("found no connection for session %x", id);
            // Not found
            return -1;
        }
        LOG_D("removed connection %p for session %x", (*it).get(), id);
        connections_.erase(it);
        return 0;
    }

    void clear() {
        connections_.clear();
    }

    bool empty() const {
        return connections_.empty();
    }
};

struct ProxyClient::Impl {
    ProtocolVersion version = PROTOCOL_V3_VERSION;
    std::string     socket_path = SOCKET_TDP_PATH;

    // Client connection
    std::shared_ptr<ClientConnection> connection;
    // Sessions
    SessionConnectionsList connections;
    std::mutex close_cb_mutex;
    std::function<void()> close_cb;
};

// Missing std::make_unique
ProxyClient::ProxyClient(): pimpl_(new Impl) {}

// Destructor needs the size of Impl
ProxyClient::~ProxyClient() {
    if (pimpl_->connection) {
        close();
    }
}

void ProxyClient::registerPlatformContext(
        void* /*globalContext*/,
        void* /*localContext*/) {
}

bool ProxyClient::open() {
    // Open the 'client' connection
    pimpl_->connection.reset();
    std::shared_ptr<ClientConnection> connection;
    connection = std::make_shared<ClientConnection>(pimpl_->version,
                                                    pimpl_->socket_path);

    // Wait up to 5s for bind to start the proxy service
    int timeout = 50;
    while (timeout--) {
        if (connection->open()) {
            pimpl_->connection = connection;
            LOG_I("proxy client open (%s:%d)",
                  pimpl_->socket_path.c_str(),
                  pimpl_->version);
            return true;
        }

        if (errno != ECONNREFUSED) {
            break;
        }

        if (timeout) {
            LOG_I("%s connecting to proxy server, retry in 100ms", strerror(errno));
            usleep(100000);
        }
    }

    return false;
}

void ProxyClient::close() {
    pimpl_->connections.clear();
    pimpl_->connection.reset();
    // If the client is closed by a GP FinalizeContext, the existing MC session
    // list also needs to be flushed
    std::lock_guard<std::mutex> lock(pimpl_->close_cb_mutex);
    if (pimpl_->close_cb) {
        pimpl_->close_cb();
    }

    LOG_I("proxy client closed");
}

void ProxyClient::registerCloseCallback(std::function<void()> closeCb) {
    std::lock_guard<std::mutex> lock(pimpl_->close_cb_mutex);
    if (!pimpl_->close_cb) {
        pimpl_->close_cb = closeCb;
    }
}

bool ProxyClient::isOpen() const {
    return pimpl_->connection.get();
}

int ProxyClient::hasConnections() const {
    if (!isOpen()) {
        errno = EBADF;
        return -1;
    }
    int ret = 0;
    if (!pimpl_->connections.empty()) {
        ret = -1;
        errno = ENOTEMPTY;
    }
    LOG_D("%s rc=%d", __FUNCTION__, ret);
    return ret;
}

int ProxyClient::call(ICaller& caller, RpcMethod method,
                      const ::google::protobuf::MessageLite& request,
                      ::google::protobuf::MessageLite* response,
                      std::function<void()> message_sent_cb) {
    LOG_D("%s", methodToString(method));

    // Make sure we are initialised
    if (!isOpen()) {
        errno = EBADF;
        LOG_ERRNO(__func__);
        return -1;
    }
    // Get connection
    std::shared_ptr<ClientConnection> connection;
    if (caller.isClientOperation()) {
        connection = pimpl_->connection;
    } else if (caller.isOpenSession()) {
        // Open new connection
        connection.reset(new ClientConnection(pimpl_->version,
                                              pimpl_->socket_path));
        if (!connection->open()) {
            LOG_ERRNO(__func__);
            return -1;
        }
    } else {
        // Find connection for session
        connection = pimpl_->connections.find(caller.sessionId());
        if (!connection) {
            LOG_ERRNO(__func__);
            return -1;
        }
    }
    // Exchange
    int saved_errno = errno;
    errno = EHOSTUNREACH;
    bool signal_received = false;
    if (connection->call(method, request, response, &signal_received,
                         message_sent_cb)) {
        // Failure
        if (!caller.isOpenSession() && connection->isBroken()) {
            pimpl_->connections.remove(caller.sessionId());
        }
        // We don't get an error in case of EINTR, but the only blocking call is
        // mcWaitNotification() where we time-out every second, so we can check
        // for a received-but-ignored EINTR here.
        if (signal_received) {
            caller.setSignalled();
        }
        return -1;
    }
    // Success
    if (caller.isOpenSession()) {
        auto session_id = caller.sessionId();
        if (session_id) {
            connection->setSessionId(caller.sessionId());
            pimpl_->connections.add(connection);
        }
    } else if (caller.isCloseSession()) {
        pimpl_->connections.remove(caller.sessionId());
    }
    errno = saved_errno;
    return 0;
}

// Must only be called in V3 protocol, when the client force-closes
// a session in case of a specific error in the InvokeCommand
int ProxyClient::forceCloseConnection(ICaller& caller) {
    LOG_D("Force closing session %x", caller.sessionId());
    return pimpl_->connections.remove(caller.sessionId());
}
