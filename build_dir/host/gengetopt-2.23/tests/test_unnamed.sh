#! /usr/bin/env bash

set -ex

"$@" ./test_unnamed --help | grep -F '[FILE]...'
