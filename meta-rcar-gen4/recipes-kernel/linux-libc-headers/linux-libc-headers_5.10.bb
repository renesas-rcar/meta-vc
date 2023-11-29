require recipes-kernel/linux-libc-headers/linux-libc-headers.inc

require ../linux/linux-renesas-source.inc

SRC_URI = "${RENESAS_BSP_URL};branch=${BRANCH};protocol=https"
SRC_URI += "${@' '.join(sorted(d.getVar('PATCHES').split()))}"

LIC_FILES_CHKSUM = "file://COPYING;md5=6bc538ed5bd9a7fc9398086aedcd7e46"

S = "${WORKDIR}/git"
PATCHTOOL = "git"
