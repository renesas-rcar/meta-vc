RENESAS_BSP_URL = "git://git@ree-dusgitlab.ree.adwin.renesas.com/r-car-gen3-yocto/renesas-linux.git"
#BRANCH = "v4.14.75-ltsi/rcar-3.9.7-rswitch2/release"
BRANCH = "v4.14.75-ltsi/rcar-3.9.7-vc-develop"
SRC_URI = "${RENESAS_BSP_URL};protocol=ssh;nocheckout=1;branch=${BRANCH}"

# Always use latest version
SRCREV = "${AUTOREV}"

PATCHES = ""

HYPERFLASH_CONFIG = " \
    file://hyperflash.cfg \
"

CONFIG = " \
    file://defconfig \
    file://touch.cfg \
    file://h3vc.cfg \
    file://rswitch2.cfg \
    ${@oe.utils.conditional("ENABLE_HYPERFLASH_LINUX", "1", "${HYPERFLASH_CONFIG}", "", d)} \
"

