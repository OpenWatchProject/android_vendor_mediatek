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

import java.util.UUID;

/**
 * Trustlet Container class
 */
public class TrustletContainer implements Parcelable {
	/** The UUID addressing the trustlet container */
	private UUID trustletId_;
	private TrustletContainerStateParcel state_;

	public TrustletContainer() {
	}

	public TrustletContainer(UUID trustletId, TrustletContainerState state) {
		this.trustletId_ = trustletId;
		this.state_ = new TrustletContainerStateParcel(state);
	}

	@Override
	public int describeContents() {
		return 0;
	}

	@Override
	public void writeToParcel(Parcel dest, int flags) {
		dest.writeString(trustletId_.toString());
		if (state_ == null)
			dest.writeString(null);
		else
			state_.writeToParcel(dest, 0);
	}

	public static TrustletContainer readFromParcel(Parcel source) {
		final TrustletContainer tc = new TrustletContainer();
		tc.setTrustletId(source.readString());
		tc.setState(TrustletContainerStateParcel.CREATOR.createFromParcel(source));
		return tc;
	}

	public static final Creator<TrustletContainer> CREATOR = new Creator<TrustletContainer>() {
		@Override
		public TrustletContainer createFromParcel(Parcel source) {
			return readFromParcel(source);
		}

		@Override
		public TrustletContainer[] newArray(int size) {
			return new TrustletContainer[size];
		}
	};

	public UUID trustletId() {
		return trustletId_;
	}

	public void setTrustletId(UUID trustletId) {
		this.trustletId_ = trustletId;
	}

	public void setTrustletId(String trustletId) {
		if (trustletId != null)
			this.trustletId_ = UUID.fromString(trustletId);
		else
			this.trustletId_ = null;
	}

	public TrustletContainerStateParcel state() {
		return state_;
	}

	public void setState(TrustletContainerStateParcel state) {
		this.state_ = state;
	}

	@Override
	public String toString() {
		return "TrustletContainer{" + "trustletId=" + trustletId_ + ", state=" + state_ + '}';
	}
}
