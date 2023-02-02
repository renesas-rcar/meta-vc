#
# REE loccl development repository override
# Internal use only - Repository unaccessable from outside REE
# Do not distribute.
#

RENESAS_RSWITCH1TOOL_URL = "git://git@gitlab.eur.eng.idt.com:2222/automotive-network-section-ans/vehicle-computer-vc/vc-tools/rswitch1tool.git"
SRC_URI = "${RENESAS_RSWITCH1TOOL_URL};protocol=ssh;branch=${BRANCH}"
BRANCH = "master"

# Use latest version
SRCREV = "${AUTOREV}"

# PV needs to be updated with ${SRCPV}, otherwise no upstream changes are detected
PV .= "+git${SRCPV}"

S = "${WORKDIR}/git/"


