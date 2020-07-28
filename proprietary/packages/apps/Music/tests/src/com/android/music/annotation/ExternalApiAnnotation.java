package com.android.music.tests.annotation;

import java.lang.annotation.ElementType;
import java.lang.annotation.Retention;
import java.lang.annotation.RetentionPolicy;
import java.lang.annotation.Target;


// use below command to run test case marked with annotation of "ExternalApiAnnotation"
// adb shell am instrument  -w -e annotation com.android.settings.tests.functional.
//annotation.ExternalApiAnnotationn com.android.settings.tests/.functional.MTKFunctionalTestRunner

@Retention(RetentionPolicy.RUNTIME)
@Target({ElementType.METHOD, ElementType.TYPE })
public @interface ExternalApiAnnotation {

}
