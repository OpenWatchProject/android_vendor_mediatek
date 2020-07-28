package com.mediatek.contacts.ext;

import android.content.Context;

public class ContactsCustomizationFactoryBase {

    protected static Context sContext = null;

    public ContactsCustomizationFactoryBase() {
    }

    public ContactsCustomizationFactoryBase(Context context) {
        sContext = context;
    }

    public IOp01Extension makeOp01Ext(Context context) {
         return new DefaultOp01Extension();
    }

    public IContactsCommonPresenceExtension makeContactsCommonPresenceExt(Context context) {
        return new DefaultContactsCommonPresenceExtension();
    }

    public IContactsPickerExtension makeContactsPickerExt(Context context) {
        return new DefaultContactsPickerExtension();
    }

    public IRcsExtension makeRcsExt(Context context) {
         return new DefaultRcsExtension();
    }

    public IRcsRichUiExtension makeRcsRichUiExt(Context context) {
         return new DefaultRcsRichUiExtension();
    }

    public IViewCustomExtension makeViewCustomExt(Context context) {
        return new DefaultViewCustomExtension();
    }
}
