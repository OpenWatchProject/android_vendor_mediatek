/*
 * Copyright (C) 2013-2016, Shenzhen Huiding Technology Co., Ltd.
 * All Rights Reserved.
 */
#ifndef __GF_ALGO_H__
#define __GF_ALGO_H__

#include "gf_error.h"
#include "gf_type_define.h"
#include "gf_common.h"

#define GF_SET_CHIP_INFO(isFloating, isPixelCancel, isCoating, chipType, sensorRow, sensorCol) \
    ((isFloating) | ((isPixelCancel) << 1) | ((isCoating) << 2) | ((chipType) << 3)         \
    | ((sensorRow) << 14) | ((sensorCol) << 23))

#define SENSOR_ROW ((g_algo_config.flag_chip_info >> 14) & 0x1FF)
#define SENSOR_COL ((g_algo_config.flag_chip_info >> 23) & 0x1FF)

#define GF_MAX_TEMPLATE_UPDATE_COUNT_SAVE_THRESHOLD (20)
#define GF_MIN_TEMPLATE_UPDATE_COUNT_SAVE_THRESHOLD (5)

typedef enum {
    TYPE_AUTHENTICATE = 0,
    TYPE_ENROLL
} gf_algo_preprocess_type_t;

typedef enum {
    FINGER_STATUS_IDLE = 0,  // < Current finger is idle. can be used to register or add new finger
    FINGER_STATUS_READY,      // < Current finger is ready, which can be used to recognize
    FINGER_STATUS_ENROLLING,  // < Current finger is enrolling, can not be used to recognize
} gf_finger_status_t;

typedef struct {
    gf_chip_series_t chip_series;

    uint32_t max_fingers;
    uint32_t max_fingers_per_user;

    uint8_t support_bio_assay;

    uint32_t enrolling_min_templates;

    uint32_t valid_image_quality_threshold;
    uint32_t valid_image_area_threshold;
    uint32_t duplicate_finger_overlay_score;
    uint32_t increase_rate_between_stitch_info;

    uint32_t forbidden_enroll_duplicate_fingers;

    gf_authenticate_order_t authenticate_order;
    /**
     * configure fingerprint template study update save threshold
    */
    uint32_t template_update_save_threshold;
} gf_algo_basic_config_t;

typedef struct gf_algo_config {
    int32_t flag_chip_info;  // < chip information that used by algorithm.
    int32_t thr_select_bmp;

    uint8_t chip_id[GF_CHIP_ID_LEN];
    uint8_t vendor_id[GF_VENDOR_ID_LEN];
    uint8_t sensor_id[GF_SENSOR_ID_LEN];

    uint8_t otp_info[GF_SENSOR_OTP_INFO_LEN];

    gf_algo_basic_config_t basic_config;
} gf_algo_config_t;

#ifdef __cplusplus
extern "C" {
#endif

/**
 * [gf_algo_init_global_variable]:Initialize the global variable in this file
 */
void gf_algo_init_global_variable(void);

void gf_algo_update_config(gf_algo_basic_config_t *config);

/**
 * \brief Initialize algorithm module.
 *
 * \param config[in]    The configuration for algorithm module.
 */
gf_error_t gf_algo_init(gf_algo_config_t *config);

/**
 * \brief Unload algorithm module.
 */
void gf_algo_destroy(void);

/**
 * \brief Query the status for the specified finger.
 *
 * \param group_id[in]  The user identifier.
 *
 * \param finger_id[in] The identifier for the specified finger.
 *
 * \param status[out]   Used to save the finger's status.
 *
 * \return If success, return #GF_SUCCESS and save the status into #status. otherwise return error code,eg:
 *  \li \c #GF_ERROR_BAD_PARAMS
 */
gf_error_t gf_algo_get_finger_status(uint32_t group_id, uint32_t finger_id,
        gf_finger_status_t *status);

/**
 * \brief Set the status for the specified finger.
 *
 * \param group_id[in]  The user identifier.
 *
 * \param finger_id[in] The identifier for the specified finger.
 *
 * \param status[out]   The finger's status.
 *
 * \return If success, return #GF_SUCCESS. otherwise return error code,eg:
 *  \li \c #GF_ERROR_BAD_PARAMS
 */
gf_error_t gf_algo_set_finger_status(uint32_t group_id, uint32_t finger_id,
        gf_finger_status_t status);

/**
 * \brief Get the data length of the specified finger.
 *
 * \param finger_id[in] The identifier of the finger.
 *
 * \param len[out]  The length of finger data.
 *
 * \return if success, return #GF_SUCCESS and set the length to #len.otheriwser return error code,
 * eg:
 *  \li \c #GF_ERROR_FINGER_NOT_EXIST
 */
gf_error_t gf_algo_get_fingerTempInfo_size(uint32_t finger_id, uint32_t *len);

/**
 * \brief Update B value for calibration parameter.
 *
 * \param buf[in] The beginning address of base value.
 *
 * \param buf_len[in] The length of buf.
 *
 * \return If success, return #GF_SUCCESS, otherwise return error code, eg:
 *  \li \c #GF_ERROR_BAD_PARAMS
 *  \li \c #GF_ERROR_PREPROCESS_FAILED
 */
gf_error_t gf_algo_calibration(uint16_t *buf, uint32_t buf_len);
gf_error_t gf_algo_test_real_time_data(uint16_t *buf, uint32_t buf_len,
        gf_test_real_time_data_t *test_real_time_data);
gf_error_t gf_algo_test_bmp_data(uint16_t *buf, uint32_t buf_len,
        gf_test_bmp_data_t *test_bmp_data);

/**
 * \brief Initialize the Kr and frameNum for calibration parameters.
 */
void gf_algo_init_calibration_kr_and_framenum(void);

/**
 * \brief Initialize the calibration parameters from the saved data.
 * Note: MILAN F will decode B value from so. Other chips won't.
 *
 * \param buf[in] The pointer of calibration parameters's data.
 *
 * \param buf_len[in] The length of calibration parameters's data.
 *
 * \return If success, return #GF_SUCCESS, otherwise return error code, eg:
 *  \li \c #GF_ERROR_BAD_PARAMS
 *  \li \c #GF_ERROR_INVALID_DATA
 *  \li \c #GF_ERROR_INVALID_PREPROCESS_VERSION
 */
gf_error_t gf_algo_load_calibration(uint8_t *buf, uint32_t buf_len);

/**
 * \brief Initialize the B field for calibration parameter.
 *
 * \param buf[in] The pointer of calibration parameters's data.
 *
 * \param buf_len[in] The length of calibration parameters's data.
 *
 * \return If success, return #GF_SUCCESS, otherwise return error code,eg:
 *  \li \c #GF_ERROR_BAD_PARAMS
 *  \li \c #GF_ERROR_INVALID_PREPROCESS_VERSION
 *  \li \c #GF_ERROR_INVALID_DATA
 */
gf_error_t gf_algo_load_calibration_b(uint8_t *buf, uint32_t buf_len);

/**
 * \brief Get the calibration data to save.
 *
 * \param buf[out]  The beginning address for calibration data, It is allocated
 *  by this API,and should release by caller.
 *
 * \param buf_len   The length of calibraiton data.
 *
 * \return If success, return #GF_SUCCESS, otherwise return error code, eg:
 *  \li \c #GF_ERROR_BAD_PARAMS
 *  \li \c #GF_ERROR_PREPROCESS_FAILED
 */
gf_error_t gf_algo_save_calibration(uint8_t **buf, uint32_t *buf_len);

/**
 * \brief Set the safe class for algorithm module. Safe class will impact FAR and FRR.
 *
 * \param safe_class[in] Algorithm's safe class.
 *
 * \return If success, return #GF_SUCCESS, otherwise return #GF_ERROR_BAD_PARAMS
 */
gf_error_t gf_algo_set_safe_class(gf_safe_class_t safe_class);


/**
 * \brief Get the current safe class.
 *
 * \return #gf_safe_class_t.
 */
gf_safe_class_t gf_algo_get_safe_class(void);

/**
 * \brief preprocess the sampled image.
 *
 * \param buf[in]   The raw data buffer.
 *
 * \param buf_len[in]   The length of raw data, Note: it should be the count of uint16_t data, not uint8_t data.
 *
 * \param frame_num[in]  If use multiple images to select better one to use.
 *
 * \param select_index[out]  which image is better.
 *
 * \param operation[in] The current fingerprint operation type.
 *
 * \param image_quality[out]    The the image quality of the preprocessed image.
 *
 * \param valid_area[out]   The valid area of the preprocessed image.
 *
 * \return If success, return $GF_SUCCESS, otherwise return error code,eg:
 *  \li \c #GF_ERROR_BAD_PARAMS
 *  \li \c #GF_ERROR_PREPROCESS_FAILED
 *  \li \c #GF_ERROR_ACQUIRED_IMAGER_DIRTY
 *  \li \c #GF_ERROR_ACQUIRED_PARTIAL
 */
gf_error_t gf_algo_get_acquired_info(uint16_t *buf, uint32_t buf_len, uint8_t frame_num,
        int32_t *select_index, gf_operation_type_t operation, int32_t *image_quality,
        int32_t *valid_area);

/**
 * \brief Prepare to enroll operation.
 *
 * \param group_id[in]  The user group identifier
 *
 * \return If success, return #GF_SUCCESS. otherwise return error code.eg:
 *  \li \c #GF_ERROR_REACH_FINGERS_UPLIMIT
 *  \li \c #GF_ERROR_OUT_OF_MEMORY
 */
gf_error_t gf_algo_pre_enroll(uint32_t group_id);

gf_error_t gf_algo_enroll(int32_t *live_data, uint32_t live_data_len, uint32_t *group_id,
        uint32_t *finger_id, uint16_t *samples_remaining, uint32_t *duplicate_finger_id);

/**
 * \brief authenticate if the current finger has enrolled.
 *
 * \param study_enable_flag[in] enable or disable study feature
 *
 * \param live_data[in]
 *
 * \param live_data_len[in]
 *
 * \param group_id[out]
 *
 * \param finger_id[out]
 *
 * \param save_flag[out]
 *
 * \param update_stitch_flag[out]
 *
 * \param image_quality[out]
 *
 * \param valid_area[out]
 *
 * \param match_score[out] range is 0-100,it could be NULL, because if only needed by fido
 *
 * \return If success, return #GF_SUCCESS, and the successfully matched finger id. otherwise return
 * error code, eg:
 *  \li \c #GF_ERROR_BAD_PARAMS
 *  \li \c #GF_ERROR_ACQUIRED_IMAGER_DIRTY
 *  \li \c #GF_ERROR_NOT_MATCH
 */
gf_error_t gf_algo_authenticate(int32_t *live_data, uint32_t live_data_len,
        int32_t study_enable_flag, uint32_t *group_id, uint32_t *finger_id, uint8_t *save_flag,
        uint8_t *update_stitch_flag, int32_t *image_quality, int32_t *valid_area,
        int32_t *match_score);

gf_error_t gf_algo_finger_feature_study(uint32_t finger_id, uint8_t *save_flag,
        uint8_t *update_stitch_flag);

void gf_algo_cancel_enroll(void);
void gf_algo_cancel_authenticate(void);

/**
 * \brief Get all finger list for the current user.
 *
 * \param group_ids[inout]  Return the crrent group id.
 *
 * \param finger_ids[inout] The memory address to save all fingers'ID
 *
 * \param size[inout]   used to save the count of fingers for the current user.
 *
 * \return If success, return #GF_SUCCESS. otherwise return error code, eg:
 *  \li \c #GF_ERROR_BAD_PARAMS
 */
gf_error_t gf_algo_enumerate(uint32_t *group_ids, uint32_t *finger_ids, uint32_t *size);

/**
 * \brief Add registered finger into algorithm module.
 *
 * \param buf[in]   The beginning address of finger buffer.
 *
 * \param len[in]   The length of finger buffer.
 *
 * \param group_id[in]  The user identifier.
 *
 * \param finger_id[out]    The identifier of this finger.
 *
 * \return If success, return #GF_SUCCESS.
 */
gf_error_t gf_algo_add(uint8_t *buf, uint32_t len, uint32_t group_id, uint32_t *finger_id);

/**
 * \brief Get the binary data for the specified finger.
 *
 * \param group_id[in]  The user identifier.
 *
 * \param finger_id[in] The finger's identifier.
 *
 * \param buf[inout]  The memory address which used to save the binanry data for finger.
 *
 * \param len[in]  The length of buf.
 *
 * \return If success, return #GF_SUCCESS, otherwise return error code, eg:
 *  \li \c #GF_ERROR_BAD_PARAMS
 *  \li \c #GF_ERROR_FINGER_NOT_EXIST
 */
gf_error_t gf_algo_get_finger_pdu(uint32_t group_id, uint32_t finger_id, uint8_t *buf,
        uint32_t len);

gf_error_t gf_algo_update_stitch(uint32_t group_id, uint32_t finger_id);

gf_error_t gf_algo_remove(uint32_t group_id, uint32_t finger_id);

/**
 * \brief Switch fingerprint user.
 *
 * \param group_id[in]  The user id for new user.
 */
gf_error_t gf_algo_set_active_group(uint32_t group_id);

gf_error_t gf_algo_get_preprocess_version(int8_t *preprocess_version);

gf_error_t gf_algo_test_performance(uint16_t *buf, uint32_t buf_len,
        gf_test_performance_t *test_performance);
void gf_algo_cancel_test_performance(void);

gf_error_t gf_algo_test_get_algo_version(int8_t *algo_version, int8_t *preprocess_version);

/**
 * \brief Debug function that used to dump the current calibration parameters to debug from algorithm module.
 *
 * \param operation[in] The operation type
 *
 * \param base[out] #gf_dump_data_t structure pointer, that used to save base frame debug data.
 *
 * \return return #GF_SUCCESS, if success, otherwise return error code.
 */
gf_error_t gf_algo_dump_algo_debug_info(gf_operation_type_t operation, gf_image_data_t *base);

// for frr far test
gf_error_t gf_frr_far_init(uint32_t safe_class, uint32_t template_num, uint32_t support_bio_assay);
gf_error_t gf_frr_far_calibration(uint16_t *buf, uint32_t buf_len);
gf_error_t gf_frr_far_enroll(uint16_t *buf, uint32_t buf_len, gf_test_frr_far_t *test_far_frr);
gf_error_t gf_frr_far_play_enroll(gf_test_frr_far_t *test_frr_far);
gf_error_t gf_frr_far_authenticate_with_greedy_strategy(uint16_t *buf, uint32_t buf_len,
        gf_test_frr_far_t *test_frr_far);
gf_error_t gf_frr_far_authenticate_without_greedy_strategy(uint16_t *buf, uint32_t buf_len,
        gf_test_frr_far_t *test_frr_far);
gf_error_t gf_frr_far_authenticate(uint16_t *buf, uint32_t buf_len,
        gf_test_frr_far_t *test_frr_far);
gf_error_t gf_frr_far_dump_info(gf_test_frr_far_t *test);

void gf_frr_far_delete_tran(void);
void gf_frr_far_cancel(void);

// for pixcel test
gf_error_t gf_pixel_test_step1(uint16_t *buf, uint32_t buf_len);
gf_error_t gf_pixel_test_step2(uint16_t *buf, uint32_t buf_len);
gf_error_t gf_oswego_m_pixel_test(uint32_t *bad_pixel_num);
gf_error_t gf_milan_pixel_test(uint32_t *bad_pixel_num);
gf_error_t gf_pixel_test(uint32_t *bad_pixel_num);
void gf_pixel_test_cancel(void);

// for sensor fine test
gf_error_t gf_sensor_fine_test_step1(uint16_t *buf, uint32_t buf_len);
gf_error_t gf_sensor_fine_test_step2(uint16_t *buf, uint32_t buf_len);
gf_error_t gf_sensor_fine_test(uint32_t *pixel_average_diff);
void gf_sensor_fine_test_cancel(void);

// for test enroll
gf_error_t gf_algo_test_pre_enroll(void);
gf_error_t gf_algo_test_enroll(uint32_t *group_id, uint32_t *finger_id,
        uint16_t *samples_remaining, uint32_t *duplicate_finger_id);
void gf_algo_cancel_test_enroll(void);

gf_error_t gf_algo_test_authenticate(int32_t study_enable_flag, uint32_t *group_id,
        uint32_t *finger_id, int32_t *image_quality, int32_t *valid_area, int32_t *match_score);
void gf_algo_cancel_test_authenticate(void);

void gf_algo_test_remove(uint32_t group_id, uint32_t finger_id);

/**
 * \brief query the state of algorithm module's calibration.
 *
 * \return Greater than zero, that means calibration success. otherwise fail.
 */
int32_t gf_algo_get_calibration_state(void);

/**
 * \brief reset the state of algorithm module's calibration.
 */
void gf_algo_reset_calibration_state(void);

/**
 * \brief query the times of successfully preprocess operation.
 *
 * \return the times of successfully preprocess.
 */
int32_t gf_algo_get_preprocess_times(void);

/**
 * \brief Check the sensor broken status
 *
 * \param raw_data[in] frame data
 *
 * \param base[in] base data
 *
 * \param white_set[out] flag matrix of setting white points
 *
 *\param broken_pixel_num[out] number of broken pixels.
 *\
 * \return If success, return #GF_SUCCESS,otherwise return error code, eg:
 *  \li \c #GF_ERROR_BAD_PARAMS
 */
gf_error_t gf_algo_check_sensor_broken(uint16_t* raw_data, uint16_t *base, uint8_t* white_set,
        int32_t* broken_pixel_num);


gf_error_t gf_algo_get_last_identify_id(uint32_t *id, uint32_t timeout);

/**
 * Determine if there is at least one fingerprint enrolled.
 *
 * @return 1 if at least one fingerprint is enrolled, 0 otherwise
 */
uint8_t gf_algo_has_enrolled_fingerprints(void);

/**
* \brief Set base frame raw data to B of struct calibrationPara.
*
* \param base_b[in] Point to base frame raw data.
*
* \param base_b_len[in] The length of raw data in uint16_t.
*
* \return If success, return #GF_SUCCESS. If fail, return #GF_ERROR_BAD_PARAMS/
*/
gf_error_t gf_algo_set_base_b(uint16_t *base_b, uint32_t base_b_len);

/**
 * \brief Get the initialized base frame from algorithm module.
 *
 * \param base_b[out] Point to base frame raw data, Note: don't free this buffer.
 *
 * \param base_b_len[out] The length of raw data in bytes, not uint16_t.
 *
 * \return If success, return #GF_SUCCESS. If fail, return #GF_ERROR_BAD_PARAMS/
 */
gf_error_t gf_algo_get_base_b(uint16_t **base_b, uint32_t *base_b_len);

/**
*
* When base is invalid ,backup the base calibration
* return GF_ERROR_OUT_OF_MEMORY if failure
**/
int32_t gf_algo_get_bad_raw_data_num(uint16_t *raw_data, uint32_t row, uint32_t colume);
int32_t gf_algo_get_raw_data_mean(uint16_t *pData, uint32_t row, uint32_t colume);
void gf_algo_print_mean_data(uint16_t *raw_data, uint32_t row, uint32_t colume);

#ifdef GF_CHIP_SERIES_MILAN_A
void gf_algo_backup_calibration_params(void);
void gf_algo_recovery_Kr_and_frameNum(void);
void gf_algo_recovery_calibration_params(void);
void gf_algo_backup_kr(void);
#endif
int32_t gf_algo_get_preprocess_fail_times(void);

/**
 * Get the token and len define for upgrade finger data from old QSEE
 */
uint8_t get_crc32_token(void);

uint8_t get_crc32_len(void);

uint8_t get_finger_id_token(void);

uint8_t get_finger_weight_token(void);

uint8_t get_finger_group_id_token(void);

uint8_t get_finger_headers_len(void);

#ifdef __cplusplus
}
#endif

#endif  // __GF_ALGO_H__
