#!/bin/sh

echo "Usage: $0 startchunk"
sleep 3

startchunk="$1"
if [ -z "$startchunk" ]; then
	startchunk=0
fi

outdir=chunks/
mkdir "$outdir"
#rm "$outdir"/*

#partstart=11206656 # 0xAB0000
partstart=2818048 # 0x2b0000

#chunksize=12288
chunksize=4096

#totalsize=5570560 # 5440KB
#totalsize=5505024 # 5376KB so 64KB margin
#totalsizekb=5376
totalsizekb=6592
totalsize=$(expr $totalsizekb \* 1024)

totalchunks=$(expr $totalsize / $chunksize)

# not needed because it starts counting from 0 AND includes the last one (due to seq): totalchunks=$(expr $totalchunks + 1)
# seq - 1 would not be good because expr does floor() so it might be a decimal which means the last part would be lost

outfile=vfs-fat-partition-"$totalsizekb"KB-chunk

for chunk in $(seq $startchunk $totalchunks); do
	start=$(expr $partstart + $chunk \* $chunksize)
	echo "start = $start and chunk = $chunk"

	result=1
	while [ $result -ne 0 ]; do
		padname=$(printf '%04d' "$chunk")
		#esptool.py read_flash $start $chunksize "$outdir"/vfs-fat-partition-5440KB-chunk-$padname.bin &
		#esptool.py -b 460080 read_flash $start $chunksize "$outdir"/vfs-fat-partition-5376KB-chunk-$padname.bin &
		esptool.py read_flash $start $chunksize "$outdir"/"$outfile"-"$padname".bin &
		bgpid=$!
		sleep 4
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
