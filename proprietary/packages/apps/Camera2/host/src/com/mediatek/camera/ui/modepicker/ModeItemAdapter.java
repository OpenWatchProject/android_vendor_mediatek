/*
 * Copyright Statement:
 *
 *   This software/firmware and related documentation ("MediaTek Software") are
 *   protected under relevant copyright laws. The information contained herein is
 *   confidential and proprietary to MediaTek Inc. and/or its licensors. Without
 *   the prior written permission of MediaTek inc. and/or its licensors, any
 *   reproduction, modification, use or disclosure of MediaTek Software, and
 *   information contained herein, in whole or in part, shall be strictly
 *   prohibited.
 *
 *   MediaTek Inc. (C) 2016. All rights reserved.
 *
 *   BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 *   THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 *   RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
 *   ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
 *   WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 *   WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 *   NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH
 *   RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 *   INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES
 *   TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
 *   RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
 *   OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN MEDIATEK
 *   SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE
 *   RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 *   STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S
 *   ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE
 *   RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE
 *   MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE
 *   CHARGE PAID BY RECEIVER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 *   The following software/firmware and/or related documentation ("MediaTek
 *   Software") have been modified by MediaTek Inc. All revisions are subject to
 *   any receiver's applicable license agreements with MediaTek Inc.
 */
package com.mediatek.camera.ui.modepicker;

import android.content.Context;
import android.support.v7.widget.RecyclerView;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.ImageView;
import android.widget.TextView;

import com.mediatek.camera.R;
import com.mediatek.camera.common.debug.LogHelper;
import com.mediatek.camera.common.debug.LogUtil;

import java.util.List;

/**
 *  Mode item adapter, to store the supported mode list for recycler view.
 */
 class ModeItemAdapter extends RecyclerView.Adapter<RecyclerView.ViewHolder> {

    /**
     * Define the Mode item type.
     */
    public enum ITEM_TYPE {
        ITEM_TYPE_IMAGE,
    }
    private static final LogUtil.Tag TAG = new LogUtil.Tag(ModeItemAdapter.class.getSimpleName());
    private String mCurrentModeName = "Normal";

    /**
     * List view item click listener.
     */
    public interface  OnViewItemClickListener {
        /**
         * When a mode item is clicked, notify the event.
         * @param modeInfo Clicked mode info.
         * @return Is the mode change success.
         */
        boolean onItemCLicked(ModePickerManager.ModeInfo modeInfo);
    }

    private LayoutInflater mLayoutInflater;
    private OnViewItemClickListener mClickedListener;
    private List<ModePickerManager.ModeInfo> mModes;

    /**
     * constructor of ModeItemAdapter.
     * @param context App context.
     */
    public ModeItemAdapter(Context context, OnViewItemClickListener listener) {
        mLayoutInflater = LayoutInflater.from(context);
        mClickedListener = listener;
    }

    @Override
    public RecyclerView.ViewHolder onCreateViewHolder(ViewGroup parent, int viewType) {
        return new ModeViewHolder(mLayoutInflater.inflate(R.layout.mode_item, parent, false));
    }

    @Override
    public void onBindViewHolder(RecyclerView.ViewHolder holder, int position) {
        if (holder instanceof ModeViewHolder) {
            ((ModeViewHolder) holder).mTextView.setText(mModes.get(position).mName);
            ((ModeViewHolder) holder).mModeView.setContentDescription(
                    mModes.get(position).mName);
            LogHelper.d(TAG, "onBindViewHolder: mode name = " + mModes.get(position).mName
                         + " position = " + position);


            if (mModes.get(position).mName.equals(mCurrentModeName)) {
                if (mModes.get(position).mSelectedIcon != null) {
                    ((ModeViewHolder) holder).mImageView.setImageDrawable(
                            mModes.get(position).mSelectedIcon);
                } else {
                    ((ModeViewHolder) holder).mImageView.setImageResource(
                            R.drawable.ic_normal_mode_selected);
                }
            } else {
                if (mModes.get(position).mUnselectedIcon != null) {
                    ((ModeViewHolder) holder).mImageView.setImageDrawable(
                            mModes.get(position).mUnselectedIcon);
                } else {
                    ((ModeViewHolder) holder).mImageView.setImageResource(
                            R.drawable.ic_normal_mode_unselected);
                }
            }
            ((ModeViewHolder) holder).mTextView.setTag(mModes.get(position));
        }
    }


    @Override
    public int getItemViewType(int position) {
        return ITEM_TYPE.ITEM_TYPE_IMAGE.ordinal();
    }

    @Override
    public int getItemCount() {
        return  mModes == null ? 0 : mModes.size();
    }


    public void setModeList(List<ModePickerManager.ModeInfo> modeList) {
        mModes = modeList;
        notifyDataSetChanged();
    }

    public void updateCurrentModeName(String name) {
        mCurrentModeName = name;
    }

    /**
     *  Mode View Holder, use to show in the Recycler view.
     */
    private class ModeViewHolder extends RecyclerView.ViewHolder
            implements View.OnClickListener {
        TextView mTextView;
        ImageView mImageView;
        View mModeView;

        ModeViewHolder(View view) {
            super(view);
            mModeView = view;
            mTextView = (TextView) view.findViewById(R.id.text_view);
            mImageView = (ImageView) view.findViewById(R.id.image_view);
            view.setOnClickListener(this);
        }

        @Override
        public void onClick(View view) {
            if (mClickedListener.onItemCLicked((ModePickerManager.ModeInfo) mTextView.getTag())) {
                mCurrentModeName = ((ModePickerManager.ModeInfo) mTextView.getTag()).mName;
                LogHelper.d(TAG, "onClick: mode name = " + mCurrentModeName);
                notifyDataSetChanged();
            }
        }
    }
}

