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
 * MediaTek Inc. (C) 2016. All rights reserved.
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
package com.android.soundrecorder;

import android.content.Context;
import android.util.SparseBooleanArray;
import android.util.SparseIntArray;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.BaseAdapter;
import android.widget.CheckBox;
import android.widget.RelativeLayout;
import android.widget.TextView;

import java.util.ArrayList;
import java.util.List;

/** M: the adapter of list view in RecordingFileList. */
public class EditViewAdapter extends BaseAdapter {
    private static final int NO_CHECK_POS = -1;
    private static final String TAG = "SR/EditViewAdapter";
    private LayoutInflater mInflater;
    private List<String> mNameList;
    private List<String> mPathList;
    private List<String> mTitleList;
    private List<String> mDurationList;
    private List<Integer> mIdList;
    private List<Integer> mCheckedItemId;
    private SparseBooleanArray mCheckStates;
    private SparseIntArray mIdPos;
    private int mCurPos = -1;

    /**
     * The constructor to construct a navigation view adapter.
     *
     * @param context
     *            the context of FileManagerActivity
     * @param nameList
     *            a list of file names
     * @param pathList
     *            a list of file path
     * @param titleList
     *            a list of file title
     * @param durationList
     *            a list of file duration
     * @param idList
     *            a list of file id
     * @param curPos
     *            current click item index
     */
    public EditViewAdapter(Context context, List<String> nameList, List<String> pathList,
            List<String> titleList, List<String> durationList, List<Integer> idList, int curPos) {
        initEditViewAdapter(context, nameList, pathList, titleList, durationList, idList);
        mCurPos = curPos;
        if (NO_CHECK_POS != mCurPos) {
            mCheckStates.put(mCurPos, true);
        }
    }

    /**
     * The constructor to construct a navigation view adapter.
     *
     * @param context
     *            the context of FileManagerActivity
     * @param nameList
     *            a list of file names
     * @param pathList
     *            a list of file path
     * @param titleList
     *            a list of file title
     * @param durationList
     *            a list of file duration
     * @param idList
     *            a list of file id
     * @param posCheckedList
     *            current checked box list
     */
    public EditViewAdapter(Context context, List<String> nameList, List<String> pathList,
            List<String> titleList, List<String> durationList, List<Integer> idList,
            List<Integer> posCheckedList) {
        initEditViewAdapter(context, nameList, pathList, titleList, durationList, idList);
        if (null != posCheckedList) {
            for (int item : posCheckedList) {
                mCheckStates.put(item, true);
            }
        }
    }

    /**
     * This method sets the item's check boxes.
     *
     * @param id
     *            the id of the item
     * @param checked
     *            the checked state
     */
    protected void setCheckBox(int id, boolean checked) {
        mCheckStates.put(id, checked);
    }

    /**
     * This method return the list of current checked box.
     *
     * @return current list of checked box
     */
    protected List<Integer> getCheckedPosList() {
        int listSize = mCheckStates.size();
        for (int i = 0; i < listSize; i++) {
            boolean state = mCheckStates.valueAt(i);
            int curPos = mCheckStates.keyAt(i);
            if (state && !mCheckedItemId.contains(curPos)) {
                LogUtils.i(TAG, "<getCheckedPosList>, curPos is:" + curPos);
                mCheckedItemId.add(curPos);
            }
        }
        return mCheckedItemId;
    }

    /**
     * This method gets the number of the checked items.
     *
     * @return the number of the checked items
     */
    protected int getCheckedItemsCount() {
        return getCheckedItemsList().size();
    }

    /**
     * This method gets the list of the checked items.
     *
     * @return the list of the checked items
     */
    protected ArrayList<String> getCheckedItemsList() {
        ArrayList<String> checkedItemsList = new ArrayList<String>();
        int listSize = mCheckStates.size();
        for (int i = 0; i < listSize; i++) {
            boolean state = mCheckStates.valueAt(i);
            int curPos = mCheckStates.keyAt(i);
            if (state) {
                LogUtils.i(TAG, "<getCheckedItemsList>, curPos is:" + curPos);
                checkedItemsList.add(mPathList.get(getItemPos(curPos)));
            }
        }
        return checkedItemsList;
    }

    /**
     * This method gets the list of the grey out items.
     *
     * @return the list of the grey out items
     */
    protected SparseBooleanArray getGrayOutItems() {
        return mCheckStates;
    }

    /**
     * This method gets the count of the items in the name list.
     *
     * @return the number of the items
     */
    @Override
    public int getCount() {
        return mNameList.size();
    }

    /**
     * This method gets the name of the item at the specified position.
     *
     * @param pos
     *            the position of item
     * @return the name of the item
     */
    @Override
    public Object getItem(int pos) {
        return mNameList.get(pos);
    }

    /**
     * This method gets the item id at the specified position.
     *
     * @param pos
     *            the position of item
     * @return the id of the item
     */
    @Override
    public long getItemId(int pos) {
        long id = mIdList.get(pos);
        mIdPos.put((int) id, pos);
        return id;
    }

    /**
     * get position.
     *
     * @param id
     *            database id
     * @return position
     */
    public int getItemPos(int id) {
        return mIdPos.get(id);
    }

    /**
     * This method gets the view for each item to be displayed in the list view.
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
        EditViewTag editViewTag;
        View editListItemView = convertView;
        if (null == editListItemView) {
            editListItemView = mInflater.inflate(R.layout.edit_adapter, null);
            // construct an item tag
            editViewTag = new EditViewTag();
            editViewTag.mName =
                    (TextView) editListItemView.findViewById(R.id.record_file_name);
            editViewTag.mCheckBox =
                    (CheckBox) editListItemView.findViewById(R.id.record_file_checkbox);
            editViewTag.mTitle =
                    (TextView) editListItemView.findViewById(R.id.record_file_title);
            editViewTag.mDuration =
                    (TextView) editListItemView.findViewById(R.id.record_file_duration);

            editListItemView.setTag(editViewTag);
        } else {
            editViewTag = (EditViewTag) editListItemView.getTag();
        }

        RelativeLayout.LayoutParams params = (RelativeLayout.LayoutParams) editViewTag.mName
                .getLayoutParams();
        editViewTag.mName.setLayoutParams(params);
        String fileName = mNameList.get(pos);
        editViewTag.mName.setText(fileName);
        String title = mTitleList.get(pos);
        editViewTag.mTitle.setText(title);
        String duration = mDurationList.get(pos);
        editViewTag.mDuration.setText(duration);
        int id = (int) getItemId(pos);
        editViewTag.mCheckBox.setChecked(mCheckStates.get(id));
        return editListItemView;
    }

    /**
     * Initial the member of the EditAdapterView class.
     * @param context
     *            the context of FileManagerActivity
     * @param nameList
     *            a list of file names
     * @param pathList
     *            a list of file path
     * @param titleList
     *            a list of file title
     * @param durationList
     *            a list of file duration
     * @param idList
     *            a list of file id
     */
     public void initEditViewAdapter(Context context, List<String> nameList, List<String> pathList,
            List<String> titleList, List<String> durationList, List<Integer> idList) {
        mInflater = LayoutInflater.from(context);
        mNameList = nameList;
        mPathList = pathList;
        mTitleList = titleList;
        mDurationList = durationList;
        mIdList = idList;
        mCheckStates = new SparseBooleanArray();
        mCheckedItemId = new ArrayList<Integer>();
        mIdPos = new SparseIntArray();
        // restore the last selected file recordId and related item position
        for (int pos = 0; pos < mIdList.size(); pos++) {
            long itemId = mIdList.get(pos);
            mIdPos.put((int) itemId, pos);
            LogUtils.i(TAG, "The recordId is:" + itemId + "; pos is:" + pos);
        }
    }

    /**
     * Edit View Holder Class.
     */
    static class EditViewTag {
        private TextView mName;
        private CheckBox mCheckBox;
        private TextView mTitle;
        private TextView mDuration;
    }
}