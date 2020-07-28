package com.mediatek.camera.feature.mode.hdr;

import android.app.Activity;

import com.mediatek.camera.common.ICameraContext;
import com.mediatek.camera.common.device.CameraDeviceManagerFactory;

import javax.annotation.Nonnull;

/**
 * Created by vend_wsd_am_029 on 2019/5/28.
 */

public class HdrControllerFactory {

    /**
     * Create the instance of {@link IHdrDeviceController} by API type.
     *
     * @param activity  the camera activity.
     * @param cameraApi the type of IDeviceController.
     * @param context   the camera context.
     * @return an instance of IDeviceController.
     */
    @Nonnull
    public IHdrDeviceController createDeviceController(
            @Nonnull Activity activity,
            @Nonnull CameraDeviceManagerFactory.CameraApi cameraApi,
            @Nonnull ICameraContext context) {
        return new HdrDevice2Controller(activity, context);
    }
}
