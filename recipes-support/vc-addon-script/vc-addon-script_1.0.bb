SUMMARY = "Addons for Renesas VC boxes"

LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://${COMMON_LICENSE_DIR}/MIT;md5=0835ade698e0bcf8506ecda2f7b4f302"

RSWITCH1_FW = " \
    file://rsw1_VC2_EP_rtlid_0x20032706_sysid_0x20050741.bin \
    file://rsw1_VC2_MT_rtlid_0x20032706_sysid_0x20050741.bin \
    file://rsw1_VC3_48_rtlid_0x20032708_sysid_0x20121641.bin \
    file://rsw1_VC3_66_rtlid_0x20032708_sysid_0x20082549.bin \
"

RSWITCH2_FW = " \
    file://rsw2_VC3_66_rtlid_0x21031644_sysid_0x21041649.bin \
"

SRC_URI = " \
    file://hwVersion.sh \
    file://printPhyRegs.tcl \
    ${@bb.utils.contains('DISTRO_FEATURES','rswitch1','${RSWITCH1_FW}','${RSWITCH2_FW}',d)} \
"

RDEPENDS_${PN} =+ "bash tcl"

FILES_${PN} = " \
    ${bindir}/hwVersion.sh \
    ${bindir}/printPhyRegs.tcl \
    ${libdir}/firmware/vehicle-computer/* \
"

do_install() {
    install -d ${D}${bindir}
    install -m 755 ${WORKDIR}/hwVersion.sh ${D}${bindir}
    install -m 755 ${WORKDIR}/printPhyRegs.tcl ${D}${bindir}

    install -d ${D}${libdir}/firmware/vehicle-computer
    if ${@bb.utils.contains('DISTRO_FEATURES','rswitch1','true','false',d)}; then
        install -m 644 ${WORKDIR}/rsw1_VC2_EP_rtlid_0x20032706_sysid_0x20050741.bin ${D}${libdir}/firmware/vehicle-computer
        install -m 644 ${WORKDIR}/rsw1_VC2_MT_rtlid_0x20032706_sysid_0x20050741.bin ${D}${libdir}/firmware/vehicle-computer
        install -m 644 ${WORKDIR}/rsw1_VC3_48_rtlid_0x20032708_sysid_0x20121641.bin ${D}${libdir}/firmware/vehicle-computer
        install -m 644 ${WORKDIR}/rsw1_VC3_66_rtlid_0x20032708_sysid_0x20082549.bin ${D}${libdir}/firmware/vehicle-computer
    else
        install -m 644 ${WORKDIR}/rsw2_VC3_66_rtlid_0x21031644_sysid_0x21041649.bin ${D}${libdir}/firmware/vehicle-computer
    fi
}
