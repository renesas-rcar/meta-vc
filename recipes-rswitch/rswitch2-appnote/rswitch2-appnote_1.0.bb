SUMMARY = "R-Switch2 Application Note"
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://${COMMON_LICENSE_DIR}/MIT;md5=0835ade698e0bcf8506ecda2f7b4f302"

SRC_URI = " \
    file://rswitch2-appnote_${PV}.tar.gz \
"

S = "${WORKDIR}/rswitch2-appnote"

include rswitch2-appnote-devel.inc

FILES_${PN} = " \
    ${datadir}/rswitch2-appnote/* \
"

do_install() {
    install -d ${D}${datadir}/rswitch2-appnote
    cp --preserve=mode,timestamps -R ${S}/* ${D}${datadir}/rswitch2-appnote
}
