/*
* Copyright (C) 2014 MediaTek Inc.
* Modification based on code covered by the mentioned copyright
* and/or permission notice(s).
*/
/*
 * Copyright (C) 2011 The Android Open Source Project
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

package com.android.email;

import java.lang.Thread.UncaughtExceptionHandler;

import android.app.Application;
import android.app.FragmentManager;
import android.app.LoaderManager;
import android.app.Notification;
import android.app.Service;
import android.content.Context;
import android.content.Intent;
import android.content.res.Configuration;
import android.os.Build;
import android.os.Trace;

import com.android.email.activity.setup.EmailPreferenceActivity;
import com.android.email.preferences.EmailPreferenceMigrator;
import com.android.email.provider.EmailProvider;
import com.android.email.provider.WidgetProvider;

import com.android.mail.browse.ConversationMessage;
import com.android.mail.browse.InlineAttachmentViewIntentBuilder;
import com.android.mail.browse.InlineAttachmentViewIntentBuilderCreator;
import com.android.mail.browse.InlineAttachmentViewIntentBuilderCreatorHolder;
import com.android.mail.preferences.BasePreferenceMigrator;
import com.android.mail.preferences.PreferenceMigratorHolder;
import com.android.mail.preferences.PreferenceMigratorHolder.PreferenceMigratorCreator;
import com.android.mail.providers.Account;
import com.android.mail.ui.settings.PublicPreferenceActivity;
import com.android.mail.utils.LogTag;
import com.android.mail.utils.LogUtils;
import com.android.mail.utils.NotificationActionUtils;
import com.android.mail.utils.StorageLowState;
import com.android.mail.utils.VipNotificationUtils;

import com.mediatek.email.extension.OPExtensionFactory;
import com.mediatek.email.util.EmailLowStorageHandler;
import com.mediatek.email.util.PermissionsUtil;
import com.mediatek.mail.vip.VipMemberCache;

public class EmailApplication extends Application {
    private static final String LOG_TAG = "Email";
    protected WidgetProvider widgetReceiver = null;
    static {
        LogTag.setLogTag(LOG_TAG);

        PreferenceMigratorHolder.setPreferenceMigratorCreator(new PreferenceMigratorCreator() {
            @Override
            public BasePreferenceMigrator createPreferenceMigrator() {
                return new EmailPreferenceMigrator();
            }
        });

        InlineAttachmentViewIntentBuilderCreatorHolder.setInlineAttachmentViewIntentCreator(
                new InlineAttachmentViewIntentBuilderCreator() {
                    @Override
                    public InlineAttachmentViewIntentBuilder
                    createInlineAttachmentViewIntentBuilder(Account account, long conversationId) {
                        return new InlineAttachmentViewIntentBuilder() {
                            @Override
                            public Intent createInlineAttachmentViewIntent(Context context,
                                    String url, ConversationMessage message) {
                                return null;
                            }
                        };
                    }
                });

        PublicPreferenceActivity.sPreferenceActivityClass = EmailPreferenceActivity.class;

        NotificationControllerCreatorHolder.setNotificationControllerCreator(
                new NotificationControllerCreator() {
                    @Override
                    public NotificationController getInstance(Context context){
                        return EmailNotificationController.getInstance(context);
                    }
                });
    }

    /**
     * M: Monitor the configuration change, and update the plugin's context.
     * @see android.app.Application#onConfigurationChanged(android.content.res.Configuration)
     */
    @Override
    public void onConfigurationChanged(Configuration newConfig) {
        super.onConfigurationChanged(newConfig);
        OPExtensionFactory.resetAllPluginObject(getApplicationContext());
    }

    @Override
    public void onCreate() {
        /** M: Use for catch UncaughtException in thread @{ */
        mExceptionHandler = new UncaughtHandler();
        mDefaultExceptionHandler = Thread.getDefaultUncaughtExceptionHandler();
        Thread.setDefaultUncaughtExceptionHandler(mExceptionHandler);
        /** @} */
        /// M: add trace with AMS tag, for AMS bindApplication process.
        Trace.beginSection("+logEmailApplicationLaunchTime : onCreate");
        LogUtils.e(LogTag.getLogTag(), "email app onCreate ");
        initAppInfo();
        NotificationActionUtils.initNotificationChannels(this);

        super.onCreate();
        /**
         * M: Enable services after Instrumentation.onCreate to avoid potential Service ANR
         * Reason:
         *     BindApplication will be called as AttachApplication->InstallProviders->
         *     Instrumentation.onCreate->Application.onCreate.
         *     It is tricky that Instrumentation.onCreate may take a long time(10s-30s+) to
         *     load classes from cached dexfile when running InstrumentationTest.
         * Solution:
         *      Move services enable from EmailProvider.onCreate to EmailApplication.onCreate
         *      to make sure we start service after Insturmentation.onCreate.
         */
        EmailProvider.setServicesEnabledAsync(this);

        // M: Init the Vip member cache
        VipMemberCache.init(this);
        /// M: Set low storage handler for email.
        StorageLowState.registerHandler(new EmailLowStorageHandler(this));
        /// M: Should active to check the storage state when we register handler to
        //  avoid email launched behind the low storage broadcast.
        StorageLowState.checkStorageLowMode(this);
        /// M: for debugging fragment issue.
        FragmentManager.enableDebugLogging(Build.TYPE.equals("eng"));
        /// M: for debuging loader issue.
        LoaderManager.enableDebugLogging(Build.TYPE.equals("eng"));
        /// M: Update widgets
        EmailProvider.updateWidgets(this);
        /// M: add trace with AMS tag, for AMS bindApplication process.
        Trace.endSection();
        if(widgetReceiver == null) {
            widgetReceiver = WidgetProvider.registerReceivers(getApplicationContext());	
        }
    }
    // Foreground Service implementation
    private static int sAppTargetVersion = 0;
    private static boolean isNotificationChannelStarted = false;
    private void initAppInfo() {
        sAppTargetVersion = getApplicationInfo().targetSdkVersion;
        LogUtils.d(LOG_TAG, "sAppTargetVersion, version = " + sAppTargetVersion);
    }

    private static boolean isForegroundServiceEnabled() {
        if (sAppTargetVersion >= Build.VERSION_CODES.O) {
            return true;
        }
        return false;
    }
    
    public static  boolean checkAndStartForegroundService(Context context, Intent intent, String callerLog) {
        //if(!isNotificationChannelStarted) {
            //initAppInfo();
            //NotificationActionUtils.initNotificationChannel(this);
            //isNotificationChannelStarted = true;
       // }
        LogUtils.d(LOG_TAG, "checkAndStartForegroundService, version = " + sAppTargetVersion + ", " + callerLog);
        if (isForegroundServiceEnabled()) {
            context.startForegroundService(intent);
            return true;
        } else {
            context.startService(intent);
            return false;
        }
    }

    public static  boolean checkAndStartForeground(Service service, int id, String callerLog) {
        LogUtils.d(LOG_TAG, "checkAndStartForeground, version = "+ sAppTargetVersion + ", id = " + id + ", " + callerLog);
        if (isForegroundServiceEnabled()) {
            Notification notice = 
                        NotificationActionUtils.getForegroundServiceNotification(service);
            service.startForeground(id, notice);
            return true;
        } else {
            return false;
        }
    }

    public static  boolean checkAndStopForeground(Service service, String callerLog) {
    	  LogUtils.d(LOG_TAG, "checkAndStopForeground, version = " + sAppTargetVersion + ", " + callerLog);
        if (isForegroundServiceEnabled()) {
            service.stopForeground(true);
            return true;
        } else {
            return false;
        }
    }        


    //M: Runtime Permission Check state
    private boolean mPermIsChecking = false;
    private boolean mDuplicateRequest = false;

    public void setPermissionCheckingStateFlag(boolean flag){
        LogUtils.d(LogTag.getLogTag(), new Throwable(),"TestLogFilter setPermissionCheckingStateFlag, Flag: " + flag);
        this.mPermIsChecking = flag;
    }

    public boolean getPermissionCheckingStateFlag(){
        LogUtils.e(LogTag.getLogTag(), "TestLogFilter getPermissionCheckingStateFlag, mPermIsChecking: " + mPermIsChecking);
        return mPermIsChecking;
    }

    public void setDuplicateRequest(boolean isDup) {
        this.mDuplicateRequest = isDup;
    }

    public boolean isDuplicateRequest() {
        return mDuplicateRequest;
    }


    private UncaughtHandler mExceptionHandler;
    private UncaughtExceptionHandler mDefaultExceptionHandler;

    /**
     * M:The class is used to catch UncaughtException in thread,mainly
     * ProviderUnavailableException. The exception don't need to be visible for
     * user,so the operation is only mainly to kill the thread which being
     * terminated by providerUnavailableException and the exception dialog will
     * not be displayed.
     */
    private class UncaughtHandler implements Thread.UncaughtExceptionHandler {

        @Override
        public void uncaughtException(Thread thread, Throwable ex) {
            LogUtils.e(LogTag.getLogTag(), ex, "email uncaughtException [%s]", ex
                    .getMessage());
            if (ex instanceof SecurityException
                    && (ex.getMessage() != null && ex.getMessage().startsWith("Permission Denial:"))) {
                LogUtils.e(LogTag.getLogTag(), "check permission uncaughtException");
                String[] permissions = PermissionsUtil.needPermissionList(EmailApplication.this);
                if (permissions.length > 0) {
                    LogUtils.e(LogTag.getLogTag(), "uncaughtException show toast");
                    /*
                     * Toast.makeText(getApplicationContext(),
                     * "Please allow permssion in setting and enter email again"
                     * , Toast.LENGTH_LONG).show();
                     */
                    LogUtils.e(LogTag.getLogTag(), "uncaughtException show Notification");
                    PermissionsUtil.createPermissionNotification(getApplicationContext());
                    /*
                     * getApplicationContext().startActivity(
                     * PermissionsUtil.getPermissionIntent
                     * (getApplicationContext()));
                     */
                    LogUtils.e(LogTag.getLogTag(), "uncaughtException myPid "
                            + android.os.Process.myPid());
                    android.os.Process.killProcess(android.os.Process.myPid());
                    LogUtils.e(LogTag.getLogTag(), "uncaughtException System.exit");
                    System.exit(-1);
                } else {
                    mDefaultExceptionHandler.uncaughtException(thread, ex);
                }
            } else {
                mDefaultExceptionHandler.uncaughtException(thread, ex);
            }
        }
    }
}
