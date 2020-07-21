# Add RSwitch1 specific init files
DESCRIPTION = "RSwitch specific init files"
SECTION = "rswitch1-init"

SRC_URI = "file://vc2startup.sh"

LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://${COMMON_LICENSE_DIR}/MIT;md5=0835ade698e0bcf8506ecda2f7b4f302"

# these 3 lines will have the script run on boot
inherit update-rc.d
INITSCRIPT_PACKAGES = "${PN}"
INITSCRIPT_NAME = "vc2startup.sh"

# install it in the correct location for update-rc.d
do_install() {
  install -d ${D}${INIT_D_DIR}
  install -m 0755 ${WORKDIR}/vc2startup.sh ${D}${INIT_D_DIR}/vc2startup.sh
}

# package it as it is not installed in a standard location
FILES_${PN} = "${INIT_D_DIR}/vc2startup.sh"

