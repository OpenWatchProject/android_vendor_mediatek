/*
 * Copyright (C) 2016 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include <errno.h>
#include <fcntl.h>
#include <inttypes.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <unistd.h>
#include <cutils/properties.h>

#include "log.h"
#include "ipc.h"
#include "storage.h"

#define FD_TBL_SIZE 64
#define MAX_READ_SIZE 4096

enum sync_state {
    SS_UNUSED = -1,
    SS_CLEAN =  0,
    SS_DIRTY =  1,
};

static int ssdir_fd = -1;
static const char *ssdir_name;

static int ss_persistent_dir_fd = -1;
static const char *ss_persistent_dir_name;

static int ss_proinfo_node_fd = -1;

#ifdef PRO_NODE
const char *ss_proinfo_name = PRO_NODE;
#else
const char *ss_proinfo_name = "/dev/block/platform/bootdevice/by-name/proinfo";
#endif

static enum sync_state fs_state;
static enum sync_state dir_state;
static enum sync_state fd_state[FD_TBL_SIZE];

static struct {
   struct storage_file_read_resp hdr;
   uint8_t data[MAX_READ_SIZE];
}  read_rsp;

static uint32_t insert_fd(int open_flags, int fd)
{
    uint32_t handle = fd;

    if (open_flags & O_CREAT) {
        dir_state = SS_DIRTY;
    }

    if (handle < FD_TBL_SIZE) {
            fd_state[fd] = SS_CLEAN; /* fd clean */
            if (open_flags & O_TRUNC) {
                fd_state[fd] = SS_DIRTY;  /* set fd dirty */
            }
    } else {
            ALOGW("%s: untracked fd %u\n", __func__, fd);
            if (open_flags & (O_TRUNC | O_CREAT)) {
                fs_state = SS_DIRTY;
            }
    }
    return handle;
}

static int lookup_fd(uint32_t handle, bool dirty)
{
    if (dirty) {
        if (handle < FD_TBL_SIZE) {
            fd_state[handle] = SS_DIRTY;
        } else {
            fs_state = SS_DIRTY;
        }
    }
    return handle;
}

static int remove_fd(uint32_t handle)
{
    if (handle < FD_TBL_SIZE) {
        fd_state[handle] = SS_UNUSED; /* set to uninstalled */
    }
    return handle;
}

static enum storage_err translate_errno(int error)
{
    enum storage_err result;
    switch (error) {
    case 0:
        result = STORAGE_NO_ERROR;
        break;
    case EBADF:
    case EINVAL:
    case ENOTDIR:
    case EISDIR:
    case ENAMETOOLONG:
        result = STORAGE_ERR_NOT_VALID;
        break;
    case ENOENT:
        result = STORAGE_ERR_NOT_FOUND;
        break;
    case EEXIST:
        result = STORAGE_ERR_EXIST;
        break;
    case EPERM:
    case EACCES:
        result = STORAGE_ERR_ACCESS;
        break;
    default:
        result = STORAGE_ERR_GENERIC;
        break;
    }

    return result;
}

static ssize_t write_with_retry(int fd, const void *buf_, size_t size, off_t offset)
{
    ssize_t rc;
    const uint8_t *buf = buf_;

    while (size > 0) {
        rc = TEMP_FAILURE_RETRY(pwrite(fd, buf, size, offset));
        if (rc < 0)
            return rc;
        size -= rc;
        buf += rc;
        offset += rc;
    }
    return 0;
}

static ssize_t read_with_retry(int fd, void *buf_, size_t size, off_t offset)
{
    ssize_t rc;
    size_t  rcnt = 0;
    uint8_t *buf = buf_;

    while (size > 0) {
        rc = TEMP_FAILURE_RETRY(pread(fd, buf, size, offset));
        if (rc < 0)
            return rc;
        if (rc == 0)
            break;
        size -= rc;
        buf += rc;
        offset += rc;
        rcnt += rc;
    }
    return rcnt;
}

int storage_file_delete(struct storage_msg *msg,
                        const void *r, size_t req_len)
{
    char *path = NULL;
    const struct storage_file_delete_req *req = r;

    if (req_len < sizeof(*req)) {
        ALOGE("%s: invalid request length (%zd < %zd)\n",
              __func__, req_len, sizeof(*req));
        msg->result = STORAGE_ERR_NOT_VALID;
        goto err_response;
    }

    size_t fname_len = strlen(req->name);
    if (fname_len != req_len - sizeof(*req)) {
        ALOGE("%s: invalid filename length (%zd != %zd)\n",
              __func__, fname_len, req_len - sizeof(*req));
        msg->result = STORAGE_ERR_NOT_VALID;
        goto err_response;
    }

    int rc = asprintf(&path, "%s/%s", ssdir_name, req->name);
    if (rc < 0) {
        ALOGE("%s: asprintf failed\n", __func__);
        msg->result = STORAGE_ERR_GENERIC;
        goto err_response;
    }

    dir_state = SS_DIRTY;
    rc = unlink(path);
    if (rc < 0) {
        rc = errno;
        if (errno == ENOENT) {
            ALOGV("%s: error (%d) unlinking file '%s'\n",
                  __func__, rc, path);
        } else {
            ALOGE("%s: error (%d) unlinking file '%s'\n",
                  __func__, rc, path);
        }
        msg->result = translate_errno(rc);
        goto err_response;
    }

    ALOGV("%s: \"%s\"\n", __func__, path);
    msg->result = STORAGE_NO_ERROR;

err_response:
    if (path)
        free(path);
    return ipc_respond(msg, NULL, 0);
}

static int prepare_floder(const char* root_dir, const char* path) {
    char* folder_name;
    int end = -1;
    int rc = 0;
    char* folder_full_path;
    for(int i = 0; i < strlen(path); i++) {
        if(path[i]== '/'){
            end = i;
        }
    }
    if(end > 0) {
        folder_name = malloc(end + 1);
        if(folder_name){
            memset(folder_name, 0, end + 1);
            memcpy(folder_name, path, end);
            rc = asprintf(&folder_full_path, "%s/%s", root_dir, folder_name);
            if(rc >= 0){
                umask(020);
                mkdir(folder_full_path, S_IRWXU);
            }
            if(folder_full_path){
                free(folder_full_path);
            }
            free(folder_name);
            return 0;
        }
    }
    return -1;
}

int storage_file_open(struct storage_msg *msg,
                      const void *r, size_t req_len)
{
    char *path = NULL;
    const struct storage_file_open_req *req = r;
    struct storage_file_open_resp resp = {0};

    if (req_len < sizeof(*req)) {
        ALOGE("%s: invalid request length (%zd < %zd)\n",
               __func__, req_len, sizeof(*req));
        msg->result = STORAGE_ERR_NOT_VALID;
        goto err_response;
    }

    size_t fname_len = strlen(req->name);
    if (fname_len != req_len - sizeof(*req)) {
        ALOGE("%s: invalid filename length (%zd != %zd)\n",
              __func__, fname_len, req_len - sizeof(*req));
        msg->result = STORAGE_ERR_NOT_VALID;
        goto err_response;
    }

    int is_persistent_file = 0;
    int is_proinfo_file = 0;
    int is_vfs_file = 0;
    is_persistent_file = req->flags & STORAGE_FILE_OPEN_PERSISTENT_FILE;
    is_proinfo_file = req->flags & STORAGE_FILE_OPEN_PROINFO_FILE;
    is_vfs_file = req->flags & STORAGE_FILE_OPEN_VFS;
    ALOGI("open file flas is %d is_persistent_file %d", req->flags, is_persistent_file);
    ALOGI("open file flas is %d is_proinfo_file %d", req->flags, is_proinfo_file);
    int rc = 0;
    if (is_persistent_file) {
        ALOGI("%s: asprintf persist file init\n", __func__);
        rc = asprintf(&path, "%s/%s", ss_persistent_dir_name, req->name);
    } else if (is_proinfo_file) {
		ALOGI("%s: asprintf proinfo file init\n", __func__);
		rc = asprintf(&path, "%s", ss_proinfo_name);
    } else if (is_vfs_file) {
		ALOGI("%s: asprintf vfs data file init\n", __func__);
		rc = asprintf(&path, "%s/%s", ssdir_name, req->name);
		prepare_floder(ssdir_name, req->name);
	} else {
        ALOGI("%s: asprintf data file init\n", __func__);
        rc = asprintf(&path, "%s/%s", ssdir_name, req->name);
    }

    if (rc < 0) {
        ALOGE("%s: asprintf failed\n", __func__);
        msg->result = STORAGE_ERR_GENERIC;
        goto err_response;
    }

    int open_flags = O_RDWR | O_SYNC;

    if (req->flags & STORAGE_FILE_OPEN_TRUNCATE)
        open_flags |= O_TRUNC;

    if (req->flags & STORAGE_FILE_OPEN_CREATE) {
        /* open or create */
        if (req->flags & STORAGE_FILE_OPEN_CREATE_EXCLUSIVE) {
            /* create exclusive */
            open_flags |= O_CREAT | O_EXCL;
            rc = TEMP_FAILURE_RETRY(open(path, open_flags, S_IRUSR | S_IWUSR));
        } else {
            /* try open first */
            rc = TEMP_FAILURE_RETRY(open(path, open_flags, S_IRUSR | S_IWUSR));
            if (rc == -1 && errno == ENOENT) {
                /* then try open with O_CREATE */
                open_flags |= O_CREAT;
                rc = TEMP_FAILURE_RETRY(open(path, open_flags, S_IRUSR | S_IWUSR));
            }

        }
    } else {
        /* open an existing file */
        ALOGI("open file flas is %d is_proinfo_file %d", req->flags, is_proinfo_file);
        rc = TEMP_FAILURE_RETRY(open(path, open_flags, S_IRUSR | S_IWUSR));
    }

    if (rc < 0) {
        rc = errno;
        if (errno == EEXIST || errno == ENOENT) {
            ALOGV("%s: failed to open file \"%s\": %s\n",
                  __func__, path, strerror(errno));
        } else {
            ALOGE("%s: failed to open file \"%s\": %s\n",
                  __func__, path, strerror(errno));
        }
        msg->result = translate_errno(rc);
        goto err_response;
    }
    free(path);

    /* at this point rc contains storage file fd */
    msg->result = STORAGE_NO_ERROR;
    resp.handle = insert_fd(open_flags, rc);
    ALOGV("%s: \"%s\": fd = %u: handle = %d\n",
          __func__, path, rc, resp.handle);

    return ipc_respond(msg, &resp, sizeof(resp));

err_response:
    if (path)
        free(path);
    return ipc_respond(msg, NULL, 0);
}

int storage_file_close(struct storage_msg *msg,
                       const void *r, size_t req_len)
{
    const struct storage_file_close_req *req = r;

    if (req_len != sizeof(*req)) {
        ALOGE("%s: invalid request length (%zd != %zd)\n",
              __func__, req_len, sizeof(*req));
        msg->result = STORAGE_ERR_NOT_VALID;
        goto err_response;
    }

    int fd = remove_fd(req->handle);
    ALOGV("%s: handle = %u: fd = %u\n", __func__, req->handle, fd);

    int rc = fsync(fd);
    if (rc < 0) {
        rc = errno;
        ALOGE("%s: fsync failed for fd=%u: %s\n",
              __func__, fd, strerror(errno));
        msg->result = translate_errno(rc);
        goto err_response;
    }

    rc = close(fd);
    if (rc < 0) {
        rc = errno;
        ALOGE("%s: close failed for fd=%u: %s\n",
              __func__, fd, strerror(errno));
        msg->result = translate_errno(rc);
        goto err_response;
    }

    msg->result = STORAGE_NO_ERROR;

err_response:
    return ipc_respond(msg, NULL, 0);
}


int storage_file_write(struct storage_msg *msg,
                       const void *r, size_t req_len)
{
    int rc;
    const struct storage_file_write_req *req = r;

    if (req_len < sizeof(*req)) {
        ALOGE("%s: invalid request length (%zd < %zd)\n",
              __func__, req_len, sizeof(*req));
        msg->result = STORAGE_ERR_NOT_VALID;
        goto err_response;
    }

    int fd = lookup_fd(req->handle, true);
    if (write_with_retry(fd, &req->data[0], req_len - sizeof(*req),
                         req->offset) < 0) {
        rc = errno;
        ALOGW("%s: error writing file (fd=%d): %s\n",
              __func__, fd, strerror(errno));
        msg->result = translate_errno(rc);
        goto err_response;
    }

    msg->result = STORAGE_NO_ERROR;

err_response:
    return ipc_respond(msg, NULL, 0);
}



int storage_file_read(struct storage_msg *msg,
                      const void *r, size_t req_len)
{
    int rc;
    const struct storage_file_read_req *req = r;

    if (req_len != sizeof(*req)) {
        ALOGE("%s: invalid request length (%zd != %zd)\n",
              __func__, req_len, sizeof(*req));
        msg->result = STORAGE_ERR_NOT_VALID;
        goto err_response;
    }

    if (req->size > MAX_READ_SIZE) {
        ALOGW("%s: request is too large (%zd > %zd) - refusing\n",
              __func__, req->size, MAX_READ_SIZE);
        msg->result = STORAGE_ERR_NOT_VALID;
        goto err_response;
    }

    int fd = lookup_fd(req->handle, false);
    ssize_t read_res = read_with_retry(fd, read_rsp.hdr.data, req->size,
                                       (off_t)req->offset);
    if (read_res < 0) {
        rc = errno;
        ALOGW("%s: error reading file (fd=%d): %s\n",
              __func__, fd, strerror(errno));
        msg->result = translate_errno(rc);
        goto err_response;
    }

    msg->result = STORAGE_NO_ERROR;
    return ipc_respond(msg, &read_rsp, read_res + sizeof(read_rsp.hdr));

err_response:
    return ipc_respond(msg, NULL, 0);
}


int storage_file_get_size(struct storage_msg *msg,
                          const void *r, size_t req_len)
{
    const struct storage_file_get_size_req *req = r;
    struct storage_file_get_size_resp resp = {0};

    if (req_len != sizeof(*req)) {
        ALOGE("%s: invalid request length (%zd != %zd)\n",
              __func__, req_len, sizeof(*req));
        msg->result = STORAGE_ERR_NOT_VALID;
        goto err_response;
    }

    struct stat stat;
    int fd = lookup_fd(req->handle, false);
    int rc = fstat(fd, &stat);
    if (rc < 0) {
        rc = errno;
        ALOGE("%s: error stat'ing file (fd=%d): %s\n",
              __func__, fd, strerror(errno));
        msg->result = translate_errno(rc);
        goto err_response;
    }

    resp.size = stat.st_size;
    msg->result = STORAGE_NO_ERROR;
    return ipc_respond(msg, &resp, sizeof(resp));

err_response:
    return ipc_respond(msg, NULL, 0);
}


int storage_file_set_size(struct storage_msg *msg,
                          const void *r, size_t req_len)
{
    const struct storage_file_set_size_req *req = r;

    if (req_len != sizeof(*req)) {
        ALOGE("%s: invalid request length (%zd != %zd)\n",
              __func__, req_len, sizeof(*req));
        msg->result = STORAGE_ERR_NOT_VALID;
        goto err_response;
    }

    int fd = lookup_fd(req->handle, true);
    int rc = TEMP_FAILURE_RETRY(ftruncate(fd, req->size));
    if (rc < 0) {
        rc = errno;
        ALOGE("%s: error truncating file (fd=%d): %s\n",
              __func__, fd, strerror(errno));
        msg->result = translate_errno(rc);
        goto err_response;
    }

    msg->result = STORAGE_NO_ERROR;

err_response:
    return ipc_respond(msg, NULL, 0);
}

int storage_init(const char *dirname)
{
    fs_state = SS_CLEAN;
    dir_state = SS_CLEAN;
    for (uint i = 0; i < FD_TBL_SIZE; i++) {
        fd_state[i] = SS_UNUSED;  /* uninstalled */
    }

    ssdir_fd = open(dirname, O_RDONLY);
    if (ssdir_fd < 0) {
        ALOGE("failed to open ss root dir \"%s\": %s\n",
               dirname, strerror(errno));
        return -1;
    }
    ssdir_name = dirname;
    return 0;
}

int storage_persistent_init(const char *dirname)
{
    ss_persistent_dir_fd = open(dirname, O_RDONLY);
    if (ss_persistent_dir_fd < 0) {
        ALOGE("failed to open ss root dir \"%s\": %s\n",
               dirname, strerror(errno));
        return -1;
    }
    ss_persistent_dir_name = dirname;
    return 0;
}

int storage_proinfo_init(const char *nodename)
{
    ss_proinfo_node_fd = open(nodename, O_RDONLY);
    if (ss_proinfo_node_fd < 0) {
        ALOGE("failed to open proinfo dev \"%s\": %s\n",
        		nodename, strerror(errno));
        return -1;
    }
    return 0;
}

int storage_init_ok(struct storage_msg *msg,
                       const void *r, size_t req_len)
{
    int rc;
    const struct storage_file_write_req *req = r;

    if (req_len < sizeof(*req)) {
        ALOGE("%s: invalid request length (%zd < %zd)\n",
              __func__, req_len, sizeof(*req));
        msg->result = STORAGE_ERR_NOT_VALID;
        goto err_response;
    }

    property_set("vendor.soter.teei.init", "INIT_OK");

    msg->result = STORAGE_NO_ERROR;

err_response:
    return ipc_respond(msg, NULL, 0);
}

int storage_init_persist_ok(struct storage_msg *msg,
                       const void *r, size_t req_len)
{
    int rc;
    const struct storage_file_write_req *req = r;

    if (req_len < sizeof(*req)) {
        ALOGE("%s: invalid request length (%zd < %zd)\n",
              __func__, req_len, sizeof(*req));
        msg->result = STORAGE_ERR_NOT_VALID;
        goto err_response;
    }

    property_set("vendor.soter.teei.persist", "INIT_OK");

    msg->result = STORAGE_NO_ERROR;

err_response:
    return ipc_respond(msg, NULL, 0);
}

int storage_init_proinfo_ok(struct storage_msg *msg,
                       const void *r, size_t req_len)
{
    int rc;
    const struct storage_file_write_req *req = r;

    if (req_len < sizeof(*req)) {
        ALOGE("%s: invalid request length (%zd < %zd)\n",
              __func__, req_len, sizeof(*req));
        msg->result = STORAGE_ERR_NOT_VALID;
        goto err_response;
    }

    property_set("vendor.soter.teei.proinfo", "INIT_OK");

    msg->result = STORAGE_NO_ERROR;

err_response:
    return ipc_respond(msg, NULL, 0);
}

int storage_sync_checkpoint(void)
{
    int rc;

    /* sync fd table and reset it to clean state first */
    for (uint fd = 0; fd < FD_TBL_SIZE; fd++) {
         if (fd_state[fd] == SS_DIRTY) {
             if (fs_state == SS_CLEAN) {
                 /* need to sync individual fd */
                 rc = fsync(fd);
                 if (rc < 0) {
                     ALOGE("fsync for fd=%d failed: %s\n", fd, strerror(errno));
                     return rc;
                 }
             }
             fd_state[fd] = SS_CLEAN; /* set to clean */
         }
    }

    /* check if we need to sync the directory */
    if (dir_state == SS_DIRTY) {
        if (fs_state == SS_CLEAN) {
            rc = fsync(ssdir_fd);
            if (rc < 0) {
                ALOGE("fsync for ssdir failed: %s\n", strerror(errno));
                return rc;
            }
            if (ss_persistent_dir_fd > 0) {
                rc = fsync(ss_persistent_dir_fd);
                if (rc < 0) {
                    ALOGE("fsync for persistent failed: %s\n", strerror(errno));
                    return rc;
                }
            }
            if (ss_proinfo_node_fd > 0) {
            	rc = fsync(ss_proinfo_node_fd);
            	if (rc < 0) {
					ALOGE("fsync for proinfo failed: %s\n", strerror(errno));
					return rc;
				}
            }
        }
        dir_state = SS_CLEAN;  /* set to clean */
    }

    /* check if we need to sync the whole fs */
    if (fs_state == SS_DIRTY) {
        rc = syscall(SYS_syncfs, ssdir_fd);
        if (rc < 0) {
            ALOGE("syncfs failed: %s\n", strerror(errno));
            return rc;
        }
		if (ss_persistent_dir_fd > 0) {
			rc = syscall(SYS_syncfs, ss_persistent_dir_fd);
				if (rc < 0) {
					ALOGE("syncfs persistent failed: %s\n", strerror(errno));
					return rc;
				}
		}
		if (ss_proinfo_node_fd > 0) {
			rc = syscall(SYS_syncfs, ss_proinfo_node_fd);
			if (rc < 0) {
				ALOGE("syncfs proinfo failed: %s\n", strerror(errno));
				return rc;
			}
		}
        fs_state = SS_CLEAN;
    }

    return 0;
}
