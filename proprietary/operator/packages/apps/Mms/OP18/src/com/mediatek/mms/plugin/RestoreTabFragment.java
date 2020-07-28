package com.mediatek.op18.mms;

import java.io.File;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;

import android.app.Activity;
import android.app.AlertDialog;
import android.app.Dialog;
import android.app.ProgressDialog;
import android.app.Service;
import android.content.ComponentName;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.ServiceConnection;
import android.os.Bundle;
import android.os.Handler;
import android.os.IBinder;
import android.os.Message;
import android.preference.Preference;
import android.preference.PreferenceCategory;
import android.preference.PreferenceFragment;
import android.preference.PreferenceScreen;
import android.view.Gravity;
import android.view.View;
import android.view.ViewGroup;
import android.view.ViewGroup.LayoutParams;
import android.view.WindowManager;
import android.view.WindowManager.BadTokenException;
import android.widget.ListView;
import android.widget.TextView;
import android.widget.Toast;

import com.mediatek.op18.mms.modules.Composer;
import com.mediatek.op18.mms.modules.MessageRestoreComposer;
import com.mediatek.op18.mms.RestoreService;
import com.mediatek.op18.mms.RestoreService.OnRestoreStatusListener;
import com.mediatek.op18.mms.RestoreService.RestoreBinder;
import com.mediatek.op18.mms.RestoreService.RestoreProgress;
import com.mediatek.op18.mms.ResultDialog.ResultEntity;
import com.mediatek.op18.mms.SDCardReceiver.OnSDCardStatusChangedListener;
import com.mediatek.op18.mms.RestoreCheckBoxPreference;
import com.mediatek.op18.mms.SDCardReceiver;
import com.mediatek.op18.mms.RecordXmlInfo;
import com.mediatek.op18.mms.RecordXmlComposer;
import com.mediatek.op18.mms.RecordXmlParser;
import com.mediatek.op18.mms.utils.BackupFilePreview;
import com.mediatek.op18.mms.utils.BackupFileScanner;
import com.mediatek.op18.mms.utils.Constants;
import com.mediatek.op18.mms.utils.Constants.MessageID;
import com.mediatek.op18.mms.utils.Constants.State;
import com.mediatek.op18.mms.utils.ModuleType;
import com.mediatek.op18.mms.utils.MyLogger;
import com.mediatek.op18.mms.utils.SDCardUtils;
import com.mediatek.op18.mms.utils.Utils;
import com.mediatek.op18.mms.ExitActivity;
import com.mediatek.op18.mms.R;

import java.io.IOException;

/**
 * @author mtk81330
 *
 */
public class RestoreTabFragment extends PreferenceFragment {

    private static final String CLASS_TAG = MyLogger.LOG_TAG + "/RestoreTabFragment";
    private static final int START_ACTION_MODE_DELAY_TIME = 100;
    private static final String STATE_DELETE_MODE = "deleteMode";
    private static final String STATE_CHECKED_ITEMS = "checkedItems";

    private ListView mListView;
    private BackupFileScanner mFileScanner;
    private Handler mHandler;
    private RestoreBinder mRestoreService;
    private ProgressDialog mProgressDialog;
    private boolean mIsStoped = false;
    private boolean mNeedUpdateResult = false;
    private boolean mIsCheckedRestoreStatus = false;
    BackupFilePreview mPreview = null;
    private File mFile;
    private String mRestoreFolderPath;
    private boolean mIsDataInitialed;
    OnRestoreStatusListener mRestoreListener;
    private boolean mIsStopped = false;
    private boolean mIsDestroyed = false;
    //private FilePreviewTask mPreviewTask;
    private PersonalDataRestoreStatusListener mRestoreStoreStatusListener;
    private RestoreCheckBoxPreference clicked_item;
    private AlertDialog mRestoreConfirmDialog = null;

    /**
     * @return Handler
     */
    public Handler getmHandler() {
        return mHandler;
    }
    private ProgressDialog mLoadingDialog;
    //private ActionMode mDeleteActionMode;
    //private DeleteActionMode mActionModeListener;
    OnSDCardStatusChangedListener mSDCardListener;
    private boolean mIsActive = false;

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        MyLogger.logI(CLASS_TAG, "RestoreTabFragment: onCreate");
        addPreferencesFromResource(R.xml.restore_tab_preference);
        initHandler();
        createProgressDlg();
        bindService();
    }

    private void bindService() {
        getActivity().getApplicationContext().bindService
        (new Intent(getActivity(), RestoreService.class), mServiceCon,
                Service.BIND_AUTO_CREATE);
    }

    ServiceConnection mServiceCon = new ServiceConnection() {

        public void onServiceConnected(ComponentName name, IBinder service) {
            mRestoreService = (RestoreBinder) service;
            if (mRestoreService != null) {
                mRestoreService.setOnRestoreChangedListner(mRestoreListener);
                afterServiceConnected();
            }
            MyLogger.logI(CLASS_TAG, " onServiceConnected");
        }

        public void onServiceDisconnected(ComponentName name) {
            mRestoreService = null;
            MyLogger.logI(CLASS_TAG, " onServiceDisconnected");
        }
    };

    private void afterServiceConnected() {
        MyLogger.logD(CLASS_TAG, "afterServiceConnected, to checkRestorestate");
        checkRestoreState();
    }

    private void checkRestoreState() {
        if (mIsCheckedRestoreStatus) {
            MyLogger.logD(CLASS_TAG, "can not checkRestoreState, as it has been checked");
            return;
        }
        if (!mIsDataInitialed) {
            MyLogger.logD(CLASS_TAG, "can not checkRestoreState, wait data to initialed");
            return;
        }
        MyLogger.logD(CLASS_TAG, "all ready. to checkRestoreState");
        mIsCheckedRestoreStatus = true;
        if (mRestoreService != null) {
            int state = mRestoreService.getState();
            MyLogger.logD(CLASS_TAG, "checkRestoreState: state = " + state);
            if (state == State.RUNNING || state == State.PAUSE) {

                RestoreProgress p = mRestoreService.getCurRestoreProgress();
                if (p.mType == 0 || p.mMax == 0) {
                    MyLogger.logE(CLASS_TAG, "CurrentProgress is not availableMax = " + p.mMax
                            + " curprogress = " + p.mCurNum);
                    mRestoreService.reset();
                    return;
                }
                MyLogger.logD(CLASS_TAG, "checkRestoreState: Max = " + p.mMax + " curprogress = "
                        + p.mCurNum);
                String msg = getProgressDlgMessage(p.mType);

                if (state == State.RUNNING) {
                    showProgressDialog();
                }
                MyLogger.logD(CLASS_TAG, "Anuj max checkRestoreState " + p.mMax);
                setProgressDialogMax(p.mMax);
                setProgressDialogProgress(p.mCurNum);
                setProgressDialogMessage(msg);
            } else if (state == State.FINISH) {
                if (mIsStoped) {
                    mNeedUpdateResult = true;
                } else {
                    showRestoreResult(mRestoreService.getRestoreResult());
                }
            }
        }
    }

    protected void showProgressDialog() {
        if (mProgressDialog == null) {
            mProgressDialog = createProgressDlg();
        }
        if (!getActivity().isFinishing()) {
            try {
                mProgressDialog.show();
                doKeepDialog(mProgressDialog);
            } catch (BadTokenException e) {
                MyLogger.logE(CLASS_TAG, " BadTokenException :" + e.toString());
            }
        }
    }


    private String getProgressDlgMessage(int type) {
        StringBuilder builder = new StringBuilder(getString(R.string.restoring));

        builder.append("(").append
        (ModuleType.getModuleStringFromType(getActivity(), type)).append(")");
        return builder.toString();
    }

    private void showRestoreResult(ArrayList<ResultEntity> list) {
        MyLogger.logD(CLASS_TAG, "showRestoreResult 1");
        dismissProgressDialog();
        Bundle args = new Bundle();
        args.putParcelableArrayList("result", list);
        try {
            //showDialog(DialogID.DLG_RESULT, args);
            if (mRestoreService != null) {
                mRestoreService.reset();
            }
            stopService();
            if (!SDCardUtils.isSdCardMissingOrFull(getActivity()))
            Toast.makeText(getActivity(), R.string.restore_done,
                    Toast.LENGTH_SHORT).show();
            startScanFiles(getActivity(), false);
            //clicked_item.getTextView().invalidate();

        } catch (BadTokenException e) {
            MyLogger.logE(CLASS_TAG, "BadTokenException");
        }
    }

    private void dismissProgressDialog() {
        MyLogger.logD(CLASS_TAG, "dismissProgressDialog 1");
        if (mProgressDialog != null && mProgressDialog.isShowing()) {
            MyLogger.logD(CLASS_TAG, "dismissProgressDialog 2");
            mProgressDialog.dismiss();
        }
    }


    private void setProgressDialogProgress(int value) {
        if (mProgressDialog == null) {
            mProgressDialog = createProgressDlg();
        }
        mProgressDialog.setProgress(value);
    }

    private void setProgressDialogMessage(CharSequence message) {
        if (mProgressDialog == null) {
            mProgressDialog = createProgressDlg();
        }
        mProgressDialog.setMessage(message);
    }

    private void setProgressDialogMax(int max) {
        if (mProgressDialog == null) {
            mProgressDialog = createProgressDlg();
        }
        mProgressDialog.setMax(max);
    }

    private ProgressDialog createProgressDlg() {
        if (mProgressDialog == null) {
            mProgressDialog = new ProgressDialog(getActivity());
            if (mProgressDialog == null)
                return null;
            mProgressDialog.setProgressStyle(ProgressDialog.STYLE_HORIZONTAL);
            //mProgressDialog.setProgressNumberFormat(null);
            //mProgressDialog.setProgressPercentFormat(null);
            mProgressDialog.setMessage(getString(R.string.restoring));
            mProgressDialog.setCancelable(false);
        }
        return mProgressDialog;
    }

    @Override
    public void onAttach(Activity activity) {
        super.onAttach(activity);
        MyLogger.logI(CLASS_TAG, "RestoreTabFragment: onAttach");
    }

    @Override
    public void onActivityCreated(final Bundle savedInstanceState) {
        super.onActivityCreated(savedInstanceState);
        MyLogger.logI(CLASS_TAG, "RestoreTabFragment: onActivityCreated");
        init();
        /*if (savedInstanceState != null) {
            boolean isActionMode = savedInstanceState.getBoolean(STATE_DELETE_MODE, false);
            if (isActionMode) {
                mHandler.postDelayed(new Runnable() {
                    @Override
                    public void run() {
                        if (getActivity() == null) {
                            return;
                        }
                        //mDeleteActionMode = getActivity().startActionMode(mActionModeListener);
                        //mActionModeListener.restoreState(savedInstanceState);
                    }
                }, START_ACTION_MODE_DELAY_TIME);
            }
        }*/
    }

    @Override
    public void onDestroy() {
        super.onDestroy();

        //mHandler = null;
        /*if (null != mPreviewTask) {
            boolean result = mPreviewTask.cancel(true);
            MyLogger.logD(CLASS_TAG, "onDestory result : " + result);
        }*/
        MyLogger.logD(CLASS_TAG, "onDestroy");


        MyLogger.logI(CLASS_TAG, "RestoreTabFragment: onDestroy");
        if (mLoadingDialog != null) {
            mLoadingDialog.dismiss();
            mLoadingDialog = null;
        }
        if (mFileScanner != null) {
            mFileScanner.setHandler(null);
        }
        unRegisteSDCardListener();

        MyLogger.logI(CLASS_TAG, "Anuj mRestoreService.getState = " + mRestoreService.getState()
         + "mRestoreService = " + mRestoreService);

        if (mRestoreService != null && mRestoreService.getState() == State.RUNNING
         && getActivity().isFinishing()) {
            MyLogger.logD(CLASS_TAG, "Anuj inside show toast activity = " + getActivity());
            Toast.makeText(getActivity().getApplicationContext(), R.string.restore_stopped,
                                        Toast.LENGTH_SHORT).show();
            stopService();
            MyLogger.logD(CLASS_TAG, "Anuj inside show toast");

        }

        // when startService when to Restore and stopService when onDestroy if
        // the service in IDLE
        if (mRestoreService != null && mRestoreService.getState() == State.INIT) {
            stopService();
        }


        if (getActivity().isFinishing()) {
            MyLogger.logD(CLASS_TAG, "Restore fragment actual finish");
            unBindService();
            mHandler = null;
            // set listener to null avoid some special case when restart after
            // configure changed
            if (mRestoreService != null) {
                mRestoreService.setOnRestoreChangedListner(null);
            }
            mIsDestroyed = true;
            ExitActivity.exitApplication(getActivity());
        }

    }


    private void unBindService() {
        if (mRestoreService != null) {
            mRestoreService.setOnRestoreChangedListner(null);
        }
        getActivity().getApplicationContext().unbindService(mServiceCon);
    }

    /**
     * onPause.
     */
    public void onPause() {
        super.onPause();
        MyLogger.logI(CLASS_TAG, "RestoreTabFragment: onPasue");
        if (mFileScanner != null) {
            mFileScanner.quitScan();
        }
        mIsActive = false;
        //getPreferenceManager().getSharedPreferences().
        //unregisterOnSharedPreferenceChangeListener(getActivity());

    }

    @Override
    public void onResume() {
        super.onResume();
        MyLogger.logI(CLASS_TAG, "RestoreTabFragment: onResume");
        mIsActive = true;
        startScanFiles(getActivity(), true);
        //getPreferenceManager().getSharedPreferences().
        //registerOnSharedPreferenceChangeListener(getActivity());

    }

    @Override
    public void onDetach() {
        super.onDetach();
        MyLogger.logI(CLASS_TAG, "RestoreTabFragment: onDetach");
    }

    /**
     * @param outState state
     */
    public void onSaveInstanceState(final Bundle outState) {
        super.onSaveInstanceState(outState);
        /*if (mDeleteActionMode != null) {
            outState.putBoolean(STATE_DELETE_MODE, true);
            mActionModeListener.saveState(outState);
        }*/
    }

    private static void doKeepDialog(Dialog dialog) {
        WindowManager.LayoutParams lp = new WindowManager.LayoutParams();
        lp.copyFrom(dialog.getWindow().getAttributes());
        lp.width = WindowManager.LayoutParams.WRAP_CONTENT;
        lp.height = WindowManager.LayoutParams.WRAP_CONTENT;
        dialog.getWindow().setAttributes(lp);
    }


    @Override
    public boolean onPreferenceTreeClick(PreferenceScreen preferenceScreen, Preference preference) {
        super.onPreferenceTreeClick(preferenceScreen, preference);
        if (SDCardUtils.getExternalStoragePath(getActivity()) == null) {
            Utils.exitLockTaskModeIfNeeded(getActivity());
            getActivity().finish();
            return true;
        }
        if (preference instanceof RestoreCheckBoxPreference) {
            RestoreCheckBoxPreference p = (RestoreCheckBoxPreference) preference;
            //if (mDeleteActionMode == null) {
                Intent intent = p.getIntent();
                String fileName = intent.getStringExtra(Constants.FILENAME);
                mFile = new File(fileName);
                if (mFile.exists()) {
                    if (mRestoreConfirmDialog != null)
                        mRestoreConfirmDialog.dismiss();
                    mRestoreConfirmDialog = createConfirmDialog();
                    mRestoreConfirmDialog.setCanceledOnTouchOutside(false);
                    mRestoreConfirmDialog.show();
                    doKeepDialog(mRestoreConfirmDialog);
                    clicked_item = p;
                    //startActivity(intent);
                } else {
                    Toast.makeText(getActivity(), R.string.file_no_exist_and_update,
                            Toast.LENGTH_SHORT).show();
                }
            /*} else if (mActionModeListener != null) {
                mActionModeListener.setItemChecked(p, !p.isChecked());
            }*/
        }
        return true;
    }

    private AlertDialog createConfirmDialog() {
        final AlertDialog dialog = new AlertDialog.Builder(getActivity())
                .setTitle(R.string.notice).setMessage(R.string.restore_confirm_notice)
                .setNegativeButton(android.R.string.cancel, null)
                .setPositiveButton(android.R.string.ok, new DialogInterface.OnClickListener() {
                    public void onClick(DialogInterface dialog, int whichButton) {
                        MyLogger.logI(CLASS_TAG, "start to Restore");
                        startRestore();
                    }
                }).setCancelable(true).create();
        return dialog;
    }

    private void startRestore() {
        if (!isCanStartRestore()) {
            Toast.makeText(getActivity(), getString(R.string.cannot_start_restore),
             Toast.LENGTH_SHORT).show();
            return;
        }
        startService();
        MyLogger.logD(CLASS_TAG, "startRestore");
        ArrayList<Integer> list = new ArrayList<Integer>();
        list.add(ModuleType.TYPE_MESSAGE);
        if (list.size() == 0) {
            Toast.makeText(getActivity(), getString(R.string.no_item_selected),
             Toast.LENGTH_SHORT).show();
            return;
        }
        mRestoreService.setRestoreModelList(list);
        mRestoreFolderPath = mFile.getAbsolutePath();
        boolean ret = mRestoreService.startRestore(mRestoreFolderPath);
        if (ret) {
            String path = SDCardUtils.getStoragePath(getActivity());
            if (path == null) {
                // no sdcard
                MyLogger.logD(CLASS_TAG, "SDCard is removed");
                return;
            }

            showProgressDialog();
            String msg = getProgressDlgMessage(list.get(0));
            setProgressDialogMessage(msg);
            setProgressDialogProgress(0);
            getActivity().setProgress(0);

            Composer composer = null;
            composer = new MessageRestoreComposer(getActivity());
            if (composer != null) {
                MyLogger.logD(CLASS_TAG, "Anuj composer: mRestoreFolderPath = "
                 + mRestoreFolderPath);
                composer.setParentFolderPath(mRestoreFolderPath);
                composer.init();
                int count = composer.getCount();
                MyLogger.logD(CLASS_TAG, "Anuj composer: count = " + count);
                //mNumberMap.put(type, count);
                setProgressDialogMax(count);
            }
            //RestoreProgress p = mRestoreService.getCurRestoreProgress();
            //MyLogger.logI(CLASS_TAG, "Anuj showProgressDialog p.mMax"
            // + p.mMax + "mPreview = "+mPreview);
            //setProgressDialogMax(p.mMax);

            /*int count = mPreview.getItemCount(list.get(0));
            MyLogger.logD(CLASS_TAG, "Anuj max count = "+count);
            setProgressDialogMax(count);*/


            if (mPreview != null) {
                int count1 = mPreview.getItemCount(list.get(0));
                MyLogger.logD(CLASS_TAG, "Anuj max count = " + count1);
                setProgressDialogMax(count1);
            }
        } else {
            stopService();
        }
    }

    private boolean isCanStartRestore() {
        if (mRestoreService == null) {
            MyLogger.logE(CLASS_TAG, "isCanStartRestore(): mRestoreService is null");
            return false;
        }

        if (mRestoreService.getState() != State.INIT) {
            MyLogger.logE(CLASS_TAG,
                    "isCanStartRestore(): Can not to start Restore. Restore Service state is "
                        + mRestoreService.getState());
            return false;
        }
        return true;
    }


    private void startService() {
        getActivity().startService(new Intent(getActivity(), RestoreService.class));
    }

    private void stopService() {
        if (mRestoreService != null) {
            mRestoreService.reset();
        }
        getActivity().stopService(new Intent(getActivity(), RestoreService.class));
    }


    @Override
    public void onStart() {
        super.onStart();
        MyLogger.logI(CLASS_TAG, "RestoreTabFragment: onStart");
        mIsStopped = false;
        if (mRestoreStoreStatusListener == null) {
            MyLogger.logI(CLASS_TAG, "RestoreTabFragment: onStart --> new restore status listener");
            mRestoreStoreStatusListener = new PersonalDataRestoreStatusListener();
        }
        setOnRestoreStatusListener(mRestoreStoreStatusListener);
        mIsDataInitialed = true;
        MyLogger.logD(CLASS_TAG, "mIsDataInitialed is ok");
        checkRestoreState();

    }

    @Override
    public void onStop() {
        super.onStop();
        MyLogger.logD(CLASS_TAG, "onStop()");
        mIsStopped = true;
        // This show the toast when switch tab on SD full (seems no need)
        //SDCardUtils.broadcastSDRemovedOrFullIfNecessary(getActivity());
    }


    private void setOnRestoreStatusListener(OnRestoreStatusListener listener) {
        mRestoreListener = listener;
        if (mRestoreService != null) {
            MyLogger.logD(CLASS_TAG, "setOnRestoreStatusListener");
            mRestoreService.setOnRestoreChangedListner(mRestoreListener);
        }
    }

    private void init() {
        initHandler();
        initListView(getView());
        initLoadingDialog();
        registerSDCardListener();
    }

    private void unRegisteSDCardListener() {
        if (mSDCardListener != null) {
            SDCardReceiver receiver = SDCardReceiver.getInstance();
            receiver.unRegisterOnSDCardChangedListener(mSDCardListener);
        }
    }

    private void registerSDCardListener() {
        mSDCardListener = new OnSDCardStatusChangedListener() {
            @Override
            public void onSDCardStatusChanged(final boolean mount) {
                MyLogger.logD(CLASS_TAG,
                 "Anuj onSDCardStatusChanged from RestoreTabFragment mount = "
                  + mount + "mIsActive = " + mIsActive);
                if (mIsActive) {
                    if (!mount) {
                        mHandler.post(new Runnable() {
                            @Override
                            public void run() {

                                PreferenceScreen ps = getPreferenceScreen();
                                ps.removeAll();
                                // fix bug for ALPS01895396 start
                                try {
                                    Toast.makeText(getActivity(),
                                            R.string.nosdcard_notice,
                                            Toast.LENGTH_SHORT).show();
                                } catch (NullPointerException e) {
                                    e.printStackTrace();
                                }
                                // end
                            }
                        });
                        Utils.exitLockTaskModeIfNeeded(getActivity());
                        getActivity().finish();
                    }
                }
            }
        };

        SDCardReceiver receiver = SDCardReceiver.getInstance();
        receiver.registerOnSDCardChangedListener(mSDCardListener);
    }

    private void initLoadingDialog() {
        mLoadingDialog = new ProgressDialog(getActivity());
        mLoadingDialog.setCancelable(false);
        mLoadingDialog.setMessage(getString(R.string.loading_please_wait));
        mLoadingDialog.setIndeterminate(true);
    }
    private TextView mEmptyView;
    private void initListView(View root) {
        View view = root.findViewById(android.R.id.list);
        if (view != null && view instanceof ListView) {
            mListView = (ListView) view;
            //mActionModeListener = new DeleteActionMode();
            /*mListView.setOnItemLongClickListener(new OnItemLongClickListener() {
                @Override
                public boolean onItemLongClick(AdapterView<?> listView, View view, int position,
                        long id) {
                    mDeleteActionMode = getActivity().startActionMode(mActionModeListener);
                    showCheckBox(true);
                    mActionModeListener.onPreferenceItemClick(getPreferenceScreen(), position);
                    return true;
                }
            });*/
        }
        mEmptyView = new TextView(getActivity());
        mEmptyView.setLayoutParams(new LayoutParams(LayoutParams.FILL_PARENT,
                LayoutParams.FILL_PARENT));
        mEmptyView.setGravity(Gravity.CENTER);
        mEmptyView.setVisibility(View.GONE);
        ((ViewGroup) getListView().getParent()).addView(mEmptyView);
        getListView().setEmptyView(mEmptyView);
    }

    /*@SuppressWarnings("unchecked")
    private void startDeleteItems(final HashSet<String> deleteItemIds) {
        PreferenceScreen ps = getPreferenceScreen();
        int count = ps.getPreferenceCount();
        HashSet<File> files = new HashSet<File>();
        for (int position = 0; position < count; position++) {
            Preference preference = ps.getPreference(position);
            if (preference != null && preference instanceof RestoreCheckBoxPreference) {
                RestoreCheckBoxPreference p = (RestoreCheckBoxPreference) preference;
                String key = p.getKey();
                if (deleteItemIds.contains(key)) {
                    files.add(p.getAccociateFile());
                }
            }
        }
        DeleteCheckItemTask deleteTask = new DeleteCheckItemTask();
        deleteTask.execute(files);
        ps.removeAll();
    }*/

    private void initHandler() {
        mHandler = new Handler() {
            @Override
            public void handleMessage(final Message msg) {
                switch (msg.what) {

                case MessageID.SCANNER_FINISH:
                    MyLogger.logD(CLASS_TAG, "recieve MessageID.SCANNER_FINISH");
                    if (getActivity() != null) {
                        if (msg.obj == null) {
                            Toast.makeText(
                                    getActivity(),
                                    R.string.toast_sd_removed_or_full,
                                    Toast.LENGTH_SHORT).show();
                        }
                        addScanResultsAsPreferences(msg.obj);
                    }
                    MyLogger.logD(CLASS_TAG, "mLoadingDialog != null: "
                            + (mLoadingDialog != null));
                    if (mLoadingDialog != null) {
                        mLoadingDialog.cancel();
                    }
                    break;

                default:
                    break;
                }
            }
        };
    }

    private void startScanFiles(Context context, boolean showLoading) {
        if (showLoading) {
            if (mLoadingDialog == null) {
                initLoadingDialog();
            }
            mLoadingDialog.show();
        }

        if (mFileScanner == null) {
            mFileScanner = new BackupFileScanner(context, mHandler);
        } else {
            mFileScanner.setHandler(mHandler);
        }
        MyLogger.logI(CLASS_TAG, "RestoreTabFragment: startScanFiles");
        mFileScanner.startScan();
    }

    @SuppressWarnings("unchecked")
    private void addScanResultsAsPreferences(Object obj) {

        PreferenceScreen ps = getPreferenceScreen();

        // clear the old items last scan
        ps.removeAll();

        if (obj == null) {
            MyLogger.logD(CLASS_TAG, "addScanResultsAsPreferences obj = " + obj);
            return;
        }

        HashMap<String, List<BackupFilePreview>> map =
                (HashMap<String, List<BackupFilePreview>>) obj;
        Preference noDataPreference = new Preference(getActivity());
        noDataPreference.setTitle("No Record");
        boolean noRecord = true;
        // personal data
        List<BackupFilePreview> items = map.get(Constants.SCAN_RESULT_KEY_PERSONAL_DATA);
        if (items != null && !items.isEmpty()) {
            noRecord = false;
            addPreferenceCategory(ps, R.string.backup_messages_history);
            for (BackupFilePreview item : items) {
                addRestoreCheckBoxPreference(ps, item, "personal data");
            }
        }

        // app data
        /*items = map.get(Constants.SCAN_RESULT_KEY_APP_DATA);
        if (items != null && !items.isEmpty()) {
            addPreferenceCategory(ps, R.string.backup_app_data_history);
            noRecord = false;
            for (BackupFilePreview item : items) {
                addRestoreCheckBoxPreference(ps, item, "app");
            }
        }*/
        if (noRecord) {
            MyLogger.logD(CLASS_TAG, "getActivity = " + getActivity());
            if (mEmptyView != null) {
                mEmptyView.setText(R.string.no_data);
            }
            // ps.addPreference(noDataPreference);
        }

        /*if (mDeleteActionMode != null && mActionModeListener != null) {
            MyLogger.logD(CLASS_TAG, " confirmSyncCheckedPositons now!!!");
            mActionModeListener.confirmSyncCheckedPositons();
        }*/
    }

    private void addPreferenceCategory(PreferenceScreen ps, int titleId) {
        PreferenceCategory category = new PreferenceCategory(getActivity());
        category.setTitle(titleId);
        ps.addPreference(category);
    }

    private void addRestoreCheckBoxPreference(PreferenceScreen ps, BackupFilePreview item,
            String type) {
        if (item == null || type == null) {
            MyLogger.logE(CLASS_TAG, "addRestoreCheckBoxPreference: Error!");
            return;
        }
        RestoreCheckBoxPreference preference = new RestoreCheckBoxPreference(getActivity());
        //if (type.equals("app")) {
            //preference.setTitle(R.string.backup_app_data_preference_title);
        //} else {
            String fileName = item.getFileName();
            preference.setTitle(fileName);
        //}
        MyLogger.logI(CLASS_TAG, "addRestoreCheckBoxPreference: type is " + type + " fileName = "
                + item.getFileName());
        StringBuilder builder = new StringBuilder(getString(R.string.click_to_restore));
        builder.append(" ");
        //builder.append(FileUtils.getDisplaySize(item.getFileSize(), getActivity()));
        preference.setRestored(item.isRestored());
        preference.setSummary(builder.toString());
        /*if (mDeleteActionMode != null) {
            preference.showCheckbox(true);
        }*/
        preference.setAccociateFile(item.getFile());

        Intent intent = new Intent();
        //if (type.equals("app")) {
            //intent.setClass(getActivity(), AppRestoreActivity.class);
        //} else {
            //todo
            //intent.setClass(getActivity(), PersonalDataRestoreActivity.class);
        //}
        intent.putExtra(Constants.FILENAME, item.getFile().getAbsolutePath());
        preference.setIntent(intent);
        ps.addPreference(preference);
    }

    /*private void showCheckBox(boolean bShow) {
        PreferenceScreen ps = getPreferenceScreen();
        int count = ps.getPreferenceCount();
        for (int position = 0; position < count; position++) {
            Preference p = ps.getPreference(position);
            if (p instanceof RestoreCheckBoxPreference) {
                ((RestoreCheckBoxPreference) p).showCheckbox(bShow);
            }
        }
    }*/

    /**
     * @author mtk81330
     *
     */
    /*class DeleteActionMode implements ActionMode.Callback {

        private int mCheckedCount;
        private HashSet<String> mCheckedItemIds;
        private ActionMode mMode;

        @Override
        public boolean onActionItemClicked(ActionMode mode, MenuItem item) {
            switch (item.getItemId()) {
            case R.id.select_all:
                setAllItemChecked(true);
                break;

            case R.id.cancel_select:
                setAllItemChecked(false);
                break;

            case R.id.delete:
                if (mCheckedCount == 0) {
                    Toast.makeText(getActivity(), R.string.no_item_selected, Toast.LENGTH_SHORT)
                            .show();
                } else {
                    startDeleteItems(mCheckedItemIds);
                    mode.finish();
                }
                break;

            default:
                break;
            }
            return false;
        }

        @Override
        public boolean onCreateActionMode(ActionMode mode, Menu menu) {
            mMode = mode;
            mListView.setLongClickable(false);
            MenuInflater inflater = getActivity().getMenuInflater();
            inflater.inflate(R.menu.multi_select_menu, menu);
            mCheckedItemIds = new HashSet<String>();
            setAllItemChecked(false);
            showCheckBox(true);
            return true;
        }

        @Override
        public void onDestroyActionMode(ActionMode mode) {
            mCheckedItemIds = null;
            mCheckedCount = 0;
            mDeleteActionMode = null;
            mListView.setLongClickable(true);
            showCheckBox(false);
        }

        @Override
        public boolean onPrepareActionMode(ActionMode mode, Menu menu) {
            return false;
        }

        private void updateTitle() {
            StringBuilder builder = new StringBuilder();
            builder.append(mCheckedCount);
            builder.append(" ");
            builder.append(getString(R.string.selected));
            mMode.setTitle(builder.toString());
            MyLogger.logI(CLASS_TAG,
                    "updateTitle() titleString : " + builder.toString());
        }

        public void onPreferenceItemClick(PreferenceScreen ps, final int position) {
            Preference preference = ps.getPreference(position);
            if (preference instanceof RestoreCheckBoxPreference) {
                RestoreCheckBoxPreference p = (RestoreCheckBoxPreference) preference;
                boolean toChecked = !p.isChecked();
                p.setChecked(toChecked);
                String key = p.getAccociateFile().getAbsolutePath();
                if (toChecked) {
                    mCheckedItemIds.add(key);
                    mCheckedCount++;
                } else {
                    mCheckedItemIds.remove(key);
                    mCheckedCount--;
                }
                updateTitle();
            }
        }

        public void setItemChecked(final RestoreCheckBoxPreference p, final boolean checked) {
            if (p.isChecked() != checked) {
                p.setChecked(checked);
                String key = p.getKey();
                if (checked) {
                    mCheckedItemIds.add(key);
                    mCheckedCount++;
                } else {
                    mCheckedItemIds.remove(key);
                    mCheckedCount--;
                }
            }
            updateTitle();
        }

        private void setAllItemChecked(boolean checked) {
            PreferenceScreen ps = getPreferenceScreen();

            mCheckedCount = 0;
            mCheckedItemIds.clear();
            int count = ps.getPreferenceCount();
            for (int position = 0; position < count; position++) {
                Preference preference = ps.getPreference(position);
                if (preference instanceof RestoreCheckBoxPreference) {
                    RestoreCheckBoxPreference p = (RestoreCheckBoxPreference) preference;
                    p.setChecked(checked);
                    if (checked) {
                        mCheckedItemIds.add(p.getAccociateFile().getAbsolutePath());
                        mCheckedCount++;
                    }
                }
            }
            updateTitle();
        }

        /**
         * @return
         * after refreshed, must sync witch mCheckedItemIds;
         */
        /*public void confirmSyncCheckedPositons() {
            mCheckedCount = 0;

            HashSet<String> tempCheckedIds = new HashSet<String>();
            PreferenceScreen ps = getPreferenceScreen();
            int count = ps.getPreferenceCount();
            for (int position = 0; position < count; position++) {
                Preference preference = ps.getPreference(position);
                if (preference instanceof RestoreCheckBoxPreference) {
                    RestoreCheckBoxPreference p = (RestoreCheckBoxPreference) preference;
                    String key = p.getAccociateFile().getAbsolutePath();
                    if (mCheckedItemIds.contains(key)) {
                        tempCheckedIds.add(key);
                        p.setChecked(true);
                        mCheckedCount++;
                    }
                }
            }
            mCheckedItemIds.clear();
            mCheckedItemIds = tempCheckedIds;
            updateTitle();
        }*/

        /*public void saveState(final Bundle outState) {
            ArrayList<String> list = new ArrayList<String>();
            for (String item : mCheckedItemIds) {
                list.add(item);
            }
            outState.putStringArrayList(STATE_CHECKED_ITEMS, list);
        }*/

        /*public void restoreState(Bundle state) {
            ArrayList<String> list = state.getStringArrayList(STATE_CHECKED_ITEMS);
            if (list != null && !list.isEmpty()) {
                for (String item : list) {
                    mCheckedItemIds.add(item);
                }
            }
            PreferenceScreen ps = getPreferenceScreen();
            if (ps.getPreferenceCount() > 0) {
                confirmSyncCheckedPositons();
            }
        }*/

    public class PersonalDataRestoreStatusListener implements OnRestoreStatusListener {

        /**
         * @param type type
         * @param max
         */
        public void onComposerChanged(final int type, final int max) {
            MyLogger.logI(CLASS_TAG, "RestoreDetailActivity: onComposerChanged type = "
             + type + "max = " + max);

            if (mHandler != null) {
                mHandler.post(new Runnable() {

                    public void run() {
                        String msg = getProgressDlgMessage(type);
                        setProgressDialogMessage(msg);
                        MyLogger.logI(CLASS_TAG, "Anuj max onComposerChanged = " + max);
                        setProgressDialogMax(max);
                        setProgressDialogProgress(0);
                    }
                });
            }
        }

        /**
         * @param composer source composer of the progress change
         * @param progress new progress
         */
        public void onProgressChanged(Composer composer, final int progress) {
            MyLogger.logI(CLASS_TAG, "onProgressChange, p = " + progress);
            if (mHandler != null && !mIsStopped) {
                mHandler.post(new Runnable() {
                    public void run() {
                        if (mProgressDialog != null && !mIsStopped) {
                            MyLogger.logI(CLASS_TAG, "onProgressChange, setProgress = " + progress);
                            mProgressDialog.setProgress(progress);
                        }
                    }
                });
            } else {
                MyLogger.logI(CLASS_TAG, "onProgressChange, mHandler is null");
            }
        }

        /**
         * @param bSuccess if success
         * @param resultRecord result
         */
        public void onRestoreEnd(boolean bSuccess, final ArrayList<ResultEntity> resultRecord) {
            final ArrayList<ResultEntity> iResultRecord = resultRecord;
            MyLogger.logD(CLASS_TAG, "onRestoreEnd");
            boolean hasSuccess = false;
            for (ResultEntity result : resultRecord) {
                if (ResultEntity.SUCCESS == result.getResult()) {
                    hasSuccess = true;
                    break;
                }
            }

            if (hasSuccess) {
                String recrodXmlFile = mRestoreFolderPath + File.separator + Constants.RECORD_XML;
                String content = Utils.readFromFile(recrodXmlFile);
                ArrayList<RecordXmlInfo> recordList = new ArrayList<RecordXmlInfo>();
                if (content != null) {
                    recordList = RecordXmlParser.parse(content.toString());
                }
                RecordXmlComposer xmlCompopser = new RecordXmlComposer();
                xmlCompopser.startCompose();

                RecordXmlInfo restoreInfo = new RecordXmlInfo();
                restoreInfo.setRestore(true);
                restoreInfo.setDevice(Utils.getPhoneSearialNumber());
                restoreInfo.setTime(String.valueOf(System.currentTimeMillis()));

                boolean bAdded = false;
                for (RecordXmlInfo record : recordList) {
                    if (record.getDevice().equals(restoreInfo.getDevice())) {
                        xmlCompopser.addOneRecord(restoreInfo);
                        bAdded = true;
                    } else {
                        xmlCompopser.addOneRecord(record);
                    }
                }

                if (!bAdded) {
                    xmlCompopser.addOneRecord(restoreInfo);
                }
                xmlCompopser.endCompose();
                Utils.writeToFile(xmlCompopser.getXmlInfo(), recrodXmlFile);
            }

            if (SDCardUtils.isSdCardMissingOrFull(getActivity())) {
                MyLogger.logD(CLASS_TAG, "SD card full/removed,");

                // Below code kills app when restore done when SD full
                /*if (mHandler != null) {
                    mHandler.post(new Runnable() {

                        public void run() {
                            Toast.makeText(getActivity(),
                             getString(R.string.toast_sd_removed_or_full),
                              Toast.LENGTH_SHORT).show();
                        }
                    });
                }
                getActivity().finish();
                return ;*/
            }

            if (mHandler != null) {
                mHandler.post(new Runnable() {
                    public void run() {

                        MyLogger.logD(CLASS_TAG, " Restore show Result Dialog");
                        int state = mRestoreService.getState();
                         MyLogger.logD(CLASS_TAG,
                          " Restore show Result state = " + state + "mIsDestroyed = "
                           + mIsDestroyed);
                        if (mIsStoped && mFile.exists() && state != State.FINISH) {
                            mNeedUpdateResult = true;
                        } else {
                            //if (!mIsDestroyed) {
                                showRestoreResult(iResultRecord);
                            //}
                        }
                    }
                });
            }
        }

        /**
         * onRestoreErr.
         */
        public void onRestoreErr(IOException e) {
            MyLogger.logI(CLASS_TAG, "onRestoreErr");
            if (!mIsStopped && errChecked()) {
                if (mRestoreService != null && mRestoreService.getState() != State.INIT
                        && mRestoreService.getState() != State.FINISH) {
                    mRestoreService.pauseRestore();
                }
            }
        }
    }

    protected boolean errChecked() {
        boolean ret = false;

        boolean isSDCardMissing = SDCardUtils.isSdCardMissing(getActivity());
        String path = SDCardUtils.getStoragePath(getActivity());

        if (isSDCardMissing) {
            MyLogger.logI(CLASS_TAG, "SDCard is removed");
            getActivity().stopService(new Intent(getActivity(), RestoreService.class));
            Utils.exitLockTaskModeIfNeeded(getActivity());
            getActivity().finish();
        } else if (SDCardUtils.getAvailableSize(path) <= SDCardUtils.MINIMUM_SIZE) {
            MyLogger.logI(CLASS_TAG, "SDCard is full");
            ret = true;
            Toast.makeText(getActivity(),
             getString(R.string.error_sd_card_full), Toast.LENGTH_SHORT).show();
            /*if (mHandler != null) {
                mHandler.post(new Runnable() {
                    public void run() {
                        showDialog(DialogID.DLG_SDCARD_FULL, null);
                    }
                });
            }*/
        } else {
            MyLogger.logE(CLASS_TAG, "Unkown error, don't pause.");
        }
        return ret;
    }


    }

    /**
     * @author mtk81330
     *
     */
    /*private class DeleteCheckItemTask extends AsyncTask<HashSet<File>, String, Long> {

        private ProgressDialog mDeletingDialog;

        public DeleteCheckItemTask() {
            mDeletingDialog = new ProgressDialog(getActivity());
            mDeletingDialog.setCancelable(false);
            mDeletingDialog.setMessage(getString(R.string.delete_please_wait));
            mDeletingDialog.setIndeterminate(true);
        }

        @Override
        protected void onPostExecute(Long arg0) {
            super.onPostExecute(arg0);
            Activity activity = getActivity();
            if (activity != null) {
                startScanFiles(getActivity());
            }
            if (activity != null && mDeletingDialog != null) {
                mDeletingDialog.dismiss();
            }
        }

        @Override
        protected void onPreExecute() {
            Activity activity = getActivity();
            if (activity != null && mDeletingDialog != null) {
                mDeletingDialog.show();
            }
        }

        @Override
        protected Long doInBackground(HashSet<File>... params) {
            HashSet<File> deleteFiles = params[0];
            for (File file : deleteFiles) {
                FileUtils.deleteFileOrFolder(file, getActivity());
            }
            return null;
        }
    }*/

    /**
     * @return ActionMode.
     */
/*    public ActionMode getDeleteMode() {
        return mDeleteActionMode;
    }
}*/
