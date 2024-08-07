#!/bin/sh

echo "Adding launcher, retro-core, prboom-go..."

#/home/user/.espressif/python_env/idf5.2_py3.9_env/bin/python3 /home/user/sources/esp-idf/components/esptool_py/esptool/esptool.py write_flash --flash_size detect 0x8000 /home/user/sources/badge_2024_micropython/ports/esp32/boards/FRI3D_BADGE_COMMON/partitions_with_retro-go.bin 0x830000 launcher/build/launcher.bin 0x930000 retro-core/build/retro-core.bin 0xA30000 prboom-go/build/prboom-go.bin

/home/user/.espressif/python_env/idf5.2_py3.9_env/bin/python3 /home/user/sources/esp-idf/components/esptool_py/esptool/esptool.py write_flash --flash_size detect 0x8000 /home/user/sources/badge_2024_micropython/ports/esp32/boards/FRI3D_BADGE_COMMON/partitions_with_retro-go.bin 0x830000 launcher/build/launcher.bin 0x990000 retro-core/build/retro-core.bin 0xaf0000 prboom-go/build/prboom-go.bin

#echo "Adding ota_1..." ; python3 /home/user/sources/esp-idf/components/esptool_py/esptool/esptool.py write_flash --flash_size detect 0x430000 /home/user/sources/badge_2024_micropython/ports/esp32/build-FRI3D_BADGE_2022/micropython.bin

#echo "Adding launcher..."
#python3 /home/user/sources/esp-idf/components/esptool_py/esptool/esptool.py write_flash --flash_size detect 0x830000 launcher/build/launcher.bin
#echo "Adding retro-core..."
#python3 /home/user/sources/esp-idf/components/esptool_py/esptool/esptool.py write_flash --flash_size detect 0x930000 retro-core/build/retro-core.bin
#echo "Adding prboom-go..."
#python3 /home/user/sources/esp-idf/components/esptool_py/esptool/esptool.py write_flash --flash_size detect 0xA30000 prboom-go/build/prboom-go.bin
