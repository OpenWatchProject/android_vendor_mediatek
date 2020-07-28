package com.mediatek.mms.callback;

import android.graphics.drawable.BitmapDrawable;

public interface IContactCallback {

    /**
     * getAvatar
     * @return avatar BitmapDrawable
     */
    public BitmapDrawable getAvatar();

    /**
     * setAvatar
     * @param avatar BitmapDrawable
     */
    public void setAvatar(BitmapDrawable avatar);

    /**
     * getContactName
     * @return contact's name String
     */
    public String getContactName();

    /**
     * getContactNumber
     * @return contact's number String
     */
    public String getContactNumber();

    /**
     * isContactExistsInDatabase
     * @return boolean; true inDatabase
     */
    public boolean isContactExistsInDatabase();
}
