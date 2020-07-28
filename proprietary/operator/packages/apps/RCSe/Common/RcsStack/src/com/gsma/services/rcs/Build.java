/*******************************************************************************
 * Software Name : RCS IMS Stack
 *
 * Copyright (C) 2010 France Telecom S.A.
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
 ******************************************************************************/
package com.gsma.services.rcs;



/**
 * This class offers information related to the build version of the API
 *
 * @author Jean-Marc AUFFRET
 */
public class Build {
    /**
     * List of version codes
     */
    public static class VERSION_CODES {
        /**
         * The original first version of joyn API
         */
        public final static int BASE = 1;

    }

    /**
     * List of GSMA codes
     */
    public static class GSMA_CODES {
        /**
         * joyn hotfixes version
         */
        public final static int RCSE_HOTFIXES_1_2 = 1;
        /**
         * joyn Blackbird version
         */
        public final static int RCSE_BLACKBIRD = 2;

    }

    /**
     * API release implementor name
     */
    public static final String API_CODENAME = "GSMA";

    /**
     * API version number
     *
     * @see Build.GSMA_CODES
     */
    public final static int GSMA_VERSION = GSMA_CODES.RCSE_BLACKBIRD;

    /**
     * GSMA version number from class Build.VERSION_CODES
     */
    public static final int API_VERSION = VERSION_CODES.BASE;

    /**
     * Internal number used by the underlying source control to represent this
     * build
     */
    public static final int API_INCREMENTAL = 0;

    private Build() {
    }
}
