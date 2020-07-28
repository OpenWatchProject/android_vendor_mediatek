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
package com.mediatek.contacts.model.account;

import android.content.Context;
import android.provider.ContactsContract.CommonDataKinds.Nickname;
import android.provider.ContactsContract.CommonDataKinds.Phone;
import android.provider.ContactsContract.CommonDataKinds.Photo;
import android.provider.ContactsContract.CommonDataKinds.StructuredName;

import com.android.contacts.R;
import com.android.contacts.model.account.SimAccountType;
import com.android.contacts.model.dataitem.DataKind;
import com.google.android.collect.Lists;

import com.mediatek.contacts.util.AccountTypeUtils;
import com.mediatek.contacts.util.Log;

public class GsimAccountType extends SimAccountType {
    private static final String TAG = "GsimAccountType";

    public static final String ACCOUNT_TYPE = AccountTypeUtils.ACCOUNT_TYPE_SIM;

    public GsimAccountType(Context context, String resPackageName) {
        Log.i(TAG, "[GsimAccountType]resPackageName:" + resPackageName);

        this.accountType = ACCOUNT_TYPE;
        this.resourcePackageName = null;
        this.syncAdapterPackageName = resPackageName;
        this.titleRes = R.string.account_sim_only;
        this.iconRes = R.drawable.quantum_ic_sim_card_vd_theme_24;
        try {
            addDataKindStructuredName2(context);
            addDataKindName2(context);
            addDataKindPhone(context);
            addDataKindPhoto(context);

            mIsInitialized = true;
        } catch (DefinitionException e) {
            Log.e(TAG, "[GsimAccountType]DefinitionException:", e);
        }
    }

    @Override
    protected DataKind addDataKindPhone(Context context) throws DefinitionException {
        final DataKind kind = super.addDataKindPhone(context);
        kind.typeColumn = Phone.TYPE;
        kind.typeList = Lists.newArrayList();
        kind.typeList.add(buildPhoneType(Phone.TYPE_MOBILE).setSpecificMax(2));
        kind.typeOverallMax = 1;
        kind.fieldList = Lists.newArrayList();
        kind.fieldList.add(new EditField(Phone.NUMBER, R.string.phoneLabelsGroup, FLAGS_PHONE));

        return kind;
    }

    @Override
    protected DataKind addDataKindPhoto(Context context) throws DefinitionException {
        final DataKind kind = super.addDataKindPhoto(context);
        kind.typeOverallMax = 1;
        kind.fieldList = Lists.newArrayList();
        kind.fieldList.add(new EditField(Photo.PHOTO, -1, -1));

        return kind;
    }

    @Override
    public boolean isGroupMembershipEditable() {
        return false;
    }

    @Override
    public boolean isIccCardAccount() {
        return true;
    }
}
