PATCHTOOL = "git"

require vc4_u-boot_patches.inc

SRC_URI_append_vc4 = " \
    ${PATCHES} \
"
