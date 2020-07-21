SUMMARY = "MOST INIC flasher application"
LICENSE = "CLOSED"

SRC_URI = " \
    file://cbx3_fw_cfg.ipf \
    file://inic-flasher \
"

S = "${WORKDIR}"

do_install() {
    install -d ${D}${bindir}
    install -m 755 ${S}/inic-flasher ${D}${bindir}

    install -d ${D}/usr/share/inic-flasher/
    install -m 755 ${S}/cbx3_fw_cfg.ipf ${D}/usr/share/inic-flasher/
}

FILES_${PN} = " \
    ${bindir}/inic-flasher \
    /usr/share/inic-flasher/cbx3_fw_cfg.ipf \
"
