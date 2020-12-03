#
# REE loccl development repository override
# Internal use only - Repository unaccessable from outside REE
# Do not distribute.
#

RENESAS_RSWITCHTOOL_URL = "git://git@ree-dusgitlab.ree.adwin.renesas.com/r-car-gen3-yocto/vc-tools/rexecd.git"
SRC_URI = "${RENESAS_RSWITCHTOOL_URL};protocol=ssh;branch=${BRANCH}"
BRANCH = "master"

# Use latest version
SRCREV = "${AUTOREV}"

# PV needs to be updated with ${SRCPV}, otherwise no upstream changes are detected
PV .= "+git${SRCPV}"


S = "${WORKDIR}/git/"


