IMAGE_INSTALL_append = " gdb gdbserver openssh openssh-sftp openssh-sftp-server "

IMAGE_INSTALL_append = " man man-pages"

GLIBC_GENERATE_LOCALES="en_US.UTF-8"
IMAGE_LINGUAS="en-us"


# Networking tools
IMAGE_INSTALL_append = " \
    ethtool \
    phytool \
    bridge-utils \
    vlan \
    netcat \
    tcpdump \
    net-tools \
    iputils \
    iproute2 \
    snott \
    packeth \
    iperf2 \
    iperf3 \
    linuxptp \
    gptp-avnu \
    sshfs-fuse \
    mka-daemon \
"

# Development tools
IMAGE_INSTALL_append = " \
    devmem2 \
    devdump \
    rsw2dump \
    nano \
    git \
    subversion \
    python3 \
    python3-numpy \
    iftop \
    htop \
    tmux \
    strace \
    e2fsprogs-mke2fs \
    i2c-tools \
    util-linux \
    hexedit \
    procps \
    pciutils \
    dosfstools \
    can-utils \
    mtd-utils \
    lrzsz \
    rpm \
    tcl \
    python3-scapy \
    stress \
    perf \
    dtc \
"

# Prebuild binary packages
IMAGE_INSTALL_append = " \
     marvell-88q211x \
     marvell-88q3344-es \
     lan865x \
"


# Configuration and addons
IMAGE_INSTALL_append = " \
    u-boot-vc4env \
    vc-addon-script \
    vc-addon-script-doc \
"

# Install modules into RFS
CORE_IMAGE_EXTRA_INSTALL += " kernel-modules "

# Install kernel and DTB
IMAGE_INSTALL_append = " kernel-image kernel-devicetree "
