SUMMARY = "Startup Service for R-Switch2"
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://${COMMON_LICENSE_DIR}/MIT;md5=0835ade698e0bcf8506ecda2f7b4f302"

inherit systemd

SRC_URI = " \
    file://rswitch2-startup.service \
    file://rswitch2-startup.sh \
    file://rswitch2.conf \
    file://fwd-default.xml \
    file://rswitch-init.target \
"

SYSTEMD_SERVICE_${PN} = "rswitch2-startup.service"

RDEPENDS_${PN} += "bash rswitch2tool"

FILES_${PN} = " \
    ${systemd_unitdir}/system/rswitch2-startup.service \
    ${systemd_unitdir}/system/rswitch-init.target \
    ${sbindir}/rswitch2-startup.sh \
    ${sysconfdir}/rswitch2/rswitch2.conf \
    ${sysconfdir}/rswitch2/fwd-default.xml \
"

do_install() {
    install -d ${D}${systemd_unitdir}/system
    install -m 0644 ${WORKDIR}/rswitch2-startup.service ${D}${systemd_unitdir}/system
    install -m 0644 ${WORKDIR}/rswitch-init.target ${D}${systemd_unitdir}/system

    install -d ${D}${sbindir}
    install -m 0755 ${WORKDIR}/rswitch2-startup.sh ${D}${sbindir}

    install -d ${D}${sysconfdir}/rswitch2
    install -m 0644 ${WORKDIR}/rswitch2.conf ${D}${sysconfdir}/rswitch2
    install -m 0644 ${WORKDIR}/fwd-default.xml ${D}${sysconfdir}/rswitch2
}
