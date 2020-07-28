package com.mediatek.providers.contacts.testsuite;

import junit.framework.Test;
import junit.framework.TestSuite;

import com.android.providers.contacts.ContactsProvider2Test;
import com.mediatek.providers.contacts.unittest.MergeAndSplitTest;
import com.mediatek.providers.contacts.unittest.ModifyPhoneContactTest;
import com.mediatek.providers.contacts.unittest.ModifySimContactTest;
import com.mediatek.providers.contacts.unittest.QueryContactTest;

public class UnitTestSuite {

    public static Test suite() {
        TestSuite suite = new TestSuite("Basic test for com.android.providers.contacts");
        // MTK added test.
        suite.addTestSuite(ModifyPhoneContactTest.class);
        suite.addTestSuite(ModifySimContactTest.class);
        suite.addTestSuite(MergeAndSplitTest.class);
        suite.addTestSuite(QueryContactTest.class);

        // add Android default test.
        //suite.addTestSuite(ContactLookupKeyTest.class);
        suite.addTestSuite(ContactsProvider2Test.class);
       /* suite.addTestSuite(NameLookupBuilderTest.class);
        suite.addTestSuite(NameNormalizerTest.class);
        suite.addTestSuite(NameSplitterTest.class);
        suite.addTestSuite(SearchIndexManagerTest.class);*/
        return suite;
    }
}