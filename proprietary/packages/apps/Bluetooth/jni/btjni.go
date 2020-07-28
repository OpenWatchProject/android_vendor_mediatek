package btjni

import (
	"android/soong/android"
	"android/soong/cc"
)

func init() {
	android.RegisterModuleType("mtk_btjni_defaults", mtkBtjniDefaultsFactory)
}

func mtkBtjniDefaultsFactory() android.Module {
	module := cc.DefaultsFactory()
	android.AddLoadHook(module, mtkBtjniLoadHook)
	return module
}

func mtkBtjniLoadHook(ctx android.LoadHookContext) {
 	type props struct {
 		Cflags []string
 		Include_dirs   []string
		Static_libs[]string
		Shared_libs []string
 	}
	p := &props{}
	vars := ctx.Config().VendorConfig("mtkPlugin")
        if vars.Bool("MTK_BSP_PACKAGE") {
                p.Static_libs = append(p.Static_libs, "libbt-mtk_cust")
        }
	ctx.AppendProperties(p)
}
