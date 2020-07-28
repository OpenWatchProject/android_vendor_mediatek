
package com.mediatek.ims.internal;

import android.net.Uri;
import android.telephony.ims.ImsCallProfile;
import android.text.TextUtils;

import android.telephony.ims.ImsExternalCallState;

import com.mediatek.ims.internal.DialogInfo.Dialog;
import com.mediatek.ims.internal.DialogInfo.MediaAttribute;

import java.util.LinkedList;
import java.util.List;


/**
 * Responsible for creating IMS external call state from result of Dialog event package.
 */
public class ExternalCallStateFactory {
    private static ExternalCallStateFactory sInstance;


    /**
     * Gets a factory instance.
     * @return the factory instance
     */
    public static ExternalCallStateFactory getInstance() {
        if (sInstance == null) {
            sInstance = new ExternalCallStateFactory();
        }
        return sInstance;
    }


    /**
     * Create IMS external call state.
     * @param dialogInfo result of Dialog event package
     * @return list of ImsExternalCallState
     */
    public List<ImsExternalCallState> makeExternalCallStates(DialogInfo dialogInfo) {
        List<ImsExternalCallState> result = new LinkedList<ImsExternalCallState>();
        List<Dialog> dialogs = dialogInfo.getDialogs();
        for (Dialog dialog : dialogs) {
            ImsExternalCallState exCallState = new ImsExternalCallState(dialog.getDialogId(),
                    getAddress(dialog), isPullable(dialog), getCallState(dialog),
                    getCallType(dialog), isCallHeld(dialog));
            result.add(exCallState);
        }
        return result;
    }

    private Uri getAddress(Dialog dialog) {
        if (dialog == null) {
            return Uri.parse("");
        }
        String address = dialog.getTargetUri();
        if (TextUtils.isEmpty(address) == false) {
            return Uri.parse(address);
        } else {
            return Uri.parse(dialog.getIdentity());
        }
    }

    private boolean isPullable(Dialog dialog) {
        if (dialog == null || dialog.getExclusive() == true) {
            return false;
        } else {
            if (isCallHeld(dialog)) {
                return false;
            } else if (isVideoCallInBackground(dialog)) {
                return false;
            }
        }
        return true;
    }

    private int getCallState(Dialog dialog) {
        if (dialog == null) {
            return ImsExternalCallState.CALL_STATE_TERMINATED;
        }
        if ("confirmed".equalsIgnoreCase(dialog.getState())) {
            return ImsExternalCallState.CALL_STATE_CONFIRMED;
        } else {
            return ImsExternalCallState.CALL_STATE_TERMINATED;
        }
    }

    private int getCallType(Dialog dialog) {
        int callType = ImsCallProfile.CALL_TYPE_VOICE;
        if (dialog == null) {
            return callType;
        }
        List<MediaAttribute> mediaAttributes = dialog.getMediaAttributes();
        for (MediaAttribute mediaAttribute : mediaAttributes) {
            if ("audio".equalsIgnoreCase(mediaAttribute.getMediaType())) {
                continue;
            }
            if (mediaAttribute.isDowngradedVideo()) {
                continue;
            } else if ("inactive".equalsIgnoreCase(mediaAttribute.getMediaDirection())) {
                callType = ImsCallProfile.CALL_TYPE_VT_NODIR;
            } else if ("sendrecv".equalsIgnoreCase(mediaAttribute.getMediaDirection())) {
                callType = ImsCallProfile.CALL_TYPE_VT;
            } else if ("sendonly".equalsIgnoreCase(mediaAttribute.getMediaDirection())) {
                callType = ImsCallProfile.CALL_TYPE_VT_TX;
            } else if ("recvonly".equalsIgnoreCase(mediaAttribute.getMediaDirection())) {
                callType = ImsCallProfile.CALL_TYPE_VT_RX;
            }
        }
        return callType;
    }

    private boolean isCallHeld(Dialog dialog) {
        if (dialog == null) {
            return false;
        }
        if ("+sip.rendering".equalsIgnoreCase(dialog.getPname())
                && "no".equalsIgnoreCase(dialog.getPval())) {
            return true;
        } else if (isCallHeld(dialog.getMediaAttributes())) {
            return true;
        }
        return false;
    }

    private boolean isCallHeld(List<MediaAttribute> mediaAttributes) {
        if (mediaAttributes == null) {
            return false;
        }
        for (MediaAttribute mediaAttribute : mediaAttributes) {
            if ("audio".equalsIgnoreCase(mediaAttribute.getMediaType())
                    && "sendrecv".equalsIgnoreCase(mediaAttribute.getMediaDirection()) == false) {
                return true;
            }
        }
        return false;
    }

    private boolean isVideoCallInBackground(Dialog dialog) {
        if (dialog == null) {
            return false;
        }
        List<MediaAttribute> mediaAttributes = dialog.getMediaAttributes();
        for (MediaAttribute mediaAttribute : mediaAttributes) {
            if ("video".equalsIgnoreCase(mediaAttribute.getMediaType())
                    && "inactive".equalsIgnoreCase(mediaAttribute.getMediaDirection())) {
                return true;
            }
        }
        return false;
    }
}
