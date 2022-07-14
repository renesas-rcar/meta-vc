SUMMARY = "Addons for Renesas VC4 boxes"

LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://${COMMON_LICENSE_DIR}/MIT;md5=0835ade698e0bcf8506ecda2f7b4f302"

SRC_URI = " \
    file://startRSW2.sh \
"

RDEPENDS_${PN} =+ "bash"

FILES_${PN} = " \
    /home/root/* \
"

do_install() {
    install -d ${D}/home/root
   install -m 755 ${WORKDIR}/startRSW2.sh ${D}/home/root
}
