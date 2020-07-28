CHKSUMFOLDER=$(pwd)/../../../../../../hardware/mtkcam/aaa/source/common/utils/gen_exif_tag_chksum_folder
CHKSUMEXE=$(pwd)/../../../../../../hardware/mtkcam/aaa/source/common/utils/AdlerCheck
MTK_CAM_EXIF_TAG_PATH=$(pwd)/cam
OUT_PATH=$(pwd)

#echo $CHKSUMFOLDER
#echo $CHKSUMEXE
#echo $MTK_CAM_EXIF_TAG_PATH
#echo $OUT_PATH
#sh $CHKSUMFOLDER $MTK_CAM_EXIF_TAG_PATH $CHKSUMEXE

rm -rf $OUT_PATH/cam_exif_tag_chksum.h
echo "$(sh $CHKSUMFOLDER $MTK_CAM_EXIF_TAG_PATH $CHKSUMEXE)" >> $OUT_PATH/cam_exif_tag_chksum.h
echo "output: " $OUT_PATH/cam_exif_tag_chksum.h
