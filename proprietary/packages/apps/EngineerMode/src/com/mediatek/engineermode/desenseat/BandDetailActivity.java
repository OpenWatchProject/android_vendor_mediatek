package com.mediatek.engineermode.desenseat;



import android.app.Activity;
import android.os.Bundle;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.Button;
import android.widget.EditText;
import android.widget.TextView;

import com.mediatek.engineermode.R;

/**
 * Band edit interface.
 *
 */
public class BandDetailActivity extends Activity {

    public static final String EXTRA_MODEM_TYPE = "type";
    public static final String EXTRA_INDEX = "index";

    private BandItem mBandItem;
    private Button mBtnSet;
    private TextView mTextChannel1;
    private TextView mTextChannel2;
    private TextView mTextChannel3;
    private EditText mEditChannel1;
    private EditText mEditChannel2;
    private EditText mEditChannel3;
    private EditText mEditPower;
    private TextView mBandName;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        // TODO Auto-generated method stub
        super.onCreate(savedInstanceState);
        setContentView(R.layout.desense_at_band_detail);
/*        Intent it = getIntent();
        mChannel = it.getIntArrayExtra(EXTRA_CHANNLE);
        mPower = it.getIntExtra(EXTRA_POWER, -1);
        mIsLte = it.getBooleanExtra(EXTRA_IS_LTE, false);*/
        mBandItem = RfBandConfigActivity.getClickedItem();
        mBtnSet = (Button) findViewById(R.id.band_detail_set);
        mTextChannel1 = (TextView) findViewById(R.id.channle1Text);
        mTextChannel2 = (TextView) findViewById(R.id.channle2Text);
        mTextChannel3 = (TextView) findViewById(R.id.channle3Text);
        mEditChannel1 = (EditText) findViewById(R.id.channle1Edit);
        mEditChannel2 = (EditText) findViewById(R.id.channle2Edit);
        mEditChannel3 = (EditText) findViewById(R.id.channle3Edit);

        mTextChannel2.setVisibility(View.GONE);
        mTextChannel3.setVisibility(View.GONE);
        mEditChannel2.setVisibility(View.GONE);
        mEditChannel3.setVisibility(View.GONE);

        mBandName = (TextView) findViewById(R.id.bandname);
        mEditPower = (EditText) findViewById(R.id.powerEdit);
        initView();
    }

    private int extractIntFromString(String str) {
        if (str != null) {
            str = str.trim();
        } else {
            return -1;
        }
        if (str.length() > 0) {
            return Integer.parseInt(str);
        }

        return -1;
    }

    private void saveSettings() {
        int ch1 = extractIntFromString(mEditChannel1.getText().toString());
        int ch2 = extractIntFromString(mEditChannel2.getText().toString());
        int ch3 = extractIntFromString(mEditChannel3.getText().toString());

        mBandItem.setChannel(ch1, ch2, ch3);

        int power = extractIntFromString(mEditPower.getText().toString());
        mBandItem.setPower(power);
    }

    private void initView() {
        mBandName.setText(mBandItem.getBandName());
        int[] channel = mBandItem.getChannel();
        if (channel[0] != -1) {
            mEditChannel1.setText(String.valueOf(channel[0]));
        }
        if (channel[1] != -1) {
            mEditChannel2.setText(String.valueOf(channel[1]));
        }
        if (channel[2] != -1) {
            mEditChannel3.setText(String.valueOf(channel[2]));
        }
        int power = mBandItem.getPower();
        if (power != -1) {
            mEditPower.setText(String.valueOf(power));
        }

        if (mBandItem.isUlFreq()) {
            mTextChannel1.setText(R.string.desense_at_ulfreq_1);
            mTextChannel2.setText(R.string.desense_at_ulfreq_2);
            mTextChannel3.setText(R.string.desense_at_ulfreq_3);
        }

        mBtnSet.setOnClickListener(new OnClickListener() {
            @Override
            public void onClick(View arg0) {
                saveSettings();
                finish();
            }
        });
    }


}
