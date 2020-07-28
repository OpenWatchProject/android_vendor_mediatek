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
package com.mediatek.rcse.api.terms;

import java.util.List;

import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.ServiceConnection;
import android.content.pm.PackageManager;
import android.content.pm.ResolveInfo;
import android.os.IBinder;
import android.os.RemoteException;

import com.gsma.services.rcs.JoynService;
import com.gsma.services.rcs.JoynServiceException;
import com.gsma.services.rcs.JoynServiceListener;
import com.gsma.services.rcs.JoynServiceNotAvailableException;

import com.mediatek.rcse.service.ApiService;
//import com.orangelabs.rcs.service.RcsCoreService;


/**
 * Terms & conditions API.
 *
 * @author jexa7410
 */
public class TermsApi extends JoynService {
    /**
     * Core service API.
     */
    private ITermsApi mCoreApi = null;
    
    /**
     * The context.
     */
    private Context mContext = null;

    /**
     * Constructor.
     *
     * @param ctx            Application context
     * @param listener the listener
     */
    public TermsApi(Context ctx, JoynServiceListener listener) {
        super(ctx, listener);
        mContext = ctx;
    }
    /* (non-Javadoc)
     * @see com.gsma.services.rcs.JoynService#connect()
     */
    @Override
    public void connect() {
        /*Intent intent = new Intent(mContext, RcsCoreService.class);
        intent.putExtra("action",ITermsApi.class.getName());
        ctx.bindService(intent, mApiConnection, 0);*/
        Intent intent = new Intent();
        ComponentName cmp = new ComponentName("com.orangelabs.rcs", "com.orangelabs.rcs.service.RcsCoreService"); 
        intent.setComponent(cmp);       
        ctx.bindService(intent, mApiConnection, 0);
    }
    /* (non-Javadoc)
     * @see com.gsma.services.rcs.JoynService#disconnect()
     */
    @Override
    public void disconnect() {
        try {
            ctx.unbindService(mApiConnection);
        } catch (IllegalArgumentException e) {
            // Nothing to do
        }
    }

    /**
     * Core service API connection.
     */
    private ServiceConnection mApiConnection = new ServiceConnection() {
        public void onServiceConnected(ComponentName className,
                IBinder service) {
            mCoreApi = ITermsApi.Stub.asInterface(service);
        }
        public void onServiceDisconnected(ComponentName className) {
            mCoreApi = null;
        }
    };

    /**
     * Accept terms and conditions via SIP.
     *
     * @param id            Request id
     * @param pin            PIN
     * @return Boolean result
     * @throws JoynServiceException the joyn service exception
     */
    public boolean acceptTerms(String id, String pin)
            throws JoynServiceException {
        if (mCoreApi != null) {
            try {
                return mCoreApi.acceptTerms(id, pin);
            } catch (RemoteException e) {
                throw new JoynServiceException(e.getMessage());
            }
        } else {
            throw new JoynServiceNotAvailableException();
        }
    }
    /**
     * Reject terms and conditions via SIP.
     *
     * @param id            Request id
     * @param pin            PIN
     * @return Boolean result
     * @throws JoynServiceException the joyn service exception
     */
    public boolean rejectTerms(String id, String pin)
            throws JoynServiceException {
        if (mCoreApi != null) {
            try {
                return mCoreApi.rejectTerms(id, pin);
            } catch (RemoteException e) {
                throw new JoynServiceException(e.getMessage());
            }
        } else {
            throw new JoynServiceNotAvailableException();
        }
    }
}
