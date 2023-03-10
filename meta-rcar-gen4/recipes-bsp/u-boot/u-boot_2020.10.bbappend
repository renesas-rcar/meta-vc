FILESEXTRAPATHS_prepend := "${THISDIR}/${BPN}:"

PATCHTOOL = "git"



PATCHES = " \
    file://0001-Add-VC4-board.patch						\
    file://0002-vc4-Default-env-boots-from-sdcard.patch				\ 
    file://0003-VC4-Added-JEDEC-ID-for-mx25uw51245g.patch			\
    file://0004-VC4-Add-CONFIG_SPI_FLASH_MACRONIX-to-r8a779f0_vc4_de.patch	\
    file://0005-dts-vc4-Fix-power-and-drive-strength-of-tsn-pins.patch		\
    file://0006-dts-vc4-Fixed-rswitch-entry.patch				\
    file://0007-Remove-debug-messages.patch					\
    file://0008-DTS-VC4-add-VCCQ-node-for-controlling-MMC-IO-voltage.patch	\
    file://0009-Fix-bauderate-in-defconfig.patch				\
    file://0010-Load-default-environment-via-TFTP-or-from-MMC.patch		\
    file://0011-Reset-the-board-after-environment-import-for-auto-bo.patch	\
"

include u-boot-devel.inc


SRC_URI_append_vc4 = " \
    ${PATCHES} \
"



