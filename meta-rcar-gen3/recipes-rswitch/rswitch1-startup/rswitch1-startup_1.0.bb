SUMMARY = "Startup Service for R-Switch1"
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://${COMMON_LICENSE_DIR}/MIT;md5=0835ade698e0bcf8506ecda2f7b4f302"

inherit systemd

SRC_URI = " \
    file://rswitch1-startup.service \
    file://rswitch1-startup.sh \
    file://rswitch1.conf \
    file://phyinit.sh \
    file://writeRTL9010.sh \
    file://tsn2ToEth1.sh \
    file://l2_gptp_default.xml \
    file://tsngw_static_entry.template \
    file://rswitch-init.target \
"

SYSTEMD_SERVICE_${PN} = "rswitch1-startup.service"

RDEPENDS_${PN} += "bash sed phytool rswitch1tool"

FILES_${PN} = " \
    ${systemd_unitdir}/system/rswitch1-startup.service \
    ${systemd_unitdir}/system/rswitch-init.target \
    ${sbindir}/rswitch1-startup.sh \
    ${sysconfdir}/rswitch1/rswitch1.conf \
    ${sysconfdir}/rswitch1/phyinit.sh \
    ${sysconfdir}/rswitch1/writeRTL9010.sh \
    ${sysconfdir}/rswitch1/tsn2ToEth1.sh \
    ${sysconfdir}/rswitch1/l2_gptp_default.xml \
    ${sysconfdir}/rswitch1/tsngw_static_entry.template \
"

do_install() {
    install -d ${D}${systemd_unitdir}/system
    install -m 0644 ${WORKDIR}/rswitch1-startup.service ${D}${systemd_unitdir}/system
    install -m 0644 ${WORKDIR}/rswitch-init.target ${D}${systemd_unitdir}/system

    install -d ${D}${sbindir}
    install -m 0755 ${WORKDIR}/rswitch1-startup.sh ${D}${sbindir}

    install -d ${D}${sysconfdir}/rswitch1
    install -m 0644 ${WORKDIR}/rswitch1.conf   ${D}${sysconfdir}/rswitch1
    install -m 0755 ${WORKDIR}/phyinit.sh      ${D}${sysconfdir}/rswitch1
    install -m 0755 ${WORKDIR}/writeRTL9010.sh ${D}${sysconfdir}/rswitch1
    install -m 0755 ${WORKDIR}/tsn2ToEth1.sh   ${D}${sysconfdir}/rswitch1
    install -m 0644 ${WORKDIR}/l2_gptp_default.xml ${D}${sysconfdir}/rswitch1
    install -m 0644 ${WORKDIR}/tsngw_static_entry.template ${D}${sysconfdir}/rswitch1
}
