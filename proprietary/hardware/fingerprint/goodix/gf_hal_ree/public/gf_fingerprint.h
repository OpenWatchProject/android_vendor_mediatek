#ifndef GF_FINGERPRINT_H
#define GF_FINGERPRINT_H

#include "gf_type_define.h"

typedef enum gf_fingerprint_msg_type {
    GF_FINGERPRINT_ERROR = -1,
    GF_FINGERPRINT_ACQUIRED = 1,
    GF_FINGERPRINT_TEMPLATE_ENROLLING = 3,
    GF_FINGERPRINT_TEMPLATE_REMOVED = 4,
    GF_FINGERPRINT_AUTHENTICATED = 5,
    GF_FINGERPRINT_ENUMERATED = 6,
    GF_FINGERPRINT_TEST_CMD = 7,
    GF_FINGERPRINT_HBD = 8,
} gf_fingerprint_msg_type_t;

typedef enum gf_fingerprint_error {
    GF_FINGERPRINT_ERROR_HW_UNAVAILABLE = 1,
    GF_FINGERPRINT_ERROR_UNABLE_TO_PROCESS = 2,
    GF_FINGERPRINT_ERROR_TIMEOUT = 3,
    GF_FINGERPRINT_ERROR_NO_SPACE = 4,
    GF_FINGERPRINT_ERROR_CANCELED = 5,
    GF_FINGERPRINT_ERROR_UNABLE_TO_REMOVE = 6,
    GF_FINGERPRINT_ERROR_LOCKOUT = 7,
    GF_FINGERPRINT_ERROR_VENDOR_BASE = 1000,
    GF_FINGERPRINT_ERROR_TOO_MUCH_UNDER_SATURATED_PIXELS = 1001,
    GF_FINGERPRINT_ERROR_TOO_MUCH_OVER_SATURATED_PIXELS = 1002,
} gf_fingerprint_error_t;

typedef enum gf_fingerprint_acquired_info {
    GF_FINGERPRINT_ACQUIRED_GOOD = 0,
    GF_FINGERPRINT_ACQUIRED_PARTIAL = 1,
    GF_FINGERPRINT_ACQUIRED_INSUFFICIENT = 2,
    GF_FINGERPRINT_ACQUIRED_IMAGER_DIRTY = 3,
    GF_FINGERPRINT_ACQUIRED_TOO_SLOW = 4,
    GF_FINGERPRINT_ACQUIRED_TOO_FAST = 5,
    GF_FINGERPRINT_ACQUIRED_DETECTED = 6,
    GF_FINGERPRINT_ERROR_CONTINUOUS_INVALID_PRESS = 7,
    GF_FINGERPRINT_ERROR_INCOMPLETE_TEMPLATE = 8,
    GF_FINGERPRINT_ACQUIRED_VENDOR_BASE = 1000,
    GF_FINGERPRINT_ACQUIRED_WAIT_FINGER_INPUT = 1001,
    GF_FINGERPRINT_ACQUIRED_FINGER_DOWN = 1002,
    GF_FINGERPRINT_ACQUIRED_FINGER_UP = 1003,
    GF_FINGERPRINT_ACQUIRED_INPUT_TOO_LONG = 1004,
    GF_FINGERPRINT_ACQUIRED_DUPLICATE_AREA = 1005,
    GF_FINGERPRINT_ACQUIRED_DUPLICATE_FINGER = 1006,
    GF_FINGERPRINT_ACQUIRED_SIMULATED_FINGER = 1007,
    GF_FINGERPRINT_ACQUIRED_SINGLE_CLICK = 1008,
    GF_FINGERPRINT_ACQUIRED_DOUBLE_CLICK = 1009,
    GF_FINGERPRINT_ACQUIRED_LONG_PRESSED = 1010,
} gf_fingerprint_acquired_info_t;

typedef enum gf_nav_click_status {
    GF_NAV_CLICK_STATUS_NONE = 0 ,
    GF_NAV_CLICK_STATUS_DOWN,
    GF_NAV_CLICK_STATUS_DOWN_UP,
} gf_nav_click_status_t;

typedef struct gf_fingerprint_finger_id {
    uint32_t gid;
    uint32_t fid;
} gf_fingerprint_finger_id_t;

typedef struct gf_fingerprint_enroll {
    gf_fingerprint_finger_id_t finger;
    uint32_t samples_remaining;
    uint64_t msg;
} gf_fingerprint_enroll_t;

typedef struct gf_fingerprint_removed {
    gf_fingerprint_finger_id_t finger;
    int32_t remaining_templates;
} gf_fingerprint_removed_t;

typedef struct gf_fingerprint_acquired {
    gf_fingerprint_acquired_info_t acquired_info;
} gf_fingerprint_acquired_t;

typedef struct gf_fingerprint_authenticated {
    gf_fingerprint_finger_id_t finger;
    gf_hw_auth_token_t hat;
} gf_fingerprint_authenticated_t;

typedef struct gf_fingerprint_test_cmd {
    int32_t cmd_id;
    int8_t *result;
    int32_t result_len;
} gf_fingerprint_test_cmd_t;

typedef enum gf_fingerprint_hbd_status {
    GF_FINGERPRINT_HBD_UNTOUCHED,
    GF_FINGERPRINT_SIGNAL_SEARCHING,
    GF_FINGERPRINT_HBD_TESTING,
    GF_FINGERPRINT_HBD_TOOHARD,
    GF_FINGERPRINT_HBD_TEST_FAIL
} gf_fingerprint_hbd_status_t;

typedef struct gf_fingerprint_hbd {
    int32_t heart_beat_rate;
    gf_fingerprint_hbd_status_t status;
    int32_t* display_data;
    int32_t data_len;
    int32_t* raw_data;
    int32_t raw_data_len;
} gf_fingerprint_hbd_t;

typedef struct gf_fingerprint_iterator {
    gf_fingerprint_finger_id_t finger;
    int32_t remaining_templates;
} gf_fingerprint_iterator_t;

typedef gf_fingerprint_iterator_t gf_fingerprint_enumerated_t;

typedef struct gf_fingerprint_msg {
    gf_fingerprint_msg_type_t type;
    union {
        gf_fingerprint_error_t error;
        gf_fingerprint_enroll_t enroll;
        gf_fingerprint_removed_t removed;
        gf_fingerprint_acquired_t acquired;
        gf_fingerprint_authenticated_t authenticated;
        gf_fingerprint_test_cmd_t test;
        gf_fingerprint_hbd_t hbd;
        gf_fingerprint_iterator_t enumerated;
    } data;
} gf_fingerprint_msg_t;

typedef struct gf_fingerprint_device {

    void (*notify)(const gf_fingerprint_msg_t *msg);

} gf_fingerprint_device_t;

#endif
