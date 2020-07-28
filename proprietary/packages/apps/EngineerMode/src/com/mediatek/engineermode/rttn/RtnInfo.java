package com.mediatek.engineermode.rttn;

import android.app.Activity;
import android.app.ProgressDialog;
import android.app.AlertDialog;
import android.content.DialogInterface;
import android.app.usage.NetworkStats;
import android.content.Context;
import android.icu.text.SimpleDateFormat;
import android.os.AsyncTask;
import android.os.Build;
import android.os.Bundle;
import android.telephony.TelephonyManager;
import android.text.InputType;
import android.widget.TableRow;
import android.widget.TextView;

import com.mediatek.custom.CustomProperties;
import com.mediatek.engineermode.Elog;
import com.mediatek.engineermode.R;
/**
 * Created by MTK33301 on 7/13/2017.
 */

public class RtnInfo extends Activity {
    private static String TAG = "EM/Rtninfo";
    protected TableRow temp_row;
    protected String temp_text;
    protected String rtn_reconditioned_status;
    protected String rtn_activation_date;
    protected String rtn_life_timer;
    protected String rtn_life_calls;
    protected String rtn_life_data_rx="0";
    protected String rtn_life_data_tx="0";
    protected String rtn_life_hfa;
    protected String rtn_prl_version;
    protected long mobileBytes = 0;
    protected long mobileBytes_tx = 0;
    protected long mobileBytes_rx = 0;
    public static int RIL_NV_CDMA_MEID = 1;                   // CDMA MEID (hex)
    public static int RIL_NV_CDMA_MIN = 2;                    // CDMA MIN (MSID)
    public static int RIL_NV_CDMA_MDN = 3;                    // CDMA MDN

    public static int RIL_NV_RTN_RECONDITIONED_STATUS = 12;   // RTN reconditioned status
    public static int RIL_NV_RTN_ACTIVATION_DATE = 13;        // RTN activation date

    public static int RIL_NV_CDMA_PRL_VERSION = 51;        // RTN prl version
    public static int RIL_NV_RTN_LIFE_TIMER = 14;             // RTN life timer
    public static int RIL_NV_RTN_LIFE_CALLS = 15;             // RTN life calls
    public static int RIL_NV_RTN_LIFE_DATA_TX = 16;           // RTN life data TX
    public static int RIL_NV_RTN_LIFE_DATA_RX = 17;           // RTN life data RX
    public static int RIL_NV_OMADM_HFA_LEVEL = 18;            // HFA in progress
    protected String MyPhoneNumber = "0000000000000";
    private static final String PRLVERSION = "cdma.prl.version";
    protected String Subscriber_id;

    protected TextView tmp_text_view;
    protected NetworkStats.Bucket bucket = null;
    protected long call_duration = 0;
    protected long call_count = 0;
    protected TelephonyManager tm;

    public static final String HARDWARE_DEFAULT = "V1";
    @Override
    protected void onResume() {
        super.onResume();
        temp_row = (TableRow) findViewById(R.id.recoditioned_status_row);
        tmp_text_view = (TextView) temp_row.getChildAt(1);
        tmp_text_view.setText(rtn_reconditioned_status);



        temp_row = (TableRow) findViewById(R.id.life_data_row);
        tmp_text_view = (TextView) temp_row.getChildAt(1);
        tmp_text_view.setText((mobileBytes) + "kB");



        temp_row = (TableRow) findViewById(R.id.life_timer_row);
        tmp_text_view = (TextView) temp_row.getChildAt(1);
        tmp_text_view.setText(rtn_life_timer + " Minutes");

        temp_row = (TableRow) findViewById(R.id.life_calls_row);
        tmp_text_view = (TextView) temp_row.getChildAt(1);
        tmp_text_view.setText(rtn_life_calls + " Calls");

        temp_row = (TableRow) findViewById(R.id.PRL_row);
        tmp_text_view = (TextView) temp_row.getChildAt(1);
        tmp_text_view.setText(rtn_prl_version);

        temp_row = (TableRow) findViewById(R.id.activation_date_row);
        tmp_text_view = (TextView) temp_row.getChildAt(1);
        tmp_text_view.setText(rtn_activation_date);
    }

    public void showInfoDialog(Context context, String content) {
        AlertDialog.Builder builder = new AlertDialog.Builder(context);

        builder.setTitle("Warning");

        builder.setMessage(content).setCancelable(false)
                .setPositiveButton("OK", new DialogInterface.OnClickListener() {
                    public void onClick(DialogInterface dialog, int id) {
                        // TODO: handle the OK
                        InitRtnInfo();
                        dialog.cancel();
                    }
                })
                .setNegativeButton("Cancel",
                        new DialogInterface.OnClickListener() {
                            public void onClick(DialogInterface dialog, int id) {
                                dialog.cancel();
                                RtnInfo.this.finish();
                            }
                 });

        AlertDialog alertDialog = builder.create();
        alertDialog.show();
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {

        tm = (TelephonyManager) RtnInfo.this.getSystemService(Context.TELEPHONY_SERVICE);
        super.onCreate(savedInstanceState);
        this.setContentView(R.layout.rtn_info);
        showInfoDialog(this, "Allow EngineerMode to access your personal data?");

    }

    protected void InitRtnInfo() {
        temp_row = (TableRow) findViewById(R.id.mobile_num_row);
        tmp_text_view = (TextView) temp_row.getChildAt(1);

        try {

            MyPhoneNumber = tm.getLine1Number(Rtnmain.sim_id);
        } catch (NullPointerException ex) {
        }


        tmp_text_view.setText(MyPhoneNumber);
        Subscriber_id = tm.getSubscriberId(Rtnmain.sim_id);
        Elog.d(TAG, "Telephone number as follow " + tm.getLine1Number(Rtnmain.sim_id));

        temp_row = (TableRow) findViewById(R.id.msid_imsi_row);
        tmp_text_view = (TextView) temp_row.getChildAt(1);
        tmp_text_view.setText(tm.getSubscriberId(Rtnmain.sim_id));


        temp_row = (TableRow) findViewById(R.id.sw_version_row);
        tmp_text_view = (TextView) temp_row.getChildAt(1);
        tmp_text_view.setText(tm.getDeviceSoftwareVersion(Rtnmain.sim_id));
        Elog.d(TAG, "Device software  as follow " + tm.getDeviceSoftwareVersion(Rtnmain.sim_id));

        String strTime = new SimpleDateFormat("MM/dd/yyyy").format(Build.TIME);
        temp_row = (TableRow) findViewById(R.id.date_of_manufacture_row);
        tmp_text_view = (TextView) temp_row.getChildAt(1);
        tmp_text_view.setText(strTime);


        temp_row = (TableRow) findViewById(R.id.os_version_row);
        tmp_text_view = (TextView) temp_row.getChildAt(1);
        tmp_text_view.setText(android.os.Build.VERSION.RELEASE);
        Elog.d(TAG, "base os version as follow " + android.os.Build.VERSION.RELEASE);

        temp_row = (TableRow) findViewById(R.id.hw_version_row);
        tmp_text_view = (TextView) temp_row.getChildAt(1);
        tmp_text_view.setText(CustomProperties.MODULE_DM + " " + "Hw ver" + HARDWARE_DEFAULT);


        temp_row = (TableRow) findViewById(R.id.esn_meid_imei_row);
        tmp_text_view = (TextView) temp_row.getChildAt(1);
        tmp_text_view.setText(tm.getDeviceId(Rtnmain.sim_id));


        temp_row = (TableRow) findViewById(R.id.radio_version_row);
        tmp_text_view = (TextView) temp_row.getChildAt(1);
        tmp_text_view.setInputType(InputType.TYPE_TEXT_FLAG_MULTI_LINE);
        tmp_text_view.setText(android.os.Build.getRadioVersion());

        new Rtn_async_task().execute("rtn_reconditioned_status");
    }

    protected class Rtn_async_task extends AsyncTask<String, String, String> {
        protected ProgressDialog progressDialog;

        protected String doInBackground(String... params)
        {

            rtn_reconditioned_status = tm.nvReadItem(RIL_NV_RTN_RECONDITIONED_STATUS);
            rtn_activation_date = tm.nvReadItem(RIL_NV_RTN_ACTIVATION_DATE);
            rtn_prl_version = tm.nvReadItem(RIL_NV_CDMA_PRL_VERSION);
            rtn_life_timer = tm.nvReadItem(RIL_NV_RTN_LIFE_TIMER);
            rtn_life_data_rx = tm.nvReadItem(RIL_NV_RTN_LIFE_DATA_RX);
            rtn_life_calls = tm.nvReadItem(RIL_NV_RTN_LIFE_CALLS);

            if(rtn_life_data_rx!=null&&!rtn_life_data_rx.equals("")) {
                mobileBytes_rx = Long.parseLong(rtn_life_data_rx);
            }
            rtn_life_data_tx = tm.nvReadItem(RIL_NV_RTN_LIFE_DATA_TX);

            if(rtn_life_data_tx!=null&&!rtn_life_data_tx.equals("")) {
                mobileBytes_tx = Long.parseLong(rtn_life_data_tx);
            }
            return new String("unknown");
        }


        protected void onPostExecute(String result) {
            // execution of result of Long time consuming operation

            progressDialog.dismiss();
        }

        @Override
        protected void onPreExecute() {
            progressDialog = new ProgressDialog(RtnInfo.this);
            progressDialog.setIndeterminate(true);
            progressDialog.setCancelable(false);
            progressDialog.setTitle(getString(R.string.rtn_main));
            progressDialog.setMessage(getString(R.string.Wait_and_load));
            progressDialog.show();
        }

    }
/*
    @Override
    public void onResume(void) {
        super.onResume();

    }
*/
}
