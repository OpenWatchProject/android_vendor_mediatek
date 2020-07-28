package com.mediatek.engineermode.mdmcomponent;

import com.mediatek.engineermode.R;

import android.app.Activity;
import android.content.Context;
import android.content.res.TypedArray;
import android.graphics.Canvas;
import android.text.Layout;
import android.util.AttributeSet;
import android.view.View;
import android.widget.LinearLayout;
import android.widget.TextView;

public class MdmTextView extends TextView {

    private static final String TAG = "MdmTextView";
    private Context mContext;
    public MdmTextView(Context context, AttributeSet attrs,
                                      int defStyle) {
        super(context, attrs, defStyle);
        mContext = context;
    }

    public MdmTextView(Context context, AttributeSet attrs) {
        super(context, attrs);
        mContext = context;
    }

    public MdmTextView(Context context) {
        super(context);
        mContext = context;
    }

    @Override
    protected void onDraw(Canvas canvas) {
        super.onDraw(canvas);
    }
    @Override
    protected void onMeasure(int widthMeasureSpec, int heightMeasureSpec) {
        super.onMeasure(widthMeasureSpec, heightMeasureSpec);

        Layout layout = getLayout();
        if (layout != null) {
            int height = (int)Math.ceil(
            getMaxLineHeight(this.getText().toString(),
            MeasureSpec.getSize(widthMeasureSpec)))
                    + getCompoundPaddingTop() + getCompoundPaddingBottom();
            int width = getMeasuredWidth();
            setMeasuredDimension(width, height);
        }
    }

    private float getMaxLineHeight(String str, int widthSpec) {
        float height = 0.0f;
        float screenW = ((Activity)mContext).getWindowManager().getDefaultDisplay().getWidth();
        float paddingLeft = this.getPaddingLeft();
        float paddingReft = this.getPaddingRight();
        int k = 0;
        int childNum = ((LinearLayout)this.getParent()).getChildCount();
        for(int i=0; i<childNum; i++){
            if(((LinearLayout)this.getParent()).getChildAt(i).getVisibility() == View.VISIBLE) {
                k++;
            }
        }
        k = k - 9;
        str = str.replace("\\r", "\n").replace("\\n", "\n").replace("\\t", "        ");
        String[] contentArray = str.split("\n");
        int line = 0;
        for(String content: contentArray) {
            line += ((int) Math.ceil( (this.getPaint().measureText(content) /
                    ((screenW-dip2px(k-1)) / k-paddingLeft-paddingReft))));
        }
        height = (this.getPaint().getFontMetrics().descent -
        this.getPaint().getFontMetrics().ascent) * line;
        return height;
    }
    private float dip2px(float dipValue) {
        final float scale = mContext.getResources().getDisplayMetrics().density;
        return (dipValue * scale);
    }

}
