#common project used in plugin bb.

do_patch_append() {
    import os
    os.system("cd ${S} && rm -rf common")
    os.system("cd ${S} && touch NEWS && touch AUTHORS && touch ChangeLog && touch README && touch REQUIREMENTS")
    cmd = "cd ${S} && git clone -b gstreamer/common/master https://source.codeaurora.org/quic/le/gstreamer/common"
    os.system(cmd)
}
