package mtkFramework

import (
	"os"

	"android/soong/android"
	"android/soong/java"
)

func init() {
	android.RegisterModuleType("mtk_framework_defaults", mtkFrameworkDefaultsFactory)
}

func mtkFrameworkDefaultsFactory() android.Module {
	module := java.DefaultsFactory()
	android.AddLoadHook(module, mtkFrameworkDefaults)
	return module
}

func mtkFrameworkDefaults(ctx android.LoadHookContext) {
	type props struct {
		Libs []string
	}
	p := &props{}
	s := "vendor/mediatek/proprietary/frameworks/opt/fm/Android.bp"
	if _, err := os.Stat(s); err == nil {
		p.Libs = append(p.Libs, "mediatek-fm-framework")
	} 
	ctx.AppendProperties(p)
}
