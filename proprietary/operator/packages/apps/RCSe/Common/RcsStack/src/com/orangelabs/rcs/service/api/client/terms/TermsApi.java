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
package com.orangelabs.rcs.service.api.client.terms;

import com.gsma.services.rcs.JoynService;
import com.gsma.services.rcs.JoynServiceException;
import com.gsma.services.rcs.JoynServiceListener;
import com.gsma.services.rcs.JoynServiceNotAvailableException;

import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.ServiceConnection;
import android.os.IBinder;


/**
 * Terms & conditions API
 * 
 * @author jexa7410
 */
public class TermsApi extends JoynService {

    /**
     * Core service API
     */
    private ITermsApi coreApi = null;
    
    /**
     * Constructor
     * 
     * @param ctx Application context
     */
    public TermsApi(Context ctx, JoynServiceListener listener) {
        super(ctx, listener);
    }
    
    @Override
    public void connect() {

        ctx.bindService(new Intent(ITermsApi.class.getName()), apiConnection, 0);
    }
    
    @Override
    public void disconnect() {
        
        try {
            ctx.unbindService(apiConnection);
        } catch (IllegalArgumentException e) {
            // Nothing to do
        }
    }

    /**
     * Core service API connection
     */
    private ServiceConnection apiConnection = new ServiceConnection() {
        public void onServiceConnected(ComponentName className, IBinder service) {
            coreApi = ITermsApi.Stub.asInterface(service);


        }

        public void onServiceDisconnected(ComponentName className) {


            coreApi = null;
        }
    };

    /**
     * Accept terms and conditions via SIP
     *
     * @param id Request id
     * @param pin PIN
     * @return Boolean result
     * @throws ClientApiException
     */
    public boolean acceptTerms(String id, String pin) throws JoynServiceException {    
        if (coreApi != null) {
            try {
                return coreApi.acceptTerms(id, pin);
            } catch(Exception e) {
                throw new JoynServiceException(e.getMessage());
            }
        } else {
            throw new JoynServiceNotAvailableException();
        }
    }

    /**
     * Reject terms and conditions via SIP
     *
     * @param id Request id
     * @param pin PIN
     * @return Boolean result
     * @throws ClientApiException
     */
    public boolean rejectTerms(String id, String pin) throws JoynServiceException {    
        if (coreApi != null) {
            try {
                return coreApi.rejectTerms(id, pin);
            } catch(Exception e) {
                throw new JoynServiceException(e.getMessage());
            }
        } else {
            throw new JoynServiceNotAvailableException();
        }
    }
}
