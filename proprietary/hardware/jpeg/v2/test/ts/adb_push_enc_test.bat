
adb remount

adb shell mount -o remount rw /

adb shell mkdir /data/otis

adb shell rm -r /data/otis/*

::
::adb shell touch /data/otis/DUMP_DEC_PIPE_1
::
::adb shell touch /data/otis/DUMP_DEC_PIPE_2
::
::adb shell touch /data/otis/DUMP_DEC_PIPE_3
::

::
::adb shell touch /data/otis/DUMP_CDP_PIPE_1
::
::adb shell touch /data/otis/DUMP_CDP_PIPE_2
::
::adb shell touch /data/otis/DUMP_CDP_PIPE_3


::adb push ../../../../../../../../out/target/product/mt6582_fpga/system/lib/libmhaldrv.so /system/lib
::
adb push ../../../../../../../../out/target/product/mt6582_fpga/system/lib/libutils.so /system/lib
::
adb push ../../../../../../../../out/target/product/mt6582_fpga/system/lib/libcutils.so /system/lib
::
::adb push ../../../../../../../../out/target/product/mt6582_fpga/system/lib/libdeltat.so /system/lib
::
::adb push ../../../../../../../../out/target/product/mt6582_fpga/system/lib/libGdmaCdp_pipe.so /system/lib
::
::adb push ../../../../../../../../out/target/product/mt6582_fpga/system/lib/libJpgDecPipe.so /system/lib
::
adb push ../../../../../../../../out/target/product/mt6582_fpga/system/lib/liblog.so /system/lib
::
::adb push ../../../../../../../../out/target/product/mt6582_fpga/system/lib/libstlport.so /system/lib
::
::adb push ../../../../../../../../out/target/product/mt6582_fpga/system/lib/libcamdrv.so /system/lib
::
::adb push ../../../../../../../../out/target/product/mt6582_fpga/system/lib/libimageio.so /system/lib
::
::adb push ../../../../../../../../out/target/product/mt6582_fpga/system/lib/libcameracustom.so /system/lib
::
::adb push ../../../../../../../../out/target/product/mt6582_fpga/system/lib/libcameraprofile.so /system/lib
::
::adb push ../../../../../../../../out/target/product/mt6582_fpga/system/lib/libmmprofile.so /system/lib
::
::adb push ../../../../../../../../out/target/product/mt6582_fpga/system/lib/libcam_camera_exif.so /system/lib
::
adb push ../../../../../../../../out/target/product/mt6582_fpga/system/lib/libm4u.so /system/lib

adb push ../../../../../../../../out/target/product/mt6582_fpga/system/lib/libJpgEncPipe.so /system/lib

::
::adb push ../../../../../../../../out/target/product/mt6582_fpga/system/lib/libscenario.so /system/lib
::
::adb push ../../../../../../../../out/target/product/mt6582_fpga/system/lib/libfeatureio.so /system/lib
::
::adb push ../../../../../../../../out/target/product/mt6582_fpga/system/lib/libcam.camadapter.so /system/lib
::
::adb push ../../../../../../../../out/target/product/mt6582_fpga/system/lib/libacdk.so /system/lib
::
::adb push ../../../../../../../../out/target/product/mt6582_fpga/system/lib/libvcodecdrv.so /system/lib
::
::adb push ../../../../../../../../out/target/product/mt6582_fpga/system/lib/libvcodec_oal.so /system/lib
::
::adb push ../../../../../../../../out/target/product/mt6582_fpga/system/lib/libvcodec_utility.so /system/lib
::
::adb push ../../../../../../../../out/target/product/mt6582_fpga/system/lib/libbinder.so /system/lib
::
::adb push ../../../../../../../../out/target/product/mt6582_fpga/system/lib/libmp4enc_xa.ca7.so /system/lib
::
::adb push ../../../../../../../../out/target/product/mt6582_fpga/system/lib/modules/vcodec_kernel_driver.ko /system/lib/modules/vcodec_kernel_driver.ko


::adb push ../../../../../../../mediatek/hardware/jpeg/mt6735/test/0079_BL_YUV420_0640x0480_no_exif.jpg /data/otis/test.jpg

:: jpeg tile test
::adb push ../../../../../../../mediatek/hardware/jpeg/mt6735/test/0076_BL_YUV422_0640x0480_no_exif.jpg /data/otis/test.jpg

::adb push ../../../../../../../mediatek/hardware/jpeg/mt6735/test/vp80-01-intra-1411.webp.ivf /data/otis/test.jpg

::adb push ../../../../../../../mediatek/hardware/jpeg/mt6735/test/earth_256x256.webp /data/otis/test.jpg

::adb push ../../../../../../../mediatek/hardware/jpeg/mt6735/test/mosta_400x300.webp /data/otis/test.jpg

::adb push ../../../../../../../mediatek/hardware/jpeg/mt6735/test/mahasi_2970x2070.webp /data/otis/test.jpg

::adb push ../yuv_src_800x600_420.hex /data/otis/yuv_src.hex

adb push ../yuv_src_800x600_422.hex /data/otis/yuv_src.hex

adb push ../../../../../../../../out/target/product/mt6582_fpga/system/bin/mhalJpegTest /system/bin/
         
adb shell chmod 777 /system/bin/mhalJpegTest

::adb shell chmod 777 /dev/mtk_gdma


