/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2010. All rights reserved.
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

package com.mediatek.cb.cbsettings;

import static com.mediatek.cb.cbsettings.TimeConsumingPreferenceActivity.EXCEPTION_ERROR;

import android.content.ContentResolver;
import android.content.Context;
import android.database.Cursor;
import android.net.Uri;
import android.os.Handler;
import android.os.Message;
import android.preference.CheckBoxPreference;
import android.telephony.SmsManager;
import android.telephony.SubscriptionManager;
import android.telephony.TelephonyManager;
import android.util.AttributeSet;

import com.android.internal.telephony.PhoneConstants;
import com.android.internal.telephony.gsm.SmsBroadcastConfigInfo;
import com.android.mms.R;
import com.android.mms.util.MmsLog;

import java.util.ArrayList;

import mediatek.telephony.MtkSmsManager;


public class CellBroadcastCheckBox extends CheckBoxPreference {
    private static final String LOG_TAG = "MmsCB/CellBroadcastCheckBox";
    private static final boolean DBG = true;
    private static final int QUERY_CBSMS_ACTIVATION = 100;

    public TimeConsumingPreferenceListener mListener;
    public boolean mIsCbEanbled = false;
    int mSubId;

    public CellBroadcastCheckBox(Context context, AttributeSet attrs) {
        super(context, attrs);
    }

    private Handler mUiHandler = new Handler() {
        @Override
        public void handleMessage(Message msg) {
            switch (msg.what) {
            case QUERY_CBSMS_ACTIVATION:
                setChecked(mIsCbEanbled);
                setSummary(mIsCbEanbled ? R.string.sum_cell_broadcast_control_on
                        : R.string.sum_cell_broadcast_control_off);
                break;
            default:
                MmsLog.d(LOG_TAG, "mUiHandler msg unhandled.");
                break;
            }
        }
    };

    @Override
    protected void onClick() {
        super.onClick();
        boolean state = isChecked();
        setCBState(state);
        setChecked(state);
    }

    public void init(int subId, TimeConsumingPreferenceListener listener, boolean skipReading) {
        MmsLog.d(LOG_TAG, "init, subId = " + subId);
        mListener = listener;
        mSubId = subId;

        if (!skipReading) {
            boolean hasIccCard;
            hasIccCard = hasIccCard(mSubId);
            MmsLog.d(LOG_TAG, "hasIccCard = " + hasIccCard);
            if (hasIccCard) {
                getCBState();
                setEnabled(true);
            } else {
                setChecked(false);
                setEnabled(false);
            }
        }
    }

    private void getCBState() {
        MmsLog.d(LOG_TAG, "getCBState start");
        new Thread(new Runnable() {
            @Override
            public void run() {
                mIsCbEanbled = MtkSmsManager.getSmsManagerForSubscriptionId(mSubId)
                        .queryCellBroadcastSmsActivation();
                mUiHandler.sendEmptyMessage(QUERY_CBSMS_ACTIVATION);
                MmsLog.d(LOG_TAG, "getCBState end mIsCbEanbled = " + mIsCbEanbled);
            }
        }).start();
    }

    private void setCBState(final boolean state) {
        MmsLog.d(LOG_TAG, "setCBState start,state = " + state);
        if (mListener != null) {
            mListener.onStarted(CellBroadcastCheckBox.this, false);
        }
        boolean isSetSuccess = MtkSmsManager.getSmsManagerForSubscriptionId(mSubId)
                .activateCellBroadcastSms(state);
        MmsLog.d(LOG_TAG, "activateCellBroadcastSms and isSetSuccess = " + isSetSuccess);
        if (!isSetSuccess) {
            handleSetStateResponse();
            return;
        }

        if (state) {
            new Thread(new Runnable() {
                public void run() {
                        RecoverChannelSettings setting =
                                new RecoverChannelSettings(mSubId,
                                        getContext().getContentResolver());
                        if (!setting.updateChannelStatus()) {
                            if (mListener != null) {
                                mListener.onError(CellBroadcastCheckBox.this, EXCEPTION_ERROR);
                            }
                        }
                        if (mListener != null) {
                            mListener.onFinished(CellBroadcastCheckBox.this, false);
                        }
                        MmsLog.d(LOG_TAG, "setCBState end");
                }
            }).start();
            return;
        }
        if (mListener != null) {
            mListener.onFinished(CellBroadcastCheckBox.this, false);
        }
        MmsLog.d(LOG_TAG, "setCBState end");
    }

    private void handleSetStateResponse() {
        if (mListener != null) {
            mListener.onFinished(CellBroadcastCheckBox.this, false);
            mListener.onError(CellBroadcastCheckBox.this, EXCEPTION_ERROR);
        }
        getCBState();
        return;
    }

    /**
     * @return true if a ICC card is present
     *
     * @param slotId
     * @return
     */
    public boolean hasIccCard(int subId) {
        boolean hasIccCard;
        if (subId > 0) {
            int slotId = SubscriptionManager.getSlotIndex(subId);
            TelephonyManager manager = TelephonyManager.getDefault();
            hasIccCard = manager.hasIccCard(slotId);
        } else {
            hasIccCard = TelephonyManager.getDefault().hasIccCard();
        }
        MmsLog.d(LOG_TAG, "[hasIccCard], subId = " + subId + "; hasIccCard = " + hasIccCard);
        return hasIccCard;
    }
}

class RecoverChannelSettings {

    private static final String LOG_TAG = "MmsCB/RecoverChannelSettings";
    private static final String KEYID = "_id";
    private static final String NAME = "name";
    private static final String NUMBER = "number";
    private static final String ENABLE = "enable";
    private static final String SUBID = "sub_id";
    private static final Uri CHANNEL_URI = Uri.parse("content://cb/channel");

    private Uri mUri = CHANNEL_URI;
    private int mSubId;
    private ContentResolver mResolver = null;

    public RecoverChannelSettings(int subId, ContentResolver resolver) {
        mSubId = subId;
        this.mResolver = resolver;
        mUri = Uri.parse("content://cb/channel").buildUpon()
                .appendQueryParameter(PhoneConstants.SUBSCRIPTION_KEY, String.valueOf(mSubId))
                .build();
        MmsLog.dpi(LOG_TAG, "mUri = " + mUri);
    }

    private ArrayList<CellBroadcastChannel> mChannelArray = new ArrayList<CellBroadcastChannel>();

    boolean queryChannelFromDatabase() {
        MmsLog.d(LOG_TAG, "queryChannelFromDatabase start");
        String[] projection = new String[] { KEYID, NAME, NUMBER, ENABLE, SUBID };
        Cursor cursor = null;
        try {
            cursor = mResolver.query(mUri, projection, null, null, null);
            if (cursor != null) {
                while (cursor.moveToNext()) {
                    CellBroadcastChannel channel = new CellBroadcastChannel();
                    channel.setChannelId(cursor.getInt(2));
                    channel.setKeyId(cursor.getInt(0)); // keyid for delete or edit
                    channel.setChannelName(cursor.getString(1));
                    channel.setChannelState(cursor.getInt(3) == 1);
                    channel.setChannelSubId(cursor.getInt(4));
                    mChannelArray.add(channel);
                }
            }
        } finally {
            if (cursor != null) {
                cursor.close();
            }
        }
        MmsLog.d(LOG_TAG, "queryChannelFromDatabase end");
        return true;
    }

    /**
     * when enable channels, we set once that these channels which are enable
     * and channelId neighboring in the DB to reduce times to reduce API.
     * eg: the channel id maybe is 1(true),2(true),3(false) ,4(true), 5(false),6(true)
     * we send three times (1,2; 4; 6)
      */
    public boolean updateChannelStatus() {
        MmsLog.d(LOG_TAG, "updateChannelStatus start");
        if (!queryChannelFromDatabase()) {
            MmsLog.d(LOG_TAG, "queryChannelFromDatabase failure!");
            return false;
        }
        int length = mChannelArray.size();
        MmsLog.d(LOG_TAG, "updateChannelStatus length: " + length);
        //SmsBroadcastConfigInfo infoList = null;
        // update the channel status to SmsManager
        int channelId = -1;
        boolean channelState = false;
        // need start & endid in case range need to be enabled.
        int startid = -1;
        int endid = -1;

        for (int i = 0; i < length; i++) {
            channelId = mChannelArray.get(i).getChannelId();
            channelState = mChannelArray.get(i).getChannelState();
            if (channelState) {
                if (!setChannelStatus(channelId, channelId, channelState)) {
                    MmsLog.d(LOG_TAG, "updateChannelStatus error");
                    //return false;
                }
            }
        }
        MmsLog.d(LOG_TAG, "updateChannelStatus end");
        return true;
    }

    private boolean setChannelStatus(int startid, int endid, boolean status) {
        MmsLog.d(LOG_TAG, "setChannelStatus channel_id:" + startid + "endid:" + endid);
        int ranType = SmsManager.CELL_BROADCAST_RAN_TYPE_GSM;
        boolean result = true;

        if (status) {
            if (endid == startid) {
                result = SmsManager.getSmsManagerForSubscriptionId(mSubId)
                    .enableCellBroadcast(startid, ranType);
            } else {
                result = SmsManager.getSmsManagerForSubscriptionId(mSubId)
                    .enableCellBroadcastRange(startid, endid, ranType);
            }
        }
        return result;
    }
}
