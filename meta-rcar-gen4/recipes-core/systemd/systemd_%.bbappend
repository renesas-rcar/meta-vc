FILESEXTRAPATHS_prepend := "${THISDIR}/${PN}:"
PR .= ".vc1"

SRC_URI += "file://0100-ethtool-Add-support-for-master-slave-link-setting.patch \
           "
