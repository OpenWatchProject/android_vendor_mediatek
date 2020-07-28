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

package com.mediatek.internal.telephony;

import android.app.AppOpsManager;
import android.content.Context;
import android.os.ServiceManager;
import android.telephony.Rlog;

import com.android.internal.telephony.Phone;
import com.mediatek.internal.telephony.IMtkPhoneSubInfoEx;

// MTK-START: SIM
import android.os.Message;
import android.os.Binder;
import android.telephony.SubscriptionManager;
import com.android.internal.telephony.uicc.UiccController;
import com.android.internal.telephony.uicc.UsimServiceTable;
import com.android.internal.telephony.uicc.IccRecords;
import com.android.internal.telephony.uicc.IsimRecords;
import com.android.internal.telephony.uicc.UiccCard;
import com.android.internal.telephony.uicc.UiccCardApplication;
import com.android.internal.telephony.PhoneFactory;
import com.android.internal.telephony.TelephonyPermissions;
import com.mediatek.internal.telephony.uicc.MtkIsimUiccRecords;

import static android.Manifest.permission.READ_PRIVILEGED_PHONE_STATE;
import static android.Manifest.permission.READ_PHONE_STATE;
// MTK-END

public class MtkPhoneSubInfoControllerEx extends IMtkPhoneSubInfoEx.Stub {
    private static final String TAG = "MtkPhoneSubInfoCtlEx";
    private static final boolean DBG = true;
    private static final boolean VDBG = false; // STOPSHIP if true

    private final Phone[] mPhone;
    private final Context mContext;
    private final AppOpsManager mAppOps;

    public MtkPhoneSubInfoControllerEx(Context context, Phone[] phone) {
        mPhone = phone;
        if (ServiceManager.getService("iphonesubinfoEx") == null) {
            ServiceManager.addService("iphonesubinfoEx", this);
        }
        mContext = context;
        mAppOps = (AppOpsManager) mContext.getSystemService(Context.APP_OPS_SERVICE);
    }

    private void log(String s) {
        Rlog.d(TAG, s);
    }

    private void loge(String s) {
        Rlog.e(TAG, s);
    }

    // MTK-START: SIM
    private boolean checkReadPhoneState(String callingPackage, String message) {
        try {
            mContext.enforceCallingOrSelfPermission(READ_PRIVILEGED_PHONE_STATE, message);

            // SKIP checking run-time OP_READ_PHONE_STATE since self or using PRIVILEGED
            return true;
        } catch (SecurityException e) {
            mContext.enforceCallingOrSelfPermission(READ_PHONE_STATE, message);
        }

        return mAppOps.noteOp(AppOpsManager.OP_READ_PHONE_STATE, Binder.getCallingUid(),
                callingPackage) == AppOpsManager.MODE_ALLOWED;
    }

    private int getDefaultSubscription() {
        return  PhoneFactory.getDefaultSubscription();
    }

    /**
     * get Phone object based on subId.
     **/
    private Phone getPhone(int subId) {
        int phoneId = SubscriptionManager.getPhoneId(subId);
        if (!SubscriptionManager.isValidPhoneId(phoneId)) {
            phoneId = 0;
        }
        return mPhone[phoneId];
    }

    private IccRecords getIccRecords(int subId) {
        Phone phone = getPhone(subId);
        if (phone != null) {
            UiccCard uiccCard = phone.getUiccCard();

            UiccCardApplication uiccApp = ((uiccCard != null)
                    ? uiccCard.getApplication(UiccController.APP_FAM_3GPP) : null);
            IccRecords iccRecords = ((uiccApp != null) ? uiccApp.getIccRecords() : null);
            return iccRecords;
        }  else {
           loge("getIccRecords phone is null for Subscription:" + subId);
           return null;
        }
    }

    public boolean getUsimService(int service, String callingPackage) {
        return getUsimServiceForSubscriber(getDefaultSubscription(), service, callingPackage);
    }

    public boolean getUsimServiceForSubscriber(int subId, int service, String callingPackage) {
        Phone phone = getPhone(subId);
        if (phone != null) {
            // This is open to apps with WRITE_SMS.
            if (!checkReadPhoneState(callingPackage, "getUsimService")) {
                return false;
            }
            UsimServiceTable ust = phone.getUsimServiceTable();

            if (ust != null) {
                return ust.isAvailable(service);
            } else {
                log("getUsimService fail due to UST is null.");
                return false;
            }
        } else {
            loge("getUsimService phone is null for Subscription:" + subId);
            return false;
        }
    }

    public byte[] getUsimPsismsc() {
        return getUsimPsismscForSubscriber(getDefaultSubscription());
    }

    public byte[] getUsimPsismscForSubscriber(int subId) {
        mContext.enforceCallingOrSelfPermission(READ_PRIVILEGED_PHONE_STATE,
                "Requires READ_PRIVILEGED_PHONE_STATE");

        IccRecords iccRecords = getIccRecords(subId);
        if (iccRecords != null) {
            return iccRecords.getEfPsismsc();
        } else {
           loge("getUsimPsismsc iccRecords is null for Subscription:" + subId);
           return null;
        }
    }

    public byte[] getUsimSmsp() {
        return getUsimSmspForSubscriber(getDefaultSubscription());
    }

    public byte[] getUsimSmspForSubscriber(int subId) {
        mContext.enforceCallingOrSelfPermission(READ_PRIVILEGED_PHONE_STATE,
                "Requires READ_PRIVILEGED_PHONE_STATE");

        IccRecords iccRecords = getIccRecords(subId);
        if (iccRecords != null) {
            return iccRecords.getEfSmsp();
        } else {
           loge("getUsimSmsp iccRecords is null for Subscription:" + subId);
           return null;
        }
    }

    public int getMncLength() {
        return getMncLengthForSubscriber(getDefaultSubscription());
    }

    public int getMncLengthForSubscriber(int subId) {
        mContext.enforceCallingOrSelfPermission(READ_PRIVILEGED_PHONE_STATE,
                "Requires READ_PRIVILEGED_PHONE_STATE");

        IccRecords iccRecords = getIccRecords(subId);
        if (iccRecords != null) {
            return iccRecords.getMncLength();
        } else {
           loge("getMncLength iccRecords is null for Subscription:" + subId);
           return 0;
        }
    }
    // MTK-END

    // MTK-START: ISIM
    public String getIsimImpiForSubscriber(int subId) {
        Phone phone = getPhone(subId);
        mContext.enforceCallingOrSelfPermission(READ_PRIVILEGED_PHONE_STATE,
                "Requires READ_PRIVILEGED_PHONE_STATE");
        if (phone != null) {
            IsimRecords isim = phone.getIsimRecords();
            if (isim != null) {
                return isim.getIsimImpi();
            } else {
                return null;
            }
        } else {
           loge("getIsimImpi phone is null for Subscription:" + subId);
           return null;
        }
    }

    public String getIsimDomainForSubscriber(int subId) {
        Phone phone = getPhone(subId);
        mContext.enforceCallingOrSelfPermission(READ_PRIVILEGED_PHONE_STATE,
                "Requires READ_PRIVILEGED_PHONE_STATE");
        if (phone != null) {
            IsimRecords isim = phone.getIsimRecords();
            if (isim != null) {
                return isim.getIsimDomain();
            } else {
                return null;
            }
        } else {
           loge("getIsimDomain phone is null for Subscription:" + subId);
           return null;
        }
    }

    public String[] getIsimImpuForSubscriber(int subId) {
        Phone phone = getPhone(subId);
        mContext.enforceCallingOrSelfPermission(READ_PRIVILEGED_PHONE_STATE,
                "Requires READ_PRIVILEGED_PHONE_STATE");
        if (phone != null) {
            IsimRecords isim = phone.getIsimRecords();
            if (isim != null) {
                return isim.getIsimImpu();
            } else {
                return null;
            }
        } else {
           loge("getIsimImpu phone is null for Subscription:" + subId);
           return null;
        }
    }

    public String getIsimIstForSubscriber(int subId) {
        Phone phone = getPhone(subId);
        mContext.enforceCallingOrSelfPermission(READ_PRIVILEGED_PHONE_STATE,
                "Requires READ_PRIVILEGED_PHONE_STATE");
        if (phone != null) {
            IsimRecords isim = phone.getIsimRecords();
            if (isim != null) {
                return isim.getIsimIst();
            } else {
                return null;
            }
        } else {
           loge("getIsimIst phone is null for Subscription:" + subId);
           return null;
        }
    }

    public String[] getIsimPcscfForSubscriber(int subId) {
        Phone phone = getPhone(subId);
        mContext.enforceCallingOrSelfPermission(READ_PRIVILEGED_PHONE_STATE,
                "Requires READ_PRIVILEGED_PHONE_STATE");
        if (phone != null) {
            IsimRecords isim = phone.getIsimRecords();
            if (isim != null) {
                return isim.getIsimPcscf();
            } else {
                return null;
            }
        } else {
           loge("getIsimPcscf phone is null for Subscription:" + subId);
           return null;
        }
    }

    public byte[] getIsimPsismsc() {
        return getIsimPsismscForSubscriber(getDefaultSubscription());
    }

    public byte[] getIsimPsismscForSubscriber(int subId) {
        Phone phone = getPhone(subId);
        mContext.enforceCallingOrSelfPermission(READ_PRIVILEGED_PHONE_STATE,
                "Requires READ_PRIVILEGED_PHONE_STATE");
        if (phone != null) {
            IsimRecords isim = phone.getIsimRecords();
            if (isim != null) {
                return ((MtkIsimUiccRecords)isim).getEfPsismsc();
            } else {
                return null;
            }
        } else {
           loge("getIsimPsismsc phone is null for Subscription:" + subId);
           return null;
        }
    }
    // MTK-END

    // MTK-START: SIM GBA
    // ISIM - GBA related support START
    public String getIsimGbabp() {
        return getIsimGbabpForSubscriber(getDefaultSubscription());
    }

    public String getIsimGbabpForSubscriber(int subId) {
        Phone phone = getPhone(subId);
        mContext.enforceCallingOrSelfPermission(READ_PRIVILEGED_PHONE_STATE,
                "Requires READ_PRIVILEGED_PHONE_STATE");
        if (phone != null) {
            IsimRecords isim = phone.getIsimRecords();
            if (isim != null) {
                return ((MtkIsimUiccRecords)isim).getIsimGbabp();
            } else {
                return null;
            }
        } else {
           loge("getIsimGbabp phone is null for Subscription:" + subId);
           return null;
        }
    }

    public void setIsimGbabp(String gbabp, Message onComplete) {
        setIsimGbabpForSubscriber(getDefaultSubscription(), gbabp, onComplete);
    }

    public void setIsimGbabpForSubscriber(int subId, String gbabp, Message onComplete) {
        Phone phone = getPhone(subId);
        mContext.enforceCallingOrSelfPermission(READ_PRIVILEGED_PHONE_STATE,
                "Requires READ_PRIVILEGED_PHONE_STATE");
        if (phone != null) {
            IsimRecords isim = phone.getIsimRecords();
            if (isim != null) {
                ((MtkIsimUiccRecords)isim).setIsimGbabp(gbabp, onComplete);
            } else {
                loge("setIsimGbabp isim is null for Subscription:" + subId);
            }
        } else {
           loge("setIsimGbabp phone is null for Subscription:" + subId);
        }
    }
    // MTK-END

    // MTK-START: SIM GBA
    public String getUsimGbabp() {
        return getUsimGbabpForSubscriber(getDefaultSubscription());
    }

    public String getUsimGbabpForSubscriber(int subId) {
        mContext.enforceCallingOrSelfPermission(READ_PRIVILEGED_PHONE_STATE,
                "Requires READ_PRIVILEGED_PHONE_STATE");

        IccRecords iccRecords = getIccRecords(subId);
        if (iccRecords != null) {
            return iccRecords.getEfGbabp();
        } else {
           loge("getUsimGbabp iccRecords is null for Subscription:" + subId);
           return null;
        }
    }

    public void setUsimGbabp(String gbabp, Message onComplete) {
        setUsimGbabpForSubscriber(getDefaultSubscription(), gbabp, onComplete);
    }

    public void setUsimGbabpForSubscriber(int subId, String gbabp, Message onComplete) {
        mContext.enforceCallingOrSelfPermission(READ_PRIVILEGED_PHONE_STATE,
                "Requires READ_PRIVILEGED_PHONE_STATE");

        IccRecords iccRecords = getIccRecords(subId);
        if (iccRecords != null) {
            iccRecords.setEfGbabp(gbabp, onComplete);
        } else {
           loge("setUsimGbabp iccRecords is null for Subscription:" + subId);
        }
    }
    // ISIM - GBA related support END
    // MTK-END

    /**
     * Retrieves the phone number string for line 1 of a subcription.
     * @param subId sub id
     * @param callingPackage calling Package
     * @return String phone number
     */
    public String getLine1PhoneNumberForSubscriber(int subId, String callingPackage) {
        Phone phone = getPhone(subId);
        if (phone != null) {
            // This is open to apps with WRITE_SMS.
            if (!TelephonyPermissions.checkCallingOrSelfReadPhoneNumber(
                    mContext, subId, callingPackage, "getLine1PhoneNumber")) {
                loge("getLine1PhoneNumber permission check fail:" + subId);
                return null;
            }
            return ((MtkGsmCdmaPhone) phone).getLine1PhoneNumber();
        } else {
            loge("getLine1PhoneNumber phone is null for Subscription:" + subId);
            return null;
        }
    }
}
