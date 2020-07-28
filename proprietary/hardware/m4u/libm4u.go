package mtkLibm4u

import (
	"strings"

	"android/soong/android"
	"android/soong/cc"
)

func init() {
	android.RegisterModuleType("mtk_libm4u_defaults", mtkLibm4uDefaultsFactory)
}

func mtkLibm4uDefaultsFactory() android.Module {
	module := cc.DefaultsFactory()
	android.AddLoadHook(module, mtkLibm4uLoadHook)
	return module
}

func mtkLibm4uLoadHook(ctx android.LoadHookContext) {
	type props struct {
		Srcs []string
		Local_include_dirs []string
		Export_include_dirs []string
	}
	p := &props{}
	vars := ctx.Config().VendorConfig("mtkPlugin")
	if vars.String("MTK_PLATFORM") != "" {
		mtkPlatformDir := strings.ToLower(vars.String("MTK_PLATFORM"))
		p.Local_include_dirs = append(p.Local_include_dirs, mtkPlatformDir)
		p.Export_include_dirs = append(p.Export_include_dirs, mtkPlatformDir)
	}
	switch vars.String("TARGET_BOARD_PLATFORM") {
	case "mt6735", "mt6737t":
		p.Srcs = append(p.Srcs, "mt6735/D1/m4u_lib_port.cpp")
	case "mt6735m", "mt6737m":
		p.Srcs = append(p.Srcs, "mt6735/D2/m4u_lib_port.cpp")
	case "mt6753":
		p.Srcs = append(p.Srcs, "mt6735/D3/m4u_lib_port.cpp")
	}
	ctx.AppendProperties(p)
}
