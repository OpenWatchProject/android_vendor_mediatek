/*
 * Copyright (c) 2015-2016 MICROTRUST Incorporated
 * All rights reserved
 *
 * This file and software is confidential and proprietary to MICROTRUST Inc.
 * Unauthorized copying of this file and software is strictly prohibited.
 * You MUST NOT disclose this file and software unless you get a license
 * agreement from MICROTRUST Incorporated.
 */

#ifndef UT_CLIENT_API_DAEMON_PROXY_H_
#define UT_CLIENT_API_DAEMON_PROXY_H_

#include "tee_client_api.h"
#include <linux/tee.h>
#include <utils/RefBase.h>

namespace android
{

class UtClientApiDaemonProxy : public RefBase
{
  public:
    static UtClientApiDaemonProxy *getInstance()
    {
        if (sInstance == NULL)
        {
            sInstance = new UtClientApiDaemonProxy();
        }
        return sInstance;
    }
    virtual TEEC_Result ut_initialize_context(const char *name, TEEC_Context *ut_context);
    virtual TEEC_Result ut_open_session(int32_t fd, uintptr_t buf, uint32_t buf_len);
    virtual TEEC_Result ut_invoke_command(int32_t fd, uintptr_t buf, uint32_t buf_len);
    virtual void ut_close_session(TEEC_Session *session);
    virtual int ut_shm_alloc(int32_t fd, uint64_t size, int *id);
    virtual int ut_shm_register_fd(int32_t fd, struct tee_ioctl_shm_register_fd_data *data);
    virtual void ut_request_cancel(int32_t fd, struct tee_ioctl_cancel_arg *arg);
    virtual int ut_shm_get_pa(int32_t fd, int32_t id, uint64_t *pa);
    virtual int ut_shm_flush_cache(int32_t fd, int32_t id);

  private:
    UtClientApiDaemonProxy();
    virtual ~UtClientApiDaemonProxy();
    static UtClientApiDaemonProxy *sInstance;
};
} // namespace android

#endif // UT_CLIENT_API_DAEMON_PROXY_H_
