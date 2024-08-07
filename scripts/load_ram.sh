xtensa-esp32-elf-objdump -h  launcher/build/launcher.elf 
python3 /home/user/sources/esp-idf/components/esptool_py/esptool/esptool.py --no-stub --chip ESP32 load_ram  launcher/build/launcher.bin
