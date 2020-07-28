package com.mediatek.mms.plugin;

import android.content.Context;

import com.mediatek.mms.ext.DefaultOpMessagePluginExt;
import com.mediatek.mms.ext.IOpConversationListExt;
import com.mediatek.mms.ext.IOpSmsPreferenceActivityExt;

public class Op02MessagePluginExt extends DefaultOpMessagePluginExt {

    public Op02MessagePluginExt(Context context) {
        super(context);
    }

    @Override
    public IOpConversationListExt getOpConversationListExt() {
        return new Op02ConversationListExt(mContext);
    }

    @Override
    public IOpSmsPreferenceActivityExt getOpSmsPreferenceActivityExt() {
        return new Op02SmsPreferenceActivityExt();
    }
}
