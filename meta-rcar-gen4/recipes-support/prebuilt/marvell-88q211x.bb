DESCRIPTION = "Marvell 88Q211X Phy driver"
SECTION = "applications"
LICENSE = "CLOSED"
PR = "r0"

SRC_URI = " \
		file://marvell_88q211x.ko \
"

S = "${WORKDIR}"

do_configure[noexec] = "1"
do_compile[noexec] = "1"

do_install() {
	install -d ${D}${libdir}
    install -m 0775 marvell_88q211x.ko ${D}${libdir}/marvell_88q211x.ko
}

FILES_${PN} = " \
     ${libdir}/marvell_88q211x.ko \
"

INSANE_SKIP_${PN} = "dev-so"

# Skip debug strip of do_populate_sysroot()
INHIBIT_SYSROOT_STRIP = "1"

# Skip debug split and strip of do_package()
INHIBIT_PACKAGE_DEBUG_SPLIT = "1"
INHIBIT_PACKAGE_STRIP = "1"

INSANE_SKIP_${PN} = "ldflags"
FILES_${PN}-dev = "${includedir}"