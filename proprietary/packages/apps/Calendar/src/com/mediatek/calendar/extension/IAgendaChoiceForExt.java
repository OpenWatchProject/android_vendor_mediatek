package com.mediatek.calendar.extension;

import android.content.Intent;

/**
 * M: This interface describes the way AgendaChoiceActivity should do to
 * support the plug-in.
 */
public interface IAgendaChoiceForExt {

    /**
     * M: the plug-in prepare the return value, and set to an Intent,
     * then the host should do something to notify the caller.
     * @param ret the return value of the item picked
     */
    void retSelectedEvent(Intent ret);
}
