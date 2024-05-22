DESCRIPTION = "Sets the uboot.env file in the rootfs"
SECTION = "bsp"
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://${COMMON_LICENSE_DIR}/MIT;md5=0835ade698e0bcf8506ecda2f7b4f302"

SRC_URI += "file://uboot.env"
S = "${WORKDIR}"

do_configure[noexec] = "1" 
do_compile[noexec] = "1"

FILES:${PN} += "/boot"
FILES:${PN} += "/boot/uboot.env"
do_install() {
    install -d ${D}/boot
    install -m 0755 ${WORKDIR}/uboot.env ${D}/boot
}
