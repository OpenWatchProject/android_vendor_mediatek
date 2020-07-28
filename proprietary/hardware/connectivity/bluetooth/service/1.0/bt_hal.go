package mtkBtHal

import (
	"android/soong/android"
	"android/soong/cc"
)

func init() {
	android.RegisterModuleType("mtk_bt_hal_defaults", mtkBtHalDefaultsFactory)
}

func mtkBtHalDefaultsFactory() android.Module {
	module := cc.DefaultsFactory()
	android.AddLoadHook(module, mtkBtHalLoadHook)
	return module
}

func mtkBtHalLoadHook(ctx android.LoadHookContext) {
	type props struct {
		Cflags []string
	}
	p := &props{}
	vars := ctx.Config().VendorConfig("mtkPlugin")
	if vars.Bool("MTK_BT_SUPPORT") {
		p.Cflags = append(p.Cflags, "-DMTK_BT_HAL_STATE_MACHINE=TRUE")
	}
	ctx.AppendProperties(p)
}
