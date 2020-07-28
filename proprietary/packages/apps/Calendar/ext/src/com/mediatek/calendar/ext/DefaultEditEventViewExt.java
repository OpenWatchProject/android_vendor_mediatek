package com.mediatek.calendar.ext;

import android.app.Activity;
import android.text.format.Time;
import android.widget.Button;
import android.widget.RadioGroup;

import com.android.datetimepicker.date.DatePickerDialog;
import com.android.datetimepicker.date.DatePickerDialog.OnDateSetListener;

public class DefaultEditEventViewExt implements IEditEventViewExt {

  /*  @Override
    public void updateDatePickerSelection(Activity activity, Object model,
            RadioGroup radioGroup, int switch_lunar_id,  int switch_gregorian_id,
            Button startDateButton, Button endDateButton, String timezone,
            Time startTime, Time endTime) {

    }*/

    @Override
    public void setDatePickerSwitchUi(Activity activity, Object model, Button startDateButton,
            Button endDateButton, String timezone, Time startTime, Time endTime) {

    }

    @Override
    public String getDateStringFromMillis(Activity activity, long millis) {
        return "";
    }

    @Override
    public DatePickerDialog createDatePickerDialog(Activity activity, OnDateSetListener listener,
            int year, int monthOfYear, int dayOfMonth) {
        return DatePickerDialog.newInstance(listener, year, monthOfYear, dayOfMonth);
    }
}
