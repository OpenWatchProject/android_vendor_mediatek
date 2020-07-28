package com.mediatek.op18.dialer.calllog;


import android.content.Context;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.graphics.Canvas;
import android.graphics.drawable.BitmapDrawable;
import android.graphics.drawable.Drawable;
import android.graphics.PorterDuff;
import android.database.Cursor;
import android.provider.CallLog.Calls;
import android.util.Log;
//import com.android.dialer.calllogutils.CallTypeIconsView;
import com.mediatek.dialer.ext.ICallLogListItemViewHolderExt;
import com.mediatek.dialer.ext.DefaultCallLogExt;
import android.support.v7.widget.RecyclerView;
import com.mediatek.op18.dialer.R;


/**
 * Class to implement VoWifi and VoLTE features in Call Log
 */
public class Op18CallLogExtension extends DefaultCallLogExt {

    private Context mContext;
    private static final String TAG = "Op18CallLogExtension";
    private static final int FEATURES_VoLTE = 0x4;
    private static final int FEATURES_VoWIFI = 0x8;
    private static final int FEATURES_ViWIFI = 0x10;

    private Drawable mVoWifiCall;
    private Drawable mVolteCall;
    private Drawable mViWifiCall;

    private Bitmap mVoWifiIcon;
    private Bitmap mVolteIcon;
    private Bitmap mViWifiIcon;

    public boolean showVoWifi = false;
    public boolean showVolte = false;
    public boolean showViWifi = false;


    public Op18CallLogExtension(Context context) {
        mContext = context;
    }

    @Override
    public ICallLogListItemViewHolderExt getCallLogListItemViewHolderExt(
            Context context, RecyclerView.ViewHolder viewHolder) {
      Log.d(TAG, "getCallLogListItemViewHolderExt");
      return new Op18CallLogListItemViewHolderExt(context, viewHolder);
    }

    public void drawWifiVolteCallIcon(int scaledHeight) {
        Log.d(TAG, "drawWifiVolteCallIcon");
        final android.content.res.Resources r = mContext.getResources();

        mVoWifiIcon = BitmapFactory.decodeResource(mContext.getResources(),
                R.drawable.vowifi_call);
        mVolteIcon = BitmapFactory.decodeResource(mContext.getResources(),
                R.drawable.volte_call);
        mViWifiIcon = BitmapFactory.decodeResource(mContext.getResources(),
                R.drawable.video_call_over_wifi);

        int scaledWidth1 = (int) ((float) mVoWifiIcon.getWidth() *
                ((float) scaledHeight /
                        (float) mVoWifiIcon.getHeight()));
        int scaledWidth2 = (int) ((float) mVolteIcon.getWidth() *
                ((float) scaledHeight /
                        (float) mVolteIcon.getHeight()));
        int scaledWidth3 = (int) ((float) mViWifiIcon.getWidth() *
                ((float) scaledHeight /
                        (float) mViWifiIcon.getHeight()));

        Bitmap scaled1 = Bitmap.createScaledBitmap(mVoWifiIcon, scaledWidth1, scaledHeight, false);
        Bitmap scaled2 = Bitmap.createScaledBitmap(mVolteIcon, scaledWidth2, scaledHeight, false);
        Bitmap scaled3 = Bitmap.createScaledBitmap(mViWifiIcon, scaledWidth3, scaledHeight, false);

        mVoWifiCall = new BitmapDrawable(mContext.getResources(), scaled1);
        mVoWifiCall.setColorFilter(r.getColor(R.color.dialtacts_secondary_text_color),
               PorterDuff.Mode.MULTIPLY);
        mVolteCall = new BitmapDrawable(mContext.getResources(), scaled2);
        mVolteCall.setColorFilter(r.getColor(R.color.dialtacts_secondary_text_color),
               PorterDuff.Mode.MULTIPLY);
        mViWifiCall = new BitmapDrawable(mContext.getResources(), scaled3);
        mViWifiCall.setColorFilter(r.getColor(R.color.dialtacts_secondary_text_color),
               PorterDuff.Mode.MULTIPLY);
        Log.d(TAG, "drawWifiVolteCanvas  mVoWifiIcon.getHeight()" +  mVoWifiIcon.getHeight());
        Log.d(TAG, "drawWifiVolteCanvas  mVoWifiCall.getIntrinsicHeight()" +
                mVoWifiCall.getIntrinsicHeight());
    }


    public void drawWifiVolteCanvas(int left, Canvas canvas, Object callTypeIconViewObj) {
        /*CallTypeIconsView callTypeIcons = (CallTypeIconsView) callTypeIconViewObj;
        Log.d(TAG, "drawWifiVolteCanvas");

        if (showVoWifi) {
            final Drawable drawable = mVoWifiCall;
            final int right = left + mVoWifiCall.getIntrinsicWidth();
            drawable.setBounds(left, 0, right, mVoWifiCall.getIntrinsicHeight());
            drawable.draw(canvas);
            Log.d(TAG, "drawWifiVolteCanvas ShowVoWifi:" + showVoWifi);
        }
        if (showVolte) {
            final Drawable drawable = mVolteCall;
            final int right = left + mVolteCall.getIntrinsicWidth();
            drawable.setBounds(left, 0, right, mVolteCall.getIntrinsicHeight());
            drawable.draw(canvas);
            Log.d(TAG, "drawWifiVolteCanvas ShowVolte:" + showVolte);
        }
        if (showViWifi) {
            final Drawable drawable = mViWifiCall;
            final int right = left + mViWifiCall.getIntrinsicWidth();
            drawable.setBounds(left, 0, right, mViWifiCall.getIntrinsicHeight());
            drawable.draw(canvas);
            Log.d(TAG, "drawWifiVolteCanvas ShowViWifi:" + showViWifi);
        }*/
    }


    public void setShowVolteWifi(Object object, int features) {
        Log.d(TAG, "setShowVolteWifi features:" + features);
        /*CallTypeIconsView callTypeIcons = (CallTypeIconsView) object;

        setShowVowifi(callTypeIcons, (features & FEATURES_VoWIFI) == FEATURES_VoWIFI);
        setShowVolte(callTypeIcons, (features & FEATURES_VoLTE) == FEATURES_VoLTE);
        setShowViWifi(callTypeIcons, (features & FEATURES_ViWIFI) == FEATURES_ViWIFI);

        if (showVoWifi) {
            callTypeIcons.width += mVoWifiCall.getIntrinsicWidth();
            callTypeIcons.height = Math.max(callTypeIcons.height,
                    mVoWifiCall.getIntrinsicHeight());
            callTypeIcons.invalidate();
        }
        if (showVolte) {
            callTypeIcons.width += mVolteCall.getIntrinsicWidth();
            callTypeIcons.height = Math.max(callTypeIcons.height,
                    mVolteCall.getIntrinsicHeight());
            callTypeIcons.invalidate();
        }

        if (showViWifi) {
            callTypeIcons.width += mViWifiCall.getIntrinsicWidth();
            callTypeIcons.height = Math.max(callTypeIcons.height,
                    mViWifiCall.getIntrinsicHeight());
            callTypeIcons.invalidate();
        }*/
    }

    public boolean isViWifiShown(Object object) {
        Log.d(TAG, "isViWifiShown");
        //CallTypeIconsView callTypeIcons = (CallTypeIconsView) object;
        return showViWifi;
    }


    public boolean sameCallFeature(Cursor cursor) {
        Log.d(TAG, "sameCallFeature");
        final long currentCallFeature =
            cursor.getLong(cursor.getColumnIndex(Calls.FEATURES));

        cursor.moveToPrevious();
        final long prevCallFeature =
            cursor.getLong(cursor.getColumnIndex(Calls.FEATURES));
        cursor.moveToNext();
        final boolean sameCallFeature = (prevCallFeature == currentCallFeature);
        Log.d(TAG, "prevCallFeature:" + prevCallFeature + "currentCallFeature:"
                + currentCallFeature);
        return sameCallFeature;
    }

    /*private void setShowVowifi(CallTypeIconsView callTypeIcons, boolean showWifi) {
        showVoWifi = showWifi;
        Log.d(TAG, "setShowVowifi:" + showWifi);
    }

    private void setShowVolte(CallTypeIconsView callTypeIcons, boolean showvolte) {
        showVolte = showvolte;
        Log.d(TAG, "setShowVolte:" + showVolte);
    }

    private void setShowViWifi(CallTypeIconsView callTypeIcons, boolean showviWifi) {
        showViWifi = showviWifi;
        Log.d(TAG, "setShowViWifi:" + showViWifi);
    }*/
}
