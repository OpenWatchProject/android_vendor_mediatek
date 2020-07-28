package com.mediatek.contacts.ext;

import android.app.Activity;
import android.content.Intent;
import android.database.Cursor;
import android.graphics.drawable.Drawable;
import android.net.Uri;
import android.view.Menu;
import android.view.ViewGroup;

import com.android.contacts.list.ContactListItemView;

public interface IContactsCommonPresenceExtension {
    /**
     * Checks if contact is video call capable.
     * @param number number.
     * @return true if contact is video call capable.
     */
    boolean isVideoCallCapable(String number);

    /**
     * Checks if plugin is active to show video icon.
     * @return true if op08 plugin active, otherwise false.
     */
    boolean isShowVideoIcon();

    /**
     * Set Video Icon alpha value.
     * @param number contact number.
     * @param thirdIcon video icon.
     * @param isVideoEnabled host video state.
     */
    void setVideoIconAlpha(String number, Drawable thirdIcon, boolean isVideoEnabled);

    /**
     * Check is video icon clickable.
     * @param Uri
     */
    boolean isVideoIconClickable(Uri uri);

    void addVideoCallView(long contactId, ViewGroup viewGroup);
    /**
    * @param widthMeasureSpec
    * @param heightMeasureSpec
    */
   void onMeasure(int widthMeasureSpec, int heightMeasureSpec);

   /**
    * @param changed
    * @param leftBound
    * @param topBound
    * @param rightBound
    * @param bottomBound
    */
   void onLayout(boolean changed, int leftBound, int topBound, int rightBound,
           int bottomBound);
   /**
    * getWidthWithPadding.
    * @return padding width
    */
    int getWidthWithPadding();

    /**
     * processIntent
     * @param intent contains contact info
     */
    void processIntent(Intent intent);

    void bindPhoneNumber(ContactListItemView view, Cursor cursor);

    void addRefreshMenu(Menu menu);

    void onHostActivityResumed(Activity activity);

    void onHostActivityPaused();

    void onHostActivityStopped();

    boolean onOptionsItemSelected(int itemId, long contactId);
}
