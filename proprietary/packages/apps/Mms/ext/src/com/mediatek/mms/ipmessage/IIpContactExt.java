package com.mediatek.mms.ipmessage;

import android.content.Context;
import android.graphics.drawable.Drawable;

import com.mediatek.mms.callback.IContactCallback;

public interface IIpContactExt {

    /**
     * M: Init IpContact
     * @param context: context
     * @param callback: Contact callback
     * @internal
     */
    public void onIpInit(Context context, IContactCallback callback);

    /**
     * M: called on updateContact
     * @param number: number
     * @param name: name
     * @return String: name
     * @internal
     */
    public String onIpUpdateContact(String number, String name);

    /**
     * M: called on getAvatar
     * @param defaultValue: defaultValue
     * @param threadId: threadId
     * @param number: number
     * @return Drawable: Avatar drawable
     * @internal
     */
    public Drawable onIpGetAvatar(Drawable defaultValue, final long threadId, String number);

    /**
     * M: called on getContactInfoForPhoneNumber
     * @param number: number
     * @return String: ip number
     * @internal
     */
    public String onIpGetNumber(String number);

    /**
     * M: called on fillPhoneTypeContact
     * @param number: number
     * @return boolean
     * @internal
     */
    public boolean onIpIsGroup(String number);

    /**
     * M: called on invalidateGroup
     * @param number: number
     * @internal
     */
    public void invalidateGroupContact(String number);
}
