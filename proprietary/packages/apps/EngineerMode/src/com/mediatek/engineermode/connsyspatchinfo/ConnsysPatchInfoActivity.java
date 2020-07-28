package com.mediatek.engineermode.connsyspatchinfo;

import android.app.Activity;
import android.os.Bundle;
import android.os.SystemProperties;
import android.view.View;
import android.widget.TextView;

import com.mediatek.engineermode.Elog;
import com.mediatek.engineermode.R;

import java.util.regex.Matcher;
import java.util.regex.Pattern;

/**
 * Show connsys patch information.
 *
 */
public class ConnsysPatchInfoActivity extends Activity  {

    private static final String TAG = "ConnsysPatchInfo";
    private static final String PROPERTY = "persist.vendor.connsys.patch.version";
    private static final String PROPERTY_FORMAT = "persist.vendor.connsys.bt_fw_ver";
    private static final String BRANCH_PATTERN = "t-neptune[\\w-]*SOC[a-zA-Z0-9]*_[a-zA-Z0-9]*_";
    private static final String VER_SPLIT = "-";
    private static final String PROP_INVALID = "-1";

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.connsys_patch_info);
        showVersion();
    }
    private void showVersion() {
        String strInfo = SystemProperties.get(PROPERTY_FORMAT);
        Elog.v(TAG, "strInfo:" + strInfo);
        if ((strInfo != null) && (!strInfo.isEmpty()) && (!strInfo.equals(PROP_INVALID))) {
            View vPatchLayout = findViewById(R.id.connsys_patch_layout);
            vPatchLayout.setVisibility(View.GONE);
            View vBranchLayout = findViewById(R.id.connsys_branch_layout);
            vBranchLayout.setVisibility(View.VISIBLE);
            View vVerLayout = findViewById(R.id.connsys_ver_layout);
            vVerLayout.setVisibility(View.VISIBLE);

            String strBranch = getBranch(strInfo);

            if (strBranch != null) {
                TextView tvBranch = (TextView) findViewById(R.id.connsys_branch_tv);
                strBranch = strBranch.substring(0, strBranch.length() - 1);
                Elog.v(TAG, "strBranch:" + strBranch);
                tvBranch.setText(strBranch);
            }
            String strVer = getVer(strInfo);
            Elog.v(TAG, "strVer:" + strVer);
            if (strVer != null) {
                TextView tvVer = (TextView) findViewById(R.id.connsys_ver_tv);
                tvVer.setText(strVer);
            }
        } else {
            String strVer = SystemProperties.get(PROPERTY);
            Elog.i(TAG, "version:" + strVer);
            TextView text = (TextView) findViewById(R.id.rom_patch_ver_tv);
            if ((strVer != null) && (!strVer.isEmpty())) {
                text.setText(strVer);
            }
        }
    }
    private String getBranch(String strInfo) {
        Pattern pattern = Pattern.compile(BRANCH_PATTERN);
        Matcher matcher = pattern.matcher(strInfo);
        if (matcher.find()) {
            return matcher.group(0);
        }
        return null;
    }

    private String getVer(String strInfo) {
        String strVersion = strInfo;
        int index = strInfo.lastIndexOf(VER_SPLIT);
        if ((index > 0) && (strInfo.length() > (index + 1))) {
            strVersion = strInfo.substring(index + 1);
        }
        int length = 8; //Date of year/month/day
        int year = 4; //End pos of year
        int month = 6; //End pos of month
        int day = 8; //End pos of day
        if (strVersion.length() < length) {
            return null;
        }
        String strYear = strVersion.substring(0, year);
        String strMonth = strVersion.substring(year, month);
        String strDay = strVersion.substring(month, day);
        String strSplit = "-";
        StringBuilder sbVer = new StringBuilder(strYear);
        sbVer.append(strSplit).append(strMonth).append(strSplit).append(strDay);
        if (strVersion.length() > length) {
            sbVer.append(strSplit).append(strVersion.substring(day));
        }
        return sbVer.toString();
    }
}
