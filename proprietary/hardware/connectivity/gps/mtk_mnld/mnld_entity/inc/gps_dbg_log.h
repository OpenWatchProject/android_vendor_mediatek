#ifndef __GPS_DBG_LOG_H__
#define __GPS_DBG_LOG_H__

#include "mtk_gps_type.h"
#include "mtk_socket_utils.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifndef MAX
#define MAX(A,B) ((A)>(B)?(A):(B))
#endif
#ifndef MIN
#define MIN(A,B) ((A)<(B)?(A):(B))
#endif

#define LOG_FILE            "/data/vendor/gps/gpsdebug.log"
#define LOG_FILE_PATH       "/data/vendor/gps/"
#define PATH_SUFFIX         "debuglogger/connsyslog/gpshost/"
#define LOG_FILE_EXTEN_NAME ".nma"
#define LOG_FILE_WRITING_EXTEN_NAME ".nma.curf"
#define GPS_LOG_PERSIST_STATE "vendor.gpsdbglog.enable"
#define GPS_LOG_PERSIST_PATH "vendor.gpsdbglog.path"
#define GPS_LOG_PERSIST_FLNM "vendor.gpsdbglog.file"

#define GPS_LOG_PERSIST_VALUE_ENABLE "1"
#define GPS_LOG_PERSIST_VALUE_DISABLE "0"
#define GPS_LOG_PERSIST_VALUE_NONE "none"

#define GPS_DBG_LOG_FILE_NUM_LIMIT 1000
#define MAX_DBG_LOG_FILE_SIZE      MIN(48*1024*1024,(g_dbglog_file_size_in_config<1024*1024?25*1024*1024:g_dbglog_file_size_in_config))
#define MAX_DBG_LOG_DIR_SIZE       MIN(512*1024*1024,MAX(g_dbglog_folder_size_in_config,MAX_DBG_LOG_FILE_SIZE*12))

#define MNLD2MTKLOGGER_ANS_BUFF_SIZE   256
#define MTKLOGGER2MNLD_READ_BUFF_SIZE  253

enum {
    MTK_GPS_DISABLE_DEBUG_MSG_WR_BY_MNL     = 0x00,
    MTK_GPS_ENABLE_DEBUG_MSG_WR_BY_MNL      = 0x01,
    MTK_GPS_DISABLE_DEBUG_MSG_WR_BY_MNLD    = 0x10,
    MTK_GPS_ENABLE_DEBUG_MSG_WR_BY_MNLD     = 0x11,
    MTK_GPS_ENCRYPT_DEBUG_MSG_BY_MNL        = 0x20, //Valid only when MTK_GPS_ENABLE_DEBUG_MSG_WR_BY_MNLD set
};
// Note: the bellow gps_dbg_log_state* APIs are defined for two purpose:
// 1. Use mutex to protect set/clear bitmask operations of gps_debuglog_state
//      (note: currently read gps_debuglog_state is thread-safe)
//
// 2. The MTK_GPS_*_BY_MNL* definition has legacy and confusing meaning,
//      so we wrapper it in the APIs for easy usage
//
// So, it's not recommand to direclty manipuate gps_debuglog_state in new code
void gps_dbg_log_state_init();
void gps_dbg_log_state_set_bitmask(unsigned int bitmask);
void gps_dbg_log_state_set_output_enable();
void gps_dbg_log_state_set_output_disable();

// Note: currently the bit 0x10 in gps_debuglog_state is always true,
// then "!is_output_enabled" should equal "is_output_disabled".
// However, we keep the two different APIs for "0x10 not true"(legacy) case for compablity.
bool gps_dbg_log_state_is_output_enabled();  //Just use it for new code
bool gps_dbg_log_state_is_output_disabled(); //For legacy, not recommnad for new

void gps_dbg_log_state_set_encrypt_enable();
void gps_dbg_log_state_set_encrypt_disable();
bool gps_dbg_log_state_is_encrypt_enabled();
bool mnld2logd_open_gpslog(char* file);
bool mnld2logd_write_gpslog(char * data, int len);
bool mnld2logd_close_gpslog(void);

bool mnld2logd_open_mpelog(char* file);
bool mnld2logd_write_mpelog(char * data, int len);
bool mnld2logd_close_mpelog(void);

bool mnld2logd_open_dumplog(char* file);
bool mnld2logd_write_dumplog(char * data, int len);
bool mnld2logd_close_dumplog(void);

// Task synchronization related type
typedef enum{
  MNLD_MUTEX_PINGPANG_WRITE = 0,
  MNLD_MUTEX_GPS_DBG_LOG_STATE = 1,
  MNLD_MUTEX_MAX
} mnld_mutex_enum;

int gps_dbg_log_thread_init();

int create_mtklogger2mnl_fd();

int mtklogger2mnl_hdlr(int fd, mtk_socket_fd* client_fd);

INT32 gps_log_dir_check(char * dirname);

void gps_stop_dbglog_release_condition(void);

void mtklogger_mped_reboot_message_update(void);

//Rename gpsdebug name, .nmac to .nma
void gps_log_file_rename(char *filename_cur);

void gps_dbg_log_property_load(void);

void gps_dbg_log_exit_flush(int force_exit) ;


typedef enum
{
    LV_DEBUG,
    LV_VERBOSE,
    LV_INFO,
    LV_WARN,
    LV_ERROR,
}
mnld_log_level_t;

extern void mnld_log_printx(mnld_log_level_t log_lv, int skip_chars, char *fmt, ...);

//#define LOGD(fmt, arg ...) ALOGD("%s: " fmt, __FUNCTION__ , ##arg)
// Put XXLOGX as the leading mark so that our decryptor can find it easily, skip the 1st X char
#define LOGDX(fmt, arg ...) mnld_log_printx(LV_DEBUG,   1, "XXLOGX %s: " fmt, __FUNCTION__ , ##arg)
#define LOGVX(fmt, arg ...) mnld_log_printx(LV_VERBOSE, 1, "XXLOGX %s: " fmt, __FUNCTION__ , ##arg)
#define LOGIX(fmt, arg ...) mnld_log_printx(LV_INFO,    1, "XXLOGX %s: " fmt, __FUNCTION__ , ##arg)
#define LOGWX(fmt, arg ...) mnld_log_printx(LV_WARN,    1, "XXLOGX %s: " fmt, __FUNCTION__ , ##arg)
#define LOGEX(fmt, arg ...) mnld_log_printx(LV_ERROR,   1, "XXLOGX %s: " fmt, __FUNCTION__ , ##arg)

#ifdef __cplusplus
}
#endif

#endif



