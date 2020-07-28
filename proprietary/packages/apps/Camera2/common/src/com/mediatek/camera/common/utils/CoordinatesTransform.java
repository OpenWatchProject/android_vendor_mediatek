/*
 *   Copyright Statement:
 *
 *     This software/firmware and related documentation ("MediaTek Software") are
 *     protected under relevant copyright laws. The information contained herein is
 *     confidential and proprietary to MediaTek Inc. and/or its licensors. Without
 *     the prior written permission of MediaTek inc. and/or its licensors, any
 *     reproduction, modification, use or disclosure of MediaTek Software, and
 *     information contained herein, in whole or in part, shall be strictly
 *     prohibited.
 *
 *     MediaTek Inc. (C) 2016. All rights reserved.
 *
 *     BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 *    THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 *     RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
 *     ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
 *     WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 *     WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 *     NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH
 *     RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 *     INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES
 *     TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
 *     RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
 *     OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN MEDIATEK
 *     SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE
 *     RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 *     STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S
 *     ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE
 *     RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE
 *     MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE
 *     CHARGE PAID BY RECEIVER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 *     The following software/firmware and/or related documentation ("MediaTek
 *     Software") have been modified by MediaTek Inc. All revisions are subject to
 *     any receiver's applicable license agreements with MediaTek Inc.
 */


package com.mediatek.camera.common.utils;

import android.annotation.TargetApi;
import android.graphics.Matrix;
import android.graphics.Point;
import android.graphics.PointF;
import android.graphics.Rect;
import android.graphics.RectF;
import android.hardware.camera2.CameraCharacteristics;
import android.os.Build;

import com.mediatek.camera.common.debug.LogHelper;
import com.mediatek.camera.common.debug.LogUtil;

/**
 * This is for coordinates transform.
 * NormalizedPreview(-1000, 1000). This is camera device coordinate that API1 used to do
 * coordinate transform from UI coordinate to camera device coordinate. It is the bridging
 * coordinate between UI coordinate and sensor coordinate.
 */

public class CoordinatesTransform {

    private static final LogUtil.Tag TAG = new LogUtil.Tag(
            CoordinatesTransform.class.getSimpleName());
    private static boolean sIsDebugMode = false;
    /**
     * Transform UI coordinate to NormalizedPreview(-1000,1000).
     * @param p the touch point.
     * @param previewRect The preview layout rect.
     * @param lengthRatio how much ratio for the preview length to do focus.
     * @param isMirror true for front camera.
     * @param displayOrientation the device display orientation.
     * @return the NormalizedPreview coordinate for camera device.
     */
    public static Rect uiToNormalizedPreview(Point p,
                                             Rect previewRect,
                                             float lengthRatio,
                                             boolean isMirror,
                                             int displayOrientation) {
        int w = previewRect.height();
        int h = previewRect.width();
        int previewHeight = previewRect.height();
        int previewWidth = previewRect.width();
        if (displayOrientation == 0 || displayOrientation == 180) {
            previewHeight = h > w ? w : h;
            previewWidth = h > w ? h : w;
        } else if (displayOrientation == 90 || displayOrientation == 270) {
            previewHeight = h > w ? h : w;
            previewWidth = h > w ? w : h;
        }
        coordinatesLog(TAG, "uiToNormalizedPreview, p.x = " + p.x + ", p.y = " + p.y
                + ", orientation = " + displayOrientation
                + ", mirror = " + isMirror);
        int focusLength = (int) (Math.min(previewHeight, previewWidth) * lengthRatio);
        coordinatesLog(TAG, "uiToNormalizedPreview, preview area = (" + previewRect.left
                + ", " + previewRect.top + ", " + previewRect.right + ", " + previewRect.bottom
                + "), " + "w = " + w + ", h = " + h);
        RectF previewRectF = new RectF(previewRect);
        Matrix matrixp = new Matrix();
        matrixp.postTranslate(-previewRect.left, -previewRect.top);
        //previewRectF (0,0,w,h)
        matrixp.mapRect(previewRectF);
        p.x = p.x - previewRect.left;
        p.y = p.y - previewRect.top;
        int left = clamp(p.x - focusLength / 2,
                (int) previewRectF.left, (int) previewRectF.right - focusLength);
        int top = clamp(p.y - focusLength / 2,
                (int) previewRectF.top, (int) previewRectF.bottom - focusLength);
        RectF focusRect = new RectF(left, top, left + focusLength, top + focusLength);
        coordinatesLog(TAG, "uiToNormalizedPreview, focus_rect = (" + left + ", " + top + ")," +
                "size = " + focusLength);
        Matrix matrix = new Matrix();
        prepareMatrix(matrix, isMirror, displayOrientation, previewWidth, previewHeight);
        Matrix transfromMatrix = new Matrix();
        matrix.invert(transfromMatrix);
        transfromMatrix.mapRect(focusRect);
        Rect deviceRect = new Rect();
        focusRect.round(deviceRect);
        if (!checkRectValidiate(deviceRect)) {
            LogHelper.i(TAG, "uiToNormalizedPreview, p.x = " + p.x + ", p.y = " + p.y
                    + ", orientation = " + displayOrientation
                    + ", mirror = " + isMirror);
            LogHelper.i(TAG, "uiToNormalizedPreview, preview area = (" + previewRect.left
                    + ", " + previewRect.top + ", " + previewRect.right + ", " + previewRect.bottom
                    + "), " + "w = " + w + ", h = " + h);
            LogHelper.i(TAG, "uiToNormalizedPreview, focus_rect = (" + left + ", " + top + ")," +
                    "size = " + focusLength);
            LogHelper.i(TAG, "uiToNormalizedPreview, result_rect = (" + deviceRect.left + ", "
                    + deviceRect.top + ", " + deviceRect.right + ", " + deviceRect.bottom + ")");
            throw new IllegalArgumentException("camera app set invalid coordinate");
        }
        return deviceRect;
    }

    /**
     * Transform NormalizedPreview(-1000,1000) coordinate to UI coordinate.
     * @param rect the rect to be transformed.
     * @param w the preview area width.
     * @param h the preview area height.
     * @param displayOrientation the preview display orientation.
     * @param isMirror it should be true for front camera.
     * @return the transfromed rect, Coordinate(0,0).
     */
    public static Rect normalizedPreviewToUi(Rect rect, int w, int h,
                                      int displayOrientation, boolean isMirror) {
        int previewHeight = 0;
        int previewWidth = 0;
        if (displayOrientation == 0 || displayOrientation == 180) {
            previewHeight = h > w ? w : h;
            previewWidth = h > w ? h : w;
        } else if (displayOrientation == 90 || displayOrientation == 270) {
            previewHeight = h > w ? h : w;
            previewWidth = h > w ? w : h;
        }
        coordinatesLog(TAG, "normalizedPreviewToUi, w = " + w + ", h = " + h
                + ", orientation = " + displayOrientation
                + ", mirror = " + isMirror);
        coordinatesLog(TAG, "normalizedPreviewToUi, rect = (" + rect.left + ", " + rect.top + ", "
                + rect.right + ", " + rect.bottom + ")");
        Matrix matrix = new Matrix();
        prepareMatrix(matrix, isMirror, displayOrientation, previewWidth, previewHeight);
        RectF rectf = new RectF(rect);
        matrix.mapRect(rectf);
        Rect resultRect = new Rect();
        rectf.round(resultRect);
        coordinatesLog(TAG, "normalizedPreviewToUi, result_rect = (" + resultRect.left + ", "
                + resultRect.top + ", "
                + resultRect.right + ", " + resultRect.bottom + ")");
        return resultRect;
    }

    /**
     * Transform the sensor coordinate from device to NormalizedPreview(-1000, 1000).
     * Center point is (0, 0).
     * @param transformRect the sensor rect.
     * @param previewWidth the camera device preview width.
     * @param previewHeight the camera device preview height.
     * @param cropRegion the sensor crop region.
     * @return the rect mapping to (-1000, 1000).
     */
    @TargetApi(Build.VERSION_CODES.LOLLIPOP)
    public static Rect sensorToNormalizedPreview(Rect transformRect,
                                          int previewWidth,
                                          int previewHeight,
                                          Rect cropRegion) {
        coordinatesLog(TAG, "sensorToNormalizedPreview, w = " + previewWidth
                + ", h = " + previewHeight);
        coordinatesLog(TAG, "sensorToNormalizedPreview, rect = (" + transformRect.left + ", "
                + transformRect.top + ", "
                + transformRect.right + ", " + transformRect.bottom + ")");
        coordinatesLog(TAG, "cropRegion = " + cropRegion.left + "," + cropRegion.top
                + "," + cropRegion.right + "," + cropRegion.bottom);
        double previewRatio = previewWidth > previewHeight ?
                ((double) previewWidth / previewHeight) :
                ((double) previewHeight / previewWidth);
        double cropRatio = (double) cropRegion.width() / cropRegion.height();
        int cropResizeWidth = cropRegion.width();
        int cropResizeHeight = cropRegion.height();
        if (previewRatio > cropRatio) {
            cropResizeHeight = (int) (cropResizeWidth / previewRatio);
        } else {
            cropResizeWidth = (int) (cropResizeHeight * previewRatio);
        }

        int deltaCropX = Math.abs(cropResizeWidth - cropRegion.width());
        int deltaCropY = Math.abs(cropResizeHeight - cropRegion.height());
        RectF rect = new RectF(transformRect);
        Matrix matrix = new Matrix();
        matrix.postTranslate(-cropRegion.left - deltaCropX / 2, -cropRegion.top - deltaCropY / 2);
        matrix.postTranslate(-cropResizeWidth / 2, -cropResizeHeight / 2);
        matrix.postScale(2000 / (float) cropResizeWidth, 2000 / (float) cropResizeHeight);
        matrix.mapRect(rect);
        Rect resultRect = new Rect();
        rect.round(resultRect);
        coordinatesLog(TAG, "sensorToNormalizedPreview, resultRect = (" + resultRect.left
                + ", " + resultRect.top + ", "
                + resultRect.right + ", " + resultRect.bottom + ")");
        return resultRect;
    }

    /**
     * It is to get the sensor touch region by the UI touch point.
     * @param p the touch point.
     * @param previewArea the preview layout area.
     * @param activityOrientation the activity orientation.
     * @param regionRatio how much ratio for the crop region shot edge
     *                    to do 3A.
     * @param cropRegion the sensor crop region.
     * @param characteristics the camera characteristics.
     * @return the touched sensor region.
     */
    @TargetApi(Build.VERSION_CODES.LOLLIPOP)
    public static Rect uiToSensor(Point p,
                           Rect previewArea,
                           int activityOrientation,
                           float regionRatio,
                           Rect cropRegion,
                           CameraCharacteristics characteristics) {
        LogHelper.d(TAG, "uiToSensor1, point = (" + p.x + ", " + p.y + "); "
            + "previewArea = (" + previewArea.left + ", " + previewArea.top
            + ", " + previewArea.right + ", " + previewArea.bottom + "); "
            + "cropRegion = (" + cropRegion.width() + ", " + cropRegion.height() + ")");
        // Normalize coordinates to [0,1]
        float points[] = new float[2];
        points[0] = (float) (p.x - previewArea.left) / previewArea.width();
        points[1] = (float) (p.y - previewArea.top) / previewArea.height();
        // Rotate coordinates to portrait orientation.
        Matrix rotationMatrix = new Matrix();
        rotationMatrix.setRotate(activityOrientation, 0.5f, 0.5f);
        rotationMatrix.mapPoints(points);

        // Invert X coordinate on front camera since the display is mirrored.
        if (characteristics.get(CameraCharacteristics.LENS_FACING)
                == CameraCharacteristics.LENS_FACING_FRONT) {
            points[0] = 1 - points[0];
        }
        int sensorOrientation = characteristics.get(
                CameraCharacteristics.SENSOR_ORIENTATION);
        // Compute the output MeteringRectangle in sensor space.
        // Crop region itself is specified in sensor coordinates.
        // Normalized coordinates, now rotated into sensor space.
        PointF nsc = normalizedSensorCoordsForNormalizedDisplayCoords(
                points[0], points[1], sensorOrientation);

        Rect meteringRegion = normalizedPreviewTransformedToSensor(
                nsc, regionRatio, previewArea, cropRegion);

        LogHelper.d(TAG, "uiToSensor1, resultRegion = (" + meteringRegion.left + ", "
                + meteringRegion.top + ", " + meteringRegion.right + ", "
                + meteringRegion.bottom + ")");
        return meteringRegion;
    }

    /**
     * It is to get the sensor touch region by the UI touch point.
     * @param p the touch point.
     * @param previewArea the preview layout area.
     * @param activityOrientation the activity orientation.
     * @param regionRatio how much ratio for the crop region shot edge
     *                    to do 3A.
     * @param cropRegion the sensor crop region.
     * @param sensorOrientation camera sensor orientation from camera CameraCharacteristics.
     * @param cameraFace front camera or back camera.
     * @return the touched sensor region.
     */
    @TargetApi(Build.VERSION_CODES.LOLLIPOP)
    public static Rect uiToSensor(Point p,
                                  Rect previewArea,
                                  int activityOrientation,
                                  float regionRatio,
                                  Rect cropRegion,
                                  int sensorOrientation,
                                  int cameraFace) {
        LogHelper.d(TAG, "uiToSensor2, point = (" + p.x + ", " + p.y + "); "
                + "previewArea = (" + previewArea.left + ", " + previewArea.top
                + ", " + previewArea.right + ", " + previewArea.bottom + "); "
                + "cropRegion = (" + cropRegion.width() + ", " + cropRegion.height() + ")");
        // Normalize coordinates to [0,1]
        float points[] = new float[2];
        points[0] = (float) (p.x - previewArea.left) / previewArea.width();
        points[1] = (float) (p.y - previewArea.top) / previewArea.height();
        // Rotate coordinates to portrait orientation.
        Matrix rotationMatrix = new Matrix();
        rotationMatrix.setRotate(activityOrientation, 0.5f, 0.5f);
        rotationMatrix.mapPoints(points);

        LogHelper.d(TAG, "uiToSensor2, sensorOrientation = " + sensorOrientation
            + ", cameraFace = " + cameraFace);
        // Invert X coordinate on front camera since the display is mirrored.
        if (cameraFace == CameraCharacteristics.LENS_FACING_FRONT) {
            points[0] = 1 - points[0];
        }
        // Compute the output MeteringRectangle in sensor space.
        // Crop region itself is specified in sensor coordinates.
        // Normalized coordinates, now rotated into sensor space.
        PointF nsc = normalizedSensorCoordsForNormalizedDisplayCoords(
                points[0], points[1], sensorOrientation);

        Rect meteringRegion = normalizedPreviewTransformedToSensor(
                nsc, regionRatio, previewArea, cropRegion);

        LogHelper.d(TAG, "uiToSensor2, resultRegion = (" + meteringRegion.left + ", "
                + meteringRegion.top + ", " + meteringRegion.right + ", "
                + meteringRegion.bottom + ")");
        return meteringRegion;
    }

    private static Rect normalizedPreviewTransformedToSensor(PointF nsc, float regionRatio,
                                                          Rect previewArea, Rect cropRegion) {
        // Compute half side length in pixels.
        int minCropEdge = Math.min(cropRegion.width(), cropRegion.height());
        int halfSideLength = (int) (0.5f * regionRatio * minCropEdge);
        double previewRatio = previewArea.width() > previewArea.height() ?
                ((double) previewArea.width() / previewArea.height()) :
                ((double) previewArea.height() / previewArea.width());
        double cropRatio = (double) cropRegion.width() / cropRegion.height();
        int cropResizeWidth = cropRegion.width();
        int cropResizeHeight = cropRegion.height();
        if (previewRatio > cropRatio) {
            cropResizeHeight = (int) (cropResizeWidth / previewRatio);
        } else {
            cropResizeWidth = (int) (cropResizeHeight * previewRatio);
        }
        int deltaCropX = (cropRegion.width() - cropResizeWidth) / 2;
        int deltaCropY = (cropRegion.height() - cropResizeHeight) / 2;
        int xCenterSensor = (int) (cropRegion.left + nsc.x * cropResizeWidth + deltaCropX);
        int yCenterSensor = (int) (cropRegion.top + nsc.y * cropResizeHeight + deltaCropY);
        Rect restrictionRegion = new Rect(cropRegion.left + deltaCropX,
                cropRegion.top + deltaCropY,
                cropRegion.right - deltaCropX,
                cropRegion.bottom - deltaCropY);

        int touchSensorLeft = clamp(xCenterSensor - halfSideLength,
                restrictionRegion.left, restrictionRegion.right);
        int touchSensorTop = clamp(yCenterSensor - halfSideLength,
                restrictionRegion.top, restrictionRegion.bottom);
        int touchSensorRight = clamp(xCenterSensor + halfSideLength,
                restrictionRegion.left, restrictionRegion.right);
        int touchSensorBottom = clamp(yCenterSensor + halfSideLength,
                restrictionRegion.top, restrictionRegion.bottom);

        Rect meteringRegion = new Rect(touchSensorLeft, touchSensorTop,
                touchSensorRight, touchSensorBottom);
        return meteringRegion;
    }

    private static void prepareMatrix(Matrix matrix, boolean mirror, int displayOrientation,
                                      int viewWidth, int viewHeight) {
        // Need mirror for front camera.
        matrix.setScale(mirror ? -1 : 1, 1);
        // This is the value for android.hardware.Camera.setDisplayOrientation.
        matrix.postRotate(displayOrientation);
        // Camera driver coordinates range from (-1000, -1000) to (1000, 1000).
        // UI coordinates range from (0, 0) to (width, height).
        matrix.postScale(viewWidth / 2000f, viewHeight / 2000f);
        matrix.postTranslate(viewWidth / 2f, viewHeight / 2f);
    }

    private static PointF normalizedSensorCoordsForNormalizedDisplayCoords(
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
                return null;
        }
    }

    private static int clamp(int x, int min, int max) {
        if (x > max) {
            return max;
        }
        if (x < min) {
            return min;
        }
        return x;
    }

    private static void coordinatesLog(LogUtil.Tag tag, String string) {
        if (sIsDebugMode) {
            LogHelper.d(tag, string);
        }
    }

    private static boolean checkRectValidiate(Rect rect) {
        if (rect.left > 1000 || rect.left < -1000) {
            return false;
        } else if (rect.top > 1000 || rect.top < -1000) {
            return false;
        } else if (rect.right > 1000 || rect.right < -1000) {
            return false;
        } else if (rect.bottom > 1000 || rect.bottom < -1000) {
            return false;
        }
        return true;
    }
}
