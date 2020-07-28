/*
 *    Filename: flashlessd.c
 *
 * Description: VIA CBP funtion for Linux
 *
 *      Author: qli@via-telecom.com
 *     Created: 07/20/2012
 *     Version: 1.0
 *    Revision:
 *              1. 07/19/2013 License statements added by xpwan@via-telecom.com
 *
 * This Software is the property of VIA Telecom, Inc. and may only be used pursuant to a license from VIA Telecom, Inc.
 * Any unauthorized use inconsistent with the terms of such license is strictly prohibited.
 *
 * Copyright (c) 2012-2013 VIA Telecom, Inc. All rights reserved.
 */

/**************************/
/* INCLUDES               */
/**************************/
#include <errno.h>
#include <fcntl.h>
//#include <features.h>
#include <paths.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <termios.h>
#include <time.h>
#include <unistd.h>
#include <pthread.h>
#include <dirent.h>
#include <ctype.h>
#include <cutils/properties.h>
#include <cutils/android_filesystem_config.h>
#include <c2kutils.h>
#include "c2k_support.h"
#include "com_intf.h"

/**************************/
/* DEFINES                */
/**************************/
//#include <syslog.h>

//To support cbp ramdump image uploading function
#define RAMDUMP_FUNC_SUPPORTED

static char *my_time();

#undef LOG_TAG
#define LOG_TAG "Flashless"
#include <utils/Log.h> //all Android LOG macros are defined here.
#define LOGFLS(lvl,f,...) \
  do{ \
    if(logToFile){ \
        if(lvl <= logLevel){ \
            if(logFile){ \
                fprintf(logFile, "%s %d:%s(): " f, my_time(), __LINE__, __FUNCTION__, ##__VA_ARGS__);\
                fflush(logFile); \
            }else{ \
                fprintf(stderr,"%d:%s(): " f, __LINE__, __FUNCTION__, ##__VA_ARGS__);\
            }  \
        }  \
    }else{ \
        if(lvl <= logLevel) \
            LOG_PRI(android_log_lvl_convert[lvl],LOG_TAG,"%d:%s(): " f, __LINE__, __FUNCTION__, ##__VA_ARGS__);\
    } \
  }while(0)

//just dummy defines since were not including syslog.h.
#define LOG_EMERG    0
#define LOG_ALERT    1
#define LOG_CRIT    2
#define LOG_ERR        3
#define LOG_WARNING    4
#define LOG_NOTICE    5
#define LOG_INFO    6
#define LOG_DEBUG    7


/* Android's log level are in opposite order of syslog.h */
int android_log_lvl_convert[8]={ANDROID_LOG_SILENT, /*8*/
                                ANDROID_LOG_SILENT, /*7*/
                                ANDROID_LOG_FATAL, /*6*/
                                ANDROID_LOG_ERROR,/*5*/
                                ANDROID_LOG_WARN,/*4*/
                                ANDROID_LOG_INFO,/*3*/
                                ANDROID_LOG_DEBUG,/*2*/
                                ANDROID_LOG_VERBOSE};/*1*/

#define ASCI_MAX_PAYLOAD_SIZE (254)
#ifdef CBP_PHY_SDIO
#define CBP_MAX_PAYLOAD_SIZE (2040 * 8 - 18)/*for decreasing ack packages */
#define MAX_DATA_WINDOW (1) /*it must be 1 for sdio*/
#define BOOT_LOAD_ADDR (0x51000000)
#define BOOT_EXE_ADDR (0x51000028)
#else
#define CBP_MAX_PAYLOAD_SIZE (940)
#define MAX_DATA_WINDOW (3)
#define BOOT_LOAD_ADDR (0x00)
#define BOOT_EXE_ADDR (0x20)
#endif

#define DEVICE_DETECT_TIMES (100)
#define DEVICE_DETECT_WAIT (100)
#define CBP_MAX_UPLOAD_SIZE (240)
#define CBP_MAX_FRAME_SIZE (CBP_MAX_PAYLOAD_SIZE * 2 + 64)

#define UPLOAD_WAIT (20)

#define BLOCK_SIZE (1024)
#define MAX_DEBUG_LOG_SIZE (512 * BLOCK_SIZE)

#ifndef min
#define min(a,b) ((a < b) ? a :b)
#endif

#ifdef MD3_SUPPORT_3g
#define BOOT_FILE_PATH   "/vendor/firmware/boot_3_3g_n.rom"
#define CP_FILE_PATH     "/vendor/firmware/modem_3_3g_n.img"

/*Version 1*/
#define FSM_FILE_PATH_DF "/vendor/firmware/fsm_df_3_3g_n.img"

/*Version 2. New rule: Seperate fsm_df.img into three images, requested by c2k md side
* fsm_rf_df.img, fsm_rw_df.img and fsm_cust_df.img will not coexist with fsm_df.img
*/
#define FSM_RF_FILE_PATH_DF     "/vendor/firmware/fsm_rf_df_3_3g_n.img"
#define FSM_RW_FILE_PATH_DF     "/vendor/firmware/fsm_rw_df_3_3g_n.img"
#define FSM_CUST_FILE_PATH_DF   "/vendor/firmware/fsm_cust_df_3_3g_n.img"

#define CHECK_HEADER_SIZE    (0)

#elif defined MD3_SUPPORT_2g
#define BOOT_FILE_PATH   "/vendor/firmware/boot_3_2g_n.rom"
#define CP_FILE_PATH     "/vendor/firmware/modem_3_2g_n.img"

/*Version 1*/
#define FSM_FILE_PATH_DF "/vendor/firmware/fsm_df_3_2g_n.img"

/*Version 2. New rule: Seperate fsm_df.img into three images, requested by c2k md side*/
#define FSM_RF_FILE_PATH_DF     "/vendor/firmware/fsm_rf_df_3_2g_n.img"
#define FSM_RW_FILE_PATH_DF     "/vendor/firmware/fsm_rw_df_3_2g_n.img"
#define FSM_CUST_FILE_PATH_DF   "/vendor/firmware/fsm_cust_df_3_2g_n.img"

#define CHECK_HEADER_SIZE    (0)

#else
#define BOOT_FILE_PATH   "/system/etc/flashless/boot.rom"
#define CP_FILE_PATH     "/system/etc/flashless/cp.rom"
#define FSM_FILE_PATH_DF "/system/etc/flashless/fsm_df.img"

/*Version 2. New rule: Seperate fsm_df.img into three images, requested by c2k md side*/
#define FSM_RF_FILE_PATH_DF     "/system/etc/flashless/fsm_rf_df.img"
#define FSM_RW_FILE_PATH_DF     "/system/etc/flashless/fsm_rw_df.img"
#define FSM_CUST_FILE_PATH_DF   "/system/etc/flashless/fsm_cust_df.img"

#define CHECK_HEADER_SIZE    (0)
#endif

/*Version 1. */
#define FSM_FILE_PATH_0  "/data/vendor/flashless/fsm_0.img"
#define FSM_FILE_PATH_1  "/data/vendor/flashless/fsm_1.img"

/*Version 2. New rule: Seperate fsm_df.img into three images, requested by c2k md side*/
#define FSM_RF_TEMP_PATH       "/data/vendor/flashless/fsm_rf_temp.img"
#define FSM_RW_FILE_PATH_0     "/data/vendor/flashless/fsm_rw_0.img"
#define FSM_RW_FILE_PATH_1     "/data/vendor/flashless/fsm_rw_1.img"
#define FSM_CUST_FILE_PATH_0   "/data/vendor/flashless/fsm_cust_0.img"
#define FSM_CUST_FILE_PATH_1   "/data/vendor/flashless/fsm_cust_1.img"

/*common images*/
#define CRASH_FILE_PATH  "/data/vendor/flashless/crash.img"
#define LOG_FILE_PATH    "/data/vendor/flashless/debug.log"

#ifdef RAMDUMP_FUNC_SUPPORTED
/* default dir for ramdump image */
#define RAMDUMP_FILE_DEFAULT_PATH   "/data/vendor/flashless"
#define RAMDUMP_FILE_REL_DIR_NAME   "vialog"

/* parse this to check sdcard mounted or not */
#define SYS_MOUNTS_FILE     "/proc/mounts"
#endif

/*Version 1. */
#define FSM_INDEX_PROP   "persist.vendor.radio.flashless.fsm"

/*Version 2. */
#define FSM_PROP_NAP_TIME (100000)  //usleep 100ms
#define FSM_PROP_MAXNAPS (10)
#define BOOT_MODE_FILE            "/sys/class/BOOT/BOOT/boot/boot_mode"
#define META_MODE                '1'
#define FSM_CUST_INDEX_PROP "persist.vendor.radio.flashless.fsm_cst"
#define FSM_RW_INDEX_PROP "persist.vendor.radio.flashless.fsm_rw"

/**************************/
/* CONSTANTS & GLOBALS    */
/**************************/
#define    BOOTLOADER       ((short)0x00E0)
#define CPFLASHPROGRAM ((short)0x04B2)

//the key word for the package
#define SYNC (0xFE)

/**************************/
/* TYPES                  */
/**************************/
#ifndef uint8
typedef unsigned char     uint8;
#endif
#ifndef uint16
typedef unsigned short    uint16;
#endif
#ifndef uint32
typedef unsigned int      uint32;
#endif

/*
 * factory mode : 0
 * normal mode  : 1
 */
typedef enum RunningMode{
    MODE_META = 0,
    MODE_NORMAL,
    MODE_END
}RunningMode;

typedef struct FlashlessFrame
{
    uint8 buf[CBP_MAX_FRAME_SIZE];
    uint16 dlen;//data length in buffer
    uint16 id;//message id
    uint16 plen;//payload length in buffer
    uint16 base; //the buf index to parse a frame
    uint8 *payload;//the point fro payload in buffer
} FlashlessFrame;

typedef enum FlashlessDeviceType{
    DEV_TYPE_ASCI,
    DEV_TYPE_CBP,
    DEV_TYPE_NUMS
}FlashlessDeviceType;

typedef enum FlashlessFrameType{
    FRAME_TYPE_ASCI_ACK = 0x00,
    FRAME_TYPE_ASCI_NACK = 0x01,
    FRAME_TYPE_ASCI_HANDSHAKE = 0x02,
    FRAME_TYPE_ASCI_HEADER = 0x03,
    FRAME_TYPE_ASCI_DATA = 0x04,
    FRAME_TYPE_ASCI_CHECKSUM = 0x05,
    FRAME_TYPE_ASCI_RUN_BOOT = 0x06,
    FRAME_TYPE_ASCI_RUN_CBP = 0x07,
    FRAME_TYPE_CBP_DOWNLOAD_CMD = 0x0500,
    FRAME_TYPE_CBP_DOWNLOAD_RSP = 0x0500,
    FRAME_TYPE_CBP_DOWNLOAD_PKG_CMD = 0x04B2,
    FRAME_TYPE_CBP_DOWNLOAD_PKG_RSP = 0x04B2,
    FRAME_TYPE_CBP_ERASE_CMD = 0x0501,
    FRAME_TYPE_CBP_ERASE_RSP = 0x0501,
    FRAME_TYPE_CBP_DOWNLOAD_DATA_CMD = 0x0502,
    FRAME_TYPE_CBP_DOWNLOAD_DATA_RSP = 0x0502,
    FRAME_TYPE_CBP_FSM_UPLOAD_CMD = 0x0503,
    FRAME_TYPE_CBP_FSM_UPLOAD_RSP = 0x0503,
    FRAME_TYPE_CBP_CRASH_UPLOAD_CMD = 0x0504,
    FRAME_TYPE_CBP_CRASH_UPLOAD_RSP = 0x0504,
    FRAME_TYPE_CBP_FSM_ERR = 0x0505,
    FRAME_TYPE_CBP_BOOTLOADER_START = 0x0506,
    FRAME_TYPE_CBP_BOOTLOADER_END = 0x0507,
    FRAME_TYPE_CBP_FSM_CALIBRATE_CMD = 0x0508,
    FRAME_TYPE_CBP_FSM_CALIBRATE_RSP = 0x0508,

#ifdef RAMDUMP_FUNC_SUPPORTED
    FRAME_TYPE_CBP_RAMDUMP_UPLOAD_CMD = 0x0509,
    FRAME_TYPE_CBP_RAMDUMP_UPLOAD_RSP = 0x0509,
#endif
    FRAME_TYPE_CBP_FSM_UPLOAD_CMD_V2 = 0x050A,
    FRAME_TYPE_CBP_FSM_UPLOAD_RSP_V2 = 0x050A,
    FRAME_TYPE_UNKNOW = 0x1FFF,
    FRAME_TYPE_TIMEOUT
}FlashlessFrameType;


typedef enum FrameSectionType{
    SECTION_CP            = 0x01,
    SECTION_FSM            = 0x02,
    SECTION_LOG            = 0x03,
    /*fsm_rf, fsm_cust, fsm_rw will not coexist with section_fsm*/
    SECTION_FSM_RF        = 0x04,
    SECTION_FSM_CUST    = 0x05,
    SECTION_FSM_RW        = 0x06,
    SECTION_NUMS
}FrameSectionType;

typedef enum FSMImageSyncType
{
    FSM_SYNC_ACT_TIMER = 0,
    FSM_SYNC_ACT_POWRDOWN = 1,
    FSM_SYNC_ACT_FLUSH = 2,
    FSM_SYNC_ACT_VERSION_MISMATCH = 3,
    FSM_SYNC_ACT_NUMS
}FSMImageSyncType;

typedef enum FlashProgType{
    PROG_INIT = 0x00,
    PROG_DATA = 0x01,
    PROG_NUMS
}FlashProgType;

typedef enum FlashProgAckType{
    PROG_ACK = 0x00,
    PROG_NACK = 0x01,
}FlashProgAckType;

typedef enum FlashlessStates
{
    FLS_STATE_ASCI_HANDSHAKE,
    FLS_STATE_WAIT_REQUEST,
    FLS_STATE_IMAGE_DOWNLOAD,
    FLS_STATE_IMAGE_UPLOAD,
    FLS_STATES_COUNT // keep this the last
} FlashlessStates;

typedef struct FlashlessDev{
    int fd;
    pthread_t tid;
    FlashlessFrame rframe;
    FlashlessFrame wframe;
}FlashlessDev;

typedef struct FlashlessHandle
{
    uint32 frame_pending;
    FlashlessStates state;
    FlashlessDev *asci;
    FlashlessDev *cbp;
} FlashlessHandle;


#ifdef RAMDUMP_FUNC_SUPPORTED
#define SUPPORT_EXTRA_FUNC_NONE       0x0
/*Function bit for RAMDUMP*/
#define SUPPORT_EXTRA_FUNC_RAMDUMP    0x1

static uint32 flsExtraFunctions = 0;
#endif

static int baudRates[] = {
    0, 9600, 19200, 38400, 57600, 115200, 230400, 460800, 921600
};
static speed_t baudBits[] = {
    0, B9600, B19200, B38400, B57600, B115200, B230400, B460800, B921600
};


//Flashless package define
#pragma pack(1)

//Download Boot to RAM header struct
typedef struct ASCI_MSG_HEADER {
    uint32 LoadAddr;
    uint32 ExeAddr;
}ASCI_MSG_HEADER;

//Download Boot to RAM checksum struct "FE 05 05 69 55 13 00 " "69 55 13 00 is the checksum"
typedef struct ASCI_MSG_CHECKSUM {
    uint32 checksum;
}ASCI_MSG_CHECKSUM;

typedef struct CBP_MSG_DOWNLOAD_CMD {
    uint16  id;
    uint8   section;
}CBP_MSG_DOWNLOAD_CMD;

typedef struct CBP_MSG_DOWNLOAD_RSP {
    uint16  id;
    uint8   section;
    uint32  size;
}CBP_MSG_DOWNLOAD_RSP;

typedef struct CBP_MSG_DOWNLOAD_DATA_CMD {
    uint16  id;
    uint32  seq;
    uint8   type;
    uint8   section;
    uint16  checksum;
    uint16  size;
}CBP_MSG_DOWNLOAD_DATA_CMD;

typedef struct CBP_MSG_DOWNLOAD_DATA_RSP {
    uint16  id;
    uint32  seq;
    uint8   TypeAck;
}CBP_MSG_DOWNLOAD_DATA_RSP;

typedef struct download_data_cache{
    uint8 sync[4];
    uint16 len;
    CBP_MSG_DOWNLOAD_DATA_CMD cmd;
    uint8 data[CBP_MAX_PAYLOAD_SIZE];
}download_data_cache;

typedef struct CBP_MSG_ERASE_CMD {
    uint16  id;
    uint8   section;
}CBP_MSG_ERASE_CMD;

typedef struct CBP_MSG_ERASE_RSP {
    uint16  id;
    uint8   section;
    uint8   status;
}CBP_MSG_ERASE_RSP;

enum OPERATE_STATUS{
    STATUS_OK = 0,
    STATUS_BAD_SEQ,
    STATUS_NO_MEM,
    STATUS_ERROR,
    STATUS_DENY
};

/*upload command is only used to upload crash log*/
typedef struct CBP_MSG_UPLOAD_CMD {
    uint16  id;
    uint32  seq;
    uint32  offset;
    uint16  length;
}CBP_MSG_UPLOAD_CMD;

typedef struct CBP_MSG_UPLOAD_RSP {
    uint16  id;
    uint32  seq;
    uint32  status;
}CBP_MSG_UPLOAD_RSP;

typedef struct CBP_MSG_FSM_CALIBRATE_CMD {
    uint16 id;
}CBP_MSG_FSM_CALIBRATE_CMD;

typedef struct CBP_MSG_FSM_CALIBRATE_RSP {
    uint16 id;
    uint32 status;
}CBP_MSG_FSM_CALIBRATE_RSP;
#pragma pack()

// config stuff
static char *revision = "2.0.1";
static char *asciDevice;
static char *cbpDevice;
static int little_endian = 1;
static int noDaemon = 0;
static int useTimeout = 5;
static int useRetry = 2;
static int comSpeed = 5; //115200 baud rate
static int logToFile = 0;
static int logLevel = LOG_NOTICE;
static FILE * logFile = NULL;
static FlashlessHandle *phd;
static time_t tStart;
static struct timeval tv_b,tv_e;

static int fsm_rw_index;
static int fsm_cust_index;
static int cur_download_fsm_type;
static int fsm_index;

static int modem_reset;
static int run_mode = MODE_NORMAL;

#ifdef RAMDUMP_FUNC_SUPPORTED
static char ramdump_img_path[512] = RAMDUMP_FILE_DEFAULT_PATH;
#endif

//pthread
pthread_t recv_frame_thread;
pthread_cond_t send_frame_signal = PTHREAD_COND_INITIALIZER;
pthread_cond_t recv_frame_signal = PTHREAD_COND_INITIALIZER;
pthread_attr_t thread_attr;
pthread_mutex_t handle_lock = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t frame_lock = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t dump_lock = PTHREAD_MUTEX_INITIALIZER;

//add timestamp in separate log file
//format: 03-29 09:22:35.887
static char log_ts[20] = {0};
static char *my_time(void)
{
    struct timeval tv;
    struct tm * tm = NULL;

    gettimeofday(&tv, NULL);
    tm = localtime(&(tv.tv_sec));
    memset(log_ts, 0, sizeof(log_ts));
    sprintf(log_ts, "%02d-%02d %02d:%02d:%02d.%03ld", tm->tm_mon + 1,
                                 tm->tm_mday,
                                 tm->tm_hour,
                                 tm->tm_min,
                                 tm->tm_sec,
                                 tv.tv_usec/1000);
    return log_ts;
}

#ifdef RAMDUMP_FUNC_SUPPORTED
/* generate four bit random number for ramdump image naming*/
static int ramdump_file_name_random_number(char num[], int len)
{
    if(!num || len != 5) {
        LOGFLS(LOG_ERR, "invalid args\n");
        return -1;
    }
    srand((unsigned)(time(NULL)));
    memset(num, 0, len);
    snprintf(num, len - 1, "%d", rand() % (9999 - 1000 + 1) + 1000);
    num[len - 1] = '\0';
    return 0;
}

/* generate timestamp for ramdump image naming, eg 2010-01-02-05-57-13*/
static int ramdump_file_name_timestamp(char ts[], int len)
{
    time_t t;
    if(!ts || len != 20) {
        LOGFLS(LOG_ERR, "invalid args\n");
        return -1;
    }

    t = time(NULL);
    memset(ts, 0, len);
    strftime(ts, len, "%Y-%m-%d-%H-%M-%S", localtime(&t));
    return 0;
}

#if 0
static int is_meta_mode(void)
{
    int fd, ret;
    size_t s;
    volatile char data[20];

    fd = open(BOOT_MODE_FILE, O_RDONLY);
    if (fd < 0) {
        LOGFLS(LOG_ERR, "fail to open %s: ", BOOT_MODE_FILE);
        perror("");
        return 0;
    }

    s = read(fd, (void *)data, sizeof(char) * 3);
    if (s <= 0) {
        LOGFLS(LOG_ERR,"fail to read %s", BOOT_MODE_FILE);
        perror("");
        ret = 0;
    } else {
        if (data[0] == META_MODE) {
            ret = 1;
        } else {
            ret = 0;
        }
    }

    close(fd);

    return ret;
}
#endif

static char *is_sdcard_mounted(void)
{
    FILE *fp = NULL;
    char line[1024] = {0};
    char *e_sdcard = NULL;
    char *i_sdcard = NULL;
    int  is_ext_sd_mounted = 0;
    int  is_in_sd_mounted  = 0;

    if (access(SYS_MOUNTS_FILE, R_OK) < 0) {
        LOGFLS(LOG_ERR, "No permission to read %s\n", SYS_MOUNTS_FILE);
        return NULL;
    }

    fp = fopen(SYS_MOUNTS_FILE,"r");
    if (fp == NULL){
        LOGFLS(LOG_ERR, "Fail to open %s\n", SYS_MOUNTS_FILE);
        return NULL;
    }

    e_sdcard = get_customized_external_sdcard_path();
    i_sdcard = get_customized_internal_sdcard_path();
    //if both external sdcard and internal sdcard are mounted, choose external sdcard first
    while (fgets(line, sizeof(line), fp) != NULL) {
        if (e_sdcard) {
            if (strstr(line, e_sdcard)) {
                is_ext_sd_mounted = 1;
                break;
            }
        }

        if (i_sdcard) {
            if (strstr(line, i_sdcard)) {
                is_in_sd_mounted = 1;
                continue;
            }
        }
    }

    fclose(fp);

    if (is_ext_sd_mounted) {
        return e_sdcard;
    }
    else if (is_in_sd_mounted) {
        return i_sdcard;
    }

    return NULL;
}

static void sdcard_mount_event_monitor(int sig)
{
    char old[512] = {0};
    char new[512] = {0};
    char buf[512]  = {0};
    int o_fd = -1;
    int n_fd = -1;
    int ret  = -1;
    int count = 0;
    int sum = 0;
    DIR *dirptr = NULL;
    struct dirent *entry   = NULL;
    char *sdcard_mount_dir = NULL;

    if(sig == SIGALRM) {
        sdcard_mount_dir = is_sdcard_mounted();
        if(!sdcard_mount_dir) {
            return;
        }

        memset(buf, 0, sizeof(buf));
        sprintf(buf, "%s/%s", sdcard_mount_dir, RAMDUMP_FILE_REL_DIR_NAME);
        if(access(buf, F_OK) < 0){
            if (0 != mkdir(buf, S_IRUSR | S_IWUSR)) {
                LOGFLS(LOG_ERR, "fail to create %s, (error:%s, errorno:%d)\n",
                        buf,  strerror(errno), errno);
                return;
            }
        }

        if((dirptr = opendir(RAMDUMP_FILE_DEFAULT_PATH)) == NULL) {
            return ;
        }

        while((entry = readdir(dirptr)) != NULL) {
            if(strstr(entry->d_name, "via_ramdump")) {
                memset(old, 0, sizeof(old));
                memset(new, 0, sizeof(new));
                sprintf(old, "%s/%s", RAMDUMP_FILE_DEFAULT_PATH, entry->d_name);
                buf[511] = 0;
                sprintf(new, "%s/%s", buf, entry->d_name);
                o_fd = open(old, O_RDONLY);
                if(o_fd < 0) {
                    LOGFLS(LOG_ERR, "fail to open old %s, (error:%s, errorno:%d)\n",
                            old, strerror(errno), errno);
                    break;
                }

                n_fd = open(new, O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR);
                if(n_fd < 0) {
                    LOGFLS(LOG_ERR, "fail to create new %s, (error:%s, errorno:%d)\n",
                            new, strerror(errno), errno);
                    close(o_fd);
                    break;
                }

                count = 0;
                memset(buf, 0, sizeof(buf));
                while((count = read(o_fd, buf, sizeof(buf) - 1)) != 0) {
                    if(count < 0 ) {
                        LOGFLS(LOG_ERR, "fail to read old %s, (error:%s, errorno:%d)\n",
                                old, strerror(errno), errno);
                        goto _exit;
                    }
                    sum = 0;
                    do {
                        ret = write(n_fd, buf + sum, count);
                        if (ret <= 0) {
                            LOGFLS(LOG_ERR, "fail to write new %s, (error:%s, errorno:%d)\n",
                                    new, strerror(errno), errno);
                            goto _exit;
                        }
                        sum += ret;
                    } while(sum < count);
                }
_exit:
                close(o_fd);
                close(n_fd);

                if(count == 0) {
                    if(unlink(old) < 0) {
                        LOGFLS(LOG_ERR, "fail to delete original %s, (error:%s, errorno:%d)\n",
                                old, strerror(errno), errno);
                        break;
                    }
                }
            }
        }
        closedir(dirptr);
    }
}

/*size should be enough to hold full path, 512 bytes by default*/
static int build_ramdump_image_name(char name[], int size)
{
    int ret = -1;
    int len = 0;

    char ts[20] = {0};
    char random[5] = {0};
    char path[256] = {0};
    char *sdcard_mount_dir = NULL;

    memset(name, 0, size);
    memset(path, 0, sizeof(path));

    sdcard_mount_dir = is_sdcard_mounted();
    if(sdcard_mount_dir) {
        sprintf(path, "%s/%s", sdcard_mount_dir, RAMDUMP_FILE_REL_DIR_NAME);
        len = strlen(path);
        if(len >= size - 1) {
            LOGFLS(LOG_WARNING, "target dir %s too long\n", path);
            strncpy(name, RAMDUMP_FILE_DEFAULT_PATH, strlen(RAMDUMP_FILE_DEFAULT_PATH));
        }
        else {
            if(access(path, F_OK) < 0){
                if (0 != mkdir(path, S_IRUSR | S_IWUSR)) {
                    LOGFLS(LOG_ERR, "fail to create %s, (error:%s, errorno:%d)\n",
                            path,  strerror(errno), errno);
                    strncpy(name, RAMDUMP_FILE_DEFAULT_PATH, strlen(RAMDUMP_FILE_DEFAULT_PATH));
                }
                else {
                    strncpy(name, path, len);
                }
            }
            else {
                strncpy(name, path, len);
            }
        }
    }
    else {
        strncpy(name, RAMDUMP_FILE_DEFAULT_PATH, strlen(RAMDUMP_FILE_DEFAULT_PATH));
    }

    LOGFLS(LOG_NOTICE, "target ramdump parent path: %s\n", name);
    len = strlen(name);
    if(name[len - 1] != '/') {
        name[len] = '/';
    }

    ret = ramdump_file_name_timestamp(ts, sizeof(ts));
    if (ret != 0) {
        LOGFLS(LOG_ERR, "Fail to generate time stamp\n");
        return -1;
    }

    ret = ramdump_file_name_random_number(random, sizeof(random));
    if (ret != 0) {
        LOGFLS(LOG_ERR, "Fail to generate random number\n");
        return -1;
    }

    len = strlen(name) + strlen("via_ramdump_") + strlen(ts) + strlen("_") + strlen(random) + strlen(".img");
    if(len < size) {
        sprintf(name + strlen(name), "via_ramdump_%s_%s.img", ts, random);
        LOGFLS(LOG_DEBUG, "dump image: %s\n", name);
    }
    else {
        LOGFLS(LOG_ERR, "length of ram dump image path name exceeds the size[%d] of the buffer\n", size);
        return -1;
    }
    return 0;
}
#endif //end of ramdump related internal routines definition

/*
* Purpose:  Determine baud-rate index for CMUX command
* Input:        baud_rate - baud rate (eg. 460800)
* Return:    0 if fail, i - baud rate index if success
*/
static int baud_rate_index(int baud_rate)
{
    unsigned int i;
    for (i = 0; i < sizeof(baudRates) / sizeof(*baudRates); ++i)
        if (baudRates[i] == baud_rate)
            return i;
    return 0;
}
/*
* Purpose:  chech the endian of the cpu
* Input:    none
* Return:   1:little-endian, 0:big-endian
*/
static int check_cpu_endian()
{
    union w{
        int a;
        char b;
    }c;

    c.a = 1;

    return (c.b == 1);
}

/*convert the host byte to fls protocol which use little endian*/
static uint32 fls_hton32(uint32 value)
{
    if(little_endian){
        return value;
    }else{
        return ( (((uint32)(value) & 0xff000000) >> 24)  | \
                  (((uint32)(value) & 0x00ff0000) >> 8)  | \
                  (((uint32)(value) & 0x0000ff00) << 8)  | \
                  (((uint32)(value) & 0x000000ff) << 24));
    }
}

/*convert the host byte to fls protocol, which use little endian*/
static uint16 fls_hton16(uint16 value)
{
     if(little_endian){
        return value;
     }else{
        return ((((uint16)(value) & 0xff00) >> 8) | \
                 (((uint16)(value) & 0x00ff) << 8));
     }
}

/*convert the fls protocol which use little endian to host byte*/
static uint32 fls_ntoh32(uint32 value)
{
    return fls_hton32(value);
}

/*convert the fls protocol which use little endian to host byte*/
static uint16 fls_ntoh16(uint16 value)
{
    return fls_hton16(value);
}

static int fsm_cust_get_pingpang_index(void)
{
    int index = 0;
    int retry = 0;
    char buf[PROPERTY_VALUE_MAX] = {0};

    do{
        if( (property_get(FSM_CUST_INDEX_PROP, buf, NULL) > 0) ){
            index = atoi(buf);
            LOGFLS(LOG_NOTICE, "fsm_cust_get_pingpang_index: %d, retry =%d\n", index, retry);
            break;
        } else {
            //sleep 100ms and retry
            usleep(FSM_PROP_NAP_TIME);
        }
        retry++;
    }while(retry <= FSM_PROP_MAXNAPS);

    if (retry > FSM_PROP_MAXNAPS)
        LOGFLS(LOG_ERR, "fsm_cust_get_pingpang_index: fail!\n");

    return !!index;
}

static void fsm_cust_set_pingpang_index(int i)
{
    if(i){
        property_set(FSM_CUST_INDEX_PROP, "1");
    }else{
        property_set(FSM_CUST_INDEX_PROP, "0");
    }
}

static int fsm_rw_get_pingpang_index(void)
{
    int index = 0;
    int retry = 0;
    char buf[PROPERTY_VALUE_MAX] = {0};

    do{
        if( (property_get(FSM_RW_INDEX_PROP, buf, NULL) > 0) ){
            index = atoi(buf);
            LOGFLS(LOG_NOTICE, "fsm_rw_get_pingpang_index: %d, retry =%d\n", index, retry);
            break;
        } else {
            //sleep 100ms and retry
            usleep(FSM_PROP_NAP_TIME);
        }
        retry++;
    }while(retry <= FSM_PROP_MAXNAPS);

    if (retry > FSM_PROP_MAXNAPS)
        LOGFLS(LOG_ERR, "fsm_rw_get_pingpang_index: fail!\n");

    return !!index;
}

static void fsm_rw_set_pingpang_index(int i)
{
    if(i){
        property_set(FSM_RW_INDEX_PROP, "1");
    }else{
        property_set(FSM_RW_INDEX_PROP, "0");
    }
}

static int fsm_get_pingpang_index(void)
{
    int index = 0;
    int retry = 0;
    char buf[PROPERTY_VALUE_MAX] = {0};

    do{
        if( (property_get(FSM_INDEX_PROP, buf, NULL) > 0) ){
            index = atoi(buf);
            LOGFLS(LOG_NOTICE, "fsm_get_pingpang_index: %d, retry =%d\n", index, retry);
            break;
        } else {
            /* FSM_INDEX_PROP is not exist, so we never wait */
            //usleep(FSM_PROP_NAP_TIME);
        }
        retry++;
    }while(0);

    if (retry > FSM_PROP_MAXNAPS)
        LOGFLS(LOG_ERR, "fsm_get_pingpang_index: fail!\n");

    return !!index;
}

static void fsm_set_pingpang_index(int i)
{
    if(i){
        property_set(FSM_INDEX_PROP, "1");
    }else{
        property_set(FSM_INDEX_PROP, "0");
    }
}
/*
* Purpose:  ascii/hexdump a byte buffer
* Input:    prefix - string to printed before hex data on every line
*           ptr - the string to be dumped
*           length - the length of the string to be dumped
* Return:   0
*/
#define MAX_DUMP_LEN (128)
static int package_dump(
    const char *prefix,
    const unsigned char *ptr,
    unsigned int length)
{
    if (logLevel < LOG_INFO) /*No need for all frame logging if it's not to be seen */
        return 0;

    char buffer[MAX_DUMP_LEN];
    unsigned int offset = 0l;
    int i;

    pthread_mutex_lock(&dump_lock);
    while (offset < length)
    {
        int off;
        strcpy(buffer, prefix);
        off = strlen(buffer);
        snprintf(buffer + off, sizeof(buffer) - off, "%04x: ", offset);
        off = strlen(buffer);
        for (i = 0; i < 16; i++)
        {
            if (offset + i < length){
                snprintf(buffer + off, sizeof(buffer) - off, "%02x%c", ptr[offset + i], i == 7 ? '-' : ' ');
            }
            else{
                snprintf(buffer + off, sizeof(buffer) - off, " .%c", i == 7 ? '-' : ' ');
            }
            off = strlen(buffer);
        }
        snprintf(buffer + off, sizeof(buffer) - off, " ");
        off = strlen(buffer);
        for (i = 0; i < 16; i++)
        {
            if (offset + i < length)
            {
                snprintf(buffer + off, sizeof(buffer) - off, "%c", (ptr[offset + i] < ' ') ? '.' : ptr[offset + i]);
                off = strlen(buffer);
            }
        }
        offset += 16;
        LOGFLS(LOG_INFO,"%s\n", buffer);
    }
    pthread_mutex_unlock(&dump_lock);

    return 0;
}

/*
* Purpose:  Allocates memory for a new frame buffer and initializes it.
* Input:    -
* Return:   NULL if error, else the point of handle
*/
static FlashlessHandle *flashless_handle_create(void)
{
    FlashlessHandle *hd = NULL;

    hd = malloc(sizeof(FlashlessHandle));
    if (NULL == hd){
        goto error_handle_init;
    }
    memset(hd, 0, sizeof(FlashlessHandle));

    hd->asci = (FlashlessDev *)malloc(sizeof(FlashlessDev));
    hd->cbp = (FlashlessDev *)malloc(sizeof(FlashlessDev));
    if(!hd->asci || !hd->cbp){
        goto error_handle_init;
    }
    memset(hd->asci, 0, sizeof(FlashlessDev));
    memset(hd->cbp, 0, sizeof(FlashlessDev));

    //SYNC(1Byte:0xFE) + Length(1Byte) + MsgId(1Byte) + Payload(plen byte)
    hd->asci->wframe.payload = hd->asci->wframe.buf + 3;
    hd->asci->rframe.payload = NULL;

    //SYNC(4Byte:0xFE 0xDC 0xBA 0x8) + Length(2Byte) + Payload(plen byte)
    hd->cbp->wframe.payload = hd->cbp->wframe.buf + 6;
    hd->cbp->rframe.payload = NULL;

    return hd;
error_handle_init:
    if(hd){
        if(hd->asci)
            free(hd->asci);
        if(hd->cbp)
            free(hd->cbp);
        free(hd);
    }
    return NULL;
}

/*
* Purpose:  Destroys the buffer (i.e. frees up the memory
* Input:    hd - the flashless handle
* Return:   -
*/
static void flashless_handle_destroy(FlashlessHandle *hd)
{
    if(hd){
        if(hd->asci)
            free(hd->asci);
        if(hd->cbp)
            free(hd->cbp);
        free(hd);
    }
}

/*
* Purpose:  set the flashless handle to be state.
* Input:    state - the state to be set
* Return:    -
*/
static void set_state(FlashlessStates state)
{
    pthread_mutex_lock(&handle_lock);
    phd->state = state;
    pthread_mutex_unlock(&handle_lock);
    LOGFLS(LOG_INFO, "Set state to be %d\n", state);
}

/*
* Purpose:  switch the index to begining.
* Input:    none
* Return:   currrent state of the flashless handle
*/
static FlashlessStates get_state(void)
{
    FlashlessStates state;

    pthread_mutex_lock(&handle_lock);
    state = phd->state;
    pthread_mutex_unlock(&handle_lock);

    return state;
}
/*
* Purpose:  some thing erro and can not resume. try to reset CBP and kill self
* Input:    none
* Return:    -
*/
static void flashless_die(char *reson)
{
#ifdef USE_STATUSD
    int ret=0;

    LOGFLS(LOG_ERR, "die for %s\n", reson);

    if(strstr(reson,"Signal")){
        ret = statusd_c2ssend_cmd(MODULE_TYPE_FLS,MODULE_TYPE_SR,CMD_CLIENT_EXITING);
        LOGFLS(LOG_ERR,"CLIENT %s::send CMD_CLIENT_EXITING cmd ret=%d\n",LOG_TAG,ret);
        if(ret != 1){
            LOGFLS(LOG_ERR,"CLIENT %s::send CMD_CLIENT_EXITING cmd failed ret=%d\n",LOG_TAG,ret);
        }
        exit(-1);
    }else{
        ret = statusd_c2ssend_cmd(MODULE_TYPE_FLS,MODULE_TYPE_SR,CMD_CLIENT_ERROR);
        LOGFLS(LOG_ERR,"CLIENT %s::send CMD_CLIENT_ERROR cmd ret=%d\n",LOG_TAG,ret);
        if(ret != 1){
            LOGFLS(LOG_ERR,"CLIENT %s::send CMD_CLIENT_ERROR cmd failed ret=%d\n",LOG_TAG,ret);
        }
    }
    flashless_handle_destroy(phd);
#else
    LOGFLS(LOG_ERR, "die for %s\n", reson);
    property_set("ctl.stop", "viaril-daemon");
    if(get_state() < FLS_STATE_WAIT_REQUEST){
        viatelModemPower(0);
        LOGFLS(LOG_NOTICE, "Power off modem.\n");
    }else if(!modem_reset){
        LOGFLS(LOG_NOTICE, "Reset modem.\n");
        modem_reset = 1;//mark the flag to ignore the reset indication
        viatelModemReset();
    }

    reset_modem_host();
    LOGFLS(LOG_NOTICE, "Reset modem host.\n");
    sleep(1);//make some delay to ignore the reset indication on
    flashless_handle_destroy(phd);
    exit(0);
#endif
}

/*
* Purpose:  Function responsible by all signal handlers treatment any new signal must be added here
* Input:    param - signal ID
* Return:   -
*/
void signal_treatment(int param)
{
    switch (param)
    {
        case SIGTERM:
        case SIGKILL:
            flashless_die("Signal");
            break;
        default:
            break;
    }
}

void vmodem_sigaction(int signum, siginfo_t *info, __attribute__((unused))void *data)
{
    LOGFLS(LOG_INFO, "Receive SIG=%d, No=%d, code=%d, val=0x%lx.\n",
                    signum, info->si_signo, info->si_code, info->si_band);

    switch(info->si_code){
        case ASC_USER_USB_WAKE:
        case ASC_USER_USB_SLEEP:
        case ASC_USER_UART_WAKE:
        case ASC_USER_UART_SLEEP:
        case ASC_USER_SDIO_WAKE:
        case ASC_USER_SDIO_SLEEP:
        case ASC_USER_MDM_POWER_ON:
        case ASC_USER_MDM_POWER_OFF:
            LOGFLS(LOG_INFO, "ignore vomdem command %x \n", info->si_code);
            break;
        case ASC_USER_MDM_RESET_ON:
            LOGFLS(LOG_NOTICE, "Vmodem reset indication on (%d).\n", modem_reset);
            if(!modem_reset){
                modem_reset = 1;//mark the flag to avoid reset modem again in flashless die
                flashless_die("modem crash");
            }
            break;
        case ASC_USER_MDM_RESET_OFF:
            LOGFLS(LOG_NOTICE, "Vmodem reset indication off.\n");
            break;
        default:
            LOGFLS(LOG_INFO, "unknow vomdem command %x \n", info->si_code);
    }
}
/*
* Purpose:  Creates a detached thread. also checks for errors on exit.
* Input:    thread_id - pointer to pthread_t id
*           thread_function - void pointer to thread function
*           thread_function_arg - void pointer to thread function args
* Return:    0 if success, -1 if fail
*/
int create_thread(pthread_t * thread_id, void * thread_function, void * thread_function_arg )
{
    pthread_attr_init(&thread_attr);
    pthread_attr_setdetachstate(&thread_attr, PTHREAD_CREATE_DETACHED);

    if(pthread_create(thread_id, &thread_attr, thread_function, thread_function_arg)!=0){
        switch (errno){
            case EAGAIN:
                LOGFLS(LOG_ERR,"Interrupt signal EAGAIN caught");
                break;
            case EINVAL:
                LOGFLS(LOG_ERR,"Interrupt signal EINVAL caught");
                break;
            default:
                LOGFLS(LOG_ERR,"Unknown interrupt signal caught");
        }

        LOGFLS(LOG_ERR, "Could not create thread");
            return -1;
        }
        pthread_attr_destroy(&thread_attr); /* Not strictly necessary */

    return 0; //thread created successfully
}

/*
* Purpose:  waiting for a frame, the read thread will be tirged at next wait frame
* Input:    time - timeout second
* Return:   the ID of received frame, FRAME_TYPE_TIMEOUT if timeout, else error
*/
static uint16 wait_frame(int time)
{
    int ret = 0;
    uint16 idRecv = FRAME_TYPE_TIMEOUT;
    struct timeval now;
    struct timespec timeout;

    /* The frame lock must be hold during wait and process frames.
     * which also requesed in receive frame threads, so the frame
     * operation can be serialized no matter what pthread conditon
     * is waiting.
     */
    pthread_mutex_lock(&frame_lock);

    /*notify the read thread to recive next frame*/
    pthread_cond_signal(&send_frame_signal);
    //wait until pending frame or error
    while(0 == phd->frame_pending && !ret){
        //wait until child thread send the frame
        ret = 0;
        if(time > 0){
            gettimeofday(&now, NULL);
            timeout.tv_sec = now.tv_sec + time;
            timeout.tv_nsec = now.tv_usec * 1000;
            ret = pthread_cond_timedwait(&recv_frame_signal, &frame_lock, &timeout);
        }else{
            ret = pthread_cond_wait(&recv_frame_signal,&frame_lock);
        }
    }

    if(phd->frame_pending & (1 << DEV_TYPE_ASCI)){
        idRecv = phd->asci->rframe.id;
        phd->frame_pending &= ~(1 << DEV_TYPE_ASCI);
    }else if(phd->frame_pending & (1 << DEV_TYPE_CBP)){
        idRecv = phd->cbp->rframe.id;
        phd->frame_pending &= ~(1 << DEV_TYPE_CBP);
    }
    pthread_mutex_unlock(&frame_lock);
    LOGFLS(LOG_INFO, "receive frame 0x%04x\n", idRecv);
    return idRecv;
}

/*
* Purpose:  init the asci exchange, make sure the UART for asciboot has been ready
* Input:    fhd FlashlessHandle struct
* Return:   0 if success, else error
*/
static int asci_device_prepare(FlashlessDev *ascDev)
{
    int retry = 0;
    int fdflags;
    struct termios t;

    gettimeofday(&tv_b, NULL);
    LOGFLS(LOG_INFO, "prepare asci device\n");
asci_device_prpare_retry:
    //wait for a while if not first time
    if(retry){
        usleep(1000*DEVICE_DETECT_WAIT);/*100ms*/
    }

    if(ascDev->fd > 0){
        close(ascDev->fd);
        ascDev->fd = -1;
    }

    if(NULL != asciDevice){
        free(asciDevice);
        asciDevice = NULL;
    }

    //Can not resume the device, send message the CBPStatus deamon to reset the CBP
    if(retry++ > DEVICE_DETECT_TIMES){
        LOGFLS(LOG_ERR, "%d times fail to prepare asci device, request cbp reset\n", retry);
        return -1;
    }

    asciDevice = viatelAdjustDevicePathFromProperty(VIATEL_CHANNEL_ASCI);
    if(NULL == asciDevice){
        LOGFLS(LOG_INFO, "fail to get asci device from property\n");
        goto asci_device_prpare_retry;
    }

    LOGFLS(LOG_NOTICE, "get asci device %s\n", asciDevice);

    /* open the serial port */
    ascDev->fd = open(asciDevice, O_RDWR | O_NOCTTY | O_NONBLOCK);
    if(ascDev->fd < 0){
        if((errno != ENOENT) || ((retry > DEVICE_DETECT_TIMES-1))){/*no such file or direction or the last retry*/
            LOGFLS(LOG_ERR, "fail to open asci device.(error:%s, errorno:%d)\n", strerror(errno), errno);
            goto asci_device_prpare_retry;
        }
    }
    fdflags = fcntl(ascDev->fd, F_GETFL);
    if(fdflags < 0){
        LOGFLS(LOG_ERR, "fail to F_GETFL asci device.(error:%s, errorno:%d)\n", strerror(errno), errno);
        goto asci_device_prpare_retry;
    }
    if(fcntl(ascDev->fd, F_SETFL, fdflags & ~O_NONBLOCK) < 0){
        LOGFLS(LOG_ERR, "fail to F_SETFL asci device.(error:%s, errorno:%d)\n", strerror(errno), errno);
        goto asci_device_prpare_retry;
    }

    if(tcgetattr(ascDev->fd, &t) < 0){
        LOGFLS(LOG_ERR, "fail to tcgetattr asci device.(error:%s, errorno:%d)\n", strerror(errno), errno);
        goto asci_device_prpare_retry;
    }

    t.c_cflag &= ~(CSIZE | CSTOPB | PARENB | PARODD);
    t.c_cflag |= (CREAD | CLOCAL | CS8);
    t.c_cflag &= ~(CRTSCTS);
    t.c_lflag &= ~(ICANON | ECHO | ECHOE | ECHOK | ECHONL | ISIG);
    t.c_iflag &= ~(INPCK | IGNPAR | PARMRK | ISTRIP | IXANY | ICRNL);
    t.c_iflag &= ~(IXON | IXOFF);
    t.c_oflag &= ~(OPOST | OCRNL);

    t.c_cc[VMIN] = 1; //the mini length for asci is SYNC+LEN+ID
    t.c_cc[VTIME] = 0;


#ifdef FLASHLESS_NOT_ANDROID
    t.c_cc[VINTR] = _POSIX_VDISABLE;
    t.c_cc[VQUIT] = _POSIX_VDISABLE;
    t.c_cc[VSTART] = _POSIX_VDISABLE;
    t.c_cc[VSTOP] = _POSIX_VDISABLE;
    t.c_cc[VSUSP] = _POSIX_VDISABLE;
#else
    //Android does not directly define _POSIX_VDISABLE. It can be fetched using pathconf()
    long posix_vdisable;
    char cur_path[FILENAME_MAX];
       if (!getcwd(cur_path, sizeof(cur_path))){
         LOGFLS(LOG_ERR,"_getcwd returned errno %d",errno);
         return -1;
       }
    posix_vdisable = pathconf(cur_path, _PC_VDISABLE);
    t.c_cc[VINTR] = posix_vdisable;
    t.c_cc[VQUIT] = posix_vdisable;
    t.c_cc[VSTART] = posix_vdisable;
    t.c_cc[VSTOP] = posix_vdisable;
    t.c_cc[VSUSP] = posix_vdisable;
#endif

    speed_t speed = baudBits[comSpeed];
    cfsetispeed(&t, speed);
    cfsetospeed(&t, speed);

    if(tcsetattr(ascDev->fd, TCSANOW, &t) < 0){
        LOGFLS(LOG_ERR, "fail to tcsetattr asci device.(error:%s, errorno:%d)\n", strerror(errno), errno);
        goto asci_device_prpare_retry;
    }

    int status = TIOCM_DTR | TIOCM_RTS;
    ioctl(ascDev->fd, TIOCMBIS, &status);
    gettimeofday(&tv_e, NULL);
    LOGFLS(LOG_NOTICE, "asci device has been prepare ok[%f]\n",(float)(tv_e.tv_sec - tv_b.tv_sec)+(float)(tv_e.tv_usec-tv_b.tv_usec)/1000000);
    //clear the data buffer
    tcflush(ascDev->fd, TCIFLUSH);

    return 0;
}

static void asci_frame_parser(FlashlessFrame *frm)
{
    if(frm->payload){
        frm->id = *frm->payload;
        if(frm->plen > 1){
            frm->payload++;
        }
    }
}

static int asci_frame_extract(FlashlessFrame *frm)
{
    int i, step, start = 0;
    //mini lenght of asci frame is 3(SYNC(1 bytes) + Length(1 bytes) + Payload(n bytes))
    if(frm->dlen < 3){
        return 0;
    }

    step = 0;
    frm->plen = 0;
    i = frm->base;

    while(i < frm->base + frm->dlen){
        switch(step){
            case 0://get asci frame sync head, which occupy 1 bytes
                if(frm->buf[i] == SYNC){
                    i++;
                    step = 1;
                }
                break;
            case 1://get length of payload, which occupy 1 bytes
                frm->plen = frm->buf[i];
                i++;
                start = i;
                step = 2;
                /*skip the invalid frame datas*/
                if(frm->plen <=0 || frm->plen > ASCI_MAX_PAYLOAD_SIZE){
                    frm->plen = 0;
                    frm->base = i = start;
                    frm->dlen -= (start - frm->base);
                    step = 0;
                }
               break;
            case 2://collect the number of data according to payload length
                if((frm->dlen - (start - frm->base)) >= frm->plen){
                    frm->payload = frm->buf + start;
                    //parse the frame type
                    package_dump("Asic PKG < ", frm->buf + frm->base, frm->dlen);
                    /*set frm->start to be the index of next frame*/
                    frm->dlen -= (start - frm->base + frm->plen);
                    if(frm->dlen > 0){
                        frm->base = start + frm->plen;
                    }else{
                        frm->base = 0;
                    }
                    asci_frame_parser(frm);
                    return 1;
                }else{
                    return 0;
                }
        }
    }

    return 0;
}

/*
* Purpose:  assemble the asci frame
* Input:    frm - Flash Frame data buffer for Asci read
* Return:   no return
*/
static void asci_frame_assemble(FlashlessFrame *frm)
{
    frm->buf[0] = (uint8)SYNC;
    frm->buf[1] = (uint8)frm->plen + 1;
    frm->buf[2] = (uint8)frm->id;
    frm->dlen = frm->plen + 3;
}
/*
* Purpose:  assemble the asci frame
* Input:    id - frame type ID
*           data - payload data buffer
*           len - length of payload
* Return:   0 if ok, else -1
*/
static int asci_frame_write(int id, uint8 *data, int len)
{
    int count = 0, sum = 0, ret = -1;
    FlashlessDev *ascDev = phd->asci;
    FlashlessFrame *frm = &ascDev->wframe;

    pthread_mutex_lock(&handle_lock);
    if(NULL == ascDev || ascDev->fd < 0){
        goto error_write;
    }

    if(len > ASCI_MAX_PAYLOAD_SIZE){
        LOGFLS(LOG_ERR, "the asci frame length %d is more than max %d\n", len, ASCI_MAX_PAYLOAD_SIZE);
        goto error_write;
    }

    if(data && data != frm->payload){
        memcpy(frm->payload, data, len);
    }

    frm->plen = len;
    frm->id = id;
    asci_frame_assemble(frm);

    package_dump("Asci PKG > ", frm->buf, frm->dlen);
    sum = count = 0;
    do{
        count = write(ascDev->fd, frm->buf + sum, frm->dlen - sum);
        if(count < 0){
            LOGFLS(LOG_ERR, "fail to write asci device.(error:%s, errorno:%d)\n", strerror(errno), errno);
            goto error_write;
        }
        sum += count;
    }while(sum < frm->dlen);

    ret = 0;
error_write:
    pthread_mutex_unlock(&handle_lock);
    return ret;
}

/*
* Purpose:  write a asci frame and wait ack from asciboot
* Input:    id - frame type ID
*           data - payload data buffer
*           len - length of payload
            retry - the times to resend if error
* Return:   0 if ok, else -1
*/
static int asci_frame_send(int id, uint8 *data, int len, int retry)
{
    int ret = -1, times=0;

retry_send:
    if(times > retry){
        LOGFLS(LOG_ERR, "fail to send asci msg\n");
        return -1;
    }
    times++;
    ret = asci_frame_write(id, data, len);
    if(ret  < 0){
        goto retry_send;
    }

    if(wait_frame(3) != FRAME_TYPE_ASCI_ACK){
        goto retry_send;
    }

    return 0;
}

static void cbp_frame_parser(FlashlessFrame *frm)
{
    uint16 id = (frm->payload[0]) | (frm->payload[1] << 8);

    LOGFLS(LOG_DEBUG, "id=0x%04x, plen=%d\n", id, frm->plen);
    if( sizeof(CBP_MSG_DOWNLOAD_CMD) == frm->plen && \
        FRAME_TYPE_CBP_DOWNLOAD_CMD == id ){
        frm->id = FRAME_TYPE_CBP_DOWNLOAD_CMD;
    }else if( sizeof(CBP_MSG_DOWNLOAD_DATA_RSP) == frm->plen && \
        FRAME_TYPE_CBP_DOWNLOAD_DATA_RSP == id){
        frm->id = FRAME_TYPE_CBP_DOWNLOAD_DATA_RSP;
    }else if( sizeof(CBP_MSG_ERASE_CMD) == frm->plen && \
        FRAME_TYPE_CBP_ERASE_CMD == id ){
        frm->id = FRAME_TYPE_CBP_ERASE_CMD;
    }else if( sizeof(CBP_MSG_UPLOAD_CMD) <= frm->plen && \
        FRAME_TYPE_CBP_FSM_UPLOAD_CMD == id ){
        frm->id = FRAME_TYPE_CBP_FSM_UPLOAD_CMD;
    }else if( sizeof(CBP_MSG_UPLOAD_CMD) <= frm->plen && \
        FRAME_TYPE_CBP_FSM_UPLOAD_CMD_V2 == id ){
        frm->id = FRAME_TYPE_CBP_FSM_UPLOAD_CMD_V2;
    }
#ifdef RAMDUMP_FUNC_SUPPORTED
    else if( sizeof(CBP_MSG_UPLOAD_CMD) <= frm->plen && \
            FRAME_TYPE_CBP_RAMDUMP_UPLOAD_CMD == id ){
        frm->id = FRAME_TYPE_CBP_RAMDUMP_UPLOAD_CMD;
    }
#endif
    else if( sizeof(CBP_MSG_UPLOAD_CMD) <= frm->plen && \
            FRAME_TYPE_CBP_CRASH_UPLOAD_CMD == id ){
        frm->id = FRAME_TYPE_CBP_CRASH_UPLOAD_CMD;
    }else if( sizeof(CBP_MSG_DOWNLOAD_DATA_RSP) == frm->plen && \
        FRAME_TYPE_CBP_DOWNLOAD_PKG_RSP == id ){
        frm->id = FRAME_TYPE_CBP_DOWNLOAD_PKG_RSP;
    }else if( sizeof(id) == frm->plen && \
        FRAME_TYPE_CBP_FSM_ERR == id ){
        frm->id = FRAME_TYPE_CBP_FSM_ERR;
    }else if( sizeof(id) == frm->plen && \
        FRAME_TYPE_CBP_BOOTLOADER_START == id ){
        frm->id = FRAME_TYPE_CBP_BOOTLOADER_START;
    }else if( sizeof(id) == frm->plen && \
        FRAME_TYPE_CBP_BOOTLOADER_END == id ){
        frm->id = FRAME_TYPE_CBP_BOOTLOADER_END;
    }else if( sizeof(CBP_MSG_FSM_CALIBRATE_CMD) == frm->plen && \
        FRAME_TYPE_CBP_FSM_CALIBRATE_CMD == id ){
        frm->id = FRAME_TYPE_CBP_FSM_CALIBRATE_CMD;
    }else{
        frm->id = FRAME_TYPE_UNKNOW;
    }
}

/*
* Purpose:  extract the cbp frame
* Input:    frm - Flash Frame data buffer for cbp read
* Return:   1 if extract a frame, else 0
*/
static int cbp_frame_extract(FlashlessFrame *frm)
{
    int i, j, step, start = 0;
    int ret = 0;
    uint8 *pbuf = NULL;
    //mini lenght of asci frame is 7(SYNC(4 bytes) + Length(2 bytes) + Payload(n bytes))
    if(frm->dlen - frm->base < 7){
        return ret;
    }

    step = 0;
    frm->plen = 0;
    i = j = 0;
    pbuf = frm->buf + frm->base;

    while((i + frm->base) < frm->dlen){
        switch(step){
            case 0://get USB frame sync head, which occupy 4 bytes
                if(pbuf[i++] == 0xFE){
                    step++;
                }else{
                    j = i;
                    step = 0;
                }

                break;
            case 1://get USB frame sync head, which occupy 4 bytes
                if(pbuf[i] == 0xDC){
                    i++;
                    step++;
                }else{
                    step = 0;
                }
                break;
            case 2://get USB frame sync head, which occupy 4 bytes
                if(pbuf[i] == 0xBA){
                    i++;
                    step++;
                }else{
                    step = 0;
                }
                break;
            case 3://get USB frame sync head, which occupy 4 bytes
                if(pbuf[i] == 0x98){
                    i++;
                    step++;
                }else{
                    step = 0;
                }
                break;
            case 4: //find first bype for plen
                step++;
                i++;
                break;
            case 5://get length of payload, which occupy 2 bytes
                frm->plen = (pbuf[i-1]) | (pbuf[i] << 8);
                i++;
                /*skip the invalid frame datas*/
                if(frm->plen > CBP_MAX_PAYLOAD_SIZE){
                    j = i;
                    frm->plen = 0;
                    step = 0;
                }else{
                    step++;
                }
                break;
            case 6://collect the number of data according to payload length
                if((frm->dlen - frm->base - i) >= frm->plen){
                    ret = 1;
                }
                goto extract_end;
        }
    }

extract_end:
    /*syntax error, ignore the invalid datas*/
    if(j > 0){
        LOGFLS(LOG_NOTICE, "Syntax error data(%d) step=%d, base=%d, dlen=%d, plen=%d\n",j, step, frm->base, frm->dlen, frm->plen);
        package_dump("ERR Cbp PKG < ", frm->buf + frm->base, frm->dlen - frm->base);
    }

    if(ret){
        frm->payload = pbuf + i;
        //parse the frame type
        package_dump("Cbp PKG < ", frm->payload - 6, frm->plen + 6);
        cbp_frame_parser(frm);
        /*set frm->base to be the index of next frame*/
        start = frm->base + i + frm->plen;
        if(start >= frm->dlen){
            frm->base = frm->dlen = 0;
        }else{
            frm->base += (frm->plen + i);
        }
    }else{
        frm->base += j;
    }

    return ret;
}

/*
* Purpose:  assemble the cbp frame
* Input:    frm - Flash Frame data buffer for cbp frame read
* Return:   no return
*/
static void cbp_frame_assemble(FlashlessFrame *frm)
{
    uint16 len;
    frm->buf[0] = 0xFE;
    frm->buf[1] = 0xDC;
    frm->buf[2] = 0xBA;
    frm->buf[3] = 0x98;
    len = frm->plen;
    len = fls_hton16(len);
    *(uint16 *)(frm->buf + 4) = len;
    frm->dlen = frm->plen + 6;
}

/*
* Purpose:  assemble the asci frame
* Input:    id - frame type ID
*           data - payload data buffer
*           len - length of payload
* Return:   0 if ok, else -1
*/
static int cbp_frame_write(uint8 *data, int len)
{
    int count = 0, sum = 0, ret = -1;
    FlashlessDev *cbpDev = phd->cbp;
    FlashlessFrame *frm = &cbpDev->wframe;

    pthread_mutex_lock(&handle_lock);
    if(NULL == cbpDev || cbpDev->fd < 0){
        goto error_write;
    }

    if(len > CBP_MAX_PAYLOAD_SIZE){
        LOGFLS(LOG_ERR, "the cbp frame length %d is more than max %d\n", len, CBP_MAX_PAYLOAD_SIZE);
        goto error_write;
    }

    if(data && data != frm->payload){
        memcpy(frm->payload, data, len);
    }

    frm->plen = len;
    cbp_frame_assemble(frm);
    do{
        count = write(cbpDev->fd, frm->buf + sum, frm->dlen - sum);
        if(count < 0){
            LOGFLS(LOG_ERR, "fail to write cbp device.(error:%s, errorno:%d)\n", strerror(errno), errno);
            goto error_write;
        }
        sum += count;
    }while(sum < frm->dlen);

    package_dump("Cbp PKG > ", frm->buf, frm->dlen);

    ret = 0;
error_write:
    pthread_mutex_unlock(&handle_lock);
    return ret;
}

/*
 * Purpose:  check whether the image is valid of the checksum
 * Input:    file - the file path
 * Return:   >0: image length, 0:invalid image
 */
 static int cbp_valid_image(const char *file)
{
    int ret = 0, fd = -1;
    uint8 buf[CBP_MAX_PAYLOAD_SIZE];
    uint32 fsizeGet, checksumGet;
    uint32 fsizeCal = 0, checksumCal = 0;
    int count = 0, i = 0;

    if(!file){
        goto invalid_image;
    }

    if(access(file, R_OK) < 0){
        goto invalid_image;
    }

    fd = open(file, O_RDONLY);
    if(fd < 0){
        goto invalid_image;
    }

    count = read(fd, (uint8 *)&fsizeGet, sizeof(fsizeGet));
    if(count <= 0){
        goto invalid_image;
    }

    fsizeGet = fls_ntoh32(fsizeGet);
    if(fsizeGet <= 0){
        goto invalid_image;
    }
    count = read(fd, (uint8 *)&checksumGet, sizeof(checksumGet));
    if(count <= 0){
        goto invalid_image;
    }
    checksumGet = fls_ntoh32(checksumGet);

    do{
        count = read(fd, buf, CBP_MAX_PAYLOAD_SIZE);
        if(count > 0){
            if(count > (int)(fsizeGet - fsizeCal)){
                count = fsizeGet - fsizeCal;
            }
            fsizeCal += count;
            for(i=0; i<count; i++){
                checksumCal += buf[i];
            }
        }else if(count <= 0){
            goto invalid_image;
        }
    }while(fsizeGet > fsizeCal);

    if((fsizeGet != fsizeCal) || (checksumGet != checksumCal)){
        goto invalid_image;
    }

    ret = fsizeGet + sizeof(fsizeGet) + sizeof(checksumCal);
invalid_image:
    if(fd >= 0){
        close(fd);
    }
    if(!ret){
        LOGFLS(LOG_WARNING, "Invalid image file %s\n", file);
        if(file && unlink(file) < 0) {
            LOGFLS(LOG_ERR, "fail to delete original %s, (error:%s, errorno:%d)\n",
                    file, strerror(errno), errno);
        }
    }
    return ret;
}

#if 0
/*
 * Purpose:  find file which modify time is last
 * Input:    file1 - the file path to compare
             file2 - the file path to compare
 * Return:   1: file1 is newer, 0: file2; else error
*/
static int file_last_modify_compare(char *file1, char *file2)
{
    int ret1, ret2;
    struct stat infor1, infor2;

    ret1 = ret2 = -1;
    if(file1){
        ret1 = access(file1, R_OK);
    }
    if(file2){
        ret2 = access(file2, R_OK);
    }

    if(ret1 < 0 || ret2 < 0){
        if(0 == ret1){
            return 1;
        }else if(0 == ret2){
            return 0;
        }else{
            return -1;
        }
    }

    ret1 = ret2 = -1;

    ret1 =  stat(file1, &infor1);
    ret2 =  stat(file2, &infor2);
    if(ret1 < 0 || ret2 < 0){
        if(0 == ret1){
            return 1;
        }else if(0 == ret2){
            return 0;
        }else{
            return -1;
        }
    }

    if(difftime(infor1.st_mtime, infor2.st_mtime) > 0){
        return 1;
    }else{
        return 0;
    }
}
#endif

/*
* Purpose:  init the cbp exchange device, make sure the USB has been ready
* Input:    fhd FlashlessHandle struct
* Return:   0 if success, else error
*/
static int cbp_device_prepare(FlashlessDev *cbpDev)
{
    int retry = 0, fdflags, status;
    struct termios t;

    gettimeofday(&tv_b, NULL);
    LOGFLS(LOG_INFO, "prepare cbp device\n");
cbp_device_retry:
    if(retry > 0){
        usleep(1000*DEVICE_DETECT_WAIT);
    }

    if(cbpDev->fd > 0){
        close(cbpDev->fd);
        cbpDev->fd = -1;
    }

    if(cbpDevice){
        free(cbpDevice);
        cbpDevice = NULL;
    }

    if(retry++ > DEVICE_DETECT_TIMES){
        LOGFLS(LOG_ERR, "Fail to prepare cbp device.\n");
        return -1;
    }

    cbpDevice = viatelAdjustDevicePathFromProperty(VIATEL_CHANNEL_FLS);
    if(NULL == cbpDevice){
        LOGFLS(LOG_INFO, "fail to detect cbp device, try %d times[%d]\n", retry,DEVICE_DETECT_TIMES);
        goto cbp_device_retry;
    }

    LOGFLS(LOG_NOTICE, "Get cbp device %s\n", cbpDevice);
    /* open the serial port */
    phd->cbp->fd = open(cbpDevice, O_RDWR | O_NOCTTY | O_NONBLOCK);
    if(phd->cbp->fd < 0){
        if((errno != ENOENT) || (retry > DEVICE_DETECT_TIMES-1)){/*no such file or direction or the last retry*/
            LOGFLS(LOG_ERR, "Fail to open cbp device %s, '%s' (code: %d)\n", cbpDevice, strerror(errno), errno);
        }
        goto cbp_device_retry;
    }
    fdflags = fcntl(phd->cbp->fd, F_GETFL);
    if (fcntl(phd->cbp->fd, F_SETFL, fdflags & ~O_NONBLOCK) < 0) {
        LOGFLS(LOG_ERR, "fail to F_SETFL cbp device.(error:%s, errorno:%d)\n", strerror(errno), errno);
    }
    tcgetattr(phd->cbp->fd, &t);
    t.c_cflag &= ~(CSIZE | CSTOPB | PARENB | PARODD);
    t.c_cflag |= CREAD | CLOCAL | CS8 ;
    t.c_cflag &= ~(CRTSCTS);
    t.c_lflag &= ~(ICANON | ECHO | ECHOE | ECHOK | ECHONL | ISIG);
    t.c_iflag &= ~(INPCK | IGNPAR | PARMRK | ISTRIP | IXANY | ICRNL);
    t.c_iflag &= ~(IXON | IXOFF);
    t.c_oflag &= ~(OPOST | OCRNL);
    t.c_cc[VMIN] = 1;
    t.c_cc[VTIME] = 0;
    tcsetattr(phd->cbp->fd, TCSANOW, &t);
    status = TIOCM_DTR | TIOCM_RTS;
    ioctl(phd->cbp->fd, TIOCMBIS, &status);
    gettimeofday(&tv_e, NULL);
    LOGFLS(LOG_NOTICE, "cbp device prepared ok[%f]\n",(float)(tv_e.tv_sec - tv_b.tv_sec)+(float)(tv_e.tv_usec-tv_b.tv_usec)/1000000);
    return 0;
}

/*
* Purpose:  notify a frame to main thread, the frame will not be send unless main thread allow
* Input:    data - the flashless handle.
* Return:   NULL if error
*/
void* receive_frame_thread(__attribute__((unused))void *data)
{
    int sel, ret=-1, len=0, count=0, retry=0, max=0;
    fd_set rfds;
    uint8 *buf = NULL;
    FlashlessDev *ascDev = phd->asci;
    FlashlessDev *cbpDev = phd->cbp;
    FlashlessFrame *ascFrm = &ascDev->rframe;
    FlashlessFrame *cbpFrm = &cbpDev->rframe;

    while(1){
        max = ascDev->fd;
        if(cbpDev->fd > max)
            max = cbpDev->fd;
        if(max < 0){
            sleep(useTimeout);
            continue;
        }
        FD_ZERO(&rfds);

        if(ascDev->fd > 0){
            if(get_state() > FLS_STATE_ASCI_HANDSHAKE){
                close(ascDev->fd);
                ascDev->fd = -1;
            }else{
                FD_SET(ascDev->fd, &rfds);
            }
        }
        if(cbpDev->fd > 0)
            FD_SET(cbpDev->fd, &rfds);

        sel = select(max + 1, &rfds, NULL, NULL, NULL);
        LOGFLS(LOG_DEBUG, "read selected %d (asc=%d,cbp=%d)\n", sel, ascDev->fd, cbpDev->fd);
        if(sel > 0){//read data
            if((cbpDev->fd > 0) && FD_ISSET(cbpDev->fd, &rfds)){
                count = read(cbpDev->fd, cbpFrm->buf + cbpFrm->dlen, CBP_MAX_FRAME_SIZE - cbpFrm->dlen);
                LOGFLS(LOG_DEBUG, "cbpDev read %d data\n", count);
                if(count <= 0 && errno != EINTR){
                    LOGFLS(LOG_ERR, "cbp device read erro '%s' (code: %d)\n", strerror(errno), errno);
                    pthread_mutex_lock(&handle_lock);
                    ret = cbp_device_prepare(cbpDev);
                    pthread_mutex_unlock(&handle_lock);
                    if(ret < 0){
                        flashless_die("cbp device read error");
                        break;
                    }else{
                        #ifndef USE_STATUSD/*if we use statusd,statusd will start ril*/
                            property_set("ctl.start", "viaril-daemon");
                        #endif
                    }
                }else if(count > 0){
                    cbpFrm->dlen += count;
                    //parse a frame
                    while(cbp_frame_extract(cbpFrm)){
                        pthread_mutex_lock(&frame_lock);
                        phd->frame_pending |= (1 << DEV_TYPE_CBP);
                        /*notify main thread to receive the frame*/
                        pthread_cond_signal(&recv_frame_signal);
                        while(phd->frame_pending & (1 << DEV_TYPE_CBP)){
                            //wait until main thread allow send next frame
                            pthread_cond_wait(&send_frame_signal,&frame_lock);
                        }
                        if(cbpFrm->base > 0){
                            len = cbpFrm->dlen - cbpFrm->base;
                            if(len > 0){
                                buf = malloc(len);
                                if(buf){
                                    memcpy(buf, cbpFrm->buf + cbpFrm->base, len);
                                    memcpy(cbpFrm->buf, buf, len);
                                    free(buf);
                                    buf = NULL;
                                    cbpFrm->base = 0;
                                    cbpFrm->dlen = len;
                                    len = 0;
                                }
                            }

                        }
                        cbpFrm->plen = 0;
                        cbpFrm->id = -1;
                        cbpFrm->payload = NULL;
                        pthread_mutex_unlock(&frame_lock);
                    }
                }else{
                    //do nothing
                }
            }

            if ((ascDev->fd > 0) && FD_ISSET(ascDev->fd, &rfds)){
                count = read(ascDev->fd, ascFrm->buf + ascFrm->dlen, CBP_MAX_FRAME_SIZE - ascFrm->dlen);
                LOGFLS(LOG_DEBUG, "asciDev read %d data\n", count);
                if(count < 0 && errno != EINTR){
                    LOGFLS(LOG_ERR, "asci device read erro '%s' (code: %d)\n", strerror(errno), errno);
                    pthread_mutex_lock(&handle_lock);
                    ret = asci_device_prepare(ascDev);
                    pthread_mutex_unlock(&handle_lock);
                    if(ret < 0){
                        flashless_die("asci device read error");
                        break;
                    }
                }else if(count > 0){
                    ascFrm->dlen += count;
                    //parse a frame
                    while(asci_frame_extract(ascFrm)){
                        pthread_mutex_lock(&frame_lock);
                        phd->frame_pending |= (1 << DEV_TYPE_ASCI);
                        /*notify main thread to receive the frame*/
                        pthread_cond_signal(&recv_frame_signal);
                        while(phd->frame_pending & (1 << DEV_TYPE_ASCI)){
                            //wait until main thread allow send next frame
                            pthread_cond_wait(&send_frame_signal,&frame_lock);
                        }
                        ascFrm->plen = 0;
                        ascFrm->id = -1;
                        ascFrm->payload = NULL;
                        pthread_mutex_unlock(&frame_lock);
                    }
                }
            }
        }else if(sel == 0){
            LOGFLS(LOG_INFO, "read select timeout\n");
        }else{
            LOGFLS(LOG_WARNING, "read select error '%s' (code: %d)\n", strerror(errno), errno);
            if(cbpDev->fd > 0){
                pthread_mutex_lock(&frame_lock);
                ret = cbp_device_prepare(cbpDev);
                pthread_mutex_unlock(&frame_lock);
                if(ret < 0){
                    flashless_die("cbp device select error");
                    break;
                }
            }

            if(ascDev->fd > 0){
                pthread_mutex_lock(&frame_lock);
                ret = asci_device_prepare(ascDev);
                pthread_mutex_unlock(&frame_lock);
                if(ret < 0){
                    flashless_die("asci device select error");
                    break;
                }
            }
        }
    }

    return (void *)1;

}

/*
 * Purpose:  asci handsake session to download boot.rom during asci boot
 * Input:    none
 * Return:   0 is ok else return -1
*/
static int session_asci_boot(void)
{
    int ret = -1;
    int fd = -1;
    int times = 0;
    FlashlessFrame *wframe = &phd->asci->wframe;
    struct stat info;
    int i, offset, rsize, fsize;
    uint32 checksum = 0;

retry_handshake_session:
    gettimeofday(&tv_b, NULL);
    LOGFLS(LOG_NOTICE, "--- Start session asci for %d times ---\n", times);
    if(times >= useRetry){
        ret = -1;
        goto error_handshake_session;
    }
    if(times){
        sleep(1);
    }
    times++;

    //step 1: handshake with asci boot
    ret = asci_frame_send(FRAME_TYPE_ASCI_HANDSHAKE, NULL, 0, 0);
    if(ret < 0){
        LOGFLS(LOG_ERR, "Fail to handshake with asci boot\n");
        dumpBootupStatus();
        goto retry_handshake_session;
    }
    LOGFLS(LOG_NOTICE, "Asci handshake Ok\n");

    // step 2: send Assci download message heaer
    // header information "FE 09 03 00 00 00 00 20 00 00 00"
    ASCI_MSG_HEADER HeaderMsg;
    HeaderMsg.LoadAddr = fls_hton32(BOOT_LOAD_ADDR);
    HeaderMsg.ExeAddr = fls_hton32(BOOT_EXE_ADDR);

    ret = asci_frame_send(FRAME_TYPE_ASCI_HEADER, (uint8 *)&HeaderMsg, sizeof(HeaderMsg), 0);
    if(ret < 0){
        LOGFLS(LOG_ERR, "Fail to send header msg to assci boot\n");
        goto retry_handshake_session;
    }

    LOGFLS(LOG_NOTICE, "Asci header message Ok\n");

    // step 3: send data message to download boot.rom
    // header information "FE 09 03 00 00 00 00 20 00 00 00"
    if(fd >= 0){
        close(fd);
        fd = -1;
    }

    fd = open(BOOT_FILE_PATH, O_RDONLY);
    if(fd < 0){
        LOGFLS(LOG_ERR, "Fail to open %s\n", BOOT_FILE_PATH);
        goto retry_handshake_session;
    }

    if(fstat(fd, &info) < 0 ){
        LOGFLS(LOG_ERR, "Fail to fstat %s\n", BOOT_FILE_PATH);
        goto retry_handshake_session;
    }

    fsize = info.st_size;
    LOGFLS(LOG_NOTICE, "%s image size is %d\n", BOOT_FILE_PATH, fsize);

    checksum = rsize = offset = 0;
    do{
        rsize = read(fd, wframe->payload, min(ASCI_MAX_PAYLOAD_SIZE, fsize - offset));
        if(rsize <= 0){
            LOGFLS(LOG_ERR, "Fail to read boot image error: %s (code: %d)\n", strerror(errno), errno);
            goto retry_handshake_session;
        }

        ret = asci_frame_send(FRAME_TYPE_ASCI_DATA, wframe->payload, rsize, 0);
        if(ret < 0){
            LOGFLS(LOG_ERR, "Fail to send data msg to asci\n");
            goto retry_handshake_session;
        }

        for(i = 0; i < rsize; i++){
            checksum += wframe->payload[i];
        }

        offset += rsize;
    }while(fsize - offset > 0);

    LOGFLS(LOG_NOTICE, "Asci download %s Ok\n", BOOT_FILE_PATH);

    //step 4: send the checksum message to ascci
    ASCI_MSG_CHECKSUM ChecksumMsg;
    ChecksumMsg.checksum = fls_hton32(checksum);
    //Do not wait the ACK, which maybe missed by CBP
    ret = asci_frame_write(FRAME_TYPE_ASCI_CHECKSUM, (uint8 *)(&ChecksumMsg), sizeof(ChecksumMsg));
    if(ret < 0){
        LOGFLS(LOG_ERR, "Fail to send checksum msg to asci\n");
        goto retry_handshake_session;
    }
    gettimeofday(&tv_e, NULL);
    LOGFLS(LOG_NOTICE, "Asci send checksum message Ok [%f]\n",(float)(tv_e.tv_sec - tv_b.tv_sec)+(float)(tv_e.tv_usec-tv_b.tv_usec)/1000000);
    ret = 0;
error_handshake_session:
    if(fd >= 0){
        close(fd);
    }
    LOGFLS(LOG_NOTICE, "--- End session asci ---\n");
    return ret;
}

/*
 * Purpose:  download image during cbp boot
 * Input:    DownloadCmd - the command frame to dowload image
 * Return:   0 is ok else return -1
*/
static int session_cbp_image_download(CBP_MSG_DOWNLOAD_CMD * DownloadCmd)
{
    int ret = -1, frmRecv;
    char *path = NULL, *path1, *path2;
    struct stat info;
    int fd = -1, offset, rsize, fsize = 0;
    FlashlessFrame *rframe = &phd->cbp->rframe;
    struct download_data_cache *cache_list[MAX_DATA_WINDOW] = {NULL};
    int size = 0, count = 0, sum = 0;
    int i = 0;
    int erase = 0;
    uint8 section;
    uint32 SeqToWrite = 0;  // the sequence of write data
    uint32 SeqGetAck = 0;   // the sequence if ack
    uint16 checksum = 0, len = 0;
    struct download_data_cache *cache = NULL;
    CBP_MSG_DOWNLOAD_DATA_CMD EraseCmd;
    CBP_MSG_DOWNLOAD_DATA_CMD *cmd = NULL;
    CBP_MSG_DOWNLOAD_DATA_RSP *rsp = NULL;

    gettimeofday(&tv_b, NULL);
    LOGFLS(LOG_NOTICE, "--- Start session image download ---\n");

    if(DownloadCmd == NULL){
        LOGFLS(LOG_WARNING, "Download command is NULL\n");
        return -1;
    }
    section = DownloadCmd->section;
    // step 1: prepare the image for download, caculate the size
    switch(DownloadCmd->section){
        case SECTION_CP:
            LOGFLS(LOG_NOTICE, "Dowload cp rom image.\n");
            path = CP_FILE_PATH;
            ret = stat(path, &info);
            if(ret < 0){
                LOGFLS(LOG_ERR, "Fail to stat image %s error: %s (code: %d)\n", path, strerror(errno), errno);
                path = NULL;
                fsize = 0;
            }else{
                fsize = info.st_size - CHECK_HEADER_SIZE;
            }
            break;
        case SECTION_FSM_RF:
            LOGFLS(LOG_NOTICE, "Download fsm rf image.\n");
            if((fsize = cbp_valid_image(FSM_FILE_PATH_RF)) > 0)
            {
                path = FSM_FILE_PATH_RF;
            }
            else
            {
                ret = fsm_calibration_restore(FSM_FILE_PATH_RF);
                if(ret < 0){
                    LOGFLS(LOG_ERR, "FSM restore API error(%d).\n", ret);
                }
                if((fsize = cbp_valid_image(FSM_FILE_PATH_RF)) > 0){
                    path = FSM_FILE_PATH_RF;
                }else if((fsize = cbp_valid_image(FSM_RF_FILE_PATH_DF)) > 0){
                    path = FSM_RF_FILE_PATH_DF;
                }else{
                    path = NULL;
                }
            }
            cur_download_fsm_type = SECTION_FSM_RF;
            break;
        case SECTION_FSM_RW:
            LOGFLS(LOG_NOTICE, "Download fsm rw image: fsm_rw_index = %d.\n", fsm_rw_index);
            if(fsm_rw_index == 0){
                path1 = FSM_RW_FILE_PATH_0;
                path2 = FSM_RW_FILE_PATH_1;
            }else{
                path1 = FSM_RW_FILE_PATH_1;
                path2 = FSM_RW_FILE_PATH_0;
            }

            /*Get the FSM from normal file*/
            if((fsize = cbp_valid_image(path1)) > 0){
                path = path1;
            }else if((fsize = cbp_valid_image(path2)) > 0){
                path = path2;
            }else if((fsize = cbp_valid_image(FSM_RW_FILE_PATH_DF)) > 0){
                path = FSM_RW_FILE_PATH_DF;
            }else{
                path = NULL;
            }
            cur_download_fsm_type = SECTION_FSM_RW;
            break;
        case SECTION_FSM_CUST:
            LOGFLS(LOG_NOTICE, "Download fsm cust image: fsm_cust_index = %d.\n", fsm_cust_index);
            if(fsm_cust_index == 0){
                path1 = FSM_CUST_FILE_PATH_0;
                path2 = FSM_CUST_FILE_PATH_1;
            }else{
                path1 = FSM_CUST_FILE_PATH_1;
                path2 = FSM_CUST_FILE_PATH_0;
            }

            /*Get the FSM from normal file*/
            if((fsize = cbp_valid_image(path1)) > 0){
                path = path1;
            }else if((fsize = cbp_valid_image(path2)) > 0){
                path = path2;
            }else if((fsize = cbp_valid_image(FSM_CUST_FILE_PATH_DF)) > 0){
                path = FSM_CUST_FILE_PATH_DF;
            }else{
                path = NULL;
            }
            cur_download_fsm_type = SECTION_FSM_CUST;
            break;
        case SECTION_FSM:
            LOGFLS(LOG_NOTICE, "Dowload fsm image: fsm_index = %d.\n", fsm_index);
            if(fsm_index){
                path1 = FSM_FILE_PATH_1;
                path2 = FSM_FILE_PATH_0;
            }else{
                path1 = FSM_FILE_PATH_0;
                path2 = FSM_FILE_PATH_1;
            }

            /*Get the FSM from normal file*/
            if((fsize = cbp_valid_image(path1)) > 0){
                path = path1;
            }else if((fsize = cbp_valid_image(path2)) > 0){
                path = path2;
            }else if((fsize = cbp_valid_image(FSM_FILE_PATH_CL)) > 0){
                path = FSM_FILE_PATH_CL;
            }else{
                path = NULL;
            }

            /*No normal file can be used, try to restore the calibration file*/
            if(!path){
                ret = fsm_calibration_restore(FSM_FILE_PATH_CL);
                if(ret < 0){
                    LOGFLS(LOG_ERR, "FSM restore API error(%d).\n", ret);
                }

                if((fsize = cbp_valid_image(FSM_FILE_PATH_CL)) > 0){
                    path = FSM_FILE_PATH_CL;
                }else if((fsize = cbp_valid_image(FSM_FILE_PATH_DF)) > 0){
                    path = FSM_FILE_PATH_DF;
                }else{
                    path = NULL;
                }
            }

            break;
        case SECTION_LOG:
            LOGFLS(LOG_NOTICE, "Dowload crash log image.\n");
            path = CRASH_FILE_PATH;
            ret = stat(path, &info);
            if(ret < 0){
                LOGFLS(LOG_ERR, "Fail to stat image %s error: %s (code: %d)\n", path, strerror(errno), errno);
                path = NULL;
                fsize = 0;
            }else{
                fsize = info.st_size;
            }
            break;
        default:
            LOGFLS(LOG_ERR, "Unknow Section ID %d for download command.\n", DownloadCmd->section);
            ret = -1;
            goto session_error;
    }

    if(NULL == path){
        fsize = 0;
    }

    if(path && fsize > 0){
        fd = open(path, O_RDONLY);
        if(fd < 0){
            LOGFLS(LOG_ERR, "Fail to open %s\n", path);
            fsize = 0;
        }
    }

    CBP_MSG_DOWNLOAD_RSP DownloadRsp;
    DownloadRsp.id = FRAME_TYPE_CBP_DOWNLOAD_RSP;
    DownloadRsp.section = DownloadCmd->section;
    DownloadRsp.size = fls_hton32(fsize);
    ret = cbp_frame_write((uint8 *)&DownloadRsp, sizeof(DownloadRsp));
    if(ret < 0){
        ret = -1;
        goto session_error;
    }

    /*No file exist*/
    if(fsize <= 0){
        ret = 0;
        goto session_end;
    }else{
        LOGFLS(LOG_NOTICE, "Good image %s size is %d\n", path, fsize);
    }

    // step 2: send download datas
    //init the cache list
    for(i=0; i<MAX_DATA_WINDOW; i++){
        cache = malloc(sizeof(struct download_data_cache));
        if(cache == NULL){
            LOGFLS(LOG_ERR, "Fail to malloc download cache slot\n");
            ret = -1;
            goto session_error;
        }
        cache->sync[0] = 0xFE;
        cache->sync[1] = 0xDC;
        cache->sync[2] = 0xBA;
        cache->sync[3] = 0x98;
        cache->len = 0;
        cmd = &cache->cmd;
        cmd->id = fls_hton16(FRAME_TYPE_CBP_DOWNLOAD_PKG_CMD);
        cmd->seq = 0;
        cmd->type = PROG_DATA; //data
        cmd->section = section;
        cmd->checksum = 0;
        cmd->size = 0;
        cache_list[i] = cache;
    }


    //send erase command firstly
    cmd = &EraseCmd;
    cmd->id = fls_hton16(FRAME_TYPE_CBP_DOWNLOAD_PKG_CMD);
    cmd->seq = 1;
    cmd->type = PROG_INIT; //erase
    cmd->section = section;
    cmd->checksum = 0;
    cmd->size = 0;
    ret = cbp_frame_write((uint8 *)cmd, sizeof(CBP_MSG_DOWNLOAD_DATA_CMD));
    if(ret < 0){
        ret = -1;
        goto session_error;
    }
    LOGFLS(LOG_INFO, "send download erase command.\n");
    frmRecv = wait_frame(useTimeout);
    if(frmRecv == FRAME_TYPE_CBP_DOWNLOAD_PKG_RSP){
        rsp = (CBP_MSG_DOWNLOAD_DATA_RSP *)rframe->payload;
        SeqGetAck = fls_ntoh32(rsp->seq);
        if(rsp->TypeAck == PROG_ACK && SeqGetAck == 1){
            /*get ack message*/
            LOGFLS(LOG_INFO, "receive download erase response.\n");
            erase = 1;
        }
    }

    if(!erase){
        LOGFLS(LOG_ERR, "no response for erase.\n");
        ret = -1;
        goto session_error;
    }

    count = sum = size = 0;
    rsize = offset = 0;
    checksum = 0;
    SeqToWrite = SeqGetAck = 1;
    set_state(FLS_STATE_IMAGE_DOWNLOAD);
    do{
        //find a free cache slot
        cache = NULL;
        for(i=0; i<MAX_DATA_WINDOW; i++){
            cache = cache_list[i];
            cmd = &cache->cmd;
            if(0 == cmd->seq){
                break;
            }else{
                cache = NULL;
                cmd = NULL;
            }
        }

        /*send data if the SeqToWrite is no more than windows and file can be read*/
        if(cache && (rsize < fsize)){
            ret = read(fd, cache->data, min(CBP_MAX_PAYLOAD_SIZE, fsize - rsize));
            if(ret < 0){
                LOGFLS(LOG_ERR, "Fail to read image error: %s (code: %d)\n", strerror(errno), errno);
                ret = -1;
                goto session_error;
            }

            rsize += ret;
            checksum = 0;
            for(i = 0; i < ret; i++){
                checksum += cache->data[i];
            }

            SeqToWrite++;

            cmd->seq = fls_hton32(SeqToWrite);
            cmd->checksum = fls_hton16(checksum);
            cmd->size = fls_hton16((uint16)ret);

            /*write a frame*/
            size = sizeof(struct download_data_cache) - (CBP_MAX_PAYLOAD_SIZE - ret);
            len = size - 6;
            cache->len = fls_hton16(len);
            pthread_mutex_lock(&handle_lock);
            count = sum = 0;
            do{
                count = write(phd->cbp->fd, (uint8 *)cache + sum, size - sum);
                if(count < 0){
                    LOGFLS(LOG_ERR, "fail to write cbp device.(error:%s, errorno:%d)\n", strerror(errno), errno);
                    ret = -1;
                    pthread_mutex_unlock(&handle_lock);
                    goto session_error;
                }
                sum += count;
            }while(sum < size);
            pthread_mutex_unlock(&handle_lock);
            LOGFLS(LOG_DEBUG, "download %d data (rsize=%d, fsize=%d) frame for SeqToWrite=%u, SeqGetAck=%u.\n",ret, rsize, fsize, SeqToWrite, SeqGetAck);
            package_dump("Cbp PKG > ", (uint8 *)cache, size);
        }

        /*wait for response if seq is more than windows or the whole file has been send*/
        if((SeqToWrite - SeqGetAck >= MAX_DATA_WINDOW) || (rsize >= fsize)){
            frmRecv = wait_frame(useTimeout * MAX_DATA_WINDOW);
            /*No ack message*/
            if(frmRecv == FRAME_TYPE_TIMEOUT){
                LOGFLS(LOG_ERR, "Fail to wait cbp data response.\n");
                ret = -1;
                goto session_error;
            }else if(frmRecv == FRAME_TYPE_CBP_DOWNLOAD_PKG_RSP){
                rsp = (CBP_MSG_DOWNLOAD_DATA_RSP *)rframe->payload;
                SeqGetAck = fls_ntoh32(rsp->seq);
                if(rsp->TypeAck == PROG_NACK){
                    /*get NACK*/
                    LOGFLS(LOG_ERR, "NACK message for seq %u image download.\n", SeqGetAck);
                    ret = -1;
                    goto session_error;
                }else{
                    /*get ACK, free the slot in cache list*/
                    SeqGetAck = fls_ntoh32(rsp->seq);
                    LOGFLS(LOG_DEBUG, "Get ack message SeqGetAck=%u.\n", SeqGetAck);
                    uint32 seq = 0;
                    int i;
                    for(i=0; i<MAX_DATA_WINDOW; i++){
                        cache = cache_list[i];
                        cmd = &cache->cmd;
                        seq = fls_ntoh32(cmd->seq);
                        if(0 != seq && seq <= SeqGetAck){
                            offset += (int)fls_ntoh16(cmd->size);
                            LOGFLS(LOG_DEBUG, "SeqGetAck %u ack package seq %u: offset=%d, fsize=%d.\n",SeqGetAck, seq, offset, fsize);
                            cmd->seq = 0;
                        }
                    }
                }
            }else{
                /*get unknow message*/
                LOGFLS(LOG_ERR, "unknow message %d.\n", frmRecv);
                ret = -1;
                goto session_error;
            }
        }
    }while(offset < fsize);

    /*
     * To avoid downloading obsoleted temporary ping pong files instead of restored calibration file to CBP
     * after restore action in meta mode happens
     */
    if (run_mode == MODE_META && section == SECTION_FSM) {
        if(access(FSM_FILE_PATH_0, F_OK) == 0){
            if(unlink(FSM_FILE_PATH_0) < 0) {
                LOGFLS(LOG_ERR, "fail to delete %s, (error:%s, errorno:%d)\n",
                        FSM_FILE_PATH_0, strerror(errno), errno);
            }
        }

        if(access(FSM_FILE_PATH_1, F_OK) == 0){
            if(unlink(FSM_FILE_PATH_1) < 0) {
                LOGFLS(LOG_ERR, "fail to delete %s, (error:%s, errorno:%d)\n",
                        FSM_FILE_PATH_1, strerror(errno), errno);
            }
        }
    }
    LOGFLS(LOG_NOTICE, "Download image %s ok.\n", path);
    ret = 0;
session_error:
    for(i=0; i<MAX_DATA_WINDOW; i++){
        if(cache_list[i])
            free(cache_list[i]);
    }
session_end:
    if(fd >= 0){
        close(fd);
    }
    gettimeofday(&tv_e, NULL);
    LOGFLS(LOG_NOTICE, "--- End session image download ---[%f]\n",(float)(tv_e.tv_sec - tv_b.tv_sec)+(float)(tv_e.tv_usec-tv_b.tv_usec)/1000000);
    return ret;
}

static inline int cbp_upload_response(uint16 type, uint32 seq, uint32 statu)
{
    CBP_MSG_UPLOAD_RSP frmUploadRsp;
    frmUploadRsp.id = fls_hton16(type);
    frmUploadRsp.seq = fls_hton32(seq);
    frmUploadRsp.status = fls_hton32(statu);

    return cbp_frame_write((uint8 *)(&frmUploadRsp), sizeof(CBP_MSG_UPLOAD_RSP));
}

static int create_fsm_rf_image_from_temp_file()
{
   int ret = -1, len;
#define BLOCK_SIZE (1024)
   char buf[BLOCK_SIZE];
   char *path;
   int fd_back = -1, fd_fsm = -1;

   path = FSM_RF_TEMP_PATH;

   LOGFLS(LOG_NOTICE, "Request calibration from %s to %s.\n", path, FSM_FILE_PATH_RF);
   if(!cbp_valid_image(path)){
       LOGFLS(LOG_ERR, "Invalid fsm image %s for backup.", path);
       goto session_error;
   }

   fd_fsm = open(path, O_RDONLY);
   if(fd_fsm < 0){
       LOGFLS(LOG_ERR, "fail to open fsm image %s.(error:%s, errorno:%d)\n", path, strerror(errno), errno);
       goto session_error;
   }
   fd_back = open(FSM_FILE_PATH_RF, O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP);
   if(fd_back < 0){
       LOGFLS(LOG_ERR, "fail to open back image %s.(error:%s, errorno:%d)\n", FSM_FILE_PATH_RF, strerror(errno), errno);
       goto session_error;
   }

   do{
       len = read(fd_fsm, buf, BLOCK_SIZE);
       if(len < 0){
           LOGFLS(LOG_ERR, "fail to read fsm image %s.(error:%s, errorno:%d)\n", FSM_FILE_PATH_RF, strerror(errno), errno);
           goto session_error;
       }
       if(len > 0){
           ret = write(fd_back, buf, len);
           if(ret < 0){
               LOGFLS(LOG_ERR, "fail to write backup image %s.(error:%s, errorno:%d)\n", path, strerror(errno), errno);
               goto session_error;
           }
       }
   }while(len > 0);

   fsync(fd_back);

session_error:
    if(fd_back >= 0){
        close(fd_back);
    }
    if(fd_fsm >= 0){
        close(fd_fsm);
    }
    return ret;

}

/*
 * Purpose:  upload image during cbp boot
 * Input:    UploadCmd - the command frame to upload image
 * Return:   0 is ok else return -1
*/
static int session_cbp_image_upload(CBP_MSG_UPLOAD_CMD * UploadCmd)
{
    char *path = "none";
    int ret = -1, fd = -1;
    uint32 total = 0, fsize = 0, syncType = 0;
    uint16 rsize, type, frmRecv;
    FlashlessFrame *rframe = &phd->cbp->rframe;
    uint8 *data;
    uint8 img_type = 0;
    uint32 SeqGet = 0, SeqAck = 0, eStatus = STATUS_BAD_SEQ;
    gettimeofday(&tv_b, NULL);
    LOGFLS(LOG_NOTICE, "Start session image upload.\n");

    //step 1: get the upload file information
    type = fls_ntoh16(UploadCmd->id);
    SeqGet = fls_ntoh32(UploadCmd->seq);
    if(SeqGet != 1){
        LOGFLS(LOG_WARNING, "Invalid first SeqNum %u.\n", SeqGet);
        goto session_error;
    }
    //get the packet data size, should be 4 Bytes in first upload msg
    rsize = fls_ntoh16(UploadCmd->length);
    if (type == FRAME_TYPE_CBP_FSM_UPLOAD_CMD) {
        if(rsize != sizeof(fsize) + sizeof(syncType)){
            LOGFLS(LOG_WARNING, "Invalid first payload length %u in fsm upload msg, should be %d.\n",
                    rsize, sizeof(rsize) + sizeof(syncType));
            goto session_error;
        }
    }else if (type == FRAME_TYPE_CBP_FSM_UPLOAD_CMD_V2) {
        if(rsize != sizeof(fsize) + sizeof(syncType) + sizeof(uint8)){
            LOGFLS(LOG_WARNING, "Invalid first payload length %u in fsm upload msg, should be %d.\n",
                    rsize, sizeof(fsize) + sizeof(syncType) + sizeof(uint8));
            goto session_error;
        }
    }
    else {
        if(rsize != sizeof(fsize)){
            LOGFLS(LOG_WARNING, "Invalid first payload length %u, should be %d.\n", rsize, sizeof(rsize));
            goto session_error;
        }
    }

    //get total file size
    data = rframe->payload + sizeof(CBP_MSG_UPLOAD_CMD);
    fsize = (data[0]) | (data[1] << 8) | (data[2] << 16) | (data[3] << 24);
    if (fsize <= 0) {
        LOGFLS(LOG_WARNING, "invalid upload file size %u\n", fsize);
        goto session_error;
    }

    if (run_mode == MODE_META && ((type == FRAME_TYPE_CBP_FSM_UPLOAD_CMD) || (type == FRAME_TYPE_CBP_FSM_UPLOAD_CMD_V2))) {
        syncType = (data[4]) | (data[5] << 8) | (data[6] << 16) | (data[7] << 24);
        if (syncType != FSM_SYNC_ACT_FLUSH) {
            LOGFLS(LOG_WARNING, "omit fsm sync message, type %u\n", syncType);
            eStatus = STATUS_DENY;
            goto session_error;
        }
        else {
            LOGFLS(LOG_DEBUG, "fsm flush sync message received, type %u\n", syncType);
        }
    }

    //step 2: prepare the storage file
    if(type == FRAME_TYPE_CBP_FSM_UPLOAD_CMD){
        if(fsm_index){
            path = FSM_FILE_PATH_0;
        }else{
            path = FSM_FILE_PATH_1;
        }
        LOGFLS(LOG_NOTICE, "Upload %d bytes fsm image into %s.\n", fsize, path);
    }else if(type == FRAME_TYPE_CBP_FSM_UPLOAD_CMD_V2){
        img_type = data[8];
        switch(img_type){
        LOGFLS(LOG_NOTICE, "Upload fsm image type %d.\n",data[8]);
        case SECTION_FSM_CUST:
            if(fsm_cust_index){
               path = FSM_CUST_FILE_PATH_0;
            }else{
               path = FSM_CUST_FILE_PATH_1;
            }
            break;
        case SECTION_FSM_RW:
            if(fsm_rw_index){
               path = FSM_RW_FILE_PATH_0;
            }else{
               path = FSM_RW_FILE_PATH_1;
            }
            break;
        case SECTION_FSM_RF:
            path = FSM_RF_TEMP_PATH;
            break;
        default:
            LOGFLS(LOG_NOTICE, "fsm upload invalid image type %d\n", img_type);
        }
    }else if(type == FRAME_TYPE_CBP_CRASH_UPLOAD_CMD){
        path = CRASH_FILE_PATH;
        LOGFLS(LOG_NOTICE, "Upload %d bytes crash image into %s.\n", fsize, path);
    }
#ifdef RAMDUMP_FUNC_SUPPORTED
    else if(type == FRAME_TYPE_CBP_RAMDUMP_UPLOAD_CMD){

        build_ramdump_image_name(ramdump_img_path, sizeof(ramdump_img_path));
        path = ramdump_img_path;
        LOGFLS(LOG_NOTICE, "Upload %d bytes ramdump image into %s.\n", fsize, path);
    }
#endif
    else{
        LOGFLS(LOG_ERR, "unknow file type 0x%04x\n", type);
        goto session_error;
    }

    fd = open(path, O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP);
    if(fd < 0){
        LOGFLS(LOG_ERR, "Fail to open %s: %s (code: %d)\n", path, strerror(errno), errno);
        goto session_error;
    }

    //setp 3:response the first command to start data upload
    ret = cbp_upload_response(type, SeqGet, STATUS_OK);
    if(ret < 0){
        LOGFLS(LOG_ERR, "Fail to respones seqnum(%u).\n", SeqGet);
        goto session_error;
    }

    SeqAck = SeqGet + 1;
    total = 0;
    set_state(FLS_STATE_IMAGE_UPLOAD);

    //step 4: receive the all file datas and save to storage file
    do{
        frmRecv = wait_frame(UPLOAD_WAIT);
        if(frmRecv != type){
            LOGFLS(LOG_ERR, "Timeout to wait upload data command.\n");
            goto session_error;
        }

        UploadCmd = (CBP_MSG_UPLOAD_CMD *)rframe->payload;
        data = rframe->payload + sizeof(CBP_MSG_UPLOAD_CMD);
        SeqGet = fls_ntoh32(UploadCmd->seq);
        rsize = fls_ntoh16(UploadCmd->length);
        if(SeqGet != SeqAck){
            LOGFLS(LOG_ERR, "Invalid SeqNum(%u) which is not equal to SeqAck(%u).\n", SeqGet, SeqAck);
            goto session_error;
        }

        if(rsize > 0){
            total += rsize;
            ret = write(fd, data, rsize);
            if(ret <= 0){
                LOGFLS(LOG_ERR, "Fail to write %s: %s (code: %d)\n", path, strerror(errno), errno);
                eStatus = STATUS_NO_MEM;
                goto session_error;
            }
        }

        ret = cbp_upload_response(type, SeqGet, STATUS_OK);
        if(ret < 0){
            LOGFLS(LOG_ERR, "Fail to respones SeqNum(%u).\n", SeqGet);
            goto session_error;
        }
        SeqAck = SeqGet + 1;
    }while(total < fsize);

    //step 4: sync the storage file and check the image
    ret = fsync(fd);
    if(ret < 0){
        LOGFLS(LOG_ERR, "Fail to fsync temp file: %s (code: %d)\n", strerror(errno), errno);
        goto session_error;
    }
    close(fd);
    fd = -1;

    //setp 5:response the upload command to end
    if(type == FRAME_TYPE_CBP_FSM_UPLOAD_CMD){
        if(!cbp_valid_image(path)){
            LOGFLS(LOG_ERR, "Invalid storage file %s.\n", path);
            goto session_error;
        }
        fsm_index = !fsm_index;
        fsm_set_pingpang_index(fsm_index);

    }else if (type == FRAME_TYPE_CBP_FSM_UPLOAD_CMD_V2){
        if(!cbp_valid_image(path)){
            LOGFLS(LOG_ERR, "Invalid storage file %s.\n", path);
            goto session_error;
        }
        if(img_type == SECTION_FSM_CUST)
        {
            fsm_cust_index = !fsm_cust_index;
            fsm_cust_set_pingpang_index(fsm_cust_index);
        }
        else if(img_type == SECTION_FSM_RW)
        {
            fsm_rw_index = !fsm_rw_index;
            fsm_rw_set_pingpang_index(fsm_rw_index);
        }
        else if(img_type == SECTION_FSM_RF)
        {
            create_fsm_rf_image_from_temp_file();
        }
        else
        {
            LOGFLS(LOG_ERR, "Invalid image type %d.\n", img_type);
        }
    }
#ifdef RAMDUMP_FUNC_SUPPORTED
    else if(type == FRAME_TYPE_CBP_RAMDUMP_UPLOAD_CMD){
        notify_taglog_ramdump_event(path);
    }
#endif
    ret = 0;
    gettimeofday(&tv_e, NULL);
    LOGFLS(LOG_NOTICE, "Image upload into %s ok\n", path);
    LOGFLS(LOG_NOTICE, "--- End session image upload --- [%f]\n",(float)(tv_e.tv_sec - tv_b.tv_sec)+(float)(tv_e.tv_usec-tv_b.tv_usec)/1000000);
    return ret;

session_error:
    ret = -1;
    if(fd >= 0){
        close(fd);
    }

    LOGFLS(LOG_WARNING, "Response error status(%u) for SeqGet(%u) according to SeqAck(%u).\n", eStatus, SeqGet, SeqAck);
    cbp_upload_response(type, SeqGet, eStatus);

    LOGFLS(LOG_NOTICE, "--- End Session image upload  ---\n");
    return ret;
}

/*
 * Purpose:  erase image during cbp boot
 * Input:    EraseCmd - the command frame to erase image
 * Return:   0 is ok else return -1
*/
static int session_cbp_image_erase(CBP_MSG_ERASE_CMD * EraseCmd)
{
    int ret = -1;
    char *path = NULL;
    CBP_MSG_ERASE_RSP EraseRsp;

    LOGFLS(LOG_NOTICE, "Start session image erase.\n");

    if(EraseCmd == NULL){
        LOGFLS(LOG_WARNING, "Erase command is NULL\n");
        return -1;
    }

    // step 1: prepare the image file for erase
    switch(EraseCmd->section){
        case SECTION_FSM_CUST:
            LOGFLS(LOG_NOTICE, "erase fsm_cust image, fsm_cust_index = %d.\n", fsm_cust_index);
            if(fsm_cust_index){
                path = FSM_CUST_FILE_PATH_1;
            }else{
                path = FSM_CUST_FILE_PATH_0;
            }
            fsm_cust_index = !fsm_cust_index;
            fsm_cust_set_pingpang_index(fsm_cust_index);
            break;
        case SECTION_FSM_RW:
            LOGFLS(LOG_NOTICE, "erase fsm image, fsm_rw_index = %d.\n", fsm_rw_index);
            if(fsm_rw_index){
                path = FSM_RW_FILE_PATH_1;
            }else{
                path = FSM_RW_FILE_PATH_0;
            }
            fsm_rw_index = !fsm_rw_index;
            fsm_rw_set_pingpang_index(fsm_rw_index);
            break;
        case SECTION_FSM:
            LOGFLS(LOG_NOTICE, "erase fsm image, fsm_index = %d.\n", fsm_index);
            if(fsm_index){
                path = FSM_FILE_PATH_1;
            }else{
                path = FSM_FILE_PATH_0;
            }
            fsm_index = !fsm_index;
            fsm_set_pingpang_index(fsm_index);
            break;
        case SECTION_LOG:
            LOGFLS(LOG_NOTICE, "erase crash log image.\n");
            path = CRASH_FILE_PATH;

            break;
        default:
            LOGFLS(LOG_WARNING, "Unknow Section ID %u for download command.\n", EraseCmd->section);
            goto session_error;
    }

    // step 2: delete the image file
    if(path){
        unlink(path);
        /*judge whether the image file has been deleted*/
        ret = access(path, R_OK);
        if(ret){
            ret = 0;
        }else{
            ret = -1;
        }
    }

    //setp 3: response the erase command
    ret = 0;
session_error:
    EraseRsp.id = fls_hton16(FRAME_TYPE_CBP_ERASE_RSP);
    EraseRsp.section = EraseCmd->section;
    if(ret){
        EraseRsp.status = STATUS_ERROR;
    }else{
        EraseRsp.status = STATUS_OK;
    }
    cbp_frame_write((uint8 *)&EraseRsp, sizeof(EraseRsp));

    LOGFLS(LOG_NOTICE, "End session image erase.\n");

    return ret;
}


/*
 * Purpose:  delete FSM
 * Input:    void
 * Return:   0 is ok else return -1
*/
static int session_cbp_fsm_error(void)
{
    char *path = NULL;

    if(cur_download_fsm_type == SECTION_FSM_CUST)
    {
       if(fsm_cust_index){
           path = FSM_CUST_FILE_PATH_1;
       }else{
           path = FSM_CUST_FILE_PATH_0;
       }
       fsm_cust_index = !fsm_cust_index;
       fsm_cust_set_pingpang_index(fsm_cust_index);
    }
    else if(cur_download_fsm_type == SECTION_FSM_RW)
    {
       if(fsm_rw_index){
           path = FSM_RW_FILE_PATH_1;
       }else{
           path = FSM_RW_FILE_PATH_0;
       }
       fsm_rw_index = !fsm_rw_index;
       fsm_rw_set_pingpang_index(fsm_rw_index);
    }
    else {
        LOGFLS(LOG_NOTICE, "using old image type...");
        if(fsm_index){
            path = FSM_FILE_PATH_1;
        }else{
            path = FSM_FILE_PATH_0;
        }
        fsm_index = !fsm_index;
        fsm_set_pingpang_index(fsm_index);
    }

    LOGFLS(LOG_NOTICE, "delete FSM file:%s.\n", path);
    unlink(path);

    return 0;
}

/*
 * Purpose:  Save the calibration FSM
 * Input:    FsmCalCmd: the command to save calibration FSM
 * Return:   0 is ok else return -1
*/
static int session_cbp_fsm_calibrate(__attribute__((unused))CBP_MSG_FSM_CALIBRATE_CMD *FsmCalCmd)
{
    int ret = -1, len;
#define BLOCK_SIZE (1024)
    char buf[BLOCK_SIZE];
    char *path;
    char *rf_bk_path;
    int fd_back = -1, fd_fsm = -1;
    CBP_MSG_FSM_CALIBRATE_RSP FsmCalRsp;

    LOGFLS(LOG_NOTICE, "--- Start session FSM calibration ---\n");

    if(fsm_index){
        path = FSM_FILE_PATH_1;
    }else{
        path = FSM_FILE_PATH_0;
    }

    rf_bk_path = FSM_FILE_PATH_CL;

    LOGFLS(LOG_NOTICE, "Request calibration from %s to %s.\n", path, rf_bk_path);
    if(!cbp_valid_image(path)){
        LOGFLS(LOG_ERR, "Invalid fsm image %s for backup.", path);
        rf_bk_path = FSM_FILE_PATH_RF;
        goto try_backup_directly;
    }

    fd_fsm = open(path, O_RDONLY);
    if(fd_fsm < 0){
        LOGFLS(LOG_ERR, "fail to open fsm image %s.(error:%s, errorno:%d)\n", path, strerror(errno), errno);
        goto session_error;
    }
    fd_back = open(rf_bk_path, O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP);
    if(fd_back < 0){
        LOGFLS(LOG_ERR, "fail to open back image %s.(error:%s, errorno:%d)\n", rf_bk_path, strerror(errno), errno);
        goto session_error;
    }

    do{
        len = read(fd_fsm, buf, BLOCK_SIZE);
        if(len < 0){
            LOGFLS(LOG_ERR, "fail to read fsm image %s.(error:%s, errorno:%d)\n", rf_bk_path, strerror(errno), errno);
            goto session_error;
        }
        if(len > 0){
            ret = write(fd_back, buf, len);
            if(ret < 0){
                LOGFLS(LOG_ERR, "fail to write backup image %s.(error:%s, errorno:%d)\n", path, strerror(errno), errno);
                goto session_error;
            }
        }
    }while(len > 0);

    fsync(fd_back);
    close(fd_back);
    close(fd_fsm);

try_backup_directly:
    fd_back = fd_fsm = -1;
    if(!cbp_valid_image(rf_bk_path)){
        LOGFLS(LOG_ERR, "Invalid backup image %s.", rf_bk_path);
        goto session_error;
    }

    ret = fsm_calibration_backup(rf_bk_path);
    if(ret){
        LOGFLS(LOG_ERR, "Backup fsm API error(%d).\n", ret);
    }else{
        LOGFLS(LOG_NOTICE, "delete temp files %s and %s\n", FSM_FILE_PATH_0, FSM_FILE_PATH_1);
        if(unlink(FSM_FILE_PATH_0) < 0) {
            LOGFLS(LOG_ERR, "fail to delete %s, (error:%s, errorno:%d)\n",
                    FSM_FILE_PATH_0, strerror(errno), errno);
        }
        if(unlink(FSM_FILE_PATH_1) < 0) {
            LOGFLS(LOG_ERR, "fail to delete %s, (error:%s, errorno:%d)\n",
                    FSM_FILE_PATH_1, strerror(errno), errno);
        }
    }

session_error:
    if(fd_back >= 0){
        close(fd_back);
    }
    if(fd_fsm >= 0){
        close(fd_fsm);
    }

    FsmCalRsp.id = fls_hton16(FRAME_TYPE_CBP_FSM_CALIBRATE_RSP);
    if(ret){
        LOGFLS(LOG_NOTICE, "Fsm calibration error.\n");
        FsmCalRsp.status= fls_hton32(STATUS_BAD_SEQ);
    }else{
        LOGFLS(LOG_NOTICE, "Fsm calibration ok.\n");
        FsmCalRsp.status= fls_hton32(STATUS_OK);

    }
    ret = cbp_frame_write((uint8 *)(&FsmCalRsp), sizeof(CBP_MSG_FSM_CALIBRATE_RSP));
    LOGFLS(LOG_NOTICE, "--- End session FSM calibration ---\n");
    return ret;
}
/*
 * Purpose:  bootloader start
 * Input:    void
 * Return:   0 is ok else return -1
*/static int session_cbp_bootloader_start(void)
{
    LOGFLS(LOG_NOTICE, "CBP bootloader start\n");
    return 0;
}

/*
 * Purpose:  bootloader end
 * Input:    void
 * Return:   0 is ok else return -1
*/static int session_cbp_bootloader_end(void)
{
    LOGFLS(LOG_NOTICE, "CBP bootloader end\n");
    return 0;
}
/*
* Purpose:  shows how to use this program
* Input:    name - string containing name of program
* Return:    -1
*/
static int usage(char *_name)
{
    fprintf(stderr, "\tUsage: %s [options]\n", _name);
    fprintf(stderr, "Options:\n");
    // process control
    fprintf(stderr, "\t-d: Run as a daemon: [%s]\n", noDaemon?"no":"yes");
    fprintf(stderr, "\t-v: verbose logging level: [%d]. 0(Silent) - 7(Debug)\n",logLevel);
    // modem control
    fprintf(stderr, "\t-a: Asciboot download serial device: [%s]\n", asciDevice);
    fprintf(stderr, "\t-c: Cbp data exchange device: [%s]\n", cbpDevice);
    fprintf(stderr, "\t-t: Timeout to wait for ack: [%d]\n", useTimeout);
    fprintf(stderr, "\t-r: Retry times to resend message if not ack: [%d]\n", useRetry);
    // legacy - will be removed
    fprintf(stderr, "\t-b: Baudrate for asciboot serial: [%d]\n", baudRates[comSpeed]);
    fprintf(stderr, "\t-m: In [%s] mode\n", run_mode == MODE_NORMAL ? "normal" : "factory");
    fprintf(stderr, "\t-l: Log to file %s: [%s]\n", LOG_FILE_PATH, logToFile?"yes":"no");
    //help
    fprintf(stderr, "\t-h: Show this help message and show current settings.\n");
    return -1;
}

/*Do nothing but monitor the CBP*/
int main_simple(void)
{
    int sel, ret=-1, count=0, max=0;
    fd_set rfds;
    FlashlessDev *cbpDev = phd->cbp;
    FlashlessFrame *cbpFrm = &cbpDev->rframe;
    LOGFLS(LOG_NOTICE, "Simple flashlessd start\n");

    ret = cbp_device_prepare(cbpDev);
    if(ret < 0){
        flashless_die("cbp device read error");
        return -1;
    }
    if (cbpDev->fd < 0) {
        flashless_die("cbpDev->fd value is -1");
        return -1;
    }
#ifndef USE_STATUSD
    property_set("ctl.start", "viaril-daemon");
    LOGFLS(LOG_NOTICE, "start viaril-daemon\n");
#endif
    while(1){
        FD_ZERO(&rfds);
        max = cbpDev->fd;
        FD_SET(cbpDev->fd, &rfds);
        sel = select(max + 1, &rfds, NULL, NULL, NULL);
        LOGFLS(LOG_NOTICE, "read selected %d (cbp=%d)\n", sel, cbpDev->fd);
        if(sel > 0){//read data
            if((cbpDev->fd > 0) && FD_ISSET(cbpDev->fd, &rfds)){
                count = read(cbpDev->fd, cbpFrm->buf + cbpFrm->dlen, CBP_MAX_FRAME_SIZE - cbpFrm->dlen);
                LOGFLS(LOG_INFO, "cbpDev read %d data\n", count);
                if(count <= 0 && errno != EINTR){
                    LOGFLS(LOG_ERR, "cbp device read erro '%s' (code: %d)\n", strerror(errno), errno);
                    ret = cbp_device_prepare(cbpDev);
                    if(ret < 0){
                        flashless_die("cbp device read error");
                        break;
                    }
                }else if(count > 0){
                    //do nothing
                }else{
                    //do nothing
                }
            }
        }
    }

    return -1;
}
#ifdef USE_STATUSD
static int flashless_com_callback(char *src_module,__attribute__((unused))char *dst_module,
					char *data_type,__attribute__((unused))int *data_len,unsigned char *buffer)
{
    char type=*data_type;
    unsigned char event=buffer[0];

    LOGFLS(LOG_NOTICE,"msg from %s:%s type=%d event=%d\n",
                    name_inquery(*src_module),type_inquery(event),type,event);
    if((type == STATUS_DATATYPE_CMD) && (event==CMD_KILL_CLIENT)){
        LOGFLS(LOG_NOTICE, "%s killed by statusd\n",LOG_TAG);
        exit(-1);
     }
    return 0;
}
#endif

/*
 * Purpose:  The main program loop, which monitor the status
 * Input:    argc - number of input arguments
 *           argv - array of strings (input arguments)
 * Return:    0
 */
int main(int argc,char *argv[])
{
    int ret = -1, opt;
    int simple = 0;
    int idRecv;
#ifdef RAMDUMP_FUNC_SUPPORTED
    struct itimerval itv;
#endif
    uid_t uid = getuid();
    struct stat infor;
#ifndef USE_STATUSD
    struct sigaction act;
#endif
    struct timeval tv_b,tv_e;

    tStart = time(NULL);
    gettimeofday(&tv_b, NULL);

#ifdef RAMDUMP_FUNC_SUPPORTED
    while ((opt = getopt(argc, argv, "h?lsdm:v:t:r:b:f:")) > 0)
#else
    while ((opt = getopt(argc, argv, "h?lsdm:v:t:r:b:")) > 0)
#endif
    {
        switch (opt)
        {
            case 'v':
                logLevel = atoi(optarg);
                if ((logLevel > LOG_DEBUG) || (logLevel < 0)){
                    usage(argv[0]);
                    exit(0);
                }
                break;
            case 'l':
                ret = stat(LOG_FILE_PATH, &infor);
                /*trunk the debug file if it is too big*/
                if(0 == ret && (infor.st_size > MAX_DEBUG_LOG_SIZE)){
                    logFile = fopen(LOG_FILE_PATH, "w+");
                }else{
                    logFile = fopen(LOG_FILE_PATH, "a+");
                }
                if (logFile == NULL){
                    fprintf(stderr, "File to open file %s, Error: %s.\n", LOG_FILE_PATH, strerror(errno));
                }else{
                    logToFile = 1;
                    if (uid == 0) {/*if user is root*/
                        chown(LOG_FILE_PATH, AID_RADIO, AID_RADIO);
                    }
                    fprintf(logFile, "\n\n################### Flashless log %s\n", ctime(&tStart));
                }
                break;
            case 'd':
                noDaemon = !noDaemon;
                break;
            case 's':
                simple = 1;
                break;
            case 't':
                useTimeout = atoi(optarg);
                break;
            case 'r':
                useRetry = atoi(optarg);
                break;
            case 'b':
                comSpeed = baud_rate_index(atoi(optarg));
                break;
            case 'm':
                run_mode= atoi(optarg);
                if(run_mode > 1)
                    run_mode = MODE_NORMAL;
                break;
#ifdef RAMDUMP_FUNC_SUPPORTED
            case 'f':
                flsExtraFunctions |= atoi(optarg);
                break;
#endif
            default:
            case '?':
            case 'h':
                usage(argv[0]);
                exit(0);
                break;
        }
    }
#ifdef USE_STATUSD
    if((uid == 0) && (run_mode == MODE_NORMAL)){/*if it is root and in normal running mode*/
        setuid(AID_RADIO);
    }
#endif

#if defined(VIA_SUFFIX_VERSION)
    LOGFLS(LOG_WARNING, "Flashless version: %s, suffix version:%s\n", revision, VIA_SUFFIX_VERSION);
#else
    LOGFLS(LOG_WARNING, "Flashless version: %s\n", revision);
#endif

#ifndef USE_STATUSD/*if we use statusd,cbp had been powered up by statusd*/
    LOGFLS(LOG_NOTICE, "Power on the modem.\n");
    viatelModemPower(1);
#endif

    umask(0007);

    //signals treatment
    signal(SIGKILL, signal_treatment);
    signal(SIGTERM, signal_treatment);

#ifdef RAMDUMP_FUNC_SUPPORTED
    if(flsExtraFunctions & SUPPORT_EXTRA_FUNC_RAMDUMP) {
        //setup timer to check dynamical sdcard mount event, cycle interval 60 seconds
        signal(SIGALRM, sdcard_mount_event_monitor);
        itv.it_value.tv_sec = 60;
        itv.it_value.tv_usec = 0;
        itv.it_interval.tv_sec = 60;
        itv.it_interval.tv_usec = 0;
        setitimer(ITIMER_REAL, &itv, NULL);
        LOGFLS(LOG_NOTICE,"setup sdcard monitor timer\n");
    }
#endif

#ifndef USE_STATUSD
    //regist SIGRTMIN to process the notification from vmodem
    sigemptyset(&act.sa_mask);
    act.sa_sigaction = vmodem_sigaction;
    act.sa_flags = SA_SIGINFO;
    sigaction(SIGRTMIN, &act, NULL);

    fd = open("/dev/vmodem", O_RDWR);
    if(fd < 0){
        LOGFLS(LOG_ERR, "Fail to open vmodem devices.");
        goto main_exit;
    }
    fcntl(fd, F_SETOWN, getpid());
    fcntl(fd, F_SETFL, fcntl(fd, F_GETFL) | FASYNC);
    fcntl(fd, F_SETSIG, SIGRTMIN);
#else
    ret = statusd_register_cominf(MODULE_TYPE_FLS,(statusd_data_callback)flashless_com_callback);
    if(ret){
        LOGFLS(LOG_ERR,"CLIENT %s :: register com interface failed ret=%d\n",LOG_TAG,ret);
    }
    ret = statusd_c2ssend_cmd(MODULE_TYPE_FLS,MODULE_TYPE_SR,CMD_CLIENT_INIT);
    if(ret < 1){
        LOGFLS(LOG_ERR,"CLIENT %s::send CMD_CLIENT_INIT cmd failed ret=%d\n",LOG_TAG,ret);
    }
#endif

#ifdef FLASHLESS_NOT_ANDROID
    if (noDaemon)
        openlog(argv[0], LOG_NDELAY | LOG_PID | LOG_PERROR, LOG_LOCAL0);
    else
        openlog(argv[0], LOG_NDELAY | LOG_PID, LOG_LOCAL0);
#endif
    phd = flashless_handle_create();
    if(phd == NULL){
        LOGFLS(LOG_ERR,"Out of memory");
        goto main_exit;
    }

    little_endian = check_cpu_endian();
    LOGFLS(LOG_EMERG, "packege size=%d window size=%d logLevel=%d\n", CBP_MAX_PAYLOAD_SIZE,MAX_DATA_WINDOW,logLevel);
    LOGFLS(LOG_NOTICE, "CPU is %s\n", little_endian ? "little-endian" : "big-endian");


    LOGFLS(LOG_DEBUG, "%s %s starting\n", *argv, revision);
    LOGFLS(LOG_NOTICE,"Called with following options:\n");
    LOGFLS(LOG_NOTICE,"\t-d: Fork, get a daemon [%s]\n", noDaemon?"no":"yes");
    LOGFLS(LOG_NOTICE,"\t-s: Simple run mode [%s]\n", simple?"yes":"no");
    LOGFLS(LOG_NOTICE,"\t-v: verbose logging level. 0 (Silent) - 7 (Debug) [%d]\n",logLevel);
    LOGFLS(LOG_NOTICE,"\t-t <timeout>: the timeout to wait for ack [%d]\n", useTimeout);
    LOGFLS(LOG_NOTICE,"\t-r <retry>: the times to resend message if not ack [%d]\n", useRetry);
    LOGFLS(LOG_NOTICE,"\t-b <baudrate>: baudrate for asciboot serial[%d]\n", baudRates[comSpeed]);
    LOGFLS(LOG_NOTICE,"\t-m: In [%s] mode\n", run_mode == MODE_NORMAL ? "normal" : "factory");
    LOGFLS(LOG_NOTICE,"\t-l <log to file>: Output log to /data/flashless/debug.log [%s]\n", logToFile?"yes":"no");
#ifdef RAMDUMP_FUNC_SUPPORTED
    LOGFLS(LOG_NOTICE,"\t-f <extra supported functions>: [%s]\n", flsExtraFunctions & SUPPORT_EXTRA_FUNC_RAMDUMP ? "RAMDUMP" : "NONE");
#endif


    /*just start ril and monitor the cbp error*/
    if(simple){
        main_simple( );
        goto main_exit;
    }

    /*check files*/
    if(access(BOOT_FILE_PATH, R_OK) < 0){
        LOGFLS(LOG_ERR, "Can not read boot file named %s.\n", BOOT_FILE_PATH);
        exit(-1);
    }

    if(access(CP_FILE_PATH, R_OK) < 0){
        LOGFLS(LOG_ERR, "Can not read cp file named %s.\n", CP_FILE_PATH);
        exit(-1);
    }

    rfs_access_ok();

    fsm_cust_index = fsm_cust_get_pingpang_index();
    fsm_rw_index = fsm_rw_get_pingpang_index();
    fsm_index = fsm_get_pingpang_index();

    set_state(FLS_STATE_ASCI_HANDSHAKE);

    if(asci_device_prepare(phd->asci) < 0){
        LOGFLS(LOG_ERR,"Fail to prepare asci device\n");
        goto main_exit;
    }

    if(create_thread(&recv_frame_thread, receive_frame_thread, NULL) < 0){
        LOGFLS(LOG_ERR,"Could not create the thread to read asci frame\n");
        goto main_exit;
    }

    if(session_asci_boot() < 0){
        LOGFLS(LOG_ERR,"Fail to try session asci\n");
        goto main_exit;
    }

    if(cbp_device_prepare(phd->cbp) < 0){
        LOGFLS(LOG_ERR,"Fail to get cbp bootloader device\n");
        goto main_exit;
    }

    while(1){
        set_state(FLS_STATE_WAIT_REQUEST);
        idRecv = wait_frame(0);
        switch(idRecv){
            case FRAME_TYPE_CBP_DOWNLOAD_CMD:
                session_cbp_image_download((CBP_MSG_DOWNLOAD_CMD *)(phd->cbp->rframe.payload));
                break;
            case FRAME_TYPE_CBP_FSM_UPLOAD_CMD:
            case FRAME_TYPE_CBP_FSM_UPLOAD_CMD_V2:
            case FRAME_TYPE_CBP_CRASH_UPLOAD_CMD:
#ifdef RAMDUMP_FUNC_SUPPORTED
            case FRAME_TYPE_CBP_RAMDUMP_UPLOAD_CMD:
#endif
                session_cbp_image_upload((CBP_MSG_UPLOAD_CMD *)(phd->cbp->rframe.payload));
                break;
            case FRAME_TYPE_CBP_ERASE_CMD:
                session_cbp_image_erase((CBP_MSG_ERASE_CMD *)(phd->cbp->rframe.payload));
                break;
            case FRAME_TYPE_CBP_FSM_ERR:
                session_cbp_fsm_error();
                break;
            case FRAME_TYPE_CBP_FSM_CALIBRATE_CMD:
                session_cbp_fsm_calibrate((CBP_MSG_FSM_CALIBRATE_CMD *)(phd->cbp->rframe.payload));
                break;
            case FRAME_TYPE_CBP_BOOTLOADER_START:
                session_cbp_bootloader_start();
                break;
            case FRAME_TYPE_CBP_BOOTLOADER_END:
                session_cbp_bootloader_end();
                gettimeofday(&tv_e, NULL);
                LOGFLS(LOG_NOTICE, "Time to boot cp: %f \n", (float)(tv_e.tv_sec - tv_b.tv_sec)+(float)(tv_e.tv_usec-tv_b.tv_usec)/1000000);
                #ifdef USE_STATUSD
                    ret = statusd_c2ssend_cmd(MODULE_TYPE_FLS,MODULE_TYPE_SR,CMD_CLIENT_READY);
                     LOGFLS(LOG_ERR,"CLIENT %s::send CMD_CLIENT_READY cmd ret=%d\n",LOG_TAG,ret);
                    if(ret != 1){
                        LOGFLS(LOG_ERR,"CLIENT %s::send CMD_CLIENT_READY cmd failed ret=%d\n",LOG_TAG,ret);
                    }
                #endif
                break;
            default:
                LOGFLS(LOG_NOTICE, "ignore frame 0x%04x\n", idRecv);
        }
    }

main_exit:
    if (logFile){
        fclose(logFile);
    }
    flashless_die("main exit");
    return ret;
}
