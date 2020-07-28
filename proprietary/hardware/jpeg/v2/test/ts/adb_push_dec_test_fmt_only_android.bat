
adb remount

adb shell mount -o remount rw /

adb shell mkdir /data/otis

adb shell rm -r /data/otis/*

::
adb shell touch /data/otis/DUMP_DEC_PIPE_1
::
adb shell touch /data/otis/DUMP_DEC_PIPE_2
::
adb shell touch /data/otis/DUMP_DEC_PIPE_3
::

::
adb shell touch /data/otis/DUMP_CDP_PIPE_1
::
::adb shell touch /data/otis/DUMP_CDP_PIPE_2
::
adb shell touch /data/otis/DUMP_CDP_PIPE_3


::adb push ../../../../../../../../out/target/product/mt6589_evb1/system/lib/libmhaldrv.so /system/lib
::
::adb push ../../../../../../../../out/target/product/mt6589_evb1/system/lib/libutils.so /system/lib
::
::adb push ../../../../../../../../out/target/product/mt6589_evb1/system/lib/libcutils.so /system/lib
::
::adb push ../../../../../../../../out/target/product/mt6589_evb1/system/lib/libdeltat.so /system/lib
::
::adb push ../../../../../../../../out/target/product/mt6589_evb1/system/lib/libGdmaScalerPipe.so /system/lib
::
::adb push ../../../../../../../../out/target/product/mt6589_evb1/system/lib/libJpgDecPipe.so /system/lib
::
::adb push ../../../../../../../../out/target/product/mt6589_evb1/system/lib/liblog.so /system/lib
::
::adb push ../../../../../../../../out/target/product/mt6589_evb1/system/lib/libstlport.so /system/lib
::
::adb push ../../../../../../../../out/target/product/mt6589_evb1/system/lib/libcamdrv.so /system/lib
::
::adb push ../../../../../../../../out/target/product/mt6589_evb1/system/lib/libimageio.so /system/lib
::
::adb push ../../../../../../../../out/target/product/mt6589_evb1/system/lib/libcameracustom.so /system/lib
::
::adb push ../../../../../../../../out/target/product/mt6589_evb1/system/lib/libcameraprofile.so /system/lib
::
::adb push ../../../../../../../../out/target/product/mt6589_evb1/system/lib/libmmprofile.so /system/lib
::
::adb push ../../../../../../../../out/target/product/mt6589_evb1/system/lib/libcam_camera_exif.so /system/lib
::
::adb push ../../../../../../../../out/target/product/mt6589_evb1/system/lib/libm4u.so /system/lib
::
::adb push ../../../../../../../../out/target/product/mt6589_evb1/system/lib/libscenario.so /system/lib
::
::adb push ../../../../../../../../out/target/product/mt6589_evb1/system/lib/libfeatureio.so /system/lib
::
::adb push ../../../../../../../../out/target/product/mt6589_evb1/system/lib/libcam.camadapter.so /system/lib
::
::adb push ../../../../../../../../out/target/product/mt6589_evb1/system/lib/libacdk.so /system/lib
::
::adb push ../../../../../../../../out/target/product/mt6589_evb1/system/lib/libvcodecdrv.so /system/lib
::
::adb push ../../../../../../../../out/target/product/mt6589_evb1/system/lib/libvcodec_oal.so /system/lib
::
::adb push ../../../../../../../../out/target/product/mt6589_evb1/system/lib/libvcodec_utility.so /system/lib
::
::adb push ../../../../../../../../out/target/product/mt6589_evb1/system/lib/libbinder.so /system/lib
::
::adb push ../../../../../../../../out/target/product/mt6589_evb1/system/lib/libmp4enc_xa.ca7.so /system/lib
::
::adb push ../../../../../../../../out/target/product/mt6589_evb1/system/lib/modules/vcodec_kernel_driver.ko /system/lib/modules/vcodec_kernel_driver.ko


::adb push ../0079_BL_YUV420_0640x0480_no_exif.jpg /data/otis/test.jpg
::adb push ../test_normal_0147_420_200x200.jpg /data/otis/test.jpg
::adb push ../test_normal_0026_420_176x220.jpg /data/otis/test.jpg

:: jpeg tile test
::adb push ../0076_BL_YUV422_0640x0480_no_exif.jpg /data/otis/test.jpg

:: 422
::adb push ../test_nb_0091_2560x1920.jpg /data/otis/test.jpg


::adb push ../../../../../../../mediatek/platform/mt6583/hardware/jpeg/test/vp80-01-intra-1411.webp.ivf /data/otis/test.jpg

::adb push ../../../../../../../mediatek/platform/mt6583/hardware/jpeg/test/earth_256x256.webp /data/otis/test.jpg

::adb push ../../../../../../../mediatek/platform/mt6583/hardware/jpeg/test/mosta_400x300.webp /data/otis/test.jpg

::adb push ../../../../../../../mediatek/platform/mt6583/hardware/jpeg/test/mahasi_2970x2070.webp /data/otis/test.jpg

::adb push ../fmt/post_0_1920x1088_blk_Y.bin /data/otis/src_blk_y.bin
::
::adb push ../fmt/post_0_1920x1088_blk_CbCr.bin /data/otis/src_blk_c.bin

::adb push ../fmt/post_0_176x144_blk_Y.bin /data/otis/src_blk_y.bin
::
::adb push ../fmt/post_0_176x144_blk_CbCr.bin /data/otis/src_blk_c.bin

adb push ../fmt/post_0_fc_1920x1088_blk_Y.bin /data/otis/src_blk_y.bin

adb push ../fmt/post_0_fc_1920x1088_blk_CbCr.bin /data/otis/src_blk_c.bin




adb push ../../../../../../../../out/target/product/mt6589_evb1/system/bin/mhalJpegTest /system/bin/

adb shell chmod 777 /system/bin/mhalJpegTest

::adb shell chmod 777 /dev/mtk_gdma


