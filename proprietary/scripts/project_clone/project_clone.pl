#! /usr/bin/perl
# this script is only for the customer release package after aosp branch
# Use library
use strict;
use Data::Dumper;

#Global variable
my $local_ws;
my $codebase_type;

my $clone_project;
my $clone_company;
my $clone_platform;
my $clone_kernel;
my $clone_set;
my $clone_project_config_path;

my $apply_project;
my $apply_company;
my $apply_project_config_path;

my $company_change=0;
my $flag_64bit=0;
my $flag_32bit=0;

&ParseParameters;

$company_change=1 if ("$clone_company" ne "$apply_company");

&Usage if(!$clone_project || !$clone_company || !$apply_project || !$apply_company || !$local_ws);

if($clone_project eq $apply_project)
{
  print "[Error]: Apply project can't the same clone project.";
  exit 1;
}

if($flag_64bit && $flag_32bit)
{
  print "[Error]: -32 and -64 can only choose one.";
  exit 1;
}

#Check codebase path exist
if(! -d $local_ws)
{
  print "[Error]: Codebase path $local_ws is not exist.";
  exit 1;
}
#if($local_ws !~ /^.*\/alps$/)
#{
  #print "[Error]: Codebase root must is 'alps' folder.ex. -p /proj/home/user/release/alps";
  #exit 1;
#}

#Check codebase, clone project information
my ($codebase_type, $clone_project_config_path) = &Get_Branch_Type($local_ws, $clone_company, $clone_project);
my ($clone_platform, $clone_kernel, $clone_set) = &Get_ProjectType("$clone_project_config_path/ProjectConfig.mk");
$apply_project_config_path = $clone_project_config_path;
$apply_project_config_path =~ s/$clone_company/$apply_company/g;
$apply_project_config_path =~ s/$clone_project/$apply_project/g;
&ShowInfo;


if($flag_64bit == 0 && $flag_32bit ==0)
{
  my $check = &Cat_File_Grep("$clone_project_config_path/ProjectConfig.mk", "MTK_K64_SUPPORT *= *yes");
  $flag_64bit = 1 if($check);
  $flag_32bit = 1 if(!$check);
}

#Get clone list from clone project to apply project
my @clone_project_filelist = &Get_FileList($local_ws, $codebase_type, $clone_company, $clone_project, $clone_platform, $clone_kernel, $clone_set);
my %hash_clone = &Get_CloneTobe(@clone_project_filelist);
my (@apply_project_filelist, $clone_error) = &Clone_Source(%hash_clone);
if($clone_error)
{
  print "[Warring]: Some file copy fail, please confirm copy fail list.";
  print "### copy fail list ###\n";
  print "$clone_error";
  print "######################\n";
}

#Replace apply project source file content
my @replace_list = &Replace_Content(@apply_project_filelist);

if(@replace_list)
{
  &Gen_File_List(@replace_list);
}

my $end_time = `date '+%Y-%m-%d %H:%M:%S'`;
chomp ($end_time);
print "##### Create $apply_project complete. ($end_time) ######\n";
exit 0;
##################################################################
# function   : Get_Branch_Type
# arguments  : codebase path, clone company name, clone project name
# return     : aosp branch type, clone project config path
# description: get codebase information
##################################################################
sub Get_Branch_Type
{
  my $codebase = shift;
  my $company = shift;
  my $project = shift;
  my $target_file = "ProjectConfig.mk";
  my $ret_codebase_type;
  my $ret_config_path = "$codebase/device/$company/$project";
  if(-e "$ret_config_path/$target_file")
  {
    $ret_codebase_type = "new_aosp";
  }
  else
  {
    $ret_config_path = "$codebase/device/$company/build/config/$project";
    if(-e "$ret_config_path/$target_file")
    {
      $ret_codebase_type = "old_aosp";
    }
    else
    {
      $ret_config_path = "$codebase/device/$company/sprout/$project";
      if(-e "$ret_config_path/$target_file")
      {
        $ret_codebase_type = "sprout_aosp";
      }
    }
  }

  if(!$ret_codebase_type)
  {
    print "[Error]:[Get_Branch_Type] can't judge branch schema(new/old/sprout) or $project is not exist.\n";
    exit 1;
  }
  return ($ret_codebase_type, $ret_config_path);
}

##################################################################
# function   : Get_ProjectType
# arguments  : Clone project's ProjectConfig.mk file path
# return     : platform, kernel version, 32/64 bit
# description: check project type and return (main/flavor)
##################################################################
sub Get_ProjectType
{
  my $config_file = shift;
  my $ret_platform;
  my $ret_kernel = "kernel-3.10";
  my $ret_set = "32";

  #check platform
  my $platform = &Cat_File_Grep($config_file,"MTK_PLATFORM \\?= \\?");
  $platform =~ s/^MTK_PLATFORM ?= ?(.*)/$1/g;
  $platform = lc($platform);
  chomp($platform);
  $ret_platform=$platform;
  if ($platform eq "")
  {
    print "[Error]:[Get_ProjectType] Can't not get MTK_PLATFORM in $config_file\n";
    exit 1;
  }

  #check kernel version
  my $kernel = &Cat_File_Grep($config_file,"LINUX_KERNEL_VERSION \\?= \\?");
  $kernel =~ s/LINUX_KERNEL_VERSION ?= ?(.*)/$1/;
  chomp($kernel);
  $ret_kernel = $kernel if ($kernel ne "");

  #check 32/64bit
  my $set = &Cat_File_Grep($config_file,"MTK_K64_SUPPORT \\?= \\?");
  $set =~ s/MTK_K64_SUPPORT ?= ?(.*)\n/$1/;
  $ret_set = "64" if ($set=~ /.*yes.*/);
  return ($ret_platform,$ret_kernel,$ret_set);
}

##################################################################
# function   : Get_File_Type
# arguments  : $file
# return     : $filepath
# description: check file p4 exist or not and return file path.
##################################################################
sub Get_File_Type
{
  my $path = shift;
  my $cmd = "file '$path'";
  my $ret = `$cmd`;
  $ret =~ /\: (.*)/;
  return $1;
}

##################################################################
# function   : Get_FileList
# arguments  : aosp branch type, clone company name, clone project name, clone project platform, clone project kernel version, clone project set
# return     : filelist array
# description: get file list to do clone action.
##################################################################
sub Get_FileList
{
  my $path=shift;
  my $codebasetype=shift;
  my $company=shift;
  my $project=shift;
  my $platform=shift;
  my $kernel=shift;
  my $set=shift;
  $set="" if ($set eq "32");
  my @clonelist = ("$path/$kernel/arch/arm$set/boot/dts/$project.dts",
                "$path/$kernel/arch/arm$set/boot/dts/mediatek/$project.dts",
                "$path/$kernel/arch/arm$set/configs/$project"."_defconfig",
                "$path/$kernel/arch/arm$set/configs/$project"."_debug_defconfig",
                "$path/$kernel/arch/arm$set/mach-"."$platform/$project/*",
                "$path/$kernel/arch/arm$set/boot/dts/$project".".dtsi",
                "$path/$kernel/arch/arm$set/boot/dts/$project"."_bat_setting.dtsi",
                "$path/$kernel/arch/arm$set/boot/dts/$project"."_fg20_bat_setting.dtsi",
                "$path/$kernel/arch/arm$set/boot/dts/$project"."_fg_oldgauge_bat_setting.dtsi",
                "$path/$kernel/arch/arm$set/boot/dts/mediatek/$project"."_bat_setting.dtsi",
                "$path/$kernel/arch/arm$set/boot/dts/mediatek/$project"."_fg20_bat_setting.dtsi",
                "$path/$kernel/arch/arm$set/boot/dts/mediatek/$project"."_fg_oldgauge_bat_setting.dtsi",
                "$path/$kernel/arch/arm$set/boot/dts/mediatek/bat_setting/$project"."_bat_setting.dtsi",
                "$path/$kernel/drivers/misc/mediatek/mach/$platform/$project/*",
                "$path/$kernel/drivers/misc/mediatek/dws/$platform/$project.dws",
                "$path/vendor/$company/libs/$project/*",
                "$path/vendor/mediatek/libs/$project/*",
                "$path/vendor/mediatek/proprietary/bootable/bootloader/lk/project/$project.mk",
                "$path/vendor/mediatek/proprietary/bootable/bootloader/lk/target/$project/*",
                "$path/vendor/mediatek/proprietary/bootable/bootloader/preloader/custom/$project/*",
                "$path/vendor/mediatek/proprietary/custom/$project/*",
                "$path/vendor/mediatek/proprietary/scp/libs/lk/project/$project.mk",
                "$path/vendor/mediatek/proprietary/tinysys/lk/secure/project/$project.mk",
                "$path/vendor/mediatek/proprietary/trustzone/custom/build/project/$project.mk",
                "$path/trusty/vendor/mediatek/proprietary/project/$project.mk",
                "$path/trusty/vendor/mediatek/proprietary/project/target/$project/*",
                "$path/vendor/mediatek/proprietary/tinysys/freertos/*/project/*/$platform/$project/*",
                "$path/vendor/mediatek/proprietary/tinysys/scp/project/*/$platform/$project/*",
                "$path/vendor/mediatek/proprietary/tinysys/adsp/HIFI3/project/$platform/*/$project/*"
                );

  if(-e "$path/vendor/$company/libs/Android.mk")
  {
    push(@clonelist, "$path/vendor/$company/libs/Android.mk");
  }
  else
  {
    push(@clonelist, "$path/vendor/mediatek/libs/Android.mk");
  }

  if($codebasetype eq "new_aosp")
  {
    push (@clonelist, "$path/device/$company/$project/*");
  }
  elsif($codebasetype eq "old_aosp")
  {
    push (@clonelist, "$path/device/$company/$project/*",
                      "$path/device/$company/build/config/$project/*");
  }
  elsif($codebasetype eq "sprout_aosp")
  {
    push (@clonelist, "$path/device/$company/sprout/$project/*");
  }

  my @return;
  foreach(@clonelist)
  {
    my $cmd = "find $_ -type f ";
    my $ret = `$cmd`;
    foreach (split(/\n/,$ret))
    {
      push (@return,$_);
    }
  }
  return @return;
}

##################################################################
# function   : Get_CloneTobe
# arguments  : clone_filelist array
# return     : clone hashtable
# description: get to be clone file list.
##################################################################
sub Get_CloneTobe
{
  my @tmplist = @_;
  my %return;
  foreach my $from (@tmplist)
  {
    my $tobe = $from;
    $tobe =~ s/$clone_project/$apply_project/g;
    $tobe =~ s/arm\//arm64\// if ($flag_64bit);
    $tobe =~ s/arm64\//arm\// if ($flag_32bit); 
    if($flag_64bit && $clone_kernel !~ /kernel-3.1/)
    {
      $tobe =~ s/\/boot\/dts\//\/boot\/dts\/mediatek\//g if ($tobe =~ /\/boot\/dts\/$apply_project\.(dts|dtsi)/);
    }
    else
    {
      $tobe =~ s/\/boot\/dts\/mediatek\//\/boot\/dts\//g if ($tobe =~ /\/boot\/dts\/mediatek\/$apply_project\.(dts|dtsi)/);
    }

    if ($company_change)
    {
      if($tobe !~ /boot\/.*\/$apply_project\.(dts|dtsi)/)
      {
        $tobe =~ s/\/$clone_company\/$apply_project/\/$apply_company\/$apply_project/g;
        $tobe =~ s/\/$clone_company\/libs\//\/$apply_company\/libs\//g if(!-d "$local_ws/device/mediateksample");
      }
    }
    $return{"$tobe"} = $from;
  }
  print "[Get_CloneTobe]: get to be clone file list. $apply_project.\n";
  return %return;
}


##################################################################
# function   : Clone_Source
# arguments  : $ref_clone (hash)
# description: clone project $clone_project, apply project $apply_project
##################################################################
sub Clone_Source
{
  my %hash = @_;
  my $ret;
  my $cmd;
  my @filelist;
  my $cloneerror;
  print "[Clone_Source]: Clone $clone_project source to $apply_project start.\n";
  foreach my $tobe (sort keys %hash)
  {
    my $from = $hash{"$tobe"};
    my $tobe_folder = $tobe;
    $tobe_folder =~ s/^(\/.*)\/.*$/$1/g;
    if(! -d $tobe_folder)
    {
      $cmd = "mkdir -p $tobe_folder";
      $ret = `$cmd`;
      print "[Clone_Source]: Create folder $tobe_folder\n";
    }

    if($tobe =~ /\/libs\/Android.mk/)
    {
      next if(-e $tobe);
    }

    $cmd = "cp -f $from $tobe";
    $ret = `$cmd`;
    if(!$ret)
    {
      push(@filelist,$tobe);
    }
    else
    {
      $cloneerror += $ret;
    }
    print "[Clone_Source]: Form $from , To $tobe finish\n";
  }
  print "[Clone_Source]: Clone $clone_project source to $apply_project finish.\n";
  return (@filelist, $cloneerror);
}

##################################################################
# function   : Replace_Content
# arguments  : clone file list array
# description: to do replace action by rule.
##################################################################
sub Replace_Content
{
  my @filelist = @_;
  my $check;
  print "[Replace_Content]: Replace content from $clone_project to $apply_project start.\n";

  # Not in clone file list and must do replace
  if ( -e "$local_ws/bootable/bootloader/lk/dev/lcm/rules.mk")
  {
    push(@filelist ,"$local_ws/bootable/bootloader/lk/dev/lcm/rules.mk");
  }

  foreach(@filelist)
  {
    chomp($_);
    my $file  = $_;
    my $type = &Get_File_Type($file);
    next if($type !~ /text/);
    print "[Replace_Content] $file file type is $type.\n";

    #common rule part1
    if ($file =~ /device\.mk/)
    {
      if(-d "$local_ws/device/mediateksample")
      {          
          &RunCommand("sed -i 's|vendor/.*/libs/|vendor/mediatek/libs/|g' $file");
      }
      else
      {
          &RunCommand("sed -i 's|vendor/$clone_company/libs/|vendor/$apply_company/libs/|g' $file");
      }
      &RunCommand("sed -i 's|vendor/$clone_company/overlay/|vendor/$apply_company/overlay/|g' $file");
    }    
    
    $check = &Cat_File_Grep($file,"$clone_project");
    if ($check)
    {
      &RunCommand("sed -i 's|$clone_project|$apply_project|g' $file");
    }

    if ($company_change)
    {
      $check = &Cat_File_Grep($file,"$clone_company");
      if ($check)
      {
        &RunCommand("sed -i 's|$clone_company/$apply_project/|$apply_company/$apply_project/|g' $file");
        &RunCommand("sed -i 's|$clone_company/$apply_project|$apply_company/$apply_project|g' $file");
        &RunCommand("sed -i 's|$clone_company/\$\(MTK_TARGET_PROJECT\)/|$apply_company/\$\(MTK_TARGET_PROJECT\)/|g' $file");
        &RunCommand("sed -i 's|$clone_company/\$\(MTK_BASE_PROJECT\)/|$apply_company/\$\(MTK_BASE_PROJECT\)/|g' $file");
      }
    }
    

    #### speciail file ####
    if ($_ =~ /device\/$apply_company\/$apply_project\/ProjectConfig\.mk/)
    {
      if($flag_64bit)
      {
        $check = &Cat_File_Grep($file,"MTK_K64_SUPPORT");
        my $line = "MTK_K64_SUPPORT=yes";
        &RunCommand("sed -i 's|MTK_K64_SUPPORT.*|$line|g' $file") if ($check);
        &WriteFile(">>$file","MTK_K64_SUPPORT=yes\n") if (!$check);
        print "[Replace_Content]: append feature option MTK_K64_SUPPORT=yes for create 64bit project.\n";
      }

      if($flag_32bit)
      {
        $check = &Cat_File_Grep($file,"MTK_K64_SUPPORT");
        my $line = "MTK_K64_SUPPORT=no";
        &RunCommand("sed -i 's|MTK_K64_SUPPORT.*|$line|g' $file") if ($check);
        &WriteFile(">>$file","MTK_K64_SUPPORT=no\n") if (!$check);
        print "[Replace_Content]: append feature option MTK_K64_SUPPORT=no for 32bit project.\n";
      }
    }
    elsif ($_ =~ /AndroidBoard\.mk/ or $_ =~ /full_.*\.mk/)
    {
      $check = &Cat_File_Grep($file,"PRELOADER_TARGET_PRODUCT");
      my $line = "PRELOADER_TARGET_PRODUCT ?= $apply_project";
      &RunCommand("sed -i 's|PRELOADER_TARGET_PRODUCT ?=.*|$line|g' $file") if ($check);

      if ($_ =~ /full_.*\.mk/)
      {
        $check = &Cat_File_Grep($file,"MTK_BASE_PROJECT");
        $line = 'MTK_BASE_PROJECT := $(MTK_TARGET_PROJECT)';
        &RunCommand("sed -i 's|MTK_BASE_PROJECT :=.*|$line|g' $file") if ($check);

        $check = &Cat_File_Grep($file,"PRODUCT_DEVICE");
        $line = 'PRODUCT_DEVICE := $(strip $(MTK_BASE_PROJECT))';
        &RunCommand("sed -i 's|PRODUCT_DEVICE :=.*|$line|g' $file") if ($check);

        if ($flag_64bit)
        {
          $check = &Cat_File_Grep($file,"core_64_bit.mk");
          if(!$check)
          {
            $line = '# Inherit for devices that support 64-bit primary and 32-bit secondary zygote startup script'."\n"
              .'$(call inherit-product, $(SRC_TARGET_DIR)/product/core_64_bit.mk)'."\n\n";
            open (FULLMK_R, "<$file") or die $!;
            while (my $tmpline =<FULLMK_R>)
            {
              $line .= $tmpline;
            }
            close(FULLMK_R);
            open (FULLMK_W, ">$file") or die $!;
            print FULLMK_W $line;
            close(FULLMK_W);
          }
        }
      
        if ($flag_32bit)
        {
          $check = &Cat_File_Grep($file,"core_64_bit.mk");
          if($check)
          {
            $line = "# Inherit for devices that support 64-bit primary and 32-bit secondary zygote startup script\\n"
            ."\$(call inherit-product, \$(SRC_TARGET_DIR)/product/core_64_bit.mk)\\n";
            &RunCommand("sed -i ':a;N;\$\!ba;s|$line||g' $file");
          }
        }
      }
    }
    elsif ($_ =~ /.*_defconfig/)
    {
      #DTB policy , requester Chia-Hao Hsu 2017/06/16
      my $clone_project_dts = "$local_ws/$clone_kernel/arch/arm/boot/dts/$clone_project.dts";
      if("$clone_set" eq "64")
      {
        $clone_project_dts = "$local_ws/$clone_kernel/arch/arm64/boot/dts/$clone_project.dts";
        $clone_project_dts = "$local_ws/$clone_kernel/arch/arm64/boot/dts/mediatek/$clone_project.dts" if ($clone_kernel !~ /kernel-3.1/);
      }
      my $platform_dts = `grep -m 1 '#include [<\"]mt.*\.dts[>\"]' $clone_project_dts | sed 's/#include [<"]//g' | sed 's/\.dts[>"]//g'`;
      chomp($platform_dts);
      my $dtb_overlay_64_fo = "CONFIG_BUILD_ARM64_DTB_OVERLAY_IMAGE";
      my $dtb_overlay_32_fo = "CONFIG_BUILD_ARM_DTB_OVERLAY_IMAGE";
      my $dtb_overlay_fo;
      my $dtb_64_fo = "CONFIG_BUILD_ARM64_APPENDED_DTB_IMAGE";
      my $dtb_32_fo = "CONFIG_BUILD_ARM_APPENDED_DTB_IMAGE";
      my $dtb_fo;

      if($flag_64bit)
      {
        $dtb_overlay_fo = $dtb_overlay_64_fo;
        $dtb_fo = $dtb_64_fo;
        &RunCommand("sed -i 's|$dtb_overlay_32_fo|$dtb_overlay_64_fo|g' $file");
        &RunCommand("sed -i 's|$dtb_32_fo|$dtb_64_fo|g' $file");
      }

      if($flag_32bit)
      {
        $dtb_overlay_fo = $dtb_overlay_32_fo;
        $dtb_fo = $dtb_32_fo;
        &RunCommand("sed -i 's|$dtb_overlay_64_fo|$dtb_overlay_32_fo|g' $file");
        &RunCommand("sed -i 's|$dtb_64_fo|$dtb_32_fo|g' $file");
      }

      $check = &Cat_File_Grep($file, "CONFIG_MTK_DTBO_FEATURE is not set");
      if($check)
      {
        my $line = "$dtb_fo\_NAMES=\"$apply_project\"";
        $line = "$dtb_fo\_NAMES=\"mediatek/$apply_project\"" if ($clone_kernel !~ /kernel-3.1/ && $flag_64bit);
        &RunCommand("sed -i 's|$dtb_fo\_NAMES=.*|$line|g' $file");
        $line = "$dtb_overlay_fo\_NAMES=\"\"";
        &RunCommand("sed -i 's|$dtb_overlay_fo\_NAMES=.*|$line|g' $file");
      }
      else
      {
        $check = &Cat_File_Grep($file, "$dtb_overlay_fo");
        if($check)
        {
          my $line = "$dtb_fo\_NAMES=\"$platform_dts\"";
          $line = "$dtb_fo\_NAMES=\"mediatek/$platform_dts\"" if ($clone_kernel !~ /kernel-3.1/ && $flag_64bit);
          &RunCommand("sed -i 's|$dtb_fo\_NAMES=.*|$line|g' $file") if("$platform_dts" ne "");
          $line = "$dtb_overlay_fo\_NAMES=\"$apply_project\"";
          $line = "$dtb_overlay_fo\_NAMES=\"mediatek/$apply_project\"" if ($clone_kernel !~ /kernel-3.1/ && $flag_64bit);
          &RunCommand("sed -i 's|$dtb_overlay_fo\_NAMES=.*|$line|g' $file");
        }
        else
        {
          my $line = "$dtb_fo\_NAMES=\"$apply_project\"";
          $line = "$dtb_fo\_NAMES=\"mediatek/$apply_project\"" if ($clone_kernel !~ /kernel-3.1/ && $flag_64bit);
          &RunCommand("sed -i 's|$dtb_fo\_NAMES=.*|$line|g' $file");
        }
      }
    }
    elsif ($_ =~ /vendorsetup\.sh/)
    {
      my $tmpline = &Cat_File_Grep($file,"-eng\$");
      chomp ($tmpline);
      my $line;
      my $wline;

      $check = &Cat_File_Grep($file,"-user\$");
      if (!$check)
      {
        $line = $tmpline;
        $line =~ s/eng/user/;
        $wline .= "$line\n";
      }
      $check = &Cat_File_Grep($file,"-userdebug\$");
      if (!$check)
      {
        $line = $tmpline;
        $line =~ s/eng/userdebug/;
        $wline .= "$line\n";
      }
      &WriteFile(">>$file","$wline") if($wline);
    }
    elsif ($_ =~ /lk\/dev\/lcm\/rules\.mk/)
    {
      $check = &Cat_File_Grep($file,"$clone_project");
      if ($check)
      {
        $check = &Cat_File_Grep($file,"$apply_project");
        if (!$check)
        {
          my $flag = 0;
          my $addcontent;
          open FILE, "$file" or die $!;
          while (my $line = <FILE>)
          {
            $flag = 1 if ($line =~ /$clone_project/);
            if ($flag)
            {
              $addcontent .= $line;
              if ($line =~/endif/)
              {
                $flag = 0;
                last;
              }
            }
          }
          close FILE;
          $addcontent =~ s|(ifeq.*)$clone_project\)|$1$apply_project\)|g;
          &WriteFile(">>$file","$addcontent");
        }
      }
    }
  }
  print "[Replace_Content]: Replace content from $clone_project to $apply_project finish.\n";
  return @filelist;
}
##################################################################
# function   : Replace_Content
# arguments  : clone file list array
# description: to do replace action by rule.
##################################################################
sub Gen_File_List
{
  my @filelist=@_;
  my $logfile= $apply_project."_source.log";

  open (OFOUT, "> $logfile");
  print OFOUT "### Create $apply_company/$apply_project from $clone_company/$clone_project ###\n";
  print OFOUT "-------------------------------------------------------------------------------\n";
  foreach(@filelist)
  {
    if($_ ne "")
    {
      print OFOUT "$_\n";
    }
  }
  print OFOUT "-------------------------------------------------------------------------------\n";
  close(OFOUT);
  my $path=`pwd`;
  chomp($path);
  print "[File_List]: Save $apply_project source file list in $path/$logfile\n";
}

##################################################################
# function   : common function
##################################################################
sub Cat_File_Grep
{
  my $file = shift;
  my $string = shift;
  my $cmd = "grep '$string' $file";
  my $ret = `$cmd`;
  return $ret;
}

sub trim
{
  my $string = shift;
  chomp($string);
  $string =~ s/^\s+//;
  $string =~ s/\s+$//;
  return $string;
}

##################################################################
# function   : RunCommand
# arguments  : $cmd
# description: run $cmd and error return.
##################################################################
sub RunCommand
{
  my $cmd = shift;
  my $ret = system("$cmd");
  if ($ret)
  {
    print "[Error]: [RunCommand]: \"$cmd\":$ret\n";
    exit 11;
  }
  print "[RunCommand]: \"$cmd\":$ret\n";
  return $ret;
}

sub WriteFile
{
  my $file = shift;
  my $content = shift;
  open (WF, "$file") || print "couldn't open context file: $!";
  print WF "$content";
  close (WF);
  return 0;
}

##################################################################
# function   : ParseParameters
# arguments  : @ARGV
# description: script parameter parse.
##################################################################
sub ParseParameters
{
  print "### perl $0 @ARGV ###\n";
  while($#ARGV != -1)
  {
    if ($ARGV[0] =~ /^-o$/i)
    {
      $clone_project =  $ARGV[1];
      if ($clone_project =~ /(.*)\/(.*)/)
      {
        $clone_company = $1;
        $clone_project = $2;
      }
    }
    elsif ($ARGV[0] =~ /^-n$/i)
    {
      $apply_project =  $ARGV[1];
      if ($apply_project =~ /(.*)\/(.*)/)
      {
        $apply_company = $1;
        $apply_project = $2;
      }
    }
    elsif ($ARGV[0] =~ /^-p$/i)
    {
      $local_ws =  $ARGV[1];
    }
    elsif ($ARGV[0] =~ /^-64$/i)
    {
      $flag_64bit = 1;
    }
    elsif ($ARGV[0] =~ /^-32$/i)
    {
      $flag_32bit = 1;
    }
    shift(@ARGV);
  }
}

##################################################################
# function   : Usage
# arguments  : wrong arg.
# description: script usage help description.
##################################################################
sub Usage
{
  warn << "__END_OF_USAGE";
####################################################################################################################################
Usage:
  [Project Creation]
    perl project_clone.pl -p {codebase_full_path} -o {base_company/base_project} -n {new_company/new_project} [-32|-64]
####################################################################################################################################
Arguments:
  [General arg.]
    -p  : source codebase full path in local device.(usually is release package uncompress path)
    -o  : base_company/base_project which the new project created from, support cross company.
    -n  : new_company/new_project need to be created. (default:new_company the same as base_company)

  [Non-essential arg.]
    [-32] : Downgrade to 32bit form 64bit. (default: the same clone_project)
    [-64] : Upgrade to 64bit form 32bit. (default: the same clone_project)
####################################################################################################################################
Example:
  Create Project tinno/tinno82_we_aosp_kk from mediatek/mt6582_phone_qhd:
    perl project_clone.pl -p "/home/user/release/v1.2.1/alps" -o "mediateksample/tk67xx_n" --n "mtk/mtk67xx_n_copy"
####################################################################################################################################
__END_OF_USAGE
  exit 1;
}

sub ShowInfo
{
  warn << "__END_OF_USAGE";
##################################################################
[ARGV] AOSP Project Creation Information
##################################################################
[ARGV] cosebase type: $codebase_type
[ARGV] clone company/project: $clone_company/$clone_project
[ARGV] clone project platform: $clone_platform
[ARGV] clone project kernel: $clone_kernel
[ARGV] clone project set: $clone_set bit
[ARGV] clone project Projectconfig.mk path: $clone_project_config_path

[ARGV] apply company/project: $apply_company/$apply_project
[ARGV] apply project Projectconfig.mk path: $apply_project_config_path
##################################################################
__END_OF_USAGE
  return 0;
}