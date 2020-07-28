/*
 * Copyright (c) 2016-2018 TRUSTONIC LIMITED
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the TRUSTONIC LIMITED nor the names of its
 *    contributors may be used to endorse or promote products derived from
 *    this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
 * TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
package com.trustonic.teeclient;

import java.util.ArrayList;
import java.util.List;
import android.content.Context;
import android.content.ComponentName;
import android.content.Intent;
import android.content.ServiceConnection;
import android.os.IBinder;
import android.util.Log;

/*
 * This class provides a wrapper to handle bind session, it can be accessed from
 * both java and native
 */
class TeeBind {
    private static final String LOG_TAG = TeeBind.class.getSimpleName() + "_JAVA";
    private static Context cont_;
    private static BindSessionList  session_list_ = new BindSessionList();

    static public void registerContext(Context ctxt) {
        Log.i(LOG_TAG, "registerContext");
        cont_ = ctxt.getApplicationContext();
    }

    static public int bind(String starterClass) {
        Log.i(LOG_TAG, "bind target : " + starterClass);
        BindSession.BindReturnCode res = session_list_.bind(cont_, starterClass);
        // Translate the error code to fit JavaProcess::BindResult structure in the JNI
        switch (res) {
            case OK :
                return 0;
            case OVERLOAD :
                return 1;
            // FAILURE :
            default :
                return 2;
        }
    }

    static public int unbind(String starterClass) {
        Log.i(LOG_TAG, "unbind target : " + starterClass);
        BindSession.BindReturnCode res = session_list_.unbind(starterClass);
        // Translate the error code to fit JavaProcess::BindResult structure in the JNI
        switch (res) {
            case OK :
                return 0;
            case OVERLOAD :
                return 1;
            // FAILURE :
            default :
                return 2;
        }
    }

    static public void broadcastIntent(String action) {
        Log.i(LOG_TAG, "broadcastIntent " + action);
        Intent i = new Intent(action);
        cont_.sendBroadcast(i);
    }


};

/*
 * Class which implement the possibility to store the active bindSessions
 */
class BindSessionList {
    private static final String             LOG_TAG = TeeClient.class.getSimpleName() + "_JAVA";
    private static ArrayList<BindSession>   session_list_ = new ArrayList<BindSession>();

    // List element + index (return 2 results)
    static final class Elem {
        private final BindSession session_;
        private final int index_;

        public Elem(BindSession session, int index) {
            this.session_ = session;
            this.index_ = index;
        }

        public BindSession getSession() {
            return session_;
        }

        public int getIndex() {
            return index_;
        }
    }

    public boolean isEmpty() {
        return session_list_.isEmpty();
    }

    public Elem getElem(String intent) {
        if (!session_list_.isEmpty()) {
            for (int i = 0; i < session_list_.size(); i++) {
                String  intentI = session_list_.get(i).getIntent();
                if (intent.compareTo(intentI) == 0) {
                    return new Elem(session_list_.get(i), i);
                }
                // Continue
            }
        }
        return null;
    }

    public BindSession.BindReturnCode bind(Context cont, String starterClass) {
        BindSession bindSession;
        Elem e = this.getElem(starterClass);
        if (e == null) {
            // Session not existing yet, create one
            bindSession = new BindSession(cont, starterClass);
        } else {
            // Update the existing session
            bindSession = e.getSession();
        }
        BindSession.BindReturnCode res = bindSession.bind();
        if (res == BindSession.BindReturnCode.OK) {
            // Bind success and the session has not been "overload binded" (ie : not present in list)
            session_list_.add(bindSession);
        }
        Log.i(LOG_TAG, this + "");
        return res;
    }

    public BindSession.BindReturnCode unbind(String starterClass) {
        Elem e = this.getElem(starterClass);
        if (e == null) {
            Log.e(LOG_TAG, "Can't unbind : session " + starterClass + " not in list");
            return BindSession.BindReturnCode.FAILURE;
        }
        // e exists
        BindSession.BindReturnCode res = e.getSession().unbind();
        if (res == BindSession.BindReturnCode.OK) {
            // The unbind has actually been done : remove the session
            session_list_.remove(e.getIndex());
            Log.i(LOG_TAG, "Session removed from list : " + starterClass);
        }
        Log.i(LOG_TAG, this + "");
        return res;
    }

    public String toString() {
        String res = "";
        if (!session_list_.isEmpty()) {
            res = "Session list : \n";
            for (int i = 0; i < session_list_.size(); i++) {
                res += "    " + session_list_.get(i) + "\n";
            }
        } else {
            res = "Session list empty";
        }
        return res;
    }
};

// NOTE : we can't use the service connection "onServiceConnected" callback
// to be sure that the service is binded.
// The first idea was to use a lock to block the execution until the callback
// is executed. This solution does not work if the binding (ie : openDevice)
// is done is a callback like onCreate. In fact the onServiceConnected is
// executed in the same thread as the onCreate function.
// Since the openDevice (in onCreate) waits until the onServiceConnected to
// finish and the onServiceConnected waits for the onCreate to finish, it's a
// dead lock.
class BindSession {
    private static final String LOG_TAG = BindSession.class.getSimpleName();

    private String       target_intent_;
    private Context      cont_;
    private boolean      effectively_binded_ = false;
    private int          nb_bind_ = 0;

    public enum BindReturnCode {
        OK,
        OVERLOAD,
        FAILURE,
    }

    private void localLogD(String log) {
        Log.d(LOG_TAG, target_intent_ + ": " + log);
    }
    private void localLogI(String log) {
        Log.i(LOG_TAG, target_intent_ + ": " + log);
    }
    private void localLogE(String log) {
        Log.e(LOG_TAG, target_intent_ + ": " + log);
    }
    private void localLogW(String log) {
        Log.w(LOG_TAG, target_intent_ + ": " + log);
    }

    // Binding communication
    private ServiceConnection connection_ = new ServiceConnection() {
        public void onServiceConnected(ComponentName className, IBinder service) {
        }

        public void onServiceDisconnected(ComponentName className) {
        }
    };

    // Constructor
    public BindSession(Context cont, String targetIntent) {
        target_intent_ = targetIntent;
        cont_ = cont;
    }

    // Getters/Setters
    boolean isBinded() {
        return effectively_binded_;
    }

    public String getIntent() {
        return target_intent_;
    }

    public int getNbBind() {
        return nb_bind_;
    }

    // Function
    public BindReturnCode bind() {
        // Bind to the service
        BindReturnCode res = BindReturnCode.FAILURE;
        try {
            if (nb_bind_ == 0) {
                // Not already binded
                Intent intent = new Intent();
                ComponentName target = ComponentName.unflattenFromString(target_intent_);
                intent.setComponent(target);
                // Call startService before bindService, this prevents the "onDestroy
                // on last unbind effect" in the service. It won't be killed due to
                // the START_STICKY return in the onStartCommand service's
                // callback
                ComponentName name = cont_.startService(intent);
                if (name == null) {
                    localLogD("startService failed");
                    return BindReturnCode.FAILURE;
                }
                boolean ret = cont_.bindService(intent, connection_, Context.BIND_AUTO_CREATE);
                if (ret == false) {
                    localLogD("bindService failed");
                    return BindReturnCode.FAILURE;
                }
                effectively_binded_ = true;
                res = BindReturnCode.OK;
            } else {
                localLogI("nbBind increased");
                res = BindReturnCode.OVERLOAD;
            }
            nb_bind_++;
        } catch (java.lang.SecurityException e) {
            localLogD("bindService failed");
            e.printStackTrace();
            return BindReturnCode.FAILURE;
        }
        return res;
    }

    public BindReturnCode unbind() {
        BindReturnCode res = BindReturnCode.FAILURE;
        try {
            if ((nb_bind_ - 1) == 0) {
                //Unbind from the service
                cont_.unbindService(connection_);
                effectively_binded_ = false;
                res = BindReturnCode.OK;
            } else {
                localLogI("nbBind decreased");
                res = BindReturnCode.OVERLOAD;
            }
            nb_bind_--;
        } catch (java.lang.SecurityException e) {
            localLogE("unbindService failed");
            e.printStackTrace();
            return BindReturnCode.FAILURE;
        }
        return res;
    }

    public String toString() {
        return "Session [ intent(" + target_intent_ + "), isBinded(" + effectively_binded_ + "), nbBind(" + nb_bind_ +") ] ";
    }
}
