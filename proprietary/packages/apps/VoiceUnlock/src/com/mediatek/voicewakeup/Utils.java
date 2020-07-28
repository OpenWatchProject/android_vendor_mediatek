package com.mediatek.voicewakeup;

import java.util.HashMap;

import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.ServiceConnection;
import android.content.pm.ActivityInfo;
import android.content.pm.PackageManager;
import android.content.pm.PackageManager.NameNotFoundException;
import android.os.Bundle;
import android.os.Handler;
import android.os.IBinder;
import android.os.Message;
import android.os.RemoteException;
import android.provider.Settings;
import android.util.Log;
import android.widget.Toast;

import com.mediatek.common.voicecommand.IVoiceCommandListener;
import com.mediatek.common.voicecommand.IVoiceCommandManagerService;
import com.mediatek.common.voicecommand.VoiceCommandListener;
import com.mediatek.provider.MtkSettingsExt;
import com.mediatek.voiceunlock.SettingsPreferenceFragment;
import com.mediatek.voiceunlock.VoiceUnlockPreference;
import com.mediatek.voicewakeup.VowNoSpeaker.VowNoSpeakerFragment;
import com.mediatek.voiceunlock.R;

public class Utils {

    static final String KEY_COMMAND_ID = "command_id";
    static final String KEY_COMMAND_SUMMARY = "command_summary";
    static final String KEY_COMMAND_TITLE = "command_title";
    // launched app component string
    static final String KEY_COMMAND_VALUE = "command_value";
    // 0: record, 1: modify
    static final String KEY_COMMAND_TYPE = "command_type";
    static final int COMMAND_TYPE_RECORD = 0;
    static final int COMMAND_TYPE_MODIFY = 2;
    // 0: voice unlock, 1: no speaker id, 2: with speak id
    static final String KEY_COMMAND_MODE = "command_mode";
    static final int VOW_NO_SPEAKER_MODE = 1;
    static final int VOW_WITH_SPEAKER_MODE = 2;

    public static final String VOICE_WAKEUP_MODE = MtkSettingsExt.System.VOICE_WAKEUP_MODE;
    public static final int VOICE_WAKEUP_ANYONE = 1;
    public static final int VOICE_WAKEUP_COMMAND = 2;

    private static final String TAG = "VowUtils";
    private Context mContext;
    private String mPkgName;
    private static final Object sLock = new Object();
    private static Utils sInstance;

    final HashMap<Integer, VowCommandInfo> mKeyCommandInfoMap = new HashMap<Integer, VowCommandInfo>();
    private VoiceServiceListener mVoiceServiceListener;

    public interface VoiceServiceListener {
        public void onVoiceServiceConnect();

        public void handleVoiceCommandNotified(int mainAction, int subAction, Bundle extraData);
    }

    IVoiceCommandManagerService mVCmdMgrService;
    private boolean mIsAttachedService = false;

    private IVoiceCommandListener mVoiceCallback = new IVoiceCommandListener.Stub() {
        public void onVoiceCommandNotified(int mainAction, int subAction, Bundle extraData)
                throws RemoteException {
            Message.obtain(mVoiceCommandHandler, mainAction, subAction, 0, extraData)
                    .sendToTarget();
        }
    };

    private Handler mVoiceCommandHandler = new Handler() {
        public void handleMessage(Message msg) {
            if (mVoiceServiceListener != null) {
                mVoiceServiceListener.handleVoiceCommandNotified(msg.what, msg.arg1,
                        (Bundle) msg.obj);
            }
        }
    };

    public static Utils getInstance() {
        synchronized (sLock) {
            if (sInstance == null) {
                sInstance = new Utils();
            }
            return sInstance;
        }
    }

    void setContext(Context context) {
        mContext = context;
        Log.d("@M_" + TAG, "mContext = " + mContext);
        if (mContext != null) {
            mPkgName = mContext.getPackageName();
        }
    }

    void setOnChangedListener(VoiceServiceListener l) {
        mVoiceServiceListener = l;
    }

    void onResume() {
        Log.d("@M_" + TAG, "register to service");
        if (mVCmdMgrService == null) {
            bindVoiceService(mContext);
        } else {
            registerVoiceCommand(mPkgName);
        }
    }

    void onPause() {
        if (mVCmdMgrService != null) {
            Log.d("@M_" + TAG, "sendCommand TRAINING_STOP");
            sendVoiceCommand(mPkgName, VoiceCommandListener.ACTION_MAIN_VOICE_TRAINING,
                    VoiceCommandListener.ACTION_VOICE_TRAINING_STOP, null);

            Log.d("@M_" + TAG, "unregister to service");
            unregisterVoicecommand(mPkgName);
        }
    }

    void onContinue() {
        if (mVCmdMgrService != null) {
            Log.d("@M_" + TAG, "sendCommand TRAINING_CONTINUE");
            sendVoiceCommand(mPkgName, VoiceCommandListener.ACTION_MAIN_VOICE_TRAINING,
                    VoiceCommandListener.ACTION_VOICE_TRAINING_CONTINUE, null);
        }
    }

    boolean isLastResetCommand() {
        int count = 0;
        for (int i = 0; i < mKeyCommandInfoMap.size(); i++) {
            Log.d("@M_" + TAG, "updateCommandStatus key: " + i);
            if (MtkSettingsExt.System.getVoiceCommandValue(mContext.getContentResolver(),
                    MtkSettingsExt.System.BASE_VOICE_WAKEUP_COMMAND_KEY,
                    mKeyCommandInfoMap.get(i).mId) != null) {
                count++;
            }
        }
        return count == 1;
    }

    /**
     * Count Enrolled model items.
     *
     * @param length
     *            max command id count to check in settings
     *
     * @return result
     *            count of the enrolled models
     */
    int countEnrolledModel(int length) {
        int count = 0;
        for (int cmdId = 0; cmdId < length; cmdId++) {
            Log.d("@M_" + TAG, "isLastEnrolledModel commandId: " + cmdId);
            if (MtkSettingsExt.System.getVoiceCommandValue(mContext.getContentResolver(),
                MtkSettingsExt.System.BASE_VOICE_WAKEUP_COMMAND_KEY, cmdId) != null) {
                count++;
            }
        }
        return count;
    }

    String getAppLabel(ComponentName componentName) {
        ActivityInfo info;
        try {
            info = mContext.getPackageManager().getActivityInfo(componentName,
                    PackageManager.GET_SHARED_LIBRARY_FILES);
        } catch (NameNotFoundException e) {
            return null;
        }

        CharSequence name = info.loadLabel(mContext.getPackageManager());
        return name.toString();
    }

    void sendVoiceCommand(String pkgName, int mainAction, int subAction, Bundle extraData) {
        if (mIsAttachedService) {
            try {
                int errorid = mVCmdMgrService
                        .sendCommand(pkgName, mainAction, subAction, extraData);
                Log.e("@M_" + TAG, "send voice Command errorid: " + errorid);
            } catch (RemoteException e) {
                mIsAttachedService = false;
                mVCmdMgrService = null;
                Log.e("@M_" + TAG, "send voice Command RemoteException =  " + e.getMessage());
            }
        } else {
            Log.d("@M_" + TAG, "didn't register , can not send voice Command  ");
        }
    }

    void playCommand(SettingsPreferenceFragment fragment, int commandId, String commandSummary,
            int vowMode) {
        Log.d("@M_" + TAG, "playCommand commandId = " + commandId + " summary = " + commandSummary);
        Intent intent = new Intent("com.mediatek.voicecommand.VOICE_UNLOCK_PSWPREVIEW");
        intent.putExtra(KEY_COMMAND_SUMMARY, commandSummary);
        intent.putExtra(KEY_COMMAND_ID, commandId);
        intent.putExtra(KEY_COMMAND_MODE, vowMode);
        try {
            fragment.startActivity(intent);
        } catch (android.content.ActivityNotFoundException e) {
            Toast.makeText(mContext, R.string.pass_word_file_missing, Toast.LENGTH_SHORT).show();
        }
    }

    void updateCommand(SettingsPreferenceFragment fragment, int commandId, String commandValue,
            int commandType, int vowMode, String commandKeyword) {
        Log.d("@M_" + TAG, "recordOrModifyCommand commandId = " + commandId + " commandValue = "
                + commandValue + " commandType = " + commandType + " vowMode = " + vowMode);
        Log.d("@M_" + TAG, "recordOrModifyCommand commandKeyword = "
                + commandKeyword);
        Intent intent = new Intent();
        intent.setClass(mContext, VowCommandRecord.class);
        intent.putExtra(KEY_COMMAND_ID, commandId);
        intent.putExtra(KEY_COMMAND_VALUE, commandValue);
        intent.putExtra(KEY_COMMAND_TYPE, commandType); // modify or
        // record
        intent.putExtra(KEY_COMMAND_MODE, vowMode); // with or no speaker id
        // mode
        if (commandKeyword != null) {
            intent.putExtra(VowNoSpeakerFragment.KEY_COMMAND_KEYWORD, commandKeyword);
        }
        fragment.startActivity(intent);
    }

    private void registerVoiceCommand(String pkgName) {
        if (!mIsAttachedService) {
            try {
                int errorid = mVCmdMgrService.registerListener(pkgName, mVoiceCallback);
                if (errorid == VoiceCommandListener.VOICE_NO_ERROR) {
                    mIsAttachedService = true;
                } else {
                    Log.e("@M_" + TAG, "register voiceCommand fail ");
                }
            } catch (RemoteException e) {
                mIsAttachedService = false;
                mVCmdMgrService = null;
                Log.e("@M_" + TAG, "register voiceCommand RemoteException =  " + e.getMessage());
            }
        } else {
            Log.d("@M_" + TAG, "register voiceCommand success ");
        }
        Log.d("@M_" + TAG, "register voiceCommand end ");
    }

    private void unregisterVoicecommand(String pkgName) {
        if (mVCmdMgrService != null) {
            try {
                int errorid = mVCmdMgrService.unregisterListener(pkgName, mVoiceCallback);
                if (errorid == VoiceCommandListener.VOICE_NO_ERROR) {
                    mIsAttachedService = false;
                }
            } catch (RemoteException e) {
                Log.e("@M_" + TAG, "unregisteVoiceCmd voiceCommand RemoteException = " + e.getMessage());
                mIsAttachedService = false;
                mVCmdMgrService = null;
            }
            Log.d("@M_" + TAG, "unregisteVoiceCmd end ");
            // ALPS01523837 , as NE happened firstly,CR:ALPS01509601
            try {
                mContext.unbindService(mVoiceSerConnection);
            } catch (IllegalArgumentException illegalArgExp) {
                Log.e("@M_" + TAG, "happen exception , maybe as NE leads to service is killed");
            }
            mVCmdMgrService = null;
            mIsAttachedService = false;
        }
    }

    private void bindVoiceService(Context context) {
        Log.d("@M_" + TAG, "bindVoiceService begin  ");
        Intent mVoiceServiceIntent = new Intent();
        mVoiceServiceIntent.setAction(VoiceCommandListener.VOICE_SERVICE_ACTION);
        mVoiceServiceIntent.addCategory(VoiceCommandListener.VOICE_SERVICE_CATEGORY);
        mVoiceServiceIntent.setPackage(VoiceCommandListener.VOICE_SERVICE_PACKAGE_NAME);
        context.bindService(mVoiceServiceIntent, mVoiceSerConnection, Context.BIND_AUTO_CREATE);
    }

    private ServiceConnection mVoiceSerConnection = new ServiceConnection() {
        @Override
        public void onServiceConnected(ComponentName name, IBinder service) {
            mVCmdMgrService = IVoiceCommandManagerService.Stub.asInterface(service);
            registerVoiceCommand(mPkgName);
            Log.d("@M_" + TAG, "onServiceConnected   ");
            if (mVoiceServiceListener != null) {
                mVoiceServiceListener.onVoiceServiceConnect();
            }
        }

        public void onServiceDisconnected(ComponentName name) {
            Log.d("@M_" + TAG, "onServiceDisconnected   ");
            mIsAttachedService = false;
            mVCmdMgrService = null;
        }
    };

    static class VowCommandInfo {

        int mId;
        String mPreferTitle;
        String mPreferSummary;
        ComponentName mLaunchedApp;

        public VowCommandInfo(int id) {
            mId = id;
        }
    }

    /*
     * Query wakeup mode from setting provider * @return
     */
    public int getWakeupMode(Context context) {
        return Settings.System.getInt(context.getContentResolver(), VOICE_WAKEUP_MODE, 1);
    }

    /*
     * convert the array to string for keywords
     */
    public String getKeywords(String[] keywordArray, String split) {
        if (keywordArray == null) {
            return null;
        }

        String keywords;
        int len = keywordArray.length;
        Log.d("@M_" + TAG, "len =  " + len);
        if (1 == len) {
            keywords = keywordArray[0];
        } else {
            StringBuffer sb = new StringBuffer();
            for (int i = 0; i < len - 1; i++) {
                sb.append(keywordArray[i]);
                sb.append(split);
            }
            sb.append(keywordArray[len - 1]);
            keywords = sb.toString();
        }
        Log.d("@M_" + TAG, "keywords = " + keywords);
        return keywords;
    }

    /**
     * Convert the class and package array to array of component names.
     *
     * @param classArray
     *            array of classes
     * @param packageArray
     *            array of packages
     * @return data
     *            Component name array
     */
    public ComponentName[] getComponentArray(String[] classArray, String[] packageArray) {
        if (classArray == null || packageArray == null
            || classArray.length == 0 || packageArray.length != classArray.length) {
            return null;
        }
        ComponentName[] componentArray = new ComponentName[classArray.length];
        for (int id = 0; id < classArray.length; id++) {
            String packageStr = packageArray[id];
            String classStr = classArray[id];
            componentArray[id] = new ComponentName(packageStr, classStr);
        }
        return componentArray;
    }
}
