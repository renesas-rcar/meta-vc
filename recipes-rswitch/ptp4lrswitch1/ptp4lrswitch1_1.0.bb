#
# This file was derived from the 'Hello World!' example recipe in the
# Yocto Project Development Manual.
#

DESCRIPTION = "ptp4l tools application for rswitch"
SECTION = "ptp4lrswitch1"
DEPENDS = ""
LICENSE = "MIT"
#LIC_FILES_CHKSUM = "file://LICENSE;md5=96af5705d6f64a88e035781ef00e98a8"
#LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://${COMMON_LICENSE_DIR}/MIT;md5=0835ade698e0bcf8506ecda2f7b4f302"
SRC_FILES := "ptp4l.c \
bmc.c \
clock.c \
clockadj.c \
clockcheck.c \
config.c \
fault.c \
filter.c \
fsm.c \
mave.c \
mmedian.c \
msg.c \
phc.c \
pi.c \
port.c \
print.c \
raw.c \
servo.c \
sk.c \
stats.c \
tlv.c \
transport.c \
udp.c \
udp6.c \
uds.c \
util.c \
version.c \
hwstamp_ctl.c \
phc2sys.c \
pmc.c \
pmc_common.c \
sysoff.c "

SRC_FILES_phc2sys := "clockadj.c \
clockcheck.c \
msg.c \
phc.c \
phc2sys.c \
pi.c \
pmc_common.c \
print.c \
raw.c \
servo.c \
sk.c \
stats.c \
sysoff.c \
tlv.c \
transport.c \
udp.c \
udp6.c \
uds.c \
util.c \
version.c \
"

SRC_FILES_ptp4l := "ptp4l.c \
bmc.c \
clock.c \
clockadj.c \
clockcheck.c \
config.c \
fault.c \
filter.c \
fsm.c \
mave.c \
mmedian.c \
msg.c \
phc.c \
pi.c \
port.c \
print.c \
raw.c \
servo.c \
sk.c \
stats.c \
tlv.c \
transport.c \
udp.c \
udp6.c \
uds.c \
util.c \
version.c \
"

SRC_FILES_pmc := "msg.c \
print.c \
raw.c \
servo.c \
sk.c \
tlv.c \
transport.c \
udp.c \
udp6.c \
uds.c \
util.c \
version.c \
pmc.c \
pi.c \
pmc_common.c "

SRC_FILES_hwstamp_ctl := "hwstamp_ctl.c  \
version.c \
"




INC_DIR = "${THISDIR}/files"

KERNEL_PATH = "${TOPDIR}/tmp/work-shared/h3vc/kernel-source/"

INCLUDEFLAGS += "-I${INC_DIR}/. -I${KERNEL_PATH}"
#OSFLAG = "-DLINUX -D_GNU_SOURCE -D_REENTRANT"
LFLAGS	= "-lc -lm"
LDFLAGS = "${LFLAGS}"
#CFLAGS = "${OSFLAG} ${CDEFS} ${WARNING} ${INCLUDEFLAGS} -D_THREAD_SAFE"
SRC_URI= "file://ptp4l.c \
          file://bmc.c \
          file://clock.c \
          file://clockadj.c \
          file://clockcheck.c \
          file://config.c \
          file://fault.c \
          file://filter.c \
          file://fsm.c \
          file://mave.c \
          file://mmedian.c \
          file://msg.c \
          file://phc.c \
          file://pi.c \
          file://port.c \
          file://print.c \
          file://raw.c \
          file://servo.c \
          file://sk.c \
          file://stats.c \
          file://tlv.c \
          file://transport.c \
          file://udp.c \
          file://udp6.c \
          file://uds.c \
          file://util.c \
          file://version.c \
          file://hwstamp_ctl.c \
          file://phc2sys.c \
          file://pmc.c \
          file://pmc_common.c \
          file://sysoff.c \         
"

S = "${WORKDIR}"
do_compile() {
	${CC} ${SRC_FILES_pmc}  ${LDFLAGS} ${INCLUDEFLAGS} -o pmc
        ${CC} ${SRC_FILES_ptp4l}  ${LDFLAGS} ${INCLUDEFLAGS} -o ptp4l
        ${CC} ${SRC_FILES_ptp4l}  ${LDFLAGS} ${INCLUDEFLAGS} -o phc2sys
        ${CC} ${SRC_FILES_hwstamp_ctl}  ${LDFLAGS} ${INCLUDEFLAGS} -o hwstamp_ctl
}
do_install() {
	install -d ${D}${bindir}
	install -m 0755 pmc ${D}${bindir}
        install -m 0755 ptp4l ${D}${bindir}
        install -m 0755 phc2sys ${D}${bindir}
        install -m 0755 hwstamp_ctl ${D}${bindir}
}




