#ifndef __MNL_FLP_MAIN_H__
#define __MNL_FLP_MAIN_H__

#include <stdint.h>
#include <stdbool.h>
#include <time.h>
#include <sys/socket.h>
#include <hardware/fused_location.h>
#include "mtk_gps_type.h"

#ifdef __cplusplus
extern "C" {
#endif

#define FLP_MNL_BUFF_SIZE           (1 * 1024)
#define FLP_MNL_INTERFACE_VERSION   1
#define OFFLOAD_PAYLOAD_LEN (600)
#define OFL_VER "OFL_VER_2016.11.01-01"

//======================================================
// FLP -> FLP HAL
//======================================================
#define FLP_TO_MNL "flp_to_mnl"
#define MNL_TO_FLP "mnl_to_flp"

#define FLP_MNL_PAYLOAD_LEN 128
#define MNLD_FLP_CAPABILITY_AP_MODE      1
#define MNLD_FLP_CAPABILITY_OFL_MODE     2
#define MNLD_FLP_CAPABILITY_AP_OFL_MODE  3

//FLP source type
#define FLP_TECH_MASK_GNSS      (1U<<0)
#define FLP_TECH_MASK_WIFI      (1U<<1)
#define FLP_TECH_MASK_SENSORS   (1U<<2)
#define FLP_TECH_MASK_CELL      (1U<<3)
#define FLP_TECH_MASK_BLUETOOTH (1U<<4)
#define FLP_TECH_MASK_PSEUDO    (1U<<5)

#define HAL_FLP_BUFF_SIZE   (1 * 1024)
#define MTK_FLP_SUCCESS     (0)
#define MTK_FLP_ERROR       (-1)

typedef struct {
    UINT32 type;
    UINT32 session;
    UINT32 length;
    UINT8  data[FLP_MNL_PAYLOAD_LEN];
} MTK_FLP_MNL_MSG_T;

typedef struct {
    size_t  size;
    unsigned short  flags;
    double          latitude;
    double          longitude;
    double          altitude;
    float           speed;
    float           bearing;
    float           accuracy;
    int64_t         timestamp;
    unsigned int    sources_used;
} MTK_FLP_LOCATION_T;

typedef struct {
    double max_power_allocation_mW;
    UINT32 sources_to_use;
    INT32 flags;
    INT64 period_ns;
} MTK_FLP_BATCH_OPTION_T;

typedef struct {
    UINT32 type;
    UINT32 length;
    UINT8 data[OFFLOAD_PAYLOAD_LEN];
} MTK_FLP_OFFLOAD_MSG_T;

typedef struct {
    UINT32 type;
    UINT32 session;
    UINT32 length;
} MTK_FLP_MNL_MSG_HEADER_T;

typedef struct {
    int (*mnld_flp_attach)();
    void (*mnld_flp_hbd_gps_open)();
    void (*mnld_flp_hbd_gps_close)();
    void (*mnld_flp_ofl_link_open)();
    void (*mnld_flp_ofl_link_close)();
    int (*mnld_flp_ofl_link_send)();
} flp2mnl_interface;

typedef struct {
    int (*flp_start_batching)(int id, FlpBatchOptions* options);
    int (*flp_update_batching_option)(int id, FlpBatchOptions* options);
    int (*flp_stop_batching)(int id);
    void (*flp_get_batched_location)(int last_n_locations);
    int (*flp_inject_location)(MTK_FLP_LOCATION_T* location);
    void (*flp_flush_batched_location)();
} flphal2flp_interface;


typedef struct {
    UINT32 type;   // 1:AP mode(default)   2: Offload mode  3:AP+Offload mode
    UINT32 pre_type;
    UINT32 id;
} MTK_FLP_Session_T;

typedef struct {
    int type;
    int length;
} MTK_FLP_MSG_T;


extern MTK_FLP_Session_T mnld_flp_session;

/*************************************************
  Message Table from FLP thread to MNL
*************************************************/
typedef enum {
    // MNL -> FLP
    MNLD_FLP_TYPE_MNL_BOOTUP = 100,
    MNLD_FLP_TYPE_FLP_ATTACH_DONE,
    MNLD_FLP_TYPE_SESSION_UPDATE,

    MNLD_FLP_TYPE_HBD_GPS_OPEN_DONE = 110,
    MNLD_FLP_TYPE_HBD_GPS_CLOSE_DONE,
    MNLD_FLP_TYPE_HBD_GPS_LOCATION,

    MNLD_FLP_TYPE_OFL_LINK_OPEN_DONE = 120,
    MNLD_FLP_TYPE_OFL_LINK_CLOSE_DONE,

    MNLD_FLP_TYPE_OFL_LINK_RECV = 130,

    // FLP -> MNL
    MNLD_FLP_TYPE_FLP_ATTACH = 200,

    MNLD_FLP_TYPE_HBD_GPS_OPEN = 210,
    MNLD_FLP_TYPE_HBD_GPS_CLOSE,

    MNLD_FLP_TYPE_OFL_LINK_OPEN = 220,
    MNLD_FLP_TYPE_OFL_LINK_CLOSE,

    MNLD_FLP_TYPE_OFL_LINK_SEND = 230,
} mnld_flp_type;

/*************************************************
  Message Table from FLP HAL to FLP thread
*************************************************/
typedef enum {
    MTK_FLP_MSG_SYS_FLPD_RESET_NTF = 100,

    //HAL messages
    MTK_FLP_MSG_HAL_INIT_CMD = 200,
    MTK_FLP_MSG_HAL_START_CMD,
    MTK_FLP_MSG_HAL_STOP_CMD,
    MTK_FLP_MSG_HAL_STOP_RSP,
    MTK_FLP_MSG_HAL_SET_OPTION_CMD,
    MTK_FLP_MSG_HAL_INJECT_LOC_CMD,
    MTK_FLP_MSG_HAL_DIAG_INJECT_DATA_NTF,
    MTK_FLP_MSG_HAL_DIAG_REPORT_DATA_NTF,

    MTK_FLP_MSG_HSB_REPORT_LOC_NTF = 300,
    MTK_FLP_MSG_OFL_REPORT_LOC_NTF,

    MTK_FLP_MSG_HAL_GEOFENCE_CALLBACK_NTF = 400,
    MTK_FLP_MSG_HAL_REQUEST_LOC_NTF,
    MTK_FLP_MSG_HAL_FLUSH_LOC_NTF,
    MTK_FLP_MSG_HAL_REPORT_STATUS_NTF,
    MTK_FLP_MSG_OFL_GEOFENCE_CALLBACK_NTF,
    MTK_FLP_MSG_OFL_GEOFENCE_CMD,

    MTK_FLP_MSG_DC_START_CMD = 500,
    MTK_FLP_MSG_DC_STOP_CMD,
    MTK_FLP_MSG_CONN_SCREEN_STATUS,
    MTK_FLP_MSG_END,
}MTK_FLP_MSG_TYPE;

typedef enum {
    FLP_AP_MODE      = 1,
    FLP_OFFLOAD_MODE = 2,
} MTK_FLP_SYS_MODE_T;

typedef enum {
    MTK_FLP_IDLE_MODE = -1,
    MTK_FLP_BATCH_SLEEP_ON_FIFO_FULL = 0,
    MTK_FLP_BATCH_WAKE_ON_FIFO_FULL = 1,
} MTK_FLP_BATCH_MODE_T;

typedef enum {
    EARLYSUSPEND_ON,
    EARLYSUSPEND_MEM,
} MTK_FLP_SCR_STATE;

enum {
    MNL_STATUS_NONE,
    MNL_STATUS_ATTACH,
    MNL_STATUS_ATTACH_DONE,
    MNL_STATUS_HBD_GPS_OPEN,
    MNL_STATUS_HBD_GPS_OPEN_DONE,
    MNL_STATUS_HBD_GPS_CLOSE,
    MNL_STATUS_HBD_GPS_CLOSE_DONE,
    MNL_STATUS_OFL_LINK_OPEN,
    MNL_STATUS_OFL_LINK_OPEN_DONE,
    MNL_STATUS_OFL_LINK_CLOSE,
    MNL_STATUS_OFL_LINK_CLOSE_DONE,
};

int mnl2flp_mnld_reboot();
int mnl2flp_data_send(UINT8 *buf, UINT32 len);
int mnld_flp_attach_done();
int mnld_flp_hbd_gps_open_done();
int mnld_flp_hbd_gps_close_done();
int mnld_flp_ofl_gps_open_done();
int mnld_flp_ofl_gps_close_done();
int mnld_flp_session_update();
int mnld_flp_gen_new_session();
int mtk_hal2flp_main_hdlr(int fd, flp2mnl_interface* hdlr);
int flp2mnl_hdlr(MTK_FLP_MNL_MSG_T  *prMsg);
int mtk_flp2mnl_process(MTK_FLP_MSG_T *prmsg);
int mnl2flp_hdlr(MTK_FLP_MNL_MSG_T* prmsg);
int mtk_flp_get_sys_mode();
void mtk_flp_set_sys_mode (unsigned int sysMode);
int mnl_send2flp(const char* buff, int len);



// -1 means failure
int create_flphal2mnl_fd();


#ifdef __cplusplus
}
#endif

#endif
