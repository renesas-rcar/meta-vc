SUMMARY = "Add on scripts for Renesas VC boxes"

LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://${COMMON_LICENSE_DIR}/MIT;md5=0835ade698e0bcf8506ecda2f7b4f302"

#PV = "1.0.1+git${SRCPV}"
#SRCREV = "3149bfdb4f513e2f0da0a7d0bc5d0873578696f2"
SRC_URI = "file://vc-addon-script-1.1.0.tar.gz"

RDEPENDS_${PN} =+ "bash tcl"
#SRC_URI_append = "file://FPGA_bitfile/rsw1_rtlid_0x20032708_sysid_0x20070749_MT25QU01G_VC3.bin"


S = "${WORKDIR}/vc-addon-script"

include vc-addon-script-devel.inc

FILES_${PN} = "/home/root/vc/* \
"


# do_compile() nothing
do_compile[noexec] = "1"


# Just install the scripte onto the target
do_install() {
    install -d ${D}/home/root/vc/booting
    cp --preserve=mode,timestamps -R ${S}/booting/* ${D}/home/root/vc/booting   
    install -d ${D}/home/root/vc/configuration
    cp --preserve=mode,timestamps -R ${S}/configuration/* ${D}/home/root/vc/configuration   
    install -d ${D}/home/root/vc/FPGA_bitfile
    cp --preserve=mode,timestamps -R ${S}/FPGA_bitfile/* ${D}/home/root/vc/FPGA_bitfile   
    install -d ${D}/home/root/vc/misc
    cp --preserve=mode,timestamps -R ${S}/misc/* ${D}/home/root/vc/misc  
}
