package com.mediatek.engineermode.wificalling;

import android.app.Activity;
import android.app.ProgressDialog;
import android.os.AsyncTask;
import android.os.Bundle;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.Button;
import android.widget.EditText;
import android.widget.RadioGroup;
import android.widget.Toast;
import android.widget.RadioGroup.OnCheckedChangeListener;

import com.mediatek.engineermode.Elog;
import com.mediatek.engineermode.EmUtils;
import com.mediatek.engineermode.R;

public class EntitlementConfigActivity extends Activity{
    private static final String KEY_EPDG_ADDRESS = "persist.vendor.net.wo.epdg_fqdn";
    private static final String KEY_ENTITLEMENT_ENABLES = "persist.vendor.entitlement_enabled";
    private static final String KEY_ENTITLEMENT_SERVER = "persist.vendor.entitlement.sesurl";
    private static final String TAG = "EM/WifiCallingConfigActivity";
    private RadioGroup entitlementStatus;
    private boolean entitlementEnable;
    private EditText entitlementServerEt;
    private String entitlementServer;
    private Button setBtn;
    private Button getBtn;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.wificalling_config);
        initView();
        new DoSystemPropTask().execute("get");
    }


    private void initView() {
        entitlementStatus = (RadioGroup) findViewById(R.id.entitlement_status);
        OnCheckedChangeListener mCheckedChangeListener = new OnCheckedChangeListener() {

            @Override
            public void onCheckedChanged(RadioGroup group, int checkedId) {
                // TODO Auto-generated method stub
                switch (checkedId) {
                case R.id.entitlement_enable:
                    entitlementEnable = true;
                    break;
                case R.id.entitlement_disable:
                    entitlementEnable = false;
                    break;
                default:
                    break;
                }
            }
        };
        entitlementStatus.setOnCheckedChangeListener(mCheckedChangeListener);
        entitlementServerEt = (EditText) findViewById(R.id.entitlement_server);
        setBtn = (Button) findViewById(R.id.wificalling_set);
        setBtn.setOnClickListener(new OnClickListener() {

            @Override
            public void onClick(View v) {
                entitlementServer = entitlementServerEt.getText().toString();
                new DoSystemPropTask().execute("set");
            }
        });
        getBtn = (Button) findViewById(R.id.wificalling_get);
        getBtn.setOnClickListener(new OnClickListener() {

            @Override
            public void onClick(View v) {
                new DoSystemPropTask().execute("get");
            }
        });
    }

    protected class DoSystemPropTask extends AsyncTask<String, String, String> {

        protected String doInBackground(String... params)
        {
            if(params[0].equals("set")) {
                Elog.d(TAG, "[Set]entitlementServer: "
                        + entitlementServer + ",entitlementEnable:" + entitlementEnable);
                boolean set1 = EmUtils.systemPropertySet(KEY_ENTITLEMENT_SERVER, entitlementServer);
                boolean set2 = EmUtils.systemPropertySet(KEY_ENTITLEMENT_ENABLES,
                        entitlementEnable ? "1" : "0");
                if(set1 && set2) {
                    return "Set succeed!";
                }
                return "Set failed!";
            } else if(params[0].equals("get")) {
                entitlementServer = EmUtils.systemPropertyGet(KEY_ENTITLEMENT_SERVER, "");
                entitlementEnable = EmUtils.systemPropertyGet(KEY_ENTITLEMENT_ENABLES, "0")
                        .equals("1") ? true : false;
                return "get";
            }
            return "UnKnown";
        }

        protected void onPostExecute(String result) {
            // execution of result of Long time consuming operation
            if(result.equals("get")) {
                entitlementStatus.check(entitlementEnable ?
                        R.id.entitlement_enable : R.id.entitlement_disable);
                entitlementServerEt.setText(entitlementServer);
                Elog.d(TAG, "[Get]EntitlementServer: " + entitlementServer
                        + ",entitlementEnable:" + entitlementEnable);
            } else {
                Toast.makeText(getApplicationContext(),
                        result, Toast.LENGTH_LONG).show();
            }
        }
    }

}
