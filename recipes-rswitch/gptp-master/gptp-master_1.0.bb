#
# This file was derived from the 'Hello World!' example recipe in the
# Yocto Project Development Manual.
#

DESCRIPTION = "gptp-master tools application"
SECTION = "gptp-master"

LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://${COMMON_LICENSE_DIR}/MIT;md5=0835ade698e0bcf8506ecda2f7b4f302"

inherit pkgconfig cmake

SRC_URI = "file://common/ap_message.cpp      \
           file://common/avbap_message.hpp    \
           file://common/avbts_clock.hpp      \
           file://common/avbts_message.hpp    \
           file://common/avbts_oscondition.hpp\
           file://common/avbts_osipc.hpp      \
           file://common/avbts_oslock.hpp     \
           file://common/avbts_osnet.cpp      \
           file://common/avbts_osnet.hpp      \
           file://common/avbts_osthread.hpp   \
           file://common/avbts_ostimer.hpp    \
           file://common/avbts_ostimerq.hpp   \
           file://common/avbts_persist.hpp    \
           file://common/common_port.cpp      \
           file://common/common_port.hpp      \
           file://common/common_tstamper.hpp  \
           file://common/ether_port.cpp       \
           file://common/ether_port.hpp       \
           file://common/ether_tstamper.hpp   \
           file://common/gptp_cfg.cpp         \
           file://common/gptp_cfg.hpp         \
           file://common/gptp_log.cpp         \
           file://common/gptp_log.hpp         \
           file://common/ieee1588clock.cpp    \
           file://common/ieee1588.hpp         \
           file://common/ini.c                \
           file://common/ini.h                \
           file://common/ipcdef.hpp           \
           file://common/ptp_message.cpp      \
           file://common/ptptypes.hpp         \
           file://common/wireless_port.cpp    \
           file://common/wireless_port.hpp    \
           file://common/wireless_tstamper.cpp\
           file://common/wireless_tstamper.hpp\
	  "
SRC_URI_append = "file://linux/src/daemon_cl.cpp \
                  file://linux/src/linux_hal_common.cpp            \
                  file://linux/src/linux_hal_common.hpp            \
                  file://linux/src/linux_hal_generic_adj.cpp       \
                  file://linux/src/linux_hal_generic.cpp           \
                  file://linux/src/linux_hal_generic.hpp           \
                  file://linux/src/linux_hal_generic_tsprivate.hpp \
                  file://linux/src/linux_hal_i210.cpp              \
                  file://linux/src/linux_hal_intelce.cpp           \
                  file://linux/src/linux_hal_intelce.hpp           \
                  file://linux/src/linux_hal_persist_file.cpp      \
                  file://linux/src/linux_hal_persist_file.hpp      \
                  file://linux/src/linux_ipc.hpp                   \
                  file://linux/src/platform.cpp                    \
                  file://linux/src/platform.hpp                    \
                  file://linux/src/watchdog.cpp                    \
                  file://linux/src/watchdog.hpp                    \
                 "

SRC_URI_append = "file://CMakeLists.txt \
                  file://gptp_cfg.ini   \
                 "


S = "${WORKDIR}"


do_install() {
    install -d ${D}${bindir}
    install -m 0755 gptp ${D}${bindir}
}


