package com.mediatek.contacts.ext;

import android.content.Context;
import android.net.Uri;
import android.view.View;
import android.view.ViewGroup;

/**
 *
 * For RCS-e view custom.
 *
 */
public interface IViewCustomExtension {
    /**
     *
     * Used in RCS-e to custom ContactListItemView in People List
     */
    public interface ContactListItemViewCustom {

        /**
         *
         * @param widthMeasureSpec
         * @param heightMeasureSpec
         * @internal
         */
        void onMeasure(int widthMeasureSpec, int heightMeasureSpec);

        /**
         *
         * @param changed
         * @param leftBound
         * @param topBound
         * @param rightBound
         * @param bottomBound
         * @internal
         */
        void onLayout(boolean changed, int leftBound, int topBound, int rightBound,
                int bottomBound);
        /**
         * addCustomView.
         * @param contactId contactId
         * @param viewGroup ViewGroup
         */
        void addCustomView(long contactId, ViewGroup viewGroup);

        /**
        * getWidthWithPadding.
        * @return padding width
        */
        int getWidthWithPadding();
    }

    /**
     *
     * Used to add Joyn card view in QuickActivity
     *
     */
    public interface QuickContactCardViewCustom {
        /**
         * Plugin should create your Joyn Card view & add into it's LinearLayout
         * parent id.card_container
         *
         * @param lookupUri
         *            Contact lookup Uri
         * @return the instance of ExpandingEntryCardView that plugin add.
         * @internal
         */
        View createCardView(View container, View anchorView, Uri lookupUri, Context context);
    }

    /**
     *
     * Used to add Joyn icon on the left top in QuickActivity
     *
     */
    public interface QuickContactScrollerCustom {
        /**
         *
         * @param container
         *            A FrameLayout placed on the left top
         * @param anchorView
         *            the NameView
         * @param lookupUri
         *            Contact uri
         * @return ImageView contains joyn icon
         * @internal
         */
        View createJoynIconView(View container, View anchorView, Uri lookupUri);

        /**
         * update the ImageView visiblity when scroll.
         * @internal
         */
        void updateJoynIconView();
    }

    /**
     *
     * @return get the object for custom ContactListItemView
     * @internal
     */
    ContactListItemViewCustom getContactListItemViewCustom();

    /**
     * @return get the object for custom QuickActivity Card
     * @internal
     */
    QuickContactCardViewCustom getQuickContactCardViewCustom();

    /**
     * @return get the object for custom MultiShrinkScroller
     * @internal
     */
    QuickContactScrollerCustom getQuickContactScrollerCustom();
}
