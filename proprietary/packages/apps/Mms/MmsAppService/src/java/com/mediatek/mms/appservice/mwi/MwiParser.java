package com.mediatek.mms.appservice.mwi;

import android.content.Context;
import android.util.Log;

import java.util.ArrayList;

public class MwiParser {
    private static final String TAG = "Mms/Mwi/MwiParser";
    private static final boolean DEBUG = true;
    private static final String sSeparator = ":";
    private static final String sTerminator = "\n";
    private static final String[] sHeaderTag = new String[]{
                "Message-Waiting", "Message-Account", "Voice-Message",
                "Video-Message", "Fax-Message", "Text-Message"
            };
    private static final String[] sMessageTag = new String[]{
                "To", "From", "Subject", "Date", "Priority",
                "Message-ID", "Message-Context", "Msg-Account"
            };

    /*
     * Parse whole data.
     */
    public static ArrayList<MwiMessage> parseMwi(Context context, String data) {
        data = data.replaceAll("\r\n", "\n");
        String header;
        String body;
        String mwiBegin = MwiMessage.Label.To.getLabel();
        int begin = data.indexOf(mwiBegin);
        if (begin < 0) {
            return null;
        }
        header = data.substring(0, begin);
        body = data.substring(begin);
        ArrayList<MwiMessage> msgList = null;

        try {
            MwiHeader mwiHeader = parseHeader(header);
            msgList = parseBody(context, body, mwiHeader);
            Log.d(TAG, "Mwi header: " + header + ", body: " + body);
        } catch (Exception e) {
            Log.e(TAG, "parseMwi error: " + e);
        }
        return msgList;
    }

    /*
     * Parse header of data.
     */
    private static MwiHeader parseHeader(String header) {
        MwiHeader mwiHeader = new MwiHeader();
        String tmp = header;
        String str = header;
        int begin = 0;
        int end = -1;
        for (int i = 0; i < sHeaderTag.length; i++) {
            if (i + 1 < sHeaderTag.length) {
                end = tmp.indexOf(sHeaderTag[i + 1]);
            }
            if (end < 0 || i + 1 >= sHeaderTag.length) {
                end = tmp.length();
            }
            Log.d(TAG, "parseHeader begin: " + begin + ", end: " + end);
            if (begin >= 0 && begin < tmp.length() && end >= 0 && end <= tmp.length()) {
                str = tmp.substring(begin, end);
                tmp = tmp.substring(str.length());
                Log.d(TAG, "parseHeader str: " + str + ", tmp: " + tmp);
            } else {
                break;
            }

            String content = parseString(sHeaderTag[i], str).trim();
            Log.d(TAG, "parseHeader content: " + content);
            if (str.contains(MwiHeader.Label.MsgAccount.getLabel())) {
                mwiHeader.setMsgAccount(content);
            }
        }
        return mwiHeader;
    }

    /*
     * Parse string
     */
    private static String parseString(String tag, String str) {
        if (str.startsWith(tag)) {
            str = str.substring(tag.length());
        }
        if (str.startsWith(sSeparator)) {
            str = str.substring(sSeparator.length());
        }
        str = str.trim();
        return str;
    }

    /*
     * Parse body to messages.
     */
    private static ArrayList<MwiMessage> parseBody(Context context,
            String body, MwiHeader mwiHeader) {
        ArrayList<MwiMessage> msgList = new ArrayList<MwiMessage>();
        // Every message begin with To label
        String remain = body;
        int next = -1;
        String curMsg;

        while (!remain.equals("")) {
            next = remain.indexOf(MwiMessage.Label.To.getLabel(),
                    MwiMessage.Label.To.getLabel().length() + 1);
            if (next >= 0) {
                curMsg = remain.substring(0, next);
                remain = remain.substring(next);
            } else {
                curMsg = remain;
                remain = "";
            }
            if (DEBUG) {
                Log.d(TAG, "parseBody, curMsg: " + curMsg);
            }

            MwiMessage mwiMsg = parseMessage(context, curMsg, mwiHeader);
            msgList.add(mwiMsg);
        }

        return msgList;
    }

    /*
     * Parse one message.
     */
    private static MwiMessage parseMessage(Context context, String message, MwiHeader mwiHeader) {
        MwiMessage mwiMessage = new MwiMessage(context);
        mwiMessage.setMsgAccount(mwiHeader.getMsgAccount());
        String tmp = message;
        String str = message;
        int begin = 0;

        while (tmp.length() > 0) {
            int index = -1;
            int nextBegin = tmp.length();
            int nextIndex = -1;
            for (int i = 0; i < sMessageTag.length; i++) {
                if (tmp.startsWith(sMessageTag[i])) {
                    index = i;
                    continue;
                }
                if (tmp.contains(sMessageTag[i])) {
                    int tmpNextBegin = tmp.indexOf(sMessageTag[i]);
                    if (tmpNextBegin >= 0 && tmpNextBegin < nextBegin) {
                        nextBegin = tmpNextBegin;
                    }
                }
            }
            Log.d(TAG, "parseMessage begin: " + begin + ", nextBegin: " + nextBegin);
            if (begin >= 0 && begin < tmp.length()
                    && nextBegin >= 0 && nextBegin <= tmp.length()) {
                str = tmp.substring(begin, nextBegin);
                tmp = tmp.substring(str.length());
                Log.d(TAG, "parseMessage str: " + str + ", tmp: " + tmp);
            } else {
                Log.d(TAG, "parseMessage break");
                break;
            }

            String label = sMessageTag[index];
            String content = parseString(sMessageTag[index], str).trim();
            Log.d(TAG, "parseMessage content: " + content + ", label: " + label);
            if (label.equals(MwiMessage.Label.To.getLabel())) {
                mwiMessage.setTo(content);
            } else if (label.equals(MwiMessage.Label.From.getLabel())) {
                mwiMessage.setFrom(content);
            } else if (label.equals(MwiMessage.Label.Subject.getLabel())) {
                mwiMessage.setSubject(content);
            } else if (label.equals(MwiMessage.Label.Date.getLabel())) {
                mwiMessage.setDate(content);
            } else if (label.equals(MwiMessage.Label.Priority.getLabel())) {
                mwiMessage.setPriority(content);
            } else if (label.equals(MwiMessage.Label.MsgId.getLabel())) {
                mwiMessage.setMsgId(content);
            } else if (label.equals(MwiMessage.Label.MsgContext.getLabel())) {
                mwiMessage.setMsgContext(content);
            }
        }
        return mwiMessage;
    }
}
