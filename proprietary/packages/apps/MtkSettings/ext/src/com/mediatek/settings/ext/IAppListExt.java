package com.mediatek.settings.ext;

import android.graphics.drawable.Drawable;
import android.widget.TextView;
import android.view.View;
import android.view.ViewGroup;


public interface IAppListExt {

    /**
    * To customise view in the sms preference list in AppListPrefernce
    * @param view original to be shown
    * @param textView TextView in the preference list
    * @param defaultLabel Defaultlabel of the textview
    * @param position View postion
    * @param image image to be drawn
    * @param parent ViewGroup parent
    * @return View if the specified item is added.
    * @internal
    */
    public View addLayoutAppView(View view, TextView textView, TextView defaultLabel,
                                 int position, Drawable image, ViewGroup parent);

    /**
    * Set applist item in the sms preference list from AppListPrefernce
    * @param packageName Package name of the sms application
    * @param position of the package name in the list
    * @internal
    */
    public void setAppListItem(String packageName, int position);
}
