/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2010. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
 * AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek Software")
 * have been modified by MediaTek Inc. All revisions are subject to any receiver's
 * applicable license agreements with MediaTek Inc.
 */

package com.mediatek.ygps;

import android.content.Context;
import android.graphics.Bitmap;
import android.graphics.Canvas;
import android.graphics.Paint;
import android.graphics.Paint.Align;
import android.graphics.Paint.Style;
import android.graphics.drawable.BitmapDrawable;
import android.util.AttributeSet;

import java.util.List;


/**
 * The view to show satellites information in circle view.
 *
 */
public class SatelLocationView extends SatelliteBaseView {

    /**
     * A class to record coordinator for a point.
     *
     */
    private static class Point {
        float mCoX;
        float mCoY;
        Point() {}
        Point(float x1, float y1) {
            mCoX = x1;
            mCoY = y1;
        }
    }

    private static final String TAG = "YGPS/SatelLocationView";
    private static final int MARGIN = 12;
    private static final float PERCENT_75 = 0.75f;
    private static final float RIGHT_ANGLE = 90.0f;
    private static final float STRAIGHT_ANGLE = 180.0f;

    private Paint mGraphicPaint = null;
    private Paint mTextPaint = null;
    private Paint mBgPaint = null;
    private Paint mCircleBorderPaint = null;
    private Bitmap mUnfixedBmp = null;
    private Bitmap mUsedInFixBmp = null;
    private Bitmap mUnusedInFixBmp = null;

    /**
     * Constructor function.
     * @param context Context for view running in
     */
    public SatelLocationView(Context context) {
        this(context, null, 0);
    }

    /**
     * Constructor function.
     * @param context Context for view running in
     * @param attrs The attributes of the XML tag that is inflating the view
     */
    public SatelLocationView(Context context, AttributeSet attrs) {
        this(context, attrs, 0);
    }

    /**
     * Constructor function.
     * @param context Context for view running in
     * @param attrs The attributes of the XML tag that is inflating the view
     * @param defStyle An attribute in the current theme that contains a reference to
     *          a style resource that supplies default values for the view
     */
    public SatelLocationView(Context context, AttributeSet attrs, int defStyle) {
        super(context, attrs, defStyle);
        onCreateView();
    }

    private void onCreateView() {
        mGraphicPaint = new Paint();
        mGraphicPaint.setStyle(Style.STROKE);
        mGraphicPaint.setAntiAlias(true);
        mGraphicPaint.setColor(getResources().getColor(R.color.grid));
        mGraphicPaint.setStrokeWidth(1.0f);

        mTextPaint = new Paint();
        mTextPaint.setTextAlign(Align.CENTER);
        mTextPaint.setTextSize(12.0f);
        mTextPaint.setColor(getResources().getColor(R.color.skyview_text_color));

        mBgPaint = new Paint();
        mBgPaint.setColor(getResources().getColor(R.color.skyview_background));

        mCircleBorderPaint = new Paint();
        mCircleBorderPaint.setStyle(Style.STROKE);
        mCircleBorderPaint.setAntiAlias(true);
        mCircleBorderPaint.setStrokeWidth(2.0f);

        mUnfixedBmp = getBmpFromRes(R.drawable.satred);
        mUnusedInFixBmp = getBmpFromRes(R.drawable.satyellow);
        mUsedInFixBmp = getBmpFromRes(R.drawable.satgreen);
    }

    private Bitmap getBmpFromRes(int rid) {
        BitmapDrawable bmpDraw = (BitmapDrawable) getResources().getDrawable(rid);
        return bmpDraw.getBitmap();
    }

    private Point computeViewPostion(SatelliteInfo si, Point origin, float baseRadius) {
        Point targPt = new Point();
        float project = baseRadius * (RIGHT_ANGLE - si.mElevation) / RIGHT_ANGLE;
        float alpha = si.mAzimuth;
        float radian = (float) (alpha * Math.PI / STRAIGHT_ANGLE);
        targPt.mCoX = origin.mCoX + (float) (project * Math.sin(radian));
        targPt.mCoY = origin.mCoY - (float) (project * Math.cos(radian));
        return targPt;
    }

    @Override
    protected void onDraw(Canvas canvas) {
        super.onDraw(canvas);
        float viewWidth = getWidth();
        float viewHeight = getHeight();
        Point origin = new Point(viewWidth / 2, viewHeight / 2);
        int maxRadius = (int) Math.floor(origin.mCoY - MARGIN);

        canvas.drawPaint(mBgPaint);
        float[] radiusArr = {maxRadius >> 2, maxRadius >> 1, maxRadius * PERCENT_75, maxRadius};
        for (int i = 0; i < radiusArr.length; i++) {
            canvas.drawCircle(origin.mCoX, origin.mCoY, radiusArr[i], mGraphicPaint);
        }
        canvas.drawLine(origin.mCoX - maxRadius, origin.mCoY, origin.mCoX - (maxRadius >> 2),
                origin.mCoY, mGraphicPaint);
        canvas.drawLine(origin.mCoX + maxRadius, origin.mCoY, origin.mCoX + (maxRadius >> 2),
                origin.mCoY, mGraphicPaint);
        canvas.drawLine(origin.mCoX, origin.mCoY - maxRadius, origin.mCoX,
                origin.mCoY - (maxRadius >> 2), mGraphicPaint);
        canvas.drawLine(origin.mCoX, origin.mCoY + maxRadius, origin.mCoX,
                origin.mCoY + (maxRadius >> 2), mGraphicPaint);

        SatelliteInfoManager simgr = getSatelliteInfoManager();
        if (simgr != null) {
            List<SatelliteInfo> siList = simgr.getSatelInfoList();
            for (SatelliteInfo si : siList) {
                //Xlog.d(TAG, "handle " + si.toString());
                if (si.mPrn <= 0 || si.mElevation <= 0
                        || si.mAzimuth < 0) {

                    continue;
                }
                Point targPt = computeViewPostion(si, origin, maxRadius);
                Bitmap drawnBmp = null;
                if (!simgr.isUsedInFix(SatelliteInfoManager.PRN_ANY) || si.mSnr <= 0) {
                    drawnBmp = mUnfixedBmp;
                } else {
                    if (simgr.isUsedInFix(si.mPrn)) {
                        drawnBmp = mUsedInFixBmp;
                    } else {
                        drawnBmp = mUnusedInFixBmp;
                    }
                }
                int bmpHeight = drawnBmp.getHeight();
                float targX = targPt.mCoX - bmpHeight / 2.0f;
                float targY = targPt.mCoY - bmpHeight / 2.0f;
                //Xlog.d(TAG, "targX:" + targX + " targY:" + targY);
                canvas.drawBitmap(drawnBmp, targX, targY, mGraphicPaint);
                mCircleBorderPaint.setColor(si.mColor);
                canvas.drawCircle(targPt.mCoX, targPt.mCoY, bmpHeight / 2.0f - 1.5f,
                        mCircleBorderPaint);
                canvas.drawText(String.valueOf(si.mPrn), targX, targY, mTextPaint);
            }

        }
    }


}
