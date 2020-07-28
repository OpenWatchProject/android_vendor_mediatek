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


public class VilteMenuCodec extends Activity implements View.OnClickListener {
    private final String TAG = "Vilte/MenuCodec";

    private final String PROP_VILTE_OPTEST_VCODEC = "persist.vendor.vt.OPTest_vcodec";
    private final String PROP_VILTE_OPTEST_VENC = "persist.vendor.vt.OPTest_venc";
    private final String PROP_VILTE_OPTEST_VDEC = "persist.vendor.vt.OPTest_vdec";

    private Button mVilte_operator_media_customize_mode_enable;
    private Button mVilte_operator_media_customize_mode_disable;
    private EditText mVilte_operator_codec_venc_values;
    private Button mVilte_operator_codec_venc_set;
    private EditText mVilte_operator_codec_vdec_values;
    private Button mVilte_operator_codec_vdec_set;

    private TextView mVilte_customize_status_view;
    private TextView mVilte_venc_view;
    private TextView mVilte_vdec_view;

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.vilte_menu_codec);
        bindViews();
    }

    private void bindViews() {
        mVilte_operator_media_customize_mode_enable = (Button) findViewById(R.id
                .vilte_operator_media_customize_mode_enable);
        mVilte_operator_media_customize_mode_disable = (Button) findViewById(R.id
                .vilte_operator_media_customize_mode_disable);
        mVilte_operator_media_customize_mode_enable.setOnClickListener(this);
        mVilte_operator_media_customize_mode_disable.setOnClickListener(this);

        mVilte_operator_codec_venc_values = (EditText) findViewById(R.id
                .vilte_operator_codec_venc_values);
        mVilte_operator_codec_venc_set = (Button) findViewById(R.id.vilte_operator_codec_venc_set);
        mVilte_operator_codec_venc_set.setOnClickListener(this);

        mVilte_operator_codec_vdec_values = (EditText) findViewById(R.id
                .vilte_operator_codec_vdec_values);
        mVilte_operator_codec_vdec_set = (Button) findViewById(R.id.vilte_operator_codec_vdec_set);
        mVilte_operator_codec_vdec_set.setOnClickListener(this);

        mVilte_customize_status_view = (TextView) findViewById(R.id.vilte_customize_status_view);
        mVilte_venc_view = (TextView) findViewById(R.id.vilte_venc_view);
        mVilte_vdec_view = (TextView) findViewById(R.id.vilte_vdec_view);
    }

    @Override
    protected void onResume() {
        super.onResume();
        Elog.d(TAG, "onResume()");
        queryCurrentValue();
    }

    private void queryCurrentValue() {
        String vcodec = EmUtils.systemPropertyGet(PROP_VILTE_OPTEST_VCODEC, "");
        String venc = EmUtils.systemPropertyGet(PROP_VILTE_OPTEST_VENC, "");
        String vdec = EmUtils.systemPropertyGet(PROP_VILTE_OPTEST_VDEC, "");

        mVilte_customize_status_view.setText(PROP_VILTE_OPTEST_VCODEC + " = " + vcodec);

        mVilte_venc_view.setText(PROP_VILTE_OPTEST_VENC + " = " + venc);
        mVilte_operator_codec_venc_values.setText(venc);

        mVilte_vdec_view.setText(PROP_VILTE_OPTEST_VDEC + " = " + vdec);
        mVilte_operator_codec_vdec_values.setText(vdec);
    }

    boolean checkValue(View v, String values) {
        if (v == mVilte_operator_codec_venc_set || v == mVilte_operator_codec_vdec_set) {
            try {
                int value = Integer.parseInt(values, 16);
            } catch (NumberFormatException e) {
                EmUtils.showToast("value should be 16 HEX", Toast.LENGTH_SHORT);
                return false;
            }
        }
        return true;
    }

    public void onClick(View v) {
        if (v == mVilte_operator_media_customize_mode_enable) {
            Elog.d(TAG, "Set " + PROP_VILTE_OPTEST_VCODEC + " = 1");
            try {
                EmUtils.getEmHidlService().setEmConfigure(PROP_VILTE_OPTEST_VCODEC, "1");
            }
            catch (Exception e) {
                e.printStackTrace();
                Elog.e(TAG, "set property failed ...");
            }
        } else if (v == mVilte_operator_media_customize_mode_disable) {
            Elog.d(TAG, "Set " + PROP_VILTE_OPTEST_VCODEC + " = 0");
            try {
                EmUtils.getEmHidlService().setEmConfigure(PROP_VILTE_OPTEST_VCODEC, "0");
            }
            catch (Exception e) {
                e.printStackTrace();
                Elog.e(TAG, "set property failed ...");
            }
        } else if (v == mVilte_operator_codec_venc_set) {
            String value = mVilte_operator_codec_venc_values.getText().toString();
            Elog.d(TAG, "Set " + PROP_VILTE_OPTEST_VENC + " = " + value);
            try {
                EmUtils.getEmHidlService().setEmConfigure(PROP_VILTE_OPTEST_VENC, value);
            }
            catch (Exception e) {
                e.printStackTrace();
                Elog.e(TAG, "set property failed ...");
            }
        } else if (v == mVilte_operator_codec_vdec_set) {
            String value = mVilte_operator_codec_vdec_values.getText().toString();
            Elog.d(TAG, "Set " + PROP_VILTE_OPTEST_VDEC + " = " + value);
            try {
                EmUtils.getEmHidlService().setEmConfigure(PROP_VILTE_OPTEST_VDEC, value);
            }
            catch (Exception e) {
                e.printStackTrace();
                Elog.e(TAG, "set property failed ...");
            }
        }
        queryCurrentValue();
    }
}