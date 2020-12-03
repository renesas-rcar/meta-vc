RENESAS_BSP_URL = "git://git@ree-dusgitlab.ree.adwin.renesas.com/r-car-gen3-yocto/renesas-linux.git"
BRANCH = "master"
SRC_URI = "${RENESAS_BSP_URL};protocol=ssh;nocheckout=1;branch=${BRANCH}"

# Always use latest version
SRCREV = "${AUTOREV}"

# Uncomment and customize the following variables if you need a kernel
# configuration different from the one in linux-renesas_4.14.bbappend
#PATCHES = ""
#HYPERFLASH_CONFIG = ""
#CONFIG = ""
