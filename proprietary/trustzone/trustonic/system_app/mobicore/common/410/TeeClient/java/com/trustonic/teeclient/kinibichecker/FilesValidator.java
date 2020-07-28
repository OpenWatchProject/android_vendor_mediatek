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

import com.trustonic.teeclient.kinibichecker.Constants;
import com.trustonic.teeclient.kinibichecker.Utils;

/**
 * Files and registry status
 *
 */
public class FilesValidator {

    /**
     * Check if MCRegistry has the right permissions and user
     * @return status
     */

    private boolean isValidMcRegistryPermissions(String mcRegistryPath)
    {
        String permissions = Utils.checkFolderPermissions(mcRegistryPath,false);
        return ( (permissions != null)
                && (permissions.startsWith("drwxrwxr-x") || (Utils.countOccurences(permissions, "rw") == 3))
                && ((Utils.countOccurences(permissions, "system") >= 2) || (Utils.countOccurences(permissions, "rw") == 3)) );
    }

    public boolean hasValidMcRegistryPermissionsExt()
    {
        return (isValidMcRegistryPermissions(Constants.MC_REGISTRY_PATH)
            || isValidMcRegistryPermissions(Constants.MC_REGISTRY_PATH_2)
            || isValidMcRegistryPermissions(Constants.MC_REGISTRY_PATH_3) );
    }

    /**
     * Check system mCRegistry permissions
     * @return status
     */
    public String getMcSystemRegistryPermissions(){
        return Utils.checkFolderPermissions(Constants.MC_SYSTEM_REGISTRY_PATH, false);
    }

    /**
     * Check if system MCRegistry has the right permissions and user
     * @return status
     */
    public boolean hasValidMcSystemRegistryPermissions(){
        String permissions = getMcSystemRegistryPermissions();
        return (permissions != null)
                && (Utils.countOccurences(permissions, "rw") >= 1)
                && ((Utils.countOccurences(permissions, "root") >= 2) || (Utils.countOccurences(permissions, "system") >= 2) || (Utils.countOccurences(permissions, "rw") == 3));
    }

    /**
     * Diplay MCSockets state
     * @return status
     */
    public String getMcSocketsPermissions(boolean dumpFormatting){
        StringBuffer sb = new StringBuffer();
        sb.append(Utils.checkFilePermissions(Constants.MC_SOCKETS_MOBICORE_PATH, false));
        if (dumpFormatting){
            sb.append(";");
        } else {
            sb.append(System.getProperty("line.separator"));
        }
        sb.append(Utils.checkFilePermissions(Constants.MC_SOCKETS_MOBICORE_USER_PATH, false));
        return sb.toString();
    }

    /**
     * Check if MCSockets have the right permissions and user
     * @return status
     */
    public boolean hasValidMcNodePermissions(){
        String permissionsMobicore = Utils.checkFilePermissions(Constants.MC_SOCKETS_MOBICORE_PATH, false);
        String permissionsMobicoreuser = Utils.checkFilePermissions(Constants.MC_SOCKETS_MOBICORE_USER_PATH, false);


        return (permissionsMobicore != null) && (permissionsMobicoreuser != null)
                && (//mobicore
                        (Utils.countOccurences(permissionsMobicoreuser, "rw") >= 3)
                        || (permissionsMobicore.startsWith("crwx") && (Utils.countOccurences(permissionsMobicore, "system") >= 2))
                    )

                && (//mobicore-user
                        (Utils.countOccurences(permissionsMobicoreuser, "rw") >= 3)
                        || (permissionsMobicoreuser.startsWith("crw-rw-rw-") && (Utils.countOccurences(permissionsMobicoreuser, "system") >= 1))
                        || ((Utils.countOccurences(permissionsMobicoreuser, "rw") >= 2) && (Utils.countOccurences(permissionsMobicoreuser, "system") >= 1))
                    );
    }
    public boolean hasValidRootPA(){
        return Utils.checkFileAvailability(Constants.ROOTPA_COMMONWRAPPER)
                && (Utils.checkFileAvailability(Constants.ROOTPA_CURL) || Utils.checkFileAvailability(Constants.ROOTPA_CURL_64));
    }

    public boolean hasTUI(){
        //TUI service will run at runtime : 310C or higher.
        //Check tui driver only.
        if ( Utils.checkFileAvailability(Constants.TUI_DRIVER_PATH) )
            return true;
        else
            return false;
    }
}
