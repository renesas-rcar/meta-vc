DESCRIPTION = "Microchip LAN865x 10Base-T1s ethernet driver"
SECTION = "applications"
LICENSE = "CLOSED"
PR = "r0"

inherit linux-kernel-base

# We need the kernel to be staged (unpacked, patched and configured) before
# we can grab the source and make the kernel-devsrc package
do_install[depends] += "virtual/kernel:do_populate_sysroot"

B = "${STAGING_KERNEL_BUILDDIR}"

KERNEL_VERSION = "${@get_kernelversion_headers('${B}')}"


SRC_URI = " \
    file://lan865x_mod.ko \
"

S = "${WORKDIR}"


do_configure[noexec] = "1"
do_compile[noexec] = "1"

do_install() {
    MODULE_DIR=${D}${nonarch_base_libdir}/modules/${KERNEL_VERSION}/kernel/drivers/net/ethernet/microchip/lan865x
    install -d $MODULE_DIR
    install -m 755 ${S}/lan865x_mod.ko $MODULE_DIR
}


FILES_${PN} += " \
    /lib/modules/${KERNEL_VERSION}/kernel/drivers/net/ethernet/microchip/lan865x/lan865x_mod.ko \
"


KERNEL_MODULE_AUTOLOAD = "lan865x_mod"
