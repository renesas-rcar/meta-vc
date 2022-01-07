FILESEXTRAPATHS_prepend := "${THISDIR}/${BPN}:"

PATCHTOOL = "git"



PATCHES = " \
    file://0001-Add-VC4-board.patch \
    file://0002-vc4-Default-env-boots-from-sdcard.patch \
"

include u-boot-devel.inc


SRC_URI_append_vc4 = " \
    ${PATCHES} \
"



