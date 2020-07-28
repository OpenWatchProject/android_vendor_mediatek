/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2011. All rights reserved.
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
 */

package com.mediatek.dialer.compat;

import android.content.Context;
import com.mediatek.provider.MtkContactsContract.Aas;
import android.provider.ContactsContract.RawContacts;
import android.provider.ContactsContract;
import android.text.TextUtils;
import com.android.dialer.util.PermissionsUtil;

import com.mediatek.provider.MtkContactsContract;

/**
 * Compatibility utility class about ContactsContract.
 */
public class ContactsCompat {
  /**
   * Compatibility utility class about android.provider.ContactsContract.RawContacts
   */
  public static class RawContactsCompat {
    //refer to RawContacts.INDICATE_PHONE_SIM
    public static final String INDICATE_PHONE_SIM = "indicate_phone_or_sim_contact";
    //refer to RawContacts.INDEX_IN_SIM
    public static final String INDEX_IN_SIM = "index_in_sim";
    //refer to RawContacts.IS_SDN_CONTACT
    public static final String IS_SDN_CONTACT = "is_sdn_contact";
  }

  /**
   * Compatibility utility class about android.provider.ContactsContract.RawContacts
   */
  public static class PhoneLookupCompat {
    //refer to PhoneLookup.INDICATE_PHONE_SIM
    public static final String INDICATE_PHONE_SIM = "indicate_phone_or_sim_contact";
    //refer to PhoneLookup.IS_SDN_CONTACT
    public static final String INDEX_IN_SIM = "index_in_sim";
    //refer to PhoneLookup.IS_SDN_CONTACT
    public static final String IS_SDN_CONTACT = "is_sdn_contact";
  }

  /**
   * Compatibility utility class about ContactsContract.CommonDataKinds.Phone.
   */
  public static class PhoneCompat {
    private static final String PHONE_CLASS =
            "com.mediatek.provider.MtkContactsContract$CommonDataKinds$Phone";
    private static final String GET_TYPE_LABEL_METHOD = "getTypeLabel";

    public static CharSequence getTypeLabel(Context context, int labelType, CharSequence label) {
        CharSequence res = "";
        if (DialerCompatExUtils.isMethodAvailable(PHONE_CLASS, GET_TYPE_LABEL_METHOD, Context.class,
                int.class, CharSequence.class)) {
            if (labelType == Aas.PHONE_TYPE_AAS && !TextUtils.isEmpty(label)
                    && !PermissionsUtil.hasContactsReadPermissions(context)) {
                return "";
            }
            ///M: Using new API for AAS phone number label lookup.
            res = MtkContactsContract.CommonDataKinds.Phone.getTypeLabel(context, labelType, label);
        } else {
            res = ContactsContract.CommonDataKinds.Phone.getTypeLabel(
                    context.getResources(), labelType, label);
        }
        return res;
    }
  }

  /**
   * Compatibility utility class android.provider.ContactsContract.CommonDataKinds.ImsCall.
   */
  public static class ImsCallCompat {
    /**
     * refer to ContactsContract.CommonDataKinds.ImsCall.CONTENT_ITEM_TYPE.
     * MIME type used when storing this in data table.
     * @internal
     */
    public static final String CONTENT_ITEM_TYPE = "vnd.android.cursor.item/ims";
  }
}
