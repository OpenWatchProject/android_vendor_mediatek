
package com.mediatek.simservs.xcap;

import java.io.IOException;
import android.util.Log;

/**
 * XCAP Exception class.
 */
public class XcapException extends Exception {

    private static final long serialVersionUID = 1L;

    public static final int NO_EXCEPTION = 0;
    public static final int CONNECTION_POOL_TIMEOUT_EXCEPTION = 1;
    public static final int CONNECT_TIMEOUT_EXCEPTION = 2;
    public static final int NO_HTTP_RESPONSE_EXCEPTION = 3;
    public static final int HTTP_RECOVERABL_EEXCEPTION = 4;
    public static final int MALFORMED_CHALLENGE_EXCEPTION = 5;
    public static final int AUTH_CHALLENGE_EXCEPTION = 6;
    public static final int CREDENTIALS_NOT_AVAILABLE_EXCEPTION = 7;
    public static final int INVALID_CREDENTIALS_EXCEPTION = 8;
    public static final int AUTHENTICATION_EXCEPTION = 9;
    public static final int MALFORMED_COOKIE_EXCEPTION = 10;
    public static final int REDIRECT_EXCEPTION = 11;
    public static final int URI_EXCEPTION = 12;
    public static final int PROTOCOL_EXCEPTION = 13;
    public static final int HTTP_EXCEPTION = 14;

    private int mHttpErrorCode = 0;
    private int mExceptionCode = NO_EXCEPTION;
    private boolean mIsConnectionError = false;
    private String mXcapErrorMessage;

    /**
     * Constructs an instance with error code.
     *
     * @param httpErrorCode XCAP error code
     */
    public XcapException(int httpErrorCode) {
        mHttpErrorCode = httpErrorCode;
    }

    /**
     * Constructs an instance with error code and message.
     *
     * @param httpErrorCode XCAP error code
     * @param xcapErrorMessage XCAP error message
     */
    public XcapException(int httpErrorCode, String xcapErrorMessage) {
        mHttpErrorCode = httpErrorCode;
        mXcapErrorMessage = xcapErrorMessage;
    }

    /**
     * Constructs an instance with IO exception.
     *
     * @param httpException I/O error exception
     */
    public XcapException(IOException httpException) {
        if ("GBA hit HTTP 403 Forbidden".equals(httpException.getMessage())) {
            mHttpErrorCode = 403;
            return;
        } else if ("GBA hit HTTP 400 Bad Request".equals(httpException.getMessage())) {
            Log.i("XcapException", "IOException 400 bad request");
            mHttpErrorCode = 400;
            return;
        }

        mIsConnectionError = true;
    }

    public boolean isConnectionError() {
        return mIsConnectionError;
    }

    public int getHttpErrorCode() {
        return mHttpErrorCode;
    }

    public int getExceptionCodeCode() {
        return mExceptionCode;
    }

    public String getErrorMessage() {
        return mXcapErrorMessage;
    }
}
