package libmtkavenhancements

import (
	"android/soong/android"
	"android/soong/cc"
)

func mtkLibmtkavenhancementsDefaults(ctx android.LoadHookContext) {
	type props struct {
		Cflags       []string
		Include_dirs []string
		Shared_libs  []string
	}
	p := &props{}
    vars := ctx.Config().VendorConfig("mtkPlugin")
    if vars.Bool("MSSI_MTK_AUDIO_ADPCM_SUPPORT") {
        p.Cflags = append(p.Cflags, "-DMSSI_MTK_AUDIO_ADPCM_SUPPORT")
    }
    if vars.Bool("MSSI_MTK_WMA_PLAYBACK_SUPPORT") {
        p.Cflags = append(p.Cflags, "-DMSSI_MTK_WMA_PLAYBACK_SUPPORT")
    }
    if vars.Bool("MSSI_MTK_SWIP_WMAPRO") {
        p.Cflags = append(p.Cflags, "-DMSSI_MTK_SWIP_WMAPRO")
    }
    if vars.Bool("MSSI_MTK_AUDIO_ALAC_SUPPORT") {
        p.Cflags = append(p.Cflags, "-DMSSI_MTK_AUDIO_ALAC_SUPPORT")
    }
    if vars.Bool("MSSI_MTK_AUDIO_APE_SUPPORT") {
        p.Cflags = append(p.Cflags, "-DMSSI_MTK_AUDIO_APE_SUPPORT")
    }

    p.Cflags = append(p.Cflags, "-DMTK_ROI_SUPPORT")

	ctx.AppendProperties(p)
}

func init() {
	android.RegisterModuleType("mtk_libmtkavenhancements_defaults", mtkLibmtkavenhancementsDefaultsFactory)
}

func mtkLibmtkavenhancementsDefaultsFactory() android.Module {
	module := cc.DefaultsFactory()
	android.AddLoadHook(module, mtkLibmtkavenhancementsDefaults)
	return module
}
