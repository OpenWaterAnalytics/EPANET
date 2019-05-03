#! /bin/bash

#
#  gen-config.sh - Generates nrtest app configuration file for test executable
#
#  Date Created: 10/16/2017
#
#  Author:       Michael E. Tryby
#                US EPA - ORD/NRMRL
#
#  Arguments:
#    1 - absolute path to test executable
#    2 - platform
#    3 - SUT build id
#    4 - SUT version id
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

cat<<EOF
{
    "name" : "epanet",
    "version" : "$4",
    "description" : "$2 $3", 
    "setup_script" : "",
    "exe" : "${abs_build_path}/${test_cmd}"
}
EOF
