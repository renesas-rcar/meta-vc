SUMMARY = "Addons for Renesas VC4 boxes"

LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://${COMMON_LICENSE_DIR}/MIT;md5=0835ade698e0bcf8506ecda2f7b4f302"

SRC_URI = " \
    file://startRSW2.sh \
    file://startCAN.sh \
    file://ptp4l.cfg \
    file://mrvl-ms \
    file://mrvl-counters \
"

RDEPENDS_${PN} =+ "bash"

FILES_${PN} = " \
    /home/root/* \
    /usr/bin/* \
"

#for the off-tree build mrvl tools
INSANE_SKIP_${PN} += "ldflags"

do_install() {
    install -d ${D}/home/root
    install -d ${D}/home/root/examples
    install -d ${D}/usr/bin
    install -m 755 ${WORKDIR}/startRSW2.sh ${D}/home/root
    install -m 755 ${WORKDIR}/startCAN.sh ${D}/home/root/examples
    install -m 644 ${WORKDIR}/ptp4l.cfg ${D}/home/root/examples
    install -m 755 ${WORKDIR}/mrvl-ms ${D}/usr/bin
    install -m 755 ${WORKDIR}/mrvl-counters ${D}/usr/bin
}
