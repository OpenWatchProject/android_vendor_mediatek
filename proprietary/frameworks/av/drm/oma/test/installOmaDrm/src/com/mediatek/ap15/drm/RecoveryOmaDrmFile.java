package com.mediatek.ap15.drm;

import java.io.FileDescriptor;

public class RecoveryOmaDrmFile {

    static {
        // Load the library
        //System.load("/data/data/com.mediatek.ap15.drm/lib/librecoveryomadrm_jni.so");
        System.loadLibrary("recoveryomadrm_jni");
    }
    /**
     * return 1 indicates success
     * return 0 indicates fail
     */
    public static native int recoveryOmaDrmFile(String deviceId, String path, FileDescriptor fdOut);
}
