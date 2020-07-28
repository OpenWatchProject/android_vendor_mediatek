/*
* Copyright (C) 2014 MediaTek Inc.
* Modification based on code covered by the mentioned copyright
* and/or permission notice(s).
*/
/* Copyright (C) 2011 The Android Open Source Project.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package com.android.exchange.adapter;

import com.android.emailcommon.utility.Utility;

import com.android.exchange.Eas;
import com.android.mail.utils.LogUtils;

import java.io.IOException;
import java.io.InputStream;
import java.text.ParseException;

/**
 * Parse the result of a Settings command.
 *
 * We only send the Settings command in EAS 14.0 after sending a Provision command for the first
 * time.  parse() returns true in the normal case; false if access to the account is denied due
 * to the actual settings (e.g. if a particular device type isn't allowed by the server)
 */
public class SettingsParser extends Parser {

    /** M: Used mOofStatus to save the oof status get from server,
     * use 0 point to server not support Oof, if mOofStatus is not changed after response parse,
     * so think server not support Oof, MTK define;
     * use 1 point to set or get Oof successful, protocol define;
     * use 2 point to set or get Oof fail, protocol define;
     * use 3 point to server Access denied or TCP down, Google define. @{ */
    private int mOofStatus = 0;
    private int mOofState = 0;
    private long mStartTimeInMillis = 0L;
    private long mEndTimeInMillis = 0L;
    private int mIsExternal = 0;
    private String mReplyMessage = null;
    /** @} */

    private static final String TAG = Eas.LOG_TAG;

    public SettingsParser(InputStream in) throws IOException {
        super(in);
    }

    /** M: Used for get the oof seting params @{ */
    public int getOofStatus() {
        return mOofStatus;
    }

    public int getOofState() {
        return mOofState;
    }

    public long getStartTimeInMillis() {
        return mStartTimeInMillis;
    }

    public long getEndTimeInMillis() {
        return mEndTimeInMillis;
    }

    public int getIsExternal() {
        return mIsExternal;
    }

    public String getReplyMessage() {
        return mReplyMessage;
    }
    /** @} */

    @Override
    public boolean parse() throws IOException {
        boolean res = false;
        if (nextTag(START_DOCUMENT) != Tags.SETTINGS_SETTINGS) {
            throw new IOException();
        }
        while (nextTag(START_DOCUMENT) != END_DOCUMENT) {
            if (tag == Tags.SETTINGS_STATUS) {
                //M: MTK oof feature
                mOofStatus = getValueInt();
                LogUtils.d(TAG, "Settings status = %d", mOofStatus);
                if (mOofStatus == 1) {
                    res = true;
                } else {
                    // Access denied = 3; others should never be seen
                    res = false;
                }
            } else if (tag == Tags.SETTINGS_DEVICE_INFORMATION) {
                parseDeviceInformation();
            /** M: Parse oof seting @{ */
            } else if (tag == Tags.SETTINGS_OOF) {
                parseOof();
            /** @} */
            } else {
                skipTag();
            }
        }
        return res;
    }

    public void parseDeviceInformation() throws IOException {
        while (nextTag(Tags.SETTINGS_DEVICE_INFORMATION) != END) {
            if (tag == Tags.SETTINGS_SET) {
                parseSet();
            } else {
                skipTag();
            }
        }
    }

    public void parseSet() throws IOException {
        while (nextTag(Tags.SETTINGS_SET) != END) {
            if (tag == Tags.SETTINGS_STATUS) {
                LogUtils.d(TAG, "Set status = %d", getValueInt());
            } else {
                skipTag();
            }
        }
    }

    /** M: Parse oof seting @{ */
    public void parseOof() throws IOException {
        while (nextTag(Tags.SETTINGS_OOF) != END) {
            if (tag == Tags.SETTINGS_STATUS) {
                mOofStatus = getValueInt();
                LogUtils.d(TAG, "Oof status = %d", mOofStatus);
            } else if (tag == Tags.SETTINGS_GET) {
                parseGet();
            } else {
                skipTag();
            }
        }
    }

    public void parseGet() throws IOException {
        while (nextTag(Tags.SETTINGS_GET) != END) {
            if (tag == Tags.SETTINGS_OOF_STATE) {
                mOofState = getValueInt();
                LogUtils.d(TAG, "Oof state = %d", mOofState);
            } else if (tag == Tags.SETTINGS_START_TIME) {
                String time = getValue();
                try {
                    mStartTimeInMillis = Utility.parseEmailDateTimeToMillis(time);
                } catch (ParseException e) {
                    throw new IOException(e);
                }
            } else if (tag == Tags.SETTINGS_END_TIME) {
                String time = getValue();
                try {
                    mEndTimeInMillis = Utility.parseEmailDateTimeToMillis(time);
                } catch (Exception e) {
                    throw new IOException(e);
                }
            } else if (tag == Tags.SETTINGS_OOF_MESSAGE) {
                parseOofMessage();
            } else {
                skipTag();
            }
        }
    }

    public void parseOofMessage() throws IOException {
        boolean enableExternal = false;
        boolean internalLabel = false;
        while (nextTag(Tags.SETTINGS_OOF_MESSAGE) != END) {
            if (tag == Tags.SETTINGS_APPLIES_TO_EXTERNAL_KNOWN) {
                enableExternal = true;
            } else if (tag == Tags.SETTINGS_APPLIES_TO_INTERNAL) {
                internalLabel = true;
            } else if (tag == Tags.SETTINGS_ENABLED) {
                if (enableExternal) {
                    mIsExternal = getValueInt();
                } else {
                    getValueInt();
                }
            } else if (tag == Tags.SETTINGS_REPLY_MESSAGE) {
                if (internalLabel) {
                    mReplyMessage = getValue();
                    internalLabel = false;
                } else {
                    getValue();
                }
            } else {
                skipTag();
            }
        }
    }
    /** @} */
}
