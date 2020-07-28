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

import java.util.ArrayList;
import java.util.List;

/**
 * Serializes the state of the sp container and a list of installed trustlet containers into a parcel.
 */
public class SPContainerStructure implements Parcelable {
	/** State of the sp container */
	private SPContainerState state_;
	/** The trustlet containers registered for this SPCont */
	private final List<TrustletContainer> tcList_;

	public SPContainerStructure(SPContainerState state, List<TrustletContainer> tcList) {
		this.state_ = state;
		this.tcList_ = tcList;
	}

	public SPContainerStructure() {
		this(null, new ArrayList<TrustletContainer>());
	}

	public void add(TrustletContainer tc) {
		tcList_.add(tc);
	}

	public SPContainerState state() {
		return state_;
	}

	public List<TrustletContainer> tcList() {
		return tcList_;
	}

	public void setState(SPContainerState state) {
		this.state_ = state;
	}

	@Override
	public int describeContents() {
		return 0;
	}

	@Override
	public void writeToParcel(Parcel dest, int flags) {
		if (state_ == null)
			dest.writeString(null);
		else
			dest.writeString(state_.toString());
		dest.writeTypedList(tcList_);
	}

	public static final Creator<SPContainerStructure> CREATOR = new Creator<SPContainerStructure>() {
		@Override
		public SPContainerStructure createFromParcel(Parcel source) {
			SPContainerStructure cs = new SPContainerStructure();
			cs.readFromParcel(source);
			return cs;
		}

		@Override
		public SPContainerStructure[] newArray(int size) {
			return new SPContainerStructure[size];
		}
	};

	/**
	 * Reads a parcel and deserializes it into an instance of this class.
	 * @param source parcel data
	 * @return never null
	 * @throws IllegalArgumentException if the parcel contains a state string that does not resolve to an enumeration
	 */
	public void readFromParcel(Parcel source) {
		final String state = source.readString();
		if (state != null && state.length() > 0)
			this.state_ = SPContainerState.valueOf(state);
		source.readTypedList(this.tcList_, TrustletContainer.CREATOR);
	}

	@Override
	public String toString() {
		return "SPContainerStructure{" + "state=" + state_ + ", tcList=" + tcList_ + '}';
	}
}
