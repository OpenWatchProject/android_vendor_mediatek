package com.mediatek.mms.ext;

import android.content.Context;

public interface IOpSettingListActivityExt {

    /**
     * @internal
     */
    public String[] setAdapter(String[] settingList);

    /**
     * @internal
     */
    public boolean onOpCreate(Context hostActivity, Context appContext);

    /**
     * @internal
     */
    public void onListItemClick(int position);
}
