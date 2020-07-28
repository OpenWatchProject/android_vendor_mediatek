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

package com.mediatek.cb.cbsettings;

public class CellBroadcastChannel {
    private int mKeyId;
    private int mChannelId;
    private String mChannelName;
    private boolean mChannelState;
    private int mSubId;

    public CellBroadcastChannel() {
    }

    public CellBroadcastChannel(int keyId, int numberId, String name,
 boolean state, int subId) {
        mKeyId = keyId;
        mChannelId = numberId;
        mChannelName = name;
        mChannelState = state;
        mSubId = subId;
    }

    public CellBroadcastChannel(int numberId, String name, boolean state, int subId) {
        mChannelId = numberId;
        mChannelName = name;
        mChannelState = state;
        mSubId = subId;
    }

    public CellBroadcastChannel(CellBroadcastChannel channel) {
        mKeyId = channel.mKeyId;
        mChannelId = channel.mChannelId ;
        mChannelName = channel.mChannelName ;
        mChannelState = channel.mChannelState;
        mSubId = channel.mSubId;
    }

    public int getKeyId() {
        return mKeyId;
    }

    public void setKeyId(int id) {
        mKeyId = id;
    }

    public int getChannelId() {
        return mChannelId;
    }

    public void setChannelId(int id) {
        mChannelId = id;
    }

    public String getChannelName() {
        return mChannelName;
    }

    public void setChannelName(String name) {
        mChannelName = name;
    }

    public boolean getChannelState() {
        return mChannelState;
    }

    public void setChannelState(boolean state) {
        mChannelState = state;
    }

    public int getChannelSubId() {
        return mSubId;
    }

    public void setChannelSubId(int subId) {
        mSubId = subId;
    }
}
