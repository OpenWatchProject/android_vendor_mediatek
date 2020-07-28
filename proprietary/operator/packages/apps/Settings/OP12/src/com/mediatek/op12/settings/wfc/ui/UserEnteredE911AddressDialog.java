package com.mediatek.op12.settings.wfc.ui;

import android.app.Dialog;
import android.content.Context;
import android.os.Bundle;
import android.util.Log;
import android.view.View;
import android.widget.Button;
import android.widget.TextView;

import com.mediatek.op12.settings.R;
import com.mediatek.optin.parser.RequestResponse;


/** LTE-RCS-UI-Guidelines-4.pdf:page#68:W16.
 * Activity displayed when address entered not found. As well as alt address also not found.
 */
public class UserEnteredE911AddressDialog extends Dialog {

    private static final String TAG = "OP12UserEnteredE911AddressDialog";
    private static final String USER_DETAILS = "userDetails";

    private static final int POPULATE_UI = 0;

    private Button mSaveButton;
    private Button mEditButton;
    private Button mCancelButton;
    private TextView mUserAddress;

    private RequestResponse.Address mUserDetails;

    private E911UiCallback mCallback;
    private Context mContext;

    private View.OnClickListener mClickListener = new View.OnClickListener() {
        public void onClick(View v) {
            Log.d(TAG, "v:" + v);
            Log.d(TAG, "v1:" + findViewById(R.id.save));
           if (v == findViewById(R.id.save)) {
                mCallback.showSaveAnywayE911Dialog(mUserDetails);
            } else if (v == findViewById(R.id.edit)) {
                mCallback.showE911AddressFragment(mUserDetails);
            } else if (v == findViewById(R.id.cancel)) {
                mCallback.onDialogDismiss();
            }
            dismiss();
        }
    };

    /** Constructor.
     * @param context context
     * @param userDetails userDetails
     */
    public UserEnteredE911AddressDialog(Context context,
            RequestResponse.Address userDetails) {
        super(context);
        mUserDetails = userDetails;
        mContext = context;
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        setContentView(R.layout.user_address_only);

        mUserAddress = (TextView) findViewById(R.id.user_address);
        mSaveButton = (Button) findViewById(R.id.save);
        mEditButton = (Button) findViewById(R.id.edit);
        mCancelButton = (Button) findViewById(R.id.cancel);

        mSaveButton.setOnClickListener(mClickListener);
        mEditButton.setOnClickListener(mClickListener);
        mCancelButton.setOnClickListener(mClickListener);

        fillUserAddress();

        try {
            mCallback = (E911UiCallback) mContext;
        } catch (ClassCastException e) {
            throw new ClassCastException(mContext.toString()
                    + " must implement E911UiCallback");
        }
    }

    private void fillUserAddress() {
        String address = mUserDetails.houseNumber + " " + mUserDetails.road
                + "\n" + mUserDetails.city + ", " + mUserDetails.state;
        mUserAddress.setText(address);
    }
}
