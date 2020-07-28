package com.mediatek.op12.settings.wfc.ui;

import android.app.Dialog;
import android.content.Context;
import android.os.Bundle;
import android.view.View;
import android.widget.Button;
import android.widget.TextView;

import com.mediatek.op12.settings.R;
import com.mediatek.optin.parser.RequestResponse;



/** LTE-RCS-UI-Guidelines-4.pdf:page#68:W17.
 * Activity intimating user about validation failure of address entered by service Provider.
 */
public class SaveAnywayE911AddressDialog extends Dialog {

    private static final String TAG = "OP12SaveAnywayE911AddressDialog";

    private Context mContext;
    private Button mSaveButton;
    private Button mCancelButton;
    private TextView mUserAddress;

    private RequestResponse.Address mUserDetails;
    private E911UiCallback mCallback;

    private View.OnClickListener mClickListener = new View.OnClickListener() {
        public void onClick(View v) {
           if (v == findViewById(R.id.save)) {
                // TODO: how to pass UI enteries to handler. I think using req class??
                // TODO: what kind of request will be made to nw for partial validation.
                // same as first add req??
                mCallback.doPartialValidationE911Add(mUserDetails);
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
    public SaveAnywayE911AddressDialog(Context context,
            RequestResponse.Address userDetails) {
        super(context);
        mUserDetails = userDetails;
        mContext = context;
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        setContentView(R.layout.confirmation_save_invalid_address);

        mUserAddress = (TextView) findViewById(R.id.user_address);
        mSaveButton = (Button) findViewById(R.id.save);
        mCancelButton = (Button) findViewById(R.id.cancel);

        mSaveButton.setOnClickListener(mClickListener);
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
