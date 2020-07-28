Usage:
  [Common]
    perl project_clone.pl -p {codebase_full_path} -o {base_company/base_project} -n {new_company/new_project}
  [Arguments]
    -p  : source codebase full path in local device.(usually is release package uncompress path)
    -o  : base_company/base_project which the new project created from, support cross company.
    -n  : new_company/new_project need to be created.
  [Non-essential arg.]
    [-32] : Downgrade to 32bit form 64bit. (default: the same clone_project)
    [-64] : Upgrade to 64bit form 32bit. (default: the same clone_project)
  [Example]
    e.g. Create Project mtk/mtk6735_n1 from mediateksample/tk6735_n1:
    perl project_clone.pl -p "/home/user/release/v1.2.1/alps" -o "mediateksample/tk6735_n1" -n "mtk/mtk6735_n1"
