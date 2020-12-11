COMPATIBLE_MACHINE = "h3vc"

IMAGE_INSTALL_append = " usbutils "
IMAGE_INSTALL_append = " gdb gdbserver openssh openssh-sftp openssh-sftp-server "

IMAGE_INSTALL_append = " sllin phytool inic-flasher wl18xx-fw "

# Vehicle Computer releated packages
IMAGE_INSTALL_append = " \
    vc-build-info \
    snott \
    rlatency \
    ptp4lrswitch \
    gptp-master \
    rexecd \
    vc-addon-script \
    flash-fpga \
    fpga-programmer \
"

# R-Switch1 releated packages
RSWITCH1_PKGS = " \
    rswitch1-startup \
    rswitch1tool \
"

# R-Switch1 releated packages
RSWITCH2_PKGS = " \
    rswitch2-startup \
    rswitch2tool \
    rswitch2-appnote \
"

IMAGE_INSTALL_append= " \
    ${@bb.utils.contains('DISTRO_FEATURES','rswitch1','${RSWITCH1_PKGS}','${RSWITCH2_PKGS}',d)} \
"

# Wifi Support
IMAGE_INSTALL_append= " \
    ti-bt-firmware \
    wl18xx-calibrator \
    wlconf \
    wl18xx-target-scripts \
    iw \
    wireless-tools \
    wpa-supplicant \
    hostapd \
"

# Networking tools
IMAGE_INSTALL_append = " \
    ethtool \
    bridge-utils \
    vlan \
    netcat \
    tcpdump \
    net-tools \
    iputils \
    eth-avb-conf \
    iproute2 \
"

# Development tools
IMAGE_INSTALL_append = " \
    devdump \
    nano \
    git \
    subversion \
    python3 \
    python-numpy \
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
"

# Test tools
IMAGE_INSTALL_append = " \
    fb-test \
"

# Flash tool
PREFERRED_VERSION_flashrom = "1.2"
IMAGE_INSTALL_append = " flashrom"

# Install modules into RFS
CORE_IMAGE_EXTRA_INSTALL += " kernel-modules "

# Install kernel and DTB
IMAGE_INSTALL_append = " kernel-image kernel-devicetree "

# Install default u-boot environment file
IMAGE_INSTALL_append = " u-boot-env "
