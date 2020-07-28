package com.mediatek.engineermode.lte;

import android.app.AlertDialog;
import android.app.Dialog;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.SharedPreferences;
import android.os.AsyncResult;
import android.os.Bundle;
import android.os.Environment;
import android.os.Handler;
import android.os.Message;
import android.preference.Preference;
import android.preference.PreferenceActivity;
import android.preference.PreferenceCategory;
import android.preference.PreferenceScreen;
import android.telephony.TelephonyManager;
import android.view.Menu;
import android.view.MenuItem;
import android.view.ViewGroup;
import android.widget.EditText;
import android.widget.LinearLayout;
import android.widget.RadioButton;
import android.widget.TextView;
import android.widget.Toast;

import com.mediatek.engineermode.Elog;
import com.mediatek.engineermode.EmUtils;
import com.mediatek.engineermode.ModemCategory;
import com.mediatek.engineermode.R;

import java.io.BufferedReader;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.IOException;
import java.io.InputStreamReader;
import java.util.ArrayList;
import java.util.HashMap;

public class CommandTool extends PreferenceActivity {
    private static final String TAG = "CommandTool";
    private static final int DIALOG_CHOOSE_CONFIG_FILE = 1;
    private static final String PREF = "command_tool";
    private static final String PREF_KEY = "config_file";
    private static final String DEFAULT_CONFIG_FILE = "/config.txt";
    private static final int MENU_CONFIG_FILE = 1000;
    private static final int MENU_SIMS = 1001;
    private static final String PREF_FILE = "CommandTool";
    private static final String KEY_LTE_SIM_STATUS = "lte_sim_status";
   // private static final String QUERY_STATUS_COMMAND = "+EFAKEID?";
   // private static final String SAME_COMMAND = "+EFAKEID:";
    private static final String QUERY_STATUS_COMMAND = "AT+EPBSE?";
    private static final String SAME_COMMAND = "+EPBSE:";
    private static final String SET_STATUS_COMMAND = "+EFAKEID=";
    private static final int MSG_AT_COMMAND_SIM1 = 1;
    private static final int MSG_AT_COMMAND_SIM2 = 2;
    private static final int MSG_AT_UPDATE_SIM_MCC_MNC = 3;
    public static int mAtCmdSentToIndex = 0;
    private String mMccMncvalues[] = {"123","456"};
    private RadioButton mLteSim1;
    private RadioButton mLteSim2;
    private TextView mLteMccMncSim1;
    private TextView mLteMccMncSim2;
    private final Handler mHandler = new Handler() {
        @Override
        public void handleMessage(Message msg) {
            Elog.d(TAG, "handleMessage() " + msg.what);
            AsyncResult ar = (AsyncResult) msg.obj;
            switch (msg.what) {
                case MSG_AT_COMMAND_SIM1:
                    if (ar.exception != null) {
                        Elog.e(TAG, "query mccmnc1 failed ");
                    } else {
                        Elog.d(TAG, "query mccmnc1 succeed");
                        final String[] result = (String[]) ar.result;
                        mMccMncvalues[0] = result[0];
                        if (TelephonyManager.getDefault().getSimCount() > 1) {
                            queryMccMnc(1, MSG_AT_COMMAND_SIM2);
                        } else {
                            mHandler.sendEmptyMessage(MSG_AT_UPDATE_SIM_MCC_MNC);
                        }
                    }

                    break;
                case MSG_AT_COMMAND_SIM2:
                    if (ar.exception != null) {
                        Elog.e(TAG, "query mccmnc2 failed ");
                    } else {
                        Elog.d(TAG, "query mccmnc2 succeed");
                        final String[] result = (String[]) ar.result;
                        mMccMncvalues[1] = result[0];
                        mHandler.sendEmptyMessage(MSG_AT_UPDATE_SIM_MCC_MNC);
                    }
                    break;
                case MSG_AT_UPDATE_SIM_MCC_MNC:
                    Elog.d(TAG, "update mcc mnc status");
                    mLteMccMncSim1.setText(mMccMncvalues[0]);
                    if (TelephonyManager.getDefault().getSimCount() > 1) {
                        mLteMccMncSim2.setText(mMccMncvalues[1]);
                    }
                    break;
                default:
                    break;
            }
        }
    };
    private LinearLayout mSimsSettingsForm = null;
    private String mConfigFilePath = "";
    private HashMap<Preference, ArrayList<String>> mTestItems =
            new HashMap<Preference, ArrayList<String>>();
    private Toast mToast;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        PreferenceScreen preferenceScreen = getPreferenceManager().createPreferenceScreen(this);
        setPreferenceScreen(preferenceScreen);
        startService(new Intent(this, CommandToolService.class));

        SharedPreferences pref = getSharedPreferences(PREF, MODE_PRIVATE);
        mConfigFilePath = pref.getString(PREF_KEY,
                Environment.getExternalStorageDirectory().getPath() + DEFAULT_CONFIG_FILE);
        mSimsSettingsForm = (LinearLayout) getLayoutInflater()
                .inflate(R.layout.lte_tool_sims_settings, null);
        mLteSim1 = (RadioButton) mSimsSettingsForm.findViewById(R.id.lte_tool_sim1);
        mLteSim2 = (RadioButton) mSimsSettingsForm.findViewById(R.id.lte_tool_sim2);
        mLteMccMncSim1 = (TextView) mSimsSettingsForm.findViewById(R.id.lte_mccmnc_sim1);
        mLteMccMncSim2 = (TextView) mSimsSettingsForm.findViewById(R.id.lte_mccmnc_sim2);
        restoreSettingState();
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
    }

    @Override
    protected void onResume() {
        super.onResume();
        parse();
    }

    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        menu.add(0, MENU_CONFIG_FILE, 0, "Choose config file");
        menu.add(0, MENU_SIMS, 0, "Sim switch and status");
        return true;
    }

    void sims_switch() {
        if (mLteSim1.isChecked()) {
            mAtCmdSentToIndex = 0;
        } else if (mLteSim2.isChecked()) {
            mAtCmdSentToIndex = 1;
        }
        saveSettingsState();
    }

    private void saveSettingsState() {
        SharedPreferences.Editor editor = getSharedPreferences(PREF_FILE, MODE_PRIVATE).edit();
        editor.putInt(KEY_LTE_SIM_STATUS, mAtCmdSentToIndex);
        editor.apply();
    }

    private void restoreSettingState() {
        SharedPreferences pref = getSharedPreferences(PREF_FILE, MODE_PRIVATE);
        mAtCmdSentToIndex = pref.getInt(KEY_LTE_SIM_STATUS, ModemCategory.getCapabilitySim());
        mLteSim1.setChecked(mAtCmdSentToIndex == 0);
        mLteSim2.setChecked(mAtCmdSentToIndex == 1);
    }

    void queryMccMnc(int mdIndex, int what) {
        String[] cmd = new String[2];
        cmd[0] = QUERY_STATUS_COMMAND;
        cmd[1] = SAME_COMMAND;
        Elog.d(TAG, "query at cmd: " + cmd[0]);
        EmUtils.invokeOemRilRequestStringsEm(mdIndex, cmd,
                mHandler.obtainMessage(what));
    }

    private void simSwitchAndStatusInit() {
        restoreSettingState();
        queryMccMnc(0, MSG_AT_COMMAND_SIM1);
     //   mHandler.sendEmptyMessageDelayed(MSG_AT_UPDATE_SIM_MCC_MNC, 2000);
    }


    private void simSwitchAndStatus() {
        DialogInterface.OnClickListener dialogOnclicListener = new DialogInterface
                .OnClickListener() {
            @Override
            public void onClick(DialogInterface dialog, int which) {
                ((ViewGroup) mSimsSettingsForm.getParent()).removeView(mSimsSettingsForm);
                switch (which) {
                    case Dialog.BUTTON_NEGATIVE:
                        break;
                    case Dialog.BUTTON_POSITIVE:
                        sims_switch();
                        break;
                }
            }
        };
        AlertDialog.Builder builder = new AlertDialog.Builder(this);
        builder.setTitle("Sim status:");
        builder.setNegativeButton("Cancel", dialogOnclicListener);
        builder.setPositiveButton("yes", dialogOnclicListener);
        builder.setCancelable(false);
        builder.setView(mSimsSettingsForm);
        builder.create().show();
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem aMenuItem) {
        switch (aMenuItem.getItemId()) {
            case MENU_CONFIG_FILE:
                showDialog(DIALOG_CHOOSE_CONFIG_FILE);
                break;
            case MENU_SIMS:
                simSwitchAndStatusInit();
                simSwitchAndStatus();
                break;
            default:
                break;
        }
        return super.onOptionsItemSelected(aMenuItem);
    }

    @Override
    public Dialog onCreateDialog(int id) {
        if (id == DIALOG_CHOOSE_CONFIG_FILE) {
            final EditText input = new EditText(this);
            input.setText(mConfigFilePath);
            DialogInterface.OnClickListener listener = new DialogInterface.OnClickListener() {
                @Override
                public void onClick(DialogInterface dialog, int which) {
                    if (input.getText() != null && (!input.getText().toString().equals(""))) {
                        mConfigFilePath = input.getText().toString().trim();
                        SharedPreferences pref = CommandTool.this.getSharedPreferences(
                                PREF, Context.MODE_PRIVATE);
                        pref.edit().putString(PREF_KEY, mConfigFilePath).commit();
                        parse();
                    }
                }
            };

            return new AlertDialog.Builder(this)
                    .setTitle("Config file:")
                    .setView(input)
                    .setPositiveButton("OK", listener)
                    .setNegativeButton("Cancel", null).create();
        }
        return super.onCreateDialog(id);
    }

    @Override
    public boolean onPreferenceTreeClick(PreferenceScreen preferenceScreen,
                                         Preference preference) {
        Intent intent = new Intent(this, CommandToolEdit.class);
        intent.putExtra("commands", mTestItems.get(preference));
        startActivity(intent);
        return true;
    }

    private void parse() {
        FileInputStream inputStream = null;
        try {
            File f = new File(mConfigFilePath);
            inputStream = new FileInputStream(f);
            doParse(new BufferedReader(new InputStreamReader(inputStream)));
            inputStream.close();
        } catch (FileNotFoundException e) {
            showToast("Config file not found.");
        } catch (IOException e) {
            showToast("Read config file error.");
        } finally {
            if (null != inputStream) {
                try {
                    inputStream.close();
                } catch (IOException e) {
                    e.printStackTrace();
                }
            }
        }
    }

    public void doParse(BufferedReader in) throws IOException {
        getPreferenceScreen().removeAll();
        mTestItems.clear();

        PreferenceCategory category = new PreferenceCategory(this);
        Preference pref = new Preference(this);
        ArrayList<String> cmds = new ArrayList<String>();

        String line = in.readLine();
        while (line != null) {
            if (!line.startsWith(" ")) {
                category = new PreferenceCategory(this);
                category.setTitle(line);
                getPreferenceScreen().addPreference(category);
            } else {
                line = line.substring(1);
                if (!line.startsWith(" ")) {
                    cmds = new ArrayList<String>();
                    pref = new Preference(this);
                    pref.setTitle(line);
                    category.addPreference(pref);
                    mTestItems.put(pref, cmds);
                } else {
                    cmds.add(line.trim());
                }
            }
            line = in.readLine();
        }
    }

    @Override
    protected void onPause() {
        super.onPause();
    }

    ;

    private void showToast(String msg) {
        if (mToast != null) {
            mToast.cancel();
        }
        mToast = Toast.makeText(this, msg, Toast.LENGTH_SHORT);
        mToast.show();
    }

    @Override
    protected boolean isValidFragment(String fragmentName) {
        // TODO Auto-generated method stub
        return false;
    }

}

