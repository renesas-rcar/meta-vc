#
# This file was derived from the 'Hello World!' example recipe in the
# Yocto Project Development Manual.
#

DESCRIPTION = "gptp-master tools application"
SECTION = "gptp-master"

LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://${COMMON_LICENSE_DIR}/MIT;md5=0835ade698e0bcf8506ecda2f7b4f302"

inherit pkgconfig cmake


SRC_URI = "file://gptp-master-v1.0.0.tar.gz"

S = "${WORKDIR}/gptp-master"


include gptp-master-devel.inc

do_install() {
    install -d ${D}${bindir}
    install -m 0755 gptp ${D}${bindir}
}


