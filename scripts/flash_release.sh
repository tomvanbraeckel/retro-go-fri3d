

#esptool.py write_flash --flash_size 16MB 0x0 "$foxdir"/bootloader/bootloader.bin 0x8000 "$foxdir"/partition_table/partition-table.bin 0x9000 "$foxdir"/ota_data_initial.bin 0x10000 "$foxdir"/fri3d_firmware_fox.bin 0x710000 "$retrodir"/launcher/build/launcher.bin 0x810000 "$retrodir"/retro-core/build/retro-core.bin 0x8b0000 "$retrodir"/prboom-go/build/prboom-go.bin 0x990000 "$fatdir"/*.bin

releasedir="$1"
sleep 1

esptool.py write_flash 0x710000 "$releasedir"/retro_go_launcher_fox.bin 0x810000 "$releasedir"/retro_go_core_fox.bin 0x8b0000 "$releasedir"/retro_go_prboom_fox.bin 0x990000 "$releasedir"/vfs_fox.bin

picocom -b 115200 /dev/ttyACM0

