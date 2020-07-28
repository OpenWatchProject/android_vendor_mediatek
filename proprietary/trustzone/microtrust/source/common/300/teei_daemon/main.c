/*
 * Copyright (c) 2015-2016 MICROTRUST Incorporated
 * All rights reserved
 *
 * This file and software is confidential and proprietary to MICROTRUST Inc.
 * Unauthorized copying of this file and software is strictly prohibited.
 * You MUST NOT disclose this file and software unless you get a license
 * agreement from MICROTRUST Incorporated.
 */

#define _LARGEFILE64_SOURCE 1
#define _FILE_OFFSET_BITS 64

#define IMSG_TAG "[mTEE]"
#include <cutils/properties.h>
#include <errno.h>
#include <fcntl.h>
#include <malloc.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/param.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <imsg_log.h>
#include <dirent.h>
#include <linux/types.h>
#include "utos_version.h"
#include "TEEI.h"

#define TEEI_IOC_MAGIC 'T'
#define TEEI_CONFIG_IOCTL_INIT_TEEI _IOWR(TEEI_IOC_MAGIC, 3, struct init_param)
#define TEEI_CONFIG_IOCTL_UNLOCK _IOWR(TEEI_IOC_MAGIC, 4, int)
#define SOTER_TUI_ENTER _IOWR(TEEI_IOC_MAGIC, 0x70, int)
#define SOTER_TUI_LEAVE _IOWR(TEEI_IOC_MAGIC, 0x71, int)
#define TEEI_VFS_NOTIFY_DRM _IOWR(TEEI_IOC_MAGIC, 0x75, int)
#define BUFFER_SIZE (512 * 1024)
#define DEV_FILE "/dev/tz_vfs"

#define PERSIST_RPMB_PATH "/persist/rpmb/rpmb.txt"
#define VENDOR_PERSIST_RPMB_PATH "/vendor/persist/rpmb/rpmb.txt"
#define MNT_PERSIST_RPMB_PATH "/mnt/vendor/persist/rpmb/rpmb.txt"
#define RPMB_PATH "/data/vendor/thh/system/rpmb.txt"
#define SETTING_PROPKEY_HEAD "vendor.soter.teei."

#define MAX_DRV_UUIDS 20
#define UUID_LEN 32
#define REAL_DRV_UUIDS 1

struct init_param {
    char uuids[MAX_DRV_UUIDS][UUID_LEN+1];
    __u32 uuid_count;
    __u32 flag;
};

static unsigned int unlock_state = 0;
static unsigned int set_partition_prop = 0;

char drv_uuid[REAL_DRV_UUIDS][UUID_LEN+1] = {
    {"93feffccd8ca11e796c7c7a21acb4932"}
};

struct init_param init_param;

/**************************** COPY FUNCTIONS ***********************/
static int do_copy(int nargs, char** args)
{
    char* p = NULL;
    char* buffer = NULL;

    int rc = 0, brtw = 0, brtr = 0;
    int fd1 = -1, fd2 = -1;
    struct stat info;


    if (nargs != 3) {
        IMSG_ERROR(" do_copy: nargs = %d, need to be 3!\n", nargs);
        return -1;
    }

    if (stat(args[1], &info) < 0) {
        IMSG_ERROR(" can not get file information!\n");
        return -1;
    }

    if ((fd1 = open(args[1], O_RDONLY)) < 0) {
        IMSG_ERROR(" can not open %s !\n", args[1]);
        goto out_err;
    }
    if ((fd2 = open(args[2], O_WRONLY | O_CREAT | O_TRUNC, 0660)) < 0) {
        IMSG_ERROR(" can not open %s ! fd=%d errno=%d from=%s\n", args[2], fd2, errno, args[1]);
        goto out_err;
    }
    if (!(buffer = malloc(info.st_size))) {
        IMSG_ERROR(" malloc failed\n");
        goto out_err;
    }

    p = buffer;
    brtr = info.st_size;

    while (brtr) {
        rc = read(fd1, p, brtr);
        if (rc < 0)
            goto out_err;
        if (rc == 0)
            break;
        p += rc;
        brtr -= rc;
    }

    p = buffer;
    brtw = info.st_size;
    while (brtw) {
        rc = write(fd2, p, brtw);
        if (rc < 0)
            goto out_err;
        if (rc == 0)
            break;
        p += rc;
        brtw -= rc;
    }

    rc = 0;
    goto out;
out_err:
    rc = -1;
out:
    if (buffer)
        free(buffer);
    if (fd1 >= 0)
        close(fd1);
    if (fd2 >= 0)
        close(fd2);
    return rc;
}

/*
if return 0, persist dir is not exist.
if return 1, "/persist" dir exist.
if return 2, "/vendor/persist" dir exist
*/
static int check_persist_path()
{
  DIR *dirptr = NULL;

  dirptr = opendir("/persist/rpmb");
  if(dirptr == NULL)
    goto vendor_path;
  else {
    closedir(dirptr);
    return 1;
  }

vendor_path:

  dirptr = opendir("/vendor/persist/rpmb");
  if(dirptr == NULL)
    goto mnt_path;
  else {
    closedir(dirptr);
    return 2;
  }

mnt_path:
  dirptr = opendir("/mnt/vendor/persist/rpmb");
  if(dirptr == NULL)
    return 0;
  else {
    closedir(dirptr);
    return 3;
  }


}
/**************************** VFS COMMAND FUNCTIONS *******************/
int vfs_analysis_command(unsigned char* p_buffer)
{
    int responseLen = 0;
    int namelength  = 0;
    char full_path_name[PATH_MAX];
    char* pathname      = NULL;
    char* param_address = NULL;
    long retVal     = 0;
    unsigned long read_count = 0;
    unsigned long count      = 0;

    struct dirent* p_dirent = NULL;
    struct TEEI_vfs_command* tz_command = NULL;
    DIR* dir = NULL;
    union TEEI_vfs_response tz_response;

    tz_command  = (struct TEEI_vfs_command*)p_buffer;
    responseLen = sizeof(tz_response);
    param_address = (char*)((unsigned long)(tz_command) + PAGE_SIZE_4K);

    switch (tz_command->func)
    {
    case TZ_VFS_OPEN:
         IMSG_INFO(" come into the TZ_VFS_OPEN function\n");

        if (*param_address != '/') {
            full_path_name[0] = 0;
            strcat(full_path_name, SYSTEM_THH_PATH);
            strcat(full_path_name, param_address);
            pathname = &full_path_name[0];

            IMSG_DEBUG(" full_path: %s thh path: %s param_address %s\n",
                        full_path_name,SYSTEM_THH_PATH, param_address);
        } else {
            pathname = param_address;
        }

        if(memcmp(pathname, RPMB_PATH, strlen(RPMB_PATH)) == 0)
        {
          retVal = check_persist_path();
          IMSG_DEBUG("check persist=%d\n", retVal);
          if(retVal == 1) {
            pathname = PERSIST_RPMB_PATH;
          } else if(retVal == 2) {
            pathname = VENDOR_PERSIST_RPMB_PATH;
          } else if(retVal == 3) {
            pathname = MNT_PERSIST_RPMB_PATH;
          }

          if (set_partition_prop == 0) {
              // property_set("vendor.soter.teei.rpmb", "VIRTUAL");

              if (retVal != 0) {
                  property_set("vendor.soter.teei.persist", "OPEN");
              } else {
                  property_set("vendor.soter.teei.persist", "CLOSE");
              }
          }
        }

        if (memcmp(pathname, RPMB_PATH, strlen(RPMB_PATH)) == 0) {
            if (unlock_state == 0) {
                retVal = 0x5EADBEEF;
                IMSG_ERROR("open virtual rpmb failed, wait for FDE done\n");
                goto OPEN_OUT;
            }
        }

        retVal = open(pathname, tz_command->args.func_open_args.flags,
                      tz_command->args.func_open_args.mode);

        if (retVal == -1) {
            IMSG_DEBUG("errno = %d\n", errno);
            retVal = -(errno);
        }

OPEN_OUT:
        IMSG_DEBUG(" pathname = %s\n", pathname);
        IMSG_DEBUG(" flags = %d mode = %d retval = %ld\n", tz_command->args.func_open_args.flags,
                    tz_command->args.func_open_args.mode, retVal);

        memset((void*)p_buffer, '\0', responseLen);
        ((union TEEI_vfs_response*)p_buffer)->value = retVal;

        return responseLen;

    case TZ_VFS_READ:

        IMSG_DEBUG(" come into the TZ_VFS_READ function\n");

        retVal = read(tz_command->args.func_read_args.fd, (char*)param_address,
                      tz_command->args.func_read_args.count);

        if (retVal == -1) {
            retVal = -(errno);
        }

        memset((void*)p_buffer, '\0', responseLen);
        ((union TEEI_vfs_response*)p_buffer)->value = retVal;

        if (retVal == -1) {
            return responseLen;
        } else {
            return PAGE_SIZE_4K + retVal;
        }

    case TZ_VFS_WRITE:

        IMSG_DEBUG(" come into the TZ_VFS_WRITE function\n");
        IMSG_DEBUG("fd = %d\n", tz_command->args.func_write_args.fd);
        IMSG_DEBUG("count = %d\n", tz_command->args.func_write_args.count);

        retVal = write(tz_command->args.func_write_args.fd, (const char*)param_address,
                       tz_command->args.func_write_args.count);

        if (retVal == -1) {
            retVal = -(errno);
        }

        memset((void*)p_buffer, '\0', responseLen);
        ((union TEEI_vfs_response*)p_buffer)->value = retVal;

        IMSG_DEBUG(" come into the TZ_VFS_WRITE function retVal = %ld\n", retVal);

        if (retVal > 0) {
            return PAGE_SIZE_4K + retVal;
        } else {
            return responseLen;
        }

    case TZ_VFS_IOCTL:

        IMSG_DEBUG(" come into the TZ_VFS_IOCTL function\n");
        IMSG_DEBUG(" fd = %d\n", tz_command->args.func_ioctl_args.fd);
        IMSG_DEBUG(" cmd = %d\n", tz_command->args.func_ioctl_args.cmd);

        if (tz_command->args.func_ioctl_args.cmd == 0x70) {
            retVal =
                ioctl(tz_command->args.func_ioctl_args.fd, SOTER_TUI_ENTER, (void*)param_address);
        } else if (tz_command->args.func_ioctl_args.cmd == 0x71) {
            retVal =
                ioctl(tz_command->args.func_ioctl_args.fd, SOTER_TUI_LEAVE, (void*)param_address);
        } else if (tz_command->args.func_ioctl_args.cmd == 0x75) {
            retVal =
                ioctl(tz_command->args.func_ioctl_args.fd, TEEI_VFS_NOTIFY_DRM, (void*)param_address);
        }else {
            retVal = ioctl(tz_command->args.func_ioctl_args.fd,
                           tz_command->args.func_ioctl_args.cmd, (void*)param_address);
        }

        if (retVal == -1) {
            retVal = -(errno);
        }

        IMSG_DEBUG("retVal = 0x%x , *((int*)param_address) = 0x%x \n", retVal, *((int*)param_address));

        memset((void*)p_buffer, '\0', responseLen);
        ((union TEEI_vfs_response*)p_buffer)->value = retVal;

        return PAGE_SIZE_4K + 0x9000;

    case TZ_VFS_CLOSE:

        IMSG_DEBUG(" come into the TZ_VFS_CLOSE function\n");
        IMSG_DEBUG(" fd = %d\n", tz_command->args.func_close_args.fd);

        retVal = close(tz_command->args.func_close_args.fd);

        if (retVal == -1) {
            retVal = -(errno);
        }

        IMSG_DEBUG(" retVal = %ld\n", retVal);
        memset((void*)p_buffer, '\0', responseLen);
        ((union TEEI_vfs_response*)p_buffer)->value = retVal;

        return responseLen;

    case TZ_VFS_TRUNC:

        IMSG_DEBUG(" come into the TZ_VFS_TRUNC function\n");

        retVal =
            ftruncate(tz_command->args.func_trunc_args.fd, tz_command->args.func_trunc_args.length);

        if (retVal == -1) {
            retVal = -(errno);
        }

        memset((void*)p_buffer, '\0', responseLen);
        ((union TEEI_vfs_response*)p_buffer)->value = retVal;

        return responseLen;

    case TZ_VFS_UNLINK:

        IMSG_DEBUG(" come into the TZ_VFS_UNLINK function\n");

        retVal = unlink((char*)param_address);

        if (retVal == -1) {
            retVal = -(errno);
        }

        memset((void*)p_buffer, '\0', responseLen);
        ((union TEEI_vfs_response*)p_buffer)->value = retVal;

        return responseLen;

    case TZ_VFS_LSEEK:

        IMSG_DEBUG(" come into the TZ_VFS_LSEEK function\n");

        retVal = lseek(tz_command->args.func_lseek_args.fd, tz_command->args.func_lseek_args.offset,
                       tz_command->args.func_lseek_args.origin);

        if (retVal == -1) {
            retVal = -(errno);
        }

        memset((void*)p_buffer, '\0', responseLen);
        ((union TEEI_vfs_response*)p_buffer)->value = retVal;

        return responseLen;

    case TZ_VFS_RENAME:

        IMSG_DEBUG(" come into the TZ_VFS_RENAME function\n");

        namelength = strlen((char*)param_address);

        IMSG_DEBUG(" namelength = %d\n", namelength);
        IMSG_DEBUG(" 1st name = %s\n", (char*)param_address);
        IMSG_DEBUG(" 2nd name = %s\n", (char*)((char*)param_address + namelength + 1));

        retVal = rename((char*)param_address, (char*)(param_address + namelength + 1));

        if (retVal == -1) {
            retVal = -(errno);
        }
        IMSG_DEBUG(" retVal = %ld\n", retVal);

        memset((void*)p_buffer, '\0', responseLen);
        ((union TEEI_vfs_response*)p_buffer)->value = retVal;

        return responseLen;

    case TZ_VFS_COPY:

        IMSG_DEBUG(" come into the TZ_VFS_COPY function\n");

        namelength = strlen((char*)param_address);

        IMSG_DEBUG(" namelength = %d\n", namelength);
        IMSG_DEBUG(" 1st name = %s\n", (char*)param_address);
        IMSG_DEBUG(" 2nd name = %s\n", (char*)((char*)param_address + namelength + 1));

        char* cpyCmd[] = {"cpyfile", (char*)param_address,
                          (char*)((char*)param_address + namelength + 1)};
        retVal = do_copy(3, cpyCmd);
        if (retVal == -1) {
            retVal = -(errno);
        }
        IMSG_DEBUG(" retVal = %ld\n", retVal);

        memset((void*)p_buffer, '\0', responseLen);
        ((union TEEI_vfs_response*)p_buffer)->value = retVal;
        return responseLen;

    case TZ_VFS_MKDIR:

        IMSG_DEBUG(" come into the TZ_VFS_MKDIR function\n");

        IMSG_DEBUG(" 1st name = %s\n", (char*)param_address);
        IMSG_DEBUG(" mode = %d\n", tz_command->args.func_mkdir_args.mode);

        retVal = mkdir((char*)param_address, tz_command->args.func_mkdir_args.mode);

        if (retVal == -1) {
            retVal = -(errno);
        }
        IMSG_DEBUG(" retVal = %ld\n", retVal);

        memset((void*)p_buffer, '\0', responseLen);
        ((union TEEI_vfs_response*)p_buffer)->value = retVal;

        return responseLen;

    case TZ_VFS_RMDIR:

        IMSG_DEBUG(" come into the TZ_VFS_RMDIR function\n");
        IMSG_DEBUG(" 1st name = %s\n", (char*)param_address);

        retVal = rmdir((char*)param_address);

        if (retVal == -1) {
            retVal = -(errno);
        }
        IMSG_DEBUG(" retVal = %ld\n", retVal);

        memset((void*)p_buffer, '\0', responseLen);
        ((union TEEI_vfs_response*)p_buffer)->value = retVal;

        return responseLen;

    case TZ_VFS_OPENDIR:

        IMSG_DEBUG(" come into the TZ_VFS_OPENDIR function\n");

        dir = opendir((char*)param_address);

        memset((void*)p_buffer, '\0', responseLen);
        ((union TEEI_vfs_response*)p_buffer)->p_dir = (unsigned long)dir;

        return responseLen;

    case TZ_VFS_READDIR:

        IMSG_DEBUG(" come into the TZ_VFS_READDIR function\n");

        dir = (DIR*)(uintptr_t)(tz_command->args.func_readdir_args.p_dir);
        read_count = tz_command->args.func_readdir_args.read_count;
        count = 0;

        while ((p_dirent = readdir(dir)) != NULL) {
            memcpy(
                (char*)((unsigned long)(tz_command) + PAGE_SIZE_4K + count * sizeof(struct dirent)),
                (char*)p_dirent, sizeof(struct dirent));

            count = count + 1;
            if (count == read_count)
                break;
        }

        memset((void*)p_buffer, '\0', responseLen);
        ((union TEEI_vfs_response*)p_buffer)->value = count;

        return PAGE_SIZE_4K + count * sizeof(struct dirent);

    case TZ_VFS_CLOSEDIR:

        IMSG_DEBUG(" come into the TZ_VFS_CLOSEDIR function\n");

        dir = (DIR*)(uintptr_t)(tz_command->args.func_closedir_args.p_dir);

        retVal = closedir(dir);

        memset((void*)p_buffer, '\0', responseLen);
        ((union TEEI_vfs_response*)p_buffer)->value = retVal;

        return responseLen;
#ifdef PRO_NODE
    case TZ_VFS_PRONODE:
        IMSG_DEBUG(" come into the TZ_VFS_PRONODE function\n");
        param_address = (char*)((unsigned long)(tz_command) + PAGE_SIZE_4K);
        retVal = strlen(PRO_NODE);
        if (retVal>0 && retVal<256) {
            memcpy((char *)param_address, PRO_NODE, retVal);
        }
        memset((void*)p_buffer, '\0', responseLen);
        ((union TEEI_vfs_response*)p_buffer)->value = retVal;
        return PAGE_SIZE_4K + retVal;
#endif
    case TZ_VFS_SETPROP:
        IMSG_DEBUG(" come into the TZ_VFS_SETPROP function\n");

        namelength = strlen((char*)param_address);

        IMSG_DEBUG(" proplength = %d\n", namelength);
        IMSG_DEBUG(" 1st key = %s\n", (char*)param_address);
        IMSG_DEBUG(" 2nd val = %s\n", (char*)((char*)param_address + namelength + 1));
#if PROP_NOT_SUPPORT_VENDOR
        // set prop
        retVal = property_set(param_address, (char*)((char*)param_address + namelength + 1));
#else
        full_path_name[0] = 0;
        strcat(full_path_name, SETTING_PROPKEY_HEAD);
        strcat(full_path_name, param_address);
        retVal = property_set(&full_path_name[0], (char*)((char*)param_address + namelength + 1));
#endif
        IMSG_DEBUG(" retVal = %ld\n", retVal);

        memset((void*)p_buffer, '\0', responseLen);
        ((union TEEI_vfs_response*)p_buffer)->value = retVal;
        return responseLen;


    default:
        IMSG_DEBUG(" come into the default branch!!!\n");

        memset(&tz_response, '\0', responseLen);
        tz_response.value = -EOPNOTSUPP;
        memset((void*)p_buffer, '\0', responseLen);
        memcpy((void*)p_buffer, &tz_response, responseLen);
        return responseLen;
    }

    return 0;
}

static int init_soter_fn(void)
{
    int fd = open("/dev/teei_config", O_RDWR);
    __u32 i;

    IMSG_INFO("come in init soter thread");

    IMSG_DEBUG("driver to be loaded:\n");
    for (i = 0; i < init_param.uuid_count; i++) {
        IMSG_DEBUG("UUID=%s\n", init_param.uuids[i]);
    }

    int ret = ioctl(fd, TEEI_CONFIG_IOCTL_INIT_TEEI, &init_param);
    if (0 != ret) {
        IMSG_ERROR("can't init soter ,ret = %d !", ret);

    } else {
        IMSG_INFO("init soter finish. flag = %d", init_param.flag);
        property_set("vendor.soter.teei.init", "INIT_OK");
    }
    close(fd);

    return 0;
}

/****************************KEYMASTER FUNCTIONS***********************/
/* for unlocking keymaster,and then can load tee in (kernel/tz_driver) */

static void keymaster_unlock(void)
{
    int fd = open("/dev/teei_config", O_RDWR);
    int flag = 0;

    int ret = ioctl(fd, TEEI_CONFIG_IOCTL_UNLOCK, &flag);
    if (0 != ret) {
        IMSG_ERROR("keymaster unlock boot_decrypt_lock failed!\n");
    } else {
        IMSG_INFO("keymaster unlock boot_decrypt_lock success\n");
    }
    close(fd);
}

/* for the system first time boot, then the keymaster can store the key
 * in rpmb block 38, this rpmb place is just for all disk encrypt rsa key */

static int load_tee_fn(void)
{
    IMSG_INFO("come in load tee thread");
    char value[PROPERTY_VALUE_MAX] = {0};
    char value_type[PROPERTY_VALUE_MAX] = {0};
    char value_state[PROPERTY_VALUE_MAX] = {0};
    char vendor_value_state[PROPERTY_VALUE_MAX] = {0}; //used for meta and factory first time start up

    property_get("ro.crypto.state", value, "");
    property_get("ro.crypto.type", value_type, "");

    if (strcmp("unencrypted", value) != 0 && strcmp("unsupported", value) != 0)
    {
        /*data encrypted, wait for decrption.*/
        if(strcmp("file", value_type) != 0) {
            property_get("vold.decrypt", value, "");
            while (strcmp("trigger_restart_framework", value) != 0) {

                property_get("vendor.soter.teei.crypto.state", vendor_value_state, "");
                if (strcmp("unencrypted", vendor_value_state) == 0) {
                    IMSG_INFO("meta or factory firsttime start up");
                    break;
                }

                /*still decrypting... wait one second.*/
                usleep(10000);
                property_get("vold.decrypt", value, "");
            }
        }
    }

	IMSG_INFO("daemon unlock keymaster signal to tz-driver");
    keymaster_unlock();

    unlock_state = 1;

    return 0;
}
/****************************MAIN THREAD ******************************/
int main(int argc, char** argv) {

    int vfs_fd = 0, len = 0;
    int retVal = 0;
    unsigned char* rw_buffer = NULL;
    int opt;
    int i = 0;

    pthread_t ntid = 0;
    pthread_t loadtee_id = 0;
    pthread_t first_time_boot_id = 0;

    IMSG_INFO("daemon version [%s]", UTOS_VERSION);
    property_set("vendor.soter.teei.init", "INIT_START");

    memset((void *)&init_param, 0, sizeof(struct init_param));

    rw_buffer = (unsigned char*)malloc(BUFFER_SIZE);
    if (rw_buffer == NULL) {
        IMSG_ERROR("daemon can not malloc enough memory\n");
        return -1;
    }
    init_param.flag = 0;
    while ((opt = getopt(argc, argv, "r:t:h")) != -1) {
        switch (opt) {
        case 'r':
#if !defined(MTK_DRM_SUPPORT) && !defined(MTK_CAM_SUPPORT)
            break;
#endif
            if (strlen(optarg) != UUID_LEN) {
                IMSG_ERROR("UUID length should be %d (uuid=%s)\n", UUID_LEN, optarg);
                exit(EXIT_FAILURE);
            }
            if(init_param.uuid_count >= MAX_DRV_UUIDS)
            {
                IMSG_ERROR("Too Much TA Boot -- MAX_DRV_UUIDS[%d]\n", MAX_DRV_UUIDS);
                break;
            }
            memcpy((void*)init_param.uuids[init_param.uuid_count], optarg, UUID_LEN);
            init_param.flag = init_param.flag & (~(0x1 << init_param.uuid_count));
            init_param.uuid_count++;
            break;
        case  't':
#if !defined(MTK_DRM_SUPPORT) && !defined(MTK_CAM_SUPPORT)
            break;
#endif
            if (strlen(optarg) != UUID_LEN) {
                IMSG_ERROR("UUID length should be %d (uuid=%s)\n", UUID_LEN, optarg);
                exit(EXIT_FAILURE);
            }

            if(init_param.uuid_count >= MAX_DRV_UUIDS)
            {
                IMSG_ERROR("Too Much TA Boot -- MAX_DRV_UUIDS[%d]\n", MAX_DRV_UUIDS);
                break;
            }
            memcpy((void*)init_param.uuids[init_param.uuid_count], optarg, UUID_LEN);
	    IMSG_INFO("[%s][%d]TA UUID[%s]\n", __func__, __LINE__, init_param.uuids[init_param.uuid_count]);
            init_param.flag = init_param.flag | (0x1 << init_param.uuid_count);
            init_param.uuid_count++;
            break;

        case 'h':
        default:
            fprintf(stderr, "Usage: %s [-h|-r|-t <UUID>]\n", argv[0]);
            exit(EXIT_FAILURE);
        }
    }

#ifndef MIN_MEM_SUPPORT
    for (i = 0; i < REAL_DRV_UUIDS; i++) {
        if (strlen(drv_uuid[i]) != UUID_LEN) {
            IMSG_ERROR("UUID length should be %d, (drv_uuid[%d]=%s)\n", UUID_LEN, i, drv_uuid[i]);
            break;
        }
        memcpy((void*)init_param.uuids[init_param.uuid_count], drv_uuid[i], UUID_LEN);
        init_param.uuid_count++;
    }
#endif
    while (1)
    {
        vfs_fd = open(DEV_FILE, O_RDWR);
        if (vfs_fd < 0) {
            IMSG_ERROR("daemon wait open the vfs node %d\n", vfs_fd);
            continue;
        }
        break;
    }
    /*create a thread for start data area working*/
    pthread_create(&ntid, NULL, (void*)init_soter_fn, NULL);
    pthread_create(&loadtee_id, NULL, (void*)load_tee_fn, NULL);

    while (1)
    {

        len = read(vfs_fd, rw_buffer, BUFFER_SIZE);
        if (len < 0) {
            IMSG_ERROR("can not read the vfs node,ret =%d ret =%d\n", len, errno);
            usleep(10000);
            continue;
        }

        retVal = vfs_analysis_command(rw_buffer);
        if (retVal < 0) {
            IMSG_ERROR("invail command read from vfs node %d\n",retVal);
            continue;
        }

        len = retVal;
        retVal = write(vfs_fd, rw_buffer, len);
        if (retVal < 0) {
            IMSG_ERROR("can not write to vfs node %d\n",retVal);
            continue;
        }
    }

    free(rw_buffer);
    close(vfs_fd);

    IMSG_INFO("daemon start OK ...");
    return 0;
}
