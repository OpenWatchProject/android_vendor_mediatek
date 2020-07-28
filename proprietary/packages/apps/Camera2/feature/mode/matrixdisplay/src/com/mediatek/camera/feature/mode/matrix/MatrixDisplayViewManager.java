/*
 *   Copyright Statement:
 *
 *     This software/firmware and related documentation ("MediaTek Software") are
 *     protected under relevant copyright laws. The information contained herein is
 *     confidential and proprietary to MediaTek Inc. and/or its licensors. Without
 *     the prior written permission of MediaTek inc. and/or its licensors, any
 *     reproduction, modification, use or disclosure of MediaTek Software, and
 *     information contained herein, in whole or in part, shall be strictly
 *     prohibited.
 *
 *     MediaTek Inc. (C) 2016. All rights reserved.
 *
 *     BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 *    THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 *     RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
 *     ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
 *     WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 *     WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 *     NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH
 *     RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 *     INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES
 *     TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
 *     RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
 *     OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN MEDIATEK
 *     SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE
 *     RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 *     STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S
 *     ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE
 *     RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE
 *     MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE
 *     CHARGE PAID BY RECEIVER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 *     The following software/firmware and/or related documentation ("MediaTek
 *     Software") have been modified by MediaTek Inc. All revisions are subject to
 *     any receiver's applicable license agreements with MediaTek Inc.
 */

package com.mediatek.camera.feature.mode.matrix;

import android.app.Activity;
import android.content.Context;
import android.content.res.Configuration;
import android.graphics.SurfaceTexture;
import android.os.Handler;
import android.os.Looper;
import android.os.Message;
import android.view.LayoutInflater;
import android.view.Surface;
import android.view.TextureView;
import android.view.View;
import android.view.ViewGroup;
import android.view.ViewGroup.LayoutParams;
import android.view.animation.Animation;
import android.view.animation.AnimationUtils;
import android.widget.BaseAdapter;
import android.widget.RelativeLayout;
import android.widget.TextView;

import com.mediatek.camera.R;
import com.mediatek.camera.common.debug.LogHelper;
import com.mediatek.camera.common.debug.LogUtil;
import com.mediatek.camera.common.widget.Rotatable;
import com.mediatek.camera.common.widget.RotateLayout;

import java.util.List;

public class MatrixDisplayViewManager {
    private final static LogUtil.Tag TAG = new
            LogUtil.Tag(MatrixDisplayViewManager.class.getSimpleName());

    private static final String MTK_CONTROL_EFFECT_MODE_OFF = "none";
    private static final String MTK_CONTROL_EFFECT_MODE_ABCOLOR = "abcolor";
    private static final String MTK_CONTROL_EFFECT_MODE_COOLCOLOR = "coolcolor";
    private static final String MTK_CONTROL_EFFECT_MODE_DEV = "dev";
    private static final String MTK_CONTROL_EFFECT_MODE_FISHEYE = "fisheye";
    private static final String MTK_CONTROL_EFFECT_MODE_FOCUS = "focus";
    private static final String MTK_CONTROL_EFFECT_MODE_GRY = "gry";
    private static final String MTK_CONTROL_EFFECT_MODE_SEPIATON = "sepiaton";
    private static final String MTK_CONTROL_EFFECT_MODE_SOFTLIGHT = "softlight";
    private static final String[] mEffectName = {
            MTK_CONTROL_EFFECT_MODE_OFF,
            MTK_CONTROL_EFFECT_MODE_ABCOLOR,
            MTK_CONTROL_EFFECT_MODE_COOLCOLOR,
            MTK_CONTROL_EFFECT_MODE_DEV,
            MTK_CONTROL_EFFECT_MODE_FISHEYE,
            MTK_CONTROL_EFFECT_MODE_FOCUS,
            MTK_CONTROL_EFFECT_MODE_GRY,
            MTK_CONTROL_EFFECT_MODE_SEPIATON,
            MTK_CONTROL_EFFECT_MODE_SOFTLIGHT,
    };

    private static final int TIME_DELAY_SHOW_DISPLAY_LAYOUT_MS = 500;

    private static final int MSG_DELAY_ROTATE = 1;
    private static final int MSG_DISPLAY = 2;
    private static final int MSG_REMOVE_GRID = 0;
    private static final int EFFECT_NUM_OF_PAGE = 9;
    private static final int GRIDVIEW_STEP = 9;

    private ViewStateCallback mViewStateCallback;
    private SurfaceAvailableListener mSurfaceAvailableListener;
    private EffectUpdateListener mEffectUpdateListener;
    private ItemClickListener mItemClickListener;

    private MatrixDisplayView mGridView;
    private ViewGroup mEffectsLayout;
    private Surface[] mSurfaceList = new Surface[EFFECT_NUM_OF_PAGE];
    private Activity mActivity;
    private MyAdapter mAdapter;

    private List<CharSequence> mEffectEntryValues;
    private List<CharSequence> mEffectEntries;

    private Animation mFadeIn;
    private Animation mFadeOut;

    private int mNumsOfEffect = 0;
    private int mDisplayWidth = 0;
    private int mDisplayHeight = 0;
    private int mOrientation = 0;
    private int mSelectedPosition = 0;
    private int mDisplayOrientation = 0;

    private boolean mMirror = false;
    private boolean mEffectsDone = false;
    private boolean mShowEffects = false;
    private boolean mNeedScrollToFirstPosition = true;
    private boolean mSizeChanged = false;

    /**
     * Callback matrix display view state.
     */
    public interface ViewStateCallback {
        /**
         * Callback when view is going to create.
         */
        void onViewCreated();

        /**
         * Callback when view is scroll out.
         */
        void onViewScrollOut();

        /**
         * Callback when view is hidden.
         */
        void onViewHidden();

        /**
         * Callback when view is destroyed.
         */
        void onViewDestroyed();
    }

    /**
     * Listener to listen surface available.
     */
    public interface SurfaceAvailableListener {
        /**
         * Invoked when surface is available.
         *
         * @param surface  The surface object.
         * @param width    The width of surface.
         * @param height   The height of surface.
         * @param position The position of surface in the nine grid layout.
         */
        void onSurfaceAvailable(Surface[] surface, int width, int height, int position);
    }


    /**
     * Listener to listen the effect going to shown updated.
     */
    public interface EffectUpdateListener {
        /**
         * Invoked when effect going to shown are updated.
         *
         * @param effectIndex The index of effect.
         * @param position    The position of effect shown in nine grid layout.
         */
        void onEffectUpdated(int position, int effectIndex);
    }

    /**
     * Listener to listen effect clicked by user.
     */
    public interface ItemClickListener {
        /**
         * Invoked when user click one effect.
         *
         * @param effect The effect name selected by user.
         * @return True means item click successfully.
         */
        boolean onItemClicked(int effect);
    }

    /**
     * Constructor,initialize activity.
     *
     * @param activity activity
     */
    public MatrixDisplayViewManager(Activity activity) {
        mActivity = activity;
    }

    /**
     * Set view state callback to listen view state changed.
     *
     * @param callback The instance of {@link MatrixDisplayViewManager.ViewStateCallback}.
     */
    public void setViewStateCallback(ViewStateCallback callback) {
        mViewStateCallback = callback;
    }

    /**
     * Set surface available listener to listen surface available.
     *
     * @param listener The instance of {@link MatrixDisplayViewManager.SurfaceAvailableListener}.
     */
    public void setSurfaceAvailableListener(SurfaceAvailableListener listener) {
        mSurfaceAvailableListener = listener;
    }

    /**
     * Set listener to listen effect updated.
     *
     * @param listener The instance of {@link MatrixDisplayViewManager.EffectUpdateListener}.
     */
    public void setEffectUpdateListener(EffectUpdateListener listener) {
        mEffectUpdateListener = listener;
    }

    /**
     * Set listener to listen the item clicked.
     *
     * @param listener The instance of {@link MatrixDisplayViewManager.ItemClickListener}.
     */
    public void setItemClickListener(MatrixDisplayViewManager.ItemClickListener listener) {
        mItemClickListener = listener;
    }

    /**
     * Set display orientation.
     *
     * @param displayOrientation display orientation
     */
    public void setDisplayOrientation(int displayOrientation) {
        mDisplayOrientation = displayOrientation;
    }

    /**
     * Set orientation.
     *
     * @param orientation orientation data
     */
    public void setOrientation(int orientation) {
        if (mOrientation != orientation) {
            LogHelper.d(TAG, "<setOrientation> mOrientation:" + mOrientation
                    + ",orientation:" + orientation);
            mOrientation = orientation;
            rotateGridViewItem(orientation);
        }
    }

    /**
     * Display effect image.
     */
    public void onEffectAvailable() {
        LogHelper.d(TAG, "[onEffectAvailable]");
        mMainHandler.sendEmptyMessage(MSG_DISPLAY);
        mEffectsDone = true;
    }

    /**
     * Set effects that going to show.
     *
     * @param effectEntries     The effect name shown on UI.
     * @param effectEntryValues The effect string used in logic.
     */
    public void setEffectEntriesAndEntryValues(List<CharSequence> effectEntries,
                                               List<CharSequence> effectEntryValues) {
        mEffectEntries = effectEntries;
        mEffectEntryValues = effectEntryValues;
        mNumsOfEffect = mEffectEntryValues.size();
    }

    /**
     * Set the selected effect.
     *
     * @param effect The selected effect.
     */
    public void setSelectedEffect(String effect) {
        int index = mEffectEntryValues.indexOf(effect);
        mSelectedPosition = index;
    }

    /**
     * Set preview mirror.
     *
     * @param mirror True means preview need to mirror.
     */
    public void setMirror(boolean mirror) {
        mMirror = mirror;
    }

    /**
     * Show nine grid view.
     */
    public void showView() {
        LogHelper.d(TAG, "[showView]..., start");
        mMainHandler.removeMessages(MSG_REMOVE_GRID);
        mShowEffects = true;

        if (mGridView != null && mNeedScrollToFirstPosition) {
            mNeedScrollToFirstPosition = false;
            mGridView.scrollToSelectedPosition(mSelectedPosition);
        }
        if (mGridView != null) {
            mGridView.showSelectedBorder(mSelectedPosition);
        }

        if (mEffectsLayout == null) {
            initialEffect();
        }

        if (mEffectsDone) {
            // delay 500ms to avoid the grid preview is frozen during animation.
            mMainHandler.sendEmptyMessageDelayed(MSG_DISPLAY, TIME_DELAY_SHOW_DISPLAY_LAYOUT_MS);
        }

        LogHelper.d(TAG, "[showEffect]..., end");
    }

    public boolean isMatrixDisplayShowing() {
        return mShowEffects;
    }


    /**
     * Hide nine grid view.
     *
     * @param animation True to run fading out animation when hidden.
     * @param delay     The time of delay to remove nine grid layout from view tree.
     */
    public void hideView(boolean animation, int delay) {
        hideEffect(animation, delay);
    }

    /**
     * Set the layout size of nine grid layout.
     *
     * @param width  The width of layout.
     * @param height The height of layout.
     */
    public void setLayoutSize(int width, int height) {
        LogHelper.d(TAG, "[setLayoutSize], inputSize, width:" + width + ", height:" + height
                + "displayOrientation:" + "" + mDisplayOrientation
                + ", mOrientation:" + mOrientation);
        mDisplayWidth = Math.max(width, height);
        mDisplayHeight = Math.min(width, height);
        //int displayRotation = mIModuleCtrl.getDisplayRotation();
        // when launch camera from WallPaper, the width and height of display
        // size should be rotate
        // in portrait
        if (false/* getContext().isVideoWallPaperIntent() */) {
            if (width < height) {
                int temp = mDisplayWidth;
                mDisplayWidth = mDisplayHeight;
                mDisplayHeight = temp;
            }
        } else {
            if (mGridView != null) {
                // this method is running in GLThread, so post runnable to set
                // display size.
                mMainHandler.post(new Runnable() {
                    @Override
                    public void run() {
                        LogHelper.d(TAG, "setDisplaySize(" + mDisplayWidth + ","
                                + mDisplayHeight + ")");
                        if (mGridView != null) {
                            mGridView.setLayoutSize(mDisplayWidth, mDisplayHeight);
                        }
                    }
                });
            }
        }
        LogHelper.d(TAG, "onSizeChanged(), outputSize, mDisplayWidth:" + mDisplayWidth
                + ", mDisplayHeight:" + mDisplayHeight);
        mSizeChanged = true;
    }

    private Handler mMainHandler = new Handler(Looper.myLooper()) {

        @Override
        public void handleMessage(Message msg) {
            LogHelper.d(TAG, "[handleMessage],msg.what:" + msg.what);
            switch (msg.what) {
                case MSG_REMOVE_GRID:
                    // If we removeView and addView frequently, drawing cache may be
                    // wrong.
                    // Here avoid do this action frequently to workaround that
                    // issue.
                    if (mGridView != null) {
                        mGridView.removeAllViews();
                    }
                    if (mEffectsLayout != null && mEffectsLayout.getParent() != null) {
                        mActivity.runOnUiThread(new Runnable() {
                            @Override
                            public void run() {
                                mEffectsLayout.removeAllViews();
                                ((ViewGroup) mEffectsLayout.getParent()).removeView(mEffectsLayout);
                                mGridView = null;
                                mEffectsLayout = null;
                                mEffectsDone = false;
                                if (mViewStateCallback != null) {
                                    mViewStateCallback.onViewDestroyed();
                                }
                            }
                        });
                    }
                    break;

                case MSG_DELAY_ROTATE:
                    rotateGridViewItem(mOrientation);
                    break;

                case MSG_DISPLAY:
                    if (mEffectsLayout != null) {
                        startFadeInAnimation(mEffectsLayout);
                        mEffectsLayout.setAlpha(1.0f);
                        mEffectsLayout.setVisibility(View.VISIBLE);
                    }
                    break;
                default:
                    break;
            }
        }
    };

    /**
     * This adapter used to set to grid view.
     */
    private class MyAdapter extends BaseAdapter {
        private LayoutInflater mLayoutInflater;

        /**
         * Constructor, initialize layout inflater.
         *
         * @param context activity context
         */
        public MyAdapter(Context context) {
            mLayoutInflater = LayoutInflater.from(context);
        }

        @Override
        public int getCount() {
            return mNumsOfEffect;
        }

        @Override
        public Object getItem(int position) {
            return mSurfaceList[position];
        }

        @Override
        public long getItemId(int position) {
            return position;
        }

        /**
         * View holder, save view information.
         */
        private class ViewHolder {
            TextureView mTextureView;
            TextView mTextView;
            int mPosition;
            RotateLayout mRotateLayout;
        }

        @Override
        public View getView(int position, View convertView, ViewGroup parent) {
            LogHelper.d(TAG, "convertView:" + convertView + ", position:" + position);
            if (mEffectsLayout == null) {
                LogHelper.d(TAG, "mEffectsLayout is null");
                return null;
            }

            ViewHolder holder = null;
//            int effectId = getEffectId(mEffectEntryValues.get(position).toString());

            // when firstly go in lomo ui, the effect of position 9 to 11 won't
            // in sight,
            // so the position 9 to 11 won't need effect processing.
//            if (position > 8 && position < 12 && convertView == null) {
//                effectId = -1;
//            }
//            if (position >= GRIDVIEW_STEP) {
//                int index = position - GRIDVIEW_STEP;
//                mEffectUpdateListener.onEffectUpdated(index, effectId);
//            } else {
//                mEffectUpdateListener.onEffectUpdated(position, effectId);
//            }

            if (convertView == null) {
                convertView = mLayoutInflater.inflate(R.layout.lomo_effects_item, null);
                holder = new MyAdapter.ViewHolder();
                holder.mTextureView = (TextureView) convertView.findViewById(R.id.textureview);
                holder.mTextView = (TextView) convertView.findViewById(R.id.effects_name);
                holder.mRotateLayout = (RotateLayout) convertView.findViewById(R.id.rotate);
                RelativeLayout.LayoutParams params = new RelativeLayout.LayoutParams(
                        (int) Math.ceil((double) mDisplayWidth / 3),
                        (int) Math.ceil((double) mDisplayHeight / 3));
                holder.mTextureView.setLayoutParams(params);
                int padding = convertView.getPaddingLeft();
                if (mDisplayOrientation == 270 || mDisplayOrientation == 180) {
                    //rotate around the center of textureview
                    holder.mTextureView.setPivotX((float) params.width / 2.f - (float) padding);
                    holder.mTextureView.setPivotY((float) params.height / 2.f - (float) padding);
                    holder.mTextureView.setRotation(180);
                }

                if (mMirror) {
                    //rotate around the center of textureview
                    holder.mTextureView.setPivotX((float) params.width / 2.f - (float) padding);
                    holder.mTextureView.setPivotY((float) params.height / 2.f - (float) padding);
                    holder.mTextureView.setRotationY(180);
                }
                SurfaceTextureListener listener = new SurfaceTextureListener(position);
                holder.mTextureView.setSurfaceTextureListener(listener);

                holder.mPosition = position;
                convertView.setTag(holder);
            } else {
                holder = (MyAdapter.ViewHolder) convertView.getTag();
            }
            holder.mTextView.setText(mEffectEntries.get(position));
            // some times the content of TextView do not update, so force to
            // call requestLayout()
            int rotation = computeRotation(mActivity, mOrientation, 270);
            layoutByOrientation(holder.mRotateLayout, rotation);
            setOrientation(holder.mRotateLayout, rotation, true);
            return convertView;
        }
    }

    private int getEffectId(String effectName) {
        for (int i = 0; i < mEffectName.length; i++) {
            if (equals(effectName, mEffectName[i])) {
                LogHelper.d(TAG, "[getEffectId] effectName:" + effectName + ", effetId:" + i);
                return i;
            }
        }
        LogHelper.d(TAG, "[getEffectId] effectName:" + effectName + ", effetId: -1");
        return -1;
    }

    private boolean equals(Object a, Object b) {
        return (a == b) || (a == null ? false : a.equals(b));
    }


    /**
     * Surface texture listener.
     */
    private class SurfaceTextureListener implements TextureView.SurfaceTextureListener {
        private int mPosition;
        private int mWidth, mHeight;

        /**
         * Consturctor, initialize postion.
         *
         * @param position position.
         */
        public SurfaceTextureListener(int position) {
            mPosition = position;
        }

        @Override
        public void onSurfaceTextureAvailable(SurfaceTexture surface, int width, int height) {
            LogHelper.d(TAG, "[onSurfaceTextureAvailable], surface:" + surface + ", width:"
                    + width + ", " + "height:" + height + ", mPosition:" + mPosition);
            mWidth = width;
            mHeight = height;

            mSurfaceList[mPosition] = new Surface(surface);
            LogHelper.i(TAG, "[onSurfaceTextureAvailable] mSurfaceAvailableListener = "
                    + mSurfaceAvailableListener
                    + ", mPosition = " + mPosition
                    + " , mNumsOfEffect = " + mNumsOfEffect);
            if (mSurfaceAvailableListener != null && mPosition == mNumsOfEffect - 1) {
                mSurfaceAvailableListener.onSurfaceAvailable(mSurfaceList,
                        mWidth, mHeight, mPosition);
            }


        }

        @Override
        public void onSurfaceTextureSizeChanged(SurfaceTexture surfaceTexture, int i, int i1) {

        }

        @Override
        public boolean onSurfaceTextureDestroyed(SurfaceTexture surface) {
            LogHelper.d(TAG, "onSurfaceTextureDestroyed(), surface:" + surface + "and mPosition:"
                    + mPosition);
            mSurfaceList[mPosition].release();
            return true;
        }

        @Override
        public void onSurfaceTextureUpdated(SurfaceTexture surfaceTexture) {

        }


    }

    private int computeRotation(Context context, int orientation, int compensation) {
        int rotation = orientation;
        int activityOrientation = context.getResources().getConfiguration().orientation;
        if (activityOrientation == Configuration.ORIENTATION_PORTRAIT) {
            // since portrait mode use ModePickerRotateLayout rotate 270, here
            // need to compensation,
            // compensation should be 270.
            rotation = (orientation - compensation + 360) % 360;
        }
        return rotation;
    }

    private void layoutByOrientation(RotateLayout layout, int orientation) {
        RelativeLayout.LayoutParams lp = (RelativeLayout.LayoutParams) layout.getLayoutParams();
        switch (orientation) {
            case 0:
                lp.addRule(RelativeLayout.ALIGN_PARENT_RIGHT);
                lp.addRule(RelativeLayout.ALIGN_PARENT_BOTTOM);
                lp.removeRule(RelativeLayout.ALIGN_PARENT_LEFT);
                lp.removeRule(RelativeLayout.ALIGN_PARENT_TOP);
                break;

            case 180:
                lp.addRule(RelativeLayout.ALIGN_PARENT_LEFT);
                lp.addRule(RelativeLayout.ALIGN_PARENT_TOP);
                lp.removeRule(RelativeLayout.ALIGN_PARENT_RIGHT);
                lp.removeRule(RelativeLayout.ALIGN_PARENT_BOTTOM);
                break;

            case 90:
                lp.addRule(RelativeLayout.ALIGN_PARENT_RIGHT);
                lp.addRule(RelativeLayout.ALIGN_PARENT_TOP);
                lp.removeRule(RelativeLayout.ALIGN_PARENT_LEFT);
                lp.removeRule(RelativeLayout.ALIGN_PARENT_BOTTOM);
                break;

            case 270:
                lp.addRule(RelativeLayout.ALIGN_PARENT_LEFT);
                lp.addRule(RelativeLayout.ALIGN_PARENT_BOTTOM);
                lp.removeRule(RelativeLayout.ALIGN_PARENT_RIGHT);
                lp.removeRule(RelativeLayout.ALIGN_PARENT_TOP);
                break;

            default:
                break;
        }
        layout.setLayoutParams(lp);
        layout.requestLayout();
    }

    private void setOrientation(View view, int orientation, boolean animation) {
        if (view == null) {
            LogHelper.d(TAG, "[setOrientation]view is null,return.");
            return;
        }
        if (view instanceof Rotatable) {
            ((Rotatable) view).setOrientation(orientation, animation);
        } else if (view instanceof ViewGroup) {
            ViewGroup group = (ViewGroup) view;
            for (int i = 0, count = group.getChildCount(); i < count; i++) {
                setOrientation(group.getChildAt(i), orientation, animation);
            }
        }
    }

    private void hideEffect(boolean animation, long delay) {
        LogHelper.d(TAG, "hideEffect(), animation:" + animation + ", mEffectsLayout:" +
                "" + mEffectsLayout);
        if (mEffectsLayout != null) {
            mMainHandler.removeMessages(MSG_REMOVE_GRID);
            mShowEffects = false;
            if (animation) {
                startFadeOutAnimation(mEffectsLayout);
            }
            mEffectsLayout.setVisibility(View.GONE);
            mMainHandler.sendEmptyMessageDelayed(MSG_REMOVE_GRID, delay);

            if (mViewStateCallback != null) {
                mViewStateCallback.onViewHidden();
            }
        }
    }

    private void startFadeOutAnimation(View view) {
        if (mFadeOut == null) {
            mFadeOut = AnimationUtils.loadAnimation(mActivity, R.anim.grid_effects_fade_out);
            mFadeOut.setAnimationListener(new Animation.AnimationListener() {

                @Override
                public void onAnimationStart(Animation animation) {

                }

                @Override
                public void onAnimationRepeat(Animation animation) {

                }

                @Override
                public void onAnimationEnd(Animation animation) {
                    // show();
                }
            });
        }
        if (view != null) {
            view.startAnimation(mFadeOut);
            mFadeOut = null;
        }
    }

    private void rotateGridViewItem(int orientation) {
        LogHelper.d(TAG, "rotateGridViewItem(), orientation:" + orientation);
        // since portrait mode use ModePickerRotateLayout rotate 270, here need
        // to compensation,
        // compensation should be 270.
        int rotation = computeRotation(mActivity, orientation, 270);
        if (mGridView != null) {
            int rows = mGridView.getChildCount();
            for (int i = 0; i < rows; i++) {
                ViewGroup rowView = (ViewGroup) mGridView.getChildAt(i);
                if (rowView != null) {
                    int cellCounts = rowView.getChildCount();
                    for (int j = 0; j < cellCounts; j++) {
                        View cellView = rowView.getChildAt(j);
                        if (cellView != null) {
                            RotateLayout layout = (RotateLayout) cellView
                                    .findViewById(R.id.rotate);
                            layoutByOrientation(layout, rotation);
                            setOrientation(layout, rotation, true);
                        }
                    }
                }
            }
        }
    }

    private void startFadeInAnimation(View view) {
        if (mFadeIn == null) {

            mFadeIn = AnimationUtils.loadAnimation(mActivity, R.anim.gird_effects_fade_in);
        }
        if (view != null && mFadeIn != null) {

            view.startAnimation(mFadeIn);
            mFadeIn = null;
        }
    }


    private void initialEffect() {
        LogHelper.d(TAG, "[initialEffect]mEffectsLayout:" + mEffectsLayout + ", mSizeChanged:"
                + mSizeChanged + ", mMirror:" + mMirror);
        if (mEffectsLayout == null) {
            LogHelper.d(TAG, "nums of effect:" + mNumsOfEffect + ",mViewStateCallback" +
                    mViewStateCallback);
            mEffectsLayout = (ViewGroup) mActivity.getLayoutInflater()
                    .inflate(R.layout.lomo_effects, null);
            RelativeLayout.LayoutParams params = new RelativeLayout.LayoutParams(
                    LayoutParams.MATCH_PARENT, LayoutParams.MATCH_PARENT);
            params.topMargin = 0;
            mActivity.addContentView(mEffectsLayout, params);
            if (mViewStateCallback != null) {
                mViewStateCallback.onViewCreated();
            }
            mGridView = (MatrixDisplayView) mEffectsLayout.findViewById(R.id.lomo_effect_gridview);
            int columnWidth = mDisplayWidth % 3 == 0 ? mDisplayWidth / 3 : (mDisplayWidth / 3 + 1);
            mGridView.setGridWidth(columnWidth);
            mGridView.setGridHeight(mDisplayHeight / 3);
            mGridView.setGridCountInColumn(3);
            mGridView.setLayoutSize(columnWidth * 3, mDisplayHeight);
            mAdapter = new MyAdapter(mActivity);
            mGridView.setAdapter(mAdapter);
            mGridView.setOnItemClickListener(new MyOnItemClickListener());
            mGridView.setSelector(R.drawable.lomo_effect_selector);
            mGridView.setOverScrollMode(View.OVER_SCROLL_ALWAYS);
            mGridView.setOnScrollListener(new MyOnScrollListener());

            mGridView.scrollToSelectedPosition(mSelectedPosition);
            mGridView.showSelectedBorder(mSelectedPosition);
            mSizeChanged = false;
            mEffectsLayout.setAlpha(0.0f);
        } else {
            if (mSizeChanged) {
                mGridView.setLayoutSize(mDisplayWidth, mDisplayHeight);
                mSizeChanged = false;
            }
        }
    }

    /**
     * Scroll listener.
     */
    private class MyOnScrollListener implements MatrixDisplayView.OnScrollListener {

        @Override
        public void onScrollOut(MatrixDisplayView view, int direction) {
            LogHelper.d(TAG, "onScrollOut()");
            if (direction == DIRECTION_UP) {
                mNeedScrollToFirstPosition = true;
                if (mViewStateCallback != null) {
                    mViewStateCallback.onViewScrollOut();
                }
            }
        }

        @Override
        public void onScrollDone(MatrixDisplayView view, int startPosition, int endPosition) {
//            LogHelper.d(TAG, "[onScrollDone], startPosition:" + startPosition + ", endPosition:"
//                    + endPosition);
//            for (int j = startPosition; j < endPosition; j++) {
//                int effectId = getEffectId(mEffectEntryValues.get(j).toString());
//                int position = j % 9;
//                if (mEffectUpdateListener != null) {
//                    mEffectUpdateListener.onEffectUpdated(position, effectId);
//                }
//            }
//
//            if (startPosition == 0) {
//                for (int i = endPosition; i < endPosition + 3; i++) {
//                    if (mEffectUpdateListener != null) {
//                        mEffectUpdateListener.onEffectUpdated(i, -1);
//                    }
//                }
//            } else {
//                for (int i = startPosition - 1; i >= startPosition - 3; i--) {
//                    if (mEffectUpdateListener != null) {
//                        mEffectUpdateListener.onEffectUpdated(i, -1);
//                    }
//                }
//            }
        }
    }

    /**
     * Item click listener.
     */
    private class MyOnItemClickListener implements MatrixDisplayView.OnItemClickListener {

        @Override
        public boolean onItemClick(View view, int position) {
            LogHelper.d(TAG, "[onItemClick], position:" + position);
            if (!mEffectsDone) {
                return false;
            }

            boolean succeed = false;
            if (mItemClickListener != null) {
//                succeed = mItemClickListener
//                        .onItemClicked(mEffectEntryValues.get(position).toString());
                succeed = mItemClickListener
                        .onItemClicked(position);
            }
            return succeed;
        }
    }
}
