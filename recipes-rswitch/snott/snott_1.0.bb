#
# This file was derived from the 'Hello World!' example recipe in the
# Yocto Project Development Manual.
#

DESCRIPTION = "SNOTT tools application"
SECTION = "snott"
#DEPENDS = "pthread"
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://${COMMON_LICENSE_DIR}/MIT;md5=0835ade698e0bcf8506ecda2f7b4f302"

SRC_URI = "file://snott-v1.0.0.tar.gz"

S = "${WORKDIR}/snott"



include snott-devel.inc

SRC_FILES := "SNOTT.c "


INC_DIR = "${TOPDIR}/../meta-renesas/meta-rcar-gen3/recipes-rswitch/snott/files"

KERNEL_PATH = "${TOPDIR}/tmp/work-shared/h3vc/kernel-source/"

INCLUDEFLAGS += "-I${INC_DIR}/. -I${KERNEL_PATH}"


LFLAGS	+= "-lc -lm -lpthread -lrt"

LDFLAGS = "${LFLAGS}"

do_compile() {
	${CC} ${SRC_FILES}  ${LDFLAGS} ${INCLUDEFLAGS} -o snott
}
do_install() {
       
	install -d ${D}${bindir}
	install -m 0755 snott ${D}${bindir}
       
}


inherit   pkgconfig gettext  upstream-version-is-even 

