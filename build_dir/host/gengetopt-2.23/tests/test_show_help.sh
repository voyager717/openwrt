#! /usr/bin/env bash

set -ex

gengetopt="$(cd ../src; pwd)/gengetopt"

"$@" "$gengetopt" --show-help -i "./test_all_opts_cmd.ggo"
"$@" "$gengetopt" --show-version -i "./test_all_opts_cmd.ggo"
"$@" "$gengetopt" --show-help -i "./test_values_cmd.ggo"
