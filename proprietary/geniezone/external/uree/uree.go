package mtkUreeFeature

import (
	"android/soong/android"
	"android/soong/cc"
)

func mtkUreeDefaults(ctx android.LoadHookContext) {
	type props struct {
		Cflags []string
		Shared_libs []string
		Static_libs []string
	}
	p := &props{}
	vars := ctx.Config().VendorConfig("mtkPlugin")
	if vars.Bool("MTK_ENABLE_GENIEZONE") {
		p.Shared_libs = append(p.Shared_libs, "libgz_uree")
	}
	if vars.Bool("MTK_TEE_GP_SUPPORT") {
		p.Cflags = append(p.Cflags, "-DGP_TEE_ENABLED")
	}
	if vars.Bool("TRUSTONIC_TEE_SUPPORT") {
		p.Cflags = append(p.Cflags, "-DTRUSTONIC_TEE_ENABLED")
		p.Cflags = append(p.Cflags, "-DTBASE_API_LEVEL=5")
		p.Shared_libs = append(p.Shared_libs, "libMcClient")
	}
	if vars.Bool("MICROTRUST_TEE_SUPPORT") {
		p.Static_libs = append(p.Static_libs, "libTeeiClient")
	}
	p.Shared_libs = append(p.Shared_libs, "liblog")
	p.Shared_libs = append(p.Shared_libs, "libion")
	if vars.String("MTK_PLATFORM") != "" {
		p.Shared_libs = append(p.Shared_libs, "libion_mtk")
	}
	ctx.AppendProperties(p)
}

func init() {
	android.RegisterModuleType("mtk_uree_defaults", mtkUreeDefaultsFactory)
}

func mtkUreeDefaultsFactory() android.Module {
	module := cc.DefaultsFactory()
	android.AddLoadHook(module, mtkUreeDefaults)
	return module
}
