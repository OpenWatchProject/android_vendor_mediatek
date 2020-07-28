/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2010. All rights reserved.
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

/*****************************************************************************
 * Include
 *****************************************************************************/
#include "RfxNwServiceState.h"

/*****************************************************************************
 * Class RfxController
 *****************************************************************************/

RfxNwServiceState::RfxNwServiceState() {
    mRilVoiceRegState = 0;
    mRilDataRegState = 0;
    mRilVoiceRadioTech = 0;
    mRilDataRadioTech = 0;
    mCdmaNetworkExist = 1;
}

RfxNwServiceState::RfxNwServiceState(int rilVoiceRegState, int rilDataRegState,
        int rilVoiceRadioTech, int rilDataRadioTech, int cdmaNetworkExist) {
    mRilVoiceRegState = rilVoiceRegState;
    mRilDataRegState = rilDataRegState;
    mRilVoiceRadioTech = rilVoiceRadioTech;
    mRilDataRadioTech = rilDataRadioTech;
    mCdmaNetworkExist = cdmaNetworkExist;
}

RfxNwServiceState::~RfxNwServiceState() {
}

int RfxNwServiceState::getRilVoiceRegState() {
    return mRilVoiceRegState;
}

void RfxNwServiceState::setRilVoiceRegState(int rilVoiceRegState) {
    mRilVoiceRegState = rilVoiceRegState;
}

int RfxNwServiceState::getRilDataRegState() {
    return mRilDataRegState;
}

void RfxNwServiceState::setRilDataRegState(int rilDataRegState) {
    mRilDataRegState = rilDataRegState;
}

int RfxNwServiceState::getRilVoiceRadioTech() {
    return mRilVoiceRadioTech;
}

void RfxNwServiceState::setRilVoiceRadioTech(int rilVoiceRadioTech) {
    mRilVoiceRadioTech = rilVoiceRadioTech;
}

int RfxNwServiceState::getRilDataRadioTech() {
    return mRilDataRadioTech;
}

void RfxNwServiceState::setRilDataRadioTech(int rilDataRadioTech) {
    mRilDataRadioTech = rilDataRadioTech;
}

int RfxNwServiceState::getCdmaNetworkExist() {
    return mCdmaNetworkExist;
}

void RfxNwServiceState::setCdmaNetworkExist(int cdmaNetworkExist) {
    mCdmaNetworkExist = cdmaNetworkExist;
}

String8 RfxNwServiceState::toString() {
    return String8::format("cs_reg=%d, cs_rat=%d, ps_reg=%d, ps_rat=%d, cdma_exist=%d",
            mRilVoiceRegState, mRilVoiceRadioTech, mRilDataRegState, mRilDataRadioTech,
            mCdmaNetworkExist);
}
