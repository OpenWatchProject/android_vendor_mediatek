#ifndef __ALGO_ADAPTOR_H__
#define __ALGO_ADAPTOR_H__

#include <sensors.h>
#include "FreeRTOS.h"

#define EVT_SENSOR_ACCEL                sensorGetMyEventType(SENS_TYPE_ACCEL)
#define EVT_SENSOR_BARO                 sensorGetMyEventType(SENS_TYPE_BARO)
#define INPUT_STATE_INIT                0
#define INPUT_STATE_PREPARE_DATA        1
#define INPUT_STATE_UPDATE              2

#define ACCEL_MIN_RATE                  SENSOR_HZ(50)
#define BARO_MIN_RATE                   SENSOR_HZ(1)
#define ACCEL_EXTENDED_LATENCY          2000000000ull   // 2000 ms
#define ACCEL_UINT_LATENCY              1000000000ull   // 1000 ms
#define ACCEL_NORMAL_LATENCY            200000000ull    // 200 ms
#define ACCEL_FASTEST_LATENCY           20000000ull     // 20 ms
#define BARO_LATENCY                    1000000000ull   // 1000 ms
#define TIME_NANO_TO_MILLI              1000000ull
#define TIME_SEC_TO_NANO                1000000000ull
#define RATE_SCALING                    1020 //NOT 1000 or 1024 due to the truncation of a float value to an integer
#define RATE_PRESERVE                   10

#define ACTIVITY_INPUT_INTERVAL         20 //ms
#define FLAT_INPUT_INTERVAL             20 //ms
#define INPK_INPUT_INTERVAL             20 //ms
#define STEP_RECOGNITION_INPUT_INTERVAL 20 //ms
#define MOTION_INPUT_INTERVAL           20 //ms
#define STATIONARY_INPUT_INTERVAL       20 //ms
#define WIN_ORI_INPUT_INTERVAL          20 //ms
#define GESTURE_INPUT_INTERVAL          20 //ms

#define FEATURE_UPDATE_TIME_MARGIN      2 //unit: ms
#define GESTURE_NOTIFY_DATA_NUM         25  // 2000/80 (2s)

#define TILT_HOLD_TIME                  1000 //ms
#define UNKNOWN_HOLD_TIME               2000 //ms

#define GESTURE_ON                      1
#define GESTURE_OFF                     0

#define RADIUS_TO_DEGREE                180/3.14159265358979323846

#define INTERPOLATION_UPPER_LIMIT       10
#define INTERPOLATION_LOWER_LIMIT       3
#define INTERPOLATION_RATIO_SCALE       100
#define ALGO_REINT_DURATION             1000 //ms

typedef struct AccelData {
    int32_t x;
    int32_t y;
    int32_t z;
} AccelData_t;

typedef struct resampling {
    union {
        AccelData_t currentAccData;
        int32_t currentBaroData;
    };
    union {
        AccelData_t lastAccData;
        int32_t lastBaroData;
    };
    union {
        AccelData_t algoAccData;
        int32_t algoBaroData;
    };
    uint32_t current_time_stamp;
    uint32_t last_time_stamp;
    uint32_t input_sample_delay;
    uint32_t init_done;
    uint32_t input_count;
    uint32_t algo_time_stamp;
    uint32_t no_timestamp_exception;
    uint32_t ts_rollback_amount;
    uint32_t do_interpolation;
    uint8_t modify_sample_scheme;
} resampling_t;

typedef enum {
    ALGO_NONE_TRC    = 0x00,
    ALGO_TRC_RAWDATA = 0x02,
    ALGO_TRC_CALI    = 0X08,
    ALGO_TRC_INFO    = 0X10,
} algo_adaptor_debug;

typedef enum {
    NORMAL_ACC_WINDOW,
    EXTENDED_ACC_WINDOW
} acc_latency_class;

typedef enum {
    TS_LARGE_GAP,
    TS_ROLL_BACK,
    TS_NORMAL,
} timestamp_class;

typedef struct sw_motion {
    bool apply_sw_anymotion;
    bool apply_sw_nomotion;
} swMotion_t;

static const char SW_AnyMotion[] = "SW_AnyMotion";
static const char SW_NoMotion[] = "SW_NoMotion";
void init_data_resampling(resampling_t *resample, uint32_t input_sample_delay);
void sensor_input_data_resampling_preparation(resampling_t *resample, uint32_t input_time_stamp_ms, uint32_t evtType);
void algo_input_data_generation(resampling_t *resample, uint32_t evtType);
int32_t check_if_same_input(struct TripleAxisDataEvent *ev, uint32_t last_time_stamp, int32_t data_count);

#endif
