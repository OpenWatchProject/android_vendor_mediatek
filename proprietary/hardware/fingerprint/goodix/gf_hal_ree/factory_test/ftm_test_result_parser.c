#include "ftm_factory_test.h"
#include "gf_error.h"
#include <string.h>
#include <stdlib.h>


#define LOG_TAG "[gf_test_result_parser]"
token_map_t g_token_map[MAX_SIZE];
uint32_t g_token_count = 0;

int g_SensorType = GF_SENSOR_318M;
int g_MaxFingers = 32;
int g_MaxFingersPerUser = 5;
int g_SupportKeyMode = 0;
int g_SupportFFMode = 1;
int g_SupportPowerKeyFeature = 0;
int g_ForbiddenUntrustedEnroll = 0;
int g_ForbiddenEnrollDuplicateFingers = 0;
int g_SupportBioAssay = 0;
int g_SupportPerformanceDump = 0;

int g_SupportNavMode = GF_NAV_MODE_NONE;

int g_EnrollingMinTemplates = 8;

int g_ValidImageQualityThreshold = 15;
int g_ValidImageAreaThreshold = 65;
int g_DuplicateFingerOverlayScore = 70;
int g_IncreaseRateBetweenStitchInfo = 15;

int g_SupportImageRescan = 1;
int g_RescanImageQualityThreshold = 10;
int g_RescanImageAreaThreshold = 60;
int g_RescanRetryCount = 1;

int g_ScreenOnAuthenticateFailRetryCount = 1;
int g_ScreenOffAuthenticateFailRetryCount = 1;

int g_ReissueKeyDownWhenEntryFfMode = 0;
int g_ReissueKeyDownWhenEntryImageMode = 1;

int g_SupportSensorBrokenCheck = 0;
int g_BrokenPixelThresholdForDisableSensor = 600;
int g_BrokenPixelThresholdForDisableStudy = 300;

int g_BadPointTestMaxFrameNumber = 2;

int g_ReportKeyEventOnlyEnrollAuthenticate = 0;

uint32_t decodeInt32(uint8_t* result, uint32_t offset) {
    return (result[offset] & 0xff) | ((result[offset + 1] & 0xff) << 8)
            | ((result[offset + 2] & 0xff) << 16)
            | ((result[offset + 3] & 0xff) << 24);
}

uint16_t decodeInt16(uint8_t* result, uint32_t offset) {
    return (uint16_t) ((result[offset] & 0xff) | ((result[offset + 1] & 0xff) << 8));
}

uint8_t decodeInt8(uint8_t* result, uint32_t offset) {
    return result[offset];
}

float decodeFloat(uint8_t* result, uint32_t offset, uint32_t size) {
    uint32_t value = (result[offset] & 0xff) | ((result[offset + 1] & 0xff) << 8)
            | ((result[offset + 2] & 0xff) << 16)
            | ((result[offset + 3] & 0xff) << 24);
    float fvalue = 0;

    memcpy(&fvalue,&value,sizeof(value));
    return fvalue;
}

double decodeDouble(uint8_t* result, uint32_t offset, uint32_t size) {
    uint64_t  value = (result[offset] & 0xff)
            | (((uint64_t) result[offset + 1] & 0xff) << 8)
            | (((uint64_t) result[offset + 2] & 0xff) << 16)
            | (((uint64_t) result[offset + 3] & 0xff) << 24)
            | (((uint64_t) result[offset + 4] & 0xff) << 32)
            | (((uint64_t) result[offset + 5] & 0xff) << 40)
            | (((uint64_t) result[offset + 6] & 0xff) << 48)
            | (((uint64_t) result[offset + 7] & 0xff) << 56);
    double dvalue = 0;
    memcpy(&dvalue,&value,sizeof(value));
    return dvalue;
}

void parser(uint8_t* result,uint32_t len) {
    uint32_t offset = 0;
    uint32_t count = 0;

    LOG_D(LOG_TAG, "[%s] enter", __func__);
    g_token_count = 0;
    memset(&g_token_map,0,sizeof(g_token_map));

    for (offset = 0; offset < len;) {

        uint32_t token = decodeInt32(result, offset);
        offset += 4;
        count++;

        switch (token) {
            case TEST_TOKEN_ERROR_CODE:
            case TEST_TOKEN_BAD_PIXEL_NUM:
            case TEST_TOKEN_LOCAL_BAD_PIXEL_NUM:
            case TEST_TOKEN_GET_DR_TIMESTAMP_TIME:
            case TEST_TOKEN_GET_MODE_TIME:
            case TEST_TOKEN_GET_CHIP_ID_TIME:
            case TEST_TOKEN_GET_VENDOR_ID_TIME:
            case TEST_TOKEN_GET_SENSOR_ID_TIME:
            case TEST_TOKEN_GET_FW_VERSION_TIME:
            case TEST_TOKEN_GET_IMAGE_TIME:
            case TEST_TOKEN_RAW_DATA_LEN:
            case TEST_TOKEN_IMAGE_QUALITY:
            case TEST_TOKEN_VALID_AREA:
            case TEST_TOKEN_KEY_POINT_NUM:
            case TEST_TOKEN_INCREATE_RATE:
            case TEST_TOKEN_OVERLAY:
            case TEST_TOKEN_GET_RAW_DATA_TIME:
            case TEST_TOKEN_PREPROCESS_TIME:
            case TEST_TOKEN_ALGO_START_TIME:
            case TEST_TOKEN_GET_FEATURE_TIME:
            case TEST_TOKEN_ENROLL_TIME:
            case TEST_TOKEN_AUTHENTICATE_TIME:
            case TEST_TOKEN_AUTHENTICATE_ID:
            case TEST_TOKEN_AUTHENTICATE_UPDATE_FLAG:
            case TEST_TOKEN_AUTHENTICATE_FINGER_COUNT:
            case TEST_TOKEN_AUTHENTICATE_FINGER_ITME:
            case TEST_TOKEN_TOTAL_TIME:
            case TEST_TOKEN_RESET_FLAG:
            case TEST_TOKEN_SINGULAR:
            case TEST_TOKEN_SENSOR_TYPE:
            case TEST_TOKEN_MAX_FINGERS:
            case TEST_TOKEN_MAX_FINGERS_PER_USER:
            case TEST_TOKEN_SUPPORT_KEY_MODE:
            case TEST_TOKEN_SUPPORT_FF_MODE:
            case TEST_TOKEN_SUPPORT_POWER_KEY_FEATURE:
            case TEST_TOKEN_FORBIDDEN_UNTRUSTED_ENROLL:
            case TEST_TOKEN_FORBIDDEN_ENROLL_DUPLICATE_FINGERS:
            case TEST_TOKEN_SUPPORT_BIO_ASSAY:
            case TEST_TOKEN_SUPPORT_PERFORMANCE_DUMP:
            case TEST_TOKEN_SUPPORT_NAV_MODE:
            case TEST_TOKEN_ENROLLING_MIN_TEMPLATES:
            case TEST_TOKEN_VALID_IMAGE_QUALITY_THRESHOLD:
            case TEST_TOKEN_VALID_IMAGE_AREA_THRESHOLD:
            case TEST_TOKEN_DUPLICATE_FINGER_OVERLAY_SCORE:
            case TEST_TOKEN_INCREASE_RATE_BETWEEN_STITCH_INFO:
            case TEST_TOKEN_SCREEN_ON_AUTHENTICATE_FAIL_RETRY_COUNT:
            case TEST_TOKEN_SCREEN_OFF_AUTHENTICATE_FAIL_RETRY_COUNT:
            case TEST_TOKEN_AUTHENTICATE_ORDER:
            case TEST_TOKEN_REISSUE_KEY_DOWN_WHEN_ENTRY_FF_MODE:
            case TEST_TOKEN_REISSUE_KEY_DOWN_WHEN_ENTRY_IMAGE_MODE:
            case TEST_TOKEN_SUPPORT_SENSOR_BROKEN_CHECK:
            case TEST_TOKEN_BROKEN_PIXEL_THRESHOLD_FOR_DISABLE_SENSOR:
            case TEST_TOKEN_BROKEN_PIXEL_THRESHOLD_FOR_DISABLE_STUDY:
            case TEST_TOKEN_BAD_POINT_TEST_MAX_FRAME_NUMBER:
            case TEST_TOKEN_REPORT_KEY_EVENT_ONLY_ENROLL_AUTHENTICATE:
            case TEST_TOKEN_GET_GSC_DATA_TIME:
            case TEST_TOKEN_BIO_ASSAY_TIME:
            case TEST_TOKEN_SENSOR_CHECK:
            case TEST_TOKEN_ALGO_INDEX:
            case TEST_TOKEN_SAFE_CLASS:
            case TEST_TOKEN_TEMPLATE_COUNT:
            case TEST_TOKEN_ELECTRICITY_VALUE:
            case TEST_TOKEN_LOCAL_SMALL_BAD_PIXEL_NUM:
            case TEST_TOKEN_LOCAL_BIG_BAD_PIXEL_NUM:
            case TEST_TOKEN_FLATNESS_BAD_PIXEL_NUM: {
                uint32_t value = decodeInt32(result, offset);
                offset += 4;
                g_token_map[count-1].token = token;
                g_token_map[count-1].value.uint32_value = value;

                break;
            }

            case TEST_TOKEN_ALGO_VERSION:
            case TEST_TOKEN_PREPROCESS_VERSION:
            case TEST_TOKEN_FW_VERSION:
            case TEST_TOKEN_TEE_VERSION:
            case TEST_TOKEN_TA_VERSION:
            case TEST_TOKEN_GSC_DATA: {
                uint32_t size = decodeInt32(result, offset);
                offset += 4;

                if (size <= MAX_LENGTH ){
                    g_token_map[count-1].token = token;
                    memcpy(g_token_map[count-1].value.buffer, (result + offset),size);
                    g_token_map[count-1].buffer_size = size;
                }

                offset += size;
                break;
            }

            case TEST_TOKEN_CHIP_ID:
            case TEST_TOKEN_VENDOR_ID:
            case TEST_TOKEN_SENSOR_ID:
            case TEST_TOKEN_PRODUCTION_DATE:{
                uint32_t size = decodeInt32(result, offset);
                offset += 4;

                if (size <= MAX_LENGTH ){
                    g_token_map[count-1].token = token;
                    memcpy(g_token_map[count-1].value.buffer, (result + offset),size);
                    g_token_map[count-1].buffer_size = size;
                }

                offset += size;
                break;
            }
            case TEST_TOKEN_RAW_DATA:
            case TEST_TOKEN_BMP_DATA:
            case TEST_TOKEN_HBD_RAW_DATA: {
                //data too long,do nothing
                break;
            }

            case TEST_TOKEN_ALL_TILT_ANGLE:
            case TEST_TOKEN_BLOCK_TILT_ANGLE_MAX: {
                uint32_t size = decodeInt32(result, offset);
                offset += 4;
                float value = decodeFloat(result, offset, size);

                g_token_map[count-1].token = token;
                g_token_map[count-1].value.float_value = value;

                offset += size;

                break;
            }

            case TEST_TOKEN_NOISE: {
                uint32_t size = decodeInt32(result, offset);
                offset += 4;
                double value = decodeDouble(result, offset, size);

                g_token_map[count-1].token = token;
                g_token_map[count-1].value.double_value = value;

                offset += size;

                break;
            }

            case TEST_TOKEN_AVG_DIFF_VAL:
            case TEST_TOKEN_LOCAL_WORST:
            case TEST_TOKEN_IN_CIRCLE:
            case TEST_TOKEN_BIG_BUBBLE:
            case TEST_TOKEN_LINE:
            case TEST_TOKEN_HBD_BASE:
            case TEST_TOKEN_HBD_AVG: {
                short value = decodeInt16(result, offset);
                offset += 2;

                g_token_map[count-1].token = token;
                g_token_map[count-1].value.uint16_value = value;
                break;
            }

            case TEST_TOKEN_SENSOR_OTP_TYPE:
            case TEST_TOKEN_IS_BAD_LINE: {
                uint8_t value = decodeInt8(result, offset);
                offset += 1;
                g_token_map[count-1].token = token;
                g_token_map[count-1].value.uint8_value = value;
                break;
            }
            default:
                count--;
                break;
        }
    }

    g_token_count = count;
    LOG_D(LOG_TAG, "[%s] exit", __func__);
}


gf_error_t update_config(token_map_t* result,uint32_t count){
    int i = 0;
    gf_error_t err = GF_SUCCESS;

    for (i = 0; i < count; i++){
        switch ( (result + i)->token){
        case TEST_TOKEN_ERROR_CODE:
            err  = (result + i)->value.uint32_value;
            if (err != GF_SUCCESS){
                return err;
            }
            break;
        case TEST_TOKEN_SENSOR_TYPE :
            g_SensorType =  (result + i)->value.uint32_value;
            break;
        case TEST_TOKEN_MAX_FINGERS :
            g_MaxFingers =  (result + i)->value.uint32_value;
            break;
        case TEST_TOKEN_MAX_FINGERS_PER_USER:
            g_MaxFingersPerUser = (result + i)->value.uint32_value;
            break;
        case TEST_TOKEN_SUPPORT_KEY_MODE :
            g_SupportKeyMode = (result + i)->value.uint32_value;
            break;
        case TEST_TOKEN_SUPPORT_FF_MODE:
            g_SupportFFMode = (result + i)->value.uint32_value;
            break;
        case TEST_TOKEN_SUPPORT_POWER_KEY_FEATURE:
            g_SupportPowerKeyFeature = (result + i)->value.uint32_value;
            break;
        case TEST_TOKEN_FORBIDDEN_UNTRUSTED_ENROLL:
            g_ForbiddenUntrustedEnroll = (result + i)->value.uint32_value;
            break;
        case TEST_TOKEN_FORBIDDEN_ENROLL_DUPLICATE_FINGERS:
            g_ForbiddenEnrollDuplicateFingers= (result + i)->value.uint32_value;
            break;
        case TEST_TOKEN_SUPPORT_BIO_ASSAY:
            g_SupportBioAssay = (result + i)->value.uint32_value;
            break;
        case TEST_TOKEN_SUPPORT_PERFORMANCE_DUMP:
            g_SupportPerformanceDump = (result + i)->value.uint32_value;
            break;
        case TEST_TOKEN_SUPPORT_NAV_MODE :
            g_SupportNavMode = (result + i)->value.uint32_value;
            break;
        case TEST_TOKEN_ENROLLING_MIN_TEMPLATES:
            g_EnrollingMinTemplates = (result + i)->value.uint32_value;
            break;
        case TEST_TOKEN_VALID_IMAGE_QUALITY_THRESHOLD:
            g_ValidImageQualityThreshold = (result + i)->value.uint32_value;
            break;
        case TEST_TOKEN_VALID_IMAGE_AREA_THRESHOLD:
            g_ValidImageAreaThreshold = (result + i)->value.uint32_value;
            break;
        case TEST_TOKEN_DUPLICATE_FINGER_OVERLAY_SCORE:
            g_DuplicateFingerOverlayScore = (result + i)->value.uint32_value;
            break;
        case TEST_TOKEN_INCREASE_RATE_BETWEEN_STITCH_INFO:
            g_IncreaseRateBetweenStitchInfo = (result + i)->value.uint32_value;
            break;
        case TEST_TOKEN_SCREEN_ON_AUTHENTICATE_FAIL_RETRY_COUNT:
            g_ScreenOnAuthenticateFailRetryCount = (result + i)->value.uint32_value;
            break;
        case TEST_TOKEN_SCREEN_OFF_AUTHENTICATE_FAIL_RETRY_COUNT:
            g_ScreenOffAuthenticateFailRetryCount = (result + i)->value.uint32_value;
            break;
        case TEST_TOKEN_REISSUE_KEY_DOWN_WHEN_ENTRY_FF_MODE:
            g_ReissueKeyDownWhenEntryFfMode = (result + i)->value.uint32_value;
            break;
        case TEST_TOKEN_REISSUE_KEY_DOWN_WHEN_ENTRY_IMAGE_MODE :
            g_ReissueKeyDownWhenEntryImageMode = (result + i)->value.uint32_value;
            break;
        case TEST_TOKEN_SUPPORT_SENSOR_BROKEN_CHECK:
            g_SupportSensorBrokenCheck = (result + i)->value.uint32_value;
            break;
        case TEST_TOKEN_BROKEN_PIXEL_THRESHOLD_FOR_DISABLE_SENSOR:
            g_BrokenPixelThresholdForDisableSensor = (result + i)->value.uint32_value;
            break;
        case TEST_TOKEN_BROKEN_PIXEL_THRESHOLD_FOR_DISABLE_STUDY:
            g_BrokenPixelThresholdForDisableStudy = (result + i)->value.uint32_value;
            break;
        case TEST_TOKEN_BAD_POINT_TEST_MAX_FRAME_NUMBER:
            g_BadPointTestMaxFrameNumber = (result + i)->value.uint32_value;
            break;
        case TEST_TOKEN_REPORT_KEY_EVENT_ONLY_ENROLL_AUTHENTICATE:
            g_ReportKeyEventOnlyEnrollAuthenticate = (result + i)->value.uint32_value;
            break;
        default:
            break;
        }
    }

    if (g_MaxFingers < g_MaxFingersPerUser) {
        g_MaxFingers = g_MaxFingersPerUser;
    }

    return err;
}
