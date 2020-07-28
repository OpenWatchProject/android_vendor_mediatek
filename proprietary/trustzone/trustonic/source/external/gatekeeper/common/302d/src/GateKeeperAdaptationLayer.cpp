/*
 * Copyright (c) 2015 TRUSTONIC LIMITED
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
#include <fstream>
#include <sstream>
#include <utility>
#include <iomanip>

#include "TeeSession.h"
#include "GateKeeperAdaptationLayer.h"
#include "log.h"

//Reference code for using fsync when writing .rec file
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>

#define GATEKEEPER_RPMB    false

#if !GATEKEEPER_RPMB
namespace {
    std::string CreateRecordFilePath(const uint32_t uid)
    {
        std::ostringstream stream;
        stream  << RECORD_DIRECTORY
                << std::setw(21)
                << std::setfill('0')
                << uid
                << ".rec";
        return stream.str();
    }
}
#endif

GateKeeperAdaptationLayer::GateKeeperAdaptationLayer()
: tee_(new TeeSession) {}

GateKeeperAdaptationLayer::~GateKeeperAdaptationLayer()
{
    delete tee_;
}

bool GateKeeperAdaptationLayer::Open()
{
    if( tee_->IsOpened() )
    {
        LOG_E("Session alredy opened");
        return false;
    }
    return TEE_OK == tee_->Open();
}

bool GateKeeperAdaptationLayer::Close()
{
    if( !tee_->IsOpened() )
    {
        LOG_E("Session not opened");
        return false;
    }
    return TEE_OK == tee_->Close();
}

uint64_t GateKeeperAdaptationLayer::GetMillisecondsSinceBoot() const
{
    uint64_t uptime = 0;
    const TEE_ReturnCode_t res = tee_->GetMillisecondsSinceBoot(uptime);
    if( TEE_OK != res )
    {
        LOG_E("Communication with TEE failed %d", res);
    }
    return uptime;
}

bool GateKeeperAdaptationLayer::ClearFailureRecord(uint32_t uid,
                                    gatekeeper::secure_id_t user_id,
                                    bool secure)
{
    // non secure failure records are not supported
    if(!secure)
        return false;

    tee_failure_record_t failure_record;
    failure_record.version = TEE_FAILURE_RECORD_VERSION;
    failure_record.suid = user_id;
    failure_record.last_access_timestamp = 0;
    failure_record.failure_counter = 0;
    return doWriteFailureRecord(uid, failure_record, false);
}

bool GateKeeperAdaptationLayer::WriteFailureRecord(uint32_t uid,
                                    gatekeeper::failure_record_t *record,
                                    bool secure)
{
    // non secure failure records are not supported
    if(!secure)
        return false;

    tee_failure_record_t failure_record;
    failure_record.version = TEE_FAILURE_RECORD_VERSION;
    failure_record.suid = record->secure_user_id;
    failure_record.last_access_timestamp = record->last_checked_timestamp;
    failure_record.failure_counter = record->failure_counter;
    return doWriteFailureRecord(uid, failure_record, true);
}

bool GateKeeperAdaptationLayer::GetFailureRecord(  uint32_t uid,
                                        gatekeeper::secure_id_t user_id,
                                        gatekeeper::failure_record_t *record,
                                        bool secure)
{
    // non secure failure records are not supported
    if(!secure)
        return false;
#if GATEKEEPER_RPMB
    tee_failure_record_t failure_record = {0,0,0,0,{0,0}};
    TEE_ReturnCode_t ret = tee_->RetrieveFailureRecord(uid, user_id, failure_record);

    if (TEE_OK != ret)
    {
        LOG_E("[GATEKEEPER] Can't retrieve failure record from RPMB: ret = %d\n", ret);
        return false;
    }
#else
    uint8_t so_buff[ ENC_FAILURE_RECORD_LEN ];

    // Read SO from a file
    const std::string record_file_path = CreateRecordFilePath(uid);
    std::fstream record_file;

    record_file.open(record_file_path.c_str(),std::ifstream::in | std::fstream::binary);
    if(!record_file.is_open())
    {
        LOG_E("Can't open failure record file [%s]", record_file_path.c_str());
        return ResetFailureRecord(uid, user_id, record, secure);
    }

    bool read_op_flag = false;
    record_file.read((char*)so_buff, ENC_FAILURE_RECORD_LEN);
    read_op_flag = record_file.good();
    record_file.close();
    if(!read_op_flag)
    {
        LOG_E("Can't read failure record file [%s]", record_file_path.c_str());
        return ResetFailureRecord(uid, user_id, record, secure);
    }

    // Decode failure record
    tee_failure_record_t failure_record = {0,0,0,0,{0,0}};
    TEE_ReturnCode_t ret = tee_->DecodeFailureRecord(user_id,
                                    so_buff, ENC_FAILURE_RECORD_LEN, failure_record);

    if( TEE_OK != ret )
    {
        LOG_E(  "Decoding of failure record has failed [%s]",
                record_file_path.c_str());
        return false;
    }
#endif
    record->secure_user_id = failure_record.suid;
    record->last_checked_timestamp = failure_record.last_access_timestamp;
    record->failure_counter = failure_record.failure_counter;
    return true;
}

bool GateKeeperAdaptationLayer::doWriteFailureRecord(
                                            uint32_t uid,
                                            const tee_failure_record_t& failure_record,
                                            bool check_exists)
{
#if GATEKEEPER_RPMB
    check_exists = false;

    TEE_ReturnCode_t ret = tee_->StoreFailureRecord(uid, failure_record);
    if (TEE_OK != ret)
    {
        LOG_E("[GATEKEEPER] Can't store failure record to RPMB: ret = %d\n", ret);
        return false;
    }
#else
    uint8_t so_buff[ ENC_FAILURE_RECORD_LEN ];

    if( TEE_OK != tee_->EncodeFailureRecord(failure_record, &so_buff[0], ENC_FAILURE_RECORD_LEN) )
        return false;

    const std::string record_file_path = CreateRecordFilePath(uid);

    // Old file not needed. This way we avoid symlink race and check access rights
    if(check_exists)
    {
        int sys_ret = ::unlink(record_file_path.c_str());
        if (sys_ret != 0 && errno != ENOENT)
        {
            LOG_E("Can't remove old failure record file [%d][%s].", sys_ret, record_file_path.c_str());
            return false;
        }
    }

    // Write SO to a file
    std::fstream record_file;
    record_file.open(record_file_path.c_str(),std::ofstream::out | std::ofstream::binary);
    if(!record_file.is_open())
    {
        LOG_E("Can't create failure record file.");
        return false;
    }

    bool write_op_flag = false;
    record_file.write((char*)so_buff, ENC_FAILURE_RECORD_LEN);
    write_op_flag = record_file.good();
    record_file.close();
    if(!write_op_flag)
    {
        LOG_E("Problem occured when writing failure record to a file.");
        return false;
    }
    //Call fsync after fstream is closed.
    //use fsync system call to sync buffer to flash
    int fd = open(record_file_path.c_str(), O_WRONLY);
    if (fd >= 0)
    {
        fsync(fd);
        close(fd);
    }
#endif
    return true;
}

bool GateKeeperAdaptationLayer::ResetFailureRecord(
                                            uint32_t uid,
                                            gatekeeper::secure_id_t user_id,
                                            gatekeeper::failure_record_t *record,
                                            bool secure)
{
    LOG_I("Reset failure record to prevent cannot unlock device forever...");
    record->secure_user_id = user_id;

    /* Need to wait for 30 seconds to unlock device */
    /* record->last_checked_timestamp = GetMillisecondsSinceBoot(); */
    /* record->failure_counter = 10; */

    /* No need to wait for 30 seconds to unlock device */
    record->last_checked_timestamp = 0;
    record->failure_counter = 0;

    return WriteFailureRecord(uid, record, secure);
}
