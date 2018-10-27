SERIAL_PORT = /dev/ttyUSB0
CHIP=esp8266
BOARD=generic
FLASH_FREQ = 40MHz
SKETCH=src/main.ino
include arduinoEsp8266.mk
FLASH_SIZE=1M