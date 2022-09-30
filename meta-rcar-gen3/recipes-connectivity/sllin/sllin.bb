SUMMARY = "Linux LIN bus drivers and tools"
SECTION = "kernel/modules"
DEPENDS = "virtual/kernel"

LICENSE = "GPLv2"
LIC_FILES_CHKSUM = "file://sllin.c;md5=e8c1259579b7df92e95b047d02cbf3ad"

PN = "sllin"
PE = "1"
PV = "0.1"

SRC_URI = "git://github.com/ppisa/linux-lin.git \
           file://0001-update-to-4.14-compatibility.patch \
"
SRCREV = "fdb6110ae3fa0f14137ffbec4af1bc124945a076"

S = "${WORKDIR}/git/sllin"

inherit module

EXTRA_OEMAKE = "KPATH=${STAGING_KERNEL_DIR} KLIB=${D}"

do_install() {
    # Create shared folder
    install -d ${D}/lib/modules/${KERNEL_VERSION}/tty/

    # Copy kernel module
    install -m 0644 ${S}/sllin.ko ${D}/lib/modules/${KERNEL_VERSION}/tty/
}

FILES_kernel-module-${PN} = " \
    /lib/modules/${KERNEL_VERSION}/tty/sllin.ko \
"
