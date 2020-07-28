/*
* Copyright (C) 2013-2018, Shenzhen Huiding Technology Co., Ltd.
* All Rights Reserved.
*/


#ifndef __GF_WECHAT_API_H__
#define __GF_WECHAT_API_H__

// Only for support keymaster Android L
extern TEE_Result gf_dr_wechat_api_get_session_id(uint64_t *session_id);
extern TEE_Result gf_dr_wechat_api_set_session_id(uint64_t session_id);

// For Android M WeChat Pay
extern TEE_Result tlApiFpGetFingerId(int32_t *finger_id);

#endif // __GF_WECHAT_API_H__
