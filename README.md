# sh-f103-network-test
Small project to test if STM32F103C8 board (blue pill?) is capable to handle w5500 as a network board and still have features such as DHCP and MQTT.

## specs
STM32 board is from ebay (around $3).
w5500 board is from ebay (around $8).
Programing with Visual Studio + [VisualGDB](https://visualgdb.com/)

## lib
Project uses wiznet [IOlib](https://github.com/Wiznet/ioLibrary_Driver) 
To make project lighter lib is in gitignore.