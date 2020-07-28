/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2015. All rights reserved.
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
package com.mediatek.contacts;

import android.app.Application;
import android.content.Context;

import com.mediatek.contacts.ext.*;
import com.mediatek.contacts.util.ContactsPortableUtils;
import com.mediatek.contacts.util.Log;

import java.lang.reflect.InvocationTargetException;
import java.lang.reflect.Method;
import java.util.HashMap;

public final class ExtensionManager {
    private static final String TAG = ExtensionManager.class.getSimpleName();
    private static ExtensionManager sInstance = null;
    private static IContactsCommonPresenceExtension sContactsCommonPresenceExtension;
    private static Context sContext = null;

    private ExtensionManager() {
        //do-nothing
    }

    public static ExtensionManager getInstance() {
        if (sInstance == null) {
            createInstanceSynchronized();
        }
        return sInstance;
    }

    private static synchronized void createInstanceSynchronized() {
        if (sInstance == null) {
            sInstance = new ExtensionManager();
        }
    }

    public static void registerApplicationContext(Application application) {
        sContext = application.getApplicationContext();
    }

    //-------------------below are getXxxExtension() methonds-----------------//
    public static IOp01Extension getOp01Extension() {
        IOp01Extension extension = ContactsCustomizationUtils.
                getOp01Factory(sContext).makeOp01Ext(sContext);
        return extension;
    }

    public static IContactsCommonPresenceExtension getContactsCommonPresenceExtension() {
        if (sContactsCommonPresenceExtension == null) {
            synchronized (IContactsCommonPresenceExtension.class) {
                if (sContactsCommonPresenceExtension == null) {
                    sContactsCommonPresenceExtension = ContactsCustomizationUtils
                            .getContactsPresenceFactory(sContext)
                            .makeContactsCommonPresenceExt(sContext);
                }
            }
        }
        return sContactsCommonPresenceExtension;
    }

    public static IContactsPickerExtension getContactsPickerExtension() {
        IContactsPickerExtension extension = ContactsCustomizationUtils.
                    getContactsPickerFactory(sContext).makeContactsPickerExt(sContext);
        return extension;
    }

    public static IRcsExtension getRcsExtension() {
        IRcsExtension extension = (IRcsExtension) ContactsCustomizationUtils.
                    getRcsFactory(sContext).makeRcsExt(sContext);
        return extension;
    }

    public static IViewCustomExtension getViewCustomExtension() {
        IViewCustomExtension extension = ContactsCustomizationUtils.
                getWWOPRcsFactory(sContext).makeViewCustomExt(sContext);
        return extension;
    }

    /**
     * Get OP01 RCS RichUi plugin.
     * @return IRcsRichUiExtension.
     */
    public static IRcsRichUiExtension getRcsRichUiExtension() {
        IRcsRichUiExtension extension = (IRcsRichUiExtension) ContactsCustomizationUtils.
                    getRcsRichUiFactory(sContext).makeRcsRichUiExt(sContext);
        return extension;
    }

    public static void resetExtensions() {
        Log.d(TAG, "[resetExtensions]");
        synchronized (IContactsCommonPresenceExtension.class) {
            sContactsCommonPresenceExtension = null;
        }
        ContactsCustomizationUtils.resetCustomizationFactory();
    }
}
