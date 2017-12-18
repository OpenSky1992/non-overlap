#!/bin/bash

echo $1
curDir=$(pwd)

if [ -d $1 ]; then 
	cd $1/pcap
	for name in $(ls); do
		tcpdump -q -nn -tt -r $name >tempFile
		$curDir/build/parseTcpdump $1/parsedTrace/$name <tempFile
		rm tempFile
	done
else
	echo "the directory $1 don't exist"  
fi
