/*
 * Copyright (C) 2006 The Android Open Source Project
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
package com.mediatek.rcse.plugin.phone;

import android.telecom.Call;
import android.telecom.VideoProfile;
import android.util.Log;

import com.android.internal.telephony.CallManager;

/**
 * "Call card" UI element: the in-call screen contains a tiled layout of call
 * cards, each representing the state of a current "call" (ie. an active call, a
 * call on hold, or an incoming call.)
 */
public class RCSeUtils {
    /**
     * The Constant LOG_TAG.
     */
    private static final String LOG_TAG = "RCSeUtils";
    /**
     * The Constant DBG.
     */
    private static final boolean DBG = true;
    /**
     * The m fg call.
     */
    private static Call sFgCall = null;

    /**
     * The state of a {@code Call} when newly created.
     */
    public static final int STATE_NEW = 0;

    /**
     * The state of an outgoing {@code Call} when dialing the remote number, but not yet connected.
     */
    public static final int STATE_DIALING = 1;

    /**
     * The state of an incoming {@code Call} when ringing locally, but not yet connected.
     */
    public static final int STATE_RINGING = 2;

    /**
     * The state of a {@code Call} when in a holding state.
     */
    public static final int STATE_HOLDING = 3;

    /**
     * The state of a {@code Call} when actively supporting conversation.
     */
    public static final int STATE_ACTIVE = 4;

    /**
     * The state of a {@code Call} when no further voice or other communication is being
     * transmitted, the remote side has been or will inevitably be informed that the {@code Call}
     * is no longer active, and the local data transport has or inevitably will release resources
     * associated with this {@code Call}.
     */
    public static final int STATE_DISCONNECTED = 7;

    /**
     * The state of an outgoing {@code Call}, but waiting for user input before proceeding.
     */
    public static final int STATE_PRE_DIAL_WAIT = 8;

    /**
     * The initial state of an outgoing {@code Call}.
     * Common transitions are to {@link #STATE_DIALING} state for a successful call or
     * {@link #STATE_DISCONNECTED} if it failed.
     */
    public static final int STATE_CONNECTING = 9;

    public static boolean isRCSCall = false;

    public static boolean isFileReadPermissionsFirstTime = true;

    public static boolean isImageShareSupported = false;
    
    public static boolean isVideoShareSupported = false;

    public static void setEnabled(boolean isRCSCall) {
        if (DBG) {
            log("RCSe set enabled as " + isRCSCall);
        }
        RCSeUtils.isRCSCall = isRCSCall;
    }

    public static boolean isEnabled() {        
        return isRCSCall;
    }
    /**
     * Gets the m fg call.
     *
     * @return the m fg call
     */
    public static Call getmFgCall() {
        if (DBG) {
            log("getmfgCall" + sFgCall);
        }
        return sFgCall;
    }
    /**
     * Sets the m fg call.
     *
     * @param mfgCall the new m fg call
     */
    public static void setmFgCall(Call mfgCall) {
        if (mfgCall != null) {
            sFgCall = mfgCall;
            if(sFgCall.getDetails().getHandle() == null){
                return;
            }                
            String number = sFgCall.getDetails().getHandle().toString().substring(4);
            if (sFgCall.getState() == STATE_DISCONNECTED) {
                RCSeInCallUIExtension.getInstance().onDisconnect(
                        number);
            }
        }
        if (DBG) {
            log("setmfgCall" + mfgCall);
        }
    }
    /**
     * Can share.
     *
     * @param cm the cm
     * @return true, if successful
     */
    public static boolean canShare(CallManager cm) {
        if (!canShareFromCallState(cm)) {
            if (DBG) {
                log("sharing canShare is false");
            }
            return false;
        }
        String number = getRCSePhoneNumber(cm);
        if (null != number) {
            return RCSeInCallUIExtension.isCapabilityToShare(number);
            //return true;
        } else {
            if (DBG) {
                log("get rcse phone number failed, return false");
            }
            return true;
        }
    }
    /**
     * Can share image.
     *
     * @param cm the cm
     * @return true, if successful
     */
    public static boolean canShareImage(CallManager cm) {
        if (!canShareFromCallState(cm)) {
            if (DBG) {
                log("sharing canShare is false");
            }
            return false;
        }
        String number = getRCSePhoneNumber(cm);
        if (null != number) {
            isImageShareSupported = RCSeInCallUIExtension
                    .isCapabilityToShareImage(number);
            return isImageShareSupported;
        } else {
            if (DBG) {
                log("sharing get rcse phone number failed in canShareImage, return false");
            }
            return false;
        }
    }
    /**
     * Can share video.
     *
     * @param cm the cm
     * @return true, if successful
     */
    public static boolean canShareVideo(CallManager cm) {
        if (!canShareFromCallState(cm)) {
            if (DBG) {
                log("canShareVideo return false");
            }
            return false;
        }
        String number = getRCSePhoneNumber(cm);
        if (null != number) {
            isVideoShareSupported = RCSeInCallUIExtension
                    .isCapabilityToShareVideo(number);
            return isVideoShareSupported;
        } else {
            if (DBG) {
                log("get rcse phone number failed in canShareVideo, return false");
            }
            return false;
        }
    }
    /**
     * Can share from call state.
     *
     * @param cm the cm
     * @return true, if successful
     */
    public static boolean canShareFromCallState(CallManager cm) {
        if (DBG) {
            log("can share from call state entry" + sFgCall);
        }
        if (sFgCall != null) {
            if (sFgCall.getState() == STATE_CONNECTING
                    || sFgCall.getState() == STATE_DIALING
                    || sFgCall.getState() == STATE_DISCONNECTED
                    || sFgCall.getState() == STATE_HOLDING
                    || sFgCall.getState() == STATE_NEW
                    || sFgCall.getState() == STATE_PRE_DIAL_WAIT
                    || sFgCall.getState() == STATE_RINGING) {
                if (DBG) {
                    log("can not share for ring call is active");
                }
                return false;
            }
            if (cm.hasActiveBgCall()) {
                if (DBG) {
                    log("can not share for background call is active");
                }
                return true;
            }
            /*
             * if (cm.getForegroundCalls().size() > 1) { if (DBG)
             * log("can not share for foreground call count > 1"); return false; }
             */
            if (cm.getFgCallConnections().size() > 1) {
                if (DBG) {
                    log("can not share for foreground call connection > 1");
                }
                return false;
            }
            if (sFgCall.getState() != STATE_ACTIVE) {
                if (DBG) {
                    log("can not share for foreground call state is not ACTIVE");
                }
                return false;
            }
            /*if (sFgCall.getDetails().getVideoState() != VideoProfile.VideoState.AUDIO_ONLY) {
                if (DBG) {
                    log("can not share for latest connection is video type");
                }
                return false;
            }*/
        }
        return true;
    }
    /**
     * Gets the RC se phone number.
     *
     * @param cm the cm
     * @return the RC se phone number
     */
    public static String getRCSePhoneNumber(CallManager cm) {
        if (sFgCall != null) {
            if(sFgCall.getDetails().getHandle() == null){
                return null;
            } 
            String number = sFgCall.getDetails().getHandle().toString().substring(4);
            if (DBG) {
                log("getRCSePhoneNumber(), call is " + sFgCall
                        + "number" + number);
            }
            if(number.startsWith("%2B"))
            {
            	number = "+" + number.substring(3);
            }
            return number;
        }
        return null;
    }
    /**
     * Should stop.
     *
     * @param cm the cm
     * @return true, if successful
     */
    public static boolean shouldStop(CallManager cm) {
        if (sFgCall != null) {
            if (sFgCall.getState() != STATE_ACTIVE) {
                if (DBG) {
                    log("should stop sharing for foreground call state is not ACTIVE");
                }
                return true;
            }
            /*if (sFgCall.getDetails().getVideoState() != VideoProfile.VideoState.AUDIO_ONLY) {
                if (DBG) {
                    log("should stop for latest connection is video type");
                }
                return true;
            }*/
        }
        return false;
    }
    /**
     * Log.
     *
     * @param msg the msg
     */
    private static void log(String msg) {
        Log.d(LOG_TAG, msg);
    }
}
