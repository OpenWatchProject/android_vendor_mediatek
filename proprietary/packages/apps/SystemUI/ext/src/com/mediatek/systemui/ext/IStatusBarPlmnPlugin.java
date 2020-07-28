package com.mediatek.systemui.ext;

import android.content.Context;
import android.view.View;
import android.view.ViewGroup;
import android.widget.TextView;
import android.widget.LinearLayout;

/**
 * M: the interface for Plug-in definition of Status bar.
 */
public interface IStatusBarPlmnPlugin {
    /**
     * Whether support customize PLMN carrier label.
     * @return Return whether support customize carrier label.
     * @internal
     */
    boolean supportCustomizeCarrierLabel();

    /**
     * Customize PLMN carrier label.
     * @param parentView parent view.
     * @param orgCarrierLabel org default carrier label.
     * @return Customize carrier label ViewGroup.
     * @internal
     */
    View customizeCarrierLabel(ViewGroup parentView, View orgCarrierLabel);

    /**
     * Update PLMN carrier label visibility.
     * @param force Whether force update.
     * @param makeVisible Whether Visible.
     * @internal
     */
    void updateCarrierLabelVisibility(boolean force, boolean makeVisible);

    /**
     * Update PLMN carrier label by slotId.
     * @param slotId The slot index.
     * @param isSimInserted Whether Sim Inserted.
     * @param isHasSimService Whether Sim service is available.
     * @param networkNames carrier plmn texts.
     * @internal
     */
    void updateCarrierLabel(int slotId, boolean isSimInserted, boolean isHasSimService,
            String[] networkNames);
    /**
     * Add PLMN.
     * @param statusBarContents LinearLayout.
     * @param contx Context.
     * @internal
     */
    void addPlmn(LinearLayout statusBarContents, Context contx);

    /**
     * setPlmnVisibility
     * @param visibility for the text view
     * @internal
     */
    void setPlmnVisibility(int visibility);
}
