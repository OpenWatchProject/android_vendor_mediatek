#ifndef __HAL2FLP_INTERFACE_H__
#define __HAL2FLP_INTERFACE_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <hardware/fused_location.h>

//======================================================
// FLP -> FLP HAL
//======================================================
#define FLP_TO_MNL "flp_to_mnl"
#define MNL_TO_FLP "mnl_to_flp"

/*****************************************************************************
 * FLP Return Value for APIs
 *****************************************************************************/
#define MTK_FLP_SUCCESS     (0)
#define MTK_FLP_ERROR       (-1)
#define MTK_FLP_TIMEOUT     (-2)

#ifndef MTK_FLP_TRUE
#define MTK_FLP_TRUE                (1)
#endif

#ifndef MTK_FLP_FALSE
#define MTK_FLP_FALSE               (0)
#endif

#define HAL_FLP_BUFF_SIZE   (1 * 1024)

#ifdef __aarch64__
#define MTK_64_PLATFORM
#endif


/*************************************************
  Message Table for MTK FLP
*************************************************/
typedef enum{
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
    MTK_FLP_MSG_OFL_GEOFENCE_CALLBACK_NTF,  //for Offload geofence use
    MTK_FLP_MSG_OFL_GEOFENCE_CMD,

    MTK_FLP_MSG_DC_START_CMD = 500,
    MTK_FLP_MSG_DC_STOP_CMD,
    MTK_FLP_MSG_CONN_SCREEN_STATUS,     //for AP to connsys screen on/off status exchange
    MTK_FLP_MSG_END,
}MTK_FLP_MSG_TYPE;

typedef enum {
    MTK_FLP_IDLE_MODE = -1,
    MTK_FLP_BATCH_SLEEP_ON_FIFO_FULL = 0,
    MTK_FLP_BATCH_WAKE_ON_FIFO_FULL = 1,
} MTK_FLP_BATCH_MODE_T;

typedef struct {
    int type;
    int length;
} MTK_FLP_MSG_T;

typedef struct {
    double max_power_allocation_mW;
    unsigned int sources_to_use;
    int flags;
    int64_t period_ns;
} MTK_FLP_BATCH_OPTION_T;


typedef FlpDiagnosticCallbacks (*FlpDiagCB)();
typedef void (* timer_callback)();

int mnl2flphal_flp_init();
int isflp_thread_exist() ;
void flphal2mnl_flp_init();
int flphal2mnl_flp_start(int id, FlpBatchOptions* options);
int flphal2mnl_flp_reboot_done_ntf();
int flphal2mnl_update_batching_options(int id, FlpBatchOptions* options);
int flphal2mnl_stop_batching(int id) ;
void flphal2mnl_get_batched_location(int last_n_locations);
int flphal2mnl_inject_location(FlpLocation* location);
int flphal2mnl_diag_inject_data(char* data, int length);
void flphal2mnl_flush_batched_locations() ;
int flp_send2mnl(const char* buff, int len);


#ifdef __cplusplus
}
#endif

#endif
