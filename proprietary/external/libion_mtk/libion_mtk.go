package mtklibion_mtk

import (
	//"strings"
	//"fmt"
	"android/soong/android"
	"android/soong/cc"
	"github.com/google/blueprint/proptools"
)

func init() {
	android.RegisterModuleType("mtk_libion_mtk_defaults", mtklibion_mtkDefaultsFactory)
}

func mtklibion_mtkDefaultsFactory() android.Module {
	module := cc.DefaultsFactory()
	android.AddLoadHook(module, mtklibion_mtkLoadHook)
	return module
}

func mtklibion_mtkLoadHook(ctx android.LoadHookContext) {
	type props struct {
		Srcs []string
		Cflags []string
		Export_include_dirs []string
		Include_dirs []string
		Enabled *bool
	}
	p := &props{}
	vars := ctx.Config().VendorConfig("mtkPlugin")
	if vars.String("MTK_PLATFORM") != "" {
		p.Export_include_dirs = append(p.Export_include_dirs, "include")
		p.Include_dirs = append(p.Include_dirs, "system/core/include")
		p.Enabled = proptools.BoolPtr(true)
	} else {
		//p.Enabled = proptools.BoolPtr(false)
		//fmt.Println("GM-T: MTK PLATFORM null")
	}

	ctx.AppendProperties(p)
}
