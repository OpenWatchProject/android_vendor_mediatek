package com.mediatek.contacts.ext;

import android.content.Context;
import android.net.Uri;
import android.view.View;
import android.view.ViewGroup;

public class DefaultViewCustomExtension implements IViewCustomExtension {

    @Override
    public ContactListItemViewCustom getContactListItemViewCustom() {
        return mContactListItemViewCustom;
    }

    @Override
    public QuickContactCardViewCustom getQuickContactCardViewCustom() {
        return mQuickContactCardViewCustom;
    }

    @Override
    public QuickContactScrollerCustom getQuickContactScrollerCustom() {
        return mQuickContactScrollerCustom;
    }

    // ///------------------below are the default implements------------------//
    private ContactListItemViewCustom mContactListItemViewCustom = new ContactListItemViewCustom() {

        @Override
        public void onMeasure(int widthMeasureSpec, int heightMeasureSpec) {
            // do-nothing
        }

        @Override
        public void onLayout(boolean changed, int leftBound, int topBound, int rightBound,
                int bottomBound) {
            // do-nothing
        }

        @Override
        public void addCustomView(long contactId, ViewGroup viewGroup) {
             //do nothing
        }
        @Override
        public int getWidthWithPadding(){
            return 0;
        }
    };

    private QuickContactCardViewCustom mQuickContactCardViewCustom =
            new QuickContactCardViewCustom() {
        @Override
        public View createCardView(View container, View anchorView, Uri lookupUri,
                Context context) {
            return null;
        }
    };

    private QuickContactScrollerCustom mQuickContactScrollerCustom =
            new QuickContactScrollerCustom() {
        @Override
        public View createJoynIconView(View container, View anchorView, Uri lookupUri) {
            return null;
        }

        @Override
        public void updateJoynIconView() {
            // do-nothing
        }
    };

}
