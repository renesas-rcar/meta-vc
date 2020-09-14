require include/hyperflash-linux-control.inc

FILESEXTRAPATHS_prepend := "${THISDIR}/${PN}/:"

COMPATIBLE_MACHINE = "salvator-x|h3ulcb|m3ulcb|m3nulcb|h3vc|ebisu"

RENESAS_BSP_URL = " \
 git://github.com/renesas-rcar/linux-bsp-vc.git"
BRANCH = "v4.14.75-ltsi/rcar-3.9.7-rswitch2"


SRCREV = "13b4b9693539795dbebb779c8ab147f23179945e"
SRC_URI = "${RENESAS_BSP_URL};protocol=git;nocheckout=1;branch=${BRANCH}"

LINUX_VERSION ?= "4.14.75"
PV = "${LINUX_VERSION}+git${SRCPV}"
PR = "r1"


SRC_URI_append = " \
    file://defconfig \
    file://touch.cfg \
    file://h3vc.cfg \
    file://rswitch2.cfg \
    file://0446-H3VC-Added-MT25QU01-flash-IC-via-SPI.patch \
    file://0447-H3VC-BD9571MWV-Disable-IRQ-for-now.patch \
    file://0001-Add-cetibox-poweroff-driver.patch \ 
    ${@base_conditional("USE_AVB", "1", " file://usb-video-class.cfg", "", d)} \
"

# Enable access to Hyperflash from Linux
ENABLE_HYPERFLASH = " \
    file://0012-mtd-Add-RPC-HyperFlash-driver.patch \
    file://0236-clk-renesas-r8a77970-Add-SD0H-SD0-clocks-for-SDHI.patch \
    file://0273-clk-renesas-rcar-gen3-Factor-out-cpg_reg_modify.patch \
    file://0274-clk-renesas-rcar-gen3-Add-spinlock.patch \
    file://0275-clk-renesas-rcar-gen3-Add-RPC-clocks.patch \
    file://0365-mtd-spi-nor-Add-R-Car-Gen3-RPC-QSPI-driver.patch \
    file://0366-mtd-spi-nor-renesas-rpc-Workaround-256-byte-data-siz.patch \
    file://0367-mtd-spi-nor-Add-s25fs512s-and-s25fs128s-01-SPI-NOR-f.patch \
    file://0375-mtd-spi-nor-renesas-rpc-Support-single-mode-write-co.patch \
    file://0376-mtd-spi-nor-renesas-rpc-Add-DMA-read-support.patch \
    file://0435-clk-renesas-r8a7795-cpg-mssr-Add-RPC-clocks.patch \
    file://0438-clk-renesas-rcar-gen3-cpg-Allow-to-set-RPCD2-clock-p.patch \
    file://0439-mtd-Consolidate-Renesas-RPC-drivers.patch \
    file://0442-arm64-dts-renesas-r8a7795-Add-RPC-device-node.patch \
    file://0443-arm64-dts-renesas-r8a7795-h3vc2-Add-Hyperflash-devic.patch \
    file://0444-mtd-spi-nor-renesas-rpc-Do-not-use-DMA-by-default.patch \
    file://hyperflash.cfg \
"


SRC_URI_append_h3vc = " \
    ${@oe.utils.conditional("ENABLE_HYPERFLASH_LINUX", "1", "${ENABLE_HYPERFLASH}", "", d)} \
"

KERNEL_DEVICETREE_append_h3vc = " \
    renesas/r8a7795-h3vc2.dtb \
"

