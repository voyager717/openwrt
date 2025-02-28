#!/bin/bash
source ./autobuild/lede-build-sanity.sh

#get the brach_name
temp=${0%/*}
branch_name=${temp##*/}
#step1 clean
#clean

#step2 choose which .config

#do prepare stuff
prepare

#prepare mtk jedi wifi stuff
prepare_mtwifi ${branch_name}

prepare_final ${branch_name}
#step3 build
if [ -z ${1} ] || [ $1 = "6E" ]; then
	build ${branch_name} -j1 || [ "$LOCAL" != "1" ]
fi
