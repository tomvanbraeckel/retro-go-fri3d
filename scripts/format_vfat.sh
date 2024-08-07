#!/bin/sh

#esptool.py write_flash --flash_size detect  0x00410000  retro-go_1.42-10-g0f987-dirty_odroid-go.img
#Running /home/user/.espressif/python_env/idf4.3_py3.9_env/bin/python /home/user/sources/esp-idf/components/esptool_py/esptool/esptool.py erase_region 983040 917504...

#esptool.py erase_region 0x410000 1024000

esptool.py erase_region 0xAB0000 40960

#esptool.py erase_region 0xAB0000 5570560
