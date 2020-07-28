#!/bin/bash
n=$1
s=$2
f=$3

#$4 indicate the format we want i.e. "gp" or "legacy".  If absent, "gp" is applied.

if [ -z "$4" ]
  then
    echo "#define ${n}_UUID { 0x${s:0:8}, 0x${s:8:4}, 0x${s:12:4}, { 0x${s:16:2}, 0x${s:18:2}, 0x${s:20:2}, 0x${s:22:2}, 0x${s:24:2}, 0x${s:26:2}, 0x${s:28:2}, 0x${s:30:2} }} " > $f
else
  if [ "$4" == "legacy" ]; then
    echo "#define ${n}_UUID {{ 0x${s:0:2}, 0x${s:2:2}, 0x${s:4:2}, 0x${s:6:2}, 0x${s:8:2}, 0x${s:10:2}, 0x${s:12:2}, 0x${s:14:2}, 0x${s:16:2}, 0x${s:18:2}, 0x${s:20:2}, 0x${s:22:2}, 0x${s:24:2}, 0x${s:26:2}, 0x${s:28:2}, 0x${s:30:2} }}" > $f
  elif [ "$4" == "gp" ]; then
    echo "#define ${n}_UUID { 0x${s:0:8}, 0x${s:8:4}, 0x${s:12:4}, { 0x${s:16:2}, 0x${s:18:2}, 0x${s:20:2}, 0x${s:22:2}, 0x${s:24:2}, 0x${s:26:2}, 0x${s:28:2}, 0x${s:30:2} }} " > $f
  else 
     echo "Error: unrecognised format parameter value: " $4
  fi
fi



