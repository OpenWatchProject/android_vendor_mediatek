package com.mediatek.ppl.ui;

import android.os.Bundle;
import android.text.Editable;
import android.text.InputType;
import android.text.TextWatcher;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.Button;
import android.widget.CheckBox;
import android.widget.CompoundButton;
import android.widget.Toast;
import android.widget.CompoundButton.OnCheckedChangeListener;
import android.widget.EditText;
import android.widget.LinearLayout;
import android.widget.ProgressBar;

import com.mediatek.ppl.PplService;
import com.mediatek.ppl.R;

public class LoginPplActivity extends PplBasicActivity {

    public static final String EXTRA_ENABLE_PREVIOUS = "enable_previous";

    private boolean mEnableEstablished;

    private ProgressBar mProgressBar;
    private LinearLayout mLayoutUp;
    private LinearLayout mLayoutDown;
    private Button mBtnConfirm;
    private EditText mEditTextPw;
    private CheckBox mCheckBoxShow;

    @Override
    protected void onCreate(Bundle saveInstanceState) {
        mEnableEstablished = getIntent().getBooleanExtra(EXTRA_ENABLE_PREVIOUS, false);
        super.onCreate(saveInstanceState);
    }

    @Override
    protected void onResume() {
        mCheckBoxShow.setText(R.string.checkbox_show_password);
        super.onResume();
    }

    @Override
    protected void onRegisterEvent() {
        mEventReceiver.addAction(PplService.Intents.UI_QUIT_SETUP_WIZARD);
        mEventReceiver.addAction(PplService.Intents.UI_NO_SIM);
    }

    @Override
    protected void onPrepareLayout() {
        setContentView(R.layout.login_ppl);

        mProgressBar = (ProgressBar) findViewById(R.id.common_progress);
        mLayoutUp = (LinearLayout) findViewById(R.id.layout_login_ppl_up);
        mLayoutDown = (LinearLayout) findViewById(R.id.layout_login_ppl_down);
        mEditTextPw = (EditText) findViewById(R.id.et_login_ppl_input);
        mCheckBoxShow = (CheckBox) findViewById(R.id.cb_login_ppl_show_pw);
        mBtnConfirm = (Button) findViewById(R.id.btn_bottom_confirm);

        mEditTextPw.addTextChangedListener(new TextWatcher() {

            @Override
            public void onTextChanged(CharSequence s, int start, int before,
                    int count) {
                if (mEditTextPw.getText().length() > 0) {
                    mBtnConfirm.setEnabled(true);
                } else {
                    mBtnConfirm.setEnabled(false);
                }
            }

            @Override
            public void beforeTextChanged(CharSequence s, int start, int count,
                    int after) {
                // TODO Auto-generated method stub
            }


            @Override
            public void afterTextChanged(Editable s) {
                // TODO Auto-generated method stub
            }

        });

        mCheckBoxShow.setOnCheckedChangeListener(new OnCheckedChangeListener() {

            @Override
            public void onCheckedChanged(CompoundButton buttonView,
                    boolean isChecked) {
                int pos = mEditTextPw.getSelectionStart() == mEditTextPw.getSelectionEnd() ?
                        mEditTextPw.getSelectionStart() : mEditTextPw.getText().length();
                if (isChecked) {
                    mEditTextPw.setInputType(InputType.TYPE_CLASS_NUMBER |
                            InputType.TYPE_NUMBER_VARIATION_NORMAL);
                } else {
                    mEditTextPw.setInputType(InputType.TYPE_CLASS_NUMBER |
                            InputType.TYPE_NUMBER_VARIATION_PASSWORD);
                }
                mEditTextPw.setSelection(pos);
            }

        });

        mBtnConfirm.setOnClickListener(new OnClickListener() {

            @Override
            public void onClick(View v) {
                gotoNext();
            }

        }) ;

    }

    @Override
    protected void onInitLayout() {
        mBtnConfirm.setEnabled(false);
        mLayoutUp.setVisibility(View.GONE);
        mLayoutDown.setVisibility(View.GONE);
        mProgressBar.setVisibility(View.VISIBLE);
        mEditTextPw.requestFocus();
    }

    @Override
    protected void onPplServiceConnected(Bundle saveInstanceState) {
        mLayoutUp.setVisibility(View.VISIBLE);
        mLayoutDown.setVisibility(View.VISIBLE);
        mProgressBar.setVisibility(View.GONE);
    }

    private void gotoNext() {
        if (mBinder.verifyPassword(mEditTextPw.getText().toString())) {
            if (mEnableEstablished) {
                mEnableEstablished = false;
                mBinder.enable(true);
            }
            gotoActivity(this, ControlPanelActivity.class);
            finish();
        } else {
            Toast.makeText(this, R.string.toast_password_is_incorrect, Toast.LENGTH_SHORT).show();
        }
    }

}
