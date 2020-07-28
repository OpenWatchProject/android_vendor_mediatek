/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2010. All rights reserved.
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

package com.mediatek.engineermode.usb;

import android.app.Activity;
import android.app.AlertDialog;
import android.app.Dialog;
import android.app.ProgressDialog;
import android.content.DialogInterface;
import android.os.Bundle;
import android.os.Handler;
import android.os.HandlerThread;
import android.os.Looper;
import android.os.Message;
import android.os.SystemProperties;
import android.os.SystemClock;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.Button;
import android.widget.Toast;

import com.mediatek.engineermode.Elog;
import com.mediatek.engineermode.R;
import com.mediatek.engineermode.EmUtils;

import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.io.BufferedReader;
import java.util.Arrays;
import java.io.OutputStreamWriter;
import java.io.PrintWriter;
import java.io.FileOutputStream;


/**
 * Test USB OTG IF/EX function test
 *
 * @author mtk54040
 *
 */
public class UsbTest extends Activity implements OnClickListener {
    private Button mBtnEnVbusStart;
    private Button mBtnEnVbusStop;
    private Button mBtnDeVbusStart;
    private Button mBtnDeVbusStop;
    private Button mBtnEnSrpStart;
    private Button mBtnEnSrpStop;
    private Button mBtnDeSrpStart;
    private Button mBtnDeSrpStop;
    private Button mBtnAUutStart;
    private Button mBtnAUutStop;
    private Button mBtnBUutStart;
    private Button mBtnBUutStop;
    private Button mBtnBUutTD59;
    private Button[] mBtnList = null;

    private static final String TAG = "Usb/Test";

    // dialog ID and MSG ID
    private static final int DLG_STOP = 1;
    private static final int DLG_MSG = 2;
    private static final int DLG_UNKNOW_MSG = 3;
    private static final int DLG_ERROR_MSG = 4;
    // private static final int DLG_IN_PROCESS = 5;

    // private static final int DLGID_OP_IN_PROCESS = 1;
    private static final int OP_IN_PROCESS = 10;
    private static final int OP_FINISH = 11;
    private static final int UPDATAT_MSG = 12;
    private static final int ERROR_MSG = 13;

    private static final int OTG20_ENTER_DONE = 14;
    private static final int OTG20_EXIT_DONE = 15;
    private static final int OTG20_START_DONE = 16;
    private static final int OTG20_STOP_DONE = 17;

    private static final int GET_MSG = 20;
    private static final int START_TEST = 21;
    // private static final int STOP_TEST = 22;
    // private static Handler mainHandler = null;
    // ProgressDialog mDialogSearchProgress = null;
    private HandlerThread mResultCollectThread = null;
    private ResultCollectHandler mResultCollectHandler = null;
    private HandlerThread mTestThread = null;
    private TestHandler mTestHandler = null;

    //private WorkHandler mWorkHandler = null; // used to handle the work
                                                // thread

    private static final int ENABLE_VBUS = 0x01;
    private static final int ENABLE_SRP = 0x02;
    private static final int DETECT_SRP = 0x03;
    private static final int DETECT_VBUS = 0x04;
    private static final int A_UUT = 0x05;
    private static final int B_UUT = 0x06;
    private static final int TEST_SE0_NAK = 0x07;
    private static final int TEST_J = 0x08;
    private static final int TEST_K = 0x09;
    private static final int TEST_PACKET = 0x0a;
    private static final int SUSPEND_RESUME = 0x0b;
    private static final int GET_DESCRIPTOR = 0x0c;
    private static final int SET_FEATURE = 0x0d;
    private static final int TD_5_9 = 0X0e;

    private static final int USBIF_OTG20_MODE = 0x50;
    private static final int USBIF_OTG20_ENABLE_VBUS = 0x51;
    //private static final int USBIF_OTG20_DETECT_VBUS = 0x52;
    //private static final int USBIF_OTG20_ENABLE_SRP = 0x53;
    //private static final int USBIF_OTG20_DETECT_SRP = 0x54;
    private static final int USBIF_OTG20_TEST_SE0_NAK = 0x55;
    private static final int USBIF_OTG20_TEST_J = 0x56;
    private static final int USBIF_OTG20_TEST_K = 0x57;
    private static final int USBIF_OTG20_TEST_PACKET = 0x58;
    private static final int USBIF_OTG20_SUSPEND_RESUME = 0x59;
    private static final int USBIF_OTG20_GET_DESCRIPTOR = 0x5a;
    private static final int USBIF_OTG20_SET_FEATURE = 0x5b;


    private static final int USBIF_OTG20_A_UUT = 0x70;
    private static final int USBIF_OTG20_B_UUT = 0x71;

    private static final int[] BUTTONS_IDS_IF = { R.id.USB_IF_Elec_EnVBUS_Start_ID,
        R.id.USB_IF_Elec_EnVBUS_Stop_ID, R.id.USB_IF_Elec_DeVBUS_Start_ID,
        R.id.USB_IF_Elec_DeVBUS_Stop_ID, R.id.USB_IF_Elec_EnSRP_Start_ID,
        R.id.USB_IF_Elec_EnSRP_Stop_ID, R.id.USB_IF_Elec_DeSRP_Start_ID,
        R.id.USB_IF_Elec_DeSRP_Stop_ID, R.id.USB_IF_Proto_AUUT_Start_ID,
        R.id.USB_IF_Proto_AUUT_Stop_ID, R.id.USB_IF_Proto_BUUT_Start_ID,
        R.id.USB_IF_Proto_BUUT_Stop_ID, R.id.USB_IF_Proto_BUUT_TD5_9_ID };

    private static final int[] BUTTONS_IDS_EX = { R.id.USB_EX_ITEM1_Start_ID,
        R.id.USB_EX_ITEM1_Stop_ID, R.id.USB_EX_ITEM2_Start_ID,
        R.id.USB_EX_ITEM2_Stop_ID, R.id.USB_EX_ITEM3_Start_ID,
        R.id.USB_EX_ITEM3_Stop_ID, R.id.USB_EX_ITEM4_Start_ID,
        R.id.USB_EX_ITEM4_Stop_ID, R.id.USB_EX_ITEM5_Start_ID,
        R.id.USB_EX_ITEM5_Stop_ID, R.id.USB_EX_ITEM6_Start_ID,
        R.id.USB_EX_ITEM6_Stop_ID, R.id.USB_EX_ITEM7_Start_ID,
        R.id.USB_EX_ITEM7_Stop_ID };

    private static final int[] BUTTONS_IDS_IF_OTG20 = {
        R.id.USB_IF_OTG20_Start_ID, R.id.USB_IF_OTG20_Stop_ID,
        R.id.USB_IF_OTG20_Elec_EnVBUS_Start_ID, R.id.USB_IF_OTG20_Elec_EnVBUS_Stop_ID,
        /*R.id.USB_IF_OTG20_Elec_DeVBUS_Start_ID, R.id.USB_IF_OTG20_Elec_DeVBUS_Stop_ID,
        R.id.USB_IF_OTG20_Elec_EnSRP_Start_ID, R.id.USB_IF_OTG20_Elec_EnSRP_Stop_ID,
        R.id.USB_IF_OTG20_Elec_DeSRP_Start_ID, R.id.USB_IF_OTG20_Elec_DeSRP_Stop_ID,*/

        R.id.USB_IF_OTG20_Elec_TEST_SE0_NAK_Start_ID, R.id.USB_IF_OTG20_TEST_SE0_NAK_Stop_ID,
        R.id.USB_IF_OTG20_Elec_TEST_J_Start_ID, R.id.USB_IF_OTG20_Elec_TEST_J_Stop_ID,
        R.id.USB_IF_OTG20_Elec_TEST_K_Start_ID, R.id.USB_IF_OTG20_Elec_TEST_K_Stop_ID,
        R.id.USB_IF_OTG20_Elec_TEST_PACKET_Start_ID, R.id.USB_IF_OTG20_Elec_TEST_PACKET_Stop_ID,
        R.id.USB_IF_OTG20_Elec_SUSPEND_RESUME_Start_ID, R.id.USB_IF_OTG20_Elec_SUSPEND_RESUME_Stop_ID,
        R.id.USB_IF_OTG20_Elec_GET_DESCRIPTOR_Start_ID, R.id.USB_IF_OTG20_Elec_GET_DESCRIPTOR_Stop_ID,
        R.id.USB_IF_OTG20_Elec_SET_FEATURE_Start_ID, R.id.USB_IF_OTG20_Elec_SET_FEATURE_Stop_ID,

        R.id.USB_IF_OTG20_Proto_AUUT_Start_ID, R.id.USB_IF_OTG20_Proto_AUUT_Stop_ID,
        R.id.USB_IF_OTG20_Proto_BUUT_Start_ID, R.id.USB_IF_OTG20_Proto_BUUT_Stop_ID };

    private static final int[] IF_COMMOND = { ENABLE_VBUS, DETECT_VBUS,
            ENABLE_SRP, DETECT_SRP, A_UUT, B_UUT };

    private static final int[] EX_COMMAND = { TEST_SE0_NAK, TEST_J, TEST_K,
            TEST_PACKET, SUSPEND_RESUME, GET_DESCRIPTOR, SET_FEATURE };

    private static final int[] IF_OTG20_COMMAND = { USBIF_OTG20_MODE, USBIF_OTG20_ENABLE_VBUS,
            /*USBIF_OTG20_DETECT_VBUS, USBIF_OTG20_ENABLE_SRP, USBIF_OTG20_DETECT_SRP*/
            USBIF_OTG20_TEST_SE0_NAK, USBIF_OTG20_TEST_J, USBIF_OTG20_TEST_K, USBIF_OTG20_TEST_PACKET, USBIF_OTG20_SUSPEND_RESUME,
            USBIF_OTG20_GET_DESCRIPTOR, USBIF_OTG20_SET_FEATURE, USBIF_OTG20_A_UUT, USBIF_OTG20_B_UUT };

    private String usb_property_set = "sys.usb.config" ;

    private String[] cmd_mu3h_driver_init_0 =
        { "/system/bin/sh", "-c", "echo 0 > /proc/mu3h_driver_init", };
    private String[] cmd_mu3h_driver_init_1 =
        { "/system/bin/sh", "-c", "echo 1 > /proc/mu3h_driver_init", };

    private String[] cmd_mu3d_driver_init_0 =
        { "/system/bin/sh", "-c", "echo 0 > /proc/mu3d_driver_init", };
    private String[] cmd_mu3d_driver_init_1 =
        { "/system/bin/sh", "-c", "echo 1 > /proc/mu3d_driver_init", };

    private String[] cmd_android_usbif_init_0 =
        { "/system/bin/sh", "-c", "echo 0 > /proc/android_usbif_init", };
    private String[] cmd_android_usbif_init_1 =
        { "/system/bin/sh", "-c", "echo 1 > /proc/android_usbif_init", };

    private String[] cmd_ts_mu3d_start = { "/vendor/bin/ts_mu3d", "auto.dev", };
    private String[] cmd_ts_mu3d_stop = { "/vendor/bin/ts_mu3d", "auto.stop", };

    private String[] cmd_ts_mu3d_uuta_start = { "/vendor/bin/ts_mu3d", "auto.otg", "8", };
    private String[] cmd_ts_mu3h_uuta_start = { "/vendor/bin/ts_mu3h", "otg.uuta", };

    private String[] cmd_ts_mu3d_uutb_start = { "/vendor/bin/ts_mu3d", "auto.otg", "9", };
    private String[] cmd_ts_mu3h_uutb_start = { "/vendor/bin/ts_mu3h", "otg.uutb", };

    private String[] cmd_ts_mu3d_otg_stop = { "/vendor/bin/ts_mu3d", "auto.otg", "10", };
    private String[] cmd_ts_mu3h_hcdinit = { "/vendor/bin/ts_mu3h", "hcd.init", };
    private String[] cmd_ts_mu3h_hcdcleanup = { "/vendor/bin/ts_mu3h", "hcd.cleanup", };

    private String[] cmd_ts_mu3h_hcddrvvbus_start = { "/vendor/bin/ts_mu3h", "hcd.drvvbus", };
    private String[] cmd_ts_mu3h_hcddrvvbus_stop = { "/vendor/bin/ts_mu3h", "hcd.stopvbus", };

    private String[] cmd_ts_mu3h_hcdhosttestmode_start =
        { "/vendor/bin/ts_mu3h", "hcd.starthosttestmode", "0"};
    private String[] cmd_ts_mu3h_hcdhosttestmode_stop =
        { "/vendor/bin/ts_mu3h", "hcd.stophosttestmode", };


    private int mCommand = 0;
    private int mMsg = 0;
    private boolean mRun = false;
    private boolean mTestIf = false;
    private boolean mTestIf_OTG20 = false;
    private int[] mBtnIds;
    private boolean mTestIf_OTG20_enabled = false ;

    /** Called when the activity is first created. */
    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        Bundle extra = getIntent().getExtras();
        if (null != extra) {
            mTestIf = extra.getBoolean(UsbList.IF_TEST);
            mTestIf_OTG20 = extra.getBoolean(UsbList.IF_OTG20_TEST);
        }
        Elog.v(TAG, "is test IF ? " + mTestIf + ", is mTestIf_OTG20 ? " + mTestIf_OTG20);

        if (mTestIf) {
            setContentView(R.layout.usb_test);
            mBtnList = new Button[BUTTONS_IDS_IF.length];
            mBtnIds = BUTTONS_IDS_IF;
        } else if (mTestIf_OTG20) {
            setTitle(R.string.USB_IF_OTG20_TEST);
            setContentView(R.layout.usb_otg20_test);
            mBtnList = new Button[BUTTONS_IDS_IF_OTG20.length];
            mBtnIds = BUTTONS_IDS_IF_OTG20;
        } else {
            setTitle(R.string.USB_EX_TEST);
            setContentView(R.layout.usb_test_ex);
            mBtnList = new Button[BUTTONS_IDS_EX.length];
            mBtnIds = BUTTONS_IDS_EX;
        }

        for (int i = 0; i < mBtnIds.length; i++) {
            mBtnList[i] = (Button) findViewById(mBtnIds[i]);
            mBtnList[i].setOnClickListener(this);
        }

        if (mTestIf_OTG20) {
            Elog.v(TAG, "mTestIf_OTG20 test");
            updateAllBtn_OTG20(mTestIf_OTG20_enabled) ;
        } else {
            if (!UsbDriver.nativeInit()) {
                Toast.makeText(this, R.string.USB_not_support, Toast.LENGTH_SHORT)
                        .show();
                finish();
            }
        }

        mResultCollectThread = new HandlerThread(TAG + "/ResultCollect");
        mResultCollectThread.start();
        mResultCollectHandler = new ResultCollectHandler(mResultCollectThread.getLooper());
        mTestThread = new HandlerThread(TAG + "/Test");
        mTestThread.start();
        mTestHandler = new TestHandler(mTestThread.getLooper());
    }

    private Handler mUiHandler = new Handler() {
        public void handleMessage(Message msg) {
            switch (msg.what) {
            case OP_IN_PROCESS:
                showDialog(DLG_STOP);
                break;
            case OP_FINISH:
                dismissDialog(DLG_STOP);
                break;
            case UPDATAT_MSG:
                if (mMsg >= UsbDriver.MSG_LEN || mMsg < 0) {
                    showDialog(DLG_UNKNOW_MSG);
                } else {
                    showDialog(DLG_MSG);
                }
                break;
            case ERROR_MSG:
                showDialog(DLG_ERROR_MSG);
                break;
            case OTG20_ENTER_DONE:
                updateAllBtn_OTG20(true) ;
                break;
            case OTG20_EXIT_DONE:
                updateAllBtn_OTG20(false) ;
                break;
            case OTG20_STOP_DONE:
                updateAllBtn_OTG20(true) ;
                break;
            default:
                break;
            }
        }
    };


    private static StringBuilder sResult = new StringBuilder("");

    public static class StreamDrainer implements Runnable {
        private InputStream ins;

        public StreamDrainer(InputStream ins) {
            this.ins = ins;
        }

        public void run() {
            try {
                BufferedReader reader = new BufferedReader(
                        new InputStreamReader(ins));
                String line = null;
                while ((line = reader.readLine()) != null) {
                    Elog.i(TAG, "thread id " + Thread.currentThread().getId()
                            + " ,line is " + line);
                }
                Elog.i(TAG, "thread id " + Thread.currentThread().getId() + " ,read line done");
            } catch (IOException e) {
                e.printStackTrace();
            }
        }

    }

    /**
     * Write proc value for USB test setting
     *
     * @param proc_path : the proc path
     * @param value : the value writing to proc path
     */
    public static void usbExecProcWrite(String proc_path , String value) {
        // Open the sysfs file for writing and write "1" to it.
        Elog.i(TAG, "myexecProc : , proc_path is " + proc_path + " value is " + value) ;

        PrintWriter outStream = null;
        try {
            FileOutputStream fos = new FileOutputStream(proc_path);
            outStream = new PrintWriter(new OutputStreamWriter(fos));
            outStream.println(value);
        } catch (IOException e) {
            e.printStackTrace();
        } finally {
            if (outStream != null)
                outStream.close();
        }

        Elog.i(TAG, "myexecProc : done") ;
    }

    /**
     * Exec command for USB test setting
     *
     * @param command : the exec command
     */
    private static int usbExecCommand(String[] command) {

        Elog.i(TAG, "usbExecCommand : , command is " + Arrays.toString(command)) ;

        sResult.delete(0, sResult.length());
        try {
            Runtime runtime = Runtime.getRuntime();
            Process proc = runtime.exec(command);

            new Thread(new StreamDrainer(proc.getInputStream())).start();
            new Thread(new StreamDrainer(proc.getErrorStream())).start();

            proc.getOutputStream().close();

            int exitValue = proc.waitFor();

            Elog.i(TAG, "waitFor value = " + exitValue + ", done");

            return 0;
        } catch (IOException e) {
            e.printStackTrace();
        } catch (InterruptedException e) {
            e.printStackTrace();
        }

        return -1;
    }

    private int usbSwitchToIFOTG20mode(boolean enable) {

        Elog.i(TAG, "usbSwitchToIFOTG20mode : enaable is " + enable);

        if (enable) {
            // 1. stop adbd
            EmUtils.systemPropertySet(usb_property_set, "none") ;

            SystemClock.sleep(2000) ;

            // 2. stop android0
            usbExecCommand(cmd_android_usbif_init_0) ;

            //SystemClock.sleep(1000) ;

            // 3. stop mu3d normal driver
            usbExecCommand(cmd_mu3d_driver_init_0) ;
            //SystemClock.sleep(1000) ;

            // 4. stop mu3h normal driver
            usbExecCommand(cmd_mu3h_driver_init_0) ;
            //SystemClock.sleep(1000) ;

            // 5. start and enter mu3d test driver test mode
            usbExecCommand(cmd_ts_mu3d_start) ;
            SystemClock.sleep(2000); // wait driver process init done


        } else {
            // 1. stop mu3d test driver test mode
            usbExecCommand(cmd_ts_mu3d_stop);
            //SystemClock.sleep(1000) ;

            // 2. start mu3d normal driver
            usbExecCommand(cmd_mu3d_driver_init_1);
            //SystemClock.sleep(1000) ;

            // 3. start android0
            usbExecCommand(cmd_android_usbif_init_1);
            //SystemClock.sleep(1000) ;

            // 4. start mu3h normal driver
            usbExecCommand(cmd_mu3h_driver_init_1);

            SystemClock.sleep(2000) ;

            // 5. start adbd
            EmUtils.systemPropertySet(usb_property_set, "mass_storage,adb");

        }

        return 0;
    }

    private int usbIFU3DrvVbus(boolean enable) {

        Elog.i(TAG, "usbIFU3enVbus : enaable is " + enable);

        if (enable) {
            usbExecCommand(cmd_ts_mu3h_hcdinit);
            usbExecCommand(cmd_ts_mu3h_hcddrvvbus_start);
        } else {
            usbExecCommand(cmd_ts_mu3h_hcddrvvbus_stop);
            usbExecCommand(cmd_ts_mu3h_hcdcleanup);
        }
        return 0;
    }

    private int usbIFU3ElecTestMode(boolean enable, String mode) {

        Elog.i(TAG, "usbIFU3ElecTestMode : enaable is " + enable + " , mode is " + mode) ;
        if (enable) {
            usbExecCommand(cmd_ts_mu3h_hcdinit) ;
            cmd_ts_mu3h_hcdhosttestmode_start[2] = mode;
            Elog.i(TAG, "usbIFU3ElecTestMode : , cmd_ts_mu3h_hcdhosttestmode_start is "
                    + Arrays.toString(cmd_ts_mu3h_hcdhosttestmode_start)) ;
            usbExecCommand(cmd_ts_mu3h_hcdhosttestmode_start);
        } else {
            usbExecCommand(cmd_ts_mu3h_hcdhosttestmode_stop);
            usbExecCommand(cmd_ts_mu3h_hcdcleanup);
        }
        return 0;
    }

    private int usbIFOTG20uutATest(boolean enable) {

        Elog.i(TAG, "usbIFOTG20uutATest : enaable is " + enable);

        if (enable) {
            // 1. start mu3h uuta mode
            usbExecCommand(cmd_ts_mu3h_uuta_start);
            // 2. start mu3d uuta mode
            usbExecCommand(cmd_ts_mu3d_uuta_start);

        } else {
            // 1. stop mu3d uuta mode
            usbExecCommand(cmd_ts_mu3d_otg_stop);
            // 2. stop mu3h uuta mode
            usbExecCommand(cmd_ts_mu3h_hcdcleanup);

            SystemClock.sleep(3000) ;
        }
        return 0;
    }

    private int usbIFOTG20uutBTest(boolean enable) {

        Elog.i(TAG, "usbIFOTG20uutBTest : enaable is " + enable);

        if (enable) {
            // 1. start mu3h uutb mode
            usbExecCommand(cmd_ts_mu3h_uutb_start);
            // 2. start mu3d uutb mode
            usbExecCommand(cmd_ts_mu3d_uutb_start);
        } else {
            // 1. stop mu3d uutb mode
            usbExecCommand(cmd_ts_mu3d_otg_stop);
            // 2. stop mu3h uutb mode
            usbExecCommand(cmd_ts_mu3h_hcdcleanup);

            SystemClock.sleep(3000) ;
        }

        return 0;
    }


    /**
     * Enable or disable the button group
     *
     * @param enable
     */
    private void updateAllBtn(boolean enable) {
        for (Button btn : mBtnList) {
            btn.setEnabled(enable);
        }
    }

    private void updateAllBtn_OTG20(boolean in_usbif_test_mode) {
        Button btn_enter_otg20;
        Button btn_exit_otg20;

        Elog.d(TAG, "-->updateAllBtn_OTG20, mBtnIds.length is " + mBtnIds.length
                + ", BUTTONS_IDS_IF_OTG20.length is " + BUTTONS_IDS_IF_OTG20.length);

        //btn_enter_otg20 = (Button) findViewById(mBtnIds[0]);
        //btn_exit_otg20 = (Button) findViewById(mBtnIds[1]);

        btn_enter_otg20 = mBtnList[0];
        btn_exit_otg20 = mBtnList[1];

        Elog.d(TAG, "-->updateAllBtn_OTG20, in_usbif_test_mode is " + in_usbif_test_mode);

        if (in_usbif_test_mode) {
            for (Button btn : mBtnList) {
                btn.setEnabled(true);
            }
            btn_enter_otg20.setEnabled(false);
        } else {
            for (Button btn : mBtnList) {
                btn.setEnabled(false);
            }
            btn_enter_otg20.setEnabled(true);
        }

    }

    /**
     * Enable only one button
     *
     * @param selBtn
     */
    private void makeOneBtnEnable(Button selBtn) {
        for (Button btn : mBtnList) {
            if (btn == selBtn) {
                btn.setEnabled(true);
            } else {
                btn.setEnabled(false);
            }
        }
    }

    @Override
    protected void onPrepareDialog(int id, Dialog dialog) {
        switch (id) {
        case DLG_MSG:
        case DLG_UNKNOW_MSG:
            removeDialog(id);
            break;
        }
    }

    @Override
    protected Dialog onCreateDialog(int id) {
        Elog.d(TAG, "-->onCreateDialog");
        if (id == DLG_STOP) {
            ProgressDialog dialog = new ProgressDialog(this);
            dialog.setTitle(R.string.USB_IF_TEST);
            dialog.setMessage(getString(R.string.USB_IF_stop));
            dialog.setCancelable(false);
            dialog.setIndeterminate(true);
            return dialog;
        } else if (id == DLG_MSG) {
            AlertDialog dialog = new AlertDialog.Builder(this).setCancelable(
                    false).setTitle(R.string.USB_message).setMessage(
                    UsbDriver.MSG[mMsg]).setPositiveButton(R.string.OK,
                    new DialogInterface.OnClickListener() {
                        public void onClick(DialogInterface dialog, int which) {
                            // finish();
                        }
                    }).create();
            return dialog;
        } else if (id == DLG_UNKNOW_MSG) {
            AlertDialog dialog = new AlertDialog.Builder(this).setCancelable(
                    false).setTitle(R.string.USB_message).setMessage(
                    String.valueOf(mMsg)).setPositiveButton(R.string.OK,
                    new DialogInterface.OnClickListener() {
                        public void onClick(DialogInterface dialog, int which) {
                            // finish();
                        }
                    }).create();
            return dialog;
        } else if (id == DLG_ERROR_MSG) {
            AlertDialog dialog = new AlertDialog.Builder(this).setCancelable(
                    false).setTitle(R.string.USB_message).setMessage(
                    R.string.USB_msg_err).setPositiveButton(R.string.OK,
                    new DialogInterface.OnClickListener() {
                        public void onClick(DialogInterface dialog, int which) {
                            // finish();
                        }
                    }).create();
            return dialog;
        }
        return null;
    }

    @Override
    protected void onDestroy() {
        Elog.v(TAG, "-->onDestroy");
        if (mRun) {
            mRun = false;
             if (!UsbDriver.nativeStopTest(mCommand)) {
                 Elog.w(TAG, "onDestroy() nativeStopTest fail");
             }
        }
        UsbDriver.nativeCleanMsg();
        UsbDriver.nativeDeInit();
        mResultCollectThread.quit();
        mTestThread.quit();
        super.onDestroy();
    }

    public void onClick(View arg0) {
        boolean isSTART = false;
        Button stopBtn = null;
        Button startBtn = null;

        if (arg0.getId() == BUTTONS_IDS_IF[BUTTONS_IDS_IF.length - 1]) {
            mCommand = TD_5_9;
            isSTART = true;
            stopBtn = mBtnList[mBtnList.length - 3];
        } else {
            int btnIndex = findBtnIndex(arg0.getId());
            if (mTestIf) {
                mCommand = IF_COMMOND[btnIndex / 2];
            } else if (mTestIf_OTG20) {
                mCommand = IF_OTG20_COMMAND[btnIndex / 2];
            } else {
                mCommand = EX_COMMAND[btnIndex / 2];
            }

            if (0 == btnIndex % 2) {
                isSTART = true;
                startBtn = mBtnList[btnIndex];
                stopBtn = mBtnList[btnIndex + 1];
            } else {
                isSTART = false;
                startBtn = mBtnList[btnIndex - 1];
                stopBtn = mBtnList[btnIndex];
            }
        }
        Elog.v(TAG, "isSTART--" + isSTART);
        Elog.v(TAG, "command--" + mCommand);

        if (mTestIf_OTG20) {
            if (mCommand == USBIF_OTG20_MODE) {
                if (isSTART) {
                    UsbDriver.nativeCleanMsg();
                    mTestHandler.sendEmptyMessage(START_TEST);
                } else {
                    usbSwitchToIFOTG20mode(false);
                    mTestIf_OTG20_enabled = false;
                    mUiHandler.sendEmptyMessage(OTG20_EXIT_DONE);
                }
            } else {  // Other TEST command
        if (isSTART) {
            UsbDriver.nativeCleanMsg();
            makeOneBtnEnable(stopBtn);
                    mRun = true;
            mTestHandler.sendEmptyMessage(START_TEST);
                    // START command
                    mResultCollectHandler.sendEmptyMessage(GET_MSG);
                } else if (mRun) {
                    mRun = false;
                    if (mCommand == USBIF_OTG20_A_UUT) {
                        usbIFOTG20uutATest(false);
                    } else if (mCommand == USBIF_OTG20_B_UUT) {
                        usbIFOTG20uutBTest(false);
                    } else if (mCommand == USBIF_OTG20_ENABLE_VBUS) {
                        usbIFU3DrvVbus(false);
                    } else if (mCommand == USBIF_OTG20_TEST_SE0_NAK) {
                        usbIFU3ElecTestMode(false, "6");
                    } else if (mCommand == USBIF_OTG20_TEST_J) {
                        usbIFU3ElecTestMode(false, "7");
                    } else if (mCommand == USBIF_OTG20_TEST_K) {
                        usbIFU3ElecTestMode(false, "8");
                    } else if (mCommand == USBIF_OTG20_TEST_PACKET) {
                        usbIFU3ElecTestMode(false, "9");
                    } else if (mCommand == USBIF_OTG20_SUSPEND_RESUME) {
                        usbIFU3ElecTestMode(false, "10");
                    } else if (mCommand == USBIF_OTG20_GET_DESCRIPTOR) {
                        usbIFU3ElecTestMode(false, "11");
                    } else if (mCommand == USBIF_OTG20_SET_FEATURE) {
                        usbIFU3ElecTestMode(false, "12");
                    }
                    // STOP command done
                    mUiHandler.sendEmptyMessage(OTG20_STOP_DONE);

                }

            }

        } else {
            if (isSTART) {
                UsbDriver.nativeCleanMsg();
                makeOneBtnEnable(stopBtn);
                mRun = true;
                mTestHandler.sendEmptyMessage(START_TEST);
                // START command
                mResultCollectHandler.sendEmptyMessage(GET_MSG);
            } else if (mRun) {
                mRun = false;
                if (!UsbDriver.nativeStopTest(mCommand)) {
                    Toast.makeText(this, R.string.USB_stop_fail,
                                    Toast.LENGTH_SHORT).show();
                    UsbDriver.nativeCleanMsg();
                }
                updateAllBtn(true);
            }
        }
    }

    private int findBtnIndex(int id) {
        for (int i = 0; i < mBtnIds.length; i++) {
            if (id == mBtnIds[i]) {
                Elog.d(TAG, "find btn index: " + i);
                return i;
            }
        }
        Elog.d(TAG, "find btn index error");
        return -1;
    }

    private final class TestHandler extends Handler {
        TestHandler(Looper looper) {
            super(looper);
        }

        @Override
        public void handleMessage(Message msg) {
            switch (msg.what) {
            case START_TEST:
                Elog.v(TAG, "command--" + mCommand);
                Elog.v(TAG, "handleMessage command--" + mCommand);
                if (mTestIf_OTG20) {  // USBIF mode enable switch
                    if (mCommand == USBIF_OTG20_MODE) {
                        mTestIf_OTG20_enabled = true ;

                        usbSwitchToIFOTG20mode(true) ;

                        mUiHandler.sendEmptyMessage(OTG20_ENTER_DONE);
                    } else {  // other USBIF test case
                        if (mCommand == USBIF_OTG20_A_UUT) {
                            usbIFOTG20uutATest(true);
                        } else if (mCommand == USBIF_OTG20_B_UUT) {
                            usbIFOTG20uutBTest(true);
                        } else if (mCommand == USBIF_OTG20_ENABLE_VBUS) {
                            usbIFU3DrvVbus(true);
                        } else if (mCommand == USBIF_OTG20_TEST_SE0_NAK) {
                            usbIFU3ElecTestMode(true, "6");
                        } else if (mCommand == USBIF_OTG20_TEST_J) {
                            usbIFU3ElecTestMode(true, "7");
                        } else if (mCommand == USBIF_OTG20_TEST_K) {
                            usbIFU3ElecTestMode(true, "8");
                        } else if (mCommand == USBIF_OTG20_TEST_PACKET) {
                            usbIFU3ElecTestMode(true, "9");
                        } else if (mCommand == USBIF_OTG20_SUSPEND_RESUME) {
                            usbIFU3ElecTestMode(true, "10");
                        } else if (mCommand == USBIF_OTG20_GET_DESCRIPTOR) {
                            usbIFU3ElecTestMode(true, "11");
                        } else if (mCommand == USBIF_OTG20_SET_FEATURE) {
                            usbIFU3ElecTestMode(true, "12");
                        }
                    }
                } else {
                    if (!UsbDriver.nativeStartTest(mCommand)) {
                        mUiHandler.sendEmptyMessage(ERROR_MSG);
                    }
                }

                Elog.v(TAG, "Task finish");
                break;
            default:
                Elog.w(TAG, "Unhandled msg: " + msg.what);
                break;
            }
            super.handleMessage(msg);
        }
    }

    private final class ResultCollectHandler extends Handler {
        ResultCollectHandler(Looper looper) {
            super(looper);
        }

        @Override
        public void handleMessage(Message msg) {
            switch (msg.what) {
            case GET_MSG:
                 if (mRun) {
                 if (!mTestIf_OTG20) {
                     mMsg = UsbDriver.nativeGetMsg();
                     Elog.d(TAG, "getMsg() " + mMsg);
                     if (0 != mMsg) {
                         mUiHandler.sendEmptyMessage(UPDATAT_MSG);
                     } else {
                         sendEmptyMessageDelayed(GET_MSG, 200);
                     }
                 }
                 }
                break;
            default:
                Elog.w(TAG, "Unhandled msg: " + msg.what);
                break;
            }
            super.handleMessage(msg);
        }
    }

}
