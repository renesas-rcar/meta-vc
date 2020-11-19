SUMMARY = "Addons for Renesas VC boxes"

LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://${COMMON_LICENSE_DIR}/MIT;md5=0835ade698e0bcf8506ecda2f7b4f302"

SRC_URI = " \
    file://hwVersion.sh \
    file://printPhyRegs.tcl \
    file://rsw1_VC2_EP_rtlid_0x20032706_sysid_0x20050741.bin \
    file://rsw1_VC2_MT_rtlid_0x20032706_sysid_0x20050741.bin \
    file://rsw1_VC3_66_rtlid_0x20032708_sysid_0x20082549.bin \
    file://rsw2_VC3_66_rtlid_0x20110444_sysid_0x20110549.bin \
"

RDEPENDS_${PN} =+ "bash tcl"

FILES_${PN} = " \
    ${bindir}/hwVersion.sh \
    ${bindir}/printPhyRegs.tcl \
    ${libdir}/firmware/vehicle-computer/rsw1_VC2_EP_rtlid_0x20032706_sysid_0x20050741.bin \
    ${libdir}/firmware/vehicle-computer/rsw1_VC2_MT_rtlid_0x20032706_sysid_0x20050741.bin \
    ${libdir}/firmware/vehicle-computer/rsw1_VC3_66_rtlid_0x20032708_sysid_0x20082549.bin \
    ${libdir}/firmware/vehicle-computer/rsw2_VC3_66_rtlid_0x20110444_sysid_0x20110549.bin \
"

do_install() {
    install -d ${D}${bindir}
    install -m 755 ${WORKDIR}/hwVersion.sh ${D}${bindir}
    install -m 755 ${WORKDIR}/printPhyRegs.tcl ${D}${bindir}

    install -d ${D}${libdir}/firmware/vehicle-computer
    install -m 644 ${WORKDIR}/rsw1_VC2_EP_rtlid_0x20032706_sysid_0x20050741.bin ${D}${libdir}/firmware/vehicle-computer
    install -m 644 ${WORKDIR}/rsw1_VC2_MT_rtlid_0x20032706_sysid_0x20050741.bin ${D}${libdir}/firmware/vehicle-computer
    install -m 644 ${WORKDIR}/rsw1_VC3_66_rtlid_0x20032708_sysid_0x20082549.bin ${D}${libdir}/firmware/vehicle-computer
    
    install -m 644 ${WORKDIR}/rsw2_VC3_66_rtlid_0x20110444_sysid_0x20110549.bin ${D}${libdir}/firmware/vehicle-computer
}
