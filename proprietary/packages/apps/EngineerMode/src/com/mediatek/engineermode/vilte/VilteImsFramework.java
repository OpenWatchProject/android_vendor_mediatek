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


public class VilteImsFramework extends Activity implements View.OnClickListener {
    private final String TAG = "Vilte/ImsFramework";

    private final String PROP_VILTE_CONFERENCE_SUPPORT =
            "persist.vendor.vt.video_conference_support";

    private Button mVilte_operator_imsframework_video_conference_enable;
    private Button mVilte_operator_imsframework_video_conference_disable;

    private TextView mVilte_video_conference_view;

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.vilte_ims_framework);
        bindViews();
    }

    private void bindViews() {
        mVilte_operator_imsframework_video_conference_enable = (Button) findViewById(R.id
                .vilte_operator_imsframework_video_conference_enable);
        mVilte_operator_imsframework_video_conference_disable = (Button) findViewById(R.id
                .vilte_operator_imsframework_video_conference_disable);
        mVilte_operator_imsframework_video_conference_enable.setOnClickListener(this);
        mVilte_operator_imsframework_video_conference_disable.setOnClickListener(this);

        mVilte_video_conference_view = (TextView) findViewById(R.id.vilte_video_conference_view);
    }

    @Override
    protected void onResume() {
        super.onResume();
        Elog.d(TAG, "onResume()");
        queryCurrentValue();
    }

    private void queryCurrentValue() {
        String videoConferenceSupport =
                EmUtils.systemPropertyGet(PROP_VILTE_CONFERENCE_SUPPORT, "");
        mVilte_video_conference_view.setText(PROP_VILTE_CONFERENCE_SUPPORT
                + " = " + videoConferenceSupport);
    }

    public void onClick(View v) {
        if (v == mVilte_operator_imsframework_video_conference_enable) {
            Elog.d(TAG, "Set " + PROP_VILTE_CONFERENCE_SUPPORT + " = 1");
            try {
                EmUtils.getEmHidlService().setEmConfigure(PROP_VILTE_CONFERENCE_SUPPORT, "1");
            }
            catch (Exception e) {
                e.printStackTrace();
                Elog.e(TAG, "set property failed ...");
            }

        } else if (v == mVilte_operator_imsframework_video_conference_disable) {
            Elog.d(TAG, "Set " + PROP_VILTE_CONFERENCE_SUPPORT + " = 0");
            try {
                EmUtils.getEmHidlService().setEmConfigure(PROP_VILTE_CONFERENCE_SUPPORT, "0");
            }
            catch (Exception e) {
                e.printStackTrace();
                Elog.e(TAG, "set property failed ...");
            }
        }
        queryCurrentValue();
    }
}