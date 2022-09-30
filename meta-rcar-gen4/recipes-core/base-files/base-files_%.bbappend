FILESEXTRAPATHS_prepend := "${THISDIR}/files:"
PR .= ".1"

SRC_URI += "file://blacklist.conf \
	   "

CONFFILES_${PN} += "${sysconfdir}/modprobe.d/blacklist.conf"

do_install_append () {
	install -d ${D}${sysconfdir}/modprobe.d/
	install -m 755 ${WORKDIR}/blacklist.conf ${D}${sysconfdir}/modprobe.d/blacklist.conf

#    # change the default hostname
#    echo vc99 > ${D}${sysconfdir}/hostname
}










