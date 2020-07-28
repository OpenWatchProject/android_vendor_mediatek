package com.mediatek.mail.vip.activity;

import android.content.Context;
import android.os.SystemClock;
import android.util.AttributeSet;
import android.util.Log;
import android.view.KeyCharacterMap;
import android.view.KeyEvent;
import android.view.ViewRootImpl;
import android.widget.TextView;

import com.android.mail.R;
import com.android.emailcommon.Logging;
import com.android.emailcommon.mail.Address;
import com.android.emailcommon.utility.Utility;
import com.mediatek.mail.ui.utils.ChipsAddressTextView;

/**
 * M : This is a MTKRecipientEditTextView which has a custom onItemClickListener.
 * add selected item into database directly.
 */
public class VipAddressTextView extends ChipsAddressTextView {
    private VipListFragment mListFragment = null;
    // /M: Define context
    private Context mContext;
    public VipAddressTextView(Context context, AttributeSet attrs) {
        super(context, attrs);
        mContext = context;
    }

    public void setTargetFragment(VipListFragment listFragment) {
        mListFragment = listFragment;
    }

    private Address[] getAddresses(TextView view) {
        /** M: check the address is valid or not, and alter user. @{ */
        String addressList = view.getText().toString().trim();
        if (!Address.isAllValid(addressList)) {
            view.setText("");
            Utility.showToast(mContext, R.string.message_compose_error_invalid_email);
        }
        /** @} */
        Address[] addresses = Address.parse(addressList, false);
        return addresses;
    }

    @Override
    public boolean onEditorAction(TextView view, int action, KeyEvent keyEvent) {
        // If the key action is "done", intercept it, and TextView default will using
        // a KeyEvent.KEYCODE_ENTER instead of it. Because some ime looklike don't support
        // "done" action. So we'd better process it in onKeyUp.
        Log.d(Logging.LOG_TAG, "VipAddressTextView onEditorAction : action = " + action);
        /// M: We should do default action when popupshowing, cause "add pop up item to textview"
        // is done in onEditorAction on KK @{
        if (!isPopupShowing()) {
            ViewRootImpl viewRootImpl = getViewRootImpl();
            if (viewRootImpl != null) {
                long eventTime = SystemClock.uptimeMillis();
                viewRootImpl.dispatchKeyFromIme(
                        new KeyEvent(eventTime, eventTime,
                        KeyEvent.ACTION_DOWN, KeyEvent.KEYCODE_ENTER, 0, 0,
                        KeyCharacterMap.VIRTUAL_KEYBOARD, 0,
                        KeyEvent.FLAG_SOFT_KEYBOARD | KeyEvent.FLAG_KEEP_TOUCH_MODE
                        | KeyEvent.FLAG_EDITOR_ACTION));
                viewRootImpl.dispatchKeyFromIme(
                        new KeyEvent(SystemClock.uptimeMillis(), eventTime,
                        KeyEvent.ACTION_UP, KeyEvent.KEYCODE_ENTER, 0, 0,
                        KeyCharacterMap.VIRTUAL_KEYBOARD, 0,
                        KeyEvent.FLAG_SOFT_KEYBOARD | KeyEvent.FLAG_KEEP_TOUCH_MODE
                        | KeyEvent.FLAG_EDITOR_ACTION));
            }
        } else {
            super.onEditorAction(view, action, keyEvent);
        }
        /// @}
        return true;
    }

    @Override
    public boolean onKeyUp(int keyCode, KeyEvent event) {
        // if KEYCODE_ENTER, don't auto complete the address, but attempt to add into vip list.
        if (keyCode == KeyEvent.KEYCODE_ENTER && !isPopupShowing()) {
            Address[] addresses = getAddresses(this);
            if (addresses.length > 0 && mListFragment != null) {
                mListFragment.onAddVip(addresses);
                setText("");
            }
            return false;
        }
        return super.onKeyUp(keyCode, event);
    }
}

