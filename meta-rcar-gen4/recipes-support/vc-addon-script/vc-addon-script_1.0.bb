SUMMARY = "Addons for Renesas VC4 boxes"

LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://${COMMON_LICENSE_DIR}/MIT;md5=0835ade698e0bcf8506ecda2f7b4f302"

inherit manpages

SRC_URI = " \
    file://startRSW2.sh \
    file://startCAN.sh \
    file://startT1S.sh \
    file://ptp4l.cfg \
    file://mrvl-ms \
    file://mrvl-counters \
    file://50-sw0* \
    file://t1s0.dtso \
    file://30-rswitch2-phy-detect.rules \
    file://45-sw0p0-2g5.link \
    file://45-sw0p1-2g5.link \
    file://50-sw0.network \
    file://50-sw0p0-1g.link \
    file://50-sw0p0.network \
    file://50-sw0p1-1g.link \
    file://50-sw0p1.network \
    file://50-sw0p2.link \
    file://50-sw0p2.network \
    file://vc4-net-config.5 \
"

RDEPENDS_${PN} =+ "bash"

FILES_${PN} = " \
    /home/root/* \
    /usr/bin/* \
    /etc/systemd/network/* \
    /etc/udev/rules.d/* \
    /usr/share/man/man5/* \
"


#for the off-tree build mrvl tools
INSANE_SKIP_${PN} += "ldflags"

do_install() {
    install -d ${D}/home/root
    install -d ${D}/home/root/examples
    install -d ${D}/usr/bin
    install -d ${D}/etc/systemd/network
    install -d ${D}/etc/udev/rules.d
    install -d ${D}/${mandir}/man5
    install -m 755 ${WORKDIR}/startRSW2.sh ${D}/home/root/examples
    install -m 755 ${WORKDIR}/startCAN.sh ${D}/home/root/examples
    install -m 755 ${WORKDIR}/startT1S.sh ${D}/home/root/examples
    install -m 755 ${WORKDIR}/t1s0.dtso ${D}/home/root/examples
    install -m 644 ${WORKDIR}/ptp4l.cfg ${D}/home/root/examples
    install -m 755 ${WORKDIR}/mrvl-ms ${D}/usr/bin
    install -m 755 ${WORKDIR}/mrvl-counters ${D}/usr/bin
    install -m 755 ${WORKDIR}/*.network ${D}/etc/systemd/network
    install -m 755 ${WORKDIR}/*.link ${D}/etc/systemd/network
    install -m 644 ${WORKDIR}/*.rules ${D}/etc/udev/rules.d
    install -m 644 ${WORKDIR}/vc4-net-config.5 ${D}/${mandir}/man5
}

FILES_${PN}-doc = "${mandir}"
