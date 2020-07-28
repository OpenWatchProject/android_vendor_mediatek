/*
 * Copyright (C) 2009 The Android Open Source Project
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

#ifndef ANDROID_AUDIOPOLICYINTERFACE_MTK_H
#define ANDROID_AUDIOPOLICYINTERFACE_MTK_H

#include <hardware_legacy/AudioPolicyInterface.h>

namespace android_audio_legacy {
using android::Vector;
using android::String8;
using android::ToneGenerator;

// ----------------------------------------------------------------------------

// The AudioPolicyInterface and AudioPolicyClientInterface classes define the communication interfaces
// between the platform specific audio policy manager and Android generic audio policy manager.
// The platform specific audio policy manager must implement methods of the AudioPolicyInterface class.
// This implementation makes use of the AudioPolicyClientInterface to control the activity and
// configuration of audio input and output streams.
//
// The platform specific audio policy manager is in charge of the audio routing and volume control
// policies for a given platform.
// The main roles of this module are:
//   - keep track of current system state (removable device connections, phone state, user requests...).
//   System state changes and user actions are notified to audio policy manager with methods of the AudioPolicyInterface.
//   - process getOutput() queries received when AudioTrack objects are created: Those queries
//   return a handler on an output that has been selected, configured and opened by the audio policy manager and that
//   must be used by the AudioTrack when registering to the AudioFlinger with the createTrack() method.
//   When the AudioTrack object is released, a putOutput() query is received and the audio policy manager can decide
//   to close or reconfigure the output depending on other streams using this output and current system state.
//   - similarly process getInput() and putInput() queries received from AudioRecord objects and configure audio inputs.
//   - process volume control requests: the stream volume is converted from an index value (received from UI) to a float value
//   applicable to each output as a function of platform specific settings and current output route (destination device). It
//   also make sure that streams are not muted if not allowed (e.g. camera shutter sound in some countries).
//
// The platform specific audio policy manager is provided as a shared library by platform vendors (as for libaudio.so)
// and is linked with libaudioflinger.so


//    Audio Policy Manager Interface
class AudioMTKPolicyInterface: public AudioPolicyInterface {
public:
    virtual ~AudioMTKPolicyInterface() {}
    virtual status_t  SetPolicyManagerParameters(int par1, int par2, int par3, int par4) = 0;
};

extern "C" AudioMTKPolicyInterface *createAudioMTKPolicyManager(AudioPolicyClientInterface *clientInterface);
extern "C" void destroyAudioMTKPolicyManager(AudioMTKPolicyInterface *interface);


}; // namespace android

#endif // ANDROID_AUDIOPOLICYINTERFACE_MTK_H
