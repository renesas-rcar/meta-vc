require include/hyperflash-linux-control.inc

COMPATIBLE_MACHINE = "(salvator-x|ulcb|ebisu|h3vc)"

PATCHTOOL = "git"

FILESEXTRAPATHS_prepend := "${THISDIR}/${BPN}:"

ATFW_OPT_append_h3vc = " RCAR_SYSTEM_SUSPEND=0"

# Set RPC-IF to Non-Secure so that it is possible to access the hyperflash from Linux
NON_SECURE_HYPERFLASH = " \
    file://0001-plat-rcar-BL2-Set-security-settings-of-RPC-IF-to-non.patch \
"

SRC_URI_append_h3vc = " \
    ${@oe.utils.conditional("ENABLE_HYPERFLASH_LINUX", "1", "${NON_SECURE_HYPERFLASH}", "", d)} \
"

