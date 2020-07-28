package com.mediatek.gallerybasic.base;

import android.view.ActionMode;
import android.view.Menu;
import android.view.MenuItem;

/**
 * If you want to add more operations in thumbnail view, implement this interface. It provides
 * chances to add/update/click menu item in thumbnail view.
 */
public interface IActionMode {
    /**
     * Callback when {@link ActionMode.Callback#onCreateActionMode(ActionMode, Menu)}.
     * @param actionMode
     *            ActionMode being created
     * @param menu
     *            Menu used to populate action buttons
     */
    public void onCreateActionMode(ActionMode actionMode, Menu menu);

    /**
     * Callback when {@link ActionMode.Callback#onPrepareActionMode(ActionMode, Menu) } or other
     * cases need to update menu view is thumbnail view.
     * @param actionMode
     *            ActionMode being prepared
     * @param menu
     *            Menu used to populate action buttons
     */
    public void onPrepareActionMode(ActionMode actionMode, Menu menu);

    /**
     * Callback when {@link ActionMode.Callback#onDestroyActionMode(ActionMode)}.
     * @param actionMode
     *            ActionMode being destroyed
     */
    public void onDestroyActionMode(ActionMode actionMode);

    /**
     * Callback when {@link ActionMode.Callback#onActionItemClicked(ActionMode, MenuItem)}.
     * @param actionMode The current ActionMode
     * @param item The menu item that was clicked
     * @return true if this callback handled this event, false if not handled
     */
    public boolean onActionItemClicked(ActionMode actionMode, MenuItem item);

    /**
     * Called to report current selection data has changed. The callback does not run in UI thread,
     * so do not update UI in this function.
     * @param data
     *            The current selection data
     */
    public void onSelectionChange(MediaData[] data);
}
