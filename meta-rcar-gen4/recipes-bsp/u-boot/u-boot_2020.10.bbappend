PATCHTOOL = "git"

require vc4_u-boot_patches.inc

# Support overriding
include vc4_u-boot_devel.inc

SRC_URI_append_vc4 = " \
    ${PATCHES} \
"
