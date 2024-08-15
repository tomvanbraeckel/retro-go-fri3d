
# These work but a bit corrupted:
#esptool.py write_flash --flash_size 16MB 0xAB0000 /home/user/ESP32_NES/FATs/cropped.bin
#esptool.py write_flash --flash_size 16MB 0xAB0000 /home/user/ESP32_NES/FATs/cropped_nolast.bin

# these dont work:
#esptool.py write_flash --flash_size 16MB 0xAB0000 /home/user/ESP32_NES/FATs/internalsd_tiny.bin
# This looks very similar, should work but doesnt:
#esptool.py write_flash --flash_size 16MB 0xAB0000 /home/user/ESP32_NES/FATs/internalsd_tiny_nowl.bin
# Trying again, created with:
# ~/sources/esp-idf-v5.2.2/components/fatfs/fatfsgen.py --output_file internalsd_tiny_nowl.bin  --partition_size detect  --sector_size 4096    --long_name_support  ~/ESP32_NES/internalsd_tiny

# works, sortof:
#esptool.py write_flash --flash_size 16MB 0xAB0000 /home/user/ESP32_NES/FATs/internalsd_tiny_nowl.bin

#esptool.py write_flash --flash_size 16MB 0xAB0000 /home/user/ESP32_NES/FATs/testfs_nowl.bin

#esptool.py write_flash --flash_size 16MB 0xAB0000 /home/user/ESP32_NES/FATs/vfs-fat-partition-5376KB.bin

# works:
#esptool.py write_flash --flash_size 16MB 0xAB0000 /home/user/ESP32_NES/releases/0.2-beta-untested-nodoom/add_homebrew_free_games-WARNING-OVERWRITES-FAT-PARTITION/vfs-fat-partition-beta-nodoom-5376KB.bin

# works:
#esptool.py write_flash --flash_size 16MB 0xAB0000 /home/user/ESP32_NES/FATs/vfs-fat-partition-5376KB-complete.bin

# dontwork:
#esptool.py write_flash --flash_size 16MB 0xAB0000 /home/user/ESP32_NES/FATs/manual.img

# works!
#esptool.py write_flash --flash_size 16MB 0xAB0000 /home/user/ESP32_NES/FATs/fatfsgen_internalsd_tiny.bin

# no longer works?!
#esptool.py write_flash --flash_size 16MB 0x2B0000 /home/user/ESP32_NES/FATs/fatfsgen_internalsd_tiny.bin
#esptool.py write_flash --flash_size 16MB 0x2B0000 /home/user/ESP32_NES/FATs/fatfsgen_internalsd_tiny_again.bin

# this works:
#esptool.py write_flash --flash_size 16MB 0x2B0000 /home/user/ESP32_NES/FATs/vfs-fat-partition-5376KB-complete.bin

#esptool.py write_flash --flash_size 16MB 0x990000 /home/user/ESP32_NES/FATs/vfs-fat-partition-6750208.bin

esptool.py write_flash --flash_size 16MB 0x990000 /home/user/ESP32_NES/releases/1.8/vfs-fat-partition-6750208.bin

picocom -b 115200 /dev/ttyACM0
