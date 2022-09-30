SUMMARY = "packETH is a packet generator tool for ethernet"
HOMEPAGE = "https://github.com/jemcek/packETH"
LICENSE = "GPLv2"

LIC_FILES_CHKSUM = "file://COPYING;md5=d32239bcb673463ab874e80d47fae504"

FILESEXTRAPATHS_prepend := "${THISDIR}/${PN}-${PV}:"

PV = "2.1.0+git${SRCPV}"

SRCREV = "a199b4fa4f2f0c9757f99c12c5d3f4fc30be122e"

SRC_URI = " \
	git://github.com/jemcek/packETH.git;protocol=https			\
        file://0001-Add-generic-IP-checksum-function.patch                     	\
        file://0002-Allow-to-pass-environment-variables-to-Makefile.patch      	\
"



#S = "${WORKDIR}"
S = "${WORKDIR}/git"
# The Makefile has "$PREFIX/bin" hardcoded into it, hence not using $bindir here
do_compile() {
    oe_runmake '-Ccli'
}

do_install() {
    install -d ${D}${prefix}/bin
    oe_runmake 'DESTDIR=${D}' 'PREFIX=${prefix}' -Ccli install
}

