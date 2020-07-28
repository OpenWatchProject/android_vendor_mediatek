package com.mediatek.providers.contacts.unittest;

import android.provider.ContactsContract.AggregationExceptions;

import com.android.providers.contacts.BaseContactsProvider2Test;
import com.android.providers.contacts.testutil.RawContactUtil;

public class MergeAndSplitTest extends BaseContactsProvider2Test {

    private static final String TAG = QueryContactTest.class.getSimpleName();

    /**
     *<Purpose>
     *Can merge contact successful
     *
     *[Initial condition]
     *no
     *
     *[Step]
     *1. insert two contacts, contains name and number
     *2. Merge them
     *
     *[Expected result]
     *Can merge Contact successful
    */
    public void testcase01_mergeContacts() {
        long rawContactId1 = RawContactUtil.createRawContact(mResolver);
        insertPhoneNumber(rawContactId1, "1112222333", true);

        long rawContactId2 = RawContactUtil.createRawContact(mResolver);
        insertPhoneNumber(rawContactId2, "123456789", true);

        // Aggregate them
        setAggregationException(AggregationExceptions.TYPE_KEEP_TOGETHER,
                rawContactId1, rawContactId2);

        assertAggregated(rawContactId1, rawContactId2, "123456789");
    }

    /**
     *<Purpose>
     *Can merge contact successful
     *
     *[Initial condition]
     *no
     *
     *[Step]
     *1. insert two contacts, contains name and number
     *2. Merge them
     *3. Split them
     *
     *[Expected result]
     *Can Split Contact successful
    */
    public void testcase02_splitContacts() {
        long rawContactId1 = RawContactUtil.createRawContact(mResolver);
        insertPhoneNumber(rawContactId1, "1112222333", true);

        long rawContactId2 = RawContactUtil.createRawContact(mResolver);
        insertPhoneNumber(rawContactId2, "123456789", true);

        // Aggregate them
        setAggregationException(AggregationExceptions.TYPE_KEEP_TOGETHER,
                rawContactId1, rawContactId2);

        // Split them
        setAggregationException(AggregationExceptions.TYPE_KEEP_SEPARATE,
                rawContactId1, rawContactId2);

        assertNotAggregated(rawContactId1, rawContactId2);
    }
}
