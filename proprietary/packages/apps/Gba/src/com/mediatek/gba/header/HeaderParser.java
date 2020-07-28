/*
 * Copyright (C) 2012 Square, Inc.
 * Copyright (C) 2011 The Android Open Source Project
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
package com.mediatek.gba.header;

import android.util.Log;

/**
 * Utility class for HTTP header parse.
 *
 * @hide
 */
public final class HeaderParser {
    private static final String TAG = "GbaBsfProcedure";

  /**
   * Returns the next index in {@code input} at or after {@code pos} that
   * contains a character from {@code characters}. Returns the input length if
   * none of the requested characters can be found.
   * @param input the search string.
   * @param pos start position of string.
   * @param characters the target string.
   * @return the position of search characters.
   */
  public static int skipUntil(String input, int pos, String characters) {
    for (; pos < input.length(); pos++) {
      if (characters.indexOf(input.charAt(pos)) != -1) {
        break;
      }
    }
    return pos;
  }

  /**
   * Returns the next non-whitespace character in {@code input} that is white
   * space. Result is undefined if input contains newline characters.
   * @param input the search string.
   * @param pos start position of string.
   * @return the position of search characters.
   */
  public static int skipWhitespace(String input, int pos) {
    for (; pos < input.length(); pos++) {
      char c = input.charAt(pos);
      if (c != ' ' && c != '\t') {
        break;
      }
    }
    return pos;
  }

  /**
   * Returns {@code value} as a positive integer, or 0 if it is negative, or
   * -1 if it cannot be parsed.
   * @param value the value to be converted.
   * @return the integer value of string. Otherwise, -1 if it could not be parsed.
   */
  public static int parseSeconds(String value) {
    try {
      long seconds = Long.parseLong(value);
      if (seconds > Integer.MAX_VALUE) {
        return Integer.MAX_VALUE;
      } else if (seconds < 0) {
        return 0;
      } else {
        return (int) seconds;
      }
    } catch (NumberFormatException e) {
      return -1;
    }
  }


  /**
   * Returns the string without double quotes.
   * @param input the search string.
   * @param flag reserved for furture use.
   * @param pos start position of string.
   * @return the string without double quotes.
   */
  public static String getQuoteString(String input, String flag, int pos) {
      int posStart = skipUntil(input, pos, "\"") + 1;
      int posEnd   = skipUntil(input, posStart + 1, "\"");
      String value = "";


      Log.i(TAG, posStart + "/" + posEnd + "/" + input.length());
      if (posEnd > input.length()) return "";

      value = input.substring(posStart, posEnd);
      Log.i(TAG, flag + ":" + value);
      return value;
  }

  private HeaderParser() {
  }
}
