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
	sudo cp deploy-ti/images/beagleplay/tiboot3.bin deploy-ti/images/beagleplay/tispl.bin deploy-ti/images/beagleplay/u-boot.img deploy-ti/images/beagleplay/Image deploy-ti/images/beagleplay/k3-am625-beagleplay.dtb tmp-rootfs
	sync
	sudo umount tmp-rootfs
	
	sudo mount $(DEV)2 tmp-rootfs
	sudo cp deploy-ti/images/beagleplay/core-image-minimal-beagleplay.rootfs.tar.xz tmp-rootfs
	cd tmp-rootfs; \
		sudo tar -xf core-image-minimal-beagleplay.rootfs.tar.xz
	sync
	sudo umount tmp-rootfs
	rm -rf tmp-rootfs
