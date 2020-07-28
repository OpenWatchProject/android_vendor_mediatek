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

/**
 * @file pal_connmo.h
 * @brief File containing PAL interface for ConnMO plugin.
 *
 * Connectivity Management.
 * Providing a standardized set of management objects for configuration of
 * data network connectivity through the OMA Device Management system will
 * improve the usability and customer experience of mobile terminals that rely
 * upon data services.
 * @see http://openmobilealliance.org/
 */

#ifndef PAL_CONNMO_H
#define PAL_CONNMO_H
#include "pal_common.h"

#ifdef __cplusplus
extern "C" {
#endif


//Reads ID of the APN with class 1
int pal_network_apn_class1_id_get(data_buffer_t *buff);

// Reads name for class 1 apn.
int pal_network_apn_class1_name_get(data_buffer_t *buff);

// Sets name for class 1 apn.
int pal_network_apn_class1_name_set(data_buffer_t *buff);

// Reads ip version for class 1 apn.
int pal_network_apn_class1_ip_get(data_buffer_t *buff);

// Sets ip version for class 1 apn.
int pal_network_apn_class1_ip_set(data_buffer_t *buff);

// Reads state for class 1 apn.
int pal_network_apn_class1_state_get(data_buffer_t *buff);

// Enable class 1 apn.
int pal_network_apn_class1_enable(data_buffer_t *ignored);

// Disable class 1 apn.
int pal_network_apn_class1_disabled(data_buffer_t *ignored);

// Getting id for class 2 apn.
int pal_network_apn_class2_id_get(data_buffer_t *buff);

// Getting name for class 2 apn.
int pal_network_apn_class2_name_get(data_buffer_t *buff);

// Setting name for class 2 apn.
int pal_network_apn_class2_name_set(data_buffer_t *buff);

// Getting ip for class 2 apn.
int pal_network_apn_class2_ip_get(data_buffer_t *buff);

// Setting ip for class 2 apn.
int pal_network_apn_class2_ip_set(data_buffer_t *buff);

// Getting state for class 2 apn.
int pal_network_apn_class2_state_get(data_buffer_t *buff);

// Enable class 2 apn.
int pal_network_apn_class2_enable(data_buffer_t *buff);

// Disable class 2 apn.
int pal_network_apn_class2_disabled(data_buffer_t *buff);

// Getting id for class 3 apn.
int pal_network_apn_class3_id_get(data_buffer_t *buff);

// Getting name for class 3 apn.
int pal_network_apn_class3_name_get(data_buffer_t *buff);

// Setting name for class 3 apn.
int pal_network_apn_class3_name_set(data_buffer_t *buff);

// Getting ip for class 3 apn.
int pal_network_apn_class3_ip_get(data_buffer_t *buff);

// Setting ip for class 3 apn.
int pal_network_apn_class3_ip_set(data_buffer_t *buff);

// Getting state for class 3 apn.
int pal_network_apn_class3_state_get(data_buffer_t *buff);

// Enable class 3 apn.
int pal_network_apn_class3_enable(data_buffer_t *buff);

// Disable class 3 apn.
int pal_network_apn_class3_disabled(data_buffer_t *buff);

// Getting id for class 4 apn.
int pal_network_apn_class4_id_get(data_buffer_t *buff);

// Getting name for class 4 apn.
int pal_network_apn_class4_name_get(data_buffer_t *buff);

// Setting name for class 4 apn.
int pal_network_apn_class4_name_set(data_buffer_t *buff);

// Getting ip for class 4 apn.
int pal_network_apn_class4_ip_get(data_buffer_t *buff);

// Setting ip for class 4 apn.
int pal_network_apn_class4_ip_set(data_buffer_t *buff);

// Getting state for class 4 apn.
int pal_network_apn_class4_state_get(data_buffer_t *buff);

// Enable class 4 apn.
int pal_network_apn_class4_enable(data_buffer_t *buff);

// Disable class 4 apn.
int pal_network_apn_class4_disabled(data_buffer_t *buff);

// Getting id for class 6 apn.
int pal_network_apn_class6_id_get(data_buffer_t *buff);

// Getting name for class 6 apn.
int pal_network_apn_class6_name_get(data_buffer_t *buff);

// Setting name for class 6 apn.
int pal_network_apn_class6_name_set(data_buffer_t *buff);

// Getting ip for class 6 apn.
int pal_network_apn_class6_ip_get(data_buffer_t *buff);

// Setting ip for class 6 apn.
int pal_network_apn_class6_ip_set(data_buffer_t *buff);

// Getting state for class 6 apn.
int pal_network_apn_class6_state_get(data_buffer_t *buff);

// Enable class 6 apn.
int pal_network_apn_class6_enable(data_buffer_t *buff);

// Disable class 6 apn.
int pal_network_apn_class6_disabled(data_buffer_t *buff);

// Gets IMS domain value.
int pal_network_ims_domain_get(data_buffer_t *buff);

// Checks is the "ims sms over ip network" feature enabled?
int pal_network_ims_sms_over_ip_network_indication_get(data_buffer_t *iobuf);

// Enables or disables the "ims sms over ip network" feature
int pal_network_ims_sms_over_ip_network_indication_set(data_buffer_t *iobuf);

// Getting the "ims smsformat" value
int pal_network_ims_smsformat_get(data_buffer_t *buff);

// Set the "ims smsformat" feature
int pal_network_ims_smsformat_set(data_buffer_t *buff);

#ifdef __cplusplus
}
#endif

#endif // PAL_CONNMO_H
