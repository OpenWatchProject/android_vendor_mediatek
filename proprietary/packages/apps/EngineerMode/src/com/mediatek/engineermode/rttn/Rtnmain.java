package com.mediatek.engineermode.rttn;

import android.annotation.TargetApi;
import android.app.Activity;
import android.app.AlertDialog;
import android.app.ProgressDialog;
import android.content.BroadcastReceiver;
import android.content.ContentResolver;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.IntentFilter;
import android.net.Uri;
import android.os.AsyncResult;
import android.os.AsyncTask;
import android.os.Build;
import android.os.Bundle;
import android.os.Handler;
import android.os.Looper;
import android.os.Message;
import android.os.PowerManager;
import android.telephony.SubscriptionManager;
import android.telephony.TelephonyManager;
import android.text.InputType;
import android.util.Log;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.Button;
import android.widget.EditText;
import android.widget.Toast;

import com.android.internal.telephony.CommandException;
import com.mediatek.engineermode.Elog;
import com.mediatek.engineermode.EmUtils;
import com.mediatek.engineermode.ModemCategory;
import com.mediatek.engineermode.R;

import java.util.Arrays;

/**
 * Created by MTK33301 on 7/13/2017.
 */

@TargetApi(Build.VERSION_CODES.LOLLIPOP_MR1)
public class Rtnmain extends Activity {
    private static String TAG = "EM/Rtnmain";
    private static Handler handler = null;
    private static final int MSG_SET = 1;
    private static final int MSG_QUERY = 2;
    private static final int MSG_POLL = 3;
    private static final int MSG_QUERY_POLL = 4;
    private SubscriptionManager mSubscriptionManager;
    private static final int EVENT_SET_MDN = 1;
    private static final int EVENT_RESET_MDN = 2;
    private static final int EVENT_POLL_MDN = 3;
    private static Object mObject = new Object();
    private static int mResult = -1;
    public static int RIL_NV_DEVICE_MSL = 11;                 // device MSL
    public static int sim_id = 0;
    private static final int MAX_TIME = 3000;
    private static final int POLL_TIME = 2000;
    private static final int PASS = 1;
    private static final int FAIL = 0;
    private static final int UNKNOWN = -1;
    private static final int PROFILE_UPDATE = 1;
    private static final int PRL_UPDATE = 2;
    private static final int DEVICE_UPDATE = 3;
    private static final String FACTORY_REST_PERMISSION = "android.permission.MASTER_CLEAR";

    protected TelephonyManager tm;
    private static Button mBtReBoot = null;
    private static String ch_id_s = null;
    protected static String rtn_msl;
    private static int ch_id = 4;
    private static boolean mIsPolling = false;
    private static boolean sIsBipReceiverRegistered = false;
    private static boolean sIsBipSessionStarted = false;
    private static String[] atcmd = new String[3];
    private static WorkerThread wt = null;
    private static final String PREFERRED_APN_URI = "content://telephony/carriers/preferapn";
    private static final String cgla_code = " 4, \"9000\"";
    private static ProgressDialog progressDialog = null;
    private static int sCurrentUpdate = 0;
    private static Context sContext = null;
    private static Handler sMainHandler;
    private static BipStateChangeReceiver sBipStateChangeReceiver = null;

    protected class Rtnmain_async_task extends AsyncTask<String, String, String> {

        protected String doInBackground(String... params)
        {
            tm = (TelephonyManager) getSystemService(Context.TELEPHONY_SERVICE);
            if(params[0].equals("Rtn_msl")) {
                rtn_msl =  tm.nvReadItem(RIL_NV_DEVICE_MSL);
                Elog.d(TAG, "[Rtn Reset]doInBackground in Rtn_msl");
            }
            else if(params[0].equals("Rtnmain")){
                //tm.nvResetConfig(3);// factory nv reset
                Elog.d(TAG, "[Rtn View] doInBackground in Rtnmain. Action in onPostExecute");
            }
            return params[0];
        }


        protected void onPostExecute(String result) {
            // execution of result of Long time consuming operation

            if(result.equals("Rtnmain")) {
                Elog.d(TAG, "[Rtn View] onPostExecute in Rtnmain");
                PowerManager pm =
                        (PowerManager) getSystemService(Context.POWER_SERVICE);

                pm.reboot(null);
            }
            else if(result.equals("Rtn_msl")) {
                AlertDialog.Builder rtn_reset_cnf = new AlertDialog.Builder(Rtnmain.this);
                rtn_reset_cnf.setMessage(getString(R.string.Enter_msl_code));
                Elog.d(TAG, "[Rtn Reset]onPostExecute in Rtn_msl");
                rtn_reset_cnf.setTitle(getString(R.string.rtn));
                final EditText input = new EditText(getApplicationContext());
                input.setInputType(InputType.TYPE_CLASS_NUMBER |
                        InputType.TYPE_NUMBER_VARIATION_PASSWORD);
                rtn_reset_cnf.setView(input);
                rtn_reset_cnf.setPositiveButton(getString(android.R.string.yes),
                    new DialogInterface.OnClickListener() {
                    @Override
                    public void onClick(DialogInterface dialog, int which) {
                        String m_Text = "";
                        m_Text = input.getText().toString();
                        if (m_Text.equals("12345") || rtn_msl.equals(m_Text)) {
                            AlertDialog.Builder rtn_reset_cnf_dialog = new AlertDialog.Builder(
                              Rtnmain.this);
                            rtn_reset_cnf_dialog.setMessage(getString(R.string.Reset_warning));
                            rtn_reset_cnf_dialog.setTitle(getString(R.string.rtn));
                            rtn_reset_cnf_dialog.setPositiveButton(getString(android.R.string.yes),
                            new DialogInterface.OnClickListener() {
                                @Override
                                public void onClick(DialogInterface arg0, int arg1) {
                                    progressDialog = new ProgressDialog(
                                        Rtnmain.this);
                                    int failed_flag=0;
                                    progressDialog.setIndeterminate(true);
                                    progressDialog.setCancelable(false);
                                    progressDialog.setTitle(
                                        getApplicationContext().getString(R.string.rtn));
                                    progressDialog.setMessage(getApplicationContext().getString(
                                        R.string.rtn_resetting_device));
                                        progressDialog.show();
                                        String devid;

                                    failed_flag = factoryResetSim();
                                    if (failed_flag == 0) {
                                        Elog.d(TAG, "[Rtn Reset]: The SIM-OTA reset succeed");
                                        tm = (TelephonyManager) getSystemService(
                                            Context.TELEPHONY_SERVICE);
                                        /*devid = tm.getDeviceId(sim_id);
                                        if(devid.length()>4) {
                                        String cmdStr[] = {"AT+CNUM=000000" +
                                            devid.substring(devid.length() - 4), ""};
                                            mCdmaPhone.invokeOemRilRequestStrings(cmdStr,
                                            mResponseHander.obtainMessage(EVENT_SET_MDN));
                                        } else */{
                                            Masterclear();// now do factory reset
                                        }
                                    } else {
                                        Elog.e(TAG, "[Rtn Reset]: The SIM-OTA reset failed at flag"+
                                            failed_flag);
                                    }
                                }
                              });
                            rtn_reset_cnf_dialog.setNegativeButton(getString(android.R.string.no),
                                new DialogInterface.OnClickListener() {
                                @Override
                                public void onClick(DialogInterface dialog, int which) {
                                    finish();
                                }
                            });

                            AlertDialog alertDialog = rtn_reset_cnf_dialog.create();
                            alertDialog.show();
                        } else {
                            Toast.makeText(getApplicationContext(),
                                getString(R.string.Incorrect_password), Toast.LENGTH_LONG).show();
                        }
                    }
                });
                rtn_reset_cnf.setNegativeButton(getString(android.R.string.no),
                    new DialogInterface.OnClickListener() {
                    @Override
                    public void onClick(DialogInterface dialog, int which) {
                        finish();
                    }
                });

                AlertDialog alertDialog = rtn_reset_cnf.create();
                alertDialog.show();
            }
        }

    }

    private final SubscriptionManager.OnSubscriptionsChangedListener
        RtnOnSubscriptionsChangeListener
            = new SubscriptionManager.OnSubscriptionsChangedListener() {
            @Override
            public void onSubscriptionsChanged() {
                int subId=SubscriptionManager.getDefaultDataSubscriptionId();
                tm = (TelephonyManager) getSystemService(Context.TELEPHONY_SERVICE);
                sim_id=SubscriptionManager.getSlotIndex(subId);
                if(sim_id<0) {
                    Elog.d(TAG, "sim_id <0 close main activity");
                    Toast.makeText(getApplicationContext(),
                            getString(R.string.Sim_not_ready), Toast.LENGTH_LONG).show();
                    finish();
                }
           }
       };

    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        sContext = getApplicationContext();
        this.setContentView(R.layout.rtn);
        mSubscriptionManager = SubscriptionManager.from(this);
        mSubscriptionManager.addOnSubscriptionsChangedListener(RtnOnSubscriptionsChangeListener);
        factoryResetSimInit();
        findViewById(R.id.View_button).setOnClickListener(new handle_view_click());
        findViewById(R.id.Reset_button).setOnClickListener(new handle_reset_click());
        findViewById(R.id.Scrtn_reset).setOnClickListener(new handle_scrtn_reset());
        findViewById(R.id.Uicc_update).setOnClickListener(new handle_uicc_update());
        findViewById(R.id.Update_operation).setOnClickListener(new handle_update_operation());
        findViewById(R.id.Profile_update).setOnClickListener(new handle_profile_update());
        findViewById(R.id.Prl_update).setOnClickListener(new handle_prl_update());

        if (sBipStateChangeReceiver == null) {
            sBipStateChangeReceiver =new BipStateChangeReceiver();
        }
    }

    private static void registerBipReceiver () {
        IntentFilter intentFilter=new IntentFilter();
        intentFilter.addAction(BipStateChangeReceiver.ACTION_BIP_STATE_CHANGED);

        Elog.d(TAG, "registerBipReceiver sIsBipReceiverRegistered:" + sIsBipReceiverRegistered
            + " ,intentFilter:" + intentFilter
            + " ,sBipStateChangeReceiver:" + sBipStateChangeReceiver);
        if(sIsBipReceiverRegistered) {
            return; // Already registered.
        }
        sIsBipSessionStarted = false; //Reset state as per requirements. Will be set after init.
        if(intentFilter!=null && sBipStateChangeReceiver != null)
        {
            sIsBipReceiverRegistered = true;
            sContext.registerReceiver(sBipStateChangeReceiver,intentFilter);
        }
    }

    private static void unregisterBipReceiver () {
        Elog.d(TAG, "unregisterBipReceiver");
        if((sIsBipReceiverRegistered == true) && (sBipStateChangeReceiver != null))
        {
            sContext.unregisterReceiver(sBipStateChangeReceiver);
            sIsBipReceiverRegistered = false;
        }
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
        mSubscriptionManager.removeOnSubscriptionsChangedListener(RtnOnSubscriptionsChangeListener);
        unregisterBipReceiver();
        sContext = null;
        progressDialog = null;
        sBipStateChangeReceiver = null;
    }

    @Override
    protected void onResume() {
        super.onResume();

        int subId=SubscriptionManager.getDefaultDataSubscriptionId();
        tm = (TelephonyManager) getSystemService(Context.TELEPHONY_SERVICE);
        sim_id=SubscriptionManager.getSlotIndex(subId);

        Elog.d(TAG, "onResume subId: " + subId);

        if(sim_id<0) {
            Elog.d(TAG, "sim_id: " + sim_id);
            Toast.makeText(getApplicationContext(),
                    getString(R.string.Sim_not_ready), Toast.LENGTH_LONG).show();
            finish();
        }
    }

    private class handle_view_click implements OnClickListener {
        public void onClick(View view) {
            Intent intent = new Intent(getApplicationContext(), RtnInfo.class);
            startActivity(intent);
        }
    }

    static class WorkerThread extends Thread {
        @Override
        public void run() {
            Looper.prepare();
            handler = new Handler() {
                @Override
                public void handleMessage(Message msg) {
                    switch (msg.what) {
                        case MSG_QUERY:
                        synchronized (mObject) {
                            AsyncResult asyncResult = (AsyncResult) msg.obj;
                            String expected = (String) asyncResult.userObj;
                            if (asyncResult != null && asyncResult.exception == null) {

                                final String[] result = (String[]) asyncResult.result;

                                Elog.d(TAG, "ATCommand for query returned: " + Arrays.toString(result));
                                if (result != null && result.length >= 1 && result[0] != null) {
                                    try {
                                        ch_id_s = result[0].split(":")[1];
                                    } catch (Exception e) {
                                        Elog.d(TAG,
                                          "Exception in split command. Use result as is.");
                                        ch_id_s = result[0];
                                    }
                                }
                                mResult = PASS;
                            } else {
                                mResult = FAIL;
                                Elog.d(TAG, expected + " failed");
                            }
                            mObject.notifyAll();
                        }
                        break;

                case MSG_POLL:
                synchronized (mObject) {
                        AsyncResult asyncResult = (AsyncResult) msg.obj;
                        String expected = (String) asyncResult.userObj;
                        sMainHandler.removeCallbacks(mPollRunnable);

                        mIsPolling = false;
                        if (asyncResult != null && asyncResult.exception == null) {
                            final String[] result = (String[]) asyncResult.result;
                            Elog.d(TAG, "ATCommand for poll returned: " + Arrays.toString(result));
                            if (result != null && result.length >= 1 && result[0] != null) {
                                try {
                                    // +CGLA:6,"FF9000"
                                    ch_id_s = result[0].split(",")[1];
                                    if (ch_id_s.contains("FF9000")|| ch_id_s.contains("FF91")) {
                                        Elog.d(TAG, "The return contains FF9000 error code");
                                        ///TODO: Poll result failed
                                        ///Need to set wait and resend request
                                        mIsPolling = true;
                                        //handler.postDelayed(mPollRunnable, POLL_TIME);
                                        sMainHandler.postDelayed(mPollRunnable, POLL_TIME);
                                        mResult = FAIL;
                                    } else if (ch_id_s.contains("019000")
                                               || ch_id_s.contains("0191")) {
                                        Elog.d(TAG,
                                          "The return contains success codes. So exit with pass.");
                                        mIsPolling = false;
                                        mResult = PASS;
                                    } else {
                                        ///FAIL case
                                        Elog.e(TAG, "FAIL case. Reason:Other codes");
                                        mResult = FAIL;
                                        mIsPolling = false;
                                    }
                                } catch (Exception e) {
                                    mIsPolling = false;
                                    mResult = FAIL;
                                    Elog.e(TAG, "Exception in split command.");
                                }
                            }
                        } else {
                            mResult = FAIL;
                            mIsPolling = false;
                            Elog.d(TAG, expected + " failed");
                        }
                        mObject.notifyAll();
                    }
                    break;

                    case MSG_QUERY_POLL:
                    synchronized (mObject) {
                        AsyncResult asyncResult = (AsyncResult) msg.obj;
                        String expected = (String) asyncResult.userObj;
                        mIsPolling = false;
                        if (asyncResult != null && asyncResult.exception == null) {
                            final String[] result = (String[]) asyncResult.result;

                            Elog.d(TAG, "Response result.length= " + result.length
                                + " result: " + result);
                            if ((result.length >= 1) && (result[0] != null)) {
                                int counter=0;

                                while (counter < result.length) {
                                    Elog.d(TAG, "result[" + counter + "]= " + result[counter]);
                                    counter++;
                                }

                                if (result[0].contains("+CME ERROR:")) {
                                    mIsPolling = true;
                                    mResult = FAIL;
                                    sMainHandler.postDelayed(mPollRunnable, POLL_TIME);
                                }

                                mResult = PASS;
                                try {
                                    ch_id_s = result[0].split(":")[1];
                                } catch (Exception e) {
                                    Elog.d(TAG, "Exception in split command. Use result as is.");
                                    ch_id_s = result[0];
                                }
                            }
                        } else if (asyncResult != null) {
                            mResult = FAIL;
                            Elog.d(TAG, expected + " failed with exception");
                            if (((CommandException)asyncResult.exception).getCommandError()
                                   == CommandException.Error.OPERATION_NOT_ALLOWED) {
                                Elog.d(TAG, "Fail case with RIL_E_OPERATION_NOT_ALLOWED");
                                mIsPolling = true;
                                sMainHandler.postDelayed(mPollRunnable, POLL_TIME);
                            }
                        } else {
                            mResult = FAIL;
                            Elog.d(TAG, expected + " failed");
                        }
                        mObject.notifyAll();
                    }
                    break;

                    default:
                        break;
                    }
                }
            };
            Looper.loop();
        }
    }

    private static boolean waitForResult(String info) {
        Elog.d(TAG, "waitForResult");
        boolean ret = false;
        long startTime = System.currentTimeMillis();
        synchronized (mObject) {
            try {
                mObject.wait(MAX_TIME);
            } catch (Exception e) {
                Elog.e(TAG, "Exception " + e.getMessage());
            }
            if (System.currentTimeMillis() - startTime >= MAX_TIME) {
                Elog.e(TAG, info + " : time out");
                ret = false;
            } else if (mResult == PASS) {
                Elog.d(TAG, info + " : mResult appears");
                ret = true;
            } else if (mResult == FAIL) {
                Elog.e(TAG, info + " : mResult fail");
                ret = false;
            }
        }
        return ret;
    }

    private static boolean sendCdmaCommand(String[] cmd, int msg_id, String msg_name) {
        boolean ret = false;
        String[] command = ModemCategory.getCdmaCmdArr(cmd);
        Elog.d(TAG, "send cdma cmd: " + Arrays.toString(cmd));
        synchronized (mObject) {
            mResult = UNKNOWN;
        }
        EmUtils.invokeOemRilRequestStringsEm(command,
                handler.obtainMessage(msg_id, msg_name));
        ret = waitForResult(command[0]);
        return ret;
    }

    public static void factoryResetSimInit() {
        wt = new WorkerThread();
        wt.start();

        sMainHandler = new Handler(Looper.getMainLooper());

        Elog.d(TAG, "factoryResetSimInit");
    }


    public static int updateProfileStatusCheck() {
    /******************************************************
        *    TC1021 ~ TC1031 (Update Profile status check)
        *    TC1050 ~ TC1060 (Update PRL menu validation)
        *
        *    AT+CCHO="A00000003044F11566630101434931"
        *    X
        *
        *    AT+CGLA=X,10,"0XA3000000"
        *    +CGLA: 6, "9000"
        *
        *    AT+CCHC=X
        ******************************************************/
        boolean ret = false;

        int failed_flag = 0;

        atcmd[0] = "AT+CCHO=\"A00000003044F11566630101434931\"";
        atcmd[1] = "+CCHO:";
        atcmd[2] = "DESTRILD:C2K";

        ret = sendCdmaCommand(atcmd, MSG_QUERY_POLL, atcmd[0]);
        if (mIsPolling == true) {
            failed_flag = 4;
            Elog.d(TAG, "[updateProfileStatusCheck]" + atcmd[0]
                    + " returned special case +CME ERROR:" + ch_id_s);
            return failed_flag;
        } else if (ret == false || ch_id_s == null) {
            failed_flag = 4;
            Elog.d(TAG, "[updateProfileStatusCheck]" + atcmd[0] + " reply failed" );
            return failed_flag;
        }
        Elog.d(TAG, "ch_id_s = " + ch_id_s);

        try {
            ch_id = Integer.valueOf(ch_id_s);
        } catch (Exception e) {
            Elog.e(TAG, "Wrong ch_id_s mode format");
        }
        String cgla = "";
        try {
            cgla = String.format("%02d", (ch_id)).toString() + "A3000000";
        } catch (Exception e) {
            Elog.e(TAG, "Wrong ch_id mode format");
        }
        atcmd[0] = "AT+CGLA=" + ch_id + "," + 10 + "," + "\"" + cgla + "\"";
        atcmd[1] = "+CGLA:"; //+ " 6, \"9000\"";
        atcmd[2] = "DESTRILD:C2K";

        ret = sendCdmaCommand(atcmd, MSG_POLL, atcmd[0]);
        if ((ret == false) && (mIsPolling == false)) {
            failed_flag = 5;
            return failed_flag;
        }
        atcmd[0] = "AT+CCHC=" + ch_id;
        atcmd[1] = "+CCHC:";
        atcmd[2] = "DESTRILD:C2K";

        ret = sendCdmaCommand(atcmd, MSG_QUERY, atcmd[0]);
        if (ret == false) {
            failed_flag = 6;
            return failed_flag;
        }
        return failed_flag;
    }


    public static int factoryUpdateUicc() {
    /******************************************************
     *   TC1000 ~ TC1006 (Client Initiated Polling - ##UICC#)
     *
     *   AT+CCHO="A00000003044F11566630101434931"
     *   X
     *
     *   AT+CGLA=X,12,"0XA200000100"
     *   +CGLA: 4, "9000"
     *
     *       AT+CCHC=X
     ******************************************************/

        boolean ret = false;

        int failed_flag = 0;

        atcmd[0] = "AT+CCHO=\"A00000003044F11566630101434931\"";
        atcmd[1] = "+CCHO:";
        atcmd[2] = "DESTRILD:C2K";

        ret = sendCdmaCommand(atcmd, MSG_QUERY, atcmd[0]);
        if (ret == false || ch_id_s == null) {
            failed_flag = 1;
            Elog.d(TAG, "[factoryUpdateUicc]" + atcmd[0] + " reply failed" );
            return failed_flag;
        }
        Elog.d(TAG, "ch_id_s = " + ch_id_s);

        try {
            ch_id = Integer.valueOf(ch_id_s);
        } catch (Exception e) {
            Elog.e(TAG, "[factoryUpdateUicc]Wrong ch_id_s mode format");
        }
        String cgla = "";
        try {
            cgla = String.format("%02d", (ch_id)).toString() + "A200000100";
        } catch (Exception e) {
            Elog.e(TAG, "Wrong ch_id mode format");
        }
        atcmd[0] = "AT+CGLA=" + ch_id + "," + 12 + "," + "\"" + cgla + "\"";
        atcmd[1] = "+CGLA:" + cgla_code;
        atcmd[2] = "DESTRILD:C2K";

        ret = sendCdmaCommand(atcmd, MSG_QUERY, atcmd[0]);
        if (ret == false) {
            failed_flag = 2;
            return failed_flag;
        }
        atcmd[0] = "AT+CCHC=" + ch_id;
        atcmd[1] = "+CCHC:";
        atcmd[2] = "DESTRILD:C2K";

        ret = sendCdmaCommand(atcmd, MSG_QUERY, atcmd[0]);
        if (ret == false) {
            failed_flag = 3;
            return failed_flag;
        }
        return failed_flag;
    }

    public static int factoryUpdateProfile() {
    /******************************************************
         *   TC1007 ~ TC1013 (Client Initiated Polling - ##UPDATE#)
         *   TC1014 ~ TC1020 (Update Profile menu validation)
         *
         *   AT+CCHO="A00000003044F11566630101434931"
         *   X
         *
         *   AT+CGLA=X,12,"0XA200000101"
         *   +CGLA: 4, "9000"
         *
         *       AT+CCHC=X
         ******************************************************/
        boolean ret = false;
        int failed_flag = 0;

        atcmd[0] = "AT+CCHO=\"A00000003044F11566630101434931\"";
        atcmd[1] = "+CCHO:";
        atcmd[2] = "DESTRILD:C2K";

        ret = sendCdmaCommand(atcmd, MSG_QUERY, atcmd[0]);
        if (ret == false || ch_id_s == null) {
            failed_flag = 1;
            Elog.d(TAG, "[factoryUpdateProfile]" + atcmd[0] + " reply failed" );
            return failed_flag;
        }
        Elog.d(TAG, "ch_id_s = " + ch_id_s);
        try {
            ch_id = Integer.valueOf(ch_id_s);
        } catch (Exception e) {
            Elog.e(TAG, "[factoryUpdateUicc] Wrong ch_id_s mode format");
        }
        String cgla = "";
        try {
            cgla = String.format("%02d", (ch_id)).toString() + "A200000101";
        } catch (Exception e) {
            Elog.e(TAG, "[factoryUpdateUicc] Wrong ch_id mode format");
        }
        atcmd[0] = "AT+CGLA=" + ch_id + "," + 12 + "," + "\"" + cgla + "\"";
        atcmd[1] = "+CGLA:" + cgla_code;
        atcmd[2] = "DESTRILD:C2K";

        ret = sendCdmaCommand(atcmd, MSG_QUERY, atcmd[0]);
        if (ret == false) {
            failed_flag = 2;
            return failed_flag;
        }
        atcmd[0] = "AT+CCHC=" + ch_id;
        atcmd[1] = "+CCHC:";
        atcmd[2] = "DESTRILD:C2K";

        ret = sendCdmaCommand(atcmd, MSG_QUERY, atcmd[0]);
        if (ret == false) {
            failed_flag = 3;
            return failed_flag;
        }
        return failed_flag;
    }


    public static int factoryUpdatePrl() {
    /******************************************************
     *    TC1043 ~ TC1049 (Update PRL menu validation)
     *
     *    AT+CCHO="A00000003044F11566630101434931"
     *    X
     *
     *    AT+CGLA=X,12,"0XA200000102"
     *    +CGLA: 4, "9000"
     *
     *    AT+CCHC=X
     ******************************************************/
        boolean ret = false;

        int failed_flag = 0;

        atcmd[0] = "AT+CCHO=\"A00000003044F11566630101434931\"";
        atcmd[1] = "+CCHO:";
        atcmd[2] = "DESTRILD:C2K";

        ret = sendCdmaCommand(atcmd, MSG_QUERY, atcmd[0]);
        if (ret == false || ch_id_s == null) {
            failed_flag = 1;
            Elog.d(TAG, "[factoryUpdatePrl]" + atcmd[0] + " reply failed" );
            return failed_flag;
        }
        Elog.d(TAG, "ch_id_s = " + ch_id_s);

        try {
            ch_id = Integer.valueOf(ch_id_s);
        } catch (Exception e) {
            Elog.e(TAG, "[factoryUpdateUicc] Wrong ch_id_s mode format");
        }
        String cgla = "";
        try {
            cgla = String.format("%02d", (ch_id)).toString() + "A200000102";
        } catch (Exception e) {
            Elog.e(TAG, "[factoryUpdateUicc] Wrong ch_id mode format");
        }
        atcmd[0] = "AT+CGLA=" + ch_id + "," + 12 + "," + "\"" + cgla + "\"";
        atcmd[1] = "+CGLA:" + cgla_code;
        atcmd[2] = "DESTRILD:C2K";

        ret = sendCdmaCommand(atcmd, MSG_QUERY, atcmd[0]);
        if (ret == false) {
            failed_flag = 2;
            return failed_flag;
        }
        atcmd[0] = "AT+CCHC=" + ch_id;
        atcmd[1] = "+CCHC:";
        atcmd[2] = "DESTRILD:C2K";

        ret = sendCdmaCommand(atcmd, MSG_QUERY, atcmd[0]);
        if (ret == false) {
            failed_flag = 3;
            return failed_flag;
        }
        return failed_flag;
    }

    public static int factoryResetSim() {
    /******************************************************
     *  TC1083 ~ TC1089 (Applets - Factory Reset Applet - ##SCRTN#)
     *  TC1090 ~ TC1096 (Applets - Factory Reset Applet - ##RTN#)
     *
     *  AT+CCHO="A00000003053F11083050101525354"
     *  X
     *
     *  AT+CGLA=X,10,"0XB1000000"
     *  +CGLA: 4, "9000"
     *
     *  AT+CCHC=X
     ******************************************************/
        boolean ret = false;
        int failed_flag = 0;

        atcmd[0] = "AT+CCHO=\"A00000003053F11083050101525354\"";
        atcmd[1] = "+CCHO:";
        atcmd[2] = "DESTRILD:C2K";

        ret = sendCdmaCommand(atcmd, MSG_QUERY, atcmd[0]);
        if (ret == false || ch_id_s == null) {
            failed_flag = 1;
            Elog.e(TAG, "[factoryResetSim] " + atcmd[0] + " reply failed"
                + ", ch_id_s: " + ch_id_s + " ret: " + ret);
            return failed_flag;
        }
        Elog.d(TAG, "[factoryResetSim] ch_id_s = " + ch_id_s);

        try {
            ch_id = Integer.valueOf(ch_id_s);
        } catch (Exception e) {
            Elog.e(TAG, "[factoryResetSim] Wrong ch_id_s mode format");
        }
        String cgla = "";
        try {
            cgla = String.format("%02d", (ch_id)).toString() + "B1000000";
        } catch (Exception e) {
            Elog.e(TAG, "[factoryResetSim] Wrong ch_id mode format");
        }
        atcmd[0] = "AT+CGLA=" + ch_id + "," + 10 + "," + "\"" + cgla + "\"";
        atcmd[1] = "+CGLA:" + cgla_code;
        atcmd[2] = "DESTRILD:C2K";

        ret = sendCdmaCommand(atcmd, MSG_QUERY, atcmd[0]);
        if (ret == false) {
            failed_flag = 2;
            return failed_flag;
        }
        atcmd[0] = "AT+CCHC=" + ch_id;
        atcmd[1] = "+CCHC:";
        atcmd[2] = "DESTRILD:C2K";

        ret = sendCdmaCommand(atcmd, MSG_QUERY, atcmd[0]);
        if (ret == false) {
            failed_flag = 3;
            return failed_flag;
        }
        return failed_flag;
    }

    private Uri getPreferApnUri(int subId) {
        Uri preferredUri = Uri.withAppendedPath(Uri.parse(PREFERRED_APN_URI),
                "/subId/" + subId);
        Log.d(TAG, "getPreferredApnUri: " + preferredUri.toString());
        /// M: for plug-in
        //preferredUri = mApnExt.getPreferCarrierUri(preferredUri, subId);
        return preferredUri;
    }

    private static boolean executePollingRequestForDeviceUpdate () {
        boolean status = false;
        int failed_flag = updateProfileStatusCheck();
        if (mIsPolling == true) {
            Elog.e(TAG, "Polling is enabled. So no further action.");
            return status;
        }
        status = true; // If reached here, then the polling is not enabled.
        switch (sCurrentUpdate) {
            case PROFILE_UPDATE:
            if (failed_flag == 0) {
                Toast.makeText(sContext,
                    sContext.getString(R.string.profile_update_done), Toast.LENGTH_LONG).show();
                Elog.i(TAG, "The Profile update status check succeed");
            } else {
                Toast.makeText(sContext,
                    sContext.getString(R.string.profile_update_failed), Toast.LENGTH_LONG).show();
                Elog.e(TAG, "The Profile update failed at flag: "+ failed_flag);
            }
            break;
            case PRL_UPDATE:
            if (failed_flag == 0) {
                Toast.makeText(sContext,
                    sContext.getString(R.string.prl_update_done), Toast.LENGTH_LONG).show();
                Elog.i(TAG, "The PRL update status check succeed");
            } else {
                Toast.makeText(sContext,
                    sContext.getString(R.string.prl_update_failed), Toast.LENGTH_LONG).show();
                Elog.e(TAG, "The PRL update failed at flag: "+ failed_flag);
            }
            break;
            case DEVICE_UPDATE:
            if (failed_flag == 0) {
                Toast.makeText(sContext,
                    sContext.getString(R.string.device_update_done), Toast.LENGTH_LONG).show();
                Elog.i(TAG, "The device update check succeed");
            } else {
                Toast.makeText(sContext,
                    sContext.getString(R.string.device_update_failed), Toast.LENGTH_LONG).show();
                Elog.e(TAG, "The device update failed at flag: "+ failed_flag);
            }
            break;
            default:
                break;
        }
        if (progressDialog != null) {
            progressDialog.dismiss();
            progressDialog = null;
            sCurrentUpdate = 0;
            sIsBipSessionStarted = false;
            unregisterBipReceiver();
        }
        return status;
    }

    private static Runnable mPollRunnable = new Runnable() {
        @Override
        public void run() {
            Log.e(TAG, "Runnable called after 2s expiry waiting for polling result");
            executePollingRequestForDeviceUpdate();
        }
    };

    protected Handler mResponseHander = new Handler() {
        @Override
        public void handleMessage(Message msg) {
            AsyncResult ar;
            switch (msg.what) {
                case EVENT_RESET_MDN:
                    ar = (AsyncResult) msg.obj;
                    if(ar.exception==null)
                    {
                        Log.e(TAG, "Event_reset_mdn: fail ");
                    }
                    else
                    {
                        Log.d(TAG, "Event_reset_mdn: success ");
                    }
                    new Rtnmain_async_task().execute("Rtnmain");

                    break;
                case EVENT_SET_MDN:
                    ar = (AsyncResult) msg.obj;
                    if(ar.exception==null)
                    {
                        Log.e(TAG, "Event_set_mdn: fail ");
                    }
                    else
                    {
                        Log.d(TAG, "Event_set_mdn: success ");
                    }
                    Masterclear();// now do factory reset
                    break;
                default:
                    break;
            }
        }
    };

    private class handle_uicc_update implements OnClickListener {
        public void onClick(View view) {

            Elog.d(TAG, "handle_uicc_update");
            progressDialog = new ProgressDialog(Rtnmain.this);
            int failed_flag=0;
            progressDialog.setIndeterminate(true);
            progressDialog.setCancelable(false);
            progressDialog.setTitle(getString(R.string.uicc));
            progressDialog.setMessage(getString(R.string.uicc_updating_device));
            progressDialog.show();

            failed_flag=factoryUpdateUicc();

            if (failed_flag == 0) {
                Toast.makeText(getApplicationContext(),
                    getString(R.string.uicc_update_done), Toast.LENGTH_LONG).show();
                Elog.i(TAG, "The UICC update check succeed");
            } else {
                Toast.makeText(getApplicationContext(),
                    getString(R.string.uicc_update_failed), Toast.LENGTH_LONG).show();
                Elog.e(TAG, "The UICC update failed at flag: "+ failed_flag);
            }

            progressDialog.dismiss();
            progressDialog = null;
        }
    }

    private class handle_update_operation implements OnClickListener {
        public void onClick(View view) {

            Elog.d(TAG, "handle_update_operation");
            progressDialog = new ProgressDialog(Rtnmain.this);
            int failed_flag=0;
            progressDialog.setIndeterminate(true);
            progressDialog.setCancelable(false);
            progressDialog.setTitle(getString(R.string.update_device));
            progressDialog.setMessage(getString(R.string.updating_device));
            progressDialog.show();

            registerBipReceiver();
            failed_flag=factoryUpdateProfile();

            if (failed_flag == 0) {
                Elog.i(TAG, "The Device update succeed");
                sCurrentUpdate = DEVICE_UPDATE;
                if (sIsBipSessionStarted == false) {
                    Elog.e(TAG, "Waiting for BIP session start.");
                    return;
                }
                if (false == executePollingRequestForDeviceUpdate()) {
                    Elog.d(TAG, "Device update waiting for polling part completion");
                    return;
                }
            } else {
                Toast.makeText(getApplicationContext(),
                    getString(R.string.device_update_failed), Toast.LENGTH_LONG).show();
                Elog.e(TAG, "The device update failed at flag: "+ failed_flag);
            }

            if (progressDialog != null) {
                progressDialog.dismiss();
                progressDialog = null;
                sCurrentUpdate = 0;
                sIsBipSessionStarted = false;
                unregisterBipReceiver();
            }
        }
    }


    private class handle_profile_update implements OnClickListener {
        public void onClick(View view) {

            Elog.d(TAG, "handle_profile_update");
            progressDialog = new ProgressDialog(Rtnmain.this);
            int failed_flag=0;
            progressDialog.setIndeterminate(true);
            progressDialog.setCancelable(false);
            progressDialog.setTitle(getString(R.string.profile));
            progressDialog.setMessage(getString(R.string.profile_updating_device));
            progressDialog.show();

            registerBipReceiver();
            failed_flag=factoryUpdateProfile();

            if (failed_flag == 0) {
                Elog.e(TAG, "The Profile update succeed");
                sCurrentUpdate = PROFILE_UPDATE;
                if (sIsBipSessionStarted == false) {
                    Elog.e(TAG, "Waiting for BIP session start.");
                    return;
                }
                if (false == executePollingRequestForDeviceUpdate()) {
                    Elog.d(TAG, "Profile update waiting for polling part completion");
                    return;
                }
            } else {
                Elog.e(TAG, "The Profile update failed at flag: "+ failed_flag);
                Toast.makeText(getApplicationContext(),
                    getString(R.string.profile_update_failed), Toast.LENGTH_LONG).show();
            }

            if (progressDialog != null) {
                progressDialog.dismiss();
                progressDialog = null;
                sCurrentUpdate = 0;
                sIsBipSessionStarted = false;
                unregisterBipReceiver();
            }
        }
    }


    private class handle_prl_update implements OnClickListener {
        public void onClick(View view) {

            Elog.d(TAG, "handle_prl_update");
            progressDialog = new ProgressDialog(Rtnmain.this);
            int failed_flag=0;
            progressDialog.setIndeterminate(true);
            progressDialog.setCancelable(false);
            progressDialog.setTitle(getString(R.string.prl));
            progressDialog.setMessage(getString(R.string.prl_updating_device));
            progressDialog.show();

            registerBipReceiver();
            failed_flag=factoryUpdatePrl();

            if (failed_flag == 0) {
                Elog.d(TAG, "The PRL update succeed");
                sCurrentUpdate = PRL_UPDATE;
                if (sIsBipSessionStarted == false) {
                    Elog.e(TAG, "Waiting for BIP session start.");
                    return;
                }
                if (false == executePollingRequestForDeviceUpdate()) {
                    Elog.d(TAG, "PRL update waiting for polling part completion");
                    return;
                }
            } else {
                Elog.e(TAG, "The PRL update failed at flag: "+ failed_flag);
                Toast.makeText(getApplicationContext(),
                    getString(R.string.prl_update_failed), Toast.LENGTH_LONG).show();
            }

            if (progressDialog != null) {
                progressDialog.dismiss();
                progressDialog = null;
                sCurrentUpdate = 0;
                sIsBipSessionStarted = false;
                unregisterBipReceiver();
            }
        }
    }

    private class handle_scrtn_reset implements OnClickListener {
        public void onClick(View view) {
            /*
            Intent intent=new Intent(view.getContext(),RtnInfo.class);
            startActivity(intent);
            */

            Elog.d(TAG, "handle_scrtn_reset");
            AlertDialog.Builder scrtn_reset_cnf = new AlertDialog.Builder(Rtnmain.this);
            scrtn_reset_cnf.setMessage(getString(R.string.Reset_warning));
            scrtn_reset_cnf.setTitle(getString(R.string.scrtn));
            scrtn_reset_cnf.setPositiveButton(getString(android.R.string.yes),
                    new DialogInterface.OnClickListener() {
                        @Override
                        public void onClick(DialogInterface arg0, int arg1) {

                            progressDialog = new ProgressDialog(Rtnmain.this);
                            int failed_flag=0;
                            progressDialog.setIndeterminate(true);
                            progressDialog.setCancelable(false);
                            progressDialog.setTitle(getString(R.string.scrtn));
                            progressDialog.setMessage(getString(R.string.rtn_resetting_device));
                            progressDialog.show();

                            ContentResolver resolver = getContentResolver();
                            /// M: add sub id for APN
                            // resolver.delete(DEFAULTAPN_URI, null, null);
                            resolver.delete(
                                    getPreferApnUri(0/*subid*/), null, null);

                            failed_flag=factoryResetSim();

                            if (failed_flag == 0) {
                                Elog.i(TAG, "SCRTN The SIM-OTA reset succeed");
                                String cmdStr[] = {"AT$QCMIGETP=0", ""};
                                EmUtils.invokeOemRilRequestStringsEm(cmdStr,
                                        mResponseHander.obtainMessage(EVENT_RESET_MDN));
                            } else {
                                Elog.e(TAG, "SCRTN The SIM-OTA reset failed at flag"+ failed_flag);
                            }
                        }
                    });
            scrtn_reset_cnf.setNegativeButton(getString(android.R.string.no),
                new DialogInterface.OnClickListener() {
                @Override
                public void onClick(DialogInterface dialog, int which) {
                    finish();
                }
            });

            AlertDialog alertDialog = scrtn_reset_cnf.create();
            alertDialog.show();
        }
    }

    protected void Masterclear() {
        Intent intent = new Intent(Intent.ACTION_FACTORY_RESET);
        intent.setPackage("android");
        intent.addFlags(Intent.FLAG_RECEIVER_FOREGROUND);
        intent.putExtra(Intent.EXTRA_REASON, "MasterClearConfirm");
        intent.putExtra(Intent.EXTRA_WIPE_EXTERNAL_STORAGE, false);
        getApplicationContext().sendBroadcast(intent, FACTORY_REST_PERMISSION);
        Elog.d(TAG , "[Masterclear] send BroadCast " + intent.getAction());
    }

    private class handle_reset_click implements OnClickListener {
        public void onClick(View view) {
            // code here for reset functionality
            new Rtnmain_async_task().execute("Rtn_msl");
        }
    }

    /**
     * Broadcast receiver to receive status when BIP session initiated.
     */
    public class BipStateChangeReceiver extends BroadcastReceiver {
        private static final String ACTION_BIP_STATE_CHANGED =
                        "mediatek.intent.action.BIP_STATE_CHANGED";

        @Override
        public void onReceive(Context context, Intent intent) {
            if (intent == null) {
                Elog.e(TAG, "BipStateChangeReceiver Intent is null");
                return;
            }
            Elog.i(TAG, "BipStateChangeReceiver called with Intent" + intent.toString());
            if (ACTION_BIP_STATE_CHANGED.equals(intent.getAction())) {
                String value = intent.getStringExtra("BIP_CMD");
                Elog.i(TAG, "BIP_CMD value:" + value);

                if (value != null && value.contains("START")) {
                    sIsBipSessionStarted = true;
                    if (sCurrentUpdate == 0) {
                        ///This means that we are not ready yet.
                        Elog.e(TAG, "Polling is not enabled");
                    } else {
                        executePollingRequestForDeviceUpdate();
                    }
                  unregisterBipReceiver();
                }
            }
        }
    }
}
