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
import java.io.*;

/**
 * Class to ease data conversion
 *
 */
public class Utils {

    /**
     * Convert integer to hex string
     * @param intNum integer
     * @return string
     */
    public static String intToHex(int intNum){
        StringBuilder sb = new StringBuilder();
        sb.append(Integer.toHexString(intNum));
        if (sb.length() < 2) {
            sb.insert(0, '0'); // pad with leading zero if needed
        }
        return sb.toString();
    }
    /**
     * Read the content of a file
     * @param file file to read
     * @return string
     * @throws IOException
     */
    public static String readFile( String file ) throws IOException  {
        BufferedReader reader = new BufferedReader(new InputStreamReader(new FileInputStream(file), "UTF-8"));
        String line = null;
        StringBuilder stringBuilder = new StringBuilder();
        String ls = System.getProperty("line.separator");

        try {
            while( ( line = reader.readLine() ) != null ) {
                stringBuilder.append( line );
                stringBuilder.append( ls );
            }
        } catch(IOException e) {
            reader.close();
            throw e;
        }
        reader.close();

        return stringBuilder.toString();
    }
    /**
     * Execute a process
     * @param inDirectory target directory
     * @param commands process parameters
     * @return status
     */
    public static String executeProcess(String inDirectory, String... commands){
        StringBuffer sb = new StringBuffer();
        ProcessBuilder processBuilder = null;

        if (inDirectory!=null){
            //Execute process in directory
            processBuilder = new ProcessBuilder(commands).directory(new File(inDirectory));
        } else {
            //Execute process on cwd
            processBuilder = new ProcessBuilder(commands);
        }

        Process process;
        try {
            process = processBuilder.start();
            // We can't use StandardCharsets.UTF_8 because of the minSdkVersion which is too low
            PrintWriter out = new PrintWriter(new OutputStreamWriter(process.getOutputStream(), "UTF-8"));
            BufferedReader in = new BufferedReader(new InputStreamReader(process.getInputStream(), "UTF-8"));
            out.flush();
            String resultLine = null;
            do {
                resultLine = in.readLine();
                if (resultLine != null) sb.append(resultLine);
            } while (resultLine != null);
        } catch (IOException e) {
            //e.printStackTrace();
            return null;
        }
        return sb.toString();
    }

    /**
     * Check folder permissions
     * @param directoryName folder name
     * @param SELinuxFlags choose to display SELinux info
     * @return status
     */
    public static String checkFolderPermissions(String directoryName, boolean SELinuxFlags){
        return executeProcess(directoryName, "ls", (SELinuxFlags)?"-ldZ":"-ld");
    }

    /**
     * Check file permissions
     * @param fileName file name
     * @param SELinuxFlags choose to display SELinux info
     * @return status
     */
    public static String checkFilePermissions(String fileName, boolean SELinuxFlags){
        return executeProcess(null, "ls", ((SELinuxFlags)?"-ldZ":"-ld"),fileName);
    }

    /**
     * Check file availability
     * @param fileName file name
     * @return status
     */
    public static boolean checkFileAvailability(String fileName){
        if (fileName != null){
            String out = executeProcess(null, "ls","-l",fileName);
            if (out != null){
                try{
                    return out.contains(fileName.substring(fileName.lastIndexOf("/")+1,fileName.length()));
                } catch (Exception e){
                    return false;
                }
            }
        }
        return false;
    }

    /**
     * Check if the device is arndale
     * @param processName process name
     * @return status
     */
    public static boolean checkArndale(){
        String arndale = executeProcess(null, "cat","/proc/cpuinfo");
        if (null != arndale){
            return arndale.contains("ARNDALE");
        } else {
            return false;
        }
    }

    /**
     * Count the numbers of occurences of a string inside another
     * @param str base string
     * @param strToSearch string to be searched
     * @return counter
     */
    public static int countOccurences(String str, String strToSearch){
        if (str == null || str.length() <1 || strToSearch==null || strToSearch.length() <1) {
            return 0;
        }
        int count = 0;
        int idx = 0;
        while ((idx = str.indexOf(strToSearch, idx)) != -1) {
            count++;
            idx += strToSearch.length();
        }
        return count;
    }

}
