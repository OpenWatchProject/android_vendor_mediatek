package com.mediatek.miravision.ui;

import android.app.Fragment;
import android.os.Bundle;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.TextView;

public class IntroductionFragment extends Fragment {

    @Override
    public View onCreateView(LayoutInflater inflater, ViewGroup container, Bundle savedInstanceState) {
        View rootView = inflater.inflate(R.layout.miravision_intro, container, false);
        TextView introView = (TextView) rootView.findViewById(R.id.text);
        introView.setText(R.string.intro_str);
        return rootView;
    }
}
