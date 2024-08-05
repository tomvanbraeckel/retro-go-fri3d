#!/bin/sh

echo "Uploads all files in this folder into /sd on the device."
echo "Ignores hidden files (.*)"

makedir() {
	dirname="$1"
	echo "Creating directory: $dirname"
	curl http://192.168.4.1/api --data-raw '{"cmd":"mkdir","arg1":"/sd/'"$dirname"'","arg2":""}'
}

rmprefix() {
	input="$1"
	echo "$input" | sed "s#^./##"
}

urlencode() {
	input="$1"
	echo "$input" | sed "s/ /%20/g"
}


uploadfile() {
	filename="$1"
	echo "Uploading file: $filename"
	urlfilename=$(urlencode "$filename")
	curl "http://192.168.4.1/sd/$urlfilename" --upload-file "$filename"
}

#inputdir="$1"
#if [ -z "$inputdir" ]; then
#	echo "Usage: $0 <inputdir>"
#	echo "Example: $0 ~/ESP32_NES/internalsd_zips"
#	exit 1
#fi

find ! -name ".*" | while read file; do
	filename=$(rmprefix "$file")
	dirname=$(dirname "$filename")

	if [ -d "$filename" ]; then
		#makedir "$dirname"
		makedir "$filename"
	else
		uploadfile "$filename"
	fi
done

