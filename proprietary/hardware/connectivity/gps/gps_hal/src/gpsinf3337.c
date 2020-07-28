/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2010. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
 * AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek Software")
 * have been modified by MediaTek Inc. All revisions are subject to any receiver's
 * applicable license agreements with MediaTek Inc.
 */
#include <errno.h>
#include <pthread.h>
#include <fcntl.h>
#include <sys/epoll.h>
#include <math.h>
#include <time.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <semaphore.h>

#include <sys/ioctl.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <cutils/android_filesystem_config.h>

#define LOCATION_NLP_FIX "/data/vendor/gps/LOCATION.DAT"
#define  LOG_TAG  "gps_mtk_3337"
#include <log/log.h>
#include <cutils/sockets.h>
#include <cutils/properties.h>
#ifdef HAVE_LIBC_SYSTEM_PROPERTIES
#define _REALLY_INCLUDE_SYS__SYSTEM_PROPERTIES_H_
#include <sys/_system_properties.h>
#endif

#include "hardware/gps_mtk.h"
#include "gpshal.h"
#include "mtk_lbs_utility.h"
#include "data_coder.h"
#include <termios.h>

/* the name of the controlled socket */
#define  GPS_POWER_NAME       "/dev/gps"
#define  GPS_TTY_NAME       "/dev/ttyMT1"

#define  MNLD_HAL2GPS       "/dev/mt3337_gpsonly"
#define MNL_CONFIG_STATUS   "persist.vendor.radio.mnl.prop"

#define PMTK_SOCKET_COM_PORT 49995
#define PMTK_SERVER_BACKLOG 5
#define INVALID_SOCKET -1
#define INVALID_FD -1
#define  GPS_OP   "AT%GPS"
#define  GNSS_OP  "AT%GNSS"
#define  GPS_AT_ACK_SIZE        60

#define MTKEPO_SV_NUMBER 32
#define MTKEPO_RECORD_SIZE 72
#define MTKEPO_SEGMENT_NUM (30 * 4)

#define  GPS_DEBUG  1
#define  NEMA_DEBUG 0   /*the flag works if GPS_DEBUG is defined*/
#if GPS_DEBUG
#define  TRC(f)       ALOGD("%s", __func__)
#define  ERR(f, ...)  ALOGE("%s: line = %d" f, __func__,__LINE__, ##__VA_ARGS__)
#define  WAN(f, ...)  ALOGW("%s: line = %d" f, __func__, __LINE__,##__VA_ARGS__)
#define DBG(f, ...) ALOGD("%s: line = %d" f, __func__, __LINE__,##__VA_ARGS__)
#define VER(f, ...) ((void)0) //((void)0)//
#else
#  define DBG(...)    ((void)0)
#  define VER(...)    ((void)0)
#endif

#define GPS_POWER_IOCTL _IOW('G', 0, int)
static int flag_unlock = 0;
GpsStatus sta;

extern struct hw_module_t HAL_MODULE_INFO_SYM;

typedef  unsigned int             UINT4;
typedef  signed int               INT4;
typedef unsigned char           UINT8;
typedef signed char             INT8;
typedef unsigned short int      UINT16;
typedef signed short int        INT16;
typedef unsigned int            UINT32;
typedef signed int              INT32;
typedef signed long long       INT64;

#pragma pack(4) // Align by 4 byte
typedef struct
{
   UINT32 size;
   UINT16 flag;
   INT16 leapsecond;
   UINT8 type;
   INT64 TimeInNs;
   double TimeUncertaintyInNs;
   INT64 FullBiasInNs;
   double BiasInNs;
   double BiasUncertaintyInNs;
   double DriftInNsPerSec;
   double DriftUncertaintyInNsPerSec;
}MTK_GPS_CLOCK;
#pragma pack()

typedef struct{
    GpsUtcTime time;
    int64_t timeReference;
    int uncertainty;
} ntp_context;

typedef struct{
    double latitude;
    double longitude;
    float accuracy;
    struct timespec ts;
    unsigned char  type;
    unsigned char  started;
} nlp_context;

/*****************************************************************************/
/*    MTK device control                                                  */
/*****************************************************************************/
enum {
    MNL_CMD_UNKNOWN = -1,
    /*command send from GPS HAL*/
    MNL_CMD_INIT            = 0x00,
    MNL_CMD_CLEANUP         = 0x01,
    MNL_CMD_STOP            = 0x02,
    MNL_CMD_START           = 0x03,
    MNL_CMD_RESTART         = 0x04,    /*restart MNL process*/
    MNL_CMD_RESTART_HOT     = 0x05,    /*restart MNL by PMTK command: hot start*/
    MNL_CMD_RESTART_WARM    = 0x06,    /*restart MNL by PMTK command: warm start*/
    MNL_CMD_RESTART_COLD    = 0x07,    /*restart MNL by PMTK command: cold start*/
    MNL_CMD_RESTART_FULL    = 0x08,    /*restart MNL by PMTK command: full start*/
    MNL_CMD_RESTART_AGPS    = 0x09,    /*restart MNL by PMTK command: AGPS start*/
    MNL_CMD_GPSMEASUREMENT_INIT = 0x15,
    MNL_CMD_GPSMEASUREMENT_CLOSE = 0x16,


    MNL_CMD_GPSNAVIGATION_INIT = 0x17,
    MNL_CMD_GPSNAVIGATION_CLOSE = 0x18,
    MNL_CMD_GPS_LOG_WRITE = 0x40,

    MNL_CMD_ENABLE_AGPS_DEBUG = 0x42,
    MNL_CMD_DISABLE_AGPS_DEBUG = 0x43,
};

enum {
    HAL_CMD_STOP_UNKNOWN = -1,
    HAL_CMD_MNL_DIE = 0x41,
    HAL_CMD_GPS_ICON = 0x42,

    MNL_CMD_GPS_INJECT_TIME = 0x46,
    MNL_CMD_GPS_INJECT_LOCATION = 0x47,
    MNL_CMD_GPS_INJECT_TIME_REQ = 0x48,
    MNL_CMD_GPS_INJECT_LOCATION_REQ = 0x49,

    MNL_CMD_GPS_NLP_LOCATION_REQ = 0x4a,
    HAL_CMD_MEASUREMENT= 0x76,
    HAL_CMD_NAVIGATION= 0x77,
    HAL_CMD_SWITCH_AGPS_DEBUG_DONE = 0x44,
    HAL_CMD_SWITCH_AGPS_DEBUG_FAIL = 0X45,
};



#define M_START 0
#define M_STOP 1
#define M_CLEANUP 2
#define M_INIT 3
#define M_THREAD_EXIT 4
#define M_MNLDIE 5
typedef struct sync_lock
{
    pthread_mutex_t mutx;
    pthread_cond_t con;
    int condtion;
}SYNC_LOCK_T;

static SYNC_LOCK_T lock_for_sync[] = {{PTHREAD_MUTEX_INITIALIZER,PTHREAD_COND_INITIALIZER, 0},
                                    {PTHREAD_MUTEX_INITIALIZER,PTHREAD_COND_INITIALIZER, 0},
                                    {PTHREAD_MUTEX_INITIALIZER,PTHREAD_COND_INITIALIZER, 0},
                                    {PTHREAD_MUTEX_INITIALIZER,PTHREAD_COND_INITIALIZER, 0},
                                    {PTHREAD_MUTEX_INITIALIZER,PTHREAD_COND_INITIALIZER, 0},
                                    {PTHREAD_MUTEX_INITIALIZER,PTHREAD_COND_INITIALIZER, 0}};

const char* gps_native_thread = "GPS NATIVE THREAD";
const char* gps_pmtk_thread = "GPS PMTK THREAD";
const char* gps_nmea_read_thread = "GPS NMEA READ THREAD";
const char* gps_send_epo_thread = "GPS SEND EPO THREAD";
static GpsCallbacks callback_backup;
static int start_flag = 0;
static int g_delete_aiding_data_flag = 0;

//for different SV parse
typedef enum{
    UNKNOWN_SV = 0,
    GPS_SV,
    GLONASS_SV,
    BDS_SV,
    GALILEO_SV,
}SV_TYPE;
static float report_time_interval = 0;
static int started = 0;

//#define  NMEA_MAX_SIZE  83
#define  NMEA_MAX_SIZE  255
/*maximum number of SV information in GPGSV*/
#define  NMEA_MAX_SV_INFO 4
#define  LOC_FIXED(pNmeaReader) ((pNmeaReader->fix_mode == 2) || (pNmeaReader->fix_mode ==3))
typedef struct {
    int     pos;
    int     overflow;
    int     utc_year;
    int     utc_mon;
    int     utc_day;
    int     utc_diff;
    GpsLocation  fix;

    /*
     * The fix flag extracted from GPGSA setence: 1: No fix; 2 = 2D; 3 = 3D
     * if the fix mode is 0, no location will be reported via callback
     * otherwise, the location will be reported via callback
     */
    int     fix_mode;
    /*
     * Indicate that the status of callback handling.
     * The flag is used to report GPS_STATUS_SESSION_BEGIN or GPS_STATUS_SESSION_END:
     * (0) The flag will be set as true when callback setting is changed via nmea_reader_set_callback
     * (1) GPS_STATUS_SESSION_BEGIN: receive location fix + flag set + callback is set
     * (2) GPS_STATUS_SESSION_END:   receive location fix + flag set + callback is null
     */
    int     cb_status_changed;
    int     sv_count;           /*used to count the number of received SV information*/
    GnssSvStatus  sv_status;
    GpsCallbacks callbacks;
    char    in[ NMEA_MAX_SIZE + 1 ];
} NmeaReader;

typedef struct {
    pthread_t               thread;
    int                     server_fd;
}PMTKThread;

/* this is the state of our connection to the daemon */
typedef struct {
    int                     init;  // to check whether, have been initialized
    int                     pow_fd; //power ctrl
    int                     tty_fd; //for read nmea
    GpsCallbacks            callbacks; // up layer callback
    pthread_t               thread;
    PMTKThread              pmtk_thread[1];
    pthread_t               nmea_read_thread;
    int                     control[2];
    int                     test_time;
    int                     epoll_hd;
    int                     flag;
    int                     start_flag;
} GpsState;

static GpsState  _gps_state[1];

/*****************************************************************/
/*****************************************************************/
/*****                                                       *****/
/*****       C O N N E C T I O N   S T A T E                 *****/
/*****                                                       *****/
/*****************************************************************/
/*****************************************************************/

/* commands sent to the gps thread */
enum {
    CMD_QUIT  = 0,
    CMD_START = 1,
    CMD_STOP  = 2,
    CMD_RESTART = 3,
    CMD_DOWNLOAD = 4,

    CMD_TEST_START = 10,
    CMD_TEST_STOP = 11,
    CMD_TEST_SMS_NO_RESULT = 12,
    CMD_EPO_DOWNLOAD_DONE = 13,
};


static int gps_nmea_end_tag = 0;

int sv_used_in_fix[256] = {0};    // for multiple sv display

unsigned char calc_nmea_checksum( const char* sentence);
void send_assistance_data (int iYr, int iMo, int iDay, int iHr, char * timeInfo, int iTimeLen);

static void get_condition(SYNC_LOCK_T *lock)
{
    int ret = 0;

    ret = pthread_mutex_lock(&(lock->mutx));
    DBG("ret get_mutex lock = %d\n",ret);

    while (!lock->condtion)
    {
        DBG("ret get_cond wait = %d\n" ,ret);
        ret = pthread_cond_wait(&(lock->con), &(lock->mutx));
    }

    lock->condtion = 0;
    DBG("ret get_mutex unlock = %d\n",ret);
    ret = pthread_mutex_unlock(&(lock->mutx));
    DBG("test");

    return;
}

static void release_condition(SYNC_LOCK_T *lock)
{
    int ret = 0;


    ret = pthread_mutex_lock(&(lock->mutx));
    DBG("ret release_mutex lock = %d\n",ret);

    lock->condtion= 1;
    ret = pthread_cond_signal(&(lock->con));
    DBG ("ret release_cond_signal = %d\n",ret);

    ret = pthread_mutex_unlock(&(lock->mutx));
    DBG("ret release_unlock= %d\n",ret);

    return;
}

int mtk_restart(unsigned char cmd)
{
    char buf[] = {cmd};
    TRC();
    return 0;
}

/*****************************************************************/
/*****************************************************************/
/*****                                                       *****/
/*****       N M E A   T O K E N I Z E R                     *****/
/*****                                                       *****/
/*****************************************************************/
/*****************************************************************/

typedef struct {
    const char*  p;
    const char*  end;
} Token;

#define  MAX_NMEA_TOKENS  24

typedef struct {
    int     count;
    Token   tokens[ MAX_NMEA_TOKENS ];
} NmeaTokenizer;

static int
nmea_tokenizer_init( NmeaTokenizer*  t, const char*  p, const char*  end ) {
    int    count = 0;
    char*  q;

    // the initial '$' is optional
    if (p < end && p[0] == '$')
        p += 1;

    // remove trailing newline
    if (end > p && end[-1] == '\n') {
        end -= 1;
        if (end > p && end[-1] == '\r')
            end -= 1;
    }

    // get rid of checksum at the end of the sentecne
    if (end >= p+3 && end[-3] == '*') {
        end -= 3;
    }

    while (p < end) {
        const char*  q = p;

        q = memchr(p, ',', end-p);
        if (q == NULL)
            q = end;

        if (q >= p) {
            if (count < MAX_NMEA_TOKENS) {
                t->tokens[count].p   = p;
                t->tokens[count].end = q;
                count += 1;
            }
        }
        if (q < end)
            q += 1;

        p = q;
    }

    t->count = count;
    DBG(" t->count is %d", t->count);
    return count;
}

static Token
nmea_tokenizer_get( NmeaTokenizer*  t, int  index )
{
    Token  tok;
    static const char*  dummy = "";

    if (index < 0 || index >= t->count) {
        tok.p = tok.end = dummy;
    } else
        tok = t->tokens[index];

    return tok;
}


static int
str2int( const char*  p, const char*  end )
{
    int   result = 0;
    int   len    = end - p;
    int   sign = 1;

    if (*p == '-')
    {
        sign = -1;
        p++;
        len = end - p;
    }

    for ( ; len > 0; len--, p++ )
    {
        int  c;

        if (p >= end)
            goto Fail;

        c = *p - '0';
        if ((unsigned)c >= 10)
            goto Fail;

        result = result*10 + c;
    }
    return  sign*result;

Fail:
    return -1;
}

static double
str2float( const char*  p, const char*  end )
{
    int   result = 0;
    int   len    = end - p;
    char  temp[16];

    if (len >= (int)sizeof(temp))
        return 0.;

    memcpy( temp, p, len );
    temp[len] = 0;
    return strtod( temp, NULL );
}

static void mtk_gps_update_location(nlp_context * location)
{
    FILE *fp = NULL;

    fp = fopen(LOCATION_NLP_FIX, "w");
    if (fp != NULL)
    {
        size_t count = 0;
        count = fwrite(location, sizeof(nlp_context), 1, fp);
        if (count != 1)
        {
            DBG("write count:%d, errno:%s\n", count, strerror(errno));
        }
        fclose(fp);
    }
}

static bool g_EpoSend = false;
static void mtk_gps_send_EPO()
{
    time_t timep;
    struct tm* pq;
    time(&timep);
    pq = gmtime(&timep);
    char strBuf[200], outBuf[200];
    memset(strBuf, 0, sizeof(strBuf));
    memset(outBuf, 0, sizeof(outBuf));
    sprintf(strBuf, "PMTK740,%d,%d,%d,%d,%d,%d",pq->tm_year + 1900, pq->tm_mon + 1, pq->tm_mday, pq->tm_hour, pq->tm_min, pq->tm_sec);
    sprintf(outBuf, "$%s*%02X\r\n", strBuf, calc_nmea_checksum(strBuf));
    DBG("final PMTK is %s, size is %d, len is %d", outBuf, sizeof(outBuf), strlen(outBuf));
    send_assistance_data(pq->tm_year + 1900, pq->tm_mon + 1, pq->tm_mday, pq->tm_hour, outBuf, strlen(outBuf));
}

/*****************************************/
/*****************************************/
/*****                                                       *****/
/*****       PowerGPS PMTK handle                 *****/
/*****                                                       *****/
/****************************************/
/****************************************/

static bool g_ThreadExitPMTKIn = false;//flag for PowerGPS thread exist
static int g_hPowerGPS = INVALID_FD;//PowerGPS socket handle
static NmeaReader  g_NmeaReader[1];

/***************Receive From PowerGPS********************/
void * pmtk_input_func(void * arg)
{
    int bytes = 0;
    int ret = 0;
    char pmtk_buf[200];
    char  cmd = CMD_RESTART;
    GpsState*   state = (GpsState*) arg;

    while(g_ThreadExitPMTKIn)
    {
        memset(pmtk_buf, 0, sizeof(pmtk_buf));
        bytes = read(g_hPowerGPS, pmtk_buf, 200);

        if (bytes > 0)
        {
            DBG("!!!!!!!!!! PowerGPS pass : %s, lenth is %d", pmtk_buf, bytes);

            do {
                ret=write( state->control[0], &cmd, 1);
            }
            while (ret < 0 && errno == EINTR);
            do {
                ret=write( state->tty_fd, pmtk_buf, bytes);
            }while (ret < 0 && errno == EINTR);
            if (ret < 0)
                DBG("!!!!!!!!!!!PowerGPS write FAIL, errno is %d  (%s)", errno, strerror(errno));
            else
                DBG("!!!!!!!!!!!!!!PowerGPS write succeed, ret is %d", ret);
            if (strstr(pmtk_buf, "PMTK101")
                || strstr(pmtk_buf, "PMTK102")
                || strstr(pmtk_buf, "PMTK103")
                || strstr(pmtk_buf, "PMTK104"))
            {
                g_EpoSend = true;
                DBG("!!!!!!!PowerGPS got A START %s", pmtk_buf);
            }
        }
        else
        {
            if (bytes == 0)
            { //client close
                DBG("!!!!!!socket client close");
                break;
            }
            else
            {
                usleep(200000);  // sleep 200 ms
                DBG("!!!!!!socket error = %d (%s)\r\n", errno, strerror(errno));
            }
        }
    }
    return NULL;
}

/***************Output To PowerGPS********************/
void pmtk_output_func(char * buf, int * length)
{
    if (buf[(*length) - 2] != '\r'){
        buf[(*length) - 1] = '\r';
        buf[(*length)] = '\n';
        (*length) += 1;
    }
    int len = (*length);
    int ret = 0;
    int written = 0;
    int retry = 0;
    if (g_hPowerGPS != INVALID_FD)
    {
        while( (written != len) && (retry <5))
        {
            ret = write(g_hPowerGPS, buf + written, len - written);
            if (ret < 0)
            {
                retry++;
            }
            else
            {
                written += ret;
            }
        }
    }
}

void powergps_thread_exit(int sig_no, int server_fd)
{
    if(server_fd != INVALID_SOCKET)
    {
        close(server_fd);
        server_fd == INVALID_SOCKET;
    }
    pthread_exit((void *)0);
}

void signal_action_handle(int signum, siginfo_t * info, void *myact)
{
    DBG("######got Signal : %d", signum);
}

/*
 *Create Socket Server to listen PMTK from PowerGPS
 */
void* powergps_pmtk_receive_thread( void*  arg )
{
    TRC();
    g_ThreadExitPMTKIn = true;
    GpsState*   state = (GpsState*) arg;
    int server_fd = state->pmtk_thread->server_fd;

    int conn_fd = INVALID_SOCKET, on;
    struct sockaddr_in server_addr;
    struct sockaddr_in client_addr;
    socklen_t size;
    int hdlsig = SIGUSR1;
    struct sigaction sa;
    char buf[128];

    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        DBG("socket error = %d (%s)\r\n", errno, strerror(errno));
        pthread_exit((void *)0);
        return NULL;
    }
    DBG("########### socket create SUCCESS - pid == %d, fd == %d", getpid(), server_fd);

    /* Enable address reuse */
    on = 1;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on)))
    {
        close(server_fd);
        pthread_exit((void *)0);
        return NULL;
    }
    DBG("########### socket set SUCCESS");

    server_addr.sin_family = AF_INET;   /*host byte order*/
    server_addr.sin_port = htons(PMTK_SOCKET_COM_PORT); /*short, network byte order*/
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY); /*automatically fill with my IP*/
    memset(server_addr.sin_zero, 0x00, sizeof(server_addr.sin_zero));

    if (bind(server_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1)
    {
        close(server_fd);
        pthread_exit((void *)0);
        return NULL;
    }
    DBG("########### socket bind SUCCESS");

    if (listen(server_fd, PMTK_SERVER_BACKLOG) == -1)
    {
        close(server_fd);
        pthread_exit((void *)0);
        return NULL;
    }
    DBG("########### socket listen SUCCESS");

    sa.sa_handler = SIG_IGN;
    sa.sa_sigaction = signal_action_handle;
    sa.sa_flags = (0 | SA_SIGINFO);
    sigemptyset(&sa.sa_mask);
   if (sigaction(SIGPIPE, &sa, NULL) < 0)
    {
        DBG("sigaction error \r\n");
    }

    while (g_ThreadExitPMTKIn)
    {
        size = sizeof(client_addr);
        DBG("########### waiting the client..........");
        conn_fd = accept(server_fd, (struct sockaddr*)&client_addr, &size);
        DBG("########### client comes in..........");

        if (conn_fd <= 0)
        {
            DBG("######### no client");
            close(server_fd);
            break;
        }
        else
        {
            g_hPowerGPS = conn_fd;
            pmtk_input_func(state);
        }
    }
    powergps_thread_exit(0, server_fd);//call pthread_exit() in this function, the code after this sentence will not be executed
    return NULL;
}

/*****************************************************************/
/*****************************************************************/
/*****                                                       *****/
/*****       N M E A   P A R S E R                           *****/
/*****                                                       *****/
/*****************************************************************/
/*****************************************************************/

static void
nmea_reader_update_utc_diff( NmeaReader* const r )
{
    time_t         now = time(NULL);
    struct tm      tm_local;
    struct tm      tm_utc;
    unsigned long  time_local, time_utc;

    gmtime_r( &now, &tm_utc );
    localtime_r( &now, &tm_local );

    time_local = mktime(&tm_local);
    time_utc = mktime(&tm_utc);
    r->utc_diff = time_utc - time_local;
}


static void
nmea_reader_init( NmeaReader* const r )
{
    memset( r, 0, sizeof(*r) );

    r->pos      = 0;
    r->overflow = 0;
    r->utc_year = -1;
    r->utc_mon  = -1;
    r->utc_day  = -1;
    r->utc_diff = 0;
    r->callbacks.location_cb= NULL;
    r->callbacks.status_cb= NULL;
    r->callbacks.gnss_sv_status_cb= NULL;
    r->sv_count = 0;
    r->fix_mode = 0;    /*no fix*/
    r->cb_status_changed = 0;
    memset((void*)&r->sv_status, 0x00, sizeof(r->sv_status));
    memset((void*)&r->in, 0x00, sizeof(r->in));

    nmea_reader_update_utc_diff( r );
    r->fix.flags |= GPS_LOCATION_HAS_ACCURACY;
    r->fix.accuracy = 10;
}

static void
nmea_reader_set_callback( NmeaReader* const r, GpsCallbacks* const cbs)
{
    if (!r) {           /*this should not happen*/
        return;
    } else if (!cbs) {  /*unregister the callback */
        return ;
    } else {/*register the callback*/
        r->sv_count = r->sv_status.num_svs = 0;
        r->fix.flags |= GPS_LOCATION_HAS_ACCURACY;
        r->fix.accuracy = 10;
    }
}


static int
nmea_reader_update_time( NmeaReader* const r, Token  tok ) {
    int        hour, minute;
    double     seconds;
    struct tm  tm;
    time_t     fix_time;

    if (tok.p + 6 > tok.end)
        return -1;

    memset((void*)&tm, 0x00, sizeof(tm));
    if (r->utc_year < 0) {
        // no date yet, get current one
        time_t  now = time(NULL);
        gmtime_r( &now, &tm );
        r->utc_year = tm.tm_year + 1900;
        r->utc_mon  = tm.tm_mon + 1;
        r->utc_day  = tm.tm_mday;
    }

    hour    = str2int(tok.p,   tok.p+2);
    minute  = str2int(tok.p+2, tok.p+4);
    seconds = str2float(tok.p+4, tok.end);

    tm.tm_hour = hour;
    tm.tm_min  = minute;
    tm.tm_sec  = (int) seconds;
    tm.tm_year = r->utc_year - 1900;
    tm.tm_mon  = r->utc_mon - 1;
    tm.tm_mday = r->utc_day;
    tm.tm_isdst = -1;

    if (mktime(&tm) == (time_t)-1)
        ERR("mktime error: %d %s\n", errno, strerror(errno));

    nmea_reader_update_utc_diff(r);

    fix_time = mktime(&tm) - r->utc_diff;
    DBG("fix_time: %d\n",fix_time);
    r->fix.timestamp = (long long)fix_time * 1000;
    return 0;
}

static int
nmea_reader_update_date( NmeaReader* const r, Token  date, Token  time )
{
    Token  tok = date;
    int    day, mon, year;

    if (tok.p + 6 != tok.end) {
        ERR("date not properly formatted: '%.*s'", tok.end-tok.p, tok.p);
        return -1;
    }
    day  = str2int(tok.p, tok.p+2);
    mon  = str2int(tok.p+2, tok.p+4);
    year = str2int(tok.p+4, tok.p+6) + 2000;

    if ((day|mon|year) < 0) {
        ERR("date not properly formatted: '%.*s'", tok.end-tok.p, tok.p);
        return -1;
    }

    r->utc_year  = year;
    r->utc_mon   = mon;
    r->utc_day   = day;

    return nmea_reader_update_time( r, time );
}


static double
convert_from_hhmm( Token  tok )
{
    double  val     = str2float(tok.p, tok.end);
    int     degrees = (int)(floor(val) / 100);
    double  minutes = val - degrees*100.;
    double  dcoord  = degrees + minutes / 60.0;
    return dcoord;
}


static int
nmea_reader_update_latlong( NmeaReader* const r,
                            Token        latitude,
                            char         latitudeHemi,
                            Token        longitude,
                            char         longitudeHemi ) {
    double   lat, lon;
    Token    tok;

    tok = latitude;
    if (tok.p + 6 > tok.end) {
        return -1;
    }
    lat = convert_from_hhmm(tok);
    if (latitudeHemi == 'S')
        lat = -lat;

    tok = longitude;
    if (tok.p + 6 > tok.end) {
        return -1;
    }
    lon = convert_from_hhmm(tok);
    if (longitudeHemi == 'W')
        lon = -lon;

    r->fix.flags    |= GPS_LOCATION_HAS_LAT_LONG;
    r->fix.latitude  = lat;
    r->fix.longitude = lon;
    return 0;
}

static int
nmea_reader_update_altitude( NmeaReader* const r,
                             Token        altitude,
                             Token        units ) {
    double  alt;
    Token   tok = altitude;

    if (tok.p >= tok.end)
        return -1;

    r->fix.flags   |= GPS_LOCATION_HAS_ALTITUDE;
    r->fix.altitude = str2float(tok.p, tok.end);
    return 0;
}


static int
nmea_reader_update_bearing( NmeaReader* const r,
                            Token        bearing )
{
    double  alt;
    Token   tok = bearing;

    if (tok.p >= tok.end)
        return -1;

    r->fix.flags   |= GPS_LOCATION_HAS_BEARING;
    r->fix.bearing  = str2float(tok.p, tok.end);
    return 0;
}


static int
nmea_reader_update_speed( NmeaReader* const r,
                          Token        speed )
{
    double  alt;
    Token   tok = speed;

    if (tok.p >= tok.end)
        return -1;

    r->fix.flags   |= GPS_LOCATION_HAS_SPEED;

    //Modify by ZQH to convert the speed unit from knot to m/s
    //r->fix.speed    = str2float(tok.p, tok.end);
    r->fix.speed = str2float(tok.p, tok.end) / 1.942795467;
    return 0;
}

//Add by LCH for accuracy
static int
nmea_reader_update_accuracy( NmeaReader* const r,
                             Token accuracy )
{
    double  alt;
    Token   tok = accuracy;

    if (tok.p >= tok.end)
        return -1;

    r->fix.flags   |= GPS_LOCATION_HAS_ACCURACY;
    r->fix.accuracy = str2float(tok.p, tok.end);
    return 0;
}

static int
nmea_reader_update_sv_status( NmeaReader* r, int sv_index,
                              int id, Token elevation,
                              Token azimuth, Token snr)
{
    //int prn = str2int(id.p, id.end);
    int prn = id;

    if ((prn <= 0) || (prn < 65 && prn > GPS_MAX_SVS)|| ((prn > 96) && (prn < 200)) || (prn > 232) || (r->sv_count >= GNSS_MAX_SVS)) {
        VER("sv_status: ignore (%d)", prn);
        return 0;
    }
    sv_index = r->sv_count+r->sv_status.num_svs;
    if(GNSS_MAX_SVS <= sv_index)
    {
        ERR("ERR: sv_index=[%d] is larger than GNSS_MAX_SVS.\n", sv_index);
        return 0;
    }

    if ((prn > 0) && (prn < 32)) {
        r->sv_status.gnss_sv_list[sv_index].svid = prn;
        r->sv_status.gnss_sv_list[sv_index].constellation = GPS_SV;
    } else if ((prn >= 65) && (prn <= 96)) {
        r->sv_status.gnss_sv_list[sv_index].svid = prn-64;
        r->sv_status.gnss_sv_list[sv_index].constellation = GLONASS_SV;
    } else if ((prn >= 201) && (prn <= 237)) {
        r->sv_status.gnss_sv_list[sv_index].svid = prn-200;
        r->sv_status.gnss_sv_list[sv_index].constellation = BDS_SV;
    } else if ((prn >= 401) && (prn <= 436)) {
        r->sv_status.gnss_sv_list[sv_index].svid = prn-400;
        r->sv_status.gnss_sv_list[sv_index].constellation = GALILEO_SV;
    } else {
        DBG("sv_status: ignore (%d)", prn);
        return 0;
    }

    r->sv_status.gnss_sv_list[sv_index].c_n0_dbhz = str2float(snr.p, snr.end);
    r->sv_status.gnss_sv_list[sv_index].elevation = str2int(elevation.p, elevation.end);
    r->sv_status.gnss_sv_list[sv_index].azimuth = str2int(azimuth.p, azimuth.end);
    if(1 == sv_used_in_fix[prn]){
        r->sv_status.gnss_sv_list[sv_index].flags |= 0x04;;
    }else{
        r->sv_status.gnss_sv_list[sv_index].flags &= 0xFB;
    }
    r->sv_count++;
    DBG("sv_status(%2d): %2d, %d, %2f, %3f, %2f, %d",
        sv_index, r->sv_status.gnss_sv_list[sv_index].svid, r->sv_status.gnss_sv_list[sv_index].constellation,
        r->sv_status.gnss_sv_list[sv_index].elevation, r->sv_status.gnss_sv_list[sv_index].azimuth,
        r->sv_status.gnss_sv_list[sv_index].c_n0_dbhz, r->sv_status.gnss_sv_list[sv_index].flags);
    return 0;
}

static int
nmea_reader_parse( NmeaReader* const r )
{
   /* we received a complete sentence, now parse it to generate
    * a new GPS fix...
    */
    NmeaTokenizer  tzer[1];
    Token          tok;
    Token          mtok;
    SV_TYPE sv_type = 0;

  /*
    if (r->pos < 9) {
        ERR("Too short. discarded. '%.*s'", r->pos, r->in);
        return;
    }
    */

    nmea_tokenizer_init(tzer, r->in, r->in + r->pos);

#if NEMA_DEBUG
    {
        int  n;
        DBG("Found %d tokens", tzer->count);
        for (n = 0; n < tzer->count; n++) {
            Token  tok = nmea_tokenizer_get(tzer,n);
            DBG("%2d: '%.*s'", n, tok.end-tok.p, tok.p);
        }
    }
#endif

    tok = nmea_tokenizer_get(tzer, 0);
    if (tok.p + 5 > tok.end) {
        ERR("sentence id '%.*s' too short, ignored.", tok.end-tok.p, tok.p);
        return -1;
    }

    // ignore first two characters.
    mtok.p = tok.p; //Mark the first two char for GPS,GLONASS,BDS SV parse.
    if(!memcmp(mtok.p, "BD", 2)){
        sv_type = BDS_SV;
        DBG("BDS SV type");
    } else if (!memcmp(mtok.p, "GP", 2)) {
        sv_type = GPS_SV;
    } else if (!memcmp(mtok.p, "GL", 2)) {
        sv_type = GLONASS_SV;
    } else if (!memcmp(mtok.p, "GA", 2)) {
        sv_type = GALILEO_SV;
    }
    #if NEMA_DEBUG
    LOGD("SV type: %d", sv_type);
    #endif
    tok.p += 2;
    if ( !memcmp(tok.p, "GGA", 3) )
    {
        if (tzer->count < 14)
            return 0;
        // GPS fix
        Token  tok_time          = nmea_tokenizer_get(tzer,1);
        Token  tok_latitude      = nmea_tokenizer_get(tzer,2);
        Token  tok_latitudeHemi  = nmea_tokenizer_get(tzer,3);
        Token  tok_longitude     = nmea_tokenizer_get(tzer,4);
        Token  tok_longitudeHemi = nmea_tokenizer_get(tzer,5);
        Token  tok_altitude      = nmea_tokenizer_get(tzer,9);
        Token  tok_altitudeUnits = nmea_tokenizer_get(tzer,10);

        nmea_reader_update_time(r, tok_time);
        nmea_reader_update_latlong(r, tok_latitude,
                                      tok_latitudeHemi.p[0],
                                      tok_longitude,
                                      tok_longitudeHemi.p[0]);
        nmea_reader_update_altitude(r, tok_altitude, tok_altitudeUnits);

    }
    else if ( !memcmp(mtok.p, "GPGSA", 5)||!memcmp(mtok.p, "BDGSA", 5)||!memcmp(mtok.p, "GLGSA", 5)) {
        if (tzer->count < 18)
            return 0;
        Token tok_fix = nmea_tokenizer_get(tzer, 2);
        int idx, max = 12; /*the number of satellites in GPGSA*/

        r->fix_mode = str2int(tok_fix.p, tok_fix.end);

        if (LOC_FIXED(r)) { /* 1: No fix; 2: 2D; 3: 3D*/
            for (idx = 0; idx < max; idx++) {
                Token tok_satellite = nmea_tokenizer_get(tzer, idx+3);
                if (tok_satellite.p == tok_satellite.end) {
                    break;
                }
                int sate_id = str2int(tok_satellite.p, tok_satellite.end);
                if (sv_type == BDS_SV){
                    sate_id += 200;
                }
                if (sate_id >= 1 && sate_id <= 32) {  //GP
                    sv_used_in_fix[sate_id] = 1;
                }else if(sate_id >= 193 && sate_id <= 197){
                    sv_used_in_fix[sate_id] = 0;
                    continue;
                }else if(sate_id >= 65 && sate_id <= 96){  //GL
                    sv_used_in_fix[sate_id] = 1;
                }else if(sate_id >= 201 && sate_id <= 232){  //BD
                    sv_used_in_fix[sate_id] = 1;

                }
                else{
                    break;
                }
            }
        }
    }
 //VER("GPGSA: mask 0x%x", r->sv_status.used_in_fix_mask);
    else if ( !memcmp(tok.p, "RMC", 3) ) {
        if (tzer->count < 13)
            return 0;
        Token  tok_time          = nmea_tokenizer_get(tzer,1);
        Token  tok_fixStatus     = nmea_tokenizer_get(tzer,2);
        Token  tok_latitude      = nmea_tokenizer_get(tzer,3);
        Token  tok_latitudeHemi  = nmea_tokenizer_get(tzer,4);
        Token  tok_longitude     = nmea_tokenizer_get(tzer,5);
        Token  tok_longitudeHemi = nmea_tokenizer_get(tzer,6);
        Token  tok_speed         = nmea_tokenizer_get(tzer,7);
        Token  tok_bearing       = nmea_tokenizer_get(tzer,8);
        Token  tok_date          = nmea_tokenizer_get(tzer,9);

        if (tok_fixStatus.p[0] == 'A') {
            nmea_reader_update_date( r, tok_date, tok_time );
            nmea_reader_update_latlong( r, tok_latitude,
                    tok_latitudeHemi.p[0],
                    tok_longitude,
                    tok_longitudeHemi.p[0] );
            nmea_reader_update_bearing( r, tok_bearing );
            nmea_reader_update_speed( r, tok_speed );
        }
    }
    else if ( !memcmp(tok.p, "GSV", 3) ) {
        if (tzer->count < 4)
            return 0;
        Token tok_num = nmea_tokenizer_get(tzer,1); //number of messages
        Token tok_seq = nmea_tokenizer_get(tzer,2); //sequence number
        Token tok_cnt = nmea_tokenizer_get(tzer,3); //Satellites in view
        int num = str2int(tok_num.p, tok_num.end);
        int seq = str2int(tok_seq.p, tok_seq.end);
        int cnt = str2int(tok_cnt.p, tok_cnt.end);
        int sv_base = (seq - 1)*NMEA_MAX_SV_INFO;
        int sv_num = cnt - sv_base;
        int idx, base = 4, base_idx;
        if (sv_num > NMEA_MAX_SV_INFO)
            sv_num = NMEA_MAX_SV_INFO;
        if (seq == 1)   /*if sequence number is 1, a new set of GSV will be parsed*/
            r->sv_count = 0;
        for (idx = 0; idx < sv_num; idx++) {
            base_idx = base*(idx+1);
            Token tok_id  = nmea_tokenizer_get(tzer, base_idx+0);
            int sv_id = str2int(tok_id.p, tok_id.end);
            if (sv_type == BDS_SV)
                sv_id += 200;
            else if (sv_type == GALILEO_SV)
                sv_id += 400;
            Token tok_ele = nmea_tokenizer_get(tzer, base_idx+1);
            Token tok_azi = nmea_tokenizer_get(tzer, base_idx+2);
            Token tok_snr = nmea_tokenizer_get(tzer, base_idx+3);
            nmea_reader_update_sv_status(r, sv_base+idx, sv_id, tok_ele, tok_azi, tok_snr);
        }
        if (seq == num) {
            if (r->sv_count <= cnt) {
                r->sv_status.num_svs += r->sv_count;
            }
            else {
                ERR("GPGSV incomplete (%d/%d), ignored!", r->sv_count, cnt);
                r->sv_count = r->sv_status.num_svs = 0;
            }
        }
    }
    //Add for Accuracy
    else if ( !memcmp(tok.p, "ACCURACY", 8)) {
        if((r->fix_mode == 3)||(r->fix_mode == 2)) {
            Token  tok_accuracy = nmea_tokenizer_get(tzer,1);
            nmea_reader_update_accuracy(r, tok_accuracy);
        }
    }
    else if ( !(memcmp(mtok.p, "GPBOD", 5)
              && memcmp(mtok.p, "GPBWC", 5)
              && memcmp(mtok.p, "GPGLL", 5)
              && memcmp(mtok.p, "GPHDT", 5)
              && memcmp(mtok.p, "GPROO", 5)
              && memcmp(mtok.p, "GPRMA", 5)
              && memcmp(mtok.p, "GPRMB", 5)
              && memcmp(mtok.p, "GPRTE", 5)
              && memcmp(mtok.p, "GPTRF", 5)
              && memcmp(mtok.p, "GPSTN", 5)
              && memcmp(mtok.p, "GPVBW", 5)
              && memcmp(mtok.p, "GPVTG", 5)
              && memcmp(mtok.p, "GPWPL", 5)
              && memcmp(mtok.p, "GPXTE", 5)
              && memcmp(mtok.p, "GPZDA", 5)))
    {
        tok.p -= 2;
    }
    else if ( !memcmp(mtok.p, "PMTK", 4))
    {
        tok.p -=2;
        DBG("PMTK is %s", r->in);
        if ( !memcmp(mtok.p, "PMTK010,002", 11) && g_EpoSend)
        {
            DBG("we got some START, ready to send EPO data");
            callback_backup.create_thread_cb(gps_send_epo_thread, mtk_gps_send_EPO, NULL);
            g_EpoSend = false;
        }
		/* disable QZSS start */
        else if ( !memcmp(mtok.p, "PMTK010,001", 11))
        {
            DBG("we got some START, ready to send 352 cmd");
			int temp_ret = 0;
            temp_ret = write(_gps_state->tty_fd, "$PMTK352,1*2B\r\n", 15);
			DBG("hao test : temp_ret is %d", temp_ret);
            //g_EpoSend = false;
        }
        else if ( !memcmp(mtok.p, "PMTK001,352,3", 13))
        {
            DBG("received 352,3 response");
        }
        /* disable QZSS end */
        return 0;
    }
    else {
        tok.p -= 2;
        return 0;
    }
    if (!LOC_FIXED(r)) {
        VER("Location is not fixed, ignored callback\n");
    }
    else if (r->fix.flags != 0 && gps_nmea_end_tag) {
#if NEMA_DEBUG
        char   temp[256];
        char*  p   = temp;
        char*  end = p + sizeof(temp);
        struct tm   utc;

        p += snprintf( p, end-p, "sending fix" );
        if (r->fix.flags & GPS_LOCATION_HAS_LAT_LONG) {
            p += snprintf(p, end-p, " lat=%g lon=%g", r->fix.latitude, r->fix.longitude);
        }
        if (r->fix.flags & GPS_LOCATION_HAS_ALTITUDE) {
            p += snprintf(p, end-p, " altitude=%g", r->fix.altitude);
        }
        if (r->fix.flags & GPS_LOCATION_HAS_SPEED) {
            p += snprintf(p, end-p, " speed=%g", r->fix.speed);
        }
        if (r->fix.flags & GPS_LOCATION_HAS_BEARING) {
            p += snprintf(p, end-p, " bearing=%g", r->fix.bearing);
        }
        if (r->fix.flags & GPS_LOCATION_HAS_ACCURACY) {
            p += snprintf(p,end-p, " accuracy=%g", r->fix.accuracy);
            DBG("GPS accuracy=%g\n", r->fix.accuracy);
        }
        gmtime_r( (time_t*) &r->fix.timestamp, &utc );
        p += snprintf(p, end-p, " time=%s", asctime( &utc ) );
        VER(temp);
#endif
        callback_backup.location_cb(&r->fix);
        {
            nlp_context fix_location;
            fix_location.latitude = r->fix.latitude;
            fix_location.longitude= r->fix.longitude;
            fix_location.accuracy= r->fix.accuracy;
            fix_location.type = 1;
            fix_location.started = started;
            fix_location.ts.tv_sec = r->fix.timestamp/1000;
            fix_location.ts.tv_nsec = (r->fix.timestamp%1000) * 1000*1000;
            mtk_gps_update_location(&fix_location);
        }
        r->fix.flags = GPS_LOCATION_HAS_ACCURACY;
    }

    DBG("r->sv_status.num_svs = %d, gps_nmea_end_tag = %d", r->sv_status.num_svs,gps_nmea_end_tag);
    if (r->sv_status.num_svs != 0 && gps_nmea_end_tag)
    {
        int idx;
        r->sv_status.size = sizeof(GnssSvStatus);
        callback_backup.gnss_sv_status_cb(&r->sv_status);
        r->sv_count = r->sv_status.num_svs = 0;
        memset( sv_used_in_fix, 0, 256*sizeof(int));
    }
    return 1;
}


static void
nmea_reader_addc( NmeaReader* const r, int  c )
{
    int ret = 0;
    if (r->overflow) {
        r->overflow = (c != '\n');
        return;
    }

    if (r->pos >= (int) sizeof(r->in)-1 ) {
        r->overflow = 1;
        r->pos      = 0;
        DBG("nmea sentence overflow\n");
        return;
    }

    r->in[r->pos] = (char)c;
    r->pos       += 1;
    //DBG(" r->in[%d] is %c, r->pos is %d", r->pos, r->in[r->pos], r->pos);
    if (c == '\n')
    {
        //r->pos < 9 :too short
        if (gps_nmea_end_tag == 1 && r->pos >= 9) {
            ret = nmea_reader_parse( r );
            if (ret == 1)
            {
                DBG("ready to send : %s, strlen is %d, sizeof is %d", r->in, strlen(r->in), sizeof(r->in));
                callback_backup.nmea_cb( r->fix.timestamp, r->in, r->pos );
                pmtk_output_func( r->in, &(r->pos));
            }
            if (ret == 0)
            {
                pmtk_output_func(r->in, &(r->pos));
            }
            r->pos = 0;
        }
        else {
            r->pos -=1;
        }
    }
}

static int
epoll_register( int  epoll_fd, int  fd )
{
    struct epoll_event  ev;
    int                 ret, flags;

    /* important: make the fd non-blocking */
    flags = fcntl(fd, F_GETFL);
    fcntl(fd, F_SETFL, flags | O_NONBLOCK);

    ev.events  = EPOLLIN;
    ev.data.fd = fd;
    do {
        ret = epoll_ctl( epoll_fd, EPOLL_CTL_ADD, fd, &ev );
    } while (ret < 0 && errno == EINTR);
    if(ret < 0)
        ERR("epoll ctl error, error num is %d\n, message is %s\n", errno, strerror(errno));
    return ret;
}


static int
epoll_deregister( int  epoll_fd, int  fd )
{
    int  ret;
    do {
        ret = epoll_ctl( epoll_fd, EPOLL_CTL_DEL, fd, NULL );
    } while (ret < 0 && errno == EINTR);
    return ret;
}

void
mtk_gps_start_driver()
{
    int fp = open("/dev/mt3337_gpsonly", O_RDWR);
    if (fp == -1) {
        DBG("open dev/mt3337_gpsonly fail, errorno is %d", errno);
    }
    else {
        ioctl(fp, GPS_POWER_IOCTL, 1);
        usleep(1000*200);
        ioctl(fp, GPS_POWER_IOCTL, 0);
        usleep(1000*200);
        ioctl(fp, GPS_POWER_IOCTL, 1);
    }
    close(fp);
}

void
mtk_gps_reset_driver()
{
    int fp = open("/dev/mt3337_gpsonly", O_RDWR);
    if (fp == -1) {
        DBG("open dev/mt3337_gpsonly fail, errorno is %d", errno);
    }
    else {
        ioctl(fp, GPS_POWER_IOCTL, 0);
        usleep(1000*1000);
        ioctl(fp, GPS_POWER_IOCTL, 1);
    }
    close(fp);
}

void
mtk_gps_stop_driver()
{
    int fp = open("/dev/mt3337_gpsonly", O_RDWR);
    if (fp == -1) {
        DBG("open dev/mt3337_gpsonly fail, errorno is %d", errno);
    }
    else {
        ioctl(fp, GPS_POWER_IOCTL, 0);
    }
    close(fp);
}

void
mnl_load_property()
{
    char result[512] = {0};
    int iEpoDisable = 0;
    int iHotstillEnable = 0;
    GpsState*  s = _gps_state;
    int ret = 0;
    if (property_get(MNL_CONFIG_STATUS, result, NULL))
    {
        iEpoDisable = result[7] - '0';
        if (iEpoDisable == 0)
        {
            ret = write(s->tty_fd, "$PMTK127*36\r\n", 13);
            DBG("we r going to clear EPO data, ret is %d", ret);
        }
        iHotstillEnable = result[4] - '0';
        if (iHotstillEnable == 1)
        {
            ret = write(s->tty_fd, "$PMTK869,1,1*35\r\n", 17);
            DBG("we r going to enable easy, ret is %d", ret);
        }
        else
        {
            ret = write(s->tty_fd, "$PMTK869,1,0*36\r\n", 17);
            DBG("we r going to disable easy, ret is %d", ret);
        }
    }
    else
    {
        DBG("YGPS config not set yet");
    }
}

unsigned char
calc_nmea_checksum( const char* sentence)
{
    unsigned char checksum = 0;
    while (*sentence)
    {
        checksum ^= (unsigned char)*sentence++;
    }

    return checksum;
}

//translate UTC to GPS_Hour
int
utc_to_gps_hour( int iYr, int iMo, int iDay, int iHr)
{
    int iYearsElapsed;
    int iDaysElapsed;
    int iLeapDays;
    int i;

    const unsigned short doy[12] = {0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334};
    iYearsElapsed = iYr - 1980;
    i = 0;
    iLeapDays = 0;

    while (i <= iYearsElapsed)
    {
        if ((i % 100) == 20)
        {
            if ((i % 400) == 20)
            {
                iLeapDays++;
            }
        }
        else if ((i % 4) == 0)
        {
            iLeapDays++;
        }
        i++;
    }
    if ((iYearsElapsed % 100) == 20)
    {
        if (((iYearsElapsed % 400) == 20) && (iMo <= 2))
        {
            iLeapDays--;
        }
    }
    else if (((iYearsElapsed % 4) == 0) && (iMo <= 2))
    {
        iLeapDays--;
    }
    iDaysElapsed = iYearsElapsed*365 + (int)doy[iMo - 1] + iDay + iLeapDays - 6;

    return (iDaysElapsed * 24 + iHr);
}

void
send_assistance_data (int iYr, int iMo, int iDay, int iHr, char * timeInfo, int iTimeLen)
{
    TRC();
    int fp, ret = 0;
    int i, segment, epo_gps_hour, current_gps_hour;
    unsigned epobuf[MTKEPO_RECORD_SIZE/sizeof(unsigned)];
    char strbuf[200], outbuf[200];
    GpsState*  s = _gps_state;
    // open EPO file and read the header (assume EPO file has passed integrity check)
    fp = open(EPO_UPDATE_HAL, O_RDONLY);
    if (fp < 0)
    {
        DBG("!!!!!!no EPO file ERROR: %d, %s", errno, strerror(errno));
        return;
    }
    ret = read(fp, &epo_gps_hour, sizeof(int));
    epo_gps_hour &= 0x00FFFFFF;

    current_gps_hour = utc_to_gps_hour(iYr, iMo, iDay, iHr);
    segment = (current_gps_hour - epo_gps_hour) / 6;
    if ((segment < 0) || (segment >= MTKEPO_SEGMENT_NUM))
    {
        DBG("!!!!!segment ERROR : %d", segment);
        return;
    }
    // read binary EPO data and sent it to MT3339
    lseek(fp, segment*(MTKEPO_RECORD_SIZE)*(MTKEPO_SV_NUMBER), SEEK_SET);
    DBG("ready to write time info before send EPO data");
    ret = write(s->tty_fd, timeInfo, iTimeLen);
    usleep(1000*200);
    for (i = 0; i < MTKEPO_SV_NUMBER; i++)
    {
        read(fp, epobuf, MTKEPO_RECORD_SIZE);
        // assume host system is little-endian
        sprintf(strbuf,"PMTK721,%X,%X,%X,%X,%X,%X,%X,%X,%X,%X,%X,%X,%X,%X,%X,%X,%X,%X,%X",
        i+1,
        epobuf[0], epobuf[1], epobuf[2], epobuf[3], epobuf[4], epobuf[5],
        epobuf[6], epobuf[7], epobuf[8], epobuf[9], epobuf[10], epobuf[11],
        epobuf[12], epobuf[13], epobuf[14], epobuf[15], epobuf[16], epobuf[17]);
        sprintf(outbuf, "$%s*%02X\r\n", strbuf, calc_nmea_checksum(strbuf));
        // send them by UART
        DBG("!!!!this EPO data len is %d,data is : %s", strlen(outbuf), outbuf); // for demo
        for (int j = 0; j < 3; j++)
        {
            ret = write(s->tty_fd, outbuf, strlen(outbuf));
            if (ret < 0)
                DBG("Write EPO data fail, errno is %d", errno);
            else
                j = 3;
        }
        if (i%10 == 0)
            usleep(1000*200);
    }
    close(fp);
}

/*for reducing the function call to get data from kernel*/
static char buffGGA[2048];
/*gps nmea read thread */
static bool g_NmeaReadStart = false;
static bool g_ThreadExistNmeaRead = false;
static bool g_GpsDriverStart = false;
static pthread_mutex_t g_mutx;

void
gps_read_nmea_from_uart_thread(void* arg)
{
    g_ThreadExistNmeaRead = true;
    GpsState*   state = (GpsState*) arg;
    int fd = state->tty_fd;
    while (g_ThreadExistNmeaRead)
    {
        if(g_GpsDriverStart && g_NmeaReadStart)
        {
            int  nn, ret;
            memset(buffGGA, 0, sizeof(buffGGA));
            ret = read( fd, buffGGA, sizeof(buffGGA) );
            gps_nmea_end_tag = 0;

            DBG("before PARSE buffer[%d] is:%s.", ret, buffGGA);
            memset(g_NmeaReader->in, 0, sizeof(g_NmeaReader->in));
            for (nn = 0; nn < ret && g_GpsDriverStart; nn++)
            {
                if(nn == (ret-1))
                    gps_nmea_end_tag = 1;

                nmea_reader_addc( g_NmeaReader, buffGGA[nn] );
            }
            g_NmeaReadStart = false;
        }
        usleep(100);
    }
    DBG("gps_read_nmea_from_uart_thread exit!");
}
/* this is the main thread, it waits for commands from gps_state_start/stop and,
 * when started, messages from the GPS daemon. these are simple NMEA sentences
 * that must be parsed to be converted into GPS fixes sent to the framework
 */
void
gps_state_thread( void*  arg )
{
    static float count = 0;
    GpsState*   state = (GpsState*) arg;
    state->test_time += 1;
    int         tty_fd     = state->tty_fd;
    int         control_fd = state->control[1];
    int         pow_fd     = state->pow_fd;

    int epoll_fd = state->epoll_hd;
    int         test_started = 0;

    nmea_reader_init( g_NmeaReader );

    // register control file descriptors for polling
    if(epoll_register( epoll_fd, control_fd ) < 0)
        ERR("control_fd register control_fd error, error num is %d\n, message is %s\n", errno, strerror(errno));
    if(epoll_register( epoll_fd, tty_fd) < 0)
        ERR("tty_fd register control_fd error, error num is %d\n, message is %s\n", errno, strerror(errno));
    //if(epoll_register( epoll_fd, pow_fd) < 0)
    if(epoll_add_fd( epoll_fd, pow_fd) < 0)
        ERR("pow_fd register control_fd error, error num is %d\n, message is %s\n", errno, strerror(errno));

    release_condition(&lock_for_sync[M_INIT]);

    // now loop
    for (;;)
    {
        struct epoll_event   events[5];
        int                  ne, nevents;
        nevents = epoll_wait( epoll_fd, events, 5, -1 );
        if (nevents < 0)
        {
            if (errno != EINTR)
                ERR("epoll_wait() unexpected error: %s", strerror(errno));
            continue;
        }

        for (ne = 0; ne < nevents; ne++)
        {
            if ((events[ne].events & (EPOLLERR|EPOLLHUP)) != 0)
            {
                ERR("EPOLLERR or EPOLLHUP after epoll_wait() !?");
                goto Exit;
            }
            if ((events[ne].events & EPOLLIN) != 0)
            {
                int  fd = events[ne].data.fd;

                if (fd == pow_fd)
                {
                    ERR("!!!!!!!!!!!EPO donwload done");
                    char  cmd = 255;
                    int   ret;

                    //socket
                    char epo_buff[100] = {0};
                    int offset = 0;
                    int read_len;

                    read_len = safe_recvfrom(fd, epo_buff, sizeof(epo_buff));
                    if (read_len <= 0) {
                        DBG("@@@@@@@@@gpsinf3337() safe_recvfrom() failed read_len=%d", read_len);
                        break;
                    }
                    cmd = get_int(epo_buff, &offset, sizeof(epo_buff));

                    if (cmd == CMD_EPO_DOWNLOAD_DONE) {
                        DBG("@@@@@@@@@@@@got the EPO download msg");
                        callback_backup.create_thread_cb(gps_send_epo_thread, mtk_gps_send_EPO, NULL);
                    }
                }
                else if (fd == control_fd)
                {
                    char  cmd = 255;
                    int   ret;
                    do {
                        ret = read( fd, &cmd, 1 );
                    } while (ret < 0 && errno == EINTR);

                    if (cmd == CMD_QUIT) {
                        DBG("gps thread quitting on demand");
                        goto Exit;
                    }
                    else if (cmd == CMD_START) {
                        if (!started) {
                            started = 1;
                            nmea_reader_set_callback( g_NmeaReader, &state->callbacks);
                            switch (g_delete_aiding_data_flag) {
                                case 1:
                                    DBG("Last time's delete_aiding : hot start");
                                    ret = write(tty_fd, "$PMTK101*32\r\n", 13);
                                    g_EpoSend = true;
                                    break;
                                case 2:
                                    DBG("Last time's delete_aiding : warm start");
                                    ret = write(tty_fd, "$PMTK102*31\r\n", 13);
                                    g_EpoSend = true;
                                    break;
                                case 3:
                                    DBG("Last time's delete_aiding : cold start");
                                    ret = write(tty_fd, "$PMTK103*30\r\n", 13);
                                    g_EpoSend = true;
                                    break;
                                case 4:
                                    DBG("Last time's delete_aiding : full start");
                                    ret = write(tty_fd, "$PMTK104*37\r\n", 13);
                                    g_EpoSend = true;
                                    break;
                                default:
                                    break;
                            }
                            g_delete_aiding_data_flag = 0;
                        }
                    }
                    else if (cmd == CMD_STOP) {
                        if (started) {
                            started = 0;
                            nmea_reader_set_callback( g_NmeaReader, NULL );
                            release_condition(&lock_for_sync[M_STOP]);
                            DBG("CMD_STOP has been receiving from HAL thread, release lock so can handle CLEAN_UP\n");
                        }
                    }
                    else if (cmd == CMD_RESTART) {
                        g_NmeaReader->fix_mode = 0;
                        DBG("gps 3337 restart!!!!");
                    }
                }
                else if (fd == tty_fd)
                {
                    if(!flag_unlock)
                    {
                        release_condition(&lock_for_sync[M_START]);
                        flag_unlock = 1;
                        DBG("got first NMEA sentence, release lock to set state ENGINE ON, SESSION BEGIN");
                    }
                    if(report_time_interval > ++count){
                        DBG("[trace]count is %f\n", count);
                        //int ret = read( fd, buff, sizeof(buff) );
                        continue;
                    }
                    count = 0;
                    g_NmeaReadStart = true;
                }
            }
        }
    }
Exit:
    DBG("HAL thread is exiting, release lock to clean resources\n");
    release_condition(&lock_for_sync[M_CLEANUP]);
    return;
}
static void
gps_state_done( GpsState*  s )
{
    TRC();
    char   cmd = CMD_QUIT;

    write( s->control[0], &cmd, 1 );
    get_condition(&lock_for_sync[M_CLEANUP]);

    g_ThreadExitPMTKIn = false;
    g_hPowerGPS = INVALID_FD;
    pthread_join(s->pmtk_thread->thread, NULL);
    g_ThreadExistNmeaRead = false;
    pthread_join(s->nmea_read_thread, NULL);

    close( s->control[0] ); s->control[0] = -1;
    close( s->control[1] ); s->control[1] = -1;
    close( s->pow_fd ); s->pow_fd = -1;
    close( s->tty_fd ); s->tty_fd = -1;
    close(s->epoll_hd); s->epoll_hd = -1;
    s->init = 0;
    s->test_time -= 1;
    DBG("GPS cleanup done");

    return;
}


static void
gps_state_start( GpsState*  s )
{
    char  cmd = CMD_START;
    int   ret;

    do { ret=write( s->control[0], &cmd, 1 ); }
    while (ret < 0 && errno == EINTR);

    if (ret != 1)
        ERR("%s: could not send CMD_START command: ret=%d: %s",
          __FUNCTION__, ret, strerror(errno));
}

static void
gps_state_stop( GpsState*  s )
{
    char  cmd = CMD_STOP;
    int   ret;

    do { ret=write( s->control[0], &cmd, 1 ); }
    while (ret < 0 && errno == EINTR);

    if (ret != 1)
        ERR("%s: could not send CMD_STOP command: ret=%d: %s",
          __FUNCTION__, ret, strerror(errno));
}

static void
gps_state_restart( GpsState*  s )
{
    char  cmd = CMD_RESTART;
    int   ret;

    do { ret=write( s->control[0], &cmd, 1 ); }
    while (ret < 0 && errno == EINTR);

    if (ret != 1)
        ERR("%s: could not send CMD_RESTART command: ret=%d: %s",
          __FUNCTION__, ret, strerror(errno));
}


static void
gps_state_init( GpsState*  state )
{
    state->control[0] = -1;
    state->control[1] = -1;
    state->pow_fd     = -1;
    state->tty_fd     = -1;

    TRC();

    //socket
    state->pow_fd = socket_bind_udp(MNLD_HAL2GPS);
    socket_set_blocking(state->pow_fd, 0);
    if (state->pow_fd < 0) {
        ERR("no gps hardware detected: %s:%d, %s", MNLD_HAL2GPS, state->pow_fd, strerror(errno));
        return;
    }

    state->tty_fd = open(GPS_TTY_NAME, O_RDWR | O_NOCTTY | O_NONBLOCK);
    if (state->tty_fd < 0) {
        close(state->pow_fd);
        state->pow_fd = -1;
        ERR("no gps hardware detected: %s:%d, %s", GPS_TTY_NAME, state->tty_fd, strerror(errno));
        return;
    }
    struct termios options;
    int i4_baud = B115200;
    tcgetattr(state->tty_fd, &options);
    cfsetispeed(&options, i4_baud);
    cfsetospeed(&options, i4_baud);

    options.c_cflag |= (CLOCAL | CREAD);
    options.c_cflag &= ~PARENB;
    options.c_cflag &= ~CSTOPB;
    options.c_cflag &= ~CSIZE;
    options.c_cflag &= ~PARODD;
    options.c_cflag |= CS8;
    options.c_cflag &= ~CRTSCTS;
    options.c_lflag &= ~(ISIG | ECHO);
    options.c_oflag &= ~OPOST;
    options.c_iflag &= ~(INLCR | INPCK | ISTRIP | IXON | BRKINT );
    options.c_cc[VMIN] = 1;
    options.c_cc[VTIME] = 0;
    tcsetattr(state->tty_fd, TCSANOW, &options);

    int epoll_fd = epoll_create(3);
    state->epoll_hd = epoll_fd;

    if ( socketpair( AF_LOCAL, SOCK_STREAM, 0, state->control ) < 0 ) {
        ERR("could not create thread control socket pair: %s", strerror(errno));
        goto Fail;
    }
    state->thread = callback_backup.create_thread_cb(gps_native_thread, gps_state_thread, state);
    if (!state->thread){
        ERR("could not create gps thread: %s", strerror(errno));
        goto Fail;
    }

    DBG("gps state initialized, the thread is %d\n", (int)state->thread);
    if (!g_ThreadExitPMTKIn)
    {
    state->pmtk_thread->thread= callback_backup.create_thread_cb(gps_pmtk_thread, powergps_pmtk_receive_thread, state);
    if (!state->pmtk_thread->thread){
        ERR("could not create gps pmtk thread: %s", strerror(errno));
        goto Fail;
        }
    }

    if (!g_ThreadExistNmeaRead)
    {
        state->nmea_read_thread = callback_backup.create_thread_cb(gps_nmea_read_thread, gps_read_nmea_from_uart_thread, state);
        if (!state->nmea_read_thread){
            ERR("could not create nmea read thread: %s", strerror(errno));
            goto Fail;
        }
    }

    DBG("gps PMTK initialized, the thread is %d\n", (int)state->pmtk_thread->thread);

    return;

Fail:
    gps_state_done( state );
}


/*****************************************************************/
/*****************************************************************/
/*****                                                       *****/
/*****       I N T E R F A C E                               *****/
/*****                                                       *****/
/*****************************************************************/
/*****************************************************************/
static int
mtk_gps_mt3337_init(GpsCallbacks* callbacks)
{
    TRC();
    GpsState*  s = _gps_state;
    int get_time = 20;
    int res = 0;
    if(s->init)
        return 0;
    s->callbacks = *callbacks;
    callback_backup = *callbacks;

    gps_state_init(s);
    get_condition(&lock_for_sync[M_INIT]);
    usleep(1000*1);
    s->init = 1;
    DBG("Set GPS_CAPABILITY_SCHEDULING \n");
    callback_backup.set_capabilities_cb(GPS_CAPABILITY_SCHEDULING);

    if (gpshal_gpscbs_save(callbacks) != 0) {
        return -1;    //  error
    }

    gpshal_set_gps_state_intent(GPSHAL_STATE_INIT);
    gpshal2mnl_gps_init();
    return 0;
}

static void
mtk_gps_mt3337_cleanup(void)
{
    GpsState*  s = _gps_state;

    TRC();

    if(s->start_flag)
    get_condition(&lock_for_sync[M_STOP]); //make sure gps_stop has set state to GPS_STATUS_ENGINE_OFF by callback function
    if(lock_for_sync[M_STOP].condtion == 1)
    {
        lock_for_sync[M_STOP].condtion = 0; //make sure gps_stop has set state to GPS_STATUS_ENGINE_OFF in next time
    }
    if (s->init)
        gps_state_done(s);
    gpshal_set_gps_state_intent(GPSHAL_STATE_CLEANUP);
    gpshal2mnl_gps_cleanup();
    DBG("mtk_gps_cleanup done");
}

int
mtk_gps_mt3337_start()
{
    TRC();
    GpsState*  s = _gps_state;
    int err;
    int count=0;


    if (!s->init) {
        ERR("%s: called with uninitialized state !!", __FUNCTION__);
        return -1;
    }

    while(s->start_flag == 1)
    {
        usleep(100000);
        count++;
        DBG("mtk_gps_start:start_flag = %d delay=%d \n",s->start_flag,count*100);
    }
    mtk_gps_start_driver();
    g_GpsDriverStart = true;
    DBG("mtk_gps_mt3337_start done!!!!");
    mnl_load_property();

    get_condition(&lock_for_sync[M_START]);
    gps_state_start(s);

    sta.status = GPS_STATUS_ENGINE_ON;
    callback_backup.status_cb(&sta);
    sta.status = GPS_STATUS_SESSION_BEGIN;
    callback_backup.status_cb(&sta);

    callback_backup.acquire_wakelock_cb();//avoid cpu to sleep
    s->start_flag = 1;

    gpshal_set_gps_state_intent(GPSHAL_STATE_START);
    gpshal2mnl_gps_start();
    return 0;
}

int
mtk_gps_mt3337_stop()
{
    TRC();
    GpsState*  s = _gps_state;
    int err;
    int count=0;

    if (!s->init) {
        ERR("%s: called with uninitialized state !!", __FUNCTION__);
        return -1;
    }

    while(s->start_flag == 0)
    {
        usleep(100000);
        count++;
        DBG("mtk_gps_stop:start_flag = %d delay=%d \n",s->start_flag,count*100);
    }
    s->start_flag = 0;

    mtk_gps_stop_driver();
    g_GpsDriverStart = false;
    flag_unlock = 0;
    gps_state_stop(s);
    gpshal_set_gps_state_intent(GPSHAL_STATE_STOP);
    gpshal2mnl_gps_stop();

    callback_backup.release_wakelock_cb();
    DBG("mtk_gps_mt3337_STOP done!!!!");
    return 0;
}

static int
mtk_gps_mt3337_inject_time(GpsUtcTime time, int64_t timeReference, int uncertainty)
{
    ntp_context inject_ntp;
    long long time_s = 0;
    int offset = 0;

    TRC();
    time_s = time/1000;
    DBG("inject time= %lld,ctime = %s, timeReference = %lld,uncertainty =%d\n", time,ctime(&time_s), timeReference,uncertainty);
    memcpy(&(inject_ntp.time),&time,sizeof(GpsUtcTime));
    inject_ntp.timeReference = timeReference;
    inject_ntp.uncertainty = uncertainty;

    //do not support detail function
    return 0;
}

static int
mtk_gps_mt3337_inject_location(double latitude, double longitude, float accuracy)
{
    nlp_context nlp_location;
    int offset = 0;

    if(clock_gettime(CLOCK_MONOTONIC , &nlp_location.ts) == -1)
    {
        ERR("clock_gettime failed reason=[%s]\n",strerror(errno));
        return -1;
    }
    DBG("ts.tv_sec= %lld,ts.tv_nsec = %lld\n", nlp_location.ts.tv_sec,nlp_location.ts.tv_nsec);
    DBG("inject location lati= %f, longi = %f,accuracy =%f\n", latitude, longitude,accuracy);

    //do not support detail function
    return 0;
}

static void
mtk_gps_mt3337_delete_aiding_data(GpsAidingData flags)
{
    /*workaround to trigger hot/warm/cold/full start*/
    #define FLAG_HOT_START  GPS_DELETE_RTI
    #define FLAG_WARM_START GPS_DELETE_EPHEMERIS
    #define FLAG_COLD_START (GPS_DELETE_EPHEMERIS | GPS_DELETE_POSITION | GPS_DELETE_TIME | GPS_DELETE_IONO | GPS_DELETE_UTC | GPS_DELETE_HEALTH)
    #define FLAG_FULL_START (GPS_DELETE_ALL)
    #define FLAG_AGPS_START (GPS_DELETE_EPHEMERIS | GPS_DELETE_ALMANAC | GPS_DELETE_POSITION | GPS_DELETE_TIME | GPS_DELETE_IONO | GPS_DELETE_UTC)
    GpsState*  s = _gps_state;
    int ret;

    DBG("%s:0x%X\n", __FUNCTION__, flags);

    gps_state_restart(s);

    if (flags == FLAG_HOT_START)
    {
        mtk_restart(MNL_CMD_RESTART_HOT);
        if (g_GpsDriverStart)
        {
            DBG("GPS driver still start!!! send it HOT_START!!!");
            ret = write(s->tty_fd, "$PMTK101*32\r\n", 13);
            if (ret < 0)
                DBG("HOT_START FAILED!!! error is %d : %s", errno, strerror(errno));
            g_EpoSend = true;
        }
        else
        {
            g_delete_aiding_data_flag = 1;
        }
    }
    else if (flags == FLAG_WARM_START)
    {
        mtk_restart(MNL_CMD_RESTART_WARM);
        if (g_GpsDriverStart)
        {
            DBG("GPS driver still start!!! send it WARM_START!!!");
            ret = write(s->tty_fd, "$PMTK102*31\r\n", 13);
            if (ret < 0)
                DBG("WARM_START FAILED!!! error is %d : %s", errno, strerror(errno));
            g_EpoSend = true;
        }
        else
        {
            g_delete_aiding_data_flag = 2;
        }
    }
    else if (flags == FLAG_COLD_START)
    {
        mtk_restart(MNL_CMD_RESTART_HOT);
        if (g_GpsDriverStart)
        {
            DBG("GPS driver still start!!! send it COLD_START!!!");
            ret = write(s->tty_fd, "$PMTK103*30\r\n", 13);
            if (ret < 0)
                DBG("COLD_START FAILED!!! error is %d : %s", errno, strerror(errno));
            g_EpoSend = true;
        }
        else
        {
            g_delete_aiding_data_flag = 3;
        }
    }
    else if (flags == FLAG_FULL_START)
    {
        mtk_restart(MNL_CMD_RESTART_HOT);
        if (g_GpsDriverStart)
        {
            DBG("GPS driver still start!!! send it COLD_START!!!");
            ret = write(s->tty_fd, "$PMTK104*37\r\n", 13);
            if (ret < 0)
                DBG("FULL_START FAILED!!! error is %d : %s", errno, strerror(errno));
            g_EpoSend = true;
        }
        else
        {
            g_delete_aiding_data_flag = 4;
        }
    }
    else if(flags == FLAG_AGPS_START)
    {
        DBG("Send MNL_CMD_RESTART_AGPS in HAL\n");
        mtk_restart(MNL_CMD_RESTART_AGPS);
    }
}

static int mtk_gps_mt3337_set_position_mode(GpsPositionMode mode, GpsPositionRecurrence recurrence,
            uint32_t min_interval, uint32_t preferred_accuracy, uint32_t preferred_time)
{
    // FIXME - support fix_frequency
    // only standalone supported for now.
    DBG("set report location time interval is %d\n", min_interval);
    if(min_interval <= 1000)
        report_time_interval = 1;
    else
        report_time_interval = (float)min_interval/1000;
    DBG("set report location time interval is %f s\n", report_time_interval);
    TRC();
    return 0;
}

static const void*
mtk_gps_mt3337_get_extension(const char* name)
{
    TRC();
    DBG("mtk_gps_get_extension name=[%s]\n", name);
    if (strcmp(name, AGPS_RIL_INTERFACE) == 0) {
        return &mtk_agps_ril_inf;
    }
/*
#if EPO_SUPPORT
    if (!strcmp(name, GPS_XTRA_INTERFACE))
        return (void*)(&mtkGpsXtraInterface);
#endif
*/
    return NULL;
}

static const GpsInterface  mtkGpsInterface = {
    sizeof(GpsInterface),
    mtk_gps_mt3337_init,
    mtk_gps_mt3337_start,
    mtk_gps_mt3337_stop,
    mtk_gps_mt3337_cleanup,
    mtk_gps_mt3337_inject_time,
    mtk_gps_mt3337_inject_location,
    mtk_gps_mt3337_delete_aiding_data,
    mtk_gps_mt3337_set_position_mode,
    mtk_gps_mt3337_get_extension,
};

//=========================================================
// Between
//     Gps Interface
//     Hardware Module Interface

static const GpsInterface* gps_device__get_gps_interface(
        __unused struct gps_device_t* device) {
    /*
     * hao mt3337 modify
     * GPS_DEVICE__GET_GPS_INTERFACE__CHECK_PARAM;
     * hal2mnl_hal_reboot();
     */
    return &mtkGpsInterface;
}

static const struct gps_device_t gps_device = {
    .common = {                           // hw_device_t
        .tag     = HARDWARE_DEVICE_TAG,
        .version = 0,                     // GPS JNI will not use it
        .module  = &HAL_MODULE_INFO_SYM,
        .reserved = {0},
        .close   = NULL                   // GPS JNI will not call it
        },
    .get_gps_interface = gps_device__get_gps_interface
};


static int open_gps(const struct hw_module_t* module, char const* name,
        struct hw_device_t** device)
{
    DBG("open_gps HAL 1\n");
    *device = (struct hw_device_t*)&gps_device;
    return 0;
}


static struct hw_module_methods_t gps_module_methods = {
    .open = open_gps
};


struct hw_module_t HAL_MODULE_INFO_SYM = {
    .tag = HARDWARE_MODULE_TAG,
    .version_major = 1,
    .version_minor = 0,
    .id  = GPS_HARDWARE_MODULE_ID,
    .name   = "MediaTek GPS Hardware Module",
    .author = "MediaTek, Inc.",
    .methods = &gps_module_methods,
    .dso     = NULL,
    .reserved = {0}
};
