package com.android.music.tests.functional;

import junit.framework.TestSuite;
import android.test.InstrumentationTestRunner;

public class MusicFunctionalAutoTestRunner extends InstrumentationTestRunner {

    @Override
    public TestSuite getAllTests() {
        TestSuite suite = new TestSuite();
        suite.addTestSuite(TrackBrowserFunctionTest.class);
        suite.addTestSuite(PlaylistBrowserFunctionTest.class);
        suite.addTestSuite(PlaybackFunctionTest.class);
        suite.addTestSuite(AlbumBrowserFunctionTest.class);
        suite.addTestSuite(ArtistAlbumBrowserFunctionTest.class);
        suite.addTestSuite(MusicPickerFunctionTest.class);
        suite.addTestSuite(AudioPreviewFunctionTest.class);
        return suite;
    }

    @Override
    public ClassLoader getLoader() {
        return MusicFunctionalAutoTestRunner.class.getClassLoader();
    }
}
