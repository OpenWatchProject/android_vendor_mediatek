package com.mediatek.miravision.ui;

import android.app.Activity;
import android.app.KeyguardManager;
import android.content.pm.PackageManager;
import android.content.res.Configuration;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.os.Bundle;
import android.os.Handler;
import android.os.RemoteException;
import android.util.DisplayMetrics;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.view.ViewTreeObserver;
import android.widget.SeekBar;

import com.mediatek.miravision.setting.MiraVisionJni;
import com.mediatek.miravision.setting.MiraVisionJni.Range;
import com.mediatek.miravision.utils.Utils;

public class BasicColorTuningFragment extends BaseTuningFragment {

    private static final String TAG = "Miravision/BasicColorTuningFragment";
    private final int[] mTuningItems = { R.string.basic_color_item_contrast,
            R.string.basic_color_item_saturation, R.string.basic_color_item_brightness };

    public BasicColorTuningFragment() {
    }

    public BasicColorTuningFragment(int title) {
        mTuningTitle = title;
        for (int i = 0; i < mTuningItems.length; i++) {
            if (mTuningTitle == mTuningItems[i]) {
                index = i;
                break;
            }
        }
        Log.d(TAG, "mTuningTitle = " + mTuningTitle + ", index = " + index);
    }

    private Range mContrastRange;
    private Range mSaturationRange;
    private Range mBrightnessRange;

    private Image mImage;
    private int mBasicImageHeight = -1;
    private int mBasicImageWidth = -1;
    private int mImageViewHeight = -1;
    private int mImageViewWidth = -1;
    private Bitmap mBitmap;
    private boolean mHasMeasured;
    private int mWindowStackId = -1;

    @Override
    public View onCreateView(LayoutInflater inflater, ViewGroup container, Bundle savedInstanceState) {
        View rootView = super.onCreateView(inflater, container, savedInstanceState);
        Log.d(TAG, "onCreateView");
        mImageAdvAfter.setVisibility(View.GONE);

        mHasMeasured = false;
        return rootView;
    }

    @Override
    public void onResume() {
        super.onResume();
        Log.d(TAG, "onResume");
        Log.d(TAG, "isInMultiWindowMode: " + getActivity().isInMultiWindowMode());

        mContrastRange = MiraVisionJni.getContrastIndexRange();
        mSaturationRange = MiraVisionJni.getSaturationIndexRange();
        mBrightnessRange = MiraVisionJni.getPicBrightnessIndexRange();

        updateUi();

        mImage = (Image) mImageBasicAfter;
/* Mark for build error.
        try {
            mWindowStackId = getActivity().getWindowStackId();
        } catch (RemoteException e) {
            Log.w(TAG, "getWindowStackId RemoteException");
        }
*/
        mImage.init(new Handler(), (KeyguardManager) getActivity().getSystemService(
                Activity.KEYGUARD_SERVICE), getActivity().isInMultiWindowMode(), mWindowStackId);
        mImage.setHostFragmentResumed(true);
        String build = Utils.getSysProperty("ro.build.characteristics", null);
        if (build != null && build.equals("tablet")) {
            mImage.postInvalidate();
        }
    }

    @Override
    public void onPause() {
        super.onPause();
        Log.d(TAG, "onPause");
        mImage.setHostFragmentResumed(false);
        MiraVisionJni.nativeSetPQColorRegion(0, 0, 0, 0, 0);
    }

    @Override
    public void onDestroy() {
        super.onDestroy();
        MiraVisionJni.nativeSetPQColorRegion(0, 0, 0, 0, 0);
        if (mBitmap != null) {
            mBitmap.recycle();
            mBitmap = null;
        }
    }

    @Override
    public void onClick(View v) {
        v.setEnabled(false);
        super.onClick(v);
        Log.d(TAG, "onClick index = " + index);
        updateUi();
        v.setEnabled(true);
    }

    @Override
    public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser) {
        if (fromUser) {
            onSeekBarChange(seekBar.getProgress());
        }
    }

    @Override
    public void onStartTrackingTouch(SeekBar seekBar) {
    }

    @Override
    public void onStopTrackingTouch(SeekBar seekBar) {
    }

    private void onSeekBarChange(int value) {
        Log.d(TAG, "onChange value = " + value);
        setBasicTuningIndex(value);
        int resId = getResId();
        if (resId != -1) {
            mdpDecode(resId);
            mImageBasicAfter.setImageBitmap(mBitmap);
            mImageBasicAfter.invalidate();
        }
    }

    private void updateUi() {
        Log.d(TAG, "updateUi index = " + index);
        mTuningTitle = mTuningItems[index];
        mSeekBarText.setText(mTuningTitle);
        switch (mTuningTitle) {
        case R.string.basic_color_item_contrast:
            mLastButton.setVisibility(View.INVISIBLE);
            mSeekBar.setMax(mContrastRange.max - mContrastRange.min);
            mSeekBar.setProgress(MiraVisionJni.getContrastIndex() - mContrastRange.min);
            break;
        case R.string.basic_color_item_saturation:
            mLastButton.setVisibility(View.VISIBLE);
            mNextButton.setVisibility(View.VISIBLE);
            mSeekBar.setMax(mSaturationRange.max - mSaturationRange.min);
            mSeekBar.setProgress(MiraVisionJni.getSaturationIndex() - mSaturationRange.min);
            break;
        case R.string.basic_color_item_brightness:
            mNextButton.setVisibility(View.INVISIBLE);
            mSeekBar.setMax(mBrightnessRange.max - mBrightnessRange.min);
            mSeekBar.setProgress(MiraVisionJni.getPicBrightnessIndex() - mBrightnessRange.min);
            break;
        default:
            break;
        }
        // If not initial image resource, get the ImageView width and height
        // firstly.
        if (mImageViewWidth == -1 || mImageViewHeight == -1) {
            ViewTreeObserver treeObserver = mImageBasicAfter.getViewTreeObserver();
            treeObserver.addOnPreDrawListener(new ViewTreeObserver.OnPreDrawListener() {
                @Override
                public boolean onPreDraw() {
                    if (!mHasMeasured) {
                        mImageViewHeight = mImageBasicAfter.getWidth();
                        mImageViewWidth = mImageBasicAfter.getHeight();
                        initImageResource();
                        mHasMeasured = true;
                    }
                    return true;
                }
            });
        } else {
            initImageResource();
        }
    }

    private void initImageResource() {
        Log.d(TAG, "initImageResource");
        int index = getBasicTuningIndex();
        int resId = getResId();
        // Disable and initial the "before" image.
        setBasicTuningIndex(0);
        if (resId != -1) {
            mdpDecode(resId);
            mImageOrignal.setImageBitmap(mBitmap);
        }

        // Reset the index and initial the "after" image.
        setBasicTuningIndex(index);
        if (resId != -1) {
            mdpDecode(resId);
            mImageBasicAfter.setImageBitmap(mBitmap);
        }
    }

    private void setBasicTuningIndex(int value) {
        switch (mTuningTitle) {
        case R.string.basic_color_item_contrast:
            MiraVisionJni.setContrastIndex(value);
            break;
        case R.string.basic_color_item_saturation:
            MiraVisionJni.setSaturationIndex(value);
            break;
        case R.string.basic_color_item_brightness:
            MiraVisionJni.setPicBrightnessIndex(value);
            break;
        default:
            break;
        }
    }

    private int getBasicTuningIndex() {
        int index = 0;
        switch (mTuningTitle) {
        case R.string.basic_color_item_contrast:
            index = MiraVisionJni.getContrastIndex();
            break;
        case R.string.basic_color_item_saturation:
            index = MiraVisionJni.getSaturationIndex();
            break;
        case R.string.basic_color_item_brightness:
            index = MiraVisionJni.getPicBrightnessIndex();
            break;
        default:
            break;
        }
        return index;
    }

    private int getResId() {
        int resId = -1;
        switch (mTuningTitle) {
        case R.string.basic_color_item_contrast:
            resId = R.drawable.basic_color_tuning_contrast;
            break;
        case R.string.basic_color_item_saturation:
            resId = R.drawable.basic_color_tuning_saturation;
            break;
        case R.string.basic_color_item_brightness:
            resId = R.drawable.basic_color_tuning_brightness;
            break;
        default:
            break;
        }
        return resId;
    }

    /// M: BMW ALPS02948821,rotation leads to ImageView return null in freeform @{
    private int dipToPx(int dip) {
        final DisplayMetrics displayMetrics = getActivity().getResources().getDisplayMetrics();
        return (int) (displayMetrics.density * dip + 0.5f);
    }
    /// @}

    private void mdpDecode(int resId) {
        Log.d(TAG, "Start decode image: " + resId);
        BitmapFactory.Options options = new BitmapFactory.Options();
        byte[] imageData = Utils.obtainByteArray(getResources(), resId);
        if (mBasicImageHeight == -1 && mBasicImageWidth == -1) {
            options.inJustDecodeBounds = true;
            BitmapFactory.decodeByteArray(imageData, 0, imageData.length, options);
            mBasicImageHeight = options.outHeight;
            mBasicImageWidth = options.outWidth;
        }

        /// M: BMW ALPS02948821,rotation leads to ImageView return null in freeform @{
        boolean multiWindowSupport = getActivity().getPackageManager().hasSystemFeature(
                PackageManager.FEATURE_FREEFORM_WINDOW_MANAGEMENT);
        if (multiWindowSupport && (mImageViewHeight == 0 || mImageViewWidth == 0)) {
            Configuration config = new Configuration(getActivity().getResources()
                    .getConfiguration());
            int desiredWindowWidth = dipToPx(config.screenWidthDp);
            int desiredWindowHeight = dipToPx(config.screenHeightDp);
            mImageViewHeight = desiredWindowHeight / 2;
            mImageViewWidth = desiredWindowWidth / 2;
        }
        /// @}

        int yRatio = (int) Math.ceil(mBasicImageHeight / mImageViewHeight);
        int xRatio = (int) Math.ceil(mBasicImageWidth / mImageViewWidth);
        Log.d(TAG, "yRatio: " + yRatio + ", xRatio: " + xRatio);
        if (yRatio > 1 || xRatio > 1) {
            if (yRatio > xRatio) {
                options.inSampleSize = yRatio;
            } else {
                options.inSampleSize = xRatio;
            }
        }

        // Set this flag for basic image decoding.
        options = Utils.setInPostProc(options, true);
        options.inJustDecodeBounds = false;
        mBitmap = BitmapFactory.decodeByteArray(imageData, 0, imageData.length, options);
        Log.d(TAG, "Completed decoding image for basic image!");
    }
}
