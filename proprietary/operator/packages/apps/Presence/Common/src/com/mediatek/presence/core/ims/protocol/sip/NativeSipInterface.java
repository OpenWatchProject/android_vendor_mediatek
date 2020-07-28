package com.mediatek.presence.core.ims.protocol.sip;

public class NativeSipInterface {

    public NativeSipInterface() {
    }

    public static native int InitVolteStack();

    public static native int DeinitVolteStack();

    public static native void VolteSipStackSendSip();

    public static native void ReadRegMsgCallbackFn();

    public static native void RuleCapabilityInit();

    public static native void RuleLevel0Set();

    public static native void RuleLevel2Set();

    public static native void VolteSipStackSipBind();

    public static native void RuleCapabilityDeinit();

    public static native void VolteSipStackRegAddCapability();

    static {
        String libname = "VolteSipStack";
        try {
            System.loadLibrary(libname);
        } catch (Exception exception) {
        }
    }
}
