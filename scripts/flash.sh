#while ! esptool.py write_flash --flash_mode dio --flash_freq keep --flash_size keep 0x0 retro-go_1.42-54-g9577a-dirty_fri3d-2024.img ; do echo nope; sleep 1; done
#while ! esptool.py write_flash --flash_mode dio --flash_freq keep --flash_size keep 0x0 retro-go_1.42-57-g63f33-dirty_fri3d-2024.img ; do echo nope; sleep 1; done
#while ! esptool.py write_flash --flash_mode dio --flash_freq keep --flash_size keep 0x0 retro-go_1.42-57-g63f33-dirty_fri3d-2024.img ; do echo nope; sleep 1; done
#while ! esptool.py write_flash --flash_mode dio --flash_freq keep --flash_size keep 0x0 retro-go_1.42-57-g63f33-dirty_fri3d-2024.img ; do echo nope; sleep 1; done

# after erase, this gives the SHA error:
while ! esptool.py write_flash --flash_size 16MB 0x0 /home/user/ESP32_NES/releases/1.3/fri3d_firmware_fox.bin ; do echo nope; sleep 1; done

# works:
#while ! esptool.py write_flash --flash_size detect 0x0 /home/user/ESP32_NES/releases/1.3/retro-go_1.3_zips_coverart_mainmenu_fri3d-2024.img ; do echo nope; sleep 1; done

picocom -b 115200 /dev/ttyACM0
