# Add VC specific build information files
DESCRIPTION = "VC specific build information"
SECTION = "vc-build-info"


LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://${COMMON_LICENSE_DIR}/MIT;md5=0835ade698e0bcf8506ecda2f7b4f302"

def get_layers(bb, d):
    layers = (d.getVar("BBLAYERS", 1) or "").split()
    layers_branch_rev = ["%-17s = \"%s:%s\"" % (os.path.basename(i), \
        base_get_metadata_git_branch(i, None).strip().strip('()'), \
        base_get_metadata_git_revision(i, None)) \
        for i in layers]
    i = len(layers_branch_rev)-1
    p1 = layers_branch_rev[i].find("=")
    s1= layers_branch_rev[i][p1:]
    while i > 0:
        p2 = layers_branch_rev[i-1].find("=")
        s2= layers_branch_rev[i-1][p2:]
        if s1 == s2:
            layers_branch_rev[i-1] = layers_branch_rev[i-1][0:p2]
            i -= 1
        else:
            i -= 1
            p1 = layers_branch_rev[i].find("=")
            s1= layers_branch_rev[i][p1:]

    layertext = "Configured layers:\n%s\n" % '\n'.join(layers_branch_rev)
    layertext = layertext.replace('<','')
    layertext = layertext.replace('>','')
    return layertext

# install it in etc
do_install() {
    install -d ${D}/etc
    echo "This image has been build on $(date) by ${USER} on $(hostname)\n" > ${D}/etc/build-info.txt
    echo "${@get_layers(bb, d)}" >> ${D}/etc/build-info.txt
}

# package it as it is not installed in a standard location
FILES_${PN} = "/etc/build-info.txt"

