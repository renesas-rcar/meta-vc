FILESEXTRAPATHS_prepend := "${THISDIR}/files:"
PR .= ".vc1"

inherit image-buildinfo

S = "${WORKDIR}"

SRC_URI += "file://motd \
            file://buildinfo \
            file://issue \
	   "

CONFFILES_${PN} += " \
            ${sysconfdir}/motd \
            ${sysconfdir}/buildinfo \
            ${sysconfdir}/issue \
           "

#BASEFILESISSUEINSTALL = ""

IMAGE_BUILDINFO_VARS_append = " DATETIME DISTRO_NAME DISTRO_CODENAME BBFILE_COLLECTIONS MACHINE TUNE_PKGARCH"
IMAGE_BUILDINFO_VARS_append = " MACHINE_FEATURES DISTRO_FEATURES COMMON_FEATURES IMAGE_FEATURES"

python do_configure () {
    full_path = d.expand("${S}/buildinfo")

    with open(full_path, 'w') as file:
        file.writelines((
            '''-----------------------
Build Configuration:  |
-----------------------
''',
            buildinfo_target(d),
            '''
-----------------------
Layer Revisions:      |
-----------------------
''',
            get_layer_revs(d),
            '''
'''
       ))
}

DISTRO_VERSION[vardepsexclude] += "DATE"
do_install_basefilesissue () {
    install -m 644 ${WORKDIR}/issue*  ${D}${sysconfdir}
    printf "\\\n \\\l\n" >> ${D}${sysconfdir}/issue
    echo >> ${D}${sysconfdir}/issue
    echo "%h"    >> ${D}${sysconfdir}/issue.net
    echo >> ${D}${sysconfdir}/issue.net
}


do_install_append () {
    install -d ${D}${sysconfdir}
    install -m 0644 ${S}/buildinfo ${D}${sysconfdir}

    install -m 644 ${WORKDIR}/motd ${D}${sysconfdir}

#    # change the default hostname
#    echo vc99 > ${D}${sysconfdir}/hostname
}

FILES_${PN} += "${sysconfdir}"
