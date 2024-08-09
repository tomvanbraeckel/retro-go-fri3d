#/home/user/.espressif/python_env/idf5.2_py3.9_env/bin/python3 /home/user/sources/esp-idf-v5.2.2/components/esptool_py/esptool/esptool.py read_flash 0x1d000 0x2000 only_mp_ota_0_again.bin
esptool.py read_flash 0x1d000 1 /dev/null

picocom -b 115200 /dev/ttyACM0
