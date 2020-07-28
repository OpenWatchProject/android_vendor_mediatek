#ifndef _GEOFENCE_H_
#define _GEOFENCE_H_

#include <time.h>
#include <stdint.h>
#include <hardware/gps.h>
#include <hardware/fused_location.h>

#define GEO_CONFIG_MASK_DISABLE_SMD     (1U<<0)

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
    SMART_NO_TRIGGER       = 0,
    SMART_ACC_TRIGGER      = 1,
    SMART_TIMEOUT_TRIGGER  = 2,
} MTK_SMART_GEOFENCE_T;

typedef enum {
    MTK_GEOFENCE_IDLE_MODE = -1,
    MTK_BATCH_GEOFENCE_ONLY = 3,
    MTK_SMART_GEOFENCE_MODE = 4,
} MTK_GFC_MODE_T;

typedef enum {
    GFC_AP_MODE      = 1,
    GFC_OFFLOAD_MODE = 2,
} MTK_GFC_SYS_MODE_T;

typedef struct {
    unsigned int type;
    unsigned int session;
    unsigned int length;
} MTK_GFC_MNL_MSG_HEADER_T;

typedef struct {
    unsigned int type;   // 1:AP mode(default)   2: Offload mode  3:AP+Offload mode
    unsigned int pre_type;
    unsigned int id;
} MTK_GFC_Session_T;

typedef enum {
    GEOFENCE_ADD_CALLBACK,
    GEOFENCE_REMOVE_CALLBACK,
    GEOFENCE_PAUSE_CALLBACK,
    GEOFENCE_RESUME_CALLBACK
} GEOFENCE_CALLBACK_T;

typedef struct mtk_geofence_callback {
    int32_t cb_id;
    int32_t geofence_id;
    int32_t result;
} MTK_GEOFENCE_CALLBACK_T;

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

typedef struct mtk_pause_geofence {
    int32_t geofence_id;
} MTK_PAUSE_GEOFENCE_T;

typedef struct mtk_resume_geofence {
    int32_t geofence_id;
    int transition;
} MTK_RESUME_GEOFENCE_T;

typedef struct mtk_remove_geofence {
    int32_t geofence_id;
} MTK_REMOVE_GEOFENCE_T;

typedef struct mtk_modify_geofence {
    int32_t geofence_id;
    uint32_t source_to_use; /* source to use for geofence */
    int last_transition; /*current state, most cases is GPS_GEOFENCE_UNCERTAIN*/
    int monitor_transition; /*bitwise or of  entered/exited/uncertain*/
    int notification_period;/*timer  interval, period of report transition status*/
    int unknown_timer;/*continue positioning time limitied while positioning*/
} MTK_MODIFY_GEOFENCE;

typedef float          R4;      // 4 byte floating point
typedef double         R8;      // 8 byte floating point

typedef struct geofence_new_alarm {
    unsigned int unknown_init_ttick;  //record the ttick of fence starting time
    int unknown_elapsed_ttick;  //update the ttick elapsed time since the starting time
    int geofence_operating_mode; //[start, stop/ pause] = [0,1]
} GEOFENCE_NEW_ALARM_T;

typedef enum {
    START,
    STOP,
    UNKNOW
} MTK_GEOFENCE_ACTION;

//static int start_unknown_monitor(const int item);
unsigned int update_fence_source();
void mtk_flp_geofence_clear_geofences();
void  mtk_set_geofence_location(const FlpLocation* location);


#endif
