package com.mediatek.digits.wsg;

import java.io.IOException;
import java.util.Date;

import android.content.Context;
import android.icu.text.SimpleDateFormat;
import android.os.Handler;
import android.os.HandlerThread;
import android.util.Log;

import com.android.okhttp.Headers;
import com.android.okhttp.HttpUrl;
import com.android.okhttp.MediaType;
import com.android.okhttp.MultipartBuilder;
import com.android.okhttp.OkHttpClient;
import com.android.okhttp.Protocol;
import com.android.okhttp.Request;
import com.android.okhttp.RequestBody;
import com.android.okhttp.Response;
import com.android.okhttp.okio.Buffer;

import com.mediatek.digits.DigitsUtil;

public class WsgServerApi {
    private static final String TAG = "WsgServerApi";
    private static final boolean DEBUG = true;

    private final Context mContext;

    private HandlerThread mThread;
    private Handler mThreadHandler;

//    public static final MediaType MIXED
//        = MediaType.parse("multipart/mixed");

    public WsgServerApi(Context context) {
        mContext = context;

        mThread = new HandlerThread("WsgServerApi-worker");
        mThread.start();
        mThreadHandler = new Handler(mThread.getLooper());
    }

    protected void finalize() throws Throwable {
        try {
            mThread.quitSafely();
        } finally {
            super.finalize();
        }
    }

    public WsgMethodBuilder createRequest() {
        return new WsgMethodBuilder(this);
    }

    boolean execute(final Request request, final Callback callback) {
        if (checkNoHandlerThread()) {
            return false;
        }

        mThreadHandler.post(new Runnable() {
                public void run() {
                OkHttpClient httpClient = new OkHttpClient();
                WsgResponse rsp = null;

                rsp = sendHttpRequest(httpClient, request);

                if (callback != null && rsp != null) {
                log("execute: callback the response");
                callback.callback(rsp);
                }
                }
                });

        return true;
    }

    public interface Callback {
        void callback(WsgResponse response);
    }

    public class WsgResponse {
        private Request mRequest = null;
        private Response mResponse = null;

        WsgResponse(Request request, Response response) {
            mRequest = request;
            mResponse = response;
        }

        public boolean isHttpSuccess() {
            return mResponse.isSuccessful();
        }

        public int getRespStatusCode() {
            return mResponse.code();
        }

        public Headers getRespHeaders() {
            return mResponse.headers();
        }

        public HttpUrl getReqHttpUrl() {
            return mRequest.httpUrl();
        }

        public String getReqBody() {
            Request copy = mRequest.newBuilder().build();
            Buffer buffer = new Buffer();
            try {
                copy.body().writeTo(buffer);
            } catch (IOException e) {
                e.printStackTrace();
            }
            return buffer.readUtf8();
        }
    }

    public class WsgMethodBuilder {
        private WsgServerApi mService;
        private Request mRequest = null;

        private WsgMethodBuilder(WsgServerApi service) {
            mService = service;
        }

        public boolean execute(Callback callback) {
            if (mService != null) {
                return mService.execute(mRequest, callback);
            }
            return false;
        }

        public WsgMethodBuilder addHuntGroup(String msisdn, String nativeMsisdn, String sit) {
            log("addHuntGroup:");
            log("  - msisdn:" + msisdn);
            log("  - nativeMsisdn:" + nativeMsisdn);
            log("  - sit:" + sit);
            mRequest = new Request.Builder()
                .url("https://wsg.t-mobile.com/phone20/huntgroupManagement/v1/lines/"
                        + msisdn + "/huntgroup/" + nativeMsisdn)
                .header("Authorization", "SIT=" + sit)
                .put(RequestBody.create(null, new byte[]{}))
                .build();

            return this;
        }

        public WsgMethodBuilder remHuntGroup(String msisdn, String nativeMsisdn, String sit) {
            log("remHuntGroup:");
            log("  - msisdn:" + msisdn);
            log("  - nativeMsisdn:" + nativeMsisdn);
            log("  - sit:" + sit);
            mRequest = new Request.Builder()
                .url("https://wsg.t-mobile.com/phone20/huntgroupManagement/v1/lines/"
                        + msisdn + "/huntgroup/" + nativeMsisdn)
                .header("Authorization", "SIT=" + sit)
                .delete(RequestBody.create(null, new byte[]{}))
                .build();

            return this;
        }

        public WsgMethodBuilder getTRN(String fromMsisdn, String toMsisdn,
            String nativeMsisdn, String sit) {
            log("getTRN:");
            log("  - fromMsisdn:" + fromMsisdn);
            log("  - toMsisdn:" + toMsisdn);
            log("  - nativeMsisdn:" + nativeMsisdn);
            log("  - sit:" + sit);
            SimpleDateFormat dateFormat = new SimpleDateFormat("yyyy-MM-dd'T'HH:mm:ssXXX");
            String xmlBody[] = new String[2];
            String imei = DigitsUtil.getInstance(mContext).getImei();

            xmlBody[0] = "<?xml version='1.0' encoding='UTF-8' ?>"
                + "<tpc:imrnInformation xmlns:tpc=\"urn:oma:xml:rest:netapi:thirdpartycall:1\">"
                + "<participantAddress>sip:" + nativeMsisdn
                + "@msg.pc.t-mobile.com</participantAddress>"
                + "<participantAddress>sip:" + toMsisdn
                + "@msg.pc.t-mobile.com</participantAddress>"
                + "<callType>MOC</callType>"
                + "<assertedAddress>sip:" + fromMsisdn + "@msg.pc.t-mobile.com</assertedAddress>"
                + "<clientCorrelator>104567</clientCorrelator>"
                + "<sipInstance>urclientCorrelatorn:gsma:imei:" + imei + "</sipInstance>"
                + "</tpc:imrnInformation>";
            xmlBody[1] = "<?xml version='1.0' encoding='UTF-8' ?>"
                + "<presence xmlns=\"urn:ietf:params:xml:ns:pidf\" entity=\"pres:" + nativeMsisdn
                + "@msg.pc.t-mobile.com\" xmlns:dm=\"urn:ietf:params:xml:ns:pidf:data-model\" "
                + "xmlns:gp=\"urn:ietf:params:xml:ns:pidf:geopriv10\" xmlns:gml=\""
                + "http://www.opengis.net/gml\" xmlns:cl=\""
                + "urn:ietf:params:xml:ns:pidf:geopriv10:civicAddr\">"
                + "<dm:device id=\"urn:gsma:imei:" + imei + "\">"
                + "<gp:geopriv>"
                + "<gp:location-info>"
                + "<cl:civicAddress>"
                + "<cl:country>US</cl:country>"
                + "</cl:civicAddress>"
                + "</gp:location-info>"
                + "</gp:geopriv>"
                + "<dm:deviceID>urn:gsma:imei:" + imei + "</dm:deviceID>"
                + "<dm:timestamp>" + dateFormat.format(new Date()) + "</dm:timestamp>"
                + "</dm:device>"
                + "</presence>";

            RequestBody requestBody = new MultipartBuilder()
                .type(MultipartBuilder.MIXED)
                .addPart(Headers.of("Content-Disposition", "form-data;name=\"attachments\""),
                        RequestBody.create(MediaType.parse("application/xml") , xmlBody[0]))
                .addPart(Headers.of("Content-Disposition", "form-data;name=\"attachments\""),
                        RequestBody.create(MediaType.parse("application/pidf+xml") , xmlBody[1]))
                .build();

            mRequest = new Request.Builder()
                .url("https://wsg.t-mobile.com/phone20/thirdpartycall/v1/imrn")
                .header("Content-type", "multipart/mixed")
                .header("Authorization", "SIT=" + sit)
                .post(requestBody)
                .build();

            return this;
        }
    }

    /**
     * This function shall not be run at main thread.
     */
    private WsgResponse sendHttpRequest(OkHttpClient client, Request request) {
        log("sendHttpRequest: url=" + request.url().toString());
        log("sendHttpRequest: Authorization=" + request.headers().get("Authorization"));
        WsgResponse result = null;

        try {
            Response response = client.newCall(request).execute();
            result = new WsgResponse(request, response);
            response.body().close();
        } catch (Exception e) {
            Log.e(TAG, "sendHttpRequest: Exception:", e);
            Response response = new Response.Builder()
                .code(408) //Request Timeout
                .request(request)
                .protocol(Protocol.HTTP_1_1)
                .message("OKHTTP exception occurs. Search log for detail information.")
                .build();
            result = new WsgResponse(request, response);
        }

        return result;
    }

    private boolean checkNoHandlerThread() {
        if (mThreadHandler == null) {
            log("No Handler Thread, return directly");
            return true;
        } else {
            return false;
        }
    }

    private static void log(String s) {
        if (DEBUG) {
            Log.d(TAG, s);
        }
    }
}
