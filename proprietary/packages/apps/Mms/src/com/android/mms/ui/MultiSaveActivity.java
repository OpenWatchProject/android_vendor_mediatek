/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2010. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
 * AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek Software")
 * have been modified by MediaTek Inc. All revisions are subject to any receiver's
 * applicable license agreements with MediaTek Inc.
 */

/*
 * Copyright (C) 2008 Esmertec AG.
 * Copyright (C) 2008 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package com.android.mms.ui;

import android.Manifest;
import android.app.ActionBar;
import android.app.Activity;
import android.content.ContentResolver;
import android.content.Context;
import android.content.Intent;
import android.net.Uri;
import android.os.Bundle;
import android.util.Log;
import android.view.ActionMode;
import android.view.LayoutInflater;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;
import android.view.ViewGroup;
import android.widget.AdapterView;
import android.widget.Button;
import android.widget.ListView;
import android.widget.PopupMenu;

import com.android.mms.model.SlideshowModel;
import com.android.mms.R;
import com.android.mms.util.MmsContentType;
import com.android.mms.util.MmsLog;
import com.google.android.mms.MmsException;
import com.google.android.mms.pdu.PduBody;
import com.google.android.mms.pdu.PduPart;

import com.mediatek.mms.callback.IMultiSaveActivityCallback;
import com.mediatek.mms.ext.IOpMultiSaveActivityExt;
import com.mediatek.mms.model.FileAttachmentModel;
import com.mediatek.mms.model.FileModel;
import com.mediatek.mms.util.PermissionCheckUtil;
import com.mediatek.opmsg.util.OpMessageUtils;

import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.util.ArrayList;

/** M:
 * This activity provides a list view of existing conversations.
 */
public class MultiSaveActivity extends Activity implements IMultiSaveActivityCallback {
    private static final String TAG = "Mms/MultiSaveActivity";

    private MultiSaveListAdapter mListAdapter;

    private ListView mMultiSaveList;

    private ContentResolver mContentResolver;

    private boolean needQuit = false;

    private SelectActionMode mSelectActionMode;
    private ActionMode mSelectMode;
    private Button mActionBarSelect;

    // / M: new feature, MultiSaveActivity redesign UI
    private MenuItem mSelectionItem;

    /// M: add for attachment enhance
    private long mSMode = -1;

    /// M: add for runtime permission request code
    private static final int PERMISSIONS_REQUEST_STORAGE = 1;

    public IOpMultiSaveActivityExt mOpMultiSaveActivityExt = null;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        if (PermissionCheckUtil.requestRequiredPermissions(this)) {
            return;
        }

        setTitle(R.string.save);
        mContentResolver = getContentResolver();
        setContentView(R.layout.multi_save);
        mMultiSaveList = (ListView) findViewById(R.id.item_list);
        mMultiSaveList.setOnItemClickListener(new AdapterView.OnItemClickListener() {
            public void onItemClick(AdapterView<?> parent, View view, int position, long id) {
                if (view != null) {
                    ((MultiSaveListItem) view).clickListItem();
                    mListAdapter.changeSelectedState(position);
                    updateActionBarText();
                }
            }
        });

        Intent i = getIntent();
        mOpMultiSaveActivityExt = OpMessageUtils.getOpMessagePlugin()
                .getOpMultiSaveActivityExt();
        mSMode = mOpMultiSaveActivityExt.onCreate(i);

        long msgId = -1;
        if (i != null && i.hasExtra("msgid")) {
            msgId = i.getLongExtra("msgid", -1);
        }

        initListAdapter(msgId);
        initActivityState(savedInstanceState);
        setUpActionBar();
    }

    private void initActivityState(Bundle savedInstanceState) {
        if (savedInstanceState != null) {
            boolean selectedAll = savedInstanceState.getBoolean("is_all_selected");
            if (selectedAll) {
                mListAdapter.setItemsValue(true, null);
                return;
            }

            int[] selectedItems = savedInstanceState.getIntArray("select_list");
            if (selectedItems != null) {
                mListAdapter.setItemsValue(true, selectedItems);
            }

        } else {
            MmsLog.i(TAG, "initActivityState, fresh start select all");
            mListAdapter.setItemsValue(true, null);
            markCheckedState(true);
        }
    }

    private void initListAdapter(long msgId) {
        PduBody body = PduBodyCache.getPduBody(MultiSaveActivity.this, msgId);

        SlideshowModel mSlideshow = null;

        if (body == null) {
            MmsLog.e(TAG, "initListAdapter, oops, getPduBody returns null");
            return;
        }
        int partNum = body.getPartsNum();

        ArrayList<MultiSaveListItemData> attachments =
            new ArrayList<MultiSaveListItemData>(partNum);

        try {
            mSlideshow = SlideshowModel.createFromPduBody(this, body);
        } catch (MmsException e) {
            MmsLog.e(TAG, "Create from pdubody exception!");
            return;
        }

        ArrayList<FileAttachmentModel> attachmentList = mSlideshow.getAttachFiles();
        for (int i = 0; i < partNum; i++) {
            PduPart part = body.getPart(i);
            byte[] cl = part.getContentLocation();
            byte[] name = part.getName();
            byte[] ci = part.getContentId();
            byte[] fn = part.getFilename();
            String filename = null;
            String src = null;
            if (cl != null) {
                filename = new String(cl);
            } else if (name != null) {
                filename = new String(name);
            } else if (ci != null) {
                filename = new String(ci);
            } else if (fn != null) {
                filename = new String(fn);
            } else {
                MmsLog.w(TAG, "initListAdapter: filename = null,continue");
                continue;
            }

            src = filename;

            if (part.getDataUri() != null) {
                MmsLog.dpi(TAG, "part Uri = " + part.getDataUri().toString());
            } else {
                MmsLog.w(TAG, "PartUri = null");
                continue;
            }

            final String type = MessageUtils
                    .getContentType(new String(part.getContentType()), src);

            /// M: OP01
            if (!mOpMultiSaveActivityExt.initListAdapter(this, src, mSMode, type, attachments,
                    part, msgId, attachmentList, this, new FileModel())) {
                part.setContentType(type.getBytes());
                if (MmsContentType.isImageType(type)
                        || MmsContentType.isVideoType(type)
                        || "application/ogg".equalsIgnoreCase(type)
                        || MmsContentType.isAudioType(type)
                        || FileAttachmentModel.isSupportedFile(part)
                        /// M: fix bug ALPS00446644, support dcf (0ct-stream) file to save
                        || (src != null && src.toLowerCase().endsWith(".dcf"))) {
                    attachments.add(new MultiSaveListItemData(this, part, msgId));
                }
            }
        }
        attachments.trimToSize();
        mListAdapter = new MultiSaveListAdapter(this, attachments);
        mMultiSaveList.setAdapter(mListAdapter);
    }


    private void setUpActionBar() {
        MmsLog.d(TAG, "setUpActionBar begin");
        mSelectActionMode = new SelectActionMode();
        mSelectMode = startActionMode(mSelectActionMode);
        ActionBar actionBar = getActionBar();
        actionBar.setDisplayShowHomeEnabled(false);
        actionBar.setDisplayHomeAsUpEnabled(false);
        actionBar.setDisplayShowTitleEnabled(false);
    }

    private void selectAll() {
        if (mListAdapter != null) {
            markCheckedState(true);
            mListAdapter.setItemsValue(true, null);
            updateActionBarText();
        }
    }

    private void cancelSelectAll() {
        if (mListAdapter != null) {
            markCheckedState(false);
            mListAdapter.setItemsValue(false, null);
            updateActionBarText();
        }
    }

    private void save() {
        if (mListAdapter.getSelectedNumber() > 0) {
            boolean succeeded = false;
            succeeded = copyMedia();
            Intent i = new Intent();
            i.putExtra("multi_save_result", succeeded);
            setResult(RESULT_OK, i);
            finish();

            /// M: OP01
            mOpMultiSaveActivityExt.save(this, mSMode, succeeded,
                    getString(R.string.copy_to_sdcard_success),
                    getString(R.string.copy_to_sdcard_fail));
        }
    }

    private void updateActionBarText() {
        if (mListAdapter != null && mActionBarSelect != null) {
            mActionBarSelect.setText(getResources().getQuantityString(
                    R.plurals.message_view_selected_message_count,
                    mListAdapter.getSelectedNumber(), mListAdapter.getSelectedNumber()));
        }

        if (mSelectionItem != null && mListAdapter != null) {
            if (mListAdapter.isAllSelected()) {
                mSelectionItem.setChecked(true);
                mSelectionItem.setTitle(R.string.unselect_all);
            } else {
                mSelectionItem.setChecked(false);
                mSelectionItem.setTitle(R.string.select_all);
            }
        }
    }

    @Override
    public void onSaveInstanceState(Bundle outState) {
        MmsLog.d(TAG, "onSaveInstanceState, with bundle " + outState);
        super.onSaveInstanceState(outState);
        if (mListAdapter != null) {
            if (mListAdapter.isAllSelected()) {
                outState.putBoolean("is_all_selected", true);
            } else if (mListAdapter.getSelectedNumber() == 0) {
                return;
            } else {
                ArrayList<MultiSaveListItemData> list = mListAdapter.getItemList();
                int[] checkedArray = new int[list.size()];
                for (int i = 0; i < list.size(); i++) {
                    if (list.get(i).isSelected()) {
                        checkedArray[i] = i;
                    } else {
                        checkedArray[i] = -1;
                    }
                }
                outState.putIntArray("select_list", checkedArray);
            }
        }
    }

    private void markCheckedState(boolean checkedState) {
        int count = mMultiSaveList.getChildCount();
        MmsLog.d(TAG, "markCheckState count is " + count + ", state is " + checkedState);
        MultiSaveListItem item = null;
        for (int i = 0; i < count; i++) {
            item = (MultiSaveListItem) mMultiSaveList.getChildAt(i);
            item.selectItem(checkedState);
        }
    }

    private class SelectActionMode implements ActionMode.Callback {

        private PopupMenu mPopup = null;

        @Override
        public boolean onCreateActionMode(final ActionMode mode, Menu menu) {
            ViewGroup v = (ViewGroup) LayoutInflater.from(MultiSaveActivity.this).inflate(
                    R.layout.chat_select_action_bar, null);
            mode.setCustomView(v);
            mPopup = new PopupMenu(MultiSaveActivity.this, v);
            mPopup.getMenuInflater().inflate(R.menu.selection, mPopup.getMenu());
            mSelectionItem = mPopup.getMenu().findItem(R.id.action_select_all);
            mActionBarSelect = ((Button) v.findViewById(R.id.bt_chat_select));

            mActionBarSelect.setOnClickListener(new View.OnClickListener() {
                @Override
                public void onClick(View v) {
                    mPopup.dismiss();

                    mPopup.setOnMenuItemClickListener(new PopupMenu.OnMenuItemClickListener() {
                        public boolean onMenuItemClick(MenuItem item) {
                            if (mListAdapter.isAllSelected()) {
                                cancelSelectAll();
                            } else {
                                selectAll();
                            }
                            return false;
                        }
                    });

                    mPopup.show();
                }
            });
            updateActionBarText();
            getMenuInflater().inflate(R.menu.multi_save_menu, menu);
            return true;
        }

        public PopupMenu getPopupMenu() {
            return mPopup;
        }
        @Override
        public boolean onPrepareActionMode(ActionMode mode, Menu menu) {
            MmsLog.d(TAG, "onPrepareActionMode() begin");
            return true;
        }

        @Override
        public boolean onActionItemClicked(ActionMode mode, MenuItem item) {
            switch(item.getItemId()) {
            case R.id.selection_cancel:
                finish();
                break;
            case R.id.selection_done:
              //  String[] storagePermission = new String[]{
              //          Manifest.permission.WRITE_EXTERNAL_STORAGE};
              //  if (PermissionCheckUtil.checkPermissions(
              //          MultiSaveActivity.this, storagePermission)) {
                    save();
             //   } else {
             //       MultiSaveActivity.this.requestPermissions(
             //               storagePermission, PERMISSIONS_REQUEST_STORAGE);
             //   }
                break;
            default:
                break;
            }
            return true;
        }

        @Override
        public void onDestroyActionMode(ActionMode mode) {
            MmsLog.d(TAG, "onDestroyActionMode() begin");
            finish();
        }
    }

    /**
     * Copies media from an Mms to the "download" directory on the SD card
     *
     * @param msgId
     */
    private boolean copyMedia() {
        boolean result = true;

        ArrayList<MultiSaveListItemData> list = mListAdapter.getItemList();
        int size = list.size();
        for (int i = 0; i < size; i++) {
            if (!list.get(i).isSelected()) {
                continue;
            }
            PduPart part = list.get(i).getPduPart();
            final String filename = list.get(i).getName();
            final String type = new String(part.getContentType());
            if (MmsContentType.isImageType(type)
                    || MmsContentType.isVideoType(type)
                    || MmsContentType.isAudioType(type)
                    || "application/ogg".equalsIgnoreCase(type)
                    || FileAttachmentModel.isSupportedFile(part)
                    /// M: fix bug ALPS00446644, support dcf (0ct-stream) file to save
                    || (filename != null && filename.toLowerCase().endsWith(".dcf"))) {
                // all parts have to be successful for a valid result.
                result &= copyPart(part, filename);
            } else if (type.equals(MmsContentType.TEXT_PLAIN)
                    || type.equals(MmsContentType.TEXT_HTML)) {
                // for text attachment or html attachment
                result &= copyPartNoUri(part, filename);
            }
        }
        return result;
    }

    /// M: add for attachment enhance, save text/plain, text/html attachment files
    private boolean copyPartNoUri(PduPart part, String filename) {
        FileOutputStream fout = null;
        try {
            File file = MessageUtils.getStorageFile(filename, this);
            if (file == null) {
                MmsLog.e(TAG, "default file is null");
                return false;
            }
            fout = new FileOutputStream(file);
            fout.write(part.getData(), 0, part.getData().length);
        } catch (IOException e) {
            MmsLog.e(TAG, "IOException caught while opening or reading stream", e);
            return false;
        } finally {
            if (null != fout) {
                try {
                    fout.close();
                } catch (IOException e) {
                    // Ignore
                    MmsLog.e(TAG, "IOException caught while closing stream", e);
                    return false;
                }
            }
        }
        return true;
    }

    private boolean copyPart(PduPart part, String filename) {
        Uri uri = part.getDataUri();
        MmsLog.ipi(TAG, "copyPart, copy part into sdcard uri " + uri);

        InputStream input = null;
        FileOutputStream fout = null;
        try {
            input = mContentResolver.openInputStream(uri);
            if (input instanceof FileInputStream) {
                FileInputStream fin = (FileInputStream) input;
                // Depending on the location, there may be an extension already on the name or not
                File file = MessageUtils.getStorageFile(filename, this);
                if (file == null) {
                    return false;
                }
                fout = new FileOutputStream(file);
                byte[] buffer = new byte[8000];
                int size = 0;
                while ((size = fin.read(buffer)) != -1) {
                    fout.write(buffer, 0, size);
                }

                // Notify other applications listening to scanner events
                // that a media file has been added to the sd card
                sendBroadcast(new Intent(Intent.ACTION_MEDIA_SCANNER_SCAN_FILE,
                        Uri.fromFile(file)));
            }
        } catch (IOException e) {
            MmsLog.e(TAG, "IOException caught while opening or reading stream", e);
            return false;
        } finally {
            if (null != input) {
                try {
                    input.close();
                } catch (IOException e) {
                    MmsLog.e(TAG, "IOException caught while closing stream", e);
                    return false;
                }
            }
            if (null != fout) {
                try {
                    fout.close();
                } catch (IOException e) {
                    MmsLog.e(TAG, "IOException caught while closing stream", e);
                    return false;
                }
            }
        }
        return true;
    }

    @Override
    public void addMultiListItemData(Context context,
            ArrayList attaches, PduPart part, long msgId) {
        attaches.add(new MultiSaveListItemData(context, part, msgId));
    }

    @Override
    public void onRequestPermissionsResult(
            final int requestCode, final String permissions[], final int[] grantResults) {
        Log.d(TAG, "onRequestPermissionsResult");

        switch (requestCode) {
            case PERMISSIONS_REQUEST_STORAGE:
             //   if (PermissionCheckUtil.hasPermission(
             //           this, Manifest.permission.WRITE_EXTERNAL_STORAGE)) {
                    save();
            //    }
                break;
        }
    }
}
