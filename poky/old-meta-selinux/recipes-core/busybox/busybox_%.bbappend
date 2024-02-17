PR .= ".1"

FILES_${PN} += "${libdir}/${PN}"

# We should use sh wrappers instead of links so the commands could get correct
# security labels
python create_sh_wrapper_reset_alternative_vars () {
    # We need to load the full set of busybox provides from the /etc/busybox.links
    # Use this to see the update-alternatives with the right information

    dvar = d.getVar('D', True)
    pn = d.getVar('PN', True)

    def create_sh_alternative_vars(links, target, mode):
        import shutil
        # Create sh wrapper template
        fwp = open("busybox_wrapper", 'w')
        fwp.write("#!%s" % (target))
        os.fchmod(fwp.fileno(), mode)
        fwp.close()
        # Install the sh wrappers and alternatives reset to link to them
        wpdir = os.path.join(d.getVar('libdir', True), pn)
        wpdir_dest = '%s%s' % (dvar, wpdir)
        if not os.path.exists(wpdir_dest):
            os.makedirs(wpdir_dest)
        f = open('%s%s' % (dvar, links), 'r')
        for alt_link_name in f:
            alt_link_name = alt_link_name.strip()
            alt_name = os.path.basename(alt_link_name)
            # Copy script wrapper to wp_path
            alt_wppath = '%s%s' % (wpdir, alt_link_name)
            alt_wppath_dest = '%s%s' % (wpdir_dest, alt_link_name) 
            alt_wpdir_dest = os.path.dirname(alt_wppath_dest)
            if not os.path.exists(alt_wpdir_dest):
                os.makedirs(alt_wpdir_dest)
            shutil.copy2("busybox_wrapper", alt_wppath_dest)
            # Re-set alternatives
            # Match coreutils
            if alt_name == '[':
                alt_name = 'lbracket'
            d.appendVar('ALTERNATIVE_%s' % (pn), ' ' + alt_name)
            d.setVarFlag('ALTERNATIVE_LINK_NAME', alt_name, alt_link_name)
            if os.path.exists(alt_wppath_dest):
                d.setVarFlag('ALTERNATIVE_TARGET', alt_name, alt_wppath)
        f.close()

        os.remove("busybox_wrapper")
        return

    if os.path.exists('%s/etc/busybox.links' % (dvar)):
        create_sh_alternative_vars("/etc/busybox.links", "/bin/busybox", 0o0755)
    else:
        create_sh_alternative_vars("/etc/busybox.links.nosuid", "/bin/busybox.nosuid", 0o0755)
        create_sh_alternative_vars("/etc/busybox.links.suid", "/bin/busybox.suid", 0o4755)
}

# Add to PACKAGEBUILDPKGD so it could override the alternatives, which are set in
# do_package_prepend() section of busybox_*.bb.
PACKAGEBUILDPKGD_prepend = "create_sh_wrapper_reset_alternative_vars "

# Use sh wrappers instead of links
pkg_postinst_${PN} () {
	# This part of code is dedicated to the on target upgrade problem.
	# It's known that if we don't make appropriate symlinks before update-alternatives calls,
	# there will be errors indicating missing commands such as 'sed'.
	# These symlinks will later be updated by update-alternatives calls.
	test -n 2 > /dev/null || alias test='busybox test'
	if test "x$D" = "x"; then
		# Remove busybox.nosuid if it's a symlink, because this situation indicates
		# that we're installing or upgrading to a one-binary busybox.
		if test -h /bin/busybox.nosuid; then
			rm -f /bin/busybox.nosuid
		fi
		for suffix in "" ".nosuid" ".suid"; do
			if test -e /etc/busybox.links$suffix; then
				while read link; do
					if test ! -e "$link"; then
						# we can use busybox here because even if we are using splitted busybox
						# we've made a symlink from /bin/busybox to /bin/busybox.nosuid.
						busybox echo "#!/bin/busybox$suffix" > $link
					fi
				done < /etc/busybox.links$suffix
			fi
		done
	fi
}

