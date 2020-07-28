package mtkImsCommon

import (
    "android/soong/android"
    "android/soong/java"
    "github.com/google/blueprint/proptools"
)

func init() {
    android.RegisterModuleType("mtk_ImsCommon_defaults", mtkImsCommonDefaultsFactory)
}

func mtkImsCommonDefaultsFactory() android.Module {
    module := java.DefaultsFactory()
    android.AddLoadHook(module, mtkImsCommonDefaults)
    return module
}

func mtkImsCommonDefaults(ctx android.LoadHookContext) {
    type props struct {
        Enabled *bool
    }
    p := &props{}
    vars := ctx.Config().VendorConfig("mtkPlugin")

    if vars.String("MSSI_MTK_TC1_COMMON_SERVICE") == "yes" {
        p.Enabled = proptools.BoolPtr(false)
    }

    if vars.String("MSSI_MTK_TELEPHONY_ADD_ON_POLICY") == "1" {
        p.Enabled = proptools.BoolPtr(false)
    }

    ctx.AppendProperties(p)
}
