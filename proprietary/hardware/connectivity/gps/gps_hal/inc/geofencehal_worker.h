#ifndef __GEOFENCE_HAL_WORKER_H__
#define __GEOFENCE_HAL_WORKER_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <hardware/gps.h>
#include <pthread.h>
#include "data_coder.h"

//======================================================
// GFC(GPS HAL) -> MNL
//======================================================
#define GEOFENCE_TO_MNL "gfc_to_mnl"
#define MNL_TO_GEOFENCE "mnl_to_gfc"

#define HAL_GFC_BUFF_SIZE   (1 * 1024)
#define MAX_GOEFENCE 30
#define GEO_CONFIG_MASK_DISABLE_SMD     (1U<<0)
#ifdef __aarch64__
#define MTK_64_PLATFORM
#endif

#define MTK_GFC_SUCCESS     (0)
#define MTK_GFC_ERROR       (-1)
#define MTK_GFC_TIMEOUT     (-2)


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
    MTK_FLP_MSG_OFL_GEOFENCE_CALLBACK_NTF,  //for Offload geofence use
    MTK_FLP_MSG_OFL_GEOFENCE_CMD,

    MTK_FLP_MSG_DC_START_CMD = 500,
    MTK_FLP_MSG_DC_STOP_CMD,
    MTK_FLP_MSG_CONN_SCREEN_STATUS,     //for AP to connsys screen on/off status exchange
    MTK_FLP_MSG_END,
}MTK_FLP_MSG_TYPE;

typedef enum {
    inside,
    outside,
    uncertain
} SET_STATE;

typedef enum {
    INIT_GEOFENCE,
    ADD_GEOFENCE_AREA,
    PAUSE_GEOFENCE,
    RESUME_GEOFENCE,
    REMOVE_GEOFENCE,
    RECOVER_GEOFENCE,
    CLEAR_GEOFENCE,
} MTK_GFC_COMMAND_T;

typedef enum {
    GEOFENCE_ADD_CALLBACK,
    GEOFENCE_REMOVE_CALLBACK,
    GEOFENCE_PAUSE_CALLBACK,
    GEOFENCE_RESUME_CALLBACK
} GEOFENCE_CALLBACK_T;

typedef struct mtk_geofence_area {
    int32_t geofence_id;
    double latitude;
    double longitude;
    double radius;
    float coordinate_dn;
    float coordinate_de;
    int last_transition; /*current state, most cases is GPS_GEOFENCE_UNCERTAIN*/
    int monitor_transition; /*bitwise or of  entered/exited/uncertain*/
    int notification_period;/*timer  interval, period of report transition status*/
    int unknown_timer;/*continue positioning time limitied while positioning*/
    int alive;/*geofence status, 1 alive, 0 sleep*/
    SET_STATE latest_state;/*latest status: outside, inside, uncertain*/
    uint32_t config;
} MTK_GEOFENCE_PROPERTY_T;

typedef struct {
    int type;
    int length;
} MTK_FLP_MSG_T;

typedef struct mtk_geofence_callback {
    int32_t cb_id;
    int32_t geofence_id;
    int32_t result;
} MTK_GEOFENCE_CALLBACK_T;


void hal2_geofence_init();
int is_gfc_exist();
void hal2_geofence_add_geofences(const MTK_GEOFENCE_PROPERTY_T dbg_fence);
void hal2_geofence_pause_geofence(const int32_t geofence_id);
void hal2_geofence_resume_geofence(const int32_t geofence_id, const int monitor_transitions);
void hal2_geofence_remove_geofences(const int32_t geofence_id);
void mtk_geo_inject_info_set(int fence_id, float dn, float de);
void mtk_geo_inject_info_get(int fence_id, float* dn, float* de);
int check_buff_fence_exist(const int32_t fence_id);
int set_buff_transition_fence(const int32_t fence_id, const int transition);
#ifdef MTK_64_PLATFORM
void mtk_loc_rearrange(unsigned char *loc_in, GpsLocation *loc_out);
#endif

//Callbacks to GPS HAL
void mtk_geofence_transition_callbacks_proc(MTK_FLP_MSG_T *prmsg);
void mtk_geofence_callbacks_proc(MTK_FLP_MSG_T *prmsg);



#ifdef __cplusplus
}
#endif

#endif


