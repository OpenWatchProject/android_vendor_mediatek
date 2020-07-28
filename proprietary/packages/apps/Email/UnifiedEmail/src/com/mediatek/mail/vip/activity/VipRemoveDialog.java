package com.mediatek.mail.vip.activity;

import com.android.mail.R;
import com.android.emailcommon.utility.EmailAsyncTask;
import com.mediatek.mail.vip.VipMember;

import android.app.AlertDialog;
import android.app.Dialog;
import android.app.DialogFragment;
import android.content.Context;
import android.content.DialogInterface;
import android.os.Bundle;

public class VipRemoveDialog extends DialogFragment {
    ///M: Add TAG for VipRemoveDialog. @{
    @SuppressWarnings("hiding")
    public static final String TAG = "VipRemoveDialog";
    /// @}
    private static final String ARG_VIP_ADDRSS = "arg_vip_addess";
    private static final String ARG_VIP_ID = "arg_vip_id";
    private long mVipId = -1;

    /**
     * M: because we don't prohibit duplicated dialog, so no need to use static
     * variable @{
     */
    private String mVipAddress;

    private EmailAsyncTask<Void, Void, Void> mRemoveTask;
    /** @} */

    @Override
    public void onSaveInstanceState(Bundle outState) {
        // save VIP Address, ID if needed.
        outState.putString(ARG_VIP_ADDRSS, mVipAddress);
        outState.putLong(ARG_VIP_ID, mVipId);
        super.onSaveInstanceState(outState);
    }

    public static VipRemoveDialog newInstance(String address, long vipId) {
        VipRemoveDialog frag = new VipRemoveDialog();
        frag.mVipAddress = address;
        frag.mVipId = vipId;
        return frag;
    }

    @Override
    public Dialog onCreateDialog(Bundle savedInstanceState) {
        // restore VIPAddress, ID if needed.
        if (mVipAddress == null && savedInstanceState != null
                && savedInstanceState.containsKey(ARG_VIP_ADDRSS)) {
            mVipAddress = savedInstanceState.getString(ARG_VIP_ADDRSS);
        }
        if (mVipId == -1 && savedInstanceState != null
                && savedInstanceState.containsKey(ARG_VIP_ID)) {
            mVipId = savedInstanceState.getLong(ARG_VIP_ID);
        }
        Context context = getActivity();
        mRemoveTask = new RemoveVipTask(context);
        final String message = String.format(
                this.getString(R.string.vip_remove_prompt), mVipAddress);
        return new AlertDialog.Builder(context)
                .setIconAttribute(android.R.attr.alertDialogIcon)
                .setTitle(R.string.vip_remove_title)
                .setMessage(message)
                .setPositiveButton(getString(android.R.string.ok),
                        new DialogInterface.OnClickListener() {
                            public void onClick(DialogInterface dialog,
                                    int which) {
                                if (mRemoveTask != null) {
                                    mRemoveTask.executeSerial();
                                }
                            }
                        })
                .setNegativeButton(getString(android.R.string.cancel), null)
                .create();
    }

    /**
     * Remove Vip contacts {@link #VipMember}.
     */
    private class RemoveVipTask extends EmailAsyncTask<Void, Void, Void> {
        private final Context mContext;

        public RemoveVipTask(Context context) {
            super(null);
            mContext = context;
        }

        @Override
        protected Void doInBackground(Void... params) {
            // Have chance restore the id from SavedInstance, check it again.
            if (mVipId > 0) {
                VipMember.delete(mContext, VipMember.CONTENT_URI, mVipId);
            }
            return null;
        }

        @Override
        protected void onSuccess(Void result) {
        }
    }
}