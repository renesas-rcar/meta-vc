DESCRIPTION = "Flash utilities to flash FPGA over SPI"

LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://${COMMON_LICENSE_DIR}/MIT;md5=0835ade698e0bcf8506ecda2f7b4f302"

PACKAGE_ARCH = "${MACHINE_ARCH}"

RDEPENDS_${PN} =+ "bash"

SRC_URI = " \
    file://flash-fpga_${PV}.tar.gz \
"

S = "${WORKDIR}/flash-fpga"


include flash-fpga-devel.inc

TARGET_CC_ARCH += "${LDFLAGS}"
do_compile() {
    oe_runmake -C nvcr-chk 
    oe_runmake -C jic-converter
}

do_install() {
    install -d ${D}/${bindir}
    install -m 0755 jic-converter/reverse-bits                        ${D}/${bindir}
    install -m 0755 jic-converter/jic2bin.sh                          ${D}/${bindir}

    install -m 0755 nvcr-chk/flash-chk-nvcr                           ${D}/${bindir}
    install -m 0755 jic-converter/jic2bin.sh                          ${D}/${bindir}
    install -m 0755 flash_fpga_over_spi.sh                            ${D}/${bindir}
}

