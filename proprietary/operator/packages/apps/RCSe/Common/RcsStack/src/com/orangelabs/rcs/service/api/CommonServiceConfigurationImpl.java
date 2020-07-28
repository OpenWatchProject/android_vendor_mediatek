package com.orangelabs.rcs.service.api;

import com.orangelabs.rcs.provider.settings.RcsSettings;
import com.orangelabs.rcs.utils.logger.Logger;
import com.gsma.services.rcs.CommonServiceConfiguration.MessagingMethod;
import com.gsma.services.rcs.CommonServiceConfiguration.MinimumBatteryLevel;
import com.gsma.services.rcs.ICommonServiceConfiguration;
import com.gsma.services.rcs.contact.ContactId;

import android.os.RemoteException;
import android.text.TextUtils;

/**
 * A class that implements interface to allow access to common service configuration from APIs
 * 
 * @author Philippe LEMORDANT
 */
public class CommonServiceConfigurationImpl extends ICommonServiceConfiguration.Stub {

    private static final Logger sLogger = Logger.getLogger(CommonServiceConfigurationImpl.class
            .getSimpleName());

    private final RcsSettings mRcsSettings;

    /**
     * Constructor
     * 
     * @param rcsSettings RCS settings accessor
     */
    public CommonServiceConfigurationImpl(RcsSettings rcsSettings) {
        mRcsSettings = rcsSettings;
    }

    @Override
    public int getDefaultMessagingMethod() throws RemoteException {
        try {
            return mRcsSettings.getDefaultMessagingMethod().toInt();

        } catch (ServerApiBaseException e) {
            if (!e.shouldNotBeLogged()) {
                sLogger.error(ExceptionUtil.getFullStackTrace(e));
            }
            throw e;

        } catch (Exception e) {
            sLogger.error(ExceptionUtil.getFullStackTrace(e));
            throw new ServerApiGenericException(e);
        }
    }

    @Override
    public int getMessagingUX() throws RemoteException {
        try {
            return  mRcsSettings.getMessagingUx();

        } catch (ServerApiBaseException e) {
            if (!e.shouldNotBeLogged()) {
                sLogger.error(ExceptionUtil.getFullStackTrace(e));
            }
            throw e;

        } catch (Exception e) {
            sLogger.error(ExceptionUtil.getFullStackTrace(e));
            throw new ServerApiGenericException(e);
        }
    }

    @Override
    public ContactId getMyContactId() throws RemoteException {
        try {
            return mRcsSettings.getUserProfileImsUserNameContact();

        } catch (ServerApiBaseException e) {
            if (!e.shouldNotBeLogged()) {
                sLogger.error(ExceptionUtil.getFullStackTrace(e));
            }
            throw e;

        } catch (Exception e) {
            sLogger.error(ExceptionUtil.getFullStackTrace(e));
            throw new ServerApiGenericException(e);
        }
        
        //return new ContactId();
    	
    }

    @Override
    public String getMyDisplayName() throws RemoteException {
        try {
            String displayName = mRcsSettings.getUserProfileImsDisplayName();
            if (TextUtils.isEmpty(displayName)) {
                return null;
            }
            return displayName;

        } catch (ServerApiBaseException e) {
            if (!e.shouldNotBeLogged()) {
                sLogger.error(ExceptionUtil.getFullStackTrace(e));
            }
            throw e;

        } catch (Exception e) {
            sLogger.error(ExceptionUtil.getFullStackTrace(e));
            throw new ServerApiGenericException(e);
        }
    }

    @Override
    public boolean isConfigValid() throws RemoteException {
        try {
            return mRcsSettings.isConfigurationValid();

        } catch (ServerApiBaseException e) {
            if (!e.shouldNotBeLogged()) {
                sLogger.error(ExceptionUtil.getFullStackTrace(e));
            }
            throw e;

        } catch (Exception e) {
            sLogger.error(ExceptionUtil.getFullStackTrace(e));
            throw new ServerApiGenericException(e);
        }
    }

    @Override
    public void setDefaultMessagingMethod(int method) throws RemoteException {
        try {
            mRcsSettings.setDefaultMessagingMethod(MessagingMethod.valueOf(method));
        } catch (ServerApiBaseException e) {
            if (!e.shouldNotBeLogged()) {
                sLogger.error(ExceptionUtil.getFullStackTrace(e));
            }
            throw e;

        } catch (Exception e) {
            sLogger.error(ExceptionUtil.getFullStackTrace(e));
            throw new ServerApiGenericException(e);
        }
    }

    @Override
    public void setMyDisplayName(String name) throws RemoteException {
        try {
            if (TextUtils.isEmpty(name)) {
                mRcsSettings.setUserProfileImsDisplayName("");
                return;
            }
            if (name.length() > mRcsSettings.getMaxAllowedDisplayNameChars()) {
                throw new ServerApiIllegalArgumentException("name exceeds max allowed characters!");
            }
            mRcsSettings.setUserProfileImsDisplayName(name);
        } catch (ServerApiBaseException e) {
            if (!e.shouldNotBeLogged()) {
                sLogger.error(ExceptionUtil.getFullStackTrace(e));
            }
            throw e;

        } catch (Exception e) {
            sLogger.error(ExceptionUtil.getFullStackTrace(e));
            throw new ServerApiGenericException(e);
        }
    }

    @Override
    public int getMinimumBatteryLevel() throws RemoteException {
        try {
            return mRcsSettings.getMinBatteryLevel();

        } catch (ServerApiBaseException e) {
            if (!e.shouldNotBeLogged()) {
                sLogger.error(ExceptionUtil.getFullStackTrace(e));
            }
            throw e;

        } catch (Exception e) {
            sLogger.error(ExceptionUtil.getFullStackTrace(e));
            throw new ServerApiGenericException(e);
        }
    }

    @Override
    public void setMinimumBatteryLevel(int level) throws RemoteException {
        try {
            mRcsSettings.setMinBatteryLevel(level);
        } catch (ServerApiBaseException e) {
            if (!e.shouldNotBeLogged()) {
                sLogger.error(ExceptionUtil.getFullStackTrace(e));
            }
            throw e;

        } catch (Exception e) {
            sLogger.error(ExceptionUtil.getFullStackTrace(e));
            throw new ServerApiGenericException(e);
        }
    }

}

