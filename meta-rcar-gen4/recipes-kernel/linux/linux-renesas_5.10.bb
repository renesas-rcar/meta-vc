DESCRIPTION = "Linux kernel for the R-Car VC4 based board"

LIC_FILES_CHKSUM = "file://COPYING;md5=6bc538ed5bd9a7fc9398086aedcd7e46"

require recipes-kernel/linux/linux-yocto.inc

COMPATIBLE_MACHINE = "vc4"

RENESAS_BSP_URL = " git://github.com/renesas-rcar/linux-bsp-vc.git"
BRANCH = "v5.10.41/rcar-5.1.3.vc4"
SRCREV = "d3cfd1b90730dc2c96cbfc46255f53fa3c8706a2"

SRC_URI = "${RENESAS_BSP_URL};nocheckout=1;branch=${BRANCH}"

LINUX_VERSION ?= "5.10.41"
PV = "${LINUX_VERSION}+git${SRCPV}"
PR = "r1"

# For generating defconfig
KCONFIG_MODE = "--alldefconfig"
KBUILD_DEFCONFIG = "defconfig"

PATCHES = ""

CONFIG = " \
    file://defconfig \
"

# Use defconfig provided with this recipe
unset KBUILD_DEFCONFIG

include linux-renesas-devel.inc

SRC_URI_append = " \
    ${CONFIG} \
    ${PATCHES} \
"

#The base device tree including all control domain functions
KERNEL_DEVICETREE_append_vc4 = " \
    renesas/r8a779f0-vc4-ctrl-domain.dtb \
"

#Install the DT overlays for phy selection
KERNEL_DEVICETREE_append_vc4 = " \
    renesas/r8a779f0-vc4-tsn0-phy-1g.dtbo \
    renesas/r8a779f0-vc4-tsn0-phy-2g5.dtbo \
    renesas/r8a779f0-vc4-tsn1-phy-1g.dtbo \
    renesas/r8a779f0-vc4-tsn1-phy-2g5.dtbo \
    renesas/r8a779f0-vc4-tsn2-phy-1g.dtbo \
    renesas/r8a779f0-vc4-tsn2-phy-1g-rh.dtbo \
"
