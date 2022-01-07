RENESAS_UBOOT_URL = "git://git@ree-dusgitlab.ree.adwin.renesas.com/r-car-gen3-yocto/u-boot.git"
BRANCH = "v2020.10/vc4"
SRC_URI = "${RENESAS_UBOOT_URL};protocol=ssh;branch=${BRANCH}"

# Always use the latest version
SRCREV = "${AUTOREV}"

PATCHES = ""


