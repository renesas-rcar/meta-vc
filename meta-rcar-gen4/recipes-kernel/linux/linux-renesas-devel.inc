RENESAS_BSP_URL = "git://git@ree-dusgitlab.ree.adwin.renesas.com/r-car-gen3-yocto/renesas-linux.git"
BRANCH = "vc4"
#BRANCH = "vc4_for_cmard"
#BRANCH = "vc4_net_phys"
#BRANCH = "151-correct-dt-for-2g5-and-rh"
#BRANCH = "158-add-stats"

SRC_URI = "${RENESAS_BSP_URL};protocol=ssh;nocheckout=1;branch=${BRANCH}"

#BRANCH = "vc4-nikita-intswcd-test"
#BRANCH = "137-activate-avb-port-2"
#SRC_URI = "git:///home/cmard/yocto/vc4/renesas-linux-cogent;protocol=file;branch=${BRANCH}"

# Always use the latest version
SRCREV = "${AUTOREV}"

PATCHES = ""

