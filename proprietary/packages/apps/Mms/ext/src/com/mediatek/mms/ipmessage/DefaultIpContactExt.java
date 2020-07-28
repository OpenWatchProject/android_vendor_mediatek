package com.mediatek.mms.ipmessage;

import android.content.Context;
import android.graphics.drawable.Drawable;

import com.mediatek.mms.callback.IContactCallback;

public class DefaultIpContactExt implements IIpContactExt {

    @Override
    public void onIpInit(Context context, IContactCallback callback) {
        return;
    }

    @Override
    public String onIpUpdateContact(String number, String name) {
        return name;
    }

    @Override
    public Drawable onIpGetAvatar(Drawable defaultValue, long threadId, String number) {
        return defaultValue;
    }

    @Override
    public String onIpGetNumber(String number) {
        return number;
    }

    @Override
    public boolean onIpIsGroup(String number) {
        return false;
    }

    @Override
    public void invalidateGroupContact(String number) {
        return;
    }
}
