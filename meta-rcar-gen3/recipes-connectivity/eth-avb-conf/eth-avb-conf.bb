SUMMARY = "Ethernet AVB configuration."
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://${COMMON_LICENSE_DIR}/MIT;md5=0835ade698e0bcf8506ecda2f7b4f302"

PV = "1.0"

SRC_URI = " \
    file://eth_avb.network \
"

FILES_${PN} = " \
    ${sysconfdir}/systemd/network/eth_avb.network \
"

do_install() {
    install -d ${D}${sysconfdir}/systemd/network
    install -m 0644 ${WORKDIR}/eth_avb.network ${D}${sysconfdir}/systemd/network
}
