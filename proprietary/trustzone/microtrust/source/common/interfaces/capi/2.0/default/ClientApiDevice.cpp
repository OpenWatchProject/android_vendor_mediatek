/*
 * Copyright (c) 2015-2016 MICROTRUST Incorporated
 * All rights reserved
 *
 * This file and software is confidential and proprietary to MICROTRUST Inc.
 * Unauthorized copying of this file and software is strictly prohibited.
 * You MUST NOT disclose this file and software unless you get a license
 * agreement from MICROTRUST Incorporated.
 */
#define IMSG_TAG "vendor.microtrust.hardware.capi@2.0-impl"

#include "ClientApiDevice.h"
#include <android/hidl/memory/1.0/IMemory.h>
#include <hidlmemory/mapping.h>
#include <imsg_log.h>

#include <hwbinder/Parcel.h>
#include <include/tee_client_api.h>
#include <stddef.h>
#include <stdint.h>
#include <sys/cdefs.h>

namespace vendor
{
namespace microtrust
{
namespace hardware
{
namespace capi
{
namespace V2_0
{
namespace implementation
{

using ::android::hardware::hidl_memory;
using ::android::hidl::memory::V1_0::IMemory;

using ::android::hardware::Return;
using ::android::hardware::Void;
using ::android::hardware::Parcel;

#define MAX_CONTEXT_NAME_LENGTH 129
#define SERVER_LOG_TAG "ClientApiDevice"

enum
{
    CLIENT_API_INITIALIZE_CONTEXT_CMD = 0,
    CLIENT_API_ALLOC_SHARED_MEMORY_CMD,
    CLIENT_API_SES_OPEN_REQ,
    CLIENT_API_SES_CLOSE_REQ,
    CLIENT_API_ALLOC_SHARED_RIGSTER_FD_CMD,
    CLIENT_API_INVOKE_CMD,
    CLIENT_API_REQUEST_CANCEL_CMD,
    CLIENT_API_FD_CLOSE_CMD,
    CLIENT_API_SHM_GET_PA_CMD,
    CLIENT_API_SHM_FLUSH_CACHE_CMD,
};
ClientApiDevice::~ClientApiDevice()
{
}

ClientApiDevice::ClientApiDevice(android::sp<android::UtClientApiDaemonProxy> _proxy)
{
    proxy = _proxy;
}

status_t ClientApiDevice::onTransact(uint32_t code, const Parcel &data, Parcel *reply, uint32_t flags)
{
    status_t ret = android::NO_ERROR;
    IMSG_DEBUG("%s server cmd received: %d", SERVER_LOG_TAG, code);

    switch (code)
    {
    case CLIENT_API_INITIALIZE_CONTEXT_CMD:
    {
        char name[MAX_CONTEXT_NAME_LENGTH];
        data.read(name, sizeof(name));
        IMSG_DEBUG("%s init context hostname: %s", SERVER_LOG_TAG, name);
        TEEC_Context context;
        memset(&context, 0x0, sizeof(TEEC_Context));
        TEEC_Result result = proxy->ut_initialize_context(name, &context);
        reply->write(&context, sizeof(TEEC_Context));
        reply->writeUint32(result);
        return ret;
    }
    case CLIENT_API_SES_OPEN_REQ:
    {
        int32_t fd = data.readInt32();
        uint32_t buf_len = data.readUint32();
        unsigned char *buf = (unsigned char *)malloc(buf_len);
        if (!buf)
        {
            return TEEC_ERROR_OUT_OF_MEMORY;
        }
        data.read(buf, buf_len);
        ret = proxy->ut_open_session(fd, (uintptr_t)buf, buf_len);
        reply->write(buf, buf_len);
        reply->writeInt32(ret);
        IMSG_DEBUG("[E]ut_open_session returned ! ret = %d.", ret);
        free(buf);
        return ret;
    }
    case CLIENT_API_INVOKE_CMD:
    {
        int32_t fd = data.readInt32();
        uint32_t buf_len = data.readUint32();
        unsigned char *buf = (unsigned char *)malloc(buf_len);
        if (!buf)
        {
            return TEEC_ERROR_OUT_OF_MEMORY;
        }
        data.read(buf, buf_len);
        ret = proxy->ut_invoke_command(fd, (uintptr_t)buf, buf_len);
        reply->write(buf, buf_len);
        reply->writeInt32(ret);
        free(buf);
        return ret;
    }
    case CLIENT_API_SES_CLOSE_REQ:
    {
        TEEC_Context context;
        TEEC_Session session;
        session.ctx = &context;
        int fd = data.readInt32();
        int session_id = data.readInt32();
        session.ctx->fd = fd;
        session.session_id = session_id;
        proxy->ut_close_session(&session);
        return ret;
    }
    case CLIENT_API_REQUEST_CANCEL_CMD:
    {
        int fd = data.readInt32();
        struct tee_ioctl_cancel_arg arg;
        data.read(&arg, sizeof(tee_ioctl_cancel_arg));
        proxy->ut_request_cancel(fd, &arg);
        return ret;
    }
    case CLIENT_API_FD_CLOSE_CMD:
    {
        int fd = data.readInt32();
        if (fd > 0)
        {
            if (close(fd))
            {
                IMSG_ERROR("%s close fd failed, errno: %d", SERVER_LOG_TAG, errno);
            }
        }
        else
        {
            IMSG_ERROR("%s wrong fd received.", SERVER_LOG_TAG);
        }

        return ret;
    }
    case CLIENT_API_SHM_GET_PA_CMD:
    {
        int32_t ret = 0;
        int fd = data.readInt32();
        int32_t shmid = data.readInt32();
        uint64_t pa = 0;
        ret = proxy->ut_shm_get_pa(fd, shmid, &pa);
        IMSG_DEBUG("%s capi get paddr %llx", SERVER_LOG_TAG, pa);
        reply->writeUint64(pa);
        reply->writeInt32(ret);
        return ret;
    }
    case CLIENT_API_SHM_FLUSH_CACHE_CMD:
    {
        int32_t ret = 0;
        int fd = data.readInt32();
        int32_t shmid = data.readInt32();
        ret = proxy->ut_shm_flush_cache(fd, shmid);
        IMSG_DEBUG("%s capi flush cache", SERVER_LOG_TAG);
        reply->writeInt32(ret);
        return ret;
    }
    default:
        IMSG_ERROR("%s default transact", SERVER_LOG_TAG);
        return -1;
    }
}

Return<ErrorCode> ClientApiDevice::capi_transact(uint32_t msgid, const hidl_memory &data)
{
    IMSG_DEBUG("capi_transact start．");
    int32_t ret = 0;

    uint32_t res_len = 0;
    uint8_t *buffer = NULL;
    uint32_t buffer_len = 0;
    uint32_t max_mem_len = 0;
    sp<IMemory> share_mem = nullptr;

    Parcel p_data, p_reply;

    share_mem = mapMemory(data);
    if (share_mem == nullptr)
    {
        IMSG_ERROR("Could not map data hidl_memory．");
        return ErrorCode(-1);
    }
    else
    {
        IMSG_DEBUG("map hidl_memory success．");
    }

    max_mem_len = share_mem->getSize();
    buffer = static_cast<uint8_t *>(static_cast<void *>(share_mem->getPointer()));

    memcpy(&buffer_len, buffer, sizeof(buffer_len));

    if (sizeof(buffer_len) + buffer_len > max_mem_len)
    {
        return ErrorCode(-1);
    }
    p_data.setData(buffer + sizeof(uint32_t), buffer_len);
    p_data.setDataPosition(0);
    IMSG_DEBUG("onTransact start．");
    ret = (int32_t)onTransact(msgid, p_data, &p_reply, 0);
    IMSG_DEBUG("onTransact end．");
    res_len = p_reply.dataSize();
    if (android::NO_ERROR == ret && res_len > 0)
    {
        if (sizeof(res_len) + res_len > max_mem_len)
        {
            IMSG_ERROR("Shared memory is not enough．");
            return ErrorCode(-1);
        }
        share_mem->update();
        memcpy(buffer, &res_len, sizeof(res_len));
        memcpy(buffer + sizeof(res_len), p_reply.data(), res_len);
        share_mem->commit();
    }
    return ErrorCode(ret);
}

Return<void> ClientApiDevice::createMmapBuffer(int32_t fd, uint64_t memory_size, createMmapBuffer_cb _hidl_cb)
{
    ErrorCode retval;
    MmapBufferInfo info;
    int32_t shmid = 0;
    int32_t shmfd = 0;
    native_handle_t *hidlHandle = nullptr;
    //TODO parameter check
    shmfd = proxy->ut_shm_alloc(fd, memory_size, &shmid);
    if (shmfd > 0)
    {
        hidlHandle = native_handle_create(1, 0);
        hidlHandle->data[0] = shmfd;
        info.sharedMemory = hidl_memory("capi_buffer", hidlHandle, memory_size);
        info.bufferSize = memory_size;
        retval = ErrorCode(0);
    }
    else
    {
        retval = ErrorCode(-1);
    }
    _hidl_cb(retval, shmid, info);
    if (hidlHandle != nullptr)
    {
        close(shmfd);
        native_handle_delete(hidlHandle);
    }
    return Void();
}

Return<void> ClientApiDevice::registerSharedFileDescriptor(int32_t fd, int32_t reg_fd, registerSharedFileDescriptor_cb _hidl_cb)
{
    ErrorCode retval;
    MmapBufferInfo info;
    int32_t shmfd = 0;
    native_handle_t *hidlHandle = nullptr;
    struct tee_ioctl_shm_register_fd_data fd_data;
    //TODO parameter check
    fd_data.fd = reg_fd;
    shmfd = proxy->ut_shm_register_fd(fd, &fd_data);
    if (shmfd > 0)
    {
        hidlHandle = native_handle_create(1, 0);
        hidlHandle->data[0] = shmfd;
        info.sharedMemory = hidl_memory("capi_buffer", hidlHandle, fd_data.size);
        info.bufferSize = fd_data.size;
        retval = ErrorCode(0);
    }
    else
    {
        retval = ErrorCode(-1);
    }
    _hidl_cb(retval, info);
    if (hidlHandle != nullptr)
    {
        close(shmfd);
        native_handle_delete(hidlHandle);
    }
    return Void();
}

} // namespace implementation
} // namespace V2_0
} // namespace capi
} // namespace hardware
} // namespace microtrust
} // namespace vendor
