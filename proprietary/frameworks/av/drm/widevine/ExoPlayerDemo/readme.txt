ExoPlayerDemo.apk can be used to do end-to-end verification of Modular DRM.

To install, side load ExoPlayerDemo.apk app to your device:

  adb install ExoPlayerDemo.apk

To run, launch ExoPlayer, then choose the clip to play.  The
   Widevine-encrypted DASH CENC assets are in the "WIDEVINE DASH GTS"
   section.

   These assets test various configurations of the Key Control Block (KCB)
   with various protections and expirations:

   WV: HDCP not specified (KCB: Observe_HDCP=false)
   WV: HDCP not required (KCB: Observe_HDCP=true && HDCP=not required && DataPath=normal)
   WV: HDCP required (KCB: Observe_HDCP=true && HDCP=required && DataPath=normal)
   WV: Secure video path required (KCB: Observe_HDCP=true && HDCP=not required && DataPath=secure)
   WV: HDCP + secure video path required (KCB: Observe_HDCP=true && HDCP=required && DataPath=secure)
   WV: 30s license duration (KCB: test timer expiration)
   WV: Secure VP9 (Requires a secure VP9 decoder. Only shows on devices that support VP9 content.)

Notes:

- The demo app shows up in the launcher as "ExoPlayer"

- The demo app contains a crude adaptive algorithm. It starts at 480p and will not switch up for 15
  seconds. This is expected (and has the benefit of more or less guaranteeing there's at least one
  switch during any playback beyond this length).

- If your device is running KLP or later, and the decoder claims to support adaptive, then ExoPlayer
  will do seamless resolution switching. If the decoder doesn't claim this then you'll still get the
  old nearly-seamless-switch (codec release/re-acquire) behavior.

- If your device is running KLP or later, the player will attempt to hook into the
  AudioTrack.getTimestamp API to do A/V sync. It will fall back to a legacy approach if
  the API isn't available.

- Exoplayer will retrieve a new license when playing "WV: 30s license duration",
  after the license duration has expired. Integrators should verify by means
  other than visual inspection that license duration is being enforced.
