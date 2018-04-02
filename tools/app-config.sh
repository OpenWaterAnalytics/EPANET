#! /bin/bash

# 
#  app-config.sh - Generates nrtest app configuration file for test executable
#
#  Date Created: 3/19/2018
#
#  Author:       Michael E. Tryby
#                US EPA - ORD/NRMRL
#
#  Arguments:
#    1 - absolute path to test executable
# 
#  NOT IMPLEMENTED YET
#    2 - test executable version number
#    3 - build description
#

unameOut="$(uname -s)"
case "${unameOut}" in
    Linux*)     ;&
    Darwin*)    abs_build_path=$1
                test_cmd="runepanet"
                ;;

    MINGW*)     ;&
    MSYS*)      # Remove leading '/c' from file path for nrtest
                abs_build_path="$( echo "$1" | sed -e 's#/c##' )"
                test_cmd="runepanet.exe"
                ;;
				
    *)          # Machine unknown
esac

version=""
build_description=""

cat<<EOF
{
    "name" : "epanet",
    "version" : "${version}",
    "description" : "${build_description}", 
    "setup_script" : "",
    "exe" : "${abs_build_path}/${test_cmd}"
}
EOF
