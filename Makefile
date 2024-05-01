all: fetch build

.PHONY: fetch
fetch: u-boot ti-linux-firmware k3-image-gen arm-trusted-firmware optee_os linux busybox

.PHONY: build
build: build-u-boot-r5 build-ti-linux-firmware build-arm-trusted-firmware build-optee-os build-u-boot-a53 build-busybox

u-boot:
	git clone https://source.denx.de/u-boot/u-boot.git


ti-linux-firmware:
	git clone https://git.ti.com/cgit/processor-firmware/ti-linux-firmware
	cd ti-linux-firmware; git checkout c125d3864b9faf725ff40e620049ab5d56dedc5b


k3-image-gen:
	git clone https://git.ti.com/cgit/k3-image-gen/k3-image-gen
	cd k3-image-gen; git checkout 150f1956b4bdcba36e7dffc78a4342df602f8d6e

arm-trusted-firmware:
	git clone https://github.com/ARM-software/arm-trusted-firmware.git
	cd arm-trusted-firmware; git checkout v2.9

optee_os:
	git clone https://github.com/OP-TEE/optee_os.git

linux:
	git clone git://git.kernel.org/pub/scm/linux/kernel/git/stable/linux.git
	cd linux; git checkout linux-6.7.y

busybox:
	git clone https://git.busybox.net/busybox
	cd busybox; git checkout 1_36_stable

drm:
	git clone https://gitlab.freedesktop.org/mesa/drm.git


.PHONY: build-u-boot-r5
build-u-boot-r5:
	cd u-boot; \
    cp ../configs/am62x_evm_r5_custom_defconfig configs/;  \
    ARCH=arm CROSS_COMPILE=~/x-tools/arm-none-eabi/bin/arm-none-eabi- make am62x_evm_r5_custom_defconfig O=beagleplay_r5_build; \
    make -j8 DEVICE_TREE=k3-am625-r5-beagleplay O=beagleplay_r5_build CROSS_COMPILE=~/x-tools/arm-none-eabi/bin/arm-none-eabi- ARCH=arm

.PHONY: build-ti-linux-firmware
build-ti-linux-firmware:
	cd k3-image-gen; make -j8 CROSS_COMPILE=~/x-tools/arm-none-eabi/bin/arm-none-eabi- SOC=am62x SBL=../u-boot/beagleplay_r5_build/spl/u-boot-spl.bin SYSFW_PATH=../ti-linux-firmware/ti-sysfw/ti-fs-firmware-am62x-gp.bin

.PHONY: build-arm-trusted-firmware
build-arm-trusted-firmware:
	cd arm-trusted-firmware; make -j8 PLAT=k3 TARGET_BOARD=lite SPD=opteed CROSS_COMPILE=~/x-tools/aarch64-unknown-linux-gnu/bin/aarch64-unknown-linux-gnu-

.PHONY: build-optee-os
build-optee-os:
	cd optee_os; make -j8 CROSS_COMPILE=~/x-tools/arm-none-eabi/bin/arm-none-eabi- CROSS_COMPILE64=~/x-tools/aarch64-unknown-linux-gnu/bin/aarch64-unknown-linux-gnu- CFG_ARM64_core=y CFG_WITH_SOFTWARE_PRNG=y PLATFORM=k3-am62x

.PHONY: build-u-boot-a53
build-u-boot-a53:
	cd u-boot; \
    cp ../configs/am62x_evm_a53_custom_defconfig configs/; \
    ARCH=arm CROSS_COMPILE=~/x-tools/aarch64-unknown-linux-gnu/bin/aarch64-unknown-linux-gnu- make am62x_evm_a53_custom_defconfig O=beagleplay_a53_build; \
    make -j8 BL31=../arm-trusted-firmware/build/k3/lite/release/bl31.bin BINMAN_INDIRS=/home/alex/Repos/beagleplay-dev/ti-linux-firmware O=beagleplay_a53_build TEE=../optee_os/out/arm-plat-k3/core/tee-raw.bin CROSS_COMPILE=~/x-tools/aarch64-unknown-linux-gnu/bin/aarch64-unknown-linux-gnu- ARCH=arm DEVICE_TREE=k3-am625-beagleplay

.PHONY: build-linux
build-linux:
	cd linux; \
	cp ../configs/custom_defconfig arch/arm64/configs; \
	cp ../configs/custom-beagleplay.dts arch/arm64/boot/dts/ti; \
	make CC=clang ARCH=arm64 custom_defconfig; \
	make CC=clang -j8 ARCH=arm64 CROSS_COMPILE=/home/alex/x-tools/aarch64-unknown-linux-gnu/bin/aarch64-unknown-linux-gnu-; \
	cp arch/arm64/boot/Image ../tftp

.PHONY: build-app-hello
build-app-hello: apps/hello/hello.c
	~/x-tools/aarch64-unknown-linux-gnu/bin/aarch64-unknown-linux-gnu-gcc apps/hello/hello.c -o apps/hello/hello
	cp apps/hello/hello /mnt/psf/nfsroot/usr/bin

.PHONY: build-app-drmtest
build-app-drmtest: apps/drmtest/drmtest.c
	~/x-tools/aarch64-unknown-linux-gnu/bin/aarch64-unknown-linux-gnu-gcc -L/mnt/psf/nfsroot/lib -I/mnt/psf/nfsroot/include/libdrm apps/drmtest/drmtest.c -ldrm -o apps/drmtest/drmtest
	cp apps/drmtest/drmtest /mnt/psf/nfsroot/usr/bin

.PHONY: build-busybox
build-busybox:
	cd busybox; \
	cp ../configs/busybox_beagleplay_defconfig configs/beagleplay_defconfig; \
	make beagleplay_defconfig; \
	make -j8 install

.PHONY: build-drm
build-drm:
	cd drm; \
	cp ../configs/drm-cross-config.txt .; \
	meson setup builddir --cross-file drm-cross-config.txt --prefix=/mnt/psf/nfsroot/; \
	sudo ninja -C builddir/ install
	sudo mv /mnt/psf/nfsroot/include/xf86drm.h /mnt/psf/nfsroot/include/xf86drmMode.h /mnt/psf/nfsroot/include/libdrm 


.PHONY: build-modules
build-modules:
	cd kernel-modules/hello; \
	make all 
	sudo cp kernel-modules/hello/hello.ko /mnt/psf/nfsroot/home/alex
	cd kernel-modules/sharp-display; \
	make all
	sudo cp kernel-modules/sharp-display/sharp-display.ko /mnt/psf/nfsroot/home/alex
	cd kernel-modules/bmi270; \
	make all
	sudo cp kernel-modules/bmi270/bmi270.ko /mnt/psf/nfsroot/home/alex

 

.PHONY: update-sdcard
update-sdcard:
	sudo mount $(DEV)1 /mnt/local
	sudo cp k3-image-gen/tiboot3.bin u-boot/beagleplay_a53_build/tispl.bin u-boot/beagleplay_a53_build/u-boot.img linux/arch/arm64/boot/Image linux/arch/arm64/boot/dts/ti/k3-am625-beagleplay.dtb /mnt/local
	sync
	sudo umount /mnt/local

.PHONY: update-tftp
update-tftp:
	sudo cp k3-image-gen/tiboot3.bin u-boot/beagleplay_a53_build/tispl.bin u-boot/beagleplay_a53_build/u-boot.img linux/arch/arm64/boot/Image linux/arch/arm64/boot/dts/ti/k3-am625-beagleplay.dtb /mnt/psf/tftpboot/	
