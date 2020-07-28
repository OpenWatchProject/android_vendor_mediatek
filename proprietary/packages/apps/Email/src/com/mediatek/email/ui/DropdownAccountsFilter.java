package com.mediatek.email.ui;

import android.widget.Filter;

import java.util.ArrayList;
import java.util.List;


/**
 * M: An array filter constrains the content of the array adapter with
 * a prefix. Each item that does not start with the supplied prefix is removed from the list.
 * and add "username" at the begin of each item.
 * @param <T>
 */
public class DropdownAccountsFilter<T> extends Filter {
    String mFilterString = null;
    DropdownAccountsArrayAdapter<T> mReferenceAdapter;
    List<T> mObjects;
    ArrayList<T> mOriginalValues;
    // Synchronized lock with referenced adapter's data collection.
    final Object mLock;

    public DropdownAccountsFilter(DropdownAccountsArrayAdapter<T> referenceAdapter) {
        super();
        mObjects = referenceAdapter.getObjects();
        mOriginalValues = referenceAdapter.getOriginalValues();
        mLock = referenceAdapter.getDataLock();
        mReferenceAdapter = referenceAdapter;
    }

    public String getFilterString() {
        return mFilterString;
    }

    @Override
    protected FilterResults performFiltering(CharSequence prefix) {

        FilterResults results = new FilterResults();

        if (mOriginalValues == null) {
            synchronized (mLock) {
                mOriginalValues = new ArrayList<T>(mObjects);
                mReferenceAdapter.setOriginalValues(mOriginalValues);
            }
        }

        if (prefix == null || prefix.length() == 0) {
            ArrayList<T> list;
            synchronized (mLock) {
                list = new ArrayList<T>(mOriginalValues);
            }
            results.values = list;
            results.count = list.size();
        } else {
            String prefixString = prefix.toString().toLowerCase();

            ArrayList<T> values;
            synchronized (mLock) {
                values = new ArrayList<T>(mOriginalValues);
            }

            final int count = values.size();
            final ArrayList<T> newValues = new ArrayList<T>();

            for (int i = 0; i < count; i++) {
                final T value = values.get(i);
                final String valueText = value.toString().toLowerCase();

                // First match against the whole, non-splitted value
                if (valueText.startsWith(prefixString)) {
                    newValues.add(value);
                } else {
                    final String[] words = valueText.split(" ");
                    final int wordCount = words.length;

                    // Start at index 0, in case valueText starts with space(s)
                    for (int k = 0; k < wordCount; k++) {
                        if (words[k].startsWith(prefixString)) {
                            newValues.add(value);
                            break;
                        }
                    }
                }
            }

            results.values = newValues;
            results.count = newValues.size();
        }
        return results;
    }

    @Override
    protected void publishResults(CharSequence constraint, FilterResults results) {
        mFilterString = (constraint != null ? constraint.toString() : null);
        //noinspection unchecked
        List<T> resultList = (List<T>) results.values;
        List<T> addUserName = new ArrayList<T>();
        for (T s : resultList) {
            addUserName.add((T) (mReferenceAdapter.getUserName() + s));
        }
        mObjects = addUserName;
        mReferenceAdapter.setObjects(mObjects);
        if (results.count > 0) {
            mReferenceAdapter.notifyDataSetChanged();
        } else {
            mReferenceAdapter.notifyDataSetInvalidated();
        }
    }
}
