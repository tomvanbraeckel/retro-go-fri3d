#!/bin/sh

# esptool.py read_flash 0xAB0000 0x550000 vfs-fat-partition-5440KB-size.bin

outdir=chunks/
mkdir "$outdir"
rm "$outdir"/*

#chunksize=4096
chunksize=12288
totalsize=5570560
totalchunks=$(expr $totalsize / $chunksize)
for chunk in $(seq 0 $totalchunks); do
	start=$(expr 11206656 + $chunk \* $chunksize)
	echo "start = $start and chunk = $chunk"

	result=1
	while [ $result -ne 0 ]; do
		padname=$(printf '%04d' "$chunk")
		#esptool.py read_flash $start $chunksize "$outdir"/vfs-fat-partition-5440KB-chunk-$padname.bin &
		esptool.py -b 460080 read_flash $start $chunksize "$outdir"/vfs-fat-partition-5440KB-chunk-$padname.bin &
		bgpid=$!
		sleep 3
		if [ -d "/proc/$bgpid" ]; then
			echo "Still running, killing!"
			killall python3
			killall -9 python3
			sleep 1
			result=2
		else
			wait "$bgpid"
			export result=$?
		fi
	done
done
