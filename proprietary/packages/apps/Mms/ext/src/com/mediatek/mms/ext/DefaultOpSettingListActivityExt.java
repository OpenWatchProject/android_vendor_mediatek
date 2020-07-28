package com.mediatek.mms.ext;

import android.content.Context;

public class DefaultOpSettingListActivityExt implements IOpSettingListActivityExt {

    public boolean onOpCreate(Context hostActivity, Context appContext) {
        return false;
    }

    @Override
    public String[] setAdapter(String[] settingList) {
        return settingList;
    }

    @Override
    public void onListItemClick(int position) {

    }
}
