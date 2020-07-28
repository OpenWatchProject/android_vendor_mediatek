package com.mediatek.op08.settings.mulitine;

import android.app.Activity;
import android.content.Intent;
import android.os.Bundle;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.Button;
import android.widget.EditText;

/**
 * Activity to add new number for request.
 */
public class ActivateNew extends Activity {
    private EditText mEditTextLineNumber;
    private Button mButtonSendNewLine;
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_activate_new);
        mEditTextLineNumber = (EditText) findViewById(R.id.editTextNewLineNumber);
        mButtonSendNewLine = (Button) findViewById(R.id.requestButton);
        mButtonSendNewLine.setOnClickListener(new OnClickListener() {
            @Override
            public void onClick(View arg0) {
                String message = mEditTextLineNumber.getText().toString();
                Intent intent = new Intent();
                intent.putExtra("MESSAGE", message);
                setResult(RESULT_OK, intent);
                finish(); //finishing activity
            }
        });
    }
}
