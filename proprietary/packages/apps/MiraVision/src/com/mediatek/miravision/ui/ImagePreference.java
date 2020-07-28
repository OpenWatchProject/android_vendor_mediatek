package com.mediatek.miravision.ui;

import android.content.Context;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.graphics.drawable.Drawable;
import android.os.AsyncTask;
import android.preference.PreferenceCategory;
import android.util.AttributeSet;
import android.util.Log;
import android.view.View;
import android.view.ViewGroup;
import android.view.ViewTreeObserver;
import android.widget.ImageView;

import com.mediatek.miravision.utils.Utils;

public class ImagePreference extends PreferenceCategory {

    private static final String TAG = "Miravision/ImagePreference";
    private int mBasicImageHeight = -1;
    private int mBasicImageWidth = -1;
    private int mImageViewHeight = -1;
    private int mImageViewWidth = -1;
    private boolean mHasMeasured;
    private ImageView mImage;

    public ImagePreference(Context context, AttributeSet attrs, int defStyle) {
        super(context, attrs, defStyle);
        setLayoutResource(R.layout.preference_image);
    }

    public ImagePreference(Context context) {
        super(context);
        setLayoutResource(R.layout.preference_image);
    }

    public ImagePreference(Context context, AttributeSet attrs) {
        super(context, attrs);
        setLayoutResource(R.layout.preference_image);
    }

    @Override
    protected void onBindView(View view) {
        super.onBindView(view);
        setLayoutResource(R.layout.preference_image);
    }

    @Override
    protected View onCreateView(ViewGroup parent) {
        mHasMeasured = false;
        View view = super.onCreateView(parent);
        mImage = (ImageView) view.findViewById(R.id.percentage_image);
        // If not initial image resource, get the ImageView width and height
        // firstly.
        if (mImageViewWidth == -1 || mImageViewHeight == -1) {
            ViewTreeObserver treeObserver = mImage.getViewTreeObserver();
            treeObserver.addOnPreDrawListener(new ViewTreeObserver.OnPreDrawListener() {
                @Override
                public boolean onPreDraw() {
                    if (!mHasMeasured) {
                        Log.d(TAG, "onPreDraw");
                        mImageViewHeight = mImage.getWidth();
                        mImageViewWidth = mImage.getHeight();
                        mHasMeasured = true;
                    }
                    return true;
                }
            });
        }
        new LoadPicture(view).execute(R.drawable.picture_mode);
        return view;
    }

    private class LoadPicture extends AsyncTask<Integer, Void, Bitmap> {

        private View mView;

        public LoadPicture(View view) {
            mView = view;
        }

        @Override
        protected Bitmap doInBackground(Integer... params) {
            return mdpDecode(ImagePreference.this.getContext(), params[0]);
        }

        protected void onPostExecute(Bitmap result) {
            ImageView view = (ImageView) mView.findViewById(R.id.percentage_image);
            if (view != null) {
                view.setImageBitmap(result);
            }
        }
    }

    public void onModeChange() {
        if (mImage != null) {
            mImage.setImageBitmap(mdpDecode(ImagePreference.this.getContext(),
                    R.drawable.picture_mode));
            mImage.invalidate();
        }
    }

    private Bitmap mdpDecode(Context context, int resId) {
        Bitmap bitmapItem = null;
        Log.d(TAG, "Start decode image: " + resId);
        BitmapFactory.Options options = new BitmapFactory.Options();
        byte[] imageData = Utils.obtainByteArray(context.getResources(), resId);
        if (mBasicImageHeight == -1 && mBasicImageWidth == -1) {
            options.inJustDecodeBounds = true;
            BitmapFactory.decodeByteArray(imageData, 0, imageData.length, options);
            mBasicImageHeight = options.outHeight;
            mBasicImageWidth = options.outWidth;
        }

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
        Log.d(TAG, "Completed decoding image!");
        return BitmapFactory.decodeByteArray(imageData, 0, imageData.length, options);
    }
}
