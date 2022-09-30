#
# This file was derived from the 'Hello World!' example recipe in the
# Yocto Project Development Manual.
#

DESCRIPTION = "Avnu gptp application"
SECTION = "gptp-avnu"

LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://${COMMON_LICENSE_DIR}/MIT;md5=0835ade698e0bcf8506ecda2f7b4f302"

inherit pkgconfig cmake

BRANCH = "master"
SRC_URI = "git://github.com/Avnu/gptp.git;branch=${BRANCH}"

# Use latest version
SRCREV = "0baef8a36a13105112862919aac0f1eed21a44ea"

# PV needs to be updated with ${SRCPV}, otherwise no upstream changes are detected
PV = "${BRANCH}+git${SRCPV}"

S = "${WORKDIR}/git/"

do_install() {
    install -d ${D}${bindir}
    install -m 0755 gptp ${D}${bindir}
}

