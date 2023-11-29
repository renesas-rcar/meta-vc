DESCRIPTION = "MACsec Key Agreement (MKA) daemon"
SECTION = "mka-daemon"

LICENSE = "GPLv2"
LIC_FILES_CHKSUM = "file://LICENSE.txt;md5=75859989545e37968a99b631ef42722e"

inherit autotools
inherit systemd


DEPENDS = "coreutils libyaml openssl libxml2 libnl libbsd glib-2.0 glib-2.0-native dbus libnl"

PV = "git${SRCPV}"
SRCREV = "49cb587c81bfb2ee85da12113c22ca50db09e9fd"
SRC_URI = "git://github.com/Technica-Engineering/MKAdaemon.git;branch=main;protocol=https"

SRC_URI += " \
    file://mkad.service 			\
    file://mkad.conf    			\
"

S = "${WORKDIR}/git"


SYSTEMD_AUTO_ENABLE = "disable"
SYSTEMD_SERVICE_${PN} = "mkad.service"


FILES_${PN} += " \
    ${systemd_unitdir}/system/mkad.service \
    ${datadir}/dbus-1/system-services/*    \
"
CONFFILES_${PN} += "${sysconfdir}/mka.d/mkad.conf "


EXTRA_OECONF_append="--top=${S}"

do_configure() {
    python3 ${S}/waf --top=${S} --out=. --prefix=${prefix} --destdir=${D} configure
}

do_compile() {
    python3 ${S}/waf --top=${S} build
}

do_install() {
    python3 ${S}/waf --top=${S} --prefix=${prefix} --destdir=${D} install

    install -d ${D}${sysconfdir}/mka.d
    install -d ${D}${systemd_unitdir}/system
    install -m 644 ${WORKDIR}/mkad.service ${D}${systemd_unitdir}/system/mkad.service 
    install -m 755 ${WORKDIR}/mkad.conf ${D}${sysconfdir}/mka.d/mkad.conf

    install -d ${D}/${sysconfdir}/dbus-1/system.d
    install -m 644 ${S}/dbus-policies/de.technica_engineering.mkad.conf ${D}/${sysconfdir}/dbus-1/system.d
}


