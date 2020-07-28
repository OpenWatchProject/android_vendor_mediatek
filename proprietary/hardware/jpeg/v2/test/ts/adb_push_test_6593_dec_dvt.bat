
adb remount

adb shell mount -o remount rw /

adb shell mkdir /data/otis

::adb shell rm -r /data/otis/*

adb shell mkdir /system/bin/out

adb shell mkdir /system/bin/pat

adb shell rm -r /system/bin/out/*

adb shell mkdir /data/otis/pattern

adb shell mkdir /data/otis/pattern/normal

adb shell mkdir /data/otis/output/

adb push U:\seq\jpeg\fpga_jpeg\regFileList /data/otis/pattern/normal

::adb push U:\seq\jpeg\fpga_jpeg /data/otis/pattern/normal

::
::adb shell touch /data/otis/DUMP_DEC_PIPE_1
::
::adb shell touch /data/otis/DUMP_DEC_PIPE_2
::
::adb shell touch /data/otis/DUMP_DEC_PIPE_3
::
::adb shell touch /data/otis/DUMP_DEC_PIPE_5

::
::adb shell touch /data/otis/DUMP_CDP_PIPE_1
::
::adb shell touch /data/otis/DUMP_CDP_PIPE_2
::
::adb shell touch /data/otis/DUMP_CDP_PIPE_3

adb shell setprop dpframework.dumpbuffer.enable 0

adb push ../../../../../../../../out/target/product/mt6595fpga_ldvt/root/system/lib/libmhalImageCodec.so /system/lib

adb push ../../../../../../../../out/target/product/mt6595fpga_ldvt/root/system/lib/libSwJpgCodec.so /system/lib


adb push ../../../../../../../../out/target/product/mt6595fpga_ldvt/root/system/lib/libutils.so /system/lib

adb push ../../../../../../../../out/target/product/mt6595fpga_ldvt/root/system/lib/libcutils.so /system/lib


adb push ../../../../../../../../out/target/product/mt6595fpga_ldvt/root/system/lib/libGdmaScalerPipe.so /system/lib

adb push ../../../../../../../../out/target/product/mt6595fpga_ldvt/root/system/lib/libJpgDecPipe.so /system/lib

adb push ../../../../../../../../out/target/product/mt6595fpga_ldvt/root/system/lib/liblog.so /system/lib

adb push ../../../../../../../../out/target/product/mt6595fpga_ldvt/root/system/lib/libjpeg.so /system/lib

adb push ../../../../../../../../out/target/product/mt6595fpga_ldvt/root/system/lib/libm4u.so /system/lib

adb push ../../../../../../../../out/target/product/mt6595fpga_ldvt/root/system/lib/libJpgEncPipe.so /system/lib

adb push ../../../../../../../../out/target/product/mt6595fpga_ldvt/root/system/lib/libc_fdleak_debug_mtk.so /system/lib

adb push ../../../../../../../../out/target/product/mt6595fpga_ldvt/root/system/lib/libc_malloc_debug_mtk.so /system/lib

adb push ../../../../../../../../out/target/product/mt6595fpga_ldvt/root/system/lib/libdpframework.so /system/lib

::DRI
::adb push ../0079_BL_YUV420_0640x0480_no_exif.jpg /data/otis/test.jpg

:: jpeg tile test
::adb push ../0076_BL_YUV422_0640x0480_no_exif.jpg /data/otis/test.jpg


adb push ../IMG_20120104_000617.jpg /data/otis/test.jpg

adb push ../0001_BL_YUV420_0240x0320.jpg /data/otis/pattern/normal/0001_BL_YUV420_0240x0320.jpg

adb push ../0001_BL_YUV420_0240x0320.raw /data/otis/golden.raw


::adb push ../../../../../../../mediatek/hardware/jpeg/mt6735/test/vp80-01-intra-1411.webp.ivf /data/otis/test.jpg

::adb push ../../../../../../../mediatek/hardware/jpeg/mt6735/test/earth_256x256.webp /data/otis/test.jpg

::adb push ../../../../../../../mediatek/hardware/jpeg/mt6735/test/mosta_400x300.webp /data/otis/test.jpg

::adb push ../../../../../../../mediatek/hardware/jpeg/mt6735/test/mahasi_2970x2070.webp /data/otis/test.jpg

::adb push ../yuv_src_800x600_420.hex /data/otis/yuv_src.hex

::adb push ../yuv_src_800x600_422.hex /data/otis/yuv_src.hex

adb push ../../../../../../../../out/target/product/mt6595fpga_ldvt/root/system/bin/mhalJpegTest /system/bin/
         
adb shell chmod 777 /system/bin/mhalJpegTest

::adb shell chmod 777 /dev/mtk_gdma


::adb push ../../../../../../../../out/target/product/mt6595fpga_ldvt/root/system/bin/UnitTest /system/bin/

::adb shell chmod 777 /system/bin/UnitTest


::adb push ../../../dpframework/src/test/unitTest/pat/640x480.bmp /system/bin/pat/640x480.bmp


::adb push U:\seq\jpeg\fpga_jpeg\0001_BL_YUV420_0240x0320.jpg /data/otis/pattern/normal
::
::adb push U:\seq\jpeg\fpga_jpeg\0003_BL_YUV444_0328x0390.jpg  /data/otis/pattern/normal
::
::adb push U:\seq\jpeg\fpga_jpeg\0004_BL_YUV444_1024x0768.jpg /data/otis/pattern/normal
::
::adb push U:\seq\jpeg\fpga_jpeg\0005_BL_YUV420_0100x0100.jpg /data/otis/pattern/normal
::
::adb push U:\seq\jpeg\fpga_jpeg\0006_BL_YUV420_0120x0160.jpg /data/otis/pattern/normal
::
::adb push U:\seq\jpeg\fpga_jpeg\0007_BL_YUV420_0160x0120.jpg /data/otis/pattern/normal
::
::adb push U:\seq\jpeg\fpga_jpeg\0008_BL_YUV420_0128x0160.jpg /data/otis/pattern/normal
::
::adb push U:\seq\jpeg\fpga_jpeg\0009_BL_YUV444_0128x0160.jpg /data/otis/pattern/normal
::
::adb push U:\seq\jpeg\fpga_jpeg\0011_BL_YUV420_0128x0120.jpg /data/otis/pattern/normal
::
::adb push U:\seq\jpeg\fpga_jpeg\0013_BL_YUV420_0128x0160.jpg /data/otis/pattern/normal







