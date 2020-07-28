package com.mediatek.engineermode.npt;


import android.app.Activity;
import android.os.Bundle;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.ArrayAdapter;
import android.widget.Button;
import android.widget.EditText;
import android.widget.RadioButton;
import android.widget.Spinner;
import android.widget.TableRow;

import com.mediatek.engineermode.Elog;
import com.mediatek.engineermode.R;

import java.util.HashMap;

/**
 * Band edit interface.
 */
public class BandDetailActivity extends Activity {
    private static final String TAG = "NPT/BandDetai";
    public static HashMap<String, Integer> sGSMBandUIIndexMapping = new HashMap<String, Integer>();
    public static HashMap<Integer, String> sTDSCDMABandUIIndexMapping = new HashMap<Integer,
            String>();
    ArrayAdapter<CharSequence> adapter;
    private BandItem mBandItem;
    private Spinner mRat_spinner;
    private Spinner mBand_spinner;
    private EditText mChannel_start_edit;
    private EditText mChannel_stepsize_edit;
    private EditText mChannel_end_edit;
    private TableRow mRb_rx_bandwidth;
    private Spinner mRx_bandwidth_spinner;
    private TableRow mRb_tx_bandwidth;
    private Spinner mTx_bandwidth_spinner;
    private RadioButton mTx_on_enable_ref;
    private RadioButton mTx_on_disable_ref;
    private RadioButton mTx_on_enable_npt;
    private RadioButton mTx_on_disable_npt;
    private EditText mTx_power_editor;
    private EditText mAntennaState;
    private EditText mRepeat_times_editer;
    private TableRow mTr_vrb_start;
    private EditText mVrb_start_editor;
    private TableRow mTr_vrb_length;
    private EditText mVrb_length_editor;
    private Button mButton_set;
    private String npt_gsm_band[] = {"GSM 850", "GSM 900", "GSM 1800", "GSM 1900"};
    private String npt_tdscdma_band[] = {"Band A", "Band B", "Band C", "Band D", "Band E", "Band " +
            "F"};

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        // TODO Auto-generated method stub
        super.onCreate(savedInstanceState);
        setContentView(R.layout.noise_profiling_band_detail);
        mBandItem = BandConfigActivity.getClickedItem();
        bindViews();
        Elog.d(TAG, "rat name = " + mBandItem.getmBandName());
        sGSMBandUIIndexMapping.put("GSM 850", 0);
        sGSMBandUIIndexMapping.put("GSM 900", 1);
        sGSMBandUIIndexMapping.put("GSM 1800", 2);
        sGSMBandUIIndexMapping.put("GSM 1900", 3);

        sTDSCDMABandUIIndexMapping.put(1, "Band A");
        sTDSCDMABandUIIndexMapping.put(2, "Band B");
        sTDSCDMABandUIIndexMapping.put(3, "Band C");
        sTDSCDMABandUIIndexMapping.put(4, "Band D");
        sTDSCDMABandUIIndexMapping.put(5, "Band E");
        sTDSCDMABandUIIndexMapping.put(6, "Band F");

        //rat
        adapter = ArrayAdapter.createFromResource(this,
                R.array.npt_rat,
                android.R.layout.simple_spinner_item);
        adapter.setDropDownViewResource(android.R.layout.simple_spinner_dropdown_item);
        mRat_spinner.setAdapter(adapter);
        mRat_spinner.setSelection(mBandItem.getmBandType().ordinal());

        //band
        adapter = new ArrayAdapter<CharSequence>(this, android.R.layout.simple_spinner_item);
        adapter.setDropDownViewResource(android.R.layout.simple_spinner_dropdown_item);
        mBand_spinner.setAdapter(adapter);
        adapter.clear();

        int index = -1;

        if (mBandItem.getmBandType() == BandItem.BandType.BAND_GSM) {
            for (int i = 0; i < npt_gsm_band.length; i++) {
                adapter.add(npt_gsm_band[i]);
            }
            index = sGSMBandUIIndexMapping.get(mBandItem.getmBandName());
        } else if (mBandItem.getmBandType() == BandItem.BandType.BAND_WCDMA) {
            for (int i = 0; i < 11; i++) {
                if (!BandConfigActivity.isSupportedBand(BandConfigActivity
                        .bandSupported[BandConfigActivity.INDEX_UMTS_WCDMA_BAND], i))
                    continue;
                adapter.add(String.valueOf(i + 1));
            }
            index = adapter.getPosition(mBandItem.getmBandValue() + "");
        } else if (mBandItem.getmBandType() == BandItem.BandType.BAND_TD) {
            for (int i = 0; i < 6; i++) {
                if (!BandConfigActivity.isSupportedBand(BandConfigActivity
                        .bandSupported[BandConfigActivity.INDEX_UMTS_TDSCDMA_BAND], i))
                    continue;
                adapter.add(npt_tdscdma_band[i]);
            }
            index = adapter.getPosition(sTDSCDMABandUIIndexMapping.get(mBandItem.getmBandValue()));
        } else if (mBandItem.getmBandType() == BandItem.BandType.BAND_LTE) {
            for (int i = 0; i < 33; i++) {
                if (!BandConfigActivity.isSupportedBand(BandConfigActivity
                        .bandSupported[BandConfigActivity.INDEX_LTE_FDD_BAND], i))
                    continue;
                adapter.add(String.valueOf(i + 1));
            }
            for (int i = 33; i < 45; i++) {
                if (!BandConfigActivity.isSupportedBand(BandConfigActivity
                        .bandSupported[BandConfigActivity.INDEX_LTE_TDD_BAND], i - 33))
                    continue;
                adapter.add(String.valueOf(i + 1));
            }
            index = adapter.getPosition(mBandItem.getmBandValue() + "");
        } else if (mBandItem.getmBandType() == BandItem.BandType.BAND_CDMA) {
            for (int i = 0; i < 11; i++) {
                if (!BandConfigActivity.isSupportedBand(BandConfigActivity
                        .bandSupported[BandConfigActivity.INDEX_CDMA_BAND], i))
                    continue;
                adapter.add(String.valueOf(i));
            }
            index = adapter.getPosition(mBandItem.getmBandValue() + "");
        }
        adapter.notifyDataSetChanged();
        if (index != -1) {
            mBand_spinner.setSelection(index);
        }
        Elog.i(TAG, "index: " + index);

        //channel
        mChannel_start_edit.setText(mBandItem.getmChannleScope()[0] + "");
        mChannel_stepsize_edit.setText(mBandItem.getmChannleScope()[1] + "");
        mChannel_end_edit.setText(mBandItem.getmChannleScope()[2] + "");

        //tx flag and power
        mTx_on_enable_ref.setChecked(mBandItem.getmTxOnflagValueRef() == 1);
        mTx_on_disable_ref.setChecked(mBandItem.getmTxOnflagValueRef() == 0);
        mTx_on_enable_npt.setChecked(mBandItem.getmTxOnflagValueNpt() == 1);
        mTx_on_disable_npt.setChecked(mBandItem.getmTxOnflagValueNpt() == 0);
        mTx_power_editor.setText(mBandItem.getmTxPowerValue() + "");
        //repea times
        mRepeat_times_editer.setText(mBandItem.getmRepeatTimesValue() + "");
        mAntennaState.setText(mBandItem.getmAntennaStateValue() + "");
        //LTE
        mVrb_start_editor.setText(mBandItem.getmRbStartValue() + "");
        mVrb_length_editor.setText(mBandItem.getmRblengthValue() + "");

        adapter = ArrayAdapter.createFromResource(this,
                R.array.npt_lte_bandwidth,
                android.R.layout.simple_spinner_item);
        adapter.setDropDownViewResource(android.R.layout.simple_spinner_dropdown_item);
        mRx_bandwidth_spinner.setAdapter(adapter);
        mTx_bandwidth_spinner.setAdapter(adapter);
        mRx_bandwidth_spinner.setSelection(mBandItem.getmRxBwValue());
        mTx_bandwidth_spinner.setSelection(mBandItem.getmTxBwValue());


        if (mBandItem.getmBandType() != BandItem.BandType.BAND_LTE) {
            mRb_rx_bandwidth.setVisibility(View.GONE);
            mRb_tx_bandwidth.setVisibility(View.GONE);
            mTr_vrb_start.setVisibility(View.GONE);
            mTr_vrb_length.setVisibility(View.GONE);
        }

    }

    private void saveSettings() {
        int[] channels = new int[3];
        channels[0] = Integer.parseInt(mChannel_start_edit.getText().toString());
        channels[1] = Integer.parseInt(mChannel_stepsize_edit.getText().toString());
        channels[2] = Integer.parseInt(mChannel_end_edit.getText().toString());
        mBandItem.setmChannleScope(channels);

        mBandItem.setmTxOnflagValueRef(mTx_on_enable_ref.isChecked() ? 1 : 0);
        mBandItem.setmTxOnflagValueRef(mTx_on_disable_ref.isChecked() ? 0 : 1);
        mBandItem.setmTxOnflagValueNpt(mTx_on_enable_npt.isChecked() ? 1 : 0);
        mBandItem.setmTxOnflagValueNpt(mTx_on_disable_npt.isChecked() ? 0 : 1);

        int txPower = Integer.parseInt(mTx_power_editor.getText().toString());
        mBandItem.setmTxPowerValue(txPower);

        int repeatTime = Integer.parseInt(mRepeat_times_editer.getText().toString());
        mBandItem.setmRepeatTimesValue(repeatTime);

        int antennaState = Integer.parseInt(mAntennaState.getText().toString());
        mBandItem.setmAntennaStateValue(antennaState);

        if (mBandItem.getmBandType() == BandItem.BandType.BAND_LTE) {
            int vrb_start = Integer.parseInt(mVrb_start_editor.getText().toString());
            mBandItem.setmRbStartValue(vrb_start);
            int vrb_length = Integer.parseInt(mVrb_length_editor.getText().toString());
            mBandItem.setmRblengthValue(vrb_length);

            mBandItem.setmRxBwValue(mRx_bandwidth_spinner.getSelectedItemPosition());
            mBandItem.setmTxBwValue(mTx_bandwidth_spinner.getSelectedItemPosition());
        }

    }

    private void bindViews() {
        mRat_spinner = (Spinner) findViewById(R.id.rat_spinner);
        mBand_spinner = (Spinner) findViewById(R.id.band_spinner);
        mChannel_start_edit = (EditText) findViewById(R.id.channel_start_edit);
        mChannel_stepsize_edit = (EditText) findViewById(R.id.channel_stepsize_edit);
        mChannel_end_edit = (EditText) findViewById(R.id.channel_end_edit);
        mRb_rx_bandwidth = (TableRow) findViewById(R.id.rb_rx_bandwidth);
        mRx_bandwidth_spinner = (Spinner) findViewById(R.id.rx_bandwidth_spinner);
        mRb_tx_bandwidth = (TableRow) findViewById(R.id.rb_tx_bandwidth);
        mTx_bandwidth_spinner = (Spinner) findViewById(R.id.tx_bandwidth_spinner);
        mTx_on_enable_ref = (RadioButton) findViewById(R.id.tx_on_enable_ref);
        mTx_on_disable_ref = (RadioButton) findViewById(R.id.tx_on_disable_ref);
        mTx_on_enable_npt = (RadioButton) findViewById(R.id.tx_on_enable_npt);
        mTx_on_disable_npt = (RadioButton) findViewById(R.id.tx_on_disable_npt);
        mTx_power_editor = (EditText) findViewById(R.id.tx_power_editor);
        mRepeat_times_editer = (EditText) findViewById(R.id.repeat_times_editer);
        mAntennaState = (EditText) findViewById(R.id.npt_config_tx_ant_set);

        mTr_vrb_start = (TableRow) findViewById(R.id.tr_vrb_start);
        mVrb_start_editor = (EditText) findViewById(R.id.vrb_start_editor);
        mTr_vrb_length = (TableRow) findViewById(R.id.tr_vrb_length);
        mVrb_length_editor = (EditText) findViewById(R.id.vrb_length_editor);
        mButton_set = (Button) findViewById(R.id.button_set);

        mButton_set.setOnClickListener(new OnClickListener() {
            @Override
            public void onClick(View arg0) {
                saveSettings();
            }
        });
    }

}
