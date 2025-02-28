#! /usr/bin/env bash

set -ex

"$@" ./test_conf_parser_ov2 -r "bar" --float 2.14 -i 100 -c \
	"./test_conf2.conf"
