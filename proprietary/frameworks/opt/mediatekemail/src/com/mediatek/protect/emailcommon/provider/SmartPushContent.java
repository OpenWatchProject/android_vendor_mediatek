package com.mediatek.protect.emailcommon.provider;

public interface SmartPushContent {
    String ID = "_id";
    String ACCOUNT_KEY = "accountKey";
    String EVENT_TYPE = "eventType";
    String TIMESTAMP = "timestamp";
    String VALUE = "value";

    int TYPE_MAIL = 1;
    int TYPE_DURATION = 2;
    int TYPE_OPEN = 3;

    String[] HABIT_PROJECTION = new String[] { SmartPushContent.EVENT_TYPE,
            SmartPushContent.TIMESTAMP, SmartPushContent.VALUE };
    String HABIT_SELECTION = SmartPushContent.ACCOUNT_KEY + " =?";
    String[] CONTENT_PROJECTION = new String[] { ID, ACCOUNT_KEY, EVENT_TYPE,
            TIMESTAMP, VALUE };
}
