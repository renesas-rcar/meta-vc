FILESEXTRAPATHS_prepend := "${THISDIR}/${BPN}:"

PATCHTOOL = "git"

SRC_URI_append_vc4 = " \
    file://0001-Add-VC4-board.patch \
"

