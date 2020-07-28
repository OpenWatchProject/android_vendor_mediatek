package com.mediatek.omacp.tests;

import java.lang.annotation.ElementType;
import java.lang.annotation.Retention;
import java.lang.annotation.Target;
import java.lang.annotation.RetentionPolicy;
//use below command to run test case marked with annotation of "ExternalApiAnnotation"
//adb shell am instrument -w -e annotation com.android.mms.tests.annotation.ExternalApiAnnotation

@Retention(RetentionPolicy.RUNTIME)
@Target({ElementType.METHOD, ElementType.TYPE })
public @interface InternalApiAnnotation {
}
