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
package com.mediatek.contacts.util;

import com.mediatek.contacts.util.Log;

public class ContactsPortableUtils {

    private static final String TAG = "ContactsPortableUtils";
    public static final boolean MTK_STORAGE_SUPPORT = isMtkStorageSupport();
    public static final boolean MTK_TELEPHONY_SUPPORT = isMtkTelephonySupport();
    public static final boolean MTK_PHONE_BOOK_SUPPORT = isMtkPhoneBookSupport();

    private static boolean isMtkStorageSupport() {
        try {
            Class<?> storageManager = Class.forName("com.mediatek.storage.StorageManagerEx");
        } catch (ClassNotFoundException e) {
            Log.e(TAG, "StorageManagerEx class not found!");
            return false;
        }
        return true;
    }

    private static boolean isMtkTelephonySupport() {
        try {
            Class<?> telephonyEx = Class.forName("com.mediatek.internal.telephony.IMtkTelephonyEx");
            Class<?> alphaTag = Class.forName("com.mediatek.internal.telephony.phb.AlphaTag");
            Class<?> usimGroup = Class.forName("com.mediatek.internal.telephony.phb.UsimGroup");
            Class<?> telephonyManagerEx = Class
                    .forName("com.mediatek.telephony.TelephonyManagerEx");
        } catch (ClassNotFoundException e) {
            Log.e(TAG, "MTK telephony class not found!");
            return false;
        }
        return true;
    }

    private static boolean isMtkPhoneBookSupport() {
        try {
            Class<?> iccPhoneBook = Class.forName(
                  "com.mediatek.internal.telephony.phb.IMtkIccPhoneBook");
            iccPhoneBook.getDeclaredMethod("getUsimAasList", int.class);
            iccPhoneBook.getDeclaredMethod("getUsimGroups", int.class);
            iccPhoneBook.getDeclaredMethod("hasSne", int.class);
        } catch (ClassNotFoundException e) {
            Log.e(TAG, "Android phonebook class not found!");
            return false;
        } catch (NoSuchMethodException e) {
            Log.e(TAG, "MTK phonebook api not found!");
            return false;
        }
        return true;
    }
}
