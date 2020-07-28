package com.mediatek.op12.settings.wfc.ui;

import android.app.Dialog;
import android.content.Context;
import android.os.Bundle;
import android.util.Log;
import android.view.View;
import android.widget.Button;
///import android.widget.CompoundButton;
import android.widget.RadioButton;
import android.widget.RadioGroup;

import com.mediatek.op12.settings.R;
import com.mediatek.optin.parser.RequestResponse;

import java.util.ArrayList;

/** LTE-RCS-UI-Guidelines-4.pdf:page#68:W15.
 * Activity displayed when address entered not found but some alt address found.
 */
public class AltE911AddressDialog extends Dialog {

    private static final String TAG = "OP12AltE911AddressDialog";
    private static final String USER_DETAILS = "userDetails";
    private static final String ALT_ADDRESS_DETAILS = "altAddresses";

    private Context mContext;

    private Button mSaveButton;
    private Button mOkButton;
    private Button mEditButton;
    private Button mCancelButton;
    private RadioButton mUserAddress;
    private RadioGroup mAltAddressGroup;

    private RequestResponse.UserDetail mUserDetails;
    private ArrayList<RequestResponse.Address> mAltAddress;
    private E911UiCallback mCallback;

    private View.OnClickListener mClickListener = new View.OnClickListener() {
        @Override
        public void onClick(View v) {
            Log.d(TAG, "view clicked:" + v);
           if (v == findViewById(R.id.save)) {
                mCallback.showSaveAnywayE911Dialog(mUserDetails.userAddress);
            } else if (v == findViewById(R.id.ok)) {
                // Radio button ids are in continuation but alt address is stored in array
                // seperate from user address, so decrement by 1 to get alt address
                RequestResponse.Address selectedDetails
                        = mAltAddress.get(mAltAddressGroup.getCheckedRadioButtonId() - 1);
                mCallback.saveE911Adress(selectedDetails);
            } else if (v == findViewById(R.id.edit)) {
                mCallback.showE911AddressFragment(mUserDetails.userAddress);
            } else if (v == findViewById(R.id.cancel)) {
                mCallback.onDialogDismiss();
            }
            dismiss();
        }
    };

    /*private CompoundButton.OnCheckedChangeListener mUserAddressCheckedListener
            = new CompoundButton.OnCheckedChangeListener() {
        @Override
        public void onCheckedChanged(CompoundButton buttonView, boolean isChecked) {
            Log.d(TAG, "mUserAddressCheckedListener:isChecked:" + isChecked);
            Log.d(TAG, "mUserAddressCheckedListener:buttonView:" + buttonView);
            if (mUserAddress == buttonView && isChecked) {
                mAltAddressGroup.clearCheck();
                mSaveButton.setVisibility(View.VISIBLE);
                mOkButton.setVisibility(View.GONE);
            }
        }
    };*/

    private RadioGroup.OnCheckedChangeListener mRadioGroupCheckedListener
            = new RadioGroup.OnCheckedChangeListener() {
        @Override
        public void onCheckedChanged(RadioGroup group, int checkedId) {
            Log.d(TAG, "mRadioGroupCheckedListener:checkedId:" + checkedId);
            boolean enableOkButton = mAltAddressGroup.getCheckedRadioButtonId() > 0;
            mSaveButton.setVisibility(checkedId > 0 ? View.GONE : View.VISIBLE);
            mOkButton.setVisibility(checkedId > 0 ? View.VISIBLE : View.GONE);
        }
    };

    /** Constructor.
     * @param context context
     * @param userDetails userDetails
     */
    public AltE911AddressDialog(Context context,
            RequestResponse.AddUpdateAddressResponse userDetails) {
        super(context);
        mUserDetails = userDetails.userDetail;
        mAltAddress = userDetails.altAddress;
        Log.d(TAG, "mUserDetails:" + mUserDetails);
        Log.d(TAG, "mAltAddress:" + mAltAddress);
        mContext = context;
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        setContentView(R.layout.alternate_address);

        mUserAddress = (RadioButton) findViewById(R.id.user_address_button);
        mUserAddress.setId(0);
        mAltAddressGroup = (RadioGroup) findViewById(R.id.altAddressgroup);
        mSaveButton = (Button) findViewById(R.id.save);
        mOkButton = (Button) findViewById(R.id.ok);
        mEditButton = (Button) findViewById(R.id.edit);
        mCancelButton = (Button) findViewById(R.id.cancel);

        mAltAddressGroup.setOnCheckedChangeListener(mRadioGroupCheckedListener);
        mSaveButton.setOnClickListener(mClickListener);
        mOkButton.setOnClickListener(mClickListener);
        mEditButton.setOnClickListener(mClickListener);
        mCancelButton.setOnClickListener(mClickListener);

        mOkButton.setVisibility(View.GONE);

        fillUserAddress();
        fillAltAddresses();

        try {
            mCallback = (E911UiCallback) mContext;
        } catch (ClassCastException e) {
            throw new ClassCastException(mContext.toString()
                    + " must implement E911UiCallback");
        }
    }

    private void fillUserAddress() {
        String address = mUserDetails.userAddress.houseNumber + " " + mUserDetails.userAddress.road
                + "\n" + mUserDetails.userAddress.city + ", " + mUserDetails.userAddress.state;
        mUserAddress.setText(address);
        mUserAddress.setChecked(true);
    }

    private void fillAltAddresses() {
        int id = 1;
        Log.d(TAG, "address array:" + mAltAddress);
        for (RequestResponse.Address address: mAltAddress) {
            String addressText = address.houseNumber + " " + address.road
                    + "\n" + address.city + ", " + address.state;
            Log.d(TAG, "id:" + id + ", address:" + addressText);
            RadioButton b = new RadioButton(mContext);
            b.setText(addressText);
            b.setId(id++);
            mAltAddressGroup.addView(b);
            Log.d(TAG, "child number:" + mAltAddressGroup.getChildCount());
        }
    }
}
