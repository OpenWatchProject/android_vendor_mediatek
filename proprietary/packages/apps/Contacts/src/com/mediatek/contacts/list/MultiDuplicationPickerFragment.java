/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2010. All rights reserved.
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
package com.mediatek.contacts.list;

import android.app.Activity;
import android.content.BroadcastReceiver;
import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.ServiceConnection;
import android.content.res.Configuration;
import android.os.Bundle;
import android.os.Handler;
import android.os.HandlerThread;
import android.os.IBinder;
import android.os.Looper;
import android.os.Message;
import android.provider.ContactsContract.Contacts;
import android.widget.Toast;

import com.android.contacts.R;
import com.android.contacts.list.ContactListFilter;
import com.android.contacts.model.account.AccountWithDataSet;
import com.android.contacts.vcard.ExportVCardActivity;
import com.android.contacts.vcard.VCardCommonArguments;
import com.android.internal.telephony.PhoneConstants;

import com.mediatek.contacts.activities.ContactImportExportActivity;
import com.mediatek.contacts.list.MultiBasePickerAdapter.PickListItemCache;
import com.mediatek.contacts.list.MultiBasePickerAdapter.PickListItemCache.PickListItemData;
import com.mediatek.contacts.list.service.MultiChoiceHandlerListener;
import com.mediatek.contacts.list.service.MultiChoiceRequest;
import com.mediatek.contacts.list.service.MultiChoiceService;
import com.mediatek.contacts.model.account.AccountWithDataSetEx;
import com.mediatek.contacts.simcontact.SimCardUtils;
import com.mediatek.contacts.simservice.SimServiceUtils;
import com.mediatek.contacts.util.AccountTypeUtils;
import com.mediatek.contacts.util.Log;
import com.mediatek.contacts.util.MtkToast;
import com.mediatek.contacts.util.ProgressHandler;

import java.util.ArrayList;
import java.util.List;

public class MultiDuplicationPickerFragment extends MultiBasePickerFragment {
    private static final String TAG = "MultiDuplicationPickerFragment";

    private static final String FROMACCOUNT = "fromaccount";
    private static final String TOACCOUNT = "toaccount";

    private static final int DST_STORE_TYPE_NONE = 0;
    private static final int DST_STORE_TYPE_PHONE = 1;
    private static final int DST_STORE_TYPE_SIM = 2;
    private static final int DST_STORE_TYPE_USIM = 3;
    private static final int DST_STORE_TYPE_STORAGE = 4;
    private static final int DST_STORE_TYPE_ACCOUNT = 5;
    // UIM
    private static final int DST_STORE_TYPE_RUIM = 6;
    private static final int DST_STORE_TYPE_CSIM = 7;
    // UIM
    private int mDstStoreType = DST_STORE_TYPE_NONE;
    // private int mSrcStoreType = DST_STORE_TYPE_NONE;
    private AccountWithDataSet mAccountSrc;
    private AccountWithDataSet mAccountDst;

    private SendRequestHandler mRequestHandler;
    private HandlerThread mHandlerThread;

    private CopyRequestConnection mCopyRequestConnection;

    private List<MultiChoiceRequest> mRequests = new ArrayList<MultiChoiceRequest>();

    private int mRetryCount = 20;

    private int mClickCounter = 1;

    private ProgressHandler mProgressHandler = new ProgressHandler();
    // add for support Dialer to use mtk contactImportExport
    private String mCallingActivityName = null;

    @Override
    public void onCreate(Bundle savedState) {
        super.onCreate(savedState);
        Intent intent = this.getArguments().getParcelable(FRAGMENT_ARGS);

        /** M : bug fixed for CR ALPS00562872 @{ */
        if (intent.getExtras() != null && intent.getExtras().getClassLoader() == null) {
            Log.w(TAG, "[onCreate] The ClassLoader of bundle is null, will reset it");
            intent.setExtrasClassLoader(getClass().getClassLoader());
        }
        /** } */

        mAccountSrc = (AccountWithDataSet) intent.getParcelableExtra(FROMACCOUNT);
        mAccountDst = (AccountWithDataSet) intent.getParcelableExtra(TOACCOUNT);

        mDstStoreType = getStoreType(mAccountDst);
        // mSrcStoreType = getStoreType(mAccountDst);

        /*
         * add for support Dialer to mtk contacts importExport,when export finished in
         * phone and sim or export to storage(internal,sd card),it will jump to
         * callingActivity.@{
         */
        Activity hostActivity = getActivity();
        if (hostActivity == null) {
            Log.e(TAG, "[onCreate] getActivity = null and return");
            return;
        } else {
            mCallingActivityName = hostActivity.getIntent().getExtras().getString(
                    VCardCommonArguments.ARG_CALLING_ACTIVITY);
            Log.i(TAG, "[onCreate] callingActivityName = " + mCallingActivityName);
        }
        //@}

        Log.i(TAG, "[onCreate]Destination store type is: " + storeTypeToString(mDstStoreType));

    }

    @Override
    protected void configureAdapter() {
        super.configureAdapter();
        Log.i(TAG, "[configureAdapter]mAccountSrc.type: " + mAccountSrc.type + ",mAccountSrc.name:"
                + Log.anonymize(mAccountSrc.name));
        ContactListFilter filter = ContactListFilter.createAccountFilter(mAccountSrc.type,
                mAccountSrc.name, null, null);
        super.setListFilter(filter);
    }

    @Override
    public boolean isAccountFilterEnable() {
        return false;
    }

    @Override
    public void onDestroyView() {
        Log.i(TAG, "[onDestroyView]");
        super.onDestroyView();

        mProgressHandler.dismissDialog(getFragmentManager());
    }

    @Override
    public void onConfigurationChanged(Configuration newConfig) {
        super.onConfigurationChanged(newConfig);
    }

    @Override
    public void onOptionAction() {
        // if not item checked, we will do nothing.
        if (getCheckedItemIds().length == 0) {
            Log.w(TAG, "[onOptionAction]length = 0");
            Toast.makeText(getContext(), R.string.multichoice_no_select_alert, Toast.LENGTH_SHORT)
                    .show();
            return;
        }
        Log.i(TAG, "[onOptionAction]mDstStoreType = " + mDstStoreType);
        if (mDstStoreType != DST_STORE_TYPE_STORAGE) {
            if (mClickCounter > 0) {
                mClickCounter--;
            } else {
                Log.w(TAG, "[onOptionAction]Avoid re-entrence");
                return;
            }
        }

        // setDataSetChangedNotifyEnable(false);
        if (mDstStoreType == DST_STORE_TYPE_STORAGE) {
            doExportVCardToSDCard();
        } else {
            startCopyService();

            if (mHandlerThread == null) {
                mHandlerThread = new HandlerThread(TAG);
                mHandlerThread.start();
                mRequestHandler = new SendRequestHandler(mHandlerThread.getLooper());
            }

            final MultiBasePickerAdapter adapter =
                    (MultiBasePickerAdapter) getAdapter();
            final PickListItemCache listItemCacher = adapter.getListItemCache();
            // Bug fix ALPS01651069, if listItemCacher is empty, just return.
            if (listItemCacher.isEmpty()) {
                Log.w(TAG, "[onOptionAction]listItemCacher is empty,return.");
                return;
            }
            final long[] checkedIds = getCheckedItemIds();
            for (long id : checkedIds) {
                PickListItemData item = listItemCacher.getItemData(id);
                mRequests.add(new MultiChoiceRequest(item.contactIndicator, item.simIndex,
                        (int) id, item.displayName));
            }

            // UIM
            if (mDstStoreType == DST_STORE_TYPE_SIM || mDstStoreType == DST_STORE_TYPE_USIM
                    || mDstStoreType == DST_STORE_TYPE_RUIM ||
                    mDstStoreType == DST_STORE_TYPE_CSIM) {
                // Check Radio state
                int subId = ((AccountWithDataSetEx) mAccountDst).getSubId();
                /** M: change for PHB Status Refactoring. @{ */
                if (SimCardUtils.isPhoneBookReady(subId)) {
                    boolean serviceRunning = SimServiceUtils.isServiceRunning(getContext(), subId);
                    Log.i(TAG, "[onOptionAction]AbstractService state is running? "
                            + serviceRunning);
                    if (serviceRunning) {
                        MtkToast.toast(getActivity(), R.string.notifier_fail_copy_title,
                                Toast.LENGTH_SHORT);
                        destroyMyself();
                    } else {
                        mRequestHandler.sendMessage(mRequestHandler.obtainMessage(
                                SendRequestHandler.MSG_REQUEST, mRequests));
                    }
                    /** @} */
                } else {
                    Log.i(TAG, "[onOptionAction] isPhoneBookReady return false.");
                    MtkToast.toast(getActivity(), R.string.notifier_fail_copy_title,
                            Toast.LENGTH_SHORT);
                    destroyMyself();
                }
                /**@}*/
            } else {
                mRequestHandler.sendMessage(mRequestHandler.obtainMessage(
                        SendRequestHandler.MSG_REQUEST, mRequests));
            }
        }
    }

    private static int getStoreType(AccountWithDataSet account) {
        if (account == null) {
            Log.w(TAG, "[getStoreType]account is null.");
            return DST_STORE_TYPE_NONE;
        }

        if (ContactImportExportActivity.STORAGE_ACCOUNT_TYPE.equals(account.type)) {
            return DST_STORE_TYPE_STORAGE;
        } else if (AccountTypeUtils.ACCOUNT_TYPE_LOCAL_PHONE.equals(account.type)) {
            return DST_STORE_TYPE_PHONE;
        } else if (AccountTypeUtils.ACCOUNT_TYPE_SIM.equals(account.type)) {
            return DST_STORE_TYPE_SIM;
        } else if (AccountTypeUtils.ACCOUNT_TYPE_USIM.equals(account.type)) {
            return DST_STORE_TYPE_USIM;
        } else if (AccountTypeUtils.ACCOUNT_TYPE_RUIM.equals(account.type)) {
            return DST_STORE_TYPE_RUIM;
        } else if (AccountTypeUtils.ACCOUNT_TYPE_CSIM.equals(account.type)) {
            return DST_STORE_TYPE_CSIM;
        }
        // / M: UIM

        return DST_STORE_TYPE_ACCOUNT;
    }

    private static String storeTypeToString(int type) {
        switch (type) {
        case DST_STORE_TYPE_NONE:
            return "DST_STORE_TYPE_NONE";
        case DST_STORE_TYPE_PHONE:
            return "DST_STORE_TYPE_PHONE";
        case DST_STORE_TYPE_SIM:
            return "DST_STORE_TYPE_SIM";
        case DST_STORE_TYPE_USIM:
            return "DST_STORE_TYPE_USIM";
        case DST_STORE_TYPE_STORAGE:
            return "DST_STORE_TYPE_STORAGE";
        case DST_STORE_TYPE_ACCOUNT:
            return "DST_STORE_TYPE_ACCOUNT";

            // UIM
        case DST_STORE_TYPE_RUIM:
            return "DST_STORE_TYPE_RUIM";
            // UIM
        case DST_STORE_TYPE_CSIM:
            return "DST_STORE_TYPE_CSIM";
        default:
            return "DST_STORE_TYPE_UNKNOWN";
        }
    }

    private void doExportVCardToSDCard() {
        final MultiBasePickerAdapter adapter =
                (MultiBasePickerAdapter) getAdapter();
        final long[] checkedIds = getCheckedItemIds();
        StringBuilder exportSelection = new StringBuilder();
        exportSelection.append(Contacts._ID + " IN (");

        int curIndex = 0;
        for (long id : checkedIds) {
            if (curIndex++ != 0) {
                exportSelection.append("," + id);
            } else {
                exportSelection.append(id);
            }
        }

        exportSelection.append(")");

        Log.d(TAG, "[doExportVCardToSDCard] exportSelection is " + exportSelection.toString());

        Intent exportIntent = new Intent(getActivity(), ExportVCardActivity.class);
        /* M:fix ALPS00997877 and new feature for Dialer using mtk contactimportExport.
         * The ExportVCardActivity need ARG_CALLING_ACTIVITYSo when user click on the
         * notification after export finished,it will jump to callingActivitiy(PeopleActivity
         * or DialtasActivity) which start ContactImportExportActivity @{
         * */
        Log.i(TAG, "[doExportVCardToSDCard] mCallingActivityName = " + mCallingActivityName);
        exportIntent.putExtra(VCardCommonArguments.ARG_CALLING_ACTIVITY, mCallingActivityName);
        //exportIntent.putExtra(VCardCommonArguments.ARG_CALLING_ACTIVITY,
        //        PeopleActivity.class.getName());
        //@}

        exportIntent.putExtra("exportselection", exportSelection.toString());
        if (mAccountDst instanceof AccountWithDataSet) {
            AccountWithDataSet account = (AccountWithDataSet) mAccountDst;
            exportIntent.putExtra("dest_path", account.dataSet);
        }

        getActivity()
                .startActivityForResult(exportIntent, ContactImportExportActivity.REQUEST_CODE);
    }

    private class CopyRequestConnection implements ServiceConnection {
        private MultiChoiceService mService;

        public boolean sendCopyRequest(final List<MultiChoiceRequest> requests) {
            Log.d(TAG, "[sendCopyRequest]Send an copy request");
            if (mService == null) {
                Log.i(TAG, "[sendCopyRequest]mService is not ready");
                return false;
            }
            mService.handleCopyRequest(requests, new MultiChoiceHandlerListener(mService,
                    mCallingActivityName), mAccountSrc, mAccountDst);
            return true;
        }

        @Override
        public void onServiceConnected(ComponentName name, IBinder binder) {
            Log.d(TAG, "[onServiceConnected]");
            mService = ((MultiChoiceService.MyBinder) binder).getService();
        }

        @Override
        public void onServiceDisconnected(ComponentName name) {
            Log.d(TAG, "[onServiceDisconnected]");
        }
    }

    private class SendRequestHandler extends Handler {

        public static final int MSG_REQUEST = 100;
        public static final int MSG_END = 300;
        public static final int MSG_WAIT_CURSOR_START = 400;
        public static final int MSG_WAIT_CURSOR_END = 500;

        public SendRequestHandler(Looper looper) {
            super(looper);
        }

        @SuppressWarnings("unchecked")
        @Override
        public void handleMessage(Message msg) {
            Log.d(TAG, "[handleMessage]msg.what = " + msg.what);
            if (msg.what == MSG_REQUEST) {
                if (!mCopyRequestConnection.sendCopyRequest((List<MultiChoiceRequest>) msg.obj)) {
                    if (mRetryCount-- > 0) {
                        sendMessageDelayed(obtainMessage(msg.what, msg.obj), 500);
                    } else {
                        sendMessage(obtainMessage(MSG_END));
                    }
                } else {
                    sendMessage(obtainMessage(MSG_END));
                }
                return;
            } else if (msg.what == MSG_END) {
                destroyMyself();
                return;
            } else if (msg.what == MSG_WAIT_CURSOR_START) {
                // Show waiting progress dialog
                mProgressHandler.showDialog(getFragmentManager());
            } else if (msg.what == MSG_WAIT_CURSOR_END) {
                // Dismiss waiting progress dialog
                mProgressHandler.dismissDialog(getFragmentManager());
            }
            super.handleMessage(msg);
        }

    }

    void startCopyService() {
        mCopyRequestConnection = new CopyRequestConnection();

        Log.i(TAG, "[startCopyService]Bind to MultiChoiceService.");
        // We don't want the service finishes itself just after this connection.
        Intent intent = new Intent(this.getActivity(), MultiChoiceService.class);
        getActivity().getApplicationContext().startService(intent);
        getActivity().getApplicationContext().bindService(intent, mCopyRequestConnection,
                Context.BIND_AUTO_CREATE);
    }

    void destroyMyself() {
        Log.d(TAG, "[destroyMyself]");
        if (mHandlerThread != null) {
            mHandlerThread.quit();
            mHandlerThread = null;
        }
        if (getActivity() != null) {
            getActivity().getApplicationContext().unbindService(mCopyRequestConnection);
            getActivity().finish();
        }
    }

    @Override
    public void onDestroy() {
        super.onDestroy();
    }
}
