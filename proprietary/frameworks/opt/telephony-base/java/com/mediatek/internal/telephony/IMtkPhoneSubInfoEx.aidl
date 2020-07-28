/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2017. All rights reserved.
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

// MTK-START: SIM GBA
import android.os.Message;
// MTK-END

/**
 * Interface used to retrieve various phone-related subscriber information.
 *
 */
interface IMtkPhoneSubInfoEx {
    //MTK-START: SIM
    /**
     * Returns the USIM Service Table (UST) that was loaded from the USIM.
     * @param service service index on UST
     * @return  the indicated service is supported or not
     */
    boolean getUsimService(int service, String callingPackage);

    //MTK-START: SIM GBA
    /**
     * Returns the GBA bootstrapping parameters (GBABP) that was loaded from the USIM.
     * @return GBA bootstrapping parameters or null if not present or not loaded
     */
    String getUsimGbabp();

    /**
     * Returns the GBA bootstrapping parameters (GBABP) that was loaded from the USIM for particular subId.
     * @param subId subscription ID to be queried
     * @return GBA bootstrapping parameters or null if not present or not loaded
     */
    String getUsimGbabpForSubscriber(int subId);

    /**
     * Set the GBA bootstrapping parameters (GBABP) value into the USIM.
     * @param gbabp a GBA bootstrapping parameters value in String type
     * @param onComplete
     *        onComplete.obj will be an AsyncResult
     *        ((AsyncResult)onComplete.obj).exception == null on success
     *        ((AsyncResult)onComplete.obj).exception != null on fail
     */
    void setUsimGbabp(String gbabp, in Message onComplete);

    /**
     * Set the GBA bootstrapping parameters (GBABP) value into the USIM for particular subId.
     * @param subId subscription ID to be queried
     * @param gbabp a GBA bootstrapping parameters value in String type
     * @param onComplete
     *        onComplete.obj will be an AsyncResult
     *        ((AsyncResult)onComplete.obj).exception == null on success
     *        ((AsyncResult)onComplete.obj).exception != null on fail
     */
    void setUsimGbabpForSubscriber(int subId, String gbabp, in Message onComplete);
    //MTK-END

    // MTK-START: SIM GBA
    /**
     * Returns the GBA bootstrapping parameters (GBABP) that was loaded from the ISIM.
     * @return GBA bootstrapping parameters or null if not present or not loaded
     */
    String getIsimGbabp();

    /**
     * Returns the GBA bootstrapping parameters (GBABP) that was loaded from the ISIM for particular subId.
     * @param subId subscription ID to be queried
     * @return GBA bootstrapping parameters or null if not present or not loaded
     */
    String getIsimGbabpForSubscriber(int subId);

    /**
     * Set the GBA bootstrapping parameters (GBABP) value into the ISIM.
     * @param gbabp a GBA bootstrapping parameters value in String type
     * @param onComplete
     *        onComplete.obj will be an AsyncResult
     *        ((AsyncResult)onComplete.obj).exception == null on success
     *        ((AsyncResult)onComplete.obj).exception != null on fail
     */
    void setIsimGbabp(String gbabp, in Message onComplete);

    /**
     * Set the GBA bootstrapping parameters (GBABP) value into the ISIM for particular subId.
     * @param subId subscription ID to be queried
     * @param gbabp a GBA bootstrapping parameters value in String type
     * @param onComplete
     *        onComplete.obj will be an AsyncResult
     *        ((AsyncResult)onComplete.obj).exception == null on success
     *        ((AsyncResult)onComplete.obj).exception != null on fail
     */
    void setIsimGbabpForSubscriber(int subId, String gbabp, in Message onComplete);
    //MTK-END

    /**
     * Returns the USIM Service Table (UST) that was loaded from the USIM for particular subId.
     * @param subId subscription ID to be queried
     * @param service service index on UST
     * @return  the indicated service is supported or not
     */
    boolean getUsimServiceForSubscriber(int subId, int service, String callingPackage);

    /**
     * Returns the Public Service Identity of the SM-SC (PSISMSC) that was loaded from the USIM.
     * @return PSISMSC or null if not present or not loaded
     */
    byte[] getUsimPsismsc();

    /**
     * Returns the Public Service Identity of the SM-SC (PSISMSC) that was loaded from the USIM for particular subId.
     * @param subId subscription ID to be queried
     * @return PSISMSC or null if not present or not loaded
     */
    byte[] getUsimPsismscForSubscriber(int subId);

    /**
     * Returns the Short message parameter (SMSP) that was loaded from the USIM.
     * @return SMSP or null if not present or not loaded
     */
    byte[] getUsimSmsp();

    /**
     * Returns the Short message parameter (SMSP) that was loaded from the USIM for particular subId.
     * @param subId subscription ID to be queried
     * @return SMSP or null if not present or not loaded
     */
    byte[] getUsimSmspForSubscriber(int subId);

    /**
     * Returns the MCC+MNC length that was loaded from the USIM.
     * @return MCC+MNC length or 0 if not present or not loaded
     */
    int getMncLength();

    /**
     * Returns the MCC+MNC length that was loaded from the USIM for particular subId.
     * @param subId subscription ID to be queried
     * @return MCC+MNC length or 0 if not present or not loaded
     */
    int getMncLengthForSubscriber(int subId);
     // MTK-END

    //MTK-START: ISIM
    /**
     * Returns the IMS private user identity (IMPI) that was loaded from the ISIM for particular subId.
     * @param subId subscription ID to be queried
     * @return the IMPI, or null if not present or not loaded
     */
    String getIsimImpiForSubscriber(int subId);

    /**
     * Returns the IMS home network domain name that was loaded from the ISIM for particular subId.
     * @param subId subscription ID to be queried
     * @return the IMS domain name, or null if not present or not loaded
     */
    String getIsimDomainForSubscriber(int subId);

    /**
     * Returns the IMS public user identities (IMPU) that were loaded from the ISIM for particular subId.
     * @param subId subscription ID to be queried
     * @return an array of IMPU strings, with one IMPU per string, or null if
     *      not present or not loaded
     */
    String[] getIsimImpuForSubscriber(int subId);

    /**
     * Returns the IMS Service Table (IST) that was loaded from the ISIM for particular subId.
     * @param subId subscription ID to be queried
     * @return IMS Service Table or null if not present or not loaded
     */
    String getIsimIstForSubscriber(int subId);

    /**
     * Returns the IMS Proxy Call Session Control Function(PCSCF) that were loaded from the ISIM for particular subId.
     * @param subId subscription ID to be queried
     * @return an array of PCSCF strings with one PCSCF per string, or null if
     *      not present or not loaded
     */
    String[] getIsimPcscfForSubscriber(int subId);

    /**
     * Returns the Public Service Identity of the SM-SC (PSISMSC) that was loaded from the ISIM.
     * @return PSISMSC or null if not present or not loaded
     */
    byte[] getIsimPsismsc();

    /**
     * Returns the Public Service Identity of the SM-SC (PSISMSC) that was loaded from the ISIM for particular subId.
     * @param subId subscription ID to be queried
     * @return PSISMSC or null if not present or not loaded
     */
    byte[] getIsimPsismscForSubscriber(int subId);
    //MTK-END

    /**
     * Retrieves the phone number string for line 1 of a subcription.
     */
    String getLine1PhoneNumberForSubscriber(int subId, String callingPackage);
}
