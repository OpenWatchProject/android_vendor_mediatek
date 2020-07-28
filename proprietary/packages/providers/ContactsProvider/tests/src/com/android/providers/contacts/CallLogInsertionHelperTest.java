/*
 * Copyright (C) 2014 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package com.android.providers.contacts;

import java.util.Locale;

import android.content.ContentValues;
import android.content.Context;
import android.location.Country;
import android.location.CountryDetector;
import android.provider.CallLog.Calls;

/**
 * Test cases for {@link com.android.providers.contacts.DefaultCallLogInsertionHelper}.
 */
public class CallLogInsertionHelperTest extends FixedAndroidTestCase {

    /**
     * The default insertion helper under test.
     */
    private CallLogInsertionHelper mInsertionHelper =
            DefaultCallLogInsertionHelper.getInstance(this.getContext());

    /** M: Fix failed testcases when running case with available sim.
        Get the country Iso of device, run valid test case of the country @{ */
    private String mCountryIso;

    @Override
    protected void setUp() throws Exception {
        final CountryDetector countryDetector =
                (CountryDetector) mContext.getSystemService(Context.COUNTRY_DETECTOR);
        if (countryDetector != null) {
            mCountryIso = countryDetector.detectCountry().getCountryIso();
        } else {
            mCountryIso = Locale.getDefault().getCountry();
        }
    }
    /** @} */

    /**
     * Tests cases where valid, normalizable phone numbers are provided.
     */
    public void testValidNumber() {
        /// M: Check chinese phone number when insert chinese sim.
        if (mCountryIso != null) {
            if(mCountryIso.equals("CN")) {
                checkNormalization("136-5555-1212", "+8613655551212");
                checkNormalization("86136-5555-1212", "+8613655551212");
                checkNormalization("+8613655551212", "+8613655551212");
            } else if(mCountryIso.equals("US")) {
                checkNormalization("650-555-1212", "+16505551212");
                checkNormalization("1-650-555-1212", "+16505551212");
                checkNormalization("+16505551212", "+16505551212");
                checkNormalization("011-81-3-6384-9000", "+81363849000");
            }
        }
    }

    /**
     * Test cases where invalid unformatted numbers are entered.
     */
    public void testInvalidNumber() {
        checkNormalization("", null);

        /// M: Check chinese phone number when insert chinese sim.
        if (mCountryIso != null) {
            if(mCountryIso.equals("CN")) {
                // Invalid operator code.
                checkNormalization("111-5555-1212", null);
                // No area code.
                checkNormalization("555-1212", null);
                // Number as it should be dialed from Japan.
                checkNormalization("03-6384-9000", null);
                // SIP address
                checkNormalization("test@sip.org", null);
            } else if(mCountryIso.equals("US")) {
                // Invalid area code.
                checkNormalization("663-555-1212", null);
                // No area code.
                checkNormalization("555-1212", null);
                // Number as it should be dialed from Japan.
                checkNormalization("03-6384-9000", null);
                // SIP address
                checkNormalization("test@sip.org", null);
            }
        }
    }

    /**
     * Runs the DefaultCallLogInsertionHelper to determine if it produces the correct normalized
     * phone number.
     *
     * @param number The unformatted phone number.
     * @param expectedNormalized The expected normalized number.
     */
    private void checkNormalization(String number, String expectedNormalized) {
        ContentValues values = new ContentValues();
        values.put(Calls.NUMBER, number);
        mInsertionHelper.addComputedValues(values);

        assertEquals(expectedNormalized, values.getAsString(Calls.CACHED_NORMALIZED_NUMBER));
    }
}
