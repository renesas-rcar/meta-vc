# This file is included by both linux and linux-libc-headers recipes,
# to ensure those use the same kernel source tree

PATCHES = ""
require vc4_kernel_patches.inc
include vc4_kernel_patches_proprietary.inc

RENESAS_BSP_URL = " \
    git://github.com/renesas-rcar/linux-bsp.git"
BRANCH = "v5.10.41/rcar-5.1.7.rc11.1"
SRCREV = "1cff967a65247a6ff21b879331cf330ec35572fc"

LINUX_VERSION ?= "5.10.41"
PV = "${LINUX_VERSION}+git${SRCPV}"
