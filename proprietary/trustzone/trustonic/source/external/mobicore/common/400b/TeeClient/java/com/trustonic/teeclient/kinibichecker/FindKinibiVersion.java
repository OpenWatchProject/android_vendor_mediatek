/*
 * Copyright (c) 2016-2018 TRUSTONIC LIMITED
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the TRUSTONIC LIMITED nor the names of its
 *    contributors may be used to endorse or promote products derived from
 *    this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
 * TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
package com.trustonic.teeclient.kinibichecker;

public class FindKinibiVersion {

    private String mId=null;

    private String[] compatibilityMatrixTEEVersion = {
        "tbase-200_EXYNOS5410_V005_Patch6(MC20_EXYNOS5410_V005_Patch6)",
        "tbase-200_EXYNOS4X12_V004(MC20_EXYNOS_4X12_V004)",
        "tbase-200_EXYNOS_4X12_V002(MC20_EXYNOS_4X12_V002)",
        "*** t-base-300-QC-8974-Android-V003 ###",
        "t-base-202-QC-8974-Android-v001",
        "tbase-202-QC-V001-fixMCTWO2033-forTests"
    };

    public FindKinibiVersion(String id) {
        mId=id;
    }

    public int getResult()
    {
        if (mId == null) return -1;
        else
            return find_index(compatibilityMatrixTEEVersion, mId);
    }

    private int find_index(String[] arr, String s) {
        for (int i=0; i< arr.length; i++)
        {
            if(arr[i].equalsIgnoreCase(s) )
                return i+1;
        }

        return -1;
    }
}

