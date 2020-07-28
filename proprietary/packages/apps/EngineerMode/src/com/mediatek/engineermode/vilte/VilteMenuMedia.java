package com.mediatek.engineermode.vilte;

import android.app.Activity;
import android.os.Bundle;
import android.view.View;
import android.widget.Button;
import android.widget.EditText;
import android.widget.TextView;
import android.widget.Toast;

import com.mediatek.engineermode.Elog;
import com.mediatek.engineermode.EmUtils;
import com.mediatek.engineermode.R;


public class VilteMenuMedia extends Activity implements View.OnClickListener {
    private final String TAG = "Vilte/MenuMedia";

    private final String PROP_VILTE_OPTEST_MM = "persist.vendor.vt.OPTest_MM";
    private final String PROP_VILTE_OPTEST_RTP = "persist.vendor.vt.OPTest_RTP";
    private final String PROP_VILTE_OPTEST_MA = "persist.vendor.vt.OPTest_MA";
    private final String PROP_VILTE_VT_RTP_QDURHW = "persist.vendor.vt.rtpQDurHW";
    private final String PROP_VILTE_VT_DOWN_GRADE_TH = "persist.vendor.vt.DownGradeTH";
    private final String PROP_VILTE_VT_DE_START = "persist.vendor.vt.de_start";
    private final String PROP_VILTE_VT_DE_DIFF = "persist.vendor.vt.de_diff";
    private final String PROP_VILTE_VT_RTP_RES1 = "persist.vendor.vt.rtp_res1";
    private final String PROP_VILTE_VT_RTP_RES2 = "persist.vendor.vt.rtp_res2";

    private Button mVilte_operator_media_customize_mode_enable;
    private Button mVilte_operator_media_customize_mode_disable;
    private EditText mVilte_operator_media_rtp_values;
    private Button mVilte_operator_media_rtp_set;
    private EditText mVilte_operator_media_ma_values;
    private Button mVilte_operator_media_ma_set;
    private EditText mVilte_operator_media_tx_fallback_values;
    private Button mVilte_operator_media_tx_fallback_set;
    private EditText mVilte_operator_media_downgrade_packetlost_values;
    private Button mVilte_operator_media_downgrade_packetlost_set;
    private EditText mVilte_operator_media_tmmbr_decrease_start_values;
    private Button mVilte_operator_media_tmmbr_decrease_start_set;
    private EditText mVilte_operator_media_tmmbr_decrease_diff_values;
    private Button mVilte_operator_media_tmmbr_decrease_diff_set;
    private EditText mVilte_operator_media_tmmbr_reserved1_values;
    private Button mVilte_operator_media_tmmbr_reserved1_set;
    private EditText mVilte_operator_media_tmmbr_reserved2_values;
    private Button mVilte_operator_media_tmmbr_reserved2_set;
    private TextView mVilte_customize_status_view;
    private TextView mVilte_rtp_view;
    private TextView mVilte_ma_view;
    private TextView mVilte_TX_fallback_queue_time_view;
    private TextView mVilte_downgrade_packetlost_view;
    private TextView mVilte_tmmbr_decrease_start_view;
    private TextView mVilte_tmmbr_decrease_diff_view;
    private TextView mVilte_Reserved1_view;
    private TextView mVilte_Reserved2_view;

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.vilte_menu_media);
        bindViews();
    }

    @Override
    protected void onResume() {
        super.onResume();
        Elog.d(TAG, "onResume()");
        queryCurrentValue();
    }

    private void queryCurrentValue() {
        String mm = EmUtils.systemPropertyGet(PROP_VILTE_OPTEST_MM, "");
        String rtp = EmUtils.systemPropertyGet(PROP_VILTE_OPTEST_RTP, "");
        String ma = EmUtils.systemPropertyGet(PROP_VILTE_OPTEST_MA, "");
        String rtp_qdurhw = EmUtils.systemPropertyGet(PROP_VILTE_VT_RTP_QDURHW, "");
        String down_grade_th = EmUtils.systemPropertyGet(PROP_VILTE_VT_DOWN_GRADE_TH, "");
        String de_start = EmUtils.systemPropertyGet(PROP_VILTE_VT_DE_START, "");
        String de_diff = EmUtils.systemPropertyGet(PROP_VILTE_VT_DE_DIFF, "");
        String rtp_res1 = EmUtils.systemPropertyGet(PROP_VILTE_VT_RTP_RES1, "");
        String rtp_res2 = EmUtils.systemPropertyGet(PROP_VILTE_VT_RTP_RES2, "");

        mVilte_customize_status_view.setText(PROP_VILTE_OPTEST_MM + " = " + mm);

        mVilte_rtp_view.setText(PROP_VILTE_OPTEST_RTP + " = " + rtp);
        mVilte_operator_media_rtp_values.setText(rtp);

        mVilte_ma_view.setText(PROP_VILTE_OPTEST_MA + " = " + ma);
        mVilte_operator_media_ma_values.setText(ma);


        mVilte_TX_fallback_queue_time_view.setText(PROP_VILTE_VT_RTP_QDURHW + " = " + rtp_qdurhw);
        mVilte_operator_media_tx_fallback_values.setText(rtp_qdurhw);


        mVilte_downgrade_packetlost_view.setText(PROP_VILTE_VT_DOWN_GRADE_TH + " = " +
                down_grade_th);
        mVilte_operator_media_downgrade_packetlost_values.setText(down_grade_th);


        mVilte_tmmbr_decrease_start_view.setText(PROP_VILTE_VT_DE_START + " = " + de_start);
        mVilte_operator_media_tmmbr_decrease_start_values.setText(de_start);


        mVilte_tmmbr_decrease_diff_view.setText(PROP_VILTE_VT_DE_DIFF + " = " + de_diff);
        mVilte_operator_media_tmmbr_decrease_diff_values.setText(de_diff);


        mVilte_Reserved1_view.setText(PROP_VILTE_VT_RTP_RES1 + " = " + rtp_res1);
        mVilte_operator_media_tmmbr_reserved1_values.setText(rtp_res1);

        mVilte_Reserved2_view.setText(PROP_VILTE_VT_RTP_RES2 + " = " + rtp_res2);
        mVilte_operator_media_tmmbr_reserved2_values.setText(rtp_res2);

    }


    boolean checkValue(View v, String values) {
        if (v == mVilte_operator_media_ma_set || v == mVilte_operator_media_rtp_set) {
            try {
                int value = Integer.parseInt(values, 16);
            } catch (NumberFormatException e) {
                EmUtils.showToast("value should be 16 HEX", Toast.LENGTH_SHORT);
                return false;
            }
        } else if (v == mVilte_operator_media_downgrade_packetlost_set
                || v == mVilte_operator_media_tmmbr_decrease_start_set
                || v == mVilte_operator_media_tmmbr_decrease_diff_set) {
            if(values.equals("")){
                return false;
            }
            int value = Integer.parseInt(values, 10);
            if (value < 0 || value > 100) {
                EmUtils.showToast("value should be 16 HEX", Toast.LENGTH_SHORT);
                return false;
            }
        }
        return true;
    }

    @Override
    public void onClick(View v) {
        if (v == mVilte_operator_media_customize_mode_enable) {
            Elog.d(TAG, "Set " + PROP_VILTE_OPTEST_MM + " = 1");
            try {
                EmUtils.getEmHidlService().setEmConfigure(PROP_VILTE_OPTEST_MM, "1");
            }
            catch (Exception e) {
                e.printStackTrace();
                Elog.e(TAG, "set property failed ...");
            }
        } else if (v == mVilte_operator_media_customize_mode_disable) {
            Elog.d(TAG, "Set " + PROP_VILTE_OPTEST_MM + " = 0");
            try {
                EmUtils.getEmHidlService().setEmConfigure(PROP_VILTE_OPTEST_MM, "0");
            }
            catch (Exception e) {
                e.printStackTrace();
                Elog.e(TAG, "set property failed ...");
            }
        } else if (v == mVilte_operator_media_rtp_set) {
            String value = mVilte_operator_media_rtp_values.getText().toString();
            if (checkValue(mVilte_operator_media_rtp_set, value) == true) {
                Elog.d(TAG, "Set " + PROP_VILTE_OPTEST_RTP + " = " + value);
                try {
                    EmUtils.getEmHidlService().setEmConfigure(PROP_VILTE_OPTEST_RTP, value);
                }
                catch (Exception e) {
                    e.printStackTrace();
                    Elog.e(TAG, "set property failed ...");
                }
            } else {
                Elog.d(TAG, "Input value format error ");
            }
        } else if (v == mVilte_operator_media_ma_set) {
            String value = mVilte_operator_media_ma_values.getText().toString();
            if (checkValue(mVilte_operator_media_ma_set, value) == true) {
                Elog.d(TAG, "Set " + PROP_VILTE_OPTEST_MA + " = " + value);
                try {
                    EmUtils.getEmHidlService().setEmConfigure(PROP_VILTE_OPTEST_MA, value);
                }
                catch (Exception e) {
                    e.printStackTrace();
                    Elog.e(TAG, "set property failed ...");
                }
            } else {
                Elog.d(TAG, "Input value format error ");
            }
        } else if (v == mVilte_operator_media_tx_fallback_set) {
            String value = mVilte_operator_media_tx_fallback_values.getText().toString();
            Elog.d(TAG, "Set " + PROP_VILTE_VT_RTP_QDURHW + " = " + value);
            try {
                EmUtils.getEmHidlService().setEmConfigure(PROP_VILTE_VT_RTP_QDURHW, value);
            }
            catch (Exception e) {
                e.printStackTrace();
                Elog.e(TAG, "set property failed ...");
            }
        } else if (v == mVilte_operator_media_downgrade_packetlost_set) {
            String value = mVilte_operator_media_downgrade_packetlost_values.getText().toString();
            if (checkValue(mVilte_operator_media_downgrade_packetlost_set, value) == true) {
                Elog.d(TAG, "Set " + PROP_VILTE_VT_DOWN_GRADE_TH + " = " + value);
                try {
                    EmUtils.getEmHidlService().setEmConfigure(PROP_VILTE_VT_DOWN_GRADE_TH, value);
                }
                catch (Exception e) {
                    e.printStackTrace();
                    Elog.e(TAG, "set property failed ...");
                }
            } else {
                Elog.d(TAG, "Input value format error ");
            }
        } else if (v == mVilte_operator_media_tmmbr_decrease_start_set) {
            String value = mVilte_operator_media_tmmbr_decrease_start_values.getText().toString();
            if (checkValue(mVilte_operator_media_tmmbr_decrease_start_set, value) == true) {
                Elog.d(TAG, "Set " + PROP_VILTE_VT_DE_START + " = " + value);
                try {
                    EmUtils.getEmHidlService().setEmConfigure(PROP_VILTE_VT_DE_START, value);
                }
                catch (Exception e) {
                    e.printStackTrace();
                    Elog.e(TAG, "set property failed ...");
                }
            } else {
                Elog.d(TAG, "Input value format error ");
            }
        } else if (v == mVilte_operator_media_tmmbr_decrease_diff_set) {
            String value = mVilte_operator_media_tmmbr_decrease_diff_values.getText().toString();
            if (checkValue(mVilte_operator_media_tmmbr_decrease_diff_set, value) == true) {
                Elog.d(TAG, "Set " + PROP_VILTE_VT_DE_DIFF + " = " + value);
                try {
                    EmUtils.getEmHidlService().setEmConfigure(PROP_VILTE_VT_DE_DIFF, value);
                }
                catch (Exception e) {
                    e.printStackTrace();
                    Elog.e(TAG, "set property failed ...");
                }
            } else {
                Elog.d(TAG, "Input value format error ");
            }
        } else if (v == mVilte_operator_media_tmmbr_reserved1_set) {
            String value = mVilte_operator_media_tmmbr_reserved1_values.getText().toString();
            Elog.d(TAG, "Set " + PROP_VILTE_VT_RTP_RES1 + " = " + value);
            try {
                EmUtils.getEmHidlService().setEmConfigure(PROP_VILTE_VT_RTP_RES1, value);
            }
            catch (Exception e) {
                e.printStackTrace();
                Elog.e(TAG, "set property failed ...");
            }
        } else if (v == mVilte_operator_media_tmmbr_reserved2_set) {
            String value = mVilte_operator_media_tmmbr_reserved2_values.getText().toString();
            Elog.d(TAG, "Set " + PROP_VILTE_VT_RTP_RES2 + " = " + value);
            try {
                EmUtils.getEmHidlService().setEmConfigure(PROP_VILTE_VT_RTP_RES2, value);
            }
            catch (Exception e) {
                e.printStackTrace();
                Elog.e(TAG, "set property failed ...");
            }
        }
        queryCurrentValue();
    }

    private void bindViews() {
        mVilte_operator_media_customize_mode_enable = (Button) findViewById(R.id
                .vilte_operator_media_customize_mode_enable);
        mVilte_operator_media_customize_mode_disable = (Button) findViewById(R.id
                .vilte_operator_media_customize_mode_disable);
        mVilte_operator_media_customize_mode_enable.setOnClickListener(this);
        mVilte_operator_media_customize_mode_disable.setOnClickListener(this);

        mVilte_operator_media_rtp_values = (EditText) findViewById(R.id
                .vilte_operator_media_rtp_values);
        mVilte_operator_media_rtp_set = (Button) findViewById(R.id.vilte_operator_media_rtp_set);
        mVilte_operator_media_rtp_set.setOnClickListener(this);

        mVilte_operator_media_ma_values = (EditText) findViewById(R.id
                .vilte_operator_media_ma_values);
        mVilte_operator_media_ma_set = (Button) findViewById(R.id.vilte_operator_media_ma_set);
        mVilte_operator_media_ma_set.setOnClickListener(this);

        mVilte_operator_media_tx_fallback_values = (EditText) findViewById(R.id
                .vilte_operator_media_tx_fallback_values);
        mVilte_operator_media_tx_fallback_set = (Button) findViewById(R.id
                .vilte_operator_media_tx_fallback_set);
        mVilte_operator_media_tx_fallback_set.setOnClickListener(this);


        mVilte_operator_media_downgrade_packetlost_values = (EditText) findViewById(R.id
                .vilte_operator_media_downgrade_packetlost_values);
        mVilte_operator_media_downgrade_packetlost_set = (Button) findViewById(R.id
                .vilte_operator_media_downgrade_packetlost_set);
        mVilte_operator_media_downgrade_packetlost_set.setOnClickListener(this);

        mVilte_operator_media_tmmbr_decrease_start_values = (EditText) findViewById(R.id
                .vilte_operator_media_tmmbr_decrease_start_values);
        mVilte_operator_media_tmmbr_decrease_start_set = (Button) findViewById(R.id
                .vilte_operator_media_tmmbr_decrease_start_set);
        mVilte_operator_media_tmmbr_decrease_start_set.setOnClickListener(this);


        mVilte_operator_media_tmmbr_decrease_diff_values = (EditText) findViewById(R.id
                .vilte_operator_media_tmmbr_decrease_diff_values);
        mVilte_operator_media_tmmbr_decrease_diff_set = (Button) findViewById(R.id
                .vilte_operator_media_tmmbr_decrease_diff_set);
        mVilte_operator_media_tmmbr_decrease_diff_set.setOnClickListener(this);


        mVilte_operator_media_tmmbr_reserved1_values = (EditText) findViewById(R.id
                .vilte_operator_media_tmmbr_reserved1_values);
        mVilte_operator_media_tmmbr_reserved1_set = (Button) findViewById(R.id
                .vilte_operator_media_tmmbr_reserved1_set);
        mVilte_operator_media_tmmbr_reserved1_set.setOnClickListener(this);

        mVilte_operator_media_tmmbr_reserved2_values = (EditText) findViewById(R.id
                .vilte_operator_media_tmmbr_reserved2_values);
        mVilte_operator_media_tmmbr_reserved2_set = (Button) findViewById(R.id
                .vilte_operator_media_tmmbr_reserved2_set);
        mVilte_operator_media_tmmbr_reserved2_set.setOnClickListener(this);

        mVilte_customize_status_view = (TextView) findViewById(R.id.vilte_customize_status_view);
        mVilte_rtp_view = (TextView) findViewById(R.id.vilte_rtp_view);
        mVilte_ma_view = (TextView) findViewById(R.id.vilte_ma_view);
        mVilte_TX_fallback_queue_time_view = (TextView) findViewById(R.id
                .vilte_TX_fallback_queue_time_view);
        mVilte_downgrade_packetlost_view = (TextView) findViewById(R.id
                .vilte_downgrade_packetlost_view);
        mVilte_tmmbr_decrease_start_view = (TextView) findViewById(R.id
                .vilte_tmmbr_decrease_start_view);
        mVilte_tmmbr_decrease_diff_view = (TextView) findViewById(R.id
                .vilte_tmmbr_decrease_diff_view);
        mVilte_Reserved1_view = (TextView) findViewById(R.id.vilte_Reserved1_view);
        mVilte_Reserved2_view = (TextView) findViewById(R.id.vilte_Reserved2_view);
    }


}