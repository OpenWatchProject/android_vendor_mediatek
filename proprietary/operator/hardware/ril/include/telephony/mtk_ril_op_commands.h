/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2017. All rights reserved.
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
#ifdef MTK_USE_HIDL
 {RIL_REQUEST_SET_DIGITS_LINE, radio::setDigitsLineResponse},
 {RIL_REQUEST_SET_TRN, radio::setTrnResponse},
 {RIL_REQUEST_VSS_ANTENNA_CONF, radio::setRxTestConfigResponse}, // Antenna Testing
 {RIL_REQUEST_VSS_ANTENNA_INFO, radio::getRxTestResultResponse}, // Antenna Testing
 {RIL_REQUEST_DIAL_FROM, radio::dialFromResponse},
 {RIL_REQUEST_SEND_USSI_FROM, radio::sendUssiFromResponse},
 {RIL_REQUEST_CANCEL_USSI_FROM, radio::cancelUssiFromResponse},
 {RIL_REQUEST_SET_EMERGENCY_CALL_CONFIG, radio::setEmergencyCallConfigResponse},
 {RIL_REQUEST_SET_DISABLE_2G, radio::setDisable2GResponse},
 {RIL_REQUEST_GET_DISABLE_2G, radio::getDisable2GResponse},
 {RIL_REQUEST_DEVICE_SWITCH, radio::deviceSwitchResponse},
 {RIL_REQUEST_CANCEL_DEVICE_SWITCH, radio::cancelDeviceSwitchResponse},
 {RIL_REQUEST_SET_INCOMING_VIRTUAL_LINE, radio::setIncomingVirtualLineResponse},
 {RIL_REQUEST_SET_DIGITS_REG_STATUS, radio::setDigitsRegStatuseResponse},
 {RIL_REQUEST_EXIT_SCBM, radio::exitSCBMResponse},
 {RIL_REQUEST_SEND_RSU_REQUEST, radio::sendRsuRequestResponse},
 {RIL_REQUEST_SWITCH_RCS_ROI_STATUS, radio::switchRcsRoiStatusResponse},
 {RIL_REQUEST_UPDATE_RCS_CAPABILITIES, radio::updateRcsCapabilitiesResponse},
 {RIL_REQUEST_UPDATE_RCS_SESSION_INFO, radio::updateRcsSessionInfoResponse},
#else
 {RIL_REQUEST_SET_DIGITS_LINE, dispatchStrings, responseVoid, RIL_CMD_PROXY_1, RIL_CHANNEL_5},
 {RIL_REQUEST_SET_TRN, dispatchStrings, responseVoid, RIL_CMD_PROXY_1, RIL_CHANNEL_5},
 {RIL_REQUEST_DIAL_FROM, dispatchDial, responseVoid, RIL_CMD_PROXY_2, RIL_CHANNEL_2},
 {RIL_REQUEST_SEND_USSI_FROM, dispatchStrings, responseVoid, RIL_CMD_PROXY_1, RIL_CHANNEL_5},
 {RIL_REQUEST_CANCEL_USSI_FROM, dispatchString, responseVoid, RIL_CMD_PROXY_1, RIL_CHANNEL_5},
 {RIL_REQUEST_SET_EMERGENCY_CALL_CONFIG, dispatchInts, responseVoid, RIL_CMD_PROXY_2, RIL_CHANNEL_1},
 {RIL_REQUEST_DEVICE_SWITCH, dispatchStrings, responseVoid, RIL_CMD_PROXY_2, RIL_CHANNEL_1},
 {RIL_REQUEST_CANCEL_DEVICE_SWITCH, dispatchVoid, responseVoid, RIL_CMD_PROXY_2, RIL_CHANNEL_1},
 {RIL_REQUEST_SET_INCOMING_VIRTUAL_LINE, dispatchStrings, responseVoid, RIL_CMD_PROXY_1, RIL_CHANNEL_5},
 {RIL_REQUEST_EXIT_SCBM, dispatchVoid, responseVoid, RIL_CMD_PROXY_1, RIL_CHANNEL_5},
 {RIL_REQUEST_SEND_RSU_REQUEST, dispatchVoid, responseVoid, RIL_CMD_PROXY_1, RIL_CHANNEL_1},
 {RIL_REQUEST_SWITCH_RCS_ROI_STATUS, radio::switchRcsRoiStatusResponse},
 {RIL_REQUEST_UPDATE_RCS_CAPABILITIES, radio::updateRcsCapabilitiesResponse},
 {RIL_REQUEST_UPDATE_RCS_SESSION_INFO, radio::updateRcsSessionInfoResponse},
#endif // MTK_USE_HIDL
