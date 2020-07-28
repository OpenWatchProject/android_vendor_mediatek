package mtkCc

import (
	"path/filepath"
	"strings"

	"android/soong/android"
	"android/soong/cc"
	"android/soong/android/mediatek"
)

var (
	pctx = android.NewPackageContext("android/soong/cc/mediatek")
)

func init() {
	android.RegisterModuleType("mtk_global_cgen_defaults", mtkGlobalCgenDefaultsFactory)
	android.RegisterModuleType("mtk_global_define_defaults", mtkGlobalDefineDefaultsFactory)
	android.RegisterMakeVarsProvider(pctx, mtkMakeVarsProvider)
}

func mtkGlobalDefine(vars android.VendorConfig, quote bool) []string {
	var cflags []string
	if !vars.Bool("MTK_BASIC_PACKAGE") {
		cflags = append(cflags, "-DMTK_AOSP_ENHANCEMENT")
	}
	for _, name := range strings.Fields(vars.String("AUTO_ADD_GLOBAL_DEFINE_BY_NAME")) {
		value := vars.String(name)
		if (value != "") &&
			(value != "no") &&
			(value != "NO") &&
			(value != "none") &&
			(value != "NONE") &&
			(value != "false") &&
			(value != "FALSE") {
			cflags = append(cflags, "-D"+name)
		}
	}
	for _, name := range strings.Fields(vars.String("AUTO_ADD_GLOBAL_DEFINE_BY_VALUE")) {
		value := vars.String(name)
		for _, def := range strings.Fields(value) {
			if (value != "") &&
				(value != "no") &&
				(value != "NO") &&
				(value != "none") &&
				(value != "NONE") &&
				(value != "false") &&
				(value != "FALSE") {
				cflags = append(cflags, "-D"+strings.ToUpper(def))
			}
		}
	}
	for _, name := range strings.Fields(vars.String("AUTO_ADD_GLOBAL_DEFINE_BY_NAME_VALUE")) {
		value := vars.String(name)
		if (value != "") &&
			(value != "no") &&
			(value != "NO") &&
			(value != "none") &&
			(value != "NONE") &&
			(value != "false") &&
			(value != "FALSE") {
			if quote {
				cflags = append(cflags, "-D"+name+"=\\\""+value+"\\\"")
			} else {
				cflags = append(cflags, "-D"+name+"=\""+value+"\"")
			}
		}
	}
	return cflags
}

func mtkGlobalCgenInclude(ctx android.BaseModuleContext, vars android.VendorConfig, allowMissing bool) []string {
	var includes []string
	mtkBaseProject := vars.String("MTK_BASE_PROJECT")
	if mtkBaseProject == "" {
		mtkBaseProject = vars.String("SYS_BASE_PROJECT")
	}
	mtkPlatform := vars.String("MTK_PLATFORM")
	mtkPlatformDir := strings.ToLower(mtkPlatform)
	mtkPathCustom := filepath.Join(mediatek.MtkPathSource, "custom", mtkBaseProject)
	mtkPathPlatform := filepath.Join(mediatek.MtkPathSource, "custom", mtkPlatformDir)
	mtkPathCommon := filepath.Join(mediatek.MtkPathSource, "custom", "common")
	for _, base := range []string{mtkPathCustom, mtkPathPlatform, mtkPathCommon} {
		if base != "" {
			for _, sub := range []string{"cgen/cfgdefault", "cgen/cfgfileinc", "cgen/inc", "cgen"} {
				dir := filepath.Join(base, sub)
				if allowMissing || android.ExistentPathForSource(ctx,dir).Valid(){
					includes = append(includes, dir)
				}
			}
		}
	}
	return includes
}

func mtkGlobalCgenDefaultsFactory() android.Module {
	module := cc.DefaultsFactory()
	android.AddLoadHook(module, mtkGlobalCgenDefaults)
	return module
}

func mtkGlobalCgenDefaults(ctx android.LoadHookContext) {
	type props struct {
		Target struct {
			Android struct {
				Include_dirs []string
			}
		}
	}
	p := &props{}
	vars := ctx.Config().VendorConfig("mtkPlugin")
	includes := mtkGlobalCgenInclude(ctx, vars, false)
	p.Target.Android.Include_dirs = append(p.Target.Android.Include_dirs, includes...)
	ctx.AppendProperties(p)
}

func mtkGlobalDefineDefaultsFactory() android.Module {
	module := cc.DefaultsFactory()
	android.AddLoadHook(module, mtkGlobalDefineDefaults)
	return module
}

func mtkGlobalDefineDefaults(ctx android.LoadHookContext) {
	type props struct {
		Target struct {
			Android struct {
				Cflags []string
			}
		}
	}
	p := &props{}
	vars := ctx.Config().VendorConfig("mtkPlugin")
	cflags := mtkGlobalDefine(vars, false)
	p.Target.Android.Cflags = append(p.Target.Android.Cflags, cflags...)
	ctx.AppendProperties(p)
}

func mtkMakeVarsProvider(ctx android.MakeVarsContext) {
	vars := ctx.Config().VendorConfig("mtkPlugin")
	cflags := mtkGlobalDefine(vars, true)
	var includes []string
	mtkAudioInclude := filepath.Join(mediatek.MtkPathSource, "hardware/audio/common/include")
	includes = append(includes, mtkAudioInclude)
	// Passing nil is okay only because we're using true for the last argument.
	mtkCgenIncludes := mtkGlobalCgenInclude(nil, vars, true)
	includes = append(includes, mtkCgenIncludes...)
	ctx.Strict("MTK_GLOBAL_CFLAGS", strings.Join(cflags, " "))
	ctx.Strict("MTK_GLOBAL_C_INCLUDES", strings.Join(includes, " "))
	ctx.Strict("MTK_PATH_SOURCE", mediatek.MtkPathSource)
}
