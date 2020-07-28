package asf

import (
	"android/soong/android"
	"android/soong/cc"
)

func mtkAsfextractorDefaults(ctx android.LoadHookContext) {
	type props struct {
		Cflags       []string
		Srcs         []string
		Include_dirs []string
		Static_libs  []string
	}
	p := &props{}
	vars := ctx.Config().VendorConfig("mtkPlugin")
	if vars.Bool("MSSI_MTK_WMV_PLAYBACK_SUPPORT") {
		p.Srcs = append(p.Srcs, "ASFExtractor.cpp")
		p.Include_dirs = append(p.Include_dirs, "vendor/mediatek/proprietary/frameworks/av/AVEnhance/mtkextractors/asf/libasf/inc")
		p.Static_libs = append(p.Static_libs, "libasf")
	}
	if vars.Bool("MSSI_MTK_SWIP_WMAPRO") {
		p.Cflags = append(p.Cflags, "-DMSSI_MTK_SWIP_WMAPRO")
	}
	ctx.AppendProperties(p)
}

func init() {
	android.RegisterModuleType("mtk_asfextractor_defaults", mtkAsfextractorDefaultsFactory)
}

func mtkAsfextractorDefaultsFactory() android.Module {
	module := cc.DefaultsFactory()
	android.AddLoadHook(module, mtkAsfextractorDefaults)
	return module
}
