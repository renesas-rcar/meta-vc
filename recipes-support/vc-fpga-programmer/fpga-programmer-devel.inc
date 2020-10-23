RENESAS_FPGA_PROGRAMMER_URL = "git://git@ree-dusgitlab.ree.adwin.renesas.com/r-car-gen3-yocto/vc-tools/vcxfpgaprogrammer.git"
BRANCH = "master"

# Use latest version
SRCREV = "${AUTOREV}"

SRC_URI = "${RENESAS_FPGA_PROGRAMMER_URL};protocol=ssh;branch=${BRANCH}"
S = "${WORKDIR}/git/"


