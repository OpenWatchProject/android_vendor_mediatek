/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2012. All rights reserved.
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

package com.mediatek.mms.plugin;

import android.app.Activity;
import android.app.AlertDialog;
import android.content.ContentUris;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.graphics.drawable.Drawable;
import android.net.Uri;
import android.os.Bundle;
import android.os.Environment;
import android.os.Handler;
import android.os.Message;
import android.provider.Browser;
import android.provider.Telephony.Mms;
import android.util.Log;
import android.view.View;
import android.widget.ImageView;
import android.widget.TextView;
import android.widget.Toast;

import com.google.android.mms.pdu.PduBody;
import com.google.android.mms.pdu.PduPart;

import com.mediatek.mms.callback.IFileAttachmentModelCallback;
import com.mediatek.mms.callback.IMessageListItemCallback;
import com.mediatek.mms.callback.IMessageUtilsCallback;
import com.mediatek.mms.callback.ISlideshowModelCallback;
import com.mediatek.mms.ext.DefaultOpMessageListItemExt;
import com.mediatek.mms.ext.IOpFileAttachmentModelExt;
import com.mediatek.mms.plugin.R;

import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.util.ArrayList;

/**
 * Op01MessageListItemExt.
 *
 */
public class Op01MessageListItemExt extends DefaultOpMessageListItemExt {
    private static final String TAG = "Mms/Op01MmsMessageListItemExt";

    /// M: New plugin API @{
    private static final int MSG_RETRIEVE_FAILURE_DEVICE_MEMORY_FULL = 2;
    private Context mContext = null;
    private Context mHostContext = null;
    private ToastHandler mToastHandler = null;
    private IMessageListItemCallback mMessageListItem;

    /**
     * Construction.
     * @param context Context
     */
    public Op01MessageListItemExt(Context context) {
        super(context);
        mContext = context;
    }

    @Override
    public void init(IMessageListItemCallback messageListItem, Context context) {
        mMessageListItem = messageListItem;
        mHostContext = context;
    }

    @Override
    public boolean onDownloadButtonClick(Activity activity, boolean storageFull) {
        Log.d(TAG, "onDownloadButtonClick showStorageFullToast");
        if (!storageFull) {
            return false;
        }

        if (null == mToastHandler) {
            mToastHandler = new ToastHandler();
        }
        mToastHandler.sendEmptyMessage(MSG_RETRIEVE_FAILURE_DEVICE_MEMORY_FULL);
        return true;
    }

    /**
     * ToastHandler.
     *
     */
    public final class ToastHandler extends Handler {
        /**
         * Construction.
         */
        public ToastHandler() {
            super();
        }

        @Override
        public void handleMessage(Message msg) {
            Log.d(TAG, "Toast Handler handleMessage :" + msg);

            switch (msg.what) {
                case MSG_RETRIEVE_FAILURE_DEVICE_MEMORY_FULL: {
                    CharSequence string = mContext.getResources()
                                .getText(R.string.download_failed_due_to_full_memory);
                    Log.d(TAG, "string =" + string);
                    Toast.makeText(mContext, string, Toast.LENGTH_LONG).show();
                    break;
                }
                default:
                    break;
            }
        }
    }
    /// @}

    @Override
    public boolean showOrHideFileAttachmentView(ArrayList files) {
        Log.d(TAG, "bindCommonMessage");
        mMessageListItem.showFileAttachmentViewCallback(files);
        if (files == null || files.size() < 1) {
            Log.e(TAG, "hideFileAttachmentViewIfNeeded ");
            mMessageListItem.hideFileAttachmentViewIfNeededCallback();
        }
        return true;
    }

    /* q0 migration, phase out attach enhance*/
    /*
    @Override
    public void showFileAttachmentView(View fileAttachmentView,
            TextView name2, TextView tvName, ImageView ivThumb,
            TextView tvSize, ArrayList attachFiles, final Intent intent,
            final long msgId, final IFileAttachmentModelCallback attach,
            final IOpFileAttachmentModelExt opFileAttachmentModelExt) {
        final int filesize = attachFiles.size();
        if (filesize > 1 ||
                (filesize == 1 && !attach.isVCardCallback() && !attach.isVCalendarCallback())) {
            fileAttachmentView.setOnClickListener(new View.OnClickListener() {
                public void onClick(View v) {
                    AlertDialog.Builder b = new AlertDialog.Builder(mHostContext);
                    b.setTitle(Op01MessagePluginExt.sMessageUtilsCallback
                            .getString(IMessageUtilsCallback.save_attachment));

                    if (filesize == 1 && !attach.isSupportFormatCallback()) {
                        b.setMessage(Op01MessagePluginExt.sMessageUtilsCallback
                                .getString(IMessageUtilsCallback.save_single_attachment_notes));
                    } else if (filesize > 1) {
                        b.setMessage(Op01MessagePluginExt.sMessageUtilsCallback
                                .getString(IMessageUtilsCallback.save_multi_attachment_notes));
                    } else {
                        b.setMessage(Op01MessagePluginExt.sMessageUtilsCallback.getString(
                               IMessageUtilsCallback.save_single_supportformat_attachment_notes));
                    }
                    ///M: Modify for ALPS01485146
                    final long iMsgId = msgId;
                    b.setCancelable(true);
                    b.setPositiveButton(android.R.string.ok,
                        new DialogInterface.OnClickListener() {
                            public final void onClick(DialogInterface dialog, int which) {
                                if (!Environment.getExternalStorageState().equals(
                                        Environment.MEDIA_MOUNTED)) {
                                    Toast.makeText(mHostContext,
                                        Op01MessagePluginExt.sMessageUtilsCallback
                                        .getString(IMessageUtilsCallback.invalid_contact_message),
                                        Toast.LENGTH_SHORT);
                                }

                                if (filesize == 1) {
                                    boolean succeeded = false;
                                    succeeded = copyTextSingleAttachment(mHostContext,
                                            iMsgId);
                                    if (!succeeded) {
                                        succeeded = copySingleAttachment(mHostContext,
                                                iMsgId);
                                    }
                                    if (succeeded) {
                                        Toast t = Toast.makeText(mHostContext,
                                        Op01MessagePluginExt.sMessageUtilsCallback
                                        .getString(IMessageUtilsCallback.copy_to_sdcard_success),
                                        Toast.LENGTH_SHORT);
                                        t.show();
                                    } else {
                                        Toast t = Toast.makeText(mHostContext,
                                        Op01MessagePluginExt.sMessageUtilsCallback
                                        .getString(IMessageUtilsCallback.copy_to_sdcard_fail),
                                        Toast.LENGTH_SHORT);
                                        t.show();
                                    }
                                } else if (filesize > 1) {
                                    Bundle data = new Bundle();
                                    data.putLong("savecontent",
                                            Op01AttachmentEnhance.MMS_SAVE_OTHER_ATTACHMENT);
                                    intent.putExtra("msgid", iMsgId);
                                    intent.putExtras(data);
                                    // mHostContext.startActivityForResult(i,0);
                                    mHostContext.startActivity(intent);
                                }
                            }
                        });
                    b.setNegativeButton(android.R.string.cancel,
                        new DialogInterface.OnClickListener() {
                            public final void onClick(DialogInterface dialog, int which) {
                                dialog.dismiss();
                            }
                        });
                    b.create().show();
                }
            });
        }

        String nameText = "";
        Drawable dbThumb = null;
        if (filesize == 1 && !attach.isVCardCallback() && !attach.isVCalendarCallback()) {
            nameText = attach.getSrcCallback();
            dbThumb = Op01MessagePluginExt.sMessageUtilsCallback
                    .getDrawable(IMessageUtilsCallback.unsupported_file);
            Log.i(TAG, "filesize=1, add attach view");
        } else if (filesize > 1) {
            nameText = Op01MessagePluginExt.sMessageUtilsCallback
                    .getString(IMessageUtilsCallback.multi_attach_files, filesize);
            dbThumb = mContext.getResources().getDrawable(R.drawable.multi_files);
            Log.i(TAG, "filesize > 1, add attach view");
        }

        if ((filesize == 1 && !attach.isVCardCallback() && !attach.isVCalendarCallback()) ||
               filesize > 1) {
            tvName.setText(nameText);
            ivThumb.setImageDrawable(dbThumb);
            tvSize.setText(mMessageListItem.getHumanReadableSizeCallback(
            Op01MmsUtils.getAllAttachSize(attachFiles)));
        }
    }
*/
    // For save single attachment(Include: copySingleAttachment,copyPart,getUniqueDestination)
    /* This function is for save single text or html attachment into filemgr */
    private boolean copyTextSingleAttachment(Context context, long msgId) {
        boolean result = false;

        PduBody body;
         try {
                body = Op01MessagePluginExt.sMessageUtilsCallback.getPduBodyCallback(mContext,
                        ContentUris.withAppendedId(Mms.CONTENT_URI, msgId));
         } catch (Exception e) {
                 Log.e(TAG, e.getMessage(), e);
                 return false;
         }
        int i = 0;

        if (body == null) {
            return false;
        }

        PduPart part = null;
        String src = null;
        String contentType = null;

        ISlideshowModelCallback slideshow = null;

        try {
            slideshow = Op01MessagePluginExt.sMessageUtilsCallback
                            .createFromPduBodyCallback(mContext, body);
        } catch (Exception e) {
             Log.v(TAG, "Create from pdubody exception!");
             return false;
        }

        if (slideshow == null) {
            Log.e(TAG, "copyTextSingleAttachment slideshow is null");
            return false;
        }
        ArrayList<IFileAttachmentModelCallback> attachmentList =
                                slideshow.getAttachFilesCallback();

        byte[] data = null;
        data = attachmentList.get(0).getDataCallback();

        contentType = new String(attachmentList.get(0).getContentTypeCallback());

        //get part filename
        src = attachmentList.get(0).getSrcCallback();

        //format filename
        src = formatFileName(src);

        if (src == null) {
            Log.v(TAG, "copyTextSingleAttachment() File name == null");
            return false;
        }

        if (!contentType.equals("text/plain") && !contentType.equals("text/html")) {
            Log.v(TAG, "copyTextSingleAttachment() It is not a text or html attachment");
            return false;
        }
        result = copyTextPart(context, data, src);

       Log.i(TAG, "copyTextSingleAttachment() result is " + result);
       return result;
    }

    private boolean copySingleAttachment(Context context, long msgId) {
        boolean result = false;
        // PduBody body = ComposeMessageActivity.getPduBody(mContext, msgId);
        PduBody body;
        try {
            body = Op01MessagePluginExt.sMessageUtilsCallback.getPduBodyCallback(context,
                    ContentUris.withAppendedId(Mms.CONTENT_URI, msgId));
        } catch (Exception e) {
            Log.e(TAG, e.getMessage(), e);
            return false;
        }

        if (body == null) {
            return false;
        }
        int partNum = body.getPartsNum();
        PduPart part = null;
        String src = null;

        ISlideshowModelCallback slideshow = null;
        try {
            slideshow = Op01MessagePluginExt.sMessageUtilsCallback
                          .createFromPduBodyCallback(mContext, body);
        } catch (Exception e) {
             Log.v(TAG, "Create from pdubody exception!");
             return false;
        }
        if (slideshow == null) {
            Log.v(TAG, "slideshow Create failed. need check it.");
            return false;
        }

        for (int i = 0; i < partNum; i++) {
            part = body.getPart(i);
            byte[] cl = part.getContentLocation();
            byte[] name = part.getName();
            byte[] ci = part.getContentId();
            byte[] fn = part.getFilename();
            //get part filename
            if (cl != null) {
                src = new String(cl);
            } else if (name != null) {
                src = new String(name);
            } else if (ci != null) {
               src = new String(ci);
            } else if (fn != null) {
               src = new String(fn);
            } else {
               continue;
            }

            // get part uri
            String partUri = null;

            if (part.getDataUri() != null) {
                Log.d(TAG, "part Uri = " + part.getDataUri().toString());
                partUri = part.getDataUri().toString();
            } else {
                Log.v(TAG, "PartUri = null");
                continue;
            }
            ArrayList<IFileAttachmentModelCallback> attachmentList =
                                    slideshow.getAttachFilesCallback();

            for (int k = 0; k < attachmentList.size(); k++) {
                if (attachmentList.get(k).getUriCallback() != null) {
                    if (partUri.compareTo(attachmentList.get(k).getUriCallback().toString()) == 0) {
                        //MmsLog.v(TAG, "part.getFilename() = "+part.getFilename());
                        result = true;
                        break;
                     }
                } else {
                    result = false;
                }
            }

            if (result) {
                break;
            }
          }

          if (result) {
              src = formatFileName(src);
              mMessageListItem.copyPartCallback(part, src);
          } else {
              Log.i(TAG, "There is no a correct part! ");
          }

          return result;
    }

    // Get rid of illegal characters in filename
    private String formatFileName(String fileName) {

        if (fileName == null) {
            Log.i(TAG, "In formatFileName filename = null");
            return null;
        }

        String extension  = null;
        int index;
        if ((index = fileName.indexOf(".")) != -1) {
            extension = fileName.substring(index + 1, fileName.length());
            fileName = fileName.substring(0, index);
        }
        final String regex = "[:\\/?,. ]";
        fileName = fileName.replaceAll(regex, "_");
        fileName = fileName.replaceAll("<", "");
        fileName = fileName.replaceAll(">", "");

        Log.i(TAG, "getNameFromPart, fileName is " + fileName + ", extension is " + extension);

        return fileName + "." + extension;
    }

    private boolean copyTextPart(Context context, byte[] data, String filename) {
        FileOutputStream fout = null;
        try {
            File file = mMessageListItem.getStorageFileCallback(filename, context);
            if (file == null) {
                return false;
            }
            fout = new FileOutputStream(file);
            fout.write(data, 0, data.length);
        } catch (IOException e) {
            Log.e(TAG, "IOException caught while opening or reading stream", e);
            return false;
        } finally {
            if (null != fout) {
                try {
                    fout.close();
                } catch (IOException e) {
                    // Ignore
                    Log.e(TAG, "IOException caught while closing stream", e);
                    return false;
                }
            }
        }

        return true;
    }

    @Override
    public void openUrl(Context context, String url) {
        Log.d(TAG, "openUrl, url=" + url);
        final String strUrl = url;
        final Context theContext = context;
        if (!url.startsWith("mailto:")) {
            AlertDialog.Builder b = new AlertDialog.Builder(theContext);
            b.setTitle(com.mediatek.internal.R.string.url_dialog_choice_title);
            b.setMessage(com.mediatek.internal.R.string.url_dialog_choice_message);
            b.setCancelable(true);
            b.setNegativeButton(android.R.string.cancel, new DialogInterface.OnClickListener() {
                public final void onClick(DialogInterface dialog, int which) {
                    dialog.dismiss();
                }
            });
            b.setPositiveButton(android.R.string.ok, new DialogInterface.OnClickListener() {

                @Override
                public void onClick(DialogInterface dialog, int which) {
                    Uri theUri = Uri.parse(strUrl);
                    Intent intent = new Intent(Intent.ACTION_VIEW, theUri);
                    intent.putExtra(Browser.EXTRA_APPLICATION_ID, theContext.getPackageName());
                    intent.setFlags(Intent.FLAG_ACTIVITY_CLEAR_WHEN_TASK_RESET);
                    theContext.startActivity(intent);
                }
            });
            b.show();
        } else {
            super.openUrl(theContext, url);
        }
    }
}