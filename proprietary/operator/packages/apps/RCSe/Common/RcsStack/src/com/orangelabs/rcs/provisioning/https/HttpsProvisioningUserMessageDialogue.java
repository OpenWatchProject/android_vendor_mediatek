package com.orangelabs.rcs.provisioning.https;

import android.app.Activity;
import com.orangelabs.rcs.provisioning.ProvisioningInfo;
import com.orangelabs.rcs.provisioning.TermsAndConditionsRequest;
import com.orangelabs.rcs.utils.logger.Logger;

import android.content.Context;
import android.content.Intent;

/**
 * HTTPS provisioning - Input of MSISDN
 *
 * @author Orange
 */
public final class HttpsProvisioningUserMessageDialogue {

    private Logger logger = Logger.getLogger(this.getClass().getName());
    private boolean accept=true;
    /**
     * HttpsProvionningMSISDNInput instance
     */
    private static volatile HttpsProvisioningUserMessageDialogue instance = null;

    
    /**
     * Constructor
     */
    private HttpsProvisioningUserMessageDialogue() {
        super();
    }

    

    /**
     * Returns the Instance of HttpsProvionningMSISDNDialog
     *
     * @return Instance of HttpsProvionningMSISDNDialog
     */
    public final static HttpsProvisioningUserMessageDialogue getInstance() {
        if (HttpsProvisioningUserMessageDialogue.instance == null) {
            synchronized (HttpsProvisioningUserMessageDialogue.class) {
                if (HttpsProvisioningUserMessageDialogue.instance == null) {
                    HttpsProvisioningUserMessageDialogue.instance = new HttpsProvisioningUserMessageDialogue();
                }
            }
        }
        return HttpsProvisioningUserMessageDialogue.instance;
    }

    /**
     * Display the MSISDN popup
     *
     * @param context
     * @return 
     */
    protected boolean displayPopupAndWaitResponse(Context context,ProvisioningInfo info) {
        
        if (logger.isActivated()) {
            logger.info("Display Popup for User message");
        }
        final Intent intent = new Intent(Intent.ACTION_VIEW);
        intent.setClass(context, TermsAndConditionsRequest.class);

        // Required as the activity is started outside of an Activity context
        intent.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
        intent.addFlags(Intent.FLAG_ACTIVITY_EXCLUDE_FROM_RECENTS);

        // Add intent parameters
        intent.putExtra(TermsAndConditionsRequest.ACCEPT_BTN_KEY, info.getAcceptBtn());
        intent.putExtra(TermsAndConditionsRequest.REJECT_BTN_KEY, info.getRejectBtn());
        intent.putExtra(TermsAndConditionsRequest.TITLE_KEY, info.getTitle());
        intent.putExtra(TermsAndConditionsRequest.MESSAGE_KEY, info.getMessage());
        

        context.startActivity(intent);
        
        try {
            synchronized (HttpsProvisioningUserMessageDialogue.instance) {
                super.wait();
            }
        } catch (InterruptedException e) {
            // nothing to do
        }

        return accept;
    }

    
    /**
     * Callback of the User Message
     *
     * @param value
     */
    public void responseReceived(boolean value) {
        if (logger.isActivated()) {
            logger.info("Response received for User Message");
        }
        synchronized (HttpsProvisioningUserMessageDialogue.instance) {
            accept=value;   
            super.notify();
        }
    }
    
       
}
