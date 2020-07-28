#!/bin/sh

rm -rf build/res
mkdir -p build/
cp -rf res/ build/
find build/res -name '*string*.xml'|xargs grep -l "nosdcard" |xargs sed -i "/nosdcard/d"
find build/res -name '*.xml'|xargs grep -l "Theme.GalleryBase" |xargs sed -i 's/Theme.GalleryBase/Theme._GalleryBase/g'
find build/res -name '*.xml'|xargs grep -l "name=\"listPreferredItemHeightSmall" |xargs sed -i 's/name=\"listPreferredItemHeightSmall/name=\"_listPreferredItemHeightSmall/g'
find build/res -name '*.xml'|xargs grep -l "name=\"switchStyle" |xargs sed -i 's/name=\"switchStyle/name=\"_switchStyle/g'
mv build/res/drawable/filtershow_state_button_background build/res/drawable/filtershow_state_button_background.png
