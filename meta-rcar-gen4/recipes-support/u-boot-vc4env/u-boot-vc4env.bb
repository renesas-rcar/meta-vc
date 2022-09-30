SUMMARY = "Add the default u-boot environment for Renesas VC4 boxes"

LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://${COMMON_LICENSE_DIR}/MIT;md5=0835ade698e0bcf8506ecda2f7b4f302"

#RDEPENDS_${PN} =+ "bash tcl"

FILES_${PN} = "/boot/u-boot.env"

SRC_URI = "file://u-boot.env"

S = "${WORKDIR}"

# do_compile() nothing
do_compile[noexec] = "1"

# Install the file into the target rootfs
do_install() {
    install -d ${D}/boot
    install -m 0644 u-boot.env ${D}/boot
}
