/*
 * Copyright (C) 2013 The Android Open Source Project
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

package com.android.incallui;

import android.net.Uri;
import android.telecom.PhoneAccount;
import android.telephony.PhoneNumberUtils;
import com.android.dialer.common.LogUtil;
import com.android.incallui.call.DialerCall;

import java.security.MessageDigest;
import java.security.NoSuchAlgorithmException;
import java.util.Map;

/** Manages logging for the entire class. */
public class Log {

  public static void d(String tag, String msg) {
    LogUtil.d(tag, msg);
  }

  public static void d(Object obj, String msg) {
    LogUtil.d(getPrefix(obj), msg);
  }

  public static void d(Object obj, String str1, Object str2) {
    LogUtil.d(getPrefix(obj), str1 + str2);
  }

  public static void v(Object obj, String msg) {
    LogUtil.v(getPrefix(obj), msg);
  }

  public static void v(Object obj, String str1, Object str2) {
    LogUtil.v(getPrefix(obj), str1 + str2);
  }

  public static void e(String tag, String msg, Exception e) {
    LogUtil.e(tag, msg, e);
  }

  public static void e(String tag, String msg) {
    LogUtil.e(tag, msg);
  }

  public static void e(Object obj, String msg, Exception e) {
    LogUtil.e(getPrefix(obj), msg, e);
  }

  public static void e(Object obj, String msg) {
    LogUtil.e(getPrefix(obj), msg);
  }

  public static void i(String tag, String msg) {
    LogUtil.i(tag, msg);
  }

  public static void i(Object obj, String msg) {
    LogUtil.i(getPrefix(obj), msg);
  }

  public static void w(Object obj, String msg) {
    LogUtil.w(getPrefix(obj), msg);
  }

  public static String piiHandle(Object pii) {
    if (pii == null || LogUtil.isVerboseEnabled()) {
      return String.valueOf(pii);
    }

    if (pii instanceof Uri) {
      Uri uri = (Uri) pii;

      // All Uri's which are not "tel" go through normal pii() method.
      if (!PhoneAccount.SCHEME_TEL.equals(uri.getScheme())) {
        return pii(pii);
      } else {
        pii = uri.getSchemeSpecificPart();
      }
    }

    String originalString = String.valueOf(pii);
    StringBuilder stringBuilder = new StringBuilder(originalString.length());
    for (char c : originalString.toCharArray()) {
      if (PhoneNumberUtils.isDialable(c)) {
        stringBuilder.append('*');
      } else {
        stringBuilder.append(c);
      }
    }
    return stringBuilder.toString();
  }

  /**
   * Redact personally identifiable information for production users. If we are running in verbose
   * mode, return the original string, otherwise return a SHA-1 hash of the input string.
   */
  public static String pii(Object pii) {
    if (pii == null || LogUtil.isVerboseEnabled()) {
      return String.valueOf(pii);
    }
    return "[" + secureHash(String.valueOf(pii).getBytes()) + "]";
  }

  private static String secureHash(byte[] input) {
    MessageDigest messageDigest;
    try {
      messageDigest = MessageDigest.getInstance("SHA-1");
    } catch (NoSuchAlgorithmException e) {
      return null;
    }
    messageDigest.update(input);
    byte[] result = messageDigest.digest();
    return encodeHex(result);
  }

  private static String encodeHex(byte[] bytes) {
    StringBuffer hex = new StringBuffer(bytes.length * 2);

    for (int i = 0; i < bytes.length; i++) {
      int byteIntValue = bytes[i] & 0xff;
      if (byteIntValue < 0x10) {
        hex.append("0");
      }
      hex.append(Integer.toString(byteIntValue, 16));
    }

    return hex.toString();
  }

  private static String getPrefix(Object obj) {
    return (obj == null ? "" : (obj.getClass().getSimpleName()));
  }

  /// M: ------------------------- MediaTek features ------------------------

  /**
   * M: [log optimize]tag for CC debug log.
   */
  private static final String CC_DEBUG_TAG = "CC";

  /**
   * M: [log optimize]The debug log template.
   * [Debug][CC][OP][Module][unique key][action]message.
   * The unique key is phone number.
   * Action includes dial/hold/...
   * e.g. [Debug][CC][InCallUI][OP][Dial][10086][1]send dial to Telecom.
   */
  private static final String OP_LOG_TEMPLATE = "[Debug][CC][InCallUI][OP][%s][%s][%s]%s";

  /**
   * M: [log optimize]The debug log template.
   * e.g. [Debug][CC][InCallUI][Notify][Dialing][10086][1]telecom call state changed.
   */
  private static final String NOTIFY_LOG_TEMPLATE = "[Debug][CC][InCallUI][Notify][%s][%s][%s]%s";

  /**
   * M: [log optimize]The debug log template.
   * e.g. [Debug][CC][InCallUI][Dump][10086][Disconnected]DisconnectCause:xxxxxx.
   */
  private static final String DUMP_LOG_TEMPLATE = "[Debug][CC][InCallUI][Dump][%s][%s]%s";
  private static final String TAG_CONFERENCE = "ConferenceCall";

  /**
   * M: [log optimize]The Operation actions who represents the commands sent to Telecom.
   */
  public static class CcOpAction {
    public static final String ANSWER = "Answer";
    public static final String REJECT = "Reject";
    public static final String HOLD = "Hold";
    public static final String UNHOLD = "Unhold";
    public static final String SWAP = "Swap";
    public static final String DISCONNECT = "Hangup";
    public static final String MERGE = "Merge";
    public static final String SEPARATE = "Split";
    public static final String INVITE = "AddMember";
    public static final String REMOVE_MEMBER = "RemoveMember";
  }

  /**
   * M: [log optimize]The notifications of received from Telecom.
   */
  public static class CcNotifyAction {
    public static final String DIALING = "Dialing";
    public static final String INCOMING = "Incoming";
    public static final String CONNECTING = "Connecting";
    public static final String ACTIVE = "Active";
    public static final String ONHOLD = "Onhold";
    public static final String DISCONNECTING = "Disconnecting";
    public static final String DISCONNECTED = "Disconnected";
    public static final String CONFERENCED = "Conferenced";
    public static final String NEW = "NewCallAdded";
  }

  /**
   * M: [log optimize]Log the operations from InCallUI for debug.
   * @param call the call being logging.
   * @param action the operation details.
   * @param msg the additional message.
   */
  public static void op(DialerCall call, String action, String msg) {
    if (call != null) {
      String callId = call.getId();
      String numberTag;
      if (call.isConferenceCall()) {
        numberTag = TAG_CONFERENCE;
      } else {
        numberTag = call.getNumber();
      }
      i(CC_DEBUG_TAG, String.format(OP_LOG_TEMPLATE, action, numberTag, callId, msg));
    }
  }

  /**
   * M: [log optimize]Log the notify from Telecom for debug.
   * @param call the call who received the notification.
   * @param action what action is received.
   * @param msg the additional message.
   */
  public static void notify(DialerCall call, String action, String msg) {
    if (call != null) {
      String callId = call.getId();
      String numberTag;
      if (call.isConferenceCall()) {
        numberTag = TAG_CONFERENCE;
      } else {
        numberTag = call.getNumber();
      }
      i(CC_DEBUG_TAG, String.format(NOTIFY_LOG_TEMPLATE, action, numberTag, callId, msg));
    }
  }

  /**
   * M: [log optimize]dump the call related information for debug.
   * @param call the call to dump.
   * @param action the dump action.
   * @param msg the additional message.
   * @param values the key:value pair to show in log.
   */
  public static void dump(DialerCall call, String action, String msg, Map<String, String> values) {
    if (call == null || values == null || values.isEmpty()) {
      return;
    }
    StringBuilder sb = new StringBuilder();
    sb.append(msg).append(".");
    for (String key : values.keySet()) {
      sb.append(key).append(":").append(values.get(key)).append(",");
    }
    sb.deleteCharAt(sb.length() - 1);
    i(CC_DEBUG_TAG, String.format(DUMP_LOG_TEMPLATE, call.getNumber(), action, sb.toString()));
  }
}
