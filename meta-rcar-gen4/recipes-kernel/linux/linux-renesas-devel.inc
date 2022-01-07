RENESAS_BSP_URL = "git://git@ree-dusgitlab.ree.adwin.renesas.com/r-car-gen3-yocto/renesas-linux.git"
BRANCH = "vc4"
SRC_URI = "${RENESAS_BSP_URL};protocol=ssh;nocheckout=1;branch=${BRANCH}"

# Always use the latest version
SRCREV = "${AUTOREV}"

PATCHES = ""

CONFIG = " \
    file://vc4.cfg \
"
