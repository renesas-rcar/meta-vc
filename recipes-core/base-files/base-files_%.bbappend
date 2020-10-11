FILESEXTRAPATHS_prepend := "${THISDIR}/files:"
PR .= ".1"

SRC_URI_append = "file://vc-startup.sh \
           "


# these 3 lines will have the script run on boot
inherit update-rc.d
INITSCRIPT_PACKAGES = "${PN}"
INITSCRIPT_NAME = "vc-startup.sh"


do_install_append () {
    # change the default hostname
    echo vc99 > ${D}${sysconfdir}/hostname

    install -d ${D}${INIT_D_DIR}
    install -m 0755 ${S}/vc-startup.sh ${D}${INIT_D_DIR}/vc-startup.sh
}










