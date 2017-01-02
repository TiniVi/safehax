CC=arm-none-eabi-gcc
CP=arm-none-eabi-g++
OC=arm-none-eabi-objcopy
OD=arm-none-eabi-objdump
LD=arm-none-eabi-ld

all:
	$(CC) -g *.c -c -march=armv6z -O2 -ffast-math -std=c99 -Wall
	$(CC) -g *.s -c -march=armv6z
	$(CC) -T 3ds.ld *.o 
	$(OC) --set-section-flags .bss=alloc,load,contents -O binary a.out arm11.bin
	rm *.out *.o
