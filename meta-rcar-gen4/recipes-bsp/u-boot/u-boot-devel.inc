RENESAS_UBOOT_URL = "git://git@gitlab.eur.eng.idt.com:2222/automotive-network-section-ans/vehicle-computer-vc/u-boot.git"
BRANCH = "v2020.10/vc4"
SRC_URI = "${RENESAS_UBOOT_URL};protocol=ssh;branch=${BRANCH}"

# Always use the latest version
SRCREV = "${AUTOREV}"

PATCHES = ""


