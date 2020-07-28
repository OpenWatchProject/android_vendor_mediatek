package com.mediatek.providers.contacts.runner;

import junit.framework.TestSuite;
import android.test.InstrumentationTestRunner;

import com.mediatek.providers.contacts.testsuite.UnitTestSuite;

public class UnitTestRunner  extends InstrumentationTestRunner{

    @Override
    public TestSuite getAllTests() {
        TestSuite suite = new TestSuite();
        suite.addTest(UnitTestSuite.suite());
        return suite;
    }
}
