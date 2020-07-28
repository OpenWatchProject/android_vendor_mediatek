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
 * MediaTek Inc. (C) 2014. All rights reserved.
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

package com.mediatek.voicecommand.vis;

import java.io.ByteArrayOutputStream;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.IOException;
import java.util.UUID;

import android.content.Context;
import android.provider.Settings;
import android.hardware.soundtrigger.SoundTrigger;
import android.hardware.soundtrigger.SoundTrigger.Keyphrase;
import android.hardware.soundtrigger.SoundTrigger.KeyphraseSoundModel;
import android.os.RemoteException;
import android.os.ServiceManager;

import com.android.internal.app.IVoiceInteractionManagerService;
import com.mediatek.provider.MtkSettingsExt;
import com.mediatek.voicecommand.vis.Log;

/**
 * Utility class for the enrollment operations like enroll; un-enroll.
 */
public class VoiceTrainingEnrollmentUtil {
    private static final String TAG = "VoiceTrainingEnrollmentUtil";

    public static final String VOICE_WAKEUP_COMMAND_STATUS =
            MtkSettingsExt.System.VOICE_WAKEUP_COMMAND_STATUS;
    public static final String VOICE_WAKEUP_MODE = MtkSettingsExt.System.VOICE_WAKEUP_MODE;


    // Keyphrase related constants, must match those defined in enrollment_application.xml.
    public static final int KEYPHRASE_ID = 101;
    public static final int RECOGNITION_MODES = SoundTrigger.RECOGNITION_MODE_VOICE_TRIGGER;
    //Using "MTKINC" as the Model.
    private static final String SOUND_MODEL_LABEL = "MTKINC";

    private final IVoiceInteractionManagerService mModelManagementService;

    /**
     * Utility class to get model management service.
     */
    public VoiceTrainingEnrollmentUtil() {
        mModelManagementService = IVoiceInteractionManagerService.Stub.asInterface(
                ServiceManager.getService(Context.VOICE_INTERACTION_MANAGER_SERVICE));
    }

    /**
     * Adds/Updates a sound model.
     * The sound model must contain a valid UUID,
     * exactly 1 keyphrase,
     * and users for which the keyphrase is valid - typically the current user.
     *
     * @param soundModel The sound model to add/update.
     * @return {@code true} if the call succeeds, {@code false} otherwise.
     */
    public boolean addOrUpdateSoundModel(KeyphraseSoundModel soundModel) {
        if (!verifyKeyphraseSoundModel(soundModel)) {
            return false;
        }

        int status = SoundTrigger.STATUS_ERROR;
        try {
            status = mModelManagementService.updateKeyphraseSoundModel(soundModel);
        } catch (RemoteException e) {
            Log.e(TAG, "[addOrUpdateSoundModel] exception: " + e.getMessage());
        }
        return status == SoundTrigger.STATUS_OK;
    }

    /**
     * Gets the sound model for the given keyphrase, null if none exists.
     * This should be used for re-enrollment purposes.
     * If a sound model for a given keyphrase exists, and it needs to be updated,
     * it should be obtained using this method, updated and then passed in to
     * {@link #addOrUpdateSoundModel(KeyphraseSoundModel)} without changing the IDs.
     *
     * @param keyphraseId The keyphrase ID to look-up the sound model for.
     * @param bcp47Locale The locale for with to look up the sound model for.
     * @return The sound model if one was found, null otherwise.
     */
    public KeyphraseSoundModel getSoundModel(int keyphraseId, String bcp47Locale) {
        if (keyphraseId <= 0) {
            Log.e(TAG, "[getSoundModel] Keyphrase must have a valid ID");
            return null;
        }

        KeyphraseSoundModel model = null;
        try {
            model = mModelManagementService.getKeyphraseSoundModel(keyphraseId, bcp47Locale);
        } catch (RemoteException e) {
            Log.e(TAG, "[getSoundModel] exception: " + e.getMessage());
        }

        if (model == null) {
            Log.w(TAG, "[getSoundModel] No models present for the gien keyphrase ID");
            return null;
        } else {
            return model;
        }
    }

    /**
     * Deletes the sound model for the given keyphrase id.
     *
     * @param keyphraseId The keyphrase ID to look-up the sound model for.
     * @param bcp47Locale The locale to look-up the sound model for.
     * @return {@code true} if the call succeeds, {@code false} otherwise.
     */
    public boolean deleteSoundModel(int keyphraseId, String bcp47Locale) {
        if (keyphraseId <= 0) {
            Log.e(TAG, "[deleteSoundModel] Keyphrase must have a valid ID");
            return false;
        }

        int status = SoundTrigger.STATUS_ERROR;
        try {
            status = mModelManagementService.deleteKeyphraseSoundModel(keyphraseId, bcp47Locale);
        } catch (RemoteException e) {
            Log.e(TAG, "[deleteSoundModel] exception: " + e.getMessage());
        }
        return status == SoundTrigger.STATUS_OK;
    }

    private boolean verifyKeyphraseSoundModel(KeyphraseSoundModel soundModel) {
        if (soundModel == null) {
            Log.e(TAG, "[verifyKeyphraseSoundModel] KeyphraseSoundModel must be non-null");
            return false;
        }
        if (soundModel.uuid == null) {
            Log.e(TAG, "[verifyKeyphraseSoundModel] KeyphraseSoundModel must have a UUID");
            return false;
        }
        if (soundModel.data == null) {
            Log.e(TAG, "[verifyKeyphraseSoundModel] KeyphraseSoundModel must have data");
            return false;
        }
        if (soundModel.keyphrases == null || soundModel.keyphrases.length != 1) {
            Log.e(TAG, "[verifyKeyphraseSoundModel] Keyphrase must be exactly 1");
            return false;
        }
        Keyphrase keyphrase = soundModel.keyphrases[0];
        if (keyphrase.id <= 0) {
            Log.e(TAG, "[verifyKeyphraseSoundModel] Keyphrase must have a valid ID");
            return false;
        }
        if (keyphrase.recognitionModes < 0) {
            Log.e(TAG, "[verifyKeyphraseSoundModel] Recognition modes must be valid");
            return false;
        }
        if (keyphrase.locale == null) {
            Log.e(TAG, "[verifyKeyphraseSoundModel] Locale must not be null");
            return false;
        }
        if (keyphrase.text == null) {
            Log.e(TAG, "[verifyKeyphraseSoundModel] Text must not be null");
            return false;
        }
        if (keyphrase.users == null || keyphrase.users.length == 0) {
            Log.e(TAG, "[verifyKeyphraseSoundModel] Keyphrase must have valid user(s)");
            return false;
        }
        return true;
    }

    /**
     * Gets the sound model byte[] data for the file path, null if none exists.
     * This should be used for enrollment purposes.
     *
     * @param filePath The file path of the sound model.
     * @return The sound model byte[] data.
     */
    public static byte[] getBytes(String filePath) {
        byte[] buffer = null;
        try {
            File file = new File(filePath);
            FileInputStream fis = new FileInputStream(file);
            ByteArrayOutputStream bos = new ByteArrayOutputStream(1000);
            byte[] b = new byte[1000];
            int n;
            while ((n = fis.read(b)) != -1) {
                bos.write(b, 0, n);
            }
            fis.close();
            bos.close();
            buffer = bos.toByteArray();
        } catch (FileNotFoundException e) {
            e.printStackTrace();
        } catch (IOException e) {
            e.printStackTrace();
        }
        return buffer;
    }

    /**
     * Gets the Long form for String input.
     * @param leastSigBits String parameter for converting to long.
     * @return long form of the string for leastSigBits.
     */
    public long stringToLong(String leastSigBits) {
        String str = "";
        for (int i = 0; i < leastSigBits.length(); i++) {
            int ch = (int) leastSigBits.charAt(i);
            String s = Integer.toHexString(ch);
            str = str + s;
        }
        // Use the HEX format radix to match sound model requirement
        return Long.parseLong(str, 16);
    }

    /**
     * Gets the UUID for the sound model.
     * Configuration utility UUID, agree with native owner:
     * The 0 bit set training mode: 0 represents speaker independent mode,
     *                              1 represents speaker dependent mode.
     * The 1~2 bits set command id: 00 represents commandId = 0,
     *                              01 represents commandId = 1,
     *                              10 represents commandId = 2.
     * @param trainingMode The training mode.
     * @param commandId The command id.
     * @return The UUID for the sound model.
     */
    public UUID getUUID(int trainingMode, int commandId) {
        long mostSigBits = (trainingMode - 1) | commandId << 1;
        return new UUID(mostSigBits, stringToLong(SOUND_MODEL_LABEL));
    }

    /**
     * Query wakeup mode from setting provider.
     *
     * @param context
     *            context
     * @return wakeup mode
     */
    public static int getWakeupMode(Context context) {
        int mode = Settings.System.getInt(context.getContentResolver(), VOICE_WAKEUP_MODE, 1);
        Log.i(TAG, "[getWakeupMode]mode : " + mode);

        return mode;
    }

    /**
     * Query wakeup command status from setting provider.
     *
     * @param context
     *            context
     * @return wakeup command status
     */
    public static int getWakeupCmdStatus(Context context) {
        int cmdStatus = Settings.System.getInt(context.getContentResolver(),
                VOICE_WAKEUP_COMMAND_STATUS, 0);
        Log.i(TAG, "[getWakeupCmdStatus]cmdStatus : " + cmdStatus);

        return cmdStatus;
    }
}
