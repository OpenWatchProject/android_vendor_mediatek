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

package com.mediatek.rcse.activities;

import android.app.Activity;
import android.app.AlertDialog;
import android.app.Dialog;
import android.app.DialogFragment;
import android.app.ProgressDialog;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.SharedPreferences;
import android.content.SharedPreferences.Editor;
import android.content.pm.PackageManager;
import android.content.pm.ResolveInfo;
import android.graphics.drawable.Drawable;
import android.media.MediaFile;
import android.os.AsyncTask;
import android.os.Bundle;
import android.os.Message;
import android.os.ParcelUuid;
import android.os.Parcelable;
import android.preference.PreferenceManager;
import android.text.TextUtils;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.view.Window;
import android.webkit.MimeTypeMap;
import android.widget.BaseAdapter;
import android.widget.CheckBox;
import android.widget.ImageView;
import android.widget.ListView;
import android.widget.TextView;

import com.mediatek.rcse.plugin.message.IpMessageConsts;
import com.mediatek.rcse.activities.widgets.ContactsListManager;
import com.mediatek.rcse.activities.widgets.DateView;
import com.mediatek.rcse.api.Logger;
import com.mediatek.rcse.api.Participant;
import com.mediatek.rcse.interfaces.ChatController;
import com.mediatek.rcse.interfaces.ChatModel.IChatManager;
import com.mediatek.rcse.mvc.ControllerImpl;
import com.mediatek.rcse.mvc.ModelImpl;
import com.mediatek.rcse.mvc.One2OneChat;
import com.mediatek.rcse.mvc.ParticipantStatusInfo;
import com.mediatek.rcse.plugin.message.PluginGroupChatActivity;
import com.mediatek.rcse.plugin.message.PluginGroupChatWindow;
import com.mediatek.rcse.plugin.message.PluginUtils;
import com.mediatek.rcse.service.ApiManager;
import com.mediatek.rcse.service.RcsNotification;
import com.mediatek.rcse.service.UnreadMessageManager;
import com.mediatek.rcse.service.Utils;
import com.mediatek.rcse.service.VideoCompress;
import com.mediatek.rcse.settings.AppSettings;

import com.mediatek.rcs.R;
import com.mediatek.rcse.service.MediatekFactory;
import com.mediatek.rcse.settings.RcsSettings;

import java.util.ArrayList;
import java.util.Date;
import java.util.HashMap;
import java.util.Iterator;
import java.util.List;
import java.util.Map;

import com.gsma.services.rcs.JoynServiceException;
import com.gsma.services.rcs.capability.Capabilities;
import com.gsma.services.rcs.capability.CapabilityService;
import com.gsma.services.rcs.chat.ChatIntent;
import com.gsma.services.rcs.chat.OneToOneChatIntent;
import com.gsma.services.rcs.chat.ChatService;
import com.gsma.services.rcs.chat.GroupChat;
import com.gsma.services.rcs.chat.GroupChatIntent;

/**
 * Provide the activity to show the dialog for user to accept or decline the
 * invitation.
 */
public class InvitationDialog extends Activity {

    private static final String TAG = "InvitationDialog";

    public static final String ACTION = "com.mediatek.rcse.action.INVITE_DIALOG";

    public static final String KEY_STRATEGY = "strategy";

    public static final String KEY_IS_FROM_CHAT_SCREEN = "from";

    public static final int STRATEGY_GROUP_INVITATION = 0;

    public static final int STRATEGY_FILE_TRANSFER_INVITATION = 1;

    public static final int STRATEGY_FILE_TRANSFER_SIZE_WARNING = 2;

    public static final int STRATEGY_IPMES_GROUP_INVITATION = 3;

    /*
     * For message status by all participants in group - read, not received,
     * received but not read
     */
    public static final int STRATEGY_GROUP_CHAT_MSG_RECEIVED_STATUS = 4;

    public static final int STRATEGY_AUTO_ACCEPT_FILE = 5;

    /* For File seen status by all participants in group - received, seen */
    public static final int STRATEGY_GROUP_FILE_VIEW_STATUS = 6;

    /* For message plugin to sent by sms */
    public static final int STRATEGY_IPMES_SEND_BY_SMS = 7;

    public static final int STRATEGY_IPMES_RESIZE_FILE = 8;

    public static final int STRATEGY_SHOW_VIDEO_PROGRESS_DIALOG = 9;

    public static final String SESSION_ID = "sessionId";
    private static HashMap<String, ArrayList<String>> sFailedJoynTextMap =
            new HashMap<String, ArrayList<String>>();
    public static boolean sNeedToShowFailedDialog = false;
    public static boolean sDialogAlreadyShown = false;

    public static DialogFragment mVideoCompressDialog = null;
    Boolean mShowVideoCompressDialog = false;

    private IInvitationStrategy mCurrentStrategy = null;
    // specific view for dialog.
    private View mContentView = null;

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        Intent intent = this.getIntent();
        int strategyType = intent.getIntExtra(KEY_STRATEGY, -1);
        Logger.d(TAG, "onCreate entry, with strategy: " + strategyType);
        switch (strategyType) {
        case STRATEGY_GROUP_INVITATION:
            mCurrentStrategy = new GroupInvitationStrategy(intent);
            break;
        case STRATEGY_FILE_TRANSFER_INVITATION:
            mCurrentStrategy = new FileTransferInvitationStrategy(intent);
            break;
        case STRATEGY_FILE_TRANSFER_SIZE_WARNING:
            mCurrentStrategy = new FileSizeWarningStrategy(intent);
            break;
        case STRATEGY_IPMES_GROUP_INVITATION:
            mCurrentStrategy = new IpMesPluginGroupInvitationStrategy(intent);
            break;
        case STRATEGY_GROUP_CHAT_MSG_RECEIVED_STATUS:
            mCurrentStrategy = new GroupchatStatusStrategy(intent);
            break;
        case STRATEGY_AUTO_ACCEPT_FILE:
            mCurrentStrategy = new FileAutoAcceptStrategy(intent);
            break;
        case STRATEGY_GROUP_FILE_VIEW_STATUS:
            mCurrentStrategy = new GroupFileViewStrategy(intent);
            break;
        case STRATEGY_IPMES_SEND_BY_SMS:
            mCurrentStrategy = new SendBySmsStrategy(intent);
            break;
        case STRATEGY_IPMES_RESIZE_FILE:
            mCurrentStrategy = new FileresizeStrategy(intent);
            break;
        case STRATEGY_SHOW_VIDEO_PROGRESS_DIALOG:
            mShowVideoCompressDialog = intent.getBooleanExtra("showProgress", false);
            if(mShowVideoCompressDialog) {
            Logger.d(TAG, "onCreate STRATEGY_SHOW_VIDEO_PROGRESS_DIALOG show dialog");
            mCurrentStrategy = new VideoResizeStrategy(intent);
            } else {
                Logger.d(TAG, "onCreate STRATEGY_SHOW_VIDEO_PROGRESS_DIALOG already showing , so dismiss");
                if(mVideoCompressDialog != null) {
                    mVideoCompressDialog.dismiss();
                }
                InvitationDialog.this.finish();
            }
            break;
        default:
            Logger.e(TAG, "onCreate unknown strategy: " + strategyType);
            break;
        }
        if (mCurrentStrategy instanceof GroupInvitationStrategy
                && ((GroupInvitationStrategy) mCurrentStrategy)
                        .onUserBehavior() == null) {
            Logger.d(TAG, "onCreate GroupInvitationStrategy time out");
            // Should decrease the number of unread message
            Logger.d(TAG, "Has read the group chat invitation");
            UnreadMessageManager.getInstance().changeUnreadMessageNum(
                    UnreadMessageManager.MIN_STEP_UNREAD_MESSAGE_NUM, false);
            TimeoutDialog dialog = new TimeoutDialog(InvitationDialog.this);
            dialog.show(getFragmentManager(), TimeoutDialog.TAG);
            if (mCurrentStrategy instanceof IpMesPluginGroupInvitationStrategy) {
                Logger.d(TAG,
                        "onCreate IpMesPluginGroupInvitationStrategy time out");
                ((IpMesPluginGroupInvitationStrategy) mCurrentStrategy)
                        .removeGroupChatInvitationInMms(intent);
            } else {
                Logger.d(TAG,
                        "onCreate not IpMesPluginGroupInvitationStrategy time out");
            }
        } else if (mCurrentStrategy instanceof FileSizeWarningStrategy) {
            FileSizeWarningDialog dialog = new FileSizeWarningDialog(InvitationDialog.this);
            dialog.show(getFragmentManager(), FileSizeWarningDialog.TAG);
        } else if (mCurrentStrategy instanceof GroupchatStatusStrategy) {
            GroupchatStatusDialog dialog = new GroupchatStatusDialog(InvitationDialog.this, new GroupMessageStatusArrayAdapter());
            dialog.show(getFragmentManager(), GroupchatStatusDialog.TAG);
        } else if (mCurrentStrategy instanceof GroupFileViewStrategy) {
            GroupFileViewDialog dialog = new GroupFileViewDialog(InvitationDialog.this, new GroupFileViewArrayAdapter());
            dialog.show(getFragmentManager(), GroupFileViewDialog.TAG);
        } else if (mCurrentStrategy instanceof FileresizeStrategy) {
            FileResizeDialog dialog = new FileResizeDialog(InvitationDialog.this);
            dialog.show(getFragmentManager(), FileResizeDialog.TAG);
            dialog.setCancelable(false);
            // dialog.setCanceledOnTouchOutside(false);
        } else if (mCurrentStrategy instanceof VideoResizeStrategy) {
            Logger.d(TAG, "onCreate VideoResizeStrategy");
            mVideoCompressDialog = new VideoResizeDialog(InvitationDialog.this);
            mVideoCompressDialog.show(getFragmentManager(), VideoResizeDialog.TAG);
            mVideoCompressDialog.setCancelable(false);
        }
        else if (mCurrentStrategy instanceof FileAutoAcceptStrategy) {
            Logger.d(TAG, "onCreate FileAutoAcceptStrategyt");
            SharedPreferences sPrefer = PreferenceManager
                    .getDefaultSharedPreferences(InvitationDialog.this);
            Boolean isRemind = sPrefer.getBoolean("fileautoacceptreminder",
                    true);
            if (isRemind) {
                Logger.d(TAG, "FileAutoAcceptStrategyt isRemind True");
                AutoAcceptFileDialog dialog = new AutoAcceptFileDialog(InvitationDialog.this);
                dialog.show(getFragmentManager(), AutoAcceptFileDialog.TAG);
            } else {
                InvitationDialog.this.finish();
            }
        } else if (mCurrentStrategy instanceof GroupInvitationStrategy) {
            // Should decrease the number of unread message
            Logger.d(TAG, "Has read the group chat invitation");
            UnreadMessageManager.getInstance().changeUnreadMessageNum(
                    UnreadMessageManager.MIN_STEP_UNREAD_MESSAGE_NUM, false);
            GroupInvitationDialog dialog = new GroupInvitationDialog(InvitationDialog.this);
            String content = intent
                    .getStringExtra(RcsNotification.NOTIFY_CONTENT);
            dialog.setContent(content);
            dialog.show(getFragmentManager(), GroupInvitationDialog.TAG);
        } else if (mCurrentStrategy instanceof SendBySmsStrategy) {

            sNeedToShowFailedDialog = ((SendBySmsStrategy) mCurrentStrategy).mIntent
                    .getBooleanExtra("showDialog", false);
            String contact = ((SendBySmsStrategy) mCurrentStrategy).mIntent
                    .getStringExtra("contact");
            ArrayList<String> failedTexts = new ArrayList<String>();

            if (contact != null) {
                if (sFailedJoynTextMap.containsKey(contact)) {
                    failedTexts = sFailedJoynTextMap.get(contact);
                }
            }
            Logger.d(TAG,
                    "When send joyn failed, inform user to send by sms size = "
                            + failedTexts.size() + "contact = " + contact);
            if (failedTexts.size() >= 1 && sNeedToShowFailedDialog) {
                if (sDialogAlreadyShown) {
                    Logger.d(TAG,
                            "When send joyn failed  already shown dialog ");
                    InvitationDialog.this.finish();
                }
                SMSStrategyDialog dialog = new SMSStrategyDialog(InvitationDialog.this);
                Logger.d(TAG, "When send joyn failed show dialog now,");
                String content = getString(R.string.send_by_sms_dialog);
                dialog.setContent(content);
                dialog.setCancelable(false);
                // dialog.setCanceledOnTouchOutside(false);
                dialog.show(getFragmentManager(), GroupInvitationDialog.TAG);
            } else {
                InvitationDialog.this.finish();
            }
        } else if (mCurrentStrategy instanceof FileTransferInvitationStrategy){
            FileTransferDialog dialog = new FileTransferDialog(InvitationDialog.this);
            String fileName = intent
                    .getStringExtra(RcsNotification.NOTIFY_FILE_NAME);
            String fileSize = intent
                    .getStringExtra(RcsNotification.NOTIFY_SIZE);
            String title = intent.getStringExtra(RcsNotification.NOTIFY_TITLE);
            String formatSize = Utils.formatFileSizeToString(
                    Long.parseLong(fileSize), Utils.SIZE_TYPE_TOTAL_SIZE);
            View view = LayoutInflater.from(this).inflate(
                    R.layout.ft_invitation_content, null);
            TextView imageName = (TextView) view.findViewById(R.id.image_name);
            TextView imageSize = (TextView) view.findViewById(R.id.image_size);
            TextView warningMessage = (TextView) view
                    .findViewById(R.id.warning_message);
            long maxFileSize = ApiManager.getInstance()
                    .getMaxSizeforFileThransfer();
            long warningFileSize = ApiManager.getInstance()
                    .getWarningSizeforFileThransfer();
            Logger.w(TAG, "onCreate() maxFileSize is " + maxFileSize);
            Logger.w(TAG, "onCreate() warningFileSize is " + warningFileSize);
            SharedPreferences sPrefer = PreferenceManager
                    .getDefaultSharedPreferences(InvitationDialog.this);
            Boolean isRemind = sPrefer.getBoolean(SettingsFragment.RCS_REMIND,
                    false);
            Logger.w(TAG,
                    "onCreate(), WarningDialog onCreateDialog the remind status is "
                            + isRemind);
            if (((FileTransferInvitationStrategy) mCurrentStrategy).mFileSize >= warningFileSize
                    && isRemind && warningFileSize != 0) {
                warningMessage.setVisibility(View.VISIBLE);
            } else {
                warningMessage.setVisibility(View.GONE);
            }
            ImageView imageType = (ImageView) view
                    .findViewById(R.id.image_type);
            String mimeType = MediaFile.getMimeTypeForFile(fileName);
            if (mimeType == null) {
                mimeType = MimeTypeMap.getSingleton().getMimeTypeFromExtension(
                        Utils.getFileExtension(fileName));
            }
            String type = getString(R.string.file_type_file);
            if (mimeType != null) {
                if (mimeType.contains(Utils.FILE_TYPE_IMAGE)) {
                    type = getString(R.string.file_type_image);
                } else if (mimeType.contains(Utils.FILE_TYPE_AUDIO)) {
                    type = getString(R.string.file_type_audio);
                } else if (mimeType.contains(Utils.FILE_TYPE_VIDEO)) {
                    type = getString(R.string.file_type_video);
                } else if (mimeType.contains(Utils.FILE_TYPE_TEXT)) {
                    type = getString(R.string.file_type_text);
                } else if (mimeType.contains(Utils.FILE_TYPE_APP)) {
                    type = getString(R.string.file_type_app);
                }
            }
            Intent it = new Intent(Intent.ACTION_VIEW);
            it.setDataAndType(Utils.getFileNameUri(fileName), mimeType);
            PackageManager packageManager = getPackageManager();
            List<ResolveInfo> list = packageManager.queryIntentActivities(it,
                    PackageManager.MATCH_DEFAULT_ONLY);
            int size = list.size();
            Drawable drawable = getResources().getDrawable(
                    R.drawable.rcs_ic_ft_default_preview);
            if (size > 0) {
                drawable = list.get(0).activityInfo.loadIcon(packageManager);
            }
            imageType.setImageDrawable(drawable);
            imageName.setText(fileName);
            imageSize.setText(formatSize);
            mContentView = view;
            Bundle arguments = new Bundle();
            arguments.putString(Utils.TITLE, type + title);
            dialog.setArguments(arguments);
            dialog.show(getFragmentManager(), FileTransferDialog.TAG);
        }
    }

    /**
     * The Class TimeoutDialog.
     */
    public static class TimeoutDialog extends DialogFragment implements
            DialogInterface.OnClickListener {
        private static final String TAG = "TimeoutDialog";
        private InvitationDialog mActivity = null;

        public TimeoutDialog(InvitationDialog activity) {
            mActivity = activity;
        }

        @Override
        public void onCancel(DialogInterface dialog) {
            dismissAllowingStateLoss();
            if (mActivity != null) {
                mActivity.finish();
            }
        }

        @Override
        public Dialog onCreateDialog(Bundle savedInstanceState) {
            final AlertDialog alertDialog;
            alertDialog = new AlertDialog.Builder(mActivity,
                    AlertDialog.THEME_HOLO_LIGHT).setIconAttribute(
                    android.R.attr.alertDialogIcon).create();
            alertDialog.setTitle(R.string.invitation_timeout_title);
            alertDialog
                    .setMessage(getString(R.string.invitation_timeout_message));
            alertDialog.setButton(DialogInterface.BUTTON_POSITIVE,
                    getString(R.string.rcs_dialog_positive_button), this);
            return alertDialog;
        }

        @Override
        public void onClick(DialogInterface dialog, int which) {
            if (mActivity != null) {
                Intent data = new Intent(mActivity.getIntent());
                String extas = data
                        .getStringExtra(GroupChatIntent.EXTRA_CHAT_ID);
                if (!TextUtils.isEmpty(extas)) {
                    RcsNotification.getInstance().removeGroupInvite(extas);
                } else {
                    Logger.d(TAG, "onClick(),extas is null");
                }
                dismissAllowingStateLoss();
                if (mActivity != null) {
                    mActivity.finish();
                }
            } else {
                dismissAllowingStateLoss();
                Logger.d(TAG, "activity is null");
            }
        }
    }

    /**
     * The Class GroupInvitationDialog.
     */
    public static class GroupInvitationDialog extends DialogFragment implements
            DialogInterface.OnClickListener {
        private static final String TAG = "GroupInvitationDialog";
        private String mContent = null;
        private InvitationDialog mActivity = null;

        public GroupInvitationDialog(InvitationDialog activity) {
            mActivity = activity;
        }

        @Override
        public void onCancel(DialogInterface dialog) {
            dismissAllowingStateLoss();
            if (mActivity != null) {
                mActivity.finish();
            }
        }

        /**
         * Sets the content.
         *
         * @param content the new content
         */
        public void setContent(String content) {
            mContent = content;
        }

        @Override
        public Dialog onCreateDialog(Bundle savedInstanceState) {
            final AlertDialog alertDialog;
            alertDialog = new AlertDialog.Builder(mActivity,
                    AlertDialog.THEME_HOLO_LIGHT).create();
            alertDialog.setTitle(R.string.chat_invitation_title);
            alertDialog.setMessage(mContent);
            alertDialog.setButton(DialogInterface.BUTTON_POSITIVE,
                    getString(R.string.file_transfer_button_accept), this);
            alertDialog.setButton(DialogInterface.BUTTON_NEGATIVE,
                    getString(R.string.file_transfer_button_reject), this);
            return alertDialog;
        }

        @Override
        public void onClick(DialogInterface dialog, int which) {
            if (which == DialogInterface.BUTTON_POSITIVE) {
                if (mActivity != null && mActivity.mCurrentStrategy != null) {
                    mActivity.mCurrentStrategy.onUserAccept();
                } else {
                    Logger.e(TAG,
                            "onClick accpect, but mCurrentStrategy is null");
                }
            } else {
                if (mActivity != null && mActivity.mCurrentStrategy != null) {
                    mActivity.mCurrentStrategy.onUserDecline();
                } else {
                    Logger.e(TAG,
                            "onClick decline, but mCurrentStrategy is null");
                }
            }
            dismissAllowingStateLoss();
            if (mActivity != null) {
                mActivity.finish();
            }
        }
    }

    /**
     * The Class SMSStrategyDialog.
     */
    public static class SMSStrategyDialog extends DialogFragment implements
            DialogInterface.OnClickListener {
        private static final String TAG = "SMSStrategyDialog";
        private String mContent = null;
        private InvitationDialog mActivity = null;

        public SMSStrategyDialog(InvitationDialog activity) {
            mActivity = activity;
        }

        @Override
        public void onCancel(DialogInterface dialog) {
            dismissAllowingStateLoss();
            if (mActivity != null) {
                mActivity.finish();
            }
        }

        /**
         * Sets the content.
         *
         * @param content the new content
         */
        public void setContent(String content) {
            mContent = content;
        }

        @Override
        public Dialog onCreateDialog(Bundle savedInstanceState) {
            final AlertDialog alertDialog;
            sDialogAlreadyShown = true;
            alertDialog = new AlertDialog.Builder(mActivity,
                    AlertDialog.THEME_HOLO_LIGHT).create();
            alertDialog.setTitle(R.string.failed_joyn_messages);
            alertDialog.setMessage(mContent);
            alertDialog.setButton(DialogInterface.BUTTON_POSITIVE,
                    getString(R.string.file_transfer_button_accept), this);
            alertDialog.setButton(DialogInterface.BUTTON_NEGATIVE,
                    getString(R.string.file_transfer_button_reject), this);
            alertDialog.setCancelable(false);
            alertDialog.setCanceledOnTouchOutside(false);
            return alertDialog;
        }

        @Override
        public void onClick(DialogInterface dialog, int which) {
            if (which == DialogInterface.BUTTON_POSITIVE) {
                if (mActivity != null && mActivity.mCurrentStrategy != null) {
                    mActivity.mCurrentStrategy.onUserAccept();
                    sDialogAlreadyShown = false;
                } else {
                    Logger.e(TAG,
                            "onClick accpect, but mCurrentStrategy is null");
                }
            } else {
                if (mActivity != null && mActivity.mCurrentStrategy != null) {
                    mActivity.mCurrentStrategy.onUserDecline();
                    sDialogAlreadyShown = false;
                } else {
                    Logger.e(TAG,
                            "onClick decline, but mCurrentStrategy is null");
                }
            }
            dismissAllowingStateLoss();
            if (mActivity != null) {
                mActivity.finish();
            }
        }
    }

    /**
     * The Class FileTransferDialog.
     */
    public static class FileTransferDialog extends DialogFragment implements
            DialogInterface.OnClickListener {
        private static final String TAG = "FileTransferDialog";
        private String mTitle = null;
        private InvitationDialog mActivity = null;

        public FileTransferDialog(InvitationDialog activity) {
            mActivity = activity;
        }

        @Override
        public void onCancel(DialogInterface dialog) {
            dismissAllowingStateLoss();
            if (mActivity != null) {
                mActivity.finish();
            }
        }

        @Override
        public Dialog onCreateDialog(Bundle savedInstanceState) {
            Bundle arguments = getArguments();
            mTitle = arguments.getString(Utils.TITLE);
            final AlertDialog alertDialog;
            alertDialog = new AlertDialog.Builder(mActivity,
                    AlertDialog.THEME_HOLO_LIGHT).setView(mActivity.mContentView)
                    .create();
            if (mTitle != null) {
                alertDialog.setTitle(mTitle);
            } else {
                alertDialog.setTitle(getString(R.string.file_transfer_title));
            }
            alertDialog.setButton(DialogInterface.BUTTON_POSITIVE,
                    getString(R.string.file_transfer_button_accept), this);
            alertDialog.setButton(DialogInterface.BUTTON_NEGATIVE,
                    getString(R.string.file_transfer_button_reject), this);
            return alertDialog;
        }

        @Override
        public void onClick(DialogInterface dialog, int which) {
            if (which == DialogInterface.BUTTON_POSITIVE) {
                if (mActivity != null && mActivity.mCurrentStrategy != null) {
                    mActivity.mCurrentStrategy.onUserAccept();
                } else {
                    Logger.e(TAG,
                            "onClick accpect, but mCurrentStrategy is null");
                }
            } else {
                if (mActivity != null && mActivity.mCurrentStrategy != null) {
                    mActivity.mCurrentStrategy.onUserDecline();
                } else {
                    Logger.e(TAG,
                            "onClick decline, but mCurrentStrategy is null");
                }
            }
            dismissAllowingStateLoss();
            if (mActivity != null) {
                mActivity.finish();
            }
        }
    }

    /**
     * The Class FileSizeWarningDialog.
     */
    public static class FileSizeWarningDialog extends DialogFragment implements
            DialogInterface.OnClickListener {
        private static final String TAG = "FileSizeWarningDialog";
        private CheckBox mCheckRemind = null;
        private InvitationDialog mActivity = null;

        public FileSizeWarningDialog(InvitationDialog activity) {
            mActivity = activity;
        }

        @Override
        public void onCancel(DialogInterface dialog) {
            super.onCancel(dialog);
            if (mActivity != null) {
                mActivity.finish();
            }
        }

        @Override
        public Dialog onCreateDialog(Bundle savedInstanceState) {
            final AlertDialog alertDialog;
            alertDialog = new AlertDialog.Builder(mActivity,
                    AlertDialog.THEME_HOLO_LIGHT).create();
            alertDialog.setTitle(R.string.file_size_warning);
            if (mActivity != null) {
                LayoutInflater inflater = LayoutInflater.from(mActivity
                        .getApplicationContext());
                View customView = inflater.inflate(R.layout.warning_dialog,
                        null);
                mCheckRemind = (CheckBox) customView
                        .findViewById(R.id.remind_notification);
                alertDialog.setView(customView);
            } else {
                Logger.e(TAG, "onCreateDialog activity is null");
            }
            alertDialog.setButton(DialogInterface.BUTTON_POSITIVE,
                    getString(R.string.rcs_dialog_positive_button), this);
            alertDialog.setButton(DialogInterface.BUTTON_NEGATIVE,
                    getString(R.string.rcs_dialog_negative_button), this);
            return alertDialog;
        }

        @Override
        public void onClick(DialogInterface dialog, int which) {
            if (which == DialogInterface.BUTTON_POSITIVE) {
                if (mActivity != null && mActivity.mCurrentStrategy != null) {
                    mActivity.mCurrentStrategy.onUserAccept();
                } else {
                    Logger.e(TAG,
                            "onClick accpect, but mCurrentStrategy is null");
                }
                if (mCheckRemind != null) {
                    boolean isCheck = mCheckRemind.isChecked();
                    SharedPreferences sPrefer = PreferenceManager
                            .getDefaultSharedPreferences(mActivity);
                    Editor remind = sPrefer.edit();
                    remind.putBoolean(SettingsFragment.RCS_REMIND, !isCheck);
                    remind.commit();
                }
            } else {
                if (mActivity != null && mActivity.mCurrentStrategy != null) {
                    mActivity.mCurrentStrategy.onUserDecline();
                } else {
                    Logger.e(TAG,
                            "onClick decline, but mCurrentStrategy is null");
                }
            }
            dismissAllowingStateLoss();
            if (mActivity != null) {
                mActivity.finish();
            }
        }
    }

    /**
     * The Class FileResizeDialog.
     */
    public static class FileResizeDialog extends DialogFragment implements
            DialogInterface.OnClickListener {
        private static final String TAG = "FileResizeDialog";
        private CheckBox mCheckRemind = null;
        private InvitationDialog mActivity = null;

        public FileResizeDialog(InvitationDialog activity) {
            mActivity = activity;
        }

        @Override
        public void onCancel(DialogInterface dialog) {
            super.onCancel(dialog);
            if (mActivity != null) {
                mActivity.finish();
            }
        }

        @Override
        public Dialog onCreateDialog(Bundle savedInstanceState) {
            final AlertDialog alertDialog;
            alertDialog = new AlertDialog.Builder(mActivity,
                    AlertDialog.THEME_HOLO_LIGHT).create();
            alertDialog.setTitle(R.string.compress_image_title);
            if (mActivity != null) {
                LayoutInflater inflater = LayoutInflater.from(mActivity
                        .getApplicationContext());
                View customView = inflater.inflate(R.layout.warning_dialog,
                        null);
                mCheckRemind = (CheckBox) customView
                        .findViewById(R.id.remind_notification);
                alertDialog.setView(customView);
            } else {
                Logger.e(TAG, "onCreateDialog activity is null");
            }
            alertDialog.setButton(DialogInterface.BUTTON_POSITIVE,
                    getString(R.string.rcs_dialog_positive_button), this);
            alertDialog.setButton(DialogInterface.BUTTON_NEGATIVE,
                    getString(R.string.rcs_dialog_negative_button), this);
            alertDialog.setIconAttribute(android.R.attr.alertDialogIcon);
            return alertDialog;
        }

        @Override
        public void onClick(DialogInterface dialog, int which) {
            if (mCheckRemind != null) {
                Logger.d(TAG,
                        "onClick mCheckRemind is " + mCheckRemind.isChecked());
                AppSettings.getInstance().saveRemindCompressFlag(
                        mCheckRemind.isChecked());
            }
            if (which == DialogInterface.BUTTON_POSITIVE) {
                if (mActivity != null && mActivity.mCurrentStrategy != null) {
                    if (mCheckRemind.isChecked()) {
                        Logger.d(TAG,
                                "the user enable compressing image and not remind again");
                        AppSettings.getInstance().setCompressingImage(true);
                    }
                    mActivity.mCurrentStrategy.onUserAccept();
                } else {
                    Logger.d(TAG,
                            "onClick accept, but mCurrentStrategy is null");
                }
            } else {
                if (mActivity != null && mActivity.mCurrentStrategy != null) {
                    mActivity.mCurrentStrategy.onUserDecline();
                } else {
                    Logger.d(TAG,
                            "onClick decline, but mCurrentStrategy is null");
                }
            }
            dismissAllowingStateLoss();
            if (mActivity != null) {
                mActivity.finish();
            }
        }
    }


    /**
     * The Class FileResizeDialog.
     */
    public static class VideoResizeDialog extends DialogFragment implements
            DialogInterface.OnClickListener {
        private static final String TAG = "VideoResizeDialog";
        private CheckBox mCheckRemind = null;
        private InvitationDialog mActivity = null;

        public VideoResizeDialog(InvitationDialog activity) {
            mActivity = activity;
        }

        @Override
        public void onCancel(DialogInterface dialog) {
            super.onCancel(dialog);
            if (mActivity != null) {
                mActivity.finish();
            }
        }

        @Override
        public Dialog onCreateDialog(Bundle savedInstanceState) {
            final ProgressDialog alertDialog;
            alertDialog = new ProgressDialog(mActivity, ProgressDialog.THEME_HOLO_LIGHT);
            //alertDialog.setTitle(R.string.compress_video_dialog_title);
            alertDialog.setMessage(getString(R.string.compress_video_dialog_content));
            alertDialog.setButton(DialogInterface.BUTTON_POSITIVE,
                    "Hide", this);
            alertDialog.setButton(DialogInterface.BUTTON_NEGATIVE,
                   "Cancel", this);
            alertDialog.setCancelable(false);
            alertDialog.requestWindowFeature(Window.FEATURE_NO_TITLE);
            alertDialog.setCanceledOnTouchOutside(false);
            return alertDialog;

        }

        @Override
        public void onClick(DialogInterface dialog, int which) {
            if (which == DialogInterface.BUTTON_POSITIVE) {
                Logger.d(TAG, "onClick Video compress Hide button clicked");
                if (mActivity != null && mActivity.mCurrentStrategy != null) {
                    mActivity.mCurrentStrategy.onUserAccept();
                }
            } else {
                if (mActivity != null && mActivity.mCurrentStrategy != null) {
                    mActivity.mCurrentStrategy.onUserDecline();
                } else {
                    Logger.d(TAG,
                            "onClick video compress decline, but mCurrentStrategy is null");
                }
            }
            dismissAllowingStateLoss();
            if (mActivity != null) {
                mActivity.finish();
            }
        }
    }

    /**
     * The Class FileresizeStrategy.
     */
    private class VideoResizeStrategy implements IInvitationStrategy {
        private String mContact = null;
        private String mFilePath = null;
        private String mFileName = null;
        private ParcelUuid mParcelUuid = null;
        private long mSize = 0;

        /**
         * Instantiates a new fileresize strategy.
         *
         * @param intent the intent
         */
        public VideoResizeStrategy(Intent intent) {
            mContact = intent.getStringExtra("contact");
            mFilePath = intent.getStringExtra("filePath");
            mFileName = intent.getStringExtra("fileName");
            mParcelUuid = (ParcelUuid) intent.getParcelableExtra("uuid");
            mSize = intent.getLongExtra("size", 0);
            if (mParcelUuid != null) {
                Logger.d(TAG, "VideoResizeStrategy, constructor " + mContact
                        + mFilePath + mParcelUuid.toString());
            }
        }

        @Override
        public void onUserDecline() {
            Logger.d(TAG, "VideoResizeStrategy, onUserDecline Compression ");
            Intent it = new Intent();
            it.setAction(PluginUtils.ACTION_SEND_CANCELLED_COMPRESSED_VIDEO);
            MediatekFactory.getApplicationContext()
                    .sendBroadcast(it);
            InvitationDialog.this.finish();
        }

        @Override
        public void onUserAccept() {
            Logger.d(TAG, "FileresizeStrategy, onUserHide Compression ");
            InvitationDialog.this.finish();
        }
    }

    /**
     * Send file from plugin.
     *
     * @param filePath the file path
     * @param contact the contact
     * @param fileTransferTag the file transfer tag
     */
    public void sendFileFromPlugin(String filePath, String contact,
            ParcelUuid fileTransferTag) {
        Logger.d(TAG, "sendFileFromPlugin() filePath: " + filePath
                + " , contact: " + contact + " , fileTransferTag: "
                + fileTransferTag);
        ControllerImpl controller = ControllerImpl.getInstance();
        Message controllerMessage = controller.obtainMessage(
                ChatController.EVENT_FILE_TRANSFER_INVITATION, contact,
                filePath);
        Bundle data = new Bundle();
        data.putParcelable(ModelImpl.SentFileTransfer.KEY_FILE_TRANSFER_TAG,
                (Parcelable) fileTransferTag);
        controllerMessage.setData(data);
        controllerMessage.sendToTarget();

    }

    /**
     * The Class AutoAcceptFileDialog.
     */
    public static class AutoAcceptFileDialog extends DialogFragment implements
            DialogInterface.OnClickListener {
        private static final String TAG = "AutoAcceptFileDialog";
        private CheckBox mCheckRemind = null;
        private InvitationDialog mActivity = null;

        public AutoAcceptFileDialog(InvitationDialog activity) {
            mActivity = activity;
        }

        @Override
        public void onCancel(DialogInterface dialog) {
            super.onCancel(dialog);
            if (mActivity != null) {
                mActivity.finish();
            }
        }

        @Override
        public Dialog onCreateDialog(Bundle savedInstanceState) {
            final AlertDialog alertDialog;
            Logger.e(TAG, "onCreateDialog AutoAccpet Strategy");
            alertDialog = new AlertDialog.Builder(mActivity,
                    AlertDialog.THEME_HOLO_LIGHT).create();
            alertDialog.setTitle("Auto Accept Files");
            if (mActivity != null) {
                Logger.e(TAG,
                        "onCreateDialog AutoAccpet Strategy activity not null");
                LayoutInflater inflater = LayoutInflater.from(mActivity
                        .getApplicationContext());
                View customView = inflater.inflate(
                        R.layout.auto_accept_file_dialog, null);
                mCheckRemind = (CheckBox) customView
                        .findViewById(R.id.remind_notification);
                alertDialog.setView(customView);
            } else {
                Logger.e(TAG, "onCreateDialog activity is null");
            }
            alertDialog.setButton(DialogInterface.BUTTON_POSITIVE,
                    getString(R.string.rcs_dialog_positive_button), this);
            alertDialog.setButton(DialogInterface.BUTTON_NEGATIVE,
                    getString(R.string.rcs_dialog_negative_button), this);
            return alertDialog;
        }

        @Override
        public void onClick(DialogInterface dialog, int which) {
            if (which == DialogInterface.BUTTON_POSITIVE) {
                RcsSettings.getInstance().setFileTransferAutoAccepted(true);
            }
            if (mCheckRemind != null) {
                boolean isCheck = mCheckRemind.isChecked();
                SharedPreferences sPrefer = PreferenceManager
                        .getDefaultSharedPreferences(mActivity);
                Editor remind = sPrefer.edit();
                remind.putBoolean("fileautoacceptreminder", !isCheck);
                remind.commit();
            }
            dismissAllowingStateLoss();
            if (mActivity != null) {
                mActivity.finish();
            }
        }
    }

    /**
     * The Class GroupMessageStatusArrayAdapter.
     */
    public class GroupMessageStatusArrayAdapter extends BaseAdapter {

        GroupchatStatusStrategy mGroupChatStatusStrategy =
                (GroupchatStatusStrategy) mCurrentStrategy;

        @Override
        public View getView(int position, View convertView, ViewGroup parent) {

            View itemView = convertView;
            if (itemView == null) {
                LayoutInflater inflater = LayoutInflater.from(parent
                        .getContext());
                itemView = inflater.inflate(
                        R.layout.group_chat_message_status_item, parent, false);
            }
            String contact = mGroupChatStatusStrategy.getParticipant(position);
            ParticipantStatusInfo status = mGroupChatStatusStrategy.getStatus(position);
            Logger.d(TAG,
                    "GroupMessageStatusArrayAdapter contact:" + contact
                            + ",status=" + status.getStatus() + ",Date="
                            + status.getDate());
            bindView(itemView, contact, status.getStatus(), status.getDate());
            return itemView;
        }

        private void bindView(View itemView, String contact, int status, Date date) {

            ImageView statusView = (ImageView) itemView
                    .findViewById(R.id.status);
            statusView.setImageDrawable(getResource(status));
            TextView remoteName = (TextView) itemView
                    .findViewById(R.id.remote_name);
            remoteName.setText(ContactsListManager.getInstance()
                    .getDisplayNameByPhoneNumber(contact));
            if(date!=null) {
                DateView dateView = (DateView) itemView.findViewById(R.id.status_time);
                if (AppSettings.getInstance()!= null && RcsSettings.getInstance().isSupportOP07()) {
                    setMessageStatusDateOnView(dateView, date);
                }

            }
        }

        private void setMessageStatusDateOnView(DateView v, Date date) {
            if (v != null) {
                v.setVisibility(View.VISIBLE);
                v.setTime(date, false);
            } else {
                Logger.w(TAG, "The view is null.");
            }
        }

        /**
         * Gets the resource.
         *
         * @param status the status
         * @return the resource
         */
        private Drawable getResource(int status) {
            Drawable image = null;
            switch (status) {
            case 0:
            case 4:
            case 5:
                image = getResources().getDrawable(R.drawable.messge_not_sent);
                break;
            case 1:
                image = getResources().getDrawable(R.drawable.messge_sent);
                break;
            case 2:
                image = getResources().getDrawable(R.drawable.messge_sent_seen);
                break;
            case 3:
                image = getResources().getDrawable(R.drawable.im_meg_status_read);
                break;
            default:
                image = getResources().getDrawable(R.drawable.messge_not_sent);

            }
            return image;
        }

        @Override
        public int getCount() {

            return mGroupChatStatusStrategy.getParticipantsSize();
        }

        @Override
        public Object getItem(int position) {

            return mGroupChatStatusStrategy.getParticipant(position);
        }

        @Override
        public long getItemId(int position) {

            return 0;
        }
    }

    /**
     * The Class GroupFileViewArrayAdapter.
     */
    public class GroupFileViewArrayAdapter extends BaseAdapter {

        GroupFileViewStrategy mGroupFileViewStrategy = (GroupFileViewStrategy) mCurrentStrategy;

        @Override
        public View getView(int position, View convertView, ViewGroup parent) {

            View itemView = convertView;
            if (itemView == null) {
                LayoutInflater inflater = LayoutInflater.from(parent
                        .getContext());
                itemView = inflater.inflate(
                        R.layout.group_file_view_status_item, parent, false);
            }
            Integer status = mGroupFileViewStrategy.getStatus(position);
            bindView(itemView, mGroupFileViewStrategy.getParticipant(position),
                    status);
            return itemView;
        }

        private void bindView(View itemView, String contact, Integer status) {

            Logger.v(TAG, "GroupFileViewArrayAdapter bindView = " + contact
                    + ",status=" + status);
            ImageView statusView = (ImageView) itemView
                    .findViewById(R.id.status);
            statusView.setImageDrawable(getResource(status));
            TextView remoteName = (TextView) itemView
                    .findViewById(R.id.remote_name);
            remoteName.setText(ContactsListManager.getInstance()
                    .getDisplayNameByPhoneNumber(contact));
        }

        /**
         * Gets the resource.
         *
         * @param status the status
         * @return the resource
         */
        private Drawable getResource(int status) {
            Drawable image = null;
            switch (status) {
            case 0:
            case 3:
            case 4:
                image = getResources().getDrawable(R.drawable.messge_not_sent);
                break;
            case 7:
                image = getResources().getDrawable(R.drawable.messge_sent);
                break;
            case 2:
                image = getResources().getDrawable(R.drawable.messge_sent_seen);
                break;
            case 5:
                image = getResources().getDrawable(
                        R.drawable.messge_not_sent);
                break;
            default:
                image = getResources().getDrawable(R.drawable.messge_not_sent);

            }
            return image;
        }

        @Override
        public int getCount() {

            return mGroupFileViewStrategy.getParticipantsSize();
        }

        @Override
        public Object getItem(int position) {

            return mGroupFileViewStrategy.getParticipant(position);
        }

        @Override
        public long getItemId(int position) {

            return 0;
        }
    }

    /**
     * The Class GroupchatStatusDialog.
     */
    public static class GroupchatStatusDialog extends DialogFragment implements
            DialogInterface.OnClickListener {
        private static final String TAG = "GroupchatStatusDialog";
        // private CheckBox mCheckRemind = null;
        private InvitationDialog mActivity = null;
        private GroupMessageStatusArrayAdapter customAdapter = null;

        public GroupchatStatusDialog(InvitationDialog activity, GroupMessageStatusArrayAdapter adapter) {
            mActivity = activity;
            customAdapter = adapter;
        }

        @Override
        public void onCancel(DialogInterface dialog) {
            super.onCancel(dialog);
            if (mActivity != null) {
                mActivity.finish();
            }
        }

        @Override
        public Dialog onCreateDialog(Bundle savedInstanceState) {
            final AlertDialog alertDialog;
            alertDialog = new AlertDialog.Builder(mActivity,
                    AlertDialog.THEME_HOLO_LIGHT).create();
            alertDialog.setTitle("Message status"); // TODO
            if (mActivity != null && customAdapter != null) {
                LayoutInflater inflater = LayoutInflater.from(mActivity
                        .getApplicationContext());
                View customView = inflater.inflate(
                        R.layout.group_chat_message_status, null);
                // mCheckRemind = (CheckBox)
                // customView.findViewById(R.id.remind_notification);
                ListView listView = (ListView) customView
                        .findViewById(R.id.messageList);

                listView.setAdapter(customAdapter);
                alertDialog.setView(customView);
            } else {
                Logger.e(TAG, "onCreateDialog activity is null");
            }
            alertDialog.setButton(DialogInterface.BUTTON_POSITIVE,
                    getString(R.string.rcs_dialog_positive_button), this);

            return alertDialog;
        }

        @Override
        public void onClick(DialogInterface dialog, int which) {
            if (which == DialogInterface.BUTTON_POSITIVE) {
                if (mActivity != null && mActivity.mCurrentStrategy != null) {
                    mActivity.mCurrentStrategy.onUserAccept();
                } else {
                    Logger.e(TAG,
                            "onClick accpect, but mCurrentStrategy is null");
                }

            }
            dismissAllowingStateLoss();
            if (mActivity != null) {
                mActivity.finish();
            }
        }
    }

    /**
     * The Class GroupFileViewDialog.
     */
    public static class GroupFileViewDialog extends DialogFragment implements
            DialogInterface.OnClickListener {
        private static final String TAG = "GroupFileViewDialog";
        // private CheckBox mCheckRemind = null;
        private InvitationDialog mActivity = null;
        private GroupFileViewArrayAdapter customAdapter = null;

        public GroupFileViewDialog(InvitationDialog activity, GroupFileViewArrayAdapter adapter) {
            mActivity = activity;
            customAdapter = adapter;
        }

        @Override
        public void onCancel(DialogInterface dialog) {
            super.onCancel(dialog);
            if (mActivity != null) {
                mActivity.finish();
            }
        }

        @Override
        public Dialog onCreateDialog(Bundle savedInstanceState) {
            final AlertDialog alertDialog;
            alertDialog = new AlertDialog.Builder(mActivity,
                    AlertDialog.THEME_HOLO_LIGHT).create();
            alertDialog.setTitle("File status"); // TODO
            if (mActivity != null && customAdapter != null) {
                LayoutInflater inflater = LayoutInflater.from(mActivity
                        .getApplicationContext());
                View customView = inflater.inflate(
                        R.layout.group_file_view_status, null);
                // mCheckRemind = (CheckBox)
                // customView.findViewById(R.id.remind_notification);
                ListView listView = (ListView) customView
                        .findViewById(R.id.messageList);

                listView.setAdapter(customAdapter);
                alertDialog.setView(customView);
            } else {
                Logger.e(TAG, "onCreateDialog activity is null");
            }
            alertDialog.setButton(DialogInterface.BUTTON_POSITIVE,
                    getString(R.string.rcs_dialog_positive_button), this);

            return alertDialog;
        }

        @Override
        public void onClick(DialogInterface dialog, int which) {
            if (which == DialogInterface.BUTTON_POSITIVE) {
                if (mActivity != null && mActivity.mCurrentStrategy != null) {
                    mActivity.mCurrentStrategy.onUserAccept();
                } else {
                    Logger.e(TAG,
                            "onClick accpect, but mCurrentStrategy is null");
                }
                /*
                 * if (mCheckRemind != null) { boolean isCheck =
                 * mCheckRemind.isChecked(); SharedPreferences sPrefer =
                 * PreferenceManager.getDefaultSharedPreferences(mActivity);
                 * Editor remind = sPrefer.edit();
                 * remind.putBoolean(SettingsFragment.RCS_REMIND, !isCheck);
                 * remind.commit(); }
                 */
            }
            dismissAllowingStateLoss();
            if (mActivity != null) {
                mActivity.finish();
            }
        }
    }

    /**
     * The Interface IInvitationStrategy.
     */
    private interface IInvitationStrategy {
        /**
         * On user decline.
         */
        void onUserDecline();

        /**
         * On user accept.
         */
        void onUserAccept();
    }

    /**
     * The Class GroupInvitationStrategy.
     */
    private class GroupInvitationStrategy implements IInvitationStrategy {
        private static final String TAG = "GroupInvitationStrategy";
        protected Intent mIntent = null;
        protected String mSessionId = null;

        /**
         * Instantiates a new group invitation strategy.
         *
         * @param intent the intent
         */
        public GroupInvitationStrategy(Intent intent) {
            mIntent = intent;
        }

        /**
         * On user behavior.
         *
         * @return the group chat
         */
        public GroupChat onUserBehavior() {
            // ApiManager
            ApiManager instance = ApiManager.getInstance();
            if (instance == null) {
                Logger.d(TAG,
                        "onUserBehavior() The ApiManager instance is null");
                finish();
                return null;
            }

            // MessagingApi
            ChatService chatApi = instance.getChatApi();
            if (chatApi == null) {
                Logger.d(TAG, "onUserBehavior() The messageApi is null");
                finish();
                return null;
            }

            // Invitation intent list
            ArrayList<Intent> invitations = new ArrayList<Intent>();
            invitations.add(mIntent);

            // session id
            String chatId = invitations.get(invitations.size() - 1)
                    .getStringExtra(GroupChatIntent.EXTRA_CHAT_ID);
            Logger.d(TAG, "onUserBehavior() sessionId + " + chatId);
            mSessionId = chatId;
            if (chatId == null) {
                Logger.d(TAG, "onUserBehavior() The chatId is null");
                return null;
            }

            // ChatSession
            GroupChat chatSession = null;
            try {
                chatSession = chatApi.getGroupChat(chatId);
            } catch (Exception e) {
                e.printStackTrace();
            } finally {
                if (chatSession == null) {
                    Logger.d(TAG, "onUserBehavior() The chatSession is null");
                    return null;
                }
            }
            return chatSession;
        }

        @Override
        public void onUserAccept() {
            GroupChat chatSession = onUserBehavior();
            if (TextUtils.isEmpty(mSessionId)) {
                Logger.d(TAG, "onUserAccept() mSessionId is empty");
                return;
            }
            if (null != chatSession) {
                Logger.d(TAG, "onUserAccept() chatSession is not null");
                RcsNotification.getInstance().removeGroupInvite(mSessionId);
                try {
                    int state = chatSession.getState().toInt();
                    Logger.v(TAG, "onUserAccept() state = " + state);
                    if (state == GroupChat.State.ABORTED.toInt()) {
                        Logger.d(TAG,
                                "onUserAccept() This group chat invitation has been aborted");
                        //chatSession.quitConversation();
                        return;
                    }
                } catch (Exception e) {
                    e.printStackTrace();
                }
                mIntent.setClass(InvitationDialog.this,
                        ChatScreenActivity.class);
                startActivity(mIntent);
            } else {
                Logger.e(TAG,
                        "onUserAccept() chatSession is null.Use session id delete" +
                        " group chat invite");
                RcsNotification.getInstance().removeGroupInvite(mSessionId);
            }
        }

        @Override
        public void onUserDecline() {
            final GroupChat chatSession = onUserBehavior();
            if (null != chatSession) {
                RcsNotification.getInstance().removeGroupInvite(mSessionId);
                AsyncTask<Void, Void, Void> task = new AsyncTask<Void, Void, Void>() {
                    @Override
                    protected Void doInBackground(Void... arg0) {
                        try {
                            //chatSession.rejectInvitation();//todo
                        } catch (Exception e) {
                            e.printStackTrace();
                        }
                        return null;
                    }
                };
                task.execute();
            } else {
                Logger.e(TAG,
                        "onUserDecline() chatSession is null.Use session id delete " +
                        "group chat invite");
                RcsNotification.getInstance().removeGroupInvite(mSessionId);
            }
        }

    }

    /**
     * The Class IpMesPluginGroupInvitationStrategy.
     */
    private class IpMesPluginGroupInvitationStrategy extends
            GroupInvitationStrategy {

        private static final String TAG = "IpMesPluginGroupInvitationStrategy";

        /**
         * Instantiates a new ip mes plugin group invitation strategy.
         *
         * @param intent the intent
         */
        public IpMesPluginGroupInvitationStrategy(Intent intent) {
            super(intent);
        }

        @Override
        public void onUserAccept() {
            removeGroupChatInvitationInMms(mIntent);

            GroupChat chatSession = onUserBehavior();
            if (TextUtils.isEmpty(mSessionId)) {
                Logger.d(TAG, "onUserAccept() mSessionId is empty");
                return;
            }
            if (null != chatSession) {
                Logger.d(TAG, "onUserAccept() chatSession is not null");
                RcsNotification.getInstance().removeGroupInvite(mSessionId);
                try {
                    int state = chatSession.getState().toInt();
                    Logger.v(TAG, "onUserAccept() state = " + state);
                    if (state == GroupChat.State.ABORTED.toInt()) {
                        Logger.d(TAG,
                                "onUserAccept() This group chat invitation has been terminated");
                        //chatSession.quitConversation();
                        return;
                    }
                } catch (Exception e) {
                    e.printStackTrace();
                }
                String action = mIntent.getAction();

                if (Logger.getIsIntegrationMode()) {
                    Logger.d(TAG, "onUserAccept() action is " + action);
                    if (ACTION.equals(action)) {
                        Logger.d(TAG,
                                "onUserAccept() accept from conversation list");
                        Intent invitation = new Intent();
                        ParcelUuid tag = (ParcelUuid) mIntent
                                .getParcelableExtra(ChatScreenActivity.KEY_CHAT_TAG);
                        invitation.putExtra(ChatScreenActivity.KEY_CHAT_TAG,
                                tag);
                        invitation.putExtra(
                                ChatScreenActivity.KEY_USED_CHATTAG, tag);
                        invitation.setClass(InvitationDialog.this,
                                PluginGroupChatActivity.class);
                        invitation.setAction(OneToOneChatIntent.ACTION_NEW_ONE_TO_ONE_CHAT_MESSAGE);
                        invitation.putExtras(mIntent.getExtras());
                        invitation.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK
                                | Intent.FLAG_ACTIVITY_CLEAR_TASK);
                        startActivity(invitation);
                    } else {
                        Logger.d(TAG, "onUserAccept() accept from notification");
                        ParcelUuid tag = (ParcelUuid) mIntent
                                .getParcelableExtra(ChatScreenActivity.KEY_CHAT_TAG);
                        mIntent.putExtra(ChatScreenActivity.KEY_USED_CHATTAG,
                                tag);
                        mIntent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK
                                | Intent.FLAG_ACTIVITY_CLEAR_TASK);
                        mIntent.setClass(InvitationDialog.this,
                                PluginGroupChatActivity.class);
                        startActivity(mIntent);
                    }
                } else {
                    Logger.d(TAG, "onUserAccept() is chat app mode");
                    mIntent.setClass(InvitationDialog.this,
                            PluginGroupChatActivity.class);
                    startActivity(mIntent);
                }
                Logger.d(TAG, "onUserAccept() action is " + mIntent.getAction());
            } else {
                Logger.e(TAG,
                        "onUserAccept() chatSession is null.Use session id delete " +
                        "group chat invite");
                RcsNotification.getInstance().removeGroupInvite(mSessionId);
            }
        }

        @Override
        public void onUserDecline() {
            Logger.d(TAG, "onUserDecline entry mIntent is " + mIntent);
            removeGroupChatInvitationInMms(mIntent);
            super.onUserDecline();
        }

        /**
         * Remove group chat invitation message in mms.
         *
         * @param intent            The invitation intent
         */
        public void removeGroupChatInvitationInMms(Intent intent) {
            Logger.d(TAG,
                    "removeGroupChatInvitationInMms in InvitationDialog entry");
            String contact = intent
                    .getStringExtra(PluginGroupChatWindow.GROUP_CHAT_CONTACT);
            if (null != contact) {
                PluginGroupChatWindow.removeGroupChatInvitationInMms(contact);
            } else {
                Logger.w(TAG, "removeGroupChatInvitationInMms contact is null");
            }
        }
    }

    /**
     * This is a strategy for File Transfer invitations.
     */
    private class FileTransferInvitationStrategy implements IInvitationStrategy {
        private static final String TAG = "FileTransferInvitationStrategy";
        private long mFileSize = 0;
        private String mContactName = null;
        private String mContactNumber = null;
        Intent mIntent = null;
        private boolean mIsFromChatScreen = false;

        /**
         * Instantiates a new file transfer invitation strategy.
         *
         * @param intent the intent
         */
        public FileTransferInvitationStrategy(Intent intent) {
            Logger.d(TAG,
                    "FileTransferInvitationStrategy() entry with intent is "
                            + intent);
            mIntent = intent;
            String fileSize = intent
                    .getStringExtra(RcsNotification.NOTIFY_SIZE);
            mContactNumber = intent.getStringExtra(RcsNotification.CONTACT);
            mContactName = ContactsListManager.getInstance()
                    .getDisplayNameByPhoneNumber(mContactNumber);
            Logger.d(TAG, "FileTransferInvitationStrategy(), mContactName is "
                    + mContactName + " mContactNumber is " + mContactNumber);
            mIsFromChatScreen = intent.getBooleanExtra(KEY_IS_FROM_CHAT_SCREEN,
                    false);
            if (fileSize != null) {
                mFileSize = Long.parseLong(fileSize);
                Logger.i(TAG, "FileTransferInvitationStrategy() mFileSize is "
                        + mFileSize);
            } else {
                Logger.e(TAG,
                        "FileTransferInvitationStrategy() filesize is null");
            }
        }

        /**
         * Gets the session id.
         *
         * @return the session id
         */
        private String getSessionId() {
            String sessionId = null;
            if (mIntent == null) {
                Logger.d(TAG, "getSessionId(), mIntent is null");
            } else {
                sessionId = mIntent.getStringExtra(RcsNotification.SESSION_ID);
            }
            Logger.d(TAG, "getSessionId(), sessionId is " + sessionId);
            return sessionId;
        }

        /**
         * Gets the chat.
         *
         * @return the chat
         */
        private One2OneChat getChat() {
            One2OneChat chat = null;
            if (mIntent == null) {
                Logger.d(TAG, "getChat(), mIntent is null");
                return null;
            }
            Participant fromSessionParticipant = new Participant(
                    mContactNumber, mContactName);
            List<Participant> participantList = new ArrayList<Participant>();
            participantList.add(fromSessionParticipant);
            IChatManager modelImpl = ModelImpl.getInstance();
            if (modelImpl != null) {
                chat = (One2OneChat) ModelImpl.getInstance().addChat(
                        participantList, null, null);
            } else {
                Logger.e(TAG, "getChat(), modelImpl is null");
            }
            Logger.d(TAG, "getChat(), chat is " + chat);
            return chat;
        }

        /**
         * Access to chat window.
         *
         * @param chat the chat
         */
        private void accessToChatWindow(One2OneChat chat) {
            if (chat == null || mIntent == null) {
                Logger.d(TAG,
                        "AccessToChatWindow)(), chat is null or mIntent is null");
                return;
            }
            Intent intent = new Intent();
            intent.setClass(InvitationDialog.this, ChatScreenActivity.class);
            intent.putExtra(ChatScreenActivity.KEY_CHAT_TAG,
                    (ParcelUuid) chat.getChatTag());
            // startActivity(intent);
        }

        @Override
        public void onUserAccept() {
            Logger.d(TAG, "onUserAccept() entry!");
            handleAcceptInvitation();
        }

        /**
         * Handle accept invitation.
         */
        protected void handleAcceptInvitation() {
            final One2OneChat chat = getChat();
            AsyncTask<Void, Void, Void> task = new AsyncTask<Void, Void, Void>() {
                @Override
                protected Void doInBackground(Void... arg0) {
                    if (null != chat) {
                        chat.handleAcceptFileTransfer(getSessionId());
                    } else {
                        Logger.e(TAG, "handleAcceptInvitation(), chat is null!");
                    }
                    return null;
                }
            };
            task.execute();
            if (!mIsFromChatScreen) {
                accessToChatWindow(chat);
            } else {
                Logger.d(TAG,
                        "handleAcceptInvitation, The chat is in the foreground!");
            }
        }

        @Override
        public void onUserDecline() {
            final One2OneChat chat = getChat();
            AsyncTask<Void, Void, Void> task = new AsyncTask<Void, Void, Void>() {
                @Override
                protected Void doInBackground(Void... arg0) {
                    if (chat != null) {
                        chat.handleRejectFileTransfer(getSessionId());
                    } else {
                        Logger.e(TAG, "onUserDecline(), chat is null!");
                    }
                    return null;
                }
            };
            task.execute();
            RcsNotification.getInstance()
                    .cancelFileTransferNotificationWithContact(mContactNumber,
                            mFileSize);
        }
    }

    /**
     * The Class FileSizeWarningStrategy.
     */
    private class FileSizeWarningStrategy extends
            FileTransferInvitationStrategy {
        /**
         * Instantiates a new file size warning strategy.
         *
         * @param intent the intent
         */
        public FileSizeWarningStrategy(Intent intent) {
            super(intent);
        }

        @Override
        public void onUserAccept() {
            InvitationDialog.this.finish();
            handleAcceptInvitation();
        }
    }

    /**
     * The Class GroupchatStatusStrategy.
     */
    private class GroupchatStatusStrategy implements IInvitationStrategy {
        // private List<Participant> mParticipants;
        // public SentMessage mMessage;
        public Map<String, ParticipantStatusInfo> mMessageStatusMap;
        public ArrayList<String> contacts = new ArrayList<String>();
        public ArrayList<ParticipantStatusInfo> status = new ArrayList<ParticipantStatusInfo>();

        /**
         * Instantiates a new groupchat status strategy.
         *
         * @param intent the intent
         */
        public GroupchatStatusStrategy(Intent intent) {
            // mParticipants =
            // intent.getParcelableArrayListExtra(Utils.CHAT_PARTICIPANTS);
            mMessageStatusMap = (Map<String, ParticipantStatusInfo>) intent.getExtras()
                    .getSerializable("statusmap");

            Iterator it = mMessageStatusMap.entrySet().iterator();
            while (it.hasNext()) {
                Map.Entry pairs = (Map.Entry) it.next();
                // System.out.println(pairs.getKey() + " = " +
                // pairs.getValue());
                contacts.add((String) pairs.getKey());
                status.add((ParticipantStatusInfo) pairs.getValue());
                it.remove(); // to avoid a ConcurrentModificationException
            }
        }

        /**
         * Gets the participant.
         *
         * @param position the position
         * @return the participant
         */
        public String getParticipant(int position) {
            return contacts.get(position);
        }

        /**
         * Gets the status.
         *
         * @param position the position
         * @return the status
         */
        public ParticipantStatusInfo getStatus(int position) {
            return status.get(position);
        }

        /**
         * Gets the participants size.
         *
         * @return the participants size
         */
        public int getParticipantsSize() {
            return contacts.size();
        }

        @Override
        public void onUserDecline() {
            // TODO Auto-generated method stub

        }

        @Override
        public void onUserAccept() {
            // TODO Auto-generated method stub
            InvitationDialog.this.finish();

        }
    }

    /**
     * The Class GroupFileViewStrategy.
     */
    private class GroupFileViewStrategy implements IInvitationStrategy {
        public Map<String, Integer> mFileStatusMap = null;
        public ArrayList<String> contacts = new ArrayList<String>();
        public ArrayList<Integer> status = new ArrayList<Integer>();

        /**
         * Instantiates a new group file view strategy.
         *
         * @param intent the intent
         */
        public GroupFileViewStrategy(Intent intent) {
            mFileStatusMap = (Map<String, Integer>) intent.getExtras()
                    .getSerializable("statusmap");

            Iterator it = mFileStatusMap.entrySet().iterator();
            while (it.hasNext()) {
                Map.Entry pairs = (Map.Entry) it.next();
                // System.out.println(pairs.getKey() + " = " +
                // pairs.getValue());
                contacts.add((String) pairs.getKey());
                status.add((Integer) pairs.getValue());
                it.remove(); // to avoid a ConcurrentModificationException
            }
        }

        /**
         * Gets the participant.
         *
         * @param position the position
         * @return the participant
         */
        public String getParticipant(int position) {
            return contacts.get(position);
        }

        /**
         * Gets the status.
         *
         * @param position the position
         * @return the status
         */
        public Integer getStatus(int position) {
            return status.get(position);
        }

        /**
         * Gets the participants size.
         *
         * @return the participants size
         */
        public int getParticipantsSize() {
            return contacts.size();
        }

        @Override
        public void onUserDecline() {
            // TODO Auto-generated method stub

        }

        @Override
        public void onUserAccept() {
            // TODO Auto-generated method stub
            InvitationDialog.this.finish();

        }
    }

    /**
     * The Class FileAutoAcceptStrategy.
     */
    private class FileAutoAcceptStrategy implements IInvitationStrategy {
        private List<Participant> mParticipants;

        /**
         * Instantiates a new file auto accept strategy.
         *
         * @param intent the intent
         */
        public FileAutoAcceptStrategy(Intent intent) {

        }

        @Override
        public void onUserDecline() {
            // TODO Auto-generated method stub

        }

        @Override
        public void onUserAccept() {
            // TODO Auto-generated method stub
            InvitationDialog.this.finish();

        }
    }

    /**
     * The Class FileresizeStrategy.
     */
    private class FileresizeStrategy implements IInvitationStrategy {
        private String mContact = null;
        private String mFilePath = null;
        private ParcelUuid mParcelUuid = null;

        /**
         * Instantiates a new fileresize strategy.
         *
         * @param intent the intent
         */
        public FileresizeStrategy(Intent intent) {
            mContact = intent.getStringExtra("contact");
            mFilePath = intent.getStringExtra("filePath");
            mParcelUuid = (ParcelUuid) intent.getParcelableExtra("uuid");
            if (mParcelUuid != null) {
                Logger.d(TAG, "FileresizeStrategy, constructor " + mContact
                        + mFilePath + mParcelUuid.toString());
            }
        }

        @Override
        public void onUserDecline() {
            Logger.d(TAG, "FileresizeStrategy, onUserDecline ");
            sendFileFromPlugin(mFilePath, mContact, mParcelUuid);
            Intent it = new Intent();
            it.setAction(PluginUtils.ACTION_SEND_RESIZED_FILE);
            it.putExtra("compressImage", false);
            MediatekFactory.getApplicationContext().sendBroadcast(it, "com.mediatek.rcse.permission.RCS_ACCESS_MESSAGE");
            InvitationDialog.this.finish();
        }

        @Override
        public void onUserAccept() {
            Logger.d(TAG, "FileresizeStrategy, onUserAccept ");

            new AsyncTask<Void, Void, String>() {
                @Override
                protected String doInBackground(Void... params) {
                    return Utils.compressImage(mFilePath);
                }

                @Override
                protected void onPostExecute(String result) {
                    Logger.v(TAG, "onPostExecute(),result = " + result);
                    if (result != null) {
                        boolean compressFail = false;
                        if(mFilePath!=null && mFilePath.equals(result)) {
                            compressFail = true;
                        }
                        mFilePath = result;
                        if(!compressFail)
                        sendFileFromPlugin(mFilePath, mContact, mParcelUuid);
                        Intent it = new Intent();
                        it.setAction(PluginUtils.ACTION_SEND_RESIZED_FILE);
                        it.putExtra("compressImage", true);
                        it.putExtra(RcsNotification.CONTACT, mContact);
                        it.putExtra("filePath", mFilePath);
                        it.putExtra("uuid", mParcelUuid);
                        it.putExtra("compressFail", compressFail);
                        MediatekFactory.getApplicationContext()
                                .sendBroadcast(it, "com.mediatek.rcse.permission.RCS_ACCESS_MESSAGE");
                    }
                }
            }.execute();
            InvitationDialog.this.finish();
        }
    }

    /**
     * The Class SendBySmsStrategy.
     */
    private class SendBySmsStrategy implements IInvitationStrategy {

        private String mAddress;
        private String mText;
        public Intent mIntent = null;
        public ArrayList<String> failedJoynText = new ArrayList<String>();

        /**
         * Instantiates a new send by sms strategy.
         *
         * @param intent the intent
         */
        public SendBySmsStrategy(Intent intent) {
            mAddress = intent.getStringExtra("contact");
            mText = intent.getStringExtra("send_by_sms_text");
            mIntent = intent;
            boolean showDialog = intent.getBooleanExtra("showDialog", false);

            ArrayList<String> failedTexts = new ArrayList<String>();
            if (mAddress != null && !showDialog) {
                if (sFailedJoynTextMap.containsKey(mAddress)) {
                    Logger.d(TAG, "Send sms strtagey, address already in map "
                            + failedTexts.size());
                    failedTexts = sFailedJoynTextMap.get(mAddress);
                    failedTexts.add(mText);
                } else {
                    failedTexts.add(mText);
                    Logger.d(TAG, "Send sms strtagey, address not in map "
                            + failedTexts.size());
                    sFailedJoynTextMap.put(mAddress, failedTexts);
                }
            }
        }

        @Override
        public void onUserDecline() {
            ArrayList<String> failedTexts = new ArrayList<String>();
            if (mAddress != null) {
                if (sFailedJoynTextMap.containsKey(mAddress)) {
                    failedTexts = sFailedJoynTextMap.get(mAddress);
                }
            }
            failedTexts.clear();
            Intent it = new Intent();
            it.setAction(IpMessageConsts.IsOnlyUseXms.ACTION_ONLY_USE_XMS);
            it.putExtra("failedStatus", 0);
            it.putExtra("contact", mAddress);
            MediatekFactory.getApplicationContext().sendBroadcast(it, "com.mediatek.rcse.permission.RCS_ACCESS_MESSAGE");
        }

        @Override
        public void onUserAccept() {
            Participant fromSessionParticipant = new Participant(mAddress,
                    mAddress);
            List<Participant> participantList = new ArrayList<Participant>();
            participantList.add(fromSessionParticipant);
            IChatManager modelImpl = ModelImpl.getInstance();
            ArrayList<String> failedTexts = new ArrayList<String>();
            if (modelImpl != null) {
                if (mAddress != null) {
                    if (sFailedJoynTextMap.containsKey(mAddress)) {
                        failedTexts = sFailedJoynTextMap.get(mAddress);
                    }
                }
                One2OneChat chat = (One2OneChat) ModelImpl.getInstance()
                        .addChat(participantList, null, null);
                for (String mText : failedTexts) {
                    chat.sendMessage(mText, 0);
                }
                failedTexts.clear();
            } else {
                Logger.e(TAG, "getChat(), modelImpl is null");
            }
            Intent it = new Intent();
            it.setAction(IpMessageConsts.IsOnlyUseXms.ACTION_ONLY_USE_XMS);
            it.putExtra("failedStatus", 0);
            it.putExtra("contact", mAddress);
            MediatekFactory.getApplicationContext().sendBroadcast(it, "com.mediatek.rcse.permission.RCS_ACCESS_MESSAGE");
            InvitationDialog.this.finish();
        }
    }
}
