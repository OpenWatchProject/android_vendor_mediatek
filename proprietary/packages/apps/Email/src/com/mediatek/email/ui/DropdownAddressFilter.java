package com.mediatek.email.ui;

import java.util.List;

/**
 * M: Don't like DropdownAccountsFilter, this class don't add username in pubishResults().
 *
 * @param <T>
 */
public class DropdownAddressFilter<T> extends DropdownAccountsFilter<T> {

    public DropdownAddressFilter(DropdownAccountsArrayAdapter<T> referenceAdapter) {
        super(referenceAdapter);
    }

    @Override
    protected void publishResults(CharSequence constraint, FilterResults results) {
        mFilterString = (constraint != null ? constraint.toString() : null);
        // noinspection unchecked
        mObjects = (List<T>) results.values;
        mReferenceAdapter.setObjects(mObjects);
        if (results.count > 0) {
            mReferenceAdapter.notifyDataSetChanged();
        } else {
            mReferenceAdapter.notifyDataSetInvalidated();
        }
    }
}
