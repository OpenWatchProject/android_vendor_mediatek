package com.mediatek.common.voicecommand;

interface IVoiceWakeupInteractionCallback {
    /**
     Callback from VoiceWakeupInteractionService for recognition result
    */
    void onVoiceWakeupDetected(int commandId);
}
