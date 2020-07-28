/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2010. All rights reserved.
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
#ifndef RIL_IMS_H
#define RIL_IMS_H 1

void imsInit(RILSubSystemId subsystem, RIL_SOCKET_ID rid);
extern int rilImsMain(int request, void *data, size_t datalen, RIL_Token t);
extern int rilImsUnsolicited(const char *s, const char *sms_pdu, RILChannelCtx* p_channel);

void requestSetImsEnabled(void * data, size_t datalen, RIL_Token t);
void onImsEnabling(int enable, RIL_SOCKET_ID rid);
void onImsEnabled(int enable, RIL_SOCKET_ID rid);
void onImsRegistrationInfo(char *urc, const RIL_SOCKET_ID rid);
/// M: set Ims capability to MD @{
void requestSetVolteEnabled(void * data, size_t datalen, RIL_Token t);
void requestSetWfcEnabled(void * data, size_t datalen, RIL_Token t);
void requestSetImsVideoEnabled(void * data, size_t datalen, RIL_Token t);
/// @}
void requestSetModemImsConfig(void * data, size_t datalen, RIL_Token t);
void requestSetWfcProfile(void * data, size_t datalen, RIL_Token t);
/// M: IMS Deregistration @{
void requestDeregisterIms(void * data, size_t datalen, RIL_Token t);
void onImsDeregistered(RIL_SOCKET_ID rid);
/// @}
/// M: IMS Provisioning. @{
void requestGetProvisionValue(void * data, size_t datalen, RIL_Token t);
void requestSetProvisionValue(void * data, size_t datalen, RIL_Token t);
/// @}
void requestImsRegistrationState(void * data, size_t datalen, RIL_Token t);
/// M: IMS Provisioning URC. @{
void onGetProvisionValue(char *urc, const RIL_SOCKET_ID rid);
/// @}
void enableImsRegistrationReport(void * data, size_t datalen, RIL_Token t);
/// M: RTP info @{
void requestSetImsRtpReport(void * data, size_t datalen, RIL_Token t);
void onImsRtpReport(char *urc, const RIL_SOCKET_ID rid);
/// @}
void onImsEccUpdated(char *urc, const RIL_SOCKET_ID rid);
void onImsVoPSIndication(char *urc, const RIL_SOCKET_ID rid);
void queryVopsStatus(void * data, size_t datalen, RIL_Token t);

#define OEM_CHANNEL_CTX getRILChannelCtxFromToken(t)

#endif /* RIL_IMS_H */
