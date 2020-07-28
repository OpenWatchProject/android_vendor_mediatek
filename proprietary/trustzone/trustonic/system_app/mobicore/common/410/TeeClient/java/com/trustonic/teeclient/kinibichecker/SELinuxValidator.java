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

import java.io.IOException;
import java.lang.reflect.InvocationTargetException;
import java.lang.reflect.Method;

import com.trustonic.teeclient.kinibichecker.Utils;

import android.os.Build;

public class SELinuxValidator {
    private final static int JELLY_BEAN_MR2 = 18;

    /**
     * Get SELinux mode if any
     * @return mode
     */
    protected String getSelinuxMode(){
        try {
            //Standard mount point in sys fs
            return Utils.readFile("/sys/fs/selinux/enforce");
        } catch (IOException e) {
            try {
                //Old mount point
                return Utils.readFile("/selinux/enforce");
            } catch (IOException io){
                return null;
            }
        }
    }

    /**
     * Check if SELinux is available
     * @return status
     */
    public boolean hasSELinuxBuiltIn() {
        String selinuxFlag = null;

        try {
            Class<?> c = Class.forName("android.os.SystemProperties");
            Method get = c.getMethod("get", String.class);
            selinuxFlag = (String) get.invoke(c, "ro.build.selinux");
        } catch (ClassNotFoundException e) {
            return false;
        } catch (NoSuchMethodException e) {
            return false;
        } catch (IllegalAccessException e) {
            return false;
        } catch (InvocationTargetException e) {
            return false;
        }

        if (selinuxFlag.length()>0) {
            return selinuxFlag.equals("1") ? true : false;
        } else {
            // 4.3 or later ?
            if(Build.VERSION.SDK_INT >= JELLY_BEAN_MR2) {
                return true;
            } else {
                return false;
            }
        }
    }

    /**
     * Check if SELinux is in enforcing mode
     * @return status
     */
    public boolean hasSELinuxEnforcing(){
        String seLinuxMode = getSelinuxMode();
        return (hasSELinuxBuiltIn() && (seLinuxMode!=null && seLinuxMode.contains("1")));
    }

    /**
     * Return the seLinux version number
     * @return seLinux version number
     */

    public String getSELinuxVersion(){
        String seLinuxVersion = "";

        try {
            seLinuxVersion = Utils.readFile("/sepolicy_version").split("[\r\n]+")[0].substring(3);
        } catch (IOException e) {
        }

        return seLinuxVersion;
    }
}
