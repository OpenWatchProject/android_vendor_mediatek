package com.mediatek.engineermode.mdmdiagnosticinfo;
import android.app.Activity;
import android.content.Intent;
import android.os.Bundle;
import android.view.View;
import android.widget.AdapterView;
import android.widget.AdapterView.OnItemClickListener;
import android.widget.ArrayAdapter;
import android.widget.ListView;

import com.android.internal.telephony.PhoneConstants;
import com.mediatek.engineermode.ModemCategory;
import com.mediatek.engineermode.R;

import java.util.ArrayList;
import java.util.List;

public class MDMSimSelectActivity extends Activity implements OnItemClickListener {

    private List<String> items = new ArrayList<String>();
    private ListView simTypeListView = null;
    private ArrayAdapter<String> adapter = null;
    private int capabilitySim;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.dualtalk_networkinfo);
        simTypeListView = (ListView) findViewById(R.id.ListView_dualtalk_networkinfo);
        adapter = new ArrayAdapter<String>(this,
                android.R.layout.simple_list_item_1, items);
        simTypeListView.setAdapter(adapter);
        simTypeListView.setOnItemClickListener(this);
    }

    @Override
    protected void onResume() {
        super.onResume();
        capabilitySim = ModemCategory.getCapabilitySim() % 2;
        items.clear();
        items.add(getString(R.string.SIM1) + "(" +
                (capabilitySim == 0 ? getString(R.string.mdm_em_components_sim1) :
                    getString(R.string.mdm_em_components_sim2)) + ")");
        items.add(getString(R.string.SIM2) + "(" +
                (capabilitySim == 1 ? getString(R.string.mdm_em_components_sim1) :
                    getString(R.string.mdm_em_components_sim2)) + ")");
        adapter.notifyDataSetInvalidated();
    }

    @Override
    public void onItemClick(AdapterView<?> arg0, View arg1, int position, long arg3) {
        Intent intent = new Intent();
        int simType = PhoneConstants.SIM_ID_1;
        switch (position) {
        case 0:
            simType = PhoneConstants.SIM_ID_1;
            break;
        case 1:
            simType = PhoneConstants.SIM_ID_2;
            break;
        default:
            break;
        }
        intent.setClassName(this,
                    "com.mediatek.engineermode.mdmdiagnosticinfo.MDMComponentSelectActivity");
        intent.putExtra("mSimType", simType);
        this.startActivity(intent);
    }
}
