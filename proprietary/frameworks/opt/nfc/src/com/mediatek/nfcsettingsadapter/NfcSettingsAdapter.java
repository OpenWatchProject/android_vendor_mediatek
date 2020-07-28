/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein is
 * confidential and proprietary to MediaTek Inc. and/or its licensors. Without
 * the prior written permission of MediaTek inc. and/or its licensors, any
 * reproduction, modification, use or disclosure of MediaTek Software, and
 * information contained herein, in whole or in part, shall be strictly
 * prohibited.
 *
 * MediaTek Inc. (C) 2016. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
 * ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 * NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH
 * RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 * INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES
 * TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
 * RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
 * OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN MEDIATEK
 * SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE
 * RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S
 * ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE
 * RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE
 * MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE
 * CHARGE PAID BY RECEIVER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek
 * Software") have been modified by MediaTek Inc. All revisions are subject to
 * any receiver's applicable license agreements with MediaTek Inc.
 */
package com.mediatek.nfcsettingsadapter;

import android.content.Context;
import android.nfc.NfcAdapter;
import android.os.IBinder;
import android.os.RemoteException;
import android.os.ServiceManager;
import android.util.Log;
import java.util.HashMap;
import java.util.List;

public class NfcSettingsAdapter {
    static INfcSettingsAdapter sService;

    private static final String TAG = "NfcSettingsAdapter";

    /**
     * The NfcAdapter object for each application context. There is a 1-1 relationship between
     * application context and NfcAdapter object.
     */
    static HashMap<Context, NfcSettingsAdapter> sNfcSettingsAdapters = new HashMap();

    final Context mContext;

    public static final String SERVICE_SETTINGS_NAME = "nfc_settings";

    /**
     * The mode flag to control each NFC mode. MODE_READER is used to switch Tag read/write mode
     *
     * @hide
     * @internal
     */
    public static final int MODE_READER = 1;

    /**
     * The mode flag to control each NFC mode. MODE_CARD is used to switch card emulation mode
     *
     * @hide
     * @internal
     */
    public static final int MODE_CARD = 2; // keep previous name for backward compatibility

    public static final int MODE_HCE = 2; // new name

    /**
     * The mode flag to control each NFC mode. MODE_P2P is used to switch P2P mode
     *
     * @hide
     * @internal
     */
    public static final int MODE_P2P = 4;

    /**
     * To disable each NFC mode.
     *
     * @hide
     * @internal
     */
    public static final int FLAG_OFF = 0;

    /**
     * To enable each NFC mode.
     *
     * @hide
     * @internal
     */
    public static final int FLAG_ON = 1;

    NfcSettingsAdapter(Context context) {
        mContext = context;
        sService = getServiceInterface();
    }

    /**
     * Helper to get the default NFC Settings Adapter.
     *
     * @param context the calling application's context
     * @return the default NFC settings adapter, or null if no NFC settings adapter exists
     */
    public static NfcSettingsAdapter getDefaultAdapter(Context context) {
        if (NfcAdapter.getDefaultAdapter(context) == null) {
            Log.d(TAG, "getDefaultAdapter = null");
            return null;
        }

        NfcSettingsAdapter adapter = sNfcSettingsAdapters.get(context);
        if (adapter == null) {
            adapter = new NfcSettingsAdapter(context);
            sNfcSettingsAdapters.put(context, adapter);
        }

        if (sService == null) {
            sService = getServiceInterface();
            Log.d(TAG, "sService = " + sService);
        }

        Log.d(TAG, "adapter = " + adapter);
        return adapter;
    }

    private static INfcSettingsAdapter getServiceInterface() {
        /* get a handle to NFC service */
        IBinder b = ServiceManager.getService(SERVICE_SETTINGS_NAME);
        Log.d(TAG, "b = " + b);
        if (b == null) {
            return null;
        }
        return INfcSettingsAdapter.Stub.asInterface(b);
    }

    /**
     * Query specific NFC mode.
     *
     * @param one of the NFC mode, candidates are MODE_READER, MODE_P2P or MODE_CARD.
     * @return FLAG_ON or FLAG_OFF, -1 for failure
     * @hide
     * @internal
     */
    public int getModeFlag(int mode) {
        try {
            if (sService == null) {
                Log.e(TAG, "getModeFlag sService = null");
                return -1;
            }

            Log.d(TAG, "sService.getModeFlag(mode)");
            return sService.getModeFlag(mode);
        } catch (RemoteException e) {
            // attemptDeadServiceRecovery(e);
            Log.e(TAG, "getModeFlag e = " + e.toString());
            return -1;
        }
    }

    /**
     * Control specific NFC mode.
     *
     * @param one of the NFC mode, candidates are MODE_READER, MODE_P2P or MODE_CARD.
     * @param FLAG_ON or FLAG_OFF
     * @hide
     * @internal
     */
    public void setModeFlag(int mode, int flag) {
        try {
            if (sService == null) {
                Log.e(TAG, "setModeFlag sService = null");
                return;
            }
            Log.d(TAG, "sService.setModeFlag(mode)");
            sService.setModeFlag(mode, flag);
        } catch (RemoteException e) {
            // attemptDeadServiceRecovery(e);
            Log.e(TAG, "setModeFlag e = " + e.toString());
        }
    }

    /**
     * GSMA feature: is there an overflow of the routing table?
     *
     * @hide
     * @internal
     */
    public boolean isRoutingTableOverflow() {
        try {
            if (sService == null) {
                Log.e(TAG, "isRoutingTableOverflow sService = null");
                return false;
            }
            Log.d(TAG, "sService.isRoutingTableOverflow()");
            return sService.isRoutingTableOverflow();
        } catch (RemoteException e) {
            // attemptDeadServiceRecovery(e);
            Log.e(TAG, "isRoutingTableOverflow e = " + e.toString());
            return false;
        }
    }

    /**
     * GSMA feature: Should overflow menu be shown to user? Yes if GSMA mode and there is at least 1
     * service not enabled.
     *
     * @hide
     * @internal
     */
    public boolean isShowOverflowMenu() {
        try {
            if (sService == null) {
                Log.e(TAG, "isShowOverflowMenu sService = null");
                return false;
            }
            Log.d(TAG, "sService.isShowOverflowMenu()");
            return sService.isShowOverflowMenu();
        } catch (RemoteException e) {
            // attemptDeadServiceRecovery(e);
            Log.e(TAG, "isShowOverflowMenu e = " + e.toString());
            return false;
        }
    }

    /**
     * GSMA feature: Retrieve map of services with AIDs registered in the category "OTHER", and for
     * each one its current state.
     *
     * @hide
     * @internal
     */
    public List<ServiceEntry> getServiceEntryList(int userHandle) {
        try {
            if (sService == null) {
                Log.e(TAG, "getServiceEntryList sService = null");
                return null;
            }
            Log.d(TAG, "sService.getServiceEntryList()");
            return sService.getServiceEntryList(userHandle);
        } catch (RemoteException e) {
            // attemptDeadServiceRecovery(e);
            Log.e(TAG, "getServiceEntryList e = " + e.toString());
            return null;
        }
    }

    /**
     * GSMA feature: Test if a proposal for enabled/disabled services can fit in the AID routing
     * table of the NFC controller.
     *
     * @hide
     * @internal
     */
    public boolean testServiceEntryList(List<ServiceEntry> proposal) {
        try {
            if (sService == null) {
                Log.e(TAG, "testServiceEntryList sService = null");
                return false;
            }
            Log.d(TAG, "sService.testServiceEntryList()");
            return sService.testServiceEntryList(proposal);
        } catch (RemoteException e) {
            // attemptDeadServiceRecovery(e);
            Log.e(TAG, "testServiceEntryList e = " + e.toString());
            return false;
        }
    }

    /**
     * GSMA feature: Commit a new state of enabled/disabled services. The operation fails silently
     * in case of error.
     *
     * @hide
     * @internal
     */
    public void commitServiceEntryList(List<ServiceEntry> proposal) {
        try {
            if (sService == null) {
                Log.e(TAG, "commitServiceEntryList sService = null");
                return;
            }
            Log.d(TAG, "sService.commitServiceEntryList()");
            sService.commitServiceEntryList(proposal);
        } catch (RemoteException e) {
            // attemptDeadServiceRecovery(e);
            Log.e(TAG, "commitServiceEntryList e = " + e.toString());
            return;
        }
    }

    /**
     * GSMA feature: Retrieve map of services with AIDs registered in the category "OTHER", and for
     * each one its current state.
     *
     * @hide
     * @internal
     */
    public List<ServiceEntry> getNonAidBasedServiceEntryList(int userHandle) {
        try {
            if (sService == null) {
                Log.e(TAG, "getNonAidBasedServiceEntryList() - sService = null");
                return null;
            }
            Log.d(TAG, "sService.getNonAidBasedServiceEntryList()");
            return sService.getNonAidBasedServiceEntryList(userHandle);
        } catch (RemoteException e) {
            // attemptDeadServiceRecovery(e);
            Log.e(TAG, "getNonAidBasedServiceEntryList() - e = " + e.toString());
            return null;
        }
    }

    /**
     * GSMA feature: Commit a new state of enabled/disabled services. The operation fails silently
     * in case of error.
     *
     * @hide
     * @internal
     */
    public void commitNonAidBasedServiceEntryList(List<ServiceEntry> proposal) {
        try {
            if (sService == null) {
                Log.e(TAG, "commitNonAidBasedServiceEntryList() - sService = null");
                return;
            }
            Log.d(TAG, "sService.commitNonAidBasedServiceEntryList()");
            sService.commitNonAidBasedServiceEntryList(proposal);
        } catch (RemoteException e) {
            // attemptDeadServiceRecovery(e);
            Log.e(TAG, "commitNonAidBasedServiceEntryList() - e = " + e.toString());
            return;
        }
    }
}
