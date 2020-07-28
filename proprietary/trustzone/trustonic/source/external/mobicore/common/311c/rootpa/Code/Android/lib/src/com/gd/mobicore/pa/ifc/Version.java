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

import java.util.Map;

/**
 * Contains the device's product ID and a collection of version numbers for various software components installed on
 * the device.
 */
public class Version implements Parcelable {
    public final static String VERSION_FIELD_TAG="TAG";
    public final static String VERSION_FIELD_TAG1ALL="TAG1ALL";
    public final static String VERSION_FIELD_MCI="MCI";
    public final static String VERSION_FIELD_SO="SO";
    public final static String VERSION_FIELD_MCLF="MCLF";
    public final static String VERSION_FIELD_CONT="CONT";
    public final static String VERSION_FIELD_MCCONF="MCCONF";
    public final static String VERSION_FIELD_TLAPI="TLAPI";
    public final static String VERSION_FIELD_DRAPI="DRAPI";
    public final static String VERSION_FIELD_CMP="CMP";

	private String productId_;
	private Bundle version_;

    public Version() {
    }

    public Version(String productId, Bundle version) {
        setVersion(version);
		setProductId(productId);
    }

	public String productId() {
		return productId_;
	}

	public void setProductId(String productId) {
		this.productId_ = productId;
	}

	public Bundle version() {
		return version_;
	}

	public void setVersion(Bundle version) {
		this.version_ = version;
	}

//parcelable interface

    public static final Creator<Version> CREATOR = new Creator<Version>() {
        public Version createFromParcel(Parcel in) {
            return new Version(in);
        }

        public Version[] newArray(int size) {
            return new Version[size];
        }
    };

    private Version(Parcel in) {
        readFromParcel(in);
    }

    public void readFromParcel(Parcel in) {
		productId_ = in.readString();
        version_ = in.readBundle();
    }

    @Override
    public int describeContents() {
        return 0;
    }

    @Override
    public void writeToParcel(Parcel out, int flags) {
        if(productId_!=null){
    		out.writeString(productId_);
        }
        if(version_!=null){
            out.writeBundle(version_);
        }
    }

}
