#! /usr/bin/env bash

set -ex

"$@" ./test_unamed --help | grep -F '[FILE]...'
