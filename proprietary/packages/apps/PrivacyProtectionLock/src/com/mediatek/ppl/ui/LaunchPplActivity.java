package com.mediatek.ppl.ui;

import com.mediatek.ppl.PplService;

import android.app.AlertDialog;
import android.content.DialogInterface;

import com.mediatek.ppl.R;

public class LaunchPplActivity extends PplBasicActivity {

    @Override
    protected void onPropertyConfig() {
        setProperty(PROPERTY_CLEAR);
    }

    @Override
    protected void onRegisterEvent() {
        mEventReceiver.addAction(PplService.Intents.UI_NO_SIM);

    }

    @Override
    protected void onPrepareLayout() {

        new AlertDialog.Builder(this, AlertDialog.THEME_HOLO_DARK)
        .setCancelable(false)
        .setTitle(R.string.title_choose_enable_mode)
        .setItems(R.array.enable_mode_list,
                new DialogInterface.OnClickListener() {
            @Override
            public void onClick(DialogInterface dialog, int which) {
                switch (which) {
                case 0:
                    UseEstablishedSettings();
                    break;
                case 1:
                    useNewSettings();
                    break;
                default:
                    break;
                }
            }
        })
        .setNegativeButton(android.R.string.cancel,
                new DialogInterface.OnClickListener() {
            @Override
            public void onClick(DialogInterface dialog, int which) {
                finish();
            }
        })
        .create().show();
    }

    @Override
    protected void onInitLayout() {
    }


    private void UseEstablishedSettings() {
        gotoActivity(this, LoginPplActivity.class,
                LoginPplActivity.EXTRA_ENABLE_PREVIOUS, true);
        finish();
     }

     private void useNewSettings() {
         gotoActivity(this, SetupPasswordActivity.class);
         finish();
     }

}
