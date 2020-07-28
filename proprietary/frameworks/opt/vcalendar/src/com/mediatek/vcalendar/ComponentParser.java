/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein is
 * confidential and proprietary to MediaTek Inc. and/or its licensors. Without
 * the prior written permission of MediaTek inc. and/or its licensors, any
 * reproduction, modification, use or disclosure of MediaTek Software, and
 * information contained herein, in whole or in part, shall be strictly
 * prohibited.
 *
 * MediaTek Inc. (C) 2010. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
 * ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 * NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH
 * RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 * INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES
 * TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
 * RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
 * OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN MEDIATEK
 * SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE
 * RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S
 * ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE
 * RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE
 * MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE
 * CHARGE PAID BY RECEIVER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek
 * Software") have been modified by MediaTek Inc. All revisions are subject to
 * any receiver's applicable license agreements with MediaTek Inc.
 */

package com.mediatek.vcalendar;

import com.mediatek.vcalendar.component.Component;
import com.mediatek.vcalendar.component.ComponentFactory;
import com.mediatek.vcalendar.parameter.Parameter;
import com.mediatek.vcalendar.parameter.ParameterFactory;
import com.mediatek.vcalendar.property.Property;
import com.mediatek.vcalendar.property.PropertyFactory;
import com.mediatek.vcalendar.utils.LogUtil;
import com.mediatek.vcalendar.utils.Utility;

/**
 * Parser for parsing a string into a iCalendar component.
 *
 */
public class ComponentParser {
    private static final String TAG = "ComponentParser";

    private ComponentParser() {
    }

    /**
     * line parser state.
     */
    private static final class ParserState {
        public String mLine;
        public int mIndex;
    }

    /**
     * Parses the provided text into an iCalendar component.
     *
     * @param componentString
     *            The text to be parsed.
     * @return The top-level VEVENT component.
     */
    public static Component parse(String componentString) {
        return parseComponentImpl(null, normalizeText(componentString));
    }

    private static String normalizeText(String text) {
        String normalizedText = text;
        // it's supposed to be \r\n, but not everyone does that
        normalizedText = normalizedText.replaceAll("\r\n", "\n");
        normalizedText = normalizedText.replaceAll("\r", "\n");

        // we deal with line folding, by replacing all "\n " strings
        // with nothing. The RFC specifies "\r\n " to be folded, but
        // we handle "\n " and "\r " too because we can get those.

        if (Utility.needQpEncode()) {
            // to remove QP-encoding soft crlf START
            normalizedText = normalizedText.replaceAll("=\n", "");
            // to remove QP-encoding soft crlf END
        }

        normalizedText = normalizedText.replaceAll("\n\t", "");
        normalizedText = normalizedText.replaceAll("\n ", "");

        LogUtil.d(TAG, "normalizeText(): normalized text: \n" + normalizedText);
        return normalizedText;
    }

    /*
     * Parses text into an iCalendar component. Parses content into the provided
     * component, if component not null, or parses content into a new component.
     * In the latter case, expects a BEGIN as the first line. Returns the
     * provided or newly created top-level component.
     */
    private static Component parseComponentImpl(Component component, String text) {
        Component current = component;
        Component finalComponent = component;
        ParserState state = new ParserState();
        state.mIndex = 0;

        // split into lines
        String[] lines = text.split("\n");

        // each line is of the format:
        // name *(";" param) ":" value
        for (String line : lines) {
            try {
                current = parseLine(line, state, current);
                // if the provided component was null, we will return the root
                // NOTE: in this case, if the first line is not a BEGIN, a
                // FormatException will get thrown.
                if (finalComponent == null) {
                    finalComponent = current;
                }
            } catch (VCalendarException fe) {
                LogUtil.e(TAG, "parseComponentImpl(): Can NOT parse line: "
                        + line, fe);
                // for now, we ignore the parse error. Calendar seems
                // to be emitting some misformatted iCalendar objects.
            }
        }

        return finalComponent;
    }

    /*
     * Parse a line into the provided component. Creates a new component if the
     * line is a BEGIN, adding the newly created component to the provided
     * parent. Returns whatever component is the current one (to which new
     * properties will be added) in the parse.
     */
    private static Component parseLine(String line, ParserState state,
            Component component) throws VCalendarException {
        state.mLine = line;
        int len = state.mLine.length();

        // grab the name
        char c = 0;
        for (state.mIndex = 0; state.mIndex < len; ++state.mIndex) {
            c = line.charAt(state.mIndex);
            if (c == ';' || c == ':') {
                break;
            }
        }

        String name = line.substring(0, state.mIndex);
        if (component == null) {
            if (!Component.BEGIN.equals(name)) {
                throw new VCalendarException("parseLine(): Expected \"BEGIN\" but NOT.");
            }
        }

        Property property;
        if (Component.BEGIN.equals(name)) {
            // start a new component
            String componentName = extractValue(state);
            Component child = ComponentFactory.createComponent(componentName, component);
            if (component != null) {
                component.addChild(child);
            }

            return child;
        } else if (Component.END.equals(name)) {
            // finish the current component
            String componentName = extractValue(state);
            if (component == null || componentName == null
                    || !componentName.equals(component.getName())) {
                throw new VCalendarException("parseLine(): Unexpected \"END\" in component: "
                        + componentName);
            }

            return component.getParent();
        } else {
            property = PropertyFactory.createProperty(name, null);
        }

        if (c == ';') {
            Parameter parameter = null;
            while ((parameter = extractParameter(state)) != null) {
                property.addParameter(parameter);
            }
        }

        String value = extractValue(state);
        if (value != null) {
            LogUtil.d(TAG, "parseLine(): property value = " + value);

            /*
             * move decode to here,it can use to all property who has encoding
             * parameter.
             */
            Parameter encodePara = property.getFirstParameter(Parameter.ENCODING);

            property.setValue(value, encodePara);
            component.addProperty(property);

            LogUtil.d(TAG, "parseLine(): " + "\"" + property.getName() + "\""
                    + " added to component:" + "\"" + component.getName() + "\"");
        }

        return component;
    }

    /*
     * Extracts the value ":..." on the current line. The first character must
     * be a ':'. If it is a invalid property and can not find value, return
     * null.
     */
    private static String extractValue(ParserState state)
            throws VCalendarException {
        String line = state.mLine;
        if (state.mIndex >= line.length() || line.charAt(state.mIndex) != ':') {
            LogUtil.d(TAG, "extractValue(): Expect ':' before end of line in: "
                    + line);
            return null;
        }

        String value = line.substring(state.mIndex + 1);
        state.mIndex = line.length() - 1;

        return value;
    }

    /*
     * Extracts the next parameter from the line if any. If there are no more
     * parameters, returns null.
     */
    private static Parameter extractParameter(ParserState state)
            throws VCalendarException {
        String text = state.mLine;
        int len = text.length();
        Parameter parameter = null;
        int startIndex = -1;
        int equalIndex = -1;

        while (state.mIndex < len) {
            char c = text.charAt(state.mIndex);
            if (c == ':') {
                if (parameter != null) {
                    if (equalIndex == -1) {
                        throw new VCalendarException("extractParameter(): Expected '=' within "
                                + "parameter in " + text);
                    }
                    parameter.setValue(text.substring(equalIndex + 1, state.mIndex));
                }
                // may be null
                return parameter;
            } else if (c == ';') {
                if (parameter == null) {
                    // the first time to meet ";" , the parameter's name haven't
                    // been gained, new a basic parameter , but it is useless.
                    startIndex = state.mIndex;
                } else {
                    if (equalIndex == -1) {
                        throw new VCalendarException("extractParameter(): Expected '=' within "
                                + "parameter in " + text);
                    }
                    parameter.setValue(text.substring(equalIndex + 1, state.mIndex));
                    return parameter;
                }
            } else if (c == '=') {
                if ((parameter == null) && (startIndex != -1)) {
                    equalIndex = state.mIndex;
                    // the first time to create a parameter, the value is not
                    // gained yet
                    parameter = ParameterFactory.createParameter(
                            text.substring(startIndex + 1, equalIndex), null);
                    startIndex = -1;
                } else {
                    //throw new FormatException("Expected one ';' before one '=' in " + text);
                    LogUtil.e(TAG, "extractParameter(): FormatException happened, Expected one ';' before one '=' in "
                            + text);
                }
            } else if (c == '"') {
                if (parameter == null) {
                    throw new VCalendarException("extractParameter(): Expected parameter before '\"' in "
                            + text);
                }
                if (equalIndex == -1) {
                    throw new VCalendarException("extractParameter(): Expected '=' within parameter in "
                            + text);
                }
                if (state.mIndex > equalIndex + 1) {
                    throw new VCalendarException("extractParameter(): Parameter value cannot contain a '\"' in "
                            + text);
                }
                final int endQuote = text.indexOf('"', state.mIndex + 1);
                if (endQuote < 0) {
                    throw new VCalendarException("extractParameter(): Expected closing '\"' in "
                            + text);
                }
                parameter.setValue(text.substring(state.mIndex + 1, endQuote));
                state.mIndex = endQuote + 1;
                return parameter;
            }

            ++state.mIndex;
        }

        throw new VCalendarException("extractParameter(): Expected ':' before end of line in "
                + text);
    }
}
