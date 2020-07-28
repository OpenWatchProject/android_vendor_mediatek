/*
 * Copyright (c) 2013 TRUSTONIC LIMITED
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the TRUSTONIC LIMITED nor the names of its
 *    contributors may be used to endorse or promote products derived from
 *    this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
 * TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

package com.gd.mobicore.pa.ifc;

import android.os.Parcel;
import android.os.Parcelable;

/**
 * A unique device ID.
 */
public class SUID implements Parcelable {
    private final static int SUID_LENGTH=16;
    private byte[] suid_;

    public SUID() {
    }

    public SUID(byte[] suid) {
        setSuid(suid);
    }

    public void setSuid(final byte[] suid) {
		if (suid == null)
			throw new IllegalArgumentException("Cannot set null SUID.");
        if (suid.length != SUID_LENGTH)
            throw new IllegalArgumentException("Bad length given, SUID has to be "+SUID_LENGTH+" bytes in length");

        this.suid_ = suid.clone();
    }

    public byte[] suid() {
        return suid_;
    }

    // parcelable interface
    public static final Parcelable.Creator<SUID> CREATOR = new Parcelable.Creator<SUID>() {
        public SUID createFromParcel(Parcel in) {
            return new SUID(in);
        }

        public SUID[] newArray(int size) {
            return new SUID[size];
        }
    };

    private SUID(Parcel in) {
        readFromParcel(in);
    }

    public void readFromParcel(Parcel in) {
        if(in.dataAvail()>=SUID_LENGTH){
            this.suid_ = new byte[SUID_LENGTH];
            in.readByteArray(suid_);
        }
    }

    @Override
    public int describeContents() {
        return 0;
    }

    @Override
    public void writeToParcel(Parcel out, int flags) {
        if(suid_!=null){
            out.writeByteArray(suid_);
        }
    }
}
