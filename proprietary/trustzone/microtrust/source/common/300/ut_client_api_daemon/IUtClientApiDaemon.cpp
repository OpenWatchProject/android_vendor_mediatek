/*
 * Copyright (c) 2015-2016 MICROTRUST Incorporated
 * All rights reserved
 *
 * This file and software is confidential and proprietary to MICROTRUST Inc.
 * Unauthorized copying of this file and software is strictly prohibited.
 * You MUST NOT disclose this file and software unless you get a license
 * agreement from MICROTRUST Incorporated.
 */
#define IMSG_TAG "ClientApi"

/* clang-format off */
#include "IUtClientApiDaemon.h"
//#include <limits>
//#include <bits/unique_ptr.h>

#include <cutils/ashmem.h>
#include <fcntl.h>
#include <hwbinder/Parcel.h>
#include <imsg_log.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <time.h>
#include <utils/Looper.h>
#include <utils/String16.h>


#include <android/hidl/allocator/1.0/IAllocator.h>
#include <android/hidl/memory/1.0/IMemory.h>
#include <hidlmemory/mapping.h>
#include <tee_client_api.h>
#include <vendor/microtrust/hardware/capi/2.0/IClientApiDevice.h>

#include "include/tee_client_api.h"
/* clang-format on */
#define MAX_CONTEXT_NAME_LENGTH 129
#define DEFAULT_CAPABILITY "bta_loader"
#define GPTEE_CAPABILITY "tta"

using ::vendor::microtrust::hardware::capi::V2_0::ErrorCode;
using ::vendor::microtrust::hardware::capi::V2_0::IClientApiDevice;
using ::vendor::microtrust::hardware::capi::V2_0::MmapBufferInfo;
using ::android::hardware::Return;
using ::android::hardware::hidl_vec;
using ::android::hardware::hidl_memory;
using ::android::hardware::Parcel;
using ::android::hidl::allocator::V1_0::IAllocator;
using ::android::hidl::memory::V1_0::IMemory;
using ::android::sp;

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
int32_t process_message_sharemem(uint32_t msgid, const uint8_t *req,
                                 uint32_t req_len, uint8_t **res, uint32_t *res_len)
{
    if (NULL == req || req_len < 1 || NULL == res || NULL == res_len)
    {
        IMSG_ERROR("Parameter missing.");
        return -1;
    }

    ErrorCode error = ErrorCode::ERROR_UNKNOWN;
    sp<IClientApiDevice> dev = nullptr;
    sp<IAllocator> ashmemAllocator = nullptr;
    hidl_memory memory;
    sp<IMemory> share_mem = nullptr;
    bool is_allocator_success = false;
    uint8_t *data = NULL;

    dev = IClientApiDevice::getService();
    if (dev == nullptr)
    {
        IMSG_ERROR("Access capi service failed.");
        return static_cast<int32_t>(error);
    }
    ashmemAllocator = IAllocator::getService("ashmem");
    if (ashmemAllocator == nullptr)
    {
        IMSG_ERROR("Access ashmem service failed.");
        return static_cast<int32_t>(error);
    }
    //alloc input share memory
    Return<void> rc = ashmemAllocator->allocate(req_len + sizeof(req_len),
                                                [&](bool success, const hidl_memory &mem) {
                                                    if (success)
                                                    {
                                                        is_allocator_success = success;
                                                        memory = mem;
                                                    }
                                                });
    if (!rc.isOk())
    {
        error = ErrorCode::ERROR_MEMORY_ALLOCATION_FAILED;
        IMSG_ERROR("HIDL call failed with %s", rc.description().c_str());
        goto RELEASE;
    }

    if (!is_allocator_success)
    {
        IMSG_ERROR("ashmem allocate memory failed");
        error = ErrorCode::ERROR_MEMORY_ALLOCATION_FAILED;
        goto RELEASE;
    }

    share_mem = mapMemory(memory);
    if (share_mem == nullptr)
    {
        IMSG_ERROR("mapMemory failed");
        error = ErrorCode::ERROR_MEMORY_ALLOCATION_FAILED;
        goto RELEASE;
    }

    data = static_cast<uint8_t *>(static_cast<void *>(share_mem->getPointer()));
    if (data == nullptr)
    {
        IMSG_ERROR("getPointer failed");
        error = ErrorCode::ERROR_MEMORY_ALLOCATION_FAILED;
        goto RELEASE;
    }

    if (share_mem->getSize() != memory.size())
    {
        IMSG_ERROR("memory size is not the expected value");
        error = ErrorCode::ERROR_MEMORY_ALLOCATION_FAILED;
        goto RELEASE;
    }

    share_mem->update();
    memcpy(data, &req_len, sizeof(req_len));
    memcpy(data + sizeof(req_len), req, req_len);
    share_mem->commit();

    error = dev->capi_transact(msgid, memory);
    if (error != ErrorCode::OK)
    {
        IMSG_ERROR("processMessage failed ,err:%d", error);
        goto RELEASE;
    }

    share_mem->read();
    memcpy(res_len, data, sizeof(uint32_t));
    if (*res_len > 0)
    {
        *res = reinterpret_cast<uint8_t *>(malloc(*res_len));
        if (NULL == *res)
        {
            IMSG_ERROR("Error allocation buffer for output");
            error = ErrorCode::ERROR_UNKNOWN;
            goto RELEASE;
        }
        IMSG_DEBUG("res len is %d", *res_len);
        IMSG_DEBUG("first int is %d", *((int32_t *)(data + sizeof(uint32_t))));
        memcpy(*res, data + sizeof(uint32_t), *res_len);
    }
    share_mem->commit();

RELEASE:
    return static_cast<int32_t>(error);
}
int32_t transact(uint32_t msgid, const Parcel &data, Parcel *reply)
{
    const uint8_t *parcel_data;
    const uint8_t *parcel_reply;
    uint8_t *res = 0;
    uint32_t reply_len = 0;
    int32_t ret;

    parcel_data = data.data();
    ret = process_message_sharemem(msgid, parcel_data, data.dataSize(), &res, &reply_len);
    reply->write(res, reply_len);
    reply->setDataPosition(0);
    if (res)
    {
        free(res);
    }
    return ret;
}

TEEC_Result ut_initialize_context(const char *name, TEEC_Context *context)
{
    Parcel data;
    Parcel reply;

    int32_t ret;
    if (!context)
    {
        IMSG_ERROR("[E]ut_initializeContext receive Bad parameter.");
        return TEEC_ERROR_BAD_PARAMETERS;
    }

    char name_buffer[MAX_CONTEXT_NAME_LENGTH];
    memset(name_buffer, 0, sizeof(name_buffer));
    if (strlen(name) > 128)
    {
        return TEEC_ERROR_BAD_PARAMETERS;
    }

#ifndef DEFAULT_TEE_GPTEE
    if (!name)
    {
        name = (char *)DEFAULT_CAPABILITY;
    }
    else
    {
        if (strcmp(name, GPTEE_CAPABILITY) == 0)
            name = (char *)GPTEE_CAPABILITY;
        else
            name = (char *)DEFAULT_CAPABILITY;
    }
#else
    if (!name)
    {
        name = (char *)GPTEE_CAPABILITY;
    }
    else
    {
        if (strcmp(name, DEFAULT_CAPABILITY) == 0)
            name = (char *)DEFAULT_CAPABILITY;
        else
            name = (char *)GPTEE_CAPABILITY;
    }
#endif

    ret = data.write(name_buffer, sizeof(name_buffer));

    transact(CLIENT_API_INITIALIZE_CONTEXT_CMD, data, &reply);
    reply.read(context, sizeof(TEEC_Context));

    //TODO
    TEEC_Result result = reply.readUint32();

    return result;
}

TEEC_Result ut_open_session(int32_t fd, uintptr_t buf, uint32_t buf_len)
{
    Parcel data, reply;
    TEEC_Result ret;

    data.writeInt32(fd);
    data.writeUint32(buf_len);
    data.write((const void *)buf, buf_len);
    IMSG_DEBUG("[E]context fd is  %d.", fd);
    transact(CLIENT_API_SES_OPEN_REQ, data, &reply);
    reply.read((void *)buf, buf_len);
    ret = reply.readInt32();
    IMSG_DEBUG("[E]ut_open_session returned ! ret = %d.", ret);
    return ret;
}

TEEC_Result ut_invoke_command(int32_t fd, uintptr_t buf, uint32_t buf_len)
{
    Parcel data, reply;
    TEEC_Result ret;

    data.writeInt32(fd);
    data.writeUint32(buf_len);
    data.write((const void *)buf, buf_len);
    transact(CLIENT_API_INVOKE_CMD, data, &reply);
    reply.read((void *)buf, buf_len);
    ret = reply.readInt32();
    return ret;
}

void ut_request_cancel(int32_t fd, struct tee_ioctl_cancel_arg *arg)
{

    Parcel data, reply;
    TEEC_Result ret;
    if (!arg)
    {
        return;
    }

    data.writeInt32(fd);
    data.write(arg, sizeof(tee_ioctl_cancel_arg));
    transact(CLIENT_API_REQUEST_CANCEL_CMD, data, &reply);
}

void ut_close_session(TEEC_Session *session)
{
    Parcel data, reply;
    int32_t ret;
    data.writeInt32(session->ctx->fd);
    data.writeInt32(session->session_id);
    transact(CLIENT_API_SES_CLOSE_REQ, data, &reply);
    return;
}
void ut_close_fd(int fd)
{
    Parcel data, reply;
    data.writeInt32(fd);
    transact(CLIENT_API_FD_CLOSE_CMD, data, &reply);
    return;
}

static int32_t createMmapBuffer(int32_t fd, uint64_t *memory_size, int32_t *id)
{
    if (fd < 1 || NULL == memory_size || NULL == id)
    {
        IMSG_ERROR("Parameter missing. fd %d memory_size %d id %d", fd, memory_size, id);
        return -1;
    }
    int32_t shmfd;
    ErrorCode error = ErrorCode::ERROR_UNKNOWN;
    ErrorCode retval;
    sp<IClientApiDevice> dev = nullptr;

    dev = IClientApiDevice::getService();
    if (dev == nullptr)
    {
        IMSG_ERROR("Access capi service failed.");
        return static_cast<int32_t>(error);
    }
    auto hidlCb = [&](ErrorCode ret, uint32_t shmid, const MmapBufferInfo &hidlInfo) {
        retval = ret;
        if (retval == ErrorCode::OK)
        {
            const native_handle *handle = hidlInfo.sharedMemory.handle();
            if (handle->numFds > 0)
            {
                shmfd = handle->data[0];
            }
            else
            {
                retval = ErrorCode::ERROR_OUTPUT_PARAMETER_NULL;
            }
            *id = shmid;
            *memory_size = hidlInfo.bufferSize;
        }
        else
        {
            IMSG_ERROR("CreateMmapBuffer failed, %d", retval);
        }
    };
    Return<void> rc = dev->createMmapBuffer(fd, *memory_size, hidlCb);
    if (!rc.isOk())
    {
        error = ErrorCode::ERROR_UNKNOWN;
        IMSG_ERROR("HIDL call failed with %s", rc.description().c_str());
    }
    if (retval == ErrorCode::OK)
    {
        IMSG_DEBUG("Shared file descriptor is %d", shmfd);
        // must return a "dup fd", because when method return, shmfd will be close when Parcel destroyed
        int result_fp = dup(shmfd);
        IMSG_DEBUG("Duplicate fd is %d", result_fp);
        if (result_fp < 1)
        {
            IMSG_ERROR("Duplicate fd faild with %d", errno);
        }
        return result_fp;
    }
    else
    {
        IMSG_ERROR("Create Memory faild with %d", retval);
        //TODO error code proceess
        return -1;
    }
}

int ut_shm_alloc(int32_t fd, uint64_t size, int *id)
{

    uint32_t shmfd = 0;
    uint64_t sizeout = 0;
    sizeout = size;
    shmfd = createMmapBuffer(fd, &sizeout, id);
    return shmfd;
}

static int32_t registerSharedFileDescriptor(int32_t fd, int32_t reg_fd, uint64_t *size)
{
    if (fd < 1 || NULL == size || reg_fd < 0)
    {
        IMSG_ERROR("Parameter missing.");
        return -1;
    }
    int32_t shmfd;
    ErrorCode error = ErrorCode::ERROR_UNKNOWN;
    ErrorCode retval;
    sp<IClientApiDevice> dev = nullptr;

    dev = IClientApiDevice::getService();
    if (dev == nullptr)
    {
        IMSG_ERROR("Access capi service failed.");
        return static_cast<int32_t>(error);
    }
    auto hidlCb = [&](ErrorCode ret, const MmapBufferInfo &hidlInfo) {
        retval = ret;
        if (retval == ErrorCode::OK)
        {
            const native_handle *handle = hidlInfo.sharedMemory.handle();
            if (handle->numFds > 0)
            {
                shmfd = handle->data[0];
            }
            else
            {
                retval = ErrorCode::ERROR_OUTPUT_PARAMETER_NULL;
            }
            *size = hidlInfo.bufferSize;
        }
    };
    Return<void> rc = dev->registerSharedFileDescriptor(fd, reg_fd, hidlCb);
    if (!rc.isOk())
    {
        error = ErrorCode::ERROR_UNKNOWN;
        IMSG_ERROR("HIDL call failed with %s", rc.description().c_str());
    }
    if (retval == ErrorCode::OK)
    {
        return shmfd;
    }
    else
    {
        //TODO error code proceess
        return -1;
    }
}

int ut_shm_register_fd(int32_t fd, struct tee_ioctl_shm_register_fd_data *fd_data)
{
    Parcel data, reply;
    TEEC_Result ret;
    int32_t rfd;
    uint64_t sizeout = 0;
    rfd = registerSharedFileDescriptor(fd, fd_data->fd, &sizeout);
    if (rfd > 0)
    {
        fd_data->size = sizeout;
        int result_fp = dup(rfd);
        return result_fp;
    }
    else
    {
        return -1;
    }
}

int ut_shm_get_pa(int32_t fd, int32_t id, uint64_t *pa)
{
    IMSG_DEBUG("[%s][%d] start!\n", "ut_shm_get_pa", __LINE__);
    if (!pa || fd < 0)
        return TEEC_ERROR_BAD_PARAMETERS;

    Parcel data, reply;
    int32_t ret;
    data.writeInt32(fd);
    data.writeInt32(id);
    data.writeInt32(0);
    transact(CLIENT_API_SHM_GET_PA_CMD, data, &reply);
    *pa = reply.readUint64();
    ret = reply.readInt32();
    IMSG_DEBUG("[%s][%d] end ret: %d paddr %llx !\n", "ut_shm_get_pa", __LINE__, ret, *pa);
    return ret;
}

int ut_shm_flush_cache(int32_t fd, int32_t id)
{
    IMSG_DEBUG("[%s][%d] start!\n", "ut_shm_flush_cache", __LINE__);
    if (fd < 0)
        return TEEC_ERROR_BAD_PARAMETERS;

    Parcel data, reply;
    int32_t ret;
    data.writeInt32(fd);
    data.writeInt32(id);
    transact(CLIENT_API_SHM_FLUSH_CACHE_CMD, data, &reply);
    ret = reply.readInt32();
    IMSG_DEBUG("[%s][%d] end ret: %d !\n", "ut_shm_flush_cache", __LINE__, ret);
    return ret;
}