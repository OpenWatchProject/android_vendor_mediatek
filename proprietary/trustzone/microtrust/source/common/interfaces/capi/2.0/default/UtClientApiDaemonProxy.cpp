/*
 * Copyright (c) 2015-2016 MICROTRUST Incorporated
 * All rights reserved
 *
 * This file and software is confidential and proprietary to MICROTRUST Inc.
 * Unauthorized copying of this file and software is strictly prohibited.
 * You MUST NOT disclose this file and software unless you get a license
 * agreement from MICROTRUST Incorporated.
 */

#define IMSG_TAG "ClientApi-Daemon-Proxy"
/* clang-format off */
//#include "IUtClientApiDaemon.h"

//#include <limits>
//#include <bits/unique_ptr.h>
#include <hwbinder/Parcel.h>

#include "teec_trace.h"
#include <errno.h>
#include <fcntl.h>
#include <imsg_log.h>
#include <include/tee_client_api.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/mman.h>


#include "UtClientApiDaemonProxy.h"
/* clang-format on */
/* How many device sequence numbers will be tried before giving up */
#define TEEC_MAX_DEV_SEQ 10
#define DEFAULT_CAPABILITY "bta_loader"
#define GPTEE_CAPABILITY "tta"

#ifndef __aligned
#define __aligned(x) __attribute__((__aligned__(x)))
#endif
#include <linux/tee.h>
#include <teec_trace.h>

/**
 * struct tee_ioctl_shm_kern_op_arg - Kernel operations for shared memory
 * @id:     [in] Identifier of the shared memory
 * @opcode: [in] kernel operation for the specific shared memory
 * @paddr:  [out] The starting physical address of the shared memory
 */
struct tee_ioctl_shm_kern_op_arg
{
    __s32 id;
    __u32 opcode;
    __u64 paddr;
};
#define TEE_IOCTL_SHM_KERN_OP_GET_PA 0
#define TEE_IOCTL_SHM_KERN_OP_FLUSH_CACHE 1

/**
 * TEE_IOC_SHM_KERN_OP - kernel operations for specific shared memory
 */
#define TEE_IOC_SHM_KERN_OP _IOWR(TEE_IOC_MAGIC, TEE_IOC_BASE + 10, \
                                  struct tee_ioctl_shm_kern_op_arg)

namespace android
{

UtClientApiDaemonProxy *UtClientApiDaemonProxy::sInstance = NULL;

UtClientApiDaemonProxy::UtClientApiDaemonProxy()
{
}

UtClientApiDaemonProxy::~UtClientApiDaemonProxy()
{
}

static TEEC_Result ioctl_errno_to_res(int err)
{
    switch (err)
    {
    case ENOMEM:
        return TEEC_ERROR_OUT_OF_MEMORY;
    default:
        return TEEC_ERROR_GENERIC;
    }
}

static int teec_open_dev(const char *devname, char *capabilities)
{
    struct tee_ioctl_version_data vers;
    struct tee_ioctl_set_hostname_arg arg;
    int fd;

    fd = open(devname, O_RDWR);
    if (fd < 0)
    {
        IMSG_ERROR("tee device  open failed errno: %d", errno);
        return -1;
    }

    if (ioctl(fd, TEE_IOC_VERSION, &vers))
    {
        IMSG_ERROR("TEE_IOC_VERSION failed");
        goto err;
    }

    /* We can only handle GP TEEs */
    if (!(vers.gen_caps & TEE_GEN_CAP_GP))
        goto err;

    memset(&arg, 0, sizeof(arg));

#ifndef DEFAULT_TEE_GPTEE
    if (capabilities && (strcmp(capabilities, GPTEE_CAPABILITY) == 0))
    {
        IMSG_DEBUG("client request will be serviced by GPTEE");
        strncpy((char *)arg.hostname, GPTEE_CAPABILITY,
                TEE_MAX_HOSTNAME_SIZE - 1);
    }
    else
    {
        IMSG_DEBUG("client request will be serviced by BTA Loader");
        strncpy((char *)arg.hostname, DEFAULT_CAPABILITY,
                TEE_MAX_HOSTNAME_SIZE - 1);
    }
#else
    if (capabilities && (strcmp(capabilities, DEFAULT_CAPABILITY) == 0))
    {
        DMSG("client request will be serviced by BTA LOADER");
        strncpy((char *)arg.hostname, DEFAULT_CAPABILITY,
                TEE_MAX_HOSTNAME_SIZE - 1);
    }
    else if (capabilities == NULL || (strcmp(capabilities, GPTEE_CAPABILITY) == 0))
    {
        DMSG("client request will be serviced by GPTEE");
        strncpy((char *)arg.hostname, GPTEE_CAPABILITY,
                TEE_MAX_HOSTNAME_SIZE - 1);
    }
    else
    {
        EMSG("client request error name");
        goto err;
    }
#endif
    if (ioctl(fd, TEE_IOC_SET_HOSTNAME, &arg))
    {
        IMSG_ERROR("TEE_IOC_SET_HOSTNAME failed");
        goto err;
    }

    return fd;
err:
    close(fd);
    return -1;
}

TEEC_Result UtClientApiDaemonProxy::ut_initialize_context(const char *name, TEEC_Context *ctx)
{
    char devname[PATH_MAX];
    int fd;
    size_t n;

    if (!ctx)
        return TEEC_ERROR_BAD_PARAMETERS;

    for (n = 0; n < TEEC_MAX_DEV_SEQ; n++)
    {
        snprintf(devname, sizeof(devname), "/dev/tee%zu", n);
        fd = teec_open_dev(devname, (char *)name);
        if (fd >= 0)
        {
            ctx->fd = fd;
            return TEEC_SUCCESS;
        }
    }

    return TEEC_ERROR_ITEM_NOT_FOUND;
}

TEEC_Result UtClientApiDaemonProxy::ut_open_session(int32_t fd, uintptr_t buf, uint32_t buf_len)
{
    TEEC_Result ret = TEEC_SUCCESS;
    int rc = 0;
    IMSG_DEBUG("ut_open_session start ");
    struct tee_ioctl_buf_data buf_data;

    buf_data.buf_ptr = buf;
    buf_data.buf_len = buf_len;
    IMSG_DEBUG("ut_open_session open fd %d ", fd);
    rc = ioctl(fd, TEE_IOC_OPEN_SESSION, &buf_data);
    if (rc)
    {
        IMSG_ERROR("TEE_IOC_OPEN_SESSION failed errno: %d", errno);
        ret = ioctl_errno_to_res(errno);
    }

    IMSG_DEBUG("ut_open_session end ");
    return ret;
}

TEEC_Result UtClientApiDaemonProxy::ut_invoke_command(int32_t fd, uintptr_t buf, uint32_t buf_len)
{
    TEEC_Result ret = TEEC_SUCCESS;
    int rc = 0;
    IMSG_DEBUG("ut_invoke_command start ");
    struct tee_ioctl_buf_data buf_data;

    buf_data.buf_ptr = buf;
    buf_data.buf_len = buf_len;

    rc = ioctl(fd, TEE_IOC_INVOKE, &buf_data);
    if (rc)
    {
        IMSG_ERROR("TEE_IOC_INVOKE failed");
        ret = ioctl_errno_to_res(errno);
    }

    IMSG_DEBUG("ut_invoke_command end ");
    return ret;
}

void UtClientApiDaemonProxy::ut_request_cancel(int32_t fd, struct tee_ioctl_cancel_arg *arg)
{

    if (ioctl(fd, TEE_IOC_CANCEL, arg))
        IMSG_ERROR("TEE_IOC_CANCEL: %s", strerror(errno));
}

void UtClientApiDaemonProxy::ut_close_session(TEEC_Session *session)
{
    struct tee_ioctl_close_session_arg arg;
    if (!session)
        return;

    arg.session = session->session_id;
    if (ioctl(session->ctx->fd, TEE_IOC_CLOSE_SESSION, &arg))
        IMSG_ERROR("Failed to close session 0x%x, errno is %d", session->session_id, errno);
    return;
}

int UtClientApiDaemonProxy::ut_shm_alloc(int32_t fd, uint64_t size, int *id)
{
    int shm_fd;
    struct tee_ioctl_shm_alloc_data data;

    memset(&data, 0, sizeof(data));
    data.size = size;
    shm_fd = ioctl(fd, TEE_IOC_SHM_ALLOC, &data);
    if (shm_fd < 0)
    {
        IMSG_ERROR("ut_shm_alloc failed, shm_fd is %d errno is %d", shm_fd, errno);
        return -1;
    }
    *id = data.id;
    return shm_fd;
}

int UtClientApiDaemonProxy::ut_shm_register_fd(int32_t fd, struct tee_ioctl_shm_register_fd_data *fd_data)
{
    int rfd;

    rfd = ioctl(fd, TEE_IOC_SHM_REGISTER_FD, fd_data);
    if (rfd < 0)
        return -1;
    return rfd;
}

int UtClientApiDaemonProxy::ut_shm_get_pa(int32_t fd, int32_t id, uint64_t *paddr)
{
    struct tee_ioctl_shm_kern_op_arg data;
    int ret;
    memset(&data, 0, sizeof(data));
    data.id = id;
    data.opcode = TEE_IOCTL_SHM_KERN_OP_GET_PA;

    ret = ioctl(fd, TEE_IOC_SHM_KERN_OP, &data);
    if (ret < 0)
    {
        return TEEC_ERROR_GENERIC;
    }
    *paddr = data.paddr;
    return TEEC_SUCCESS;
}

int UtClientApiDaemonProxy::ut_shm_flush_cache(int32_t fd, int32_t id)
{
    int ret;
    struct tee_ioctl_shm_kern_op_arg data;

    memset(&data, 0, sizeof(data));
    data.id = id;
    data.opcode = TEE_IOCTL_SHM_KERN_OP_FLUSH_CACHE;

    ret = ioctl(fd, TEE_IOC_SHM_KERN_OP, &data);
    if (ret < 0)
        return ret;

    return 0;
}
}
