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
package com.mediatek.rcse.plugin.message;

import android.content.Intent;
import android.os.RemoteException;
import android.widget.Toast;

import com.mediatek.rcse.api.Logger;
import com.mediatek.rcse.interfaces.ChatView.IFileTransfer.Status;
import com.mediatek.rcse.service.MediatekFactory;
import com.mediatek.rcse.service.binder.FileStructForBinder;
import com.mediatek.rcse.service.binder.IRemoteFileTransfer;

/**
 * Plugin ChatWindow file transfer.
 */
public class PluginChatWindowFileTransfer extends IRemoteFileTransfer.Stub {
    /**
     * The Constant TAG.
     */
    private static final String TAG = "PluginChatWindowFileTransfer";
    /**
     * The Constant STARTING.
     */
    public static final int STARTING = 0;
    /**
     * The Constant DOWNLOADING.
     */
    public static final int DOWNLOADING = 1;
    /**
     * The Constant DONE.
     */
    public static final int DONE = 2;
    /**
     * The m file transfer.
     */
    private FileStructForBinder mFileTransfer;
    /**
     * The m message box.
     */
    private int mMessageBox;
    /**
     * The m remote.
     */
    private String mRemote;
    /**
     * The m ip message.
     */
    private IpMessage mIpMessage = null;

    /**
     * Instantiates a new plugin chat window file transfer.
     *
     * @param fileTransfer the file transfer
     * @param messageBox the message box
     * @param remote the remote
     */
    public PluginChatWindowFileTransfer(FileStructForBinder fileTransfer,
            int messageBox, String remote) {
        mFileTransfer = fileTransfer;
        mMessageBox = messageBox;
        mRemote = remote;
    }
    /**
     * Inits the ip message in cache.
     */
    public void initIpMessageInCache() {
        try {
        Logger.d(TAG, "initIpMessageInCache() entry!  ");
        mIpMessage = PluginUtils.analysisFileType(mRemote, mFileTransfer);
        } catch (SecurityException e) {
            Logger.e(TAG, "Security Permission Exception()");
            //Toast.makeText(MediatekFactory.getApplicationContext(), "Permission denied to receive file. Please enable in Settings->Apps", Toast.LENGTH_LONG).show();
            e.printStackTrace();
        }
    }
    /**
     * Store in cache.
     *
     * @param messageIdInMms the message id in mms
     */
    public void storeInCache(Long messageIdInMms) {
        Logger.d(TAG, "storeInCache() entry! filePath = "
                + mFileTransfer.mFilePath);
        //IpMessage ipMessage = PluginUtils.analysisFileType(mRemote, mFileTransfer);
        IpMessageManager.addMessage(messageIdInMms,
                mFileTransfer.mFileTransferTag, mIpMessage);
        Logger.d(TAG, "storeInCache() exit!");
    }
    /**
     * Sets the file path.
     *
     * @param filePath the new file path
     * @throws RemoteException the remote exception
     */
    @Override
    public void setFilePath(String filePath) throws RemoteException {
        Logger.d(TAG, "setFilePath() entry! filePath = " + filePath);
        Long messageIdInMms = IpMessageManager
                .getMessageId(mFileTransfer.mFileTransferTag);
        if (messageIdInMms != 0) {
            IpMessage ipMessage = IpMessageManager.getMessage(messageIdInMms);
            if (ipMessage != null && ipMessage instanceof IpAttachMessage) {
                ((IpAttachMessage) ipMessage).setPath(filePath);
            } else {
                Logger.w(TAG, "setFilePath(), ipMessage is null!");
            }
        } else {
            Logger.w(TAG, "setFilePath(), not in cache!");
        }
    }
    /**
     * Sets the progress.
     *
     * @param progress the new progress
     * @throws RemoteException the remote exception
     */
    @Override
    public void setProgress(long progress) throws RemoteException {
        Logger.d(TAG, "setProgress() entry! progress = " + progress);
        Long messageIdInMms = IpMessageManager
                .getMessageId(mFileTransfer.mFileTransferTag);
        if (messageIdInMms != 0) {
            IpMessage ipMessage = IpMessageManager.getMessage(messageIdInMms);
            if (ipMessage != null) {
                progress = (progress * 100) / mFileTransfer.mFileSize;
                int messageType = ipMessage.getType();
                switch (messageType) {
                case IpMessageConsts.IpMessageType.PICTURE:
                    ((PluginIpImageMessage) ipMessage).setProgress(progress);
                    break;
                case IpMessageConsts.IpMessageType.VIDEO:
                    ((PluginIpVideoMessage) ipMessage).setProgress(progress);
                    break;
                case IpMessageConsts.IpMessageType.VOICE:
                    ((PluginIpVoiceMessage) ipMessage).setProgress(progress);
                    break;
                case IpMessageConsts.IpMessageType.VCARD:
                    ((PluginIpVcardMessage) ipMessage).setProgress(progress);
                    break;
                case IpMessageConsts.IpMessageType.CALENDAR:
                    ((PluginIpVCalendarMessage) ipMessage)
                            .setProgress(progress);
                    break;
                default:
                    ((PluginIpAttachMessage) ipMessage).setProgress(progress);
                }
                Intent it = new Intent();
                it.setAction(IpMessageConsts.DownloadAttachStatus.ACTION_DOWNLOAD_ATTACH_STATUS);
                it.putExtra(IpMessageConsts.STATUS, DOWNLOADING);
                it.putExtra(
                        IpMessageConsts.DownloadAttachStatus.DOWNLOAD_PERCENTAGE,
                        progress);
                it.putExtra(
                        IpMessageConsts.DownloadAttachStatus.DOWNLOAD_MSG_ID,
                        messageIdInMms);
                IpNotificationsManager.notify(it);
            } else {
                Logger.w(TAG, "setProgress(), ipMessage is null!");
            }
        } else {
            Logger.w(TAG, "setProgress(), not in cache!");
        }
    }
    /**
     * Sets the status.
     *
     * @param status the new status
     * @throws RemoteException the remote exception
     */
    @Override
    public void setStatus(int status) throws RemoteException {
        Logger.d(TAG, "setStatus() entry! status = " + status);
        int statusInMms = convertStatus(status);
        String path = null;
        String text = null;
        Long messageIdInMms = IpMessageManager
                .getMessageId(mFileTransfer.mFileTransferTag);
        if (messageIdInMms != null && messageIdInMms != 0) {
            IpMessage ipMessage = IpMessageManager.getMessage(messageIdInMms);
            if (ipMessage != null) {
                int messageType = ipMessage.getType();
                switch (messageType) {
                case IpMessageConsts.IpMessageType.PICTURE:
                    if (ipMessage instanceof PluginIpImageMessage) {
                        ((PluginIpImageMessage) ipMessage)
                                .setStatus(statusInMms);
                        ((PluginIpImageMessage) ipMessage).setRcsStatus(status);
                        text = ((PluginIpImageMessage) ipMessage).getCaption();
                        path = ((PluginIpImageMessage) ipMessage).getPath();
                    }
                    break;
                case IpMessageConsts.IpMessageType.VIDEO:
                    ((PluginIpVideoMessage) ipMessage).setStatus(statusInMms);
                    ((PluginIpVideoMessage) ipMessage).setRcsStatus(status);
                    text = ((PluginIpVideoMessage) ipMessage).getCaption();
                    path = ((PluginIpVideoMessage) ipMessage).getPath();
                    break;
                case IpMessageConsts.IpMessageType.VOICE:
                    ((PluginIpVoiceMessage) ipMessage).setStatus(statusInMms);
                    ((PluginIpVoiceMessage) ipMessage).setRcsStatus(status);
                    text = ((PluginIpVoiceMessage) ipMessage).getCaption();
                    path = ((PluginIpVoiceMessage) ipMessage).getPath();
                    break;
                case IpMessageConsts.IpMessageType.VCARD:
                    ((PluginIpVcardMessage) ipMessage).setStatus(statusInMms);
                    ((PluginIpVcardMessage) ipMessage).setRcsStatus(status);
                    path = ((PluginIpVcardMessage) ipMessage).getPath();
                    break;
                case IpMessageConsts.IpMessageType.CALENDAR:
                    ((PluginIpVCalendarMessage) ipMessage)
                            .setStatus(statusInMms);
                    ((PluginIpVCalendarMessage) ipMessage).setRcsStatus(status);
                    path = ((PluginIpVCalendarMessage) ipMessage).getPath();
                    break;
                default:
                    ((PluginIpAttachMessage) ipMessage).setStatus(statusInMms);
                    ((PluginIpAttachMessage) ipMessage).setRcsStatus(status);
                }
                Intent it = new Intent();
                it.setAction(IpMessageConsts.IpMessageStatus.ACTION_MESSAGE_STATUS);
                it.putExtra(IpMessageConsts.STATUS, statusInMms);
                it.putExtra(IpMessageConsts.IpMessageStatus.IP_MESSAGE_ID,
                        messageIdInMms);
                IpNotificationsManager.notify(it);
                
                if(statusInMms == IpMessageConsts.IpMessageStatus.MO_FALLBACK) {
                    Logger.d(TAG, "setStatus() fallback entry! status = " + status);
                    Intent fallbackIntent = new Intent();
                    fallbackIntent.setAction(IpMessageConsts.IpMessageStatus.IP_MESSAGE_FALLBACK_MMS);
                    fallbackIntent.putExtra(IpMessageConsts.STATUS, statusInMms);
                    fallbackIntent.putExtra(IpMessageConsts.IpMessageStatus.IP_MESSAGE_ID,
                            messageIdInMms);
                    fallbackIntent.putExtra("fileUrl", path);
                    fallbackIntent.putExtra("type", messageType);
                    fallbackIntent.putExtra("caption", text);
                    Logger.d(TAG, "setStatus() fallback exit! path = " + path + 
                            "caption=" + text + "type=" + messageType);
                    IpNotificationsManager.notify(fallbackIntent);
                    
                }
            }
        }
    }
    /**
     * Convert status.
     *
     * @param status the status
     * @return the int
     */
    private int convertStatus(int status) {
        Logger.d(TAG, "convertStatus() entry! status = " + status);
        Status enumStatus = Status.values()[status];
        int statusInMms = 0;
        if (mMessageBox == PluginUtils.OUTBOX_MESSAGE) {
            switch (enumStatus) {
            case WAITING:
            case PENDING:
                statusInMms = IpMessageConsts.IpMessageStatus.MO_INVITE;
                break;
            case TRANSFERING:
                statusInMms = IpMessageConsts.IpMessageStatus.MO_SENDING;
                break;
            case CANCEL:
                statusInMms = IpMessageConsts.IpMessageStatus.MO_CANCEL;
                break;
            case CANCELED:
                statusInMms = IpMessageConsts.IpMessageStatus.MT_CANCEL;
                break;
            case REJECTED:
                statusInMms = IpMessageConsts.IpMessageStatus.MO_REJECTED;
                break;
            case FINISHED:
                statusInMms = IpMessageConsts.IpMessageStatus.MO_SENT;
                break;
            case FAILED:
                statusInMms = IpMessageConsts.IpMessageStatus.FAILED;
                break;
            case DELIVERED:
                statusInMms = IpMessageConsts.IpMessageStatus.MO_DELIVERED;
                break;
            case DISPLAYED:
                statusInMms = IpMessageConsts.IpMessageStatus.MO_VIEWED;
                break;
            case NOT_SUPPORTED:
                statusInMms = IpMessageConsts.IpMessageStatus.MEDIA_NOT_SUPPORTED;
                break;
            case TIMEOUT:
                statusInMms = IpMessageConsts.IpMessageStatus.MO_TIMEOUT;
                break;
            case FALLBACK_MMS:
                statusInMms = IpMessageConsts.IpMessageStatus.MO_FALLBACK;
                break;
            case RESUME:
                statusInMms = IpMessageConsts.IpMessageStatus.MT_RESUME;
                break;
            default:
                break;
            }
        } else {
            switch (enumStatus) {
            case WAITING:
                statusInMms = IpMessageConsts.IpMessageStatus.MT_INVITED;
                break;
            case TRANSFERING:
                statusInMms = IpMessageConsts.IpMessageStatus.MT_RECEIVING;
                break;
            case CANCEL:
                statusInMms = IpMessageConsts.IpMessageStatus.MO_CANCEL;
                break;
            case CANCELED:
                statusInMms = IpMessageConsts.IpMessageStatus.MT_CANCEL;
                break;
            case REJECTED:
                statusInMms = IpMessageConsts.IpMessageStatus.MT_REJECT;
                break;
            case FINISHED:
                statusInMms = IpMessageConsts.IpMessageStatus.MT_RECEIVED;
                break;
            case FAILED:
                statusInMms = IpMessageConsts.IpMessageStatus.MT_CANCEL;
                break;
            case MEDIA_NOT_RECEIVED:
                statusInMms = IpMessageConsts.IpMessageStatus.MT_NOT_RECEIVED;
                break;
            default:
                break;
            }
        }
        Logger.d(TAG, "convertStatus() exit! status = " + statusInMms);
        return statusInMms;
    }
    /**
     * Update tag.
     *
     * @param transferTag the transfer tag
     * @param transferSize the transfer size
     * @throws RemoteException the remote exception
     */
    @Override
    public void updateTag(String transferTag, long transferSize)
            throws RemoteException {
        String oldTag = mFileTransfer.mFileTransferTag;
        mFileTransfer.mFileTransferTag = transferTag;
        mFileTransfer.mFileSize = transferSize;
        PluginUtils.updateMessageIdInMmsDb(oldTag, transferTag);
        IpMessageManager.updateCache(oldTag, mFileTransfer, mRemote);
    }
}
