DESCRIPTION = "Simple helloworld application"
SECTION = "examples"
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://${COMMON_LICENSE_DIR}/MIT;md5=0835ade698e0bcf8506ecda2f7b4f302"

SRC_URI = "file://main.c \
           file://game.c \
           file://game.h \
           file://renderer.c \
           file://renderer.h \
           file://log.h"

S = "${WORKDIR}"

DEPENDS = "libdrm"

do_compile() {
	${CC} ${LDFLAGS} -I${RECIPE_SYSROOT}/usr/include/drm main.c game.c renderer.c -ldrm -o pong
}

FILES:${PN} += "${bindir}"
FILES:${PN} += "${bindir}/pong"

do_install() {
	install -d ${D}${bindir}
	install -m 0755 pong ${D}${bindir}
}
