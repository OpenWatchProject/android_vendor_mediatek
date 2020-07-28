package com.mediatek.contacts.ext;

import android.app.Activity;
import android.content.Intent;
import android.database.Cursor;
import android.graphics.drawable.Drawable;
import android.net.Uri;
import android.util.Log;
import android.view.Menu;
import android.view.ViewGroup;

import com.android.contacts.list.ContactListItemView;

public class DefaultContactsCommonPresenceExtension implements IContactsCommonPresenceExtension {
    private static final String TAG = "DefaultContactsCommonPresenceExtension";
    /**
     * Checks if contact is video call capable
     * @param number number.
     * @return true if contact is video call capable,false if not capable.
     */
    public boolean isVideoCallCapable(String number){
        Log.d(TAG, "[isVideoCallCapable] number:" + number);
        return true;
    }

    /**
     * Checks if plugin is active to show video icon.
     * @return true if op08 plugin active, otherwise false.
     */
    public boolean isShowVideoIcon(){
        Log.d(TAG, "[isShowVideoIcon] default implementation");
        return false;
    }

    /**
     * Set Video Icon alpha value.
     * @param number contact number.
     * @param thirdIcon video icon.
     * @param isVideoEnabled host video state.
     */
    public void setVideoIconAlpha(String number, Drawable thirdIcon,
            boolean isVideoEnabled) {
    }

    /**
     * Check is video icon clickable.
     * @param Uri
     */
    public boolean isVideoIconClickable(Uri uri) {
        return true;
    }

    /**
     * Checks if any number in contactId is video call capable,
     * if capable, add the view in contact list item.
     * @param contactId Contact Id.
     * @param viewGroup host view.
     */
    public void addVideoCallView(long contactId, ViewGroup viewGroup){
        Log.d(TAG, "[addVideoCallView] contactId:" + contactId);
    }

    @Override
    public void onMeasure(int widthMeasureSpec, int heightMeasureSpec) {
        // do-nothing
    }

    @Override
    public void onLayout(boolean changed, int leftBound, int topBound, int rightBound,
            int bottomBound) {
        // do-nothing
    }

    @Override
    public int getWidthWithPadding(){
        return 0;
    }

    @Override
    public void processIntent(Intent intent) {
        Log.d(TAG, "[processIntent]");
    }

    @Override
    public void bindPhoneNumber(ContactListItemView view, Cursor cursor) {
        Log.d(TAG, "[bindPhoneNumber]");
    }


    @Override
    public void addRefreshMenu(Menu menu) {
        Log.d(TAG, "[addRefreshMenu]");
    }

    @Override
    public void onHostActivityResumed(Activity activity) {
    }

    @Override
    public void onHostActivityPaused() {
    }

    @Override
    public void onHostActivityStopped() {
    }

    @Override
    public boolean onOptionsItemSelected(int itemId, long contactId) {
        return false;
    }
}
