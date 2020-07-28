package com.ct.selfregister.dm.utils;

import android.util.Base64;
import android.util.Log;

import com.ct.selfregister.dm.Const;

import org.json.JSONException;
import org.json.JSONObject;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStreamReader;
import java.io.OutputStreamWriter;
import java.net.HttpURLConnection;
import java.net.URL;
import java.util.Arrays;

public class Utils {
    private static final String TAG = Const.TAG_PREFIX + "Utils";

    private static final String SERVER_URL = "http://zzhc.vnet.cn";
    private static final String CONTENT_TYPE = "Content-Type";
    private static final String CONTENT_JSON = "application/encrypted-json";

    private static final String JSON_RESULT_CODE = "resultCode";
    private static final String JSON_RESULT_DESC = "resultDesc";

    /**
     * Send message to the server.
     * @param data The message data to be sent.
     * @return JSONObject Response from server or null if error.
     */
    public static JSONObject httpSend(String data) {
        Log.d(TAG, "Enter httpSend()...");
        HttpURLConnection urlConnection = null;
        OutputStreamWriter out = null;
        JSONObject result = null;

        try {
            // 1. obtain connection and set parameters
            urlConnection = prepareConnection();

            // 2. set request body
            out = new OutputStreamWriter(urlConnection.getOutputStream());
            out.write(data);
            // must do flush here, otherwise body is empty
            out.flush();

            // 3. interact with server
            Log.d(TAG, "httpSend(), ===Before connect()");
            urlConnection.connect();
            Log.d(TAG, "httpSend(), ===After connect()");

            // 4. parse response from server
            result = parseResponse(urlConnection);

        } catch (IOException e) {
            Log.e(TAG, "Exception in httpSend()!");
            e.printStackTrace();

        } finally {
            doCleanResource(urlConnection, out);
        }

        return result;
    }

    /**
     * Obtain connection and set parameters
     */
    private static HttpURLConnection prepareConnection() throws IOException {
        URL url = new URL(SERVER_URL);
        HttpURLConnection urlConnection = (HttpURLConnection) url.openConnection();
        urlConnection.setRequestProperty(CONTENT_TYPE, CONTENT_JSON);
        urlConnection.setDoInput(true);
        urlConnection.setDoOutput(true);  // will use post method
        return urlConnection;
    }

    /**
     * Get response from server and wrap into JSONObject
     */
    private static JSONObject parseResponse(HttpURLConnection urlConnection) {
        BufferedReader in = null;
        JSONObject result = null;

        try {
            int statusCode = urlConnection.getResponseCode();
            Log.d(TAG, "parseResponse, Status code: " + statusCode);

            if (statusCode == HttpURLConnection.HTTP_OK) {
                in = new BufferedReader(new InputStreamReader(urlConnection.getInputStream()));
                String inputLine;
                StringBuffer response = new StringBuffer();

                while ((inputLine = in.readLine()) != null) {
                    response.append(inputLine);
                }

                result = new JSONObject(response.toString());
            }

        } catch (IOException | JSONException e) {
            e.printStackTrace();

        } finally {
            if (in != null) {
                try {
                    in.close();
                } catch (IOException e) {
                    e.printStackTrace();
                }
            }
        }
        return result;
    }

    private static void doCleanResource(HttpURLConnection urlConnection, OutputStreamWriter out) {
        if (urlConnection != null) {
            urlConnection.disconnect();
        }

        // close stream for request
        if (out != null) {
            try {
                out.close();
            } catch (IOException e) {
                e.printStackTrace();
            }
        }
    }

    public static boolean checkRegisterResult(JSONObject response) {
        if (response == null) {
            Log.e(TAG, "checkRegisterResult(), response is null!");
            return false;
        }

        int resultCode = -1;
        String resultDesc = null;

        try {
            resultCode = response.getInt(JSON_RESULT_CODE);
            resultDesc = response.getString(JSON_RESULT_DESC);
        } catch (JSONException e) {
            e.printStackTrace();
        }

        Log.i(TAG, "Result code " + resultCode + ", desc " + resultDesc);

        if (resultCode == 0) {
            return true;
        }

        return false;
    }

    public static String bytesToHexString(byte[] src) {
        StringBuilder stringBuilder = new StringBuilder("");

        if (src == null || src.length <= 0) {
            return null;
        }

        for (int i = 0; i < src.length; i++) {
            int v = src[i] & 0xFF;
            String hv = Integer.toHexString(v);
            if (hv.length() < 2) {
                stringBuilder.append(0);
            }
            stringBuilder.append(hv);
        }

        return stringBuilder.toString();
    }

    public static String encodeBase64(String data) {
        byte[] encodeByte = Base64.encode(data.getBytes(), Base64.DEFAULT);
        return new String(encodeByte);
    }

    public static boolean compareUnsortArray(String[] src, String[] dst) {
        String[] srcClone = src.clone();
        String[] dstClone = dst.clone();

        // sort the clone and compare
        Arrays.sort(srcClone);
        Arrays.sort(dstClone);

        return Arrays.equals(srcClone, dstClone);
    }
}
