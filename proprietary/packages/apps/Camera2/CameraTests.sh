#!/bin/sh
#
# ---------------------------------------------------------------------
# Camera Enhancement Run Tests with Gradle
# ---------------------------------------------------------------------
#
export PATH=~/Tools/AndroidStudio/android-studio-2.2-rc/android-studio/gradle/gradle-2.14.1/bin:~/Tools/AndroidStudio/android-studio-2.2-rc/android-studio/jre/bin:$PATH

#Unit test and code coverage report
gradle testDebugUnitTest
gradle jacocoUnitTestReport

#Instrumentation test and code coverage report
gradle jacocoAndroidTest
gradle jacocoAndroidTestReport

#All test jacoco report
gradle jacocoAllTestReport
