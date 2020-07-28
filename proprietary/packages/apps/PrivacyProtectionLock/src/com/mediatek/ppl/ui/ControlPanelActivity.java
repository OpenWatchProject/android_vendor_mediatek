package com.mediatek.ppl.ui;

import java.util.List;

import android.content.Intent;
import android.os.Bundle;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.Button;
import android.widget.ProgressBar;
import android.widget.ScrollView;
import android.widget.TextView;

import com.mediatek.ppl.PlatformManager;
import com.mediatek.ppl.PplService;
import com.mediatek.ppl.R;

public class ControlPanelActivity extends PplBasicActivity {

    private static final String TAG = "ControlPanelActivity";
    private ScrollView mScrollView;
    private ProgressBar mProgressBar;
    private TextView mTextViewStatus;
    private TextView mTextViewContacts[] = new TextView[3];
    private Button mBtnChangePin;
    private Button mBtnUpdateEm;
    private Button mBtnViewCmd;
    private Button mBtnTurnOff;
    private boolean mIsStopped;

    @Override
    public void onResume() {
        super.onResume();
        if (null != mBinder) {
            updateStatus();
        }
        mIsStopped = false;
    }

    @Override
    public void onStop() {
        mIsStopped = true;
        super.onStop();
    }

    @Override
    public void onDestroy() {
        if (null != mBinder) {
            mBinder.unregisterSensitiveActivity(this);
        }
        super.onDestroy();
    }

    @Override
    protected void onPropertyConfig() {
        setProperty(PROPERTY_NEED_SERVICE | PROPERTY_HAS_ACTIONBAR | PROPERTY_QUIT_BACKGROUND);
    }

    @Override
    protected void onRegisterEvent() {
        mEventReceiver.addAction(Intent.ACTION_SCREEN_OFF);
        mEventReceiver.addAction(PplService.Intents.UI_NO_SIM);
    }

    @Override
    protected void onPrepareLayout() {

        setContentView(R.layout.control_panel);

        mScrollView = (ScrollView) findViewById(R.id.layout_control_panel_scrollview);
        mProgressBar = (ProgressBar) findViewById(R.id.common_progress);

        mTextViewStatus = (TextView) findViewById(R.id.tv_control_panel_status);
        mTextViewContacts[0] = (TextView) findViewById(R.id.tv_control_panel_contact_1);
        mTextViewContacts[1] = (TextView) findViewById(R.id.tv_control_panel_contact_2);
        mTextViewContacts[2] = (TextView) findViewById(R.id.tv_control_panel_contact_3);
        mBtnChangePin = (Button) findViewById(R.id.btn_control_panel_changepin);
        mBtnUpdateEm = (Button) findViewById(R.id.btn_control_panel_updateem);
        mBtnViewCmd = (Button) findViewById(R.id.btn_control_panel_viewcmd);
        mBtnTurnOff = (Button) findViewById(R.id.btn_control_panel_turnoff);

        mBtnChangePin.setOnClickListener(clickListner);
        mBtnUpdateEm.setOnClickListener(clickListner);
        mBtnViewCmd.setOnClickListener(clickListner);
        mBtnTurnOff.setOnClickListener(clickListner);
    }

    OnClickListener clickListner = new OnClickListener() {

        @Override
        public void onClick(View v) {
            switch(v.getId()) {
            case R.id.btn_control_panel_changepin:
                gotoChangePin();
                break;

            case R.id.btn_control_panel_updateem:
                gotouUpdateContacts();
                break;

            case R.id.btn_control_panel_viewcmd:
                gotoViewManual();
                break;

            case R.id.btn_control_panel_turnoff:
                DisablePpl();
                break;

            default:

                break;
            }
        }
    };


    @Override
    protected void onInitLayout() {
        mScrollView.setVisibility(View.GONE);
        mProgressBar.setVisibility(View.VISIBLE);
    }

    @Override
    protected void onPplServiceConnected(Bundle saveInstanceState) {
        mBinder.registerSensitiveActivity(ControlPanelActivity.this);
        updateStatus();
        mScrollView.setVisibility(View.VISIBLE);
        mProgressBar.setVisibility(View.GONE);
    }

    private void updateStatus() {
        mTextViewStatus.setText(mBinder.isEnabled() ?
                                R.string.status_control_panel_enabled :
                                R.string.status_control_panel_disabled);

        List<String> contacts = mBinder.loadTrustedNumberList(PplService.EDIT_TYPE_UPDATE);
        for (int i = 0; i < mTextViewContacts.length; i++) {
            if (null != contacts && i < contacts.size()) {
                String name = PlatformManager.getContactNameByPhoneNumber(this, contacts.get(i));
                mTextViewContacts[i].setText(null == name ?
                        contacts.get(i) : contacts.get(i) + " " + name);
                mTextViewContacts[i].setVisibility(View.VISIBLE);
            } else {
                mTextViewContacts[i].setVisibility(View.GONE);
            }
        }
    }

    private void gotoChangePin() {
        gotoActivity(this, UpdatePasswordActivity.class);
        finish();
    }

    private void gotouUpdateContacts() {
        gotoActivity(this, UpdateTrustedContactsActivity.class);
        finish();
    }

    private void gotoViewManual() {
        gotoActivity(this, ViewManualActivity.class);
        finish();
    }

    private void DisablePpl() {
        if (!mIsStopped) {
            DialogDisablePplFragment frg = DialogDisablePplFragment.newInstance();
            frg.show(getFragmentManager(), "disable_dialog");
        }
    }

    public void onDiableConfirmed() {
        mBinder.disable();
        finish();
    }
}
