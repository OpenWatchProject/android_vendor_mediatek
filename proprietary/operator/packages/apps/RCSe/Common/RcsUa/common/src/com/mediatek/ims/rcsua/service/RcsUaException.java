/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2018. All rights reserved.
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

package com.mediatek.ims.rcsua.service;

import android.os.Parcel;
import android.os.Parcelable;

import java.io.IOException;
import java.util.Arrays;

public class RcsUaException implements Parcelable {

    public static final Parcelable.Creator<RcsUaException> CREATOR =
        new Parcelable.Creator<RcsUaException>() {
        public RcsUaException createFromParcel(Parcel in) {
            return new RcsUaException(in);
        }

        public RcsUaException[] newArray(int size) {
            return new RcsUaException[size];
        }
    };

    public RcsUaException() {
        this.clazz = "";
        this.message = "";
    }

    protected RcsUaException(Parcel in) {
        readFromParcel(in);
    }

    public void set(Exception e) throws IllegalArgumentException {
        if (e == null) {
            throw new IllegalArgumentException("Cannot set a null exception");
        }
        Class clazz = e.getClass();
        if (!Arrays.asList(EXCEPTIONS).contains(clazz)) {
            throw new IllegalArgumentException("Unexpected exception class: " +
                clazz.getCanonicalName());
        }
        this.clazz = clazz.getCanonicalName();
        this.message = e.getMessage() != null ? e.getMessage() : "";
    }

    /**
     * @return true if this error has been set, false otherwise.
     */
    public boolean isSet() {
        return clazz != null && !clazz.isEmpty();
    }

    /**
     * Throws the exception this object represents.
     *
     * @throws IOException
     * @throws SecurityException
     * @throws IllegalStateException
     * @throws IllegalArgumentException
     * @throws UnsupportedOperationException
     * @throws NullPointerException
     */
    public void throwException() throws
            IOException,
            SecurityException,
            IllegalStateException,
            IllegalArgumentException,
            UnsupportedOperationException,
            NullPointerException {
        if (clazz.equals(java.io.IOException.class.getCanonicalName())) {
            throw new IOException(message) ;
        } else if (clazz.equals(java.lang.SecurityException.class.getCanonicalName())) {
            throw new SecurityException(message);
        } else if (clazz.equals(java.lang.IllegalStateException.class.getCanonicalName())) {
            throw new IllegalStateException(message);
        } else if (clazz.equals(java.lang.IllegalArgumentException.class.getCanonicalName())) {
            throw new IllegalArgumentException(message);
        } else if (clazz.equals(
            java.lang.UnsupportedOperationException.class.getCanonicalName())) {
            throw new UnsupportedOperationException(message);
        } else if (clazz.equals(java.lang.NullPointerException.class.getCanonicalName())) {
            throw new NullPointerException(message);
        }
    }

    public int describeContents() {
        return 0;
    }

    public void writeToParcel(Parcel out, int flags) {
        out.writeString(clazz);
        out.writeString(message);
    }

    public void readFromParcel(Parcel in) {
        clazz = in.readString();
        message = in.readString();
    }

    /**
     * Clears the error.
     */
    public void clear() {
        this.clazz = "";
        this.message = "";
    }

    @Override
    public String toString() {

        if (isSet()) {
            return "Exception: name[" + clazz + "], message[" + message + "]";
        }
        return "Exception: none";
    }

    private static final Class[] EXCEPTIONS = {
            java.io.IOException.class,
            java.lang.SecurityException.class,
            java.lang.IllegalStateException.class,
            java.lang.IllegalArgumentException.class,
            java.lang.UnsupportedOperationException.class,
            java.lang.NullPointerException.class
    };

    private String clazz;
    private String message;
}
