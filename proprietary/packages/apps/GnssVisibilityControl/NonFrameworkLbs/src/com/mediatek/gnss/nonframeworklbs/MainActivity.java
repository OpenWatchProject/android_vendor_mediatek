package com.mediatek.gnss.nonframeworklbs;

import android.app.Activity;
import android.content.Intent;
import android.net.Uri;
import android.os.Bundle;
import android.provider.Settings;
import android.view.View;
import android.widget.Button;

public class MainActivity extends Activity {


    private Button          mButtonLaunchPermission;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        mButtonLaunchPermission      = (Button) findViewById(R.id.Button_permission_settings);

        mButtonLaunchPermission.setOnClickListener(new View.OnClickListener() {
            public void onClick(View v) {
                launchPackageSettings();
            }
        });
    }

    private void launchPackageSettings() {
        Intent intent = new Intent();
        intent.setAction(Settings.ACTION_APPLICATION_DETAILS_SETTINGS);
        Uri uri = Uri.fromParts("package", getApplicationContext().getPackageName(), null);
        intent.setData(uri);
        startActivity(intent);
    }
}
