DESCRIPTION = "VC FPGA programming application"
SECTION = "fpga-programmer"
LICENSE = "MIT"
inherit autotools
LIC_FILES_CHKSUM = "file://${COMMON_LICENSE_DIR}/MIT;md5=0835ade698e0bcf8506ecda2f7b4f302"

DEPENDS = "linux-renesas"

RENESAS_FPGA_PROGRAMMER_URL = "git://git@ree-dusgitlab.ree.adwin.renesas.com/rswitch2-utils/vcxfpgaprogrammer.git"
BRANCH = "master"

# Use latest version
SRCREV = "${AUTOREV}"

SRC_URI = "${RENESAS_FPGA_PROGRAMMER_URL};protocol=ssh;branch=${BRANCH}"
S = "${WORKDIR}/git/"


SRC_URI = "file://vcxfpgaprogrammer-1.0.0.tar.gz"

S = "${WORKDIR}/vcxfpgaprogrammer"

include fpga-programmer-devel.inc



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
	install -m 0755 ${S}/Release/vcxfpgaprogrammer ${D}${bindir}/fpga-programmer
}

INSANE_SKIP_${PN} = "ldflags"


