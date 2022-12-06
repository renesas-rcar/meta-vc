RENESAS_BSP_URL = "git://git@ree-dusgitlab.ree.adwin.renesas.com/r-car-gen3-yocto/renesas-linux.git"

BRANCH = "vc4"
#BRANCH = "205-add-dedicated-phy-for-marvell-m88q3344-engineering-sample"
SRC_URI = "${RENESAS_BSP_URL};protocol=ssh;nocheckout=1;branch=${BRANCH}"


#BRANCH = "137-activate-avb-port-2"
#SRC_URI = "git:///home/cmard/yocto/vc4/renesas-linux-cogent;protocol=file;branch=${BRANCH}"

# Always use the latest version
SRCREV = "${AUTOREV}"

PATCHES = ""

