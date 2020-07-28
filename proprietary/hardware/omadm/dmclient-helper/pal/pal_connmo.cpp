/* Copyright Statement:
*
* This software/firmware and related documentation ("MediaTek Software") are
* protected under relevant copyright laws. The information contained herein
* is confidential and proprietary to MediaTek Inc. and/or its licensors.
* Without the prior written permission of MediaTek inc. and/or its licensors,
* any reproduction, modification, use or disclosure of MediaTek Software,
* and information contained herein, in whole or in part, shall be strictly prohibited.
*
* MediaTek Inc. (C) 2017. All rights reserved.
*
* BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
* THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
* RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
* AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
* NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
* SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
* SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
* THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
* THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
* CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
* SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
* STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
* CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
* AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
* OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
* MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
*
* The following software/firmware and/or related documentation ("MediaTek Software")
* have been modified by MediaTek Inc. All revisions are subject to any receiver's
* applicable license agreements with MediaTek Inc.
*/
    
#include <cutils/properties.h>
    
#include "omadm_service_api.h"
#include "pal.h"
    
#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "PAL"
    
#include "./pal_internal.h"
    
using namespace android;
using namespace omadm_service_api;

__BEGIN_DECLS

static int _get_apn_id(int32_t apn_cls, int32_t* p_id)
{
    OMADM_SRV_READY();
    AutoMutex _l(OmadmServiceAPI::mPalLock);
    return OmadmServiceAPI::getApnIntTypeInfo(HIDL_APN_GET_ID, apn_cls, p_id);
}

static int _get_apn_name(int32_t apn_cls, data_buffer_t *p_buffer)
{
    OMADM_SRV_READY();
    AutoMutex _l(OmadmServiceAPI::mPalLock);
    return OmadmServiceAPI::getApnChrBuff(
        HIDL_APN_GET_NAME, apn_cls, p_buffer);
}

static int _set_apn_name(int32_t apn_cls, const char* apn_name)
{
    OMADM_SRV_READY();
    AutoMutex _l(OmadmServiceAPI::mPalLock);
    return OmadmServiceAPI::setApnChrBuff(
        HIDL_APN_SET_NAME, apn_cls, apn_name);
}

static int _get_apn_ip(int32_t apn_cls, int32_t* p_ipvX)
{
    OMADM_SRV_READY();
    AutoMutex _l(OmadmServiceAPI::mPalLock);
    return OmadmServiceAPI::getApnIntTypeInfo(HIDL_APN_GET_IPVX, apn_cls, p_ipvX);
}
    
const static char* PROTO_IPV4 = "IPV4";
const static char* PROTO_IPV6 = "IPV6";
const static char* PROTO_IPV4V6 = "IPV4 and IPV6";
    
#define MASK_IPV4 0x00000001
#define MASK_IPV6 0x00000002
#define MASK_IPV4V6 0x00000003
    
static int _get_apn_ip_str(int32_t apn_cls, data_buffer_t *obuf)
{
    int proto_mask = 0;
    char* proto_str = NULL;
    int rc = _get_apn_ip(apn_cls, &proto_mask);
    PRINT_DBG("proto_mask = %d", proto_mask);
    if (RESULT_SUCCESS != rc) {
        return rc;
    }
    switch(proto_mask) {
        case MASK_IPV4:
            proto_str = (char*)PROTO_IPV4;
            break;
        case MASK_IPV6:
            proto_str = (char*)PROTO_IPV6;
            break;
        case MASK_IPV4V6:
            proto_str = (char*)PROTO_IPV4V6;
            break;
    }
    if (NULL == proto_str) {
        return RESULT_ERROR_UNDEFINED;
    }
    CHECK_AND_SET_BUFFER_STR2STR(obuf, proto_str);
    return RESULT_SUCCESS;
}

static int _set_apn_ip(int32_t apn_cls, int32_t ipvX)
{
    OMADM_SRV_READY();
    AutoMutex _l(OmadmServiceAPI::mPalLock);
    return OmadmServiceAPI::setApnIntTypeInfo(HIDL_APN_SET_IPVX, apn_cls, ipvX);
}

static int _set_apn_ip_str(int32_t apn_cls, data_buffer_t* ibuf) {
    CHECK_BUFFER(ibuf);
    int ipvX = 0;
    if (0 == strcasecmp(ibuf->data, PROTO_IPV4)) {
        ipvX = MASK_IPV4;
    } else
    if (0 == strcasecmp(ibuf->data, PROTO_IPV6)) {
        ipvX = MASK_IPV6;
    } else
    if (0 == strcasecmp(ibuf->data, PROTO_IPV4V6)) {
        ipvX = MASK_IPV4V6;
    }
    PRINT_DBG("%s => %d", ibuf->data, ipvX);
    if (0 == ipvX) {
        return RESULT_ERROR_INVALID_ARGS;
    }
    return _set_apn_ip(apn_cls, ipvX);
}

static int _get_apn_state(int32_t apn_cls, int32_t* p_is_enabled)
{
    OMADM_SRV_READY();
    AutoMutex _l(OmadmServiceAPI::mPalLock);
    return OmadmServiceAPI::getApnIntTypeInfo(
        HIDL_APN_IS_ENABLE, apn_cls, p_is_enabled);
}

static const char* STR_TRUE = "TRUE";
static const char* STR_FALSE = "FALSE";

static int _get_apn_state_str(int32_t apn_cls, data_buffer_t* obuf)
{
    int is_enabled = -1;
    int rc = _get_apn_state(apn_cls, &is_enabled);
    if (rc != RESULT_SUCCESS) {
        return rc;
    }
    const char* is_enabled_str = is_enabled ? STR_TRUE : STR_FALSE;
    CHECK_AND_SET_BUFFER_STR2STR(obuf, is_enabled_str);
    return RESULT_SUCCESS;
}

static int _en_apn_class(int32_t apn_cls, int32_t enable)
{
    OMADM_SRV_READY();
    AutoMutex _l(OmadmServiceAPI::mPalLock);
    return OmadmServiceAPI::setApnIntTypeInfo(
        HIDL_APN_SET_ENABLE, apn_cls, enable);
}

int pal_network_apn_class1_id_get(data_buffer_t *obuf)
{
    int32_t tmp_val;
    int rc = _get_apn_id(1, &tmp_val);
    if (rc != RESULT_SUCCESS) {
        return rc;
    }
    CHECK_AND_SET_BUFFER_INT2STR(obuf, tmp_val);
    return RESULT_SUCCESS;
}

int pal_network_apn_class1_name_get(data_buffer_t *obuf)
{
    return _get_apn_name(1, obuf);
}

int pal_network_apn_class1_name_set(data_buffer_t *ibuf)
{
    CHECK_BUFFER(ibuf);
    return _set_apn_name(1, ibuf->data);
}

int pal_network_apn_class1_ip_get(data_buffer_t *obuf)
{
    return _get_apn_ip_str(1, obuf);
}

int pal_network_apn_class1_ip_set(data_buffer_t *ibuf)
{
    return _set_apn_ip_str(1, ibuf);
}

int pal_network_apn_class1_state_get(data_buffer_t *obuf)
{
    return _get_apn_state_str(1, obuf);
}

int pal_network_apn_class1_enable(data_buffer_t* buff)
{
    UNUSED(buff);
    return _en_apn_class(1, 1);
}

int pal_network_apn_class1_disabled(data_buffer_t* buff)
{
    UNUSED(buff);
    return _en_apn_class(1, 0);
}

int pal_network_apn_class2_id_get(data_buffer_t *obuf)
{
    int32_t tmp_val;
    int rc = _get_apn_id(2, &tmp_val);
    if (rc != RESULT_SUCCESS) {
        return rc;
    }
    CHECK_AND_SET_BUFFER_INT2STR(obuf, tmp_val);
    return RESULT_SUCCESS;
}

int pal_network_apn_class2_name_get(data_buffer_t *obuf)
{
    return _get_apn_name(2, obuf);
}

int pal_network_apn_class2_name_set(data_buffer_t *ibuf)
{
    CHECK_BUFFER(ibuf);
    return _set_apn_name(2, ibuf->data);
}

int pal_network_apn_class2_ip_get(data_buffer_t *buff)
{
    return _get_apn_ip_str(2, buff);
}

int pal_network_apn_class2_ip_set(data_buffer_t *ibuf)
{
    return _set_apn_ip_str(2, ibuf);
}

int pal_network_apn_class2_state_get(data_buffer_t *obuf)
{
    return _get_apn_state_str(2, obuf);
}

int pal_network_apn_class2_enable(data_buffer_t* buff)
{
    UNUSED(buff);
    return _en_apn_class(2, 1);
}

int pal_network_apn_class2_disabled(data_buffer_t* buff)
{
    UNUSED(buff);
    return _en_apn_class(2, 0);
}

int pal_network_apn_class3_id_get(data_buffer_t *obuf)
{
    int32_t tmp_val;
    int rc = _get_apn_id(3, &tmp_val);
    if (rc != RESULT_SUCCESS) {
        return rc;
    }
    CHECK_AND_SET_BUFFER_INT2STR(obuf, tmp_val);
    return RESULT_SUCCESS;
}

int pal_network_apn_class3_name_get(data_buffer_t *obuf)
{
    return _get_apn_name(3, obuf);
}

int pal_network_apn_class3_name_set(data_buffer_t *ibuf)
{
    CHECK_BUFFER(ibuf);
    return _set_apn_name(3, ibuf->data);
}

int pal_network_apn_class3_ip_get(data_buffer_t *buff)
{
    return _get_apn_ip_str(3, buff);
}

int pal_network_apn_class3_ip_set(data_buffer_t *ibuf)
{
    return _set_apn_ip_str(3, ibuf);
}

int pal_network_apn_class3_state_get(data_buffer_t *obuf)
{
    return _get_apn_state_str(3, obuf);
}

int pal_network_apn_class3_enable(data_buffer_t* buff){
    UNUSED(buff);
    return _en_apn_class(3, 1);
}

int pal_network_apn_class3_disabled(data_buffer_t* buff){
    UNUSED(buff);
    return _en_apn_class(3, 0);
}

int pal_network_apn_class4_id_get(data_buffer_t *obuf)
{
    int32_t tmp_val;
    int rc = _get_apn_id(4, &tmp_val);
    if (rc != RESULT_SUCCESS) {
        return rc;
    }
    CHECK_AND_SET_BUFFER_INT2STR(obuf, tmp_val);
    return RESULT_SUCCESS;
}

int pal_network_apn_class4_name_get(data_buffer_t *obuf)
{
    return _get_apn_name(4, obuf);
}

int pal_network_apn_class4_name_set(data_buffer_t *ibuf)
{
    CHECK_BUFFER(ibuf);
    return _set_apn_name(4, ibuf->data);
}

int pal_network_apn_class4_ip_get(data_buffer_t *buff)
{
    return _get_apn_ip_str(4, buff);
}

int pal_network_apn_class4_ip_set(data_buffer_t *ibuf)
{
    return _set_apn_ip_str(4, ibuf);
}

int pal_network_apn_class4_state_get(data_buffer_t *obuf)
{
    return _get_apn_state_str(4, obuf);
}

int pal_network_apn_class4_enable(data_buffer_t* buff)
{
    UNUSED(buff);
    return _en_apn_class(4, 1);
}

int pal_network_apn_class4_disabled(data_buffer_t* buff)
{
    UNUSED(buff);
    return _en_apn_class(4, 0);
}

int pal_network_apn_class6_id_get(data_buffer_t* obuf)
{
    int32_t tmp_val;
    int rc = _get_apn_id(5, &tmp_val);
    if (rc != RESULT_SUCCESS) {
        return rc;
    }
    CHECK_AND_SET_BUFFER_INT2STR(obuf, tmp_val);
    return RESULT_SUCCESS;
}

int pal_network_apn_class6_name_get(data_buffer_t* obuf)
{
    return _get_apn_name(5, obuf);
}

int pal_network_apn_class6_name_set(data_buffer_t *ibuf)
{
    CHECK_BUFFER(ibuf);
    return _set_apn_name(5, ibuf->data);
}

int pal_network_apn_class6_ip_get(data_buffer_t *buff)
{
    return _get_apn_ip_str(5, buff);
}

int pal_network_apn_class6_ip_set(data_buffer_t *ibuf)
{
    return _set_apn_ip_str(5, ibuf);
}

int pal_network_apn_class6_state_get(data_buffer_t *obuf)
{
    return _get_apn_state_str(5, obuf);
}

int pal_network_apn_class6_enable(data_buffer_t* buff)
{
    UNUSED(buff);
    return _en_apn_class(5, 1);
}

int pal_network_apn_class6_disabled(data_buffer_t* buff)
{
    UNUSED(buff);
    return _en_apn_class(5, 0);
}

static const char* PROP_SMS_IMS = "persist.radio.imsallowmtsms";

int pal_network_ims_sms_over_ip_network_indication_get(data_buffer_t *iobuf)
{
    int is_enabled = 0; // the same behaviour as in the RadioInfo.java
    int rc = RESULT_ERROR_UNDEFINED;
    char buf[PROPERTY_VALUE_MAX];
    int len = property_get(PROP_SMS_IMS, buf, "1");
    PRINT_DBG("property_get(%s, %s, \"\"", PROP_SMS_IMS, buf);
    if (len == 1) {
        char ch = buf[0];
        if (ch == '0' || ch == 'n') {
            is_enabled = 0;
            rc = RESULT_SUCCESS;
        } else if (ch == '1' || ch == 'y') {
            is_enabled = 1;
            rc = RESULT_SUCCESS;
        } else {
            PRINT_WRN("%s: undefined", PROP_SMS_IMS);
        }
    } else if (len > 1) {
        if (!strcmp(buf, "no") || !strcmp(buf, "false") ||
            !strcmp(buf, "off")) {
            is_enabled = 0;
            rc = RESULT_SUCCESS;
        } else if (!strcmp(buf, "yes") || !strcmp(buf, "true") ||
            !strcmp(buf, "on")) {
            is_enabled = 1;
            rc = RESULT_SUCCESS;
        } else {
            PRINT_WRN("%s: undefined", PROP_SMS_IMS);
        }
    } else {
        PRINT_WRN("%s: not found", PROP_SMS_IMS);
    }
    if (is_enabled == -1) {
        return RESULT_ERROR_UNDEFINED;
    }
    const char* str_value = is_enabled ? STR_TRUE : STR_FALSE;
    CHECK_AND_SET_BUFFER_STR2STR(iobuf, str_value);
    return RESULT_SUCCESS;
}

int pal_network_ims_sms_over_ip_network_indication_set(data_buffer_t *ibuf){
    CHECK_BUFFER(ibuf);
    PRINT_DBG("(%s)", ibuf->data);
    int enable = -1;
    if (strncasecmp(ibuf->data, STR_TRUE, ibuf->size) == 0) {
        enable = 1;
    } else if (strncasecmp(ibuf->data, STR_FALSE, ibuf->size) == 0) {
        enable = 0;
    }
    if (enable == -1) {
        PRINT_WRN("ibuf->data = %s", ibuf->data);
        return RESULT_ERROR_INVALID_ARGS;
    }
    char buf[PROPERTY_VALUE_MAX];
    int len = property_get(PROP_SMS_IMS, buf, "");
    if (len <= 1) {
        snprintf(buf, 4, "%d", enable);
    } else {
        if (strcmp(buf, "no") == 0 || strcmp(buf, "yes")) {
            snprintf(buf, PROPERTY_VALUE_MAX - 1, enable ? "yes" : "no");
        } else if (strcmp(buf, "true") == 0 || strcmp(buf, "false")){
            snprintf(buf, PROPERTY_VALUE_MAX - 1, enable ? "true" : "false");
        } else if (strcmp(buf, "on") == 0 || strcmp(buf, "off")){
            snprintf(buf, PROPERTY_VALUE_MAX - 1, enable ? "on" : "off");
        } else {
            return RESULT_ERROR_RESOURCE_IS_NOT_AVAILABLE;
        }
    }
    if (property_set(PROP_SMS_IMS, buf) != 0) {
        PRINT_WRN("set : %s : %s : FAILED", PROP_SMS_IMS, buf);
        return RESULT_ERROR;
    } else {
        PRINT_DBG("set : %s : %s : OK", PROP_SMS_IMS, buf);
        return RESULT_SUCCESS;
    }
    return RESULT_SUCCESS;
}

int pal_network_ims_domain_get(data_buffer_t *obuf){
    OMADM_SRV_READY();
    AutoMutex _l(OmadmServiceAPI::mPalLock);
    return OmadmServiceAPI::getImsDomain(obuf);
}

const static char* const SMS_3GPP_STR = "3GPP";
const static char* const SMS_3GPP2_STR = "3GPP2";
#define SMS_3GPP_ENUM 1
#define SMS_3GPP2_ENUM 0

int pal_network_ims_smsformat_get(data_buffer_t* obuf)
{
    PRINT_DBG("(%p)", obuf);
    OMADM_SRV_READY();
    AutoMutex _l(OmadmServiceAPI::mPalLock);
    int32_t smsFormat = 0;
    int errCode = OmadmServiceAPI::getSmsImsFormat(&smsFormat);
    if (errCode != RESULT_SUCCESS) {
        return errCode;
    }
    char* smsFormatStr = NULL;
    switch(smsFormat) {
        case SMS_3GPP_ENUM:
            smsFormatStr = (char*)SMS_3GPP_STR;
            break;
        case SMS_3GPP2_ENUM:
            smsFormatStr = (char*)SMS_3GPP2_STR;
            break;
    }
    if (smsFormatStr == NULL) {
        PRINT_WRN("smsFormatStr == NULL, smsFormat = %d", smsFormat);
        return RESULT_ERROR_INVALID_STATE;
    }
    PRINT_DBG("smsFormat = %d, smsFormatStr = %s", smsFormat, smsFormatStr);
    CHECK_AND_SET_BUFFER_STR2STR(obuf, smsFormatStr);
    return RESULT_SUCCESS;
}

int pal_network_ims_smsformat_set(data_buffer_t* ibuf)
{
    CHECK_BUFFER(ibuf);
    int smsFormat = -1;
    if (strcasecmp(ibuf->data, SMS_3GPP_STR) == 0) {
        smsFormat = SMS_3GPP_ENUM;
    } else if (strcasecmp(ibuf->data, SMS_3GPP2_STR) == 0) {
        smsFormat = SMS_3GPP2_ENUM;
    }
    if (smsFormat < 0) {
        PRINT_WRN("ibuf->data = %s", ibuf->data);
        return RESULT_ERROR_INVALID_ARGS;
    }
    OMADM_SRV_READY();
    AutoMutex _l(OmadmServiceAPI::mPalLock);
    return OmadmServiceAPI::setSmsImsFormat(smsFormat);
}

__END_DECLS

