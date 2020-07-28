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

import android.os.Bundle;
import android.os.Parcel;
import android.os.Parcelable;

/**
 * An 'out' value for an AIDL method that represents a boolean value.
 */
public class BooleanResult implements Parcelable {
    private Boolean result_;

    public BooleanResult() {
    }

    public BooleanResult(Boolean result) {
        setResult(result);
    }

    public Boolean result() {
        return result_;
    }

    public void setResult(final Boolean result) {
        if(result == null) {
            throw new IllegalStateException("Result is null!");
        }

        this.result_ = result;
    }

    //parcelable interface

    public static final Creator<BooleanResult> CREATOR = new Creator<BooleanResult>() {
        public BooleanResult createFromParcel(Parcel in) {
            return new BooleanResult(in);
        }

        public BooleanResult[] newArray(int size) {
            return new BooleanResult[size];
        }
    };

    private BooleanResult(Parcel in) {
        readFromParcel(in);
    }

    public void readFromParcel(Parcel in) {
        result_ = (in.readByte() == 1);
    }

    @Override
    public int describeContents() {
        return 0;
    }

    @Override
    public void writeToParcel(Parcel out, int flags) {
        if(result_ != null){
            out.writeByte((byte) (result_ ? 1 : 0));
        }
    }
}
