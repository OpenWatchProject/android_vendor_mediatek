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

package com.mediatek.ims;

/**
 * Contains SIP message.
 *
 */
public class SipMessage {
    public static final int DIR_SEND = 0;
    public static final int DIR_RECEIVE = 1;

    public static final int TYPE_REQUEST = 0;
    public static final int TYPE_RESPONSE = 1;

    public static final int METHOD_INVITE = 1;
    public static final int METHOD_CANCEL = 4;
    public static final int METHOD_BYE = 7;

    public static final int CODE_FOR_REQUEST = 0;
    public static final int CODE_SESSION_PROGRESS = 183;
    public static final int CODE_SESSION_INVITE_FAILED_REMOTE_BUSY = 486;

    public static final String REMOTE_DECLINE_HEADER
            = "Another device sent All Devices Busy response";
    public static final String COMPETION_ELSEWHERE_HEADER = "Call Completion Elsewhere";
    public static final String PULLED_AWAY_HEADER = "Call Has Been Pulled by Another Device";
    public static final String VIDEO_CALL_NOT_AVAILABLE_HEADER
            = "Video call is currently not available";
    public static final String VIDEO_CALL_UNAVAILABLE_HEADER
            = "Video calling is unavailable";
    public static final String NO_RTP_TIMEOUT_HEADER = "rtp-rtcp timeout";
    public static final String CALL_COMPLETED_ELSEWHERE_HEADER = "call completed elsewhere";
    public static final String CALL_COMPLETED_BUSY_EVERYWHERE_HEADER = "busy everywhere";
    public static final String CALL_DECLINED_HEADER = "declined";
    public static final String CALL_REJECTED_HEADER = "call rejected by";
    public static final String CALL_MAXIMUM_ALREADY_REACHED =
            "Simultaneous Call Limit Has Already Been Reached";

    private int mDir;
    private int mType;
    private int mMethod;
    private int mCode;
    private String mReasonHeader = null;

    public SipMessage(String[] msg) {
        mDir = Integer.parseInt(msg[1]);
        mType = Integer.parseInt(msg[2]);
        mMethod = Integer.parseInt(msg[3]);
        mCode = Integer.parseInt(msg[4]);
        mReasonHeader = msg[5];
    }

    public int getDir() {
        return mDir;
    }

    public int getType() {
        return mType;
    }

    public int getMethod() {
        return mMethod;
    }

    public int getCode() {
        return mCode;
    }

    public String getReasonHeader() {
        return mReasonHeader;
    }

    public boolean isRejectedByOthers() {
        if (mDir == DIR_RECEIVE && mType == TYPE_REQUEST && mMethod == METHOD_CANCEL && mCode ==
                CODE_FOR_REQUEST && mReasonHeader != null && mReasonHeader.toLowerCase().contains
                (CALL_REJECTED_HEADER)) {
            return true;
        } else {
            return false;
        }
    }
}
