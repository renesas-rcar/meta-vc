do_init_submodule_prepend() {
    cd ${S}
    sed -i 's,url = https://anongit.freedesktop.org/git/gstreamer/common,url = https://gitlab.freedesktop.org/gstreamer/common,g' ./.gitmodules
}

