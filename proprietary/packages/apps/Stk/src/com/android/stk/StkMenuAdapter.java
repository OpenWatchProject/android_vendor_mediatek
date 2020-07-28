/*
 * Copyright (C) 2008 The Android Open Source Project
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

package com.android.stk;

import com.android.internal.telephony.cat.Item;

import android.content.Context;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.ArrayAdapter;
import android.widget.ImageView;
import android.widget.TextView;

import java.util.List;
import com.android.internal.telephony.cat.AppInterface;
import com.mediatek.internal.telephony.cat.MtkAppInterface;

/**
 * Icon list view adapter to show the list of STK items.
 */
public class StkMenuAdapter extends ArrayAdapter<Item> {
    private final LayoutInflater mInflater;
    private boolean mIcosSelfExplanatory = false;
    private byte[] mNextActionIndicator = null;

    public StkMenuAdapter(Context context, List<Item> items, byte[] nextActionIndicator,
            boolean icosSelfExplanatory) {
        super(context, 0, items);
        mInflater = LayoutInflater.from(context);
        mIcosSelfExplanatory = icosSelfExplanatory;
        mNextActionIndicator = nextActionIndicator;
    }

    @Override
    public View getView(int position, View convertView, ViewGroup parent) {
        final Item item = getItem(position);
        String summaryStr = null;

        if (convertView == null) {
            convertView = mInflater.inflate(R.layout.stk_menu_item, parent,
                    false);
        }

        if (!mIcosSelfExplanatory || (mIcosSelfExplanatory && item.icon == null)) {
            if (mNextActionIndicator != null) {
                MtkAppInterface.CommandType type = MtkAppInterface.CommandType
                        .fromInt(mNextActionIndicator[position]);
                if (type != null) {
                    summaryStr = getSummary(type);
                }
            }
            if (summaryStr != null) {
                convertView = mInflater.inflate(R.layout.stk_menu_item2, parent, false);
                ((TextView) convertView.findViewById(R.id.text)).setText(item.text);
                ((TextView) convertView.findViewById(R.id.summary)).setText(summaryStr);

            } else {
                convertView = mInflater.inflate(R.layout.stk_menu_item, parent, false);
                ((TextView) convertView.findViewById(R.id.text)).setText(item.text);
            }
        } else {
            convertView = mInflater.inflate(R.layout.stk_menu_item, parent,
                    false);
            ((TextView) convertView.findViewById(R.id.text)).setVisibility(View.GONE);
        }

        ImageView imageView = ((ImageView) convertView.findViewById(R.id.icon));
        if (item.icon == null) {
            imageView.setVisibility(View.GONE);
        } else {
            imageView.setImageBitmap(item.icon);
            imageView.setVisibility(View.VISIBLE);
        }

        return convertView;
    }

    private String getSummary(MtkAppInterface.CommandType cmdType){
        int strId = 0;
        String summaryString = null;
        switch (cmdType) {
            case SET_UP_MENU:
                strId = R.string.lable_setup_menu;
                break;
            case DISPLAY_TEXT:
                strId = R.string.lable_display_text;
                break;
            case REFRESH:
                strId = R.string.lable_refresh;
                break;
            case SET_UP_IDLE_MODE_TEXT:
                strId = R.string.lable_setup_idle_modetext;
                break;
            case LAUNCH_BROWSER:
                strId = R.string.lable_launch_browser;
                break;
            case SELECT_ITEM:
                strId = R.string.lable_select_item;
                break;
            case GET_INPUT:
                strId = R.string.lable_get_input;
                break;
            case GET_INKEY:
                strId = R.string.lable_get_inkey;
                break;
            case SEND_DTMF:
                strId = R.string.lable_send_dtmf;
                break;
            case SET_UP_EVENT_LIST:
                strId = R.string.lable_setup_event_list;
                break;
            case SEND_SMS:
                strId = R.string.lable_send_sms;
                break;
            case SEND_SS:
                strId = R.string.lable_send_ss;
                break;
            case SEND_USSD:
                strId = R.string.lable_send_ussd;
                break;
            case PLAY_TONE:
                strId = R.string.lable_play_tone;
                break;
            case SET_UP_CALL:
                strId = R.string.lable_setup_call;
                break;
            case MORE_TIME:
                strId = R.string.lable_more_time;
                break;
            case POLL_INTERVAL:
                strId = R.string.lable_poll_interval;
                break;
            case POLLING_OFF:
                strId = R.string.lable_polling_off;
                break;
            case PROVIDE_LOCAL_INFORMATION:
                strId = R.string.lable_provide_local_information;
                break;
            case TIMER_MANAGEMENT:
                strId = R.string.lable_timer_management;
                break;
            case PERFORM_CARD_APDU:
                strId = R.string.lable_perform_card_apdu;
                break;
            case POWER_ON_CARD:
                strId = R.string.lable_power_on_card;
                break;
            case POWER_OFF_CARD:
                strId = R.string.lable_power_off_card;
                break;
            case GET_READER_STATUS:
                strId = R.string.lable_get_reader_status;
                break;
            case RUN_AT:
                strId = R.string.lable_run_at_command;
                break;
            case LANGUAGE_NOTIFICATION:
                strId = R.string.lable_language_notification;
                break;
            case OPEN_CHANNEL:
                strId = R.string.lable_open_channel;
                break;
            case CLOSE_CHANNEL:
                strId = R.string.lable_close_channel;
                break;
            case RECEIVE_DATA:
                strId = R.string.lable_receive_data;
                break;
            case SEND_DATA:
                strId = R.string.lable_send_data;
                break;
            case GET_CHANNEL_STATUS:
                strId = R.string.lable_get_channel_status;
                break;
            case SERVICE_SEARCH:
                strId = R.string.lable_service_search;
                break;
            case GET_SERVICE_INFORMATION:
                strId = R.string.lable_get_service_information;
                break;
            case DECLARE_SERVICE:
                strId = R.string.lable_declare_service;
                break;
            case SET_FRAME:
                strId = R.string.lable_set_frame;
                break;
            case GET_FRAME_STATUS:
                strId = R.string.lable_get_frame_status;
                break;
            case RETRIEVE_MULTIMEDIA_MESSAGE:
                strId = R.string.lable_retrieve_multimedia_message;
                break;
            case SUBMIT_MULTIMEDIA_MESSAGE:
                strId = R.string.lable_submit_multimedia_message;
                break;
            case DISPLAY_MULTIMEDIA_MESSAGE:
                strId = R.string.lable_display_multimedia_message;
                break;
            case ACTIVATE:
                strId = R.string.lable_activate;
                break;
            default:
                break;
        }
        if (strId != 0) {
            summaryString = this.getContext().getString(strId);
            return summaryString;
        }
        return null;
    }
}
