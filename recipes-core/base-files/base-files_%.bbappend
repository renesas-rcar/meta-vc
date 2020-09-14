FILESEXTRAPATHS_prepend := "${THISDIR}/files:"
PR .= ".1"

do_install_append () {
    # change the default hostname
    echo vc99 > ${D}${sysconfdir}/hostname
}
