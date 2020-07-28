package com.mediatek.op12.settings.wfc.ui;

import android.app.Activity;
import android.app.Fragment;
import android.os.Bundle;
import android.text.Html;
import android.text.method.LinkMovementMethod;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.Button;
import android.widget.CheckBox;
import android.widget.CompoundButton;

import com.mediatek.op12.settings.R;
import com.mediatek.optin.parser.RequestResponse;


/** LTE-RCS-UI-Guidelines-4.pdf:page#70:W12.a.
 * Fragment to display T&C.
 */

public class TCFragment extends Fragment {

    private static final String TAG = "OP12TCFragment";

    private Button mCancelButton;
    private Button mContinueButton;
    private CheckBox mTcCheckBox;

    private View mRootView;

    private RequestResponse.Address mUserDetails;

    private E911UiCallback mCallback;

    private View.OnClickListener mClickListener = new View.OnClickListener() {
        public void onClick(View v) {
            Log.d(TAG, "v:" + v);
           if (v == mRootView.findViewById(R.id.continue_button)) {
                // TODO: need to send tc acceptance to server???
                mCallback.showE911AddressFragment(mUserDetails);
            } else if (v == mRootView.findViewById(R.id.cancel)) {
               mCallback.handleUserCancel();
            }
        }
    };

    private CompoundButton.OnCheckedChangeListener mCheckChangeListener
            = new CompoundButton.OnCheckedChangeListener() {
        public void onCheckedChanged(CompoundButton buttonView, boolean isChecked) {
            mContinueButton.setEnabled(isChecked);
        }
    };

    /** Constructor.
     * @param rsp rsp
     */
    public TCFragment(RequestResponse.Address rsp) {
        mUserDetails = rsp;
    }

    @Override
    public View onCreateView(LayoutInflater inflater, ViewGroup container,
            Bundle savedInstanceState) {
        mRootView = inflater.inflate(R.layout.tc_fragment_layout, container, false);
        Log.d(TAG, "OncreateView:" + mRootView);
        return mRootView;
    }

    @Override
    public void onActivityCreated(Bundle savedInstanceState) {
        super.onActivityCreated(savedInstanceState);

        Log.d(TAG, "Oncreate");
        mTcCheckBox = (CheckBox) mRootView.findViewById(R.id.tc_checkBox);
        //mTcCheckBox.setLinksClickable(true);
        mTcCheckBox.setMovementMethod(LinkMovementMethod.getInstance());
        // TODO: get string from resource
        mTcCheckBox.setText(Html.fromHtml(getActivity().getResources()
                .getString(R.string.tc_text_link)));
        mTcCheckBox.setOnCheckedChangeListener(mCheckChangeListener);

        mCancelButton = (Button) mRootView.findViewById(R.id.cancel);
        mCancelButton.setOnClickListener(mClickListener);

        mContinueButton = (Button) mRootView.findViewById(R.id.continue_button);
        mContinueButton.setOnClickListener(mClickListener);

        mContinueButton.setEnabled(mTcCheckBox.isChecked());
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
}
