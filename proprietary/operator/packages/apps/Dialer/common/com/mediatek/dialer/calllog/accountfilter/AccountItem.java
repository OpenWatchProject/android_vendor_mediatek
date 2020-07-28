package com.mediatek.dialer.calllog.accountfilter;

import android.telecom.PhoneAccountHandle;


/**
* Data structure to encapsulation account info.
*/
public class AccountItem {

    public final PhoneAccountHandle accountHandle;
    public final int title;
    public final int type;
    public final String id;

    /**
     * create a phoneAccount type data Item it will be displayed as
     * icon/name/number.
     * @param accountHandle accountHandle
     */
    public AccountItem(PhoneAccountHandle accountHandle) {
        this.type = PhoneAccountPickerAdapter.ITEM_TYPE_ACCOUNT;
        this.accountHandle = accountHandle;
        this.id = accountHandle.getId();
        this.title = 0;
    }

    /**
     * create a text type data item it will be displayed as textview.
     * @param title title
     * @param id id
     */
    public AccountItem(int title, String id) {
        this.type = PhoneAccountPickerAdapter.ITEM_TYPE_TEXT;
        this.title = title;
        this.id = id;
        this.accountHandle = null;
    }

}
