do_configure_prepend () {

    #Allow build paths with containing AU.
    sed 's|AC_CANONICAL_SYSTEM|m4_pattern_allow([^AU_])\nAC_CANONICAL_SYSTEM|' -i ${S}/configure.*
}
