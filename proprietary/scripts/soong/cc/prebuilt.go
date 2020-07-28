package mtkCc

import (
	"strings"

	"android/soong/android"
	"android/soong/cc"
)

func init() {
	android.PreArchMutators(func(ctx android.RegisterMutatorsContext) {
		ctx.BottomUp("mtk_prebuilts", prebuiltMutator).Parallel()
	})
}

func prebuiltMutator(ctx android.BottomUpMutatorContext) {
	moduleName := ctx.ModuleName()
	if strings.Contains(moduleName, "@MTK,") {
		var board, platform, productLine, exception string
		items := strings.Split(moduleName, "@MTK,")
		name := items[0]
		for _, variant := range strings.Split(items[1], ",") {
			if variant == "phone" {
				productLine = "smart_phone"
			} else if strings.HasPrefix(variant, "mt") {
				board = variant
			} else if strings.HasPrefix(variant, "MT") {
				platform = variant
			} else if strings.Contains(variant, "aosp") {
				exception = variant
			} else {
				productLine = variant
			}
		}
		vars := ctx.Config().VendorConfig("mtkPlugin")
		skip := false
		if ctx.OtherModuleExists(name) {
			skip = true
		}
		if (board != "") && (board != vars.String("TARGET_BRM_PLATFORM")) {
			skip = true
		}
		if (platform != "") && (platform != vars.String("MTK_PLATFORM")) {
			skip = true
		}
		if (productLine != "") && (productLine != vars.String("MTK_PRODUCT_LINE")) {
			skip = true
		}
		if (exception != "") && ((vars.String("MTK_TARGET_PROJECT") != "") || (vars.String("SYS_TARGET_PROJECT") != "")) {
			skip = true
		}
		//if m, ok := ctx.Module().(android.PrebuiltInterface); ok && m.Prebuilt() != nil {
		if m, ok := ctx.Module().(*cc.Module); ok {
			if skip {
				m.SkipInstall()
			} else {
				ctx.Rename(name)
			}
			m.Properties.HideFromMake = true
		}
	}
}
