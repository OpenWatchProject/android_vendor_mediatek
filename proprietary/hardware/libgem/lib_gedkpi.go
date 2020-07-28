package mtkLibGedKpiFeature

import (
	"android/soong/android"
	"android/soong/cc"
)

func init() {
	android.RegisterModuleType("mtk_lib_gedkpi_defaults", mtkLibGedKpiDefaultsFactory)
}

func mtkLibGedKpiDefaultsFactory() android.Module {
	module := cc.DefaultsFactory()
	android.AddLoadHook(module, mtkLibGedKpiDefaults)
	return module
}

func mtkLibGedKpiDefaults(ctx android.LoadHookContext) {
	type props struct {
		Cflags []string
	}
	p := &props{}
	vars := ctx.Config().VendorConfig("mtkPlugin")
	if vars.Bool("MTK_DO_NOT_USE_GPU_EXT") {
		p.Cflags = append(p.Cflags, "-DMTK_DO_NOT_USE_GPU_EXT")
	}
	ctx.AppendProperties(p)
}
