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

#include <iostream>
#include <fstream>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <libgen.h>
#include <pthread.h>

#include "MobiCoreDriverApi.h"
#include "mcSo.h"

#include "UpgradeStorage.h"

#include "tee_client_types.h"
#include "tee_client_error.h"

#include "drUpgradeStorage_Api.h"

#undef LOG_TAG
#define LOG_TAG "TeeUpgradeStorage"
#include "log.h"

extern const std::string& getTbStoragePath();

struct UpgradeStorage::Impl {
    pthread_t thread;
    SecureWorld& secure_world;
    mcSessionHandle_t sessionHandle;
    dciMessage_t* dci;
    uint32_t dci_len;

    Impl(SecureWorld& sw): thread(pthread_self()), secure_world(sw), dci(NULL),
        dci_len(0) {
        ::memset(&sessionHandle, 0, sizeof(sessionHandle));
    }
    void run();
    mcResult_t UpgradeFile(const char* sname, const char* fname, int level);
    mcResult_t UpgradeDir(const char* name, int level);
};

static void uuidSetToNil(
    TEEC_UUID* pIdentifier) {
    if (pIdentifier != NULL) {
        memset(pIdentifier, 0, sizeof(TEEC_UUID));
    }
}

static void mcuuid_to_tee_uuid(const mcUuid_t* in, TEEC_UUID* out) {
    out->timeLow = in->value[3] +
                   (in->value[2] << 8) +
                   (in->value[1] << 16) +
                   (in->value[0] << 24);
    out->timeMid = static_cast<uint16_t>(in->value[5] + (in->value[4] << 8));
    out->timeHiAndVersion =
        static_cast<uint16_t>(in->value[7] + (in->value[6] << 8));
    memcpy(out->clockSeqAndNode, in->value + 8, 8);
}

static bool isFileName(
    const char* pIdentifierString) {
    uint32_t nLen=0;
    if (pIdentifierString == NULL) {
        return false;
    }

    while (pIdentifierString[nLen] != 0) {
        char c = pIdentifierString[nLen++];

        if (c < '0') {
            return false;
        } else if (c <= '9') {
            continue;
        } else if (c < 'A') {
            return false;
        } else if (c <= 'F') {
            continue;
        } else if (c < 'a') {
            return false;
        } else if (c <= 'f') {
            continue;
        } else {
            return false;
        }
    }

    if (nLen == 40) {
        return true;
    }
    return false;
}

static bool uuidFromString(
        const uint8_t* pIdentifierString,
        TEEC_UUID* uuid_out) {
    if (uuid_out) {
        /* Initialize uuid_out to Nil in case of error */
        uuidSetToNil(uuid_out);
    }
    if (!pIdentifierString) {
        return false;
    }

    mcUuid_t uuid;
    ::memset(&uuid, 0, sizeof(uuid));
    auto pIdentifierCursor = reinterpret_cast<uint8_t*>(&uuid);
    /* A bit vector. 0 represents a dash. 1 a two-digit hex number */
    uint32_t syntax = 0xFFFF;
    while (syntax != 0) {
        if ((syntax & 1) == 0) {
            /* Dash expected */
            if (*pIdentifierString++ != '-') {
                return false;
            }
        } else {
            /* Two-digit hex number expected */
            uint32_t number = 0xF;
            do {
                uint8_t c = *pIdentifierString++;
                int digit;
                if (c < '0') {
                    return false;
                } else if (c <= '9') {
                    digit = c - '0';
                } else if (c < 'A') {
                    return false;
                } else if (c <= 'F') {
                    digit = c - 'A' + 0xA;
                } else if (c < 'a') {
                    return false;
                } else if (c <= 'f') {
                    digit = c - 'a' + 0xa;
                } else {
                    return false;
                }
                number = (number << 4) | (digit & 0xff);
            } while ((number & 0xF00) == 0);
            *pIdentifierCursor = (uint8_t)number;
            pIdentifierCursor++;
        }
        syntax >>= 1;
    }
    if (uuid_out) {
        mcuuid_to_tee_uuid(&uuid, uuid_out);
    }
    return true;
}

mcResult_t UpgradeStorage::Impl::UpgradeFile(const char* sname,
        const char* fname, int level) {
    mcResult_t    nError = MC_DRV_OK;

    char path[1024];
    int len;
    char* uuid;

    dciMessage_t* work_dci;
    mcSessionHandle_t* work_sessionHandle;

    work_dci = dci;
    work_sessionHandle = &sessionHandle;

    len = snprintf(path, sizeof(path)-1, "%s/%s", sname, fname);
    path[len] = 0;
    uuid = basename((char*)sname);

    LOG_I("[%s] %*s- %s (%s)", __func__, level*2, "", fname, uuid);

    if (!uuidFromString((uint8_t*)uuid, &work_dci->sth_request.uuid)) {
        LOG_I("[%s] unable to get uuid from %s", __func__, uuid);
    } else {
        std::ifstream fd(path, std::ios_base::binary);

        if (fd) {
            struct stat stat;

            int rc = ::stat(path, &stat);

            if ((rc == 0) && (stat.st_size < MC_SO_PAYLOAD_MAX_SIZE)) {
                fd.read(reinterpret_cast<char*>(work_dci->sth_request.payload), stat.st_size);
                LOG_I("[%s] %*s- file size %zu", __func__, level * 2, "", static_cast<size_t>(stat.st_size));
                fd.close();

                // Prepare command
                work_dci->sth_request.payloadLen = static_cast<uint32_t>(stat.st_size);
                work_dci->command.header.commandId = POTATO_UPGRADE_COMMAND_IMPORT;

                nError = mcNotify(work_sessionHandle);
                if (nError == MC_DRV_OK) {
retry:
                    nError = mcWaitNotification(work_sessionHandle, MC_INFINITE_TIMEOUT);
                    if (nError == MC_DRV_OK) {
                        LOG_I("[%s] %*s- %s notification id 0x%08X res 0x%08X", __func__, level*2, "",
                              fname, work_dci->response.header.responseId,
                              work_dci->response.header.returnCode);

                        if (work_dci->response.header.responseId == RSP_ID(
                                    POTATO_UPGRADE_COMMAND_IMPORT)) {
                            LOG_I("[%s] %*s- %s upgrade result 0x%08X", __func__, level*2, "", fname,
                                  work_dci->response.header.returnCode);
                            if (work_dci->response.header.returnCode == MC_DRV_OK) {
                                LOG_I("[%s] %*s- remove file %s", __func__, level*2, "", path);
                                if (unlink(path) !=0) {
                                    LOG_E("[%s] %*s- remove fail %s", __func__, level*2, "", strerror(errno));
                                }
                            } else {
                                LOG_E("[%s] %*s upgrade fail %s", __func__, level*2, "", path);
                            }
                            nError = work_dci->response.header.returnCode;
                            /* In case of a transient error (out of memory or out of storage space), stop performing any upgrades, don’t
                                rename or delete anything, and don’t report the upgrade as finished. We’ll naturally retry the upgrade on
                                the next boot. */
                            switch (nError) {
                                case TEEC_SUCCESS:
                                case TEEC_ERROR_BUSY:
                                case TEEC_ERROR_OUT_OF_MEMORY:
                                case TEEC_ERROR_STORAGE_NO_SPACE:
                                    break;
                                default: {
                                    char new_path[1024];
                                    len = snprintf(new_path, sizeof(new_path)-1, "%s.failed-upgrade-310", path);
                                    new_path[len] = 0;
                                    if (rename(path, new_path) != 0) {
                                        LOG_E("[%s] %*s- rename fail %s", __func__, level*2, "", strerror(errno));
                                    }
                                }
                                break;
                            }

                        } else {
                            LOG_E("[%s] unknown responseId 0x%08X", __func__,
                                  work_dci->response.header.responseId);
                            goto retry;
                        }
                    } else {
                        LOG_E("[%s] wait fail 0x%08X", __func__, nError);
                        nError = MC_DRV_ERR_UNKNOWN;
                    }
                }
            } else {
                LOG_E("[%s] unable to get file size %s", __func__, path);
                nError = MC_DRV_ERR_UNKNOWN;
            }
        } else {
            LOG_E("[%s] unable to open %s", __func__, path);
            nError = MC_DRV_ERR_UNKNOWN;
        }
    }

    return nError;
}

mcResult_t UpgradeStorage::Impl::UpgradeDir(const char* name, int level) {
    mcResult_t nError = MC_DRV_OK;
    DIR* dir;
    struct dirent* entry;

    LOG_I("[%s] >%*s[%s]", __func__, level*2, "", name);

    if (!(dir = ::opendir(name))) {
        LOG_ERRNO("opendir");
        return MC_DRV_ERR_UNKNOWN;
    }
    if (!(entry = ::readdir(dir))) {
        LOG_ERRNO("readdir");
        ::closedir(dir);
        return MC_DRV_ERR_UNKNOWN;
    }

    do {
        if (entry->d_type == DT_DIR) {
            if (strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0) {
                if (uuidFromString((uint8_t*)entry->d_name, NULL)) {
                    char path[1024];
                    int len = snprintf(path, sizeof(path)-1, "%s/%s", name, entry->d_name);
                    path[len] = 0;

                    nError = UpgradeDir(path, level + 1);
                }
            }
        } else {
            if (isFileName(entry->d_name)) {
                nError = UpgradeFile(name, entry->d_name, level);
            }
        }
    } while ((nError==0) && (entry = readdir(dir)));
    ::closedir(dir);

    if (uuidFromString((uint8_t*)basename((char*)name), NULL)) {
        /* In case of a transient error (out of memory or out of storage space), stop performing any upgrades, don’t
            rename or delete anything, and don’t report the upgrade as finished. We’ll naturally retry the upgrade on
            the next boot. */
        switch (nError) {
            case TEEC_ERROR_BUSY:
            case TEEC_ERROR_OUT_OF_MEMORY:
            case TEEC_ERROR_STORAGE_NO_SPACE:
                break;
            default: {
                LOG_I("[%s] %*s- remove dir %s", __func__, level*2, "", name);
                if (rmdir(name) != 0) {
                    char new_name[1024];
                    uint32_t len = snprintf(new_name, sizeof(new_name)-1, "%s.failed-upgrade-310",
                                            name);
                    new_name[len] = 0;
                    if (rename(name, new_name) != 0) {
                        LOG_E("[%s] %*s- rename fail %s", __func__, level*2, "", strerror(errno));
                    }
                    LOG_E("[%s] %*s remove fail %s", __func__, level*2, "", strerror(errno));
                }
            }
            break;
        }
    }

    LOG_I("[%s] <%*s[%s]", __func__, level*2, "", name);

    return nError;
}

UpgradeStorage::UpgradeStorage(SecureWorld& secure_world):
    pimpl_(new Impl(secure_world)) {
    //secure_world.blockGpLoading();
}

UpgradeStorage::~UpgradeStorage() {
    delete pimpl_;
}

void* UpgradeStorage::run(void* arg) {
    UpgradeStorage::Impl* pimpl = static_cast<UpgradeStorage::Impl*>(arg);
    pimpl->run();
    return NULL;
}

int UpgradeStorage::open() {
    mcResult_t nError;

    LOG_I("%s: open upgrade session", __func__);

    /* Prepare DCI message buffer */
    // Allocate and clear WSM buffer
    pimpl_->dci_len = static_cast<uint32_t>(sizeof(dciMessage_t) +
                                            MC_SO_PAYLOAD_MAX_SIZE);
    nError = mcMallocWsm(MC_DEVICE_ID_DEFAULT, 0, pimpl_->dci_len,
                         (uint8_t**) &pimpl_->dci,
                         0);
    if (nError != MC_DRV_OK) {
        return -1;
    }
    ::memset(pimpl_->dci, 0, pimpl_->dci_len);

    mcResult_t mc_ret = mcOpenDevice(MC_DEVICE_ID_DEFAULT);
    if (MC_DRV_OK != mc_ret) {
        LOG_E("%s: mcOpenDevice returned: 0x%08X\n", __func__, mc_ret);
        return -1;
    }

    const mcUuid_t uuid = DRV_STHU_MCUUID;
    ::memset(&pimpl_->sessionHandle, 0, sizeof(pimpl_->sessionHandle));
    pimpl_->sessionHandle.deviceId = MC_DEVICE_ID_DEFAULT;
    mc_ret = mcOpenSession(&pimpl_->sessionHandle, &uuid,
                           reinterpret_cast<uint8_t*>(pimpl_->dci), pimpl_->dci_len);
    if (MC_DRV_OK != mc_ret) {
        LOG_E("%s: mcOpenSession returned: 0x%08X\n", __func__, mc_ret);
        mcCloseDevice(MC_DEVICE_ID_DEFAULT);
        return -1;
    }

    // Create listening thread
    int ret = pthread_create(&pimpl_->thread, NULL, run, pimpl_);
    if (ret) {
        LOG_E("pthread_create failed with error code %d", ret);
        return -1;
    }

    pthread_setname_np(pimpl_->thread, "McDaemon.UpgradeStorage");
    return 0;
}

void UpgradeStorage::close() {
    mcResult_t mc_ret;

    LOG_I("%s: close upgrade session", __func__);

    // Stop listening thread
    pthread_kill(pimpl_->thread, SIGUSR1);
    pthread_join(pimpl_->thread, NULL);

    mc_ret = mcCloseSession(&pimpl_->sessionHandle);
    if (MC_DRV_OK != mc_ret) {
        LOG_E("%s: mcCloseSession returned: 0x%08X\n", __func__, mc_ret);
    }

    mc_ret = mcCloseDevice(MC_DEVICE_ID_DEFAULT);
    if (MC_DRV_OK != mc_ret) {
        LOG_E("%s: mcCloseDevice returned: 0x%08X\n", __func__, mc_ret);
    }

    /* Clear DCI message buffer */
    ::memset(pimpl_->dci, 0, pimpl_->dci_len);
    mc_ret = mcFreeWsm(MC_DEVICE_ID_DEFAULT, (uint8_t*)pimpl_->dci);
    if (MC_DRV_OK != mc_ret) {
        LOG_E("%s: mcFreeWsm failed with status: %d\n", __func__, mc_ret);
    }
    pimpl_->dci = NULL;
}

/*
 * main
 */
void UpgradeStorage::Impl::run() {
    mcResult_t nError;
    const std::string storage_dir_name = getTbStoragePath();

    dciMessage_t* work_dci;
    mcSessionHandle_t* work_sessionHandle;

    work_dci = dci;
    work_sessionHandle = &sessionHandle;

    LOG_I("%s: Starting UpgradeStorage thread", __func__);

    // Only accept USR1, to give up
    sigset_t sigmask;
    sigemptyset(&sigmask);
    sigaddset(&sigmask, SIGUSR1);
    pthread_sigmask(SIG_UNBLOCK, &sigmask, (sigset_t*)0);

    nError = mcWaitNotification(work_sessionHandle, MC_INFINITE_TIMEOUT);
    if (nError == MC_DRV_OK) {
        LOG_I("%s: UpgradeStorage TA is ready", __func__);
        nError = UpgradeDir(storage_dir_name.c_str(), 0);
    }

    //secure_world.unblockGpLoading();

    /* In case of a transient error (out of memory or out of storage space), stop performing any upgrades, don’t
        rename or delete anything, and don’t report the upgrade as finished. We’ll naturally retry the upgrade on
        the next boot. */
    switch (nError) {
        case TEEC_ERROR_BUSY:
        case TEEC_ERROR_OUT_OF_MEMORY:
        case TEEC_ERROR_STORAGE_NO_SPACE:
            break;
        default:
            work_dci->sth_request.payloadLen = 0;
            work_dci->command.header.commandId = POTATO_UPGRADE_COMMAND_LOCK;
            nError = mcNotify(work_sessionHandle);
            if (nError == MC_DRV_OK) {
retry:
                nError = mcWaitNotification(work_sessionHandle, MC_INFINITE_TIMEOUT);
                if (nError == MC_DRV_OK) {
                    if (work_dci->response.header.responseId == RSP_ID(
                                POTATO_UPGRADE_COMMAND_LOCK)) {
                        if (work_dci->response.header.returnCode != MC_DRV_OK) {
                            LOG_E("[%s] lock fail 0x%08X", __func__, work_dci->response.header.returnCode);
                        }
                    } else {
                        LOG_E("[%s] unknown responseId 0x%08X", __func__,
                              work_dci->response.header.responseId);
                        goto retry;
                    }
                } else {
                    LOG_E("[%s] wait fail 0x%08X", __func__, nError);
                    nError = MC_DRV_ERR_UNKNOWN;
                }
            }
            break;
    }

    LOG_I("%s: Exiting UpgradeStorage thread", __func__);
}
