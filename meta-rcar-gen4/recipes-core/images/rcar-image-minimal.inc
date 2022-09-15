IMAGE_INSTALL_append = " gdb gdbserver openssh openssh-sftp openssh-sftp-server "

IMAGE_INSTALL_append = " phytool "

# Networking tools
IMAGE_INSTALL_append = " \
    ethtool \
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
    strace \
    e2fsprogs-mke2fs \
    i2c-tools \
    util-linux \
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
"

# Configuration and addons
IMAGE_INSTALL_append = " \
    u-boot-vc4env \
    vc-addon-script \
"

# Install modules into RFS
CORE_IMAGE_EXTRA_INSTALL += " kernel-modules "

# Install kernel and DTB
IMAGE_INSTALL_append = " kernel-image kernel-devicetree "
