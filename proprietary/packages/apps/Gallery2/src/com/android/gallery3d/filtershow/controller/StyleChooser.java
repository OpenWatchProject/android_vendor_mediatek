package com.android.gallery3d.filtershow.controller;

import android.app.ActionBar.LayoutParams;
import android.content.Context;
import android.content.res.Resources;
import android.graphics.Bitmap;
import android.graphics.drawable.GradientDrawable;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.ImageButton;
import android.widget.ImageView.ScaleType;
import android.widget.LinearLayout;

import com.android.gallery3d.R;
import com.android.gallery3d.filtershow.editors.Editor;

import java.util.Vector;

public class StyleChooser implements Control {
    private final String LOGTAG = "StyleChooser";
    protected ParameterStyles mParameter;
    protected LinearLayout mLinearLayout;
    protected Editor mEditor;
    private View mTopView;
    private Vector<ImageButton> mIconButton = new Vector<ImageButton>();
    protected int mLayoutID = R.layout.filtershow_control_style_chooser;

    @Override
    public void setUp(ViewGroup container, Parameter parameter, Editor editor) {
        container.removeAllViews();
        mEditor = editor;
        Context context = container.getContext();
        mParameter = (ParameterStyles) parameter;
        LayoutInflater inflater =
                (LayoutInflater) context.getSystemService(Context.LAYOUT_INFLATER_SERVICE);
        mTopView = inflater.inflate(mLayoutID, container, true);
        mLinearLayout = (LinearLayout) mTopView.findViewById(R.id.listStyles);
        mTopView.setVisibility(View.VISIBLE);
        int n = mParameter.getNumberOfStyles();
        mIconButton.clear();
        Resources res = context.getResources();
        int dim = res.getDimensionPixelSize(R.dimen.draw_style_icon_dim);
        LayoutParams lp = new LayoutParams(dim, dim);
        for (int i = 0; i < n; i++) {
            final ImageButton button = new ImageButton(context);
            button.setScaleType(ScaleType.CENTER_CROP);
            button.setLayoutParams(lp);
            /// M: [BEHAVIOR.MODIFY] add border effect to the icon of selected style @{
            // button.setBackgroundResource(android.R.color.transparent);
            button.setBackgroundResource(R.drawable.filtershow_color_picker_circle);
            GradientDrawable sd = ((GradientDrawable) button.getBackground());
            sd.setColor(android.R.color.transparent);
            /// @}
            mIconButton.add(button);
            final int buttonNo = i;
            button.setOnClickListener(new View.OnClickListener() {
                @Override
                public void onClick(View arg0) {
                    mParameter.setSelected(buttonNo);
                    /// M: [BEHAVIOR.ADD] add border effect to the icon of selected style @{
                    resetBorders();
                    /// @}
                }
            });
            mLinearLayout.addView(button);
            mParameter.getIcon(i, new BitmapCaller() {
                @Override
                public void available(Bitmap bmap) {

                    if (bmap == null) {
                        return;
                    }
                    button.setImageBitmap(bmap);
                }
            });
        }
        /// M: [BEHAVIOR.ADD] add border effect to the icon of selected style @{
        mTransparent = res.getColor(R.color.color_chooser_unslected_border);
        mSelected = res.getColor(R.color.color_chooser_slected_border);
        resetBorders();
        /// @}
    }

    @Override
    public View getTopView() {
        return mTopView;
    }

    @Override
    public void setPrameter(Parameter parameter) {
        mParameter = (ParameterStyles) parameter;
        updateUI();
    }

    @Override
    public void updateUI() {
        if (mParameter == null) {
            return;
        }
    }

    /// M: [BEHAVIOR.ADD] add border effect to the icon of selected style @{
    private static final int ICON_BORDER_WIDTH = 3;
    private int mTransparent;
    private int mSelected;

    private void resetBorders() {
        int selected = mParameter.getSelected();
        GradientDrawable sd;
        int n = mParameter.getNumberOfStyles();
        for (int i = 0; i < n; i++) {
            sd = ((GradientDrawable) mIconButton.get(i).getBackground());
            sd.setStroke(ICON_BORDER_WIDTH, (selected == i) ? mSelected : mTransparent);
        }
    }
    /// @}
}
