/*
* Copyright (C) 2014 MediaTek Inc.
* Modification based on code covered by the mentioned copyright
* and/or permission notice(s).
*/

/*
 * Copyright (C) 2008 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package com.android.music;

import android.app.Activity;
import android.app.Dialog;
import android.app.DialogFragment;
import android.content.DialogInterface;
import android.media.AudioManager;
import android.os.Bundle;
import android.view.View;
import android.view.View.OnClickListener;
import android.view.Window;
import android.widget.Button;
import android.widget.EditText;
import android.widget.NumberPicker;
import android.widget.NumberPicker.OnValueChangeListener;

public class WeekSelector extends DialogFragment {
    /// M: constant and variable @{
    private static final String TAG = "WeekSelector";
    private static final int ALERT_DIALOG_KEY = 0;
    private static final int WEEK_START = 1;
    private static final int WEEK_END = 12;
    private static final int UPDATE_INTERVAL = 200;
    private static final int EDITTEXT_POSITION = 0;
    private int mCurrentSelectedPos;
    private NumberPicker mNumberPicker;
    private Dialog dialog;
    /// @}
     public static WeekSelector newInstance(Boolean single) {
        WeekSelector frag = new WeekSelector();
        Bundle args = new Bundle();
        frag.setArguments(args);
        return frag;
    }
    @Override
 public Dialog onCreateDialog(Bundle savedInstanceState) {

        MusicLogUtils.v(TAG, "onCreateView>>");
        dialog = new Dialog(getActivity());

        dialog.setContentView(R.layout.weekpicker);
        dialog.setTitle(R.string.weekpicker_title);
        Button tv = (Button) dialog.findViewById(R.id.weeks_cancel);
        tv.setText(getResources().getString(R.string.cancel));
        tv.setOnClickListener(new OnClickListener() {
    @Override
            public void onClick(View viewIn) {

                if (dialog != null) {
                        dialog.dismiss();
                        dialog = null;
                    }
                }
        });
        tv = (Button) dialog.findViewById(R.id.weeks_done);
        tv.setText(getResources().getString(R.string.weekpicker_set));
        tv.setOnClickListener(new OnClickListener() {
            @Override
            public void onClick(View viewIn) {
                int numweeks = mCurrentSelectedPos;
                MusicUtils.setIntPref(getActivity().getApplicationContext(), "numweeks", numweeks);
                if (dialog != null) {
                        dialog.dismiss();
                        dialog = null;
                    }
                }
        });
       // dialog.setPositiveButton(getResources().getString(R.string.weekpicker_set));
      //  dialog.setNeutralButton(getResources().getString(R.string.cancel));
        dialog.setCanceledOnTouchOutside(true);
        dialog.setCancelable(true);
        mNumberPicker = (NumberPicker) dialog.findViewById(R.id.weeks);
        mNumberPicker.setOnValueChangedListener(mChangeListener);
        mNumberPicker.setDisplayedValues(getResources().getStringArray(R.array.weeklist));

        int def = MusicUtils.getIntPref(getActivity().getApplicationContext(),
                   "numweeks", WEEK_START);
        int pos = savedInstanceState != null ? savedInstanceState.getInt("numweeks", def) : def;
        mCurrentSelectedPos = pos;
        mNumberPicker.setMinValue(WEEK_START);
        mNumberPicker.setMaxValue(WEEK_END);
        mNumberPicker.setValue(pos);
        mNumberPicker.setWrapSelectorWheel(false);
        mNumberPicker.setOnLongPressUpdateInterval(UPDATE_INTERVAL);

        EditText spinnerInput = (EditText) mNumberPicker.getChildAt(EDITTEXT_POSITION);
        if (spinnerInput != null) {
            spinnerInput.setFocusable(false);
        }
        dialog.show();
        return dialog;
    }

    @Override
    public void onSaveInstanceState(Bundle outcicle) {
        outcicle.putInt("numweeks", mCurrentSelectedPos);
    }

   public void setOnClickListener(DialogInterface.OnClickListener listener) {
        //mClickListener = listener;
      }

    /**
     * M: This listener save the week number to SharedPreferences when user click positive button
     * otherwise finish current acitivity
     */
    final private DialogInterface.OnClickListener  mButtonClicked =
     new DialogInterface.OnClickListener() {
        public void onClick(DialogInterface mDialogInterface, int button) {
            if (button == DialogInterface.BUTTON_POSITIVE) {
                int numweeks = mCurrentSelectedPos;
                MusicUtils.setIntPref(getActivity().getApplicationContext(), "numweeks", numweeks);
                //setResult(RESULT_OK);
            } else if (button == DialogInterface.BUTTON_NEUTRAL) {
               // setResult(RESULT_CANCELED);
            }
            //finish();
        }
    };

    /**
     * M: to create dialogs for week select
     */


    /**
     * M: This listener monitor the value change of NumberPicker
     */
    OnValueChangeListener mChangeListener = new OnValueChangeListener() {
        public void onValueChange(NumberPicker picker, int oldVal, int newVal) {
            if (picker == mNumberPicker) {
                mCurrentSelectedPos = newVal;
            }
        }
    };
}
