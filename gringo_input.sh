#!/bin/sh

echo "fuck my life"

path="benchmark/hc/facts/"

for i in `ls $path`;do
	gringo -t benchmark/hc/hc.lp "$path${i}" > benchmark/hc/gringo_input/${i}.input  
done
