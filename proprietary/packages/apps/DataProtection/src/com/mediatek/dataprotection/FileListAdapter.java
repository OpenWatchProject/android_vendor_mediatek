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

package com.mediatek.dataprotection;

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

import java.util.ArrayList;
import java.util.Collections;
import java.util.List;

import com.mediatek.dataprotection.utils.FileUtils;
import com.mediatek.dataprotection.utils.Log;

public class FileListAdapter extends BaseAdapter {
    private static final String TAG = "FileListAdapter";

    public static final int DEFAULT_SECONDARY_SIZE_TEXT_COLOR = 0xff414141;
    public static final int THEME_COLOR_DEFAULT = 0x7F33b5e5;
    private static final int DEFAULT_PRIMARY_TEXT_COLOR = Color.WHITE;
    private static final float CUT_ICON_ALPHA = 0.6f;
    private static final float HIDE_ICON_ALPHA = 0.3f;
    private static final float DEFAULT_ICON_ALPHA = 1f;
    public static final int MODE_NORMAL = 0;
    public static final int MODE_EDIT = 1;

    private static final int THUMBNAIL_BUFFER_SIZE = 128 * 1024;

    private Context mContext;
    private final Resources mResources;
    private final LayoutInflater mInflater;
    private final List<FileInfo> mFileInfoList;

    /*
     * private final List<FileInfo> mFileInfoList; private final FileInfoManager
     * mFileInfoManager;
     */

    private int mMode = AddFileToLockActivity.MODE_VIEW_FILE;

    // FileManagerService mService = null;

    /**
     * The constructor to construct a FileInfoAdapter.
     *
     * @param context
     *            the context of FileManagerActivity
     * @param fileManagerService
     *            the service binded with FileManagerActivity
     * @param fileInfoManager
     *            a instance of FileInfoManager, which manages all files.
     */
    /*
     * public FileListAdapter(Context context, FileManagerService
     * fileManagerService, FileInfoManager fileInfoManager) { mContext =
     * context; mResources = context.getResources(); mInflater =
     * LayoutInflater.from(context); mService = fileManagerService;
     * mFileInfoManager = fileInfoManager; mFileInfoList =
     * fileInfoManager.getShowFileList(); }
     */

    public FileListAdapter(Context context, List<FileInfo> files) {
        mFileInfoList = files;
        mContext = context;
        mResources = mContext.getResources();
        mInflater = LayoutInflater.from(mContext);
    }

    /**
     * This method gets index of certain fileInfo(item) in fileInfoList
     *
     * @param fileInfo
     *            the fileInfo which wants to be located.
     * @return the index of the item in the listView.
     */
    public int getPosition(FileInfo fileInfo) {
        return mFileInfoList.indexOf(fileInfo);
    }

    /**
     * This method sets the item's check boxes
     *
     * @param id
     *            the id of the item
     * @param checked
     *            the checked state
     */
    public void setChecked(int id, boolean checked) {
        FileInfo checkInfo = mFileInfoList.get(id);
        if (checkInfo != null) {
            checkInfo.setChecked(checked);
        }
    }

    /**
     * This method sets all items' check boxes
     *
     * @param checked
     *            the checked state
     */
    public void setAllItemChecked(boolean checked) {
        for (FileInfo info : mFileInfoList) {
            if (!info.isDirectory()) {
                info.setChecked(checked);
            } else {
                info.setChecked(false);
            }
        }
        notifyDataSetChanged();
    }

    /**
     * This method gets the number of the checked items
     *
     * @return the number of the checked items
     */
    public int getCheckedItemsCount() {
        int count = 0;
        for (FileInfo fileInfo : mFileInfoList) {
            if (fileInfo.isChecked()) {
                count++;
            }
        }
        return count;
    }

    /**
     * This method gets the list of the checked items
     *
     * @return the list of the checked items
     */
    public List<FileInfo> getCheckedFileInfoItemsList() {
        List<FileInfo> fileInfoCheckedList = new ArrayList<FileInfo>();
        for (FileInfo fileInfo : mFileInfoList) {
            if (fileInfo.isChecked()) {
                fileInfoCheckedList.add(fileInfo);
            }
        }
        return fileInfoCheckedList;
    }

    /**
     * This method gets the list of the checked items
     *
     * @return the list of the checked items
     */
    public List<FileInfo> getCheckedFileInfoItemsListAndRemove() {
        List<FileInfo> fileInfoCheckedList = new ArrayList<FileInfo>();
        /*
         * for (int i = 0; i < mFileInfoList.size();i++) { FileInfo iFile =
         * mFileInfoList.get(i); if(iFile.isChecked()) {
         * fileInfoCheckedList.add(iFile); } }
         */
        for (FileInfo fileInfo : mFileInfoList) {
            if (fileInfo.isChecked()) {
                fileInfoCheckedList.add(fileInfo);
                fileInfo.setChecked(false);
                // mFileInfoList.remove(fileInfo);
            }
        }
        mFileInfoList.removeAll(fileInfoCheckedList);
        return fileInfoCheckedList;
    }

    /**
     * This method gets the first item in the list of the checked items
     *
     * @return the first item in the list of the checked items
     */
    public FileInfo getFirstCheckedFileInfoItem() {
        for (FileInfo fileInfo : mFileInfoList) {
            if (fileInfo.isChecked()) {
                return fileInfo;
            }
        }
        return null;
    }

    public void addFileInfo(List<FileInfo> files) {
        mFileInfoList.addAll(files);
        //notifyDataSetChanged();
    }

    public void reSort() {
        Collections
        .sort(mFileInfoList, FileInfoComparator
                .getInstance(FileInfoComparator.SORT_BY_TYPE));
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
     * @param pos
     *            the position of item
     * @return the name of the item
     */
    @Override
    public FileInfo getItem(int pos) {
        return mFileInfoList.get(pos);
    }

    /**
     * This method gets the item id at the specified position
     *
     * @param pos
     *            the position of item
     * @return the id of the item
     */
    @Override
    public long getItemId(int pos) {
        return pos;
    }

    /**
     * This method change all checked items to be unchecked state
     */
    private void clearChecked() {
        for (FileInfo fileInfo : mFileInfoList) {
            if (fileInfo.isChecked()) {
                fileInfo.setChecked(false);
            }
        }
    }

    /**
     * This method changes the display mode of adapter between MODE_NORMAL,
     * MODE_EDIT, and MODE_SEARCH
     *
     * @param mode
     *            the mode which will be changed to be.
     */
    public void changeMode(int mode) {
        // LogUtils.d(TAG, "changeMode, mode = " + mode);
        switch (mode) {
        case AddFileToLockActivity.MODE_VIEW_FILE:
            clearChecked();
            break;
        default:
            break;
        }
        mMode = mode;
        notifyDataSetChanged();
    }

    public boolean isAllItemDirectory() {
        boolean res = true;
        for (FileInfo fileInfo : mFileInfoList) {
            if (!fileInfo.isDirectory()) {
                res = false;
                break;
            }
        }
        if (mFileInfoList.size() == 0) {
            res = false;
        }
        return res;
    }

    /**
     * This method gets current display mode of the adapter.
     *
     * @return current display mode of adapter
     */
    public int getMode() {
        return mMode;
    }

    /**
     * This method checks that current mode equals to certain mode, or not.
     *
     * @param mode
     *            the display mode of adapter
     * @return true for equal, and false for not equal
     */
    public boolean isMode(int mode) {
        return mMode == mode;
    }

    /**
     * This method gets the view for each item to be displayed in the list view
     *
     * @param pos
     *            the position of the item
     * @param convertView
     *            the view to be shown
     * @param parent
     *            the parent view
     * @return the view to be shown
     */
    @Override
    public View getView(int pos, View convertView, ViewGroup parent) {
        // LogUtils.d(TAG, "getView, pos = " + pos + ",mMode = " + mMode);
        Log.d(TAG, "getView...");
        FileViewHolder viewHolder;
        View view = convertView;

        if (view == null) {
            view = mInflater.inflate(R.layout.select_file_list_item,
                    null);
            viewHolder = new FileViewHolder(
                    (TextView) view.findViewById(R.id.list_item_name),
                    (ImageView) view.findViewById(R.id.list_item_icon),
                    (TextView) view.findViewById(R.id.list_item_size));
            view.setTag(viewHolder);
            // navListItemView.setMinimumHeight(mListItemMinHeight);
        } else {
            viewHolder = (FileViewHolder) view.getTag();
        }

        FileInfo currentItem = mFileInfoList.get(pos);
        String showName = currentItem.getShowName();
        viewHolder.mName.setText(showName);
        if (currentItem.isDirectory()) {
            viewHolder.mSize.setVisibility(View.GONE);
        } else {
            viewHolder.mSize.setVisibility(View.VISIBLE);
            viewHolder.mSize.setText(FileUtils.sizeToString(currentItem.getFileSize()));
        }

        // viewHolder.mSize.setTextColor(DEFAULT_SECONDARY_SIZE_TEXT_COLOR);
        view.setBackgroundColor(Color.TRANSPARENT);

        if (mMode == AddFileToLockActivity.MODE_SELECT_FILE
                && currentItem.isDirectory()) {
            view.setEnabled(false);
            viewHolder.mName.setTextColor(R.color.grey_500);
        } else {
            view.setEnabled(true);
            viewHolder.mName.setTextColor(0xff101010); // default
        }

        Log.s(TAG, "getView..." + currentItem.getShowName());
        if (currentItem.isChecked()) {
            view.setBackgroundColor(THEME_COLOR_DEFAULT);
        }

        // setIcon(viewHolder, currentItem,parent.getLayoutDirection());
        // viewHolder.mIcon.setId(R.drawable.phone_storage);
        // viewHolder.mIcon.setImageBitmap(BitmapFactory.decodeResource(mResources,
        // R.drawable.phone_storage));
        /*
         * Bitmap icon = null; if (currentItem.isDirectory()) { icon =
         * BitmapFactory.decodeResource(mResources, R.drawable.fm_folder); }
         * else { icon = BitmapFactory .decodeResource(mResources,
         * R.drawable.fm_video); }
         */
        // viewHolder.mIcon.setImageBitmap(icon);
        setIcon(viewHolder, currentItem, 0);

        return view;
    }

    private void setIcon(FileViewHolder viewHolder, FileInfo fileInfo,
            int viewDirection) {
        String path = fileInfo.getFileAbsolutePath();

        if (MountPointManager.getInstance().isInternalMountPath(path)) {
            // return getDefaultIcon(R.drawable.phone_storage);
            viewHolder.mIcon.setImageDrawable(mResources
                    .getDrawable(R.drawable.ic_phone_storage));
        } else if (MountPointManager.getInstance().isExternalMountPath(path)) {
            viewHolder.mIcon.setImageDrawable(mResources
                    .getDrawable(R.drawable.ic_sdcard));
        } else {
            viewHolder.mIcon.setImageBitmap(getIcon(fileInfo));
            /*
             * if (fileInfo.isDirectory()) {
             * viewHolder.mIcon.setImageDrawable(mResources
             * .getDrawable(R.drawable.fm_folder)); } else { if
             * (fileInfo.getFileType() == FileInfo.IMAGE_FILE ||
             * fileInfo.getFileType() == FileInfo.VIDEO_FILE) { if
             * (viewHolder.mThumbRequest == null) { viewHolder.mThumbRequest =
             * new ThumbnailAsyncTask( fileInfo.getFileAbsolutePath(),
             * viewHolder.mIcon, viewHolder.mIcon, new Point( 96, 96), null);
             * viewHolder.mIcon.setTag(viewHolder.mThumbRequest);
             * viewHolder.mThumbRequest.execute(); } } else if
             * (fileInfo.getFileType() == FileInfo.AUDIO_FILE) {
             * viewHolder.mIcon.setImageBitmap(BitmapFactory
             * .decodeResource(mResources, R.drawable.fm_audio)); } else {
             * viewHolder.mIcon.setImageResource(R.drawable.fm_unknown); } }
             */
        }
    }

    private Bitmap getIcon(FileInfo fileInfo) {
        Bitmap icon = null;
        if (fileInfo != null) {
            if (fileInfo.isDirectory()) {
                icon = BitmapFactory.decodeResource(mResources,
                        R.drawable.fm_folder);
            } else {
/*                String mimeType = FileUtils.getFileMimeType(mContext,
                        Uri.fromFile(fileInfo.getFile()));*/
                String mimeType = fileInfo.getMimeType();
                if (mimeType != null) {
                    if (FileUtils.isAudio(mimeType)) {
                        icon = BitmapFactory.decodeResource(mResources,
                                R.drawable.fm_audio);
                    } else if (FileUtils.isImage(mimeType)) {
                        icon = BitmapFactory.decodeResource(mResources,
                                R.drawable.fm_picture);
                    } else if (FileUtils.isVideo(mimeType)) {
                        icon = BitmapFactory.decodeResource(mResources,
                                R.drawable.fm_video);
                    } else {
                        icon = BitmapFactory.decodeResource(mResources,
                                R.drawable.fm_unknown);
                    }
                } else {
                    icon = BitmapFactory.decodeResource(mResources,
                            R.drawable.fm_unknown);
                }
            }
        }
        return icon;
    }

    static class FileViewHolder {
        protected TextView mName;
        protected ImageView mIcon;
        protected TextView mSize;

        /**
         * The constructor to construct an edit view tag
         *
         * @param name
         *            the name view of the item
         * @param size
         *            the size view of the item
         * @param icon
         *            the icon view of the item
         * @param box
         *            the check box view of the item
         */
        public FileViewHolder(TextView name, ImageView icon, TextView size) {
            this.mName = name;
            this.mIcon = icon;
            this.mSize = size;
        }
    }

}
