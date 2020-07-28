package com.mediatek.common.voiceextension;

import android.os.ParcelFileDescriptor;
import com.mediatek.common.voiceextension.IVoiceExtCommandListener;

interface IVoiceExtCommandManager {

    int createCommandSet(String name);

    int isCommandSetCreated(String name);

    String getCommandSetSelected();

    int deleteCommandSet(String name);

    int selectCurrentCommandSet(String name);

    void setCommandsStrArray(in String[] commands);

    void setCommandsFile(in ParcelFileDescriptor pFd, int offset, int length);

    int registerListener(IVoiceExtCommandListener listener);

    String[] getCommands();

    String[] getCommandSets();

    void startRecognition();

    void stopRecognition();

    void pauseRecognition();

    void resumeRecognition();

}