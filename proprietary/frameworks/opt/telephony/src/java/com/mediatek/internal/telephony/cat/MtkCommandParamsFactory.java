/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2017. All rights reserved.
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
 * have been modified by MediaTek Inc. All revisions are subject to any receiver\'s
 * applicable license agreements with MediaTek Inc.
 */

package com.mediatek.internal.telephony.cat;

import android.content.Context;
import android.content.Intent;
import android.graphics.Bitmap;
import android.os.Handler;
import android.os.Message;

import com.android.internal.telephony.cat.BerTlv;
import com.android.internal.telephony.cat.CommandDetails;
import com.android.internal.telephony.cat.CommandParams;
import com.android.internal.telephony.cat.CommandParamsFactory;
import com.android.internal.telephony.cat.ComprehensionTlv;
import com.android.internal.telephony.cat.ResultException;
import com.android.internal.telephony.cat.ResultCode;
import com.android.internal.telephony.cat.ComprehensionTlv;
import com.android.internal.telephony.cat.ValueParser;
import com.android.internal.telephony.cat.TextMessage;
import com.android.internal.telephony.cat.Input;
import com.android.internal.telephony.cat.PresentationType;
import com.android.internal.telephony.cat.Tone;
import com.android.internal.telephony.cat.Item;
import com.android.internal.telephony.cat.Duration;
import com.android.internal.telephony.cat.RilMessageDecoder;
import com.android.internal.telephony.cat.AppInterface;
import com.android.internal.telephony.cat.SelectItemParams;
import com.android.internal.telephony.cat.DisplayTextParams;
import com.android.internal.telephony.cat.LaunchBrowserParams;
import com.android.internal.telephony.cat.PlayToneParams;
import com.android.internal.telephony.cat.CallSetupParams;
import com.android.internal.telephony.cat.GetInputParams;
import com.android.internal.telephony.cat.LaunchBrowserMode;
import com.android.internal.telephony.cat.ItemsIconId;
import com.android.internal.telephony.cat.IconId;
import com.android.internal.telephony.cat.SetEventListParams;
import com.android.internal.telephony.cat.Menu;
import com.android.internal.telephony.cat.ComprehensionTlvTag;
import com.android.internal.telephony.uicc.IccFileHandler;
import com.android.internal.telephony.GsmAlphabet;
import com.android.internal.telephony.uicc.IccFileHandler;

import java.util.Iterator;
import java.util.List;

import static com.android.internal.telephony.cat.CatCmdMessage.
                   SetupEventListConstants.LANGUAGE_SELECTION_EVENT;
import static com.android.internal.telephony.cat.CatCmdMessage.
                   SetupEventListConstants.BROWSER_TERMINATION_EVENT;
import static com.android.internal.telephony.cat.CatCmdMessage.
                   SetupEventListConstants.BROWSING_STATUS_EVENT;

/**
 * Factory class, used for decoding raw byte arrays, received from baseband,
 * into a CommandParams object.
 *
 */

public class MtkCommandParamsFactory extends CommandParamsFactory {

    private Context mContext;
    // used to mark the index of tlv object in a tlv list
    int tlvIndex = -1;

    public static final int BATTERY_STATE = 0x0A;
    public  MtkCommandParamsFactory(RilMessageDecoder caller, IccFileHandler fh) {
        super(caller, fh);
        mContext =((MtkCatService)(caller.mCaller)).getContext();
    }

    protected void sendCmdParams(ResultCode resCode) {
        if (mCaller != null) {
            mCaller.sendMsgParamsDecoded(resCode, mCmdParams);
        } else {
            MtkCatLog.e(this,"mCaller is null!!!");
        }
    }

    private void resetTlvIndex() {
        tlvIndex = -1;
    }

    /**
     * Search for the next COMPREHENSION-TLV object with the given tag from a
     * list iterated by {@code iter}. {@code iter} points to the object next to
     * the found object when this method returns. Used for searching the same
     * list for similar tags, usually item id. At the same time, this method
     * will update a index to mark the position of the tlv object in the
     * comprehension- tlv.
     *
     * @param tag A tag to search for
     * @param iter Iterator for ComprehensionTlv objects used for search
     * @return A ComprehensionTlv object that has the tag value of {@code tag}.
     *         If no object is found with the tag, null is returned.
     */
    private ComprehensionTlv searchForNextTagAndIndex(ComprehensionTlvTag tag,
            Iterator<ComprehensionTlv> iter) {
        if (tag == null || iter == null) {
            MtkCatLog.d(this, "CPF-searchForNextTagAndIndex: Invalid params");
            return null;
        }

        int tagValue = tag.value();

        while (iter.hasNext()) {
            ++tlvIndex;
            ComprehensionTlv ctlv = iter.next();
            if (ctlv.getTag() == tagValue) {
                return ctlv;
            }
        }

        // tlvIndex = -1;
        return null;
    }

    /**
     * Search for a COMPREHENSION-TLV object with the given tag from a list and
     * provide the index of searched tlv object.
     *
     * @param tag A tag to search for
     * @param ctlvs List of ComprehensionTlv objects used to search in
     * @return A ComprehensionTlv object that has the tag value of {@code tag}.
     *         If no object is found with the tag, null is returned.
     */
    private ComprehensionTlv searchForTagAndIndex(ComprehensionTlvTag tag,
            List<ComprehensionTlv> ctlvs) {
        // tlvIndex = -1;
        resetTlvIndex();
        Iterator<ComprehensionTlv> iter = ctlvs.iterator();
        return searchForNextTagAndIndex(tag, iter);
    }

    /**
     * Processes DISPLAY_TEXT proactive command from the SIM card.
     *
     * @param cmdDet Command Details container object.
     * @param ctlvs List of ComprehensionTlv objects following Command Details
     *        object and Device Identities object within the proactive command
     * @return true if the command is processing is pending and additional
     *         asynchronous processing is required.
     * @throws ResultException
     */
    @Override
    protected boolean processDisplayText(CommandDetails cmdDet,
            List<ComprehensionTlv> ctlvs)
            throws ResultException {

        MtkCatLog.d(this, "process DisplayText");

        TextMessage textMsg = new TextMessage();
        IconId iconId = null;

        ComprehensionTlv ctlv = searchForTag(ComprehensionTlvTag.TEXT_STRING,
                ctlvs);
        if (ctlv != null) {
            textMsg.text = ValueParser.retrieveTextString(ctlv);
        }
        // If the tlv object doesn't exist or the it is a null object reply
        // with command not understood.
        if (textMsg.text == null) {
            throw new ResultException(ResultCode.CMD_DATA_NOT_UNDERSTOOD);
        }

        ctlv = searchForTag(ComprehensionTlvTag.IMMEDIATE_RESPONSE, ctlvs);
        if (ctlv != null) {
            textMsg.responseNeeded = false;
        }
        // parse icon identifier
        ctlv = searchForTag(ComprehensionTlvTag.ICON_ID, ctlvs);
        if (ctlv != null) {
            try {
                iconId = ValueParser.retrieveIconId(ctlv);
            } catch (ResultException e) {
                MtkCatLog.e(this, "retrieveIconId ResultException: " + e.result());
            }
            try {
                textMsg.iconSelfExplanatory = iconId.selfExplanatory;
            } catch (NullPointerException ne) {
                MtkCatLog.e(this, "iconId is null.");
            }
        }
        // parse tone duration
        ctlv = searchForTag(ComprehensionTlvTag.DURATION, ctlvs);
        if (ctlv != null) {
            try {
                textMsg.duration = ValueParser.retrieveDuration(ctlv);
            } catch (ResultException e) {
                MtkCatLog.e(this, "retrieveDuration ResultException: " + e.result());
            }
        }

        // Parse command qualifier parameters.
        textMsg.isHighPriority = (cmdDet.commandQualifier & 0x01) != 0;
        textMsg.userClear = (cmdDet.commandQualifier & 0x80) != 0;

        mCmdParams = new DisplayTextParams(cmdDet, textMsg);

        if (iconId != null) {
            mloadIcon = true;
            mIconLoadState = LOAD_SINGLE_ICON;
            mIconLoader.loadIcon(iconId.recordNumber, this
                    .obtainMessage(MSG_ID_LOAD_ICON_DONE));
            return true;
        }
        return false;
    }

    /**
     * Processes GET_INKEY proactive command from the SIM card.
     *
     * @param cmdDet Command Details container object.
     * @param ctlvs List of ComprehensionTlv objects following Command Details
     *        object and Device Identities object within the proactive command
     * @return true if the command is processing is pending and additional
     *         asynchronous processing is required.
     * @throws ResultException
     */
    @Override
    protected boolean processGetInkey(CommandDetails cmdDet,
            List<ComprehensionTlv> ctlvs) throws ResultException {

        MtkCatLog.d(this, "process GetInkey");

        Input input = new Input();
        IconId iconId = null;

        ComprehensionTlv ctlv = searchForTag(ComprehensionTlvTag.TEXT_STRING,
                ctlvs);
        if (ctlv != null) {
            input.text = ValueParser.retrieveTextString(ctlv);
        } else {
            throw new ResultException(ResultCode.REQUIRED_VALUES_MISSING);
        }
        // parse icon identifier
        ctlv = searchForTag(ComprehensionTlvTag.ICON_ID, ctlvs);
        if (ctlv != null) {
            try {
                iconId = ValueParser.retrieveIconId(ctlv);
            } catch (ResultException e) {
                MtkCatLog.e(this, "retrieveIconId ResultException: " + e.result());
            }

            try {
                input.iconSelfExplanatory = iconId.selfExplanatory;
            } catch (NullPointerException ne) {
                MtkCatLog.e(this, "iconId is null.");
            }
        }

        // parse duration
        ctlv = searchForTag(ComprehensionTlvTag.DURATION, ctlvs);
        if (ctlv != null) {
            try {
                input.duration = ValueParser.retrieveDuration(ctlv);
            } catch (ResultException e) {
                MtkCatLog.e(this, "retrieveDuration ResultException: " + e.result());
            }
        }

        input.minLen = 1;
        input.maxLen = 1;

        input.digitOnly = (cmdDet.commandQualifier & 0x01) == 0;
        input.ucs2 = (cmdDet.commandQualifier & 0x02) != 0;
        input.yesNo = (cmdDet.commandQualifier & 0x04) != 0;
        input.helpAvailable = (cmdDet.commandQualifier & 0x80) != 0;
        input.echo = true;

        mCmdParams = new GetInputParams(cmdDet, input);

        if (iconId != null) {
            mloadIcon = true;
            mIconLoadState = LOAD_SINGLE_ICON;
            mIconLoader.loadIcon(iconId.recordNumber, this
                    .obtainMessage(MSG_ID_LOAD_ICON_DONE));
            return true;
        }
        return false;
    }

    /**
     * Processes GET_INPUT proactive command from the SIM card.
     *
     * @param cmdDet Command Details container object.
     * @param ctlvs List of ComprehensionTlv objects following Command Details
     *        object and Device Identities object within the proactive command
     * @return true if the command is processing is pending and additional
     *         asynchronous processing is required.
     * @throws ResultException
     */
    @Override
    protected boolean processGetInput(CommandDetails cmdDet,
            List<ComprehensionTlv> ctlvs) throws ResultException {

        MtkCatLog.d(this, "process GetInput");

        Input input = new Input();
        IconId iconId = null;

        ComprehensionTlv ctlv = searchForTag(ComprehensionTlvTag.TEXT_STRING,
                ctlvs);
        if (ctlv != null) {
            input.text = ValueParser.retrieveTextString(ctlv);
        } else {
            throw new ResultException(ResultCode.REQUIRED_VALUES_MISSING);
        }

        ctlv = searchForTag(ComprehensionTlvTag.RESPONSE_LENGTH, ctlvs);
        if (ctlv != null) {
            try {
                byte[] rawValue = ctlv.getRawValue();
                int valueIndex = ctlv.getValueIndex();
                // The maximum input lenght is 239, because the
                // maximum length of proactive command is 255
                input.minLen = rawValue[valueIndex] & 0xff;
                if (input.minLen > 239) {
                    input.minLen = 239;
                }

                input.maxLen = rawValue[valueIndex + 1] & 0xff;
                if (input.maxLen > 239) {
                    input.maxLen = 239;
                }
            } catch (IndexOutOfBoundsException e) {
                throw new ResultException(ResultCode.CMD_DATA_NOT_UNDERSTOOD);
            }
        } else {
            throw new ResultException(ResultCode.REQUIRED_VALUES_MISSING);
        }

        ctlv = searchForTag(ComprehensionTlvTag.DEFAULT_TEXT, ctlvs);
        if (ctlv != null) {
            try {
                input.defaultText = ValueParser.retrieveTextString(ctlv);
            } catch (ResultException e) {
                MtkCatLog.e(this, "retrieveTextString ResultException: " + e.result());
            }
        }
        // parse icon identifier
        ctlv = searchForTag(ComprehensionTlvTag.ICON_ID, ctlvs);
        if (ctlv != null) {
            try {
                iconId =ValueParser.retrieveIconId(ctlv);
            } catch (ResultException e) {
                MtkCatLog.e(this, "retrieveIconId ResultException: " + e.result());
            }

            try {
                input.iconSelfExplanatory = iconId.selfExplanatory;
            } catch (NullPointerException ne) {
                MtkCatLog.e(this, "iconId is null.");
            }
        }

        // parse duration
        ctlv = searchForTag(ComprehensionTlvTag.DURATION, ctlvs);
        if (ctlv != null) {
            try {
                input.duration = ValueParser.retrieveDuration(ctlv);
            } catch (ResultException e) {
                MtkCatLog.e(this, "retrieveDuration ResultException: " + e.result());
            }
        }

        input.digitOnly = (cmdDet.commandQualifier & 0x01) == 0;
        input.ucs2 = (cmdDet.commandQualifier & 0x02) != 0;
        input.echo = (cmdDet.commandQualifier & 0x04) == 0;
        input.packed = (cmdDet.commandQualifier & 0x08) != 0;
        input.helpAvailable = (cmdDet.commandQualifier & 0x80) != 0;

        // Truncate the maxLen if it exceeds the max number of chars that can
        // be encoded. Limit depends on DCS in Command Qualifier.
        if (input.ucs2 && input.maxLen > MAX_UCS2_CHARS) {
            MtkCatLog.d(this, "UCS2: received maxLen = " + input.maxLen +
                  ", truncating to " + MAX_UCS2_CHARS);
            input.maxLen = MAX_UCS2_CHARS;
        } else if (!input.packed && input.maxLen > MAX_GSM7_DEFAULT_CHARS) {
            MtkCatLog.d(this, "GSM 7Bit Default: received maxLen = " + input.maxLen +
                  ", truncating to " + MAX_GSM7_DEFAULT_CHARS);
            input.maxLen = MAX_GSM7_DEFAULT_CHARS;
        }

        mCmdParams = new GetInputParams(cmdDet, input);

        if (iconId != null) {
            mloadIcon = true;
            mIconLoadState = LOAD_SINGLE_ICON;
            mIconLoader.loadIcon(iconId.recordNumber, this
                    .obtainMessage(MSG_ID_LOAD_ICON_DONE));
            return true;
        }
        return false;
    }

    /**
     * Processes SELECT_ITEM proactive command from the SIM card.
     *
     * @param cmdDet Command Details container object.
     * @param ctlvs List of ComprehensionTlv objects following Command Details
     *        object and Device Identities object within the proactive command
     * @return true if the command is processing is pending and additional
     *         asynchronous processing is required.
     * @throws ResultException
     */
    @Override
    protected boolean processSelectItem(CommandDetails cmdDet,
            List<ComprehensionTlv> ctlvs) throws ResultException {

        MtkCatLog.d(this, "process SelectItem");

        MtkMenu menu = new MtkMenu();
        IconId titleIconId = null;
        ItemsIconId itemsIconId = null;
        Iterator<ComprehensionTlv> iter = ctlvs.iterator();

        AppInterface.CommandType cmdType =
                                    AppInterface.CommandType.fromInt(cmdDet.typeOfCommand);

        ComprehensionTlv ctlv = searchForTag(ComprehensionTlvTag.ALPHA_ID,
                ctlvs);
        if (ctlv != null) {
            try {
                menu.title = MtkValueParser.retrieveAlphaId(ctlv);
            } catch (ResultException e) {
                MtkCatLog.e(this, "retrieveAlphaId ResultException: " + e.result());
            }
            MtkCatLog.d(this, "add AlphaId: " + menu.title);
        } else if (cmdType == AppInterface.CommandType.SET_UP_MENU) {
            // According to spec ETSI TS 102 223 section 6.10.3, the
            // Alpha ID is mandatory (and also part of minimum set of
            // elements required) for SET_UP_MENU. If it is not received
            // by ME, then ME should respond with "error: missing minimum
            // information" and not "command performed successfully".
            throw new ResultException(ResultCode.REQUIRED_VALUES_MISSING);
        }

        while (true) {
            ctlv = searchForNextTag(ComprehensionTlvTag.ITEM, iter);
            if (ctlv != null) {
                Item item = MtkValueParser.retrieveItem(ctlv);
                MtkCatLog.d(this, "add menu item: " + ((item == null) ? "" : item.toString()));
                menu.items.add(item);
            } else {
                break;
            }
        }

        // We must have at least one menu item.
        if (menu.items.size() == 0) {
            MtkCatLog.d(this, "no menu item");
            throw new ResultException(ResultCode.REQUIRED_VALUES_MISSING);
        }

        ctlv = searchForTag(ComprehensionTlvTag.NEXT_ACTION_INDICATOR, ctlvs);
        if (ctlv != null) {
            try {
            menu.nextActionIndicator = MtkValueParser.retrieveNextActionIndicator(ctlv);
            } catch (ResultException e) {
                MtkCatLog.e(this, "retrieveNextActionIndicator ResultException: " + e.result());
            }
            try {
            if (menu.nextActionIndicator.length != menu.items.size()) {
                MtkCatLog.d(this, "nextActionIndicator.length != number of menu items");
                menu.nextActionIndicator = null;
            }
            } catch (NullPointerException ne) {
                MtkCatLog.e(this, "nextActionIndicator is null.");
            }
        }

        ctlv = searchForTag(ComprehensionTlvTag.ITEM_ID, ctlvs);
        if (ctlv != null) {
            // CAT items are listed 1...n while list start at 0, need to
            // subtract one.
            try {
                menu.defaultItem = ValueParser.retrieveItemId(ctlv) - 1;
            } catch (ResultException e) {
                MtkCatLog.e(this, "retrieveItemId ResultException: " + e.result());
            }
            MtkCatLog.d(this, "default item: " + menu.defaultItem);
        }

        ctlv = searchForTag(ComprehensionTlvTag.ICON_ID, ctlvs);
        if (ctlv != null) {
            mIconLoadState = LOAD_SINGLE_ICON;
            try {
                titleIconId = ValueParser.retrieveIconId(ctlv);
            } catch (ResultException e) {
                MtkCatLog.e(this, "retrieveIconId ResultException: " + e.result());
            }
            try {
                menu.titleIconSelfExplanatory = titleIconId.selfExplanatory;
            } catch (NullPointerException ne) {
                MtkCatLog.e(this, "titleIconId is null.");
            }
        }

        ctlv = searchForTag(ComprehensionTlvTag.ITEM_ICON_ID_LIST, ctlvs);
        if (ctlv != null) {
            mIconLoadState = LOAD_MULTI_ICONS;
            try {
                itemsIconId = ValueParser.retrieveItemsIconId(ctlv);
            } catch (ResultException e) {
                MtkCatLog.e(this, "retrieveItemsIconId ResultException: " + e.result());
            }
            try {
                menu.itemsIconSelfExplanatory = itemsIconId.selfExplanatory;
            } catch (NullPointerException ne) {
                MtkCatLog.e(this, "itemsIconId is null.");
            }
        }

        boolean presentTypeSpecified = (cmdDet.commandQualifier & 0x01) != 0;
        if (presentTypeSpecified) {
            if ((cmdDet.commandQualifier & 0x02) == 0) {
                menu.presentationType = PresentationType.DATA_VALUES;
            } else {
                menu.presentationType = PresentationType.NAVIGATION_OPTIONS;
            }
        }
        menu.softKeyPreferred = (cmdDet.commandQualifier & 0x04) != 0;
        menu.helpAvailable = (cmdDet.commandQualifier & 0x80) != 0;

        mCmdParams = new SelectItemParams(cmdDet, menu, titleIconId != null);

        // Load icons data if needed.
        switch(mIconLoadState) {
        case LOAD_NO_ICON:
            return false;
        case LOAD_SINGLE_ICON:
            if (titleIconId != null && titleIconId.recordNumber > 0) {
                mloadIcon = true;
                mIconLoader.loadIcon(titleIconId.recordNumber, this
                        .obtainMessage(MSG_ID_LOAD_ICON_DONE));
                break;
            } else {
                return false;
            }
        case LOAD_MULTI_ICONS:
            if (itemsIconId != null) {
                int[] recordNumbers = itemsIconId.recordNumbers;
                if (titleIconId != null) {
                    // Create a new array for all the icons (title and items).
                    recordNumbers = new int[itemsIconId.recordNumbers.length + 1];
                    recordNumbers[0] = titleIconId.recordNumber;
                    System.arraycopy(itemsIconId.recordNumbers, 0, recordNumbers,
                            1, itemsIconId.recordNumbers.length);
                }
                mloadIcon = true;
                mIconLoader.loadIcons(recordNumbers, this
                        .obtainMessage(MSG_ID_LOAD_ICON_DONE));
                break;
            } else {
                return false;
            }
        }
        return true;
    }

    /**
     * Processes EVENT_NOTIFY message from baseband.
     *
     * @param cmdDet Command Details container object.
     * @param ctlvs List of ComprehensionTlv objects following Command Details
     *        object and Device Identities object within the proactive command
     * @return true if the command is processing is pending and additional
     *         asynchronous processing is required.
     */
    @Override
    protected boolean processEventNotify(CommandDetails cmdDet,
            List<ComprehensionTlv> ctlvs) throws ResultException {

        MtkCatLog.d(this, "process EventNotify");

        TextMessage textMsg = new TextMessage();
        IconId iconId = null;

        ComprehensionTlv ctlv = searchForTag(ComprehensionTlvTag.ALPHA_ID,
                ctlvs);
        if (ctlv != null) {
            textMsg.text = MtkValueParser.retrieveAlphaId(ctlv);
        } else {
            // throw new ResultException(ResultCode.REQUIRED_VALUES_MISSING);
            textMsg.text = null;
        }

        ctlv = searchForTag(ComprehensionTlvTag.ICON_ID, ctlvs);
        if (ctlv != null) {
            iconId = ValueParser.retrieveIconId(ctlv);
            textMsg.iconSelfExplanatory = iconId.selfExplanatory;
        }

        textMsg.responseNeeded = false;
        mCmdParams = new DisplayTextParams(cmdDet, textMsg);

        if (iconId != null) {
            mloadIcon = true;
            mIconLoadState = LOAD_SINGLE_ICON;
            mIconLoader.loadIcon(iconId.recordNumber, this
                    .obtainMessage(MSG_ID_LOAD_ICON_DONE));
            return true;
        }
        return false;
    }


    /**
     *            object and Device Identities object within the proactive
     *            command.
     * @return true if the command is processing is pending and additional
     *         asynchronous processing is required.
     */
    @Override
    protected  boolean processSetUpEventList(CommandDetails cmdDet,
            List<ComprehensionTlv> ctlvs){

        MtkCatLog.d(this, "process SetUpEventList");
        int[] eventList;
        ComprehensionTlv ctlv = searchForTag(ComprehensionTlvTag.EVENT_LIST, ctlvs);
        if (ctlv != null) {
            try {
                byte[] rawValue = ctlv.getRawValue();
                int valueIndex = ctlv.getValueIndex();
                int valueLen = ctlv.getLength();

                eventList = new int[valueLen];
                for (int index = 0; index < valueLen;) {
                    eventList[index] = rawValue[valueIndex];
                    MtkCatLog.v(this, "CPF-processSetUpEventList: eventList[" + index + "] = "
                            + eventList[index]);
                    index++;
                    valueIndex++;
                }
                mCmdParams = new SetEventListParams(cmdDet, eventList);
            } catch (IndexOutOfBoundsException e) {
                MtkCatLog.e(this, " IndexOutofBoundException in processSetUpEventList");;
            }
        }

        return false;
    }

    /**
     * Processes LAUNCH_BROWSER proactive command from the SIM card.
     *
     * @param cmdDet Command Details container object.
     * @param ctlvs List of ComprehensionTlv objects following Command Details
     *        object and Device Identities object within the proactive command
     * @return true if the command is processing is pending and additional
     *         asynchronous processing is required.
     * @throws ResultException
     */
    @Override
    protected boolean processLaunchBrowser(CommandDetails cmdDet,
            List<ComprehensionTlv> ctlvs) throws ResultException {

        MtkCatLog.d(this, "process LaunchBrowser");

        TextMessage confirmMsg = new TextMessage();
        IconId iconId = null;
        String url = null;

        ComprehensionTlv ctlv = searchForTag(ComprehensionTlvTag.URL, ctlvs);
        if (ctlv != null) {
            try {
                byte[] rawValue = ctlv.getRawValue();
                int valueIndex = ctlv.getValueIndex();
                int valueLen = ctlv.getLength();
                if (valueLen > 0) {
                    url = GsmAlphabet.gsm8BitUnpackedToString(rawValue,
                            valueIndex, valueLen);
                } else {
                    url = null;
                }
            } catch (IndexOutOfBoundsException e) {
                throw new ResultException(ResultCode.CMD_DATA_NOT_UNDERSTOOD);
            }
        }

        // parse alpha identifier.
        ctlv = searchForTag(ComprehensionTlvTag.ALPHA_ID, ctlvs);
        if (ctlv != null) {
            confirmMsg.text = MtkValueParser.retrieveAlphaId(ctlv);
        }
        // parse icon identifier
        ctlv = searchForTag(ComprehensionTlvTag.ICON_ID, ctlvs);
        if (ctlv != null) {
            iconId = ValueParser.retrieveIconId(ctlv);
            confirmMsg.iconSelfExplanatory = iconId.selfExplanatory;
        }

        // parse command qualifier value.
        LaunchBrowserMode mode;
        switch (cmdDet.commandQualifier) {
        case 0x00:
        default:
            mode = LaunchBrowserMode.LAUNCH_IF_NOT_ALREADY_LAUNCHED;
            break;
        case 0x02:
            mode = LaunchBrowserMode.USE_EXISTING_BROWSER;
            break;
        case 0x03:
            mode = LaunchBrowserMode.LAUNCH_NEW_BROWSER;
            break;
        }

        mCmdParams = new LaunchBrowserParams(cmdDet, confirmMsg, url, mode);

        if (iconId != null) {
            mIconLoadState = LOAD_SINGLE_ICON;
            mIconLoader.loadIcon(iconId.recordNumber, this
                    .obtainMessage(MSG_ID_LOAD_ICON_DONE));
            return true;
        }
        return false;
    }

     /**
     * Processes PLAY_TONE proactive command from the SIM card.
     *
     * @param cmdDet Command Details container object.
     * @param ctlvs List of ComprehensionTlv objects following Command Details
     *        object and Device Identities object within the proactive command
     * @return true if the command is processing is pending and additional
     *         asynchronous processing is required.t
     * @throws ResultException
     */
    @Override
    protected boolean processPlayTone(CommandDetails cmdDet,
            List<ComprehensionTlv> ctlvs) throws ResultException {

        MtkCatLog.d(this, "process PlayTone");

        Tone tone = null;
        TextMessage textMsg = new TextMessage();
        Duration duration = null;
        IconId iconId = null;

        ComprehensionTlv ctlv = searchForTag(ComprehensionTlvTag.TONE, ctlvs);
        if (ctlv != null) {
            // Nothing to do for null objects.
            if (ctlv.getLength() > 0) {
                try {
                    byte[] rawValue = ctlv.getRawValue();
                    int valueIndex = ctlv.getValueIndex();
                    int toneVal = rawValue[valueIndex];
                    tone = Tone.fromInt(toneVal);
                } catch (IndexOutOfBoundsException e) {
                    throw new ResultException(
                            ResultCode.CMD_DATA_NOT_UNDERSTOOD);
                }
            }
        }
        // parse alpha identifier
        ctlv = searchForTag(ComprehensionTlvTag.ALPHA_ID, ctlvs);
        if (ctlv != null) {
            try {
            textMsg.text = MtkValueParser.retrieveAlphaId(ctlv);
            } catch (ResultException e) {
                MtkCatLog.e(this, "retrieveAlphaId ResultException: " + e.result());
            }
        }
        // parse tone duration
        ctlv = searchForTag(ComprehensionTlvTag.DURATION, ctlvs);
        if (ctlv != null) {
            try {
            duration = ValueParser.retrieveDuration(ctlv);
            } catch (ResultException e) {
                MtkCatLog.e(this, "retrieveDuration ResultException: " + e.result());
            }
        }
        // parse icon identifier
        ctlv = searchForTag(ComprehensionTlvTag.ICON_ID, ctlvs);
        if (ctlv != null) {
            iconId = ValueParser.retrieveIconId(ctlv);
            textMsg.iconSelfExplanatory = iconId.selfExplanatory;
        }

        boolean vibrate = (cmdDet.commandQualifier & 0x01) != 0x00;

        textMsg.responseNeeded = false;
        mCmdParams = new PlayToneParams(cmdDet, textMsg, tone, duration, vibrate);

        if (iconId != null) {
            mIconLoadState = LOAD_SINGLE_ICON;
            mIconLoader.loadIcon(iconId.recordNumber, this
                    .obtainMessage(MSG_ID_LOAD_ICON_DONE));
            return true;
        }
        return false;
    }

    /**
     * Processes SETUP_CALL proactive command from the SIM card.
     *
     * @param cmdDet Command Details object retrieved from the proactive command
     *        object
     * @param ctlvs List of ComprehensionTlv objects following Command Details
     *        object and Device Identities object within the proactive command
     * @return true if the command is processing is pending and additional
     *         asynchronous processing is required.
     */
    @Override
    protected boolean processSetupCall(CommandDetails cmdDet,
            List<ComprehensionTlv> ctlvs) throws ResultException {
        MtkCatLog.d(this, "process SetupCall");

        Iterator<ComprehensionTlv> iter = ctlvs.iterator();
        ComprehensionTlv ctlv = null;
        // User confirmation phase message.
        TextMessage confirmMsg = new TextMessage();
        // Call set up phase message.
        TextMessage callMsg = new TextMessage();
        IconId confirmIconId = null;
        IconId callIconId = null;

        // The structure of SET UP CALL
        // alpha id -> address -> icon id -> alpha id -> icon id
        // We use the index of alpha id to judge the type of alpha id:
        // confirm or call
        final int addrIndex = getAddrIndex(ctlvs);
        if (-1 == addrIndex) {
            MtkCatLog.d(this, "fail to get ADDRESS data object");
            return false;
        }

        final int alpha1Index = getConfirmationAlphaIdIndex(ctlvs, addrIndex);
        final int alpha2Index = getCallingAlphaIdIndex(ctlvs, addrIndex);

        ctlv = getConfirmationAlphaId(ctlvs, addrIndex);
        if (ctlv != null) {
            confirmMsg.text = MtkValueParser.retrieveAlphaId(ctlv);
        }

        ctlv = getConfirmationIconId(ctlvs, alpha1Index, alpha2Index);
        if (ctlv != null) {
            confirmIconId = ValueParser.retrieveIconId(ctlv);
            confirmMsg.iconSelfExplanatory = confirmIconId.selfExplanatory;
        }

        ctlv = getCallingAlphaId(ctlvs, addrIndex);
        if (ctlv != null) {
            callMsg.text = MtkValueParser.retrieveAlphaId(ctlv);
        }

        ctlv = getCallingIconId(ctlvs, alpha2Index);
        if (ctlv != null) {
            callIconId = ValueParser.retrieveIconId(ctlv);
            callMsg.iconSelfExplanatory = callIconId.selfExplanatory;
        }

        mCmdParams = new CallSetupParams(cmdDet, confirmMsg, callMsg);

        if (confirmIconId != null || callIconId != null) {
            mIconLoadState = LOAD_MULTI_ICONS;
            int[] recordNumbers = new int[2];
            recordNumbers[0] = confirmIconId != null
                    ? confirmIconId.recordNumber : -1;
            recordNumbers[1] = callIconId != null ? callIconId.recordNumber
                    : -1;

            mIconLoader.loadIcons(recordNumbers, this
                    .obtainMessage(MSG_ID_LOAD_ICON_DONE));
            return true;
        }
        return false;
    }

    /**
     * Get the index of ADDRESS data object.
     *
     * @param list List of ComprehensionTlv
     * @return the index of ADDRESS data object.
     */
    private int getAddrIndex(final List<ComprehensionTlv> list) {
        int addrIndex = 0;

        ComprehensionTlv temp = null;
        Iterator<ComprehensionTlv> iter = list.iterator();
        while (iter.hasNext()) {
            temp = iter.next();
            if (temp.getTag() == ComprehensionTlvTag.ADDRESS.value()
                    || temp.getTag() == ComprehensionTlvTag.URL.value()) {
                return addrIndex;
            }
            ++addrIndex;
        } // end while

        return -1;
    }

    /**
     * Get the index of ALPHA_ID data object in confirmation phase data object.
     *
     * @param list List of ComprehensionTlv
     * @param addrIndex The index of ADDRESS data object
     * @return the index of ALPHA_ID data object.
     */
    private int getConfirmationAlphaIdIndex(final List<ComprehensionTlv> list,
            final int addrIndex) {
        int alphaIndex = 0;

        ComprehensionTlv temp = null;
        Iterator<ComprehensionTlv> iter = list.iterator();
        while (iter.hasNext()) {
            temp = iter.next();
            if (temp.getTag() == ComprehensionTlvTag.ALPHA_ID.value()
                    && alphaIndex < addrIndex) {
                return alphaIndex;
            }
            ++alphaIndex;
        } // end while

        return -1;
    }

    /**
     * Get the index of ALPHA_ID data object in call phase data object.
     *
     * @param list List of ComprehensionTlv
     * @param addrIndex The index of ADDRESS data object
     * @return the index of ALPHA_ID data object.
     */
    private int getCallingAlphaIdIndex(final List<ComprehensionTlv> list,
            final int addrIndex) {
        int alphaIndex = 0;

        ComprehensionTlv temp = null;
        Iterator<ComprehensionTlv> iter = list.iterator();
        while (iter.hasNext()) {
            temp = iter.next();
            if (temp.getTag() == ComprehensionTlvTag.ALPHA_ID.value()
                    && alphaIndex > addrIndex) {
                return alphaIndex;
            }
            ++alphaIndex;
        } // end while

        return -1;
    }

    /**
     * Get the ALPHA_ID data object in confirmation phase data object.
     *
     * @param list List of ComprehensionTlv
     * @param addrIndex The index of ADDRESS data object
     * @return ALPHA_ID data object.
     */
    private ComprehensionTlv getConfirmationAlphaId(final List<ComprehensionTlv> list,
            final int addrIndex) {
        int alphaIndex = 0;

        ComprehensionTlv temp = null;
        Iterator<ComprehensionTlv> iter = list.iterator();
        while (iter.hasNext()) {
            temp = iter.next();
            if (temp.getTag() == ComprehensionTlvTag.ALPHA_ID.value()
                    && alphaIndex < addrIndex) {
                return temp;
            }
            ++alphaIndex;
        } // end while

        return null;
    }

    /**
     * Get the ALPHA_ID data object in call phase data object.
     *
     * @param list List of ComprehensionTlv
     * @param addrIndex The index of ADDRESS data object
     * @return ALPHA_ID data object.
     */
    private ComprehensionTlv getCallingAlphaId(final List<ComprehensionTlv> list,
            final int addrIndex) {
        int alphaIndex = 0;

        ComprehensionTlv temp = null;
        Iterator<ComprehensionTlv> iter = list.iterator();
        while (iter.hasNext()) {
            temp = iter.next();
            if (temp.getTag() == ComprehensionTlvTag.ALPHA_ID.value()
                    && alphaIndex > addrIndex) {
                return temp;
            }
            ++alphaIndex;
        } // end while

        return null;
    }

    /**
     * Get the ICON_ID data object in confirmation phase data object.
     *
     * @param list List of ComprehensionTlv
     * @param alpha1Index The index of ALPHA_ID data object of confirmation
     *            phase
     * @param alpha2Index The index of ALPHA_ID data object of call phase
     * @return ICON_ID data object.
     */
    private ComprehensionTlv getConfirmationIconId(final List<ComprehensionTlv> list,
            final int alpha1Index,
            final int alpha2Index) {
        if (-1 == alpha1Index) {
            return null;
        }

        int iconIndex = 0;

        ComprehensionTlv temp = null;
        Iterator<ComprehensionTlv> iter = list.iterator();
        while (iter.hasNext()) {
            temp = iter.next();
            if (temp.getTag() == ComprehensionTlvTag.ICON_ID.value()
                    && (-1 == alpha2Index || iconIndex < alpha2Index)) {
                return temp;
            }
            ++iconIndex;
        } // end while

        return null;
    }

    /**
     * Get the ICON_ID data object in call phase data object.
     *
     * @param list List of ComprehensionTlv
     * @param alpha2Index The index of ALPHA_ID data object of call phase
     * @return ICON_ID data object.
     */
    private ComprehensionTlv getCallingIconId(final List<ComprehensionTlv> list,
            final int alpha2Index) {
        if (-1 == alpha2Index) {
            return null;
        }

        int iconIndex = 0;

        ComprehensionTlv temp = null;
        Iterator<ComprehensionTlv> iter = list.iterator();
        while (iter.hasNext()) {
            temp = iter.next();
            if (temp.getTag() == ComprehensionTlvTag.ICON_ID.value()
                    && iconIndex > alpha2Index) {
                return temp;
            }
            ++iconIndex;
        } // end while

        return null;
    }
}
