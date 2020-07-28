package com.mediatek.engineermode.hqanfc;

import android.app.AlertDialog;
import android.app.Dialog;
import android.app.ProgressDialog;
import android.content.DialogInterface;
import android.nfc.NfcAdapter;
import android.os.AsyncTask;
import android.os.Bundle;
import android.preference.Preference;
import android.preference.PreferenceActivity;
import android.preference.PreferenceScreen;
import android.widget.Toast;

import com.mediatek.engineermode.ChipSupport;
import com.mediatek.engineermode.Elog;
import com.mediatek.engineermode.FeatureSupport;
import com.mediatek.engineermode.R;
import com.mediatek.engineermode.ShellExe;
import com.mediatek.engineermode.hqanfc.NfcCommand.CommandType;

import java.io.File;
import java.io.IOException;

/**
 * NFC main menu.
 */
public class NfcMainPage extends PreferenceActivity {
    public static final String TAG = "Nfc";
    private static final String START_LIB_COMMAND = "./system/bin/nfcstackp";
    private static final String KILL_LIB_COMMAND = "kill -9 nfcstackp";
    private static final String QUICK_MODE_FILE = "/sdcard/mtknfcSyncQuickMode";
    private static final int DIALOG_WAIT = 1;
    private static final int DLG_NOT_SUPPORT = 2;
    private ConnectServerTask mTask;
    private boolean mIsSupport = true;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        if (!FeatureSupport.isNfcSupport()) {
            showDialog(DLG_NOT_SUPPORT);
            mIsSupport = false;
            return;
        }
        addPreferencesFromResource(R.xml.hqa_nfc_main);
        showDialog(DIALOG_WAIT);
        mTask = new ConnectServerTask();
        mTask.execute();
    }

    protected void onDestroy() {
        if (mIsSupport) {
            Elog.i(TAG, "[NfcMainPage]Nfc main page onDestroy().");
            // nfc enter nci idle flow , wait command ...
            NfcClient.getInstance().sendCommand(CommandType.MTK_NFC_EM_DEACTIVATE_CMD, null);
            // nfc enter nci deinit flow
            NfcClient.getInstance().sendCommand(CommandType.MTK_NFC_EM_STOP_CMD, null);
            NfcClient.getInstance().closeConnection();
            mTask.cancel(true);
        }

        super.onDestroy();
    }

    @Override
    protected Dialog onCreateDialog(int id) {
        switch (id) {
        case DIALOG_WAIT:
            ProgressDialog dialog = new ProgressDialog(this);
            dialog.setMessage("Please Wait...");
            dialog.setCancelable(false);
            dialog.setIndeterminate(true);
            return dialog;
        case DLG_NOT_SUPPORT:
            return new AlertDialog.Builder(this).setTitle(R.string.nfc_st)
                    .setCancelable(false)
                    .setMessage(getString(R.string.nfc_st_not_support))
                    .setPositiveButton(R.string.dialog_ok,
                new DialogInterface.OnClickListener() {
                    public void onClick(DialogInterface dialog, int which) {
                        finish();
                    }
                }).create();
        default:
            Elog.d(TAG, "error dialog ID");
            break;
        }
        return super.onCreateDialog(id);
    }

    private void closeNFCServiceAtStart() {
        NfcAdapter adp = NfcAdapter.getDefaultAdapter(getApplicationContext());
        if (adp.isEnabled()) {
            if (adp.disable()) {
                Elog.i(TAG, "[NfcMainPage]Nfc service set off.");
            } else {
                Elog.i(TAG, "[NfcMainPage]Nfc service set off Fail.");
            }
        } else {
            Elog.i(TAG, "[NfcMainPage]Nfc service is off");
        }
    }

    private void executeXbinFile(final String command, int sleepTime) {
        new Thread() {
            @Override
            public void run() {
                Elog.d(TAG, "[NfcMainPage]nfc command:" + command);
                try {
                    int err = ShellExe.execCommand(command, true);
                    Elog.d(TAG, "[NfcMainPage]nfc command:result: " + err);
                } catch (IOException e) {
                    Elog.e(TAG, "[NfcMainPage]executeXbinFile IOException: " + e.getMessage());
                    e.printStackTrace();
                }
            }
        } .start();
        try {
            Thread.sleep(500);
        } catch (InterruptedException e) {
            Elog.e(TAG, "[NfcMainPage]executeXbinFile InterruptedException: " + e.getMessage());
        }
    }

    private void setNfcQuickMode(int mode) {
        File file = new File(QUICK_MODE_FILE);
        boolean result = false;
        try {
            Elog.i(TAG, "[QE]setNfcQuickMode(" + mode);
            if (!file.exists()) {
                if (mode == 1) {
                    result = file.createNewFile();
                }
            } else {
                if (mode == 0) {
                    result = file.delete();
                }
            }
            Elog.i(TAG, "[QE]setNfcQuickMode(" + mode + ") result:" + result);
        } catch (IOException e) {
            e.printStackTrace();
        }
    }

    private void init() {
        NfcAdapter adp = NfcAdapter.getDefaultAdapter(getApplicationContext());
        if (adp == null) {
            Elog.e(TAG, "adp == null");
            return;
        }
        Elog.i(TAG, "[QE]Engineer Mode clear all.");
        setNfcQuickMode(0);
        Elog.i(TAG, "[QE]set file");
        setNfcQuickMode(1);
        Elog.i(TAG, "[QE]NFC Disable.");
        if (adp.isEnabled()) {
            Elog.i(TAG, "[QE] force NFC Disable.");
            adp.disable();
        } else {
            Elog.i(TAG, "[QE]NFC Enable -->Disable.");
            adp.enable();
            try {
                Thread.sleep(100);
            } catch (InterruptedException e) {
                Elog.i(TAG, "InterruptedException");
            }
            adp.disable();
        }
        do {
            try {
                Thread.sleep(2000);
            } catch (InterruptedException e) {
                Elog.i(TAG, "InterruptedException");
            }
        } while (adp.isEnabled());
        executeXbinFile(START_LIB_COMMAND, 500);
    }

    private class ConnectServerTask extends AsyncTask<Void, Void, Boolean> {

        @Override
        protected Boolean doInBackground(Void... params) {
            init();
            return NfcClient.getInstance().createConnection(NfcMainPage.this);
        }

        @Override
        protected void onPostExecute(Boolean result) {
            if (null != result && result.booleanValue()) {
                PreferenceScreen screen = getPreferenceScreen();
                int count = screen.getPreferenceCount();
                for (int index = 0; index < count; index++) {
                    screen.getPreference(index).setEnabled(true);
                }
                // nfc enter nci init flow
                NfcClient.getInstance().sendCommand(CommandType.MTK_NFC_EM_START_CMD, null);
                // nfc enter nci idle flow , wait command ...
                NfcClient.getInstance().sendCommand(CommandType.MTK_NFC_EM_DEACTIVATE_CMD, null);
            } else {
                Toast.makeText(NfcMainPage.this, R.string.hqa_nfc_connect_fail, Toast.LENGTH_SHORT).show();
            }
            NfcMainPage.this.dismissDialog(DIALOG_WAIT);
        }
    }

    @Override
    protected boolean isValidFragment(String fragmentName) {
        // TODO Auto-generated method stub
        Elog.i(TAG, "fragmentName is " + fragmentName);
        return false;
    }

}
