#ifndef  _FTM_FACTORY_TEST_H
#define _FTM_FACTORY_TEST_H

#include "gf_type_define.h"
#include "gf_hal_common.h"

#define MAX_LENGTH  512
#define MAX_SIZE 50

typedef struct{
    uint32_t token;

    union{
        int8_t int8_value;
        uint8_t uint8_value;
        int16_t int16_value;
        uint16_t uint16_value;
        int32_t int32_value;
        uint32_t uint32_value;
        int64_t int64_value;
        uint64_t uint64_value;
        float float_value;
        double double_value;
        char  buffer[MAX_LENGTH];
    } value;

    uint32_t buffer_size;
} token_map_t;


#define  MAX_TEMPLATE_COUNT   20
#define  MAX_SAMPLE_COUNT   50

#define isMilanFSeries(sensorType) \
         (GF_SENSOR_3208 == sensorType || GF_SENSOR_3206 == sensorType \
                || GF_SENSOR_3266 == sensorType || GF_SENSOR_3288 == sensorType \
                || GF_SENSOR_3208FN == sensorType)

#define isMilanASeries(sensorType) \
         (GF_SENSOR_5206 == sensorType || GF_SENSOR_5216 == sensorType \
                || GF_SENSOR_5208 == sensorType || GF_SENSOR_5218 == sensorType)

#define isOswego(sensorType) \
         (GF_SENSOR_316M == sensorType || GF_SENSOR_318M == sensorType \
                || GF_SENSOR_3118M == sensorType || GF_SENSOR_516M == sensorType \
                || GF_SENSOR_518M == sensorType || GF_SENSOR_5118M == sensorType \
        || GF_SENSOR_816M == sensorType || GF_SENSOR_316 == sensorType \
        || GF_SENSOR_318 == sensorType || GF_SENSOR_516 == sensorType  \
        || GF_SENSOR_518 == sensorType)


/***oswego***/
#define TEST_SPI_GFX18  "GFx18M"
#define TEST_SPI_GFX16  "GFx16M"

#define TEST_FW_VERSION_GFX18  "GFx18M_1.05.11"
#define TEST_FW_VERSION_GFX16  "GFx16M_1.05.11"

#define TEST_SENSOR_BAD_POINT_COUNT_OSWEGO  25

#define TEST_BAD_POINT_BAD_PIXEL_NUM_OSWEGO  25
#define TEST_BAD_POINT_LOCAL_BAD_PIXEL_NUM_OSWEGO  4
#define TEST_BAD_POINT_AVG_DIFF_VAL_OSWEGO  800
#define TEST_BAD_POINT_ALL_TILT_ANGLE_OSWEGO  0.1f
#define TEST_BAD_POINT_BLOCK_TILT_ANGLE_MAX_OSWEGO  0.269f
#define TEST_BAD_POINT_LOCAL_SMALL_BAD_POINT_OSWEGO  10
#define TEST_BAD_POINT_LOCAL_BIG_BAD_POINT_OSWEGO  5

/***milan-f***/
#define  TEST_SPI_CHIP_ID_MILAN_F  0x002202
#define  TEST_SENSOR_BAD_POINT_COUNT_MILAN_F  35
#define  TEST_BAD_POINT_TOTAL_BAD_PIXEL_NUM_MILAN_F  45
#define  TEST_BAD_POINT_LOCAL_BAD_PIXEL_NUM_MILAN_F  15
#define  TEST_BAD_POINT_LOCAL_WORST_MILAN_F  8

/***milan-fn***/
#define  TEST_SPI_CHIP_ID_MILAN_FN  0x00220C
#define  TEST_SENSOR_BAD_POINT_COUNT_MILAN_FN  35
#define  TEST_BAD_POINT_TOTAL_BAD_PIXEL_NUM_MILAN_FN  45
#define  TEST_BAD_POINT_LOCAL_BAD_PIXEL_NUM_MILAN_FN  15
#define  TEST_BAD_POINT_LOCAL_WORST_MILAN_FN  8

/***milan-E***/
#define  TEST_SPI_CHIP_ID_MILAN_E  0x002207

#define TEST_SENSOR_BAD_POINT_COUNT_MILAN_E  35

#define  TEST_BAD_POINT_TOTAL_BAD_PIXEL_NUM_MILAN_E  45
#define  TEST_BAD_POINT_LOCAL_BAD_PIXEL_NUM_MILAN_E  15
#define  TEST_BAD_POINT_LOCAL_WORST_MILAN_E   8


/***milan-G***/
#define TEST_SPI_CHIP_ID_MILAN_G  0x002208

#define TEST_SENSOR_BAD_POINT_COUNT_MILAN_G  35

#define TEST_BAD_POINT_TOTAL_BAD_PIXEL_NUM_MILAN_G  45
#define TEST_BAD_POINT_LOCAL_BAD_PIXEL_NUM_MILAN_G  15
#define TEST_BAD_POINT_LOCAL_WORST_MILAN_G  8


/******milan-L****/
#define TEST_SPI_CHIP_ID_MILAN_L  0x002205

#define TEST_SENSOR_BAD_POINT_COUNT_MILAN_L  35

#define TEST_BAD_POINT_TOTAL_BAD_PIXEL_NUM_MILAN_L  45
#define TEST_BAD_POINT_LOCAL_BAD_PIXEL_NUM_MILAN_L  15
#define TEST_BAD_POINT_LOCAL_WORST_MILAN_L  8


/******milan-A****/
#define TEST_SPI_FW_VERSION_MILAN_A  "GF5206_4.11.39"

#define TEST_FW_VERSION_SENSOR_TYPE_1_MILAN_A  "GF5206_4.11.39"
#define TEST_FW_VERSION_SENSOR_TYPE_2_MILAN_A  "GF5206_4.11.39"
#define TEST_FW_VERSION_SENSOR_TYPE_3_MILAN_A  "GF5206_4.11.39"
#define TEST_FW_VERSION_SENSOR_TYPE_4_MILAN_A  "GF5206_4.11.39"
#define TEST_FW_VERSION_SENSOR_TYPE_5_MILAN_A  "GF5206_4.11.39"

#define TEST_MILAN_A_SENSOR_OTP_TYPE_1  0x01
#define TEST_MILAN_A_SENSOR_OTP_TYPE_2  0x02
#define TEST_MILAN_A_SENSOR_OTP_TYPE_3  0x03
#define TEST_MILAN_A_SENSOR_OTP_TYPE_4  0x04
#define TEST_MILAN_A_SENSOR_OTP_TYPE_5  0x05

#define TEST_PERFORMANCE_TOTAL_TIME_MILAN_A   500
#define TEST_SPI_CHIP_ID_MILAN_A   0x12A

#define TEST_SENSOR_BAD_POINT_COUNT_MILAN_A   35
#define TEST_BAD_POINT_TOTAL_BAD_PIXEL_NUM_MILAN_A   50
#define TEST_BAD_POINT_LOCAL_BAD_PIXEL_NUM_MILAN_A   15
#define TEST_BAD_POINT_LOCAL_WORST_MILAN_A   8
#define TEST_BAD_POINT_INCIRCLE_MILAN_A   30

#define TEST_BIO_THRESHOLD_UNTOUCHED_MILAN_A   200
#define TEST_BIO_THRESHOLD_TOUCHED_MIN_MILAN_A   800
#define TEST_BIO_THRESHOLD_TOUCHED_MAX_MILAN_A   2500

#define TEST_HBD_THRESHOLD_AVG_MIN_MILAN_A   500
#define TEST_HBD_THRESHOLD_AVG_MAX_MILAN_A   3500
#define TEST_HBD_THRESHOLD_ELECTRICITY_MIN_MILAN_A   10
#define TEST_HBD_THRESHOLD_ELECTRICITY_MAX_MILAN_A   200

/***************milan-B****************/
#define TEST_SPI_FW_VERSION_MILAN_B  "GF5206_4.11.39"

#define TEST_FW_VERSION_SENSOR_TYPE_1_MILAN_B  "GF5206_4.11.39"
#define TEST_FW_VERSION_SENSOR_TYPE_2_MILAN_B  "GF5206_4.11.39"
#define TEST_FW_VERSION_SENSOR_TYPE_3_MILAN_B  "GF5206_4.11.39"
#define TEST_FW_VERSION_SENSOR_TYPE_4_MILAN_B  "GF5206_4.11.39"
#define TEST_FW_VERSION_SENSOR_TYPE_5_MILAN_B  "GF5216_4.00.15"
#define TEST_FW_VERSION_SENSOR_TYPE_6_MILAN_B  "GF5216_4.00.17"
#define TEST_FW_VERSION_SENSOR_TYPE_7_MILAN_B  "GF5216_4.00.18"
#define TEST_FW_VERSION_SENSOR_TYPE_8_MILAN_B  "GF5216_4.00.19"
#define TEST_FW_VERSION_SENSOR_TYPE_9_MILAN_B  "GF5216_4.00.19"

#define TEST_MILAN_B_SENSOR_OTP_TYPE_1  0x01
#define TEST_MILAN_B_SENSOR_OTP_TYPE_2  0x02
#define TEST_MILAN_B_SENSOR_OTP_TYPE_3  0x03
#define TEST_MILAN_B_SENSOR_OTP_TYPE_4  0x04
#define TEST_MILAN_B_SENSOR_OTP_TYPE_5  0x05
#define TEST_MILAN_B_SENSOR_OTP_TYPE_6  0x06
#define TEST_MILAN_B_SENSOR_OTP_TYPE_7  0x07
#define TEST_MILAN_B_SENSOR_OTP_TYPE_8  0x08
#define TEST_MILAN_B_SENSOR_OTP_TYPE_9  0x09

    /******milan-C****/
#define TEST_SPI_FW_VERSION_MILAN_C  "GF52X8_1.00.12"

#define TEST_MILAN_C_SENSOR_OTP_TYPE_1  0x01
#define TEST_MILAN_C_SENSOR_OTP_TYPE_2  0x02
#define TEST_MILAN_C_SENSOR_OTP_TYPE_3  0x03
#define TEST_MILAN_C_SENSOR_OTP_TYPE_4  0x04
#define TEST_MILAN_C_SENSOR_OTP_TYPE_5  0x05

#define TEST_FW_VERSION_SENSOR_TYPE_1_MILAN_C  "GF52X8_1.00.12"
#define TEST_FW_VERSION_SENSOR_TYPE_2_MILAN_C  "GF52X8_1.00.24"
#define TEST_FW_VERSION_SENSOR_TYPE_3_MILAN_C  "GF52X8_1.00.23"
#define TEST_FW_VERSION_SENSOR_TYPE_4_MILAN_C  "GF52X8_2.00.02"
#define TEST_FW_VERSION_SENSOR_TYPE_5_MILAN_C  "GF52X8_2.00.04"

#define TEST_PERFORMANCE_TOTAL_TIME_MILAN_C  500
#define TEST_SPI_CHIP_ID_MILAN_C  0x12B

#define TEST_SENSOR_BAD_POINT_COUNT_MILAN_C  35
#define TEST_BAD_POINT_TOTAL_BAD_PIXEL_NUM_MILAN_C  50
#define TEST_BAD_POINT_LOCAL_BAD_PIXEL_NUM_MILAN_C  15
#define TEST_BAD_POINT_LOCAL_WORST_MILAN_C  8
#define TEST_BAD_POINT_INCIRCLE_MILAN_C  30

#define TEST_BIO_THRESHOLD_UNTOUCHED_MILAN_C  200
#define TEST_BIO_THRESHOLD_TOUCHED_MIN_MILAN_C  800
#define TEST_BIO_THRESHOLD_TOUCHED_MAX_MILAN_C  2500

#define TEST_HBD_THRESHOLD_AVG_MIN_MILAN_C  500
#define TEST_HBD_THRESHOLD_AVG_MAX_MILAN_C  3500
#define TEST_HBD_THRESHOLD_ELECTRICITY_MIN_MILAN_C  10
#define TEST_HBD_THRESHOLD_ELECTRICITY_MAX_MILAN_C  200


#define ALGO_VERSION_INFO_LEN   64
#define FW_VERSION_INFO_LEN   64
#define TEE_VERSION_INFO_LEN   72
#define TA_VERSION_INFO_LEN   64
#define VENDOR_ID_LEN   32
#define PRODUCTION_DATE_LEN   32

#define TEST_CAPTURE_VALID_IMAGE_QUALITY_THRESHOLD   15
#define TEST_CAPTURE_VALID_IMAGE_AREA_THRESHOLD   65

#define TEST_PERFORMANCE_TOTAL_TIME   400

#define TEST_TIMEOUT_MS   30 * 1000

#define TEST_HISTORY_PATH   "GFTest"

#define AUTO_TEST_TIME_INTERVAL   5000 // 5 seconds
#define AUTO_TEST_BIO_PREPARE_TIME   5000

uint32_t decodeInt32(uint8_t* result, uint32_t offset);
uint16_t decodeInt16(uint8_t* result, uint32_t offset);
uint8_t decodeInt8(uint8_t* result, uint32_t offset);
float decodeFloat(uint8_t* result, uint32_t offset, uint32_t size);
double decodeDouble(uint8_t* result, uint32_t offset, uint32_t size) ;
void parser(uint8_t* result,uint32_t len) ;


uint8_t checkErrcode(token_map_t* result,uint32_t count);
uint8_t checkSpiTestResult(token_map_t* result,uint32_t count);
uint8_t checkResetPinTestReuslt(token_map_t* result,uint32_t count);
uint8_t checkPixelTestResult(token_map_t* result,uint32_t count);
uint8_t checkFwVersionTestResult(token_map_t* result,uint32_t count);
uint8_t checkPerformanceTestResult(token_map_t* result,uint32_t count);
uint8_t checkCaptureTestResult(token_map_t* result,uint32_t count);
uint8_t checkAlgoTestResult(token_map_t* result,uint32_t count);
uint8_t checkBadPointTestResult(token_map_t* result,uint32_t count) ;
uint8_t checkBioTestResultWithoutTouched(token_map_t* result,uint32_t count);
uint8_t checkBioTestResultWithTouched(token_map_t* result,uint32_t count);


void initOswegoChecker(int sensorType);
uint8_t Oswego_checkSpiTestResult(token_map_t* result,uint32_t count) ;
uint8_t Oswego_checkFwVersionTestResult(token_map_t* result,uint32_t count) ;
uint8_t Oswego_checkBadPointTestResult(token_map_t* result,uint32_t count) ;

void initMilanASeriesChecker(int sensorType);
uint8_t MilanASeries_checkSpiTestResult(token_map_t* result,uint32_t count);
uint8_t MilanASeries_checkFwVersionTestResult(token_map_t* result,uint32_t count);
uint8_t MilanASeries_checkBioTestResultWithTouched(token_map_t* result,uint32_t count);
uint8_t MilanASeries_checkBioTestResultWithoutTouched(token_map_t* result,uint32_t count);

void initMilanFSeriesChecker(int sensorType);
uint8_t MilanFSeries_checkSpiTestResult(token_map_t* result,uint32_t count) ;
uint8_t MilanFSeries_checkBadPointTestResult(token_map_t* result,uint32_t count);

#endif
