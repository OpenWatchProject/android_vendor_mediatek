package  com.mediatek.gallery3d.video;
/**
 * Collection of utility functions used in this package.
 */
public class ClearMotionQualityJni {

    private ClearMotionQualityJni() {
    }

    static {
        System.loadLibrary("MJCjni");
    }

    public static native int nativeGetFallbackRange();
    public static native int nativeGetFallbackIndex();
    public static native boolean nativeSetFallbackIndex(int index);
    public static native int nativeGetDemoMode();
    public static native boolean nativeSetDemoMode(int index);
}