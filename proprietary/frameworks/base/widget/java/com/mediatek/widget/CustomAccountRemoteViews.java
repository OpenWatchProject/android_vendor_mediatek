package com.mediatek.widget;

import java.util.List;

import android.app.PendingIntent;
import android.content.Context;
import android.content.Intent;
import android.graphics.Bitmap;
import android.graphics.drawable.BitmapDrawable;
import android.graphics.drawable.Drawable;
import android.util.Log;
import android.view.View;
import android.widget.RemoteViews;

import com.mediatek.internal.R;

public class CustomAccountRemoteViews {

    private static final String TAG = "CustomAccountRemoteViews";
    private static final int ROWACCOUNTNUMBER = 4;
    private static final int MOSTACCOUNTNUMBER = 8;

    private RemoteViews mNormalRemoteViews;
    private RemoteViews mBigRemoteViews;
    private Context mContext;
    private int mRequestCode;

    private List<AccountInfo> mData;

    private final int RESOURCE_ID[][] = {
            { R.id.account_zero_container, R.id.account_zero_img, R.id.account_zero_name, R.id.account_zero_number,
                    R.id.account_zero_normal_divider, R.id.account_zero_highlight_divider },
            { R.id.account_one_container, R.id.account_one_img, R.id.account_one_name, R.id.account_one_number,
                    R.id.account_one_normal_divider, R.id.account_one_highlight_divider },
            { R.id.account_two_container, R.id.account_two_img, R.id.account_two_name, R.id.account_two_number,
                    R.id.account_two_normal_divider, R.id.account_two_highlight_divider },
            { R.id.account_three_container, R.id.account_three_img, R.id.account_three_name, R.id.account_three_number,
                    R.id.account_three_normal_divider, R.id.account_three_highlight_divider },
            { R.id.account_four_container, R.id.account_four_img, R.id.account_four_name, R.id.account_four_number,
                    R.id.account_four_normal_divider, R.id.account_four_highlight_divider },
            { R.id.account_five_container, R.id.account_five_img, R.id.account_five_name, R.id.account_five_number,
                    R.id.account_five_normal_divider, R.id.account_five_highlight_divider },
            { R.id.account_six_container, R.id.account_six_img, R.id.account_six_name, R.id.account_six_number,
                    R.id.account_six_normal_divider, R.id.account_six_highlight_divider },
            { R.id.account_seven_container, R.id.account_seven_img, R.id.account_seven_name, R.id.account_seven_number,
                    R.id.account_seven_normal_divider, R.id.account_seven_highlight_divider } };

    private final class IdIndex {

        public static final int CONTAINER_ID = 0;
        public static final int IMG_ID = 1;
        public static final int NAME_ID = 2;
        public static final int NUMBER_ID = 3;
        public static final int NORMAL_DIVIDER_ID = 4;
        public static final int HIGHTLIGHT_DIVIDER_ID = 5;

        private IdIndex() {
        }
    }

    /**
     * Constructor
     * @param context The context where the view associated with the notification is posted. 
     * @param packageName The package name of the component which posts the notification.
     */
    public CustomAccountRemoteViews(Context context, String packageName) {

        this(context, packageName, null);
    }

    /**
     * Constructor
     * @param context The context where the view associated with the notification is posted. 
     * @param packageName The package name of the component which posts the notification.
     * @param data The content of the Account {@link AccountInfo} which will be shown in the notification.
     */
    public CustomAccountRemoteViews(Context context, String packageName, final List<AccountInfo> data) {

        mNormalRemoteViews = new RemoteViews(packageName, R.layout.normal_default_account_select_title);
        mBigRemoteViews = new RemoteViews(packageName, R.layout.custom_select_default_account_notification);

        mData = data;
        mContext = context;
        mRequestCode = 0;
    }

    /**
     * 
     * @return The normal remoteviews {@link RemoteViews} of the notification.
     * @see Notification
     */
    public RemoteViews getNormalRemoteViews() {
        return mNormalRemoteViews;
    }

    /**
     * 
     * @return The big remoteviews {@link RemoteViews} of the notification.
     * @see Notification
     */
    public RemoteViews getBigRemoteViews() {
        return mBigRemoteViews;
    }

    /**
     * 
     * @param data The content of the Account {@link AccountInfo} which will be shown in the notification.
     */
    public void setData(final List<AccountInfo> data) {
        mData = data;
    }

    /**
     * Configure all the account views of the notification.
     */
    public void configureView() {
        if (mData != null) {

            Log.d(TAG, "---configureView---view size = " + mData.size());
            if (mData.size() > ROWACCOUNTNUMBER) {
                mBigRemoteViews.setViewVisibility(R.id.select_account_row_two_container, View.VISIBLE);
            }
            for (int i = 0; i < mData.size() && i < MOSTACCOUNTNUMBER; i++) {
                Log.d(TAG, "--- configure account id: " + i);

                AccountInfo accountInfo = mData.get(i);

                int resourceId[] = RESOURCE_ID[i];
                configureAccount(resourceId, accountInfo);
            }
            // Hide the redundant views
            int end = MOSTACCOUNTNUMBER;
            if (mData.size() <= ROWACCOUNTNUMBER) {
                end = ROWACCOUNTNUMBER;
                mBigRemoteViews.setViewVisibility(R.id.select_account_row_two_container, View.GONE);
            }
            for (int i = mData.size(); i < end; i++) {
                mBigRemoteViews.setViewVisibility(RESOURCE_ID[i][IdIndex.CONTAINER_ID], View.GONE);
            }
        } else {
            Log.w(TAG, "Data can not be null");
        }

    }

    private void configureAccount(int resourceId[], AccountInfo accountInfo) {

        if (accountInfo.getIcon() != null) {
            mBigRemoteViews.setViewVisibility(resourceId[IdIndex.CONTAINER_ID], View.VISIBLE);
            mBigRemoteViews.setImageViewBitmap(resourceId[IdIndex.IMG_ID], accountInfo.getIcon());
        } else if (accountInfo.getIconId() != 0) {
            mBigRemoteViews.setViewVisibility(resourceId[IdIndex.CONTAINER_ID], View.VISIBLE);
            mBigRemoteViews.setImageViewResource(resourceId[IdIndex.IMG_ID], accountInfo.getIconId());
        } else {
            Log.w(TAG, "--- The icon of account is null ---");
        }

        if (accountInfo.getLabel() == null) {
            mBigRemoteViews.setViewVisibility(resourceId[IdIndex.NAME_ID], View.GONE);
        } else {
            mBigRemoteViews.setTextViewText(resourceId[IdIndex.NAME_ID], accountInfo.getLabel());
        }

        if (accountInfo.getNumber() == null) {
            mBigRemoteViews.setViewVisibility(resourceId[IdIndex.NUMBER_ID], View.GONE);
        } else {
            mBigRemoteViews.setTextViewText(resourceId[IdIndex.NUMBER_ID], accountInfo.getNumber());
        }

        Log.d(TAG, "active: " + accountInfo.isActive());

        if (accountInfo.isActive()) {

            mBigRemoteViews.setViewVisibility(resourceId[IdIndex.NORMAL_DIVIDER_ID], View.GONE);
            mBigRemoteViews.setViewVisibility(resourceId[IdIndex.HIGHTLIGHT_DIVIDER_ID], View.VISIBLE);
        } else {

            mBigRemoteViews.setViewVisibility(resourceId[IdIndex.NORMAL_DIVIDER_ID], View.VISIBLE);
            mBigRemoteViews.setViewVisibility(resourceId[IdIndex.HIGHTLIGHT_DIVIDER_ID], View.GONE);
        }

        if (accountInfo.getIntent() != null) {
            PendingIntent pendingIntent = PendingIntent.getBroadcast(mContext, mRequestCode ++, accountInfo.getIntent(), PendingIntent.FLAG_UPDATE_CURRENT);
            mBigRemoteViews.setOnClickPendingIntent(resourceId[IdIndex.CONTAINER_ID], pendingIntent);
        }
    }

    public static class AccountInfo {
        private int mIconId;
        private Bitmap mIcon;
        private String mLabel;
        private String mNumber;
        private Intent mIntent;
        private boolean mIsActive;

        /**
         * Constructor
         * @param icon The icon of the account.
         * @param label The label of the account.
         * @param number The number of the account.
         * @param intent The behavior of the account.
         */
        public AccountInfo(Bitmap icon, String label, String number, Intent intent) {
            this(0, icon, label, number, intent, false);

        }

        /**
         * Constructor
         * @param iconId The icon id of the account.
         * @param label The label of the account.
         * @param number The number of the account.
         * @param intent The behavior of the account.
         */
        public AccountInfo(int iconId, String label, String number, Intent intent) {
            this(iconId, null, label, number, intent, false);
        }

        /**
         * Constructor
         * @param iconId The icon id of the account.
         * @param icon The icon of the account.
         * @param label The label of the account.
         * @param number The number of the account.
         * @param intent The behavior of the account.
         * @param isActive Whether the account is active or not.
         */
        public AccountInfo(int iconId, Bitmap icon, String label, String number, Intent intent, boolean isActive) {
            mIconId = iconId;
            mIcon = icon;
            mLabel = label;
            mNumber = number;
            mIntent = intent;
            mIsActive = isActive;
        }

        /**
         * 
         * @return Get the Icon id of the account.
         */
        public int getIconId() {
            if (mIconId != 0) {
                return mIconId;
            }
            return 0;
        }

        /**
         * 
         * @return Get the Icon of the account.
         */
        public Bitmap getIcon() {
            if (mIcon != null) {
                return mIcon;
            }
            return null;
        }

        /**
         * 
         * @return Get the label of the account.
         */
        public String getLabel() {
            return mLabel;
        }

        /**
         * 
         * @return Get the number of the account.
         */
        public String getNumber() {
            return mNumber;
        }

        /**
         * 
         * @return Get the intent of the account.
         */
        public Intent getIntent() {
            return mIntent;
        }

        /**
         * 
         * @return Get the active status of the account.
         */
        public boolean isActive() {
            return mIsActive;
        }

        /**
         * 
         * @param active Set the active status of the account.
         */
        public void setActiveStatus(boolean active) {
            mIsActive = active;
        }
    }
}
