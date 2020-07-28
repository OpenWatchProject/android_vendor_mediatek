/*
* Copyright (C) 2013-2018, Shenzhen Huiding Technology Co., Ltd.
* All Rights Reserved.
*/
#ifndef __GF_HAL_H__
#define __GF_HAL_H__

#include "gf_error.h"
#include "gf_type_define.h"
#include "gf_fingerprint.h"

#ifdef __cplusplus
extern "C" {
#endif

gf_error_t gf_hal_open(void *dev);
gf_error_t gf_hal_close(void *dev);
gf_error_t gf_hal_cancel(void *dev);

uint64_t gf_hal_pre_enroll(void *dev);
gf_error_t gf_hal_enroll(void *dev, const void *hat, uint32_t group_id, uint32_t timeout_sec);
gf_error_t gf_hal_post_enroll(void *dev);
gf_error_t gf_hal_authenticate(void *dev, uint64_t operation_id, uint32_t group_id);
uint64_t gf_hal_get_auth_id(void *dev);
gf_error_t gf_hal_remove(void *dev, uint32_t group_id, uint32_t finger_id);

//#ifdef __ANDROID_O
gf_error_t gf_hal_set_active_group(void *dev, uint32_t group_id);
//#else
//gf_error_t gf_hal_set_active_group(void *dev, uint32_t group_id, uint32_t* list, int32_t count);
//#endif

gf_error_t gf_hal_enumerate(void *dev, void *results, uint32_t *max_size);
gf_error_t gf_hal_enumerate_with_callback(void *dev);
gf_error_t gf_hal_irq(void);
gf_error_t gf_hal_screen_on(void);
gf_error_t gf_hal_screen_off(void);

gf_error_t gf_hal_test_cmd(void* dev, uint32_t cmdId, const uint8_t* param, uint32_t param_len);

gf_error_t gf_hal_set_safe_class(void *dev, gf_safe_class_t safe_class);
gf_error_t gf_hal_set_session_id(void *dev, uint64_t session_id) ;
gf_error_t gf_hal_get_session_id(void *dev, uint64_t *session_id);
gf_error_t gf_hal_navigate(void *dev, gf_nav_mode_t nav_mode);

gf_error_t gf_hal_enable_fingerprint_module(void *dev, uint8_t enable_flag);
gf_error_t gf_hal_camera_capture(void *dev);
gf_error_t gf_hal_enable_ff_feature(void *dev, uint8_t enable_flag);


gf_error_t gf_hal_enable_bio_assay_feature(void *dev, uint8_t enable_flag);
gf_error_t gf_hal_start_hbd(void* dev);
gf_error_t gf_hal_reset_lockout(void *dev, const void *hat);

gf_error_t gf_hal_sync_finger_list(void* dev, uint32_t* list, int32_t count);

gf_error_t gf_hal_user_invoke_command(uint32_t cmd_id, void *buffer, int len);

void gf_hal_notify_test_authentication_failed();
void gf_hal_notify_test_remove_succeeded(uint32_t group_id, uint32_t finger_id);
void gf_hal_notify_test_error_info(gf_fingerprint_error_t err_code);
void gf_hal_notify_remove_succeeded(uint32_t group_id, uint32_t finger_id);
void hal_notify_authentication_succeeded(uint32_t group_id, uint32_t finger_id,
                                         gf_hw_auth_token_t *auth_token);
uint32_t gf_hal_is_inited(void);

#ifdef __cplusplus
}
#endif

#endif //__GF_HAL_H__
