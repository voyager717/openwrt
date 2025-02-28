#! /usr/bin/env bash

set -ex

! "$@" ./test_conf_parser -r "bar" -i 100 -c "./test_conf_err.conf" \
	--opta "FOO"
