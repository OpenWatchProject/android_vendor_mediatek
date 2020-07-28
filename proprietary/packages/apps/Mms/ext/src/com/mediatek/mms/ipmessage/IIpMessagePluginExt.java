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

package com.mediatek.mms.ipmessage;




public interface IIpMessagePluginExt {

    /**
     * get IpMessageListItem from plugin or default.
     * @return IpMessageListItem
     * @internal
     */
    IIpMessageListItemExt getIpMessageListItem();

    /**
     * get IpMessageItem from plugin or default.
     * @return IpMessageItem
     * @internal
     */
    IIpMessageItemExt getIpMessageItem();

    /**
     * get IpMessageItem from plugin or default.
     * @return IpMessageItem
     * @internal
     */
    IIpComposeActivityExt getIpComposeActivity();

    /**
     * get IpMessageItem from plugin or default.
     * @return IpMessageItem
     * @internal
     */
    IIpContactExt getIpContact();

    /**
     * get IpMessageItem from plugin or default.
     * @return IpMessageItem
     * @internal
     */
    IIpConversationExt getIpConversation();

    /**
     * get IpMessageItem from plugin or default.
     * @return IpMessageItem
     * @internal
     */
    IIpConversationListExt getIpConversationList();

    /**
     * get IpConversationListItem from plugin or default.
     * @return IpConversationListItem
     * @internal
     */
    IIpConversationListItemExt getIpConversationListItem();

    /**
     * get IpDialogModeActivity from plugin or default.
     * @return IpDialogModeActivity
     * @internal
     */
    IIpDialogModeActivityExt getIpDialogModeActivity();

    /**
     * get IpMessageListAdapter from plugin or default.
     * @return IpMessageListAdapter
     * @internal
     */
    IIpMessageListAdapterExt getIpMessageListAdapter();

    /**
     * get IpMessagingNotification from plugin or default.
     * @return IpMessagingNotification
     * @internal
     */
    IIpMessagingNotificationExt getIpMessagingNotification();

    /**
     * get IpMultiDeleteActivity from plugin or default.
     * @return IpMultiDeleteActivity
     * @internal
     */
    IIpMultiDeleteActivityExt getIpMultiDeleteActivity();

    /**
     * get IpSearchActivity from plugin or default.
     * @return IpSearchActivity
     * @internal
     */
    IIpSearchActivityExt getIpSearchActivity();

    /**
     * get IpConfig from plugin or default.
     * @return IpConfig
     * @internal
     */
    IMmsConfigExt getIpConfig();

    /**
     * get IpSettingListActivity from plugin or default.
     * @return IpSettingListActivity
     * @internal
     */
    IIpSettingListActivityExt getIpSettingListActivity();

    /**
     * get IpUtils from plugin or default.
     * @return IpUtils
     * @internal
     */
    IIpUtilsExt getIpUtils();

    /**
     * get IpScrollListener from plugin or default.
     * @return IpScrollListener
     * @internal
     */
    IIpScrollListenerExt getIpScrollListener();

    /**
     * get IpSpamMsgReceiver from plugin or default.
     * @return IpSpamMsgReceiver
     * @internal
     */
    IIpSpamMsgReceiverExt getIpSpamMsgReceiver();

    /**
     * getIpColumnsMap.
     * @return IIpColumnsMapExt
     * @internal
     */
    IIpColumnsMapExt getIpColumnsMap();

    /**
     * getIpSuggestionsProvider
     * @return IIpSuggestionsProviderExt
     * @internal
     */
    IIpSuggestionsProviderExt getIpSuggestionsProvider();

    /**
     * getIpWidgetService.
     * @return IIpWidgetServiceExt
     * @internal
     */
    IIpWidgetServiceExt getIpWidgetService();

    /**
     * getIpRecipientsEditorExt
     * @return IIpRecipientsEditorExt
     * @internal
     */
    IIpRecipientsEditorExt getIpRecipientsEditorExt();
}

