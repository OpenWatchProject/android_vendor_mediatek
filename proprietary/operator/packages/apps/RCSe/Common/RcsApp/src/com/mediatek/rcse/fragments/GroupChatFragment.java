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

package com.mediatek.rcse.fragments;

import android.app.Activity;
import android.app.AlertDialog;
import android.app.Dialog;
import android.app.DialogFragment;
import android.app.FragmentManager;
import android.app.FragmentTransaction;

import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.res.Configuration;
import android.content.res.Resources;
import android.database.Cursor;
import android.os.AsyncTask;
import android.os.Bundle;
import android.os.IBinder;
import android.os.Message;
import android.os.Parcelable;
import android.text.InputFilter;
import android.view.ContextMenu;
import android.view.ContextMenu.ContextMenuInfo;
import android.view.LayoutInflater;
import android.view.MenuInflater;
import android.view.MenuItem;
import android.view.View;
import android.view.View.OnCreateContextMenuListener;
import android.view.ViewGroup;
import android.view.inputmethod.InputMethodManager;
import android.widget.AdapterView;

import android.widget.BaseAdapter;
import android.widget.EditText;
import android.widget.HorizontalScrollView;
import android.widget.ImageButton;
import android.widget.LinearLayout;
import android.widget.LinearLayout.LayoutParams;
import android.widget.ListView;
import android.widget.RadioButton;
import android.widget.RadioGroup;
import android.widget.RelativeLayout;
import android.widget.TextView;

import com.mediatek.rcse.activities.ChatMainActivity;
import com.mediatek.rcse.activities.ChatScreenActivity;
import com.mediatek.rcse.activities.InvitationDialog;
import com.mediatek.rcse.activities.widgets.AsyncImageView;
import com.mediatek.rcse.activities.widgets.ContactsListManager;
import com.mediatek.rcse.api.Logger;
import com.mediatek.rcse.api.Participant;
import com.mediatek.rcse.emoticons.EmoticonsModelImpl;
import com.mediatek.rcse.fragments.ChatFragment.FileTransfer;
import com.mediatek.rcse.fragments.One2OneChatFragment.ReceivedFileTransfer;
import com.mediatek.rcse.fragments.One2OneChatFragment.SentFileTransfer;
import com.mediatek.rcse.fragments.One2OneChatFragment.SentMessage;
import com.mediatek.rcse.interfaces.ChatController;
import com.mediatek.rcse.interfaces.ChatView;
import com.mediatek.rcse.interfaces.ChatView.IChatEventInformation;
import com.mediatek.rcse.interfaces.ChatView.IChatEventInformation.Information;
import com.mediatek.rcse.interfaces.ChatView.IChatWindowMessage;
import com.mediatek.rcse.interfaces.ChatView.IFileTransfer;
import com.mediatek.rcse.interfaces.ChatView.IReceivedChatMessage;
import com.mediatek.rcse.interfaces.ChatView.ISentChatMessage;
import com.mediatek.rcse.interfaces.ChatView.ISentChatMessage.Status;
import com.mediatek.rcse.mvc.ControllerImpl;
import com.mediatek.rcse.mvc.ChatMessage;
import com.mediatek.rcse.mvc.GroupChat1;
import com.mediatek.rcse.mvc.ModelImpl.ChatEventStruct;
import com.mediatek.rcse.mvc.ModelImpl.FileStruct;
import com.mediatek.rcse.mvc.ParticipantInfo;
import com.mediatek.rcse.mvc.ParticipantStatusInfo;
import com.mediatek.rcse.plugin.message.PluginGroupChatActivity;
import com.mediatek.rcse.plugin.message.PluginMultiChatActivity;
import com.mediatek.rcse.provider.RichMessagingDataProvider;
import com.mediatek.rcse.service.ApiManager;
import com.mediatek.rcse.service.ContactIdUtils;
import com.mediatek.rcse.service.PluginApiManager;
import com.mediatek.rcse.service.Utils;

import com.mediatek.rcs.R;
import com.orangelabs.rcs.core.ims.service.im.chat.event.User;
import com.mediatek.rcse.service.MediatekFactory;
import com.mediatek.rcse.settings.AppSettings;
//import com.orangelabs.rcs.provider.eab.ContactsManager;
import com.mediatek.rcse.settings.RcsSettings;
//import com.orangelabs.rcs.service.api.server.gsma.GetContactCapabilitiesReceiver;
//import com.orangelabs.rcs.utils.PhoneUtils;

import java.text.SimpleDateFormat;
import java.util.ArrayList;
import java.util.Collections;
import java.util.Date;
import java.util.List;
import java.util.Vector;
import java.util.concurrent.ConcurrentHashMap;
import java.util.concurrent.CopyOnWriteArrayList;

import com.gsma.services.rcs.RcsService;
import com.gsma.services.rcs.chat.ChatLog;
//import com.gsma.services.rcs.chat.ChatMessage;
import com.gsma.services.rcs.contact.ContactService;
import com.gsma.services.rcs.JoynServiceConfiguration;
import com.gsma.services.rcs.capability.Capabilities;
import com.gsma.services.rcs.capability.CapabilityService;
import com.gsma.services.rcs.RcsServiceException;
import com.gsma.services.rcs.contact.ContactId;

/**
 * The Class GroupChatFragment.
 */
public class GroupChatFragment extends ChatFragment implements
        ChatView.IGroupChatWindow, View.OnClickListener,
        OnCreateContextMenuListener {

    private static final String TAG = "GroupChatFragment";

    public static final int ALPHA_VALUE_ENABLE = 255;
    // default load value for number of messages
    public static final int LOAD_DEFAULT = 20;
    private int mCurrentNumberMsgLoaded = 0;
    private String mChatID = "";
    public static final int ALPHA_VALUE_DISABLE = 75;
    private int mGroupMemberHorMarginLeft = 6;
    private int mGroupMemberHorMarginRight = 6;
    private int mGroupMemberHorWidth = 48;
    private int mGroupMemberHorHeight = 48;
    private static final int ZERO_PARTICIPANT = 0;
    public static final String SHOW_REJOING_MESSAGE_REMINDER = "d";
    private List<String> mDateList = new ArrayList<String>();
    private List<IChatMessage> mMessageList = new Vector<IChatMessage>();
    private Activity mActivity = null;
    private boolean mShowHeader = false;
    private String mPreviousDate = null;
    protected ArrayList<Integer> mMessageIdArray = new ArrayList<Integer>();
    private int mItemIDPosition;
    View mSubjectView;
    private static final String COLON = ": ";
    private final CopyOnWriteArrayList<Participant> mParticipantComposList =
            new CopyOnWriteArrayList<Participant>();
    protected TextView mRejoiningText = null;
    private List<ParticipantInfo> mGroupChatParticipantList = new ArrayList<ParticipantInfo>();
    // For test case to test whether max group chat participant mechanism work
    // when current participants is already the max number
    private boolean mIsMaxGroupChatParticipantsWork = false;
    private final Object mLock = new Object();
    private int mMessageSequenceOrder = -1;
    protected String mSubjectGroupChat = "";
    protected boolean mIsClosedGroupChat = false;

    public boolean isClosedGroupChat() {
        return mIsClosedGroupChat;
    }

    public void setIsClosedGroupChat(boolean isClosedGroupChat) {
        Logger.d(TAG, "setIsClosedGroupChat() type: " + isClosedGroupChat);
        this.mIsClosedGroupChat = isClosedGroupChat;
    }

    /**
     * Gets the m subject group chat.
     *
     * @return the m subject group chat
     */
    public String getmSubjectGroupChat() {
        return mSubjectGroupChat;
    }

    /**
     * Sets the m subject group chat.
     *
     * @param mSubjectGroupChat the new m subject group chat
     */
    public void setmSubjectGroupChat(String mSubjectGroupChat) {
        Logger.d(TAG, "setmSubjectGroupChat() subject: " + mSubjectGroupChat);
        if (mSubjectGroupChat != null && !mSubjectGroupChat.equals("")) {
            this.mSubjectGroupChat = mSubjectGroupChat;
        }
    }

    /**
     * Sets the tag.
     *
     * @param tag the new tag
     */
    public void setTag(Object tag) {
        Logger.d(TAG, "setTag() tag: " + tag);
        mTag = tag;
    }

    /**
     * Sets the participant list.
     *
     * @param participantList the new participant list
     */
    public void setParticipantList(
            final CopyOnWriteArrayList<ParticipantInfo> participantList) {
        Logger.d(TAG, "setParticipantList() entry the participants is "
                + participantList);
        mGroupChatParticipantList = participantList;
        List<Participant> participants = new ArrayList<Participant>();
        for (ParticipantInfo participantInfo : participantList) {
            participants.add(participantInfo.getParticipant());
        }
        mParticipantList = participants;
    }

    /**
     * Get the participants list in the group chat fragment.
     *
     * @return participants list in the group chat fragment
     */
    public List<Participant> getParticipants() {
        return mParticipantList;
    }

    @Override
    public void onAttach(Activity activity) {
        mActivity = activity;
        super.onAttach(activity);
    }

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        Logger.d(TAG, "onCreate() GroupChatFragment entry");
        Bundle arguments = getArguments();
        mTag = arguments.getParcelable(Utils.CHAT_TAG);
        mGroupChatParticipantList = arguments
                .getParcelableArrayList(Utils.CHAT_PARTICIPANTS);
        Logger.d(TAG, "onCreate() GroupChatFragment tag: " + mTag
                + "participantlist:" + mGroupChatParticipantList);
        List<Participant> participants = new ArrayList<Participant>();
        for (ParticipantInfo participantInfo : mGroupChatParticipantList) {
            participants.add(participantInfo.getParticipant());
        }
        mParticipantList = participants;
        loadDimension(getResources());
        mMessageListView.setOnCreateContextMenuListener(this);
        mMessageListView
                .setOnItemClickListener(new AdapterView.OnItemClickListener() {

                    @Override
                    public void onItemClick(AdapterView<?> parent, View view,
                            int position, long id) {
                        // TODO Auto-generated method stub

                    }
                });
        Configuration configuration = getResources().getConfiguration();
        mParticipantListDisplayer.onFragmentCreate(configuration.orientation);
        mRejoiningText = (TextView) mContentView
                .findViewById(R.id.text_rejoining_prompt);
        // loadmessageIdList();
    }

    /**
     * Show subject dialog.
     */
    public void showSubjectDialog() {
        final AlertDialog.Builder alert = new AlertDialog.Builder(getActivity());
        final String subject;
        int maxLength = 15;
        Logger.d(TAG, "showSubjectDialog entry");
        mSubjectView = LayoutInflater.from(getActivity()).inflate(
                R.layout.group_subject_dialog, null);
        final TextView titleText = (TextView) mSubjectView
                .findViewById(R.id.titleTextView);
        final EditText input = (EditText) mSubjectView
                .findViewById(R.id.inputEditText);
        final RadioGroup radioOptionGrp = (RadioGroup) mSubjectView
                .findViewById(R.id.group_type);
        if(JoynServiceConfiguration.isClosedGroupSupported(getActivity().getApplicationContext())){
            titleText.setText(" Please enter the group name and select type");
            radioOptionGrp.setVisibility(View.VISIBLE);
        } else {
        titleText.setText(" Please enter the group name");
            radioOptionGrp.setVisibility(View.GONE);
        }
        InputFilter[] filterArray = new InputFilter[1];
        filterArray[0] = new InputFilter.LengthFilter(maxLength);
        input.setFilters(filterArray);
        mBtnEmotion = (ImageButton) mSubjectView
                .findViewById(R.id.btn_chat_emoticon_subject);
        mBtnEmotion.setOnClickListener(mBtnEmotionClickListener);
        input.setOnClickListener(mMessageEditorClickListener);
        super.updateEmoticonLayout(true, mSubjectView);
        alert.setView(mSubjectView);

        /*
         * LinearLayout lila1 = new LinearLayout(getActivity());
         * lila1.setOrientation(1); // 1 is for vertical orientation final
         * TextView titleText = new TextView(getActivity()); final EditText
         * input = new EditText(getActivity());
         * titleText.setText(" Please enter the group name"); InputFilter[]
         * filterArray = new InputFilter[1]; filterArray[0] = new
         * InputFilter.LengthFilter(maxLength); input.setFilters(filterArray);
         * lila1.addView(titleText); lila1.addView(input); alert.setView(lila1);
         */
        alert.setTitle("Group Title");
        alert.setCancelable(false);

        alert.setPositiveButton("Ok", new DialogInterface.OnClickListener() {
            public void onClick(DialogInterface dialog, int whichButton) {
                setmSubjectGroupChat(input.getText().toString().trim());
                String subject = getmSubjectGroupChat();
                String radioButtonText = "";
                if (subject.equals("")) {
                    subject = getParticipantsName(mParticipantList
                            .toArray(new Participant[1]));
                }
                Logger.d(TAG, "showSubjectDialog subject:" + subject);
                if(JoynServiceConfiguration.isClosedGroupSupported(getActivity().getApplicationContext())){
                    int selectOptId = radioOptionGrp.getCheckedRadioButtonId();
                    RadioButton radioOptBtn = (RadioButton) mSubjectView.findViewById(selectOptId);
                    radioButtonText = radioOptBtn.getText().toString();
                    Logger.d(TAG, "showSubjectDialog radioButtonText:" + radioButtonText);
                }
                if(radioButtonText.equals("Closed")){
                    Logger.d(TAG, "showSubjectDialog radio text is Closed");
                    setIsClosedGroupChat(true);
                    Message controllerMessage = ControllerImpl.getInstance()
                    .obtainMessage(ChatController.ADD_CLOSED_GROUP_SUBJECT, mTag,
                            subject);
                    controllerMessage.arg1 = 1;
                    controllerMessage.sendToTarget();
                } else {
                Message controllerMessage = ControllerImpl.getInstance()
                        .obtainMessage(ChatController.ADD_GROUP_SUBJECT, mTag,
                                subject);
                controllerMessage.arg1 = 1;
                controllerMessage.sendToTarget();
                }
                updateChatUi();

                if (((ChatScreenActivity) getActivity()).isUrlShare == true) {
                    // send this message for share text only
                    String message = ((ChatScreenActivity) getActivity()).url;
                    int messageTag = onSentMessage(message);

                    ControllerImpl controller = ControllerImpl.getInstance();
                    Message controllerMessage = controller.obtainMessage(
                            ChatController.EVENT_SEND_MESSAGE,
                            ((ChatScreenActivity) getActivity()).chat
                                    .getChatTag(), message);
                    controllerMessage.arg1 = messageTag;
                    controllerMessage.sendToTarget();
                }

                GroupChatFragment.this.updateEmoticonLayout(false,
                        GroupChatFragment.this.mSubjectView);

            }
        });
        alert.setNegativeButton("Cancel",
                new DialogInterface.OnClickListener() {
                    public void onClick(DialogInterface dialog, int whichButton) {
                        Logger.d(TAG, "showSubjectDialog cancel click");
                        Message controllerMessage = ControllerImpl
                                .getInstance().obtainMessage(
                                        ChatController.EVENT_CLOSE_WINDOW,
                                        mTag, null);

                        controllerMessage.sendToTarget();
                        dialog.cancel();
                    }
                });
        alert.create();
        alert.show();
    }

    @Override
    public void onCreateContextMenu(ContextMenu menu, View v,
            ContextMenuInfo menuInfo) {
        Logger.d(TAG, "onCreateContextMenu entry");
        try {
            super.onCreateContextMenu(menu, v, menuInfo);
            AdapterView.AdapterContextMenuInfo info = (AdapterView.AdapterContextMenuInfo) menuInfo;
            menu.setHeaderTitle("Options");
            // get the postion of the elemet that is clicked
            mItemIDPosition = info.position;
            Logger.d(TAG, "onCreateContextMenu position:" + mItemIDPosition);
            IChatMessage chatMessage = mMessageList.get(mItemIDPosition - 1);
            Logger.d(TAG, "onCreateContextMenu chatMessage:" + chatMessage);
            MenuInflater inflater = getActivity().getMenuInflater();

            if (chatMessage instanceof SentMessage
                    && ((SentMessage) chatMessage).getStatus().equals(
                            Status.FAILED)) {
                inflater.inflate(R.menu.chatmessagewithresent, menu);
            } else if (chatMessage instanceof DateMessage) {
                Logger.d(TAG, "onCreateContextMenu DateMessage: ");
                // Don't Inflate menu
            } else if (chatMessage instanceof FileTransfer) {
                com.mediatek.rcse.interfaces.ChatView.IFileTransfer.Status status =
                        ((FileTransfer) chatMessage)
                        .getStatue();
                /*
                 * if(status.equals(com.mediatek.rcse.interfaces.ChatView.
                 * IFileTransfer.Status.CANCEL) ||
                 * status.equals(com.mediatek.rcse
                 * .interfaces.ChatView.IFileTransfer.Status.CANCELED) ||
                 * status.
                 * equals(com.mediatek.rcse.interfaces.ChatView.IFileTransfer
                 * .Status.FAILED) ) { inflater.inflate(R.menu.chatmessagemenu,
                 * menu); } else { inflater.inflate(R.menu.chatmessagemenu,
                 * menu); }
                 */
                if (chatMessage instanceof SentFileTransfer) {
                    inflater.inflate(R.menu.chatmessagemenuwithstatus, menu);
                } else {
                    inflater.inflate(R.menu.chatmessagemenu, menu);
                }
            } else if (chatMessage instanceof SentMessage) {
                Logger.d(TAG, "onCreateContextMenu SentMessage: ");
                inflater.inflate(R.menu.chatmessagemenuwithstatus, menu);
            } else {
                inflater.inflate(R.menu.chatmessagemenu, menu);
            }
        } catch (Exception e) {
            e.printStackTrace();
        }

    }

    @Override
    public boolean onContextItemSelected(MenuItem item) {
        Logger.d(TAG, "onContextItemSelected entry position:" + mItemIDPosition);
        if(mItemIDPosition <= 0 || mItemIDPosition > mMessageList.size()) {
            Logger.d(TAG, "onContextItemSelected return false");
            return false;
        }
        IChatMessage chatMessage = mMessageList.get(mItemIDPosition - 1);
        Logger.d(TAG, "onContextItemSelected chatMessage:" + chatMessage);
        if (chatMessage == null) {
            return false;
        }
        switch (item.getItemId()) {

        case R.id.delete:
            mMessageAdapter.removeMessage(mItemIDPosition - 1);
            mMessageSequenceOrder = mMessageSequenceOrder - 1;
            Message controllerMessage = null;
            if(chatMessage.getId().equals("-1")){
                controllerMessage = ControllerImpl.getInstance()
                        .obtainMessage(ChatController.EVENT_DELETE_MESSAGE_UNREG_MESSAGES, mTag,
                                chatMessage.getId());
                int messageTag = 0;
                if (chatMessage instanceof SentMessage) {
                    messageTag = ((SentMessage) chatMessage).getMessageTag();
                }
                controllerMessage.arg1 = messageTag;
            } else{
                controllerMessage = ControllerImpl.getInstance()
                    .obtainMessage(ChatController.EVENT_DELETE_MESSAGE, mTag,
                            chatMessage.getId());
            }
            controllerMessage.sendToTarget();
            break;

        case R.id.info: {
            MessageInfoDialog infoDialog = new MessageInfoDialog(new Date(),
                    "", "");
            if (chatMessage instanceof SentMessage) {
                infoDialog = new MessageInfoDialog(
                        ((SentMessage) chatMessage).getMessageDate(),
                        ((SentMessage) chatMessage).getMessageText(), "");
            } else if (chatMessage instanceof ReceivedMessage) {
                infoDialog = new MessageInfoDialog(
                        ((ReceivedMessage) chatMessage).getMessageDate(),
                        ((ReceivedMessage) chatMessage).getMessageText(),
                        ((ReceivedMessage) chatMessage).getMessageSender());
            }

            infoDialog.show(getFragmentManager(), "RepickDialog");
        }

            break;

        case R.id.resend:

            if (chatMessage instanceof SentMessage) {
                int messageTag = ((SentMessage) chatMessage).getMessageTag();
                controllerMessage = ControllerImpl.getInstance().obtainMessage(
                        ChatController.EVENT_SEND_MESSAGE, mTag,
                        ((SentMessage) chatMessage).getMessageText());
                controllerMessage.arg1 = messageTag;
                controllerMessage.sendToTarget();
            }

            break;

        case R.id.block:
            if (chatMessage instanceof FileTransfer) {
                // mark future requests for this contact as spam

            }

            break;

        case R.id.status:
            if (chatMessage instanceof SentMessage) {
                Logger.d(TAG, "show status dialog SentMessage ");
                showStatusDialog(getActivity(), chatMessage);
            } else if (chatMessage instanceof SentFileTransfer) {
                Logger.d(TAG, "show status dialog SentFileTransfer ");
                showStatusDialog(getActivity(), chatMessage);
            }
            break;
        default:
             break;

        }
        return true;
    }

    /**
     * Show status dialog.
     *
     * @param context the context
     * @param chatMessage the chat message
     */
    public void showStatusDialog(Context context, IChatMessage chatMessage) {

        Logger.d(TAG, "show status dialog enter " + chatMessage);
        Intent intent = new Intent(InvitationDialog.ACTION);

        if (chatMessage instanceof GroupChatFragment.SentMessage) {
            GroupChatFragment.SentMessage message = (GroupChatFragment.SentMessage) chatMessage;
            intent.putExtra(InvitationDialog.KEY_STRATEGY,
                    InvitationDialog.STRATEGY_GROUP_CHAT_MSG_RECEIVED_STATUS);
            intent.putExtra(
                    "statusmap",
                    ((GroupChatFragment.SentMessage) chatMessage).mMessageStatusMap);
        } else if (chatMessage instanceof GroupChatFragment.SentFileTransfer) {
            intent.putExtra(InvitationDialog.KEY_STRATEGY,
                    InvitationDialog.STRATEGY_GROUP_FILE_VIEW_STATUS);
            intent.putExtra(
                    "statusmap",
                    ((GroupChatFragment.SentFileTransfer) chatMessage).mFileStatusMap);

        } else {
            Logger.d(TAG, "mMessage not instance of SentMessage ");
        }
        context.startActivity(intent);

    }

    /**
     * The Class MessageInfoDialog.
     */
    public static class MessageInfoDialog extends DialogFragment implements
            DialogInterface.OnClickListener {
        private static final String TAG = "RepickDialog";
        private int mRequestCode = 0;
        String mDate;
        String mText;
        String mParticipant;

        /**
         * Instantiates a new message info dialog.
         *
         * @param date the date
         * @param text the text
         * @param participant the participant
         */
        public MessageInfoDialog(Date date, String text, String participant) {
            mDate = date.toString();
            mText = text;
            if (participant.equals("")) {
                mParticipant = "You";
            } else {
                mParticipant = Utils.extractNumberFromUri(participant);
            }

        }

        @Override
        public Dialog onCreateDialog(Bundle savedInstanceState) {
            final AlertDialog alertDialog;
            alertDialog = new AlertDialog.Builder(getActivity(),
                    AlertDialog.THEME_HOLO_LIGHT).create();
            alertDialog.setTitle(mParticipant);
            StringBuilder mInfoMessage = new StringBuilder();
            String newline = System.getProperty("line.separator");
            mInfoMessage.append("Message :" + mText);
            mInfoMessage.append(newline);
            mInfoMessage.append("Date :" + mDate);
            alertDialog.setMessage(mInfoMessage);
            alertDialog.setButton(DialogInterface.BUTTON_POSITIVE,
                    getString(R.string.rcs_dialog_positive_button), this);
            return alertDialog;
        }

        @Override
        public void onClick(DialogInterface dialog, int which) {
            Logger.i(TAG, "onClick() which is " + which);
            this.dismissAllowingStateLoss();
        }
    }

    @Override
    public void onDestroy() {
        Logger.d(TAG, "onDestroy");
        mPreMessageMap.clear();
        mPreviousDate = null;
        // clear extra messages from group chat adapter
        Message controllerMessage = ControllerImpl.getInstance()
                .obtainMessage(
                        ChatController.EVENT_CLEAR_EXTRA_MESSAGE_GROUP_CHAT,
                        mTag, null);
        controllerMessage.sendToTarget();
        super.onDestroy();
    }

    @Override
    public void onDetach() {
        mActivity = null;
        super.onDetach();
    }

    @Override
    protected void onSend(String message) {
        Logger.d(TAG, "onSend() The message is " + message);
        int messageTag = onSentMessage(message);
        Message controllerMessage = ControllerImpl.getInstance().obtainMessage(
                ChatController.EVENT_SEND_MESSAGE, mTag, message);
        controllerMessage.arg1 = messageTag;
        Bundle bundle = new Bundle();
        bundle.putString("subject", mSubjectGroupChat);
        controllerMessage.setData(bundle);
        controllerMessage.sendToTarget();
    }

    /**
     * On sent message.
     *
     * @param content the content
     * @return the int
     */
    private int onSentMessage(String content) {
        int messageTag = Utils.RANDOM.nextInt(Integer.MAX_VALUE);
        ChatMessage message = new ChatMessage("", Utils.DEFAULT_REMOTE,
                content, new Date(), true, RcsSettings.getInstance()
                        .getJoynUserAlias());
        /*Date date = new Date();
        ChatMessage message = new ChatMessage("", Utils.DEFAULT_REMOTE,
                content, null, Utils.DEFAULT_REMOTE, RcsService.Direction.OUTGOING.toInt(), date.getTime(), date.getTime(), 0L, 0L,
                ChatLog.Message.Content.Status.SENDING.toInt() , ChatLog.Message.Content.ReasonCode.UNSPECIFIED.toInt(), null, true, false);*/
        // message.setDate(new Date());
        ISentChatMessage sentChatMessage = addSentMessage(message, messageTag);
        if (sentChatMessage != null) {
            mPreMessageMap.put(messageTag, (SentMessage) sentChatMessage);
        }
        return messageTag;
    }

    /**
     * Handle show reminder.
     *
     * @param reminder the reminder
     * @return true, if successful
     */
    protected boolean handleShowReminder(String reminder) {
        Logger.d(TAG, "handleShowReminder() reminder is " + reminder);
        if (SHOW_REJOING_MESSAGE_REMINDER.equals(reminder)) {
            if (mRejoiningText != null) {
                mRejoiningText.setVisibility(View.VISIBLE);
                super.handleClearReminder();
            } else {
                Logger.e(TAG, "handleShowReminder() the mRejoiningText is null");
            }
            return true;
        } else {
            if (mRejoiningText != null) {
                mRejoiningText.setVisibility(View.GONE);
            } else {
                Logger.e(TAG, "handleClearReminder() mTypingText is null");
            }
            return super.handleShowReminder(reminder);
        }
    }

    /**
     * Handle clear reminder.
     */
    protected void handleClearReminder() {
        Logger.d(TAG, "handleClearReminder() entry");
        if (mRejoiningText != null) {
            mRejoiningText.setVisibility(View.GONE);
        } else {
            Logger.e(TAG, "showReminderList() mRejoiningText is null");
        }
        super.handleClearReminder();
    }

    /**
     * Adds the received message.
     *
     * @param message the message
     * @param isRead the is read
     * @return the i received chat message
     */
    @Override
    public IReceivedChatMessage addReceivedMessage(ChatMessage message,
            boolean isRead) {
        Logger.d(TAG, "addReceivedMessage() mIsBottom: " + mIsBottom
                + " message: " + message.getContent() + " isRead: " + isRead);
        if (message != null) {
            final ReceivedMessage msg = new ReceivedMessage(message);
            Date date = message.getReceiptDate();
            if (mMessageAdapter == null) {
                Logger.d(TAG, "addReceivedMessage mMessageAdapter is null");
                return null;
            }
            addMessageDate(date);
            if (!mIsBottom) {
                String remote = ((ReceivedMessage) msg).getMessageSender();
                Logger.d(TAG, "addReceivedMessage() mIsBottom: " + mIsBottom
                        + " remote: " + remote + " mParticipantList: "
                        + mParticipantList);
                if (remote != null) {
                    remote = Utils.extractNumberFromUri(remote);
                    String name = null;
                    if (mParticipantList != null) {
                        for (Participant participant : mParticipantList) {
                            Logger.d(TAG,
                                    "participant  = " + participant.toString());
                            if (participant.getContact().equals(remote)) {
                                name = participant.getDisplayName();
                                break;
                            }
                        }
                        Logger.d(TAG, "addReceivedMessage  the remote name is "
                                + name);
                        Thread currentThread = Thread.currentThread();
                        if (THREAD_ID_MAIN == currentThread.getId()) {
                            mMessageReminderText.setText(SPACE);
                            if (name != null) {
                                mMessageReminderText.append(name);
                            } else {
                                if (getActivity() != null) {
                                    mMessageReminderText
                                            .append(getResources()
                                                    .getString(
                                                            R.string.group_chat_stranger));
                                }
                                Logger.w(TAG,
                                        "name is null, so append stranger getActivity: "
                                                + getActivity());
                            }
                            mMessageReminderText.append(COLON);
                            String rcvMsg = ((ReceivedMessage) msg)
                                    .getMessageText();
                            if (rcvMsg != null) {
                                mMessageReminderText.append(rcvMsg);
                            }
                            mTextReminderSortedSet
                                    .add(SHOW_NEW_MESSAGE_REMINDER);
                            showReminderList();
                        } else {
                            final String contactName = name;
                            mUiHandler.post(new Runnable() {
                                @Override
                                public void run() {
                                    mMessageReminderText.setText(SPACE);
                                    if (contactName != null) {
                                        mMessageReminderText
                                                .append(contactName);
                                    }
                                    mMessageReminderText.append(COLON);
                                    String rcvMsg = ((ReceivedMessage) msg)
                                            .getMessageText();
                                    if (rcvMsg != null) {
                                        mMessageReminderText.append(rcvMsg);
                                    }
                                    mTextReminderSortedSet
                                            .add(SHOW_NEW_MESSAGE_REMINDER);
                                    showReminderList();
                                }
                            });
                        }
                        mIsNewMessageNotify = Boolean.TRUE;
                    }
                }
            }
            addMessageAtomic(msg);
            return msg;
        } else {
            Logger.d(TAG, "The received chat message is null");
            return null;
        }
    }

    /**
     * Sets the file transfer enable.
     *
     * @param status the new file transfer enable
     */
    @Override
    public void setFileTransferEnable(final int status) {
        Logger.d(TAG, " setFileTransferEnable() entry " + status);
        mFiletransferEnableStatus = status;
        if (mContentView != null) {
            mUiHandler.post(new Runnable() {
                @Override
                public void run() {
                    ImageButton btnAddView = (ImageButton) mContentView
                            .findViewById(R.id.btn_chat_add);
                    ApiManager manager = ApiManager.getInstance();
                    if (null != manager) {
                        switch (mFiletransferEnableStatus) {
                        case GroupChat1.FILETRANSFER_DISABLE_REASON_CAPABILITY_FAILED:
                        case GroupChat1.FILETRANSFER_DISABLE_REASON_NOT_REGISTER:
                        case GroupChat1.FILETRANSFER_DISABLE_REASON_REMOTE:
                        case GroupChat1.GROUPFILETRANSFER_DISABLE:
                            btnAddView.setAlpha(ALPHA_VALUE_DISABLE);
                            btnAddView.setClickable(false);
                            btnAddView.setFocusable(false);
                            btnAddView.setEnabled(false);
                            break;
                        case GroupChat1.FILETRANSFER_ENABLE_OK:
                            btnAddView.setAlpha(ALPHA_VALUE_ENABLE);
                            btnAddView.setClickable(true);
                            btnAddView.setFocusable(true);
                            btnAddView.setEnabled(true);
                            btnAddView
                                    .setOnClickListener(new View.OnClickListener() {
                                        public void onClick(View v) {
                                            // queryCapablility();
                                            onAddAttachment();
                                        }

                                    });
                            break;
                        default:
                            break;
                        }
                    } else {
                        Logger.e(TAG,
                                "setFileTransferEnable() the manager is null");
                    }
                }
            });
        } else {
            Logger.e(TAG, "setFileTransferEnable the btnAddView is null");
        }
    }

    /**
     * Adds the received file transfer.
     *
     * @param file the file
     * @param isAutoAccept the is auto accept
     * @return the i file transfer
     */
    @Override
    public IFileTransfer addReceivedFileTransfer(FileStruct file,
            boolean isAutoAccept, boolean isRead) {
        Logger.d(TAG, "addFileTransferInvitation() file: " + file
                + " mRemoteIsRcse: " + "TODO" + " mIsBottom: " + mIsBottom
                + " mMessageAdapter: " + mMessageAdapter);
        FileTransfer fileTransfer = null;
        if (file == null) {
            return null;
        }
        fileTransfer = new ReceivedFileTransfer(file, isAutoAccept);
        /*
         * if (!mRemoteIsRcse) { mRemoteIsRcse = true; final String number =
         * mParticipant.getContact(); Thread currentThread =
         * Thread.currentThread(); if (THREAD_ID_MAIN == currentThread.getId())
         * { Logger.w(TAG, "addReceivedFileTransfer  the currentThread is " +
         * currentThread.getId()); if
         * (ContactsListManager.getInstance().isLocalContact(number)) {
         * showAsLocal(number); } else if
         * (ContactsListManager.getInstance().isStranger(number)) {
         * showAsStranger(number); } else {
         * ContactsListManager.getInstance().setStrangerList(number, true);
         * showAsStranger(number); } } else { mUiHandler.post(new Runnable() {
         *
         * @Override public void run() { Logger.w(TAG,
         * "addReceivedFileTransfer  it is mUiHandler"); if
         * (ContactsListManager.getInstance().isLocalContact(number)) {
         * showAsLocal(number); } else if
         * (ContactsListManager.getInstance().isStranger(number)) {
         * showAsStranger(number); } else {
         * ContactsListManager.getInstance().setStrangerList(number, true);
         * showAsStranger(number); } } }); } }
         */
        Date date = file.mDate;
        addMessageAtomic(fileTransfer);

        if (null != fileTransfer && mMessageAdapter != null) {
            ((ReceivedFileTransfer) fileTransfer).setTag(mTag);
            if (!mIsBottom) {
                Thread currentThread = Thread.currentThread();
                if (mMessageReminderText != null) {

                    final String display = ((ReceivedFileTransfer) fileTransfer)
                            .getFileStruct().mName
                            + SPACE
                            + getString(R.string.file_transfer_titile_after);
                    if (THREAD_ID_MAIN == currentThread.getId()) {
                        Logger.w(TAG,
                                "addReceivedMessage  the currentThread is "
                                        + currentThread.getId());
                        mMessageReminderText.setText(display);
                        mTextReminderSortedSet.add(SHOW_NEW_MESSAGE_REMINDER);
                        showReminderList();
                    } else {
                        mUiHandler.post(new Runnable() {
                            @Override
                            public void run() {
                                Logger.w(TAG,
                                        "addReceivedMessage  it is mUiHandler");
                                mMessageReminderText.setText(display);
                                mTextReminderSortedSet
                                        .add(SHOW_NEW_MESSAGE_REMINDER);
                                showReminderList();
                            }
                        });
                    }
                    mIsNewMessageNotify = true;
                }
            }
        }
        return fileTransfer;
    }

    /**
     * The Class SentFileTransfer.
     */
    public class SentFileTransfer extends FileTransfer {
        public ConcurrentHashMap<String, Integer> mFileStatusMap =
                new ConcurrentHashMap<String, Integer>();

        /**
         * Instantiates a new sent file transfer.
         *
         * @param fileStruct the file struct
         */
        public SentFileTransfer(FileStruct fileStruct) {
            super(fileStruct);
            mStatus = Status.PENDING;
            for (Participant participant : mParticipantList) {
                mFileStatusMap.put(participant.getContact(),
                        Status.PENDING.ordinal());
            }
        }

        /**
         * Sets the status.
         *
         * @param status the status
         * @param contact the contact
         */
        public void setStatus(Status status, String contact) {
            Logger.d(TAG, "SentFileTransfer,setStatus()  status: " + status
                    + "contact :" + contact + " name:" + mFileStruct.mName);
            // update the map
            synchronized (mFileStatusMap) {
                mFileStatusMap.put(contact, status.ordinal());
            }
        }

    }

    /**
     * The Class ReceivedFileTransfer.
     */
    public class ReceivedFileTransfer extends FileTransfer {
        /**
         * Instantiates a new received file transfer.
         *
         * @param fileStruct the file struct
         * @param isAutoAccept the is auto accept
         */
        public ReceivedFileTransfer(FileStruct fileStruct, boolean isAutoAccept) {
            super(fileStruct);
            if (isAutoAccept) {
                mStatus = Status.TRANSFERING;
            }

        }

    }

    /**
     * Adds the sent file transfer.
     *
     * @param file the file
     * @return the i file transfer
     */
    @Override
    public IFileTransfer addSentFileTransfer(FileStruct file) {
        Logger.d(TAG, "addSentFileTransfer()  file: " + file);
        Object fileTransferTag = file.mFileTransferTag;
        SentFileTransfer fileTransfer = (SentFileTransfer) mPreFileTransferMap
                .get(fileTransferTag);
        if (null != fileTransfer) {
            Logger.d(TAG, "addSentFileTransfer()  fileTransfer with tag: "
                    + fileTransferTag + " found");
            mPreFileTransferMap.remove(fileTransferTag);
        } else {
            Logger.d(TAG, "addSentFileTransfer()  fileTransfer with tag: "
                    + fileTransferTag + " not found");
            fileTransfer = new SentFileTransfer(file);
            if (null != fileTransfer && mMessageAdapter != null) {
                fileTransfer.setTag(this.getChatFragmentTag());
                Date date = file.mDate;
                addMessageAtomic(fileTransfer);
            } else {
                Logger.e(TAG, "addSentFileTransfer() fileTransfer is null");
            }
        }
        return fileTransfer;
    }

    /**
     * Adds the sent message.
     *
     * @param message the message
     * @param messageTag the message tag
     * @return the i sent chat message
     */
    @Override
    public ISentChatMessage addSentMessage(ChatMessage message, int messageTag) {
        Logger.d(TAG, "addSentMessage(), message: " + message);
        if (message != null) {
            SentMessage msg = (SentMessage) mPreMessageMap.get(messageTag);
            if (null == msg) {
                msg = new SentMessage(message);
                Date date = message.getReceiptDate();
                if (mMessageAdapter == null) {
                    Logger.d(TAG, "addSentMessage mMessageAdapter is null");
                    return null;
                }
                addMessageDate(date);
                addMessageAtomic(msg);
                return msg;
            } else {
                String messageRemote = message.getRemoteContact().toString();
                Logger.d(TAG, "addSentMessage(), messageRemote: "
                        + messageRemote);
                if (!Utils.DEFAULT_REMOTE.equals(messageRemote)) {
                     if(message.getId()!= null && !message.getId().equals("-1"))
                     {
                    mPreMessageMap.remove(messageTag);
                    msg.updateMessage(message);
                     }
                    return msg;
                }
            }
        }
        return null;
    }

    /*
     * private interface IChatMessage extends IChatWindowMessage { Date
     * getMessageDate(); }
     */

    /**
     * The Class DateMessage.
     */
    public static class DateMessage implements IChatMessage {
        private Date mDate = null;

        /**
         * Instantiates a new date message.
         *
         * @param date the date
         */
        public DateMessage(Date date) {
            mDate = (Date) date.clone();
        }

        /**
         * Gets the id.
         *
         * @return the id
         */
        @Override
        public String getId() {
            // TODO Auto-generated method stub
            return null;
        }

        /**
         * Gets the message date.
         *
         * @return the message date
         */
        @Override
        public Date getMessageDate() {
            // TODO Auto-generated method stub
            return (Date) mDate.clone();
        }

        /**
         * Equals.
         *
         * @param o the o
         * @return true, if successful
         */
        @Override
        public boolean equals(Object o) {
            if (o instanceof DateMessage) {
                return mDate.equals(((DateMessage) o).mDate);
            } else {
                return false;
            }

        }

        /**
         * Hash code.
         *
         * @return the int
         */
        @Override
        public int hashCode() {
            return mDate.hashCode();
        }

    }

    /**
     * Add message date in the ListView.Messages sent and received in the same
     * day have only one date.
     *
     * @param date
     *            The date of the messages. Each date stands for a section in
     *            fastScroll.
     */
    public void addMessageDate(Date date) {
        Logger.d(TAG, "updateDateList() entry, the size of dateList is "
                + mDateList.size() + " and the date is " + date);
        SimpleDateFormat dateFormat = new SimpleDateFormat(DATE_FORMAT_STRING);
        String currentDate = dateFormat.format(date);
        Logger.d(TAG, "currentDate is " + currentDate);
        Date dateMsg = new Date(date.getYear(), date.getMonth(),
                date.getDate(), 0, 0, 0);
        if (mPreviousDate == null || !mPreviousDate.equals(currentDate)) {
            if (mMessageAdapter == null) {
                Logger.d(TAG, "addMessageDate mMessageAdapter is null");
                return;
            }
            if (mMessageList == null) {
                Logger.d(TAG, "addMessageDate mMessageAdapter is null");
                return;
            }
            synchronized (mLock) {
                DateMessage dateMessage = new DateMessage(dateMsg);
                mMessageSequenceOrder = mMessageSequenceOrder + 1;
                int position = mMessageSequenceOrder;
                mMessageAdapter.addMessage(date, position);
                mDateList.add(currentDate);
                mMessageList.add(position, dateMessage);
            }
        }
        mPreviousDate = currentDate;
    }

    /**
     * Removes the all messages.
     */
    @Override
    public void removeAllMessages() {
        Logger.d(TAG, "removeAllMessages() entry, mMessageAdapter: "
                + mMessageAdapter);
        if (mMessageAdapter != null) {
            mMessageAdapter.removeAllItems();
        }
        mUiHandler.post(new Runnable() {
            @Override
            public void run() {
                mMessageList.clear();
                mDateList.clear();
            }
        });
        mPreviousDate = null;
        mMessageSequenceOrder = -1;
        mCurrentNumberMsgLoaded = 0;
        Logger.d(TAG, "removeAllMessages exit");
    }

    /**
     * Gets the sent chat message.
     *
     * @param messageId the message id
     * @return the sent chat message
     */
    @Override
    public IChatWindowMessage getSentChatMessage(String messageId) {
        for (IChatWindowMessage message : mMessageList) {
            if (message.getId().equals(messageId)) {
                return message;
            }
        }
        return null;
    }

    /**
     * Close group chat.
     */
    public void closeGroupChat() {
        // find out current window? TODO
        // only this participant remains in chat, others have left, chat window
        // should close
        Message controllerMessage = ControllerImpl.getInstance().obtainMessage(
                ChatController.EVENT_QUIT_GROUP_CHAT, mTag, null);

        controllerMessage.sendToTarget();
        mUiHandler.post(new Runnable() {

            public void run() {
                removeChatUi();
                hideSoftKeyboard();

            }
        });

        // show a reminder that group chat need to end due to all participant
        // has left
        showAllParticipantLeftReminder();
    }

    /**
     * Show all participant left reminder.
     */
    public void showAllParticipantLeftReminder() {
        Logger.v(TAG,
                "showAllParticipantLeftReminder() mGroupParticipantsLeftText: "
                        + mGroupParticipantsLeftText + " getActivity: "
                        + getActivity());
        if (mGroupParticipantsLeftText != null) {
            if (getActivity() != null) {

                final String groupParticipantsLeftText = getResources()
                        .getString(R.string.label_all_participants_left);
                mUiHandler.post(new Runnable() {
                    @Override
                    public void run() {
                        mGroupParticipantsLeftText
                                .setText(groupParticipantsLeftText);
                        mTextReminderSortedSet.add(SHOW_ALL_PARTICIPANTS_LEFT);
                        showReminderList();
                    }
                });

            }

        }
    }

    /**
     * Hide soft keyboard.
     */
    public void hideSoftKeyboard() {
        InputMethodManager inputMethodManager = (InputMethodManager) getActivity()
                .getSystemService(Context.INPUT_METHOD_SERVICE);
        Logger.d(TAG, "hideSoftKeyboard(), inputMethodManager is: "
                + inputMethodManager);
        if (inputMethodManager != null) {
            View view = getActivity().getCurrentFocus();
            Logger.d(TAG, "hideSoftKeyboard(), view is: " + view);
            if (view != null) {
                IBinder binder = view.getWindowToken();
                inputMethodManager.hideSoftInputFromWindow(binder,
                        InputMethodManager.HIDE_NOT_ALWAYS);
            }
        }
    }

    /**
     * Update participants.
     *
     * @param participantList the participant list
     */
    @Override
    public void updateParticipants(final List<ParticipantInfo> participantList) {
        Logger.d(TAG, "updateParticipants entry:" + participantList);
        if (mActivity == null) {
            Logger.d(TAG, "updateParticipants mActivity is null");
            return;
        }
        mActivity.runOnUiThread(new Runnable() {
            @Override
            public void run() {
                ArrayList<Participant> participants = new ArrayList<Participant>();
                String blockedParticipants = null;
                boolean flag = true;
                ParticipantInfo tempInfo = null;
                mGroupChatParticipantList.clear();
                mGroupChatParticipantList.addAll(participantList);
                if (mGroupChatParticipantList.size() <= 0) { // zero size
                    Logger.d(TAG, "updateParticipants entry size: "
                            + mGroupChatParticipantList.size());
                    // only this participant remains in chat, others have left,
                    // chat window should close
                    closeGroupChat();
                }
                Logger.d(TAG,
                        "updateParticipants() mGroupChatParticipantList: "
                                + mGroupChatParticipantList);
                for (ParticipantInfo info : mGroupChatParticipantList) {
                    participants.add(info.getParticipant());
                }
                Logger.d(TAG, "updateParticipants() participants: "
                        + participants);
                mParticipantList = participants;
                updateChatUi();
                if (Logger.getIsIntegrationMode()
                        && mActivity instanceof PluginGroupChatActivity) {
                    ((PluginGroupChatActivity) mActivity)
                            .updateParticipants(mParticipantList);
                } else {
                    Logger.d(
                            TAG,
                            "updateParticipants() it is not integration mode or " +
                            "not instace of PluginGroupChatActivity.");
                }

                // if blocked contact is added, update top reminder
                try {
                ApiManager instance = ApiManager.getInstance();
                if (instance == null) {
                    Logger.i(TAG, "ApiManager instance is null");
                } else {
                        ContactService contacsApi = instance.getContactsApi();
                    if (contacsApi == null) {
                        Logger.d(TAG, "contacsApi instance is null");
                    } else {
                            List<String> blockList = contacsApi
                                    .getImBlockedContactsFromLocal();
                            if(blockList != null) {
                for (Participant participant : mParticipantList) {
                    if (blockList.contains(participant.getContact())) {
                        // blockedParticipantsGroup.add(participant);
                        blockedParticipants = blockedParticipants + " "
                                + participant.getDisplayName();
                    }
                }
                            }
                if (mBlockedParticipantText != null
                        && blockedParticipants != null) {
                                mBlockedParticipantText
                                        .setText("Blocked participant"
                                                + blockedParticipants
                                                + " is added to Group chat");
                    mTopReminderSortedSet
                            .add(SHOW_BLOCKED_PARTICIPANT_MESSAGE_REMINDER);
                    showTopReminder();
                } else {
                                Logger.e(TAG,
                                        " mGroupParticipantsLeftText is null!");
                            }
                }
                    }
                } catch (Exception e) {
                    e.printStackTrace();
                }
            }
        });
        Logger.d(TAG, "updateParticipants exit");
    }

    /**
     * Notify controller participants has been added.
     *
     * @param participantList
     *            The participants to be added.
     */
    public void addParticipants(List<Participant> participantList) {
        Logger.d(TAG, "addParticipants entry");
        ControllerImpl controller = ControllerImpl.getInstance();
        Message controllerMessage = controller.obtainMessage(
                ChatController.EVENT_GROUP_ADD_PARTICIPANT, mTag,
                participantList);
        controllerMessage.sendToTarget();
    }

    /**
     * Sets the is composing.
     *
     * @param isComposing the is composing
     * @param participant the participant
     */
    @Override
    public void setIsComposing(boolean isComposing,
            final Participant participant) {
        Logger.v(TAG, "setIsComposing status is " + isComposing
                + "participant is" + participant);
        if (mTypingText != null) {
            if (isComposing) {
                if (participant != null) {
                    mParticipantComposList.add(participant);
                    mUiHandler.post(new Runnable() {
                        @Override
                        public void run() {
                            showComposingInformation();
                        }
                    });
                } else {
                    Logger.e(TAG, "setIsComposing the participant is null");
                }
            } else {
                if (participant != null) {
                    int listSize = mParticipantComposList.size();
                    if (listSize > 0) {
                        mParticipantComposList.remove(participant);
                        mUiHandler.post(new Runnable() {
                            @Override
                            public void run() {
                                showComposingInformation();
                            }
                        });
                    } else {
                        Logger.e(TAG, "setIsComposing false the listSize is "
                                + listSize);
                        mUiHandler.post(new Runnable() {
                            @Override
                            public void run() {
                                mTextReminderSortedSet
                                        .remove(SHOW_IS_TYPING_REMINDER);
                                showReminderList();
                            }
                        });
                    }
                } else {
                    Logger.e(TAG, "setIsComposing false participant is null");
                    mUiHandler.post(new Runnable() {
                        @Override
                        public void run() {
                            mTextReminderSortedSet
                                    .remove(SHOW_IS_TYPING_REMINDER);
                            showReminderList();
                        }
                    });
                }
            }
        } else {
            Logger.e(TAG, "setIsComposing the typingtext is null");
        }
    }

    /**
     * Show composing information.
     */
    private void showComposingInformation() {
        String moreTyping = getResources().getString(
                R.string.label_contact_imore_composing);
        ArrayList<Participant> tmpList = new ArrayList<Participant>(
                mParticipantComposList);
        int listSize = tmpList.size();
        Logger.w(TAG, "setIsComposing + listSize" + listSize);
        if (listSize == 0) {
            mTextReminderSortedSet.remove(SHOW_IS_TYPING_REMINDER);
            showReminderList();
        } else if (listSize == 1) {
            Participant participant = tmpList.get(ZERO_PARTICIPANT);
            final String isTyping = getResources().getString(
                    R.string.label_contact_is_composing,
                    ContactsListManager.getInstance()
                            .getDisplayNameByPhoneNumber(
                                    participant.getContact()));
            mTypingText.setText(isTyping);
            mTextReminderSortedSet.add(SHOW_IS_TYPING_REMINDER);
            showReminderList();
        } else if (listSize > 1) {
            mTypingText.setText(moreTyping);
            mTextReminderSortedSet.add(SHOW_IS_TYPING_REMINDER);
            showReminderList();
        }
    }

    /**
     *   Check contact list presence status
     */
    private void checkContactListPresence() {
        Logger.d(TAG, "checkContactListPresence");
        if (mActivity == null) {
            Logger.e(TAG, "mActivity is null");
            return;
        }
        mActivity.runOnUiThread(new Runnable() {
            @Override
            public void run() {
                CapabilityService capabilityApi = ApiManager.getInstance()
                        .getCapabilityApi();
                try {
                    List<ContactId> pListParticipant = new ArrayList<ContactId>();
                    for (Participant participant : mParticipantList) {
                        pListParticipant.add(new ContactId(participant.getContact()));
                    }
                    capabilityApi.requestContactsCapabilities(pListParticipant);
                } catch (Exception e1) {
                    e1.printStackTrace();
                }
            }
        });
    }

    private List<String> convertSetToList(List<Participant> participantInfos) {
        List<String> pList = new ArrayList<String>();
        if (participantInfos == null) {
            return null;
        }
        for (Participant participant : participantInfos) {
            pList.add(participant.getContact());
        }
        return pList;
    }

    /**
     * Update chat ui.
     */
    private void updateChatUi() {
        Logger.d(TAG, "updateChatUi entry");
        Activity activity = getActivity();
        if (activity != null) {
            LayoutInflater inflater = LayoutInflater.from(activity
                    .getApplicationContext());
            View customView = inflater.inflate(
                    R.layout.group_chat_screen_title, null);
            activity.getActionBar().setCustomView(customView);
            setChatScreenTitle();
            TextView groupTitle = (TextView) activity
                    .findViewById(R.id.peer_name);
            groupTitle.setOnClickListener(this);

            RelativeLayout groupChatTitleLayout = (RelativeLayout) activity
                    .findViewById(R.id.group_chat_title_layout);
            groupChatTitleLayout.setOnClickListener(this);
            /*
             * ImageButton expandGroupChat = (ImageButton)
             * activity.findViewById(R.id.group_chat_expand);
             * expandGroupChat.setOnClickListener(this);
             */
            FragmentManager fragmentManager = getFragmentManager();
            FragmentTransaction fragmentTransaction = fragmentManager
                    .beginTransaction();
            fragmentTransaction.show(this);
            fragmentTransaction.commitAllowingStateLoss();
            fragmentManager.executePendingTransactions();
            addGroupChatMembersIcon();
            activity.invalidateOptionsMenu();
            Logger.d(TAG, "updateGroupChatUi exit");
        } else {
            Logger.w(TAG, "activity is null.");
        }
    }

    /**
     * When switch ChatFragment this method should be called to remove ui.
     */
    public void removeChatUi() {
        Activity activity = getActivity();
        if (activity != null) {
            HorizontalScrollView groupChatBannerScroller = (HorizontalScrollView) activity
                    .findViewById(R.id.group_chat_banner_scroller);
            if (groupChatBannerScroller != null) {
                Logger.v(TAG, "groupChatBannerScroller is not null");
                groupChatBannerScroller.setVisibility(View.GONE);
            } else {
                Logger.v(TAG, "groupChatBannerScroller is null.");
            }
            activity.getActionBar().setCustomView(null);
        } else {
            Logger.w(TAG, "activity is null.");
        }
    }

    /**
     * Adds the group chat members icon.
     */
    public void addGroupChatMembersIcon() {
        List<ParticipantInfo> participantInfos = new ArrayList<ParticipantInfo>(
                mGroupChatParticipantList);
        mParticipantListDisplayer.updateBanner(participantInfos);
    }

    /**
     * Set chat screen's title.
     */
    public void setChatScreenTitle() {
        Logger.v(TAG, "setChatScreenTitle()");
        Activity activity = getActivity();
        if (activity != null) {
            int num = getParticipantsNum();
            if (num > ChatFragment.ONE) {
                if (getmSubjectGroupChat().equals("")) {
                    setGroupChatTitleNumbers();
                }
            }
            TextView titleView = (TextView) activity
                    .findViewById(R.id.peer_name);
            TextView numView = (TextView) activity
                    .findViewById(R.id.peer_number);
            Logger.w(TAG, "setChatScreenTitle() num: " + num + " titleView: "
                    + titleView);
            if (titleView != null) {
                if (null != mParticipantList && mParticipantList.size() > 0) {
                    if (getmSubjectGroupChat().equals("")) {
                        titleView.setText(getParticipantsName(mParticipantList
                                .toArray(new Participant[1])));
                    } else {
                        String text = EmoticonsModelImpl.getInstance()
                        .formatMessage(mSubjectGroupChat).toString();
                        if(isClosedGroupChat()){
                            text = text + " " + "(Closed)";
                        }

                        titleView.setText(text);
                        setGroupChatTitleNumbers();
                    }
                }
            }
        }
    }

    /**
     * Gets the active user count.
     *
     * @return the active user count
     */
    private String getActiveUserCount() {
        int activeCount = 0;
        int inActiveCount = 0;
        String activeUserCount = "";
        for (int i = 0; i < mGroupChatParticipantList.size(); i++) {
            if (mGroupChatParticipantList.get(i).getState()
                    .equals(User.STATE_CONNECTED)) {
                activeCount++;
            } else {
                inActiveCount++;
            }
        }
        activeUserCount = "" + activeCount + "/"
                + mGroupChatParticipantList.size();
        return activeUserCount;
    }

    /**
     * Set group chat members's number.
     */
    private void setGroupChatTitleNumbers() {
        Logger.v(TAG, "setGroupChatTitleNumbers(),num = ");
        Activity activity = getActivity();
        if (activity != null) {
            TextView numView = (TextView) activity
                    .findViewById(R.id.peer_number);
            Logger.w(TAG, "setGroupChatTitleNumbers() numView: " + numView);
            if (numView != null) {
                String numStr = ChatFragment.OPEN_PAREN + getActiveUserCount()
                        + ChatFragment.CLOSE_PAREN;
                numView.setText(numStr);
            }
        }
    }

    /**
     * Expand group chat.
     */
    public void expandGroupChat() {
        Logger.v(TAG, "expandGroupChat() entry");
        mParticipantListDisplayer.expand();
    }

    /**
     * Collapse group chat.
     */
    public void collapseGroupChat() {
        Logger.v(TAG, "collapseGroupChat() entry");
        mParticipantListDisplayer.collapse();
    }

    /**
     * On activity result.
     *
     * @param requestCode the request code
     * @param resultCode the result code
     * @param data the data
     */
    @Override
    public void onActivityResult(int requestCode, int resultCode, Intent data) {
        super.onActivityResult(requestCode, resultCode, data);
        Logger.v(TAG, "onActivityResult() requestCode = " + requestCode
                + ",resultCode = " + resultCode + ",data = " + data);
        if (requestCode == RESULT_CODE_ADD_CONTACTS) {
            if (data != null) {
                ArrayList<Participant> participantList = ContactsListManager
                        .getInstance().parseParticipantsFromIntent(data);
                Logger.d(TAG, "onActivityResult() participantList is "
                        + participantList);
                if (participantList != null && participantList.size() != 0) {
                    participantList.addAll(0, mParticipantList);
                    if(isClosedGroupChat()){
                        //if closed group chat, start new chat, do not add in existing chat
                        Logger.d(TAG, "onActivityResult() for start new thread for closed group chat:"
                                + participantList);
                        boolean isSupportedRcsContact = true;
                        CapabilityService capabilityApi = ApiManager.getInstance()
                                .getCapabilityApi();
                        if (capabilityApi != null) {
                            try {
                                for (Participant participant : participantList) {
                                    Capabilities capabilities = capabilityApi
                                            .getContactCapabilities(ContactIdUtils.createContactIdFromTrustedData(participant.getContact()));
                                    if(capabilities == null){
                                        Logger.v(TAG, "onActivityResult capabilities is null ");
                                        isSupportedRcsContact = false;
                                    }
                                    if(isSupportedRcsContact){
                                        isSupportedRcsContact = capabilities.isImSessionSupported();
                                        Logger.d(TAG, "onActivityResult isSupportedRcsContact:" + isSupportedRcsContact + ",contact:" + participant.getContact());
                                    }
                                }
                            } catch (RcsServiceException e) {
                                Logger.d(TAG,
                                        "onActivityResult() RcsServiceException");
                                e.printStackTrace();
                            } catch (Exception e) {
                                Logger.d(TAG,
                                        "onActivityResult() Exception");
                                e.printStackTrace();
                            }
                        }
                        Intent intent = null;
                        Logger.d(TAG, "onActivityResult() isSupportedRcsContact is " + isSupportedRcsContact);
                        if(isSupportedRcsContact){
                            intent = getIntent(participantList);
                        }
                        else{
                            intent = getMultiIntent(participantList);
                        }
                        try {
                            startActivity(intent);
                        } catch (android.content.ActivityNotFoundException e) {
                            e.printStackTrace();
                        }

                    }else{
                        //add member to existing group
                        if(RcsSettings.getInstance().isSupportOP07()){
                            Logger.d(TAG, "onActivityResult() open chat op07" );
                            boolean isSupportedRcsContact = true;
                            CapabilityService capabilityApi = ApiManager.getInstance()
                                    .getCapabilityApi();
                            if (capabilityApi != null) {
                                try {
                                    for (Participant participant : participantList) {
                                        Capabilities capabilities = capabilityApi
                                                .getContactCapabilities(ContactIdUtils.createContactIdFromTrustedData(participant.getContact()));
                                        if(capabilities == null){
                                            Logger.v(TAG, "onActivityResult capabilities is null ");
                                            isSupportedRcsContact = false;
                                        }
                                        if(isSupportedRcsContact){
                                            isSupportedRcsContact = capabilities.isImSessionSupported();
                                            Logger.d(TAG, "onActivityResult isSupportedRcsContact:" + isSupportedRcsContact + ",contact:" + participant.getContact());
                                        }
                                    }
                                } catch (RcsServiceException e) {
                                    Logger.d(TAG,
                                            "onActivityResult() RcsServiceException");
                                    e.printStackTrace();
                                } catch (Exception e) {
                                    Logger.d(TAG,
                                            "onActivityResult() Exception");
                                    e.printStackTrace();
                                }
                            }
                            Intent intent = null;
                            Logger.d(TAG, "onActivityResult() open isSupportedRcsContact is " + isSupportedRcsContact);
                            if(isSupportedRcsContact){
                                addContactsToGroupChat(participantList);
                            }
                            else{
                                intent = getMultiIntent(participantList);
                                try {
                                    startActivity(intent);
                                } catch (android.content.ActivityNotFoundException e) {
                                    e.printStackTrace();
                                }
                            }
                        } else {
                    addContactsToGroupChat(participantList);
                    }
                    }
                } else {
                    Logger.w(
                            TAG,
                            "onActivityResult() participantList size is 0," +
                            "so do not add member to group chat");
                }
            }
        } else if (requestCode == RESULT_CODE_REMOVE_CONTACTS) {
            //remove contacts from group chat
            if (data != null) {
                ArrayList<Participant> participantList = ContactsListManager
                        .getInstance().parseParticipantsFromIntent(data);
                Logger.d(TAG, "onActivityResult() participantList is "
                        + participantList);
                if (participantList != null && participantList.size() != 0) {
                    removeContactsFromGroupChat(participantList);
                } else {
                    Logger.w(
                            TAG,
                            "onActivityResult() participantList size is 0," +
                            "so do not remove member from group chat");
                }
            }
        }
    }

    private Intent getIntent(ArrayList<Participant> participants){
        Intent intent = new Intent();
        intent.setAction(PluginGroupChatActivity.ACTION);
        intent.setFlags(Intent.FLAG_ACTIVITY_CLEAR_TASK | Intent.FLAG_ACTIVITY_NEW_TASK);
        intent.putParcelableArrayListExtra(
                Participant.KEY_PARTICIPANT_LIST, participants);
        intent.putExtra("addContacts",
                ChatMainActivity.VALUE_ADD_CONTACTS);
        intent.putExtra(Utils.IS_GROUP_CHAT, true);
        //intent.putExtra("chatmode", IpMessageConsts.ChatMode.JOYN);
        return intent;
    }

    private Intent getMultiIntent(ArrayList<Participant> participants){
        Intent intent = new Intent();
        intent.setAction(PluginMultiChatActivity.ACTION);
        intent.setFlags(Intent.FLAG_ACTIVITY_CLEAR_TASK | Intent.FLAG_ACTIVITY_NEW_TASK);
        intent.putParcelableArrayListExtra(
                Participant.KEY_PARTICIPANT_LIST, participants);
        intent.putExtra("addContacts",
                ChatMainActivity.VALUE_ADD_CONTACTS);
        intent.putExtra(Utils.IS_GROUP_CHAT, false);
        intent.putExtra(Utils.IS_MULTI_CHAT, true);
        return intent;
    }
    /**
     * Adds the contacts to group chat.
     *
     * @param participantList the participant list
     */
    private void addContactsToGroupChat(ArrayList<Participant> participantList) {
        Logger.v(TAG, "addContactsToGroupChat");
        ControllerImpl controller = ControllerImpl.getInstance();
        Message controllerMessage = controller.obtainMessage(
                ChatController.EVENT_GROUP_ADD_PARTICIPANT, mTag,
                participantList);
        controllerMessage.sendToTarget();
    }

    /**
     * Remove the contacts from group chat.
     *
     * @param participantList the participant list
     */
    private void removeContactsFromGroupChat(ArrayList<Participant> participantList) {
        Logger.v(TAG, "removeContactsFromGroupChat");
        ControllerImpl controller = ControllerImpl.getInstance();
        Message controllerMessage = controller.obtainMessage(
                ChatController.EVENT_GROUP_REMOVE_PARTICIPANT, mTag,
                participantList);
        controllerMessage.sendToTarget();
    }

    /**
     * Adds the contacts to exist chat window.
     *
     * @param participantList the participant list
     */
    protected void addContactsToExistChatWindow(
            List<Participant> participantList) {
        int size = participantList == null ? 0 : participantList.size();
        Logger.w(TAG, "size = " + size);
        if (size == 0) {
            Logger.w(TAG, "participantList is null");
            return;
        }
        addParticipants(participantList);
    }

    /**
     * This is an Information for chat event.
     */
    public static class ChatEventInformation implements IChatEventInformation {
        protected ChatEventStruct mChatEventStruct = null;

        /**
         * Instantiates a new chat event information.
         *
         * @param chatEventStruct the chat event struct
         */
        public ChatEventInformation(ChatEventStruct chatEventStruct) {
            mChatEventStruct = chatEventStruct;
        }

        /**
         * Gets the information.
         *
         * @return the information
         */
        public Information getInformation() {
            if (mChatEventStruct != null) {
                return mChatEventStruct.information;
            } else {
                Logger.e(TAG, "getInformation the mChatEventStruct is null");
                return null;
            }
        }

        /**
         * Gets the related info.
         *
         * @return the related info
         */
        public Object getRelatedInfo() {
            if (mChatEventStruct != null) {
                return mChatEventStruct.relatedInformation;
            } else {
                Logger.e(TAG, "getInformation the mChatEventStruct is null");
                return null;
            }
        }

        /**
         * Gets the date.
         *
         * @return the date
         */
        public Date getDate() {
            if (mChatEventStruct != null) {
                return mChatEventStruct.date;
            } else {
                Logger.e(TAG, "getInformation the mChatEventStruct is null");
                return null;
            }
        }
    }

    /**
     * The Class SentMessage.
     */
    public class SentMessage implements ISentChatMessage, IChatMessage {
        private ChatMessage mMessage = null;

        private Status mStatus = Status.SENDING;

        public ConcurrentHashMap<String, ParticipantStatusInfo> mMessageStatusMap =
                new ConcurrentHashMap<String, ParticipantStatusInfo>();

        /**
         * Instantiates a new sent message.
         *
         * @param msg the msg
         */
        public SentMessage(ChatMessage msg) {
            mMessage = msg;
            for (Participant participant : mParticipantList) {
                mMessageStatusMap.put(participant.getContact(),
                        new ParticipantStatusInfo(Status.UNKNOWN.ordinal(), null));
            }
        }

        /*
         * public Map<String, ISentChatMessage.Status> getStatusMap() { return
         * mMessageStatusMap; }
         */

        /**
         * Gets the message tag.
         *
         * @return the message tag
         */
        public int getMessageTag() {
            // TODO Auto-generated method stub
            return 0;
        }

        /**
         * Gets the id.
         *
         * @return the id
         */
        @Override
        public String getId() {
            if (mMessage == null) {
                Logger.w(TAG, "mMessage is null, as a result no id returned");
                return null;
            }
            return mMessage.getId();
        }

        /**
         * Update status.
         *
         * @param s the status
         */
        @Override
        public void updateStatus(Status s) {
            final Status status = s;
            Logger.d(TAG, "updateStatus:" + s + ",message:" + mMessage);
            mUiHandler.post(new Runnable() {
                @Override
                public void run() {
                    mStatus = status;
                    if (mMessageAdapter != null) {
                        mMessageAdapter.notifyDataSetChanged();
                    }
                }
            });
        }

        /**
         * Get message text.
         *
         * @return Message text.
         */
        public String getMessageText() {
            if (mMessage == null) {
                return null;
            }
            return mMessage.getContent();
        }

        /**
         * Get status of message.
         *
         * @return status of message.
         */
        public Status getStatus() {
            return mStatus;
        }

        /**
         * Return the time when then message was sent.
         *
         * @return Message sent time.
         */
        public Date getMessageDate() {
            if (mMessage != null) {
                //return new Date(mMessage.getTimestamp());
                return mMessage.getReceiptDate();
            } else {
                Logger.d(TAG, "getMessageText mMessage is null");
                return null;
            }
        }

        /**
         * Update message.
         *
         * @param message the message
         */
        public void updateMessage(ChatMessage message) {
            Logger.d(TAG, "updateMessage() message: " + message);
            mMessage = message;
        }

        /**
         * Update date.
         *
         * @param date the date
         */
        @Override
        public void updateDate(Date date) {
            Logger.d(TAG, "SentMessage, updateDate()  status: " + mStatus
                    + "Date :" + date + " id:" + mMessage.getId());
        }

        /**
         * Update status.
         *
         * @param s the s
         * @param contact the contact
         */
        public void updateStatus(Status s, String contact, Date date) {
            Logger.d(TAG, "SentMessage, updateStatus()  status: " + s
                    + "contact :" + contact + " id:" + mMessage.getId() + " date:" + date);
            // update the map
            synchronized (mMessageStatusMap) {
                ParticipantStatusInfo participantStatusInfo = mMessageStatusMap.get(contact);
                if(participantStatusInfo != null){
                    participantStatusInfo = null;
                }
                mMessageStatusMap.put(contact, new ParticipantStatusInfo(s.ordinal(), date));
            }

        }

    }

    /**
     * ReceivedMessage provided for window to get message.
     */
    public static class ReceivedMessage implements IReceivedChatMessage,
            IChatMessage {
        private Status mStatus = Status.SENDING;

        private ChatMessage mMessage = null;

        /**
         * Instantiates a new received message.
         *
         * @param msg the msg
         */
        public ReceivedMessage(ChatMessage msg) {
            mMessage = msg;
        }

        /**
         * Gets the id.
         *
         * @return the id
         */
        @Override
        public String getId() {
            if (mMessage == null) {
                Logger.w(TAG, "mMessage is null, as a result no id returned");
                return null;
            }
            return mMessage.getId();
        }

        /**
         * Return the message text.
         *
         * @return Message text.
         */
        public String getMessageText() {
            if (mMessage == null) {
                Logger.w(TAG, "mMessage is null, as a result no text returned");
                return null;
            }
            return mMessage.getContent();
        }

        /**
         * Return the time when then message was sent.
         *
         * @return Message sent time.
         */
        public Date getMessageDate() {
            if (mMessage != null) {
                return mMessage.getReceiptDate();
            } else {
                Logger.d(TAG, "getMessageText mMessage is null");
                return null;
            }
        }

        /**
         * Return who send this message.
         *
         * @return The sender.
         */
        public String getMessageSender() {
            if (mMessage == null) {
                Logger.w(TAG,
                        "mMessage is null, as a result no remote returned");
                return null;
            }
            return mMessage.getRemoteContact().toString();
        }

        /**
         * Get status of message.
         *
         * @return status of message.
         */
        public Status getStatus() {
            return mStatus;
        }

        /**
         * Gets the alias name.
         *
         * @return the alias name
         */
        public String getAliasName() {
            if (mMessage == null) {
                Logger.w(TAG,
                        "ABCG mMessage is null, as a result no alias returned");
                return null;
            }
            String alias = mMessage.getDisplayName();
            Logger.w(TAG, "ABCG alias is:" + alias);
            if (alias != null) {
                return alias;
            } else {
                return "default";
            }
        }

    }

    /**
     * reload old history according to number of current shown messages.
     */

    private void handleGetHistory() {
        Logger.d(TAG, "handleGetHistory entry mCurrentNumberMsgLoaded="
                + mCurrentNumberMsgLoaded);
        try {
            reloadMessageIdList();
            if (mMessageIdArray != null
                    && mMessageIdArray.size() > mCurrentNumberMsgLoaded) {
                List<Integer> mMessageIdCurrentArray = null;
                if ((mMessageIdArray.size() -
                        (1 + mCurrentNumberMsgLoaded + GroupChatFragment.LOAD_DEFAULT)) < 0) {
                    mMessageIdCurrentArray = mMessageIdArray;
                    addLoadHistoryHeader(false);
                } else {
                    // mMessageIdCurrentArray =
                    // mMessageIdArray.subList(mMessageIdArray.size()-(mCurrentNumberMsgLoaded+
                    // GroupChatFragment.LOAD_DEFAULT), mMessageIdArray.size());
                    mMessageIdCurrentArray = mMessageIdArray.subList(0,
                            mCurrentNumberMsgLoaded
                                    + GroupChatFragment.LOAD_DEFAULT);
                    addLoadHistoryHeader(true);
                }

                // mCurrentNumberMsgLoaded = mMessageIdCurrentArray.size();
                Logger.d(TAG, "handleGetHistory size current id list"
                        + mMessageIdCurrentArray.size());
                try {
                    Message controllerMessage = ControllerImpl
                            .getInstance()
                            .obtainMessage(
                                    ChatController.EVENT_CLEAR_CHAT_HISTORY_MEMORY_ONLY,
                                    mTag, null);
                    controllerMessage.sendToTarget();
                    Message controllerMessage1 = ControllerImpl.getInstance()
                            .obtainMessage(ChatController.EVENT_RELOAD_MESSAGE,
                                    mTag.toString(), mMessageIdCurrentArray);
                    controllerMessage1.sendToTarget();
                } catch (Exception e) {
                    e.printStackTrace();
                }
            } else {
                Logger.d(TAG, "handleGetHistory no need to load");
            }
        } catch (Exception e) {
            e.printStackTrace();
        }
    }

    /**
     * Adds the load history header.
     *
     * @param showHeader the show header
     */
    @Override
    public void addLoadHistoryHeader(boolean showHeader) {

        if (mMessageAdapter != null && mMessageAdapter.mHeaderView != null) {
            mShowHeader = showHeader;
            mMessageAdapter.mHeaderView
                    .setOnClickListener(new View.OnClickListener() {

                        @Override
                        public void onClick(View v) {
                            new AsyncTask<Void, Void, Void>() {
                                @Override
                                protected Void doInBackground(Void... params) {
                                    handleGetHistory();
                                    return null;
                                }
                            } .execute();

                        }

                    });

            mUiHandler.post(new Runnable() {

                @Override
                public void run() {

                    mMessageAdapter.showHeaderView(mShowHeader);
                }
            });
        } else {
            Logger.w(TAG, "addLoadHistoryHeader, mMessageAdapter is not ready!");
        }

    }

    /**
     * On adapter prepared.
     */
    @Override
    protected void onAdapterPrepared() {
        // TODO Auto-generated method stub

    }

    /**
     * Gets the fragment resource.
     *
     * @return the fragment resource
     */
    @Override
    protected int getFragmentResource() {
        return R.layout.chat_fragment_group;
    }

    /**
     * Adds the chat event information.
     *
     * @param chatEventStruct the chat event struct
     * @return the i chat event information
     */
    @Override
    public IChatEventInformation addChatEventInformation(
            ChatEventStruct chatEventStruct) {
        if (chatEventStruct != null) {
            Logger.d(TAG, "addChatEventInformation chatEventStruct is "
                    + chatEventStruct);
            Date date = chatEventStruct.date;
            addMessageDate(date);
            IChatEventInformation chatEvent = new ChatEventInformation(
                    chatEventStruct);
            Information information = ((ChatEventInformation) chatEvent)
                    .getInformation();
            Object contactObj = ((ChatEventInformation) chatEvent)
                    .getRelatedInfo();
            String contact = null;
            if(contactObj instanceof String){
                contact = (String)contactObj;
                Logger.d(TAG, "addChatEventInformation contact is object of string = " + contact);
            }
            if(contact != null && information == Information.JOIN){
                for(ParticipantInfo info : mGroupChatParticipantList){
                    if(contact.equals(info.getContact()))
                    {
                        Logger.d(TAG, "addChatEventInformation Participant already joined");
                        return null;
                    }
                }
            }
            switch (information) {
            case LEFT:
            case JOIN:
                addChatEventInfo(chatEvent);
                break;
            default:
                Logger.e(TAG,
                        "addChatEventInformation the information is not defined and it is "
                                + information);
                break;
            }
            return null;
        } else {
            Logger.d(TAG, "The sent chat message is null");
            return null;
        }
    }

    /**
     * Adds the chat event info.
     *
     * @param chatEvent the chat event
     * @return the i chat event information
     */
    private IChatEventInformation addChatEventInfo(
            IChatEventInformation chatEvent) {
        Logger.d(TAG, "addChatEventInfo entry");
        if (mMessageAdapter != null) {
            mMessageSequenceOrder = mMessageSequenceOrder + 1;
            int position = mMessageSequenceOrder;
            mMessageAdapter.addMessage(chatEvent, position);
            mMessageList.add(mMessageList.size(), null);
            return chatEvent;
        } else {
            Logger.d(TAG, "addChatEventInformation mMessageAdapter is null");
            return null;
        }
    }

    /**
     * Update chat status.
     *
     * @param status the status
     */
    @Override
    public void updateChatStatus(final int status) {
        Logger.d(TAG, "updateChatStatus() status: " + status);
        mUiHandler.post(new Runnable() {
            @Override
            public void run() {
                final Activity activity = getActivity();
                if (activity == null) {
                    Logger.d(TAG, "updateChatStatus() activity is null");
                    return;
                }
                Logger.d(TAG, "updateChatStatus() status run:" + status);
                switch (status) {
                case Utils.GROUP_STATUS_TERMINATED:
                    mBtnSend.setVisibility(View.GONE);
                    mBtnAddView.setVisibility(View.GONE);
                    mShareGridView.setVisibility(View.GONE);
                    mMessageEditor.setVisibility(View.GONE);
                    mBtnEmotion.setVisibility(View.GONE);
                    mBtnSend.setOnClickListener(new View.OnClickListener() {
                        public void onClick(View v) {
                            /*
                             * Toast.makeText(activity,
                             * getString(R.string.group_terminated),
                             * Toast.LENGTH_SHORT).show();
                             */
                        }
                    });
                    break;
                case Utils.GROUP_STATUS_REJOINING:
                    mBtnSend.setClickable(true);
                    mBtnSend.setOnClickListener(new View.OnClickListener() {
                        public void onClick(View v) {
                            /*
                             * Toast.makeText(activity,
                             * getString(R.string.group_rejoining),
                             * Toast.LENGTH_SHORT).show();
                             */
                        }
                    });
                    break;
                case Utils.GROUP_STATUS_RESTARTING:
                    mBtnSend.setClickable(true);
                    mBtnSend.setOnClickListener(new View.OnClickListener() {
                        public void onClick(View v) {
                            /*
                             * Toast.makeText(activity,
                             * getString(R.string.group_restarting),
                             * Toast.LENGTH_SHORT).show();
                             */
                        }
                    });
                    break;
                case Utils.GROUP_STATUS_CANSENDMSG:
                    Logger.d(TAG, "updateChatStatus() cansendmessage");
                    mBtnSend.setVisibility(View.VISIBLE);
                    mMessageEditor.setVisibility(View.VISIBLE);
                    mBtnEmotion.setVisibility(View.VISIBLE);
                    mBtnSend.setClickable(true);
                    mMessageEditor.setFocusable(true);
                    mMessageEditor.setClickable(true);
                    int maxLength = 0;
                    if (mChatConfiguration != null) {
                        try {
                            maxLength = (int) mChatConfiguration
                                    .getGroupChatMessageMaxLength();
                        } catch (Exception e) {
                            maxLength = -1;
                            e.printStackTrace();
                        }
                    } else {
                        maxLength = -1;
                    }
                    Logger.d(TAG, "maxLength for editor is " + maxLength);
                    if(maxLength > 0) {
                        InputFilter[] filterArray = new InputFilter[1];
                        filterArray[0] = new InputFilter.LengthFilter(maxLength);
                        mMessageEditor.setFilters(filterArray);
                    }
                    mBtnEmotion.setClickable(true);
                    mBtnAddView.setEnabled(true);
                    mBtnAddView.setClickable(true);
                    mBtnSend.setOnClickListener(mBtnSendClickListener);
                    break;
                case Utils.GROUP_STATUS_UNAVIALABLE:
                    Logger.d(TAG, "updateChatStatus() unavailable");
                    mBtnSend.setClickable(false);
                    //mMessageEditor.setFocusable(false);
                   // mMessageEditor.setClickable(false);
                    mBtnAddView.setEnabled(false);
                    mBtnAddView.setClickable(false);
                    mBtnEmotion.setClickable(false);
                    mBtnSend.setOnClickListener(new View.OnClickListener() {
                        public void onClick(View v) {
                            /*
                             * Toast.makeText(activity,
                             * getString(R.string.group_terminated),
                             * Toast.LENGTH_SHORT).show();
                             */
                        }
                    });
                    break;
                default:
                    break;
                }
            }
        });
    }

    /**
     * Update all msg as read.
     */
    @Override
    public void updateAllMsgAsRead() {
        // Do Nothing
    }

    /**
     * Load dimension.
     *
     * @param resources the resources
     */
    private void loadDimension(Resources resources) {
        mGroupMemberHorMarginLeft = resources
                .getDimensionPixelSize(R.dimen.group_member_hor_margin_left);
        mGroupMemberHorMarginRight = resources
                .getDimensionPixelSize(R.dimen.group_member_hor_margin_right);
        mGroupMemberHorHeight = resources
                .getDimensionPixelSize(R.dimen.group_member_hor_width);
        mGroupMemberHorWidth = resources
                .getDimensionPixelSize(R.dimen.group_member_hor_height);
    }

    /**
     * On configuration changed.
     *
     * @param newConfig the new config
     */
    @Override
    public void onConfigurationChanged(Configuration newConfig) {
        super.onConfigurationChanged(newConfig);
        Logger.d(TAG, "onConfigurationChanged() ");
        mParticipantListDisplayer.onScreenSwitched(newConfig.orientation);
    }

    private ParticipantListDisplayer mParticipantListDisplayer = new ParticipantListDisplayer();

    /**
     * Defines a common interface for ParticipantListDisplayer.
     */
    private interface IDisplayStrategy {
        /**
         * Expand.
         */
        void expand();

        /**
         * Collapse.
         */
        void collapse();

        /**
         * Show.
         */
        void show();

        /**
         * Dismiss.
         */
        void dismiss();

        /**
         * Update banner.
         *
         * @param participantInfos the participant infos
         */
        void updateBanner(List<ParticipantInfo> participantInfos);
    }

    /**
     * This is a helper class, to manage the participant list banner.
     */
    private class ParticipantListDisplayer {
        private static final String TAG = "ParticipantListDisplayer";

        private IDisplayStrategy mCurrentStrategy = null;

        private final LandscapeStrategy mLandscapeStrategy = new LandscapeStrategy();
        private final PortraitStrategy mPortraitStrategy = new PortraitStrategy();

        private List<ParticipantInfo> mParticipantInfos = null;

        /**
         * On fragment create.
         *
         * @param orientation the orientation
         */
        public void onFragmentCreate(int orientation) {
            Logger.d(TAG, "onActivityCreate entry, orientation: " + orientation);
            onStatusUpdated(orientation, false);
        }

        /**
         * Expand.
         */
        public void expand() {
            if (null != mCurrentStrategy) {
                mCurrentStrategy.expand();
            } else {
                Logger.w(TAG, "expand() mCurrentStrategy is null");
            }
        }

        /**
         * Collapse.
         */
        public void collapse() {
            if (null != mCurrentStrategy) {
                mCurrentStrategy.collapse();
            } else {
                Logger.w(TAG, "collapse() mCurrentStrategy is null");
            }
        }

        /**
         * This method should only be called from the main thread to update the
         * banner.
         *
         * @param participantInfos            The latest participant list
         */
        public void updateBanner(List<ParticipantInfo> participantInfos) {
            mParticipantInfos = participantInfos;
            if (null != mCurrentStrategy) {
                mCurrentStrategy.updateBanner(participantInfos);
                if (mCurrentStrategy.equals(mPortraitStrategy)) {
                    mPortraitStrategy.show();
                    mLandscapeStrategy.dismiss();
                } else {
                    mPortraitStrategy.dismiss();
                    mLandscapeStrategy.show();
                }
            } else {
                Logger.w(TAG, "updateBanner() mCurrentStrategy is null");
            }
        }

        /**
         * On screen switched.
         *
         * @param orientation the orientation
         */
        public void onScreenSwitched(int orientation) {
            onStatusUpdated(orientation, true);
        }

        /**
         * On status updated.
         *
         * @param orientation the orientation
         * @param isNeedUpdate the is need update
         */
        public void onStatusUpdated(int orientation, boolean isNeedUpdate) {
            Logger.d(TAG, "onScreenSwitched entry, orientation: " + orientation);
            if(getActivity() != null){
                if(getActivity().isInMultiWindowMode()){
                    Logger.d(TAG, "onStatusUpdated, multiwindow mode is activated");
                    //In multiwindow mode, for landscape mode, there is not enough space for participant
                    //banner, so mode should be portrait
                    mPortraitStrategy.show();
                    mLandscapeStrategy.dismiss();
                    mCurrentStrategy = mPortraitStrategy;
                    return;
                }
            }
            switch (orientation) {
            case Configuration.ORIENTATION_LANDSCAPE:
                mPortraitStrategy.dismiss();
                mLandscapeStrategy.show();
                mCurrentStrategy = mLandscapeStrategy;
                break;
            case Configuration.ORIENTATION_PORTRAIT:
                mPortraitStrategy.show();
                mLandscapeStrategy.dismiss();
                mCurrentStrategy = mPortraitStrategy;
                break;
            default:
                Logger.w(TAG, "onScreenSwitched() unknown orientation: "
                        + orientation);
                break;
            }
            if (isNeedUpdate) {
                mCurrentStrategy.updateBanner(mParticipantInfos);
            }
        }
    }

    /**
     * The strategy used in Landscape screen.
     */
    private class LandscapeStrategy extends BaseAdapter implements
            IDisplayStrategy {
        private static final String TAG = "LandscapeStrategy";

        private ListView mBanner = null;
        private View mArea = null;
        private LayoutInflater mInflator = null;
        private final List<ParticipantInfo> mParticipantInfoList = new ArrayList<ParticipantInfo>();

        /**
         * Check area.
         */
        private void checkArea() {
            if (null != mBanner) {
                Logger.d(TAG, "checkArea() already initialized");
            } else {
                Logger.d(TAG, "checkArea() not initialized");
                mArea = mContentView.findViewById(R.id.participant_list_area);
                mBanner = (ListView) mContentView
                        .findViewById(R.id.participant_banner);
                mBanner.setAdapter(this);
                mInflator = LayoutInflater.from(getActivity()
                        .getApplicationContext());
            }
        }

        /**
         * Collapse.
         */
        @Override
        public void collapse() {
            Logger.v(TAG, "collapse() entry");
            // Do nothing
        }

        /**
         * Dismiss.
         */
        @Override
        public void dismiss() {
            Logger.v(TAG, "dismiss() entry");
            checkArea();
            mArea.setVisibility(View.GONE);
        }

        /**
         * Expand.
         */
        @Override
        public void expand() {
            Logger.v(TAG, "expand() entry");
            // Do nothing
        }

        /**
         * Show.
         */
        @Override
        public void show() {
            Logger.v(TAG, "show() entry");
            checkArea();
            mArea.setVisibility(View.VISIBLE);
        }

        /**
         * Update banner.
         *
         * @param participantInfos the participant infos
         */
        @Override
        public void updateBanner(List<ParticipantInfo> participantInfos) {
            Logger.d(TAG, "updateBanner() entry, participantInfos: "
                    + participantInfos);
            if(mParticipantInfoList == null || participantInfos == null){
                return;
            }
            mParticipantInfoList.clear();
            mParticipantInfoList.addAll(participantInfos);
            notifyDataSetChanged();
        }

        /**
         * Gets the count.
         *
         * @return the count
         */
        @Override
        public int getCount() {
            return mParticipantInfoList.size();
        }

        /**
         * Gets the item.
         *
         * @param position the position
         * @return the item
         */
        @Override
        public Object getItem(int position) {
            return mParticipantInfoList.get(position);
        }

        /**
         * Gets the item id.
         *
         * @param position the position
         * @return the item id
         */
        @Override
        public long getItemId(int position) {
            return 0;
        }

        /**
         * Gets the view.
         *
         * @param position the position
         * @param convertView the convert view
         * @param parent the parent
         * @return the view
         */
        @Override
        public View getView(int position, final View convertView,
                ViewGroup parent) {
            Logger.v(TAG, "getView() entry, position: " + position);
            View itemView = convertView;
            if (null == itemView) {
                Logger.d(TAG, "getView() inflate a new item view");
                itemView = mInflator.inflate(
                        R.layout.participant_list_item_vertical, null);
            } else {
                Logger.d(TAG, "getView() use convertView");
            }
            bindView(itemView, mParticipantInfoList.get(position));
            Logger.v(TAG, "getView() exit");
            return itemView;
        }

        /**
         * Bind view.
         *
         * @param itemView the item view
         * @param info the info
         */
        private void bindView(View itemView, ParticipantInfo info) {
            Logger.d(TAG, "bindView() info:" + info);
            String contact = info.getContact();
            AsyncImageView avatar = (AsyncImageView) itemView
                    .findViewById(R.id.peer_avatar);
            boolean active = User.STATE_CONNECTED.equals(info.getState());
            avatar.setAsyncContact(contact, !active);
            TextView statusView = (TextView) itemView
                    .findViewById(R.id.participant_status);
            statusView.setText(active ? getString(R.string.group_active)
                    : getString(R.string.group_inactive));
            TextView remoteName = (TextView) itemView
                    .findViewById(R.id.remote_name);
            remoteName.setText(ContactsListManager.getInstance()
                    .getDisplayNameByPhoneNumber(contact));
        }
    }

    /**
     * The strategy used in Portrait screen.
     */
    private class PortraitStrategy implements IDisplayStrategy {

        private static final String TAG = "PortraitStrategy";
        private boolean mIsExpand = false;

        /**
         * Collapse.
         */
        @Override
        public void collapse() {
            Logger.v(TAG, "collapse() entry");
            Activity activity = getActivity();
            if (null != activity) {
                setGroupChatBannerScrollerVisibility(activity, View.GONE);
                setGroupChatCollapseVisibility(activity, View.GONE);
                setGroupChatExpandVisibility(activity, View.VISIBLE);
            } else {
                Logger.w(TAG, "collapse() activity is null");
            }
            mIsExpand = false;

        }

        /**
         * Dismiss.
         */
        @Override
        public void dismiss() {
            Logger.v(TAG, "dismiss() entry");
            Activity activity = getActivity();
            if (null != activity) {
                setGroupChatBannerScrollerVisibility(activity, View.GONE);
                setGroupChatCollapseVisibility(activity, View.GONE);
                setGroupChatExpandVisibility(activity, View.GONE);
            } else {
                Logger.w(TAG, "dismiss() activity is null");
            }

        }

        /**
         * Expand.
         */
        @Override
        public void expand() {
            Logger.v(TAG, "expand() entry");
            Activity activity = getActivity();
            if (null != activity) {
                setGroupChatBannerScrollerVisibility(activity, View.VISIBLE);
                setGroupChatCollapseVisibility(activity, View.VISIBLE);
                setGroupChatExpandVisibility(activity, View.GONE);
            } else {
                Logger.w(TAG, "expand() activity is null");
            }
            mIsExpand = true;

        }

        /**
         * Show.
         */
        @Override
        public void show() {
            if (mIsExpand) {
                Logger.d(TAG, "show() mIsExpand is true");
                expand();
            } else {
                Logger.d(TAG, "show() mIsExpand is false");
                collapse();
            }
        }

        /**
         * Update banner.
         *
         * @param participantInfos the participant infos
         */
        @Override
        public void updateBanner(List<ParticipantInfo> participantInfos) {
            Logger.d(TAG, "updateBanner() entry, participantInfos: "
                    + participantInfos);
            final Activity activity = getActivity();
            if (activity != null) {
                LinearLayout grouChatMemberIconsLayout = (LinearLayout) activity
                        .findViewById(R.id.group_chat_banner_container);
                if (grouChatMemberIconsLayout == null) {
                    Logger.w(TAG,
                            "updateBanner() grouChatMemberIconsLayout is null");
                    return;
                }
                final int childCount = grouChatMemberIconsLayout
                        .getChildCount();
                LayoutParams layoutParams = new LayoutParams(
                        mGroupMemberHorWidth, mGroupMemberHorHeight);
                layoutParams.setMargins(mGroupMemberHorMarginLeft, 0,
                        mGroupMemberHorMarginRight, 0);
                int num = 0;

                if (participantInfos != null) {
                    num = participantInfos.size();
                    Logger.d(TAG, "updateBanner() num: " + num
                            + " , childCount: " + childCount);
                    int i = 0;
                    for (; i < num; ++i) {
                        Logger.d(TAG, "updateBanner() current i: " + i);
                        ParticipantInfo participant = participantInfos.get(i);
                        if (i < childCount) {
                            Logger.d(TAG, "updateBanner() use an existing view");
                            getView(activity, participant,
                                    (AsyncImageView) grouChatMemberIconsLayout
                                            .getChildAt(i));
                        } else {
                            Logger.d(TAG,
                                    "updateBanner() need to inflate a new view");
                            View itemView = getView(activity, participant, null);
                            if (null != itemView) {
                                grouChatMemberIconsLayout.addView(itemView,
                                        layoutParams);
                            } else {
                                Logger.w(TAG, "updateBanner() inflate failed");
                            }
                        }
                    }
                    Logger.d(TAG, "updateBanner() add view done, i: " + i);
                    int invalidItemCount = childCount - i;
                    if (invalidItemCount > 0) {
                        Logger.d(TAG,
                                "updateBanner() need to remove child view from "
                                        + i + " count " + invalidItemCount);
                        grouChatMemberIconsLayout.removeViews(i,
                                invalidItemCount);
                    } else {
                        Logger.d(TAG,
                                "updateBanner() no need to remove child view");
                    }
                } else {
                    Logger.e(TAG, "updateBanner() the participantInfos is null");
                }
            } else {
                Logger.w(TAG, "updateBanner() activity is null.");
            }
        }

        /**
         * Gets the view.
         *
         * @param context the context
         * @param info the info
         * @param convertView the convert view
         * @return the view
         */
        private View getView(final Context context, final ParticipantInfo info,
                final AsyncImageView convertView) {
            Logger.d(TAG, "getView() info: " + info + " , convertView: "
                    + convertView);
            if (null == info) {
                Logger.w(TAG, "getView() info is null");
                return null;
            }
            AsyncImageView itemView = convertView;
            if (null == itemView) {
                itemView = inflateView(context);
            }
            String state = info.getState();
            String contact = info.getContact();
            Logger.d(TAG, "getView(), contact: " + contact + ", state: "
                    + state);
            boolean isGrey = !(User.STATE_CONNECTED.equals(info.getState()));
            itemView.setAsyncContact(contact, isGrey);
            return itemView;
        }

        /**
         * Inflate view.
         *
         * @param context the context
         * @return the async image view
         */
        private AsyncImageView inflateView(Context context) {
            return new AsyncImageView(context);
        }

        /**
         * Sets the group chat banner scroller visibility.
         *
         * @param activity the activity
         * @param visible the visible
         */
        private void setGroupChatBannerScrollerVisibility(Activity activity,
                int visible) {
            HorizontalScrollView groupChatBannerScrollerLayout = (HorizontalScrollView) activity
                    .findViewById(R.id.group_chat_banner_scroller);
            if (groupChatBannerScrollerLayout != null) {
                Logger.v(TAG,
                        "setGroupChatBannerScrollerVisibility() " +
                        "groupChatBannerScroller is not null");
                groupChatBannerScrollerLayout.setVisibility(visible);
            } else {
                Logger.v(
                        TAG,
                        "setGroupChatBannerScrollerVisibility() groupChatBannerScroller " +
                        "is null.visible = "
                                + visible);
            }
        }

        /**
         * Sets the group chat expand visibility.
         *
         * @param activity the activity
         * @param visible the visible
         */
        private void setGroupChatExpandVisibility(Activity activity, int visible) {

            Logger.v(TAG,
                    "setGroupChatExpandVisibility() groupChatExpandView is null");

        }

        /**
         * Sets the group chat collapse visibility.
         *
         * @param activity the activity
         * @param visible the visible
         */
        private void setGroupChatCollapseVisibility(Activity activity,
                int visible) {

            Logger.v(TAG,
                    "setGroupChatCollapseVisibility() groupChatExpandView is null");
        }
    }

    /**
     * Show particpants.
     */
    public void showParticpants() {
        Intent intent = new Intent("com.mediatek.rcse.action.SHOW_PARTICIPANTS");
        intent.putParcelableArrayListExtra("participantsinfo",
                (ArrayList<? extends Parcelable>) mGroupChatParticipantList);
        startActivity(intent);
    }

    /**
     * On click.
     *
     * @param v the v
     */
    @Override
    public void onClick(View v) {
        Logger.d(TAG, "onClick() entry");
        showParticpants();
        /*
         * if (activity != null) { if (v.getId() == R.id.group_chat_expand) {
         * expandGroupChat(); } else if (v.getId() == R.id.group_chat_collapse)
         * { collapseGroupChat(); } else if (v.getId() ==
         * R.id.group_chat_title_layout) { Logger.v(TAG,
         * "onClick() group_chat_title_layout clicked"); // Now should judge
         * current status is collapsed or expanded. HorizontalScrollView
         * groupChatBannerScroller = (HorizontalScrollView) activity
         * .findViewById(R.id.group_chat_banner_scroller); if
         * (groupChatBannerScroller != null &&
         * groupChatBannerScroller.getVisibility() == View.VISIBLE) { // Now it
         * is in expanded. collapseGroupChat(); } else { // Now it is in
         * collapsed. expandGroupChat(); } } } else { Logger.w(TAG,
         * "onClick() activity is null"); }
         */
    }

    /**
     * Add contacts to current chat fragment.
     *
     * @return true, if successful
     */
    @Override
    public boolean addContacts() {
        Logger.d(TAG, "addContacts()");
        int currentParticipantsNum = getParticipantsNum();
        int maxNum = 0;
        try {
            maxNum = mChatConfiguration.getGroupChatMaxParticipants() - 1;
        } catch (Exception e) {
            e.printStackTrace();
        }
        Logger.d(TAG, "currentParticipantsNum = " + currentParticipantsNum
                + ", maxNum = " + maxNum);
        if (currentParticipantsNum >= maxNum) {
            mIsMaxGroupChatParticipantsWork = true;
            showToast(R.string.cannot_add_any_more_member);
            return false;
        }
        return super.addContacts();
    }

    /**
     * remove contacts from current chat fragment.
     *
     * @return true, if successful
     */
    public boolean removeContacts() {
        Logger.d(TAG, "removeContacts()");
        int currentParticipantsNum = getParticipantsNum();
        return super.removeContacts();
    }

    /**
     * Check whether current participants is already max. It's used by test case
     *
     * @return True if current participants is already max, otherwise return
     *         false.
     */
    public boolean isMaxGroupChatParticipantsWork() {
        return mIsMaxGroupChatParticipantsWork;
    }

    /**
     * Add message to chat adpater and chat list atomic.
     *
     * @param msg the msg
     */
    private void addMessageAtomic(IChatMessage msg) {
        synchronized (mLock) {
            mMessageSequenceOrder = mMessageSequenceOrder + 1;
            int position = mMessageSequenceOrder;
            // Adding this new message on this position
            mMessageAdapter.addMessage(msg, position);
            mMessageList.add(mMessageList.size(), msg);
            mCurrentNumberMsgLoaded++;
        }
    }

    /**
     * Removes the chat message.
     *
     * @param messageId the message id
     */
    @Override
    public void removeChatMessage(String messageId) {
        mMessageList.remove(mItemIDPosition - 1);
        mCurrentNumberMsgLoaded--;

    }

    /**
     * Addgroup subject.
     *
     * @param subject the subject
     */
    @Override
    public void addgroupSubject(final String subject) {

        if (mActivity == null) {
            Logger.d(TAG, "addgroupSubject mActivity is null");
            return;
        }
        mActivity.runOnUiThread(new Runnable() {
            @Override
            public void run() {
                setmSubjectGroupChat(subject);
                setChatScreenTitle();
            }
        });

    }

    @Override
    public void setClosedGroupType(final boolean type) {

        Logger.d(TAG, "GroupChatFragment setClosedGroupType type:"+ type);
        if (mActivity == null) {
            Logger.d(TAG, "setClosedGroupType mActivity is null");
            return;
        }
        mActivity.runOnUiThread(new Runnable() {
            @Override
            public void run() {
                setIsClosedGroupChat(type);
                setChatScreenTitle();
            }
        });

    }

    /**
     * Update all msg as read for contact.
     *
     * @param participant the participant
     */
    @Override
    public void updateAllMsgAsReadForContact(Participant participant) {
        // TODO Auto-generated method stub

    }

    /**
     * Sets the m chat id.
     *
     * @param chatID the new m chat id
     */
    @Override
    public void setmChatID(String chatID) {
        mChatID = chatID;
        // loadmessageIdList();

    }

    /**
     * Clear extra message.
     */
    @Override
    public void clearExtraMessage() {
        // TODO Auto-generated method stub

    }

    /**
     * Load message id list.
     */
    @Override
    public void loadMessageIdList() {
        Logger.d(TAG, "loadmessageIdList() entry");
        // load message id list when create window
        Cursor cursor = null;
        if (RichMessagingDataProvider.getInstance() == null) {
            RichMessagingDataProvider.createInstance(MediatekFactory
                    .getApplicationContext());
        }

        try {
            cursor = RichMessagingDataProvider.getInstance()
                    .getAllMessageforGroupChat(mChatID);
            mMessageIdArray.clear();
            Logger.d(TAG,
                    "loadmessageIdList() cursor count = " + cursor.getCount()
                            + ",chatid =" + mChatID);
            if (cursor != null && cursor.moveToFirst()) {
                do {

                    Integer messageId = cursor.getInt(cursor
                            .getColumnIndex(ChatLog.Message.ID));

                    mMessageIdArray.add(messageId);
                } while (cursor.moveToNext());
            } else {
                Logger.d(TAG, "loadmessageIdList() empty cursor");
            }
        } catch (Exception e) {
            e.printStackTrace();
        } finally {
            if (null != cursor) {
                cursor.close();
            }
        }
        if (mMessageIdArray != null) {
            if (!mMessageIdArray.isEmpty()) {
                Collections.reverse(mMessageIdArray);
                Logger.d(TAG, "loadmessageIdList() message id array "
                        + mMessageIdArray);
                /*
                 * try { Message controllerMessage = ControllerImpl
                 * .getInstance() .obtainMessage(
                 * ChatController.EVENT_RELOAD_MESSAGE, mTag.toString(),
                 * mMessageIdArray); controllerMessage.sendToTarget(); } catch
                 * (Exception e) {
                 *
                 * e.printStackTrace(); }
                 */
            }
            /*
             * if(mMessageIdArray.size() < LOAD_DEFAULT) {
             * mCurrentNumberMsgLoaded = mMessageIdArray.size(); } else {
             * mCurrentNumberMsgLoaded = LOAD_DEFAULT; }
             */

            if (mMessageIdArray.size() > LOAD_DEFAULT) {
                addLoadHistoryHeader(true);
            }
        }

    }

    /**
     * Reload message id list.
     */
    public void reloadMessageIdList() {
        Logger.d(TAG, "reloadMessageIdList() entry");
        // reload message list when get history
        Cursor cursor = null;
        if (RichMessagingDataProvider.getInstance() == null) {
            RichMessagingDataProvider.createInstance(MediatekFactory
                    .getApplicationContext());
        }

        try {
            cursor = RichMessagingDataProvider.getInstance()
                    .getAllMessageforGroupChat(mChatID);
            mMessageIdArray.clear();
            Logger.d(TAG,
                    "loadmessageIdList() cursor count = " + cursor.getCount()
                            + ",chatid =" + mChatID);
            if (cursor != null && cursor.moveToFirst()) {
                do {

                    Integer messageId = cursor.getInt(cursor
                            .getColumnIndex(ChatLog.Message.ID));

                    mMessageIdArray.add(messageId);
                } while (cursor.moveToNext());
            } else {
                Logger.d(TAG, "loadmessageIdList() empty cursor");
            }
        } catch (Exception e) {
            e.printStackTrace();
        } finally {
            if (null != cursor) {
                cursor.close();
            }
        }
        if (mMessageIdArray != null) {
            if (!mMessageIdArray.isEmpty()) {
                Collections.reverse(mMessageIdArray);
                Logger.d(TAG, "loadmessageIdList() message id array "
                        + mMessageIdArray);

            }
            /*
             * if(mMessageIdArray.size() < LOAD_DEFAULT) {
             * mCurrentNumberMsgLoaded = mMessageIdArray.size(); }
             */
            /*
             * if(mMessageIdArray.size() > mCurrentNumberMsgLoaded) {
             * addLoadHistoryHeader(true); }
             */
        }

    }

    @Override
    public void onSessionStarted() {
        // TODO Auto-generated method stub

    }

}
