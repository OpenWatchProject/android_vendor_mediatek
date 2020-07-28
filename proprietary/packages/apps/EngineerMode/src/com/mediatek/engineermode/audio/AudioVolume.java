package com.mediatek.engineermode.audio;


import android.app.ListActivity;
import android.content.Intent;
import android.os.Bundle;
import android.view.View;
import android.widget.ArrayAdapter;
import android.widget.ListView;

import com.mediatek.engineermode.Elog;
import com.mediatek.engineermode.R;

import java.util.ArrayList;

/**
 * Activity for showing volume items.
 *
 */
public class AudioVolume extends ListActivity {

    public static final String TAG = "Audio/Volume";
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        // TODO Auto-generated method stub
        super.onCreate(savedInstanceState);
        setContentView(R.layout.audio_volume);
        ArrayList<String> itemList = new ArrayList<String>();
        itemList.add(getString(R.string.audio_volume_voice));
        itemList.add(getString(R.string.audio_volume_voip));
        itemList.add(getString(R.string.audio_volume_playback));
        itemList.add(getString(R.string.audio_volume_record));
        ArrayAdapter<String> moduleAdapter = new ArrayAdapter<String>(this,
                android.R.layout.simple_list_item_1, itemList);
        setListAdapter(moduleAdapter);
    }

    @Override
    protected void onListItemClick(ListView l, View v, int position, long id) {
        // TODO Auto-generated method stub
        switch (position) {
            case 0 : {
                Intent intent = new Intent(this, AudioVolumeVoice.class);
                startActivity(intent);
                return;
            }
            case 1 : {
                Intent intent = new Intent(this, AudioVolumeVoIP.class);
                startActivity(intent);
                return;
            }
            case 2 : {
                Intent intent = new Intent(this, AudioVolumePlayback.class);
                startActivity(intent);
                return;
            }
            case 3 : {
                Intent intent = new Intent(this, AudioVolumeRecord.class);
                startActivity(intent);
                return;
            }
        default:
            break;

        }
    }

}
