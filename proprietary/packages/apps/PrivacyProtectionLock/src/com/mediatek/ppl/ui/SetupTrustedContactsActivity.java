/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein is
 * confidential and proprietary to MediaTek Inc. and/or its licensors. Without
 * the prior written permission of MediaTek inc. and/or its licensors, any
 * reproduction, modification, use or disclosure of MediaTek Software, and
 * information contained herein, in whole or in part, shall be strictly
 * prohibited.
 *
 * MediaTek Inc. (C) 2010. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
 * ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 * NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH
 * RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 * INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES
 * TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
 * RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
 * OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN MEDIATEK
 * SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE
 * RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S
 * ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE
 * RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE
 * MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE
 * CHARGE PAID BY RECEIVER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek
 * Software") have been modified by MediaTek Inc. All revisions are subject to
 * any receiver's applicable license agreements with MediaTek Inc.
 */

package com.mediatek.ppl.ui;

import java.util.LinkedList;
import java.util.List;

import com.mediatek.ppl.PlatformManager;
import com.mediatek.ppl.PlatformManager.ContactQueryResult;
import com.mediatek.ppl.PplService;
import com.mediatek.ppl.R;

import android.content.Context;
import android.content.Intent;
import android.net.Uri;
import android.os.Bundle;
import android.provider.ContactsContract;
import android.text.Editable;
import android.text.TextWatcher;
import android.util.Log;
import android.view.KeyEvent;
import android.view.LayoutInflater;
import android.view.View;
import android.view.View.OnClickListener;
import android.view.View.OnFocusChangeListener;
import android.view.inputmethod.EditorInfo;
import android.view.inputmethod.InputMethodManager;
import android.widget.Button;
import android.widget.EditText;
import android.widget.ImageButton;
import android.widget.LinearLayout;
import android.widget.ProgressBar;
import android.widget.ScrollView;
import android.widget.TextView;
import android.widget.TextView.OnEditorActionListener;
import android.widget.Toast;


interface IContactLineListener {
    public void onAddBtnClicked(int id);
    public void onDelBtnClicked(int id);
    public void onEditTextClicked(int id);
    public void onFocusChanged(int id, boolean hasFocus);
    public void afterTextChanged(int id, Editable s);
}

public class SetupTrustedContactsActivity extends PplBasicActivity
        implements PplRelativeLayout.IOnResizeListener, IContactLineListener {

    private static final String TAG = "PPL/SetupTrustedContactsActivity";
    private static final String KEY_COUNT = "count";
    private static final String KEY_FOCUS = "focus";
    private static final String KEY_NUMBERS = "numbers";
    private static final String KEY_NAMES = "names";

    private static final boolean VDBG = false;

    private LayoutInflater mInflater;
    private ScrollView mScrollView;
    private ProgressBar mProgressBar;
    private PplRelativeLayout mLayoutOuter;
    private LinearLayout mLayoutUp;
    private LinearLayout mLayoutDown;
    private LinearLayout mLayoutEditor;
    protected Button mNextButton;
    private Button mNewContactButton;

    private boolean mTextChangeByManul = true;
    private LinkedList<ContactLine> mContactLines = new LinkedList<ContactLine>();
    private PendingActivityResult mPendingActivityResult;

    private boolean mScrollFlag = false;

    private class ContactLine {
        public LinearLayout mLine;
        public EditText     mEditText;
        private ImageButton  mAddBtn;
        private ImageButton  mDelBtn;
        private IContactLineListener mListener;

        private String mInputStr = null;
        private String mNameStr = null;

        public ContactLine(LayoutInflater inflater, int id, IContactLineListener l) {
            mLine = (LinearLayout) inflater.inflate(R.layout.contact_line, null);
            mLine.setId(id);
            mEditText = (EditText) mLine.findViewById(R.id.et_contact_line_edit);
            mAddBtn = (ImageButton) mLine.findViewById(R.id.ib_contact_line_add);
            mDelBtn = (ImageButton) mLine.findViewById(R.id.ib_contact_line_delete);
            mDelBtn.setVisibility(View.INVISIBLE);
            mListener = l;

            mAddBtn.setOnClickListener(new OnClickListener() {
                @Override
                public void onClick(View v) {
                    mListener.onAddBtnClicked(mLine.getId());
                }
            });

            mDelBtn.setOnClickListener(new OnClickListener() {
                @Override
                public void onClick(View v) {
                    mInputStr = null;
                    mNameStr = null;
                    updateContactText(mLine.getId());
                    mDelBtn.setVisibility(View.INVISIBLE);
                    mListener.onDelBtnClicked(mLine.getId());
                }
            });

            mEditText.addTextChangedListener(new TextWatcher() {
                @Override
                public void beforeTextChanged(CharSequence s, int start, int count, int after) {
                }
                @Override
                public void onTextChanged(CharSequence s, int start, int before, int count) {
                }
                @Override
                public void afterTextChanged(Editable s) {
                    if (mTextChangeByManul) {
                        mInputStr = s.toString();
                        mNameStr = null;
                    }
                    if (0 == s.length()) {
                        mDelBtn.setVisibility(View.INVISIBLE);
                    } else {
                        mDelBtn.setVisibility(View.VISIBLE);
                    }
                    mListener.afterTextChanged(mLine.getId(), s);
                }
            });

            mEditText.setOnClickListener(new OnClickListener() {
                @Override
                public void onClick(View v) {
                    Log.d(TAG, "onClick item index = " + mLine.getId());
                    if (!mEditText.getText().toString().equals(mInputStr)) {
                        updateContactText(mLine.getId());
                    }
                    mEditText.setSelection(mEditText.getText().length());
                    mListener.onEditTextClicked(mLine.getId());
                }
            });

            mEditText.setOnFocusChangeListener(new OnFocusChangeListener() {
                @Override
                public void onFocusChange(View v, boolean hasFocus) {
                    if (VDBG) Log.d(TAG, "onFocusChange index = " + mLine.getId());
                    if (mScrollFlag) {
                        // Just do nothing if focus change is trigger from onPanelChange(int).
                        // For CR[ALPS01936645].
                        return;
                    }
                    if (!hasFocus) {
                        if (null != mInputStr && mInputStr.length() > 0 && null == mNameStr) {
                            mNameStr = PlatformManager.getContactNameByPhoneNumber(
                                    v.getContext(), mInputStr);
                        }
                        if (null != mNameStr && mNameStr.length() > 0) {
                            updateContactText(mLine.getId());
                        }
                    } else {
                        if (!mEditText.getText().toString().equals(mInputStr) && null != mInputStr) {
                            updateContactText(mLine.getId());
                        }
                    }
                    mListener.onFocusChanged(mLine.getId(), hasFocus);
                }
            });

            mEditText.setOnEditorActionListener(new OnEditorActionListener() {
                @Override
                public boolean onEditorAction(TextView v, int actionId, KeyEvent event) {
                    if (EditorInfo.IME_ACTION_DONE == actionId) {
                        InputMethodManager imm = (InputMethodManager) v.getContext().
                                getSystemService(Context.INPUT_METHOD_SERVICE);
                        imm.hideSoftInputFromWindow(v.getWindowToken(), 0);
                        return true;
                    }
                    return false;
                }
            });
        }
    }

    public class PendingActivityResult {
        public int requestCode;
        public int resultCode;
        public Intent data;

        public PendingActivityResult(int requestCode, int resultCode, Intent data) {
            this.requestCode = requestCode;
            this.resultCode = resultCode;
            this.data = data;
        }
    }

    @Override
    protected void onActivityResult(int requestCode, int resultCode, Intent data) {
        Log.d(TAG, "onActivityResult: " + requestCode + ", " + resultCode);
        if (null != mPendingActivityResult) {
            throw new Error("mPendingActivityResult is no null, check for bug");
        } else {
            mPendingActivityResult = new PendingActivityResult(requestCode, resultCode, data);
        }
        if (mBinder != null) {
            progressPendingActivityResult();
        }
    }

    private void progressPendingActivityResult() {

        if (null == mPendingActivityResult) {
            Log.d(TAG, "No pending activity result");
            return;
        }

        PendingActivityResult par = mPendingActivityResult;
        mPendingActivityResult = null;
        ContactQueryResult info;

        if (RESULT_OK != par.resultCode) {
            Log.d(TAG, "Pending activity error: " + par.requestCode);
            return;
        }

        Uri contactURI = par.data.getData();
        if (null == contactURI) {
            Toast.makeText(this, R.string.toast_no_phone_number, Toast.LENGTH_SHORT).show();
            return;
        }

        info = PlatformManager.getContactInfo(this, contactURI);
        if (0 == info.phones.size()) {
            Toast.makeText(this, R.string.toast_no_phone_number, Toast.LENGTH_SHORT).show();
            return;
        }

        if (1 == info.phones.size()) {
            setContectBySelect(info.phones.get(0), info.name, par.requestCode);
        } else {
            Log.d(TAG, "Multiple phone Number: " + info.phones);
            DialogChooseNumFragment frg = DialogChooseNumFragment.
                    newInstance(info.phones.toArray(new String[0]), info.name, par.requestCode);
            frg.show(getFragmentManager(), "choose_num");
        }
    }

    public void setContectBySelect(final String number, final String name, int index) {
        ContactLine cl = mContactLines.get(index);
        cl.mNameStr = name;
        cl.mInputStr = number;
        mLayoutEditor.requestFocus();
        updateContactText(index);
    }

    @Override
    public void onSaveInstanceState(Bundle outState) {
        super.onSaveInstanceState(outState);
        int size = mContactLines.size();
        int focus = -1;
        String[] numbers = new String[size];
        String[] names = new String[size];

        for (int i = 0; i < size ; i++) {
            ContactLine cl = mContactLines.get(i);
            if (null != cl.mEditText && cl.mEditText.hasFocus()) {
                focus = i;
            }
            if (null != cl.mEditText) {
                numbers[i] = mContactLines.get(i).mInputStr;
            }
            if (null != cl.mNameStr) {
                names[i] = mContactLines.get(i).mNameStr;
            }
        }
        outState.putInt(KEY_COUNT, size);
        outState.putInt(KEY_FOCUS, focus);
        outState.putStringArray(KEY_NUMBERS, numbers);
        outState.putStringArray(KEY_NAMES, names);
    }

    @Override
    protected void onRegisterEvent() {
        mEventReceiver.addAction(PplService.Intents.UI_QUIT_SETUP_WIZARD);
        mEventReceiver.addAction(PplService.Intents.UI_NO_SIM);
    }

    @Override
    protected void onPrepareLayout() {

        setContentView(R.layout.setup_trusted);

        mInflater = getLayoutInflater();

        mScrollView = (ScrollView) findViewById(R.id.scrollview_setup_trusted);
        mLayoutOuter = (PplRelativeLayout) findViewById(R.id.layout_setup_trusted_outer);
        mLayoutUp = (LinearLayout) findViewById(R.id.layout_setup_trusted_up);
        mLayoutDown = (LinearLayout) findViewById(R.id.layout_setup_trusted_down);
        mLayoutEditor = (LinearLayout) findViewById(R.id.layout_setup_trusted_editor);

        mProgressBar = (ProgressBar) findViewById(R.id.common_progress);
        mNextButton = (Button) findViewById(R.id.btn_bottom_next);
        mNextButton.setOnClickListener(new OnClickListener() {

            @Override
            public void onClick(View v) {
                onButtonClicked();
            }

        });

        mNewContactButton = (Button) findViewById(R.id.btn_setup_trusted_add_contact);
        mNewContactButton.setOnClickListener(new OnClickListener() {
            @Override
            public void onClick(View v) {
                addNewContact(null);
                v.requestFocus();
            }
        });


        mLayoutOuter.setOnResizeListener(this);

    }

    private void addNewContact(String number) {
        if (mContactLines.size() >=  PplService.MAX_CONTACTS_NUM) {
            Log.e(TAG, "ERROR: contact has reach limitation, new contact fail");
            return;
        }

        ContactLine cl = new ContactLine(mInflater, mContactLines.size(), this);
        if (null != number) {
            cl.mInputStr = number;
            String name = PlatformManager.getContactNameByPhoneNumber(this, number);
            if (null != name && name.length() > 0) {
                cl.mNameStr = name;
            }
        }
        mContactLines.addLast(cl);
        mLayoutEditor.addView(cl.mLine, mContactLines.size() - 1);
        cl.mEditText.requestFocus();
        updateContactText(mContactLines.size() - 1);
        updateNewBtnStatus();
    }


    @Override
    protected void onInitLayout() {
        mProgressBar.setVisibility(View.VISIBLE);
        mLayoutUp.setVisibility(View.GONE);
        mLayoutDown.setVisibility(View.GONE);
        mNextButton.setEnabled(false);
        mNewContactButton.setVisibility(View.GONE);
    }

    @Override
    protected void onPplServiceConnected(Bundle saveInstanceState) {
        mProgressBar.setVisibility(View.GONE);
        mLayoutUp.setVisibility(View.VISIBLE);
        mLayoutDown.setVisibility(View.VISIBLE);

        String[] numbers = null;
        int focus = -1;
        if (null != saveInstanceState) {
            focus = saveInstanceState.getInt(KEY_FOCUS, -1);
            numbers = saveInstanceState.getStringArray(KEY_NUMBERS);
            //String[] names = saveInstanceState.getStringArray(KEY_NAMES);
        } else {
            focus = 0;
            List<String> numberList = mBinder.loadTrustedNumberList(PplService.EDIT_TYPE_SETUP);
            if (null != numberList) {
                numbers = (String[]) numberList.toArray(new String[numberList.size()]);
            }
        }
        if (null != numbers && numbers.length > 0) {
            int i = 0;
            while (i < numbers.length) {
                addNewContact(numbers[i]);
                i ++;
            }
        } else {
            addNewContact(null);
        }
        if (focus >= 0) {
            mContactLines.get(focus).mEditText.requestFocus();
        }
        progressPendingActivityResult();
        updateNextBtnStatus();
        updateNewBtnStatus();
        Log.d(TAG, "onPplServiceConnected done");
    }

    @Override
    public void onPanelChange(int h) {
        if (VDBG) Log.d(TAG, "onPanelChange Scroll to " + h);
        int index = -1;
        int bPos = 0;
        int ePos = 0;
        for (ContactLine cl : mContactLines) {
            if (cl.mEditText.isFocused()) {
                index = cl.mLine.getId();
                bPos = cl.mEditText.getSelectionStart();
                ePos = cl.mEditText.getSelectionEnd();
                break;
            }
        }
        mScrollFlag = true;
        mScrollView.fullScroll(ScrollView.FOCUS_DOWN);
        if (index >= 0) {
            mContactLines.get(index).mEditText.requestFocus();
            mContactLines.get(index).mEditText.setSelection(bPos, ePos);
        }
        mScrollFlag = false;
    }

    @Override
    public void onAddBtnClicked(int id) {
        Intent intent = new Intent(Intent.ACTION_PICK, ContactsContract.Contacts.CONTENT_URI);
        startActivityForResult(intent, id);
    }

    @Override
    public void onDelBtnClicked(int id) {
        if (mContactLines.size() > 1) {
            mLayoutEditor.removeViewAt(id);
            mContactLines.remove(id);
            int index = id;
            while (index < mContactLines.size()) {
                mContactLines.get(index).mLine.setId(index);
                index++;
            }
        }
        updateNewBtnStatus();
        updateNextBtnStatus();
    }

    @Override
    public void onEditTextClicked(int id) {
        // TODO Auto-generated method stub

    }

    @Override
    public void onFocusChanged(int id, boolean hasFocus) {
        // TODO Auto-generated method stub

    }

    @Override
    public void afterTextChanged(int id, Editable s) {
        updateNextBtnStatus();
        updateNewBtnStatus();
    }

    private void updateContactText(int id) {
        mTextChangeByManul = false;
        ContactLine cl = mContactLines.get(id);
        if (cl.mEditText.isFocused() || null == cl.mNameStr) {
            cl.mEditText.setText(cl.mInputStr);
        } else {
            cl.mEditText.setText(cl.mInputStr + "(" + cl.mNameStr + ")");
        }
        mTextChangeByManul = true;
    }

    private void updateNextBtnStatus() {
        boolean bNextButtonEnabled = false;
        for (ContactLine cl : mContactLines) {
            if (cl.mEditText.getText().length() > 0) {
                bNextButtonEnabled = true;
            }
        }
        mNextButton.setEnabled(bNextButtonEnabled);
    }

    private void updateNewBtnStatus() {
        boolean bNewButtonVisible = false;
        if (mContactLines.size() < PplService.MAX_CONTACTS_NUM) {
            for (ContactLine cl : mContactLines) {
                if (0 == cl.mEditText.getText().length()) {
                    bNewButtonVisible = false;
                    break;
                } else {
                    bNewButtonVisible = true;
                }
            }
        }
        mNewContactButton.setVisibility(bNewButtonVisible ? View.VISIBLE : View.GONE);
    }

    protected List<String> getNumberList() {
        LinkedList<String> numberList = new LinkedList<String>();
        for (ContactLine cl : mContactLines) {
            if (null != cl.mInputStr && cl.mInputStr.length() > 0) {
                numberList.add(cl.mInputStr);
            }
        }
        return numberList;
    }

    protected void onButtonClicked() {

        mBinder.saveTustedNumberList(getNumberList(), PplService.EDIT_TYPE_SETUP);
        gotoActivity(this, SetupManualActivity.class);
    }

}
