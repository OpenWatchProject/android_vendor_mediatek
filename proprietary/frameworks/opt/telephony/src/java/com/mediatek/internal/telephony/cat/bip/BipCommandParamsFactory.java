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

import android.graphics.Bitmap;
import android.os.Handler;
import android.os.Message;

import com.android.internal.telephony.cat.AppInterface;
import com.android.internal.telephony.cat.BerTlv;
import com.android.internal.telephony.cat.ComprehensionTlv;
import com.android.internal.telephony.cat.ComprehensionTlvTag;
import com.android.internal.telephony.cat.CommandParams;
import com.android.internal.telephony.cat.CommandDetails;
import com.android.internal.telephony.cat.IconId;
import com.android.internal.telephony.cat.ResultException;
import com.android.internal.telephony.cat.ResultCode;
import com.android.internal.telephony.cat.TextMessage;
import com.android.internal.telephony.cat.ValueParser;
import com.android.internal.telephony.GsmAlphabet;
import com.android.internal.telephony.uicc.IccFileHandler;
import com.android.internal.telephony.cat.SetEventListParams;

import java.util.Iterator;
import java.util.List;
import static com.android.internal.telephony.cat.CatCmdMessage.
                   SetupEventListConstants.USER_ACTIVITY_EVENT;
import static com.android.internal.telephony.cat.CatCmdMessage.
                   SetupEventListConstants.IDLE_SCREEN_AVAILABLE_EVENT;
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
class BipCommandParamsFactory extends Handler {
    private static BipCommandParamsFactory sInstance = null;
    private BipIconLoader mIconLoader;
    private CommandParams mCmdParams = null;
    private int mIconLoadState = LOAD_NO_ICON;
    private BipRilMessageDecoder mCaller = null;
    // used to mark the index of tlv object in a tlv list
    int tlvIndex = -1;

    // constants
    static final int MSG_ID_LOAD_ICON_DONE = 1;

    // loading icons state parameters.
    static final int LOAD_NO_ICON           = 0;
    static final int LOAD_SINGLE_ICON       = 1;
    static final int LOAD_MULTI_ICONS       = 2;

    static synchronized BipCommandParamsFactory getInstance(BipRilMessageDecoder caller,
            IccFileHandler fh) {
        if (sInstance != null) {
            return sInstance;
        }
        if (fh != null) {
            return new BipCommandParamsFactory(caller, fh);
        }
        return null;
    }

    private BipCommandParamsFactory(BipRilMessageDecoder caller, IccFileHandler fh) {
        mCaller = caller;
        mIconLoader = BipIconLoader.getInstance(this, fh, mCaller.getSlotId());
    }

    private CommandDetails processCommandDetails(List<ComprehensionTlv> ctlvs)
        throws ResultException {

        CommandDetails cmdDet = null;

        if (ctlvs != null) {
            // Search for the Command Details object.
            ComprehensionTlv ctlvCmdDet = searchForTag(
                    ComprehensionTlvTag.COMMAND_DETAILS, ctlvs);
            if (ctlvCmdDet != null) {
                try {
                    cmdDet = ValueParser.retrieveCommandDetails(ctlvCmdDet);
                } catch (ResultException e) {
                    MtkCatLog.d(this, "Failed to procees command details");
                    throw new ResultException(ResultCode.CMD_DATA_NOT_UNDERSTOOD);
                }
            }
        }
        return cmdDet;
    }

    void make(BerTlv berTlv) {
        if (berTlv == null) {
            return;
        }
        // reset global state parameters.
        mCmdParams = null;
        mIconLoadState = LOAD_NO_ICON;
        // only proactive command messages are processed.
        if (berTlv.getTag() != BerTlv.BER_PROACTIVE_COMMAND_TAG) {
            MtkCatLog.e(this, "CPF-make: Ununderstood proactive command tag");
            sendCmdParams(ResultCode.CMD_TYPE_NOT_UNDERSTOOD);
            return;
        }
        boolean cmdPending = false;
        List<ComprehensionTlv> ctlvs = berTlv.getComprehensionTlvs();
        // process command dtails from the tlv list.
        CommandDetails cmdDet = null;
        try {
            cmdDet = processCommandDetails(ctlvs);
        } catch (ResultException e) {
            MtkCatLog.e(this, "CPF-make: Except to procees command details : " + e.result());
            sendCmdParams(e.result());
            return;
        }
        if (cmdDet == null) {
            MtkCatLog.e(this, "CPF-make: No CommandDetails object");
            sendCmdParams(ResultCode.CMD_TYPE_NOT_UNDERSTOOD);
            return;
        }

        // extract command type enumeration from the raw value stored inside
        // the Command Details object.
        AppInterface.CommandType cmdType = AppInterface.CommandType
                .fromInt(cmdDet.typeOfCommand);
        if (cmdType == null) {
            MtkCatLog.d(this, "CPF-make: Command type can't be found");
            // Different from 2.3.5
            // This PROACTIVE COMMAND is presently not handled. Hence set
            // result code as BEYOND_TERMINAL_CAPABILITY in TR.
            mCmdParams = new CommandParams(cmdDet);
            // sendCmdParams(ResultCode.CMD_TYPE_NOT_UNDERSTOOD);
            sendCmdParams(ResultCode.BEYOND_TERMINAL_CAPABILITY);
            return;
        }

        // proactive command length is incorrect.
        if (!berTlv.isLengthValid()) {
            mCmdParams = new CommandParams(cmdDet);
            sendCmdParams(ResultCode.CMD_DATA_NOT_UNDERSTOOD);
            return;
        }

        try {
            switch (cmdType) {
            case SET_UP_EVENT_LIST:
                cmdPending = processSetUpEventList(cmdDet, ctlvs);
                break;
            case OPEN_CHANNEL:
                cmdPending = processOpenChannel(cmdDet, ctlvs);
                MtkCatLog.d(this, "process OpenChannel");
                break;
            case CLOSE_CHANNEL:
                cmdPending = processCloseChannel(cmdDet, ctlvs);
                MtkCatLog.d(this, "process CloseChannel");
                break;
            case SEND_DATA:
                cmdPending = processSendData(cmdDet, ctlvs);
                MtkCatLog.d(this, "process SendData");
                break;
            case RECEIVE_DATA:
                cmdPending = processReceiveData(cmdDet, ctlvs);
                MtkCatLog.d(this, "process ReceiveData");
                break;
            case GET_CHANNEL_STATUS:
                cmdPending = processGetChannelStatus(cmdDet, ctlvs);
                MtkCatLog.d(this, "process GetChannelStatus");
                break;
            default:
                // unsupported proactive commands
                mCmdParams = new CommandParams(cmdDet);
                MtkCatLog.d(this, "CPF-make: default case");
                // sendCmdParams(ResultCode.CMD_TYPE_NOT_UNDERSTOOD);
                sendCmdParams(ResultCode.BEYOND_TERMINAL_CAPABILITY);
                return;
            }
        } catch (ResultException e) {
            MtkCatLog.d(this, "make: caught ResultException e=" + e);
            mCmdParams = new CommandParams(cmdDet);
            sendCmdParams(e.result());
            return;
        }
        if (!cmdPending) {
            sendCmdParams(ResultCode.OK);
        }
    }

    @Override
    public void handleMessage(Message msg) {
        switch (msg.what) {
        case MSG_ID_LOAD_ICON_DONE:
            sendCmdParams(setIcons(msg.obj));
            break;
        }
    }

    private ResultCode setIcons(Object data) {
        Bitmap[] icons = null;
        int iconIndex = 0;

        if (data == null) {
            return ResultCode.PRFRMD_ICON_NOT_DISPLAYED;
        }
        switch(mIconLoadState) {
        case LOAD_SINGLE_ICON:
           //  mCmdParams.setIcon((Bitmap) data);
            break;
        case LOAD_MULTI_ICONS:
            icons = (Bitmap[]) data;
            // set each item icon.
            for (Bitmap icon : icons) {
               //  mCmdParams.setIcon(icon);
            }
            break;
        }
        return ResultCode.OK;
    }

    private void sendCmdParams(ResultCode resCode) {
        mCaller.sendMsgParamsDecoded(resCode, mCmdParams);
    }

    /**
     * Search for a COMPREHENSION-TLV object with the given tag from a list
     *
     * @param tag A tag to search for
     * @param ctlvs List of ComprehensionTlv objects used to search in
     *
     * @return A ComprehensionTlv object that has the tag value of {@code tag}.
     *         If no object is found with the tag, null is returned.
     */
    private ComprehensionTlv searchForTag(ComprehensionTlvTag tag,
            List<ComprehensionTlv> ctlvs) {
        Iterator<ComprehensionTlv> iter = ctlvs.iterator();
        return searchForNextTag(tag, iter);
    }

    /**
     * Search for the next COMPREHENSION-TLV object with the given tag from a
     * list iterated by {@code iter}. {@code iter} points to the object next to
     * the found object when this method returns. Used for searching the same
     * list for similar tags, usually item id.
     *
     * @param tag A tag to search for
     * @param iter Iterator for ComprehensionTlv objects used for search
     *
     * @return A ComprehensionTlv object that has the tag value of {@code tag}.
     *         If no object is found with the tag, null is returned.
     */
    private ComprehensionTlv searchForNextTag(ComprehensionTlvTag tag,
            Iterator<ComprehensionTlv> iter) {
        int tagValue = tag.value();
        while (iter.hasNext()) {
            ComprehensionTlv ctlv = iter.next();
            if (ctlv.getTag() == tagValue) {
                return ctlv;
            }
        }
        return null;
    }

    // Add by Huibin Mao Mtk80229
    // ICS Migration start
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
     * provide the index of searched tlv object
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

    // ICS Migration end

    /**
     * Processes SET_UP_EVENT_LIST proactive command from the SIM card.
     *
     * @param cmdDet Command Details object retrieved.
     * @param ctlvs List of ComprehensionTlv objects following Command Details
     *        object and Device Identities object within the proactive command
     * @return false. This function always returns false meaning that the command
     *         processing is  not pending and additional asynchronous processing
     *         is not required.
     */
    private boolean processSetUpEventList(CommandDetails cmdDet,
            List<ComprehensionTlv> ctlvs) {

        MtkCatLog.d(this, "process SetUpEventList");
        ComprehensionTlv ctlv = searchForTag(ComprehensionTlvTag.EVENT_LIST, ctlvs);
        if (ctlv != null) {
            try {
                byte[] rawValue = ctlv.getRawValue();
                int valueIndex = ctlv.getValueIndex();
                int valueLen = ctlv.getLength();
                int[] eventList = new int[valueLen];
                int eventValue = -1;
                int i = 0;
                while (valueLen > 0) {
                    eventValue = rawValue[valueIndex] & 0xff;
                    valueIndex++;
                    valueLen--;

                    switch (eventValue) {
                        case USER_ACTIVITY_EVENT:
                        case IDLE_SCREEN_AVAILABLE_EVENT:
                        case LANGUAGE_SELECTION_EVENT:
                        case BROWSER_TERMINATION_EVENT:
                        case BROWSING_STATUS_EVENT:
                            eventList[i] = eventValue;
                            i++;
                            break;
                        default:
                            break;
                    }

                }
                mCmdParams = new SetEventListParams(cmdDet, eventList);
            } catch (IndexOutOfBoundsException e) {
                MtkCatLog.e(this, " IndexOutofBoundException in processSetUpEventList");
            }
        }
        return false;
    }

    /**
     * Processes OPEN_CHANNEL proactive command from the SIM card.
     *
     * @param cmdDet Command Details container object.
     * @param ctlvs List of ComprehensionTlv objects following Command Details
     *            object and Device Identities object within the proactive
     *            command
     * @return true if the command is processing is pending and additional
     *         asynchronous processing is required.
     * @throws ResultException
     */
    private boolean processOpenChannel(CommandDetails cmdDet,
            List<ComprehensionTlv> ctlvs)
            throws ResultException {

        MtkCatLog.d(this, "enter: process OpenChannel");

        // Iterator for searching tlv
        Iterator<ComprehensionTlv> iter = null;
        ComprehensionTlv ctlv = null;
        // int tlvIndex = -1;

        BearerDesc bearerDesc = null;
        int bufferSize = 0;
        int linkMode = ((cmdDet.commandQualifier & 0x01) == 1)
                ? BipUtils.LINK_ESTABLISHMENT_MODE_IMMEDIATE
                : BipUtils.LINK_ESTABLISHMENT_MODE_ONDEMMAND;
        boolean isAutoReconnect = ((cmdDet.commandQualifier & 0x02) == 0) ? false : true;

        String accessPointName = null;
        OtherAddress localAddress = null;
        String userLogin = null;
        String userPwd = null;

        TransportProtocol transportProtocol = null;
        OtherAddress dataDestinationAddress = null;

        TextMessage confirmText = new TextMessage();
        IconId confirmIcon = null;

        // Two other address data objects may contain in one
        // OpenChannel data object. We can distinguish them
        // by their indices. The index of LocalAddress data
        // object should be less than the index of Transport-
        // Protocol data object and the index of DataDestination-
        // Address should be greater than the index of Trans-
        // port Protocol.
        int indexTransportProtocol = -1;

        ctlv = searchForTag(ComprehensionTlvTag.ALPHA_ID,
                ctlvs);
        if (ctlv != null) {
            confirmText.text = MtkValueParser.retrieveAlphaId(ctlv);
        }

        ctlv = searchForTag(ComprehensionTlvTag.ICON_ID, ctlvs);
        if (ctlv != null) {
            mIconLoadState = LOAD_SINGLE_ICON;
            confirmIcon = ValueParser.retrieveIconId(ctlv);
            confirmText.iconSelfExplanatory = confirmIcon.selfExplanatory;
        }

        // parse bearer description data object
        ctlv = searchForTag(ComprehensionTlvTag.BEARER_DESCRIPTION, ctlvs);
        if (ctlv != null) {
            bearerDesc = BipValueParser.retrieveBearerDesc(ctlv);
            MtkCatLog.d("[BIP]", "bearerDesc bearer type: " + bearerDesc.bearerType);
            if (bearerDesc instanceof GPRSBearerDesc) {
                MtkCatLog.d("[BIP]", "\nprecedence: " + ((GPRSBearerDesc) bearerDesc).precedence
                    + "\ndelay: " + ((GPRSBearerDesc) bearerDesc).delay
                    + "\nreliability: " + ((GPRSBearerDesc) bearerDesc).reliability
                    + "\npeak: " + ((GPRSBearerDesc) bearerDesc).peak
                    + "\nmean: " + ((GPRSBearerDesc) bearerDesc).mean
                    + "\npdp type: " + ((GPRSBearerDesc) bearerDesc).pdpType);
            } else if (bearerDesc instanceof UTranBearerDesc) {
                MtkCatLog.d("[BIP]",
                    "\ntrafficClass: " + ((UTranBearerDesc) bearerDesc).trafficClass
                    + "\nmaxBitRateUL_High: " + ((UTranBearerDesc) bearerDesc).maxBitRateUL_High
                    + "\nmaxBitRateUL_Low: " + ((UTranBearerDesc) bearerDesc).maxBitRateUL_Low
                    + "\nmaxBitRateDL_High: " + ((UTranBearerDesc) bearerDesc).maxBitRateDL_High
                    + "\nmaxBitRateUL_Low: " + ((UTranBearerDesc) bearerDesc).maxBitRateDL_Low
                    + "\nguarBitRateUL_High: " + ((UTranBearerDesc) bearerDesc).guarBitRateUL_High
                    + "\nguarBitRateUL_Low: " + ((UTranBearerDesc) bearerDesc).guarBitRateUL_Low
                    + "\nguarBitRateDL_High: " + ((UTranBearerDesc) bearerDesc).guarBitRateDL_High
                    + "\nguarBitRateDL_Low: " + ((UTranBearerDesc) bearerDesc).guarBitRateDL_Low
                    + "\ndeliveryOrder: " + ((UTranBearerDesc) bearerDesc).deliveryOrder
                    + "\nmaxSduSize: " + ((UTranBearerDesc) bearerDesc).maxSduSize
                    + "\nsduErrorRatio: " + ((UTranBearerDesc) bearerDesc).sduErrorRatio
                    + "\nresidualBitErrorRadio: "
                    + ((UTranBearerDesc) bearerDesc).residualBitErrorRadio
                    + "\ndeliveryOfErroneousSdus: "
                    + ((UTranBearerDesc) bearerDesc).deliveryOfErroneousSdus
                    + "\ntransferDelay: " + ((UTranBearerDesc) bearerDesc).transferDelay
                    + "\ntrafficHandlingPriority: "
                    + ((UTranBearerDesc) bearerDesc).trafficHandlingPriority
                    + "\npdp type: " + ((UTranBearerDesc) bearerDesc).pdpType);
            } else if (bearerDesc instanceof EUTranBearerDesc) {
                MtkCatLog.d("[BIP]", "\nQCI: " + ((EUTranBearerDesc) bearerDesc).QCI
                        + "\nmaxBitRateU: " + ((EUTranBearerDesc) bearerDesc).maxBitRateU
                        + "\nmaxBitRateD: " + ((EUTranBearerDesc) bearerDesc).maxBitRateD
                        + "\nguarBitRateU: " + ((EUTranBearerDesc) bearerDesc).guarBitRateU
                        + "\nguarBitRateD: " + ((EUTranBearerDesc) bearerDesc).guarBitRateD
                        + "\nmaxBitRateUEx: " + ((EUTranBearerDesc) bearerDesc).maxBitRateUEx
                        + "\nmaxBitRateDEx: " + ((EUTranBearerDesc) bearerDesc).maxBitRateDEx
                        + "\nguarBitRateUEx: " + ((EUTranBearerDesc) bearerDesc).guarBitRateUEx
                        + "\nguarBitRateDEx: " + ((EUTranBearerDesc) bearerDesc).guarBitRateDEx
                        + "\npdn Type: " + ((EUTranBearerDesc) bearerDesc).pdnType);
            } else if (bearerDesc instanceof DefaultBearerDesc) {
            } else {
                MtkCatLog.d("[BIP]", "Not support bearerDesc");
            }
        } else {
            MtkCatLog.d("[BIP]", "May Need BearerDescription object");
            //throw new ResultException(ResultCode.REQUIRED_VALUES_MISSING);
        }

        // parse buffer size data object
        ctlv = searchForTag(ComprehensionTlvTag.BUFFER_SIZE, ctlvs);
        if (ctlv != null) {
            bufferSize = BipValueParser.retrieveBufferSize(ctlv);
            MtkCatLog.d("[BIP]", "buffer size: " + bufferSize);
        } else {
            MtkCatLog.d("[BIP]", "Need BufferSize object");
            throw new ResultException(ResultCode.REQUIRED_VALUES_MISSING);
        }

        // parse network access name data object
        ctlv = searchForTag(ComprehensionTlvTag.NETWORK_ACCESS_NAME, ctlvs);
        if (ctlv != null) {
            accessPointName = BipValueParser.retrieveNetworkAccessName(ctlv);
            MtkCatLog.d("[BIP]", "access point name: " + accessPointName);
        }

        // parse user login & password
        iter = ctlvs.iterator();
        ctlv = searchForNextTag(ComprehensionTlvTag.TEXT_STRING, iter);
        if (ctlv != null) {
            userLogin = ValueParser.retrieveTextString(ctlv);
            MtkCatLog.d("[BIP]", "user login: " + userLogin);
        }
        ctlv = searchForNextTag(ComprehensionTlvTag.TEXT_STRING, iter);
        if (ctlv != null) {
            userPwd = ValueParser.retrieveTextString(ctlv);
            MtkCatLog.d("[BIP]", "user password: " + userPwd);
        }

        // parse SIM/ME interface transport level & data destination address
        ctlv = searchForTagAndIndex(ComprehensionTlvTag.SIM_ME_INTERFACE_TRANSPORT_LEVEL, ctlvs);
        if (ctlv != null) {
            indexTransportProtocol = tlvIndex;
            MtkCatLog.d("[BIP]", "CPF-processOpenChannel: indexTransportProtocol = "
                    + indexTransportProtocol);
            transportProtocol = BipValueParser.retrieveTransportProtocol(ctlv);
            MtkCatLog.d("[BIP]", "CPF-processOpenChannel: transport protocol(type/port): "
                    + transportProtocol.protocolType + "/" + transportProtocol.portNumber);
            if ((BipUtils.TRANSPORT_PROTOCOL_UDP_REMOTE == transportProtocol.protocolType) ||
                (BipUtils.TRANSPORT_PROTOCOL_TCP_REMOTE == transportProtocol.protocolType)) {
                if (null == bearerDesc) {
                    MtkCatLog.d("[BIP]", "Need BearerDescription object");
                    throw new ResultException(ResultCode.REQUIRED_VALUES_MISSING);
                }
            }
        } else {
            if (null == bearerDesc) {
                MtkCatLog.d("[BIP]", "BearerDescription & transportProtocol object are null");
                throw new ResultException(ResultCode.REQUIRED_VALUES_MISSING);
            }
        }

        if (transportProtocol != null) {
            MtkCatLog.d("[BIP]", "CPF-processOpenChannel: transport protocol is existed");
            iter = ctlvs.iterator();
            resetTlvIndex();
            ctlv = searchForNextTagAndIndex(ComprehensionTlvTag.OTHER_ADDRESS, iter);
            if (ctlv != null) {
                if (tlvIndex < indexTransportProtocol) {
                    // this tlv is local address
                    MtkCatLog.d("[BIP]", "CPF-processOpenChannel: get local address, index is "
                            + tlvIndex);
                    localAddress = BipValueParser.retrieveOtherAddress(ctlv);

                    // we should also get destination address, because transport
                    // protocol object is existed
                    ctlv = searchForNextTagAndIndex(ComprehensionTlvTag.OTHER_ADDRESS, iter);
                    if (ctlv != null && tlvIndex > indexTransportProtocol) {
                        MtkCatLog.d("[BIP]", "CPF-processOpenChannel: get dest address, index is "
                                + tlvIndex);
                        dataDestinationAddress = BipValueParser.retrieveOtherAddress(ctlv);
                    } else {
                        MtkCatLog.d("[BIP]", "CPF-processOpenChannel: missing dest address "
                                + tlvIndex
                                + "/" + indexTransportProtocol);
                        throw new ResultException(ResultCode.REQUIRED_VALUES_MISSING);
                    }
                } else if (tlvIndex > indexTransportProtocol) {
                    MtkCatLog.d("[BIP]",
                            "CPF-processOpenChannel: get dest address, but no local address");
                    dataDestinationAddress = BipValueParser.retrieveOtherAddress(ctlv);
                } else {
                    MtkCatLog.d("[BIP]", "CPF-processOpenChannel: Incorrect index");
                }
            } else {
                MtkCatLog.d("[BIP]", "CPF-processOpenChannel: No other address object");
            }
            if (null == dataDestinationAddress) {
                if (BipUtils.TRANSPORT_PROTOCOL_TCP_REMOTE == transportProtocol.protocolType ||
                    BipUtils.TRANSPORT_PROTOCOL_UDP_REMOTE == transportProtocol.protocolType) {
                    MtkCatLog.d("[BIP]", "BM-openChannel: dataDestinationAddress is null.");
                    throw new ResultException(ResultCode.REQUIRED_VALUES_MISSING);
                }
            } //But if null == transportProtocol ??
        }

        // Undo: construct OpenChannelParams here
        if (bearerDesc != null) {
            if (bearerDesc.bearerType == BipUtils.BEARER_TYPE_GPRS ||
                bearerDesc.bearerType == BipUtils.BEARER_TYPE_DEFAULT ||
                bearerDesc.bearerType == BipUtils.BEARER_TYPE_UTRAN ||
                bearerDesc.bearerType == BipUtils.BEARER_TYPE_EUTRAN) {
                mCmdParams = new OpenChannelParams(cmdDet, bearerDesc, bufferSize, localAddress,
                        transportProtocol, dataDestinationAddress,
                        accessPointName, userLogin, userPwd, confirmText);
            } else {
                MtkCatLog.d("[BIP]", "Unsupport bearerType: " + bearerDesc.bearerType);
            }
        }

        mCmdParams = new OpenChannelParams(cmdDet, bearerDesc, bufferSize, localAddress,
                transportProtocol, dataDestinationAddress, accessPointName, userLogin, userPwd,
                confirmText);

        if (confirmIcon != null) {
            mIconLoadState = LOAD_SINGLE_ICON;
            mIconLoader.loadIcon(confirmIcon.recordNumber, this
                    .obtainMessage(MSG_ID_LOAD_ICON_DONE));
            return true;
        }

        return false;
    }

    /**
     * Processes CLOSE_CHANNEL proactive command from the SIM card.
     *
     * @param cmdDet Command Details container object.
     * @param ctlvs List of ComprehensionTlv objects following Command Details
     *            object and Device Identities object within the proactive
     *            command
     * @return true if the command is processing is pending and additional
     *         asynchronous processing is required.
     * @throws ResultException
     */
    private boolean processCloseChannel(CommandDetails cmdDet,
            List<ComprehensionTlv> ctlvs)
            throws ResultException {

        MtkCatLog.d(this, "enter: process CloseChannel");

        ComprehensionTlv ctlv = null;

        TextMessage textMsg = new TextMessage();
        IconId iconId = null;

        int channelId = 0;

        ctlv = searchForTag(ComprehensionTlvTag.ALPHA_ID, ctlvs);
        if (ctlv != null) {
            textMsg.text = MtkValueParser.retrieveAlphaId(ctlv);
        }

        ctlv = searchForTag(ComprehensionTlvTag.ICON_ID, ctlvs);
        if (ctlv != null) {
            iconId = ValueParser.retrieveIconId(ctlv);
            textMsg.iconSelfExplanatory = iconId.selfExplanatory;
        }

        ctlv = searchForTag(ComprehensionTlvTag.DEVICE_IDENTITIES, ctlvs);
        if (ctlv != null) {
            byte cidByte = ctlv.getRawValue()[ctlv.getValueIndex() + 1];
            channelId = cidByte & 0x0f;
            MtkCatLog.d("[BIP]", "To close channel " + channelId);
        }
        boolean backToTcpListen = (1 == (cmdDet.commandQualifier & 0x01)) ? true : false;

        mCmdParams = new CloseChannelParams(cmdDet, channelId, textMsg, backToTcpListen);

        if (iconId != null) {
            mIconLoadState = LOAD_SINGLE_ICON;
            mIconLoader.loadIcon(iconId.recordNumber, this
                    .obtainMessage(MSG_ID_LOAD_ICON_DONE));
            return true;
        }

        return false;
    }

    /**
     * Processes RECEIVE_DATA proactive command from the SIM card.
     *
     * @param cmdDet Command Details container object.
     * @param ctlvs List of ComprehensionTlv objects following Command Details
     *            object and Device Identities object within the proactive
     *            command
     * @return true if the command is processing is pending and additional
     *         asynchronous processing is required.
     * @throws ResultException
     */
    private boolean processReceiveData(CommandDetails cmdDet,
            List<ComprehensionTlv> ctlvs)
            throws ResultException {

        MtkCatLog.d(this, "enter: process ReceiveData");

        ComprehensionTlv ctlv = null;

        int channelDataLength = 0;

        TextMessage textMsg = new TextMessage();
        IconId iconId = null;
        int channelId = 0;

        ctlv = searchForTag(ComprehensionTlvTag.CHANNEL_DATA_LENGTH, ctlvs);
        if (ctlv != null) {
            channelDataLength = BipValueParser.retrieveChannelDataLength(ctlv);
            MtkCatLog.d("[BIP]", "Channel data length: " + channelDataLength);
        }

        // mCmdParams = new ReceiveDataParams(cmdDet, channelDataLength,
        // textMsg);

        ctlv = searchForTag(ComprehensionTlvTag.ALPHA_ID, ctlvs);
        if (ctlv != null) {
            textMsg.text = MtkValueParser.retrieveAlphaId(ctlv);
        }

        ctlv = searchForTag(ComprehensionTlvTag.ICON_ID, ctlvs);
        if (ctlv != null) {
            iconId = ValueParser.retrieveIconId(ctlv);
            textMsg.iconSelfExplanatory = iconId.selfExplanatory;
        }

        ctlv = searchForTag(ComprehensionTlvTag.DEVICE_IDENTITIES, ctlvs);
        if (ctlv != null) {
            byte cidByte = ctlv.getRawValue()[ctlv.getValueIndex() + 1];
            channelId = cidByte & 0x0f;
            MtkCatLog.d("[BIP]", "To Receive data: " + channelId);
        }

        mCmdParams = new ReceiveDataParams(cmdDet, channelDataLength, channelId, textMsg);

        if (iconId != null) {
            mIconLoadState = LOAD_SINGLE_ICON;
            mIconLoader.loadIcon(iconId.recordNumber, this
                    .obtainMessage(MSG_ID_LOAD_ICON_DONE));
            return true;
        }

        return false;
    }

    /**
     * Processes SEND_DATA proactive command from the SIM card.
     *
     * @param cmdDet Command Details container object.
     * @param ctlvs List of ComprehensionTlv objects following Command Details
     *            object and Device Identities object within the proactive
     *            command
     * @return true if the command is processing is pending and additional
     *         asynchronous processing is required.
     * @throws ResultException
     */
    private boolean processSendData(CommandDetails cmdDet,
            List<ComprehensionTlv> ctlvs)
            throws ResultException {

        MtkCatLog.d(this, "enter: process SendData");

        ComprehensionTlv ctlv = null;

        byte[] channelData = null;

        TextMessage textMsg = new TextMessage();
        IconId iconId = null;
        int channelId = 0;

        int sendMode = ((cmdDet.commandQualifier & 0x01) == 1)
                ? BipUtils.SEND_DATA_MODE_IMMEDIATE : BipUtils.SEND_DATA_MODE_STORED;

        ctlv = searchForTag(ComprehensionTlvTag.CHANNEL_DATA, ctlvs);
        if (ctlv != null) {
            channelData = BipValueParser.retrieveChannelData(ctlv);
        }

        // mCmdParams = new SendDataParams(cmdDet, channelData, textMsg);

        ctlv = searchForTag(ComprehensionTlvTag.ALPHA_ID, ctlvs);
        if (ctlv != null) {
            textMsg.text = MtkValueParser.retrieveAlphaId(ctlv);
        }

        ctlv = searchForTag(ComprehensionTlvTag.ICON_ID, ctlvs);
        if (ctlv != null) {
            iconId = ValueParser.retrieveIconId(ctlv);
            textMsg.iconSelfExplanatory = iconId.selfExplanatory;
        }

        ctlv = searchForTag(ComprehensionTlvTag.DEVICE_IDENTITIES, ctlvs);
        if (ctlv != null) {
            byte cidByte = ctlv.getRawValue()[ctlv.getValueIndex() + 1];
            channelId = cidByte & 0x0f;
            MtkCatLog.d("[BIP]", "To send data: " + channelId);
        }

        mCmdParams = new SendDataParams(cmdDet, channelData, channelId, textMsg, sendMode);

        if (iconId != null) {
            mIconLoadState = LOAD_SINGLE_ICON;
            mIconLoader.loadIcon(iconId.recordNumber, this
                    .obtainMessage(MSG_ID_LOAD_ICON_DONE));
            return true;
        }

        return false;
    }

    /**
     * Processes GET_CHANNEL STATUS proactive command from the SIM card.
     *
     * @param cmdDet Command Details container object.
     * @param ctlvs List of ComprehensionTlv objects following Command Details
     *            object and Device Identities object within the proactive
     *            command
     * @return true if the command is processing is pending and additional
     *         asynchronous processing is required.
     * @throws ResultException
     */
    private boolean processGetChannelStatus(CommandDetails cmdDet,
            List<ComprehensionTlv> ctlvs)
            throws ResultException {

        MtkCatLog.d(this, "enter: process GetChannelStatus");

        ComprehensionTlv ctlv = null;

        TextMessage textMsg = new TextMessage();
        IconId iconId = null;

        ctlv = searchForTag(ComprehensionTlvTag.ALPHA_ID, ctlvs);
        if (ctlv != null) {
            textMsg.text = MtkValueParser.retrieveAlphaId(ctlv);
        }

        ctlv = searchForTag(ComprehensionTlvTag.ICON_ID, ctlvs);
        if (ctlv != null) {
            iconId = ValueParser.retrieveIconId(ctlv);
            textMsg.iconSelfExplanatory = iconId.selfExplanatory;
        }

        mCmdParams = new GetChannelStatusParams(cmdDet, textMsg);

        if (iconId != null) {
            mIconLoadState = LOAD_SINGLE_ICON;
            mIconLoader.loadIcon(iconId.recordNumber, this
                    .obtainMessage(MSG_ID_LOAD_ICON_DONE));
            return true;
        }

        return false;
    }

    public void dispose() {
        mIconLoader.dispose();
        mIconLoader = null;
        mCmdParams = null;
        mCaller = null;
        synchronized (BipCommandParamsFactory.class) {
            sInstance = null;
        }
    }
}
