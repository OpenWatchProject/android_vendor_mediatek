package com.mediatek.presence.service;

import com.android.ims.internal.uce.UceServiceBase;
import com.android.ims.internal.uce.uceservice.IUceListener;
import com.android.ims.internal.uce.uceservice.ImsUceManager;
import com.android.ims.internal.uce.presence.IPresenceService;
import com.android.ims.internal.uce.presence.IPresenceListener;
import com.android.ims.internal.uce.options.IOptionsListener;
import com.android.ims.internal.uce.options.IOptionsService;
import com.android.ims.internal.uce.common.UceLong;

import com.mediatek.presence.core.Core;
import com.mediatek.presence.core.CoreException;
import com.mediatek.presence.core.ims.network.ImsConnectionManager;
import com.mediatek.presence.core.ims.service.presence.PresenceService;
import com.mediatek.presence.core.ims.service.capability.CapabilityService;
import com.mediatek.presence.utils.logger.Logger;
import com.mediatek.presence.service.api.PresenceServiceImpl;
import com.mediatek.presence.service.api.OptionsServiceImpl;
import com.mediatek.presence.service.api.ServerApiUtils;
import com.gsma.services.rcs.ICoreServiceWrapper;

import android.content.Context;
import android.content.ServiceConnection;
import android.content.Intent;
import android.content.ComponentName;
import android.os.RemoteException;
import android.os.IBinder;

import java.lang.IllegalArgumentException;

public class MtkUceService extends UceServiceBase {
    public static final int UCE_SERVUCE_NOT_STARTED = -1;
    public static final int PRESENCE_SERVICE_HDL = 0;
    public static final int OPTIONS_SERVICE_HDL = 0;
    public static final int DEFAULT_SLOT_ID = 0;
    private static Logger logger = Logger.getLogger(MtkUceService.class.getSimpleName());
    private Context mContext;
    private IPresenceService mPresenceService;
    private IOptionsService mOptionsService;
    private boolean mIsServiceStarted = false;
    private ICoreServiceWrapper mCoreServiceWrapperBinder = null;
    private IUceListener mUceListener = null;

    public MtkUceService(Context context) {
        mContext = context;
    }

    @Override
    protected boolean onServiceStart(IUceListener uceListener) {
        logger.debug("onServiceStart Entry");

        mUceListener = uceListener;
        if (mIsServiceStarted) {
            notifyUceServiceUpStatus();
        } else {
            connectToRcsCoreService();
        }

        return true;

    }

    @Override
    protected boolean onStopService() {
        logger.debug("onStopService Entry");

        if (!mIsServiceStarted) {
            logger.debug("onStopService Fail. UCE Service have not started");
            return false;
        }

        mCoreServiceWrapperBinder = null;
        mPresenceService = null;
        mOptionsService = null;
        mIsServiceStarted = false;
        notifyUceServiceDownStatus();

        mContext.unbindService(serviceConnection);
        return true;

    }

    @Override
    protected boolean onIsServiceStarted() {
        logger.debug("onIsServiceStarted Entry");
        logger.debug("mIsServiceStarted = " + mIsServiceStarted);

        return mIsServiceStarted;
    }

    @Override
    protected int onCreateOptionsService(IOptionsListener optionsListener,
            UceLong optionsServiceListenerHdl) {
        logger.debug("onCreateOptionsService Entry");

        if (!mIsServiceStarted) {
            logger.debug("onCreateOptionsService Fail. UCE Service have not started");
            return UCE_SERVUCE_NOT_STARTED;
        }

        if (mOptionsService != null) {
            return OPTIONS_SERVICE_HDL;
        }

        try {
            IBinder optionsServiceImpl =
                    mCoreServiceWrapperBinder.getAospOptionsServiceBinder();
            mOptionsService = IOptionsService.Stub.asInterface(optionsServiceImpl);
        } catch (RemoteException e) {
            logger.error("getAospOptionsServiceBinder Fail", e);
        }
        return OPTIONS_SERVICE_HDL;
    }

    @Override
    protected void onDestroyOptionsService(int cdServiceHandle) {
        logger.debug("onDestroyOptionsService Entry");

        if (!mIsServiceStarted) {
            logger.debug("onCreateOptionsService Fail. UCE Service have not started");
            return;
        }

        OptionsServiceImpl.serviceUnAvailable();
        mOptionsService = null;
        return;
    }

    @Override
    protected int onCreatePresService(IPresenceListener presServiceListener,
            UceLong presServiceListenerHdl) {
        logger.debug("onCreatePresService Entry");

        if (!mIsServiceStarted) {
            logger.debug("onCreatePresenceService Fail. UCE Service have not started");
            return UCE_SERVUCE_NOT_STARTED;
        }

        if (mPresenceService != null) {
            return PRESENCE_SERVICE_HDL;
        }

        try {
            IBinder presenceServiceImpl =
                    mCoreServiceWrapperBinder.getAospPresenceServiceBinder();
            mPresenceService = IPresenceService.Stub.asInterface(presenceServiceImpl);
        } catch (RemoteException e) {
            logger.error("getAospPresenceServiceBinder Fail", e);
        }
        return PRESENCE_SERVICE_HDL;
    }

    @Override
    protected void onDestroyPresService(int presServiceHdl) {
        logger.debug("onDestroyPresService Entry");

        if (!mIsServiceStarted) {
            logger.debug("onCreatePresenceService Fail. UCE Service have not started");
            return;
        }

        PresenceServiceImpl.serviceUnAvailable();
        mPresenceService = null;
        return;
    }

    @Override
    protected boolean onGetServiceStatus() {
        logger.debug("onCetServiceStatus Entry");

        if (!mIsServiceStarted) {
            return false;
        }

        boolean ret = ServerApiUtils.isImsConnected(DEFAULT_SLOT_ID);
        logger.debug("onGetServiceStatus: " + ret);

        return ret;
    }

    @Override
    protected IPresenceService onGetPresenceService() {
        logger.debug("onCetPresenceService Entry");

        if (!mIsServiceStarted) {
            logger.debug("onGetPresenceService Fail. UCE Service have not started");
            return null;
        }

        return mPresenceService;
    }

    @Override
    protected IOptionsService onGetOptionsService() {
        logger.debug("onGetOptionsService Entry");

        if (!mIsServiceStarted) {
            logger.debug("onGetOptionsService Fail. UCE Service have not started");
            return null;
        }

        return mOptionsService;
    }

    private void notifyUceServiceUpStatus() {
        if (mUceListener == null) {
            return;
        }

        try {
            if (ServerApiUtils.isImsConnected(DEFAULT_SLOT_ID)) {
                logger.debug("Notify Uce Service Status ON");
                mUceListener.setStatus(ImsUceManager.UCE_SERVICE_STATUS_ON);
            } else {
                logger.debug("Notify Uce Service Status READY");
                mUceListener.setStatus(ImsUceManager.UCE_SERVICE_STATUS_READY);
            }
            mUceListener = null;
        } catch (RemoteException e) {
            logger.error("Cannot notify UCE Service status to client", e);
        }
    }

    private void notifyUceServiceDownStatus() {
        if (mUceListener == null) {
            return;
        }

        try {
            logger.debug("Notify Uce Service Status DOWN");
            mUceListener.setStatus(ImsUceManager.UCE_SERVICE_STATUS_CLOSED);
            mUceListener = null;
        } catch (RemoteException e) {
            logger.error("Cannot notify UCE Service status to client", e);
        }
    }

    private void connectToRcsCoreService() {
        logger.debug("connectToRcsCoreService");
        ComponentName cmp = new ComponentName("com.mediatek.presence",
                "com.mediatek.presence.service.RcsCoreService");
        Intent intent = new Intent();
        intent.setComponent(cmp);
        mContext.bindService(intent, serviceConnection, Context.BIND_AUTO_CREATE);
    }

    private ServiceConnection serviceConnection = new ServiceConnection() {
        public void onServiceConnected(ComponentName className, IBinder service) {
            mCoreServiceWrapperBinder = ICoreServiceWrapper.Stub.asInterface(service);
            notifyUceServiceUpStatus();
            mIsServiceStarted = true;
        }

        public void onServiceDisconnected(ComponentName className) {
            logger.debug("onServiceDisconnected entry " + className);
            mCoreServiceWrapperBinder = null;
            mPresenceService = null;
            mOptionsService = null;
            mIsServiceStarted = false;
            notifyUceServiceDownStatus();
        }
    };

}

