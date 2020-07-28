/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2010. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
 * AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek Software")
 * have been modified by MediaTek Inc. All revisions are subject to any receiver's
 * applicable license agreements with MediaTek Inc.
 */

package com.mediatek.engineermode;

import com.mediatek.engineermode.emsvr.AFMFunctionCallEx;
import com.mediatek.engineermode.emsvr.FunctionReturn;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStreamReader;
import java.nio.charset.Charset;

/**
 * Class for shell command control.
 *
 */
public class ShellExe {

    private static final String TAG = "ShellExe";
    public static final String ERROR = "ERROR";
    public static final int RESULT_SUCCESS = 0;
    public static final int RESULT_FAIL = -1;
    public static final int RESULT_EXCEPTION = -2;
    private static final String OPERATION_ERROR_PREFIX = "#$ERROR^&";
    private static StringBuilder sResultBuilder = new StringBuilder("");

    /**
     * Get shell command output.
     *
     * @return Shell command output
     */
    public static String getOutput() {
        return sResultBuilder.toString();
    }

    /**
     * Execute shell command.
     * @param command Command string to execute
     * @return Result
     * @throws IOException Throws when occurs #IOException
     */
    public static int execCommand(String command) throws IOException {
        return execCommand(new String[] { "sh", "-c", command });
    }

    /**
     * Execute shell command.
     * @param command Command array to execute
     * @return command result
     * @throws IOException Throws when occurs #IOException
     */
    public static int execCommand(String[] command) throws IOException {
        return execCommandOnServer(command);
    }

    /**
     * Execute shell command.
     * @param command Command string to execute
     * @param execOnLocal True if the command is sent by EM application. False for em_svr.
     * @return command result
     * @throws IOException Throws when occurs #IOException
     */
    public static int execCommand(String command, boolean execOnLocal) throws IOException {
        return execCommand(new String[] {"sh", "-c", command}, execOnLocal);
    }

    /**
     * Execute shell command.
     * @param command Command array to execute
     * @param execOnLocal True if the command is sent by EM application. False for em_svr.
     * @return command result
     * @throws IOException Throws when occurs #IOException
     */
    private static int execCommand(String[] command, boolean execOnLocal) throws IOException {
        if (execOnLocal) {
            return execCommandLocal(command);
        } else {
            return execCommandOnServer(command);
        }
    }

    /**
     * Execute shell command.
     * @param command Command array to execute
     * @return Result
     * @throws IOException Throws when occurs #IOException
     */
    private static int execCommandLocal(String[] command) throws IOException {
        int result = RESULT_FAIL;
        Runtime runtime = Runtime.getRuntime();
        Process proc = runtime.exec(command);
        BufferedReader bufferedReader = null;
        sResultBuilder.delete(0, sResultBuilder.length());
        try {
            bufferedReader = new BufferedReader(new InputStreamReader(proc
                    .getInputStream(), Charset.defaultCharset()));
            if (proc.waitFor() == 0) {
                String line = bufferedReader.readLine();
                if (line != null) {
                    sResultBuilder.append(line);
                    while (true) {
                        line = bufferedReader.readLine();
                        if (line == null) {
                            break;
                        } else {
                            sResultBuilder.append('\n');
                            sResultBuilder.append(line);
                        }
                    }
                }
                result = RESULT_SUCCESS;
            } else {
                Elog.e(TAG, "exit value = " + proc.exitValue());
                sResultBuilder.append(ERROR);
                result = RESULT_FAIL;
            }
        } catch (InterruptedException e) {
            Elog.e(TAG, "exe shell command InterruptedException: "
                    + e.getMessage());
            sResultBuilder.append(ERROR);
            result = RESULT_EXCEPTION;
        } finally {
            if (null != bufferedReader) {
                try {
                    bufferedReader.close();
                } catch (IOException e) {
                    Elog.e(TAG, "close reader in finally block exception: " + e.getMessage());
                }
            }
        }
        return result;
    }

    /**
     * Execute shell command.
     * @param command Command string to execute
     * @return command result
     * @throws IOException Throws when occurs #IOException
     */
    private static int execCommandOnServer(String[] command) throws IOException {
        if (command == null || command.length == 0) {
            throw new IllegalArgumentException("Invalid shell command to execute");
        }
        StringBuilder cmdBuilder = new StringBuilder();
        int i = 0;
        // ignore specified sh, use the server default sh
        if ("sh".equals(command[0]) || command[0].endsWith("/sh")) {
            if (command.length < 3) {
                throw new IllegalArgumentException("invalid or unknown cmd to execute");
            }
            i = 2;
        }
        for (; i < command.length; i++) {
            cmdBuilder.append(command[i]);
            if (i < command.length - 1) {
                cmdBuilder.append(" ");
            }
        }
        String cmd = cmdBuilder.toString();
        cmd = cmd.trim();
        sResultBuilder.delete(0, sResultBuilder.length());
        boolean ret = true;
        AFMFunctionCallEx functionCall = new AFMFunctionCallEx();
        ret = functionCall.startCallFunctionStringReturn(
                AFMFunctionCallEx.FUNCTION_EM_SHELL_CMD_EXECUTION);
        if (ret) {
            functionCall.writeParamNo(1);
            functionCall.writeParamString(cmd);
            FunctionReturn funcRet = null;
            do {
                funcRet = functionCall.getNextResult();
                if (funcRet.mReturnString != null && funcRet.mReturnString.length() > 0) {
                    sResultBuilder.append(funcRet.mReturnString);
                }
            } while (funcRet.mReturnCode == AFMFunctionCallEx.RESULT_CONTINUE);
            // trim the tail newline character
            while (true) {
                int len = sResultBuilder.length();
                if (len <= 0) {
                    break;
                }
                char c = sResultBuilder.charAt(len - 1);
                if (c == '\n') {
                    sResultBuilder.delete(len - 1, len);
                } else {
                    break;
                }
            }
            String output = sResultBuilder.toString();
            if (output != null && output.startsWith(OPERATION_ERROR_PREFIX)) {
                Elog.e(TAG, "error operation:" + output);
                sResultBuilder.delete(0, OPERATION_ERROR_PREFIX.length() + 1);
                return RESULT_FAIL;
            }
            return RESULT_SUCCESS;
        }
        Elog.e(TAG, "Function call start fail");
        sResultBuilder.append(ERROR);
        return RESULT_FAIL;
    }
}
