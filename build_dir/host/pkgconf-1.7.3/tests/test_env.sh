srcdir="$(atf_get_srcdir)"
export PATH="$srcdir/..:${PATH}"

#--- begin windows kludge ---
# When building with Visual Studio, binaries are in a subdirectory named after the configration...
# and the configuration is not known unless you're in the IDE, or something.
# So just guess.  This won't work well if you build more than one configuration.
the_configuration=""
for configuration in Debug Release RelWithDebInfo
do
    if test -d "$srcdir/../$configuration"
    then
        if test "$the_configuration" != ""
        then
            echo "test_env.sh: FAIL: more than one configuration found"
            exit 1
        fi
        the_configuration=$configuration
        export PATH="$srcdir/../${configuration}:${PATH}"
    fi
done
#--- end kludge ---

selfdir="/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/host/pkgconf-1.7.3/tests"
LIBRARY_PATH_ENV="LIBRARY_PATH"
PATH_SEP=":"
SYSROOT_DIR="${selfdir}/test"
case "$(uname -s)" in
Msys|CYGWIN*) PATH_SEP=";";;
Haiku) LIBRARY_PATH_ENV="BELIBRARIES";;
esac

prefix="/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/staging_dir/host"
exec_prefix="/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/staging_dir/host"
datarootdir="${prefix}/share"
pcpath="${exec_prefix}/lib/pkgconfig:${datarootdir}/pkgconfig"

tests_init()
{
	TESTS="$@"
	export TESTS
	for t ; do
		atf_test_case $t
	done
}

atf_init_test_cases() {
	for t in ${TESTS}; do
		atf_add_test_case $t
	done
}
