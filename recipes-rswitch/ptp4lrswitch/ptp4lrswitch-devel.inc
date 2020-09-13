#
# REE loccl development repository override
# Internal use only - Repository unaccessable from outside REE
# Do not distribute.
#

RENESAS_RSWITCHTOOL_URL = "git://git@ree-dusgitlab.ree.adwin.renesas.com/r-car-gen3-yocto/vc-tools/ptp4lrswitch.git"
BRANCH = "master"

# Use latest version
SRCREV = "${AUTOREV}"


SRC_URI = "${RENESAS_RSWITCHTOOL_URL};protocol=ssh;branch=${BRANCH}"
S = "${WORKDIR}/git/"


