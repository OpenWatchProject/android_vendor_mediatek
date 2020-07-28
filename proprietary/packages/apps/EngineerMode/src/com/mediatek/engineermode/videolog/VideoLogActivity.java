package com.mediatek.engineermode.videolog;

import android.app.Activity;
import android.os.Bundle;
import android.view.View;
import android.widget.CheckBox;

import com.mediatek.engineermode.Elog;
import com.mediatek.engineermode.R;

import java.util.ArrayList;
import java.util.List;

/**
 * Class for showing UI of video log control.
 *
 */
public class VideoLogActivity extends Activity implements android.view.View.OnClickListener {

    protected static final String TAG = "VideoLog";
    private List<CheckBox> mCheckBoxList;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        // TODO Auto-generated method stub
        super.onCreate(savedInstanceState);
        setContentView(R.layout.video_log);
        mCheckBoxList = new ArrayList<CheckBox>();
        initUI();
    }

    private void initUI() {
        CheckBox cbOmxVenc = (CheckBox) findViewById(R.id.cbOmxVenc);
        cbOmxVenc.setTag(new OmxVenc());
        cbOmxVenc.setOnClickListener(this);
        mCheckBoxList.add(cbOmxVenc);

        CheckBox cbOmxVdec = (CheckBox) findViewById(R.id.cbOmxVdec);
        cbOmxVdec.setTag(new OmxVdec());
        cbOmxVdec.setOnClickListener(this);
        mCheckBoxList.add(cbOmxVdec);

        CheckBox cbVdecDriver = (CheckBox) findViewById(R.id.cbVdecDriver);
        cbVdecDriver.setTag(new VdecDriver());
        cbVdecDriver.setOnClickListener(this);
        mCheckBoxList.add(cbVdecDriver);

        CheckBox cbSvp = (CheckBox) findViewById(R.id.cbSvp);
        cbSvp.setTag(new Svp());
        cbSvp.setOnClickListener(this);
        mCheckBoxList.add(cbSvp);

        CheckBox cbOmxCore = (CheckBox) findViewById(R.id.cbOmxCore);
        cbOmxCore.setTag(new OmxCore());
        cbOmxCore.setOnClickListener(this);
        mCheckBoxList.add(cbOmxCore);

        CheckBox cbVencDriver = (CheckBox) findViewById(R.id.cbVencDriver);
        cbVencDriver.setTag(new VencDriver());
        cbVencDriver.setOnClickListener(this);
        mCheckBoxList.add(cbVencDriver);
    }

    @Override
    protected void onResume() {
        // TODO Auto-generated method stub
        super.onResume();
        updateUI();
    }


    private void updateUI() {

        for (CheckBox cb : mCheckBoxList) {
            VideoLogItem item = (VideoLogItem) cb.getTag();
            cb.setChecked(item.getStatus());
        }
    }


    @Override
    public void onClick(View view) {
        // TODO Auto-generated method stub

        int checkNum = 0;
        for (CheckBox cb : mCheckBoxList) {
            VideoLogItem itemIns = (VideoLogItem) cb.getTag();
            if (itemIns.getStatus()) {
                checkNum++;
            }
        }

        VideoLogItem item = (VideoLogItem) view.getTag();

        boolean logMuchSwitchResult = true;
        if (!item.getStatus() && (checkNum == 0)) {
            logMuchSwitchResult = VideoLogItem.switchLogMuchDetect(false);
        } else if (item.getStatus() && (checkNum == 1)) {
            logMuchSwitchResult = VideoLogItem.switchLogMuchDetect(true);
        }
        Elog.i(TAG, "logMuchSwitchResult: " + logMuchSwitchResult);
        if (logMuchSwitchResult) {
            item.switchStatus();
            ((CheckBox) view).setChecked(item.getStatus());
        }

    }

}
