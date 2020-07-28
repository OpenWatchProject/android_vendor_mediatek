package com.mediatek.op12.settings.wfc.ui;

import android.app.Dialog;
import android.content.Context;
import android.os.Bundle;
import android.view.View;
import android.widget.Button;
import android.widget.TextView;

import com.mediatek.op12.settings.R;
import com.mediatek.optin.parser.RequestResponse;


/** LTE-RCS-UI-Guidelines-4.pdf:page#68:W14.
 * Activity displayed when even partial validation of entered address failed .
 */
public class E911ValidationFailedDialog extends Dialog {

    private static final String TAG = "OP12E911ValidationFailedDialog";

    private Context mContext;

    private Button mEditButton;
    private Button mCancelButton;
    private TextView mUserAddress;

    private RequestResponse.Address mUserDetails;
    private E911UiCallback mCallback;

    private View.OnClickListener mClickListener = new View.OnClickListener() {
        public void onClick(View v) {
           if (v == findViewById(R.id.edit)) {
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
    public E911ValidationFailedDialog(Context context, RequestResponse.Address userDetails) {
        super(context);
        mUserDetails = userDetails;
        mContext = context;
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        setContentView(R.layout.address_partial_validation_fail);

        mUserAddress = (TextView) findViewById(R.id.user_address);
        mEditButton = (Button) findViewById(R.id.edit);
        mCancelButton = (Button) findViewById(R.id.cancel);

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
