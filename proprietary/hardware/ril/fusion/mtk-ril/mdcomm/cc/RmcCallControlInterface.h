/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2016. All rights reserved.
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

#ifndef __RMC_CALL_CONTROL_INTERFACE_H__
#define __RMC_CALL_CONTROL_INTERFACE_H__

// AT cmd
#define AT_CONF_DIAL                 (char*)"AT+EDCONF"
#define AT_SIP_URI_DIAL              (char*)"AT+CDU"
#define AT_CONF_MODIFY               (char*)"AT+ECONF"
#define AT_IMS_CALL_CTRL             (char*)"AT+ECCTRL"
#define AT_VDS_MODE                  (char*)"AT+EVADSMOD"
#define AT_FORCE_RELEASE             (char*)"AT+ECHUP"
#define AT_EMERGENCY_DIAL            (char*)"ATDE"
#define AT_DIAL                      (char*)"ATD"
#define AT_ANSWER                    (char*)"ATA"
#define AT_ALLOW_MT                  (char*)"AT+EAIC"
#define AT_EXIT_ECBM                 (char*)"AT+VMEMEXIT"
#define AT_ENABLE_VDS_REPORT         (char*)"AT+EVADSREP"
#define AT_ANSWER_VT_FUNCTIONAL      (char*)"AT+EVTA"
#define AT_APPROVE_ECC_REDIAL        (char*)"AT+ERDECCAPV"
#define AT_PULL_CALL                 (char*)"AT+ECALLPULL"
#define AT_IMS_ECT                   (char*)"AT+ECT"
#define AT_SET_RTT_MODE              (char*)"AT+EIMSRTT"
#define AT_SET_SIP_HEADER            (char*)"AT+ESIPHEADER"
#define AT_SIP_HEADER_REPORT         (char*)"AT+EIMSHEADER"
#define AT_SET_IMS_CALL_MODE         (char*)"AT+EIMSCALLMODE"
#define AT_SET_IMS_CALL_ADDITIONAL   (char*)"AT+EIMSCAI"

// URC
#define URC_CONF_SRVCC               (char*)"+ECONFSRVCC"
#define URC_CONF_MODIFIED_RESULT     (char*)"+ECONF:"
#define URC_VDM_SELECT_RESULT        (char*)"+EVADSREP"
#define URC_IMS_CALL_MODE            (char*)"+EIMSCMODE"
#define URC_IMS_VIDEO_CAP            (char*)"+EIMSVCAP"
#define URC_ECPLICIT_CALL_TRANSFER   (char*)"+ECT:"
#define URC_IMS_CALL_CONTROL_RESULT  (char*)"+ECCTRL"
#define URC_IMS_EVENT_PACKAGE        (char*)"+EIMSEVTPKG"
#define URC_SRVCC                    (char*)"+CIREPH"
#define URC_CONF_CHECK               (char*)"+ECONFCHECK"
#define URC_ECC_BEARER               (char*)"+CNEMS1"
#define URC_REDIAL_ECC_INDICATION    (char*)"+ERDECCIND"
#define URC_IMS_HEADER               (char*)"+EIMSHEADER"
#define URC_CALL_ADDITIONAL_INFO     (char*)"+EIMSCAI"

#endif
