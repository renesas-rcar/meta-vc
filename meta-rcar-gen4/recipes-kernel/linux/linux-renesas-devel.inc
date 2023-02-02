#This files overwites the configuration for Rnesas internal build
#It includes the proprietary drivers and generates the device tree files for VC4 v1.0 (green PCB)

RENESAS_BSP_URL = "git://git@gitlab.eur.eng.idt.com:2222/automotive-network-section-ans/vehicle-computer-vc/renesas-linux.git"

BRANCH = "vc4"
#BRANCH = "205-add-dedicated-phy-for-marvell-m88q3344-engineering-sample"
SRC_URI = "${RENESAS_BSP_URL};protocol=ssh;nocheckout=1;branch=${BRANCH}"


#BRANCH = "137-activate-avb-port-2"
#SRC_URI = "git:///home/cmard/yocto/vc4/renesas-linux-cogent;protocol=file;branch=${BRANCH}"

# Always use the latest version
SRCREV = "${AUTOREV}"

CONFIG_append = " \
    file://proprietary.cfg \
"

KERNEL_DEVICETREE_append_vc4 = " \
    renesas/r8a779f0-vc4V1.dtb \
    renesas/r8a779f0-vc4-ctrl-domainV1.dtb \
    renesas/r8a779f0-vc4-tsn0-phy-1gV1.dtbo \
    renesas/r8a779f0-vc4-tsn0-phy-2g5V1.dtbo \
    renesas/r8a779f0-vc4-tsn1-phy-1gV1.dtbo \
    renesas/r8a779f0-vc4-tsn1-phy-2g5V1.dtbo \
"
