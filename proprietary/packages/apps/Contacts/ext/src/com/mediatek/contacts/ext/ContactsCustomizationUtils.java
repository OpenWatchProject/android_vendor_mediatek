package com.mediatek.contacts.ext;

import android.content.Context;
import android.util.Log;

import com.mediatek.contacts.ext.ContactsCustomizationFactoryBase;
import com.mediatek.common.util.OperatorCustomizationFactoryLoader;
import com.mediatek.common.util.OperatorCustomizationFactoryLoader.OperatorFactoryInfo;

import java.util.ArrayList;
import java.util.List;

public class ContactsCustomizationUtils {
    static ContactsCustomizationFactoryBase sFactory = null;
    static ContactsCustomizationFactoryBase sRcsFactory = null;
    static ContactsCustomizationFactoryBase sRcsRichUiFactory = null;
    static ContactsCustomizationFactoryBase sWwopRcsFactory = null;

    private static final List<OperatorFactoryInfo> sFactoryInfoList
                                                            = new ArrayList<OperatorFactoryInfo>();
    static {
        Log.d("ContactsCustomizationUtils", "Init contacts plugin list begin");
        //-------------------------- IOp01Extension  --------------------------------------
        sFactoryInfoList.add(
                new OperatorFactoryInfo(
                    "Op01Contacts.apk",
                    "com.mediatek.contacts.plugin.Op01ContactsCustomizationFactory",
                    "com.mediatek.contacts.plugin",
                    "OP01"
        ));

        //-------------------------- IContactsCommonPresenceExtension ---------------------
        sFactoryInfoList.add(new OperatorFactoryInfo("OP07Contacts.apk",
                    "com.mediatek.contacts.plugin.Op07ContactsCustomizationFactory",
                    "com.mediatek.contacts.plugin",
                    "OP07"
            ));

        sFactoryInfoList.add(new OperatorFactoryInfo("OP08Contacts.apk",
                    "com.mediatek.contacts.plugin.Op08ContactsCustomizationFactory",
                    "com.mediatek.contacts.plugin",
                    "OP08"
            ));

        sFactoryInfoList.add(new OperatorFactoryInfo("OP12Contacts.apk",
                    "com.mediatek.contacts.plugin.Op12ContactsCustomizationFactory",
                    "com.mediatek.contacts.plugin",
                    "OP12"
            ));

        //--------------------- IContactsPickerExtension --------------------
        sFactoryInfoList.add(new OperatorFactoryInfo("Op18Contacts.apk",
                    "com.mediatek.op18.contacts.OP18ContactsCustomizationFactory",
                    "com.mediatek.op18.contacts",
                    "OP18"
            ));
        Log.d("ContactsCustomizationUtils", "Init contacts plugin list end");
    }


    public static synchronized ContactsCustomizationFactoryBase getFactory(Context context) {
        if (sFactory == null) {
            sFactory = (ContactsCustomizationFactoryBase) OperatorCustomizationFactoryLoader
                    .loadFactory(context, sFactoryInfoList);
            if (sFactory == null) {
                sFactory = new ContactsCustomizationFactoryBase();
            }
        }
        return sFactory;
    }

    //-------------------------- IOp01Extension  --------------------------------------
    public static synchronized ContactsCustomizationFactoryBase getOp01Factory(Context context) {
        return getFactory(context);
    }

    //-------------------------- IContactsCommonPresenceExtension ---------------------
    public static synchronized ContactsCustomizationFactoryBase getContactsPresenceFactory(
                                Context context) {
        return getFactory(context);
    }

    //--------------------- IContactsPickerExtension --------------------
    public static synchronized ContactsCustomizationFactoryBase getContactsPickerFactory(
                                Context context) {
        return getFactory(context);
    }

    //--------------------- IRcsExtension --------------------
    private static final List<OperatorFactoryInfo> sRcsInfoList
                                                            = new ArrayList<OperatorFactoryInfo>();
    static {
        sRcsInfoList.add(
                new OperatorFactoryInfo(
                    "RCSContacts.apk",
                    "com.mediatek.rcs.contacts.ext.Op01ContactsCustomizationFactory",
                    "com.mediatek.rcs.contacts.ext",
                    "OP01"
        ));
    }
    public static synchronized ContactsCustomizationFactoryBase getRcsFactory(Context context) {
        if (sRcsFactory == null) {
            sRcsFactory = (ContactsCustomizationFactoryBase) OperatorCustomizationFactoryLoader
                    .loadFactory(context, sRcsInfoList);
            if (sRcsFactory == null) {
                sRcsFactory = new ContactsCustomizationFactoryBase();
            }
        }
        return sRcsFactory;
    }

    //--------------------- IRcsRichUiExtension --------------------
    private static final List<OperatorFactoryInfo> sRcsRichUiInfoList
                                                            = new ArrayList<OperatorFactoryInfo>();
    static {
        sRcsRichUiInfoList.add(
                new OperatorFactoryInfo(
                    "RCSPhone.apk",
                    "com.mediatek.rcs.incallui.ext.Op01ContactsCustomizationFactory",
                    "com.mediatek.rcs.incallui.ext",
                    "OP01"
        ));
    }
    public static synchronized ContactsCustomizationFactoryBase getRcsRichUiFactory(Context context) {
        if (sRcsRichUiFactory == null) {
            sRcsRichUiFactory = (ContactsCustomizationFactoryBase) OperatorCustomizationFactoryLoader
                    .loadFactory(context, sRcsRichUiInfoList);
            if (sRcsRichUiFactory == null) {
                sRcsRichUiFactory = new ContactsCustomizationFactoryBase();
            }
        }
        return sRcsRichUiFactory;
    }

    //------------------- WWOP RCS IViewCustomExtension ---------------------
    private static final List<OperatorFactoryInfo> sWwopRcsFactoryInfoList
                                                            = new ArrayList<OperatorFactoryInfo>();

    static {
        sWwopRcsFactoryInfoList.add(new OperatorFactoryInfo("Rcse.apk",
                    "com.mediatek.rcse.plugin.contacts.RCSeContactsCustomizationFactory",
                    "com.mediatek.rcs",
                    "OP07"
        ));

        sWwopRcsFactoryInfoList.add(new OperatorFactoryInfo("Rcse.apk",
                    "com.mediatek.rcse.plugin.contacts.RCSeContactsCustomizationFactory",
                    "com.mediatek.rcs",
                    "OP08"
        ));
    }

    public static synchronized ContactsCustomizationFactoryBase getWWOPRcsFactory(Context context) {
        if (sWwopRcsFactory == null) {
            sWwopRcsFactory = (ContactsCustomizationFactoryBase) OperatorCustomizationFactoryLoader
                    .loadFactory(context, sWwopRcsFactoryInfoList);
            if (sWwopRcsFactory == null) {
                sWwopRcsFactory = new ContactsCustomizationFactoryBase();
            }
        }
        return sWwopRcsFactory;
    }

    public static synchronized void resetCustomizationFactory() {
        sFactory = null;
        sRcsFactory = null;
        sRcsRichUiFactory = null;
        sWwopRcsFactory = null;
    }
}

