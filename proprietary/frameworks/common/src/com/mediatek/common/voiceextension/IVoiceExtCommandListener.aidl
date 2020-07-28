package com.mediatek.common.voiceextension;

interface IVoiceExtCommandListener {

   /**
    Callback from Voice Command Service for start result
    */
    void onStartRecognition(int retCode);

   /**
    Callback from Voice Command Service for stopped result
    */
    void onStopRecognition(int retCode);

   /**
    Callback from Voice Command Service for paused result
    */    
    void onPauseRecognition(int retCode);

   /**
    Callback from Voice Command Service for resumed result
    */    
    void onResumeRecognition(int retCode);

   /**
    Callback from Voice Command Service for recognized result
    */      
    void onCommandRecognized(int retCode, int commandId , String commandStr);

   /**
    Callback from Voice Command Service when error happened
    */    
    void onError(int retCode);

    /**
    Callback from Voice Command Service for commands setting result
    */      
    void onSetCommands(int retCode);

}