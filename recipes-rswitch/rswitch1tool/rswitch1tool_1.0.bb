#
# This file was derived from the 'Hello World!' example recipe in the
# Yocto Project Development Manual.
#

DESCRIPTION = "rswitch1 tools application"
SECTION = "rswitch1tool"
inherit autotools
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://${COMMON_LICENSE_DIR}/MIT;md5=0835ade698e0bcf8506ecda2f7b4f302"

DEPENDS = "linux-renesas"

SRC_URI = "file://rswitch1tool-v1.0.0.tar.gz"

S = "${WORKDIR}/rswitch1tool"

# Only used for REE local development
include rswitch1tool-devel.inc

CFLAGS[unexport] = "1"
LDFLAGS[unexport] = "1"
AS[unexport] = "1"
LD[unexport] = "1"
EXTRA_OEMAKE = "'CC=${CC}' 'CXX=${CXX}'"
do_compile() {
        cd ${S}
        export KERNEL_PATH="${TOPDIR}/tmp/work-shared/h3vc/kernel-source/"
        oe_runmake
}
do_install() {

        install -d ${D}${bindir}
        install -m 0755 ${S}/Release/rswitchtool ${D}${bindir}
}

INSANE_SKIP_${PN} = "ldflags"


