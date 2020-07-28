package com.mediatek.camera.feature.setting.imageproperties;

import android.content.Context;
import android.view.View;
import android.widget.RadioButton;
import android.widget.TextView;

import com.mediatek.camera.R;
import com.mediatek.camera.common.preference.Preference;

/**
 * Created by mtk54473 on 10/30/16.
 */

public class ImagePropertiesPreference extends Preference implements View.OnClickListener {
    private static final String LOW_VALUE = "low";
    private static final String MIDDLE_VALUE = "middle";
    private static final String HIGH_VALUE = "high";

    private TextView mTitleView;
    private String mKey;
    private String mValue;
    private CharSequence mTitle;

    private RadioButton mRadioButton1;
    private RadioButton mRadioButton2;
    private RadioButton mRadioButton3;

    private OnPreferenceChangeListener mListener;

    /**
     * Image properties preference constructor.
     *
     * @param context The instance of {@link Context}.
     */
    public ImagePropertiesPreference(Context context) {
        super(context);
        setLayoutResource(R.layout.image_properties_preference_layout);
    }

    @Override
    protected void onBindView(View view) {
        super.onBindView(view);
        mTitleView = (TextView) view.findViewById(R.id.title);
        mTitleView.setText(mTitle);

        mRadioButton1 = (RadioButton) view.findViewById(R.id.radio1);
        mRadioButton2 = (RadioButton) view.findViewById(R.id.radio2);
        mRadioButton3 = (RadioButton) view.findViewById(R.id.radio3);
        mRadioButton1.setOnClickListener(this);
        mRadioButton2.setOnClickListener(this);
        mRadioButton3.setOnClickListener(this);

        if (LOW_VALUE.equals(mValue)) {
            mRadioButton1.setChecked(true);
        } else if (MIDDLE_VALUE.equals(mValue)) {
            mRadioButton2.setChecked(true);
        } else if (HIGH_VALUE.equals(mValue)) {
            mRadioButton3.setChecked(true);
        }
    }

    @Override
    public void setTitle(CharSequence title) {
        mTitle = title;
        if (mTitleView != null) {
            mTitleView.setText(title);
        }
    }

    @Override
    public void onClick(View view) {
        if (mRadioButton1 == view) {
            mRadioButton2.setChecked(false);
            mRadioButton3.setChecked(false);
            mListener.onPreferenceChange(this, LOW_VALUE);
        } else if (mRadioButton2 == view) {
            mRadioButton1.setChecked(false);
            mRadioButton3.setChecked(false);
            mListener.onPreferenceChange(this, MIDDLE_VALUE);
        } else if (mRadioButton3 == view) {
            mRadioButton1.setChecked(false);
            mRadioButton2.setChecked(false);
            mListener.onPreferenceChange(this, HIGH_VALUE);
        }
    }

    @Override
    public void setOnPreferenceChangeListener(
            OnPreferenceChangeListener onPreferenceChangeListener) {
        mListener = onPreferenceChangeListener;
    }

    /**
     * Set key to indicator this preference.
     *
     * @param key The string to indicator this preference.
     */
    public void setKey(String key) {
        mKey = key;
    }

    /**
     * Get the key of this preference.
     *
     * @return The key of this preference.
     */
    public String getKey() {
        return mKey;
    }

    /**
     * Set the value of this preference.
     *
     * @param value The value of this preference.
     */
    public void setValue(String value) {
        mValue = value;
    }
}
