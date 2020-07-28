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

import android.app.Service;
import android.content.Intent;
import android.os.IBinder;

import android.hardware.soundtrigger.SoundTrigger.Keyphrase;
import android.hardware.soundtrigger.SoundTrigger.KeyphraseSoundModel;

import com.mediatek.common.voicecommand.IVoiceTrainingEnrollmentService;
import com.mediatek.common.voicecommand.VoiceCommandListener;
import com.mediatek.voicecommand.vis.Log;

import java.util.UUID;

/**
 * Service implementation for VoiceWakeup.
 */
public class VoiceTrainingEnrollmentService extends Service {
    private static final String TAG = "VoiceTrainingEnrollmentService";
    public static final String VOICE_TRAINING_SERVICE_ACTION =
        "com.mediatek.intent.action.bindEnrollmentService";

    private VoiceTrainingEnrollmentUtil mEnrollmentUtil = null;

    @Override
    public IBinder onBind(Intent intent) {
        Log.d(TAG, "onBind");
        Log.d(TAG, "Intent action : " + intent.getAction());
        if (VOICE_TRAINING_SERVICE_ACTION.equals(intent.getAction())) {
            return mBinder.asBinder();
        } else {
            return null;
        }
    }

    @Override
    public void onCreate() {
        Log.d(TAG, "onCreate");
        super.onCreate();
        if (mEnrollmentUtil == null) {
            mEnrollmentUtil = new VoiceTrainingEnrollmentUtil();
        }
    }

    @Override
    public int onStartCommand(Intent intent, int flags, int startId) {
        Log.d(TAG, "onStartCommand ");
        return START_NOT_STICKY;
    }


    /**
     * It is used to implement the VoiceWakeupInteractionService public methods.
     */
    private IVoiceTrainingEnrollmentService.Stub mBinder =
            new IVoiceTrainingEnrollmentService.Stub() {

    @Override
        /**
         * Called when the user training success. Performs a fresh enrollment.
         */
        public boolean enrollSoundModel(int trainingMode, int commandId,
            String patternPath, int user) {
            Log.d(TAG, "[enrollSoundModel], trainingMode = " + trainingMode +
                 ", commandId = " + commandId + ", patternPath = " + patternPath +
                 ", user = " + user);
            if (mEnrollmentUtil == null) { /*Init on reference*/
                mEnrollmentUtil = new VoiceTrainingEnrollmentUtil();
            }

            Keyphrase kp = new Keyphrase(VoiceTrainingEnrollmentUtil.KEYPHRASE_ID,
                    VoiceTrainingEnrollmentUtil.RECOGNITION_MODES,
                    VoiceCommandListener.VOW_ENROLLMENT_BCP47_LOCALE,
                    VoiceCommandListener.VOW_ENROLLMENT_TEXT,
                    new int[] {user});
            UUID modelUuid = mEnrollmentUtil.getUUID(trainingMode, commandId);
            byte[] data = mEnrollmentUtil.getBytes(patternPath + commandId + ".dat");
            KeyphraseSoundModel soundModel = new KeyphraseSoundModel(modelUuid, null, data,
                    new Keyphrase[] { kp });
            boolean status = mEnrollmentUtil.addOrUpdateSoundModel(soundModel);
            if (!status) {
           //Toast.makeText(mContext, "Failed to enroll!!" + modelUuid, Toast.LENGTH_SHORT).show();
                Log.d(TAG, "[enrollSoundModel] Failed to enroll!!! model: " + modelUuid
                   + ", status =" + status);
            } else {
                Log.d(TAG, "[enrollSoundModel] Successfully = " + status + ", model UUID = "
                   + modelUuid + ", data.length = " + data.length);
            }
            return status;
        }

        @Override
        /**
         * Called when the training reset. Clears the enrollment information for the user.
         */
        public boolean unEnrollSoundModel() {
            boolean status = false;
            if (mEnrollmentUtil == null) {
                mEnrollmentUtil = new VoiceTrainingEnrollmentUtil();
            }

            KeyphraseSoundModel soundModel = mEnrollmentUtil.getSoundModel(
              VoiceTrainingEnrollmentUtil.KEYPHRASE_ID,
              VoiceCommandListener.VOW_ENROLLMENT_BCP47_LOCALE);
            if (soundModel == null) {
                //Toast.makeText(mContext, "Sound model not found!!!", Toast.LENGTH_SHORT).show();
                Log.e(TAG, "[unEnrollSoundModel] Sound model not found!!!");
                return status;
            }
            status = mEnrollmentUtil.deleteSoundModel(VoiceTrainingEnrollmentUtil.KEYPHRASE_ID,
                    VoiceCommandListener.VOW_ENROLLMENT_BCP47_LOCALE);
            Log.d(TAG, "[unEnrollSoundModel] Successfully = " + status + ", model UUID="
                    + soundModel.uuid);
            if (!status) {
                //Toast.makeText(mContext, "Failed to un-enroll!!!", Toast.LENGTH_SHORT).show();
                Log.d(TAG, "[unEnrollSoundModel] Failed to un-enroll!!!");
            }
            return status;
        }
    };

    @Override
    public void onDestroy() {
        Log.d(TAG, "onDestroy");
        mEnrollmentUtil = null;
        super.onDestroy();
    }
}
