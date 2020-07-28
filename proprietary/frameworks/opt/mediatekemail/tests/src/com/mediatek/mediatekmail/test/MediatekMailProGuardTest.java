 /* Execute
 * adb shell am instrument -w -e class com.mediatek.mediatekmail.test.MediatekMailProGuardTest com.mediatek.mediatekmail.test/android.test.InstrumentationTestRunner
 */

package com.mediatek.mediatekmail.test;

import android.test.AndroidTestCase;
import android.util.Log;
import java.lang.reflect.Method;

//Target Package com.mediatek.protect.emailcommon.utility; MediatekMail
import com.mediatek.protect.emailcommon.utility.ExtendBase64InputStream;
import java.io.InputStream;
import java.io.ByteArrayInputStream;

public class MediatekMailProGuardTest extends AndroidTestCase {
    private static final String TAG = "MediatekMailProGuardTest";
    private boolean MediatekMailObjNameObfuscated = true;
    private boolean MediatekMailAMethodName = false;

    @Override
    protected void setUp() throws Exception {
        super.setUp();
        Log.i(TAG, " Setup ProGuard Test Case ");

    }

    @Override
    protected void tearDown() throws Exception {
        super.tearDown();
        Log.i(TAG, " TearDown ProGuard Test Case ");
    }

  public void testMediatekMailProGuard() throws Throwable {
      String charAtoZ = "abcdefghijklmnopqrstuvwxyz";
      Log.i(TAG, " testMediatekMailProGuard ");

      InputStream instream = new ByteArrayInputStream("testMediatekMailProGuard".getBytes());
      ExtendBase64InputStream checkClass = new ExtendBase64InputStream(instream, 0);
      Method[] methods = checkClass.getClass().getDeclaredMethods();

      for (Method checkMethodName : methods) {
          Log.i(TAG, "ExtendBase64InputStream checkMethodName = " + checkMethodName.getName());
          if (MediatekMailObjNameObfuscated && checkMethodName.getName().equals("refill")
            && (checkMethodName.getParameterTypes().length == 0)) {
            MediatekMailObjNameObfuscated = false;
          }

          // check method name "a" ~ "z"
          for (int i = 0; i < charAtoZ.length(); i++)
          {
              String testMethodName = "" + charAtoZ.charAt(i);
              if (!MediatekMailAMethodName && checkMethodName.getName().equals(testMethodName)) {
                  MediatekMailAMethodName = true;
                  break;
              } //if
          } //for

          // check method name "aa" ~ "zz"
          for (int i = 0; i < charAtoZ.length(); i++)
              for (int j = 0; j < charAtoZ.length(); j++)
              {
                  String testMethodName = "" + charAtoZ.charAt(i) + charAtoZ.charAt(j);
                  if (!MediatekMailAMethodName && checkMethodName.getName().equals(testMethodName)) {
                      MediatekMailAMethodName = true;
                      break;
                  } //if
              } //for
      }

      Log.i(TAG, "MediatekMail checkMethodName = refill is obfusted : " + MediatekMailObjNameObfuscated);
      Log.i(TAG, "MediatekMail checkMethodName = a is               : " + MediatekMailAMethodName);
      assertTrue("MediatekMail object name is not obfuscated", (MediatekMailObjNameObfuscated & MediatekMailAMethodName));

  }

}

