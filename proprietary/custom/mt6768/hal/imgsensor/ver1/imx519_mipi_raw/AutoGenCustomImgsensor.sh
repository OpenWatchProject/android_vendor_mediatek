#!/bin/bash
# Decroption:
#       Generate target custom/imgsensor folder automatically.
# User Manual:
#       Please modify variable: REF_SENSOR & TAR_SENSOR
# History:
#       2017.01.11   Support recursive editing
#       2016.09.12   

# Modify reference sensor name
REF_SENSOR="imx519"

# Modify target sensor name
TAR_SENSOR="imx519"



###################AUTO GEN PART######################################
REF_SENSOR_UPPER=${REF_SENSOR^^}
TAR_SENSOR_UPPER=${TAR_SENSOR^^}

REF_PATH=${REF_SENSOR}_mipi_raw
TAR_PATH=${TAR_SENSOR}_mipi_raw

# Copy from reference files
cp -r ${REF_PATH} ${TAR_PATH}

# Rename files recursively
find ${TAR_PATH} -name "*${REF_SENSOR}*" -exec rename "s/${REF_SENSOR}/${TAR_SENSOR}/" '{}' \;

# Edit lowercase text recursively
grep -rl ${REF_SENSOR} ${TAR_PATH} | xargs sed -i "s/${REF_SENSOR}/${TAR_SENSOR}/g"

# Edit uppercase text recursively
grep -rl ${REF_SENSOR_UPPER} ${TAR_PATH} | xargs sed -i "s/${REF_SENSOR_UPPER}/${TAR_SENSOR_UPPER}/g"

echo "Done auto generate folder:${TAR_PATH}"
echo "Please check it!"
