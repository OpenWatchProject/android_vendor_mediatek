#!/bin/bash

astyle_param="--style=kr --indent=spaces=4 --indent-switches --max-code-length=120 --pad-header --pad-oper --convert-tabs --unpad-paren --suffix=none"
echo $astyle_param

list=$(find . -regex '.*\.c\|.*\.cpp\|.*\.h\|.*\.hpp')

for file in $list
do
    #echo "chmod 644 $file"
    chmod 644 $file
    
    #echo "astyle $astyle_param $file"
    astyle $astyle_param $file
done

