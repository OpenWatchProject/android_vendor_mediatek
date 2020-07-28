package com.android.mms.draft;

import android.net.Uri;

/*******************************************************************************
 *
 * Interface : whihc used to do the job in the WorkingMessage after save and load
 * finished
 * The interface implements in WorkingMessage
 *
*******************************************************************************/
public interface IDraftInterface {
    public void updateAfterSaveDraftFinished(final Uri msgUri,
            final int create, final boolean result);

    public void loadFinished(MmsDraftData mdd);
}
