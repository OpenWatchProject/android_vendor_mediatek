
extern "C" {

// The LCM number which this project can support
int LCM_COUNT = 1;

// The gamma of LCM
// This value must be configured correctly.
// LcmGamma[0] is for LCM 0, LcmGamma[1] is for LCM 1, ... etc,
// if this project supports multiple LCM.
float LcmGamma[] = { 2.2 };


// --------------------------------------------------------------------------
//  Behavior configuration
// --------------------------------------------------------------------------

// The enhancement level of DRE for sunlight in [0, 255]
// Larger value means stronger
// Supports multiple LCM. The number of elements must equal to LCM_COUNT.
int ReadabilityLevel[] = { 207 };

// The enhancement level of DRE for low backlight in [0, 255]
// Larger value means stronger
// Supports multiple LCM. The number of elements must equal to LCM_COUNT.
int LowBLReadabilityLevel[] = { 128 };

// Strength of Content-adaptive backlight control
// In [0, 255]
// This function could intelligently reduce backlight to save power according to content.
// The larger SmartBacklightStrength value, the more power saving.
// However, the excessive large value may degrade image¡¦s brightness.
// Supports multiple LCM. The number of elements must equal to LCM_COUNT.
int SmartBacklightStrength[] = { 128 };

// Effective range of Content-adaptive backlight control
// In [0, 255]
// This parameter influences the image type which SmartBacklightStrength applied.
// Using small SmartBacklightRange value, SmartBacklightStrength only effects on bright image scene.
// SmartBacklightStrength effects on mid-bright image scene by using larger SmartBacklightStrength value.
// Supports multiple LCM. The number of elements must equal to LCM_COUNT.
int SmartBacklightRange[] = { 128 };

// Minimum backlight value of AAL output
// In [0, 1023]
// Supports multiple LCM. The number of elements must equal to LCM_COUNT.
#if defined(MTK_ULTRA_DIMMING_SUPPORT)
int MinOutBL[] = { 32 };
int UDEnable = 1;
int UDBL10bThH = 32;
int UDBL10bThM = 0;
int UDBL10bWhH = 32;
int UDBL10bWhL = 0;
int UDPL08bThL = 128;
#else
int MinOutBL[] = { 0 };
#endif

// Default use ESS only
int InitFunction = 2;
}

