/*******************************************************************************
 * Software Name : RCS IMS Stack
 *
 * Copyright (C) 2010 France Telecom S.A.
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
 ******************************************************************************/
package com.orangelabs.rcs.core.ims.service.im.chat.event;

import com.gsma.services.rcs.chat.GroupChat.ParticipantStatus;

public class User {
    public final static String STATE_UNKNOWN = "unknown";
    public final static String STATE_CONNECTED = "connected";
    public final static String STATE_DISCONNECTED = "disconnected";
    public final static String STATE_DEPARTED = "departed";
    public final static String STATE_BOOTED = "booted";
    public final static String STATE_FAILED = "failed";
    public final static String STATE_BUSY = "busy";
    public final static String STATE_DECLINED = "declined";
    public final static String STATE_PENDING = "pending";
    public final static String STATE_PARTIAL = "partial"; // partial means, user is offline, not removed
    public final static String STATE_DELETED = "deleted"; //for participant removed by chairman
    public final static String STATE_FULL = "full";
    
    public final static String ROLE_CHAIRMAN = "chairman";
    public final static String ROLE_PARTICIPANT = "participant";
    public final static String ROLE_UNKNOWN = "unknown";
    
    private String entity;
    
    private boolean me;
    
    private String state = STATE_UNKNOWN;// element status in user
    
    private String displayName = null;
    
    private String disconnectionMethod = null;
    
    private String failureReason = null;
    
    private String userState = STATE_FULL; //parameter in user
    
    private String role = ROLE_UNKNOWN; // role can be chairman, participant
    
    private String etype = null;
    
    public User(String entity, boolean me, String userState) {
        this.entity = entity;
        this.me = me;
        if(userState != null){
        this.userState = userState;
    }
    }
    
    public String getEntity() {
        return entity;
    }
    
    public void setEtype(String etype) {
        this.etype = etype;
    }

    public String getEtype() {
        return etype;
    }

    public boolean isMe() {
        return me;
    }
    
    public String getState() {
        return state;
    }
    
    public static ParticipantStatus getParticipantStatus(String mstate){
        switch(mstate){
            case STATE_UNKNOWN:  return ParticipantStatus.valueOf(1);
            case STATE_PENDING:  return ParticipantStatus.valueOf(2);
            case STATE_CONNECTED: return ParticipantStatus.valueOf(3);
            case STATE_DISCONNECTED: return ParticipantStatus.valueOf(4);
            case STATE_DEPARTED: return ParticipantStatus.valueOf(5);
            case STATE_BOOTED: return ParticipantStatus.valueOf(5);
            case STATE_FAILED: return ParticipantStatus.valueOf(6);
            case STATE_BUSY: return ParticipantStatus.valueOf(8);
            case STATE_DECLINED: return ParticipantStatus.valueOf(7);
            case STATE_DELETED: return ParticipantStatus.valueOf(4);
            default: return ParticipantStatus.valueOf(0);
            
        }
    }

    public void setState(String state) {
        this.state = state;
    }
    
    public void setUserState(String userState) {
        this.userState = userState;
    }

    public String getUserState() {
        return userState;
    }

    public String getDisplayName() {
        return displayName;
    }

    public void setDisplayName(String displayName) {
        this.displayName = displayName;
    }
    
    public void setRole(String role) {
        this.role = role;
    }

    public String getRole() {
        return role;
    }

    public void setDisconnectionMethod(String method) {
        this.disconnectionMethod = method;
    }

    public String getDisconnectionMethod() {
        return disconnectionMethod;
    }
    
    public void setFailureReason(String reason) {
        this.failureReason = reason;
    }

    public String getFailureReason() {
        return failureReason;
    }

    public String toString() {
        String result = "user=" + entity + ", state=" + state;
        if (disconnectionMethod != null) {
            result += ", method=" + disconnectionMethod; 
        }
        if (failureReason != null) {
            result += ", reason=" + failureReason; 
        }
        if (userState != null) {
            result += ", userState=" + userState; 
        }
        return result;
    }

    public static boolean isConnected(String state) {
        return (state.equals(User.STATE_CONNECTED) || state.equals(User.STATE_PENDING) || state.equals(User.STATE_BOOTED));
    }
    
    public static boolean isDisconnected(String state) {
        return !isConnected(state);
    }
}
