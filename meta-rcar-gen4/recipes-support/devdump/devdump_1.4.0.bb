#
# This file was derived from the 'Hello World!' example recipe in the
# Yocto Project Development Manual.
#

DESCRIPTION = "devdump tools application"
SECTION = "devdump"

LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://${COMMON_LICENSE_DIR}/MIT;md5=0835ade698e0bcf8506ecda2f7b4f302"

SRC_URI = " \
    file://devdump_${PV}.tar.gz \
"

S = "${WORKDIR}/devdump"

include devdump-devel.inc 

SRC_FILES := "dump.c mem.c"


INC_DIR = "${TOPDIR}/../meta-renesas/meta-rcar-gen3/recipes-rswitch/devdump/files"

KERNEL_PATH = "${TOPDIR}/tmp/work-shared/h3vc/kernel-source/"

INCLUDEFLAGS += "-I${INC_DIR}/. -I${KERNEL_PATH}"


LFLAGS	+= "-lc"

LDFLAGS = "${LFLAGS}"

do_compile() {
	${CC} dump.c  ${LDFLAGS} ${INCLUDEFLAGS} -o devdump
	${CC} mem.c  ${LDFLAGS} ${INCLUDEFLAGS} -o devmem
}
do_install() {
       
	install -d ${D}${bindir}
	install -m 0755 devdump ${D}${bindir}
	install -m 0755 devmem ${D}${bindir}

}


inherit   pkgconfig gettext upstream-version-is-even 

