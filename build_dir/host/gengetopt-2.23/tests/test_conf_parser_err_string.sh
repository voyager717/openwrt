#! /usr/bin/env bash

set -ex

! "$@" ./test_conf_parser -r "bar" -a 100 -c "./test_conf_err_string.conf"
! "$@" ./test_conf_parser -r "bar" -a 100 -c "./test_conf_err_string2.conf"
