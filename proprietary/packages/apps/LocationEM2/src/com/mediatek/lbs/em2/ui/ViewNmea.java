package com.mediatek.lbs.em2.ui;

import android.content.Context;
import android.graphics.Canvas;
import android.graphics.Paint;
import android.graphics.Paint.Align;
import android.graphics.Paint.Style;
import android.location.GpsSatellite;
import android.location.GpsStatus;
import android.location.GnssStatus;
import android.util.AttributeSet;
import android.util.Log;
import android.view.View;

import java.util.ArrayList;
import com.mediatek.lbs.em2.ui.NmeaParser.SatelliteInfo;


public class ViewNmea extends View {
    private final int MAX_SATELLITE_NUM = 64;
    private final int MAX_SATELLITE_SIGNAL_IN_UI = 16;

    private Paint mBackground;
    private Paint mLine;
    private Paint mColoredText;
    private Paint mGreen;
    private Paint mRed;
    private Paint mText;

    float mTextSize = 20f;
    int mSatInView = 0;
    ArrayList<SvData> mSvList = new ArrayList<SvData>();

    int mPage = 0;

    class SvData {
        int svid;
        float snr;
        float ele;
        float azi;
        boolean used;
        int type;
        int color;
    }

    public void setGpsStatus(ArrayList<SatelliteInfo> SatList) {
        int satCount = 0;

        mSvList.clear();
        for (SatelliteInfo sat : SatList) {
            if (satCount >= MAX_SATELLITE_NUM) {
                log("ERR: the number of satellite is over " + MAX_SATELLITE_NUM);
                break;
            }
            SvData svData = new SvData();
            svData.svid = sat.mPrn;
            svData.snr = sat.mSnr;
            svData.ele = sat.mElevation;
            svData.azi = sat.mAzimuth;
            svData.used = sat.mUsedInFix;
            svData.type = getSvType(svData.svid);
            svData.color = sat.mColor;

            int j = 0;
            for(j=0; j < satCount; j++) {
                SvData indxSvData = mSvList.get(j);
                if (svData.snr > indxSvData.snr)
                    break;
            }
            mSvList.add(j, svData);
            satCount++;
        }
        mSatInView = satCount;
        sortByTypeAndSvidInView();
        invalidate();
    }

    public void setDisplayPage(int page) {
        mPage = page;
        invalidate();
    }

    private void sortByTypeAndSvidInView() {
        int pages = mSatInView / MAX_SATELLITE_SIGNAL_IN_UI;
        if (mSatInView % MAX_SATELLITE_SIGNAL_IN_UI != 0) {
            pages++;
        }

        for (int p=0; p < pages; p++) {
            int satInView = (mSatInView > MAX_SATELLITE_SIGNAL_IN_UI*(p+1)) ?
                    MAX_SATELLITE_SIGNAL_IN_UI : (mSatInView - MAX_SATELLITE_SIGNAL_IN_UI*p) ;
            for (int i=MAX_SATELLITE_SIGNAL_IN_UI*p;i<MAX_SATELLITE_SIGNAL_IN_UI*p+satInView; i++) {
                SvData svData = mSvList.remove(i);
                int j = 0;
                for (j=MAX_SATELLITE_SIGNAL_IN_UI*p; j<i; j++) {
                    SvData indxSvData = mSvList.get(j);
                    if (svData.type < indxSvData.type) {
                        break;
                    } else if (svData.type == indxSvData.type && svData.svid < indxSvData.svid) {
                        break;
                    }
                }
                mSvList.add(j, svData);
            }
        }
    }

    private int getSvType(int svid) {

        if (svid > 200) { // GnssStatus.CONSTELLATION_BEIDOU
            return GnssStatus.CONSTELLATION_BEIDOU;
        } else if (svid > 64) { // GnssStatus.CONSTELLATION_GLONASS
            return GnssStatus.CONSTELLATION_GLONASS;
        } else if (svid > 0) {  // GnssStatus.CONSTELLATION_GPS)
            return GnssStatus.CONSTELLATION_GPS;
        }
        return GnssStatus.CONSTELLATION_GPS;
    }

    public void resetNmeaView() {
        mSatInView = 0;
        invalidate();
    }

    public ViewNmea(Context context) {
        this(context, null, 0);
    }
    public ViewNmea(Context context, AttributeSet attrs) {
        this(context, attrs, 0);
    }
    public ViewNmea(Context context, AttributeSet attrs, int defStyle) {
        super(context, attrs, defStyle);
        /*
        //-------------TEST
        mSatInView = 16;
        for(int i = 0; i < mSatInView; i ++) {
            mPrns[i] = i;
            mSnrs[i] = 50;
            mEle[i] = i*2;
            mAzi[i] = i*9;
            mUsed[i] = (i%2 == 0);
            if(i % 3 == 0) {
                mColor[i] = 0xffff0000;
            } else if(i % 3 == 1) {
                mColor[i] = 0xff00ff00;
            } else {
                mColor[i] = 0xff0000ff;
            }
        }
        //-------------TEST
         */

        mBackground = new Paint();
        mBackground.setColor(0xff333333);

        mLine = new Paint();
        mLine.setColor(0xffffffff); //WHITE
        mLine.setAntiAlias(true);
        mLine.setStyle(Style.STROKE);
        mLine.setStrokeWidth(1.0f);

        mColoredText = new Paint();
        mColoredText.setColor(0xffffffff); //WHITE
        mColoredText.setAntiAlias(true);
        mColoredText.setStyle(Style.STROKE);
        mColoredText.setStrokeWidth(2.0f);

        mRed = new Paint();
        mRed.setColor(0xEFFF0000); //Red
        mRed.setAntiAlias(true);
        mRed.setStyle(Style.FILL_AND_STROKE);
        mRed.setStrokeWidth(1.0f);

        mGreen = new Paint();
        mGreen.setColor(0xBB00BB00); //Green
        mGreen.setAntiAlias(true);
        mGreen.setStyle(Style.FILL);
        mGreen.setStrokeWidth(1.0f);

        mText = new Paint();
        mText.setColor(0xFFFFFFFF); //Black word
        mText.setAntiAlias(true);
        mText.setStyle(Style.FILL);
        mText.setStrokeWidth(1.0f);
        mText.setTextAlign(Align.CENTER);
        mText.setTextSize(20.0f);
    }

    private static float getHeight(Paint p) {
        return -(p.ascent() + p.descent());
    }

    private static void drawTextInCenter(Canvas c, String text, float x,
            float y, Paint p) {
        c.drawText(text, x, y + getHeight(p) / 2, p);
    }

    private static void drawCircleAndText(Canvas c, String text, float x,
            float y, Paint pCircle, Paint pText) {
        c.drawCircle(x, y, getHeight(pText) * 1.2f, pCircle);
        drawTextInCenter(c, text, x, y, pText);
    }

    private static float checkEle(float ele) {
        if (ele > 90.0f) {
            return 90.0f;
        } else if (ele < 0.0f) {
            return 0.0f;
        } else {
            return ele;
        }
    }

    private static float checkCn(float cn) {
        if (cn > 50.0f) {
            return 50.0f;
        } else {
            return cn;
        }
    }

    protected void onDraw(Canvas canvas) {
        super.onDraw(canvas);

        canvas.drawPaint(mBackground);

        draw(canvas, getWidth(), getHeight());
    }

    private void draw(Canvas c, float w, float h) {
        float min = (w > h) ? h : w;

        mTextSize = (min / 1000) * 30;
        mText.setTextSize(mTextSize);

        log("w=" + w + " h=" + h + " min=" + min + " mTextSize=" + mTextSize);

        drawSignals(c, 0, h * 3 / 4, w, h);
        drawGpsView(c, mTextSize, mTextSize, w - mTextSize, (h * 3 / 4)
                - mTextSize);
    }

    private void drawGpsView(Canvas c, float left, float top, float right, float bottom) {
        float w = right - left; // width
        float h = bottom - top; // height
        float min = (w < h) ? w : h; // min
        float cx = w / 2 + left; // center x
        float cy = h / 2 + top; // center y

        c.drawCircle(cx, cy, (min / 6) * 3, mLine);
        c.drawCircle(cx, cy, (min / 6) * 2, mLine);
        c.drawCircle(cx, cy, (min / 6) * 1, mLine);
        c.drawLine(cx - min / 2, cy, cx + min / 2, cy, mLine);
        c.drawLine(cx, cy - min / 2, cx, cy + min / 2, mLine);

        for (int i = 0; i < mSatInView; i++) {
            SvData svData = mSvList.get(i);
            String svid = "" + svData.svid;
            float ele = checkEle(svData.ele);
            float azi = svData.azi;
            boolean inUsed = svData.used;
            mColoredText.setColor(svData.color);

            ele = (1.0f - (ele / 90.0f)) * (min / 2);
            float x = ((float) Math.sin(azi * Math.PI / 180.0f) * ele);
            float y = (-(float) Math.cos(azi * Math.PI / 180.0f) * ele);

            Paint color;
            if (inUsed) {
                color = mGreen;
            } else {
                color = mRed;
            }
            drawCircleAndText(c, svid, cx + x, cy + y, color, mText);
            c.drawCircle(cx + x, cy + y, getHeight(mText) * 1.2f, mColoredText);
        }
    }

    private void drawSignals(Canvas c, float left, float top, float right, float bottom) {
        float w = right - left;
        float h = bottom - top;
        float sw = w / MAX_SATELLITE_SIGNAL_IN_UI;
        float sh = (h - mTextSize) / 50; // signal height per CN
        float by = bottom - mTextSize; // base Y
        int satInView = (mSatInView > MAX_SATELLITE_SIGNAL_IN_UI*(mPage+1)) ?
                MAX_SATELLITE_SIGNAL_IN_UI : mSatInView-MAX_SATELLITE_SIGNAL_IN_UI*mPage;

        satInView = satInView < 0 ? 0 : satInView;

        // draw separator
        c.drawLine(left, by, right, by, mLine);

        for (int i = 0; i < satInView; i++) {
            int index = i + MAX_SATELLITE_SIGNAL_IN_UI*mPage;
            SvData svData = mSvList.get(index);
            String svid = "" + svData.svid;
            float cn = svData.snr;
            boolean inUsed = svData.used;
            mColoredText.setColor(svData.color);
            float bx = i * sw + sw / 2;

            // draw bar
            Paint color;
            if (inUsed) {
                color = mGreen;
            } else {
                color = mRed;
            }
            c.drawRect(i * sw, by - (sh * checkCn(cn)), (i + 1) * sw, by, color);
            c.drawRect(i * sw, by - (sh * checkCn(cn)), (i + 1) * sw, by, mColoredText);

            // draw svid
            drawTextInCenter(c, svid, bx, by + mTextSize / 2, mText);

            // draw cn
            drawTextInCenter(c, String.format("%.01f", cn), bx, by - mTextSize / 2, mText);
        }
    }

    //=================== basic utility ========================\\
    private void log(String msg) {
        Log.d("LocationEM", msg);
    }
}
