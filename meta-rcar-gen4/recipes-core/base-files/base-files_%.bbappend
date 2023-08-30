FILESEXTRAPATHS_prepend := "${THISDIR}/files:"
PR .= ".vc1"

SRC_URI += "file://blacklist.conf \
            file://motd \
	   "

CONFFILES_${PN} += " \
            ${sysconfdir}/modprobe.d/blacklist.conf \
            ${sysconfdir}/motd \
           "

do_install_append () {
	install -d ${D}${sysconfdir}/modprobe.d/
	install -m 755 ${WORKDIR}/blacklist.conf ${D}${sysconfdir}/modprobe.d
	install -m 644 ${WORKDIR}/motd ${D}${sysconfdir}

#    # change the default hostname
#    echo vc99 > ${D}${sysconfdir}/hostname
}
