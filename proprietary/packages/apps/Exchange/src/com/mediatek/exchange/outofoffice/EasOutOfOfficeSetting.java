package com.mediatek.exchange.outofoffice;

import java.io.IOException;
import java.io.InputStream;
import java.security.cert.CertificateException;
import java.util.HashMap;

import android.content.Context;
import android.text.format.Time;

import com.android.emailcommon.mail.ServerCommandInfo;
import com.android.emailcommon.provider.Account;
import com.android.emailcommon.provider.HostAuth;
import com.android.emailcommon.service.OofParams;
import com.android.exchange.Eas;
import com.android.exchange.EasResponse;
import com.android.exchange.adapter.Serializer;
import com.android.exchange.adapter.SettingsParser;
import com.android.exchange.adapter.Tags;
import com.android.exchange.service.EasServerConnection;
import com.android.mail.utils.LogUtils;

import org.apache.http.HttpStatus;

public class EasOutOfOfficeSetting extends EasServerConnection {
    private static final String TAG = Eas.LOG_TAG;
    private static final String BODY_TYPE = "Text";
    private static final String SETTINGS_ENABLE = "1";
    private static final String SETTINGS_DISABLE = "0";
    private static HashMap<Long, EasServerConnection> sOofSvc =
        new HashMap<Long, EasServerConnection>();

    public EasOutOfOfficeSetting(Context context, Account account, HostAuth hostAuth) {
        super(context, account, hostAuth);
    }

    public static OofParams doSyncOof(final Context context, long accountId, OofParams oofParams, boolean isGet) {
        OofParams oofParam = null;
        try {
            byte[] entity = buildRequestSerializer(oofParams, isGet);
            Account account = Account.restoreAccountWithId(context, accountId);
            if (account == null) {
                return null;
            }
            HostAuth hostAuth = HostAuth.restoreHostAuthWithId(context, account.mHostAuthKeyRecv);
            if (hostAuth == null) {
                return null;
            }
            EasOutOfOfficeSetting oofSetting = new EasOutOfOfficeSetting(context, account, hostAuth);
            sOofSvc.put(accountId, oofSetting);
            EasResponse response = oofSetting.sendHttpClientPost("Settings", entity);
            oofParam = oofSetting.parseResponse(response);
        } catch (IOException e) {
            e.printStackTrace();
            oofParam = new OofParams(ServerCommandInfo.OofInfo.NETWORK_SHUT_DOWN, 0, 0,
                    0, 0, null);
        } catch (CertificateException e) {
            ///M: Added for catch CeritifcationExcetion
            LogUtils.w(TAG, "CertificateException while Sync OutOfOffice Settings from server: %s",
                    e.getMessage());
            return null;
        } finally {
            sOofSvc.remove(accountId);
        }
        return oofParam;
    }

    public static void doStopOof(long accountId) {
        EasServerConnection esc = sOofSvc.get(accountId);
        if (esc != null) {
            esc.stop(EasServerConnection.STOPPED_REASON_ABORT);
            sOofSvc.remove(accountId);
        }
    }

    private static byte[] buildRequestSerializer(OofParams oofParams, boolean isGet) throws IOException {
        Serializer s = new Serializer();
        s.start(Tags.SETTINGS_SETTINGS)
        .start(Tags.SETTINGS_OOF);
        if (isGet) {
            s.start(Tags.SETTINGS_GET)
            .data(Tags.SETTINGS_BODY_TYPE, BODY_TYPE)
            .end().end().end().done();
        } else {
            String stime = convertMillisTimeToEmailDateTime(
                    oofParams.getStartTimeInMillis());
            String etime = convertMillisTimeToEmailDateTime(
                    oofParams.getEndTimeInMillis());
            if (oofParams.getOofState() != 0) {
                s.start(Tags.SETTINGS_SET).data(Tags.SETTINGS_OOF_STATE,
                        String.valueOf(ServerCommandInfo.OofInfo.OOF_IS_TIME_BASED)).data(
                        Tags.SETTINGS_START_TIME, stime)
                .data(Tags.SETTINGS_END_TIME, etime)
                .start(Tags.SETTINGS_OOF_MESSAGE)
                .tag(Tags.SETTINGS_APPLIES_TO_INTERNAL)
                .data(Tags.SETTINGS_ENABLED, SETTINGS_ENABLE)
                .data(Tags.SETTINGS_REPLY_MESSAGE, oofParams.getReplyMessage())
                .data(Tags.SETTINGS_BODY_TYPE, BODY_TYPE)
                .end();
                if (oofParams.getIsExternal() != 0) {
                    s.start(Tags.SETTINGS_OOF_MESSAGE)
                    .tag(Tags.SETTINGS_APPLIES_TO_EXTERNAL_KNOWN)
                    .data(Tags.SETTINGS_ENABLED, SETTINGS_ENABLE)
                    .data(Tags.SETTINGS_REPLY_MESSAGE, oofParams.getReplyMessage())
                    .data(Tags.SETTINGS_BODY_TYPE, BODY_TYPE)
                    .end()
                    .start(Tags.SETTINGS_OOF_MESSAGE)
                    .tag(Tags.SETTINGS_APPLIES_TO_EXTERNAL_UNKNOWN)
                    .data(Tags.SETTINGS_ENABLED, SETTINGS_ENABLE)
                    .data(Tags.SETTINGS_REPLY_MESSAGE, oofParams.getReplyMessage())
                    .data(Tags.SETTINGS_BODY_TYPE, BODY_TYPE)
                    .end().end().end().end().done();
                } else {
                    s.start(Tags.SETTINGS_OOF_MESSAGE)
                    .tag(Tags.SETTINGS_APPLIES_TO_EXTERNAL_KNOWN)
                    .data(Tags.SETTINGS_ENABLED, SETTINGS_DISABLE)
                    .end()
                    .start(Tags.SETTINGS_OOF_MESSAGE)
                    .tag(Tags.SETTINGS_APPLIES_TO_EXTERNAL_UNKNOWN)
                    .data(Tags.SETTINGS_ENABLED, SETTINGS_DISABLE)
                    .end().end().end().end().done();
                }
            } else {
                s.start(Tags.SETTINGS_SET)
                .data(Tags.SETTINGS_OOF_STATE, SETTINGS_DISABLE)
                .end().end().end().done();
            }
        }
        return s.toByteArray();
    }

    private OofParams parseResponse(EasResponse resp) throws IOException {
        OofParams result = null;
        int code = resp.getStatus();
        if (code == HttpStatus.SC_OK) {
            InputStream is = resp.getInputStream();
            SettingsParser sp = new SettingsParser(is);
            sp.parse();
            int status = sp.getOofStatus();
            int oofState = sp.getOofState();
            long startTimeInMillis = sp.getStartTimeInMillis();
            long endTimeInMillis = sp.getEndTimeInMillis();
            String oofMessage = sp.getReplyMessage();
            int isExternal = sp.getIsExternal();
            result = new OofParams(status, oofState, startTimeInMillis,
                    endTimeInMillis, isExternal, oofMessage);
        } else {
            LogUtils.d(TAG, "OOF returned " + code);
        }
        return result;
    }

    /**
     * M: Convert a time in millis format to email date time
     * @param millisTime
     * @return EmailDateTime
     */
    public static String convertMillisTimeToEmailDateTime(long millisTime) {
        String month;
        String day;
        String hour;
        String minute;
        int time;

        Time actualTime = new Time();
        actualTime.set(millisTime);
        actualTime.normalize(true);
        actualTime.switchTimezone("GMT");

        time = actualTime.month + 1;
        if (time < 10) {
            month = getTimeString(time);
        } else {
            month = String.valueOf(time);
        }
        time = actualTime.monthDay;
        if (time < 10) {
            day = getTimeString(time);
        } else {
            day = String.valueOf(time);
        }
        time = actualTime.hour;
        if (time < 10) {
            hour = getTimeString(time);
        } else {
            hour = String.valueOf(time);
        }
        time = actualTime.minute;
        if (time < 10) {
            minute = getTimeString(time);
        } else {
            minute = String.valueOf(time);
        }
        return String.valueOf(actualTime.year) + "-" + month + "-" + day
                + "T" + hour + ":" + minute + ":00.000Z";
    }

    private static String getTimeString(int time) {
        return "0" + time;
    }
}
