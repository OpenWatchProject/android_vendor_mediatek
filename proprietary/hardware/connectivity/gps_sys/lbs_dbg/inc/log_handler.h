#ifndef __Log_handler_H__
#define __Log_handler_H__

#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <sys/un.h>
#include <sys/epoll.h>
#include <sys/stat.h>
#include <dirent.h>
#include <cutils/sockets.h>
#include <log/log.h>     /*logging in logcat*/
#define LOGD(fmt, arg ...) ALOGD("%s: " fmt, __FUNCTION__ , ##arg)
#define LOGW(fmt, arg ...) ALOGW("%s: " fmt, __FUNCTION__ , ##arg)
#define LOGE(fmt, arg ...) ALOGE("%s: " fmt, __FUNCTION__ , ##arg)

#define MAX_PATH_LEN    256
#define LBS_LOG_INTERFACE_BUFF_CMD_SIZE 512


#define MNLD_STRNCPY(dst,src,size) do{\
                                       strncpy((char *)(dst), (src), (size - 1));\
                                      (dst)[size - 1] = '\0';\
                                     }while(0)


struct log_thread_database {
    FILE*           file;
    char*           sock_addr;
    char*           log_name_prefix;
    char*           log_name_suffix;
    char*           log_name_suffix_writing;
    char            log_folder_name[MAX_PATH_LEN];
    char            log_name_base[MAX_PATH_LEN];

    unsigned int    max_log_size;
    unsigned int    max_folder_size;
    short           max_log_num;


    unsigned int    current_log_size;
    unsigned int    current_folder_size;
    short           current_log_num;

    short           split_number;

    bool            (*split_log)(struct log_thread_database*  database);
    bool            (*check_dir)(struct log_thread_database*  database);
    bool            (*log_rename)(struct log_thread_database*  database);
};

typedef struct log_thread_database lbs_log_thread_database;

typedef struct {
    bool     (*create_logfile)(const char* path, lbs_log_thread_database* database);
    bool     (*write_logdata)(char* data, int len, lbs_log_thread_database* database);
    bool     (*close_logfile)(lbs_log_thread_database* database);
} lbslogd_handler_interface;

typedef enum{
  LBS_DBG_CREATE_DIR = 0,
  LBS_DBG_MUTEX_MAX
} lbs_dbg_mutex_enum;

#endif

