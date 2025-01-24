DESCRIPTION = "Linux kernel for the R-Car VC4 based board"

LIC_FILES_CHKSUM = "file://COPYING;md5=6bc538ed5bd9a7fc9398086aedcd7e46"

require recipes-kernel/linux/linux-yocto.inc

COMPATIBLE_MACHINE = "vc4"

PATCHES = ""
require vc4_kernel_patches.inc
#include vc4_kernel_patches_proprietary.inc

RENESAS_BSP_URL = " \
    git://github.com/renesas-rcar/linux-bsp.git"
BRANCH = "v5.10.41/rcar-5.1.7.rc11.2"
SRCREV = "0fc797171e95ae55eca74bceff6679b162dec47b"

SRC_URI = "${RENESAS_BSP_URL};nocheckout=1;branch=${BRANCH};protocol=https"
SRC_URI += "file://r8a779f0_ufs.bin"
SRC_URI += "${@' '.join(sorted(d.getVar('PATCHES').split()))}"

# Using in-tree defconfig does not work if the defconfig comes via patches,
# because kernel-yocto.bbclass uses it before applying patches.
# KBUILD_DEFCONFIG = "renesas-vc4_defconfig"
# Have to depend on a copy of that file copied to recipe.
# Thus depend on a copy of arch/arm64/configs/renesas-vc4_defconfig copied to
# the recipes's defconfig
KCONFIG_MODE = "alldefconfig"
SRC_URI += "file://defconfig"

# Allow temporary overrides for development
# include vc4_kernel_devel.inc

LINUX_VERSION ?= "5.10.41"
PV = "${LINUX_VERSION}+git${SRCPV}"
PR = "r1"

# Install device tree including control domain functions
KERNEL_DEVICETREE_append = " \
    renesas/r8a779f0-vc4-ctrl-domain.dtb \
"

# Install device tree overlays for phy selection
KERNEL_DEVICETREE_append = " \
    renesas/r8a779f0-vc4-tsn0-phy-1g.dtbo \
    renesas/r8a779f0-vc4-tsn0-phy-2g5.dtbo \
    renesas/r8a779f0-vc4-tsn1-phy-1g.dtbo \
    renesas/r8a779f0-vc4-tsn1-phy-2g5.dtbo \
    renesas/r8a779f0-vc4-tsn2-phy-1g.dtbo \
    renesas/r8a779f0-vc4-tsn2-phy-1g-rh.dtbo \
"

do_download_firmware () {
    install -d ${STAGING_KERNEL_DIR}/firmware
    install -m 755 ${WORKDIR}/r8a779f0_ufs.bin ${STAGING_KERNEL_DIR}/firmware/
}

addtask do_download_firmware after do_configure before do_compile

# Install S4 specific UAPI headers and ufs firmware
do_install_append() {
    install -d ${D}/lib/firmware/
    install -m 0644 ${S}/firmware/r8a779f0_ufs.bin ${D}/lib/firmware/
}
