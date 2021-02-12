require include/hyperflash-linux-control.inc

FILESEXTRAPATHS_prepend := "${THISDIR}/${PN}/:"

COMPATIBLE_MACHINE = "salvator-x|h3ulcb|m3ulcb|m3nulcb|h3vc|ebisu"

RENESAS_BSP_URL = " \
 git://github.com/renesas-rcar/linux-bsp-vc.git"
BRANCH = "v4.14.75-ltsi/rcar-3.9.7-rswitch2"

SRCREV = "7be42d9a05fe4afa1d2689ed15375d887edbf610"
SRC_URI = "${RENESAS_BSP_URL};protocol=git;nocheckout=1;branch=${BRANCH}"

PATCHES = ""

HYPERFLASH_CONFIG = " \
    file://hyperflash.cfg \
"

RSWITCH1_CONFIG = " \
    file://rswitch1.cfg \
"

RSWITCH2_CONFIG = " \
    file://rswitch2.cfg \
"

CONFIG = " \
    file://defconfig \
    file://touch.cfg \
    file://h3vc.cfg \
    ${@oe.utils.conditional("ENABLE_HYPERFLASH_LINUX", "1", "${HYPERFLASH_CONFIG}", "", d)} \
    ${@bb.utils.contains('DISTRO_FEATURES','rswitch1','${RSWITCH1_CONFIG}','${RSWITCH2_CONFIG}',d)} \
"

# Uncomment if you want to compile USB Gadget drivers
#CONFIG += "file://usb_device.cfg"

include linux-renesas-devel.inc

LINUX_VERSION ?= "4.14.75"
PV = "${LINUX_VERSION}+git${SRCPV}"
PR = "r1"

SRC_URI_append = " \
    ${CONFIG} \
    ${PATCHES} \
    ${@base_conditional("USE_AVB", "1", " file://usb-video-class.cfg", "", d)} \
"

KERNEL_DEVICETREE_append_h3vc = " \
    renesas/r8a7795-h3vc3_mode1.dtb \
    renesas/r8a7795-h3vc3_mode2.dtb \
    renesas/r8a7795-h3vc2.dtb \
"

