SUMMARY = "An example kernel recipe that uses the linux-yocto and oe-core"
SECTION = "kernel"
LICENSE = "GPL-2.0-only"

inherit kernel

SRC_URI = "git://git.kernel.org/pub/scm/linux/kernel/git/stable/linux.git;protocol=git;branch=linux-6.7.y \
           file://defconfig \
           file://k3-am625-beagleplay-dev.dtsi \
           file://0001-regulatory-firmware.patch \
           file://0002-add-custom-dtsi.patch" 

SRCREV:aarch64="004dcea13dc10acaf1486d9939be4c793834c13c"
LIC_FILES_CHKSUM = "file://COPYING;md5=6bc538ed5bd9a7fc9398086aedcd7e46"
PV = "${LINUX_VERSION}+git"

S = "${WORKDIR}/git"

LINUX_VERSION ?= "6.7"
COMPATIBLE_MACHINE = "beagleplay-dev"

do_configure:append() {
    cp ${WORKDIR}/k3-am625-beagleplay-dev.dtsi ${S}/arch/arm64/boot/dts/ti/
}
