.PHONY: build-container
build-container: Dockerfile
	docker build -t beagleplay .

.PHONY: run-container
run-container:
	docker run -it -v ./:/home/builder beagleplay:latest /bin/bash


.PHONY: build
build:
	docker run -t -v ./:/home/builder beagleplay:latest bash -c "source oe-init-build-env ./ && bitbake core-image-minimal"

.PHONY: update-sdcard
update-sdcard:
	mkdir -p tmp-rootfs
	sudo mount $(DEV)1 tmp-rootfs
	sudo cp deploy-ti/images/beagleplay-dev/tiboot3.bin deploy-ti/images/beagleplay-dev/tispl.bin deploy-ti/images/beagleplay-dev/u-boot.img deploy-ti/images/beagleplay-dev/Image deploy-ti/images/beagleplay-dev/k3-am625-beagleplay.dtb tmp-rootfs
	sync
	sudo umount tmp-rootfs
	rm -rf tmp-rootfs
	sudo dd if=deploy-ti/images/beagleplay-dev/core-image-minimal-beagleplay-dev.rootfs.ext4 of=$(DEV)2 bs=1M
	sync

.PHONY: update-nfsroot
update-nfsroot:
	-tar -xf deploy-ti/images/beagleplay-dev/core-image-minimal-beagleplay-dev.rootfs.tar.xz -C $(NFSROOT)
	-chmod a+w -R $(NFSROOT)
	-chmod a+r -R $(NFSROOT)
	-chmod a+X -R $(NFSROOT)
