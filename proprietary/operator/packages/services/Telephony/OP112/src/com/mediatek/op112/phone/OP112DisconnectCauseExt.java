package com.mediatek.op112.phone;

import android.content.Context;
import android.util.Log;

import com.mediatek.phone.ext.DefaultDisconnectCauseExt;
import com.mediatek.op112.phone.R;
import mediatek.telephony.MtkDisconnectCause;


/**
 * Plugin implementation for OP112.
 */
public class OP112DisconnectCauseExt extends DefaultDisconnectCauseExt {
    private static final String TAG = "OP112DisconnectCauseExt";
    private Context mContext = null;

    /**
     * Plugin implementation for CallfeatureSettings.
     * @param context context
     */
    public OP112DisconnectCauseExt(Context context) {
        mContext = context;
    }

    @Override
    public int toTelecomDisconnectCauseCode(int telephonyDisconnectCause, int error) {
        boolean result = false;
        Log.d(TAG, "toTelecomDisconnectCauseCode " + telephonyDisconnectCause);
        switch (telephonyDisconnectCause) {
            case MtkDisconnectCause.CAUSE_MOVED_PERMANENTLY:
            case MtkDisconnectCause.CAUSE_BAD_REQUEST:
            case MtkDisconnectCause.CAUSE_UNAUTHORIZED:
            case MtkDisconnectCause.CAUSE_PAYMENT_REQUIRED:
            case MtkDisconnectCause.CAUSE_FORBIDDEN:
            case MtkDisconnectCause.CAUSE_NOT_FOUND:
            case MtkDisconnectCause.CAUSE_METHOD_NOT_ALLOWED:
            case MtkDisconnectCause.CAUSE_NOT_ACCEPTABLE:
            case MtkDisconnectCause.CAUSE_PROXY_AUTHENTICATION_REQUIRED:
            case MtkDisconnectCause.CAUSE_REQUEST_TIMEOUT:
            case MtkDisconnectCause.CAUSE_CONFLICT:
            case MtkDisconnectCause.CAUSE_GONE:
            case MtkDisconnectCause.CAUSE_LENGTH_REQUIRED:
            case MtkDisconnectCause.CAUSE_REQUEST_ENTRY_TOO_LONG:
            case MtkDisconnectCause.CAUSE_REQUEST_URI_TOO_LONG:
            case MtkDisconnectCause.CAUSE_UNSUPPORTED_MEDIA_TYPE:
            case MtkDisconnectCause.CAUSE_UNSUPPORTED_URI_SCHEME:
            case MtkDisconnectCause.CAUSE_BAD_EXTENSION:
            case MtkDisconnectCause.CAUSE_EXTENSION_REQUIRED:
            case MtkDisconnectCause.CAUSE_INTERVAL_TOO_BRIEF:
            case MtkDisconnectCause.CAUSE_TEMPORARILY_UNAVAILABLE:
            case MtkDisconnectCause.CAUSE_CALL_TRANSACTION_NOT_EXIST:
            case MtkDisconnectCause.CAUSE_LOOP_DETECTED:
            case MtkDisconnectCause.CAUSE_TOO_MANY_HOPS:
            case MtkDisconnectCause.CAUSE_ADDRESS_INCOMPLETE:
            case MtkDisconnectCause.CAUSE_AMBIGUOUS:
            case MtkDisconnectCause.CAUSE_BUSY_HERE:
            case MtkDisconnectCause.CAUSE_REQUEST_TERMINATED:
            case MtkDisconnectCause.CAUSE_NOT_ACCEPTABLE_HERE:
            case MtkDisconnectCause.CAUSE_SERVER_INTERNAL_ERROR:
            case MtkDisconnectCause.CAUSE_NOT_IMPLEMENTED:
            case MtkDisconnectCause.CAUSE_BAD_GATEWAY:
            case MtkDisconnectCause.CAUSE_SERVICE_UNAVAILABLE:
            case MtkDisconnectCause.CAUSE_GATEWAY_TIMEOUT:
            case MtkDisconnectCause.CAUSE_VERSION_NOT_SUPPORTED:
            case MtkDisconnectCause.CAUSE_MESSAGE_TOO_LONG:
            case MtkDisconnectCause.CAUSE_BUSY_EVERYWHERE:
            case MtkDisconnectCause.CAUSE_DECLINE:
            case MtkDisconnectCause.CAUSE_DOES_NOT_EXIST_ANYWHERE:
            case MtkDisconnectCause.CAUSE_SESSION_NOT_ACCEPTABLE:
                return mediatek.telecom.MtkDisconnectCause.SIP_INVITE_ERROR;
            default:
                return error;
        }
    }

    @Override
    public CharSequence toTelecomDisconnectCauseLabel(int telephonyDisconnectCause) {
        Log.d(TAG, "toTelecomDisconnectCauseLabel " + telephonyDisconnectCause);
        CharSequence resource = "";
        switch (telephonyDisconnectCause) {
            case MtkDisconnectCause.CAUSE_MOVED_PERMANENTLY:
            case MtkDisconnectCause.CAUSE_BAD_REQUEST:
            case MtkDisconnectCause.CAUSE_UNAUTHORIZED:
            case MtkDisconnectCause.CAUSE_PAYMENT_REQUIRED:
            case MtkDisconnectCause.CAUSE_FORBIDDEN:
            case MtkDisconnectCause.CAUSE_NOT_FOUND:
            case MtkDisconnectCause.CAUSE_METHOD_NOT_ALLOWED:
            case MtkDisconnectCause.CAUSE_NOT_ACCEPTABLE:
            case MtkDisconnectCause.CAUSE_PROXY_AUTHENTICATION_REQUIRED:
            case MtkDisconnectCause.CAUSE_REQUEST_TIMEOUT:
            case MtkDisconnectCause.CAUSE_CONFLICT:
            case MtkDisconnectCause.CAUSE_GONE:
            case MtkDisconnectCause.CAUSE_LENGTH_REQUIRED:
            case MtkDisconnectCause.CAUSE_REQUEST_ENTRY_TOO_LONG:
            case MtkDisconnectCause.CAUSE_REQUEST_URI_TOO_LONG:
            case MtkDisconnectCause.CAUSE_UNSUPPORTED_MEDIA_TYPE:
            case MtkDisconnectCause.CAUSE_UNSUPPORTED_URI_SCHEME:
            case MtkDisconnectCause.CAUSE_BAD_EXTENSION:
            case MtkDisconnectCause.CAUSE_EXTENSION_REQUIRED:
            case MtkDisconnectCause.CAUSE_INTERVAL_TOO_BRIEF:
            case MtkDisconnectCause.CAUSE_TEMPORARILY_UNAVAILABLE:
            case MtkDisconnectCause.CAUSE_CALL_TRANSACTION_NOT_EXIST:
            case MtkDisconnectCause.CAUSE_LOOP_DETECTED:
            case MtkDisconnectCause.CAUSE_TOO_MANY_HOPS:
            case MtkDisconnectCause.CAUSE_ADDRESS_INCOMPLETE:
            case MtkDisconnectCause.CAUSE_AMBIGUOUS:
            case MtkDisconnectCause.CAUSE_BUSY_HERE:
            case MtkDisconnectCause.CAUSE_REQUEST_TERMINATED:
            case MtkDisconnectCause.CAUSE_NOT_ACCEPTABLE_HERE:
            case MtkDisconnectCause.CAUSE_SERVER_INTERNAL_ERROR:
            case MtkDisconnectCause.CAUSE_NOT_IMPLEMENTED:
            case MtkDisconnectCause.CAUSE_BAD_GATEWAY:
            case MtkDisconnectCause.CAUSE_SERVICE_UNAVAILABLE:
            case MtkDisconnectCause.CAUSE_GATEWAY_TIMEOUT:
            case MtkDisconnectCause.CAUSE_VERSION_NOT_SUPPORTED:
            case MtkDisconnectCause.CAUSE_MESSAGE_TOO_LONG:
            case MtkDisconnectCause.CAUSE_BUSY_EVERYWHERE:
            case MtkDisconnectCause.CAUSE_DECLINE:
            case MtkDisconnectCause.CAUSE_DOES_NOT_EXIST_ANYWHERE:
            case MtkDisconnectCause.CAUSE_SESSION_NOT_ACCEPTABLE:
             resource = mContext.getResources().getString(R.string.call_error);
             break;
            default :
                break;
        }
        return resource;
    }

    @Override
    public CharSequence toTelecomDisconnectCauseDescription(int telephonyDisconnectCause) {
        Log.d(TAG, "toTelecomDisconnectCauseDescription " + telephonyDisconnectCause);
        CharSequence resource = "";
        switch (telephonyDisconnectCause) {
            case MtkDisconnectCause.CAUSE_MOVED_PERMANENTLY:
                resource = mContext.getResources().getString(R.string.cause_moved_permanently);
                break;
            case MtkDisconnectCause.CAUSE_BAD_REQUEST:
                resource = mContext.getResources().getString(R.string.cause_bad_request);
                break;
            case MtkDisconnectCause.CAUSE_UNAUTHORIZED:
                resource = mContext.getResources().getString(R.string.cause_unauthorized);
                break;
            case MtkDisconnectCause.CAUSE_PAYMENT_REQUIRED:
                resource = mContext.getResources().getString(R.string.cause_payment_required);
                break;
            case MtkDisconnectCause.CAUSE_FORBIDDEN:
                resource = mContext.getResources().getString(R.string.cause_forbidden);
                break;
            case MtkDisconnectCause.CAUSE_NOT_FOUND:
                resource = mContext.getResources().getString(R.string.cause_not_found);
                break;
            case MtkDisconnectCause.CAUSE_METHOD_NOT_ALLOWED:
                resource = mContext.getResources().getString(R.string.cause_method_not_allowed);
                break;
            case MtkDisconnectCause.CAUSE_NOT_ACCEPTABLE:
                resource = mContext.getResources().getString(R.string.cause_not_acceptable);
                break;
            case MtkDisconnectCause.CAUSE_PROXY_AUTHENTICATION_REQUIRED:
                resource = mContext.getResources().getString(R.string.cause_proxy_authentication_required);
                break;
            case MtkDisconnectCause.CAUSE_REQUEST_TIMEOUT:
                resource = mContext.getResources().getString(R.string.cause_request_timeout);
                break;
            case MtkDisconnectCause.CAUSE_CONFLICT:
                resource = mContext.getResources().getString(R.string.cause_conflict);
                break;
            case MtkDisconnectCause.CAUSE_GONE:
                resource = mContext.getResources().getString(R.string.cause_gone);
                break;
            case MtkDisconnectCause.CAUSE_LENGTH_REQUIRED:
                resource = mContext.getResources().getString(R.string.cause_length_required);
                break;
            case MtkDisconnectCause.CAUSE_REQUEST_ENTRY_TOO_LONG:
                resource = mContext.getResources().getString(R.string.cause_request_entry_too_long);
                break;
            case MtkDisconnectCause.CAUSE_REQUEST_URI_TOO_LONG:
                resource = mContext.getResources().getString(R.string.cause_request_uri_too_long);
                break;
            case MtkDisconnectCause.CAUSE_UNSUPPORTED_MEDIA_TYPE:
                resource = mContext.getResources().getString(R.string.cause_unsupported_media_type);
                break;
            case MtkDisconnectCause.CAUSE_UNSUPPORTED_URI_SCHEME:
                resource = mContext.getResources().getString(R.string.cause_unsupported_uri_scheme);
                break;
            case MtkDisconnectCause.CAUSE_BAD_EXTENSION:
                resource = mContext.getResources().getString(R.string.cause_bad_extension);
                break;
            case MtkDisconnectCause.CAUSE_EXTENSION_REQUIRED:
                resource = mContext.getResources().getString(R.string.cause_extension_required);
                break;
            case MtkDisconnectCause.CAUSE_INTERVAL_TOO_BRIEF:
                resource = mContext.getResources().getString(R.string.cause_interval_too_brief);
                break;
            case MtkDisconnectCause.CAUSE_TEMPORARILY_UNAVAILABLE:
                resource = mContext.getResources().getString(R.string.cause_temporarily_unavailable);
                break;
            case MtkDisconnectCause.CAUSE_CALL_TRANSACTION_NOT_EXIST:
                resource = mContext.getResources().getString(R.string.cause_call_transaction_not_exist);
                break;
            case MtkDisconnectCause.CAUSE_LOOP_DETECTED:
                resource = mContext.getResources().getString(R.string.cause_loop_detected);
                break;
            case MtkDisconnectCause.CAUSE_TOO_MANY_HOPS:
                resource = mContext.getResources().getString(R.string.cause_too_many_hops);
                break;
            case MtkDisconnectCause.CAUSE_ADDRESS_INCOMPLETE:
                resource = mContext.getResources().getString(R.string.cause_address_incomplete);
                break;
            case MtkDisconnectCause.CAUSE_AMBIGUOUS:
                resource = mContext.getResources().getString(R.string.cause_ambiguous);
                break;
            case MtkDisconnectCause.CAUSE_BUSY_HERE:
                resource = mContext.getResources().getString(R.string.cause_busy_here);
                break;
            case MtkDisconnectCause.CAUSE_REQUEST_TERMINATED:
                resource = mContext.getResources().getString(R.string.cause_request_terminated);
                break;
            case MtkDisconnectCause.CAUSE_NOT_ACCEPTABLE_HERE:
                resource = mContext.getResources().getString(R.string.cause_not_acceptable_here);
                break;
            case MtkDisconnectCause.CAUSE_SERVER_INTERNAL_ERROR:
                resource = mContext.getResources().getString(R.string.cause_server_internal_error);
                break;
            case MtkDisconnectCause.CAUSE_NOT_IMPLEMENTED:
                resource = mContext.getResources().getString(R.string.cause_not_implemented);
                break;
            case MtkDisconnectCause.CAUSE_BAD_GATEWAY:
                resource = mContext.getResources().getString(R.string.cause_bad_gateway);
                break;
            case MtkDisconnectCause.CAUSE_SERVICE_UNAVAILABLE:
                resource = mContext.getResources().getString(R.string.cause_service_unavailable);
                break;
            case MtkDisconnectCause.CAUSE_GATEWAY_TIMEOUT:
                resource = mContext.getResources().getString(R.string.cause_gateway_timeout);
                break;
            case MtkDisconnectCause.CAUSE_VERSION_NOT_SUPPORTED:
                resource = mContext.getResources().getString(R.string.cause_version_not_supported);
                break;
            case MtkDisconnectCause.CAUSE_MESSAGE_TOO_LONG:
                resource = mContext.getResources().getString(R.string.cause_message_too_long);
                break;
            case MtkDisconnectCause.CAUSE_BUSY_EVERYWHERE:
                resource = mContext.getResources().getString(R.string.cause_busy_everywhere);
                break;
            case MtkDisconnectCause.CAUSE_DECLINE:
                resource = mContext.getResources().getString(R.string.cause_decline);
                break;
            case MtkDisconnectCause.CAUSE_DOES_NOT_EXIST_ANYWHERE:
                resource = mContext.getResources().getString(R.string.cause_does_not_exist_anywhere);
                break;
            case MtkDisconnectCause.CAUSE_SESSION_NOT_ACCEPTABLE:
                resource = mContext.getResources().getString(R.string.cause_session_not_acceptable);
                break;
            default :
                break;
        }
        return resource;
     }
}