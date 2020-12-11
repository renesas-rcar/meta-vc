FILESEXTRAPATHS_prepend := "${THISDIR}/${PN}/:"

PATCHES = ""

CONFIG = " \
    file://defconfig \
"


SRC_URI_append = " \
    ${CONFIG} \
    ${PATCHES} \ 
"

