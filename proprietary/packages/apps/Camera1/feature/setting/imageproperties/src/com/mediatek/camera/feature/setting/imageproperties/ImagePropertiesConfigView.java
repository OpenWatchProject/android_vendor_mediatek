package com.mediatek.camera.feature.setting.imageproperties;

import android.os.Bundle;
import android.preference.PreferenceFragment;
import android.preference.PreferenceScreen;
import android.support.v7.widget.Toolbar;

import com.mediatek.camera.R;
import com.mediatek.camera.common.debug.LogUtil;
import com.mediatek.camera.common.preference.Preference;

import java.util.List;

/**
 * Image properties configure view.
 */

public class ImagePropertiesConfigView extends PreferenceFragment {
    private static final LogUtil.Tag TAG
            = new LogUtil.Tag(ImagePropertiesConfigView.class.getSimpleName());

    private OnValueChangeListener mListener;
    private List<ImagePropertiesSettingView.ItemInfo> mItemsInfo;
    private Preference.OnPreferenceChangeListener mOnPreferenceChangeListener
            = new MyOnPreferenceChangeListener();

    /**
     * Listener to listen value changed.
     */
    public interface OnValueChangeListener {
        /**
         * Callback when value changed.
         *
         * @param key The string that indicator the changed item.
         * @param value The changed value.
         */
        void onValueChanged(String key, String value);
    }

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        Toolbar toolbar = (Toolbar) getActivity().findViewById(R.id.toolbar);
        if (toolbar != null) {
            toolbar.setTitle(getActivity().getResources()
                    .getString(R.string.pref_camera_image_properties_title));
        }
        addPreferencesFromResource(R.xml.image_properties_config_preference);
        PreferenceScreen screen = getPreferenceScreen();
        for (ImagePropertiesSettingView.ItemInfo info : mItemsInfo) {
            ImagePropertiesPreference preference = new ImagePropertiesPreference(getActivity());
            preference.setKey(info.key);
            preference.setTitle(info.title);
            preference.setValue(info.value);
            preference.setOnPreferenceChangeListener(mOnPreferenceChangeListener);
            screen.addPreference(preference);
        }
    }

    /**
     * Set listener to lister changed value.
     *
     * @param listener The instance of {@link OnValueChangeListener}.
     */
    public void setOnValueChangeListener(OnValueChangeListener listener) {
        mListener = listener;
    }

    /**
     * Set image properties child item info.
     *
     * @param itemsInfo The mage properties child item info.
     */
    public void setItemsInfo(List<ImagePropertiesSettingView.ItemInfo> itemsInfo) {
        mItemsInfo = itemsInfo;
    }

    /**
     * Preference click listener.
     */
    private class MyOnPreferenceChangeListener implements Preference.OnPreferenceChangeListener {
        @Override
        public boolean onPreferenceChange(android.preference.Preference preference, Object o) {
            String key = ((ImagePropertiesPreference) preference).getKey();
            String value = (String) o;
            for (ImagePropertiesSettingView.ItemInfo info : mItemsInfo) {
                if (info.key.equals(key)) {
                    info.value = value;
                    break;
                }
            }
            if (mListener != null) {
                mListener.onValueChanged(key, value);
            }
            return true;
        }
    }
}
