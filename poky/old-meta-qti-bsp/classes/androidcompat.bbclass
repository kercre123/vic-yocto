LA_COMPAT_DIR = "${TMPDIR}/work-shared/${BASEMACHINE}/android_compat"

S = "${LA_COMPAT_DIR}"

do_unpack[cleandirs] += " ${S} ${LA_COMPAT_DIR}"
do_clean[cleandirs] += " ${S} ${LA_COMPAT_DIR}"

base_do_unpack_append () {
    s = d.getVar("S", True)
    if s[-1] == '/':
        # drop trailing slash, so that os.symlink(compat_dir, s) doesn't use s as directory name and fail
        s=s[:-1]
    compat_dir = d.getVar("LA_COMPAT_DIR", True)
    if s != compat_dir:
        bb.utils.mkdirhier(compat_dir)
        bb.utils.remove(compat_dir, recurse=True)
        import shutil
        shutil.move(s, compat_dir)
        os.symlink(compat_dir, s)
}
