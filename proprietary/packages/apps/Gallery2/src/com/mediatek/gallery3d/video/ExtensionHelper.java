
package com.mediatek.gallery3d.video;

import android.content.Context;

import com.mediatek.gallery3d.ext.IActivityHooker;
import com.mediatek.gallery3d.util.Log;

import java.util.ArrayList;
import java.util.List;

public class ExtensionHelper {
    private static final String TAG = "VP_ExtensionHelper";
    private static final boolean LOG = true;

    /// M: [FEATURE.ADD] Mutil-window @{
    private static String sContextString;
    /// @}

    public static IActivityHooker getHooker(final Context context) {
        final ActivityHookerGroup group = new ActivityHookerGroup();
        group.addHooker(new StopVideoHooker()); // add it for common feature.
        group.addHooker(new LoopVideoHooker()); // add it for common feature.
        group.addHooker(new TrimVideoHooker());
        group.addHooker(new NfcHooker());
        group.addHooker(new VideoTitleHooker());

        if (MtkVideoFeature.isClearMotionMenuEnabled(context)) {
            group.addHooker(new ClearMotionHooker());
        }

        // / M: [FEATURE.ADD] CTA @ {
        if (MtkVideoFeature.isSupportCTA()) {
            group.addHooker(new CTAHooker());
        }
        // / @}

        for (int i = 0, count = group.size(); i < count; i++) {
            if (LOG) {
                Log.v(TAG, "getHooker() [" + i + "]=" + group.getHooker(i));
            }
        }
        /// M: [FEATURE.ADD] Mutil-window @{
        sContextString = context.toString();
        /// @}
        return group;
    }

    private static IMovieDrmExtension sMovieDrmExtension;

    public static IMovieDrmExtension getMovieDrmExtension(final Context context) {
        if (sMovieDrmExtension == null) {
            /*
             * try { sMovieDrmExtension = (IMovieDrmExtension)
             * PluginManager.createPluginObject(
             * context.getApplicationContext(),
             * IMovieDrmExtension.class.getName()); } catch
             * (Plugin.ObjectCreationException e) { sMovieDrmExtension = new
             * MovieDrmExtension(); }
             */
            // should be modified for common feature
            if (MtkVideoFeature.isOmaDrmSupported()) {
                sMovieDrmExtension = new MovieDrmExtensionImpl();
            } else {
                sMovieDrmExtension = new DefaultMovieDrmExtension();
            }
        }
        return sMovieDrmExtension;
    }
}
