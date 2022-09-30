DESCRIPTION = "LAN865x driver start service"
SECTION = "startLAN865x"
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://${COMMON_LICENSE_DIR}/MIT;md5=0835ade698e0bcf8506ecda2f7b4f302"
PR = "r0"

DEPENDS = "systemd jansson"

SRC_URI = " \
		file://startLAN865x.service \
"

S = "${WORKDIR}"

inherit systemd
SYSTEMD_SERVICE_${PN} = "startLAN865x.service"

do_install_append() {
	if ${@bb.utils.contains('DISTRO_FEATURES', 'systemd', 'true', 'false', d)}; then
		install -d ${D}/${systemd_system_unitdir}
		install -m 0644 ${WORKDIR}/startLAN865x.service ${D}/${systemd_system_unitdir}
	fi
}
