package com.mediatek.mms.plugin;

import com.mediatek.mms.ext.DefaultOpSmilHelperExt;

public class Op01SmilHelperExt extends DefaultOpSmilHelperExt {

    /*op01 return true, it don't support
    add vcard or vcalendar into smil */
    @Override
    public boolean createSmilDocument() {
        return true;
    }

}
