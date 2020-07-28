/*
 * Copyright (c) 2013-2016 TRUSTONIC LIMITED
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

#include <thread>

#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <string.h>
#include <signal.h>
#include <libgen.h>
#include <list>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>

// Logging
#include "dynamic_log.h"

#include "PrivateRegistry.h"
#include "RegistryServer.h"
#include "buildTag.h"

/** Number of incoming connections that can be queued.
 * Additional clients will generate the error ECONNREFUSED. */
#define LISTEN_QUEUE_LEN    8

typedef std::list<Connection *>        connectionList_t;
typedef connectionList_t::iterator     connectionIterator_t;

struct RegistryServer::Impl: public ConnectionHandler {
    struct cmd_map_item_t {
        uint32_t (RegistryServer::Impl:: *handler)(
            const std::string& rx_data,
            std::string& tx_data);
        uint32_t min_rx_size;
    };

    // Server
    int                         server_sock = -1;
    connectionList_t            peer_connection;
    const char*                 local_address = SOCK_PATH;
    int                         listen_queue_size = LISTEN_QUEUE_LEN;
    std::thread                 thread;
    bool                        server_started = false;

    // Connection Handler
    SecureWorld* secure_world;
    static const cmd_map_item_t reg_cmd_map[];
    static const uint MAX_DATA_SIZE = 512;

    Impl(SecureWorld* sw): secure_world(sw) {}

    // Server thread
    void run();

    // Connection handler functions
    /**
     * installEndorsementToken
     * Look for tokens in the registry and pass them to Kinibi for endorsement purposes
     * Search order:  1. authtoken 2. authtoken backup 3. root container
     */
    bool handleConnection(Connection &connection);
    void dropConnection(Connection&) {}
    void installEndorsementToken();

    /* Registry commands, arrived through socket */
    uint32_t reg_store_auth_token(
        const std::string& rx_data, std::string& tx_data);
    uint32_t reg_store_root_cont(
        const std::string& rx_data, std::string& tx_data);
    uint32_t reg_store_sp_cont(
        const std::string& rx_data, std::string& tx_data);
    uint32_t reg_store_tl_cont(
        const std::string& rx_data, std::string& tx_data);
    uint32_t reg_store_so_data(
        const std::string& rx_data, std::string& tx_data);
    uint32_t reg_store_ta_blob(
        const std::string& rx_data, std::string& tx_data);
    uint32_t reg_read_auth_token(
        const std::string& rx_data, std::string& tx_data);
    uint32_t reg_read_root_cont(
        const std::string& rx_data, std::string& tx_data);
    uint32_t reg_read_sp_cont(
        const std::string& rx_data, std::string& tx_data);
    uint32_t reg_read_tl_cont(
        const std::string& rx_data, std::string& tx_data);
    uint32_t reg_delete_auth_token(
        const std::string& rx_data, std::string& tx_data);
    uint32_t reg_delete_root_cont(
        const std::string& rx_data, std::string& tx_data);
    uint32_t reg_delete_sp_cont(
        const std::string& rx_data, std::string& tx_data);
    uint32_t reg_delete_tl_cont(
        const std::string& rx_data, std::string& tx_data);
    uint32_t reg_delete_ta_objs(
        const std::string& rx_data, std::string& tx_data);
};

RegistryServer::RegistryServer(SecureWorld* p_secure_world):
    pimpl_(new Impl(p_secure_world)) {}

RegistryServer::~RegistryServer() {
    delete pimpl_;
}

int RegistryServer::open() {
    //Initialize socket
    // Check local address
    if (pimpl_->local_address == NULL || strlen(pimpl_->local_address) == 0
        || pimpl_->listen_queue_size <= 0) {
        LOG_E("Local address not correct");
        return -1;
    }

    LOG_D("Start listening on socket %s", pimpl_->local_address);

    // Open a socket (a UNIX domain stream socket)
    int sock;
    sock = ::socket(AF_UNIX, SOCK_STREAM, 0);
    if (sock < 0) {
        LOG_ERRNO("Can't open stream socket, because socket");
        return -1;
    }

    struct sockaddr_un serverAddr;
    serverAddr.sun_family = AF_UNIX;
    ::strncpy(serverAddr.sun_path, pimpl_->local_address, sizeof(serverAddr.sun_path) - 1);
    socklen_t len = static_cast<socklen_t>(::strlen(serverAddr.sun_path) +
                                           sizeof(serverAddr.sun_family));
    // Make the socket in the Abstract Domain(no path but everyone can connect)
    serverAddr.sun_path[0] = 0;

    if (::bind(sock, reinterpret_cast<struct sockaddr*>(&serverAddr), len)) {
        LOG_ERRNO("Binding to server socket failed, because bind");
        ::close(sock);
        return -1;
    }

    if (::listen(sock, pimpl_->listen_queue_size)) {
        LOG_ERRNO("listen");
        ::close(sock);
        return -1;
    }
    pimpl_->server_sock = sock;
    // Socket initialized successfully

    // Create Registry server thread
    pimpl_->thread = std::thread(&Impl::run, pimpl_);
    pimpl_->server_started = true;
    LOG_D("Server started");
    return 0;
}

void RegistryServer::close() {
    // Stop the thread
    LOG_D("Killing RegistryServer thread");
    ::pthread_kill(pimpl_->thread.native_handle(), SIGUSR1);
    pimpl_->thread.join();

    // Cleanup, no need for mutex, join blocks util thread ends
    while(!pimpl_->peer_connection.empty()) {
            Connection *c = pimpl_->peer_connection.front();
        pimpl_->peer_connection.pop_front();
        delete c;
    }

    // Shut down the server socket
    if(pimpl_->server_sock != -1) {
        ::close(pimpl_->server_sock);
        pimpl_->server_sock = -1;
    }

    LOG_W("closed");
}

// SIGUSR1 is required to unlock the select function, but we have to override
// the handler with the following dummy function to override the default
// handler : "Term"
static void dummyHandler(int /* sig */) {
    LOG_D("dummyHandler");
    return;
}

void RegistryServer::Impl::run() {
    ::pthread_setname_np(pthread_self(), "McDaemon.Server");
    // Process SIGUSR1 action
    struct sigaction actionSIGUSR1;
    ::memset(&actionSIGUSR1, 0, sizeof(actionSIGUSR1));
    actionSIGUSR1.sa_handler = dummyHandler;
    ::sigemptyset(&actionSIGUSR1.sa_mask);
    // Explicitely specify "no flags" to avoid the reexecution of select
    // in case of interruption.
    actionSIGUSR1.sa_flags = 0;
    if (::sigaction(SIGUSR1, &actionSIGUSR1, NULL)) {
        LOG_ERRNO("actionSIGUSR1");
        return;
    }
    ::sigaddset(&actionSIGUSR1.sa_mask, SIGUSR1);
    ::pthread_sigmask(SIG_UNBLOCK, &actionSIGUSR1.sa_mask, NULL);

    LOG_I("%s: Starting server", __func__);
    while (true) {
        fd_set fdReadSockets;

        // Clear FD for select()
        FD_ZERO(&fdReadSockets);

        // Select server socket descriptor
        FD_SET(server_sock, &fdReadSockets);
        int maxSocketDescriptor = server_sock;

        // Select socket descriptor of all connections
        for (auto it = peer_connection.begin(); it != peer_connection.end(); it++) {
            auto& conn = *it;
            int peerSocket = conn->socket();
            FD_SET(peerSocket, &fdReadSockets);
            if (peerSocket > maxSocketDescriptor)
                maxSocketDescriptor = peerSocket;
        }

        // Wait for activities, select() returns the number of sockets
        // which require processing
        LOG_D("Waiting on sockets");
        int numSockets = ::select(maxSocketDescriptor + 1,
                                    &fdReadSockets, NULL, NULL, NULL);
        // Check if select failed
        if (numSockets < 0) {
            int err = errno;
            if (err == EINTR) {
                LOG_D("Giving up on signal");
                break;
            } else {
                LOG_ERRNO("select failed");
                continue;
            }
        }

        // actually, this should not happen.
        if (0 == numSockets) {
            LOG_W("Select() returned 0, spurious event?");
            continue;
        }

        LOG_D("Events on %d socket(s)", numSockets);

        // Check if a new client connected to the server socket
        if (FD_ISSET(server_sock, &fdReadSockets)) {
            LOG_D("New connection attempt");
            numSockets--;

            int clientSock = ::accept(server_sock, NULL, NULL);
            if (clientSock > 0) {
                Connection *connection = new Connection(clientSock);
                peer_connection.push_back(connection);
                LOG_D("New socket connection established and start listening");
            } else {
                LOG_ERRNO("accept");
            }

            // we can ignore any errors from accepting a new connection.
            // If this fail, the client has to deal with it, we are done
            // and nothing has changed.
        }

        // Handle traffic on existing client connections
        auto it = peer_connection.begin();
        while ((it != peer_connection.end()) && (numSockets > 0)) {
            Connection *connection = *it;

            if (!FD_ISSET(connection->socket(), &fdReadSockets)) {
                ++it;
                continue;
            }

            numSockets--;

            // the connection will be terminated if command processing
            // fails
            if (!handleConnection(*connection)) {
                LOG_D("Dropping connection");

                //Inform the driver
                dropConnection(*connection);

                // Remove connection from list
                it = peer_connection.erase(it);
                delete connection;
            } else {
                it++;
            }
        }
    }
    LOG_D("end thread");
}

void RegistryServer::Impl::installEndorsementToken(void) {
        /* Look for tokens in the registry and pass them to Kinibi for endorsement
         * purposes.
         */
        LOG_D("Looking for suitable tokens");

        mcSoAuthTokenCont_t authtoken;
        mcSoAuthTokenCont_t authtokenbackup;
        mcSoRootCont_t rootcont;
        uint32_t sosize;
        const void *p = NULL;

        // Search order:  1. authtoken 2. authtoken backup 3. root container
        sosize = 0;
        mcResult_t ret = mcRegistryReadAuthToken(&authtoken);
        if (ret != MC_DRV_OK) {
            LOG_D("Failed to read AuthToken (ret=%u), trying AuthToken backup", ret);

            ret = mcRegistryReadAuthTokenBackup(&authtokenbackup);
            if (ret != MC_DRV_OK) {
                LOG_D("Failed to read AuthToken backup (ret=%u). Trying Root Cont", ret);

            sosize = sizeof(rootcont);
            ret = mcRegistryReadRoot(&rootcont, &sosize);
            if (ret != MC_DRV_OK) {
                    LOG_D("Failed to read Root Cont, (ret=%u)", ret);
                    LOG_W("Device endorsements not supported");
                    sosize = 0;
                } else {
                    LOG_D("Found Root Cont");
                    p = &rootcont;
                }

            } else {
                LOG_D("Found AuthToken backup");
                p = &authtokenbackup;
                sosize = sizeof(authtokenbackup);
            }

        } else {
            LOG_D("Found AuthToken");
            p = &authtoken;
            sosize = sizeof(authtoken);
        }

        if (sosize) {
            LOG_D("Found token of size: %u", sosize);
            if (secure_world->loadToken(p, sosize) < 0) {
                LOG_E("Failed to pass token to Kinibi"
                      " device endorsements disabled");
            }
        }
}

void RegistryServer::installEndorsementToken(void) {
    // Make the function available outside of the implementation
    pimpl_->installEndorsementToken();
}

////////////////////////////////////////////////////////////////////////////////
// Connection handler functions
////////////////////////////////////////////////////////////////////////////////
bool RegistryServer::Impl::handleConnection(Connection &conn) {
    LOG_D("handleConnection()==== %p", &conn);

    CommandHeader cmd;
    switch (conn.readData(&cmd, sizeof(cmd))) {
    case sizeof(cmd):
        break;
    case 0:
        LOG_D(" handleConnection(): Connection closed");
        return false;
    case -1:
        LOG_E("Socket error");
        return false;
    case -2:
        LOG_E("Timeout");
        return false;
    default:
        LOG_E("Insufficient data arrived");
        return false;
    }

    ResponseHeader result = { 0, MC_DRV_OK, 0 };
    std::string rx_data;
    std::string tx_data;

    if (MC_DRV_REG_END > cmd.cmd) {
        if (cmd.data_size) {
            if (cmd.data_size >= reg_cmd_map[cmd.cmd].min_rx_size) {
                rx_data.resize(cmd.data_size);
                size_t total = 0;
                while (total < rx_data.size()) {
                    ssize_t sz = conn.readData(&rx_data[total], cmd.data_size - total);
                    if (sz <= 0) {
                        LOG_E("Payload reading failed for command %d expected=%zu received=%zd", cmd.cmd, cmd.data_size - total, sz);
                        result.result = MC_DRV_ERR_UNKNOWN;
                        break;
                    }
                    total += sz;
                }
            } else {
                LOG_E("Invalid payload size for command %d", cmd.cmd);
            }
        }

        if (result.result == MC_DRV_OK) {
            result.result = (this->*(reg_cmd_map[cmd.cmd].handler))(rx_data, tx_data);
        }
    } else {
        LOG_E("Unknown command %d", cmd.cmd);
        result.result = MC_DRV_ERR_INVALID_OPERATION;
    }

    struct iovec iov[2];
    iov[0].iov_len  = sizeof(result);
    iov[0].iov_base = const_cast<ResponseHeader *>(&result);
    int count = 1;
    ssize_t write_sz = sizeof(result);

    if ((result.result == MC_DRV_OK) && !tx_data.empty()) {
        iov[1].iov_len = tx_data.size();
        iov[1].iov_base = &tx_data[0];
        count++;
        write_sz += tx_data.size();
    }

    LOG_D("handleConnection()<-------");
    return conn.writeMsg(iov, count) == write_sz;
}

uint32_t RegistryServer::Impl::reg_store_auth_token(
        const std::string& rx_data, std::string&) {
    mcResult_t mcRet = mcRegistryStoreAuthToken(&rx_data[0], rx_data.size());
    /* Load authentication token into Kinibi */
    if (MC_DRV_OK == mcRet) {
        if (secure_world->loadToken(&rx_data[0], rx_data.size()) < 0) {
            LOG_E("Failed to pass Auth Token to Kinibi");
        }
    }
    return mcRet;
}

uint32_t RegistryServer::Impl::reg_store_root_cont(
        const std::string& rx_data, std::string&) {
    mcResult_t mcRet = mcRegistryStoreRoot(&rx_data[0], rx_data.size());
    /* Load root container into Kinibi */
    if (MC_DRV_OK == mcRet) {
        if (secure_world->loadToken(&rx_data[0], rx_data.size()) < 0) {
            LOG_E("Failed to pass Root Container to Kinibi");
        }
    }
    return mcRet;
}

uint32_t RegistryServer::Impl::reg_store_sp_cont(
        const std::string& rx_data, std::string&) {
    auto blob = reinterpret_cast<const TaBlob*>(&rx_data[0]);
    size_t blobSize = rx_data.size() - sizeof(*blob);
    return mcRegistryStoreSp(blob->spid, blob->blob, blobSize);
}

uint32_t RegistryServer::Impl::reg_store_tl_cont(
        const std::string& rx_data, std::string&) {
    auto blob = reinterpret_cast<const TlBlob*>(&rx_data[0]);
    size_t blobSize = rx_data.size() - sizeof(*blob);
    return mcRegistryStoreTrustletCon(&blob->uuid, blob->spid, blob->blob, blobSize);
}

uint32_t RegistryServer::Impl::reg_store_so_data(
        const std::string& rx_data, std::string&) {
    return mcRegistryStoreData(&rx_data[0], rx_data.size());
}

uint32_t RegistryServer::Impl::reg_store_ta_blob(
        const std::string& rx_data, std::string&) {
    auto blob = reinterpret_cast<const TaBlob*>(&rx_data[0]);
    if (rx_data.size() < sizeof(*blob)) {
        return MC_DRV_ERR_INVALID_PARAMETER;
    }
    size_t blobSize = rx_data.size() - sizeof(*blob);
    if (secure_world->loadCheck(blob->spid, blob->blob,
                                 static_cast<uint32_t>(blobSize)) < 0) {
        return MC_DRV_ERR_TRUSTLET_NOT_FOUND;
    }
    return mcRegistryStoreTABlob(blob->spid, blob->blob, blobSize);
}

uint32_t RegistryServer::Impl::reg_read_auth_token(
        const std::string&, std::string& tx_data) {
    // Make buffer big enough
    tx_data.resize(sizeof(mcSoAuthTokenCont_t));
    return mcRegistryReadAuthToken(reinterpret_cast<mcSoAuthTokenCont_t*>(&tx_data[0]));
}

uint32_t RegistryServer::Impl::reg_read_root_cont(
        const std::string&, std::string& tx_data) {
    // Make buffer big enough
    tx_data.resize(MAX_DATA_SIZE);
    uint32_t size = MAX_DATA_SIZE;
    uint32_t ret = mcRegistryReadRoot(&tx_data[0], &size);
    if (ret == MC_DRV_OK) {
        tx_data.resize(size);
    }
    return ret;
}

uint32_t RegistryServer::Impl::reg_read_sp_cont(
        const std::string& rx_data, std::string& tx_data) {
    const mcSpid_t &spid = *reinterpret_cast<const mcSpid_t *>(&rx_data[0]);
    // Make buffer big enough
    tx_data.resize(MAX_DATA_SIZE);
    uint32_t size = MAX_DATA_SIZE;
    uint32_t ret = mcRegistryReadSp(spid, &tx_data[0], &size);
    if (ret == MC_DRV_OK) {
        tx_data.resize(size);
    }
    return ret;
}

uint32_t RegistryServer::Impl::reg_read_tl_cont(
        const std::string& rx_data, std::string& tx_data) {
    auto blob = reinterpret_cast<const TlBlob*>(&rx_data[0]);
    // Make buffer big enough
    tx_data.resize(MAX_DATA_SIZE);
    uint32_t size = MAX_DATA_SIZE;
    uint32_t ret = mcRegistryReadTrustletCon(&blob->uuid, blob->spid, &tx_data[0], &size);
    if (ret == MC_DRV_OK) {
        tx_data.resize(size);
    }
    return ret;
}

uint32_t RegistryServer::Impl::reg_delete_auth_token(
        const std::string&, std::string&) {
    return mcRegistryDeleteAuthToken();
}

uint32_t RegistryServer::Impl::reg_delete_root_cont(
        const std::string&, std::string&) {
    mcResult_t mcRet = mcRegistryCleanupRoot();

    // Look for tokens and send it to Kinibi if any
    if (MC_DRV_OK == mcRet)
        installEndorsementToken();

    return mcRet;
}

uint32_t RegistryServer::Impl::reg_delete_sp_cont(
        const std::string& rx_data, std::string&) {
    const mcSpid_t &spid = *reinterpret_cast<const mcSpid_t *>(&rx_data[0]);
    return mcRegistryCleanupSp(spid);
}

uint32_t RegistryServer::Impl::reg_delete_tl_cont(
        const std::string& rx_data, std::string&) {
    auto blob = reinterpret_cast<const TlBlob*>(&rx_data[0]);
    return mcRegistryCleanupTrustlet(&blob->uuid, blob->spid);
}

uint32_t RegistryServer::Impl::reg_delete_ta_objs(
        const std::string& rx_data, std::string&) {
    const mcUuid_t *uuid = reinterpret_cast<const mcUuid_t *>(&rx_data[0]);
    return mcRegistryCleanupGPTAStorage(uuid);
}

/**
* Signal handler for daemon termination
* Using this handler instead of the standard libc one ensures the daemon
* can cleanup everything -> read() on a FD will now return EINTR
*/
const RegistryServer::Impl::cmd_map_item_t
RegistryServer::Impl::reg_cmd_map[] = {
    {
        &RegistryServer::Impl::reg_read_auth_token,
        0
    },
    {
        &RegistryServer::Impl::reg_store_auth_token,
        0
    },
    {
        &RegistryServer::Impl::reg_delete_auth_token,
        0
    },
    {
        &RegistryServer::Impl::reg_read_root_cont,
        0
    },
    {
        &RegistryServer::Impl::reg_store_root_cont,
        0
    },
    {
        &RegistryServer::Impl::reg_delete_root_cont,
        0
    },
    {
        &RegistryServer::Impl::reg_read_sp_cont,
        sizeof(mcSpid_t)
    },
    {
        &RegistryServer::Impl::reg_store_sp_cont,
        sizeof(TaBlob)
    },
    {
        &RegistryServer::Impl::reg_delete_sp_cont,
        sizeof(mcSpid_t)
    },
    {
        &RegistryServer::Impl::reg_read_tl_cont,
        sizeof(TlBlob)
    },
    {
        &RegistryServer::Impl::reg_store_tl_cont,
        sizeof(TlBlob)
    },
    {
        &RegistryServer::Impl::reg_delete_tl_cont,
        sizeof(TlBlob)
    },
    {
        &RegistryServer::Impl::reg_store_so_data,
        0
    },
    {
        &RegistryServer::Impl::reg_store_ta_blob,
        sizeof(TaBlob)
    },
    {
        &RegistryServer::Impl::reg_delete_ta_objs,
        sizeof(mcUuid_t)
    },
};
