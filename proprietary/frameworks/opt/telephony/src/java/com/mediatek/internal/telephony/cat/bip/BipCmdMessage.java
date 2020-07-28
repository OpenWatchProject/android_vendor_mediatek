/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2017. All rights reserved.
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
 * have been modified by MediaTek Inc. All revisions are subject to any receiver\'s
 * applicable license agreements with MediaTek Inc.
 */

package com.mediatek.internal.telephony.cat;

import android.os.Parcel;
import android.os.Parcelable;
import java.util.List;

import com.android.internal.telephony.cat.AppInterface;
import com.android.internal.telephony.cat.CommandDetails;
import com.android.internal.telephony.cat.CommandParams;
import com.android.internal.telephony.cat.TextMessage;
import com.android.internal.telephony.cat.SetEventListParams;

/**
 * Class used to pass CAT messages from telephony to application. Application
 * should call getXXX() to get commands's specific values.
 *
 */
public class BipCmdMessage implements Parcelable {
    // members
    CommandDetails mCmdDet;
    private TextMessage mTextMsg;
    public BearerDesc mBearerDesc = null;
    public int mBufferSize = 0;
    public OtherAddress mLocalAddress = null;
    public DnsServerAddress mDnsServerAddress = null;
    public TransportProtocol mTransportProtocol = null;
    public OtherAddress mDataDestinationAddress = null;

    public String mApn = null;
    public String mLogin = null;
    public String mPwd = null;

    public int mChannelDataLength = 0;
    public int mRemainingDataLength = 0;
    public byte[] mChannelData = null;

    public ChannelStatus mChannelStatusData = null;

    public int mCloseCid = 0;
    public int mSendDataCid = 0;
    public int mReceiveDataCid = 0;
    public boolean mCloseBackToTcpListen = false;
    public int mSendMode = 0;
    public List<ChannelStatus> mChannelStatusList = null;

    public int mInfoType = 0;
    public String mDestAddress = null;
    private SetupEventListSettings mSetupEventListSettings = null;

    public class SetupEventListSettings {
        public int[] eventList;
    }

    BipCmdMessage(CommandParams cmdParams) {
        mCmdDet = cmdParams.mCmdDet;
        if (getCmdType() == null) {
            MtkCatLog.e("[BIP]", "cmd type is null!");
            return;
        }
        switch(getCmdType()) {
        case GET_CHANNEL_STATUS:
            mTextMsg = ((GetChannelStatusParams) cmdParams).textMsg;
            break;
        case OPEN_CHANNEL:
            mBearerDesc = ((OpenChannelParams) cmdParams).bearerDesc;
            mBufferSize = ((OpenChannelParams) cmdParams).bufferSize;
            mLocalAddress = ((OpenChannelParams) cmdParams).localAddress;
            mTransportProtocol = ((OpenChannelParams) cmdParams).transportProtocol;
            mDataDestinationAddress = ((OpenChannelParams) cmdParams).dataDestinationAddress;
            mTextMsg = ((OpenChannelParams) cmdParams).textMsg;

            if (mBearerDesc != null) {
                if (mBearerDesc.bearerType == BipUtils.BEARER_TYPE_GPRS ||
                    mBearerDesc.bearerType == BipUtils.BEARER_TYPE_DEFAULT ||
                    mBearerDesc.bearerType == BipUtils.BEARER_TYPE_UTRAN ||
                    mBearerDesc.bearerType == BipUtils.BEARER_TYPE_EUTRAN) {
                    mApn = ((OpenChannelParams) cmdParams).gprsParams.accessPointName;
                    mLogin = ((OpenChannelParams) cmdParams).gprsParams.userLogin;
                    mPwd = ((OpenChannelParams) cmdParams).gprsParams.userPwd;
                }
            } else {
                MtkCatLog.d("[BIP]", "Invalid BearerDesc object");
            }
            break;
        case CLOSE_CHANNEL:
            mTextMsg = ((CloseChannelParams) cmdParams).textMsg;
            mCloseCid = ((CloseChannelParams) cmdParams).mCloseCid;
            mCloseBackToTcpListen = ((CloseChannelParams) cmdParams).mBackToTcpListen;
            break;
        case RECEIVE_DATA:
            mTextMsg = ((ReceiveDataParams) cmdParams).textMsg;
            mChannelDataLength = ((ReceiveDataParams) cmdParams).channelDataLength;
            mReceiveDataCid = ((ReceiveDataParams) cmdParams).mReceiveDataCid;
            break;
        case SEND_DATA:
            mTextMsg = ((SendDataParams) cmdParams).textMsg;
            mChannelData = ((SendDataParams) cmdParams).channelData;
            mSendDataCid = ((SendDataParams) cmdParams).mSendDataCid;
            mSendMode = ((SendDataParams) cmdParams).mSendMode;
            break;
        case SET_UP_EVENT_LIST:
            mSetupEventListSettings = new SetupEventListSettings();
            mSetupEventListSettings.eventList = ((SetEventListParams) cmdParams).mEventInfo;
            break;
        default:
            break;
        }
    }

    public BipCmdMessage(Parcel in) {
        mCmdDet = in.readParcelable(null);
        mTextMsg = in.readParcelable(null);
        if (getCmdType() == null) {
            MtkCatLog.e("[BIP]", "cmd type is null");
            return;
        }
        switch (getCmdType()) {
        case SET_UP_EVENT_LIST:
            mSetupEventListSettings = new SetupEventListSettings();
            int length = in.readInt();
            mSetupEventListSettings.eventList = new int[length];
            for (int i = 0; i < length; i++) {
                mSetupEventListSettings.eventList[i] = in.readInt();
            }
            break;
        case OPEN_CHANNEL:
            mBearerDesc = in.readParcelable(null);
            break;
        default:
            break;
        }
    }

    @Override
    public void writeToParcel(Parcel dest, int flags) {
        dest.writeParcelable(mCmdDet, 0);
        dest.writeParcelable(mTextMsg, 0);
        if (getCmdType() == null) {
            MtkCatLog.e("[BIP]", "cmd type is null");
            return;
        }
        switch(getCmdType()) {
        case SET_UP_EVENT_LIST:
            dest.writeIntArray(mSetupEventListSettings.eventList);
            break;
        case OPEN_CHANNEL:
            dest.writeParcelable(mBearerDesc, 0);
            break;
        default:
            break;
        }
    }

    public static final Parcelable.Creator<BipCmdMessage> CREATOR =
            new Parcelable.Creator<BipCmdMessage>() {
        @Override
        public BipCmdMessage createFromParcel(Parcel in) {
            return new BipCmdMessage(in);
        }

        @Override
        public BipCmdMessage[] newArray(int size) {
            return new BipCmdMessage[size];
        }
    };

    @Override
    public int describeContents() {
        return 0;
    }

    /* external API to be used by application */
    /**
     * Return command qualifier
     * @internal
     */
    public int getCmdQualifier() {
        return mCmdDet.commandQualifier;
    }

    public AppInterface.CommandType getCmdType() {
        return AppInterface.CommandType.fromInt(mCmdDet.typeOfCommand);
    }

    /**
     * Return bearer description
     * @internal
     */
    public BearerDesc getBearerDesc() {
        return mBearerDesc;
    }
}
