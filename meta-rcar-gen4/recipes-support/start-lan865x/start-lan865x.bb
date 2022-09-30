DESCRIPTION = "LAN865x start script"
SECTION = "startLAN865x"
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://${COMMON_LICENSE_DIR}/MIT;md5=0835ade698e0bcf8506ecda2f7b4f302"

DEPENDS = " linux-renesas bash "


SRC_URI = " \
                file://startLAN865x.sh \
"

S = "${WORKDIR}"

do_install() {
    install -d ${D}${bindir}
	install -m 0775 startLAN865x.sh ${D}${bindir}
}

do_configure[noexec] = "1"
do_compile[noexec] = "1"

RDEPENDS_${PN} += " bash "

FILES_${PN} = " \
     ${bindir}/startLAN865x.sh \
"

