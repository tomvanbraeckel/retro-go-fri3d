killall picocom
killall -9 picocom
sleep 1
/home/user/sources/esp-idf/components/esptool_py/esptool/esptool.py write_flash --flash_size detect  0x830000 launcher/build/launcher.bin
