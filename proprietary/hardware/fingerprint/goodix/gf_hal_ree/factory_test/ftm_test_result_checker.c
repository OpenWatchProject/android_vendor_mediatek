#include "ftm_factory_test.h"
#include "gf_fingerprint.h"
#include "string.h"
#include <stdlib.h>

#define LOG_TAG "[gf_test_result_checker]"

#ifndef NULL
#define NULL 0
#endif

#define  TEST_NONE  0
#define  TEST_SPI  1
#define  TEST_PIXEL  2
#define  TEST_RESET_PIN  3
#define  TEST_BAD_POINT  4
#define  TEST_PERFORMANCE 5
#define  TEST_CAPTURE  6
#define  TEST_ALGO  7
#define  TEST_FW_VERSION  8
#define  TEST_SENSOR_CHECK  9
#define  TEST_BIO_CALIBRATION  10
#define  TEST_HBD_CALIBRATION  11
#define  TEST_MAX  11

static char g_spiFwVersion[MAX_LENGTH];
static int g_chipId;
static int g_badBointNum = 0;
static char g_fwVersion[MAX_LENGTH];
static long g_totalTime = TEST_PERFORMANCE_TOTAL_TIME;
static int g_imageQuality = TEST_CAPTURE_VALID_IMAGE_QUALITY_THRESHOLD;
static int g_validArea = TEST_CAPTURE_VALID_IMAGE_AREA_THRESHOLD;
static short g_avgDiffVal;
static int g_badPixelNum = 0;
static int g_localBadPixelNum = 0;
static float g_allTiltAngle = 0;
static float g_blockTiltAngleMax = 0;
static short g_localWorst = 0;
static int g_singular = 0;
static int g_inCircle = 0;
static int g_osdUntouched = 0;
static int g_osdTouchedMin = 0;
static int g_osdTouchedMax = 0;
static int g_hbdAvgMax = 0;
static int g_hbdAvgMin = 0;
static int g_hbdElectricityMin = 0;
static int g_hbdElectricityMax = 0;
static int g_localSmallBadPixel = 0;
static int g_localBigBadPixel = 0;
static int g_sensorType;

void initThreshold(int sensorType) {
	LOG_D(LOG_TAG, "[%s] zwq sensorType=%d,GF_SENSOR_5216 = %d", __func__,sensorType,GF_SENSOR_5216);
    g_sensorType = sensorType;
    switch (sensorType) {
    case GF_SENSOR_318M:
    case GF_SENSOR_3118M:
    case GF_SENSOR_518M:
    case GF_SENSOR_5118M:
    case GF_SENSOR_318:
    case GF_SENSOR_518:
        strcpy(g_spiFwVersion, TEST_SPI_GFX18);
        strcpy(g_fwVersion, TEST_FW_VERSION_GFX18);

        g_badBointNum = TEST_SENSOR_BAD_POINT_COUNT_OSWEGO;

        g_badPixelNum = TEST_BAD_POINT_BAD_PIXEL_NUM_OSWEGO;
        //g_localBadPixelNum = TEST_BAD_POINT_LOCAL_BAD_PIXEL_NUM_OSWEGO;
        g_avgDiffVal = TEST_BAD_POINT_AVG_DIFF_VAL_OSWEGO;
        g_localSmallBadPixel = TEST_BAD_POINT_LOCAL_SMALL_BAD_POINT_OSWEGO;
        g_localBigBadPixel = TEST_BAD_POINT_LOCAL_BIG_BAD_POINT_OSWEGO;
        break;

    case GF_SENSOR_316M:
    case GF_SENSOR_516M:
    case GF_SENSOR_816M:
    case GF_SENSOR_316:
    case GF_SENSOR_516:
        strcpy(g_spiFwVersion, TEST_SPI_GFX16);
        strcpy(g_fwVersion, TEST_FW_VERSION_GFX16);
        g_badBointNum = TEST_SENSOR_BAD_POINT_COUNT_OSWEGO;

        g_badPixelNum = TEST_BAD_POINT_BAD_PIXEL_NUM_OSWEGO;
        //g_localBadPixelNum = TEST_BAD_POINT_LOCAL_BAD_PIXEL_NUM_OSWEGO;
        g_avgDiffVal = TEST_BAD_POINT_AVG_DIFF_VAL_OSWEGO;
        g_localSmallBadPixel = TEST_BAD_POINT_LOCAL_SMALL_BAD_POINT_OSWEGO;
        g_localBigBadPixel = TEST_BAD_POINT_LOCAL_BIG_BAD_POINT_OSWEGO;

        break;

    case GF_SENSOR_3208:
        g_chipId = TEST_SPI_CHIP_ID_MILAN_F;
        g_badBointNum = TEST_SENSOR_BAD_POINT_COUNT_MILAN_F;

        g_badPixelNum = TEST_BAD_POINT_TOTAL_BAD_PIXEL_NUM_MILAN_F;
        g_localBadPixelNum = TEST_BAD_POINT_LOCAL_BAD_PIXEL_NUM_MILAN_F;
        g_localWorst = TEST_BAD_POINT_LOCAL_WORST_MILAN_F;

        break;

    case GF_SENSOR_3208FN:
        g_chipId = TEST_SPI_CHIP_ID_MILAN_FN;
        g_badBointNum = TEST_SENSOR_BAD_POINT_COUNT_MILAN_FN;

        g_badPixelNum = TEST_BAD_POINT_TOTAL_BAD_PIXEL_NUM_MILAN_FN;
        g_localBadPixelNum = TEST_BAD_POINT_LOCAL_BAD_PIXEL_NUM_MILAN_FN;
        g_localWorst = TEST_BAD_POINT_LOCAL_WORST_MILAN_FN;

        break;

    case GF_SENSOR_3206:
        g_chipId = TEST_SPI_CHIP_ID_MILAN_G;
        g_badBointNum = TEST_SENSOR_BAD_POINT_COUNT_MILAN_G;

        g_badPixelNum = TEST_BAD_POINT_TOTAL_BAD_PIXEL_NUM_MILAN_G;
        g_localBadPixelNum = TEST_BAD_POINT_LOCAL_BAD_PIXEL_NUM_MILAN_G;
        g_localWorst = TEST_BAD_POINT_LOCAL_WORST_MILAN_G;
        break;

    case GF_SENSOR_3266:
        g_chipId = TEST_SPI_CHIP_ID_MILAN_E;
        g_badBointNum = TEST_SENSOR_BAD_POINT_COUNT_MILAN_E;

        g_badPixelNum = TEST_BAD_POINT_TOTAL_BAD_PIXEL_NUM_MILAN_E;
        g_localBadPixelNum = TEST_BAD_POINT_LOCAL_BAD_PIXEL_NUM_MILAN_E;
        g_localWorst = TEST_BAD_POINT_LOCAL_WORST_MILAN_E;
        break;

    case GF_SENSOR_3288:
        g_chipId = TEST_SPI_CHIP_ID_MILAN_L;
        g_badBointNum = TEST_SENSOR_BAD_POINT_COUNT_MILAN_L;

        g_badPixelNum = TEST_BAD_POINT_TOTAL_BAD_PIXEL_NUM_MILAN_L;
        g_localBadPixelNum = TEST_BAD_POINT_LOCAL_BAD_PIXEL_NUM_MILAN_L;
        g_localWorst = TEST_BAD_POINT_LOCAL_WORST_MILAN_L;
        break;

    case GF_SENSOR_5206:
    case GF_SENSOR_5216:
        if (sensorType == GF_SENSOR_5206) {
            strcpy(g_spiFwVersion, TEST_SPI_FW_VERSION_MILAN_A);
            strcpy(g_fwVersion, TEST_FW_VERSION_SENSOR_TYPE_1_MILAN_A);
        } else {
            strcpy(g_spiFwVersion, TEST_SPI_FW_VERSION_MILAN_B);
            strcpy(g_fwVersion, TEST_FW_VERSION_SENSOR_TYPE_1_MILAN_B);
        }

        g_badBointNum = TEST_SENSOR_BAD_POINT_COUNT_MILAN_A;
        g_totalTime = TEST_PERFORMANCE_TOTAL_TIME_MILAN_A;

        g_badPixelNum = TEST_BAD_POINT_TOTAL_BAD_PIXEL_NUM_MILAN_A;
        g_localBadPixelNum = TEST_BAD_POINT_LOCAL_BAD_PIXEL_NUM_MILAN_A;
        g_localWorst = TEST_BAD_POINT_LOCAL_WORST_MILAN_A;
        g_inCircle = TEST_BAD_POINT_INCIRCLE_MILAN_A;

        if (sensorType == GF_SENSOR_5206) {
            g_osdUntouched = TEST_BIO_THRESHOLD_UNTOUCHED_MILAN_A;
            g_osdTouchedMin = TEST_BIO_THRESHOLD_TOUCHED_MIN_MILAN_A;
            g_osdTouchedMax = TEST_BIO_THRESHOLD_TOUCHED_MAX_MILAN_A;

            g_hbdAvgMin = TEST_HBD_THRESHOLD_AVG_MIN_MILAN_A;
            g_hbdAvgMax = TEST_HBD_THRESHOLD_AVG_MAX_MILAN_A;
            g_hbdElectricityMin = TEST_HBD_THRESHOLD_ELECTRICITY_MIN_MILAN_A;
            g_hbdElectricityMax = TEST_HBD_THRESHOLD_ELECTRICITY_MAX_MILAN_A;
        }
        break;

    case GF_SENSOR_5208:
    case GF_SENSOR_5218:

        strcpy(g_spiFwVersion, TEST_SPI_FW_VERSION_MILAN_C);
        strcpy(g_fwVersion, TEST_FW_VERSION_SENSOR_TYPE_1_MILAN_C);
        g_badBointNum = TEST_SENSOR_BAD_POINT_COUNT_MILAN_C;
        g_totalTime = TEST_PERFORMANCE_TOTAL_TIME_MILAN_C;

        g_badPixelNum = TEST_BAD_POINT_TOTAL_BAD_PIXEL_NUM_MILAN_C;
        g_localBadPixelNum = TEST_BAD_POINT_LOCAL_BAD_PIXEL_NUM_MILAN_C;
        g_localWorst = TEST_BAD_POINT_LOCAL_WORST_MILAN_C;
        g_inCircle = TEST_BAD_POINT_INCIRCLE_MILAN_C;

        if (sensorType == GF_SENSOR_5208) {
            g_osdUntouched = TEST_BIO_THRESHOLD_UNTOUCHED_MILAN_C;
            g_osdTouchedMin = TEST_BIO_THRESHOLD_TOUCHED_MIN_MILAN_C;
            g_osdTouchedMax = TEST_BIO_THRESHOLD_TOUCHED_MAX_MILAN_C;
            g_hbdAvgMin = TEST_HBD_THRESHOLD_AVG_MIN_MILAN_C;
            g_hbdAvgMax = TEST_HBD_THRESHOLD_AVG_MAX_MILAN_C;
            g_hbdElectricityMin = TEST_HBD_THRESHOLD_ELECTRICITY_MIN_MILAN_C;
            g_hbdElectricityMax = TEST_HBD_THRESHOLD_ELECTRICITY_MAX_MILAN_C;
        }

        break;

    default:
        break;
    }
}

uint8_t checkErrcode(token_map_t* result, uint32_t count) {
    int errorCode = GF_FINGERPRINT_ERROR_VENDOR_BASE;
    int i = 0;

    for (i = 0; i < count; i++) {
        if (TEST_TOKEN_ERROR_CODE == (result + i)->token) {
            errorCode = (result + i)->value.uint32_value;
            break;
        }
    }

    return (errorCode == 0);
}

uint8_t checkSpiTestResult(token_map_t* result, uint32_t count) {
    return checkErrcode(result, count);
}

uint8_t checkSpiTestResult2(int errCode, char* fwVersion, int ChipId) {
    return (errCode == 0);
}

uint8_t checkResetPinTestReuslt2(int errCode, int resetFlag) {
    return (errCode == 0) && (resetFlag > 0);
}

uint8_t checkResetPinTestReuslt(token_map_t* result, uint32_t count) {
    if (checkErrcode(result, count)) {
        int resetFlag = 0;

        int i = 0;

        for (i = 0; i < count; i++) {
            if (TEST_TOKEN_RESET_FLAG == (result + i)->token) {
                resetFlag = (result + i)->value.uint32_value;
                break;
            }
        }

        if (checkResetPinTestReuslt2(0, resetFlag)) {
            return 1;
        }
    }

    return 0;
}

uint8_t checkPixelTestResult2(int errCode, int badPixelNum) {
    return (errCode == 0) & (badPixelNum <= g_badBointNum);
}

uint8_t checkPixelTestResult(token_map_t* result, uint32_t count) {
    if (checkErrcode(result, count)) {
        int badPixelNum = 999;
        int i = 0;

        for (i = 0; i < count; i++) {
            if (TEST_TOKEN_BAD_PIXEL_NUM == (result + i)->token) {
                badPixelNum = (result + i)->value.uint32_value;
                break;
            }
        }

        if (checkPixelTestResult2(0, badPixelNum)) {
            return 1;
        }
    }
    return 0;
}

uint8_t checkFwVersionTestResult(token_map_t* result, uint32_t count) {
    return checkErrcode(result, count);
}

uint8_t checkFwVersionTestResult2(int errCode, char* fwVersion) {
    return (errCode == 0);
}

uint8_t checkPerformanceTestResult2(int errCode, int totalTime) {
    return (errCode == 0) && (totalTime < g_totalTime);
}

uint8_t checkPerformanceTestResult(token_map_t* result, uint32_t count) {
    if (checkErrcode(result, count)) {
        int totalTime = 0;
        int i = 0;

        for (i = 0; i < count; i++) {
            if (TEST_TOKEN_TOTAL_TIME == (result + i)->token) {
                totalTime = (result + i)->value.uint32_value;
                break;
            }
        }

        if (checkPerformanceTestResult2(0, totalTime)) {
            return 1;
        }
    }

    return 0;
}

uint8_t checkCaptureTestResult2(int errCode, int imageQuality, int validArea) {
    return (errCode == 0) && (imageQuality >= g_imageQuality)
            && (validArea >= g_validArea);
}

uint8_t checkCaptureTestResult(token_map_t* result, uint32_t count) {
    if (checkErrcode(result, count)) {
        int imageQuality = 0;
        int validArea = 0;

        int i = 0;

        for (i = 0; i < count; i++) {
            if (TEST_TOKEN_IMAGE_QUALITY == (result + i)->token) {
                imageQuality = (result + i)->value.uint32_value;
            } else if (TEST_TOKEN_VALID_AREA == (result + i)->token) {
                validArea = (result + i)->value.uint32_value;
            }
        }

        if (checkCaptureTestResult2(0, imageQuality, validArea)) {
            return 1;
        }
    }

    return 0;
}

uint8_t checkAlgoTestResult(token_map_t* result, uint32_t count) {
    return checkErrcode(result, count);
}

uint8_t checkAlgoTestResult2(int errCode) {
    return (errCode == 0);
}

uint8_t checkBadPointTestResult(token_map_t* result, uint32_t count) {
    return checkErrcode(result, count);
}

uint8_t checkBadPointTestResult2(int errCode, int badPixelNum,
        int localBadPixelNum, float allTiltAngle, float blockTiltAngleMax,
        short avgDiffVal, short localWorst, int singular) {
    return (errCode == 0);
}

uint8_t checkBioTestResultWithoutTouched(token_map_t* result, uint32_t count) {
    return checkErrcode(result, count);
}

uint8_t checkBioTestResultWithoutTouched2(int errCode, int base, int avg) {
    return (errCode == 0);
}

uint8_t checkBioTestResultWithTouched(token_map_t* result, uint32_t count) {
    return checkErrcode(result, count);
}

uint8_t checkBioTestResultWithTouched2(int errCode, int base, int avg) {
    return (errCode == 0);
}

static int TEST_ITEM_OSWEGO[] = {
TEST_SPI, /**/
TEST_PIXEL, /**/
TEST_RESET_PIN, /**/
TEST_BAD_POINT, /**/
TEST_PERFORMANCE, /**/
TEST_CAPTURE, /**/
TEST_ALGO, /**/
TEST_FW_VERSION };

void initOswegoChecker(int sensorType) {
    initThreshold(sensorType);
}

uint8_t Oswego_checkSpiTestResult2(int errCode, char* fwVersion, int ChipId) {

    return (errCode == 0)
            && (fwVersion != NULL
                    && strstr(fwVersion, g_spiFwVersion) == fwVersion);
}

uint8_t Oswego_checkSpiTestResult(token_map_t* result, uint32_t count) {
    if (checkSpiTestResult(result, count)) {
        char* fwVersion = NULL;
        int i = 0;

        for (i = 0; i < count; i++) {
            if (TEST_TOKEN_FW_VERSION == (result + i)->token) {
                fwVersion = (result + i)->value.buffer;
                break;
            }
        }

        if (Oswego_checkSpiTestResult2(0, fwVersion, 0)) {
            return 1;
        }
    }

    return 0;
}

uint8_t Oswego_checkFwVersionTestResult2(int errCode, char* fwVersion) {
    return (errCode == 0) && (fwVersion != NULL)
            && (strstr(fwVersion, g_fwVersion) == fwVersion);
}

uint8_t Oswego_checkFwVersionTestResult(token_map_t* result, uint32_t count) {

    if (checkFwVersionTestResult(result, count)) {
        char* fwVersion = NULL;
        int i = 0;

        for (i = 0; i < count; i++) {
            if (TEST_TOKEN_FW_VERSION == (result + i)->token) {
                fwVersion = (result + i)->value.buffer;
                break;
            }
        }

        if (Oswego_checkFwVersionTestResult2(0, fwVersion)) {
            return 1;
        }
    }
    return 0;
}

uint8_t Oswego_checkBadPointTestResult2(int errCode, int badPixelNum,
        int localBadPixelNum, float allTiltAngle, float blockTiltAngleMax,
        short avgDiffVal, short localWorst, int singular) {
    return (errCode == 0)
            & (badPixelNum < g_badPixelNum
                    && localBadPixelNum < g_localBadPixelNum
                    && avgDiffVal > g_avgDiffVal
                    && allTiltAngle < g_allTiltAngle
                    && blockTiltAngleMax < g_blockTiltAngleMax);
}

uint8_t Oswego_checkBadPointTestResult(token_map_t* result, uint32_t count) {
    if (checkBadPointTestResult(result, count)) {
        short avgDiffVal = 0;
        int badPixelNum = 0;
        int localBadPixelNum = 0;
        float allTiltAngle = 0;
        float blockTiltAngleMax = 0;

        int i = 0;

        for (i = 0; i < count; i++) {
            switch ((result + i)->token) {
            case TEST_TOKEN_AVG_DIFF_VAL:
                avgDiffVal = (result + i)->value.uint16_value;
                break;
            case TEST_TOKEN_BAD_PIXEL_NUM:
                badPixelNum = (result + i)->value.uint32_value;
                break;
            case TEST_TOKEN_LOCAL_BAD_PIXEL_NUM:
                localBadPixelNum = (result + i)->value.uint32_value;
                break;
            case TEST_TOKEN_ALL_TILT_ANGLE:
                allTiltAngle = (result + i)->value.float_value;
                break;
            case TEST_TOKEN_BLOCK_TILT_ANGLE_MAX:
                blockTiltAngleMax = (result + i)->value.float_value;
                break;
            default:
                break;
            }
        }

        if (Oswego_checkBadPointTestResult2(0, badPixelNum, localBadPixelNum,
                allTiltAngle, blockTiltAngleMax, avgDiffVal, (short) 0, 0)) {
            return 1;
        }
    }

    return 0;
}

static int TEST_ITEM_MILANA[] = { //
        TEST_SPI, /**/
        TEST_PIXEL, /**/
        TEST_RESET_PIN, /**/
        TEST_FW_VERSION, /**/
        TEST_PERFORMANCE, /**/
        /* TEST_BAD_POINT, */
        TEST_CAPTURE, /**/
        TEST_BIO_CALIBRATION, /**/
        TEST_ALGO /**/
        };

void initMilanASeriesChecker(int sensorType) {
    initThreshold(sensorType);
}

uint8_t MilanASeries_checkSpiTestResult2(int errCode, char* fwVersion,
        int sensorOtpType) {
        
	LOG_D(LOG_TAG, "[%s] zwq enter MilanASeries_checkSpiTestResult2,g_sensorType = %d", __func__,g_sensorType);
    if ((errCode != 0) || (fwVersion == NULL)) {
        return 0;
    }

    if (GF_SENSOR_5208 == g_sensorType) {
        if (sensorOtpType == TEST_MILAN_C_SENSOR_OTP_TYPE_1) {
            return strstr(fwVersion, TEST_FW_VERSION_SENSOR_TYPE_1_MILAN_C)
                    == fwVersion;
        } else if (sensorOtpType == TEST_MILAN_C_SENSOR_OTP_TYPE_2) {
            return strstr(fwVersion, TEST_FW_VERSION_SENSOR_TYPE_2_MILAN_C)
                    == fwVersion;
        } else if (sensorOtpType == TEST_MILAN_C_SENSOR_OTP_TYPE_3) {
            return strstr(fwVersion, TEST_FW_VERSION_SENSOR_TYPE_3_MILAN_C)
                    == fwVersion;
        } else if (sensorOtpType == TEST_MILAN_C_SENSOR_OTP_TYPE_4) {
            return strstr(fwVersion, TEST_FW_VERSION_SENSOR_TYPE_4_MILAN_C)
                    == fwVersion;
        } else if (sensorOtpType == TEST_MILAN_C_SENSOR_OTP_TYPE_5) {
            return strstr(fwVersion, TEST_FW_VERSION_SENSOR_TYPE_5_MILAN_C)
                    == fwVersion;
        }
    } else if (GF_SENSOR_5206 == g_sensorType) {
        if (sensorOtpType == TEST_MILAN_A_SENSOR_OTP_TYPE_1) {
            return strstr(fwVersion, TEST_FW_VERSION_SENSOR_TYPE_1_MILAN_A)
                    == fwVersion;
        } else if (sensorOtpType == TEST_MILAN_A_SENSOR_OTP_TYPE_2) {
            return strstr(fwVersion, TEST_FW_VERSION_SENSOR_TYPE_2_MILAN_A)
                    == fwVersion;
        } else if (sensorOtpType == TEST_MILAN_A_SENSOR_OTP_TYPE_3) {
            return strstr(fwVersion, TEST_FW_VERSION_SENSOR_TYPE_3_MILAN_A)
                    == fwVersion;
        } else if (sensorOtpType == TEST_MILAN_A_SENSOR_OTP_TYPE_4) {
            return strstr(fwVersion, TEST_FW_VERSION_SENSOR_TYPE_4_MILAN_A)
                    == fwVersion;
        } else if (sensorOtpType == TEST_MILAN_A_SENSOR_OTP_TYPE_5) {
            return strstr(fwVersion, TEST_FW_VERSION_SENSOR_TYPE_5_MILAN_A)
                    == fwVersion;
        }
    } else if (GF_SENSOR_5216 == g_sensorType) {
        if (sensorOtpType == TEST_MILAN_B_SENSOR_OTP_TYPE_1) {
            return strstr(fwVersion, TEST_FW_VERSION_SENSOR_TYPE_1_MILAN_B)
                    == fwVersion;
        } else if (sensorOtpType == TEST_MILAN_B_SENSOR_OTP_TYPE_2) {
            return strstr(fwVersion, TEST_FW_VERSION_SENSOR_TYPE_2_MILAN_B)
                    == fwVersion;
        } else if (sensorOtpType == TEST_MILAN_B_SENSOR_OTP_TYPE_3) {
            return strstr(fwVersion, TEST_FW_VERSION_SENSOR_TYPE_3_MILAN_B)
                    == fwVersion;
        } else if (sensorOtpType == TEST_MILAN_B_SENSOR_OTP_TYPE_4) {
            return strstr(fwVersion, TEST_FW_VERSION_SENSOR_TYPE_4_MILAN_B)
                    == fwVersion;
        } else if (sensorOtpType == TEST_MILAN_B_SENSOR_OTP_TYPE_5) {
            return strstr(fwVersion, TEST_FW_VERSION_SENSOR_TYPE_5_MILAN_B)
                    == fwVersion;
        } else if (sensorOtpType == TEST_MILAN_B_SENSOR_OTP_TYPE_6) {
            return strstr(fwVersion, TEST_FW_VERSION_SENSOR_TYPE_6_MILAN_B)
                    == fwVersion;
        } else if (sensorOtpType == TEST_MILAN_B_SENSOR_OTP_TYPE_7) {
            return strstr(fwVersion, TEST_FW_VERSION_SENSOR_TYPE_7_MILAN_B)
                    == fwVersion;
        } else if (sensorOtpType == TEST_MILAN_B_SENSOR_OTP_TYPE_8) {
            return strstr(fwVersion, TEST_FW_VERSION_SENSOR_TYPE_8_MILAN_B)
                    == fwVersion;
        } else if (sensorOtpType == TEST_MILAN_B_SENSOR_OTP_TYPE_9) {
            return strstr(fwVersion, TEST_FW_VERSION_SENSOR_TYPE_9_MILAN_B)
                    == fwVersion;
        }
    }

    return 0;
}

uint8_t MilanASeries_checkSpiTestResult(token_map_t* result, uint32_t count) {
    if (checkSpiTestResult(result, count)) {
        char* fwVersion = NULL;
        int sensorOtpType = 0;
        int i = 0;
				
		LOG_D(LOG_TAG, "[%s] zwq enter MilanASeries_checkSpiTestResult", __func__);

        for (i = 0; i < count; i++) {
            if (TEST_TOKEN_FW_VERSION == (result + i)->token) {
                fwVersion = (result + i)->value.buffer;
            } else if (TEST_TOKEN_SENSOR_OTP_TYPE == (result + i)->token) {
                sensorOtpType = (result + i)->value.uint32_value;
            }
        }

        LOG_D(LOG_TAG, "[%s] exit,version:%s,otp:%d", __func__, fwVersion,
                sensorOtpType);//GF5206_4.11.39  sensorOtpType:4
        if (MilanASeries_checkSpiTestResult2(0, fwVersion, sensorOtpType)) {
            return 1;
        }
    }

    return 0;
}

uint8_t MilanASeries_checkFwVersionTestResult(token_map_t* result,
        uint32_t count) {

    if (checkFwVersionTestResult(result, count)) {
        return MilanASeries_checkSpiTestResult(result, count);
    }

    return 0;
}

uint8_t MilanASeries_checkBioTestResultWithTouched2(int errCode, int base,
        int avg) {
    return (errCode == 0) && (abs(base - avg) >= g_osdTouchedMin)
            && (abs(base - avg) <= g_osdTouchedMax);
}

uint8_t MilanASeries_checkBioTestResultWithTouched(token_map_t* result,
        uint32_t count) {

    if (checkBioTestResultWithTouched(result, count)) {
        int baseValue = 0;
        int avgValue = 0;
        int i = 0;

        for (i = 0; i < count; i++) {
            if (TEST_TOKEN_HBD_BASE == (result + i)->token) {
                baseValue = (result + i)->value.uint32_value;
            } else if (TEST_TOKEN_HBD_AVG == (result + i)->token) {
                avgValue = (result + i)->value.uint32_value;
            }
        }

        if (MilanASeries_checkBioTestResultWithTouched2(0, baseValue,
                avgValue)) {
            return 1;
        }
    }

    return 0;
}

uint8_t MilanASeries_checkBioTestResultWithoutTouched2(int errCode, int base,
        int avg) {
    return (errCode == 0) && (abs(base - avg) <= g_osdUntouched);
}

uint8_t MilanASeries_checkBioTestResultWithoutTouched(token_map_t* result,
        uint32_t count) {

    if (checkBioTestResultWithTouched(result, count)) {
        int baseValue = 0;
        int avgValue = 0;
        int i = 0;

        for (i = 0; i < count; i++) {
            if (TEST_TOKEN_HBD_BASE == (result + i)->token) {
                baseValue = (result + i)->value.uint32_value;
            } else if (TEST_TOKEN_HBD_AVG == (result + i)->token) {
                avgValue = (result + i)->value.uint32_value;
            }
        }

        if (MilanASeries_checkBioTestResultWithoutTouched2(0, baseValue,
                avgValue)) {
            return 1;
        }
    }

    return 0;
}

static uint8_t MilanASeries_checkHBDTestResultWithTouched2(int errCode, int avg,
        int electricity) {
    return (errCode == 0) && (avg <= g_hbdAvgMax && avg >= g_hbdAvgMin)
            && (electricity >= g_hbdElectricityMin
                    && electricity <= g_hbdElectricityMax);
}

uint8_t MilanASeries_checkHBDTestResultWithTouched(token_map_t* result,
        uint32_t count) {
    if (checkErrcode(result, count)) {
        int avgValue = 0;
        int electricity = 0;
        int i = 0;

        for (i = 0; i < count; i++) {
            if (TEST_TOKEN_HBD_AVG == (result + i)->token) {
                avgValue = (result + i)->value.uint32_value;
            } else if (TEST_TOKEN_ELECTRICITY_VALUE == (result + i)->token) {
                electricity = (result + i)->value.uint32_value;
            }
        }

        if (MilanASeries_checkHBDTestResultWithTouched2(0, avgValue,
                electricity)) {
            return 1;
        }
    }

    return 0;
}

static int TEST_ITEM_MILAN_F_SERIES[] = { //
        TEST_SPI, /**/
        TEST_PIXEL, /**/
        TEST_RESET_PIN, /**/
        TEST_BAD_POINT, /**/
        TEST_PERFORMANCE, /**/
        TEST_CAPTURE, /**/
        TEST_ALGO /**/
        };

void initMilanFSeriesChecker(int sensorType) {
    initThreshold(sensorType);
}

uint8_t MilanFSeries_checkSpiTestResult2(int errCode, char* fwVersion,
        int chipId) {
    return (errCode == 0) && (chipId == g_chipId);
}

uint8_t MilanFSeries_checkSpiTestResult(token_map_t* result, uint32_t count) {
    if (checkSpiTestResult(result, count)) {
        int chipID = 0;
        int i = 0;
        char* chip = NULL;
        uint32_t size = 0;
        for (i = 0; i < count; i++) {
            if (TEST_TOKEN_CHIP_ID == (result + i)->token) {
                chip = (result + i)->value.buffer;
                size = (result + i)->buffer_size;
                break;
            }
        }

        if (chip != NULL && size >= 4) {
            chipID = decodeInt32((uint8_t*)chip, 0) >> 8;
        }

        if (MilanFSeries_checkSpiTestResult2(0, NULL, chipID)) {
            return 1;
        }
    }

    return 0;
}

uint8_t MilanFSeries_checkBadPointTestResult2(int errCode, int badPixelNum,
        int localBadPixelNum, float allTiltAngle, float blockTiltAngleMax,
        short avgDiffVal, short localWorst) {
    return (errCode == 0)
            && (badPixelNum < g_badPixelNum
                    && localBadPixelNum < g_localBadPixelNum
                    && localWorst < g_localWorst);
}

uint8_t MilanFSeries_checkBadPointTestResult(token_map_t* result,
        uint32_t count) {
    if (checkBadPointTestResult(result, count)) {

        int badPixelNum = 0;
        int localBadPixelNum = 0;
        short localWorst = 0;
        int i = 0;

        for (i = 0; i < count; i++) {
            if (TEST_TOKEN_BAD_PIXEL_NUM == (result + i)->token) {
                badPixelNum = (result + i)->value.uint32_value;
            } else if (TEST_TOKEN_LOCAL_BAD_PIXEL_NUM == (result + i)->token) {
                localBadPixelNum = (result + i)->value.uint32_value;
            } else if (TEST_TOKEN_LOCAL_WORST == (result + i)->token) {
                localWorst = (result + i)->value.uint16_value;
            }
        }

        if (MilanFSeries_checkBadPointTestResult2(0, badPixelNum,
                localBadPixelNum, 0, 0, (short) 0, localWorst)) {
            return 1;
        }
    }

    return 0;
}
