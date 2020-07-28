package com.mediatek.engineermode.vilte;

import android.app.Activity;
import android.app.AlertDialog;
import android.app.Dialog;
import android.content.DialogInterface;
import android.os.Bundle;
import android.view.View;
import android.widget.Button;
import android.widget.TextView;

import com.mediatek.engineermode.Elog;
import com.mediatek.engineermode.EmUtils;
import com.mediatek.engineermode.R;


public class ViLTEVtService extends Activity implements View.OnClickListener {
    private final String TAG = "Vilte/VtService";
    private final int TEST_OP_CODE = 100;
    private final String PROP_VILTE_TEST_OP_CODE = "persist.vendor.vt.lab_op_code";
    private final String[] Test_OP_CODE_label = {
            "Null(0)", "CMCC(1)", "CU(2)", "Orange(3)", "DTAG(5)",
            "Vodafone(6)", "AT&T(7)", "TMO US(8)", "CT(9)",
            "H3G(11)", "Verizon(12)", "DoCoMo(17)",
            "Reliance Jio(18)", "Telstra(19)", "Softbank(50)", "CSL(100)",
            "3HK(106)", "Smartfren(117)", "APTG(124)", "SmartTone(138)",
            "CMHK(149)"
    };
    private final String[][] Test_OP_CODE_VALUES = {
            {"Null", "0"}, {"CMCC", "1"}, {"CU", "2"}, {"Orange", "3"},
            {"DTAG", "5"}, {"Vodafone", "6"}, {"AT&T", "7"}, {"TMO US", "8"},
            {"CT", "9"}, {"H3G", "11"}, {"Verizon", "12"}, {"DoCoMo", "17"},
            {"Reliance Jio", "18"}, {"Telstra", "19"}, {"Softbank", "50"}, {"CSL", "100"},
            {"3HK", "106"}, {"Smartfren", "117"}, {"APTG", "124"}, {"SmartTone", "138"},
            {"CMHK", "149"},
    };
    private AlertDialog M = null;
    private int test_op_index = 0;
    private TextView mTextviewTestOpCode;
    private Button mButtonTestOpCode;
    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.vilte_menu_vt_service);
        mTextviewTestOpCode = (TextView) findViewById(R.id.vilte_test_op_code_status);
        mButtonTestOpCode = (Button) findViewById(R.id.vilte_test_op_code);
        mButtonTestOpCode.setOnClickListener(this);
    }

    @Override
    protected void onResume() {
        super.onResume();
        Elog.d(TAG, "onResume()");
        queryTestOpMode();
    }

    void queryTestOpMode() {
        String testOpCode = EmUtils.systemPropertyGet(PROP_VILTE_TEST_OP_CODE, "0");
        mTextviewTestOpCode.setText(PROP_VILTE_TEST_OP_CODE + " = " + testOpCode);

        for (int i = 0; i < Test_OP_CODE_VALUES.length; i++) {
            if (testOpCode.equals(Test_OP_CODE_VALUES[i][1])) {
                test_op_index = i;
                break;
            }
        }
        Elog.d(TAG, "test_op_index: " + test_op_index);
    }


    @Override
    protected Dialog onCreateDialog(int id) {
        switch (id) {
            case TEST_OP_CODE:
                M = new AlertDialog.Builder(ViLTEVtService.this).setCancelable(false).setTitle(
                        "test op code").setSingleChoiceItems(Test_OP_CODE_label, test_op_index,
                        new DialogInterface.OnClickListener() {
                            public void onClick(DialogInterface dialog, int which) {
                                Elog.d(TAG, "Set test op code: "
                                        + Test_OP_CODE_VALUES[which][0] + ": "
                                        + Test_OP_CODE_VALUES[which][1]);
                                try {
                                    EmUtils.getEmHidlService().setEmConfigure(
                                        PROP_VILTE_TEST_OP_CODE, Test_OP_CODE_VALUES[which][1]);
                                }
                                catch (Exception e) {
                                    e.printStackTrace();
                                    Elog.e(TAG, "set property failed ...");
                                }
                            }
                        }).setPositiveButton("Set",
                        new DialogInterface.OnClickListener() {
                            public void onClick(DialogInterface dialog,
                                                int whichButton) {
                                queryTestOpMode();
                            }
                        }).create();
                return M;
        }
        return null;
    }

    public void onClick(View v) {
        if (v == mButtonTestOpCode) {
            queryTestOpMode();
            showDialog(TEST_OP_CODE);
        }
    }
}