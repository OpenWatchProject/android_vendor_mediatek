
/*
 * Copyright (c) 2008 Travis Geiselbrecht
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files
 * (the "Software"), to deal in the Software without restriction,
 * including without limitation the rights to use, copy, modify, merge,
 * publish, distribute, sublicense, and/or sell copies of the Software,
 * and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
#ifndef __CONTEXTHUB_FW_H__
#define __CONTEXTHUB_FW_H__
#include <hostIntf.h>
#include <eventnums.h>
#include <plat/inc/mtkQ.h>

#define SENS_TYPE_GYRO_BIAS        (SENS_TYPE_FIRST_USER + 0)
#define SENS_TYPE_MAG_BIAS        (SENS_TYPE_FIRST_USER + 1)
#define SENS_TYPE_ACC_BIAS        (SENS_TYPE_FIRST_USER + 2)

#define SENSOR_TYPE_ACCELEROMETER               1
#define SENSOR_TYPE_MAGNETIC_FIELD              2
#define SENSOR_TYPE_ORIENTATION                 3 /*Add kernel driver*/
#define SENSOR_TYPE_GYROSCOPE                   4
#define SENSOR_TYPE_LIGHT                       5
#define SENSOR_TYPE_PRESSURE                    6
#define SENSOR_TYPE_TEMPERATURE                 7
#define SENSOR_TYPE_PROXIMITY                   8
#define SENSOR_TYPE_GRAVITY                     9 /*Add kernel driver*/
#define SENSOR_TYPE_LINEAR_ACCELERATION         10 /*Add kernel driver*/
#define SENSOR_TYPE_ROTATION_VECTOR             11 /*Add kernel driver*/
#define SENSOR_TYPE_RELATIVE_HUMIDITY           12 /*Add kernel driver*/
#define SENSOR_TYPE_AMBIENT_TEMPERATURE         13 /*Add kernel driver*/
#define SENSOR_TYPE_MAGNETIC_FIELD_UNCALIBRATED 14 /*Add kernel driver*/
#define SENSOR_TYPE_GAME_ROTATION_VECTOR        15 /*Add kernel driver*/
#define SENSOR_TYPE_GYROSCOPE_UNCALIBRATED      16 /*Add kernel driver*/
#define SENSOR_TYPE_SIGNIFICANT_MOTION          17
#define SENSOR_TYPE_STEP_DETECTOR               18
#define SENSOR_TYPE_STEP_COUNTER                19
#define SENSOR_TYPE_GEOMAGNETIC_ROTATION_VECTOR 20 /*Add kernel driver*/
#define SENSOR_TYPE_HEART_RATE                  21
#define SENSOR_TYPE_TILT_DETECTOR               22
#define SENSOR_TYPE_WAKE_GESTURE                23 /* these three type can be confirguratured */
#define SENSOR_TYPE_GLANCE_GESTURE              24 /* these three type can be confirguratured */
#define SENSOR_TYPE_PICK_UP_GESTURE             25 /* these three type can be confirguratured */
#define SENSOR_TYPE_WRIST_TITL_GESTURE          26 /* these three type can be confirguratured */
#define SENSOR_TYPE_DEVICE_ORIENTATION          27
#define SENSOR_TYPE_POSE_6DOF                   28
#define SENSOR_TYPE_STATIONARY_DETECT           29
#define SENSOR_TYPE_MOTION_DETECT               30
#define SENSOR_TYPE_HEART_BEAT                  31
#define SENSOR_TYPE_DYNAMIC_SENSOR_META         32
#define SENSOR_TYPE_ADDITIONAL_INFO             33
#define SENSOR_TYPE_LOW_LATENCY_OFFBODY_DETECT  34
#define SENSOR_TYPE_ACCELEROMETER_UNCALIBRATED  35

#define SENSOR_TYPE_PEDOMETER                   55
#define SENSOR_TYPE_INPOCKET                    56
#define SENSOR_TYPE_ACTIVITY                    57
#define SENSOR_TYPE_PDR                         58 /*Add kernel driver*/
#define SENSOR_TYPE_FREEFALL                    59
#define SENSOR_TYPE_FLAT                        60
#define SENSOR_TYPE_FACE_DONE                   61
#define SENSOR_TYPE_SHAKE                       62
#define SENSOR_TYPE_BRINGTOSEE                  63
#define SENSOR_TYPE_ANSWER_CALL                 64
#define SENSOR_TYPE_GEOFENCE                    65
#define SENSOR_TYPE_FLOOR_COUNTER               66
#define SENSOR_TYPE_EKG                         67
#define SENSOR_TYPE_PPG1                        68
#define SENSOR_TYPE_PPG2                        69
#define SENSOR_TYPE_RGBW                        70

#define SENSOR_TYPE_SAR                         72


/* if you want to add sensType in this file, you should add it before
 * SENSOR_TYPE_MAX,  SENSOR_TYPE_MAX is the end mark
 */
#define SENSOR_TYPE_MAX                         73


#define GRAVITY_EARTH_SCALAR            9.807f
#define DEGREE_TO_RADIRAN_SCALAR        0.017453f
#define RADIRAN_TO_DEGREE_SCALAR        57.29578f
#define GRAVITY_EARTH_1000              9807
#define ACCELEROMETER_INCREASE_NUM_AP   1000
#define GYROSCOPE_INCREASE_NUM_AP       131000
#define ALS_INCREASE_NUM_AP             1000
#define BAROMETER_INCREASE_NUM_AP       100
#define MAGNETOMETER_INCREASE_NUM_AP    100
#define ORIENTATION_INCREASE_NUM_AP     100
#define RV_INCREASE_NUM_AP              1000000
#define GMRV_INCREASE_NUM_AP            1000000
#define GRV_INCREASE_NUM_AP             1000000
#define GRAV_INCREASE_NUM_AP            1000
#define LINEARACCEL_INCREASE_NUM_AP     1000
typedef bool (*SimpleQueueForciblyDiscardCbkF)(void *data, bool onDelete); //return false to reject
struct mtkActiveSensor {
    uint64_t latency;
    uint64_t firstTime;
    uint64_t lastTime;
    struct HostIntfDataBuffer buffer;
    uint32_t rate;
    uint32_t sensorHandle;
    uint16_t minSamples;
    uint16_t curSamples;
    uint8_t numAxis;
    uint8_t interrupt;
    uint8_t numSamples;
    uint8_t packetSamples;
    uint8_t oneshot : 1;
    uint8_t discard : 1;
    uint8_t raw : 1;
    uint8_t reserved : 5;
    float rawScale;
} __attribute__((packed));

struct CalibrationData {
    struct HostHubRawPacket header;
    struct SensorAppEventHeader data_header;
    int32_t values[6];
} __attribute__((packed));

struct TestResultData {
    struct HostHubRawPacket header;
    struct SensorAppEventHeader data_header;
} __attribute__((packed));

typedef struct {
    union {
        struct {
            int32_t x;
            int32_t y;
            int32_t z;
            int32_t x_bias;
            int32_t y_bias;
            int32_t z_bias;
            int32_t reserved : 14;
            int32_t temp_result : 2;
            int32_t temperature : 16;
        };
        struct {
            int32_t azimuth;
            int32_t pitch;
            int32_t roll;
            int32_t scalar;
        };
    };
    uint32_t status;
} sensor_vec_t;

typedef struct {
    int32_t bpm;
    int32_t status;
} heart_rate_event_t;

typedef enum {
    STILL,
    STANDING,
    SITTING,
    LYING,
    ON_FOOT,
    WALKING,
    RUNNING,
    CLIMBING,
    ON_BICYCLE,
    IN_VEHICLE,
    TILTING,
    UNKNOWN,
    ACTIVITY_MAX
} activity_type_t;

typedef struct {
    uint8_t probability[ACTIVITY_MAX];    // 0~100
} activity_t;

typedef enum {
    GESTURE_NONE,
    SHAKE,
    TAP,
    TWIST,
    FLIP,
    SNAPSHOT,
    ANSWERCALL,
    PICKUP,
    GESTURE_MAX
} gesture_type_t;

typedef struct {
    int32_t probability;
} gesture_t;

typedef struct {
    uint32_t accumulated_step_count;
    uint32_t accumulated_step_length;
    uint32_t step_frequency;
    uint32_t step_length;
} pedometer_event_t;

typedef struct {
    uint32_t steps;
    int32_t oneshot;
} proximity_vec_t;

typedef struct {
    int32_t pressure;  // Pa, i.e. hPa * 100
    int32_t temperature;
    uint32_t status;
} pressure_vec_t;

typedef struct {
    int32_t relative_humidity;
    int32_t temperature;
    uint32_t status;
} relative_humidity_vec_t;


typedef struct {
    int32_t state;  // sleep, restless, awake
} sleepmonitor_event_t;

typedef struct {
    uint32_t state; //geofence [data, report_source, operation_mode]
} geofence_event_t;

typedef enum {
    FALL_NONE,
    FALL,
    FLOP,
    FALL_MAX
} fall_type;

typedef struct {
    uint8_t probability[FALL_MAX];  // 0~100
} fall_t;

typedef struct {
    int32_t state;  // 0,1
} tilt_event_t;

typedef struct {
    int32_t state;  // 0,1
} in_pocket_event_t;

typedef struct {
    int32_t state;  // 0,1
} stationary_event_t;

typedef struct {
    int32_t state;
} significant_motion_event_t;

typedef struct {
    uint32_t accumulated_step_count;
} step_counter_event_t;

typedef struct {
    uint32_t step_detect;
} step_detector_event_t;

typedef struct {
    int32_t state;
} flat_event_t;

typedef struct {
    uint32_t accumulated_floor_count;
} floor_counter_event_t;

typedef struct {
    struct {
        int32_t data[3];
        int32_t x_bias;
        int32_t y_bias;
        int32_t z_bias;
    };
    uint32_t status;
} sar_event_t;

enum flush_action {
    DATA_ACTION,
    FLUSH_ACTION,
    BIAS_ACTION,
    CALI_ACTION,
    TEMP_ACTION,
    TEST_ACTION,
};

struct data_unit_t {
    uint8_t  sensor_type;
    uint8_t  flush_action;
    uint8_t  reserve[2];
    uint64_t time_stamp;
    union {
        sensor_vec_t accelerometer_t;
        sensor_vec_t gyroscope_t;
        sensor_vec_t magnetic_t;
        sensor_vec_t orientation_t;
        sensor_vec_t pdr_event;

        int32_t light;
        proximity_vec_t proximity_t;
        int32_t temperature;
        pressure_vec_t pressure_t;
        relative_humidity_vec_t relative_humidity_t;

        sensor_vec_t uncalibrated_acc_t;
        sensor_vec_t uncalibrated_mag_t;
        sensor_vec_t uncalibrated_gyro_t;

        pedometer_event_t pedometer_t;

        heart_rate_event_t heart_rate_t;

        activity_t activity_data_t;
        gesture_t gesture_data_t;
        fall_t fall_data_t;
        significant_motion_event_t smd_t;
        step_detector_event_t step_detector_t;
        step_counter_event_t step_counter_t;
        floor_counter_event_t floor_counter_t;
        tilt_event_t tilt_event;
        in_pocket_event_t inpocket_event;
        stationary_event_t stationary_event;
        geofence_event_t geofence_data_t;
        flat_event_t flat_t;
        sar_event_t sar_event;
        int32_t value[8];
    };
} __attribute__((packed));

struct data_t {
    struct data_unit_t *data;  // data fifo pointer
    uint32_t fifo_max_size;
    uint32_t data_exist_count;
    struct data_t *next_data;
};

struct sensorFIFO {
    uint32_t rp;
    uint32_t wp;
    uint32_t                FIFOSize;
    uint32_t                reserve;
    struct data_unit_t   data[0];
};

/*------------------------------IPI commands Related Structures-----------------------------*/

/* SCP_NOTIFY EVENT */
#define    SCP_INIT_DONE            0
#define    SCP_FIFO_FULL            1
#define    SCP_NOTIFY               2
#define    SCP_BATCH_TIMEOUT        3
#define    SCP_DIRECT_PUSH          4


/* SCP_ACTION */
#define    SENSOR_HUB_ACTIVATE      0
#define    SENSOR_HUB_SET_DELAY     1
#define    SENSOR_HUB_GET_DATA      2
#define    SENSOR_HUB_BATCH         3
#define    SENSOR_HUB_SET_CONFIG    4
#define    SENSOR_HUB_SET_CUST      5
#define    SENSOR_HUB_NOTIFY        6
#define    SENSOR_HUB_BATCH_TIMEOUT 7
#define    SENSOR_HUB_SET_TIMESTAMP 8
#define    SENSOR_HUB_POWER_NOTIFY  9

#define SCP_SENSOR_HUB_TEMP_BUFSIZE     256
//#define SCP_SENSOR_HUB_FIFO_SIZE        0x800000
#define SCP_SENSOR_BATCH_FIFO_BATCH_SIZE    (20480)

#define DRAM_FIFO_FULL                1
#define DRAM_FIFO_NO_FULL             0
#define DRAM_FIFO_EMPTY               2
#define SRAM_FIFO_EMPTY               0
#define SRAM_FIFO_USED                1
#define SRAM_FIFO_FULL                2
#define FLUSH_TO_DRAM_ERR            -1
#define FLUSH_TO_DRAM_SUCCESS         1

/* ALGIN TO AP SENSOR_IPI_SIZE AT FILE SCP_NANOHUB.C, ALGIN
 * TO SCP_SENSOR_HUB_DATA UNION, ALGIN TO STRUCT DATA_UNIT_T
 * SIZEOF(STRUCT DATA_UNIT_T) = SCP_SENSOR_HUB_DATA = SENSOR_IPI_SIZE
 * BUT AT THE MOMENT AP GET DATA THROUGH IPI, WE ONLY TRANSFER
 * 44 BYTES DATA_UNIT_T, THERE ARE 4 BYTES HEADER IN SCP_SENSOR_HUB_DATA
 * HEAD
 */
#define SENSOR_IPI_SIZE 48
/*
 * SENSOR_IPI_SIZE ALGIN TO SCP_SENSOR_HUB_RSP
 * SENSOR_IPI_SIZE + SENSOR_IPI_SIZE = SCP_SENSOR_HUB_DATA
 */
#define SENSOR_IPI_HEADER_SIZE 4
#define SENSOR_IPI_PACKET_SIZE (SENSOR_IPI_SIZE - SENSOR_IPI_HEADER_SIZE)
#define SENSOR_DATA_SIZE 44

#if SENSOR_DATA_SIZE > SENSOR_IPI_PACKET_SIZE
#error "SENSOR_DATA_SIZE > SENSOR_IPI_PACKET_SIZE, out of memory"
#endif

typedef struct {
    uint8_t    sensorType;
    uint8_t    action;
    uint8_t    event;
    uint8_t    reserve;
    uint32_t   data[11];
} SCP_SENSOR_HUB_REQ;

typedef struct {
    uint8_t    sensorType;
    uint8_t    action;
    int8_t     errCode;
    uint8_t    reserve[1];
} SCP_SENSOR_HUB_RSP;

typedef struct {
    uint8_t    sensorType;
    uint8_t    action;
    uint8_t    reserve[2];
    uint32_t   enable;  // 0 : disable ; 1 : enable
} SCP_SENSOR_HUB_ACTIVATE_REQ;

typedef SCP_SENSOR_HUB_RSP SCP_SENSOR_HUB_ACTIVATE_RSP;

typedef struct {
    uint8_t    sensorType;
    uint8_t    action;
    uint8_t    reserve[2];
    uint32_t    delay;  // ms
} SCP_SENSOR_HUB_SET_DELAY_REQ;

typedef SCP_SENSOR_HUB_RSP SCP_SENSOR_HUB_SET_DELAY_RSP;

typedef struct {
    uint8_t    sensorType;
    uint8_t    action;
    uint8_t    reserve[2];
} SCP_SENSOR_HUB_GET_DATA_REQ;

typedef struct {
    uint8_t    sensorType;
    uint8_t    action;
    int8_t     errCode;
    uint8_t    reserve[1];
    union {
        int8_t      int8_Data[0];
        int16_t     int16_Data[0];
        int32_t     int32_Data[0];
    } data;
} SCP_SENSOR_HUB_GET_DATA_RSP;

typedef struct {
    uint8_t    sensorType;
    uint8_t    action;
    uint8_t    flag;           /* see SENSORS_BATCH_WAKE_UPON_FIFO_FULL definition in */
    /* hardware/libhardware/include/hardware/sensors.h */
    uint8_t    reserve[1];
    uint32_t    period_ms;      // batch reporting time in ms
    uint32_t    timeout_ms;     // sampling time in ms
} SCP_SENSOR_HUB_BATCH_REQ;

typedef SCP_SENSOR_HUB_RSP SCP_SENSOR_HUB_BATCH_RSP;

typedef struct {
    uint8_t            sensorType;
    uint8_t            action;
    uint8_t            reserve[2];
    struct sensorFIFO   *bufferBase;
    uint32_t            bufferSize;
    uint64_t            ap_timestamp;
    uint64_t            arch_counter;
} SCP_SENSOR_HUB_SET_CONFIG_REQ;

typedef SCP_SENSOR_HUB_RSP SCP_SENSOR_HUB_SET_CONFIG_RSP;
typedef enum {
    CUST_ACTION_SET_CUST = 1,
    CUST_ACTION_SET_CALI,
    CUST_ACTION_RESET_CALI,
    CUST_ACTION_SET_TRACE,
    CUST_ACTION_SET_DIRECTION,
    CUST_ACTION_SHOW_REG,
    CUST_ACTION_GET_RAW_DATA,
    CUST_ACTION_SET_PS_THRESHOLD,
    CUST_ACTION_SHOW_ALSLV,
    CUST_ACTION_SHOW_ALSVAL,
    CUST_ACTION_SET_FACTORY,
    CUST_ACTION_GET_SENSOR_INFO,
} CUST_ACTION;

typedef struct {
    CUST_ACTION    action;
} SCP_SENSOR_HUB_CUST;

typedef struct {
    CUST_ACTION    action;
    int32_t     data[0];
} SCP_SENSOR_HUB_SET_CUST;

typedef struct {
    CUST_ACTION    action;
    int trace;
} SCP_SENSOR_HUB_SET_TRACE;

typedef struct {
    CUST_ACTION    action;
    int         direction;
} SCP_SENSOR_HUB_SET_DIRECTION;

typedef struct {
    CUST_ACTION    action;
    unsigned int    factory;
} SCP_SENSOR_HUB_SET_FACTORY;

typedef struct {
    CUST_ACTION    action;
    union {
        int8_t        int8_data[0];
        uint8_t       uint8_data[0];
        int16_t       int16_data[0];
        uint16_t      uint16_data[0];
        int32_t       int32_data[0];
        uint32_t      uint32_data[3];
    };
} SCP_SENSOR_HUB_SET_CALI;

typedef SCP_SENSOR_HUB_CUST SCP_SENSOR_HUB_RESET_CALI;
typedef struct {
    CUST_ACTION    action;
    int32_t     threshold[2];
} SCP_SENSOR_HUB_SETPS_THRESHOLD;

typedef SCP_SENSOR_HUB_CUST SCP_SENSOR_HUB_SHOW_REG;
typedef SCP_SENSOR_HUB_CUST SCP_SENSOR_HUB_SHOW_ALSLV;
typedef SCP_SENSOR_HUB_CUST SCP_SENSOR_HUB_SHOW_ALSVAL;

typedef struct {
    CUST_ACTION    action;
    union {
        int8_t        int8_data[0];
        uint8_t       uint8_data[0];
        int16_t       int16_data[0];
        uint16_t      uint16_data[0];
        int32_t       int32_data[0];
        uint32_t      uint32_data[3];
    };
} SCP_SENSOR_HUB_GET_RAW_DATA;

struct mag_dev_info_t {
    char libname[16];
    int8_t layout;
    int8_t deviceid;
};
struct sensorInfo_t {
    char name[16];
    struct mag_dev_info_t mag_dev_info;
};
typedef struct {
    CUST_ACTION action;
    union {
        int32_t int32_data[0];
        struct sensorInfo_t sensorInfo;
    };
} SCP_SENSOR_HUB_GET_SENSOR_INFO;

typedef struct {
    union {
        SCP_SENSOR_HUB_CUST             cust;
        SCP_SENSOR_HUB_SET_CUST         setCust;
        SCP_SENSOR_HUB_SET_CALI         setCali;
        SCP_SENSOR_HUB_RESET_CALI       resetCali;
        SCP_SENSOR_HUB_SET_TRACE        setTrace;
        SCP_SENSOR_HUB_SET_DIRECTION    setDirection;
        SCP_SENSOR_HUB_SHOW_REG         showReg;
        SCP_SENSOR_HUB_GET_RAW_DATA     getRawData;
        SCP_SENSOR_HUB_SETPS_THRESHOLD  setPSThreshold;
        SCP_SENSOR_HUB_SHOW_ALSLV       showAlslv;
        SCP_SENSOR_HUB_SHOW_ALSVAL      showAlsval;
        SCP_SENSOR_HUB_SET_FACTORY      setFactory;
        SCP_SENSOR_HUB_GET_SENSOR_INFO  getInfo;
    };
} CUST_SET_REQ, *CUST_SET_REQ_P;
typedef struct {
    uint8_t    sensorType;
    uint8_t    action;
    uint8_t    reserve[2];
    union {
        uint32_t        custData[11];
        CUST_SET_REQ    cust_set_req;
    };
} SCP_SENSOR_HUB_SET_CUST_REQ, *SCP_SENSOR_HUB_SET_CUST_REQ_P;

typedef struct {
    uint8_t    sensorType;
    uint8_t    action;
    int8_t     errCode;
    uint8_t    reserve[1];
    union {
        uint32_t                          custData[11];
        CUST_SET_REQ                      cust_set_rsp;
    };
} SCP_SENSOR_HUB_SET_CUST_RSP;

typedef struct {
    uint8_t            sensorType;
    uint8_t            action;
    uint8_t            event;
    uint8_t            reserve[1];
    union {
        int8_t      int8_Data[0];
        int16_t     int16_Data[0];
        int32_t     int32_Data[0];
        struct {
            uint32_t    currWp;
            uint64_t    scp_timestamp;
            uint64_t    arch_counter;
        };
    };
} SCP_SENSOR_HUB_NOTIFY_RSP;

typedef union {
    SCP_SENSOR_HUB_REQ req;
    SCP_SENSOR_HUB_RSP rsp;
    SCP_SENSOR_HUB_ACTIVATE_REQ activate_req;
    SCP_SENSOR_HUB_ACTIVATE_RSP activate_rsp;
    SCP_SENSOR_HUB_SET_DELAY_REQ set_delay_req;
    SCP_SENSOR_HUB_SET_DELAY_RSP set_delay_rsp;
    SCP_SENSOR_HUB_GET_DATA_REQ get_data_req;
    SCP_SENSOR_HUB_GET_DATA_RSP get_data_rsp;
    SCP_SENSOR_HUB_BATCH_REQ batch_req;
    SCP_SENSOR_HUB_BATCH_RSP batch_rsp;
    SCP_SENSOR_HUB_SET_CONFIG_REQ set_config_req;
    SCP_SENSOR_HUB_SET_CONFIG_RSP set_config_rsp;
    SCP_SENSOR_HUB_SET_CUST_REQ set_cust_req;
    SCP_SENSOR_HUB_SET_CUST_RSP set_cust_rsp;
    SCP_SENSOR_HUB_NOTIFY_RSP notify_rsp;
} SCP_SENSOR_HUB_DATA, *SCP_SENSOR_HUB_DATA_P;

static inline int activityDataToTripleAxisData(const uint8_t *activityData, uint8_t size,
    struct TripleAxisDataPoint *sample)
{
    if (NULL == activityData || NULL == sample)
        return -1;
    if (size > (sizeof(struct TripleAxisDataPoint) - sizeof(struct SensorFirstSample)))
        return -1;
    if (size == 0)
        return -1;
    sample->ix  = ((activityData[LYING] << 24 & 0XFF000000UL) | (activityData[SITTING] << 16 & 0XFF0000UL) |
        (activityData[STANDING] << 8 & 0XFF00UL) | (activityData[STILL] & 0XFFUL));
    sample->iy  = ((activityData[CLIMBING] << 24 & 0XFF000000UL) | (activityData[RUNNING] << 16 & 0XFF0000UL) |
        (activityData[WALKING] << 8 & 0XFF00UL) | (activityData[ON_FOOT] & 0XFFUL));
    sample->iz  = ((activityData[UNKNOWN] << 24 & 0XFF000000UL) | (activityData[TILTING] << 16 & 0XFF0000UL) |
        (activityData[IN_VEHICLE] << 8 & 0XFF00UL) | (activityData[ON_BICYCLE] & 0XFFUL));
    return 0;
}
static inline int tripleAxisDataToActivityData(uint8_t *activityData, uint8_t size,
    const struct TripleAxisDataPoint *sample)
{
    if (NULL == activityData || NULL == sample)
        return -1;
    if (size < (sizeof(struct TripleAxisDataPoint) - sizeof(struct SensorFirstSample)))
        return -1;

    activityData[STILL] = sample->ix & 0XFFUL;
    activityData[STANDING] = (sample->ix & 0XFF00UL) >> 8;
    activityData[SITTING] = (sample->ix & 0XFF0000UL) >> 16;
    activityData[LYING] = (sample->ix & 0XFF000000UL) >> 24;

    activityData[ON_FOOT] = sample->iy & 0XFFUL;
    activityData[WALKING] = (sample->iy & 0XFF00UL) >> 8;
    activityData[RUNNING] = (sample->iy & 0XFF0000UL) >> 16;
    activityData[CLIMBING] = (sample->iy & 0XFF000000UL) >> 24;


    activityData[ON_BICYCLE] = sample->iz & 0XFFUL;
    activityData[IN_VEHICLE] = (sample->iz & 0XFF00UL) >> 8;
    activityData[TILTING] = (sample->iz & 0XFF0000UL) >> 16;
    activityData[UNKNOWN] = (sample->iz & 0XFF000000UL) >> 24;
    return 0;
}

static inline int rgbwDataToTripleAxisData(const uint32_t *rgbwData,
    struct TripleAxisDataPoint *sample)
{
    if (NULL == rgbwData || NULL == sample)
        return -1;
    sample->ix  = ((rgbwData[1] << 16 & 0XFFFF0000UL) | (rgbwData[0] & 0X0000FFFFUL));
    sample->iy  = ((rgbwData[3] << 16 & 0XFFFF0000UL) | (rgbwData[2] & 0X0000FFFFUL));
    sample->iz  = 0;
    return 0;
}
static inline int tripleAxisDataToRgbwData(uint32_t *rgbwData,
    const struct TripleAxisDataPoint *sample)
{
    if (NULL == rgbwData || NULL == sample)
        return -1;

    rgbwData[0] = sample->ix & 0X0000FFFFUL;
    rgbwData[1] = (sample->ix & 0XFFFF0000UL) >> 16;
    rgbwData[2] = sample->iy & 0X0000FFFFUL;
    rgbwData[3] = (sample->iy & 0XFFFF0000UL) >> 16;

    return 0;
}

int hostIntfWakeUp(void);

uint8_t chreTypeToMtkType(uint8_t sensortype);
uint8_t mtkTypeToChreType(uint8_t sensortype);
void registerDownSampleInfo(int mtkType, uint32_t rate);
void registerHwSampleInfo(int chreType, uint32_t hwDelay);
#endif

