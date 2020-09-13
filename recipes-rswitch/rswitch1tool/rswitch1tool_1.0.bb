#
# This file was derived from the 'Hello World!' example recipe in the
# Yocto Project Development Manual.
#

DESCRIPTION = "rswitch1 tools application"
SECTION = "rswitch1tool"
LICENSE = "MIT"
inherit autotools
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://${COMMON_LICENSE_DIR}/MIT;md5=0835ade698e0bcf8506ecda2f7b4f302"
RENESAS_RSWITCHTOOL_URL = " \
git://git@ree-dusgitlab.ree.adwin.renesas.com/rswitch-utils/rswitchtool.git"
BRANCH = "master"

# Use latest version
SRCREV = "${AUTOREV}"
#SRCREV = "ce387eb870d73fef9d38750ab4dd39e1dcba72e8"

SRC_URI = "${RENESAS_RSWITCHTOOL_URL};protocol=ssh;branch=${BRANCH}"
S = "${WORKDIR}/git/"
CFLAGS[unexport] = "1"
LDFLAGS[unexport] = "1"
AS[unexport] = "1"
LD[unexport] = "1"
EXTRA_OEMAKE = "'CC=${CC}' 'CXX=${CXX}'"
do_compile() {
        cd ${S}
	oe_runmake 
}
do_install() {
        
	install -d ${D}${bindir}
	install -m 0755 ${S}/Release/rswitchtool ${D}${bindir}
}

INSANE_SKIP_${PN} = "ldflags"


