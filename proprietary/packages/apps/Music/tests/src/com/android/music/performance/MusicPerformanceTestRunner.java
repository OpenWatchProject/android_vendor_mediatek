package com.android.music.tests.performance;

import junit.framework.TestSuite;
import android.test.InstrumentationTestRunner;

public class MusicPerformanceTestRunner extends InstrumentationTestRunner {

    @Override
    public TestSuite getAllTests() {
        TestSuite suite = new TestSuite();
        suite.addTestSuite(MusicPerformanceTest.class);
        return suite;
    }

    @Override
    public ClassLoader getLoader() {
        return MusicPerformanceTestRunner.class.getClassLoader();
    }
}
