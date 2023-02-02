RENESAS_FPGA_PROGRAMMER_URL = "git://git@gitlab.eur.eng.idt.com:2222/automotive-network-section-ans/vehicle-computer-vc/vc-tools/vcxfpgaprogrammer.git"
BRANCH = "master"

# Use latest version
SRCREV = "${AUTOREV}"

# PV needs to be updated with ${SRCPV}, otherwise no upstream changes are detected
PV .= "+git${SRCPV}"

SRC_URI = "${RENESAS_FPGA_PROGRAMMER_URL};protocol=ssh;branch=${BRANCH}"
S = "${WORKDIR}/git/"


