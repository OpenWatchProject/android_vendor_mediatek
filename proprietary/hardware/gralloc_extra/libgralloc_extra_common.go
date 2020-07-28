package mtkLibgrallocExtraCommon

import (
//	"os"
//	"path/filepath"
//	"strings"

	"android/soong/android"
	"android/soong/cc"
)

func mtkDebugFWDefaults(ctx android.LoadHookContext) {
	type props struct {
		Srcs []string
		Cflags []string
	}
	p := &props{}
	vars := ctx.Config().VendorConfig("mtkPlugin")
//	platformDir := strings.ToLower(vars.String("MTK_PLATFORM"))
//	platformDir := strings.ToLower(mediatek.GetFeature("MTK_PLATFORM"))
//	localPath := ctx.ModuleDir()
//	if _, err := os.Stat(filepath.Join(localPath, "platform", platformDir, "platform.c")); err == nil {
//		p.Srcs = append(p.Srcs, filepath.Join("platform", platformDir, "platform.c"))
//	} else {
//		p.Srcs = append(p.Srcs, filepath.Join("platform", "default", "platform.c"))
//	}

	if vars.Bool("MTK_CUSTOM_BUF_IMPL_DEF_FT") {
		p.Cflags = append(p.Cflags, "-DGE_PIXEL_FORMAT_IMPL_CUSTOMIZATION")
		// custom_header := vars.String("MTK_CUSTOM_BUF_IMPL_DEF_FT_HEADER")
		custom_header := "mexico"
		p.Cflags = append(p.Cflags, "-DGE_PIXEL_FORMAT_CUSTOM_HEADER=\""+custom_header+".h\"")
	}

	ctx.AppendProperties(p)
}

func init() {
	android.RegisterModuleType("mtk_libgralloc_extra_common_defaults", mtkLibgrallocExtraCommonDefaultsFactory)
}

func mtkLibgrallocExtraCommonDefaultsFactory() android.Module {
	module := cc.DefaultsFactory()
	android.AddLoadHook(module, mtkDebugFWDefaults)
	return module
}

