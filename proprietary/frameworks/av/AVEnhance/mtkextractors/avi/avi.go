package avi

import (
	"android/soong/android"
	"android/soong/cc"
)

func mtkAviextractorDefaults(ctx android.LoadHookContext) {
	type props struct {
		Cflags       []string
		Srcs         []string
		Include_dirs []string
		Static_libs  []string
	}
	p := &props{}
	vars := ctx.Config().VendorConfig("mtkPlugin")
	if vars.Bool("MSSI_MTK_AVI_PLAYBACK_SUPPORT") {
		p.Srcs = append(p.Srcs, "MtkAVIExtractor.cpp")
	}
	ctx.AppendProperties(p)
}

func init() {
	android.RegisterModuleType("mtk_aviextractor_defaults", mtkAviextractorDefaultsFactory)
}

func mtkAviextractorDefaultsFactory() android.Module {
	module := cc.DefaultsFactory()
	android.AddLoadHook(module, mtkAviextractorDefaults)
	return module
}
