/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein is
 * confidential and proprietary to MediaTek Inc. and/or its licensors. Without
 * the prior written permission of MediaTek inc. and/or its licensors, any
 * reproduction, modification, use or disclosure of MediaTek Software, and
 * information contained herein, in whole or in part, shall be strictly
 * prohibited.
 *
 * MediaTek Inc. (C) 2014. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
 * ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 * NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH
 * RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 * INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES
 * TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
 * RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
 * OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN MEDIATEK
 * SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE
 * RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S
 * ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE
 * RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE
 * MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE
 * CHARGE PAID BY RECEIVER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek
 * Software") have been modified by MediaTek Inc. All revisions are subject to
 * any receiver's applicable license agreements with MediaTek Inc.
 */

package com.mediatek.camera.v2.control;

import android.graphics.PointF;
import android.graphics.Rect;
import android.hardware.camera2.params.MeteringRectangle;

public class ControlHelper {

    public static final MeteringRectangle[] ZERO_WEIGHT_3A_REGION = new MeteringRectangle[]{
        new MeteringRectangle(0, 0, 0, 0, 0)
    };

    /**
     * Width of touch AF region in [0,1] relative to shorter edge of the current
     * crop region. Multiply this number by the number of pixels along the
     * shorter edge of the current crop region's width to get a value in pixels.
     *
     * <p>
     * This value has been tested on Nexus 5 and Shamu, but will need to be
     * tuned per device depending on how its ISP interprets the metering box and weight.
     * </p>
     *
     * <p>
     * Values prior to L release:
     * Normal mode: 0.125 * longest edge
     * Gcam: Fixed at 300px x 300px.
     * </p>
     */
    private static final float AF_REGION_BOX = 0.2f;

    /**
     * Width of touch metering region in [0,1] relative to shorter edge of the
     * current crop region. Multiply this number by the number of pixels along
     * shorter edge of the current crop region's width to get a value in pixels.
     *
     * <p>
     * This value has been tested on Nexus 5 and Shamu, but will need to be
     * tuned per device depending on how its ISP interprets the metering box and weight.
     * </p>
     *
     * <p>
     * Values prior to L release:
     * Normal mode: 0.1875 * longest edge
     * Gcam: Fixed at 300px x 300px.
     * </p>
     */
    private static final float AE_REGION_BOX = 0.3f;
    /** Metering region weight between 0 and 1.
    *
    * <p>
    * This value has been tested on Nexus 5 and Shamu, but will need to be
    * tuned per device depending on how its ISP interprets the metering box and weight.
    * </p>
    */
   private static final float REGION_WEIGHT = 0.022f;
   /** camera2 API metering region weight. */
   private static final int CAMERA2_REGION_WEIGHT = (int)
        (lerp(MeteringRectangle.METERING_WEIGHT_MIN, MeteringRectangle.METERING_WEIGHT_MAX,
                REGION_WEIGHT));

    /** Compute 3A regions for a sensor-referenced touch coordinate.
     * Returns a MeteringRectangle[] with length 1.
     *
     * @param nx x coordinate of the touch point, in normalized portrait coordinates.
     * @param ny y coordinate of the touch point, in normalized portrait coordinates.
     * @param fraction Fraction in [0,1]. Multiplied by min(cropRegion.width(), cropRegion.height())
     *             to determine the side length of the square MeteringRectangle.
     * @param cropRegion Crop region of the image.
     * @param sensorOrientation sensor orientation as defined by
     *             CameraCharacteristics.get(CameraCharacteristics.SENSOR_ORIENTATION).
     */
    public static MeteringRectangle[] regionsForNormalizedCoord(float nx, float ny,
        float fraction, final Rect cropRegion, int sensorOrientation) {
        // Compute half side length in pixels.
        int minCropEdge = Math.min(cropRegion.width(), cropRegion.height());
        int halfSideLength = (int) (0.5f * fraction * minCropEdge);

        // Compute the output MeteringRectangle in sensor space.
        // nx, ny is normalized to the screen.
        // Crop region itself is specified in sensor coordinates.

        // Normalized coordinates, now rotated into sensor space.
        PointF nsc = normalizedSensorCoordsForNormalizedDisplayCoords(
            nx, ny, sensorOrientation);

        int xCenterSensor = (int) (cropRegion.left + nsc.x * cropRegion.width());
        int yCenterSensor = (int) (cropRegion.top + nsc.y * cropRegion.height());

        Rect meteringRegion = new Rect(xCenterSensor - halfSideLength,
            yCenterSensor - halfSideLength,
            xCenterSensor + halfSideLength,
            yCenterSensor + halfSideLength);

        // Clamp meteringRegion to cropRegion.
        meteringRegion.left = clamp(meteringRegion.left, cropRegion.left, cropRegion.right);
        meteringRegion.top = clamp(meteringRegion.top, cropRegion.top, cropRegion.bottom);
        meteringRegion.right = clamp(meteringRegion.right, cropRegion.left, cropRegion.right);
        meteringRegion.bottom = clamp(meteringRegion.bottom, cropRegion.top, cropRegion.bottom);

        return new MeteringRectangle[]{new MeteringRectangle(meteringRegion,
                CAMERA2_REGION_WEIGHT)};
    }

    /**
     * Return AF region(s) for a sensor-referenced touch coordinate.
     *
     * <p>
     * Normalized coordinates are referenced to portrait preview window with
     * (0, 0) top left and (1, 1) bottom right. Rotation has no effect.
     * </p>
     *
     * @return AF region(s).
     */
    public static MeteringRectangle[] afRegionsForNormalizedCoord(float nx,
        float ny, final Rect cropRegion, int sensorOrientation) {
        return regionsForNormalizedCoord(nx, ny, AF_REGION_BOX,
            cropRegion, sensorOrientation);
    }

    /**
     * Return AE region(s) for a sensor-referenced touch coordinate.
     *
     * <p>
     * Normalized coordinates are referenced to portrait preview window with
     * (0, 0) top left and (1, 1) bottom right. Rotation has no effect.
     * </p>
     *
     * @return AE region(s).
     */
    public static MeteringRectangle[] aeRegionsForNormalizedCoord(float nx,
        float ny, final Rect cropRegion, int sensorOrientation) {
        return regionsForNormalizedCoord(nx, ny, AE_REGION_BOX,
            cropRegion, sensorOrientation);
    }

    /**
     * Clamps x to between min and max (inclusive on both ends, x = min --> min,
     * x = max --> max).
     */
    public static int clamp(int x, int min, int max) {
        if (x > max) {
            return max;
        }
        if (x < min) {
            return min;
        }
        return x;
    }

    /**
     * Clamps x to between min and max (inclusive on both ends, x = min --> min,
     * x = max --> max).
     */
    public static float clamp(float x, float min, float max) {
        if (x > max) {
            return max;
        }
        if (x < min) {
            return min;
        }
        return x;
    }
    /**
     * Linear interpolation between a and b by the fraction t. t = 0 --> a, t =
     * 1 --> b.
     */
    public static float lerp(float a, float b, float t) {
        return a + t * (b - a);
    }

    /**
     * Given (nx, ny) \in [0, 1]^2, in the display's portrait coordinate system,
     * returns normalized sensor coordinates \in [0, 1]^2 depending on how
     * the sensor's orientation \in {0, 90, 180, 270}.
     *
     * <p>
     * Returns normalized sensor coordinates \in [0, 1]^2 depending on the sensor's orientation in
     * 0, if sensorOrientation is not one of the above.
     * </p>
     */
    public static PointF normalizedSensorCoordsForNormalizedDisplayCoords(
        float nx, float ny, int sensorOrientation) {
        switch (sensorOrientation) {
        case 0:
            return new PointF(nx, ny);
        case 90:
            return new PointF(ny, 1.0f - nx);
        case 180:
            return new PointF(1.0f - nx, 1.0f - ny);
        case 270:
            return new PointF(1.0f - ny, nx);
        default:
            return new PointF(nx, ny);
        }
    }
}
