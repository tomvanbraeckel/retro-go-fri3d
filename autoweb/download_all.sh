#!/bin/bash

startdir="$1"
if [ -z "$startdir" ]; then
	echo "Usage: $0 <startdir>"
	echo "Example: $0 /"
	echo "Example: $0 /romart"
	exit 1
fi


function listtocsv() {
	local ldir="$1"
	echo "Doing curl:" >&2
	echo curl 'http://192.168.4.1/api' --data-raw '{"cmd":"list","arg1":"/sd/'"$ldir"'","arg2":""}' >&2
	listing=$(curl 'http://192.168.4.1/api' --data-raw '{"cmd":"list","arg1":"/sd/'"$ldir"'","arg2":""}')
	echo "listing: $listing" >&2
	listing=$(echo "$listing" | jq -r '.files[] | [.name, .is_dir] | @csv')
	echo "listing: $listing" >&2
	listing=$(echo "$listing"  | sed 's/^"//' | sed 's/",/|/')
	echo "listing: $listing" >&2
	echo "$listing"

}

#listtocsv "/romart"

function downloadfolder() {
	local dir="$1"
	echo "Downloading folder: $dir"
	listtocsv "$dir" | while read line; do
		name=$(echo "$line" | cut -d '|' -f 1)
		isdir=$(echo "$line" | cut -d '|' -f 2)
		echo "Checking file: $name (isdir = $isdir)"
		echo "While dir = $dir"
		if [ "$isdir" = "true" ]; then
			mkdir "$name"
			cd "$name"
			downloadfolder "$dir/$name"
			cd ..
		else
			#downloadfile "$dir"/$name"
			wget "http://192.168.4.1/sd/$dir/$name"
		fi
	done
	echo "finished downloading $dir"
}

downloadfolder "$startdir"

exit

listtocsv "."
listtocsv "/"

#curl 'http://192.168.4.1/api' --data-raw '{"cmd":"list","arg1":"/sd/retro-go","arg2":""}' \

#curl 'http://192.168.4.1/api'  --data-raw '{"cmd":"list","arg1":"/sd/retro-go/saves/gbc/homebrew","arg2":""}' \
