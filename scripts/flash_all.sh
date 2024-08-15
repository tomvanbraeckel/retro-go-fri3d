
#dir="/home/user/ESP32_NES/releases/0.1-beta-slow-nodoom/add_retro-go_to_fri3d_badge_2024"
#dir="/home/user/ESP32_NES/releases/0.2-beta-untested-nodoom/add_retro-go_to_fri3d_badge_2024"
#esptool.py write_flash --flash_size 16MB 0x830000 launcher/build/launcher.bin 0x930000 retro-core/build/retro-core.bin 0x9D0000 prboom-go/build/prboom-go.bin

#dir="/home/user/ESP32_NES/releases/1.3"
#retrodir="$dir"/add_retro-go_to_fri3d_badge_2024/

echo "Usage: $0 [retro-dir with bin files]"
sleep 3

retrodir="$1"
if [ -z "$retrodir" ]; then
	retrodir=.
fi

fatdir=/home/user/ESP32_NES/releases/1.4/only_homebrew_free_games_for_webflasher_OVERWRITES_FAT_STORAGE_PARTITION/

foxdir=/home/user/sources/fri3d_firmware/boards/fox/build/

#esptool.py write_flash --flash_size 16MB 0x8000 "$retrodir"/partitions_with_retro-go.bin 0x830000 "$retrodir"/launcher.bin 0x930000 "$retrodir"/retro-core.bin 0x9D0000 "$retrodir"//prboom-go.bin 0xAB0000 "$dir"/add_homebrew_free_games-WARNING-OVERWRITES-FAT-PARTITION/vfs-fat-partition-5376KB-complete.bin

#esptool.py write_flash --flash_size 16MB 0x710000 "$retrodir"/launcher.bin 0x810000 "$retrodir"/retro-core.bin 0x8b0000 "$retrodir"//prboom-go.bin 0x990000 "$fatdir"/*.bin



#esptool.py write_flash --flash_size 16MB 0x0 "$foxdir"/bootloader/bootloader.bin 0x8000 "$foxdir"/partition_table/partition-table.bin 0x9000 "$foxdir"/ota_data_initial.bin 0x10000 "$foxdir"/fri3d_firmware_fox.bin 0x710000 "$retrodir"/launcher/build/launcher.bin 0x810000 "$retrodir"/retro-core/build/retro-core.bin 0x8b0000 "$retrodir"/prboom-go/build/prboom-go.bin 0x990000 "$fatdir"/*.bin

#esptool.py write_flash --flash_size 16MB 0x710000 "$retrodir"/launcher/build/launcher.bin 0x810000 "$retrodir"/retro-core/build/retro-core.bin 0x8b0000 "$retrodir"/prboom-go/build/prboom-go.bin 
#esptool.py write_flash --flash_size 16MB 0x710000 "$retrodir"/launcher/build/launcher.bin 

esptool.py write_flash --flash_size 16MB 0x710000 "$retrodir"/launcher/build/launcher.bin 0x810000 "$retrodir"/retro-core/build/retro-core.bin 0x8b0000 "$retrodir"/prboom-go/build/prboom-go.bin 0x990000 /home/user/ESP32_NES/releases/1.8/vfs_fox.bin

picocom -b 115200 /dev/ttyACM0

