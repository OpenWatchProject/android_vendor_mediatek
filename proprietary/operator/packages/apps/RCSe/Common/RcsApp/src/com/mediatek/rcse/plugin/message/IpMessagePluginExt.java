/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2012. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
 * AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek Software")
 * have been modified by MediaTek Inc. All revisions are subject to any receiver's
 * applicable license agreements with MediaTek Inc.
 */
package com.mediatek.rcse.plugin.message;


import android.content.BroadcastReceiver;

import android.content.ComponentName;
import android.content.ContentResolver;
import android.content.ContentValues;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.ServiceConnection;
import android.content.pm.PackageManager;
import android.content.pm.ResolveInfo;
import android.database.SQLException;
import android.net.Uri;
import android.os.AsyncTask;
import android.os.Handler;
import android.os.IBinder;
import android.os.IMessenger;
import android.os.Parcel;
import android.os.ParcelUuid;
import android.os.RemoteException;
import android.provider.Telephony.Sms;
import android.telephony.TelephonyManager;

import com.mediatek.mms.ipmessage.DefaultIpMessagePluginImplExt;
import com.mediatek.rcse.activities.widgets.ContactsListManager;
import com.mediatek.rcse.activities.widgets.PhotoLoaderManager;
import com.mediatek.rcse.api.Logger;
import com.mediatek.rcse.api.RegistrationApi;
import com.mediatek.rcse.interfaces.ChatController;
import com.mediatek.rcse.service.ApiManager;
import com.mediatek.rcse.service.ApiService;
import com.mediatek.rcse.service.IApiServiceWrapper;
import com.mediatek.rcse.service.PluginApiManager;
import com.mediatek.rcse.service.Utils;
import com.mediatek.rcse.service.VideoCompress;
import com.mediatek.rcse.plugin.message.IRemoteWindowBinder;
import com.mediatek.rcse.service.binder.ThreadTranslater;

import com.mediatek.rcse.service.MediatekFactory;
import com.mediatek.rcse.settings.RcsSettings;

import java.io.File;
import java.util.ArrayList;
import java.util.List;

import com.mediatek.mms.ipmessage.DefaultIpContactExt;
import com.mediatek.mms.ipmessage.DefaultIpConversationExt;
import com.mediatek.mms.ipmessage.DefaultIpConversationListExt;
import com.mediatek.mms.ipmessage.DefaultIpConversationListItemExt;
import com.mediatek.mms.ipmessage.DefaultIpMessageListItemExt;
import com.mediatek.mms.ipmessage.DefaultIpMessageItemExt;
import com.mediatek.mms.ipmessage.DefaultIpComposeActivityExt;
import com.mediatek.mms.ipmessage.DefaultIpDialogModeActivityExt;
import com.mediatek.mms.ipmessage.DefaultIpMessageListAdapterExt;
import com.mediatek.mms.ipmessage.DefaultIpMessagingNotificationExt;
import com.mediatek.mms.ipmessage.DefaultIpMultiDeleteActivityExt;
import com.mediatek.mms.ipmessage.DefaultIpConfigExt;
import com.mediatek.mms.ipmessage.DefaultIpScrollListenerExt;
import com.mediatek.mms.ipmessage.IIpScrollListenerExt;
import com.mediatek.mms.ipmessage.DefaultIpUtilsExt;

/**
 * Used to access RCSe plugin.
 */
// @PluginImpl(interfaceName="com.mediatek.mms.ipmessage.IIpMessagePluginExt")
public class IpMessagePluginExt extends DefaultIpMessagePluginImplExt implements
        PluginApiManager.RegistrationListener {
    /**
     * The Constant TAG.
     */
    private static final String TAG = "IpMessagePluginExt";
    /**
     * The m message manager.
     */
    private IpMessageManager mMessageManager;

    public static Context mContext;
    /**
     * The m receiver.
     */
    private ApiReceiver mReceiver = null;
    /**
     * The intent filter.
     */
    private IntentFilter mIntentFilter = null;
    /**
     * The m plugin chat window manager.
     */
    private PluginChatWindowManager mPluginChatWindowManager;

    private IRemoteWindowBinder mRemoteWindowBinder;

    /**
     * The m remote window binder.
     */
    private IApiServiceWrapper mApiServiceWrapperBinder;

    /**
     * The m is sim card available.
     */
    private final boolean mIsSimCardAvailable;
    /**
     * The m host context.
     */
    private Context mHostContext = null;
    /**
     * The enable joyn.
     */
    private static final int sENABLE_JOYN = 0;
    /**
     * The deactivate joyn temp.
     */
    private static final int sDEACTIVATE_JOYN_TEMP = 1;
    /**
     * The deactivate joyn permanently.
     */
    private static final int sDEACTIVATE_JOYN_PERMANENTLY = 2;
    /**
     * The m plugin cache cleared.
     */
    private static boolean sPluginCacheCleared = false;
    /**
     * The is rc se configured.
     */
    private static boolean sIsRCSeConfigured = false;
    /**
     * The Constant RCS_SETTINGS_PATH.
     */
    private static final String RCS_SETTINGS_PATH =
            "data/data/com.orangelabs.rcs/databases/rcs_settings.db";

    /**
     * Instantiates a new ip message plugin ext.
     *
     * @param context the context
     */
    public IpMessagePluginExt(Context context) {
        super(context);
        Logger.initialize(context);
        mContext = context;
        MediatekFactory.setApplicationContext(context);
        isActualPlugin();
        PluginApiManager.initialize(context);
        PluginApiManager.getInstance().addRegistrationListener(this);
        mIsSimCardAvailable = isSimCardAvailable(context);
        if (mIsSimCardAvailable) {
            PluginUtils.initializeSimIdFromTelephony(context);
        } else {
            Logger.d(TAG, "IpMessagePluginExt mIsSimCardAvailable"
                    + " is false, no need to initialize simId");
        }
        initialize(context);
    }
    /**
     * Gets the resource manager.
     *
     * @param context the context
     * @return the resource manager
     */

    @Override
    public DefaultIpMessageListItemExt getIpMessageListItem() {
        return new RcseMessageListItem();
    }

    @Override
    public DefaultIpMessageItemExt getIpMessageItem() {
        return new RcseMessageItem();
    }

    @Override
    public DefaultIpComposeActivityExt getIpComposeActivity() {
        return new RcseComposeActivity(mContext);
    }

    @Override
    public DefaultIpContactExt getIpContact() {
        return new RcseContact();
    }

    @Override
    public DefaultIpConversationExt getIpConversation() {
        return new RcseConversation();
        }

    @Override
    public DefaultIpConversationListExt getIpConversationList() {
        return new RcseConversationList();
        }

    @Override
    public DefaultIpConversationListItemExt getIpConversationListItem() {
        return new RcseConversationListItem();
    }

    @Override
    public DefaultIpDialogModeActivityExt getIpDialogModeActivity() {
        return new RcseDialogModeActivity(mHostContext);
        }

    @Override
    public DefaultIpMessageListAdapterExt getIpMessageListAdapter() {
        return new RcseMessageListAdapter(mContext);
        }

    @Override
    public DefaultIpMessagingNotificationExt getIpMessagingNotification() {
        return new RcseMessagingNotification();
    }

    @Override
    public DefaultIpMultiDeleteActivityExt getIpMultiDeleteActivity() {
        return new RcseMultiDeleteActivity();
        }

    @Override
    public DefaultIpConfigExt getIpConfig() {
        return new IpMmsConfig();
    }

    @Override
    public RcseSettingListActivity getIpSettingListActivity() {
        return new RcseSettingListActivity();
        }

    @Override
    public DefaultIpUtilsExt getIpUtils() {
        return new IpMessageUtils();
        }

    @Override
    public IIpScrollListenerExt getIpScrollListener() {
        return new RcseScrollListener();
    }
    /**
     * Initialize.
     *
     * @param context the context
     */
    private void initialize(Context context) {
    	Logger.v(TAG, "initialize , context = " + context);
        if (mHostContext == null) {
            mHostContext = context;
            Logger.v(TAG, "initialize , mHostContext null = " + mHostContext);
            mMessageManager = IpMessageManager.getInstance(context);

            //PhotoLoaderManager.initialize(context);
            if (!ApiManager.initialize(context)) {
                Logger.e(TAG,
                        "IpMessageServiceMananger() ApiManager initialization failed!");
            }
            RcsSettings.createInstance(context);
            RegistrationApi registrationApi = ApiManager.getInstance()
                    .getRegistrationApi();
            if (registrationApi != null) {
                IpMessageServiceMananger.getInstance(context).serviceIsReady();
            } else {
                Logger.d(TAG,
                        "getServiceManager() registrationApi is null, has not connected");
            }
            bindRcseService(context);
            ContactsListManager.initialize(context);
            if (mReceiver == null && mIntentFilter == null) {
                mReceiver = new ApiReceiver();
                mIntentFilter = new IntentFilter();
                mIntentFilter.addAction(PluginUtils.ACTION_DB_CHANGE_RELOAD);
                mIntentFilter
                        .addAction(IpMessageConsts.DisableServiceStatus.
                                ACTION_DISABLE_SERVICE_STATUS);
                mIntentFilter
                        .addAction(IpMessageConsts.IsOnlyUseXms.ACTION_ONLY_USE_XMS);
                mIntentFilter
                        .addAction(IpMessageConsts.JoynGroupInvite.ACTION_GROUP_IP_INVITATION);
                mIntentFilter.addAction(PluginUtils.ACTION_FILE_SEND);
                mIntentFilter.addAction(PluginUtils.ACTION_MODE_CHANGE);
                mIntentFilter.addAction(PluginUtils.ACTION_REALOD_FAILED);
                mIntentFilter.addAction(PluginUtils.ACTION_SEND_URL);
                mIntentFilter.addAction(PluginUtils.ACTION_SEND_RESIZED_FILE);
                mIntentFilter.addAction(PluginUtils.ACTION_SEND_CANCELLED_COMPRESSED_VIDEO);
                MediatekFactory.getApplicationContext().registerReceiver(
                        mReceiver, mIntentFilter);
            }
        } else {
            if (mMessageManager != null) {
                File dbFile = new File(RCS_SETTINGS_PATH);
                boolean isActivated = true;
                if (!dbFile.exists()) {
                    Logger.d(TAG, "dbFile not exists ");
                    if (RcsSettings.getInstance() != null) {
                        RcsSettings.getInstance().setIMSProfileValue(null);
                    }
                    isActivated = false;
                } else {
                    if (RcsSettings.getInstance() == null) {
                        Logger.d(TAG, "RcsSettings is null");
                        RcsSettings.createInstance(mHostContext);
                    }
                    String profile = RcsSettings.getInstance()
                            .getIMSProfileValue();
                    if (profile == null || profile.equals("")) {
                        isActivated = false;
                        Logger.d(TAG, "Configuration Profile Zero" + profile);
                    }
                }
                if (!mMessageManager.getsCacheRcseMessage().isEmpty()
                        && !isActivated) {
                    mMessageManager.getsCacheRcseMessage().clear();
                    Logger.d(TAG,
                            "Configuration Validity Zero but Plugin cache not empty");
                    ContentResolver contentResolver = mHostContext
                            .getContentResolver();
                    contentResolver.delete(Uri.parse("content://sms/"),
                            "ipmsg_id>0", null);
                }
            }
            Logger.d(TAG, "setContext(), context is exist!");
        }
    }
    /**
     * Bind rcse service.
     *
     * @param context the context
     */
    private void bindRcseService(Context context) {
        if (context == null) {
            Logger.e(TAG, "bindRcseService() context is null");
            return;
        }
        /*Intent intent = new Intent(mContext,ApiService.class);*/
       // ComponentName cmp = new ComponentName("com.orangelabs.rcs", "com.mediatek.rcse.service.RemoteBinderService");
       // intent.setComponent(cmp);
        //intent.setAction(IRemoteWindowBinder.class.getName());
            /*boolean result = context.bindService(intent, mServiceConnection,
                Context.BIND_AUTO_CREATE);*/

        Intent intent = new Intent();
        ComponentName cmp = new ComponentName("com.mediatek.rcs", "com.mediatek.rcse.service.ApiService");
        intent.setComponent(cmp);
        mContext.bindService(intent, mServiceConnection, 0);

    }

    /**
     * The m service connection.
     */
    private ServiceConnection mServiceConnection = new ServiceConnection() {
        @Override
        public void onServiceDisconnected(ComponentName className) {
            mRemoteWindowBinder = null;
            PluginController.destroyInstance();

        }
        @Override
        public void onServiceConnected(ComponentName className, IBinder service) {

            mApiServiceWrapperBinder = IApiServiceWrapper.Stub.asInterface(service);

            IBinder IWindowBinder = null;
            try {
                IWindowBinder = mApiServiceWrapperBinder.getRemoteWindowBinder();
            } catch (RemoteException e1) {
                // TODO Auto-generated catch block
                e1.printStackTrace();
            }
            mRemoteWindowBinder = IRemoteWindowBinder.Stub.asInterface(IWindowBinder);
            mPluginChatWindowManager = new PluginChatWindowManager(
                    mMessageManager);
            try {
                mRemoteWindowBinder.addChatWindowManager(
                        mPluginChatWindowManager, true);
                IMessenger messenger = IMessenger.Stub
                        .asInterface(mRemoteWindowBinder.getController());
                PluginController.initialize(messenger);
                Logger.d(TAG, "onServiceConnected(), messenger = " + messenger);
                PluginUtils.reloadRcseMessages();
            } catch (Exception e) {
                e.printStackTrace();
            }
        }
    };

    /**
     * This class used to receive broadcasts from RCS Core Service.
     */
    private class ApiReceiver extends BroadcastReceiver {
        /* (non-Javadoc)
         * @see android.content.BroadcastReceiver#onReceive
         * (android.content.Context, android.content.Intent)
         */
        @Override
        public void onReceive(final Context context, final Intent intent) {
            Logger.d(TAG, "API receiver() entry IpMessage");
            if (intent.getAction().equals(
                    PluginUtils.ACTION_SEND_CANCELLED_COMPRESSED_VIDEO)) {
                Logger.d(TAG,
                        "API receiver() ACTION_SEND_CANCELLED_COMPRESSED_VIDEO");
                VideoCompress.cancelCompression();
            }
            new AsyncTask<Void, Void, Void>() {
                @Override
                protected Void doInBackground(Void... params) {
                    try {
                        if (intent
                                .getAction().equals(IpMessageConsts.DisableServiceStatus
                                        .ACTION_DISABLE_SERVICE_STATUS)) {
                            Logger.d(TAG,
                                    "API receiver() entry disable status,  "
                                            + context);
                            Intent it = new Intent();
                            it.setAction(IpMessageConsts.DisableServiceStatus.
                                    ACTION_DISABLE_SERVICE_STATUS);
                            int status = intent.getIntExtra("status", 0);
                            if (status == sDEACTIVATE_JOYN_PERMANENTLY) {
                                mMessageManager.clearAllHistory();
                                PluginUtils.removeMessagesFromMMSDatabase();
                                Logger.d(TAG,
                                        "API receiver() disable permanently");
                                PluginUtils.sJOYN_SERVICE_STATUS = sDEACTIVATE_JOYN_PERMANENTLY;
                            } else if (status == sDEACTIVATE_JOYN_TEMP) {
                                Logger.d(TAG,
                                        "API receiver() disable temporarily");
                                PluginUtils.sJOYN_SERVICE_STATUS = sDEACTIVATE_JOYN_TEMP;
                            }
                            else if(status == sENABLE_JOYN)
                            {
                                PluginUtils.sJOYN_SERVICE_STATUS = sENABLE_JOYN;
                            }
                            Logger.d(TAG, "API receiver() status = ,  "
                                    + status);
                            it.putExtra(IpMessageConsts.STATUS, status);
                            IpNotificationsManager.notify(it);
                        } else if (intent
                                .getAction()
                                .equals(IpMessageConsts.IsOnlyUseXms.ACTION_ONLY_USE_XMS)) {
                            String contact = intent.getStringExtra("contact");
                            Integer failedStatus = intent.getIntExtra(
                                    "failedStatus", 0);
                            if (contact != null && failedStatus != null) {
                                Logger.d(TAG,
                                        "API receiver() entry only use xms when failed = "
                                                + contact + "failedStatus"
                                                + failedStatus);
                                PluginUtils.saveThreadandTag(failedStatus,
                                        contact);
                            }
                            Logger.d(TAG, "API receiver() entry only use xms "
                                    + context);
                            Intent it = new Intent();
                            it.setAction(IpMessageConsts.IsOnlyUseXms.ACTION_ONLY_USE_XMS);
                            int status = intent.getIntExtra("status", 0);
                            Logger.d(TAG, "API receiver() status = ,  "
                                    + status);
                            it.putExtra(IpMessageConsts.STATUS, status);
                            IpNotificationsManager.notify(it);
                        } else if (intent.getAction().equals(
                                PluginUtils.ACTION_FILE_SEND)) {
                            boolean newFile = intent.getBooleanExtra(
                                    "sendFileFromContacts", false);
                            String contact = intent.getStringExtra("contact");
                            String filePath = intent.getStringExtra("filePath");
                            ArrayList<String> filePaths = intent
                                    .getStringArrayListExtra("filePaths");
                            ArrayList<Integer> sizeFiles = intent
                                    .getIntegerArrayListExtra("size");
                            Logger.d(TAG,
                                    "API receiver() entry send file sizes,contact,filepaths are "
                                            + sizeFiles + contact + filePaths);
                            if (newFile && filePaths != null) {
                                for (int i = 0; i < sizeFiles.size(); i++) {
                                    Logger.d(TAG,
                                            "API receiver() new file entry size & path is "
                                                    + sizeFiles.get(i)
                                                    + filePaths.get(i));
                                    IpAttachMessage ipMessage = new IpAttachMessage();
                                    ipMessage.setSize(sizeFiles.get(i));
                                    ipMessage.setPath(filePaths.get(i));
                                    ipMessage.setTo(contact);
                                    mMessageManager.saveIpMsg(ipMessage, 0);
                                }
                            }
                        } else if (intent.getAction().equals(
                                PluginUtils.ACTION_MODE_CHANGE)) {
                            int mode = intent.getIntExtra("mode", 0);
                            Logger.d(TAG,
                                    "API receiver() mode Change , mode = "
                                            + mode);
                            CombineAndSeperateUtil utils = new CombineAndSeperateUtil();
                            if (mode == 1) {
                                Logger.d(TAG,
                                        "API receiver() mode Change , Full Integrated mode");
                                utils.combine(mHostContext);
                            } else {
                                Logger.d(TAG,
                                        "API receiver() mode Change , Converged mode");
                                utils.separate(mHostContext);
                            }
                            PluginUtils.sMessagingUx = -1;
                            PluginUtils.FAILED_MESSAGE_CACHE.evictAll();
                        } else if (intent.getAction().equals(
                                PluginUtils.ACTION_SEND_URL)) {
                            String url = intent
                                    .getStringExtra(PluginApiManager.RcseAction.SHARE_URL);
                            String number = intent
                                    .getStringExtra(PluginApiManager.RcseAction.CONTACT_NUMBER);
                            IpTextMessage ipMessage = new IpTextMessage();
                            ipMessage.setBody(url);
                            ipMessage.setTo(number);
                            mMessageManager.saveIpMsg(ipMessage, 0);
                        } else if (intent.getAction().equals(
                                PluginUtils.ACTION_SEND_RESIZED_FILE)) {
                            boolean compress = intent.getBooleanExtra(
                                    "compressImage", false);
                            String contact = intent.getStringExtra("contact");
                            String filePath = intent.getStringExtra("filePath");
                            ParcelUuid parcelUuid = (ParcelUuid) intent.getParcelableExtra("uuid");
                            boolean compressFail = intent.getBooleanExtra(
                                    "compressFail", false);
                            if (compressFail) {
                                filePath = Utils.compressImage(filePath);
                                mMessageManager.sentFileViaRCSe(filePath, contact, parcelUuid);
                            }
                            mMessageManager.sendResizedFiles(compress);
                        } else if (intent.getAction().equals(
                                PluginUtils.ACTION_DB_CHANGE_RELOAD)) {
                            mMessageManager.getsCacheRcseMessage().clear();
                            Logger.d(TAG, "SIM Change Plugin cache not empty"
                                    + PluginUtils.ACTION_DB_CHANGE_RELOAD);
                            ContentResolver contentResolver = mHostContext
                                    .getContentResolver();
                            contentResolver.delete(Uri.parse("content://sms/"),
                                    "ipmsg_id>0", null);
                            PluginController.obtainMessage(
                                    ChatController.EVENT_RELOAD_NEW_MESSAGE,
                                    "+34667265212").sendToTarget();
                        } else if (intent.getAction().equals(
                                PluginUtils.ACTION_REALOD_FAILED)) {
                            int ipMsgId = intent.getIntExtra("ipMsgId", 0);
                            Logger.d(TAG, "ACTION_REALOD_FAILED() id is"
                                    + ipMsgId);
                            if (ipMsgId > 0) {
                                ContentResolver contentResolver = MediatekFactory
                                        .getApplicationContext()
                                        .getContentResolver();
                                String where = "ipmsg_id=" + ipMsgId;
                                contentResolver.delete(
                                        Uri.parse("content://sms/"), where,
                                        null);
                            } else if (ipMsgId == -1) {
                                mHostContext = null;
                            }
                        } else if (intent.getAction()
                                .equals(IpMessageConsts.JoynGroupInvite.
                                        ACTION_GROUP_IP_INVITATION)) {
                            boolean deletefromMMS = intent.getBooleanExtra(
                                    "removeFromMms", false);
                            if (deletefromMMS) {
                                String contact = intent
                                        .getStringExtra("contact");
                                Logger.d(TAG,
                                        "API receiver() entry only new Group" +
                                        " Invite Delete, contact = "
                                                + contact);
                                MediatekFactory.getApplicationContext().sendBroadcast(intent, "com.mediatek.rcse.permission.RCS_ACCESS_MESSAGE");
                                ContentResolver contentResolver = MediatekFactory
                                        .getApplicationContext()
                                        .getContentResolver();
                                String[] args = { contact };
                                contentResolver.delete(
                                        PluginUtils.SMS_CONTENT_URI,
                                        Sms.ADDRESS + "=?", args);
                            } else {
                                String notifyInfo = intent
                                        .getStringExtra("notify");
                                String contact = intent
                                        .getStringExtra("contact");
                                int messageTag = intent.getIntExtra(
                                        "messageTag", 0);
                                String groupSubject = intent
                                        .getStringExtra("subject");
                                Logger.d(TAG,
                                        "API receiver() entry only new Group Invite");
                                Long messageIdInMms = PluginUtils
                                        .insertDatabase(notifyInfo, contact,
                                                messageTag,
                                                PluginUtils.INBOX_MESSAGE);
                                if (ThreadTranslater.tagExistInCache(contact)) {
                                    Logger.d(TAG,
                                            "plugingroupchatinvitation() Tag exists"
                                                    + contact);
                                    Long thread = ThreadTranslater
                                            .translateTag(contact);
                                    PluginUtils.insertThreadIDInDB(thread,
                                            groupSubject);
                                }
                                ContentValues values = new ContentValues();
                                values.put(
                                        Sms.SEEN,
                                        PluginGroupChatWindow.GROUP_CHAT_INVITATION_SEEN);
                                values.put(Sms.READ, PluginUtils.STATUS_IS_READ);
                                ContentResolver contentResolver = mHostContext
                                        .getContentResolver();
                                contentResolver.update(
                                        PluginUtils.SMS_CONTENT_URI, values,
                                        Sms._ID + " = " + messageIdInMms, null);
                            }
                        }
                    } catch (SQLException e) {
                        // TODO Auto-generated catch block
                        Logger.d(TAG, "API receiver() exception, ");
                        e.printStackTrace();
                    } catch (UnsupportedOperationException ee) {
                        Logger.d(TAG, "API receiver() exception:" + ee);
                    }
                    return null;
                }
            } .execute();
        }
    }

    /**
     * Check whether SIM card is available in the device.
     *
     * @param context the context
     * @return False if no SIM card is available in the device
     */
    private boolean isSimCardAvailable(Context context) {

        Logger.d(TAG, "isSimCardAvailable() entry, context is " + context);

        TelephonyManager manager = (TelephonyManager) context
                .getSystemService(Context.TELEPHONY_SERVICE);

        int state = TelephonyManager.SIM_STATE_ABSENT;

        if (manager == null) {
            Logger.e(TAG, "isSimCardAvailable() entry, manager is " + null);
        } else {
            state = manager.getSimState();
            Logger.d(TAG, "isSimCardAvailable() state is " + state);
        }

        if ((TelephonyManager.SIM_STATE_ABSENT) == state) {
            Logger.d(TAG, "isSimCardAvailable() ,no SIM card found");
            return false;
        } else {
            Logger.w(TAG, "isSimCardAvailable() ,SIM card found");
            return true;
        }
    }
    /**
     * Checks if is actual plugin.
     *
     * @return true, if is actual plugin
     */
    public boolean isActualPlugin() {
        Logger.d(TAG, "isActualPlugin entry ");
        //return true;
        File dbFile = new File(RCS_SETTINGS_PATH);
        if (!dbFile.exists()) {
            Logger.d(TAG, "dbFile not exists ");
            if (RcsSettings.getInstance() != null) {
                RcsSettings.getInstance().setIMSProfileValue(null);
            }
            PluginUtils.setRCSeConfigured(false);
            return false;
        } else {
            if (RcsSettings.getInstance() == null) {
                Logger.d(TAG, "RcsSettings is null");
                RcsSettings.createInstance(MediatekFactory
                        .getApplicationContext());
            }
            String profile = RcsSettings.getInstance().getIMSProfileValue();
            if (profile == null || profile.equals("")) {
                Logger.d(TAG, "Configuration Profile Zero" + profile);
                PluginUtils.setRCSeConfigured(false);
                return false;
            }
        }
        PluginUtils.setRCSeConfigured(true);
        return true;
    }

    @Override
    public void onApiConnectedStatusChanged(boolean isConnected) {
        if (isConnected) {
            Logger.d(TAG, "onApiConnectedStatusChanged(), rebind rcse service");
            bindRcseService(mHostContext);
        } else {
            Logger.d(TAG, "onApiConnectedStatusChanged(), disconnect!");
        }
    }

    @Override
    public void onRcsCoreServiceStatusChanged(int status) {
    }

    @Override
    public void onStatusChanged(boolean status) {
    }
}
