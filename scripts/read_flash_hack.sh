#!/bin/sh

echo "Usage: $0 startchunk"
sleep 3

startchunk="$1"
if [ -z "$startchunk" ]; then
	startchunk=0
fi

outdir=read_flash_hack/
mkdir "$outdir"
#rm "$outdir"/*

#partstart=11206656 # 0xAB0000
partstart=2818048 # 0x2b0000

#chunksize=12288
#chunksize=4096 # try to read all
chunksize=16777216 # try to read all

#totalsize=5570560 # 5440KB
#totalsize=5505024 # 5376KB so 64KB margin
#totalsizekb=5376
totalsizekb=6592
totalsize=$(expr $totalsizekb \* 1024)

totalchunks=$(expr $totalsize / $chunksize)

# not needed because it starts counting from 0 AND includes the last one (due to seq): totalchunks=$(expr $totalchunks + 1)
# seq - 1 would not be good because expr does floor() so it might be a decimal which means the last part would be lost

outfile=vfs-fat-partition-"$totalsizekb"KB-chunk

totalbytesread=0
while true; do
	start=$(expr $partstart + $totalbytesread)
	echo "start = $start"
	padname=$(printf '%10d' "$start")
	totalfile="$outdir"/"$outfile"-"$padname".bin
	result=1

	esptool.py read_flash $start $chunksize "$totalfile" &
	bgpid=$!
	sleep 40 # PROBLEM: how to detect that it's hanging?!
	if [ -d "/proc/$bgpid" ]; then
		echo "Still running, killing!"
		killall python3
		sleep 5
		killall -9 python3
		sleep 1
		result=2
	else
		wait "$bgpid"
		export result=$?
	fi
	# now it read a bit, see how much
	bytesread=0
	if [ -f "$totalfile" ]; then
		bytesread=$(du -b "$totalfile")
	fi
	echo "bytesread = $bytesread"
	totalbytesread=$(expr "$totalbytesread" + "$bytesread")
done
