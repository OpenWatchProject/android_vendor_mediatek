package com.mediatek.op12.settings.wfc.ui;

import android.app.Activity;
import android.app.Fragment;
import android.os.Bundle;
import android.text.Editable;
import android.text.TextWatcher;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.AdapterView;
import android.widget.AdapterView.OnItemSelectedListener;
import android.widget.Button;
import android.widget.EditText;
import android.widget.Spinner;
import android.widget.TextView;

import com.mediatek.op12.settings.R;
import com.mediatek.optin.parser.RequestResponse;

import java.util.Arrays;

/** LTE-RCS-UI-Guidelines-4.pdf:page#68:W72.
 * Activity to display/edit emergency address to user.
 */

public class E911AddressFragment extends Fragment {

    private static final String TAG = "OP12E911AddressFragment";

    private String mDefaultStateValue;
    private String mCountry;

    private Button mSaveButton;
    private Button mCancelButton;
    private EditText mAddressNumber;
    private EditText mStreet;
    private EditText mAptSuite;
    private EditText mCity;
    private Spinner mState;
    private EditText mZipCode;

    private View mRootView;

    private RequestResponse.Address mUserDetails;

    private E911UiCallback mCallback;

    private View.OnClickListener mClickListener = new View.OnClickListener() {
        public void onClick(View v) {
           Log.d(TAG, "onClick, v:" + v);
           if (v == mRootView.findViewById(R.id.save)) {
                mCallback.saveE911Adress(getUserDetails());
            } else if (v == mRootView.findViewById(R.id.cancel)) {
                mCallback.handleUserCancel();
            }
        }
    };

    private TextWatcher mEditTextWatcher = new TextWatcher() {
        @Override
        public void afterTextChanged(Editable s) {
            mSaveButton.setEnabled(enableSaveButton());
        }

        @Override
        public void beforeTextChanged(CharSequence s, int start, int count, int after) {}

        @Override
        public void onTextChanged(CharSequence s, int start, int before, int count) {}
    };

    private OnItemSelectedListener mSpinnerItemChangeListener = new OnItemSelectedListener() {
        @Override
        public void onItemSelected(AdapterView<?> parentView, View selectedItemView,
                int position, long id) {
            Log.d(TAG, "onItemSelected");
            mSaveButton.setEnabled(enableSaveButton());
        }

        @Override
        public void onNothingSelected(AdapterView<?> parentView) {}

    };

    /** Constructor.
     * @param rsp rsp
     */
    public E911AddressFragment(RequestResponse.Address rsp) {
        mUserDetails = rsp;
    }

    @Override
    public View onCreateView(LayoutInflater inflater, ViewGroup container,
            Bundle savedInstanceState) {
        mRootView = inflater.inflate(R.layout.e911_address_fragment_layout, container, false);
        Log.d(TAG, "OncreateView:" + mRootView);
        return mRootView;
    }

    @Override
    public void onActivityCreated(Bundle savedInstanceState) {
        super.onActivityCreated(savedInstanceState);

        Log.d(TAG, "Oncreate");
        mAddressNumber = (EditText) mRootView.findViewById(R.id.addressNumberEditText);
        mStreet = (EditText) mRootView.findViewById(R.id.streetEditText);
        mAptSuite = (EditText) mRootView.findViewById(R.id.aptEditText);

        mCity = (EditText) mRootView.findViewById(R.id.cityEditText);
        mCity.addTextChangedListener(mEditTextWatcher);

        mState = (Spinner) mRootView.findViewById(R.id.stateSpinner);
        mState.setOnItemSelectedListener(mSpinnerItemChangeListener);

        mZipCode = (EditText) mRootView.findViewById(R.id.zipText);
        mZipCode.addTextChangedListener(mEditTextWatcher);

        mSaveButton = (Button) mRootView.findViewById(R.id.save);
        mSaveButton.setOnClickListener(mClickListener);

        mCancelButton = (Button) mRootView.findViewById(R.id.cancel);
        mCancelButton.setOnClickListener(mClickListener);

        mCountry = getActivity().getResources().getString(R.string.united_states_country);
        mDefaultStateValue = getActivity().getResources().getString(R.string.default_state_value);

        fillFields(mUserDetails);
    }

    @Override
    public void onAttach(Activity activity) {
        super.onAttach(activity);

        try {
            mCallback = (E911UiCallback) activity;
        } catch (ClassCastException e) {
            throw new ClassCastException(activity.toString()
                    + " must implement E911UiCallback");
        }
    }

    private void fillFields(RequestResponse.Address rsp) {
        try {
            if(rsp == null) {
                return;
            }
        Log.d(TAG, "userDetails:" + rsp);
        mAddressNumber.setText(rsp.houseNumber, TextView.BufferType.EDITABLE);
        mStreet.setText(rsp.road, TextView.BufferType.SPANNABLE);
        mAptSuite.setText(rsp.location, TextView.BufferType.SPANNABLE);
        mCity.setText(rsp.city, TextView.BufferType.NORMAL);
        mState.setSelection(getPosition(rsp.state));
        mZipCode.setText(rsp.zip, TextView.BufferType.NORMAL);

        mSaveButton.setEnabled(enableSaveButton());
        } catch (Exception e) {
            // TODO Auto-generated catch block
            e.printStackTrace();
        }
    }

    private RequestResponse.Address getUserDetails() {
        String[] stateArray = getResources().getStringArray(R.array.state_array);
        String selectedState = stateArray[mState.getSelectedItemPosition()];
        return new RequestResponse.Address(mAddressNumber.getText().toString(),
                mStreet.getText().toString(), mAptSuite.getText().toString(),
                mCity.getText().toString(), selectedState, mZipCode.getText().toString(), mCountry);
    }

    private int getPosition(String state) {
        String[] stateArray = getResources().getStringArray(R.array.state_array);
        int position = Arrays.asList(stateArray).indexOf(state);
        Log.d(TAG, "stateposition:" + position);
        return position == -1 ? 0 : position;
    }

    private boolean enableSaveButton() {
        // Enable save button only when mandatory fields are filled(city, state, zip, cntry)
        boolean isEmpty = mCity.getText().toString().trim().length() == 0;
        isEmpty |= mZipCode.getText().toString().trim().length() == 0;
        isEmpty |= mDefaultStateValue.equals(mState.getSelectedItem().toString());
        Log.d(TAG, "enableSaveButton:" + !isEmpty);
        return !isEmpty;
    }
}
