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
 * have been modified by MediaTek Inc. All revisions are subject to any receiver's
 * applicable license agreements with MediaTek Inc.
 */

package com.mediatek.internal.telephony.cat;

import android.content.BroadcastReceiver;
import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.pm.PackageManager;
import android.content.pm.ResolveInfo;
import android.content.SharedPreferences;
import android.content.SharedPreferences.Editor;
import android.content.res.Resources.NotFoundException;
import android.os.AsyncResult;
import android.os.BatteryManager;
import android.os.Handler;
import android.os.Message;
import android.os.SystemProperties;
import android.provider.Settings;
import android.provider.Settings.SettingNotFoundException;
import android.provider.Settings.System;
import android.telephony.SubscriptionManager;
import android.telephony.TelephonyManager;
import android.net.Uri;

import com.android.internal.telephony.cat.AppInterface;
import com.android.internal.telephony.cat.CatCmdMessage;
import com.android.internal.telephony.CommandsInterface;
import com.android.internal.telephony.PhoneConstants;
import com.android.internal.telephony.SubscriptionController;
import com.android.internal.telephony.uicc.IccFileHandler;
import com.android.internal.telephony.uicc.IccRecords;
import com.android.internal.telephony.uicc.IccUtils;
import com.android.internal.telephony.TelephonyIntents;
import com.android.internal.telephony.uicc.UiccCard;
import com.android.internal.telephony.uicc.UiccCardApplication;
import com.android.internal.telephony.uicc.IccCardStatus.CardState;
import com.android.internal.telephony.uicc.IccRefreshResponse;
import com.android.internal.telephony.uicc.UiccController;
import com.android.internal.telephony.uicc.UiccProfile;
import com.android.internal.telephony.IccCardConstants;

import java.util.Calendar;
import java.util.List;
import java.util.Locale;
import java.util.LinkedList;

import com.android.internal.telephony.cat.CatService;
import com.android.internal.telephony.cat.CommandParams;
import com.android.internal.telephony.cat.CommandParamsFactory;
import com.android.internal.telephony.cat.AppInterface;
import com.android.internal.telephony.cat.ResponseData;
import com.android.internal.telephony.cat.ResultCode;
import com.android.internal.telephony.cat.RilMessage;
import com.android.internal.telephony.cat.ComprehensionTlvTag;
import com.android.internal.telephony.cat.BerTlv;
import com.android.internal.telephony.cat.CommandDetails;
import com.android.internal.telephony.cat.Input;
import com.android.internal.telephony.cat.TextMessage;
import com.android.internal.telephony.cat.CatResponseMessage;
import com.android.internal.telephony.cat.BIPClientParams;
import com.android.internal.telephony.cat.DisplayTextParams;
import com.android.internal.telephony.cat.LaunchBrowserParams;
import com.android.internal.telephony.cat.CallSetupParams;

import com.mediatek.internal.telephony.ModemSwitchHandler;
import com.mediatek.internal.telephony.MtkRIL;
import com.mediatek.telephony.MtkTelephonyManagerEx;

/**
 * Class that implements SIM Toolkit Telephony Service. Interacts with the RIL
 * and application. {@hide}
 */
public class MtkCatService extends CatService implements MtkAppInterface {
    private static final boolean DBG = true;

    // Service constants.
    static final int MSG_ID_EVENT_DOWNLOAD = 11;
    static final int MSG_ID_DB_HANDLER = 12;
    static final int MSG_ID_LAUNCH_DB_SETUP_MENU = 13;

    // MTK-START [mtk80589][121026][ALPS00376525] STK dialog pop up caused ISVR
    private static final int MSG_ID_IVSR_DELAYED = 14;
    // MTK-END [mtk80589][121026][ALPS00376525] STK dialog pop up caused ISVR
    private static final int MSG_ID_DISABLE_DISPLAY_TEXT_DELAYED = 15;
    // Events to signal SIM presence or absent in the device.
    private static final int MSG_ID_SETUP_MENU_RESET = 24;
    private static final int MSG_ID_CALL_CTRL = 25;

    public static final int MSG_ID_CACHED_DISPLAY_TEXT_TIMEOUT = 46;
    public static final int MSG_ID_CONN_RETRY_TIMEOUT = 47;

    private static String[] sInstKey = {
            "sInstanceSim1", "sInstanceSim2", "sInstanceSim3", "sInstanceSim4"
    };
    protected static Object mLock = new Object();
    public boolean mSaveNewSetUpMenu = false;
    private boolean mSetUpMenuFromMD = false;
    private boolean mReadFromPreferenceDone = false;

    private boolean mMtkStkAppInstalled = false;

    /// M: BIP {
    private BipService mBipService = null;
    final static String BIP_STATE_CHANGED =
            "mediatek.intent.action.BIP_STATE_CHANGED";
    /// M: BIP }

    // [20120420,mtk80601,ALPS264008]
    private int simState = TelephonyManager.SIM_STATE_UNKNOWN;
    private int simIdfromIntent = 0;

    // MTK-START [mtk80589][121026][ALPS00376525] STK dialog pop up caused ISVR
    private boolean isIvsrBootUp = false;
    private static final int IVSR_DELAYED_TIME = 60 * 1000;
    // MTK-END [mtk80589][121026][ALPS00376525] STK dialog pop up caused ISVR
    private boolean isDisplayTextDisabled = false;
    private static final int DISABLE_DISPLAY_TEXT_DELAYED_TIME = 30 * 1000;

    private boolean mIsProactiveCmdResponsed = false;

    private  MtkRIL mMtkCmdIf;

    private int mPhoneType = PhoneConstants.PHONE_TYPE_NONE;

    Handler mTimeoutHandler = new Handler() {
        @Override
        public void handleMessage(Message msg) {
            switch (msg.what) {
                case MSG_ID_CACHED_DISPLAY_TEXT_TIMEOUT:
                    MtkCatLog.d(this, "Cache DISPLAY_TEXT time out, sim_id: " + mSlotId);
                    break;
                case MSG_ID_DISABLE_DISPLAY_TEXT_DELAYED:
                    MtkCatLog.d(this, "[Reset Disable Display Text flag because timeout");
                    isDisplayTextDisabled = false;
                    break;
                default:
                    break;
            }
        }
    };

    void cancelTimeOut(int msg) {
        MtkCatLog.d(this, "cancelTimeOut, sim_id: " + mSlotId + ", msg id: " + msg);
        mTimeoutHandler.removeMessages(msg);
    }

    void startTimeOut(int msg, long delay) {
        MtkCatLog.d(this, "startTimeOut, sim_id: " + mSlotId + ", msg id: " + msg);
        cancelTimeOut(msg);
        mTimeoutHandler.sendMessageDelayed(mTimeoutHandler.obtainMessage(msg), delay);
    }

    /* For multisim catservice should not be singleton */
    public MtkCatService(CommandsInterface ci, UiccCardApplication ca, IccRecords ir,
            Context context, IccFileHandler fh, UiccProfile uiccProfile, int slotId) {
        super(ci, ca, ir, context, fh, uiccProfile, slotId);

        MtkCatLog.d(this, "slotId " + slotId);
        mMtkCmdIf = (MtkRIL)ci;

        /// M: BIP {
        if (!(SystemProperties.get("ro.vendor.mtk_ril_mode").equals("c6m_1rild"))) {
            mBipService = BipService.getInstance(mContext, this, mSlotId, mCmdIf, fh);
        }
        /// M: BIP }

        // MTK-START [mtk80589][121026][ALPS00376525] STK dialog pop up caused ISVR
        IntentFilter intentFilter = new IntentFilter(TelephonyIntents.ACTION_IVSR_NOTIFY);
        // MTK-END [mtk80589][121026][ALPS00376525] STK dialog pop up caused ISVR
        intentFilter.addAction(TelephonyIntents.ACTION_SIM_RECOVERY_DONE);
        intentFilter.addAction(ModemSwitchHandler.ACTION_MD_TYPE_CHANGE);
        IntentFilter mSIMStateChangeFilter = new IntentFilter(
                TelephonyManager.ACTION_SIM_CARD_STATE_CHANGED);
        mContext.registerReceiver(MtkCatServiceReceiver, intentFilter);
        mContext.registerReceiver(MtkCatServiceReceiver, mSIMStateChangeFilter);
        MtkCatLog.d(this, "CatService: is running");

        mMtkCmdIf.setOnStkSetupMenuReset(this, MSG_ID_SETUP_MENU_RESET, null);

        mMtkStkAppInstalled = isMtkStkAppInstalled();
        MtkCatLog.d(this, "MTK STK app installed = " + mMtkStkAppInstalled);
    }

    private void sendTerminalResponseByCurrentCmd(CatCmdMessage catCmd) {
        if (catCmd == null) {
            MtkCatLog.e(this, "catCmd is null.");
            return;
        }
        CommandType cmdType = AppInterface.CommandType.fromInt(catCmd.mCmdDet.typeOfCommand);
        MtkCatLog.d(this, "Send TR for cmd: " + cmdType);
        switch (cmdType) {
            case SET_UP_MENU:
            case SET_UP_IDLE_MODE_TEXT:
                sendTerminalResponse(catCmd.mCmdDet, ResultCode.OK, false, 0, null);
                break;
            case SET_UP_CALL:
                mMtkCmdIf.handleStkCallSetupRequestFromSimWithResCode(false,
                        ResultCode.OK.value(), null);
                break;
            default:
                sendTerminalResponse(catCmd.mCmdDet, ResultCode.UICC_SESSION_TERM_BY_USER, false,
                        0, null);
                break;
        }
    }

    public void dispose() {
        synchronized (sInstanceLock) {
            MtkCatLog.d(this, "Disposing MtkCatService object : " + mSlotId);

            mContext.unregisterReceiver(MtkCatServiceReceiver);
            if (!mIsProactiveCmdResponsed && mCurrntCmd != null) {
                MtkCatLog.d(this, "Send TR for the last pending commands.");
                sendTerminalResponseByCurrentCmd(mCurrntCmd);
            }
            mMtkCmdIf.unSetOnStkSetupMenuReset(this);
            mCmdIf.unregisterForIccRefresh(this);
            // Clean SharedPreferences
            handleDBHandler(mSlotId);
        }
        /// M: BIP{
        if (null != mBipService) {
            mBipService.dispose();
        }
        /// M: BIP }
        super.dispose();
    }

    @Override
    protected  void handleRilMsg(RilMessage rilMsg) {
        if (rilMsg == null) {
            return;
        }

        // dispatch messages
        CommandParams cmdParams = null;
        switch (rilMsg.mId) {
            case MSG_ID_EVENT_NOTIFY:
                cmdParams = (CommandParams) rilMsg.mData;
                if (cmdParams != null) {
                    if (rilMsg.mResCode == ResultCode.OK) {
                        handleCommand(cmdParams, false);
                    } else {
                        MtkCatLog.d(this, "event notify error code: " + rilMsg.mResCode);
                        if (rilMsg.mResCode == ResultCode.PRFRMD_ICON_NOT_DISPLAYED
                                && (cmdParams.mCmdDet.typeOfCommand == 0x11// send SS
                                || cmdParams.mCmdDet.typeOfCommand == 0x12 // send USSD
                                || cmdParams.mCmdDet.typeOfCommand == 0x13 // send SMS
                                || cmdParams.mCmdDet.typeOfCommand == 0x14 // send DTMF
                                )) {
                            MtkCatLog.d(this, "notify user text message even though get icon fail");
                            handleCommand(cmdParams, false);
                        }
                        if (cmdParams.mCmdDet.typeOfCommand == 0x40) {
                            MtkCatLog.d(this, "Open Channel with ResultCode");
                            handleCommand(cmdParams, false);
                        }
                    }
                }
                return;
            case MSG_ID_PROACTIVE_COMMAND:
                if (rilMsg.mId == MSG_ID_PROACTIVE_COMMAND) {
                    mIsProactiveCmdResponsed = false;
                }
                try {
                    cmdParams = (CommandParams) rilMsg.mData;
                } catch (ClassCastException e) {
                    // for error handling : cast exception
                    MtkCatLog.d(this, "Fail to parse proactive command");
                    // Don't send Terminal Resp if command detail is not
                    // available
                    if (mCurrntCmd != null) {
                        sendTerminalResponse(mCurrntCmd.mCmdDet,
                                ResultCode.CMD_DATA_NOT_UNDERSTOOD, false, 0x00, null);
                    }
                    return;
                }
                if (cmdParams != null) {
                    if (cmdParams.getCommandType() == CommandType.SET_UP_MENU) {
                        mSetUpMenuFromMD = ((MtkRilMessage) (rilMsg)).mSetUpMenuFromMD;
                    }
                    if (rilMsg.mResCode == ResultCode.OK
                            || rilMsg.mResCode == ResultCode.PRFRMD_ICON_NOT_DISPLAYED) {
                        handleCommand(cmdParams, true);
                    } else {
                        // for proactive commands that couldn't be decoded
                        // successfully respond with the code generated by the
                        // message decoder.
                        MtkCatLog.d("CAT", "SS-handleMessage: invalid proactive command: "
                                + cmdParams.mCmdDet.typeOfCommand);
                        sendTerminalResponse(cmdParams.mCmdDet, rilMsg.mResCode, false, 0, null);
                    }
                }
                return;
        }
        super.handleRilMsg(rilMsg);
    }

    /**
     * Handles RIL_UNSOL_STK_EVENT_NOTIFY or RIL_UNSOL_STK_PROACTIVE_COMMAND
     * command from RIL. Sends valid proactive command data to the application
     * using intents. RIL_REQUEST_STK_SEND_TERMINAL_RESPONSE will be send back
     * if the command is from RIL_UNSOL_STK_PROACTIVE_COMMAND.
     */
    protected void handleCommand(CommandParams cmdParams, boolean isProactiveCmd) {
        MtkCatLog.d(this, cmdParams.getCommandType().name());

        // Log all proactive commands.
        if (isProactiveCmd) {
            if (mUiccController != null) {
                mUiccController.addCardLog("ProactiveCommand mSlotId=" + mSlotId + " cmdParams="
                        + cmdParams);
            }
        }

        CharSequence message;
        ResultCode resultCode;
        MtkCatCmdMessage cmdMsg = new MtkCatCmdMessage(cmdParams);

        Message response = null;

        // add for [ALPS00245360] should not show DISPLAY_TEXT dialog when alarm booting
        boolean isAlarmState = false;
        boolean isFlightMode = false;
        int flightMode = 0;

        switch (cmdParams.getCommandType()) {
            case SET_UP_MENU:
                if (removeMenu(cmdMsg.getMenu())) {
                    mMenuCmd = null;
                } else {
                    mMenuCmd = cmdMsg;
                }
                MtkCatLog.d("CAT", "mSetUpMenuFromMD: " + mSetUpMenuFromMD);
                if (cmdMsg.getMenu() != null) {
                    ((MtkMenu) (cmdMsg.getMenu())).setSetUpMenuFlag(((mSetUpMenuFromMD == true) ? 1
                            : 0));
                }
                if (!mSetUpMenuFromMD) {
                    mIsProactiveCmdResponsed = true;
                    break;
                }
                mSetUpMenuFromMD = false;

                resultCode = cmdParams.mLoadIconFailed ? ResultCode.PRFRMD_ICON_NOT_DISPLAYED
                        : ResultCode.OK;
                sendTerminalResponse(cmdParams.mCmdDet, resultCode, false, 0, null);
                break;
            case DISPLAY_TEXT:
                // add for [ALPS00245360] should not show DISPLAY_TEXT dialog
                // when alarm booting
                isAlarmState = isAlarmBoot();
                try {
                    flightMode = Settings.Global.getInt(mContext.getContentResolver(),
                            Settings.Global.AIRPLANE_MODE_ON);
                } catch (SettingNotFoundException e) {
                    MtkCatLog.d(this, "fail to get property from Settings");
                    flightMode = 0;
                }
                isFlightMode = (flightMode != 0);
                MtkCatLog.d(this, "isAlarmState = " + isAlarmState + ", isFlightMode = "
                        + isFlightMode + ", flightMode = " + flightMode);

                if (isAlarmState && isFlightMode) {
                    sendTerminalResponse(cmdParams.mCmdDet, ResultCode.OK, false, 0, null);
                    return;
                }

                // add for SetupWizard
                if (checkSetupWizardInstalled() == true) {
                    sendTerminalResponse(cmdParams.mCmdDet, ResultCode.BACKWARD_MOVE_BY_USER,
                            false, 0, null);
                    return;
                }

                // MTK-START [mtk80589][121026][ALPS00376525] STK dialog pop up caused ISVR
                if (isIvsrBootUp) {
                    MtkCatLog.d(this, "[IVSR send TR directly");
                    sendTerminalResponse(cmdParams.mCmdDet, ResultCode.BACKWARD_MOVE_BY_USER,
                            false, 0, null);
                    return;
                }
                // MTK-END [mtk80589][121026][ALPS00376525] STK dialog pop up caused ISVR
                if (isDisplayTextDisabled) {
                    MtkCatLog.d(this, "[Sim Recovery send TR directly");
                    sendTerminalResponse(cmdParams.mCmdDet, ResultCode.BACKWARD_MOVE_BY_USER,
                            false, 0, null);
                    return;
                }
                break;
            case SET_UP_IDLE_MODE_TEXT:
                resultCode = cmdParams.mLoadIconFailed ? ResultCode.PRFRMD_ICON_NOT_DISPLAYED
                        : ResultCode.OK;
                sendTerminalResponse(cmdParams.mCmdDet, resultCode, false, 0, null);
                break;
            case SET_UP_EVENT_LIST:
                /// M: BIP {
                if (mBipService != null) {
                    mBipService.setSetupEventList(cmdMsg);
                }
                /// M: BIP }
                mIsProactiveCmdResponsed = true;
                break;
            case PROVIDE_LOCAL_INFORMATION:
                ResponseData resp = null;

                if (cmdParams.mCmdDet.commandQualifier == CommandParamsFactory.DTTZ_SETTING) {

                    Calendar cal = Calendar.getInstance();
                    int temp = 0;
                    int hibyte = 0;
                    int lobyte = 0;
                    byte[] datetime = new byte[7];

                    temp = cal.get(Calendar.YEAR) - 2000;
                    hibyte = temp / 10;
                    lobyte = (temp % 10) << 4;
                    datetime[0] = (byte) (lobyte | hibyte);

                    temp = cal.get(Calendar.MONTH) + 1;
                    hibyte = temp / 10;
                    lobyte = (temp % 10) << 4;
                    datetime[1] = (byte) (lobyte | hibyte);

                    temp = cal.get(Calendar.DATE);
                    hibyte = temp / 10;
                    lobyte = (temp % 10) << 4;
                    datetime[2] = (byte) (lobyte | hibyte);

                    temp = cal.get(Calendar.HOUR_OF_DAY);
                    hibyte = temp / 10;
                    lobyte = (temp % 10) << 4;
                    datetime[3] = (byte) (lobyte | hibyte);

                    temp = cal.get(Calendar.MINUTE);
                    hibyte = temp / 10;
                    lobyte = (temp % 10) << 4;
                    datetime[4] = (byte) (lobyte | hibyte);

                    temp = cal.get(Calendar.SECOND);
                    hibyte = temp / 10;
                    lobyte = (temp % 10) << 4;
                    datetime[5] = (byte) (lobyte | hibyte);

                    // the ZONE_OFFSET is expressed in quarters of an hour
                    temp = cal.get(Calendar.ZONE_OFFSET) / (15 * 60 * 1000);
                    hibyte = temp / 10;
                    lobyte = (temp % 10) << 4;
                    datetime[6] = (byte) (lobyte | hibyte);

                    resp = new MtkProvideLocalInformationResponseData(datetime[0], datetime[1],
                            datetime[2], datetime[3], datetime[4], datetime[5], datetime[6]);

                    sendTerminalResponse(cmdParams.mCmdDet, ResultCode.OK, false, 0, resp);

                    return;
                } else if (cmdParams.mCmdDet.commandQualifier
                        == CommandParamsFactory.LANGUAGE_SETTING) {

                    byte[] lang = new byte[2];
                    Locale locale = Locale.getDefault();

                    lang[0] = (byte) locale.getLanguage().charAt(0);
                    lang[1] = (byte) locale.getLanguage().charAt(1);

                    resp = new MtkProvideLocalInformationResponseData(lang);

                    sendTerminalResponse(cmdParams.mCmdDet, ResultCode.OK, false, 0, resp);

                    return;
                } else if (cmdParams.mCmdDet.commandQualifier
                        == MtkCommandParamsFactory.BATTERY_STATE) {
                    int batterystate = getBatteryState(mContext);
                    resp = new MtkProvideLocalInformationResponseData(batterystate);
                    sendTerminalResponse(cmdParams.mCmdDet, ResultCode.OK, false, 0, resp);
                    return;
                }
                // No need to start STK app here.
                return;
            case LAUNCH_BROWSER:
                if ((((LaunchBrowserParams) cmdParams).mConfirmMsg.text != null)
                        && (((LaunchBrowserParams) cmdParams).mConfirmMsg.text
                                .equals(STK_DEFAULT))) {
                    message = mContext.getText(com.android.internal.R.string.launchBrowserDefault);
                    ((LaunchBrowserParams) cmdParams).mConfirmMsg.text = message.toString();
                }
                break;
            case SELECT_ITEM:
                // add for [ALPS00245360] should not show DISPLAY_TEXT dialog
                // when alarm booting
                isAlarmState = isAlarmBoot();
                try {
                    flightMode = Settings.Global.getInt(mContext.getContentResolver(),
                            Settings.Global.AIRPLANE_MODE_ON);
                } catch (SettingNotFoundException e) {
                    MtkCatLog.d(this, "fail to get property from Settings");
                    flightMode = 0;
                }
                isFlightMode = (flightMode != 0);
                MtkCatLog.d(this, "isAlarmState = " + isAlarmState + ", isFlightMode = "
                        + isFlightMode + ", flightMode = " + flightMode);
                if (isAlarmState && isFlightMode) {
                    sendTerminalResponse(cmdParams.mCmdDet, ResultCode.UICC_SESSION_TERM_BY_USER,
                            false, 0, null);
                    return;
                }
                break;
            case GET_INPUT:
            case GET_INKEY:
                simState = MtkTelephonyManagerEx.getDefault().getSimCardState(mSlotId);
                MtkCatLog.d(this, "simState: " + simState);
                if (simState != TelephonyManager.SIM_STATE_PRESENT) {
                    sendTerminalResponse(cmdParams.mCmdDet,
                            ResultCode.TERMINAL_CRNTLY_UNABLE_TO_PROCESS, false, 0, null);
                    return;
                }
                break;
            case REFRESH:
            case RUN_AT:
                if (STK_DEFAULT.equals(((DisplayTextParams)cmdParams).mTextMsg.text)) {
                    // Remove the default text which was temporarily added and shall not be shown
                    ((DisplayTextParams)cmdParams).mTextMsg.text = null;
                }
                mIsProactiveCmdResponsed = true;
                break;
            case SEND_DTMF:
            case SEND_SMS:
            case SEND_SS:
            case SEND_USSD:
                mIsProactiveCmdResponsed = true;
                if ((((DisplayTextParams) cmdParams).mTextMsg.text != null)
                        && (((DisplayTextParams) cmdParams).mTextMsg.text.equals(STK_DEFAULT))) {
                    message = mContext.getText(com.android.internal.R.string.sending);
                    ((DisplayTextParams) cmdParams).mTextMsg.text = message.toString();
                }
                break;
            case PLAY_TONE:
                mIsProactiveCmdResponsed = true;
                break;
            case SET_UP_CALL:
                if ((((CallSetupParams) cmdParams).mConfirmMsg.text != null)
                        && (((CallSetupParams) cmdParams).mConfirmMsg.text.equals(STK_DEFAULT))) {
                    message = mContext.getText(com.android.internal.R.string.SetupCallDefault);
                    ((CallSetupParams) cmdParams).mConfirmMsg.text = message.toString();
                }
                break;
            case OPEN_CHANNEL:
            case CLOSE_CHANNEL:
            case RECEIVE_DATA:
            case SEND_DATA:
                BIPClientParams cmd = (BIPClientParams) cmdParams;
                /*
                 * Per 3GPP specification 102.223, if the alpha identifier is
                 * not provided by the UICC, the terminal MAY give information
                 * to the user noAlphaUsrCnf defines if you need to show user
                 * confirmation or not
                 */
                boolean noAlphaUsrCnf = false;
                try {
                    noAlphaUsrCnf = mContext.getResources().getBoolean(
                            com.android.internal.R.bool.config_stkNoAlphaUsrCnf);
                } catch (NotFoundException e) {
                    noAlphaUsrCnf = false;
                }
                if ((cmd.mTextMsg.text == null) && (cmd.mHasAlphaId || noAlphaUsrCnf)) {
                    MtkCatLog.d(this, "cmd " + cmdParams.getCommandType() + " with null alpha id");
                    // If alpha length is zero, we just respond with OK.
                    if (isProactiveCmd) {
                        sendTerminalResponse(cmdParams.mCmdDet, ResultCode.OK, false, 0, null);
                    } else if (cmdParams.getCommandType() == CommandType.OPEN_CHANNEL) {
                        mMtkCmdIf.handleStkCallSetupRequestFromSimWithResCode(true,
                                ResultCode.OK.value(), null);
                    }
                    return;
                }
                // Respond with permanent failure to avoid retry if STK app is
                // not present.
                if (!(mStkAppInstalled || mMtkStkAppInstalled)) {
                    MtkCatLog.d(this, "No STK application found.");
                    if (isProactiveCmd) {
                        sendTerminalResponse(cmdParams.mCmdDet,
                                ResultCode.BEYOND_TERMINAL_CAPABILITY, false, 0, null);
                        return;
                    }
                }
                /*
                 * CLOSE_CHANNEL, RECEIVE_DATA and SEND_DATA can be delivered by
                 * either PROACTIVE_COMMAND or EVENT_NOTIFY. If
                 * PROACTIVE_COMMAND is used for those commands, send terminal
                 * response here.
                 */
                if (isProactiveCmd
                        && ((cmdParams.getCommandType() == CommandType.CLOSE_CHANNEL)
                                || (cmdParams.getCommandType() == CommandType.RECEIVE_DATA)
                                || (cmdParams.getCommandType() == CommandType.SEND_DATA))) {
                    sendTerminalResponse(cmdParams.mCmdDet, ResultCode.OK, false, 0, null);
                }
                break;
            default:
                MtkCatLog.d(this, "HandleCommand Callback to CatService");
                super.handleCommand(cmdParams, isProactiveCmd);
                return;
        }
        mCurrntCmd = cmdMsg;
        // Only one STK app on device, MTK or AOSP.
        // For compatibility AOSP and MTK STK APP, we send two broadcast
        mtkBroadcastCatCmdIntent(cmdMsg);
        broadcastCatCmdIntent(cmdMsg.convertToCatCmdMessage(cmdParams, cmdMsg));
    }

    private void mtkBroadcastCatCmdIntent(CatCmdMessage cmdMsg) {
        Intent intent = new Intent(MtkAppInterface.MTK_CAT_CMD_ACTION);
        intent.putExtra("STK CMD", cmdMsg);
        intent.putExtra("SLOT_ID", mSlotId);
        intent.setComponent(AppInterface.getDefaultSTKApplication());
        MtkCatLog.d(this, "mtkBroadcastCatCmdIntent Sending CmdMsg: " + cmdMsg + " on slotid:"
                + mSlotId);
        mContext.sendBroadcast(intent, AppInterface.STK_PERMISSION);
    }

    @Override
    protected void onSetResponsedFlag() {
         mIsProactiveCmdResponsed = true;
    }

    @Override
    protected void sendMenuSelection(int menuId, boolean helpRequired) {
        MtkCatLog.d("CatService", "sendMenuSelection SET_UP_MENU");
        super.sendMenuSelection(menuId, helpRequired);
        cancelTimeOut(MSG_ID_DISABLE_DISPLAY_TEXT_DELAYED);
        isDisplayTextDisabled = false;
    }

    public static CatService getInstance(CommandsInterface ci, Context context,
            UiccProfile uiccProfile) {
        MtkCatLog.d("CatService", "call getInstance 2");
        int sim_id = PhoneConstants.SIM_ID_1;
        if (uiccProfile != null) {
            sim_id = uiccProfile.getPhoneId();
            MtkCatLog.d("CatService", "get SIM id from UiccCard. sim id: " + sim_id);
        }
        return CatService.getInstance(ci, context, uiccProfile, sim_id);
    }

    /**
     * Used by application to get an AppInterface object.
     *
     * @return The only Service object in the system
     */
    public static MtkAppInterface getInstance() {
        MtkCatLog.d("CatService", "call getInstance 4");
        return (MtkCatService) getInstance(null, null, null, PhoneConstants.SIM_ID_1);
    }

    /**
     * Used by application to get an AppInterface object by slotId.
     *
     * @return The only Service object in the system
     */
    public static MtkAppInterface getInstance(int slotId) {
        MtkCatLog.d("CatService", "call getInstance 3");
        return (MtkCatService) getInstance(null, null, null, slotId);
    }

    /* when read set up menu data from db, handle it */
    private static void handleProactiveCmdFromDB(MtkCatService inst, String data) {
        if (data == null) {
            MtkCatLog.d("MtkCatService", "handleProactiveCmdFromDB: cmd = null");
            return;
        }

        MtkCatLog.d("MtkCatService", " handleProactiveCmdFromDB: cmd = " + data + " from: " + inst);
        MtkRilMessage rilMsg = new MtkRilMessage(MSG_ID_PROACTIVE_COMMAND, data);
        inst.mMsgDecoder.sendStartDecodingMessageParams(rilMsg);
        MtkCatLog.d("MtkCatService", "handleProactiveCmdFromDB: over");

    }

    /*
     * if the second byte is "81", and the seventh byte is "25", this cmd is
     * valid set up menu cmd if the second byte is not "81", but the sixth byte
     * is "25", this cmd is valid set up menu cmd, too. else, it is not a set up
     * menu, no need to save it into db
     */
    private boolean isSetUpMenuCmd(String cmd) {
        boolean validCmd = false;

        if (cmd == null) {
            return false;
        }
        try {
            if ((cmd.charAt(2) == '8') && (cmd.charAt(3) == '1')) {
                if ((cmd.charAt(12) == '2') && (cmd.charAt(13) == '5')) {
                    validCmd = true;
                }
            } else {
                if ((cmd.charAt(10) == '2') && (cmd.charAt(11) == '5')) {
                    validCmd = true;
                }
            }
        } catch (IndexOutOfBoundsException e) {
            MtkCatLog.d(this, "IndexOutOfBoundsException isSetUpMenuCmd: " + cmd);
            e.printStackTrace();
            return false;
        }

        return validCmd;
    }

    /**
     * Query if the framework got SET_UP_MENU from modem or not.
     *
     * @internal
     */
    public static boolean getSaveNewSetUpMenuFlag(int sim_id) {
        boolean result = false;
        if ((sInstance != null) && (sInstance[sim_id] != null)) {
            result = ((MtkCatService) sInstance[sim_id]).mSaveNewSetUpMenu;
            MtkCatLog.d("CatService", sim_id + " , mSaveNewSetUpMenu: " + result);
        }

        return result;
    }

    @Override
    public void handleMessage(Message msg) {
        MtkCatLog.d(this, "MtkCatservice handleMessage[" + msg.what + "]");
        MtkCatCmdMessage cmd = null;
        ResponseData resp = null;
        int ret = 0;

        switch (msg.what) {
            case MSG_ID_SESSION_END:
            case MSG_ID_PROACTIVE_COMMAND:
            case MSG_ID_EVENT_NOTIFY:
            case MSG_ID_REFRESH:
                MtkCatLog.d(this, "ril message arrived, slotid:" + mSlotId);
                String data = null;
                boolean flag = false;
                if (msg.obj != null) {
                    AsyncResult ar = (AsyncResult) msg.obj;
                    if (mMsgDecoder == null) {
                        MtkCatLog.e(this, "mMsgDecoder == null, return.");
                        return;
                    }
                    if (ar != null && ar.result != null) {
                        try {
                            data = (String) ar.result;

                            // if the data is valid set up cmd, save it into db
                            boolean isValid = isSetUpMenuCmd(data);
                            if (isValid && this == sInstance[mSlotId]) {
                                saveCmdToPreference(mContext, sInstKey[mSlotId], data);
                                mSaveNewSetUpMenu = true;
                                flag = true;
                                MtkRilMessage rilMsg = new MtkRilMessage(msg.what, data);
                                rilMsg.setSetUpMenuFromMD(flag);
                                mMsgDecoder.sendStartDecodingMessageParams(rilMsg);
                                return;
                            }
                            if (data.contains("BIP")) {
                                Intent intent = new Intent(BIP_STATE_CHANGED);
                                intent.putExtra("BIP_CMD", data);
                                intent.putExtra("SLOT_ID", mSlotId);
                                final String packageName = "com.mediatek.engineermode";
                                intent.setPackage(packageName);
                                MtkCatLog.d(this, "Broadcast BIP Intent: Sending data: " +
                                    data + " on slotid:" + mSlotId);
                                mContext.sendBroadcast(intent);
                                return;
                            }
                        } catch (ClassCastException e) {
                            return;
                        }
                    }
                }
                break;
            case MSG_ID_EVENT_DOWNLOAD:
                handleEventDownload((MtkCatResponseMessage) msg.obj);
                return;
            case MSG_ID_DB_HANDLER:
                handleDBHandler(msg.arg1);
                return;
                // MTK-START [mtk80589][121026][ALPS00376525] STK dialog pop up caused ISVR
            case MSG_ID_IVSR_DELAYED:
                MtkCatLog.d(this, "[IVSR cancel IVSR flag");
                isIvsrBootUp = false;
                return;
                // MTK-END [mtk80589][121026][ALPS00376525] STK dialog pop up caused  ISVR
            case MSG_ID_SETUP_MENU_RESET:
                MtkCatLog.d(this, "SETUP_MENU_RESET : Setup menu reset.");
                AsyncResult ar = (AsyncResult) msg.obj;
                if (ar != null && ar.exception == null) {
                    mSaveNewSetUpMenu = false;
                } else {
                    MtkCatLog.d(this, "SETUP_MENU_RESET : AsyncResult null.");
                }
                return;
            case MSG_ID_LAUNCH_DB_SETUP_MENU:
                MtkCatLog.d(this, "MSG_ID_LAUNCH_DB_SETUP_MENU");
                String strCmd = null;
                MtkCatService inst = null;

                strCmd = readCmdFromPreference((MtkCatService) sInstance[mSlotId], mContext,
                        sInstKey[mSlotId]);

                if (null != sInstance[mSlotId] && null != strCmd) {
                    handleProactiveCmdFromDB((MtkCatService) sInstance[mSlotId], strCmd);
                    mSaveNewSetUpMenu = true;
                }
                return;
        }
        super.handleMessage(msg);
    }

    @Override
    public synchronized void onCmdResponse(CatResponseMessage resMsg) {
        MtkCatLog.d(this, "MtkCatService onCmdResponse");
        if (resMsg == null) {
            return;
        }
        Message msg = null;
        if (MtkCatResponseMessage.class.isInstance(resMsg)) {
            // queue a response message.
            msg = obtainMessage(MSG_ID_RESPONSE, resMsg);
            msg.sendToTarget();
        } else {
            // queue a response message.
            MtkCatResponseMessage resMtkMsg = null;
            if (mCurrntCmd != null) {
                resMtkMsg = new MtkCatResponseMessage(mCurrntCmd, resMsg);
            } else {
                resMtkMsg = new MtkCatResponseMessage(MtkCatCmdMessage.getCmdMsg(), resMsg);
            }
            msg = obtainMessage(MSG_ID_RESPONSE, resMtkMsg);
            msg.sendToTarget();
        }
    }

    public synchronized void onEventDownload(MtkCatResponseMessage resMsg) {
        if (resMsg == null) {
            return;
        }
        // queue a response message.
        Message msg = obtainMessage(MSG_ID_EVENT_DOWNLOAD, resMsg);
        msg.sendToTarget();
    }

    public synchronized void onDBHandler(int sim_id) {
        // queue a response message.
        Message msg = obtainMessage(MSG_ID_DB_HANDLER, sim_id, 0);
        msg.sendToTarget();
    }

    public synchronized void onLaunchCachedSetupMenu() {
        // launch SET UP MENU from DB.
        Message msg = obtainMessage(MSG_ID_LAUNCH_DB_SETUP_MENU, mSlotId, 0);
        msg.sendToTarget();
    }

    private void handleEventDownload(MtkCatResponseMessage resMsg) {
        eventDownload(resMsg.mEvent, resMsg.mSourceId, resMsg.mDestinationId,
                resMsg.mAdditionalInfo, resMsg.mOneShot);
    }

    private void handleDBHandler(int sim_id) {
        MtkCatLog.d(this, "handleDBHandler, sim_id: " + sim_id);
        saveCmdToPreference(mContext, sInstKey[sim_id], null);
    }

    @Override
    protected void handleCmdResponse(CatResponseMessage resMsg) {
        if (!validateResponse(resMsg)) {
            return;
        }
        ResponseData resp = null;
        boolean helpRequired = false;
        CommandDetails cmdDet = resMsg.getCmdDetails();
        AppInterface.CommandType type = AppInterface.CommandType.fromInt(cmdDet.typeOfCommand);

        switch (resMsg.mResCode) {
            case HELP_INFO_REQUIRED:
                helpRequired = true;
                // fall through
            case OK:
            case PRFRMD_WITH_PARTIAL_COMPREHENSION:
            case PRFRMD_WITH_MISSING_INFO:
            case PRFRMD_WITH_ADDITIONAL_EFS_READ:
            case PRFRMD_ICON_NOT_DISPLAYED:
            case PRFRMD_MODIFIED_BY_NAA:
            case PRFRMD_LIMITED_SERVICE:
            case PRFRMD_WITH_MODIFICATION:
            case PRFRMD_NAA_NOT_ACTIVE:
            case PRFRMD_TONE_NOT_PLAYED:
            case TERMINAL_CRNTLY_UNABLE_TO_PROCESS:
            case NO_RESPONSE_FROM_USER:
            case UICC_SESSION_TERM_BY_USER:
            case BACKWARD_MOVE_BY_USER:
            case CMD_DATA_NOT_UNDERSTOOD:
            case USER_NOT_ACCEPT:
                if (type == CommandType.SET_UP_CALL || type == CommandType.OPEN_CHANNEL) {
                    // 3GPP TS.102.223: Open Channel alpha confirmation should
                    // not send TR
                    mMtkCmdIf.handleStkCallSetupRequestFromSimWithResCode(resMsg.mUsersConfirm,
                            resMsg.mResCode.value(), null);
                    mCurrntCmd = null;
                    return;
                }
                break;
            case NETWORK_CRNTLY_UNABLE_TO_PROCESS:
                if (type == CommandType.SET_UP_CALL) {
                    mMtkCmdIf.handleStkCallSetupRequestFromSimWithResCode(resMsg.mUsersConfirm,
                            resMsg.mResCode.value(), null);
                    mCurrntCmd = null;
                    return;
                }
                if (type == CommandType.DISPLAY_TEXT) {
                    sendTerminalResponse(cmdDet, resMsg.mResCode, resMsg.mIncludeAdditionalInfo,
                            resMsg.mAdditionalInfo, resp);
                    mCurrntCmd = null;
                    return;
                }
                break;
            case LAUNCH_BROWSER_ERROR:
                if (type == CommandType.LAUNCH_BROWSER) {
                    if (resMsg.mAdditionalInfo == 0) {
                        // Additional info for Default URL unavailable.
                        resMsg.setAdditionalInfo(0x04);
                    }
                    sendTerminalResponse(cmdDet, resMsg.mResCode, resMsg.mIncludeAdditionalInfo,
                            resMsg.mAdditionalInfo, resp);
                    mCurrntCmd = null;
                    return;
                }
                break;
        }
    super.handleCmdResponse(resMsg);
    }

    public Context getContext() {
        return mContext;
    }

    private BroadcastReceiver MtkCatServiceReceiver = new BroadcastReceiver() {

        @Override
        public void onReceive(Context context, Intent intent) {
            String action = intent.getAction();
            MtkCatLog.d(this, "CatServiceReceiver action: " + action);
            if (action.equals(TelephonyIntents.ACTION_IVSR_NOTIFY)) {
                if (mSlotId != intent.getIntExtra(PhoneConstants.SLOT_KEY,
                        PhoneConstants.SIM_ID_1)) {
                    return;
                }
                // don't send DISPLAY_TEXT to app becasue of IVSR
                String ivsrAction = intent.getStringExtra(
                        TelephonyIntents.INTENT_KEY_IVSR_ACTION);
                if (ivsrAction.equals("start")) {
                    MtkCatLog.d(this, "[IVSR set IVSR flag");
                    isIvsrBootUp = true;
                    sendEmptyMessageDelayed(MSG_ID_IVSR_DELAYED, IVSR_DELAYED_TIME);
                }
            } else if (action.equals(TelephonyIntents.ACTION_SIM_RECOVERY_DONE)
                    || action.equals(ModemSwitchHandler.ACTION_MD_TYPE_CHANGE)) {
                // Do not show display text because sim reset this time
                // may be triggerd by SIM Recovery or World Phone
                if (action.equals(TelephonyIntents.ACTION_SIM_RECOVERY_DONE)) {
                    MtkCatLog.d(this, "[Set SIM Recovery flag, sim: " + mSlotId
                            + ", isDisplayTextDisabled: " + ((isDisplayTextDisabled) ? 1 : 0));
                } else {
                    MtkCatLog.d(this, "[World phone flag: " + mSlotId + ", isDisplayTextDisabled: "
                            + ((isDisplayTextDisabled) ? 1 : 0));
                }
                startTimeOut(MSG_ID_DISABLE_DISPLAY_TEXT_DELAYED,
                        DISABLE_DISPLAY_TEXT_DELAYED_TIME);
                isDisplayTextDisabled = true;
            } else if (action.equals(TelephonyManager.ACTION_SIM_CARD_STATE_CHANGED)) {
                int id = intent.getIntExtra(PhoneConstants.SLOT_KEY, -1);
                MtkCatLog.d(this, "SIM state change, id: " + id + ", simId: " + mSlotId);
                if (id == mSlotId) {
                    simState = intent.getIntExtra(TelephonyManager.EXTRA_SIM_STATE,
                            TelephonyManager.SIM_STATE_UNKNOWN);
                    simIdfromIntent = id;
                    MtkCatLog.d(this, "simIdfromIntent[" + simIdfromIntent + "],simState["
                            + simState + "]");
                    if (simState == TelephonyManager.SIM_STATE_ABSENT) {
                        if (TelephonyManager.getDefault().hasIccCard(mSlotId)) {
                            MtkCatLog.d(this, "Igonre absent sim state");
                            return;
                        }
                        mSaveNewSetUpMenu = false;
                        handleDBHandler(mSlotId);
                    }
                }
            }
        }
    };

    @Override
    public void update(CommandsInterface ci, Context context, UiccProfile uiccProfile) {
        UiccCardApplication ca = null;
        IccRecords ir = null;

        if (uiccProfile != null) {
            /*
             * Since Cat is not tied to any application, but rather is Uicc
             * application in itself - just get first FileHandler and IccRecords
             * object
             */
            /*
             * For CDMA dual mode SIM card,when the phone type is CDMA, need get
             * Uicc application of 3GPP2.
             */
            int newPhoneType = PhoneConstants.PHONE_TYPE_GSM;
            newPhoneType = getPhoneType();
            MtkCatLog.d("MtkCatService", "update PhoneType : " + newPhoneType + ", mSlotId: "
                    + mSlotId);
            int oldPhoneType = mPhoneType;
            mPhoneType = newPhoneType;
            /**
            * When phone type is changed from C to G or G to C, card state
            * is always present and if CatService don't be disposed,has no chance to
            * report STK service is running and update menu.
            * So when phone type changed, set mCardState to initial value
            * and then send STK service is running after receive icc change
            */
            if (oldPhoneType != PhoneConstants.PHONE_TYPE_NONE && oldPhoneType != newPhoneType) {
                MtkCatLog.d("MtkCatService", "phone type change,reset card state to absent.....");
                mCardState = CardState.CARDSTATE_ABSENT;
            }
            if (mPhoneType == PhoneConstants.PHONE_TYPE_CDMA) {
                ca = uiccProfile.getApplication(UiccController.APP_FAM_3GPP2);
            } else {
                ca = uiccProfile.getApplicationIndex(0);
            }
            if (ca != null) {
                ir = ca.getIccRecords();
            }
        }

        synchronized (sInstanceLock) {
            if ((ir != null) && (mIccRecords != ir)) {
                if (mIccRecords != null) {
                    mIccRecords.unregisterForRecordsLoaded(this);
                }

                MtkCatLog.d(this,
                        "Reinitialize the Service with SIMRecords and UiccCardApplication");
                mIccRecords = ir;
                mUiccApplication = ca;

                // re-Register for SIM ready event.
                mIccRecords.registerForRecordsLoaded(this, MSG_ID_ICC_RECORDS_LOADED, null);
                MtkCatLog.d(this, "registerForRecordsLoaded slotid=" + mSlotId + " instance:"
                        + this);
            }
        }
    }

    @Override
    protected void updateIccAvailability() {
        if (null == mUiccController) {
            MtkCatLog.d(this, "updateIccAvailability, mUiccController is null");
            return;
        }

        CardState newState = CardState.CARDSTATE_ABSENT;
        UiccCard newCard = mUiccController.getUiccCard(mSlotId);
        if (newCard != null) {
            newState = newCard.getCardState();
        }
        CardState oldState = mCardState;
        mCardState = newState;
        MtkCatLog.d(this, "Slot id: " + mSlotId + " New Card State = " + newState + " "
                + "Old Card State = " + oldState);
        if (oldState == CardState.CARDSTATE_PRESENT && newState != CardState.CARDSTATE_PRESENT) {
            broadcastCardStateAndIccRefreshResp(newState, null);
        } else if (oldState != CardState.CARDSTATE_PRESENT
                && newState == CardState.CARDSTATE_PRESENT) {
            if (mCmdIf.getRadioState() == TelephonyManager.RADIO_POWER_UNAVAILABLE) {
                MtkCatLog.w(this, "updateIccAvailability(): Radio unavailable");
                mCardState = oldState;
            } else {
                // Card moved to PRESENT STATE.
                MtkCatLog.d(this, "SIM present. Reporting STK service running now...");
                mCmdIf.reportStkServiceIsRunning(null);
            }
        }
    }

    // should not show DISPLAY_TEXT dialog when alarm booting
    private boolean isAlarmBoot() {
        String bootReason = SystemProperties.get("vendor.sys.boot.reason");
        return (bootReason != null && bootReason.equals("1"));
    }

    private boolean checkSetupWizardInstalled() {
        final String packageName = "com.google.android.setupwizard";
        final String activityName = "com.google.android.setupwizard.SetupWizardActivity";

        PackageManager pm = mContext.getPackageManager();
        if (pm == null) {
            MtkCatLog.d(this, "fail to get PM");
            return false;
        }

        // ComponentName cm = new ComponentName(packageName, activityName);
        boolean isPkgInstalled = true;
        try {
            pm.getInstallerPackageName(packageName);
        } catch (IllegalArgumentException e) {
            MtkCatLog.d(this, "fail to get SetupWizard package");
            isPkgInstalled = false;
        }

        if (isPkgInstalled == true) {
            int pkgEnabledState = pm.getComponentEnabledSetting(new ComponentName(packageName,
                    activityName));
            if (pkgEnabledState == PackageManager.COMPONENT_ENABLED_STATE_ENABLED
                    || pkgEnabledState == PackageManager.COMPONENT_ENABLED_STATE_DEFAULT) {
                MtkCatLog.d(this, "should not show DISPLAY_TEXT immediately");
                return true;
            } else {
                MtkCatLog.d(this, "Setup Wizard Activity is not activate");
            }
        }

        MtkCatLog.d(this, "isPkgInstalled = false");
        return false;
    }

    /**
     * Add IccRecords interface for STK application to get menu title from SIM.
     *
     * @return IccRecords
     */
    public IccRecords getIccRecords() {
        synchronized (sInstanceLock) {
            return mIccRecords;
        }
    }

    private static void saveCmdToPreference(Context context, String key, String cmd) {
        SharedPreferences preferences = null;
        Editor editor = null;
        synchronized (mLock) {
            MtkCatLog.d("MtkCatService", "saveCmdToPreference, key: " + key + ", cmd: " + cmd);
            preferences = context.getSharedPreferences("set_up_menu", Context.MODE_PRIVATE);
            editor = preferences.edit();
            editor.putString(key, cmd);
            editor.apply();
        }
    }

    private static String readCmdFromPreference(MtkCatService inst, Context context, String key) {
        SharedPreferences preferences = null;
        String cmd = String.valueOf("");

        if (inst == null) {
            MtkCatLog.d("MtkCatService", "readCmdFromPreference with null instance");
            return null;
        }

        synchronized (mLock) {
            if (!inst.mReadFromPreferenceDone) {
                preferences = context.getSharedPreferences("set_up_menu", Context.MODE_PRIVATE);
                cmd = preferences.getString(key, "");
                inst.mReadFromPreferenceDone = true;
                MtkCatLog
                        .d("MtkCatService", "readCmdFromPreference, key: " + key + ", cmd: " + cmd);
            } else {
                MtkCatLog.d("MtkCatService", "readCmdFromPreference, do not read again");
            }
        }
        if (cmd.length() == 0) {
            cmd = null;
        }
        return cmd;
    }

    /**
     * get battery state for PROVIDE LOCAL INFORMATION:battery state.
     *
     * @param context this context.
     * @return battery state.
     * Battery state:
         * '00' = battery very low,<=5%
         * '01' = battery low, 5-15%
         * '02' = battery average, 15-60%
         * '03' = battery good, > = 60
         * '04' = battery full,100%
         * 'FF' = Status Unknown.
     */
    public static int getBatteryState(Context context) {
        int batteryState = 0xFF;
        IntentFilter filter = new IntentFilter(Intent.ACTION_BATTERY_CHANGED);
        Intent batteryStatus = context.registerReceiver(null, filter);
        if (batteryStatus != null) {
            int level = batteryStatus.getIntExtra(BatteryManager.EXTRA_LEVEL, -1);
            int scale = batteryStatus.getIntExtra(BatteryManager.EXTRA_SCALE, -1);
            int status = batteryStatus.getIntExtra(BatteryManager.EXTRA_STATUS, -1);
            boolean isCharging = status == BatteryManager.BATTERY_STATUS_CHARGING
                    || status == BatteryManager.BATTERY_STATUS_FULL;

            float batteryPct = level / (float) scale;
            MtkCatLog.d("MtkCatService", " batteryPct == " + batteryPct + "isCharging:"
                    + isCharging);
            if (isCharging) {
                batteryState = 0xFF;
            } else if (batteryPct <= 0.05) {
                batteryState = 0x00;
            } else if (batteryPct > 0.05 && batteryPct <= 0.15) {
                batteryState = 0x01;
            } else if (batteryPct > 0.15 && batteryPct <= 0.6) {
                batteryState = 0x02;
            } else if (batteryPct > 0.6 && batteryPct < 1) {
                batteryState = 0x03;
            } else if (batteryPct == 1) {
                batteryState = 0x04;
            }
        }
        MtkCatLog.d("MtkCatService", "getBatteryState() batteryState = " + batteryState);
        return batteryState;
    }

    private boolean isMtkStkAppInstalled() {
        Intent intent = new Intent(MtkAppInterface.MTK_CAT_CMD_ACTION);
        PackageManager pm = mContext.getPackageManager();
        List<ResolveInfo> broadcastReceivers = pm.queryBroadcastReceivers(intent,
                PackageManager.GET_META_DATA);
        int numReceiver = broadcastReceivers == null ? 0 : broadcastReceivers.size();

        return (numReceiver > 0);
    }

    private int getPhoneType() {
        int phoneType = PhoneConstants.PHONE_TYPE_NONE;
        int subId[] = SubscriptionManager.getSubId(mSlotId);
        if (subId != null) {
            phoneType = TelephonyManager.getDefault().getCurrentPhoneType(subId[0]);
            MtkCatLog.v(this, "getPhoneType phoneType:  " + phoneType + ", mSlotId: " + mSlotId);
        }
        return phoneType;
    }
}
