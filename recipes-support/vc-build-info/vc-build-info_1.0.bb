# Add VC specific build information files
DESCRIPTION = "VC specific build information"
SECTION = "vc-build-info"


LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://${COMMON_LICENSE_DIR}/MIT;md5=0835ade698e0bcf8506ecda2f7b4f302"


do_compile() {
    echo "This image has been build on $(hostname) at $(date) by ${USER}\n" > build-info.txt
    git log -1 >> build-info.txt
    git submodule foreach "git log -1" >> build-info.txt
    echo "\n" >>  build-info.txt
}


# install it in etc
do_install() {
  install -d ${D}/etc
  install -m 0644 build-info.txt ${D}/etc/build-info.txt
}

# package it as it is not installed in a standard location
FILES_${PN} = "/etc/build-info.txt"

