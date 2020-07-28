package com.android.music.tests.functional;

import junit.framework.TestSuite;
import android.test.InstrumentationTestRunner;
import android.test.InstrumentationTestSuite;

public class MusicFunctionalAutoTestByAnnotationTestRunner extends InstrumentationTestRunner {
    @Override
    public TestSuite getAllTests() {
        InstrumentationTestSuite suite = new InstrumentationTestSuite(this);
        suite.addTestSuite(MusicFunctionalAutoTestByAnnotation.class);
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
        return MusicFunctionalAutoTestByAnnotationTestRunner.class.getClassLoader();
    }
}
