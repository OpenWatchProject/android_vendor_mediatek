package com.mediatek.presence.core.ims.service.presence;

import android.content.Context;
import android.content.Intent;
import android.os.Handler;
import android.os.HandlerThread;
import android.os.Looper;

import com.mediatek.presence.core.CoreException;
import com.mediatek.presence.core.ims.ImsModule;
import com.mediatek.presence.platform.AndroidFactory;
import com.mediatek.presence.core.ims.service.ImsService;
import com.mediatek.presence.utils.logger.Logger;

import com.android.ims.internal.uce.presence.PresServiceInfo;

public class VzwPresenceService extends PresenceService{

    private Context mContext;
    public Logger logger = null;

    private HandlerThread mHandlerThread;
    private VzwPublishManager mVzwPublishManager = null;
    private int mSlotId = 0;

    public VzwPresenceService(ImsModule imsModule) throws CoreException {
        super(imsModule);
        mSlotId = imsModule.getSlotId();
        logger = Logger.getLogger(mSlotId, "VzwPresenceService");

        mContext = AndroidFactory.getApplicationContext();
        mHandlerThread = new HandlerThread("VzwPresenceService");
        mHandlerThread.start();
        mVzwPublishManager = new VzwPublishManager(
                    VzwPublishManager.class.getSimpleName(),
                    new Handler(mHandlerThread.getLooper()),
                    imsModule);
    }

    @Override
    public synchronized void start() {
        logger.debug("start()");
        mVzwPublishManager.startService();
    }

    /**
     * Stop the IMS service
     */
    @Override
    public synchronized void stop() {
        logger.debug("stop()");
        mVzwPublishManager.stopService();
    }

    /**
     * Check the IMS service
     */
    @Override
    public void check() {
    }

    @Override
    public void onForbiddenReceived(ImsModule.EabActions action, String reason) {
        mVzwPublishManager.onForbiddenReceived(action, reason);
    }

    @Override
    public void onNotProvisionedReceived() {
        mVzwPublishManager.onNotProvisionedReceived();
    }

    //AOSP procedure
    @Override
    public void setNewFeatureTagInfo(String featureTag, PresServiceInfo serviceInfo) {
        mVzwPublishManager.setNewFeatureTagInfo(featureTag, serviceInfo);
    }

    //AOSP procedure
    @Override
    public void publishCapability(int requestId) {
        mVzwPublishManager.setRequestId(requestId);
        mVzwPublishManager.sendInitialOrModifyPublish();
    }


}
