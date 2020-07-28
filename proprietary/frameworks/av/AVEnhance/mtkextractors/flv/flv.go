package flv

import (
	"android/soong/android"
	"android/soong/cc"
)

func mtkFlvextractorDefaults(ctx android.LoadHookContext) {
	type props struct {
		Cflags []string
		Srcs   []string
	}
	p := &props{}
	vars := ctx.Config().VendorConfig("mtkPlugin")
	if vars.Bool("MSSI_MTK_FLV_PLAYBACK_SUPPORT") {
		p.Srcs = append(p.Srcs, "MtkFLVExtractor.cpp")
	}
	ctx.AppendProperties(p)
}

func init() {
	android.RegisterModuleType("mtk_flvextractor_defaults", mtkFlvextractorDefaultsFactory)
}

func mtkFlvextractorDefaultsFactory() android.Module {
	module := cc.DefaultsFactory()
	android.AddLoadHook(module, mtkFlvextractorDefaults)
	return module
}
