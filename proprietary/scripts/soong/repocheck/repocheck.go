package repoCheck

import (
	"fmt"
	"os"
	"os/exec"
	"strings"

	"android/soong/android"
)

const (
	repo_check_tool = "device/mediatek/build/build/tools/repo_dep_check.py"
)

var repocheck_vars = map[string]string{
	"PRODUCTS":              "",
	"TARGET_PRODUCT":        "",
	"MTK_TARGET_PROJECT":    "",
	"MTK_BASE_PROJECT":      "",
	"TARGET_BOARD_PLATFORM": "",
	"LINUX_KERNEL_VERSION":  "",
	"TARGET_ARCH":           "",
}

func init() {
	android.RegisterSingletonType("repocheck", RepoCheckkSingleton)
}

func RepoCheckkSingleton() android.Singleton {
	return &repoCheckkSingleton{}
}

type repoCheckkSingleton struct{}

func (c *repoCheckkSingleton) GenerateBuildActions(ctx android.SingletonContext) {
	if _, err := os.Stat(repo_check_tool); err != nil {
		// skip check if script is missing
		return
	}
	vars := ctx.Config().VendorConfig("mtkPlugin")
	config := ctx.Config()
	var vars_list []string
	for k, _ := range repocheck_vars {
		repocheck_vars[k] = vars.String(k)
		// try if env has the variable
		if repocheck_vars[k] == "" {
			repocheck_vars[k] = config.Getenv(k)
		}
		vars_list = append(vars_list, k+"="+repocheck_vars[k])
	}
	// fmt.Println(vars_list)
	cmd := exec.Command(repo_check_tool, repocheck_vars["TARGET_PRODUCT"], "--vars", strings.Join(vars_list, " "))
	out, err := cmd.CombinedOutput()
	fmt.Printf("%s", string(out))
	if err != nil {
		ctx.Errorf("licensed repo check failed, %v\n", err)
	}
}
