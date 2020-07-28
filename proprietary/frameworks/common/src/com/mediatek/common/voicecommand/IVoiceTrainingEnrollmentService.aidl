package com.mediatek.common.voicecommand;

interface IVoiceTrainingEnrollmentService {
    /**
     Called when the user training success. Performs a fresh enrollment.
    */
    boolean enrollSoundModel(int traningMode, int commandId,
        String patternPath, int user);


    /**
     Called when the training reset. Clears the enrollment information for the user.
    */
    boolean unEnrollSoundModel();
}
