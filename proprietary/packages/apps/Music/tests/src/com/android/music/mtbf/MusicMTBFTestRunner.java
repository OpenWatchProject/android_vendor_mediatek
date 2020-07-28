package com.android.music.tests.mtbf;

import junit.framework.TestSuite;
import android.test.InstrumentationTestRunner;

public class MusicMTBFTestRunner extends InstrumentationTestRunner {

    @Override
    public TestSuite getAllTests() {
        TestSuite suite = new TestSuite();
        suite.addTestSuite(MusicMTBFTest.class);
        return suite;
    }

    @Override
    public ClassLoader getLoader() {
        return MusicMTBFTestRunner.class.getClassLoader();
    }
}
