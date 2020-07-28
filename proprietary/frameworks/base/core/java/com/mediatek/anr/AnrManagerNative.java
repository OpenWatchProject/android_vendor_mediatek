/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2017. All rights reserved.
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
package com.mediatek.anr;

import android.os.Binder;
import android.os.IBinder;
import android.os.Parcel;
import android.os.RemoteException;

import java.lang.reflect.Method;

/**
 * @hide
 */
public abstract class AnrManagerNative extends Binder implements IAnrManager {

    private static Method sGetService = getServiceManagerMethod("getService",
            new Class[]{String.class});
    private static Method getServiceManagerMethod(String func, Class[] cls) {
        try {
            Class<?> serviceManager = Class.forName("android.os.ServiceManager");
            return serviceManager.getDeclaredMethod(func, cls);
        } catch (Exception e) {
            return null;
        }
    }

    /**
     * Cast a Binder object into an AnrManager interface, generating
     * a proxy if needed.
     */
    public static IAnrManager asInterface(IBinder obj) {
        if (obj == null) {
            return null;
        }
        IAnrManager in =
            (IAnrManager) obj.queryLocalInterface(descriptor);
        if (in != null) {
            return in;
        }

        return new AnrManagerProxy(obj);
    }

    /**
     * Retrieve the system's default/global AnrManager.
     */
    public static IAnrManager getDefault() {
        return gDefault.get();
    }

    public AnrManagerNative() {
        attachInterface(this, descriptor);
    }

    public boolean onTransact(int code, Parcel data, Parcel reply, int flags)
            throws RemoteException {
        switch (code) {
            case INFORM_MESSAGE_DUMP_TRANSACTION: {
                data.enforceInterface(IAnrManager.descriptor);
                String msgInfo = data.readString();
                int pid = data.readInt();
                informMessageDump(msgInfo, pid);
                return true;
            }
        }

        return super.onTransact(code, data, reply, flags);
    }

    public IBinder asBinder() {
        return this;
    }

    private static final Singleton<IAnrManager> gDefault = new Singleton<IAnrManager>() {
        protected IAnrManager create() {
            IBinder binder = null;
            try {
                binder = (IBinder) sGetService.invoke(null, "anrmanager");
            } catch (Exception e) { }

            return asInterface(binder);
        }
    };

    static abstract class Singleton<T> {
        private T mInstance;
        protected abstract T create();

        public final T get() {
            synchronized (this) {
                if (mInstance == null) {
                    mInstance = create();
                }
                return mInstance;
            }
        }
    }
}
