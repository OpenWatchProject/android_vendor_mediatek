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

//package com.mediatek.omadownload;
package com.android.providers.downloads;

class OmaStatusHandler {

    //Installation status code
    static final int ATTRIBUTE_MISMATCH = 905;
    static final int DEVICE_ABORTED = 952;
    static final int INSUFFICIENT_MEMORY = 901;
    static final int INVALID_DDVERSION = 951;
    static final int INVALID_DESCRIPTOR = 906;
    static final int LOADER_ERROR = 954;
    static final int LOSS_OF_SERVICE = 903;
    static final int NON_ACCEPTABLE_CONTENT = 953;
    static final int SUCCESS = 900;
    static final int USER_CANCELLED = 902;

    //InstallNotify status code
    static final int DISCARD = 0;
    static final int READY = 1;

    //Maximum number of times to retry a request
    static final int MAXIMUM_RETRY = 3;

    protected static String statusCodeToString(int code) {
        String s = null;
        if (code == ATTRIBUTE_MISMATCH) {
            s = "905 Attribute mismatch";
        } else if (code == DEVICE_ABORTED) {
            s = "952 Device aborted";
        } else if (code == INSUFFICIENT_MEMORY) {
            s = "901 Insufficient memory";
        } else if (code == INVALID_DDVERSION) {
            s = "951 Invalid DDVersion";
        } else if (code == INVALID_DESCRIPTOR) {
            s = "906 Invalid descriptor";
        } else if (code == LOADER_ERROR) {
            s = "954 Loader Error";
        } else if (code == LOSS_OF_SERVICE) {
            s = "903 Loss of Service";
        } else if (code == NON_ACCEPTABLE_CONTENT) {
            s = "953 Non-Acceptable Content";
        } else if (code == SUCCESS) {
            s = "900 Success";
        } else if (code == USER_CANCELLED) {
            s = "902 User Cancelled";
        }
        return s;
    }
}