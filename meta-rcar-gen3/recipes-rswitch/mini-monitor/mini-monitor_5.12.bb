DESCRIPTION = "Renesas Mini Monitor"

LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://${COMMON_LICENSE_DIR}/MIT;md5=0835ade698e0bcf8506ecda2f7b4f302"

PACKAGE_ARCH = "${MACHINE_ARCH}"

inherit deploy

SRC_URI = " \
    file://mini-monitor_${PV}.tar.gz \
"

SRC_URI_append = " \
    file://flash-all.sh \
"


S = "${WORKDIR}/mini-monitor/Gen3_MiniMonitor"


include mini-monitor-devel.inc


COMPATIBLE_MACHINE = "(salvator-x|ulcb|ebisu|h3vc)"
PLATFORM = "rcar"


# requires CROSS_COMPILE set by hand as there is no configure script
export CROSS_COMPILE="${TARGET_PREFIX}"
export CROSS_SYSROOT="${WORKDIR}/recipe-sysroot"
#export CROSS_SYSROOT="${S}/recipe-sysroot"

# Let the Makefile handle setting up the CFLAGS and LDFLAGS as it is a standalone application
CFLAGS[unexport] = "1"
LDFLAGS[unexport] = "1"
AS[unexport] = "1"
LD[unexport] = "1"

#CFLAGS += " --nostdinc "
#LDFLAGS += " --nostdlib "
do_compile() {
    echo ${S}
    oe_runmake clean
    mkdir -p obj
#    oe_runmake -k BOOT=SCIF AArch=64 LSI=H3
    oe_runmake BOOT=SCIF AArch=64 LSI=H3
}

# do_install() nothing
do_install[noexec] = "1"

do_deploy() {
    # Create deploy folder
    install -d ${DEPLOYDIR}

    # Copy Mini Monitor to deploy folder
    install -m 0644 ${S}/AArch64_output/AArch64_Gen3_H3_M3_Scif_MiniMon*.mot ${DEPLOYDIR}
    install -m 0644 ${S}/AArch64_output/AArch64_Gen3_H3_M3_Scif_MiniMon*.axf ${DEPLOYDIR}
    install -m 0644 ${S}/AArch64_output/AArch64_Gen3_H3_M3_Scif_MiniMon*.bin ${DEPLOYDIR}
    install -m 0644 ${WORKDIR}/flash-all.sh ${DEPLOYDIR}
}
addtask deploy before do_build after do_compile
