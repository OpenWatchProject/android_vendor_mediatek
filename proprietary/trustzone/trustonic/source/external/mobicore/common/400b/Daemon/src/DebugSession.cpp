/*
 * Copyright (c) 2015-2016 TRUSTONIC LIMITED
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

/**
 * DebugSession server.
 *
 * Maintains a session to SWd and dumps session information into dump file when
 * signal USR2 is received.
 */

#include <string>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <signal.h>

#include "dynamic_log.h"

#include "MobiCoreDriverApi.h"
#include "MobiCoreRegistry.h"
#include "mcdebugsession.h" // access the size offset in the tci
#include "DebugSession.h"

#define DEFAULT_DEBUG_SESSION_SIZE (42 * 1024)

struct DebugSession::Impl {
    mcSessionHandle_t   session_handle; /**< current session */
    bool                session_open = false;
    std::string         tci;
};

DebugSession::DebugSession(): pimpl_(new Impl) {}

DebugSession::~DebugSession() {
    delete pimpl_;
}

int DebugSession::open() {
    mcResult_t mcRet;
    const mcUuid_t uuid = MC_UUID_DEBUG_SESSION;

    // Open device
    mcRet = mcOpenDevice(MC_DEVICE_ID_DEFAULT);
    if (MC_DRV_OK != mcRet) {
        LOG_E("mcOpenDevice returned: 0x%08X\n", mcRet);
        return -1;
    }

    // Open session
    ::memset(&pimpl_->session_handle, 0, sizeof(pimpl_->session_handle));
    pimpl_->session_handle.deviceId = MC_DEVICE_ID_DEFAULT;
    pimpl_->tci.resize(DEFAULT_DEBUG_SESSION_SIZE);
    mcRet = mcOpenSession(&pimpl_->session_handle,
                          &uuid,
                          reinterpret_cast<uint8_t*>(&pimpl_->tci[0]),
                          static_cast<uint32_t>(pimpl_->tci.size()));

    if (MC_DRV_ERR_INVALID_OPERATION == mcRet) { //TODO verify
        // The tci can be too short, the SWd has maj the ideal tci size in the
        // corresponding tci buffer
        auto header = reinterpret_cast<const debugsession_header_t*>
                      (pimpl_->tci.c_str());

        LOG_W("Tci buffer might be too short, trying again with new size sent by SWd (size = %u bytes)",
              header->size);
        pimpl_->tci.resize(header->size);

        // Try to open the session with new size
        mcRet = mcOpenSession(&pimpl_->session_handle,
                              &uuid,
                              reinterpret_cast<uint8_t*>(&pimpl_->tci[0]),
                              static_cast<uint32_t>(pimpl_->tci.size()));

        if (MC_DRV_OK != mcRet) {
            LOG_E("Can't open session after two attempts");
            return -1;
        }
    } else if (MC_DRV_OK != mcRet) {
        LOG_E("mcOpenSession returned: 0x%08X\n", mcRet);
        return -1;
    }

    pimpl_->session_open = true;
    LOG_I("session opened");
    return 0;
}

void DebugSession::close() {
    mcResult_t mcRet;

    // Close session
    mcRet = mcCloseSession(&pimpl_->session_handle);
    if (MC_DRV_OK != mcRet) {
        LOG_E("%s: mcCloseSession returned: 0x%08X", __func__, mcRet);
    }
    memset(&pimpl_->session_handle, 0, sizeof(mcSessionHandle_t));
    pimpl_->session_open = false;

    // Close device
    mcRet = mcCloseDevice(MC_DEVICE_ID_DEFAULT);
    if (MC_DRV_OK != mcRet) {
        LOG_E("%s: mcCloseDevice returned: 0x%08X", __func__, mcRet);
    }

    LOG_I("closed");
}

void DebugSession::receiveSignal(int signum) {
    if ((signum == SIGUSR2) || (signum == 0)) {
        dump();
    }
}

int DebugSession::dump() {
    if (!pimpl_->session_open) {
        LOG_E("Can't dump the info page: the session isn't opened");
        return -1;
    }

    const char* file_name = DEBUG_SESSION_DUMP_FILE;
    FILE* fd = ::fopen(file_name, "w");
    if (fd == NULL) {
        LOG_ERRNO("open");
        return -1;
    }

    size_t res = ::fwrite(pimpl_->tci.c_str(), 1, pimpl_->tci.size(), fd);
    if (res != pimpl_->tci.size()) {
        LOG_ERRNO("dumping info page");
        ::fclose(fd);
        return -1;
    }

    if (::fclose(fd)) {
        LOG_ERRNO("close");
        return -1;
    }

#ifndef NDEBUG
    struct stat st;
    if (::stat(DEBUG_SESSION_DUMP_FILE, &st)) {
        LOG_ERRNO("stat");
        return -1;
    }
    LOG_D("Dump time %ld", st.st_mtime);
#endif

    LOG_I("Info page dumped in %s", file_name);

    return 0;
}

//------------------------------------------------------------------------------
