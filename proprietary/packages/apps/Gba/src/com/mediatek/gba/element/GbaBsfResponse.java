package com.mediatek.gba.element;

import android.util.Log;

import com.mediatek.gba.header.AuthInfoHeader;
import com.mediatek.gba.header.WwwAuthHeader;

import java.io.BufferedInputStream;
import java.io.ByteArrayOutputStream;
import java.io.IOException;
import java.net.HttpURLConnection;
import java.util.StringTokenizer;

/**
 * implementation for GbaBsfResponse.
 *
 * @hide
 */
public class GbaBsfResponse {
    private static final String TAG = "GbaBsfResponse";

    private static final String BSF_XML_CONTENT_TYPE = "application/vnd.3gpp.bsf+xml";
    private static final String TMPI_INDICATOR = "3gpp-gba-tmpi";
    private static final String CONTENT_TYPE_HDR = "Content-Type";
    private static final String CONTENT_LENGTH_HDR = "Content-Length";
    private static final String SERVER_HDR = "Server";
    private static final String WWW_AUTH_HDR = "WWW-Authenticate";

    private int mStatusCode;
    private boolean mIsTmpiSupported;
    private String mSrverHeader;

    private WwwAuthHeader mWwwAuthHeader;

    private AuthInfoHeader mAuthInfoHeader;

    private String mXmlContent;

    protected GbaBsfResponse() {
        mIsTmpiSupported = false;
    }

    protected void parse(HttpURLConnection urlConn) throws IOException {

        mStatusCode = urlConn.getResponseCode();
        Log.d(TAG, "Response Code:" + mStatusCode);

        parseServer(urlConn.getHeaderField(SERVER_HDR));

        parseWwwAuthenticate(urlConn.getHeaderField(WWW_AUTH_HDR));

        if (mStatusCode == HttpURLConnection.HTTP_OK) {
            parseAuthInfo(urlConn.getHeaderField(AuthInfoHeader.HEADER_NAME));

            if (isBsfXmlContentTypePresent(urlConn.getContentType())) {
                parseXmlContent(urlConn);
            }
        }
    }

    protected void parseServer(String serverHeader) {
        if (serverHeader != null) {
            mSrverHeader = serverHeader;

            StringTokenizer st = new StringTokenizer(mSrverHeader, " ");

            while (st.hasMoreTokens()) {
                String token = st.nextToken();

                if (token.contains(TMPI_INDICATOR)) {
                    mIsTmpiSupported = true;
                    break;
                }
            }
        } else {
            Log.e(TAG, "Server name is null");
        }
    }

    public boolean isTmpiSupported() {
        return mIsTmpiSupported;
    }

    public String getServerHeader() {
        return mSrverHeader;
    }

    public int getStatusCode() {
        return mStatusCode;
    }

    protected void parseWwwAuthenticate(String wwwAuthHeaderValue) throws IOException {

        if (wwwAuthHeaderValue != null) {
                mWwwAuthHeader = WwwAuthHeader.parse(wwwAuthHeaderValue);
        } else {
            Log.e(TAG, "WWW header is null");
        }
    }

    public WwwAuthHeader getWwwAuthenticateHeader() {
        return mWwwAuthHeader;
    }

    private boolean isBsfXmlContentTypePresent(String contentTypeHeaderValue) {

        if (contentTypeHeaderValue != null) {
            Log.d(TAG, "bsf xml contenttype = " + contentTypeHeaderValue);

            if (contentTypeHeaderValue.toLowerCase().contains(BSF_XML_CONTENT_TYPE)) {
                return true;
            }
        } else {
            Log.e(TAG, "content type is null");
        }

        return false;
    }

    private void parseXmlContent(HttpURLConnection urlConn) throws IOException {
        BufferedInputStream in = null;
        ByteArrayOutputStream sink = null;
        try {
            int contentLen = urlConn.getContentLength();
            String transferEncoding = urlConn.getHeaderField("Transfer-Encoding");
            byte[] buffer = null;
            if (contentLen == -1 && transferEncoding == null) {
                Log.e(TAG, "No content length:" + urlConn.getResponseMessage());
                return;
            }
            if (transferEncoding != null && transferEncoding.equals("chunked")) {
                sink = new ByteArrayOutputStream();
                byte[] temp = new byte[1024];
                int len = 0;
                while ((len = urlConn.getInputStream().read(temp)) >= 0) {
                    sink.write(temp, 0, len);
                }
                buffer = sink.toByteArray();
            } else {
                in = new BufferedInputStream(urlConn.getInputStream());
                buffer = new byte[contentLen];
                in.read(buffer, 0, contentLen);
            }
            mXmlContent = new String(buffer, "UTF-8");
            Log.i(TAG, "\r\n" + mXmlContent + "\r\n");
        } finally {
            if (in != null) {
                in.close();
            }
            if (sink != null) {
                sink.close();
            }
        }

    }

    private void parseAuthInfo(String authInfoHttpHeaderValue) {

        if (authInfoHttpHeaderValue != null) {
                mAuthInfoHeader = AuthInfoHeader.parse(authInfoHttpHeaderValue);
        } else {
            Log.e(TAG, "auth info header is null");
        }
    }

    public AuthInfoHeader getAuthenticationInfoHeader() {
        return mAuthInfoHeader;
    }

    public String getXmlContent() {
        return mXmlContent;
    }

    /**
     * Utility function to parse BSF's HTTP response.
     *
     * @param urlConn the HTTP connection.
     * @return the GbaBsfResponse object.
     * @throws IOException if there is any IO error occurred.
     *
     */
    public static GbaBsfResponse parseResponse(HttpURLConnection urlConn) throws IOException {
        GbaBsfResponse bsfResponse = new GbaBsfResponse();
        bsfResponse.parse(urlConn);
        return bsfResponse;
    }

}
