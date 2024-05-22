DESCRIPTION = "A small application to test a drm display"
SECTION = "applications"
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://${COMMON_LICENSE_DIR}/MIT;md5=0835ade698e0bcf8506ecda2f7b4f302"

SRC_URI = "file://drmtest.c"

DEPENDS = "libdrm"

S = "${WORKDIR}"

do_compile() {
	${CC} ${LDFLAGS} -I${RECIPE_SYSROOT}/usr/include/drm drmtest.c -ldrm -o drmtest 
}

FILES:${PN} += "${bindir}"
FILES:${PN} += "${bindir}/drmtest"

do_install() {
	install -d ${D}${bindir}
	install -m 0755 drmtest ${D}${bindir}
}
