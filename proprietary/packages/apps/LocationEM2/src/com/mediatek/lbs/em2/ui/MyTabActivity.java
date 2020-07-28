package com.mediatek.lbs.em2.ui;

import java.io.File;
import java.util.Arrays;
import android.Manifest;
import android.app.TabActivity;
import android.content.Intent;
import android.content.pm.PackageManager;
import android.content.SharedPreferences;
import android.os.Bundle;
import android.os.SystemProperties;
import android.util.Log;
import android.widget.TabHost;
import android.widget.Toast;

@SuppressWarnings("deprecation")
public class MyTabActivity extends TabActivity {
    private final static String [] sNeededPermissions = {
            Manifest.permission.ACCESS_FINE_LOCATION,
            Manifest.permission.ACCESS_COARSE_LOCATION,
            Manifest.permission.ACCESS_BACKGROUND_LOCATION,
            Manifest.permission.WRITE_EXTERNAL_STORAGE,
            Manifest.permission.READ_PHONE_STATE};
    final static int PERMISSION_REQUEST_BITS_ALL = (int)(Math.pow(2, sNeededPermissions.length)-1);

    private TabHost mTabHost;

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.main);
    }

    protected boolean isDebugModeEnabled() {
        File file = new File(getFilesDir() + "/../show");
        return file.exists();
    }

    private void addAllTab() {
        if (mTabHost != null) {
            log("Tabs are already added");
            return;
        }

        mTabHost = getTabHost(); // The activity TabHost
        // 0
        mTabHost.addTab(mTabHost.newTabSpec("GPS").setIndicator("GPS")
                .setContent(new Intent().setClass(this, LbsGps.class)));
        // 1
        mTabHost.addTab(mTabHost.newTabSpec("VIEW").setIndicator("VIEW")
                .setContent(new Intent().setClass(this, LbsView.class)));
        // 2
        mTabHost.addTab(mTabHost.newTabSpec("AGPS").setIndicator("AGPS")
                .setContent(new Intent().setClass(this, LbsAgps.class)));
        // 3
        mTabHost.addTab(mTabHost.newTabSpec("FLOW").setIndicator("FLOW")
                .setContent(new Intent().setClass(this, LbsFlow.class)));
        // 4
        mTabHost.addTab(mTabHost.newTabSpec("MISC").setIndicator("MISC")
                .setContent(new Intent().setClass(this, LbsMisc.class)));

        // 6
        mTabHost.addTab(mTabHost.newTabSpec("RAW DATA").setIndicator("RAW DATA")
                .setContent(new Intent().setClass(this, LbsRawData.class)));

        // 7
        mTabHost.addTab(mTabHost.newTabSpec("NLP").setIndicator("NLP")
                .setContent(new Intent().setClass(this, LbsNlp.class)));

        // 8
        mTabHost.addTab(mTabHost.newTabSpec("C2K DBG").setIndicator("C2K DBG")
                .setContent(new Intent().setClass(this, LbsC2kDbg.class)));

        mTabHost.addTab(mTabHost.newTabSpec("BATCH").setIndicator("BATCH")
                .setContent(new Intent().setClass(this, LbsBatch.class)));

        //pre-load tab view
        mTabHost.setCurrentTab(0);
        mTabHost.setCurrentTab(1);
        mTabHost.setCurrentTab(3);
        mTabHost.setCurrentTab(2);
        // start locationEM service to avoid process to be killed
        SharedPreferences sharedPreferences =
                getSharedPreferences("locationem2.data" , MODE_PRIVATE);
        boolean launchForegroundService =
                sharedPreferences.getBoolean("launchForegroundService" , true);
        if (launchForegroundService) {
            startService(new Intent().setClass(this, LocationEMService.class));
        }
    }

    @Override
    protected void onResume() {
        super.onResume();
        if (checkSelfPermission(Manifest.permission.ACCESS_FINE_LOCATION) == PackageManager.PERMISSION_GRANTED
                && checkSelfPermission(Manifest.permission.ACCESS_COARSE_LOCATION)== PackageManager.PERMISSION_GRANTED
                && checkSelfPermission(Manifest.permission.ACCESS_BACKGROUND_LOCATION)== PackageManager.PERMISSION_GRANTED
                && checkSelfPermission(Manifest.permission.WRITE_EXTERNAL_STORAGE)== PackageManager.PERMISSION_GRANTED
                && checkSelfPermission(Manifest.permission.READ_PHONE_STATE)== PackageManager.PERMISSION_GRANTED) {
            if (mTabHost == null) {
                addAllTab();
            }
        } else {
            int grantResultBits = checkAndRequestPermission(
                     PERMISSION_REQUEST_BITS_ALL);
            log("onResume: checkAndRequestPermission");
        }
    }
    // =================== basic utility ========================\\
    protected void onDestroy() {
        super.onDestroy();
        // stop locationEM service
        stopService(new Intent().setClass(this, LocationEMService.class));
    }

    @SuppressWarnings(value = { "unused" })
    private void log(String msg) {
        Log.d("LocationEM", msg);
    }

    private int checkAndRequestPermission(int reqBitmap) {
        int grantResultBits = 0;
        reqBitmap &= PERMISSION_REQUEST_BITS_ALL;
        if (reqBitmap == 0) return grantResultBits;

        int total = Integer.bitCount(reqBitmap);
        String [] perms = new String [total];

        int i, j;
        for (i = j = 0; i < sNeededPermissions.length; i++) {
            if (((1 << i) & reqBitmap) != 0) {
                if (checkSelfPermission(sNeededPermissions[i])
                        != PackageManager.PERMISSION_GRANTED) {
                    perms[j++] = sNeededPermissions[i];
                   // Should we show an explanation?
                   if (shouldShowRequestPermissionRationale(
                           sNeededPermissions[i])) {
                       // Explain to the user why we need to read the contacts
                       log("We need " + sNeededPermissions[i] + " to work");
                   }
                } else {
                    reqBitmap &= ~(1 << i);
                    grantResultBits |= (1 << i);
                }
            }
        }

        if (j < total) {
            total = j;
            if (total == 0) return grantResultBits;
            perms = Arrays.copyOf(perms, total);
        }

        requestPermissions(perms, reqBitmap);

        return grantResultBits;
    }

    @Override
    public void onRequestPermissionsResult(int requestCode,
            String permissions[], int[] grantResults) {
                log("onRequestPermissionsResult: reqCode=" + requestCode);
                for (int i = 0; i < permissions.length; i++) {
                    log("onRequestPermissionsResult: perm[" + i + "]=" + permissions[i] + ", result=" + grantResults[i]);
                    if (grantResults[i] == PackageManager.PERMISSION_GRANTED) {
                        if (checkSelfPermission(Manifest.permission.ACCESS_FINE_LOCATION) == PackageManager.PERMISSION_GRANTED
                                && checkSelfPermission(Manifest.permission.ACCESS_COARSE_LOCATION)== PackageManager.PERMISSION_GRANTED
                                && checkSelfPermission(Manifest.permission.ACCESS_BACKGROUND_LOCATION)== PackageManager.PERMISSION_GRANTED
                                && checkSelfPermission(Manifest.permission.WRITE_EXTERNAL_STORAGE)== PackageManager.PERMISSION_GRANTED
                                && checkSelfPermission(Manifest.permission.READ_PHONE_STATE)== PackageManager.PERMISSION_GRANTED) {
                            addAllTab();
                        // permission was granted, yay! do the
                        // calendar task you need to do.
                        }
                    } else if (grantResults[i] != PackageManager.PERMISSION_GRANTED) {
                        if (shouldShowRequestPermissionRationale(Manifest.permission.ACCESS_FINE_LOCATION) == false
                                && shouldShowRequestPermissionRationale(Manifest.permission.ACCESS_COARSE_LOCATION) == false
                                && shouldShowRequestPermissionRationale(Manifest.permission.ACCESS_BACKGROUND_LOCATION) == false
                                && shouldShowRequestPermissionRationale(Manifest.permission.WRITE_EXTERNAL_STORAGE) == false
                                && shouldShowRequestPermissionRationale(Manifest.permission.READ_PHONE_STATE) == false) {
                            Toast.makeText(this, "You've disabled the permission, please allow permission from Settings -> Apps -> Menu -> Show system -> LocationEM2 -> Permissions to enable all permissions",
                                Toast.LENGTH_SHORT).show();
                        }
                        finish();
                        // permission denied, boo! Disable the
                        // functionality that depends on this permission.
                    }
                }
                return ;
    }
}
