all: build

.PHONY: build
build:
	bitbake core-image-minimal

.PHONY: update-sdcard-beagleplay
update-sdcard-beagleplay:
	cd deploy-ti/images/beagleplay-dev/; \
	xz -d -f -k core-image-minimal-beagleplay-dev.rootfs.wic.xz
	sudo dd if=deploy-ti/images/beagleplay-dev/core-image-minimal-beagleplay-dev.rootfs.wic of=$(DEV) bs=1M

.PHONY: update-sdcard-beagley
update-sdcard-beagley:
	cd deploy-ti/images/beagleyai/; \
	xz -d -f -k core-image-minimal-beagleyai.rootfs.wic.xz
	sudo dd if=deploy-ti/images/beagleyai/core-image-minimal-beagleyai.rootfs.wic of=$(DEV) bs=1M

.PHONY: update-nfsroot
update-nfsroot:
	-tar -xf deploy-ti/images/beagleplay-dev/core-image-minimal-beagleplay-dev.rootfs.tar.xz -C $(NFSROOT)
	-sudo chmod a+w -R $(NFSROOT)
	-sudo chmod a+r -R $(NFSROOT)
	-sudo chmod a+X -R $(NFSROOT)
	cp deploy-ti/images/beagleplay-dev/Image deploy-ti/images/beagleplay-dev/k3-am625-beagleplay.dtb ./tftp 

.PHONY: update-nfsroot-beagley
update-nfsroot-beagley:
	-tar -xf deploy-ti/images/beagleyai/core-image-minimal-beagleyai.rootfs.tar.xz -C $(NFSROOT)
	-chmod a+w -R $(NFSROOT)
	-chmod a+r -R $(NFSROOT)
	-chmod a+X -R $(NFSROOT)
	cp deploy-ti/images/beagleyai/Image deploy-ti/images/beagleyai/k3-am67a-beagley-ai.dtb ./tftp
