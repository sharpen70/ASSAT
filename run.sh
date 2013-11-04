#!/bin/sh

benchpath="benchmark/hc/"
inputpath="benchmark/hc/gringo_input/"
progpath="dist/Debug/GNU-Linux-x86/"
outputpath="benchmark/hc/result/"


for i in `ls $inputpath`;do
	${progpath}assat "${inputpath}${i}" "${outputpath}${i}.res" "${benchpath}improve_summary"
	echo "$i done"
done
