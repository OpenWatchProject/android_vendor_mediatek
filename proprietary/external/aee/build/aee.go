package mtkAeeFeature

import (
	"android/soong/android"
	"android/soong/cc"
)

func mtkAeeDefaults(ctx android.LoadHookContext) {
	type props struct {
		Cflags []string
		Shared_libs []string
	}
	p := &props{}
	vars := ctx.Config().VendorConfig("mtkPlugin")
	if vars.Bool("HAVE_AEE_FEATURE") {
		p.Cflags = append(p.Cflags, "-DHAVE_AEE_FEATURE")
		p.Shared_libs = append(p.Shared_libs, "libaed")
	}
	ctx.AppendProperties(p)
}

func mtkAeeVendorDefaults(ctx android.LoadHookContext) {
	type props struct {
		Cflags []string
		Shared_libs []string
	}
	p := &props{}
	vars := ctx.Config().VendorConfig("mtkPlugin")
	if vars.Bool("HAVE_AEE_FEATURE") {
		p.Cflags = append(p.Cflags, "-DHAVE_AEE_FEATURE")
		p.Shared_libs = append(p.Shared_libs, "libaedv")
	}
	ctx.AppendProperties(p)
}

func init() {
	android.RegisterModuleType("mtk_aee_defaults", mtkAeeDefaultsFactory)
	android.RegisterModuleType("mtk_aee_vendor_defaults", mtkAeeVendorDefaultsFactory)
}

func mtkAeeDefaultsFactory() android.Module {
	module := cc.DefaultsFactory()
	android.AddLoadHook(module, mtkAeeDefaults)
	return module
}

func mtkAeeVendorDefaultsFactory() android.Module {
	module := cc.DefaultsFactory()
	android.AddLoadHook(module, mtkAeeVendorDefaults)
	return module
}
