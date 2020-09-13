#
# REE loccl development repository override
# Internal use only - Repository unaccessable from outside REE
# Do not distribute.
#

RENESAS_DEVDUMP_URL = "git://git@ree-dusgitlab.ree.adwin.renesas.com/r-car-gen3-yocto/vc-tools/devdump.git"
BRANCH = "master"

# Use latest version
SRCREV = "${AUTOREV}"


SRC_URI = "${RENESAS_DEVDUMP_URL};protocol=ssh;branch=${BRANCH}"
S = "${WORKDIR}/git/"


