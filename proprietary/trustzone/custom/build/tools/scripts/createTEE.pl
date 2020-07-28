#!/usr/bin/perl

use Getopt::Long;
use Cwd;

my $project_name = "";
my $project_cfg_file = "";
my $project_is_flavor = "no";
my $create_action = "on";
my $base_project_cfg = "";
my $base_project_name = "";
my $pwd = cwd();

&GetOptions(
  "prj_name=s" => \$project_name,
  "prj_cfg=s" => \$project_cfg_file,
  "base_prj_cfg=s" => \$base_project_cfg,
  "flavor=s" => \$project_is_flavor,
  "action=s" => \$create_action,
);

if (($project_cfg_file eq "" ) || (!-e $project_cfg_file)) {
  print "ERROR: Can not find TARGET_PROJECT ProjectConfig.mk: $project_cfg_file\n";
  &usage();
}

if (($base_project_cfg eq "" ) || (!-e $base_project_cfg)) {
  print "ERROR: Can not find TARGET_PROJECT full_xxx.mk: $base_project_cfg\n";
}

# check project type (TEE must be main project)
if ($project_is_flavor eq "yes")
{
  my $base_proj_tmp = getOptionValue($base_project_cfg, "MTK_BASE_PROJECT");
  print "INFO: MTK_BASE_PROJECT is $base_proj_tmp\n";
  if (($base_proj_tmp eq "") || ($base_proj_tmp eq "\$(MTK_TARGET_PROJECT)")) {
    die "ERROR: flavor project but MTK_BASE_PROJECT incorrect\n";
  }
  $base_project_name = $base_proj_tmp;
  print "INFO: MTK_BASE_PROJECT is $base_project_name\n";
  #die "ERROR: TEE can't create on flavor project\n";
}

# check kernel version and platform name in project config
my $kernel_version = getOptionValue($project_cfg_file, "LINUX_KERNEL_VERSION");
my $platform_name = getOptionValue($project_cfg_file, "MTK_PLATFORM");
my $trustonic_tee = getOptionValue($project_cfg_file, "TRUSTONIC_TEE_SUPPORT");
my $microtrust_tee = getOptionValue($project_cfg_file, "MICROTRUST_TEE_SUPPORT");
my $microtrust_tee_lite = getOptionValue($project_cfg_file, "MICROTRUST_TEE_LITE_SUPPORT");
my $watchdata_tee = getOptionValue($project_cfg_file, "WATCHDATA_TEE_SUPPORT");
my $google_trusty = getOptionValue($project_cfg_file, "MTK_GOOGLE_TRUSTY_SUPPORT");
my $in_house_tee = getOptionValue($project_cfg_file, "MTK_IN_HOUSE_TEE_SUPPORT");
my $tee_support = getOptionValue($project_cfg_file, "MTK_TEE_SUPPORT");
my $trustkernel_tee = getOptionValue($project_cfg_file, "TRUSTKERNEL_TEE_SUPPORT");

print "INFO: kernel version is $kernel_version\n";
print "INFO: platform name is $platform_name\n";

# check for other option
if (($tee_support ne "yes") && ($tee_support ne "no")) {
  die "ERROR: MTK_TEE_SUPPORT is not configured!\n"
} elsif (($create_action eq "on") && ($tee_support ne "yes")) {
  die "ERROR: MTK_TEE_SUPPORT is not set to \"yes\"!\n"
} elsif (($create_action eq "off") && ($tee_support ne "no")) {
  die "ERROR: MTK_TEE_SUPPORT is not set to \"no\"!\n"
}



if (($create_action eq "on") && ($tee_support eq "yes")) {
	my @tee_ary;
	my $tee_cnt=0;

	if ($trustonic_tee eq "yes") {
		$tee_ary[0]=1;}
	if ($microtrust_tee eq "yes") {
		$tee_ary[1]=1;}
	if ($microtrust_tee_lite eq "yes") {
		$tee_ary[1]=1;}
	if ($watchdata_tee eq "yes") {
		$tee_ary[2]=1;}
	if ($google_trusty eq "yes") {
		$tee_ary[3]=1;}
	if ($trustkernel_tee eq "yes") {
		$tee_ary[4]=1;}

	for($cnt=0; $cnt<=$#tee_ary; $cnt++) {
		$tee_cnt+=$tee_ary[$cnt];
		print "DBG: tee_ary[$cnt]=$tee_ary[$cnt];tee_cnt=$tee_cnt\n";
		if ($tee_cnt>=2) {
			die "ERROR: {TEEs}_SUPPORT can't set to \"yes\" in the same project!\n"}
	}
}

# check ARMv8 or ARMv7
my $arch_type = "Armv8";
if (($platform_name eq "MT6582") || ($platform_name eq "MT6592") || ($platform_name eq "MT6580") || ($platform_name eq "MT6570"))
{
  $arch_type = "Armv7";
}

# check TEE type
my $tee_type = "";
if ($trustonic_tee eq "yes") {
  $tee_type = "trustonic";
} elsif($microtrust_tee eq "yes") {
  $tee_type = "teei";
} elsif($microtrust_tee_lite eq "yes") {
  $tee_type = "teeilite";
} elsif($watchdata_tee eq "yes") {
  $tee_type = "watchdata";
} elsif($google_trusty eq "yes") {
  $tee_type = "trusty";
} elsif($in_house_tee eq "yes") {
  $tee_type = "mtee";
} elsif($trustkernel_tee eq "yes") {
  $tee_type = "tkcore";
}

if (($create_action eq "on") && ($tee_type eq "")) {
  die "ERROR: TRUSTONIC_TEE_SUPPORT || MICROTRUST_TEE_SUPPORT || MICROTRUST_TEE_LITE_SUPPORT || WATCHDATA_TEE_SUPPORT || MTK_GOOGLE_TRUSTY_SUPPORT || TRUSTKERNEL_TEE_SUPPORT are not set to \"yes\"!\n"
}

if (($create_action eq "on") && ($in_house_tee eq "yes")) {
  print "INFO: Skip for MTK_IN_HOUSE_TEE_SUPPORT!\n";
  exit 0;
}

print "INFO: tee type is $tee_type\n";

if ($microtrust_tee eq "yes") {
  if($arch_type eq "Armv7") {
    die "ERROR: ARMv7 chipsets are not supported in MICROTRUST TEE!\n";
  }
  if (($platform_name eq "MT6752") || ($platform_name eq "MT6795")) {
    die "ERROR: MT6752/MT6795 chipsets are not supported in MICROTRUST TEE!\n";
  }
}

my $pre_process_result = teeOnOffPreProcess();
my $result1 = teeOnOffCheckOperation($trustonic_tee, "trustonic");
my $result2 = teeOnOffCheckOperation($microtrust_tee, "teei");
my $result3 = teeOnOffCheckOperation($microtrust_tee_lite, "teei lite");
my $result4 = teeOnOffCheckOperation($google_trusty, "trusty");
my $result5 = teeOnOffCheckOperation($watchdata_tee, "watchdata");
my $result6 = teeOnOffCheckOperation($trustkernel_tee, "tkcore");
my $final1 = "not an enabling action!\n";
if (($create_action eq "on") && ($tee_support eq "yes")) {
  $final1 = teeOnOffCheckOperation("yes", $tee_type);
}
my $post_process_result = teeOnOffPostProcess();

print "======================================================\n";
print "======================================================\n";
print "[PRE]: ".$pre_process_result;
print "[OP1]: ".$result1;
print "[OP2]: ".$result2;
print "[OP3]: ".$result3;
print "[OP4]: ".$result4;
print "[OP5]: ".$result5;
print "[OP6]: ".$result6;
print "[FINAL]: ".$final1;
print "[POST]: ".$post_process_result;
print "======================================================\n";
print "======================================================\n";

exit 0;

sub trim { my $s = shift; $s =~ s/^\s+|\s+$//g; return $s };

sub teeOnOffCheckOperation{
  my $enable_tee_script = "./changeTEE.pl";
  my $config_str = "";
  my $project_str = "--project=".$project_name;
  my $base_project_str = ($base_project_name ne "") ? "--base_project=".$base_project_name : "";
  my $kernel_str = "--kernel=".$kernel_version;
  my $change_tee_cmd = "";
  my $final_action = "no change";
  my $option_str = "";

  my $prj_tee_setting=$_[0];
  my $prj_tee_type=$_[1];


  if(($prj_tee_setting eq "no") || ($prj_tee_setting eq "yes")) {
    if (($create_action eq "on") && ($prj_tee_setting eq "yes")) {
      $final_action = "on";
    } else {
      $final_action = "off";
    }

# Microtrust cancel low memory solution
#    if (($platform_name eq "MT6735") && ($prj_tee_type eq "teei") && ($final_action eq "on"))
#    {
#      $option_str = "_low_mem";
#    }

    # Trustonic TEE GP Support
    if (($prj_tee_type eq "trustonic") && ($arch_type eq "Armv8") && ($final_action eq "on"))
    {
      $option_str = "_gp";
      if (($platform_name eq "MT6735") || ($platform_name eq "MT6755") || ($platform_name eq "MT6757"))
      {
        $option_str = "";
      }
    }

    $config_str = "--config=".$prj_tee_type.$arch_type."_".$final_action.$option_str.".xml";
    $change_tee_cmd = $enable_tee_script." ".$config_str." ".$project_str." ".$base_project_str." ".$kernel_str;
    print "EXEC($final_action): $change_tee_cmd\n";
    system($change_tee_cmd);
    print "======================================================\n";
    print "$prj_tee_type options are turned $final_action successfully!\n";
    print "======================================================\n";
  }

  return "operation for ".$prj_tee_type." is ".$final_action."!\n";
}

sub teeOnOffPreProcess {
  my $enable_tee_script = "./preChangeTEE.py";
  my $project_str = "--project=".$project_name;
  my $base_project_str = ($base_project_name ne "") ? "--base_project=".$base_project_name : "";
  my $change_tee_cmd = "";

  $change_tee_cmd = $enable_tee_script." ".$project_str." ".$base_project_str;
  print "EXEC(PreProcess): $change_tee_cmd\n";
  system($change_tee_cmd);
  print "======================================================\n";
  print "PRE-Processing successfully!\n";
  print "======================================================\n";
  return "operation for \"PRE-Processing\" is successfully!\n";
}

sub teeOnOffPostProcess {
  my $enable_tee_script = "./postChangeTEE.py";
  my $project_str = "--project=".$project_name;
  my $base_project_str = ($base_project_name ne "") ? "--base_project=".$base_project_name : "";
  my $tee_type_str = "--tee_type=".$tee_type;
  my $change_tee_cmd = "";

  $change_tee_cmd = $enable_tee_script." ".$project_str." ".$base_project_str." ".$tee_type_str;
  print "EXEC(PostProcess): $change_tee_cmd\n";
  system($change_tee_cmd);
  print "======================================================\n";
  print "POST-Processing successfully!\n";
  print "======================================================\n";
  return "operation for \"POST-Processing\" is successfully!\n";
}

sub getOptionValue{
  my $file=$_[0];
  my $search_str=$_[1];
  my $key;
  my $value;

  open MAKEFILE,"<$file" or die "Can not open $file\n";
  while(<MAKEFILE>){
    my $line = $_;
    chomp $line;
    if($line =~ /^\s*$search_str\s*(:?)=/){
      print "DBG: ".$line."\n";
      ($key, $value) = split /=/, $line;
      #print "key=".trim($key)."\n";
      #print "value=".trim($value)."\n";
    }
  }
  close MAKEFILE;

  return trim($value);
}

sub usage{
  warn << "__END_OF_USAGE";
Usage: ./createTEE.pl --prj_name=TARGET_PROJECT_Name --prj_cfg=TARGET_PROJECT_ProjectConfig.mk --base_prj_cfg=TARGET_PROJECT_full_xxx.mk --flavor=no --action=on
       ./createTEE.pl --prj_name=TARGET_PROJECT_Name --prj_cfg=TARGET_PROJECT_ProjectConfig.mk --base_prj_cfg=TARGET_PROJECT_full_xxx.mk --flavor=yes --action=on

Examples:
       ./createTEE.pl --prj_name=evb6755_64_tee_4gb --prj_cfg=../../../../../../../../device/mediatek/evb6755_64_tee_4gb/ProjectConfig.mk --base_prj_cfg=../../../../../../../../device/mediatek/evb6755_64_tee_4gb/full_evb6755_64_tee_4gb.mk --flavor=no --action=on
       ./createTEE.pl --prj_name=k99v1_64_tee --prj_cfg=../../../../../../../../device/mediatek/k99v1_64_tee/ProjectConfig.mk --base_prj_cfg=../../../../../../../../device/mediatek/k99v1_64_tee/full_k99v1_64_tee.mk --flavor=yes --action=on

Options:
  --prj_name    : [MUST]     TARGET_PROJECT name
  --prj_cfg     : [MUST]     TARGET_PROJECT ProjectConfig.mk path
  --base_prj_cfg: [MUST]     TARGET_PROJECT full_xxx.mk path
  --flavor      : [OPTIONAL] yes | no
  --action      : [MUST]     on  | off

__END_OF_USAGE

  exit 1;
}
