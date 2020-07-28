package com.mediatek.common.voicecommand;

import com.mediatek.common.voicecommand.IVoiceWakeupInteractionCallback;

interface IVoiceWakeupInteractionService {
    /**
     Register VoiceWakeupInteractionService callback.
    */
    void registerCallback(IVoiceWakeupInteractionCallback callback);

    /**
     Start recognition.
    */
    boolean startRecognition();

    /**
     Stop recognition.
    */
    boolean stopRecognition();

    /**
     On user switch, handle for current user.
    */
     void setCurrentUserOnSwitch(int userId);
}
