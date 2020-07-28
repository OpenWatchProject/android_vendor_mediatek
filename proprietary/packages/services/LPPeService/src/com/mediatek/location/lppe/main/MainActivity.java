package com.mediatek.location.lppe.main;

import android.app.Activity;
import android.os.Bundle;
import android.util.Log;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.Button;
import android.widget.TextView;

import com.mediatek.location.lppe.main.LPPeService.LPPeServiceLogCallback;

public class MainActivity extends Activity {

    Button mButtonTest1;
    Button mButtonTest2;
    Button mButtonTest3;
    Button mButtonTest4;
    Button mButtonTest5;
    Button mButtonTest6;
    TextView mTextViewInfo;
    static TextStringList sInfo = new TextStringList(16);

    LPPeService lppe;
    LPPeServiceLogCallback lppeLog = new LPPeServiceLogCallback() {
        @Override
        public void onLog(boolean isError, String msg) {
            sInfo.print((isError ? "ERR: " : "") + msg);
        }
    };

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        lppe = new LPPeService(this);

        // TODO enable android.os.NetworkOnMainThreadException
        // StrictMode.ThreadPolicy policy = new
        // StrictMode.ThreadPolicy.Builder()
        // .permitAll().build();
        // StrictMode.setThreadPolicy(policy);

        mButtonTest1 = (Button) findViewById(R.id.button_1);
        mButtonTest2 = (Button) findViewById(R.id.button_2);
        mButtonTest3 = (Button) findViewById(R.id.button_3);
        mButtonTest4 = (Button) findViewById(R.id.button_4);
        mButtonTest5 = (Button) findViewById(R.id.button_5);
        mButtonTest6 = (Button) findViewById(R.id.button_6);
        mTextViewInfo = (TextView) findViewById(R.id.fused_textview_info);
        sInfo.updateUiThread();
        sInfo.clear();
        sInfo.setTextView(mTextViewInfo);

        mButtonTest1.setText("LPPe Start");
        mButtonTest1.setOnClickListener(new OnClickListener() {
            @Override
            public void onClick(View view) {
                lppe.triggerWifiStartMeas();
                lppe.triggerBtStartMeas();
                lppe.triggerSensorStartMeas();
                lppe.triggerNetworkStartMeas();
                lppe.triggerIpAddrRequestInformation();
            }
        });
        mButtonTest2.setText("LPPe Stop");
        mButtonTest2.setOnClickListener(new OnClickListener() {
            @Override
            public void onClick(View view) {
                lppe.triggerWifiStopMeas();
                lppe.triggerBtStopMeas();
                lppe.triggerSensorStopMeas();
                lppe.triggerNetworkStopMeas();
            }
        });
        mButtonTest3.setText("Request Capabilities");
        mButtonTest3.setOnClickListener(new OnClickListener() {
            @Override
            public void onClick(View view) {
                lppe.triggerWifiRequestCapabilities();
                lppe.triggerBtRequestCapabilities();
                lppe.triggerSensorRequestCapabilities();
                lppe.triggerNetworkRequestCapabilities();
                lppe.triggerIpAddrRequestCapabilities();
                lppe.triggerLbsRequestCapabilities();
            }
        });
        mButtonTest4.setText("recv SUPL INIT");
        mButtonTest4.setOnClickListener(new OnClickListener() {
            @Override
            public void onClick(View view) {
                lppe.triggerLbsReceiveSuplInit();
            }
        });
        mButtonTest5.setText("Sensor stop");
        mButtonTest5.setOnClickListener(new OnClickListener() {
            @Override
            public void onClick(View view) {
                lppe.triggerSensorStopMeas();
            }
        });
        mButtonTest6.setText("Clear");
        mButtonTest6.setOnClickListener(new OnClickListener() {
            @Override
            public void onClick(View view) {
                sInfo.clear();
            }
        });

        lppe.registerLog(lppeLog);
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
        log("onDestroy");
        if (lppe != null) {
            lppe.cleanup();
        }
    }

    public static void log(Object msg) {
        Log.d("hugo_app", "" + msg);
        sInfo.print("" + msg);
    }

    public static void msleep(long milliseconds) {
        try {
            Thread.sleep(milliseconds);
        } catch (InterruptedException e) {
            e.printStackTrace();
        }
    }
}
