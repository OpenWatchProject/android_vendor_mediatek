package com.mediatek.engineermode.networkinfotc1;

import android.app.Activity;
import android.os.Bundle;
import android.view.View;
import android.widget.Button;
import android.widget.TextView;

import com.android.internal.telephony.PhoneConstants;
import com.mediatek.engineermode.Elog;
import com.mediatek.engineermode.R;
import com.mediatek.mdml.Msg;

import java.util.ArrayList;
import java.util.List;


public class MDMSample extends Activity implements View.OnClickListener, MDMCoreOperation
        .IMDMSeiviceInterface {
    private static final String TAG = "MDMSample";
    private static final int MSG_UPDATE_UI = 0;
    private Button mStart_listen;
    private Button mStop_listen;
    private TextView mResult;
    private String info = "";
    private List<MdmBaseComponent> componentsArray = new ArrayList<MdmBaseComponent>();
    private int mSimTypeToShow = 0;
    private String SubscribeMsgIdName[] = {"MSG_ID_EM_ERRC_MOB_MEAS_INTRARAT_INFO_IND",
            "MSG_ID_EM_EL1_STATUS_IND",
            "EM_EL1_STATUS_DL_INFO"};

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.mdm_sample);
        bindViews();

        mSimTypeToShow = PhoneConstants.SIM_ID_1;
        MdmBaseComponent components = new MdmBaseComponent();
        components.setEmComponentName(SubscribeMsgIdName);
        componentsArray.add(components);
        MDMCoreOperation.getInstance().mdmParametersSeting(componentsArray, mSimTypeToShow);
        MDMCoreOperation.getInstance().setOnMDMChangedListener(this);
    }

    @Override
    public void onClick(View view) {
        switch (view.getId()) {
            case R.id.start_listen:
                MDMCoreOperation.getInstance().mdmInitialize(this);
                break;
            case R.id.stop_listen:
                MDMCoreOperation.getInstance().mdmlUnSubscribe();
                break;
        }

    }

    private void bindViews() {
        mStart_listen = (Button) findViewById(R.id.start_listen);
        mStop_listen = (Button) findViewById(R.id.stop_listen);
        mResult = (TextView) findViewById(R.id.result);
        mStart_listen.setOnClickListener(this);
        mStop_listen.setOnClickListener(this);
    }


    @Override
    public void onUpdateMDMStatus(int msg_id) {
        switch (msg_id) {
            case MDMCoreOperation.MDM_SERVICE_INIT: {
                Elog.d(TAG, "MDM Service init done");
                MDMCoreOperation.getInstance().mdmlSubscribe();
                break;
            }
            case MDMCoreOperation.SUBSCRIBE_DONE: {
                Elog.d(TAG, "Subscribe message id done");
                MDMCoreOperation.getInstance().mdmlEnableSubscribe();
                break;
            }
            case MDMCoreOperation.UNSUBSCRIBE_DONE: {
                Elog.d(TAG, "UnSubscribe message id done");
                MDMCoreOperation.getInstance().mdmlClosing();
                break;
            }
            default:
                break;
        }
    }

    @Override
    public void onUpdateMDMData(String name, Msg data) {
        Elog.d(TAG, "update = " + name);

        if (name.equals("MSG_ID_EM_ERRC_MOB_MEAS_INTRARAT_INFO_IND")) {
            String coName = "serving_info.";
            info = "";
            int rsrp = MDMCoreOperation.getInstance().getFieldValue(data, coName + "rsrp", true);
            int rsrq = MDMCoreOperation.getInstance().getFieldValue(data, coName + "rsrq", true);
            int sinr = MDMCoreOperation.getInstance().getFieldValue(data, coName +
                    "rs_snr_in_qdb", true);
            int earfcn = MDMCoreOperation.getInstance().getFieldValue(data, coName + "earfcn");
            int pci = MDMCoreOperation.getInstance().getFieldValue(data, coName + "pci");
            int band = MDMCoreOperation.getInstance().getFieldValue(data, coName + "serv_lte_band");
            int dlBandwidth = MDMCoreOperation.getInstance().getFieldValue(data, coName +
                    "DlBandwidth", true);
            int ulBandwidth = MDMCoreOperation.getInstance().getFieldValue(data, coName +
                    "UlBandwidth", true);

            info += "rsrp: " + rsrp + "\n";
            info += "rsrq: " + rsrq + "\n";
            info += "sinr: " + sinr + "\n";
            info += "earfcn: " + earfcn + "\n";
            info += "pci: " + pci + "\n";
            info += "band: " + band + "\n";
            info += "dlBandwidth: " + dlBandwidth + "\n";
            info += "ulBandwidth: " + ulBandwidth + "\n";

            Elog.d(TAG, "info = " + info);

            mResult.setText(info);

        }

        //  for( MdmBaseComponent c : componentsArray){
        //       if(c.hasEmType(name)){
        //          c.update(name, data);
        //        }
        //     }
    }


}