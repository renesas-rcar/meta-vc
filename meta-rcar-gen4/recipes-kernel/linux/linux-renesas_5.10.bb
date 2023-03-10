DESCRIPTION = "Linux kernel for the R-Car VC4 based board"

LIC_FILES_CHKSUM = "file://COPYING;md5=6bc538ed5bd9a7fc9398086aedcd7e46"

require recipes-kernel/linux/linux-yocto.inc

COMPATIBLE_MACHINE = "vc4"

RENESAS_BSP_URL = " \
    git://github.com/renesas-rcar/linux-bsp.git"
BRANCH = "v5.10.41/rcar-5.1.7.rc5"
SRCREV = "5b26eb114c74c907671d996128bb0233f516703c"

SRC_URI = "${RENESAS_BSP_URL};nocheckout=1;branch=${BRANCH}"

LINUX_VERSION ?= "5.10.41"
PV = "${LINUX_VERSION}+git${SRCPV}"
PR = "r1"

# For generating defconfig
KCONFIG_MODE = "--alldefconfig"
KBUILD_DEFCONFIG = "defconfig"


PATCHES = " \
    file://0001-arm64-clk-renesas-Added-R-Car-S4-SoC-r8a779f0.patch         \
    file://0002-arm64-pinctrl-renesas-Added-R-Car-S4-SoC-r8a779f0.patch     \
    file://0003-VC4-Added-initial-device-tree-base-on-spider-board.patch    \
    file://0004-dts-arm64-renesas-Fix-phy-handles-for-vc4-rswitch-po.patch  \
    file://0005-rswitch-Increase-divider-for-MDC-clock.patch                \
    file://0006-Correct-the-phy-address-on-MDIO.patch                       \
    file://0007-Add-the-Marvell-88Q2112-API-for-phy-initialisation.-.patch  \
    file://0008-Added-VC4-DTB-to-Makefile.patch                             \
    file://0009-Added-template-for-Marvell-88Q221x-PHY.patch                \
    file://0010-VC4-Specified-Marvell-88Q221x-PHY.patch                     \
    file://0011-dts-renesas-vc4-Add-RTC-clock-to-devicetree.patch           \
    file://0012-dts-renesas-vc4-Add-i2c-eeprom-to-devicetree.patch          \
    file://0013-soc-renesas-add-missed-select-SYSC_RCAR.patch               \
    file://0014-dts-renesas-vc4-Add-RTC-clock-to-devicetree.patch           \
    file://0015-dts-renesas-vc4-remove-i2c0-definitions.patch               \
    file://0016-arm64-restart-handle-arm_pm_restart-in-restart-notif.patch  \
    file://0017-i2c-rcar-implement-atomic-transfers.patch                   \
    file://0018-mfd-add-renesas-vc4-fpga4000-driver.patch                   \
    file://0019-adm64-configs-add-renesas-vc4_defconfig.patch               \
    file://0020-dts-renesas-vc4-clenup.patch                                \
    file://0021-dts-Add-RSwitch2-for-VC4-board.patch                        \
    file://0022-net-phy-Add-Marvell-88q211x-phy.patch                       \
    file://0023-net-rswitch2-Add-driver.patch                               \
    file://0024-dts-VC4-Add-dedicated-DT-for-peripherals-in-control-.patch  \
    file://0025-DT-Renesas-VC4-Add-dedicated-DTB-for-REL-rswitch-dri.patch  \
    file://0026-Add-DTS-for-the-control-domain-version.patch                \
    file://0027-dts-VC4-Fixed-wrong-wthernet-handle.patch                   \
    file://0028-net-Quick-and-dirty-fix-to-prevent-napi_enable-is-ca.patch  \
    file://0029-arm64-dts-renesas-r8a779f0-remove-references-to-gpio.patch  \
    file://0030-arm64-dts-renesas-r8a779f0-cleanup-switch-definition.patch  \
    file://0031-net-rswitch2-remove-space-from-compatible-string.patch      \
    file://0032-arm64-dts-renesas-r8a779f0-vc4-ctrl-domain.dts-dedup.patch  \
    file://0033-arm64-dts-renesas-r8a779f0-add-rpc-node.patch               \
    file://0034-arm64-dts-renesas-r8a779f0-vc4-add-SPI-flash-chip.patch     \
    file://0035-mtd-spi-nor-macronix-Add-support-for-mx25uw51245g.patch     \
    file://0036-scripts-dtc-Update-to-upstream-version-v1.6.0-51-g18.patch  \
    file://0037-arm64-dts-renesas-r8a779f0-vc4-remove-tsn0-pinmux-no.patch  \
    file://0038-arm64-dts-renesas-r8a779f0-vc4-dt-overlay-based-phy-.patch  \
    file://0039-net-rswitch2-Do-PHY-and-SerDes-init-on-netdev-open.patch    \
    file://0040-net-phy-m88q211x-sanitize-return-values.patch               \
    file://0041-net-rswitch2-Add-SerDes-init-for-2.5G.patch                 \
    file://0042-irqchip-renesas-INTSWCD-driver.patch                        \
    file://0043-clk-renesas-add-stubs-for-control-domain-support.patch      \
    file://0044-pinctrl-renesas-r8a779f0-add-RICC0-pin-settings.patch       \
    file://0045-irqchip-renesas-intswcd-move-S4-defs-into-a-separate.patch  \
    file://0046-irqchip-renesas-intswcd-add-full-interrupt-table-for.patch  \
    file://0047-dts-cleanup-vc4-dts-files.patch                             \
    file://0048-OF-DT-Overlay-configfs-interface-v7.patch                   \
    file://0049-net-rswitch2-Fix-module-unload.patch                        \
    file://0050-Resolve-Driver-should-initalize-PHY-without-link.patch      \
    file://0051-net-rswitch2-Add-statistic-counter.patch                    \
    file://0052-Correct-DT-for-2G5-and-RH.-Improved-SERDES-init-for-.patch  \
    file://0053-SERDES-PLLs-controlled-by-device-tree.patch                 \
    file://0054-Unify-phy-interface-configuration-enum.patch                \
    file://0055-Add-deferred-SerDes-operational-state-check.patch           \
    file://0056-pinctrl-renesas-r8a779f0-add-ETNB0-pin-settings.patch       \
    file://0057-clk-renesas-ensure-CPG_CD_MOD-clocks-are-PM-clocks.patch    \
    file://0058-clk-renesas-r8a779f0-add-mssr-clock-for-etnb0.patch         \
    file://0059-dts-VC4-add-ethb0-ether-avb-device.patch                    \
    file://0060-irqchip-renesas-intswcd-fix-typo-in-CAN14-irq-defs.patch    \
    file://0061-clk-renesas-r8a779f0-add-stub-for-control-domain-CLK.patch  \
    file://0062-clk-renesas-r8a779f0-add-more-stubs-for-control-doma.patch  \
    file://0063-pinctrl-renesas-r8a779f0-add-CANFD-pin-settings.patch       \
    file://0064-clk-renesas-r8a779f0-add-mssr-clocks-for-CANFD.patch        \
    file://0065-renesas-vc4-fpga4000-add-CANFD-signals-control.patch        \
    file://0066-dts-VC4-add-CANFD-devices.patch                             \
    file://0067-Change-to-20-MHz-main-oszillator.patch                      \
    file://0068-Disable-SERDES-debug-messages.patch                         \
    file://0069-Increase-serset-retry-interval.patch                        \
    file://0070-Checked-in-1G-mode.patch                                    \
    file://0071-Update-SD-CARD-voltage-for-V200.patch                       \
    file://0072-Correct-speed-setting-and-prevent-SERDES-reset-durin.patch  \
    file://0073-Update-CAN-multiplexer-pins.patch                           \
    file://0074-pinctrl-renesas-r8a779f0-add-MSPI-pin-settings.patch        \
    file://0075-clk-renesas-r8a779f0-add-clocks-for-MSPI.patch              \
    file://0076-dts-r8a779f0-add-MSPI-devices.patch                         \
    file://0077-spi-add-initial-driver-for-Renesas-MSPI.patch               \
    file://0078-pinctrl-renesas-r8a779f0-set-MODSEL4-bits-for-MSPIn_.patch  \
    file://0079-clk-renesas-r8a779f0-fix-typo-CD_CLK_HSB-CD_CLK_LSB.patch   \
    file://0080-clk-renesas-r8a779f0-improve-control-domain-stub.patch      \
    file://0081-drivers-net-lan865x-import-vendor-files-as-is.patch         \
    file://0082-drivers-net-lan865x-add-Kconfig-and-Makefile.patch          \
    file://0083-drivers-net-lan865x-fix-compilation-warning.patch           \
    file://0084-drivers-net-lan865x-remove-code-for-hardcoded-gpio.patch    \
    file://0085-drivers-net-lan865x-update-reset-gpio-setting.patch         \
    file://0086-drivers-net-lan865x-allow-up-to-3-devices.patch             \
    file://0087-dts-VC4-add-LAN8650-devices.patch                           \
    file://0088-Activate-SPI-also-in-V1-device-tree.patch                   \
    file://0089-net-rswitch2-avoid-out-of-bound-array-access.patch          \
    file://0090-net-rswitch2-fix-error-return-values.patch                  \
    file://0091-clk-renesas-r8a779f0-do-not-hardcode-CANFD-and-MSPI-.patch  \
    file://0092-Add-recent-PTP-clock-driver.patch                           \
    file://0093-Add-PTP-initialization-to-RSwitch2-driver.patch             \
    file://0094-Add-forwarding-rules-for-PTP-link-local-address.patch       \
    file://0095-Get-IRQs-from-device-tree.patch                             \
    file://0096-Add-ethtool-support-and-IOCTLs-for-HW-time-stamping.patch   \
    file://0097-Add-multi-q-and-timestamp-to-support-PTP.patch              \
    file://0098-Change-SEL-CAN-FLXRAY-pin-location-for-VC4-V2.22.patch      \
    file://0099-Enable-the-both-gPTP-PPS-outputs.patch                      \
    file://0100-Fix-merge-loss-Assign-base-address-for-PTP.patch            \
    file://0101-Use-MAC-address-provided-by-platform.patch                  \
    file://0102-Update-DTS-to-have-number-from-EEPROM-on-all-interfa.patch  \
    file://0103-Small-code-fixups.patch                                     \
    file://0104-Fixed-comparision-of-states.patch                           \
    file://0105-Fixed-port-up-down-handling.patch                           \
    file://0106-rcu-Fix-to-include-first-blocked-task-in-stall-warni.patch  \
    file://0107-rcu-Add-lockdep_assert_irqs_disabled-to-rcu_sched_cl.patch  \
    file://0108-rcu-Fix-stall-warning-deadlock-due-to-non-release-of.patch  \
    file://0109-Disable-AVB-port-in-DT-as-this-is-currently-not-func.patch  \
    file://0110-Force-SPI-to-32-bit-data-to-prevent-buffer-underrun-.patch  \
    file://0111-Increase-the-SPI-speed-to-13.3-MHz-to-prevent-buffer.patch  \
    file://0112-Increase-SPI-speed-to-20-MHz-and-correct-printk.patch       \
    file://0113-Added-ethool-master-slave-support-1GBit-only.patch          \
    file://0114-Quick-fix.patch                                             \
    file://0115-drivers-net-lan865x-fix-register-access-completion-r.patch  \
    file://0116-drivers-net-lan865x-update-thread-and-interrupt-name.patch  \
    file://0117-dts-VC4-define-names-for-can-channel-gpios.patch            \
    file://0118-This-Linux-version-is-based-on-LTSI-version.patch           \
    file://0119-Fixed-PHY-mode-evaluation-in-SerDes-init.patch              \
    file://0120-Fixed-memory-leaks-on-module-unload.patch                   \
    file://0121-Re-configure-RMAC-if-PHY-reports-new-speed.patch            \
    file://0122-Add-logging-functions-with-section-support.patch            \
    file://0123-Make-use-of-new-logging.patch                               \
    file://0124-Activate-full-4Gbyte-of-RAM-as-in-Spider.patch              \
    file://0125-Moved-phy-connect-to-init.patch                             \
    file://0126-Add-T1S-PLCA-configuration-template-to-DT.patch             \
    file://0127-rswitch2-Temoprary-fix-to-use-upstream-PTP-driver.patch     \
    file://0128-Match-compatible-string-as-in-upstream-.dtsi.patch          \
    file://0129-Checkout-for-invalid-PHY-configuration.patch                \
    file://0130-Add-port-sub-nodes-to-DT-switch-entry.patch                 \
    file://0131-RSwitch2-Minor-fixes.patch                                  \
    file://0132-can-rcar_canfd-Add-support-for-RZ-G2L-family.patch          \
    file://0133-can-rcar_canfd-rcar_canfd_handle_channel_tx-fix-redu.patch  \
    file://0134-can-rcar_canfd-rcar_canfd_channel_probe-make-sure-we.patch  \
    file://0135-can-rcar_canfd-rcar_canfd_channel_probe-register-the.patch  \
    file://0136-can-rcar_can-do-not-report-txerr-and-rxerr-during-bu.patch  \
    file://0137-can-rcar_canfd-rcar_canfd_handle_global_receive-fix-.patch  \
    file://0138-can-rcar_canfd-fix-channel-specific-IRQ-handling-for.patch  \
    file://0139-can-rcar_canfd-Add-missing-ECC-error-checks-for-chan.patch  \
    file://0140-can-rcar_canfd-Introduce-is_gen4-helper.patch               \
    file://0141-can-rcar_canfd-Add-support-for-r8a779f0-SoC.patch           \
    file://0142-can-rcar_canfd-add-per-channel-gpio-control.patch           \
    file://0143-can-rcar_canfd-fix-AFL-setup.patch                          \
    file://0144-can-rcar_canfd-export-gpio-names-for-better-debugfs-.patch  \
    file://0145-Correct-the-SERDES-register-and-activate-the-check-a.patch  \
    file://0146-Resolve-Add-dedicated-PHY-for-Marvell-M88Q3344-engin.patch  \
    file://0147-Import-of-driver-version-0.6.patch                          \
    file://0148-pinctrl-renesas-r8a779f0-add-INTP-pin-settings.patch        \
    file://0149-Set-INTP-interrupts-in-DT-for-T1S.patch                     \
    file://0150-Some-comments-and-changes-during-Microchip-meeting.patch    \
    file://0151-Fix-usage-of-rtsn_ptp-driver-in-RSwitch-Rswitch2-dri.patch  \
    file://0152-Revert-can-rcar_canfd-Correct-the-value-of-tseg1.patch	    \
"

CONFIG = " \
    file://defconfig \
"

# Use defconfig provided with this recipe
unset KBUILD_DEFCONFIG


# Don't modules we properly do not provide
unset KERNEL_MODULE_AUTOLOAD
unset KERNEL_MODULE_PROBECONF


include linux-renesas-devel.inc

SRC_URI_append = " \
    ${CONFIG} \
    ${PATCHES} \
"

#The base device tree including all control domain functions
KERNEL_DEVICETREE_append_vc4 = " \
    renesas/r8a779f0-vc4-ctrl-domain.dtb \
"

#Install the DT overlays for phy selection
KERNEL_DEVICETREE_append_vc4 = " \
    renesas/r8a779f0-vc4-tsn0-phy-1g.dtbo \
    renesas/r8a779f0-vc4-tsn0-phy-2g5.dtbo \
    renesas/r8a779f0-vc4-tsn1-phy-1g.dtbo \
    renesas/r8a779f0-vc4-tsn1-phy-2g5.dtbo \
    renesas/r8a779f0-vc4-tsn2-phy-1g.dtbo \
    renesas/r8a779f0-vc4-tsn2-phy-1g-rh.dtbo \
"

KERNEL_DEVICETREE_append_vc4 = " \
    renesas/r8a779f0-vc4V1.dtb \
    renesas/r8a779f0-vc4-ctrl-domainV1.dtb \
    renesas/r8a779f0-vc4-tsn0-phy-1gV1.dtbo \
    renesas/r8a779f0-vc4-tsn0-phy-2g5V1.dtbo \
    renesas/r8a779f0-vc4-tsn1-phy-1gV1.dtbo \
    renesas/r8a779f0-vc4-tsn1-phy-2g5V1.dtbo \
"
