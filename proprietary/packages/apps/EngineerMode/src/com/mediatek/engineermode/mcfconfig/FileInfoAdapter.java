/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein is
 * confidential and proprietary to MediaTek Inc. and/or its licensors. Without
 * the prior written permission of MediaTek inc. and/or its licensors, any
 * reproduction, modification, use or disclosure of MediaTek Software, and
 * information contained herein, in whole or in part, shall be strictly
 * prohibited.
 *
 * MediaTek Inc. (C) 2010. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
 * ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 * NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH
 * RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 * INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES
 * TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
 * RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
 * OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN MEDIATEK
 * SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE
 * RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S
 * ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE
 * RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE
 * MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE
 * CHARGE PAID BY RECEIVER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek
 * Software") have been modified by MediaTek Inc. All revisions are subject to
 * any receiver's applicable license agreements with MediaTek Inc.
 */

package com.mediatek.engineermode.mcfconfig;

import java.util.List;

import android.content.Context;
import android.content.res.Resources;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.graphics.Color;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.BaseAdapter;
import android.widget.ImageView;
import android.widget.TextView;

import com.mediatek.engineermode.Elog;
import com.mediatek.engineermode.R;


public class FileInfoAdapter extends BaseAdapter {
    private static final String TAG = "McfConfig/FileInfoAdapter";

    private static final float HIDE_ICON_ALPHA = 0.3f;
    private static final float DEFAULT_ICON_ALPHA = 1f;

    private final Resources mResources;
    private Bitmap icon = null;
    private final LayoutInflater mInflater;
    private final List<FileInfo> mFileInfoList;

    public FileInfoAdapter(Context context, List<FileInfo> fileList) {
        mResources = context.getResources();
        mInflater = LayoutInflater.from(context);
        mFileInfoList = fileList;
    }

    /**
     * This method gets index of certain fileInfo(item) in fileInfoList
     *
     * @param fileInfo the fileInfo which wants to be located.
     * @return the index of the item in the listView.
     */
    public int getPosition(FileInfo fileInfo) {
        return mFileInfoList.indexOf(fileInfo);
    }

    /**
     * This method gets the count of the items in the name list
     *
     * @return the number of the items
     */
    @Override
    public int getCount() {
        return mFileInfoList.size();
    }

    /**
     * This method gets the name of the item at the specified position
     *
     * @param pos the position of item
     * @return the name of the item
     */
    @Override
    public FileInfo getItem(int pos) {
        return mFileInfoList.get(pos);
    }


    @Override
    public long getItemId(int pos) {
        return pos;
    }

    /**
     * This method gets the view for each item to be displayed in the list view
     *
     * @param pos the position of the item
     * @param convertView the view to be shown
     * @param parent the parent view
     * @return the view to be shown
     */
    @Override
    public View getView(int pos, View convertView, ViewGroup parent) {
        Elog.d(TAG, "getView, pos = " + pos);
        FileViewHolder viewHolder;
        View view = convertView;

        if (view == null) {
            view = mInflater.inflate(R.layout.mcf_adapter_fileinfos, null);
            viewHolder = new FileViewHolder((TextView) view.findViewById(R.id.edit_adapter_name),
                    (TextView) view.findViewById(R.id.edit_adapter_size), (ImageView) view
                            .findViewById(R.id.edit_adapter_img));
            view.setTag(viewHolder);
        } else {
            viewHolder = (FileViewHolder) view.getTag();
        }

        FileInfo currentItem = mFileInfoList.get(pos);
        viewHolder.mName.setText(currentItem.getFileName());
        view.setBackgroundColor(Color.TRANSPARENT);
        setSizeText(viewHolder.mSize, currentItem);

        setIcon(viewHolder, currentItem);

        return view;
    }

    private void setSizeText(TextView textView, FileInfo fileInfo) {
        StringBuilder sb = new StringBuilder();
        sb.append(mResources.getString(R.string.size)).append(" ");
        sb.append(fileInfo.getFileSizeStr());
        textView.setText(sb.toString());
        textView.setVisibility(View.VISIBLE);
    }

    private void setIcon(FileViewHolder viewHolder, FileInfo fileInfo) {
        Bitmap icon = getIcon();

        viewHolder.mIcon.setImageBitmap(icon);
        viewHolder.mIcon.setAlpha(DEFAULT_ICON_ALPHA);
        if (fileInfo.isHideFile()) {
            viewHolder.mIcon.setAlpha(HIDE_ICON_ALPHA);
        }
    }

    public Bitmap getIcon() {

        int iconId = R.drawable.fm_unknown;
        if (icon == null) {
            icon = BitmapFactory.decodeResource(mResources, iconId);
        }

        return icon;
    }


    static class FileViewHolder {
        protected TextView mName;
        protected TextView mSize;
        protected ImageView mIcon;

        /**
         * The constructor to construct an edit view tag
         *
         * @param name the name view of the item
         * @param size the size view of the item
         * @param icon the icon view of the item
         */
        public FileViewHolder(TextView name, TextView size, ImageView icon) {
            this.mName = name;
            this.mSize = size;
            this.mIcon = icon;
        }
    }
}
