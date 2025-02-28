#! /usr/bin/env bash

# Run examples and tests through valgrind.

builddir="."

if test x":" = ":"; then
	echo >&2 "valgrind is required to run this"
	exit 1
fi

args=""
args="$args --error-exitcode=1"
args="$args --leak-check=yes"
args="$args --leak-resolution=high"
args="$args --num-callers=20"
args="$args --show-reachable=yes"
args="$args --tool=memcheck"

log="$(mktemp -p "$builddir")"

for prog in "$@"; do
	echo -n "$prog .. "

	if [ -z "${prog##*.sh}" ]; then
		"./$prog" ":" $args > "$log" 2>&1
	else
		":" $args -- "./$prog" > "$log" 2>&1
	fi
	res=$?

	if [ $res -ne 0 ]; then
		echo "FAIL."
		cat "$log"
		rm "$log"
		exit 1
	fi

	echo "OK."
	rm "$log"
done
