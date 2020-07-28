package com.via.bypass.test;

import android.app.Instrumentation;
import android.os.Bundle;
import android.test.AndroidTestRunner;
import android.test.InstrumentationTestRunner;
import android.util.Log;

import junit.framework.AssertionFailedError;
import junit.framework.Test;
import junit.framework.TestListener;
import junit.framework.TestSuite;

/**
 * Bypass test.
 */
public class BypassTestRunner extends InstrumentationTestRunner {
    private static final String TAG = "BypassTestRunner";
    private int mMode = -1;
    private int mAction = -1;

    @Override
    public TestSuite getAllTests() {
        TestSuite suite = new TestSuite();
        suite.addTestSuite(BypassTest.class);
        return suite;
    }

    @Override
    public void onCreate(Bundle arguments) {
        String mode = arguments.getString("mode");
        String action = arguments.getString("action");
        Log.d(TAG, "mode " + mode);
        Log.d(TAG, "action " + action);
        if ("gps".equals(mode)) {
            mMode = 1;
        } else if ("pcv".equals(mode)) {
            mMode = 2;
        } else if ("atc".equals(mode)) {
            mMode = 4;
        } else if ("ets".equals(mode)) {
            mMode = 8;
        } else if ("data".equals(mode)) {
            mMode = 16;
        } else if ("all".equals(mode)) {
            mMode = 31;
        } else {
            mMode = -1;
        }
        if ("enable".equals(action)) {
            mAction = 0;
        } else if ("disable".equals(action)) {
            mAction = 1;
        } else if ("query".equals(action)) {
            mAction = 2;
        } else {
            mAction = -1;
        }
        super.onCreate(arguments);
    }

    @Override
    protected AndroidTestRunner getAndroidTestRunner() {
        Log.d(TAG, "Getting android test runner");
        AndroidTestRunner runner = super.getAndroidTestRunner();
        runner.addTestListener(new MyListener());
        return runner;
    }

    /**
     * Test listener.
     */
    private class MyListener implements TestListener {
        Bundle mTestResult;

        @Override
        public void startTest(final Test test) {
            Log.d(TAG, "Starting test: " + test);
            mTestResult = new Bundle();
            if (test instanceof BypassTest) {
                final BypassTest t = (BypassTest) test;
                t.mode = mMode;
                t.action = mAction;
            }
        }

        @Override
        public void endTest(final Test test) {
            Log.d(TAG, "Ending test: " + test);
            if (test instanceof BypassTest) {
                final BypassTest t = (BypassTest) test;
                Log.d(TAG, "Ending test: " + t.result);
                if (mAction == 2) {
                    mTestResult.putString(Instrumentation.REPORT_KEY_STREAMRESULT,
                           "\nBypassResult\n" + t.result + "\n");
                }
                sendStatus(0, mTestResult);
            }
        }

        @Override
        public void addError(final Test test, final Throwable t) {
        }

        @Override
        public void addFailure(final Test test, final AssertionFailedError f) {
        }
    }
}
