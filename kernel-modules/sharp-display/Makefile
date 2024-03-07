obj-m += sharp-display.o

all:
	make CC=clang ARCH=arm64 CROSS_COMPILE=/home/alex/x-tools/aarch64-unknown-linux-gnu/bin/aarch64-unknown-linux-gnu- -C /home/alex/Repos/beagleplay-dev/linux/ M=$(PWD) modules

clean:
	make -C /home/alex/Repos/beagleplay-dev/linux/ M=$(PWD) clean


