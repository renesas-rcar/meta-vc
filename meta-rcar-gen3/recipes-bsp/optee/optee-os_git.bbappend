require include/hyperflash-linux-control.inc

COMPATIBLE_MACHINE = "(salvator-x|h3ulcb|m3ulcb|m3nulcb|ebisu|h3vc)"

# Disable the Secure Storage on Hyperflash if we want to access the
# hyperflash from linux
DISABLE_HYPERFLASH_SECURE_STORAGE = "CFG_REE_FS=y CFG_STANDALONE_FS=n"

OPTEE_OPT = " \
    ${@oe.utils.conditional("ENABLE_HYPERFLASH_LINUX", "1", "${DISABLE_HYPERFLASH_SECURE_STORAGE}", "", d)} \
"

do_compile() {
    oe_runmake PLATFORM=${PLATFORM} CFG_ARM64_core=y \
               ${OPTEE_OPT}
}

